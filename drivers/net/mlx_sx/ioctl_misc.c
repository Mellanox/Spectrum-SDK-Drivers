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

#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/pagemap.h>
#include <linux/mlx_sx/cmd.h>
#include <linux/mlx_sx/kernel_user.h>
#include "sx.h"
#include "dq.h"
#include "alloc.h"
#include "ptp.h"
#include "sgmii.h"
#include "ioctl_internal.h"
#include "bulk_cntr_event.h"
#include "bulk_cntr_db.h"
#include "sx_af_counters.h"
#include "dev_init.h"

/*defines for host memory read ioctl*/
#define NUM_PAGES_IN_PAGE_ARRY                                   (SX_ICM_ALLOC_SIZE / PAGE_SIZE) /* 2M/4K = 512 */
#define MLX_TRACE_PAGE_OFFSET_IN_HOST_MEM_ALLOCATED_MAP_FA_PAGES 5
#define MLX_TRACE_MAX_PAGE_NUM                                   512
#define MAX_CHUNK_PAGE_NUM                                       (SX_ICM_CHUNK_LEN * NUM_PAGES_IN_PAGE_ARRY)

static bool g_sdk_health_en[MAX_NUM_OF_REMOTE_SWITCHES + 1] = {0};
static DEFINE_MUTEX(sdk_health_mutex);

bool sdk_health_test_and_disable(uint8_t dev_id)
{
    bool ret = false;

    if (!DEV_ID_IS_VALID(dev_id)) {
        return false;
    }

    mutex_lock(&sdk_health_mutex);
    ret = g_sdk_health_en[dev_id];
    g_sdk_health_en[dev_id] = 0;
    mutex_unlock(&sdk_health_mutex);

    return ret;
}


void sdk_health_set(uint8_t dev_id, bool is_enabled)
{
    if (!DEV_ID_IS_VALID(dev_id)) {
        return;
    }

    mutex_lock(&sdk_health_mutex);
    g_sdk_health_en[dev_id] = is_enabled;
    mutex_unlock(&sdk_health_mutex);
}

bool sdk_health_get(uint8_t dev_id)
{
    bool is_enabled;

    if (!DEV_ID_IS_VALID(dev_id)) {
        return false;
    }

    mutex_lock(&sdk_health_mutex);
    is_enabled = g_sdk_health_en[dev_id];
    mutex_unlock(&sdk_health_mutex);
    return is_enabled;
}

long ctrl_cmd_get_capabilities(struct file *file, unsigned int cmd, unsigned long data)
{
    struct sx_dev  *dev;
    struct sx_priv *priv;

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    priv = sx_priv(dev);
    return copy_to_user((void*)data, (void*)&priv->dev_cap, sizeof(priv->dev_cap));
}


long ctrl_cmd_issu_fw(struct file *file, unsigned int cmd, unsigned long data)
{
    struct sx_dev            *dev;
    int                       err = 0;
    u8                        status = 0;
    u8                        retry_num = 0;
    u16                       system_status = 0;
    struct ku_set_issu_fw_cmd issu_fw_cmd;

    err = copy_from_user(&issu_fw_cmd, (struct ku_set_issu_fw_cmd *)data, sizeof(issu_fw_cmd));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &issu_fw_cmd.dev_id);
    if (!dev) {
        return -ENODEV;
    }

#define NUM_OF_QUERIES 20
    /* Verify ISSU FW state is idle */
    err = sx_ISSU_FW_STATUS_GET(dev, &status);
    if (err) {
        sxd_log_err(dev, "Fail to get ISSU FW state - before performing ISSU.\n");
        goto out;
    }

    if (status) {
        sxd_log_err(dev,
                    "Fail to start ISSU FW: Cannot start while ISSU FW is not idle! status is (%u).\n",
                    status);
        err = -EPERM;
        goto out;
    }

    /* Perform ISSU FW */
    err = sx_ISSU_FW_START(dev, issu_fw_cmd.fw_cmd);
    if (err) {
        sxd_log_err(dev, "Fail to start ISSU FW.\n");
        goto out;
    }

    /* Wait for ISSU FW to end */
    while (retry_num < NUM_OF_QUERIES) {
        err = sx_ISSU_FW_STATUS_GET(dev, &status);
        if (err) {
            sxd_log_err(dev, "Fail to get ISSU FW state - after performing ISSU.\n");
            if (!get_system_status(dev, &system_status)) {
                sxd_log_err(dev, "Device %u status is 0x%x\n", dev->device_id, system_status);
            }
            goto out;
        }
        if (status) {
            msleep(10);
            retry_num++;
        } else {
            sxd_log_info(dev, "ISSU FW DONE.\n");
            break;
        }
    }

    if (retry_num >= NUM_OF_QUERIES) {
        sxd_log_err(dev, "ISSU FW timeout. status is (%u)\n", status);
        err = -ETIME;
        goto out;
    }

out:
    return err;
}


long ctrl_cmd_enable_swid(struct file *file, unsigned int cmd, unsigned long data)
{
    struct sx_dev *dev;

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    return sx_core_ioctl_enable_swid(dev, data);
}


long ctrl_cmd_disable_swid(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_swid_details swid_data;
    struct sx_dev         *dev;
    struct sx_priv        *priv;
    int                    err;

    err = copy_from_user(&swid_data, (void*)data, sizeof(swid_data));
    if (err) {
        return err;
    }

    sxd_log_notice(SXD_DEV_NONE, "Disable SWID %u for device %u\n", swid_data.swid, swid_data.dev_id);

    sx_core_get_dev_and_id_from_fd(file, &dev, &swid_data.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    priv = sx_priv(dev);

    spin_lock(&priv->profile.profile_lock);
    if (!priv->profile.pci_profile_set || (swid_data.swid >= NUMBER_OF_SWIDS) ||
        !sx_bitmap_test(&priv->swid_bitmap, swid_data.swid)) {
        spin_unlock(&priv->profile.profile_lock);
        return -EINVAL;
    }

    spin_unlock(&priv->profile.profile_lock);
    sx_disable_swid(dev, swid_data.swid);
    return 0;
}

#ifdef SW_PUDE_EMULATION
/* PUDE WA for NOS (PUDE events are handled by SDK). Needed for BU. */
long ctrl_cmd_set_port_admin_status(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_admin_status_data admin_status_data;
    struct sx_dev              *dev;
    unsigned long               flags;
    int                         err;

    err = copy_from_user(&admin_status_data, (void*)data, sizeof(admin_status_data));
    if (err) {
        return err;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &admin_status_data.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    spin_lock_irqsave(&sx_priv(dev)->db_lock, flags);
    sx_priv(dev)->admin_status_sysport_db[admin_status_data.sysport] = admin_status_data.admin_status;
    spin_unlock_irqrestore(&sx_priv(dev)->db_lock, flags);

    return 0;
}
#endif /* SW_PUDE_EMULATION */

long ctrl_cmd_rearm_module_event(struct file *file, unsigned int cmd, unsigned long data)
{
    int                          err = 0;
    struct sx_dev               *dev;
    struct ku_rearm_module_event rearm;

    err = copy_from_user(&rearm, (void*)data, sizeof(rearm));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &rearm.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    err =
        sx_core_rearm_module_event(dev, rearm.slot, rearm.module, (sxd_pmaos_e_t)rearm.gen_mode,
                                   &(rearm.ret));
    if (err) {
        sxd_log_err(dev, "Failed in sx_core_rearm_module_event.\n");
        goto out;
    }

    err = copy_to_user((void*)data, &rearm, sizeof(rearm));
    if (err) {
        goto out;
    }
out:
    return err;
}


long ctrl_cmd_set_ptp_mode(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_ptp_mode ptp_mode;
    struct sx_dev     *dev;
    int                err;

    err = copy_from_user(&ptp_mode, (void*)data, sizeof(ptp_mode));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    if ((ptp_mode.ptp_mode != KU_PTP_MODE_EVENTS) &&
        (ptp_mode.ptp_mode != KU_PTP_MODE_POLLING) &&
        (ptp_mode.ptp_mode != KU_PTP_MODE_DISABLED)) {
        err = -EINVAL;
        goto out;
    }

    if (ptp_mode.ptp_mode != KU_PTP_MODE_DISABLED) {
        err = sx_core_ptp_dev_init(sx_priv(dev), &ptp_mode);
        if (err) {
            sxd_log_err(dev, "__sx_core_ptp_init failed, err:%d\n", err);
            goto out;
        }

        err = copy_to_user((void*)data, &ptp_mode, sizeof(ptp_mode));
        if (err) {
            goto out;
        }
    } else { /* ptp_mode.ptp_mode == KU_PTP_MODE_DISABLED */
        sx_priv(dev)->ptp.hw_cleanup_required = true;
        err = sx_core_ptp_dev_cleanup(sx_priv(dev));
        if (err) {
            sxd_log_err(dev, "__sx_core_ptp_cleanup failed, err:%d\n", err);
        }
    }

out:
    return err;
}


static uint8_t __sx_is_default_device(int dev_id)
{
    int default_dev_id;
    int ret;

    if (is_sgmii_supported()) { /* Chassis managed by OOB network */
        ret = sgmii_default_dev_id_get(&default_dev_id);
        return (ret) ? 0 : (dev_id == default_dev_id);
    }

    /* Chassis managed by PCI & in-band network */
    return (sx_glb.sx_dpt.dpt_info[dev_id].cmd_path == DPT_PATH_PCI_E);
}


long ctrl_cmd_set_sw_ib_node_desc(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_ib_node_description node_desc;
    union sx_event_data          *event_data;
    struct sx_dev                *dev;
    int                           err;

    err = copy_from_user(&node_desc, (struct ku_ib_node_description*)data, sizeof(node_desc));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &node_desc.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    if (!__sx_is_default_device(node_desc.dev_id)) {
        /* setting node description only on chassis's default device. ignoring silently. */
        goto out;
    }

    sxd_log_info(dev, "Setting node description on device %u\n", node_desc.dev_id);

    event_data = kzalloc(sizeof(union sx_event_data), GFP_KERNEL);
    if (!event_data) {
        err = -ENOMEM;
        goto out;
    }

    event_data->node_desc_update.swid = node_desc.swid;
    memcpy(event_data->node_desc_update.NodeDescription,
           node_desc.node_description,
           sizeof(event_data->node_desc_update.NodeDescription));

    sx_core_dispatch_event(dev, SX_DEV_EVENT_NODE_DESC_UPDATE, event_data);
    kfree(event_data);

out:
    return err;
}


long ctrl_cmd_set_sw_ib_up_down(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_ib_swid_up_down event_params;
    union sx_event_data      *event_data;
    struct sx_dev            *dev;
    int                       err;

    err = copy_from_user(&event_params, (struct ku_ib_node_description*)data, sizeof(event_params));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    sxd_log_info(dev, "Setting swid [%u] device [%u] [%s]\n",
                 event_params.swid,
                 event_params.dev_id,
                 event_params.up ? "up" : "down");

    event_data = kzalloc(sizeof(union sx_event_data), GFP_KERNEL);
    if (!event_data) {
        err = -ENOMEM;
        goto out;
    }

    event_data->ib_swid_change.swid = event_params.swid;
    event_data->ib_swid_change.dev_id = event_params.dev_id;

    sx_core_dispatch_event(dev, event_params.up ? SX_DEV_EVENT_IB_SWID_UP : SX_DEV_EVENT_IB_SWID_DOWN, event_data);
    kfree(event_data);

out:
    return err;
}


long ctrl_cmd_bulk_cntr_tr_add(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_bulk_cntr_transaction_add bulk_cntr_tr_add;
    int                                 err;
    struct sx_dev                      *dev = NULL;

    err = copy_from_user(&bulk_cntr_tr_add, (void*)data, sizeof(bulk_cntr_tr_add));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    err = bulk_cntr_db_add(dev, &bulk_cntr_tr_add);

out:
    return err;
}

long ctrl_cmd_bulk_cntr_tr_finalize(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_bulk_cntr_transaction_finalize finalize;
    int                                      err;
    struct sx_dev                           *dev = NULL;

    err = copy_from_user(&finalize, (void*)data, sizeof(finalize));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    err = bulk_cntr_db_finalize(dev,
                                finalize.client_pid,
                                finalize.buffer_id,
                                finalize.is_streaming,
                                finalize.cookie);
out:
    return err;
}

long ctrl_cmd_bulk_cntr_tr_del(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_bulk_cntr_transaction bulk_cntr_tr_del;
    int                             err;
    struct sx_dev                  *dev = NULL;

    err = copy_from_user(&bulk_cntr_tr_del, (void*)data, sizeof(bulk_cntr_tr_del));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    err = bulk_cntr_db_del(dev,
                           bulk_cntr_tr_del.client_pid,
                           bulk_cntr_tr_del.buffer_id,
                           bulk_cntr_tr_del.is_streaming_buffer);
out:
    return err;
}


long ctrl_cmd_bulk_cntr_tr_cancel(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_bulk_cntr_transaction bulk_cntr_tr_cancel;
    int                             err;
    struct sx_dev                  *dev = NULL;

    err = copy_from_user(&bulk_cntr_tr_cancel, (void*)data, sizeof(bulk_cntr_tr_cancel));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    err = bulk_cntr_db_cancel(dev,
                              bulk_cntr_tr_cancel.client_pid,
                              bulk_cntr_tr_cancel.buffer_id,
                              bulk_cntr_tr_cancel.is_streaming_buffer);

out:
    return err;
}


long ctrl_cmd_bulk_cntr_tr_ack(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_bulk_cntr_transaction_ack bulk_cntr_tr_ack;
    int                                 err;
    struct sx_dev                      *dev = NULL;

    err = copy_from_user(&bulk_cntr_tr_ack, (void*)data, sizeof(bulk_cntr_tr_ack));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    err = sx_bulk_cntr_handle_ack(dev, &bulk_cntr_tr_ack.event_id, bulk_cntr_tr_ack.buffer_id);
    if (err) {
        goto out;
    }

out:
    return err;
}


long ctrl_cmd_bulk_cntr_tr_continue_ack(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_bulk_cntr_transaction_ack bulk_cntr_tr_ack;
    int                                 err;
    struct sx_dev                      *dev = NULL;

    err = copy_from_user(&bulk_cntr_tr_ack, (void*)data, sizeof(bulk_cntr_tr_ack));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    err = sx_bulk_cntr_handle_continue_ack(dev, &bulk_cntr_tr_ack.event_id, bulk_cntr_tr_ack.buffer_id);
    if (err) {
        goto out;
    }

out:
    return err;
}

long ctrl_cmd_send_issu_notification(struct file *file, unsigned int cmd, unsigned long data)
{
    struct sx_dev  *dev = NULL;
    struct sx_priv *priv = NULL;
    int             err = 0;
    unsigned long   is_issu_start = 0;

    /* 'data' is a pointer but a boolean value:
     * 0 - ISSU finished, FW/ASIC is accessible
     * 1 - ISSU started, FW/ASIC is inaccessible
     */
    err = copy_from_user(&is_issu_start, (void*)data, sizeof(unsigned long));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    if (!dev->pdev) {
        sxd_log_err(dev, "ISSU-in-progress: PCI device is not attached\n");
        err = -ENODEV;
        goto out;
    }

    priv = sx_priv(dev);

    /* Send ASIC not ready notification */
    sx_send_udev_event(dev->pdev, priv, KOBJ_REMOVE, "ISSU notification");


    /* Stop the RDQ monitor thread.
     * During ISSU there is no traffic to CPU, and QUERY_CQ is not allowed
     * between consolidations (MPCIR).
     * RDQ monitor will be restored during probe flow (either PCI restart
     * or regular sx_core probe).
     */
    sx_core_rdq_usage_monitor_deinit(dev);

    sx_core_sysfs_dev_disable(dev, "ISSU-Start"); /* SysFS will be enabled in SDK init. */

    err = sx_core_dispatch_event(dev, SX_DEV_EVENT_ISSU_START, NULL);
    if (err) {
        sxd_log_err(dev, "ISSU event failed, err:%d\n", err);
        goto out;
    }

out:
    return err;
}

long ctrl_cmd_bulk_cntr_tr_in_progress(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_bulk_cntr_transaction bulk_cntr_tr_in_progress;
    int                             err;
    struct sx_dev                  *dev = NULL;

    err = copy_from_user(&bulk_cntr_tr_in_progress, (void*)data, sizeof(bulk_cntr_tr_in_progress));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    err = bulk_cntr_db_in_progress(dev, &bulk_cntr_tr_in_progress, (void*)data);
    if (err) {
        goto out;
    }

out:
    return err;
}

long ctrl_cmd_bulk_cntr_tr_in_progress_info(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_bulk_cntr_transaction_in_progress_info bulk_cntr_tr_in_progress_info;
    int                                              err;
    struct sx_dev                                   *dev = NULL;

    err = copy_from_user(&bulk_cntr_tr_in_progress_info, (void*)data, sizeof(bulk_cntr_tr_in_progress_info));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    err = bulk_cntr_db_in_progress_info_get(dev, &bulk_cntr_tr_in_progress_info, (void*)data);
    if (err) {
        goto out;
    }

out:
    return err;
}

long ctrl_cmd_bulk_cntr_per_prio_cache_set(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_bulk_cntr_per_prio_cache bulk_cntr_per_prio_cache;
    int                                err;
    struct sx_dev                     *dev = NULL;

    err = copy_from_user(&bulk_cntr_per_prio_cache, (void*)data, sizeof(bulk_cntr_per_prio_cache));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    err = bulk_cntr_db_per_prio_cache_set(dev, &bulk_cntr_per_prio_cache);

out:
    return err;
}

long ctrl_cmd_bulk_cntr_buffer_stats_set(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_bulk_cntr_buffer_stats bulk_cntr_buffer_stats_data;
    int                              err;
    struct sx_dev                   *dev = NULL;

    err = copy_from_user(&bulk_cntr_buffer_stats_data, (void*)data, sizeof(bulk_cntr_buffer_stats_data));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    err = bulk_cntr_db_buffer_stats_set(dev, &bulk_cntr_buffer_stats_data);

out:
    return err;
}

long ctrl_cmd_bulk_cntr_buffer_info_get(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_bulk_cntr_buffer_info bulk_cntr_buffer_info;
    int                             err;
    struct sx_dev                  *dev = NULL;

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    err = bulk_cntr_db_buffer_info_get(dev, &bulk_cntr_buffer_info);
    if (err) {
        goto out;
    }

    err = copy_to_user((void*)data, &bulk_cntr_buffer_info, sizeof(bulk_cntr_buffer_info));
    if (err) {
        goto out;
    }

out:
    return err;
}

long ctrl_cmd_bulk_cntr_buffer_info_set(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_bulk_cntr_buffer_info bulk_cntr_buffer_info;
    int                             err;
    struct sx_dev                  *dev = NULL;

    err = copy_from_user(&bulk_cntr_buffer_info, (void*)data, sizeof(bulk_cntr_buffer_info));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    err = bulk_cntr_db_buffer_info_set(dev, &bulk_cntr_buffer_info);
    if (err) {
        goto out;
    }
out:
    return err;
}

long ctrl_cmd_bulk_cntr_stateful_db_keys_write(struct file *file, unsigned int cmd, unsigned long data)
{
    ku_stateful_db_translated_entry_t translated_entry;
    int                               err;
    struct sx_dev                    *dev = NULL;

    err = copy_from_user(&translated_entry, (void *)data,
                         sizeof(ku_stateful_db_translated_entry_t));


    if (err) {
        sxd_log_err(SXD_DEV_NONE, "Failed to copy translated stateful DB entry, err:%d\n", err);
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    err = bulk_cntr_stateful_db_entry_write(dev, &translated_entry);
    if (err) {
        sxd_log_err(dev, "Failed to write translated stateful DB entry, err:%d\n", err);
        goto out;
    }


out:
    return err;
}

long ctrl_cmd_bulk_cntr_streaming_profile_add(struct file *file, unsigned int cmd, unsigned long data)
{
    ku_bulk_cntr_streaming_profile_t bulk_cntr_streaming_profile;
    int                              err;
    struct sx_dev                   *dev = NULL;

    err = copy_from_user(&bulk_cntr_streaming_profile, (void*)data, sizeof(bulk_cntr_streaming_profile));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    err = bulk_cntr_streaming_profile_add(dev, &bulk_cntr_streaming_profile);

out:
    return err;
}

long ctrl_cmd_bulk_cntr_streaming_profile_del(struct file *file, unsigned int cmd, unsigned long data)
{
    int            err;
    struct sx_dev *dev = NULL;
    uint32_t       profile_id;

    err = copy_from_user(&profile_id, (void*)data, sizeof(profile_id));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    err = bulk_cntr_streaming_profile_del(dev, profile_id);

out:
    return err;
}

long ctrl_cmd_bulk_cntr_streaming_buffer_add(struct file *file, unsigned int cmd, unsigned long data)
{
    ku_bulk_cntr_streaming_buffer_info_t bulk_cntr_streaming_buffer;
    int                                  err;
    struct sx_dev                       *dev = NULL;

    err = copy_from_user(&bulk_cntr_streaming_buffer, (void*)data, sizeof(bulk_cntr_streaming_buffer));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    err = bulk_cntr_streaming_buffer_add(dev, &bulk_cntr_streaming_buffer);

out:
    return err;
}

long ctrl_cmd_bulk_cntr_streaming_buffer_del(struct file *file, unsigned int cmd, unsigned long data)
{
    ku_bulk_cntr_streaming_buffer_info_t bulk_cntr_streaming_buffer;
    int                                  err;
    struct sx_dev                       *dev = NULL;

    err = copy_from_user(&bulk_cntr_streaming_buffer, (void*)data, sizeof(bulk_cntr_streaming_buffer));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    err = bulk_cntr_streaming_buffer_del(dev, &bulk_cntr_streaming_buffer);

out:
    return err;
}

/*
 * The reason we return current->tgid:
 * 1. When client calls sxd_memory_map to create shared memory between kernel and application,
 *    the kernel mmap implementation is using current->tgid, which is the real PID of the client
 *    process, no matter the client process is inside a docker container or on a host.
 * 2. Inside a docker container, the getpid system call returns PID from the docker
 *    PID name space, but not the real PID. We need to provide a generic way for the client
 *    process to get the real PID, which should work for both docker and non-docker scenarios.
 */
void sx_current_pid_get(struct ku_client_pid_get *client_pid_p)
{
    client_pid_p->gtid = task_pid_nr(current);
    client_pid_p->gpid = task_tgid_nr(current);
    client_pid_p->lpid = task_tgid_vnr(current);
}
EXPORT_SYMBOL(sx_current_pid_get);

long ctrl_cmd_client_pid_get(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_client_pid_get client_pid;
    int                      err;

    sx_current_pid_get(&client_pid);
    err = copy_to_user((void*)data, &client_pid, sizeof(client_pid));
    if (err) {
        goto out;
    }

out:
    return err;
}

long ctrl_cmd_sdk_health_set(struct file *file, unsigned int cmd, unsigned long data)
{
    int                                err;
    struct ku_sdk_health_event_control sdk_health_ctl;

    err = copy_from_user(&sdk_health_ctl, (void*)data, sizeof(sdk_health_ctl));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, NULL, &sdk_health_ctl.dev_id);
    sdk_health_set(sdk_health_ctl.dev_id, sdk_health_ctl.enable);

out:
    return err;
}

long ctrl_cmd_sdk_health_get(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_sdk_health_event_control sdk_health_ctl;
    int                                err;

    err = copy_from_user(&sdk_health_ctl, (void*)data, sizeof(sdk_health_ctl));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, NULL, &sdk_health_ctl.dev_id);

    if (sdk_health_ctl.get_and_disable) {
        sdk_health_ctl.enable = sdk_health_test_and_disable(sdk_health_ctl.dev_id);
    } else {
        sdk_health_ctl.enable = sdk_health_get(sdk_health_ctl.dev_id);
    }
    err = copy_to_user((void*)data, &sdk_health_ctl, sizeof(sdk_health_ctl));
    if (err) {
        goto out;
    }

out:
    return err;
}

long ctrl_cmd_accuflow_set(struct file *file, unsigned int cmd, unsigned long data)
{
    int                                err = 0;
    struct ku_accuflow_counters_params params;
    struct sx_dev                     *dev = NULL;

    err = copy_from_user(&params, (void*)data, sizeof(params));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    switch (params.op) {
    case KU_ACCUFLOW_COUNTERS_PARAMS_OP_INIT_E:
        err = sx_af_counters_init(dev, &params);
        goto out;

    case KU_ACCUFLOW_COUNTERS_PARAMS_OP_DEINIT_E:
        err = sx_af_counters_deinit(dev);
        goto out;

    case KU_ACCUFLOW_COUNTERS_PARAMS_OP_TRASACTION_RESERVATION_E:
        err = sx_af_counters_tr_reservation_set(dev, &params.op_data.tr_reservation);
        goto out;

    default:
        err = sx_af_counters_user_job_schedule(dev, &params);
        if ((err == 0) && (params.op == KU_ACCUFLOW_COUNTERS_PARAMS_OP_READ_E)) {
            err = copy_to_user((void*)data, &params, sizeof(params));
        }
        goto out;
    }

out:
    return err;
}

long ctrl_cmd_port_module_update(struct file *file, unsigned int cmd, unsigned long data)
{
    int                                 err = 0;
    struct sx_dev                      *dev;
    struct sx_priv                     *priv;
    struct ku_port_module_update_params params;
    unsigned long                       flags;
    bool                                port_up = false;

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    priv = sx_priv(dev);

    err = copy_from_user(&params, (void*)data, sizeof(params));
    if (err) {
        goto out;
    }

    if ((params.admin_status == SXD_PAOS_ADMIN_STATUS_UP_E) ||
        (params.admin_status == SXD_PAOS_ADMIN_STATUS_UP_ONCE_E)) {
        port_up = true;
    }

    if (!port_up) {
        spin_lock_irqsave(&priv->db_lock, flags);
        priv->local_port_state[params.local_port] = params.admin_status;
        spin_unlock_irqrestore(&priv->db_lock, flags);
    }

    err = sx_core_update_module_power_mode(dev, params.slot_id, params.module_id, port_up);
    if (err) {
        sxd_log_err(dev, "Failed in sx_core_update_module_power_mode.\n");
        goto out;
    }

    if (port_up) {
        spin_lock_irqsave(&priv->db_lock, flags);
        priv->local_port_state[params.local_port] = params.admin_status;
        spin_unlock_irqrestore(&priv->db_lock, flags);
    }

out:
    return err;
}

static int sx_core_ioctl_port_module_map_set(struct sx_dev *dev, struct ku_port_module_map_set_params *params)
{
    struct sx_priv *priv = sx_priv(dev);
    int             err = 0;
    int             i = 0, j = 0;
    uint8_t         module_id = params->module_id;
    uint8_t         submodule_id = params->submodule_id;
    uint8_t         slot_id = params->slot_id;
    uint16_t        local_port = params->local_port;
    unsigned long   flags;
    uint16_t        phy_port_max = 0;
    uint8_t         locked = 0;

    if (sx_core_get_phy_port_max(dev, &phy_port_max)) {
        sxd_log_err(dev, "Failed to get max number of phy ports.\n");
        err = -EINVAL;
        goto out;
    }

    if (local_port > phy_port_max) {
        sxd_log_rl_err(dev, "Local port %d is invalid. (MAX %d).\n",
                       local_port, phy_port_max);
        err = -EINVAL;
        goto out;
    }

    if ((slot_id >= MAX_SLOT_NUM) || (module_id >= MAX_MODULE_NUM)) {
        err = -EINVAL;
        sxd_log_rl_err(dev, "slot id %d or module id %d is out of range.\n", slot_id, module_id);
        goto out;
    }

    spin_lock_irqsave(&priv->db_lock, flags);
    locked = 1;

    if (params->is_set) {
        /* Set mapping */
        for (i = 0; i < priv->modules_to_port_map_count[slot_id][module_id]; i++) {
            if (priv->module_to_ports_map[slot_id][module_id][i] == local_port) {
                goto out;
            }
        }

        if (priv->modules_to_port_map_count[slot_id][module_id] == MAX_LANE_NUM_PER_MODULE) {
            /* modules to port map is full */
            err = -EPERM;
            sxd_log_err(dev, "Modules to port map is full.\n");
            goto out;
        }
        priv->module_to_ports_map[slot_id][module_id][priv->modules_to_port_map_count[slot_id][module_id]] =
            local_port;
        priv->modules_to_port_map_count[slot_id][module_id]++;
        priv->local_to_module_map[local_port] = module_id;
        priv->local_to_submodule_map[local_port] = submodule_id;
        priv->local_to_slot_map[local_port] = slot_id;
        priv->module_to_slot_map[module_id] = slot_id;
    } else {
        /* Delete mapping */
        if (((priv->local_to_slot_map[local_port] != slot_id) ||
             (priv->local_to_module_map[local_port] != module_id)) ||
            (priv->modules_to_port_map_count[slot_id][module_id] == 0)) {
            goto out;
        }
        if (priv->module_to_slot_map[module_id] == slot_id) {
            priv->module_to_slot_map[module_id] = 0;
        }
        for (i = 0; i < priv->modules_to_port_map_count[slot_id][module_id]; i++) {
            if (priv->module_to_ports_map[slot_id][module_id][i] == local_port) {
                for (j = i; j < priv->modules_to_port_map_count[slot_id][module_id] - 1; j++) {
                    priv->module_to_ports_map[slot_id][module_id][j] =
                        priv->module_to_ports_map[slot_id][module_id][j + 1];
                }
                priv->modules_to_port_map_count[slot_id][module_id]--;
                break;
            }
        }
    }

out:
    if (locked) {
        spin_unlock_irqrestore(&priv->db_lock, flags);
    }
    return err;
}

long ctrl_cmd_port_module_map_set(struct file *file, unsigned int cmd, unsigned long data)
{
    int                             err = 0;
    struct sx_dev                  *dev;
    ku_port_module_map_set_params_t params;

    err = copy_from_user(&params, (void*)data, sizeof(params));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &params.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    err = sx_core_ioctl_port_module_map_set(dev, &params);
    if (err) {
        goto out;
    }

out:
    return err;
}

long ctrl_cmd_set_dev_info(struct file *file, unsigned int cmd, unsigned long data)
{
    int                err = 0;
    struct ku_dev_info dev_info;
    struct sx_dev     *dev;
    struct sx_priv    *priv;

    err = copy_from_user(&dev_info, (void*)data, sizeof(dev_info));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &dev_info.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    priv = sx_priv(dev);
    memcpy(&(priv->dev_info.dev_info_set), &(dev_info.dev_info.dev_info_set), sizeof(dev_info.dev_info.dev_info_set));

out:
    return err;
}

long ctrl_cmd_get_dev_info(struct file *file, unsigned int cmd, unsigned long data)
{
    struct sx_dev     *dev;
    int                err = 0;
    struct ku_dev_info dev_info;
    struct sx_priv    *priv;

    err = copy_from_user(&dev_info, (void*)data, sizeof(dev_info));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &dev_info.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    priv = sx_priv(dev);
    memcpy(&dev_info.dev_info, &priv->dev_info, sizeof(dev_info.dev_info));

    err = copy_to_user((void*)data, &dev_info, sizeof(dev_info));
    if (err) {
        goto out;
    }

out:
    return err;
}


long ctrl_cmd_read_host_mem_page(struct file *file, unsigned int cmd, unsigned long data)
{
    struct sx_dev                *dev;
    int                           err = 0;
    struct ku_host_mem_read_page *host_mem_pg_p;
    struct sx_priv               *priv;
    void                         *fw_page = NULL;
    uint32_t                      pg_index = 0;

    host_mem_pg_p = (struct ku_host_mem_read_page*)kmalloc(sizeof(struct ku_host_mem_read_page), GFP_KERNEL);
    if (host_mem_pg_p == NULL) {
        sxd_log_err(SXD_DEV_NONE, "Failed to allocate %ld bytes.\n", PAGE_SIZE);
        err = -ENOMEM;
        goto out;
    }

    err = copy_from_user(host_mem_pg_p, (void*)data, sizeof(*host_mem_pg_p));
    if (err) {
        goto out_and_free;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        err = -ENODEV;
        goto out_and_free;
    }

    if (host_mem_pg_p->page_index > MLX_TRACE_MAX_PAGE_NUM) {
        sxd_log_err(dev, "Failed to get next mlx trace page from host memory,"
                    " The page index [%d] is out of range (%d).\n", host_mem_pg_p->page_index, MLX_TRACE_MAX_PAGE_NUM);
        err = -EINVAL;
        goto out_and_free;
    }

    /* The page index in the allocated pages = MLX_TRACE page offset in the allocated pages + MLX_TRACE page offset*/
    pg_index = MLX_TRACE_PAGE_OFFSET_IN_HOST_MEM_ALLOCATED_MAP_FA_PAGES + host_mem_pg_p->page_index;

    priv = sx_priv(dev);

    fw_page = sx_map_fa_item_get_page(priv->fw.map_fa_items[SXD_MAP_MEMORY_TYPE_FW_LOAD_E], pg_index);
    memcpy(host_mem_pg_p->pg_data_arr, fw_page, PAGE_SIZE);

    err = copy_to_user((void*)data, host_mem_pg_p, sizeof(*host_mem_pg_p));
    if (err) {
        goto out_and_free;
    }
out_and_free:
    kfree(host_mem_pg_p);

out:
    return err;
}

long ctrl_cmd_set_mmap_alloc_scheme(struct file *filp, unsigned int cmd, unsigned long data)
{
    int                      err;
    ku_mmap_mem_alloc_data_t mem_alloc_ctl;
    struct sx_rsc           *rsc = filp->private_data;

    err = copy_from_user(&mem_alloc_ctl, (void*)data, sizeof(mem_alloc_ctl));

    if (err) {
        goto out;
    }
    rsc->mmap_alloc_scheme = mem_alloc_ctl.alloc_scheme;

out:
    return err;
}
