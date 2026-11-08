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

#include <linux/types.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/seq_file.h>

#include <linux/mlx_sx/kernel_user.h>
#include <linux/mlx_sx/cmd.h>

#include "sx.h"
#include "dev_db.h"
#include "sx_dbg_dump_proc.h"
#include "ber_monitor.h"

struct ber_bitmask_set_work_data {
    struct sx_priv    *priv;
    u16                local_port;
    u8                 bitmask;
    struct completion *wq_completion;
    int                ret;
    struct work_struct work;
};
struct ber_work_data {
    struct sx_priv    *priv;
    u16                local_port;
    struct work_struct work;
};

int sx_core_ber_monitor_dev_init(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);

    sxd_log_info(dev, "ber-monitor init\n");

    spin_lock_init(&priv->bm_info.lock);
    memset(&priv->bm_info.port_ber_monitor_bitmask, 0, sizeof(priv->bm_info.port_ber_monitor_bitmask));
    memset(&priv->bm_info.port_ber_monitor_state, 0, sizeof(priv->bm_info.port_ber_monitor_state));

    priv->bm_info.wq = sx_health_check_create_monitored_workqueue(dev, "ber_mon");
    return (priv->bm_info.wq != NULL) ? 0 : -ENOMEM;
}

void sx_core_ber_monitor_dev_deinit(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);

    sxd_log_info(dev, "ber-monitor deinit\n");

    flush_workqueue(priv->bm_info.wq);
    sx_health_check_destroy_monitored_workqueue(dev, priv->bm_info.wq);
}

int sx_core_ber_monitor_set_state(struct sx_dev *dev, u16 local_port, u8 state)
{
    struct sx_priv *priv = sx_priv(dev);
    int             err = 0;

    if (local_port > MAX_PHYPORT_NUM) {
        sxd_log_err(dev, "Ber-Monitor (set-state): local port %u is invalid (max. %u)\n",
                    local_port,
                    MAX_PHYPORT_NUM);
        err = -EINVAL;
        goto out;
    }

    spin_lock_bh(&priv->bm_info.lock);
    priv->bm_info.port_ber_monitor_state[local_port] = state;
    spin_unlock_bh(&priv->bm_info.lock);

out:
    return err;
}

static void __ber_monitor_bitmask_work_handler(struct work_struct *work)
{
    struct ber_bitmask_set_work_data *ber_wdata = container_of(work,
                                                               struct ber_bitmask_set_work_data,
                                                               work);
    struct sx_priv *priv = ber_wdata->priv;
    u16             local_port = ber_wdata->local_port;
    u8              bitmask = ber_wdata->bitmask;

    spin_lock_bh(&priv->bm_info.lock);
    priv->bm_info.port_ber_monitor_bitmask[local_port] = bitmask;

    /* If BER monitor is disabled, clear the operational state */
    if (bitmask == 0) {
        priv->bm_info.port_ber_monitor_state[local_port] = 0;
    }

    spin_unlock_bh(&priv->bm_info.lock);
    complete(ber_wdata->wq_completion);
}

/* this call should be out of the lock, it will schedule a work to do the job
 * inside of the lock
 */
int sx_core_ber_monitor_set_bitmask(struct sx_dev *dev, u16 local_port, u8 bitmask)
{
    struct sx_priv                  *priv = sx_priv(dev);
    struct completion                __ber_wq_completion;
    struct ber_bitmask_set_work_data ber_wdata;
    int                              err = 0;

    if (local_port > MAX_PHYPORT_NUM) {
        sxd_log_err(dev, "Ber-Monitor (set-bitmask): local_port %u is invalid (max. %u)\n",
                    local_port,
                    MAX_PHYPORT_NUM);
        err = -EINVAL;
        goto out;
    }

    memset(&ber_wdata, 0, sizeof(ber_wdata));

    /* The ber monitor DB update is done in worker thread to prevent
     * race between DB update and PPBME handler */
    init_completion(&__ber_wq_completion);
    ber_wdata.priv = priv;
    ber_wdata.local_port = local_port;
    ber_wdata.bitmask = bitmask;
    ber_wdata.wq_completion = &__ber_wq_completion;
    INIT_WORK(&ber_wdata.work, __ber_monitor_bitmask_work_handler);
    queue_work(priv->bm_info.wq, &ber_wdata.work);

    /* Wait for DB update completion for synchronous ioctl */
    do {
        err = wait_for_completion_interruptible(&__ber_wq_completion);
    } while (err == -ERESTARTSYS);
    err = ber_wdata.ret;

    if (err) {
        sxd_log_err(dev,
                    "Ber-Monitor (set-bitmask): failed to set bitmask (dev=%u, port=%u, bitmask=%u)\n",
                    dev->device_id,
                    local_port,
                    bitmask);
    }

out:
    return err;
}

static void __ber_monitor_work_handler(struct work_struct *work)
{
    struct ber_work_data      *ber_wdata = container_of(work, struct ber_work_data, work);
    struct sx_priv            *priv = ber_wdata->priv;
    struct ku_access_ppbmc_reg ppbmc_reg_data;
    int                        err = 0, to_rearm = 0;

    memset(&ppbmc_reg_data, 0, sizeof(ppbmc_reg_data));

    ppbmc_reg_data.dev_id = priv->dev.device_id;
    sx_cmd_set_op_tlv(&ppbmc_reg_data.op_tlv, PPBMC_REG_ID, EMAD_METHOD_QUERY);

    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(ppbmc_reg_data.ppbmc_reg.local_port,
                                        ppbmc_reg_data.ppbmc_reg.lp_msb,
                                        ber_wdata->local_port);

    spin_lock_bh(&priv->bm_info.lock);
    if (priv->bm_info.port_ber_monitor_bitmask[ber_wdata->local_port]) {
        to_rearm = 1;
    }
    spin_unlock_bh(&priv->bm_info.lock);

    if (!to_rearm) {
        sxd_log_rl_notice(SXD_PRIV_DEV(priv), "Ber-Monitor (event-handler): "
                          "local port [dev-id=%u, local_port=%u] is disabled, skip re-arm\n",
                          priv->dev.device_id, ber_wdata->local_port);
        goto out;
    }

    /* Read PPBMC */
    err = sx_ACCESS_REG_PPBMC_wrapper(&priv->dev, EMAD_METHOD_QUERY, &ppbmc_reg_data);
    if (err) {
        sxd_log_err(SXD_PRIV_DEV(priv), "Ber-Monitor (event-handler) [dev_id=%u, local_port=%u]: "
                    "query PPBMC failed (err %d)\n",
                    priv->dev.device_id, ber_wdata->local_port, err);
        goto out;
    }

    /* If armed - no need to re-arm */
    if (ppbmc_reg_data.ppbmc_reg.e != 0) {
        sxd_log_rl_notice(SXD_PRIV_DEV(priv), "Ber-Monitor (event-handler): Already armed, skip re-arm "
                          "[dev=%u, port=%u, to_rearm=%u, e=%u)\n",
                          priv->dev.device_id, ber_wdata->local_port, to_rearm, ppbmc_reg_data.ppbmc_reg.e);
        goto out;
    }

    ppbmc_reg_data.ppbmc_reg.e = 2; /* Generate single event */
    ppbmc_reg_data.ppbmc_reg.monitor_type = 0; /* monitor type is RO field */
    sx_cmd_set_op_tlv(&ppbmc_reg_data.op_tlv, PPBMC_REG_ID, EMAD_METHOD_WRITE);

    /* Set PPBMC */
    err = sx_ACCESS_REG_PPBMC_wrapper(&priv->dev, EMAD_METHOD_WRITE, &ppbmc_reg_data);
    if (err) {
        sxd_log_err(SXD_PRIV_DEV(priv), "Ber-Monitor (event-handler) [dev_id=%u, local_port=%u]: "
                    "set PPBMC failed (err %d)\n",
                    priv->dev.device_id, ber_wdata->local_port, err);
    }

out:
    /* Free memory */
    kfree(ber_wdata);
}

/* this call should be out of the lock */
void sx_core_ber_monitor_handle_ppbme_event(struct completion_info *ci)
{
    struct sxd_emad_ppbme_reg* ppbme = (struct sxd_emad_ppbme_reg *)ci->skb->data;
    struct sx_priv            *priv = sx_priv(ci->dev);
    struct ber_work_data      *ber_wdata = NULL;
    u8                         monitor_state = 0, old_monitor_state = 0, ber_monitor_bitmask = 0;
    u16                        local_port = 0;

    SX_PORT_BUILD_PHY_ID_FROM_LSB_MSB(local_port, ppbme->local_port, ((ppbme->pnat_msb_monitor_type >> 4) & 0x3));

    if (local_port > MAX_PHYPORT_NUM) {
        sxd_log_err(SXD_DEV_FROM_COMP_INFO(ci), "Ber-Monitor (event-handler) [dev_id=%u, local_port=%u]: "
                    "received invalid port (max. %d)\n",
                    ci->dev_id, local_port, MAX_PHYPORT_NUM);
        return;
    }

    spin_lock_bh(&priv->bm_info.lock);

    sxd_log_rl_notice(SXD_DEV_FROM_COMP_INFO(ci), "Ber-Monitor (event-handler): [dev=%u, port=%u]\n",
                      ci->dev_id, local_port);

    old_monitor_state = priv->bm_info.port_ber_monitor_state[local_port];
    priv->bm_info.port_ber_monitor_state[local_port] = ppbme->monitor_state;
    ber_monitor_bitmask = priv->bm_info.port_ber_monitor_bitmask[local_port];

    spin_unlock_bh(&priv->bm_info.lock);

    /* If monitor state didn't change: silently drop the event */
    monitor_state = ppbme->monitor_state;
    if (old_monitor_state == monitor_state) {
        ci->info.dont_care.drop_enable = 1;
        goto rearm;
    }

    /* If user didn't request to be notified for the received monitor state:
     * Silently drop the event  */
    if (monitor_state == 0) {
        ci->info.dont_care.drop_enable = 1;
    } else if ((1 << (monitor_state - 1)) & ber_monitor_bitmask) {
        ci->info.dont_care.drop_enable = 0;
    } else {
        ci->info.dont_care.drop_enable = 1;
    }

rearm:
    /* Re-arm */
    ber_wdata = kzalloc(sizeof(*ber_wdata), GFP_ATOMIC);
    if (!ber_wdata) {
        sxd_log_err(SXD_DEV_FROM_COMP_INFO(ci), "Ber-Monitor (event-handler) [dev_id=%u, local_port=%u]: "
                    "failed to allocate re-arm event\n",
                    ci->dev_id, local_port);
        return;
    }

    ber_wdata->priv = priv;
    ber_wdata->local_port = local_port;
    INIT_WORK(&ber_wdata->work, __ber_monitor_work_handler);
    queue_work(priv->bm_info.wq, &ber_wdata->work);
}

int sx_core_ber_monitor_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_priv *priv;
    const char    * state_2_str[] = {"Disable", "Normal", "Warning", "Alarm"};
    const char    * bitmask_2_str[] = {"Disable", "N", "W", "NW", "A", "AN", "AW", "ANW"};
    u16             local_port = 0;
    u8              bitmask = 0;
    u8              state = 0;
    struct sx_dev * dev = sx_dbg_dump_get_device(m);

    if (!dev) {
        return 0;
    }

    priv = sx_priv(dev);

    sx_dbg_dump_print_header(m, "BER monitor dump");

    sx_dbg_dump_print_table_header(m, "%-11s| %-11s| %-11s", "port", "bitmask", "state");

    spin_lock_bh(&priv->bm_info.lock);

    for (local_port = 0; local_port <= MAX_PHYPORT_NUM; local_port++) {
        if (priv->bm_info.port_ber_monitor_bitmask[local_port] != 0) {
            bitmask = priv->bm_info.port_ber_monitor_bitmask[local_port];
            state = priv->bm_info.port_ber_monitor_state[local_port];

            if ((bitmask < 8) && (state < 4)) {
                seq_printf(m, "%-11u| %-11s| %-11s\n",
                           local_port,
                           bitmask_2_str[bitmask],
                           state_2_str[state]);
            } else {
                seq_printf(m, "%-11u| %-11d| %-11d\n",
                           local_port,
                           bitmask,
                           state);
            }
        }
    }

    spin_unlock_bh(&priv->bm_info.lock);

    sx_dbg_dump_print_empty_line(m);
    return 0;
}
