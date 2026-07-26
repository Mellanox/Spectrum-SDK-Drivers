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

#include <linux/mlx_sx/kernel_user.h>
#include <linux/mlx_sx/cmd.h>

#include "sx.h"
#include "sgmii.h"
#include "alloc.h"
#include "dev_init.h"
#include "ioctl_internal.h"
#include "health_check.h"


long ctrl_cmd_query_fw(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_query_fw query_fw;
    struct sx_dev     *dev;
    int                err;

    err = copy_from_user(&query_fw, (struct ku_query_fw *)data, sizeof(query_fw));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &query_fw.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    err = sx_QUERY_FW(dev, &query_fw);
    if (err) {
        goto out;
    }

    err = copy_to_user((void*)data, &query_fw, sizeof(query_fw));

out:
    return err;
}


long ctrl_cmd_mad_demux(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_mad_demux mad_demux;
    struct sx_dev      *dev;
    int                 err;

    err = copy_from_user(&mad_demux, (struct ku_mad_demux*)data, sizeof(mad_demux));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &mad_demux.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    err = sx_MAD_DEMUX_GET_METHOD(dev, mad_demux.dev_id, mad_demux.enable);
    if (err) {
        sx_err(dev, "Failed to set MAD DEMUX (get method), err: %d\n", err);
        goto out;
    }

    err = sx_MAD_DEMUX_SET_METHOD(dev, mad_demux.dev_id, mad_demux.enable);
    if (err) {
        sx_err(dev, "Failed to set MAD DEMUX (set method), err: %d\n", err);
        goto out;
    }


out:
    return err;
}


long ctrl_cmd_query_rsrc(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_query_rsrc query_rsrc;
    struct sx_dev       *dev;
    int                  err;

    err = copy_from_user(&query_rsrc, (struct ku_query_rsrc *)data, sizeof(query_rsrc));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &query_rsrc.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    err = sx_QUERY_RSRC(dev, &query_rsrc);
    if (err) {
        goto out;
    }

    err = copy_to_user((void*)data, &query_rsrc, sizeof(query_rsrc));

out:
    return err;
}


long ctrl_cmd_query_board_info(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_query_board_info board_info;
    struct sx_dev             *dev;
    int                        err = 0;

    err = copy_from_user(&board_info, (struct ku_query_board_info *)data, sizeof(board_info));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &board_info.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    err = sx_QUERY_BOARDINFO(dev, &board_info);
    if (err) {
        goto out;
    }

    err = copy_to_user((void*)data, &board_info, sizeof(board_info));

out:
    return err;
}

static sx_map_fa_handle_t __map_fa_item_alloc_and_map(struct sx_dev *dev, u16 npages, sxd_map_memory_type_e mem_type)
{
    sx_map_fa_handle_t handle = SX_MAP_FA_INVALID_HANDLE;
    int                err = 0;

    err = sx_map_fa_item_alloc(dev, npages, &handle);
    if (err) {
        sx_err(dev, "Couldn't allocate ICM (pages=%u, mem_type=%d)\n", npages, mem_type);
        goto out;
    }

    err = sx_MAP_FA(dev, mem_type, handle);
    if (err) {
        sx_err(dev, "MAP_FA command failed (pages=%u, mem_type=%d)\n", npages, mem_type);
        sx_map_fa_item_dealloc(handle);
        handle = SX_MAP_FA_INVALID_HANDLE;
        goto out;
    }

out:
    return handle;
}

long ctrl_cmd_memory_to_cpu_map(struct file *file, unsigned int cmd, unsigned long data)
{
    struct sx_dev       *dev = NULL;
    struct sx_priv      *priv = NULL;
    int                  err = 0;
    struct ku_map_memory map_memory;
    sx_map_fa_handle_t  *p_target = NULL;

    err = copy_from_user(&map_memory, (void*)data, sizeof(map_memory));
    if (err) {
        goto out;
    }

    if (sx_cr_mode()) {
        sxd_log_notice(SXD_DEV_NONE, "Memory-to-CPU map is not supported in CR mode, ignoring\n");
        goto out; /* err = 0 */
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &map_memory.dev_id);
    if (!dev) {
        err = -ENODEV;
        goto out;
    }

    priv = sx_priv(dev);

    switch (map_memory.mem_type) {
    case SXD_MAP_MEMORY_TYPE_PORT_PROFILE_MEM_E:
    case SXD_MAP_MEMORY_TYPE_SWITCH_TO_SWITCH_LATENCY_E:
    case SXD_MAP_MEMORY_TYPE_ICMP_OFFLOAD_E:
    case SXD_MAP_MEMORY_TYPE_BFD_TX_OFFLOAD_E:
    case SXD_MAP_MEMORY_TYPE_BFD_RX_OFFLOAD_E:
    case SXD_MAP_MEMORY_TYPE_CABLE_DELAY_MEASUREMENT_E:
        break;

    default:
        sxd_log_err(dev, "Invalid memory type for MAP_FA (mem_type=%d)\n", map_memory.mem_type);
        err = -EINVAL;
        goto out;
    }

    p_target = &priv->fw.map_fa_items[map_memory.mem_type];
    if (*p_target != SX_MAP_FA_INVALID_HANDLE) {
        sxd_log_info(dev,
                     "mem_type=%d has already a valid allocation - deallocating it for a new one.\n",
                     map_memory.mem_type);
        sx_map_fa_item_dealloc(*p_target);
    }

    *p_target = __map_fa_item_alloc_and_map(dev, map_memory.mem_size, map_memory.mem_type);
    if (*p_target == SX_MAP_FA_INVALID_HANDLE) {
        err = -ENOMEM;
        goto out;
    }

out:
    return err;
}

long ctrl_cmd_set_device_profile(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_profile            profile;
    struct ku_swid_config       *swid_config_type_arr = &profile.swid0_config_type;
    struct profile_driver_params addition_params;
    u8                           swid;
    struct sx_dev               *dev;
    struct sx_priv              *priv;
    int                          err;
    sxd_dev_id_t                 dev_id = 0;
    bool                         skip_module_sysfs_tree = false;

    memset(&addition_params, 0, sizeof(addition_params));
    err = copy_from_user(&profile, (void*)data, sizeof(profile));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &profile.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    priv = sx_priv(dev);

    if ((profile.dev_id != DEFAULT_DEVICE_ID) && (profile.do_not_config_profile_to_device == 0)) {
        err = __sx_core_dev_specific_cb_get_reference(dev);
        if (err) {
            sxd_log_err(dev, "__sx_core_dev_specific_cb_get_reference failed.\n");
            goto out;
        }

        __sx_core_dev_specific_cb_release_reference(dev);

        err = sx_SET_PROFILE(dev, &profile, &addition_params);
        if (err) {
            goto out;
        }
    }

    priv->independent_module_params.module_support_type = profile.module_master_fw_default;

    /* Add module sysfs entries when module support type is not standalone and not using SGMII */
    if ((profile.module_master_fw_default != SXD_MODULE_MASTER_MODE_SW_CONTROL_NO_I2C_CONNECTIVITY_E) &&
        !is_sgmii_supported()) {
        if (sx_core_has_pci_multi_dev() && MULTI_ASIC_MODULE_SYSFS_IS_SUPPORT(priv)) {
            err = sx_multi_asic_module_sysfs_get_devid_via_ga(priv->dev_info.dev_info_ro.mgir.hw_info.ga, &dev_id);
            if (err) {
                sxd_log_err(dev, "Failed to get device via ga %d, err: %d.\n",
                            priv->dev_info.dev_info_ro.mgir.hw_info.ga, err);
                goto out;
            }
            /* Multi-asic module sysfs entries are only under asic0 (i.e., dev_id ==1) directory */
            if (dev_id != 1) {
                skip_module_sysfs_tree = true;
            }
        }

/* For Palladium skip sysfs tree creation and module event registration */
#if !defined(PD_BU)
        if (!skip_module_sysfs_tree) {
            err = sx_core_create_default_modules_sysfs_tree(dev,
                                                            priv->kobj[SX_KOBJECT_ASIC],
                                                            &(priv->module_sysfs_arr));
            if (err) {
                sxd_log_err(dev, "Failed to create module sysfs entries.\n");
                goto out;
            }
            sx_core_set_default_modules_sysfs_tree_root(priv->module_sysfs_arr);
        }
        err = sx_core_init_module_eeprom_access_mode(dev);
        if (err) {
            sxd_log_err(dev, "Failed to init module eeprom access mode.\n");
            goto out;
        }
        err = sx_module_sysfs_register_module_event_handler(dev);
        if (err) {
            sxd_log_err(dev, "Failed to register module events handlers\n");
            goto out;
        }

        err = sx_core_dispatch_event(dev, SX_DEV_EVENT_MODULE_SYSFS_CREATE, NULL);
        if (err) {
            sxd_log_err(dev, "Failed to dispatch module sysfs create event\n");
            goto out;
        }
#endif
    }

    /* Add independent module sysfs entries when module support type is independent */
    if (profile.module_master_fw_default == SXD_MODULE_MASTER_MODE_SW_CONTROL_E) {
        err = sx_sysfs_asic_independent_init(dev);
        if (err) {
            goto out;
        }
    }

    /*
     * if chip is PCI attached or using SGMII, set the global profile and
     * create ib devices if needed
     */

    if ((is_sgmii_supported() && !priv->profile.dev_profile_set) || sx_dpt_is_dev_pci_attached(profile.dev_id)) {
        priv->profile.dev_profile = profile;

        /*
         * set events for all IB enabled swids if
         * swid is enabled
         * */
        for (swid = 0; swid < NUMBER_OF_SWIDS; swid++) {
            /* if swid isn't enabled skip it */
            if (!sx_bitmap_test(&sx_priv(dev)->swid_bitmap, swid)) {
                continue;
            }

            /* check the swid is IB in device profile */
            if (swid_config_type_arr->type != KU_SWID_TYPE_INFINIBAND) {
                continue;
            }

            err = sx_send_enable_ib_swid_events(dev, swid);
            if (err) {
                spin_lock(&priv->profile.profile_lock);
                priv->profile.dev_profile_set = false;
                priv->dev_info.dev_info_ro.flags &= ~SX_DEV_INFO_F_DEV_PROFILE_SET;
                spin_unlock(&priv->profile.profile_lock);
                goto out;
            }
        }

        /* Mark profile set */
        spin_lock(&priv->profile.profile_lock);
        priv->profile.dev_profile_set = true;
        priv->dev_info.dev_info_ro.flags |= SX_DEV_INFO_F_DEV_PROFILE_SET;
        spin_unlock(&priv->profile.profile_lock);
    }

    err = sx_core_dispatch_event(dev, SX_DEV_EVENT_POST_SET_PROFILE, NULL);
    if (err) {
        sxd_log_err(dev, "Post set profile event failed, err:%d\n", err);
        goto out;
    }

out:
    return err;
}


long ctrl_cmd_get_device_profile(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_profile profile;
    int               err;
    struct sx_dev    *dev;

    err = copy_from_user(&profile, (void*)data, sizeof(profile));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &profile.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    err = sx_GET_PROFILE(dev, &profile, NULL);
    if (err) {
        goto out;
    }

    err = copy_to_user((void*)data, &profile, sizeof(profile));
    if (err) {
        goto out;
    }

out:
    return err;
}


long ctrl_cmd_set_system_mkey(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_system_m_key system_m_key;
    struct sx_dev         *dev;
    int                    err;

    err = copy_from_user(&system_m_key, (struct ku_system_m_key *)data, sizeof(system_m_key));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &system_m_key.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    err = sx_SET_SYSTEM_M_KEY(dev, &system_m_key);
    if (err) {
        goto out;
    }

    err = copy_to_user((void*)data, &system_m_key, sizeof(system_m_key));

out:
    return err;
}


long ctrl_cmd_get_system_mkey(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_system_m_key system_m_key;
    struct sx_dev         *dev;
    int                    err;

    err = copy_from_user(&system_m_key, (struct ku_system_m_key *)data, sizeof(system_m_key));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &system_m_key.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    err = sx_GET_SYSTEM_M_KEY(dev, &system_m_key);
    if (err) {
        goto out;
    }

    err = copy_to_user((void*)data, &system_m_key, sizeof(system_m_key));

out:
    return err;
}

long ctrl_cmd_send_fatal_failure_detect_info_set(struct file *file, unsigned int cmd, unsigned long data)
{
    ku_dbg_health_check_params_t params;
    struct sx_dev               *dev = NULL;
    int                          err = 0;

    if (sx_cr_mode()) {
        sxd_log_notice(SXD_DEV_NONE, "Health-Check feature is not supported in CR mode, ignoring\n");
        return 0;
    }

    if (is_sgmii_supported()) { /* Chassis managed by OOB network */
        return 0;
    }

    err = copy_from_user(&params, (struct ku_dbg_health_check_params_t *)data, sizeof(params));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &params.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    err = sx_health_check_configure(dev, &params);

out:
    return err;
}

long ctrl_cmd_send_fatal_failure_sdk_terminate_fd_set(struct file *file, unsigned int cmd, unsigned long data)
{
    struct sx_rsc                               *rsc = file->private_data;
    sxd_dev_id_t                                 device_id = 0;
    ku_dbg_health_check_sdk_terminate_fd_param_t params;
    int                                          err = 0;

    memset(&params, 0, sizeof(params));

    err = copy_from_user(&params, (struct ku_dbg_health_check_sdk_terminate_fd_param_t *)data, sizeof(params));
    if (err) {
        goto out;
    }

    if (params.is_fd_created) {
        sx_core_get_dev_and_id_from_fd(file, NULL, &device_id);

        /*Update fd with the health check dev id*/
        sx_core_set_health_dev_id(rsc, device_id);
    } else { /* health check go down gracefully */
        sx_core_set_health_dev_id(rsc, 0);
    }
out:
    return err;
}


#define KTHREAD_NAME_BUF_SIZE 64

long ctrl_cmd_health_event_simulate(struct file *file, unsigned int cmd, unsigned long data)
{
    ku_health_event_simulate_t            ku;
    struct sx_health_check_trigger_params trigger_params;
    struct sx_dev                        *dev = NULL;
    sxd_dev_id_t                          dev_id = 0;
    int                                   err = 0;
    char                                  kthread_name[KTHREAD_NAME_BUF_SIZE];

    memset(&ku, 0, sizeof(ku));
    memset(&trigger_params, 0, sizeof(trigger_params));

    err = copy_from_user(&ku, (ku_health_event_simulate_t *)data, sizeof(ku));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &dev_id);
    if (!dev) {
        return -ENODEV;
    }

    trigger_params.dev_id = (u8)dev_id;
    trigger_params.fatal_error_mode_active = false;

    if (!ku.is_set) {
        if (ku.cause == SXD_HEALTH_CAUSE_CLEAR_ALL_E) {
            trigger_params.op = SX_HEALTH_CHECK_TRIGGER_OP_CANCEL_ALL;
            sxd_log_notice(dev, "Health-Check: Clear all triggers on device %u\n", dev_id);
        } else {
            trigger_params.op = SX_HEALTH_CHECK_TRIGGER_OP_CANCEL_ONE;
            trigger_params.params.cancel_one_params.cause = ku.cause;
        }
        sx_health_check_set_debug_trigger(dev, &trigger_params);
        goto out;
    }

    /* is_set TRUE: map cause to trigger op */
    switch (ku.cause) {
    case SXD_HEALTH_CAUSE_CATAS:
        trigger_params.op = SX_HEALTH_CHECK_TRIGGER_OP_CATAS;
        break;

    case SXD_HEALTH_CAUSE_NO_CMDIFC_COMPLETION:
        trigger_params.op = SX_HEALTH_CHECK_TRIGGER_OP_CMD_IFC;
        trigger_params.params.cmd_ifc_trigger_params.cause = ku.cause;
        break;

    case SXD_HEALTH_CAUSE_RDQ:
        trigger_params.op = SX_HEALTH_CHECK_TRIGGER_OP_RDQ;
        err = sx_health_check_get_first_rdq_dqn(dev, &trigger_params.params.dq_params.dqn);
        if (err) {
            sxd_log_err(dev, "Health-Check: No RDQ in health DB (err=%d)\n", err);
            goto out;
        }
        break;

    case SXD_HEALTH_CAUSE_SDQ:
        trigger_params.op = SX_HEALTH_CHECK_TRIGGER_OP_SDQ;
        err = sx_health_check_get_first_sdq_dqn(dev, &trigger_params.params.dq_params.dqn);
        if (err) {
            sxd_log_err(dev, "Health-Check: No SDQ in health DB (err=%d)\n", err);
            goto out;
        }
        break;

    case SXD_HEALTH_CAUSE_SYSFS:
        trigger_params.op = SX_HEALTH_CHECK_TRIGGER_OP_SYSFS;
        break;

    case SXD_HEALTH_CAUSE_KERNEL_THREADS:
        trigger_params.op = SX_HEALTH_CHECK_TRIGGER_OP_KERNEL_THREADS;
        err = sx_health_check_get_first_kthread_name(dev, kthread_name, sizeof(kthread_name));
        if (err) {
            sxd_log_err(dev, "Health-Check: could not get kthread name from health DB (err=%d)\n", err);
            goto out;
        }
        trigger_params.params.kthread_params.name = (char *)kthread_name;
        break;

    case SXD_HEALTH_CAUSE_FW:
        trigger_params.op = SX_HEALTH_CHECK_TRIGGER_OP_FW_FATAL_EVENT_TEST;
        break;

    case SXD_HEALTH_CAUSE_FW_HETT:
        trigger_params.op = SX_HEALTH_CHECK_TRIGGER_OP_FW_FATAL_EVENT_TEST;
        break;

    case SXD_HEALTH_CAUSE_FW_FATAL_EVENT:
        trigger_params.op = SX_HEALTH_CHECK_TRIGGER_OP_FW_FATAL_EVENT_TEST;
        break;

    case SXD_HEALTH_CAUSE_FW_FATAL_CAUSE:
        trigger_params.op = SX_HEALTH_CHECK_TRIGGER_OP_FW_CAUSE_TEST;
        break;

    case SXD_HEALTH_CAUSE_FW_ASSERT:
        trigger_params.op = SX_HEALTH_CHECK_TRIGGER_OP_FW_ASSERT_TEST;
        break;

    case SXD_HEALTH_CAUSE_FW_TILE_ASSERT:
        trigger_params.op = SX_HEALTH_CHECK_TRIGGER_OP_FW_TILE_ASSERT_TEST;
        break;

    case SXD_HEALTH_CAUSE_FW_TILE_FATAL_CAUSE:
        trigger_params.op = SX_HEALTH_CHECK_TRIGGER_OP_FW_TILE_FATAL_CAUSE_TEST;
        break;

    case SXD_HEALTH_CAUSE_PLL_E:
        trigger_params.op = SX_HEALTH_CHECK_TRIGGER_OP_FW_PLL_FATAL_CAUSE_TEST;
        break;

    default:
        err = -EINVAL;
        goto out;
    }

    sxd_log_notice(dev, "Health-Check: Trigger %d failure on device %u\n", ku.cause, dev_id);
    sx_health_check_set_debug_trigger(dev, &trigger_params);

out:
    return err;
}

long ctrl_cmd_set_sysfs_sniffer_mode(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_sysfs_sniffer_params sysfs_sniffer_params;
    struct sx_dev                 *dev;
    int                            err;

    err = copy_from_user(&sysfs_sniffer_params, (struct ku_sysfs_sniffer_params *)data, sizeof(sysfs_sniffer_params));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    sx_priv(dev)->sniffer_read_enable = false;
    sx_priv(dev)->sniffer_enable = false;

    if (sysfs_sniffer_params.sysfs_sniffer_mode != SXD_SYSFS_SNIFFER_NONE_E) {
        sx_priv(dev)->sniffer_enable = true;
    }
    if (sysfs_sniffer_params.sysfs_sniffer_mode == SXD_SYSFS_SNIFFER_READ_WRITE_E) {
        sx_priv(dev)->sniffer_read_enable = true;
    }

out:
    return err;
}

long ctrl_cmd_set_prm_sniffer_mode(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_prm_sniffer_params prm_sniffer_params;
    struct sx_dev               *dev;
    int                          err;

    err = copy_from_user(&prm_sniffer_params, (struct ku_prm_sniffer_params *)data, sizeof(prm_sniffer_params));
    if (err) {
        pr_err("ctrl_cmd_set_prm_sniffer_mode: copy_from_user failed (%d)\n", err);
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        pr_err("ctrl_cmd_set_prm_sniffer_mode: dev is NULL\n");
        return -ENODEV;
    }

    sx_priv(dev)->prm_sniffer_enable = (prm_sniffer_params.enable != 0);
    pr_info("ctrl_cmd_set_prm_sniffer_mode: prm_sniffer_enable set to %d\n",
            sx_priv(dev)->prm_sniffer_enable);

out:
    return err;
}

long ctrl_cmd_sysfs_sniffer_flush(struct file *file, unsigned int cmd, unsigned long data)
{
    sxd_sysfs_access_sniffer_notification_t *marker = NULL;
    int                                      sniff_struct_len = sizeof(*marker);
    struct sx_dev                           *dev = NULL;
    int                                      ret = 0;

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    marker = kzalloc(sniff_struct_len, GFP_KERNEL);
    if (!marker) {
        return -ENOMEM;
    }

    marker->device_id = dev->device_id;
    strncpy(marker->func_name, SXD_SYSFS_SNIFFER_FLUSH_MARKER, SXD_SYSFS_CB_NAME_MAX);
    marker->len = 0;

    ret = send_trap(marker,
                    sniff_struct_len,
                    SXD_TRAP_ID_SYSFS_SNIFFER,
                    0,
                    dev->device_id,
                    TARGET_PID_DONT_CARE,
                    GFP_KERNEL);
    if (ret) {
        sxd_log_err(dev, "ctrl_cmd_sysfs_sniffer_flush: send_trap failed [%d]\n", ret);
    }

    kfree(marker);
    return ret;
}
