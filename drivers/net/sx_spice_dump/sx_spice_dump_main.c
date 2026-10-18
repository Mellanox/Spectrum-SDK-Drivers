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

#include <linux/version.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/vmalloc.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <net/netlink.h>
#include <linux/mlx_sx/kernel_user.h>
#include <linux/mlx_sx/skb_hook.h>
#include <linux/mlx_sx/driver.h>
#include <linux/mlx_sx/ku_emad.h>
#include <linux/mlx_sx/auto_registers/cmd_auto.h>
#include <linux/mlx_sx/auto_registers/reg.h>
#include <linux/mlx_sx/cmd.h>
#include "sx_spice_tree_auto.h"
#include "sx_spice_wrapper.h"
#include "sx_spice_access_reg_auto.h"
#include "sx_spice_access_reg.h"
#include "sx_spice_bulk_cntr.h"
#include "sx_spice_asic_ports.h"
#include "sx_spice_uphy_reg_auto.h"

MODULE_AUTHOR("Lior Kaplan");
MODULE_DESCRIPTION("Spice-Dump driver");
MODULE_LICENSE("Dual BSD/GPL");

/************************************************
 *  Local variables
 ***********************************************/

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0))
static struct kmem_cache *__kbuffer_cache;
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0)) */

int loading_mode = SPICE_LOADING_MODE_DEFAULT;
module_param_named(loading_mode, loading_mode, int, 0444);
MODULE_PARM_DESC(loading_mode, "spice loading mode");

/************************************************
 *  Type definitions
 ***********************************************/

/************************************************
 * Functions                                    *
 ***********************************************/

int __init sx_spice_dump_init(void);
void sx_spice_dump_exit(void);

static int __sx_spice_tree_create(dev_private_data_t *dev_priv);
static int __sx_spice_tree_app_directories_create(void);
static int __sx_spice_sensor_count_get(dev_private_data_t *priv_data);
static int __sx_spice_pll_group_num_get(dev_private_data_t *priv_data);
static int __sx_spice_module_count_get(dev_private_data_t *priv_data);
static int __sx_spice_bulk_cntr_init(dev_private_data_t *dev_data);
static int __sx_spice_bulk_cntr_deinit(dev_private_data_t *dev_data);
static void __sx_spice_access_reg_lists_free(void);
static void __sx_spice_access_reg_lists_dev_data_free(dev_private_data_t *dev_data);
static void __sx_spice_port_map_changed_handle(dev_private_data_t *dev_data, uint16_t local_port, bool is_valid);

void sx_spice_add_stats(dev_private_data_t *dev_priv, struct sx_emad_out_params *out_params)
{
    static const u32              histogram_ranges[] = { 10, 20, 30, 40, 50, 100, 200, UINT_MAX };
    struct sx_spice_latency_entry tmp;
    bool                          need_to_sort = false;
    int                           i, j;

    BUILD_BUG_ON(sizeof(histogram_ranges) / sizeof(histogram_ranges[0]) !=
                 sizeof(dev_priv->stats.latency_histogram) / sizeof(dev_priv->stats.latency_histogram[0]));

    /* add to histogram */
    for (i = 0; i < sizeof(histogram_ranges) / sizeof(histogram_ranges[0]); i++) {
        if (out_params->fw_latency <= histogram_ranges[i]) {
            dev_priv->stats.latency_histogram[i]++;
            break;
        }
    }

    /* add to max latency chart */
    if (dev_priv->stats.num_of_max_latency_entries < MAX_LATENCY_SIZE) {
        dev_priv->stats.max_latency[dev_priv->stats.num_of_max_latency_entries].reg_id = out_params->reg_id;
        dev_priv->stats.max_latency[dev_priv->stats.num_of_max_latency_entries].latency = out_params->fw_latency;
        dev_priv->stats.num_of_max_latency_entries++;
        need_to_sort = true;
    } else if (out_params->fw_latency > dev_priv->stats.max_latency[MAX_LATENCY_SIZE - 1].latency) {
        dev_priv->stats.max_latency[MAX_LATENCY_SIZE - 1].reg_id = out_params->reg_id;
        dev_priv->stats.max_latency[MAX_LATENCY_SIZE - 1].latency = out_params->fw_latency;
        need_to_sort = true;
    }

    if (need_to_sort) {
        for (i = 0; i < dev_priv->stats.num_of_max_latency_entries; i++) {
            for (j = i + 1; j < dev_priv->stats.num_of_max_latency_entries; j++) {
                if (dev_priv->stats.max_latency[i].latency < dev_priv->stats.max_latency[j].latency) {
                    tmp = dev_priv->stats.max_latency[i];
                    dev_priv->stats.max_latency[i] = dev_priv->stats.max_latency[j];
                    dev_priv->stats.max_latency[j] = tmp;
                }
            }
        }
    }

    /* update last entry */
    dev_priv->stats.recent.reg_id = out_params->reg_id;
    dev_priv->stats.recent.latency = out_params->fw_latency;

    dev_priv->stats.num_of_reg_calls++;
    if (out_params->fw_status != 0) {
        dev_priv->stats.num_of_reg_calls_errors++;
    }
    dev_priv->stats.total_fw_latency += out_params->fw_latency;
}

void sx_spice_kbuffer_free(dev_private_data_t *dev_data)
{
    if (dev_data->kbuffer) {
        sx_spice_buffer_free(dev_data->kbuffer);
        dev_data->kbuffer = NULL;
        up(&dev_data->kbuffer_sem);
    }
}

static int __spice_stats_dump(struct seq_file *m, void *v, void *context)
{
    dev_private_data_t *dev_data = (dev_private_data_t*)context;
    u32                 aver_latency = 0;
    int                 i;

    seq_printf(m, "SPICE stats (device #%u)\n", dev_data->dev->device_id);
    seq_printf(m, "\n");

    seq_printf(m, "Latency histogram (usecs)\n");
    seq_printf(m, "\t<= 10:   %llu\n", dev_data->stats.latency_histogram[0]);
    seq_printf(m, "\t11-20:   %llu\n", dev_data->stats.latency_histogram[1]);
    seq_printf(m, "\t21-30:   %llu\n", dev_data->stats.latency_histogram[2]);
    seq_printf(m, "\t31-40:   %llu\n", dev_data->stats.latency_histogram[3]);
    seq_printf(m, "\t41-50:   %llu\n", dev_data->stats.latency_histogram[4]);
    seq_printf(m, "\t51-100:  %llu\n", dev_data->stats.latency_histogram[5]);
    seq_printf(m, "\t101-200: %llu\n", dev_data->stats.latency_histogram[6]);
    seq_printf(m, "\t> 200:   %llu\n", dev_data->stats.latency_histogram[7]);
    seq_printf(m, "\n");

    seq_printf(m, "Recent entry stats\n");
    if (dev_data->stats.recent.reg_id != 0) {
        seq_printf(m, "\treg_id: 0x%04x    latency: %u\n",
                   dev_data->stats.recent.reg_id,
                   dev_data->stats.recent.latency);
    }
    seq_printf(m, "\n");

    seq_printf(m, "Max latency (usecs)\n");
    for (i = 0; i < dev_data->stats.num_of_max_latency_entries; i++) {
        seq_printf(m, "\treg_id: 0x%04x    latency: %u\n",
                   dev_data->stats.max_latency[i].reg_id,
                   dev_data->stats.max_latency[i].latency);
    }
    seq_printf(m, "\n");

    if (dev_data->stats.num_of_reg_calls > 0) {
        aver_latency = dev_data->stats.total_fw_latency / dev_data->stats.num_of_reg_calls;
    }

    seq_printf(m, "Total register calls:    %llu\n", dev_data->stats.num_of_reg_calls);
    seq_printf(m, "Failed register calls:   %llu\n", dev_data->stats.num_of_reg_calls_errors);
    seq_printf(m, "Average latency (usecs): %u\n", aver_latency);
    seq_printf(m, "\n");

    return 0;
}

static void * sx_spice_register(struct sx_dev *dev, struct sx_flow_info *fi)
{
    int                 err = 0;
    dev_private_data_t *priv_data;

    priv_data = vmalloc(sizeof(*priv_data));
    if (!priv_data) {
        return NULL;
    }
    memset(priv_data, 0, sizeof(dev_private_data_t));

    mutex_init(&priv_data->reg_access.reg_mutex);

    sema_init(&priv_data->reg_access.reg_sem, 1);

    sema_init(&priv_data->kbuffer_sem, 1);

    INIT_LIST_HEAD(&priv_data->spice_bulk_cntr_dev_files_list);
    INIT_LIST_HEAD(&priv_data->port_data_list);

    priv_data->dev = dev;

    priv_data->is_available = sx_core_is_profile_set(dev);

    err = sx_spice_system_type_get(priv_data);
    if (err) {
        sxd_log_err(priv_data->dev, "SPICE: Failed to get system type\n");
        vfree(priv_data);
        return NULL;
    }

    err = __sx_spice_tree_create(priv_data);
    if (err) {
        if (err == -EFAULT) {
            sxd_log_err(priv_data->dev,
                        "SPICE: SPICE isn't supported with dev_id: %d \n",
                        priv_data->hw_device_id);
            vfree(priv_data);
            return NULL;
        }
        sxd_log_err(priv_data->dev, "SPICE: Failed to create tree\n");
    }

    if (priv_data->eth_mode) {
        err = __sx_spice_bulk_cntr_init(priv_data);
        if (err) {
            sxd_log_err(priv_data->dev, "SPICE: Failed to init spice_bulk_cntr\n");
        }
    }

    if ((priv_data->hw_device_id == SXD_MGIR_HW_DEV_ID_QUANTUM2) ||
        (priv_data->hw_device_id == SXD_MGIR_HW_DEV_ID_SPECTRUM4) ||
        (priv_data->hw_device_id == SXD_MGIR_HW_DEV_ID_SPECTRUM5) ||
        (priv_data->hw_device_id == SXD_MGIR_HW_DEV_ID_SPECTRUM6) ||
        (priv_data->hw_device_id == SXD_MGIR_HW_DEV_ID_SPECTRUM7)) {
        err = sx_spice_uphy_reg_init();
        if (err) {
            sxd_log_err(priv_data->dev, "SPICE: Failed to initialize uPHY reg module\n");
        }
    }

    if (loading_mode != SPICE_LOADING_MODE_PERF_CNTR_ONLY) {
        sx_dbg_dump_read_handler_register("spice_stats", __spice_stats_dump, NULL, dev, priv_data);
    }

    return priv_data;
}

static void sx_spice_unregister(struct sx_dev *dev, void *private_data, struct sx_flow_info *fi)
{
    dev_private_data_t *priv_data = (dev_private_data_t*)private_data;

    if (priv_data->is_available) {
        if (down_timeout(&priv_data->reg_access.reg_sem, msecs_to_jiffies(5000)) == -ETIME) {
            sxd_log_err(dev, "SPICE: FW is busy, cannot proceed with spice deinit process");
        }
    }

    if (loading_mode != SPICE_LOADING_MODE_PERF_CNTR_ONLY) {
        sx_dbg_dump_read_handler_unregister("spice_stats", dev);
    }

    if (priv_data->eth_mode) {
        /*deinit bulk_cntr registered files for device*/
        __sx_spice_bulk_cntr_deinit(priv_data);

        /* Unlink debugfs before dropping file_params refs so new opens fail; last close frees via kref. */
        debugfs_remove_recursive(priv_data->asic_counters_root_inode_p);

        /* Drop driver-held ref on each counter file (kfree when last fd is closed). */
        sx_spice_access_reg_bulk_counters_list_free(priv_data);

        sx_spice_asic_ports_data_list_free(priv_data);
    }

    debugfs_remove_recursive(priv_data->ber_root_inode_p);

    __sx_spice_access_reg_lists_dev_data_free(priv_data);

    vfree(priv_data);
}

/* Update one cached PMLP validity bit (no EMAD, so safe to run regardless of is_available). */
static void __sx_spice_port_map_changed_handle(dev_private_data_t *dev_data, uint16_t local_port, bool is_valid)
{
    if ((local_port == 0) || (local_port > dev_data->max_local_port)) {
        sxd_log_err(dev_data->dev, "SPICE: Got port map change notification for out of range local_port %u\n",
                    local_port);
        return;
    }

    sx_spice_local_port_validity_set(dev_data, local_port, is_valid);
}

static int sx_spice_event(struct sx_dev       *dev,
                          void                *resources,
                          enum sx_dev_event    event,
                          union sx_event_data *event_data)
{
    dev_private_data_t *dev_data = (dev_private_data_t*)resources;
    int                 err = 0;

    switch (event) {
    case SX_DEV_EVENT_ISSU_START:
    case SX_DEV_EVENT_PRE_RESET:
        sxd_log_notice(dev, "sx_spice_event: Got ISSU/PRE_RESET start event. "
                       "dev = %p, dev_id = %u",
                       dev, dev->device_id);

        if (down_timeout(&dev_data->reg_access.reg_sem, msecs_to_jiffies(5000)) == -ETIME) {
            err = -ETIMEDOUT;
            goto out;
        }

        if (dev_data->eth_mode) {
            /*update issu indication for bulk_cntr*/
            spice_bulk_cntr_is_issu = true;
            /*unregister bulk counter*/
            err = __sx_spice_bulk_cntr_deinit(dev_data);
            if (err) {
                sxd_log_err(dev, "SPICE: Failed to unregister spice_bulk_cntr "
                            "for device_id: %d on ISSU/PRE_RESET start event\n", dev->device_id);
                goto out;
            }
        }
        dev_data->is_available = false;
        break;

    case SX_DEV_EVENT_PORT_MAP_SET:
        /* Dispatched on real PMLP SET/DELETE (see sx_core_ioctl_port_module_map_set()). Known
         * gap: loopback-port disable doesn't unmap PMLP, so it won't dispatch this event. */
        if (!event_data) {
            sxd_log_err(dev, "sx_spice_event: Got PORT_MAP_SET event with no event_data\n");
            break;
        }
        __sx_spice_port_map_changed_handle(dev_data,
                                           event_data->port_map_changed.local_port,
                                           event_data->port_map_changed.is_valid);
        break;

    case SX_DEV_EVENT_POST_SET_PROFILE:
        sxd_log_notice(dev, "sx_spice_event: Got POST RESET event. "
                       "dev = %p, dev_id = %u",
                       dev, dev->device_id);
        up(&dev_data->reg_access.reg_sem);

        if (dev_data->eth_mode) {
            /*register bulk counter*/
            err = __sx_spice_bulk_cntr_init(dev_data);
            if (err) {
                sxd_log_err(dev, "SPICE: Failed to register spice_bulk_cntr"
                            "for device_id: %d on POST_RESET start event\n", dev->device_id);
                goto out;
            }
            spice_bulk_cntr_is_issu = false;
        }
        dev_data->is_available = true;
        break;

    default:
        break;
    }

out:
    return err;
}

static struct sx_interface sx_spice_interface = {
    .add = sx_spice_register,
    .remove = sx_spice_unregister,
    .event = sx_spice_event,
    .name = "sx_spice_dump"
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0))
static int cache_create(void)
{
    __kbuffer_cache = kmem_cache_create_usercopy("spice_kbuffer",
                                                 KERNEL_BUFFER_SIZE,
                                                 0,
                                                 0,
                                                 0,
                                                 KERNEL_BUFFER_SIZE,
                                                 NULL);
    if (!__kbuffer_cache) {
        sxd_log_err(SXD_DEV_NONE, "failed to allocate memory cache for spice kernel buffer\n");
        return -ENOMEM;
    }

    return 0;
}

static void cache_destroy(void)
{
    if (__kbuffer_cache) {
        kmem_cache_destroy(__kbuffer_cache);
    }
}
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0)) */

char * sx_spice_buffer_alloc(gfp_t gfp)
{
    char *kbuffer;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0))
    kbuffer = (char*)kmem_cache_alloc(__kbuffer_cache, gfp);
#else
    kbuffer = (char*)kmalloc(KERNEL_BUFFER_SIZE, gfp);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0)) */

    return kbuffer;
}

void sx_spice_buffer_free(void *kbuffer)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0))
    kmem_cache_free(__kbuffer_cache, kbuffer);
#else
    kfree(kbuffer);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0)) */
}

ssize_t sx_spice_read_buffer_prepare(struct file *filp, dev_private_data_t *dev_data, const char *reg_name)
{
    if (down_interruptible(&dev_data->kbuffer_sem)) {
        sxd_log_info(dev_data->dev, "received signal while trying to read from a file and waiting for semaphore \n");
        return -ERESTARTSYS;
    }

    filp->private_data = (void*)0xdeadbeaf; /* release callback frees the buffer and releases the semaphore */
    dev_data->print_data_size = 0;
    dev_data->kbuffer = sx_spice_buffer_alloc(GFP_KERNEL);
    if (!dev_data->kbuffer) {
        printk(KERN_ERR "Failed to allocate kernel buffer for %s SPICE file generation\n", reg_name);
        /* Undo the sentinel and release the semaphore, else it leaks (sx_spice_kbuffer_free() only
         * ups it when kbuffer is non-NULL). */
        filp->private_data = NULL;
        up(&dev_data->kbuffer_sem);
        return -ENOMEM;
    }

    memset(dev_data->kbuffer, 0, sizeof(char) * KERNEL_BUFFER_SIZE);
    return 1;
}

/* "N/A" response using a small on-stack buffer, so callers can skip taking the shared kbuffer/sem. */
ssize_t sx_spice_read_na_stack(char *buf, size_t lbuf, loff_t *ppos)
{
    char    kbuffer[8];
    ssize_t print_data_size;

    if (*ppos != 0) {
        return 0;
    }

    print_data_size = snprintf(kbuffer, sizeof(kbuffer), "N/A\n");
    return simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);
}

ssize_t sx_spice_read_kbuffer(char *buf, size_t lbuf, loff_t *ppos, dev_private_data_t *dev_data)
{
    size_t data_left_to_read;

    if (*ppos > dev_data->print_data_size) {
        return 0;
    }

    data_left_to_read = (size_t)(dev_data->print_data_size - *ppos);
    return simple_read_from_buffer(buf,
                                   data_left_to_read < lbuf ? data_left_to_read : lbuf,
                                   ppos,
                                   dev_data->kbuffer,
                                   dev_data->print_data_size + 1);
}

static int __sx_spice_tree_app_directories_create(void)
{
    int err = 0;

    if (loading_mode != SPICE_LOADING_MODE_PERF_CNTR_ONLY) {
        err = sx_spice_tree_ber_app_dir_create();
        if (err) {
            sxd_log_err(SXD_DEV_NONE, "SPICE: Failed to create root directory for BER application\n");
            goto out;
        }
    }

    err = sx_spice_tree_asic_counters_app_dir_create();
    if (err) {
        sxd_log_err(SXD_DEV_NONE, "SPICE: Failed to create root directory for asic_counters application\n");
        goto out;
    }
out:
    return err;
}

int __init sx_spice_dump_init(void)
{
    int err = 0;

    /* Validate loading_mode parameter - clamp to valid range */
    if ((loading_mode < SPICE_LOADING_MODE_DEFAULT) || (loading_mode > SPICE_LOADING_MODE_PERF_CNTR_ONLY)) {
        sxd_log_warning(SXD_DEV_NONE,
                        "SPICE: Invalid loading_mode %d, using default mode %d\n",
                        loading_mode, SPICE_LOADING_MODE_DEFAULT);
        loading_mode = SPICE_LOADING_MODE_DEFAULT;
    }

    err = sx_spice_tree_init();
    if (err) {
        sxd_log_err(SXD_DEV_NONE, "SPICE: Failed to initialize spice tree module\n");
        goto out;
    }
    err = __sx_spice_tree_app_directories_create();
    if (err) {
        sxd_log_err(SXD_DEV_NONE, "SPICE: Failed to initialize spice tree module\n");
        goto out;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0))
    err = cache_create();
    if (err) {
        sxd_log_err(SXD_DEV_NONE, "Failed to create cache for SPICE\n");
        goto cache_create_failed;
    }
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0)) */

    err = sx_register_interface(&sx_spice_interface);
    if (err) {
        sxd_log_err(SXD_DEV_NONE, "Failed to register SPICE kernel module\n");
        goto sx_register_failed;
    }

    return err;

sx_register_failed:
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0))
    cache_destroy();

cache_create_failed:
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0)) */
    sx_spice_tree_deinit();

out:
    return err;
}

static int __sx_spice_bulk_cntr_init(dev_private_data_t *dev_data)
{
    int                            err = 0;
    spice_bulk_cntr_file_params_t *iter;

    /* Init each created file (list is always initialized; empty when no files created). */
    list_for_each_entry(iter, &dev_data->spice_bulk_cntr_dev_files_list, list_file_params) {
        mutex_lock(&iter->file_db.tr_file_op_mutex);
        err = sx_spice_bulk_cntr_file_init(iter->dev, iter->key_type, &iter->file_db);
        if (err) {
            sxd_log_err(iter->dev, "%s: Failed to initialize spice bulk counters type: %d on device: %d\n",
                        __func__, iter->key_type, iter->dev->device_id);
            mutex_unlock(&iter->file_db.tr_file_op_mutex);
            goto out;
        }
        mutex_unlock(&iter->file_db.tr_file_op_mutex);

#ifdef SPICE_BULK_CNTR_LOG_EN
        sxd_log_info(iter->dev,
                     "%s: successfully initialized spice bulk counter type:%d of device_id: %d\n",
                     __func__,
                     iter->key_type,
                     iter->dev->device_id);
#endif
    }

    /*register to mocs_done trap*/
    err = sx_spice_bulk_cntr_listener_set(CTRL_CMD_ADD_SYND,
                                          dev_data->dev,
                                          SXD_TRAP_ID_MOCS_DONE,
                                          (void *)&dev_data->spice_bulk_cntr_dev_files_list);
    if (err) {
        sxd_log_err(dev_data->dev, "%s: failed to set trap_id SXD_TRAP_ID_MOCS_DONE", __func__);
        goto out;
    }

out:
    return err;
}

static int __sx_spice_bulk_cntr_deinit(dev_private_data_t *dev_data)
{
    int                            err = 0;
    spice_bulk_cntr_file_params_t *iter;

    /* Deinit each created file (list is always initialized; empty when no files created). */
    list_for_each_entry(iter, &dev_data->spice_bulk_cntr_dev_files_list, list_file_params) {
        mutex_lock(&iter->file_db.tr_file_op_mutex);
        err = sx_spice_bulk_cntr_file_deinit(iter->dev, iter->key_type, &iter->file_db);
        if (err) {
            sxd_log_err(iter->dev, "%s: Failed to deinit spice bulk counters type: %d on device: %d\n",
                        __func__, iter->key_type, iter->dev->device_id);
            mutex_unlock(&iter->file_db.tr_file_op_mutex);
            goto out;
        }
        mutex_unlock(&iter->file_db.tr_file_op_mutex);

#ifdef SPICE_BULK_CNTR_LOG_EN
        sxd_log_info(iter->dev, "%s: successfully deinit spice bulk counter type:%d of device_id: %d\n",
                     __func__, iter->key_type, iter->dev->device_id);
#endif
    }
    /*unregister to mocs_done trap once tr files are done*/
    err = sx_spice_bulk_cntr_listener_set(CTRL_CMD_REMOVE_SYND,
                                          dev_data->dev,
                                          SXD_TRAP_ID_MOCS_DONE,
                                          (void *)&dev_data->spice_bulk_cntr_dev_files_list);
    if (err) {
        sxd_log_err(dev_data->dev, "%s: failed to remove trap_id SXD_TRAP_ID_MOCS_DONE",
                    __func__);
        goto out;
    }

out:
    return err;
}

static void __sx_spice_access_reg_lists_free(void)
{
    sx_spice_access_reg_pddr_list_free();

    sx_spice_access_reg_ppcnt_list_free();

    sx_spice_access_reg_spzr_list_free();

    sx_spice_access_reg_pmdr_list_free();

    sx_spice_access_reg_ptys_list_free();

    sx_spice_access_reg_pmaos_list_free();

    sx_spice_access_reg_pmlp_list_free();

    sx_spice_access_reg_pemi_list_free();

    sx_spice_access_reg_mtcap_list_free();

    sx_spice_access_reg_mvcap_list_free();

    sx_spice_access_reg_msgi_list_free();

    sx_spice_access_reg_mgir_list_free();

    sx_spice_access_reg_mdir_list_free();

    sx_spice_access_reg_sltp_list_free();

    sx_spice_access_reg_sllm_list_free();

    sx_spice_access_reg_spad_list_free();

    sx_spice_access_reg_slrip_list_free();

    sx_spice_access_reg_slrg_list_free();

    sx_spice_access_reg_pphcr_list_free();

    sx_spice_access_reg_mtmp_list_free();

    sx_spice_access_reg_mvcr_list_free();

    sx_spice_access_reg_slrp_list_free();

    sx_spice_access_reg_peucg_list_free();

    sx_spice_access_reg_ppll_list_free();
}

static void __sx_spice_access_reg_lists_dev_data_free(dev_private_data_t *dev_data)
{
    sx_spice_access_reg_pddr_list_dev_data_free(dev_data);

    sx_spice_access_reg_ppcnt_list_dev_data_free(dev_data);

    sx_spice_access_reg_spzr_list_dev_data_free(dev_data);

    sx_spice_access_reg_pmdr_list_dev_data_free(dev_data);

    sx_spice_access_reg_ptys_list_dev_data_free(dev_data);

    sx_spice_access_reg_pmaos_list_dev_data_free(dev_data);

    sx_spice_access_reg_pmlp_list_dev_data_free(dev_data);

    sx_spice_access_reg_pemi_list_dev_data_free(dev_data);

    sx_spice_access_reg_mtcap_list_dev_data_free(dev_data);

    sx_spice_access_reg_mvcap_list_dev_data_free(dev_data);

    sx_spice_access_reg_msgi_list_dev_data_free(dev_data);

    sx_spice_access_reg_mgir_list_dev_data_free(dev_data);

    sx_spice_access_reg_mdir_list_dev_data_free(dev_data);

    sx_spice_access_reg_sltp_list_dev_data_free(dev_data);

    sx_spice_access_reg_sllm_list_dev_data_free(dev_data);

    sx_spice_access_reg_spad_list_dev_data_free(dev_data);

    sx_spice_access_reg_slrip_list_dev_data_free(dev_data);

    sx_spice_access_reg_slrg_list_dev_data_free(dev_data);

    sx_spice_access_reg_pphcr_list_dev_data_free(dev_data);

    sx_spice_access_reg_mtmp_list_dev_data_free(dev_data);

    sx_spice_access_reg_mvcr_list_dev_data_free(dev_data);

    sx_spice_access_reg_slrp_list_dev_data_free(dev_data);

    sx_spice_access_reg_peucg_list_dev_data_free(dev_data);

    sx_spice_access_reg_ppll_list_dev_data_free(dev_data);
}

void sx_spice_dump_exit(void)
{
    sxd_log_info(SXD_DEV_NONE, "finish spice-dump\n");

    sx_unregister_interface(&sx_spice_interface);

    sx_spice_tree_deinit();

    sx_spice_uphy_reg_deinit();

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0))
    cache_destroy();
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0)) */

    __sx_spice_access_reg_lists_free();
}

module_init(sx_spice_dump_init);
module_exit(sx_spice_dump_exit);


int sx_spice_system_type_get(dev_private_data_t *priv_data)
{
    int                       err = 0;
    struct ku_access_mgir_reg reg_data;

    memset(&reg_data, 0, sizeof(struct ku_access_mgir_reg));

    sx_cmd_set_op_tlv(&reg_data.op_tlv, MLXSW_MGIR_ID, 1);
    reg_data.dev_id = priv_data->dev->device_id;
    err = sx_ACCESS_REG_MGIR(priv_data->dev, &reg_data);
    if (err) {
        sxd_log_err(priv_data->dev, "SPICE: Failed to get system info from MGIR register\n");
        goto out;
    }

    priv_data->hw_device_id = reg_data.mgir_reg.hw_info.device_id;
    priv_data->cpo_indication = reg_data.mgir_reg.hw_info.cpo_indication;

out:
    return err;
}

static int __sx_spice_sensor_count_get(dev_private_data_t *priv_data)
{
    int                        err = 0;
    struct ku_access_mtcap_reg reg_data;

    memset(&reg_data, 0, sizeof(struct ku_access_mtcap_reg));
    sx_cmd_set_op_tlv(&reg_data.op_tlv, MLXSW_MTCAP_ID, SXD_EMAD_METHOD_QUERY_E);
    reg_data.dev_id = priv_data->dev->device_id;

    err = sx_ACCESS_REG_MTCAP(priv_data->dev, &reg_data);
    if (err) {
        sxd_log_err(priv_data->dev,
                    "SPICE: Failed to get system info from MLXSW_MTCAP_ID register\n");
        goto out;
    }
    priv_data->max_sensor_index = reg_data.mtcap_reg.sensor_count;

out:
    return err;
}

static int __sx_spice_pll_group_num_get(dev_private_data_t *priv_data)
{
    int                       err = 0;
    struct ku_access_ppll_reg reg_data;

    memset(&reg_data, 0, sizeof(reg_data));
    sx_cmd_set_op_tlv(&reg_data.op_tlv, MLXSW_PPLL_ID, 1);
    reg_data.dev_id = priv_data->dev->device_id;
    err = sx_ACCESS_REG_PPLL(priv_data->dev, &reg_data);
    if (err) {
        sxd_log_err(priv_data->dev,
                    "SPICE: Failed to get num of pll groups from PPLL register\n");
        goto out;
    }

    priv_data->max_pll_group = reg_data.ppll_reg.num_pll_groups;

out:
    return err;
}

static int __sx_spice_module_count_get(dev_private_data_t *priv_data)
{
    int                        err = 0;
    struct ku_access_mgpir_reg reg_data;
    u8                         fw_status = 0;

    memset(&reg_data, 0, sizeof(reg_data));

    reg_data.dev_id = priv_data->dev->device_id;
    sx_cmd_set_op_tlv(&reg_data.op_tlv, MLXSW_MGPIR_ID, SXD_EMAD_METHOD_QUERY_E);
    reg_data.mgpir_reg.hw_info.slot_index = 0; /*slot is set 0 by default */

    err = sx_ACCESS_REG_MGPIR(priv_data->dev, &reg_data);
    fw_status = sxd_emad_operation_tlv_status_get(&reg_data.op_tlv);

    if (err || fw_status) {
        sxd_log_err(priv_data->dev,
                    "SPICE: Fails to access register MGPIR, err: %d, status: %d.\n",
                    err,
                    fw_status);
        err = -EFAULT;
        goto out;
    }
    priv_data->max_module_index = reg_data.mgpir_reg.hw_info.num_of_modules - 1; /* local modules per ASIC */

out:
    return err;
}

static int __sx_spice_tree_create(dev_private_data_t *dev_priv)
{
    int                  err = 0;
    struct ku_query_rsrc query_rsrc;

    memset(&query_rsrc, 0, sizeof(struct ku_query_rsrc));

    query_rsrc.rsrc_id = KU_RES_ID_CAP_MAX_SWITCH_PORTS;

    if (!dev_priv) {
        sxd_log_err(SXD_DEV_NONE, "SPICE: in __sx_spice_tree_create dev is NULL\n");
        goto out;
    }

    err = sx_QUERY_RSRC(dev_priv->dev, &query_rsrc);
    if (err) {
        sxd_log_err(dev_priv->dev, "SPICE: Failed to retrieve system number of ports\n");
        goto out;
    }

    if (query_rsrc.rsrc_val > MAX_PHYPORT_NUM) {
        /* valid_local_ports_bitmap is sized for MAX_PHYPORT_NUM+1 bits; a larger max_local_port
         * would make set_bit/clear_bit/test_bit write past it. */
        sxd_log_err(dev_priv->dev,
                    "SPICE: reported max switch ports %llu exceeds MAX_PHYPORT_NUM %u\n",
                    query_rsrc.rsrc_val, MAX_PHYPORT_NUM);
        err = -EINVAL;
        goto out;
    }

    dev_priv->max_local_port = query_rsrc.rsrc_val;

    err = sx_spice_local_port_validity_bitmap_init(dev_priv);
    if (err) {
        sxd_log_err(dev_priv->dev, "SPICE: Failed to initialize local port validity bitmap\n");
        goto out;
    }

    err = __sx_spice_sensor_count_get(dev_priv);
    if (err) {
        sxd_log_err(dev_priv->dev, "SPICE: Failed to get ambient sensor count\n");
        goto out;
    }

    err = __sx_spice_pll_group_num_get(dev_priv);
    if (err) {
        sxd_log_err(dev_priv->dev, "SPICE: Failed to get pll group count\n");
        goto out;
    }

    err = __sx_spice_module_count_get(dev_priv);
    if (err) {
        sxd_log_err(dev_priv->dev, "SPICE: Failed to get ASIC module count\n");
        goto out;
    }

    switch (dev_priv->hw_device_id) {
    case SXD_MGIR_HW_DEV_ID_QUANTUM:
    case SXD_MGIR_HW_DEV_ID_QUANTUM2:
        err = sx_spice_tree_ber_ib_create(dev_priv);
        if (err) {
            sxd_log_err(dev_priv->dev, "SPICE: Failed to create IB ber tree\n");
            goto out;
        }
        break;

    case SXD_MGIR_HW_DEV_ID_SPECTRUM2:
    case SXD_MGIR_HW_DEV_ID_SPECTRUM3:
    case SXD_MGIR_HW_DEV_ID_SPECTRUM4:
    case SXD_MGIR_HW_DEV_ID_SPECTRUM5:
    case SXD_MGIR_HW_DEV_ID_SPECTRUM6:
    case SXD_MGIR_HW_DEV_ID_SPECTRUM7:
        dev_priv->eth_mode = true;

        if (loading_mode != SPICE_LOADING_MODE_PERF_CNTR_ONLY) {
            err = sx_spice_tree_ber_eth_create(dev_priv);
            if (err) {
                sxd_log_err(dev_priv->dev, "SPICE: Failed to create ETH ber tree\n");
                goto out;
            }
        }

        err = sx_spice_tree_asic_counters_eth_create(dev_priv);
        if (err) {
            sxd_log_err(dev_priv->dev, "SPICE: Failed to create ETH asic_counters tree\n");
            goto out;
        }
        break;

    case SXD_MGIR_HW_DEV_ID_SPECTRUM:
        sxd_log_err(dev_priv->dev, "ERROR: SPICE is not supported in Spectrum.\n");
        err = -EFAULT;
        goto out;

    default:
        sxd_log_err(dev_priv->dev,
                    "ERROR: Unresolved device id (%u), SPICE is not supported.\n",
                    dev_priv->hw_device_id);
        return -EFAULT;
    }

out:
    return err;
}
