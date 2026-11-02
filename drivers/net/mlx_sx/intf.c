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

#include <linux/mlx_sx/driver.h>
#include <linux/mlx_sx/device.h>
#include "sx.h"
#include "sgmii.h"
#include "alloc.h"
#include "health_check.h"
#include "sx_dbg_dump_proc.h"

struct sx_device_context {
    struct list_head     list;
    struct sx_interface *intf;
    void                *context;
};

static LIST_HEAD(__intf_list);
static LIST_HEAD(__dev_list);
static DEFINE_MUTEX(__lock);

/* must be called under __lock */
static void __sx_core_add_device(struct sx_interface *intf, struct sx_priv *priv,  struct sx_flow_info *fi)
{
    struct sx_device_context *dev_ctx;
    union sx_event_data      *event_data, *rpa_init_event_data;
    int                       i;
    u8                        ib_swid_present = 0;

    dev_ctx = kmalloc(sizeof *dev_ctx, GFP_KERNEL);
    if (!dev_ctx) {
        return;
    }

    event_data = kmalloc(sizeof(union sx_event_data), GFP_KERNEL);
    if (!event_data) {
        kfree(dev_ctx);
        return;
    }

    rpa_init_event_data = kzalloc(sizeof(union sx_event_data), GFP_KERNEL);
    if (!rpa_init_event_data) {
        kfree(dev_ctx);
        kfree(event_data);
        return;
    }

    dev_ctx->intf = intf;
    dev_ctx->context = intf->add(&priv->dev, fi);

    if (dev_ctx->context) {
        for (i = 0; i < NUMBER_OF_SWIDS; i++) {
            memset(event_data, 0, sizeof(*event_data));
            if (sx_bitmap_test(&priv->swid_bitmap, i)) {
                if (priv->profile.pci_profile.swid_type[i] ==
                    SX_KU_L2_TYPE_IB) {
                    ib_swid_present = 1;
                    sxd_log_debug(SXD_PRIV_DEV(priv), "sx_core_add_device: "
                                  "dispatching IB swid up event\n");
                    event_data->ib_swid_change.swid = i;
                    event_data->ib_swid_change.dev_id = 1;
                    dev_ctx->intf->event(&priv->dev,
                                         dev_ctx->context,
                                         SX_DEV_EVENT_IB_SWID_UP, event_data);
                    if (priv->profile.pci_profile.ipoib_router_port_enable[i]) {
                        rpa_init_event_data->rpa_init.swid[rpa_init_event_data->rpa_init.num_of_ib_swids++] = i;
                    }
                } else if (priv->profile.pci_profile.swid_type[i] ==
                           SX_KU_L2_TYPE_ETH) {
                    sxd_log_debug(SXD_PRIV_DEV(priv), "sx_core_add_device: "
                                  "dispatching ETH swid up event\n");
                    event_data->eth_swid_up.swid = i;
                    event_data->eth_swid_up.synd = priv->swid_data[i].eth_swid_data.synd;
                    event_data->eth_swid_up.mac = priv->swid_data[i].eth_swid_data.mac;
                    dev_ctx->intf->event(&priv->dev, dev_ctx->context, SX_DEV_EVENT_ETH_SWID_UP, event_data);
                }
            }
        }

        if (ib_swid_present) {
            if (rpa_init_event_data->rpa_init.num_of_ib_swids != 0) {
                rpa_init_event_data->rpa_init.max_pkey =
                    priv->profile.pci_profile.max_pkey;
                dev_ctx->intf->event(&priv->dev,
                                     dev_ctx->context,
                                     SX_DEV_EVENT_TYPE_RPA_INIT,
                                     rpa_init_event_data);
            }
        }

        list_add_tail(&dev_ctx->list, &priv->ctx_list);
    } else {
        kfree(dev_ctx);
    }

    kfree(rpa_init_event_data);
    kfree(event_data);
}

/* must be called under __lock */
static void __sx_core_remove_device(struct sx_interface *intf, struct sx_priv *priv, struct sx_flow_info *fi)
{
    struct sx_device_context *dev_ctx, *tmp;

    list_for_each_entry_safe(dev_ctx, tmp, &priv->ctx_list, list) {
        if (dev_ctx->intf == intf) {
            list_del(&dev_ctx->list);

            intf->remove(&priv->dev, dev_ctx->context, fi);
            kfree(dev_ctx);
            return;
        }
    }
}

int sx_register_interface(struct sx_interface *intf)
{
    struct sx_priv     *priv;
    struct sx_flow_info fi = { .in_pci_restart = false };

    if (!intf->add || !intf->remove) {
        return -EINVAL;
    }

    mutex_lock(&__lock);
    list_add_tail(&intf->list, &__intf_list);
    list_for_each_entry(priv, &__dev_list, dev_list) {
        __sx_core_add_device(intf, priv, &fi);
    }

    mutex_unlock(&__lock);

    return 0;
}
EXPORT_SYMBOL(sx_register_interface);

void sx_unregister_interface(struct sx_interface *intf)
{
    struct sx_priv     *priv;
    struct sx_flow_info fi = { .in_pci_restart = false};

    mutex_lock(&__lock);
    list_for_each_entry(priv, &__dev_list, dev_list) {
        __sx_core_remove_device(intf, priv, &fi);
    }

    list_del(&intf->list);

    mutex_unlock(&__lock);
}
EXPORT_SYMBOL(sx_unregister_interface);

int sx_core_dispatch_event(struct sx_dev *dev, enum sx_dev_event type, union sx_event_data *event_data)
{
    struct sx_priv           *priv = sx_priv(dev);
    struct sx_device_context *dev_ctx;
    int                       err, err_ret = 0;

    mutex_lock(&__lock);

    list_for_each_entry(dev_ctx, &priv->ctx_list, list) {
        if (dev_ctx->intf->event) {
            err = dev_ctx->intf->event(dev, dev_ctx->context, type, event_data);
            if (err && (err_ret == 0)) {
                err_ret = err;
            }
        }
    }

    mutex_unlock(&__lock);
    return err_ret;
}

int sx_core_register_device(struct sx_dev *dev, struct sx_flow_info *fi)
{
    struct sx_priv      *priv = sx_priv(dev);
    struct sx_interface *intf;

    mutex_lock(&__lock);
    list_add_tail(&priv->dev_list, &__dev_list);
    list_for_each_entry(intf, &__intf_list, list) {
        __sx_core_add_device(intf, priv, fi);
    }

    mutex_unlock(&__lock);
    return 0;
}

void sx_core_unregister_device(struct sx_dev *dev, struct sx_flow_info *fi)
{
    struct sx_priv      *priv = sx_priv(dev);
    struct sx_interface *intf;

    mutex_lock(&__lock);
    list_for_each_entry(intf, &__intf_list, list) {
        __sx_core_remove_device(intf, priv, fi);
    }

    list_del(&priv->dev_list);

    mutex_unlock(&__lock);
}

int sx_intf_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_interface      *intf;
    struct sx_priv           *priv = NULL;
    struct sx_dev            *dev = sx_dbg_dump_get_device(m);
    struct sx_device_context *dev_ctx;

    if (!dev) {
        return 0;
    }

    priv = sx_priv(dev);

    mutex_lock(&__lock);

    list_for_each_entry(intf, &__intf_list, list) {
        seq_printf(m, "Interface: %s (%p)\n", ((intf->name[0] == 0) ? "N/A" : intf->name), intf);
    }

    list_for_each_entry(dev_ctx, &priv->ctx_list, list) {
        seq_printf(m,
                   "ctx_list: %s (%p)\n",
                   ((dev_ctx->intf->name[0] == 0) ? "N/A" : dev_ctx->intf->name),
                   dev_ctx->intf);
    }

    mutex_unlock(&__lock);
    return 0;
}
