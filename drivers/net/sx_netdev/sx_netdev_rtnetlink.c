/*
 * SPDX-FileCopyrightText: Copyright (c) 2010-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/etherdevice.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/netdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/tcp.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <net/ip.h>
#include <linux/vmalloc.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/rtnetlink.h>

#include <linux/mlx_sx/cmd.h>
#include <linux/mlx_sx/device.h>
#include <linux/mlx_sx/driver.h>
#include <linux/mlx_sx/cmd.h>
#include <linux/mlx_sx/kernel_user.h>
#include "sx_netdev.h"
#include <linux/mlx_sx/device.h>
#include <linux/mlx_sx/auto_registers/reg.h>
#include <linux/mlx_sx/auto_registers/cmd_auto.h>

#if defined(CONFIG_NET_PORT) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)))
#include <linux/port.h>
#endif

int two_phase_port_init = 0;
#if defined(CONFIG_NET_PORT)
module_param_named(two_phase_port_init, two_phase_port_init, int, 0644);
MODULE_PARM_DESC(two_phase_port_init, "en/dis two phase port initialization");
#endif

/* module argument defined in sx_netdev_main.c */
extern int single_netdev_per_port_enable;

typedef struct oper_state_wdata {
    struct sx_net_priv *net_priv;
    struct completion   wq_completion;
    struct work_struct  work;
} oper_state_wdata_t;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) || defined(CONFIG_SX_RHEL_8_6)
void sx_netdev_setup(struct net_device *dev)
#else
void sx_netdev_setup(struct net_device *dev)
#endif
{
    struct sx_net_priv *net_priv = netdev_priv(dev);

    sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: called for %s\n", __func__, dev->name);

    memset(net_priv, 0, sizeof(*net_priv));
    ether_setup(dev);
    dev->hard_header_len = ETH_HLEN + ISX_HDR_SIZE;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
    dev->max_mtu = 0; /* do not limit the max MTU */
    dev->needs_free_netdev = true;
#endif
    net_priv->netdev = dev;

    sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: exit for %s\n", __func__, dev->name);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
static int sx_netdev_validate(struct nlattr *tb[], struct nlattr *data[], struct netlink_ext_ack *extack)
#else
static int sx_netdev_validate(struct nlattr *tb[], struct nlattr *data[])
#endif
{
    __u32                      id;
    int                        is_lag;
    int                        err = 0;
    uint16_t                   lag_max = 0, lag_member_max = 0;
    struct sx_dev_netdev_priv *priv = NULL;

    sxd_log_debug(SXD_DEV_NONE, "%s: called\n", __func__);

    if (tb[IFLA_ADDRESS]) {
        if (nla_len(tb[IFLA_ADDRESS]) != ETH_ALEN) {
            sxd_log_debug(SXD_DEV_NONE, "invalid link address (not ethernet)\n");
            err = -EINVAL;
            goto out;
        }

        if (!is_valid_ether_addr(nla_data(tb[IFLA_ADDRESS]))) {
            sxd_log_debug(SXD_DEV_NONE, "invalid all zero ethernet address\n");
            err = -EADDRNOTAVAIL;
            goto out;
        }
    }

    if (!data) {
        sxd_log_info(SXD_DEV_NONE, "%s: Data invalid\n", __func__);
        err = -EINVAL;
        goto out;
    }

    if (!data[IFLA_SX_NETDEV_SWID]) {
        sxd_log_info(SXD_DEV_NONE, "%s: SWID doesn't exists\n", __func__);
        err = -EINVAL;
        goto out;
    }

    id = nla_get_u32(data[IFLA_SX_NETDEV_SWID]);
    if (id >= NUMBER_OF_SWIDS) {
        sxd_log_info(SXD_DEV_NONE, "%s: SWID is out of range - %d\n", __func__, id);
        err = -ERANGE;
        goto out;
    }

    if (data[IFLA_SX_NETDEV_PIPELINE]) {
        id = nla_get_u32(data[IFLA_SX_NETDEV_PIPELINE]);
        if (id >= SX_NETDEV_PIPELINE_MAX) {
            sxd_log_err(SXD_DEV_NONE, "pipeline is out of range - %d\n", id);
            err = -ERANGE;
            goto out;
        }
    }

    if (data[IFLA_SX_NETDEV_PORT]) {
        priv = dev_netdev_priv_from_curr_net_namespace();

        if (!priv) {
            return -ENODEV;
        }

        id = translate_user_port_to_sysport(priv->dev, nla_get_u32(data[IFLA_SX_NETDEV_PORT]), &is_lag);
        if (id > MAX_SYSPORT_NUM) {
            sxd_log_info(SXD_DEV_FROM_NETDEV_PRIV(priv), "%s: PORT is out of range - %d\n", __func__, id);
            err = -ERANGE;
            goto out;
        }

        if (is_lag) {
            err = sx_core_get_lag_max(priv->dev, &lag_max, &lag_member_max);
            if (err) {
                sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(priv), "Failed to get max number of LAGs. err = %d.\n", err);
                goto out;
            }

            if (id >= lag_max) {
                sxd_log_info(SXD_DEV_FROM_NETDEV_PRIV(priv), "%s: LAG is out of range - %d\n", __func__, id);
                err = -ERANGE;
                goto out;
            }
        }
    }

out:
    if (priv) {
        dev_netdev_priv_put(priv);
    }
    sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(priv), "%s: exit\n", __func__);
    return err;
}

static size_t sx_netdev_get_size(const struct net_device *dev)
{
    return nla_total_size(sizeof(__u32)) +  /* IFLA_SX_NETDEV_SWID */
           nla_total_size(sizeof(__u32)) + /* IFLA_SX_NETDEV_PORT */
           nla_total_size(sizeof(__u32)) + /* IFLA_SX_NETDEV_PIPELINE */
           0;
}

static int sx_netdev_fill_info(struct sk_buff *skb, const struct net_device *dev)
{
    struct sx_net_priv *net_priv = netdev_priv(dev);

    if (nla_put_u32(skb, IFLA_SX_NETDEV_SWID, net_priv->swid)) {
        goto nla_put_failure;
    }

    if (nla_put_u32(skb, IFLA_SX_NETDEV_PORT, net_priv->log_port)) {
        goto nla_put_failure;
    }

    if (nla_put_u32(skb, IFLA_SX_NETDEV_PIPELINE, net_priv->pipeline)) {
        goto nla_put_failure;
    }
    return 0;

nla_put_failure:
    return -EMSGSIZE;
}

static int sx_netdev_port_mac_get(struct sx_dev *dev, u32 log_port, u64 *mac)
{
    int                        err = 0;
    u8                         dev_addr[ETH_ALEN];
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return -ENODEV;
    }

    memset(&dev_addr, 0, sizeof(dev_addr));

    /* Read MAC address */
    if (SX_PORT_TYPE_ID_GET(log_port) == SX_PORT_TYPE_LAG) {
        *mac = 0;
    } else { /* the port is a system port */
        err = sx_core_get_port_mac_db(dev, SX_PORT_PHY_ID_GET(log_port), mac);
        if (err) {
            sxd_log_info(dev,
                         "%s: Unable to get port 0x%x MAC address, err = %d\n",
                         __func__,
                         log_port,
                         err);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0))
            eth_random_addr(dev_addr);
#else
            random_ether_addr(dev_addr);
#endif
            *mac = sx_netdev_mac_to_u64(dev_addr);
        }
    }

    dev_netdev_priv_put(priv);
    return err;
}

static void __sx_netdev_oper_state_get_work_handler(struct work_struct *work_p)
{
    oper_state_wdata_t *wdata_p = container_of(work_p, oper_state_wdata_t, work);
    struct sx_net_priv *net_priv = wdata_p->net_priv;
    int                 err = 0;
    u8                  oper_state = 0;

    /* Get operational state */
    err = sx_netdev_oper_state_get(net_priv->dev, net_priv->log_port, &oper_state);
    if (err) {
        sxd_log_info(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: Unable to get port state, port = %d,"
                     " is_lag = %d, err = %d\n", __func__, net_priv->port, net_priv->is_lag, err);
        oper_state = 0;
    }
    net_priv->is_oper_state_up = oper_state;

    complete(&wdata_p->wq_completion);
}

static int sx_netdev_port_init(struct net_device *dev,
                               struct nlattr     *tb[],
                               struct nlattr     *data[],
                               struct sx_dev    * sx_dev,
                               bool               newlink)
{
    struct sx_net_priv        *net_priv = netdev_priv(dev);
    int                        swid = 0;
    int                        err = 0;
    int                        i = 0;
    u32                        logical_port = 0;
    u8                         uc_type = 0;
    u16                        sys_port = 0;
    u64                        mac = 0;
    u8                         port_type = 0;
    oper_state_wdata_t         wdata;
    u32                        pipeline = SX_NETDEV_PIPELINE_ADAPTIVE;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(sx_dev);

    sxd_log_debug(sx_dev, "%s: called for %s\n", __func__, dev->name);

    if (!priv) {
        return -ENODEV;
    }

    if (!data[IFLA_SX_NETDEV_SWID]) {
        err = -EINVAL;
        goto out;
    }

    if (!data[IFLA_SX_NETDEV_PORT]) {
        err = -EINVAL;
        goto out;
    }

    if (data[IFLA_SX_NETDEV_PIPELINE]) {
        pipeline = nla_get_u32(data[IFLA_SX_NETDEV_PIPELINE]);
        if (pipeline >= SX_NETDEV_PIPELINE_MAX) {
            sxd_log_err(sx_dev, "pipeline is out of range - %d\n", pipeline);
            err = -EINVAL;
            goto out;
        }
        sxd_log_debug(sx_dev, "%s: pipeline is %d\n", __func__, pipeline);
    }

    swid = nla_get_u32(data[IFLA_SX_NETDEV_SWID]);
    if (swid >= NUMBER_OF_SWIDS) {
        sxd_log_info(sx_dev, "%s: SWID is out of range - %d\n", __func__, swid);
        err = -EINVAL;
        goto out;
    }

    if ((swid != ROUTER_PORT_SWID) && (!priv || !priv->netdev_resources.sx_netdevs[swid])) {
        sxd_log_info(sx_dev, "%s: SWID %d, doesn't exists\n", __func__, swid);
        err = -EINVAL;
        goto out;
    }
    logical_port = nla_get_u32(data[IFLA_SX_NETDEV_PORT]);

    net_priv->swid = swid;
    net_priv->dev = priv->dev;
    net_priv->pipeline = pipeline;

    sys_port = translate_user_port_to_sysport(priv->dev,
                                              logical_port,
                                              &net_priv->is_lag);
    net_priv->port = sys_port;
    net_priv->log_port = logical_port;
    net_priv->is_port_netdev = 1;
    err = sx_core_get_send_to_port_as_data(net_priv->dev,
                                           &net_priv->send_to_port_as_data_supported,
                                           1);
    if (err) {
        sxd_log_err(sx_dev, "sx_core_get_send_to_port_as_data failed, err: %d \n", err);
        goto out;
    }

    /* mark as alloc in DB */
    port_type = net_priv->is_lag ? PORT_TYPE_LAG : PORT_TYPE_SINGLE;
    mutex_lock(&priv->netdev_resources.rsc_lock);
    if (priv->netdev_resources.port_allocated[port_type][net_priv->port] == MAX_PORT_NETDEV_NUM) {
        mutex_unlock(&priv->netdev_resources.rsc_lock);
        sxd_log_err(sx_dev, "No more resources - Max Net Device for port %u was already"
                    " allocated\n", sys_port);
        err = -EEXIST;
        goto out;
    }

    if (single_netdev_per_port_enable) {
        priv->netdev_resources.port_allocated[port_type][net_priv->port] = 1;
    } else {
        priv->netdev_resources.port_allocated[port_type][net_priv->port]++;
    }

    mutex_unlock(&priv->netdev_resources.rsc_lock);

    /* Get MAC address */
    err = sx_netdev_port_mac_get(net_priv->dev, logical_port, &mac);
    if (err) {
        sxd_log_debug(sx_dev, "%s: Unable to get mac, port = %d,"
                      " is_lag = %d, err = %d\n", __func__, net_priv->port, net_priv->is_lag, err);
        mac = 0;
    }
    net_priv->mac = mac;

    net_priv->skip_tunnel = priv->skip_tunnel;

    sxd_log_debug(sx_dev, "%s: Newly device %s log port 0x%x MAC address = %llx\n",
                  __func__,
                  dev->name,
                  logical_port,
                  net_priv->mac);

    /* Init trap DB */
    for (uc_type = USER_CHANNEL_L3_NETDEV; uc_type < NUM_OF_NET_DEV_TYPE; uc_type++) {
        for (i = 0; i < MAX_NUM_TRAPS_TO_REGISTER; i++) {
            net_priv->trap_ids[uc_type][i].synd = SX_INVALID_TRAP_ID;
        }
        net_priv->num_of_traps[uc_type] = 0;
    }

    /* Get LAG mid */
    if (net_priv->is_lag) {
        err = sx_core_get_lag_mid(net_priv->dev, net_priv->port, &(net_priv->mid));
        if (err) {
            sxd_log_info(sx_dev, "%s: sx_core_get_lag_mid() failed error - %d\n", __func__, err);
            net_priv->mid = net_priv->port + 0x100;
        }

        net_priv->module_present = SX_MODULE_PRESENT_INVALID;
    } else {
        /* Get module status */
        err = sx_core_get_port_module_present_db(net_priv->dev,
                                                 net_priv->port,
                                                 &net_priv->module_present);
        if (err) {
            sxd_log_info(sx_dev,
                         "%s: Failed to get port 0x%x module present from DB, error - %d\n",
                         __func__,
                         net_priv->log_port,
                         err);
            goto out;
        }
    }


    if (newlink && !two_phase_port_init) {
        /* Register device */
        err = sx_netdev_register_device(dev, 0, 0);
        if (err) {
            sxd_log_info(sx_dev, "%s: sx_netdev_register_device() failed error - %d\n", __func__,
                         err);
            mutex_lock(&priv->netdev_resources.rsc_lock);
            priv->netdev_resources.port_allocated[port_type][net_priv->port]--;
            mutex_unlock(&priv->netdev_resources.rsc_lock);
            err = -ENXIO;
            goto out;
        }
    }

    mutex_lock(&priv->netdev_resources.rsc_lock);

    /* Add netdev to resources db */
    port_type = net_priv->is_lag ? PORT_TYPE_LAG : PORT_TYPE_SINGLE;
    if (single_netdev_per_port_enable) {
        rcu_assign_pointer(priv->netdev_resources.port_netdev[port_type][net_priv->port][0], dev);
        for (i = 1; i < MAX_PORT_NETDEV_NUM; i++) {
            rcu_assign_pointer(priv->netdev_resources.port_netdev[port_type][net_priv->port][i], NULL);
        }
    } else {
        for (i = 0; i < MAX_PORT_NETDEV_NUM; i++) {
            if (priv->netdev_resources.port_netdev[port_type][net_priv->port][i] == NULL) {
                rcu_assign_pointer(priv->netdev_resources.port_netdev[port_type][net_priv->port][i], dev);
                break;
            }
        }
    }

    if (net_priv->kobj[SX_NET_KOBJECT_PORT] == NULL) {
        /* Add sysfs node for physical port */
        err = sx_netdev_create_port_sysfs_nodes(dev);
        if (err) {
            sxd_log_err(sx_dev,
                        "%s: sx_netdev_create_port_sysfs_nodes() failed error - %d\n",
                        __func__,
                        err);
            mutex_unlock(&priv->netdev_resources.rsc_lock);
            goto out;
        }
    }

    mutex_unlock(&priv->netdev_resources.rsc_lock);

    if (net_priv->is_lag) {
        err = sx_core_get_lag_oper_state(net_priv->dev, net_priv->port, &(net_priv->is_oper_state_up));
        if (err) {
            sxd_log_info(sx_dev, "%s: Unable to get lag [0x%x] oper state."
                         " err = %d\n", __func__, net_priv->log_port, err);
            net_priv->is_oper_state_up = 0;
        }
        sxd_log_info(sx_dev, "%s: LAG port state for 0x%x = %d\n", __func__,
                     net_priv->log_port, net_priv->is_oper_state_up);
    } else {
        /* If carrier_set_on_pude_disable is set to non zero,
         * we don't need to get the port operation state at newlink */
        if (sx_netdev_carrier_set_on_pude_disable_get() == 0) {
            /* The operation state get is done in worker thread to prevent
             * race between PAOS query and PUDE */
            memset(&wdata, 0, sizeof(wdata));
            init_completion(&wdata.wq_completion);
            INIT_WORK(&wdata.work, __sx_netdev_oper_state_get_work_handler);
            wdata.net_priv = net_priv;
            queue_work(priv->netdev_wq, &wdata.work);

            /* Wait for completion for synchronous operation */
            wait_for_completion_interruptible(&wdata.wq_completion);

            if (!newlink) { /* if newlink, sx_netdev_open() that comes right after will call link-state-set */
                netdev_linkstate_set(dev);
            }
        }
    }

#if defined(CONFIG_NET_PORT) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)))
    port_init_ethtool_stats(dev);
#endif

    sxd_log_debug(sx_dev, "%s: exit for %s\n", __func__, dev->name);

out:
    dev_netdev_priv_put(priv);
    return err;
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
static int sx_netdev_newlink(struct net             *net,
                             struct net_device      *dev,
                             struct nlattr          *tb[],
                             struct nlattr          *data[],
                             struct netlink_ext_ack *extack)
#else
static int sx_netdev_newlink(struct net *net, struct net_device *dev, struct nlattr *tb[], struct nlattr *data[])
#endif
{
    struct sx_dev_netdev_priv *priv = NULL;
    int                        err = 0;

    sxd_log_debug(SXD_DEV_NONE, "%s: called for %s\n", __func__, dev->name);

    if (two_phase_port_init) {
        /* Register device */
        err = sx_netdev_register_device(dev, 0, 0);
        if (err) {
            sxd_log_info(SXD_DEV_NONE, "%s: sx_netdev_register_device() failed error - %d\n", __func__, err);
            err = -ENXIO;
            goto out;
        }
    } else {
        /* priv is only needed here; otherwise for CL with two_phase_port_init, allow adding new link before netdev initialization completes */
        priv = dev_netdev_priv_from_curr_net_namespace();

        if (!priv) {
            return -ENODEV;
        }

        if (!priv->sx_core_if.bound) {
            sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(priv),
                        "%s: can't add new port netdevice during sx_core detach/reset. \n",
                        __func__);
            err = -ENXIO;
            goto out;
        }
        sx_netdev_port_init(dev, tb, data, priv->dev, true);
    }

out:
    sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(priv), "%s: exit for %s\n", __func__, dev->name);
    if (priv) {
        dev_netdev_priv_put(priv);
    }
    return err;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
static int sx_netdev_changelink(struct net_device      *dev,
                                struct nlattr          *tb[],
                                struct nlattr          *data[],
                                struct netlink_ext_ack *extack)
#else
static int sx_netdev_changelink(struct net_device *dev, struct nlattr *tb[], struct nlattr *data[])
#endif
{
    int                        err = 0;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_curr_net_namespace();

    sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(priv), "%s: called for %s\n", __func__, dev->name);

    if (!priv) {
        return -ENODEV;
    }

    if (!priv->sx_core_if.bound) {
        sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(priv),
                    "%s: can't change port netdevice during sx_core detach/reset. \n",
                    __func__);
        err = -ENXIO;
        goto out;
    }

    err = sx_netdev_port_init(dev, tb, data, priv->dev, false);
    sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(priv), "%s: exit for %s\n", __func__, dev->name);

out:
    dev_netdev_priv_put(priv);
    return err;
}

static void sx_netdev_dellink(struct net_device *dev, struct list_head *head)
{
    struct sx_net_priv        *net_priv = netdev_priv(dev);
    u8                         port_type = 0, i = 0;
    struct sx_dev_netdev_priv *priv;

    sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: called for %s\n", __func__, dev->name);

    port_type = net_priv->is_lag ? PORT_TYPE_LAG : PORT_TYPE_SINGLE;

    priv = dev_netdev_priv_from_netdev(dev);

    /* Remove sysfs node for physical port */
    sx_netdev_remove_port_sysfs_nodes(dev);

    if (priv) {
        if (!priv->sx_core_if.bound) {
            sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv),
                        "%s: can't delete port netdevice during sx_core detach/reset. \n",
                        __func__);
            goto cont_flow;
        }

        /* rsc_lock is put after sysfs node operations to avoid circular lock from parallel sysfs access */
        mutex_lock(&priv->netdev_resources.rsc_lock);
        if (single_netdev_per_port_enable) {
            for (i = 0; i < MAX_PORT_NETDEV_NUM; i++) {
                rcu_assign_pointer(priv->netdev_resources.port_netdev[port_type][net_priv->port][i], NULL);
            }
        } else {
            for (i = 0; i < MAX_PORT_NETDEV_NUM; i++) {
                if (priv->netdev_resources.port_netdev[port_type][net_priv->port][i] == dev) {
                    rcu_assign_pointer(priv->netdev_resources.port_netdev[port_type][net_priv->port][i], NULL);
                    break;
                }
            }
        }
        priv->netdev_resources.port_allocated[port_type][net_priv->port]--;
        mutex_unlock(&priv->netdev_resources.rsc_lock);
    }

cont_flow:

    /* wait until priv->netdev_resource->port_netdev[port_type][net_priv->port][[0..MAX_PORT_NETDEV_NUM]
     * are no longer in use by RCU readers before we remove the netdevice.
     */
    synchronize_rcu();

    netif_tx_disable(dev);
    netif_carrier_off(dev);

    if (priv) {
        sx_core_flush_synd_by_context(net_priv->dev, dev);
        dev_netdev_priv_put(priv);
    }

    sx_netdev_devlink_port_unregister(dev);
    unregister_netdevice_queue(dev, head);

    sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: exit for %s\n", __func__, dev->name);
}


static const struct nla_policy                 sx_netdev_policy[IFLA_SX_NETDEV_MAX + 1] = {
    [IFLA_SX_NETDEV_SWID] = { .type = NLA_U32 },
    [IFLA_SX_NETDEV_PORT] = { .type = NLA_U32 },
    [IFLA_SX_NETDEV_PIPELINE] = { .type = NLA_U32 },
};
static struct rtnl_link_ops sx_netdev_link_ops __read_mostly = {
    .kind = NETDEV_DRV_NAME,
    .maxtype = IFLA_SX_NETDEV_MAX,
    .policy = sx_netdev_policy,
    .priv_size = sizeof(struct sx_net_priv),
    .setup = sx_netdev_setup,
    .validate = sx_netdev_validate,
    .newlink = sx_netdev_newlink,
    .changelink = sx_netdev_changelink,
    .dellink = sx_netdev_dellink,
    .get_size = sx_netdev_get_size,
    .fill_info = sx_netdev_fill_info
};

int sx_netdev_rtnl_link_register(void)
{
    return rtnl_link_register(&sx_netdev_link_ops);
}

void sx_netdev_rtnl_link_unregister(void)
{
    rtnl_link_unregister(&sx_netdev_link_ops);
}
