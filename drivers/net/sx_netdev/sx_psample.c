/*
 * SPDX-FileCopyrightText: Copyright (c) 2010-2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

#if IS_ENABLED(CONFIG_PSAMPLE)

#include <linux/module.h>

#include <linux/if_vlan.h>
#include <net/psample.h>

#include <linux/mlx_sx/driver.h>

#if defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
#include <net/pkt_cls.h>
#include <net/tc_act/tc_sample.h>

extern int sx_netdev_tc_psample_enable;
#endif /* if defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) */

#include "sx_netdev.h"
#include "sx_psample.h"

#define MAX_PSAMPLE_LISTENERS (16)

struct psample_listener_context {
    struct sx_psample_listener_context sx_psample_context; /* must be first in this struct! */
    struct list_head                   listeners_list;
    struct psample_group              *group;
};

/*
 *  PSAMPLE rate related dbs
 */
#define PSAMPLE_DEFAULT_RATE 1

void psample_rate_dump(struct seq_file *m, struct sx_dev *dev)
{
    int                        i, j, count;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return;
    }

    seq_printf(m, "\nPSAMPLE rate db dump: \n");

    seq_printf(m, "\nPORT rate db dump: \n");
    count = 0;
    for (i = 0; i < MAX_PHYPORT_NUM + 1; i++) {
        for (j = 0; j < KU_PSAMPLE_RATE_TYPE_MAX_E + 1; j++) {
            if (priv->psample_params.psample_port_rate_db[i][j].valid == 0) {
                continue;
            }
            count++;
            seq_printf(m,
                       "\t port %d, rate_type:%d, rate:%d \n",
                       i,
                       j,
                       priv->psample_params.psample_port_rate_db[i][j].rate);
        }
    }
    seq_printf(m, "\t PORT rate db entries count: %d\n", count);

    seq_printf(m, "\nMIRROR rate db dump: \n");
    count = 0;
    for (i = 0; i < KU_PSAMPLE_RATE_TYPE_MAX_E + 1; i++) {
        if (priv->psample_params.psample_mirror_bind_rates_db[i].valid == 0) {
            continue;
        }
        count++;
        seq_printf(m,
                   "\t mirror_rate_type:%d, rate:%d \n",
                   i,
                   priv->psample_params.psample_mirror_bind_rates_db[i].rate);
    }
    seq_printf(m, "\t MIRROR rate db entries count: %d\n", count);

    seq_printf(m, "\nSPAN session rate db dump: \n");
    count = 0;
    for (i = 0; i < KU_PSAMPLE_RATE_TYPE_MAX_E + 1; i++) {
        if (priv->psample_params.psample_span_session_rates_db[i].valid == 0) {
            continue;
        }
        count++;
        seq_printf(m,
                   "\t span_session_rate_type:%d, rate:%d \n",
                   i,
                   priv->psample_params.psample_span_session_rates_db[i].rate);
    }
    seq_printf(m, "\t SPAN session rate db entries count: %d\n", count);

    dev_netdev_priv_put(priv);
}

/* check if current packet rate type is KU_PSAMPLE_RATE_TYPE_PORT_... type ,
 *  if not psample_port_rate_type_p will be KU_PSAMPLE_RATE_TYPE_INVALID */
static void __psample_port_rate_type_get(struct completion_info* ci, ku_psample_rate_type_e *psample_port_rate_type_p)
{
    *psample_port_rate_type_p = KU_PSAMPLE_RATE_TYPE_INVALID;

    if (ci->cqe_params.trap_id == SXD_TRAP_ID_ETH_L2_PKT_SAMPLE) {
        *psample_port_rate_type_p = KU_PSAMPLE_RATE_TYPE_PORT_SFLOW_SAMPLE;
        goto out;
    }

    switch (ci->cqe_params.mirror_reason) {
    case SXD_SPAN_MIRROR_REASON_ING_PORT_E:
        *psample_port_rate_type_p = KU_PSAMPLE_RATE_TYPE_PORT_ING_PORT_E;
        break;

    case SXD_SPAN_MIRROR_REASON_EGR_PORT_E:
        *psample_port_rate_type_p = KU_PSAMPLE_RATE_TYPE_PORT_EGR_PORT_E;
        break;

    default:
        break;
    }

out:
    return;
}

/* check if current packet rate type is KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_... type ,
 *  if not psample_mirror_rate_type_p will be KU_PSAMPLE_RATE_TYPE_INVALID */
static void __psample_mirror_rate_type_get(struct completion_info* ci,
                                           ku_psample_rate_type_e *psample_mirror_rate_type_p)
{
    *psample_mirror_rate_type_p = KU_PSAMPLE_RATE_TYPE_INVALID;

    switch (ci->cqe_params.mirror_reason) {
    case SXD_SPAN_MIRROR_REASON_ING_SHARED_BUFFER_DROP_E:
        *psample_mirror_rate_type_p = KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_ING_SHARED_BUFFER_DROP_E;
        break;

    case SXD_SPAN_MIRROR_REASON_ING_WRED_E:
        *psample_mirror_rate_type_p = KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_ING_WRED_E;
        break;

    case SXD_SPAN_MIRROR_REASON_ING_PG_CONGESTION_E:
        *psample_mirror_rate_type_p = KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_ING_PG_CONGESTION_E;
        break;

    case SXD_SPAN_MIRROR_REASON_ING_TC_CONGESTION_E:
        *psample_mirror_rate_type_p = KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_ING_TC_CONGESTION_E;
        break;

    case SXD_SPAN_MIRROR_REASON_EGR_TC_LATENCY_E:
        *psample_mirror_rate_type_p = KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_EGR_TC_LATENCY_E;
        break;

    case SXD_SPAN_MIRROR_REASON_EGR_ECN_E:
        *psample_mirror_rate_type_p = KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_EGR_ECN_E;
        break;

    default:
        break;
    }
}

/* check if current packet rate type is KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_... type ,
 *  if not psample_span_session_rate_type_p will be KU_PSAMPLE_RATE_TYPE_INVALID */
static void __psample_span_session_rate_type_get(struct completion_info* ci,
                                                 ku_psample_rate_type_e *psample_span_session_rate_type_p)
{
    *psample_span_session_rate_type_p = KU_PSAMPLE_RATE_TYPE_INVALID;

    switch (ci->cqe_params.trap_id) {
    case SXD_TRAP_ID_MIRROR_AGENT0:
        *psample_span_session_rate_type_p = KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_0_E;
        break;

    case SXD_TRAP_ID_MIRROR_AGENT1:
        *psample_span_session_rate_type_p = KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_1_E;
        break;

    case SXD_TRAP_ID_MIRROR_AGENT2:
        *psample_span_session_rate_type_p = KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_2_E;
        break;

    case SXD_TRAP_ID_MIRROR_AGENT3:
        *psample_span_session_rate_type_p = KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_3_E;
        break;

    case SXD_TRAP_ID_MIRROR_AGENT4:
        *psample_span_session_rate_type_p = KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_4_E;
        break;

    case SXD_TRAP_ID_MIRROR_AGENT5:
        *psample_span_session_rate_type_p = KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_5_E;
        break;

    case SXD_TRAP_ID_MIRROR_AGENT6:
        *psample_span_session_rate_type_p = KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_6_E;
        break;

    case SXD_TRAP_ID_MIRROR_AGENT7:
        *psample_span_session_rate_type_p = KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_7_E;
        break;

    default:
        break;
    }
}

static int __psample_port_rate_get(ku_psample_rate_type_e psample_port_rate_type,
                                   uint16_t               ingress_local_port,
                                   uint16_t               egress_local_port,
                                   struct sx_dev         *dev,
                                   int                   *rate_p)
{
    int                        err = 0;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return -ENODEV;
    }

    switch (psample_port_rate_type) {
    case KU_PSAMPLE_RATE_TYPE_PORT_SFLOW_SAMPLE:
    case KU_PSAMPLE_RATE_TYPE_PORT_ING_PORT_E:
        if (ingress_local_port > MAX_PHYPORT_NUM) {
            sxd_log_err(dev,
                        "PSAMPLE sample port rate get failed , ingress local port (%u) is our of range.\n",
                        ingress_local_port);
            err = -EINVAL;
            goto out;
        }

        if (priv->psample_params.psample_port_rate_db[ingress_local_port][psample_port_rate_type].valid == false) {
            err = -EINVAL;
            goto out;
        }
        *rate_p = priv->psample_params.psample_port_rate_db[ingress_local_port][psample_port_rate_type].rate;
        break;

    case KU_PSAMPLE_RATE_TYPE_PORT_EGR_PORT_E:
        if (egress_local_port > MAX_PHYPORT_NUM) {
            sxd_log_err(dev,
                        "PSAMPLE sample port rate get failed , egress local port (%u) is our of range.\n",
                        egress_local_port);
            err = -EINVAL;
            goto out;
        }

        if (priv->psample_params.psample_port_rate_db[egress_local_port][psample_port_rate_type].valid == false) {
            err = -EINVAL;
            goto out;
        }
        *rate_p = priv->psample_params.psample_port_rate_db[egress_local_port][psample_port_rate_type].rate;
        break;

    default:
        err = -EINVAL;
        goto out;
    }

out:
    dev_netdev_priv_put(priv);
    return err;
}

static int __psample_port_rate_set(struct sx_dev         *dev,
                                   ku_psample_rate_type_e psample_port_rate_type,
                                   uint16_t               local_port,
                                   int                    rate,
                                   sxd_boolean_t          enable)
{
    int                        err = 0;
    uint16_t                   ingress_local_port, egress_local_port;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return -ENODEV;
    }

    switch (psample_port_rate_type) {
    case KU_PSAMPLE_RATE_TYPE_PORT_SFLOW_SAMPLE:
    case KU_PSAMPLE_RATE_TYPE_PORT_ING_PORT_E:
        ingress_local_port = local_port;
        if (ingress_local_port > MAX_PHYPORT_NUM) {
            sxd_log_err(dev,
                        "PSAMPLE sample port rate type %d set failed , ingress local port (%u) is our of range.\n",
                        psample_port_rate_type,
                        ingress_local_port);
            err = -EINVAL;
            goto out;
        }
        priv->psample_params.psample_port_rate_db[ingress_local_port][psample_port_rate_type].rate = rate;
        priv->psample_params.psample_port_rate_db[ingress_local_port][psample_port_rate_type].valid = enable;
        break;

    case KU_PSAMPLE_RATE_TYPE_PORT_EGR_PORT_E:
        egress_local_port = local_port;
        if (egress_local_port > MAX_PHYPORT_NUM) {
            sxd_log_err(dev,
                        "PSAMPLE sample port rate get failed , egress local port (%u) is our of range.\n",
                        egress_local_port);
            err = -EINVAL;
            goto out;
        }

        priv->psample_params.psample_port_rate_db[egress_local_port][psample_port_rate_type].rate = rate;
        priv->psample_params.psample_port_rate_db[egress_local_port][psample_port_rate_type].valid = enable;
        break;

    default:
        err = -EINVAL;
        goto out;
    }

out:
    dev_netdev_priv_put(priv);
    return err;
}

static int __psample_mirror_rate_get(struct sx_dev *dev, ku_psample_rate_type_e psample_mirror_rate_type, int *rate_p)
{
    int                        err = 0;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return -ENODEV;
    }

    switch (psample_mirror_rate_type) {
    case KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_ING_SHARED_BUFFER_DROP_E:
    case KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_ING_WRED_E:
    case KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_ING_PG_CONGESTION_E:
    case KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_ING_TC_CONGESTION_E:
    case KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_EGR_TC_LATENCY_E:
    case KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_EGR_ECN_E:
        if (priv->psample_params.psample_mirror_bind_rates_db[psample_mirror_rate_type].valid == false) {
            err = -EINVAL;
            goto out;
        }
        *rate_p = priv->psample_params.psample_mirror_bind_rates_db[psample_mirror_rate_type].rate;
        break;

    default:
        err = -EINVAL;
        break;
    }

out:
    dev_netdev_priv_put(priv);
    return err;
}

static int __psample_mirror_rate_set(struct sx_dev         *dev,
                                     ku_psample_rate_type_e psample_mirror_rate_type,
                                     int                    rate,
                                     sxd_boolean_t          enable)
{
    int                        err = 0;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return -ENODEV;
    }

    switch (psample_mirror_rate_type) {
    case KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_ING_SHARED_BUFFER_DROP_E:
    case KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_ING_WRED_E:
    case KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_ING_PG_CONGESTION_E:
    case KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_ING_TC_CONGESTION_E:
    case KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_EGR_TC_LATENCY_E:
    case KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_EGR_ECN_E:
        priv->psample_params.psample_mirror_bind_rates_db[psample_mirror_rate_type].rate = rate;
        priv->psample_params.psample_mirror_bind_rates_db[psample_mirror_rate_type].valid = enable;
        break;

    default:
        err = -EINVAL;
        break;
    }

    dev_netdev_priv_put(priv);
    return err;
}


static int __psample_span_session_rate_get(struct sx_dev         *dev,
                                           ku_psample_rate_type_e psample_span_session_rate_type,
                                           int                   *rate_p)
{
    int                        err = 0;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return -ENODEV;
    }

    switch (psample_span_session_rate_type) {
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_0_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_1_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_2_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_3_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_4_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_5_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_6_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_7_E:
        if (priv->psample_params.psample_span_session_rates_db[psample_span_session_rate_type].valid == false) {
            err = -EINVAL;
            goto out;
        }
        *rate_p = priv->psample_params.psample_span_session_rates_db[psample_span_session_rate_type].rate;
        break;

    default:
        err = -EINVAL;
        break;
    }

out:
    dev_netdev_priv_put(priv);
    return err;
}

static int __psample_span_session_rate_set(struct sx_dev         *dev,
                                           ku_psample_rate_type_e psample_span_session_rate_type,
                                           int                    rate,
                                           sxd_boolean_t          enable)
{
    int                        err = 0;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return -ENODEV;
    }

    switch (psample_span_session_rate_type) {
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_0_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_1_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_2_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_3_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_4_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_5_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_6_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_7_E:
        priv->psample_params.psample_span_session_rates_db[psample_span_session_rate_type].rate = rate;
        priv->psample_params.psample_span_session_rates_db[psample_span_session_rate_type].valid = enable;
        break;

    default:
        err = -EINVAL;
        break;
    }

    dev_netdev_priv_put(priv);
    return err;
}

/*
 * Define 'rate_source_type' according to the sampled packet's mirror_reason and hw_synd:
 * P1: If the trapped is sFlow sample / per-port sampling and has a valid per-port psample rate: Can use packet's mirror_reason to set the corresponding psample rate.
 *     In kernel versions that supports it: set the rate_as_probability bit to 1.
 * P2: If the trapped packet is per-mirror_bind sampling: Can use packet's mirror_reason to set the corresponding psample rate.
 *     In kernel versions that supports it: set the rate_as_probability bit to 1.
 * P3: Else, If the packet is per-span_session sampling and has a valid per-span_session psample_rate: Can use packet's hw_synd to set the corresponding psample rate.
 *     In kernel versions that supports it: set the rate_as_probability bit to 0.
 * P4: Else: Use rate = 1 to send to psample module.
 *     In kernel versions that supports it: set the rate_as_probability bit to 1.
 */
static int __psample_rate_get(struct completion_info* ci,
                              uint16_t                ingress_local_port,
                              uint16_t                egress_local_port,
                              uint8_t                *rate_as_probability_p)
{
    int                    rate = PSAMPLE_DEFAULT_RATE;
    int                    err;
    ku_psample_rate_type_e psample_port_rate_type, psample_mirror_rate_type, psample_span_session_rate_type;

    __psample_port_rate_type_get(ci, &psample_port_rate_type);
    if (psample_port_rate_type != KU_PSAMPLE_RATE_TYPE_INVALID) {     /* P1 */
        err = __psample_port_rate_get(psample_port_rate_type, ingress_local_port, egress_local_port, ci->dev, &rate);
        if (err == 0) {
            /* valid rate is found */
            *rate_as_probability_p = 1;
            goto out;
        }
        /* valid rate isn't found, so continue to look further */
    }

    __psample_mirror_rate_type_get(ci, &psample_mirror_rate_type);
    if (psample_mirror_rate_type != KU_PSAMPLE_RATE_TYPE_INVALID) {     /* P2 */
        err = __psample_mirror_rate_get(ci->dev, psample_mirror_rate_type, &rate);
        if (err == 0) {
            /* valid rate is found */
            *rate_as_probability_p = 1;
            goto out;
        }
        /* valid rate isn't found, so continue to look further */
    }

    __psample_span_session_rate_type_get(ci, &psample_span_session_rate_type);
    if (psample_span_session_rate_type != KU_PSAMPLE_RATE_TYPE_INVALID) { /* P3 */
        err = __psample_span_session_rate_get(ci->dev, psample_span_session_rate_type, &rate);
        if (err == 0) {
            /* valid rate is found */
            *rate_as_probability_p = 0;
            goto out;
        }
        /* valid rate isn't found, so continue to look further */
    }

    /* valid rate isn't found, use the default */
    *rate_as_probability_p = 1;
    rate = PSAMPLE_DEFAULT_RATE;

out:
    return rate;
}

int sx_psample_rate_set(struct sx_dev *dev, struct ku_psample_sample_rate *psample_sample_rate_p)
{
    int err = 0;

    switch (psample_sample_rate_p->rate_type) {
    case KU_PSAMPLE_RATE_TYPE_PORT_SFLOW_SAMPLE:
    case KU_PSAMPLE_RATE_TYPE_PORT_ING_PORT_E:
    case KU_PSAMPLE_RATE_TYPE_PORT_EGR_PORT_E:
        err = __psample_port_rate_set(dev, psample_sample_rate_p->rate_type,
                                      psample_sample_rate_p->rate_data.port_sample_rate.local_port,
                                      psample_sample_rate_p->rate_data.port_sample_rate.rate,
                                      psample_sample_rate_p->rate_data.port_sample_rate.enable);
        break;

    case KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_ING_SHARED_BUFFER_DROP_E:
    case KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_ING_WRED_E:
    case KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_ING_PG_CONGESTION_E:
    case KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_ING_TC_CONGESTION_E:
    case KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_EGR_TC_LATENCY_E:
    case KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_EGR_ECN_E:
        err = __psample_mirror_rate_set(dev, psample_sample_rate_p->rate_type,
                                        psample_sample_rate_p->rate_data.mirror_reason_sample_rate.rate,
                                        psample_sample_rate_p->rate_data.mirror_reason_sample_rate.enable);
        break;

    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_0_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_1_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_2_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_3_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_4_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_5_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_6_E:
    case KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_7_E:
        err = __psample_span_session_rate_set(dev, psample_sample_rate_p->rate_type,
                                              psample_sample_rate_p->rate_data.mirror_reason_sample_rate.rate,
                                              psample_sample_rate_p->rate_data.mirror_reason_sample_rate.enable);
        break;

    default:
        err = -EINVAL;
        break;
    }

    return err;
}

struct psample_work_data {
    struct work_struct    work;
    struct psample_group *group;
    struct sk_buff       *skb;
    u32                   original_packet_size;
    u32                   ingress_ifindex;
    u32                   egress_ifindex;
    u32                   rate;
#if defined(PSAMPLE_MD_EXTENDED_ATTR) || defined(CONFIG_SX_RHEL_8_6) || defined(CONFIG_SX_CENTOS_8_4) || \
    (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
    struct psample_metadata md;
#endif
};

#ifdef PSAMPLE_MD_EXTENDED_ATTR

static void psample_wq_add_work_ext(struct psample_group    *group,
                                    struct completion_info  *ci,
                                    u32                      rate,
                                    struct psample_metadata *md);
#else
static void psample_wq_add_work(struct psample_group   *group,
                                struct completion_info *ci,
                                u32                     ingress_ifindex,
                                u32                     egress_ifindex,
                                u32                     rate);
#endif


#ifdef PSAMPLE_MD_EXTENDED_ATTR

static void __psample_sample_packet_ext(struct completion_info* ci,
                                        struct psample_group   *group,
                                        u32                     ingress_ifindex,
                                        u32                     egress_ifindex,
                                        u32                     rate,
                                        u8                      rate_as_probability)
{
    struct psample_metadata md = {};

    /* trunc_size is the cap for PSAMPLE_ATTR_DATA (kernel uses min(skb->len, trunc_size)).
     * ORIGSIZE is set by the kernel from skb->len; we restore skb->len in the work handler.
     */
    md.trunc_size = ci->skb->len;
    md.in_ifindex = ingress_ifindex;
    md.out_ifindex = egress_ifindex;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
    /* rate_as_probability was added from kernel 6.11 */
    md.rate_as_probability = rate_as_probability;
#endif

    /* The fields below are relevant only for packets that mirrored to CPU */
    if ((ci->cqe_params.trap_id >= SXD_TRAP_ID_MIRROR_AGENT0) &&
        (ci->cqe_params.trap_id <= SXD_TRAP_ID_MIRROR_AGENT7)) {
        md.out_tc_valid = (ci->cqe_params.mirror_tclass == SX_CQE_MIRROR_TCLASS_INVALID) ? 0 : 1;
        if (md.out_tc_valid) {
            md.out_tc = ci->cqe_params.mirror_tclass;
        }

        /* Congestion level in units of 8KB of the egress traffic class of the original
         * packet that does mirroring to the CPU.
         * Value of 0xFFFF means that the congestion level is invalid.
         */
        md.out_tc_occ_valid = (ci->cqe_params.mirror_cong == SX_CQE_MIRROR_CONG_INVALID) ? 0 : 1;
        if (md.out_tc_occ_valid) {
            md.out_tc_occ = ci->cqe_params.mirror_cong;
            md.out_tc_occ <<= SX_CQE_MIRROR_CONG_SHIFT;
        }

        /* The latency units are determined according to MOGCR.mirror_latency_units.
         * It defaults to 64 nanoseconds.
         * Value of 0xFFFFFF means that the latency value is invalid.
         */
        md.latency_valid = (ci->cqe_params.mirror_lantency == SX_CQE_MIRROR_LATENCY_INVALID) ? 0 : 1;
        if (md.latency_valid) {
            md.latency = ci->cqe_params.mirror_lantency;
            md.latency <<= SX_CQE_MIRROR_LATENCY_SHIFT;
        }
    }

    psample_wq_add_work_ext(group, ci, rate, &md);
}

#endif /* #ifndef PSAMPLE_MD_EXTENDED_ATTR */

static void __psample_handler(struct completion_info* ci, void *context)
{
    struct psample_listener_context *listener_ctx = (struct psample_listener_context*)context;
    struct net_device               *netdev;
    u32                              ingress_ifindex = 0, egress_ifindex = 0;
    u32                              rate = 0;
    u16                              ingress_local_port = INVALID_LOCAL_PORT;
    u16                              egress_local_port = INVALID_LOCAL_PORT;
    int                              i, ret;
    uint8_t                          rate_as_probability = 0;
    struct sx_dev_netdev_priv       *priv = dev_netdev_priv_from_dev(ci->dev);

    if (!priv) {
        return;
    }

    /*
     * find ingress local port
     */
    if (ci->cqe_params.is_lag) {
        ret = sx_core_get_local(ci->dev,
                                ci->cqe_params.sysport_lag_id,
                                ci->cqe_params.lag_subport,
                                &ingress_local_port);
        if (ret) {
            goto skip_local_port;
        }
    } else {
        ingress_local_port = ci->cqe_params.sysport_lag_id;
    }

    if (ingress_local_port > MAX_PHYPORT_NUM) {
        goto skip_local_port;
    }

    /*
     * find egress local port
     */
    if (ci->cqe_params.dest_is_lag) {
        ret = sx_core_get_local(ci->dev,
                                ci->cqe_params.dest_sysport_lag_id,
                                ci->cqe_params.dest_lag_subport,
                                &egress_local_port);

        if (ret) {
            egress_local_port = INVALID_LOCAL_PORT;
        }
    } else {
        egress_local_port = ci->cqe_params.dest_sysport_lag_id;
    }


    /* there may be 2 net-devices on the same sysport.
     * only one of them will get the psample packet (the first one).
     */
    rcu_read_lock();

    for (i = 0; i < MAX_PORT_NETDEV_NUM; i++) {
        netdev = rcu_dereference(priv->netdev_resources.port_netdev[PORT_TYPE_SINGLE][ingress_local_port][i]);
        if (netdev) {
            ingress_ifindex = netdev->ifindex;
            break;
        }
    }

    if (egress_local_port <= MAX_PHYPORT_NUM) {
        for (i = 0; i < MAX_PORT_NETDEV_NUM; i++) {
            netdev = rcu_dereference(priv->netdev_resources.port_netdev[PORT_TYPE_SINGLE][egress_local_port][i]);
            if (netdev) {
                egress_ifindex = netdev->ifindex;
                break;
            }
        }
    }

    rcu_read_unlock();

    rate = __psample_rate_get(ci, ingress_local_port, egress_local_port, &rate_as_probability);

skip_local_port:

#ifdef PSAMPLE_MD_EXTENDED_ATTR
    __psample_sample_packet_ext(ci, listener_ctx->group, ingress_ifindex,
                                egress_ifindex, rate, rate_as_probability);
#else
    psample_wq_add_work(listener_ctx->group, ci, ingress_ifindex, egress_ifindex, rate);
#endif

    dev_netdev_priv_put(priv);
}


/* must be called when DB is locked! */
static struct psample_listener_context * __db_lookup(struct sx_dev_netdev_priv *priv, u32 group_id)
{
    struct psample_listener_context *iter;

    list_for_each_entry(iter, &priv->psample_params.psample_listener_db, listeners_list) {
        if (iter->sx_psample_context.group_num == group_id) {
            return iter;
        }
    }

    return NULL;
}


static int __psample_listener_get(struct sx_dev *dev, u32 group_id, struct psample_listener_context **listener_ctx_pp)
{
    union ku_filter_critireas        crit;
    struct psample_listener_context *listener_ctx = NULL;
    int                              err = 0, is_listener_allocated = 0;
    struct sx_dev_netdev_priv       *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return -ENODEV;
    }

    memset(&crit, 0, sizeof(crit));

    listener_ctx = __db_lookup(priv, group_id);
    if (listener_ctx == NULL) {
        if (priv->psample_params.psample_listener_db_size == MAX_PSAMPLE_LISTENERS) {
            sxd_log_err(dev,
                        "PSAMPLE [add group_id=%u]: database is full, cannot add another entry\n",
                        group_id);
            err = -EUSERS;
            goto out;
        }

        listener_ctx = kzalloc(sizeof(struct psample_listener_context), GFP_KERNEL);
        if (!listener_ctx) {
            sxd_log_err(dev, "PSAMPLE [add group_id=%u]: could not allocate context\n", group_id);
            err = -ENOMEM;
            goto out;
        }
        is_listener_allocated = 1;

        listener_ctx->sx_psample_context.group_num = group_id;
        listener_ctx->sx_psample_context.refcnt = 0;
        INIT_LIST_HEAD(&listener_ctx->listeners_list);
        listener_ctx->group = psample_group_get(priv->netns, group_id);

        if (!listener_ctx->group) {
            sxd_log_err(dev, "PSAMPLE [add group_id=%u]: could not get group\n", group_id);
            err = -ENOMEM;
            goto cleanup;
        }

        list_add_tail(&listener_ctx->listeners_list, &priv->psample_params.psample_listener_db);
        priv->psample_params.psample_listener_db_size++;
    }

    listener_ctx->sx_psample_context.refcnt++;
    *listener_ctx_pp = listener_ctx;

cleanup:
    /* error can happen only in case of newly allocated listener */
    if (err) {
        /* at this point, it is guaranteed that listener_ctx is a new valid allocation */
        if (listener_ctx->group) {
            psample_group_put(listener_ctx->group);
        }

        if (is_listener_allocated) {
            kfree(listener_ctx);
            listener_ctx = NULL;
        }
    }

out:
    dev_netdev_priv_put(priv);
    return err;
}


static int __psample_listener_put(struct sx_dev *dev, u32 group_id)
{
    union ku_filter_critireas        crit;
    struct psample_listener_context *listener_ctx;
    int                              err = 0;
    struct sx_dev_netdev_priv       *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return -ENODEV;
    }

    memset(&crit, 0, sizeof(crit));

    listener_ctx = __db_lookup(priv, group_id);
    if (!listener_ctx) { /* did not find it in DB */
        sxd_log_err(dev,
                    "PSAMPLE [del group_id=%u]: could not find listener in database\n",
                    group_id);
        err = -ENOENT;
        goto out;
    }

    if (--listener_ctx->sx_psample_context.refcnt > 0) {
        sxd_log_info(dev, "PSAMPLE [del group_id=%u]: listener still has references\n", group_id);
        goto out;
    }

    list_del(&listener_ctx->listeners_list);
    psample_group_put(listener_ctx->group);
    kfree(listener_ctx);

    priv->psample_params.psample_listener_db_size--;

out:
    dev_netdev_priv_put(priv);
    return err;
}


static int __psample_synd_add(struct sx_dev *dev, u16 synd, union sx_event_data *event_data)
{
    union ku_filter_critireas        crit;
    struct psample_listener_context *listener_ctx = NULL;
    u32                              group_id;
    int                              err = 0;
    char                             synd_str[SX_LISTENER_HANDLER_STR_MAX_LEN] = { 0 };
    struct sx_dev_netdev_priv       *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return -ENODEV;
    }

    memset(&crit, 0, sizeof(crit));
    group_id = event_data->psample_synd.psample_info.group_id;

    mutex_lock(&priv->psample_params.psample_listener_db_lock);

    err = __psample_listener_get(dev, group_id, &listener_ctx);
    if (err) {
        sxd_log_err(dev, "__psample_listener_get failed for group_id=%u (err=%d)\n", group_id,
                    err);
        goto out;
    }

    snprintf(synd_str, sizeof(synd_str) - 1, "PSAMPLE [g=%u,rc=%u]",
             listener_ctx->sx_psample_context.group_num, listener_ctx->sx_psample_context.refcnt);

    err = sx_core_add_synd(event_data->psample_synd.swid,
                           event_data->psample_synd.hw_synd,
                           L2_TYPE_ETH,
                           "sx_psample",
                           0,
                           &crit,
                           synd_str,
                           __psample_handler,
                           listener_ctx,
                           CHECK_DUP_DISABLED_E,
                           dev,
                           &event_data->psample_synd.port_vlan_params,
                           event_data->psample_synd.is_register);

    if (err) {
        sxd_log_err(dev,
                    "PSAMPLE [add group_id=%u]: could not add synd listener (err=%d)\n",
                    group_id,
                    err);
        goto out;
    }

out:
    if (err && (listener_ctx != NULL)) {
        __psample_listener_put(dev, group_id);
    }

    mutex_unlock(&priv->psample_params.psample_listener_db_lock);

    dev_netdev_priv_put(priv);
    return err;
}


static int __psample_synd_remove(struct sx_dev *dev, u16 synd, union sx_event_data *event_data)
{
    union ku_filter_critireas        crit;
    struct psample_listener_context *listener_ctx;
    u32                              group_id;
    int                              err = 0;
    struct sx_dev_netdev_priv       *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return -ENODEV;
    }

    memset(&crit, 0, sizeof(crit));
    group_id = event_data->psample_synd.psample_info.group_id;

    mutex_lock(&priv->psample_params.psample_listener_db_lock);

    listener_ctx = __db_lookup(priv, group_id);
    if (!listener_ctx) { /* did not find it in DB */
        sxd_log_err(dev,
                    "PSAMPLE [del group_id=%u]: could not find listener in database\n",
                    group_id);
        err = -ENOENT;
        goto out;
    }

    /* when entry is removed from DB, it is guaranteed that the PSAMPLE handler is not in
     * the middle of execution [dispatch_pkt() that runs the handler and sx_core_remove_synd()
     * share the same lock] */
    err = sx_core_remove_synd(event_data->psample_synd.swid,
                              event_data->psample_synd.hw_synd,
                              L2_TYPE_ETH,
                              0,
                              &crit,
                              listener_ctx,
                              dev,
                              __psample_handler,
                              &event_data->psample_synd.port_vlan_params,
                              event_data->psample_synd.is_register, NULL);
    if (err) {
        listener_ctx->sx_psample_context.refcnt++; /* rollback */
        sxd_log_err(dev,
                    "PSAMPLE [del group_id=%u]: could not remove synd listener (err=%d)\n",
                    group_id,
                    err);
        goto out;
    }

    __psample_listener_put(dev, group_id);

out:
    mutex_unlock(&priv->psample_params.psample_listener_db_lock);
    dev_netdev_priv_put(priv);
    return err;
}


int sx_psample_set_synd(struct sx_dev *dev, u16 synd, enum sx_dev_event event, union sx_event_data *event_data)
{
    int err = 0;

    switch (event) {
    case SX_DEV_EVENT_ADD_SYND_PSAMPLE:
        err = __psample_synd_add(dev, synd, event_data);
        break;

    case SX_DEV_EVENT_REMOVE_SYND_PSAMPLE:
        err = __psample_synd_remove(dev, synd, event_data);
        break;

    default:
        err = -EINVAL;
        sxd_log_err(dev, "invalid PSAMPLE sx-dev-event (event=%d)\n", event);
        break;
    }

    return err;
}


static void psample_work_handler(struct work_struct *work)
{
    struct psample_work_data * data = (struct psample_work_data *)work;

#if defined(PSAMPLE_MD_EXTENDED_ATTR) || defined(CONFIG_SX_RHEL_8_6) || defined(CONFIG_SX_CENTOS_8_4) || \
    (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
    u32 saved_len = 0;

    /* Kernel sets PSAMPLE_ATTR_ORIGSIZE from skb->len; restore original length
     * so userspace gets correct origsize. data_len is already capped by md.trunc_size.
     * Safe: this skb is owned exclusively by this work (taken via skb_get in
     * __psample_wq_add_work); no other code touches it until we consume_skb below.
     */
    if (data->skb->len != data->original_packet_size) {
        saved_len = data->skb->len;
        data->skb->len = data->original_packet_size;
    }
    psample_sample_packet(data->group, data->skb, data->rate, &data->md);
    if (saved_len != 0) {
        data->skb->len = saved_len;
    }
#else
    psample_sample_packet(data->group, data->skb, data->original_packet_size,
                          data->ingress_ifindex, data->egress_ifindex, data->rate);
#endif
    consume_skb(data->skb); /* free unused skb, use consume_skb */
    kfree(data);
}

static void __psample_wq_add_work(struct psample_work_data * external_data, struct sx_dev  *dev)
{
    struct psample_work_data * data;
    bool                       rc;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return;
    }

    data = kmalloc(sizeof(struct psample_work_data), GFP_ATOMIC);
    if (data == NULL) {
        sxd_log_err(dev, "PSAMPLE : failed to alloc work_data \n");
        goto out;
    }

    INIT_WORK(&data->work, psample_work_handler);
    data->group = external_data->group;
    data->skb = skb_get(external_data->skb);
    data->rate = external_data->rate;
    data->original_packet_size = external_data->original_packet_size;
#ifdef PSAMPLE_MD_EXTENDED_ATTR
    data->md = external_data->md;
#else
    data->ingress_ifindex = external_data->ingress_ifindex;
    data->egress_ifindex = external_data->egress_ifindex;
#endif

    rc = queue_work(priv->psample_params.psample_wq, &data->work);
    if (rc == false) {
        kfree_skb(data->skb); /* drop flow, use kfree_skb */
        kfree(data);
    }

out:
    dev_netdev_priv_put(priv);
}

#ifdef PSAMPLE_MD_EXTENDED_ATTR

static void psample_wq_add_work_ext(struct psample_group    *group,
                                    struct completion_info  *ci,
                                    u32                      rate,
                                    struct psample_metadata *md)
{
    struct psample_work_data data;

    data.group = group;
    data.skb = ci->skb;
    data.rate = rate;
    data.md = *md;
    data.original_packet_size = ci->hw_original_packet_size;

    __psample_wq_add_work(&data, ci->dev);
}

#else

static void psample_wq_add_work(struct psample_group   *group,
                                struct completion_info *ci,
                                u32                     ingress_ifindex,
                                u32                     egress_ifindex,
                                u32                     rate)
{
    struct psample_work_data data;

    data.group = group;
    data.skb = ci->skb;
    data.rate = rate;
    data.original_packet_size = ci->original_packet_size;
    data.ingress_ifindex = ingress_ifindex;
    data.egress_ifindex = egress_ifindex;

    __psample_wq_add_work(&data, ci->dev);
}

#endif

static void psample_wq_init(struct sx_dev *dev)
{
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);
    char                       wq_str[32];

    if (!priv) {
        return;
    }

    sprintf(wq_str, "psample_wq_dev_id_%u", dev->device_id);

    if (priv->psample_params.psample_wq == NULL) {
        priv->psample_params.psample_wq = create_workqueue(wq_str);
    }

    dev_netdev_priv_put(priv);
}

static void psample_wq_deinit(struct sx_dev *dev)
{
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return;
    }

    if (priv->psample_params.psample_wq != NULL) {
        flush_workqueue(priv->psample_params.psample_wq);
        destroy_workqueue(priv->psample_params.psample_wq);
        priv->psample_params.psample_wq = NULL;
    }

    dev_netdev_priv_put(priv);
}

void sx_psample_init(struct sx_dev *dev)
{
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return;
    }

    INIT_LIST_HEAD(&priv->psample_params.psample_listener_db);
    priv->psample_params.psample_listener_db_size = 0;
    mutex_init(&priv->psample_params.psample_listener_db_lock);
    priv->psample_params.psample_wq = NULL;

    psample_wq_init(dev);
    dev_netdev_priv_put(priv);
}

void sx_psample_cleanup(struct sx_dev *dev)
{
    struct psample_listener_context *psample_listener_context_p;
    struct list_head                *pos, *q;
    struct sx_dev_netdev_priv       *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return;
    }

    sx_core_flush_synd_by_handler(dev, __psample_handler);

    psample_wq_deinit(dev);
    mutex_lock(&priv->psample_params.psample_listener_db_lock);

    if (priv->psample_params.psample_listener_db.next == NULL) {
        goto out;
    }
    if (!list_empty(&priv->psample_params.psample_listener_db)) {
        list_for_each_safe(pos, q, &priv->psample_params.psample_listener_db) {
            psample_listener_context_p = list_entry(pos, struct psample_listener_context,
                                                    listeners_list);

            list_del(&psample_listener_context_p->listeners_list);
            psample_group_put(psample_listener_context_p->group);
            kfree(psample_listener_context_p);
            priv->psample_params.psample_listener_db_size--;
        }
    }
out:
    mutex_unlock(&priv->psample_params.psample_listener_db_lock);
    dev_netdev_priv_put(priv);
}

#if defined(CONFIG_NET_PORT) &&                         \
    (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) && \
    (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
static int sx_psample_add_cls_matchall_sample(struct sx_net_priv             *sx_port,
                                              struct tc_cls_matchall_offload *cls,
                                              const struct tc_action         *a,
                                              bool                            ingress)
{
    return 0;
}

static void sx_psample_del_cls_matchall_sample(struct sx_net_priv *sx_port, bool ingress)
{
}

static int sx_psample_add_cls_matchall(struct sx_net_priv *sx_port, struct tc_cls_matchall_offload *f, bool ingress)
{
    __be16                  protocol = f->common.protocol;
    const struct tc_action *a;
    int                     err;

    if (!tcf_exts_has_one_action(f->exts)) {
        sxd_log_err(SXD_DEV_NONE, "only singular actions are supported\n");
        return -EOPNOTSUPP;
    }

    a = tcf_exts_first_action(f->exts);

    if (is_tcf_sample(a) && (protocol == htons(ETH_P_ALL))) {
        err = sx_psample_add_cls_matchall_sample(sx_port, f, a, ingress);
    } else {
        err = -EOPNOTSUPP;
    }

    return err;
}

static void sx_psample_del_cls_matchall(struct sx_net_priv *sx_port, struct tc_cls_matchall_offload *f, bool ingress)
{
    sx_psample_del_cls_matchall_sample(sx_port, ingress);
}

static int sx_psample_setup_tc_cls_matchall(struct sx_net_priv             *sx_port,
                                            struct tc_cls_matchall_offload *f,
                                            bool                            ingress)
{
    switch (f->command) {
    case TC_CLSMATCHALL_REPLACE:
        return sx_psample_add_cls_matchall(sx_port, f, ingress);

    case TC_CLSMATCHALL_DESTROY:
        sx_psample_del_cls_matchall(sx_port, f, ingress);
        return 0;

    default:
        return -EOPNOTSUPP;
    }
}

static int sx_psample_setup_tc_block_cb(enum tc_setup_type type, void *type_data, void *cb_priv, bool ingress)
{
    struct sx_net_priv *sx_port = cb_priv;

    switch (type) {
    case TC_SETUP_CLSMATCHALL:
        if (!tc_cls_can_offload_and_chain0(sx_port->netdev, type_data)) {
            return -EOPNOTSUPP;
        }

        return sx_psample_setup_tc_cls_matchall(sx_port, type_data, ingress);

    default:
        return -EOPNOTSUPP;
    }
}

static int sx_psample_setup_tc_block_cb_ingress(enum tc_setup_type type, void *type_data, void *cb_priv)
{
    return sx_psample_setup_tc_block_cb(type, type_data, cb_priv, true);
}

static int sx_psample_setup_tc_block_cb_egress(enum tc_setup_type type, void *type_data, void *cb_priv)
{
    return sx_psample_setup_tc_block_cb(type, type_data, cb_priv, false);
}

static int sx_psample_setup_tc_block(struct sx_net_priv *sx_port, struct tc_block_offload *f)
{
    tc_setup_cb_t *cb;

    if (f->binder_type == TCF_BLOCK_BINDER_TYPE_CLSACT_INGRESS) {
        cb = sx_psample_setup_tc_block_cb_ingress;
    } else if (f->binder_type == TCF_BLOCK_BINDER_TYPE_CLSACT_EGRESS) {
        cb = sx_psample_setup_tc_block_cb_egress;
    } else {
        return -EOPNOTSUPP;
    }

    switch (f->command) {
    case TC_BLOCK_BIND:
        return tcf_block_cb_register(f->block, cb, sx_port, sx_port,
                                     f->extack);

    case TC_BLOCK_UNBIND:
        tcf_block_cb_unregister(f->block, cb, sx_port);
        return 0;

    default:
        return -EOPNOTSUPP;
    }
}

int sx_psample_setup_tc(struct net_device *dev, enum tc_setup_type type, void *type_data)
{
    struct sx_net_priv *sx_port = netdev_priv(dev);

    if (!sx_netdev_tc_psample_enable || (type != TC_SETUP_BLOCK)) {
        return -EOPNOTSUPP;
    }

    return sx_psample_setup_tc_block(sx_port, type_data);
}
#endif /* if defined(CONFIG_NET_PORT) && \
        *     (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) && \
        *     (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)) */

#endif /* IS_ENABLED(CONFIG_PSAMPLE) */
