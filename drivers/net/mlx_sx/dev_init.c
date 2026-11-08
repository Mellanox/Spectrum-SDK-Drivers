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

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/workqueue.h>

#include <linux/mlx_sx/auto_registers/reg.h>
#include <linux/mlx_sx/device.h>
#include <linux/mlx_sx/cmd.h>

#include "sx.h"
#include "alloc.h"
#include "sx_af_counters.h"
#include "cq.h"
#include "dq.h"
#include "sx_clock.h"
#include "ptp.h"
#include "ber_monitor.h"
#include "dev_db.h"
#include "dev_init.h"
#include "dev_callbacks.h"
#include "health_check.h"
#include "emad.h"
#include "sgmii.h"
#include "sx_dbg_dump_proc.h"
#include "synce.h"

/****************************************************************************************
 * MODULE PARAMETERS
 ***************************************************************************************/

#ifdef CONFIG_PCI_MSI
static int msi_x = 1;
module_param(msi_x, int, 0444);
MODULE_PARM_DESC(msi_x, "attempt to use MSI-X if nonzero");
#else /* CONFIG_PCI_MSI */
static int msi_x = 0;
#endif /* CONFIG_PCI_MSI */

int chip_info_type = -1;
module_param_named(chip_info_type, chip_info_type, int, 0444);
MODULE_PARM_DESC(chip_info_type, "chip_info: type");

int chip_info_revision = -1;
module_param_named(chip_info_revision, chip_info_revision, int, 0444);
MODULE_PARM_DESC(chip_info_revision, "chip_info: revision");

char chip_info_psid[16] = "";
module_param_string(chip_info_psid, chip_info_psid, 16, 0444);
MODULE_PARM_DESC(chip_info_psid, "chip_info: psid");

int g_chip_type = 0;
module_param_named(g_chip_type,
                   g_chip_type, int, 0644);
MODULE_PARM_DESC(g_chip_type, " set chip type for NO PCI and SGMII");

int debug_sdq_size = -1;
module_param_named(debug_sdq_size, debug_sdq_size, int, 0644);
MODULE_PARM_DESC(debug_sdq_size, "DEBUG feature: set SDQ size to 2^n entries");

int debug_rdq_size = -1;
module_param_named(debug_rdq_size, debug_rdq_size, int, 0644);
MODULE_PARM_DESC(debug_rdq_size, "DEBUG feature: set SDQ size to 2^n entries");

int multi_asic_fs_mode = 0;
module_param_named(multi_asic_fs_mode, multi_asic_fs_mode, int, 0444);
MODULE_PARM_DESC(multi_asic_fs_mode,
                 "Driver works in multi-ASIC FS mode (even if running on single ASIC, override legacy mode)");

extern int skip_reset;

/****************************************************************************************
 * forward declaration of static symbols
 ***************************************************************************************/

struct pci_restart_saved_params {
    sxd_dev_id_t                  dev_id;
    struct listeners_and_rf_info *listeners;
    struct sx_mmap_info          *mmap_info;
    struct sx_device_info_set     dev_info_set;
    bool                          emergency_reset_done;
    u32                           reset_duration_msec;
};

static int __pci_probe_common(struct pci_dev                  *pdev,
                              struct sx_dev                  **new_dev,
                              struct pci_restart_saved_params *saved_params);
static void __pci_remove_common(struct pci_dev *pdev, struct pci_restart_saved_params *saved_params);
#ifndef SXD_KERNEL_DISABLE_PCI_DRV_SHUTDOWN
static void __pci_shutdown_common(struct pci_dev *pdev);
#endif

static int __pci_probe_core(struct pci_dev *pdev, const struct pci_device_id *id);
static void __pci_remove_core(struct pci_dev *pdev);
#ifndef SXD_KERNEL_DISABLE_PCI_DRV_SHUTDOWN
static void __pci_shutdown_core(struct pci_dev *pdev);
#endif

static int __pci_probe_oob(struct pci_dev *pdev, const struct pci_device_id *id);
static void __pci_remove_oob(struct pci_dev *pdev);
#ifndef SXD_KERNEL_DISABLE_PCI_DRV_SHUTDOWN
static void __pci_shutdown_oob(struct pci_dev *pdev);
#endif

static int __sx_core_init_cb(struct sx_dev *dev, uint16_t device_id, uint16_t device_hw_revision);
static int __sx_load_fw(struct sx_dev *dev);
static void __sx_core_disconnect_all_trap_groups(struct sx_dev *dev);

static struct sx_priv * __create_dummy_device(int chip_type, struct pci_dev *pdev);
static void __remove_dummy_device(struct sx_priv *priv);

#define SX_DEV_INIT_MAX_WAIT_SYSTEM_READY_MSEC (20000)

/****************************************************************************************
 * STATIC VARIABLES
 ***************************************************************************************/

/* in use by probe function. must be accessed within pci_restart_lock! */
static struct sx_pci_probe_params {
    bool do_reset;
    u32  total_probes;
    u32  successful_probes;
    u32  fw_boot_failures;
    u32  total_removes;
    bool is_oob;
} __probe_params;

/****************************************************************************************
 * core PCI driver structures
 ***************************************************************************************/
static struct pci_device_id __sx_pci_table[] = {
    /* Spectrum PCI device ID */
    { PCI_VDEVICE(MELLANOX, SPECTRUM_PCI_DEV_ID) },

    /* Spectrum 'in flash recovery mode' PCI device ID */
    { PCI_VDEVICE(MELLANOX, SPECTRUM_FLASH_MODE_PCI_DEV_ID) },

    /* Spectrum2 PCI device ID */
    { PCI_VDEVICE(MELLANOX, SPECTRUM2_PCI_DEV_ID) },

    /* Spectrum2 'in flash recovery mode' PCI device ID */
    { PCI_VDEVICE(MELLANOX, SPECTRUM2_FLASH_MODE_PCI_DEV_ID) },

    /* Spectrum3 PCI device ID */
    { PCI_VDEVICE(MELLANOX, SPECTRUM3_PCI_DEV_ID) },

    /* Spectrum3 'in flash recovery mode' PCI device ID */
    { PCI_VDEVICE(MELLANOX, SPECTRUM3_FLASH_MODE_PCI_DEV_ID) },

    /* Spectrum4 PCI device ID */
    { PCI_VDEVICE(MELLANOX, SPECTRUM4_PCI_DEV_ID) },

    /* Spectrum4 'in flash recovery mode' PCI device ID */
    { PCI_VDEVICE(MELLANOX, SPECTRUM4_FLASH_MODE_PCI_DEV_ID) },

    /* Spectrum4 'RMA' PCI device ID */
    { PCI_VDEVICE(MELLANOX, SPECTRUM4_RMA_PCI_DEV_ID) },

    /* Spectrum5 PCI device ID */
    { PCI_VDEVICE(MELLANOX, SPECTRUM5_PCI_DEV_ID) },

    /* Spectrum5 'in flash recovery mode' PCI device ID */
    { PCI_VDEVICE(MELLANOX, SPECTRUM5_FLASH_MODE_PCI_DEV_ID) },

    /* Spectrum5 'RMA' PCI device ID */
    { PCI_VDEVICE(MELLANOX, SPECTRUM5_RMA_PCI_DEV_ID) },

    /* Spectrum6 PCI device ID */
    { PCI_VDEVICE(MELLANOX, SPECTRUM6_PCI_DEV_ID) },

    /* Spectrum6 'in flash recovery mode' PCI device ID */
    { PCI_VDEVICE(MELLANOX, SPECTRUM6_FLASH_MODE_PCI_DEV_ID) },

    /* Spectrum6 'RMA' PCI device ID */
    { PCI_VDEVICE(MELLANOX, SPECTRUM6_RMA_PCI_DEV_ID) },

    /* Spectrum7 PCI device ID */
    { PCI_VDEVICE(MELLANOX, SPECTRUM7_PCI_DEV_ID) },

    /* Spectrum7 'in flash recovery mode' PCI device ID */
    { PCI_VDEVICE(MELLANOX, SPECTRUM7_FLASH_MODE_PCI_DEV_ID) },

    /* Spectrum7 'RMA' PCI device ID */
    { PCI_VDEVICE(MELLANOX, SPECTRUM7_RMA_PCI_DEV_ID) },

    /* Quantum2 PCI device ID */
    { PCI_VDEVICE(MELLANOX, QUANTUM2_PCI_DEV_ID) },

    /* Quantum2 'in flash recovery mode' PCI device ID */
    { PCI_VDEVICE(MELLANOX, QUANTUM2_FLASH_MODE_PCI_DEV_ID) },

    /* Quantum3 PCI device ID */
    { PCI_VDEVICE(MELLANOX, QUANTUM3_PCI_DEV_ID) },

    /* Quantum3 'in flash recovery mode' PCI device ID */
    { PCI_VDEVICE(MELLANOX, QUANTUM3_FLASH_MODE_PCI_DEV_ID) },

    /* Quantum4 PCI device ID */
    { PCI_VDEVICE(MELLANOX, QUANTUM4_PCI_DEV_ID) },

    /* Quantum4 'in flash recovery mode' PCI device ID */
    { PCI_VDEVICE(MELLANOX, QUANTUM4_FLASH_MODE_PCI_DEV_ID) },

    /* Quantum5 PCI device ID */
    { PCI_VDEVICE(MELLANOX, QUANTUM5_PCI_DEV_ID) },

    /* Quantum5 'in flash recovery mode' PCI device ID */
    { PCI_VDEVICE(MELLANOX, QUANTUM5_FLASH_MODE_PCI_DEV_ID) },

    { 0, }
};

MODULE_DEVICE_TABLE(pci, __sx_pci_table);

static struct pci_driver __sx_driver = {
    .name = DRV_NAME,
    .id_table = __sx_pci_table,
    .probe = __pci_probe_core,
    .remove = __pci_remove_core,
#ifndef SXD_KERNEL_DISABLE_PCI_DRV_SHUTDOWN
    .shutdown = __pci_shutdown_core
#endif
};

/****************************************************************************************
 * OOB PCI backbone driver structures
 ***************************************************************************************/
static struct pci_device_id __sx_oob_pci_table[] = {
    /* SwitchX PCI device ID */
    { PCI_VDEVICE(MELLANOX, SWITCHX_PCI_DEV_ID) },
    { 0, }
};

MODULE_DEVICE_TABLE(pci, __sx_oob_pci_table);

static struct pci_driver __sx_oob_driver = {
    .name = DRV_NAME "_oob",
    .id_table = __sx_oob_pci_table,
    .probe = __pci_probe_oob,
    .remove = __pci_remove_oob,
#ifndef SXD_KERNEL_DISABLE_PCI_DRV_SHUTDOWN
    .shutdown = __pci_shutdown_oob
#endif
};

/****************************************************************************************
 * STATIC functions
 ***************************************************************************************/
static void __sx_close_board(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);
    int             i;

    sx_UNMAP_FA(dev);

    for (i = 0; i < ARRAY_SIZE(priv->fw.map_fa_items); i++) {
        sx_map_fa_item_dealloc(priv->fw.map_fa_items[i]);
        priv->fw.map_fa_items[i] = SX_MAP_FA_INVALID_HANDLE;
    }
}

static void __sx_cr_space_cleanup(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);

    iounmap(priv->cr_space_start);
    priv->cr_space_size = 0;
}

static void __sx_setup_sx_cleanup(struct sx_dev *dev)
{
    sx_af_counters_deinit(dev);
    sx_bulk_cntr_streaming_profile_unregister(dev);
    sx_core_destroy_rdq_table(dev, true);
    sx_core_destroy_sdq_table(dev, true);
    sx_cmd_use_polling(dev);
    sx_cleanup_eq_table(dev);
    sx_core_destroy_cq_table(dev);
}

static int __sx_map_cr_space_area(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);

    priv->cr_space_size = pci_resource_end(dev->pdev, 0) -
                          pci_resource_start(dev->pdev, 0);
    priv->cr_space_start = ioremap(pci_resource_start(dev->pdev, 0),
                                   priv->cr_space_size);
    if (!priv->cr_space_start) {
        sxd_log_err(dev, "%s(): cr_space ioremap failed \n", __func__);
        return -EINVAL;
    }

    sxd_log_debug(dev, "%s(): map cr_space area p:0x%llx, size:%d, cr_space start:%p \n",
                  __func__,  pci_resource_start(dev->pdev, 0), priv->cr_space_size, priv->cr_space_start);

    return 0;
}

static void __sx_doorbell_cleanup(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);

    iounmap(priv->db_base);
}

static void __sx_enable_msi_x(struct sx_dev *dev)
{
    struct sx_priv   *priv = sx_priv(dev);
    struct msix_entry entry;
    int               err;
    int               i;

    if (msi_x) {
        entry.entry = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
        err = pci_enable_msix_range(dev->pdev, &entry, 1, 1);
#else
        err = pci_enable_msix(dev->pdev, &entry, 1);
        if (err > 0) {
            sxd_log_info(dev, "Only %d MSI-X vectors available, "
                         "not using MSI-X\n", err);

            goto no_msi;
        }
#endif
        if (err < 0) {
            sxd_log_debug(dev, "Failed enabling MSI-X interrupts. "
                          "Going to use standard interrupts instead\n");

            goto no_msi;
        }

        sx_info(dev, "MSI-X interrupts were enabled successfully\n");
        for (i = 0; i < SX_NUM_EQ; ++i) {
            priv->eq_table.eq[i].irq = entry.vector;
        }

        priv->flags |= SX_FLAG_MSI_X;
        return;
    }

no_msi:
    msi_x = 0;
    for (i = 0; i < SX_NUM_EQ; ++i) {
        priv->eq_table.eq[i].irq = dev->pdev->irq;
    }
}

static int __call_MGIR(struct sx_dev *dev)
{
    struct ku_access_mgir_reg reg_data;
    int                       err = 0;
    int                       retry_num = 0;

    /*
     *   This is a workaround to race condition happens when FW
     *   boot isn't finished and we start to read MGIR.
     *   We post the in_mailbox but FW zero GO bit. So we think
     *   that command is done.
     *   After this race we get 0 in all MGIR fields.
     *   The temporary solution is to reread again.
     *   The real solution should provide interface to read HEALTH
     *   bits which will indicate that FW boot is finished.
     */
    while (retry_num < 3) {
        memset(&reg_data, 0, sizeof(reg_data));
        reg_data.dev_id = dev->device_id;
        sx_cmd_set_op_tlv(&reg_data.op_tlv, MLXSW_MGIR_ID, SXD_EMAD_METHOD_QUERY_E);

        err = sx_ACCESS_REG_MGIR(dev, &reg_data);
        if (err) {
            sxd_log_err(dev, "MGIR call failed (err=%d)\n", err);
        } else if (reg_data.mgir_reg.hw_info.device_id == 0) {
            sxd_log_warning(dev, "MGIR call succeeded but device-id is not initialized\n");
            err = -ENODEV;
        } else {
            memcpy(&sx_priv(dev)->dev_info.dev_info_ro.mgir, &reg_data.mgir_reg, sizeof(struct ku_mgir_reg));
            err = 0;
            break;
        }

        msleep(500 * retry_num);
        retry_num++;
    }

    return err;
}


static int __sx_init_chip_callbacks(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);
    int             err = 0;

    err = __call_MGIR(dev);
    if (err) {
        goto out;
    }

    MODULE_PARAM_INSTANCE_SET(priv, chip_info_type, priv->dev_info.dev_info_ro.mgir.hw_info.device_id);
    MODULE_PARAM_INSTANCE_SET(priv, chip_info_revision, priv->dev_info.dev_info_ro.mgir.hw_info.device_hw_revision);
    strncpy(chip_info_psid, (const char*)priv->dev_info.dev_info_ro.mgir.fw_info.psid, sizeof(chip_info_psid) - 1);

    err =
        __sx_core_init_cb(dev, MODULE_PARAM_INSTANCE_GET(priv, chip_info_type),
                          MODULE_PARAM_INSTANCE_GET(priv, chip_info_revision));
    if (err) {
        sxd_log_err(dev, "Failed to set callbacks for device (dev_id=%u)\n", dev->device_id);
    }

out:
    return err;
}

static int __sx_init_board(struct sx_dev *dev)
{
    struct sx_priv            *priv = sx_priv(dev);
    int                        err;
    struct ku_query_board_info board;

    err = sx_QUERY_FW(dev, NULL);
    if (err) {
        sx_err(dev, "QUERY_FW command failed, aborting.\n");
        return err;
    }

    /* init local mailboxes */
    err = sx_QUERY_FW_2(dev, dev->device_id, HCR1);
    if (err) {
        sx_err(dev, "QUERY_FW_2 command failed, aborting.\n");
        return err;
    }

    priv->bar0_dbregs_offset = priv->fw.doorbell_page_offset;
    priv->bar0_dbregs_bar = priv->fw.doorbell_page_bar;

    err = __sx_load_fw(dev);
    if (err) {
        sx_err(dev, "Failed to start FW, aborting.\n");
        return err;
    }

    err = sx_QUERY_AQ_CAP(dev);
    if (err) {
        sx_err(dev, "QUERY_AQ_CAP command failed, aborting.\n");
        goto err_stop_fw;
    }
    priv->dev_cap.max_num_cpu_egress_tcs = 12;
    priv->dev_cap.max_num_cpu_ingress_tcs = 16;

    err = sx_QUERY_BOARDINFO(dev, &board);
    if (err) {
        sx_err(dev, "QUERY_BOARDINFO command failed, aborting.\n");
        goto err_stop_fw;
    }

    priv->eq_table.inta_pin = board.inta_pin;
    memcpy(priv->board_id, board.board_id, sizeof(priv->board_id));
    priv->vsd_vendor_id = board.vsd_vendor_id;
    return 0;

err_stop_fw:
    sx_UNMAP_FA(dev);
    sx_map_fa_item_dealloc(priv->fw.map_fa_items[SXD_MAP_MEMORY_TYPE_FW_LOAD_E]);
    priv->fw.map_fa_items[SXD_MAP_MEMORY_TYPE_FW_LOAD_E] = SX_MAP_FA_INVALID_HANDLE;

    return err;
}

/* PCI-device-ID to sxd-chip-type (RAW, without hw_revision resolution) */
static enum sxd_chip_types __pci_dev_id_to_chip_type(u16 device_id)
{
    switch (device_id) {
    case SXD_MGIR_HW_DEV_ID_SX:
        return SXD_CHIP_TYPE_SWITCHX_A0;

    case SXD_MGIR_HW_DEV_ID_SWITCH_IB:
        return SXD_CHIP_TYPE_SWITCH_IB;

    case SXD_MGIR_HW_DEV_ID_SPECTRUM:
        return SXD_CHIP_TYPE_SPECTRUM;

    case SXD_MGIR_HW_DEV_ID_SWITCH_IB2:
        return SXD_CHIP_TYPE_SWITCH_IB2;

    case SXD_MGIR_HW_DEV_ID_QUANTUM:
        return SXD_CHIP_TYPE_QUANTUM;

    case SXD_MGIR_HW_DEV_ID_QUANTUM2:
        return SXD_CHIP_TYPE_QUANTUM2;

    case SXD_MGIR_HW_DEV_ID_QUANTUM3:
        return SXD_CHIP_TYPE_QUANTUM3;

    case SXD_MGIR_HW_DEV_ID_QUANTUM4:
        return SXD_CHIP_TYPE_QUANTUM4;

    case SXD_MGIR_HW_DEV_ID_QUANTUM5:
        return SXD_CHIP_TYPE_QUANTUM5;

    case SXD_MGIR_HW_DEV_ID_SPECTRUM2:
        return SXD_CHIP_TYPE_SPECTRUM2;

    case SXD_MGIR_HW_DEV_ID_SPECTRUM3:
        return SXD_CHIP_TYPE_SPECTRUM3;

    case SXD_MGIR_HW_DEV_ID_SPECTRUM4:
        return SXD_CHIP_TYPE_SPECTRUM4;

    case SXD_MGIR_HW_DEV_ID_SPECTRUM5:
        return SXD_CHIP_TYPE_SPECTRUM5;

    case SXD_MGIR_HW_DEV_ID_SPECTRUM6:
        return SXD_CHIP_TYPE_SPECTRUM6;

    case SXD_MGIR_HW_DEV_ID_SPECTRUM7:
        return SXD_CHIP_TYPE_SPECTRUM7;

    default:
        break;
    }

    sxd_log_err(SXD_DEV_NONE, "ERROR: Unresolved chip type\n");
    return SXD_CHIP_TYPE_UNKNOWN;
}

static int __sx_core_init_cb(struct sx_dev *dev, uint16_t device_id, uint16_t device_hw_revision)
{
    int                 err = 0;
    enum sxd_chip_types chip_type;

    chip_type = __pci_dev_id_to_chip_type(device_id);

    switch (chip_type) {
    case SXD_CHIP_TYPE_SWITCHX_A0:
        if (device_hw_revision == 0xA1) {
            chip_type = SXD_CHIP_TYPE_SWITCHX_A1;
        } else if (device_hw_revision == 0xA2) {
            chip_type = SXD_CHIP_TYPE_SWITCHX_A2;
        } else {
            sxd_log_err(dev, "The SwitchX device revision (0x%x) "
                        "is not supported by SX driver\n", device_hw_revision);
            return -EFAULT;
        }
        break;

    case SXD_CHIP_TYPE_SPECTRUM:
        if (device_hw_revision == 0xA1) {
            chip_type = SXD_CHIP_TYPE_SPECTRUM_A1;
        } else if (device_hw_revision == 0xA0) {
            chip_type = SXD_CHIP_TYPE_SPECTRUM;
        } else {
            sxd_log_err(dev, "The Spectrum device revision (0x%x) "
                        "is not supported by SX driver\n", device_hw_revision);
            return -EFAULT;
        }
        break;

    case SXD_CHIP_TYPE_UNKNOWN:
        sxd_log_err(dev, "ERROR: Unresolved chip type. device_id (%u)\n", device_id);
        return -EFAULT;

    default:
        /* leave chip_type as is */
        break;
    }

    err = sx_core_dev_init_switchx_cb(dev, chip_type, 0);
    if (err) {
        sxd_log_err(dev, "callback device init failed for device (%u)\n",
                    dev->device_id);
        return err;
    }

    return err;
}

/* this will initialize device callbacks from pci-dev ID and not from MGIR
 *  return value. it means that we can initialize the callbacks even if FW
 *  is stuck */
static int __sx_core_init_cb_by_pdev(struct sx_dev *dev)
{
    struct sx_priv     *priv = sx_priv(dev);
    enum sxd_chip_types chip_type;
    int                 err = 0;

    chip_type = __pci_dev_id_to_chip_type(dev->pdev->device);
    if (chip_type == SXD_CHIP_TYPE_UNKNOWN) {
        sxd_log_err(dev, "failed to get chip type from pci-device (0x%x)\n", dev->pdev->device);
        return -EINVAL;
    }

    err = sx_core_dev_init_cb(priv, chip_type);
    if (err) {
        sxd_log_err(dev,
                    "callback device init failed for device (%u)\n",
                    dev->device_id);
    }

    return err;
}

/**
 * Update the device's cap struct with the default capabilities of the HW
 * (number of RDQs, SDQs, CQs Etc.)
 */
static void __set_default_capabilities(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);

    if (debug_sdq_size >= 0) {
        sxd_log_notice(dev, "DEBUG: SDQ size is set to be 2^%d entries\n", debug_sdq_size);
    }

    if (debug_rdq_size >= 0) {
        sxd_log_notice(dev, "DEBUG: RDQ size is set to be 2^%d entries\n", debug_rdq_size);
    }

    priv->dev_cap.log_max_rdq_sz = SX_MAX_LOG_DQ_SIZE;
    priv->dev_cap.log_max_sdq_sz = SX_MAX_LOG_DQ_SIZE;
    priv->dev_cap.log_max_cq_sz = 10;

    priv->dev_cap.max_num_rdqs = NUMBER_OF_RDQS;
    priv->dev_cap.max_num_sdqs = NUMBER_OF_SDQS;
    priv->dev_cap.max_num_cqs = NUMBER_OF_RDQS + NUMBER_OF_SDQS;

    priv->dev_cap.max_num_cpu_egress_tcs = 12;
    priv->dev_cap.max_num_cpu_ingress_tcs = 16;
}

static int __sx_load_fw(struct sx_dev *dev)
{
    struct sx_priv    *priv = sx_priv(dev);
    sx_map_fa_handle_t fw_load = SX_MAP_FA_INVALID_HANDLE;
    int                err = 0;

    err = sx_map_fa_item_alloc(dev, priv->fw.fw_pages, &fw_load);
    if (err) {
        sx_err(dev, "Couldn't allocate FW area, aborting. (err=%d)\n", err);
        return err;
    }

    err = sx_MAP_FA(dev, SXD_MAP_MEMORY_TYPE_FW_LOAD_E, fw_load);
    if (err) {
        sx_err(dev, "MAP_FA command failed, aborting.\n");
        goto err_free;
    }

    priv->fw.map_fa_items[SXD_MAP_MEMORY_TYPE_FW_LOAD_E] = fw_load;

    return 0;

err_free:
    /* in case of (err == -ETIMEDOUT), we don't know if FW is really stuck or it just
     * takes some more time to prepare itself to use the host memory. Therefore, we're
     * not going to release the host memory here, only when driver goes down (__pci_remove_common())
     * and only after sx_UNMAP_FA() so we're sure FW is done using the host memory.
     */
    if (err != -ETIMEDOUT) {
        sx_map_fa_item_dealloc(fw_load);
    }

    return err;
}

static int __sx_map_doorbell_area(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);

    priv->db_base =
        ioremap(pci_resource_start(dev->pdev, priv->bar0_dbregs_bar)
                + priv->bar0_dbregs_offset,
                SX_DBELL_REGION_SIZE);
    if (!priv->db_base) {
        sxd_log_err(dev, "%s(): bar: %d doorbell base: is NULL \n",
                    __func__, priv->bar0_dbregs_bar);

        return -EINVAL;
    }

    sxd_log_debug(dev, "%s(): bar: %d dev->db_base phys: 0x%llx , doorbell base: %p \n",
                  __func__,
                  priv->bar0_dbregs_bar,
                  pci_resource_start(dev->pdev, priv->bar0_dbregs_bar)
                  + priv->bar0_dbregs_offset,
                  priv->db_base);

    return 0;
}

static int __sx_setup_sx(struct sx_dev *dev)
{
    int err = 0;

    err = sx_init_eq_table(dev);
    if (err) {
        sx_err(dev, "Failed to initialize "
               "event queue table, aborting.\n");
        goto out_ret;
    }

    err = sx_cmd_use_events(dev);
    if (err) {
        sx_err(dev, "Failed to switch to event-driven "
               "firmware commands, aborting.\n");
        goto err_eq_table_free;
    }

    err = sx_core_init_cq_table(dev);
    if (err) {
        sx_err(dev, "Failed to initialize CQ table, aborting.\n");
        goto err_cmd_poll;
    }

    err = sx_core_init_sdq_table(dev);
    if (err) {
        sx_err(dev, "Failed to initialize SDQ table, aborting.\n");
        goto err_cq_table_free;
    }

    err = sx_core_init_rdq_table(dev);
    if (err) {
        sx_err(dev, "Failed to initialize RDQ table, aborting.\n");
        goto err_sdq_table_free;
    }

    err = sx_core_rdq_usage_monitor_init(dev);
    if (err) {
        sx_err(dev, "Failed to initialize RDQ usage monitor, aborting.\n");
        goto err_rdq_table_free;
    }

    return 0;

err_rdq_table_free:
    sx_core_destroy_rdq_table(dev, true);

err_sdq_table_free:
    sx_core_destroy_sdq_table(dev, true);

err_cq_table_free:
    sx_core_destroy_cq_table(dev);

err_cmd_poll:
    sx_cmd_use_polling(dev);

err_eq_table_free:
    sx_cleanup_eq_table(dev);

out_ret:
    return err;
}

static bool __check_fw_boot_status(struct pci_dev *pdev, struct sx_priv **priv)
{
    struct sx_priv           *dummy_priv = NULL;
    const char               *busname = dev_name(&pdev->dev);
    unsigned short            pci_dev_id = pdev->device;
    enum sxd_fw_boot_status_e fw_boot_status = SXD_FW_BOOT_STATUS_OK_E;
    int                       chip_type = 0;
    int                       chip_info_type_instance = chip_info_type;
    int                       chip_info_revision_instance = chip_info_revision;

    switch (pci_dev_id) {
    case SPECTRUM_FLASH_MODE_PCI_DEV_ID:
        chip_info_type_instance = SPECTRUM_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_SPECTRUM;
        fw_boot_status = SXD_FW_BOOT_STATUS_IN_FLASH_RECOVERY_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Spectrum on bus %s is in 'Flash Mode' status\n", busname);
        break;

    case SPECTRUM2_FLASH_MODE_PCI_DEV_ID:
        chip_info_type_instance = SPECTRUM2_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_SPECTRUM2;
        fw_boot_status = SXD_FW_BOOT_STATUS_IN_FLASH_RECOVERY_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Spectrum-2 on bus %s is in 'Flash Mode' status\n", busname);
        break;

    case SPECTRUM3_FLASH_MODE_PCI_DEV_ID:
        chip_info_type_instance = SPECTRUM3_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_SPECTRUM3;
        fw_boot_status = SXD_FW_BOOT_STATUS_IN_FLASH_RECOVERY_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Spectrum-3 on bus %s is in 'Flash Mode' status\n", busname);
        break;

    case SPECTRUM4_FLASH_MODE_PCI_DEV_ID:
        chip_info_type_instance = SPECTRUM4_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_SPECTRUM4;
        fw_boot_status = SXD_FW_BOOT_STATUS_IN_FLASH_RECOVERY_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Spectrum-4 on bus %s is in 'Flash Mode' status\n", busname);
        break;

    case SPECTRUM4_RMA_PCI_DEV_ID:
        chip_info_type_instance = SPECTRUM4_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_SPECTRUM4;
        fw_boot_status = SXD_FW_BOOT_STATUS_RMA_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Spectrum-4 on bus %s is in 'RMA' status\n", busname);
        break;

    case SPECTRUM5_FLASH_MODE_PCI_DEV_ID:
        chip_info_type_instance = SPECTRUM5_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_SPECTRUM5;
        fw_boot_status = SXD_FW_BOOT_STATUS_IN_FLASH_RECOVERY_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Spectrum-5 on bus %s is in 'Flash Mode' status\n", busname);
        break;

    case SPECTRUM5_RMA_PCI_DEV_ID:
        chip_info_type_instance = SPECTRUM5_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_SPECTRUM5;
        fw_boot_status = SXD_FW_BOOT_STATUS_RMA_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Spectrum-5 on bus %s is in 'RMA' status\n", busname);
        break;

    case SPECTRUM6_FLASH_MODE_PCI_DEV_ID:
        chip_info_type_instance = SPECTRUM6_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_SPECTRUM6;
        fw_boot_status = SXD_FW_BOOT_STATUS_IN_FLASH_RECOVERY_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Spectrum-6 on bus %s is in 'Flash Mode' status\n", busname);
        break;

    case SPECTRUM6_RMA_PCI_DEV_ID:
        chip_info_type_instance = SPECTRUM6_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_SPECTRUM6;
        fw_boot_status = SXD_FW_BOOT_STATUS_RMA_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Spectrum-6 on bus %s is in 'RMA' status\n", busname);
        break;

    case SPECTRUM7_FLASH_MODE_PCI_DEV_ID:
        chip_info_type_instance = SPECTRUM7_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_SPECTRUM7;
        fw_boot_status = SXD_FW_BOOT_STATUS_IN_FLASH_RECOVERY_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Spectrum-7 on bus %s is in 'Flash Mode' status\n", busname);
        break;

    case SPECTRUM7_RMA_PCI_DEV_ID:
        chip_info_type_instance = SPECTRUM7_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_SPECTRUM7;
        fw_boot_status = SXD_FW_BOOT_STATUS_RMA_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Spectrum-7 on bus %s is in 'RMA' status\n", busname);
        break;

    case SWITCH_IB_FLASH_MODE_PCI_DEV_ID:
        chip_info_type_instance = SWITCH_IB_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_SWITCH_IB;
        fw_boot_status = SXD_FW_BOOT_STATUS_IN_FLASH_RECOVERY_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Switch-IB on bus %s is in 'Flash Mode' status\n", busname);
        break;

    case SWITCH_IB2_FLASH_MODE_PCI_DEV_ID:
        chip_info_type_instance = SWITCH_IB2_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_SWITCH_IB2;
        fw_boot_status = SXD_FW_BOOT_STATUS_IN_FLASH_RECOVERY_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Switch-IB/2 on bus %s is in 'Flash Mode' status\n", busname);
        break;

    case QUANTUM_FLASH_MODE_PCI_DEV_ID:
        chip_info_type_instance = QUANTUM_PCI_DEV_ID;
        chip_info_revision_instance = 0;
        chip_type = SXD_CHIP_TYPE_QUANTUM;
        fw_boot_status = SXD_FW_BOOT_STATUS_IN_FLASH_RECOVERY_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Quantum on bus %s is in 'Flash Mode' status\n", busname);
        break;

    case QUANTUM2_FLASH_MODE_PCI_DEV_ID:
        chip_info_type_instance = QUANTUM2_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_QUANTUM2;
        fw_boot_status = SXD_FW_BOOT_STATUS_IN_FLASH_RECOVERY_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Quantum-2 on bus %s is in 'Flash Mode' status\n", busname);
        break;

    case QUANTUM2_RMA_PCI_DEV_ID:
        chip_info_type_instance = QUANTUM2_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_QUANTUM2;
        fw_boot_status = SXD_FW_BOOT_STATUS_RMA_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Quantum-2 on bus %s is in 'RMA' status\n", busname);
        break;

    case QUANTUM3_FLASH_MODE_PCI_DEV_ID:
        chip_info_type_instance = QUANTUM3_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_QUANTUM3;
        fw_boot_status = SXD_FW_BOOT_STATUS_IN_FLASH_RECOVERY_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Quantum-3 on bus %s is in 'Flash Mode' status\n", busname);
        break;

    case QUANTUM3_RMA_PCI_DEV_ID:
        chip_info_type_instance = QUANTUM3_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_QUANTUM3;
        fw_boot_status = SXD_FW_BOOT_STATUS_RMA_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Quantum-3 on bus %s is in 'RMA' status\n", busname);
        break;

    case QUANTUM4_FLASH_MODE_PCI_DEV_ID:
        chip_info_type_instance = QUANTUM4_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_QUANTUM4;
        fw_boot_status = SXD_FW_BOOT_STATUS_IN_FLASH_RECOVERY_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Quantum-4 on bus %s is in 'Flash Mode' status\n", busname);
        break;

    case QUANTUM5_FLASH_MODE_PCI_DEV_ID:
        chip_info_type_instance = QUANTUM5_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_QUANTUM5;
        fw_boot_status = SXD_FW_BOOT_STATUS_IN_FLASH_RECOVERY_E;
        break;

    case QUANTUM4_RMA_PCI_DEV_ID:
        chip_info_type_instance = QUANTUM4_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_QUANTUM4;
        fw_boot_status = SXD_FW_BOOT_STATUS_RMA_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Quantum-4 on bus %s is in 'RMA' status\n", busname);
        break;

    case QUANTUM5_RMA_PCI_DEV_ID:
        chip_info_type_instance = QUANTUM5_PCI_DEV_ID;
        chip_type = SXD_CHIP_TYPE_QUANTUM5;
        fw_boot_status = SXD_FW_BOOT_STATUS_RMA_E;
        sxd_log_err(SXD_DEV_NONE, "FW-Status: Quantum-5 on bus %s is in 'RMA' status\n", busname);
        break;

    default:
        return true; /* everything's fine with this device, no fw-boot issue */
    }

    dummy_priv = __create_dummy_device(chip_type, pdev);
    if (dummy_priv) {
        dummy_priv->dev.pdev = NULL;
        dummy_priv->dev_info.dev_info_ro.chip_type = chip_type;
        dummy_priv->dev_info.dev_info_ro.fw_boot_status = fw_boot_status;

        MODULE_PARAM_INSTANCE_SET(dummy_priv, chip_info_type, chip_info_type_instance);
        MODULE_PARAM_INSTANCE_SET(dummy_priv, chip_info_revision, chip_info_revision_instance);
    }

    *priv = dummy_priv;
    return false;
}

static u32 __sx_get_system_status_mem_offset(struct sx_dev *dev)
{
    u32 ret = 0;
    int err;

    err = __sx_core_dev_specific_cb_get_reference(dev);
    if (err) {
        sxd_log_err(dev,
                    "dev_specific_cb_get_ref failed. Failed to get system status memory offset (err=%d)\n",
                    err);
        goto out;
    }

    if (sx_priv(dev)->dev_specific_cb.get_system_status_mem_offset_cb) {
        ret = sx_priv(dev)->dev_specific_cb.get_system_status_mem_offset_cb();
    }

    __sx_core_dev_specific_cb_release_reference(dev);

out:
    return ret;
}

static void print_fw_version(const struct sx_priv *priv)
{
    const char *fw_branch_tag = "N/A";

    if (((char)priv->dev_info.dev_info_ro.mgir.dev_info.dev_branch_tag[0]) != '\0') {
        fw_branch_tag = (const char*)priv->dev_info.dev_info_ro.mgir.dev_info.dev_branch_tag;
    }

    sxd_log_pci_notice(priv->dev.pdev,
                       "FW version: %d.%d.%04d [dev_branch_tag: %s]\n",
                       (int)(priv->dev_info.dev_info_ro.mgir.fw_info.extended_major),
                       (int)(priv->dev_info.dev_info_ro.mgir.fw_info.extended_minor),
                       (int)(priv->dev_info.dev_info_ro.mgir.fw_info.extended_sub_minor),
                       fw_branch_tag);
}

const char * sx_cq_thread_name(enum sx_cq_thread cq_thread)
{
    static const char *cq_thread_name[] = {
        [SX_CQ_THREAD_WJH] = "wjh",
        [SX_CQ_THREAD_SDK_INTERNAL_HIGH] = "sdk_int_high",
        [SX_CQ_THREAD_SDK_INTERNAL_LOW] = "sdk_int_low",
        [SX_CQ_THREAD_SDQ] = "sdq",
        [SX_CQ_THREAD_USER] = "user"
    };

    return (cq_thread >= 0 && cq_thread < SX_CQ_THREAD_MAX) ? cq_thread_name[cq_thread] : "N/A";
}

static int __create_cq_threads(struct sx_priv *priv)
{
    sxd_dev_id_t dev_id = priv->dev.device_id;
    char         wq_name[32];
    int          i, err = 0;

    for (i = 0; i < SX_CQ_THREAD_MAX; i++) {
        if (snprintf(wq_name, sizeof(wq_name), "wq_dev%u_%s", dev_id, sx_cq_thread_name(i)) >= sizeof(wq_name)) {
            /* somehow truncated (should not be here) */
            wq_name[sizeof(wq_name) - 1] = '\0';
        }

        priv->cq_threads.wq[i] = create_singlethread_workqueue(wq_name);
        if (!priv->cq_threads.wq[i]) {
            err = -ENOMEM;
            break;
        }
    }

    if (err) {
        while (--i >= 0) {
            destroy_workqueue(priv->cq_threads.wq[i]);
            priv->cq_threads.wq[i] = NULL;
        }
    }

    return err;
}

static void __delete_cq_threads(struct sx_priv *priv)
{
    int i;

    for (i = 0; i < SX_CQ_THREAD_MAX; i++) {
        flush_workqueue(priv->cq_threads.wq[i]);
        destroy_workqueue(priv->cq_threads.wq[i]);
        priv->cq_threads.wq[i] = NULL;
    }
}

static int __pci_probe_common(struct pci_dev                  *pdev,
                              struct sx_dev                  **new_dev,
                              struct pci_restart_saved_params *saved_params)
{
    struct sx_priv     *priv = NULL;
    struct sx_dev      *dev = NULL;
    sxd_dev_id_t        dev_id = 0;
    int                 err = 0;
    int                 orig_err = 0;
    bool                coming_from_emergency_reset = false;
    struct sx_flow_info fi = {.in_pci_restart = false};

    /* at this point 'pci_restart_lock' must be acquired (write lock)! */
    BUG_ON(!rwsem_is_locked(&sx_glb.pci_restart_lock));

    __probe_params.total_probes++;

    sxd_log_pci_notice(pdev, "Probe device %u (0x%x)\n", pdev->device, pdev->device);

    if (!__check_fw_boot_status(pdev, &priv)) {
        if (!priv) {
            err = -ENODEV;
        } else {
            __probe_params.fw_boot_failures++;
        }

        err = sx_dev_db_pdev_to_dev_id(pdev, &dev_id);
        if (err) {
            sxd_log_pci_err(pdev, "failed to get device ID from PCI-device (err=%d)\n", err);
        } else {
            err = sx_add_char_dev(&sx_glb.cdev, dev_id);
        }

        goto out;
    }

    err = sx_core_init_one(&priv, saved_params);
    if (err) {
        sxd_log_pci_err(pdev, "failed to initialize device private data (err=%d)\n", err);
        goto out;
    }

    dev = &priv->dev;

    err = pci_enable_device(pdev);
    if (err) {
        sxd_log_pci_err(pdev, "failed to enable PCI device (err=%d)\n", err);
        goto pci_enable_failed;
    }

    /* Check for BARs. Length of PCI space: LOG2_CR_BAR_SIZE
     * We expect BAR0:
     * - 1MB in Baz;
     * - 4MB in Pelican;
     * - 16MB in Quantum, Phoenix;
     * - 32MB in Quantum3 (Juliet);
     * - 64MB in Firebird;
     * - 128MB in Blackbird;
     * - 256MB in Albatross, Quantum3/4;
     * - 512MB in Maglan;
     */
    if (!(pci_resource_flags(pdev, 0) & IORESOURCE_MEM) ||
        ((pci_resource_len(pdev, 0) != 1 << 20) &&
         (pci_resource_len(pdev, 0) != 1 << 22) &&
         (pci_resource_len(pdev, 0) != 1 << 24) &&
         (pci_resource_len(pdev, 0) != 1 << 25) &&
         (pci_resource_len(pdev, 0) != 1 << 26) &&
         (pci_resource_len(pdev, 0) != 1 << 27) &&
         (pci_resource_len(pdev, 0) != 1 << 28) &&
         (pci_resource_len(pdev, 0) != 1 << 29))) {
        sxd_log_pci_err(pdev, "missing BAR0 or invalid size [flags=0x%lx, len=0x%llx]\n",
                        pci_resource_flags(pdev, 0),
                        pci_resource_len(pdev, 0));
        err = -ENODEV;
        goto invalid_bar0;
    }

    err = pci_request_region(pdev, 0, DRV_NAME);
    if (err) {
        sxd_log_pci_err(pdev, "failed to request control region (err=%d)\n", err);
        goto pci_req_region_failed;
    }

    pci_set_master(pdev);
    err = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
    if (err) {
        sxd_log_pci_warning(pdev, "failed to set DMA mask to 64-bit, fallback to 32-bit (err=%d)\n", err);
        err = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
        if (err) {
            sxd_log_pci_err(pdev, "failed to set DMA mask to 32-bit (err=%d)\n", err);
            goto set_dma_mask_failed;
        }
    }

    dev->pdev = pdev;
    pci_set_drvdata(pdev, dev);

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_PCI_SETTINGS); /* successful PCI settings */

    err = sx_dev_db_add_device(dev); /* also allocated the device ID */
    if (err) {
        sxd_log_pci_err(pdev, "failed to add device to database (err=%d)\n", err);
        goto dev_db_add_failed;
    }

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_ADD_DEV); /* successful sx_dev_db_add_device() */

    if (saved_params) { /* PCI_RESTART flow */
        dev->device_id = saved_params->dev_id;
        coming_from_emergency_reset = saved_params->emergency_reset_done;

        /* no need to create a new char device, it is already created in old PCI life */
    } else { /* Normal probe flow, need to create a new char device */
        err = sx_add_char_dev(&sx_glb.cdev, dev->device_id);
        if (err) {
            sxd_log_pci_err(pdev, "failed to add character-device (err=%d)\n", err);
            goto add_char_dev_failed;
        }
    }

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_CHAR_DEV); /* successful sx_add_char_dev() */

    sx_dbg_dump_proc_fs_dev_init(dev);
    sx_internal_log_dev_init(dev);

    err = sx_dpt_init_dev_pci(dev);
    if (err) {
        sxd_log_pci_err(pdev, "failed to initialize DPT device attributes to PCI (err=%d)\n", err);
        goto dpt_init_pci_failed;
    }

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_INIT_DPT); /* successful sx_dpt_init_dev_pci() */

    err = __create_cq_threads(priv);
    if (err) {
        sxd_log_pci_err(pdev, "failed to create CQ threads (err=%d)\n", err);
        goto cq_threads_failed;
    }

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_CREATE_CQ_THREADS); /* successful __create_cq_threads() */

    /* disable any access to sysfs entries as long as we are in PCI restart flow.
     * sysfs will be enabled after profile is set with the new device */
    if (sx_core_sysfs_dev_is_enabled(dev)) {
        sx_core_sysfs_dev_disable(dev, "PCI-Probe-Common");
    }

    if (new_dev) {
        *new_dev = dev;
    }

    /* ################################################################################################# */
    /* ########## From this point, any HW/FW failure will prepare the device to CR-Space dump ########## */

    if (sx_cmd_pool_create(dev)) {
        sxd_log_pci_err(pdev, "failed to create command buffer pool (err=%d)\n", err);
        goto prepare_dev_for_cr_dump;
    }

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_CMD_POOL_CREATE); /* successful sx_cmd_pool_create() */

    if (sx_cmd_init_pci(dev)) {
        sxd_log_pci_err(pdev, "failed to initialize command interface (err=%d)\n", err);
        goto prepare_dev_for_cr_dump;
    }

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_CMD_INIT_PCI); /* successful sx_cmd_init_pci() */

    err = __sx_core_init_cb_by_pdev(dev);
    if (err) {
        sxd_log_pci_err(pdev, "failed to initialize device callbacks (err=%d)\n", err);
        goto prepare_dev_for_cr_dump;
    }

    priv->reset_info.system_status_mem_offset = __sx_get_system_status_mem_offset(dev);

    sxd_log_pci_notice(pdev, "waiting for system status to be ready (up to %u msecs)\n",
                       SX_DEV_INIT_MAX_WAIT_SYSTEM_READY_MSEC);
    err = sx_wait_for_system_ready(dev, SX_DEV_INIT_MAX_WAIT_SYSTEM_READY_MSEC, NULL);
    if (err) {
        sxd_log_pci_err(pdev, "system status is not ready, aborting probe\n");
        goto prepare_dev_for_cr_dump;
    }

    sxd_log_pci_notice(pdev, "system status is ready\n");

    /* ######################################################### */
    /* ########## Until now there was no access to FW ########## */

    /* This will call MGIR on the running FW (old image) */
    err = __sx_init_chip_callbacks(dev);
    if (err) {
        sxd_log_pci_err(pdev, "failed to initialize chip-type callbacks (err=%d)\n", err);
        goto prepare_dev_for_cr_dump;
    }

    /* This will call MCAM on the currently running FW */
    err = sx_init_reset_capabilities(dev);
    if (err) {
        sxd_log_pci_err(pdev, "failed to get chip reset capabilities (err=%d)\n", err);
        goto prepare_dev_for_cr_dump;
    }

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_INIT_CALLBACKS); /* successful __sx_init_chip_callbacks() */
    print_fw_version(priv); /* print current FW version, before reset */

    if (coming_from_emergency_reset) {
        /* in PCI-restart flow, we might have needed an emergency reset to take place in tear-down (PCI-remove) flow.
         * in this case, there is no point to reset the ASIC again in new life (PCI-probe).
         */
        sxd_log_notice(dev,
                       "emergency reset already done on device, no need for another reset\n");
        goto after_reset;
    }

#if defined(PD_BU) && !defined(PD_BU_ENABLE_SW_RESET)
    sxd_log_pci_notice(pdev, "performing SW reset is SKIPPED in PD mode\n");
#else
    /* This will call MRSR on the running FW (old image), loading the new image */
    err = sx_reset(dev, __probe_params.do_reset);
    if (err) {
        sxd_log_pci_err(pdev, "failed to reset ASIC (err=%d)\n", err);

        /*
         * sx_reset() will return -ENOTTY to indicate that SBR failed and device can be inaccessible ,
         * As a result, error handling is started and the device will be removed from the devices list
         */
        if (err == -ENOTTY) {
            goto dpt_init_pci_failed; /* the error handling flow is similar to dpt_init_pci_failed so it will be reused */
        }
        goto prepare_dev_for_cr_dump;
    }
#endif

    /* call MGIR again (after reset) to get the it updated (FW version, capabilities, etc.) */
    err = __call_MGIR(dev);
    if (err) {
        sxd_log_pci_err(pdev, "failed to call MGIR() (err=%d)\n", err);
        goto prepare_dev_for_cr_dump;
    }

    print_fw_version(priv); /* print new FW version (upon reset, if done, FW version may change) */

after_reset:

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_RESET); /* successful sx_reset() */

    /* This will call QUERY_FW, MAP_FA, QUERY_AQ_CAP, QUERY_BOARDINFO */
    err = __sx_init_board(dev);
    if (err) {
        sxd_log_pci_err(pdev, "failed to initialize board (err=%d)\n", err);
        goto prepare_dev_for_cr_dump;
    }

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_INIT_BOARD); /* successful __sx_init_board() */

    __sx_enable_msi_x(dev);

    err = __sx_map_doorbell_area(dev);
    if (err) {
        sxd_log_pci_err(pdev, "failed to map doorbell area (err=%d)\n", err);
        goto prepare_dev_for_cr_dump;
    }

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_MAP_DOORBELL); /* successful __sx_map_doorbell_area() */

    err = __sx_map_cr_space_area(dev);
    if (err) {
        sxd_log_pci_err(pdev, "failed to map CR-Space area (err=%d)\n", err);
        goto prepare_dev_for_cr_dump;
    }

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_MAP_CR_SPACE); /* successful __sx_map_cr_space_area() */

    err = sx_sysfs_get_mcam_reg_data(dev);
    if (err) {
        sxd_log_pci_err(pdev, "failed to get MCAM register data (err=%d)\n", err);
        /* Don't fail device init, just log the error */
        err = 0;
    }

    if (!__probe_params.is_oob) {
        err = sx_sysfs_asic_create(dev);
        if (err) {
            sxd_log_pci_err(pdev, "failed to create ASIC sysfs (err=%d)\n", err);
            goto prepare_dev_for_cr_dump;
        }

        SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_INIT_SYSFS); /* successful sx_sysfs_asic_create() */

        /* This will call MTECR, MTMP */
        err = sx_sysfs_asic_init_tempeature(dev);
        if (err) {
            sxd_log_pci_err(pdev, "failed to initialize temperature monitoring (err=%d)\n", err);
            goto prepare_dev_for_cr_dump;
        }

        err = sx_asic_perf_counter_init(dev);
        if (err) {
            sxd_log_pci_err(pdev, "failed to initialize performance counters (err=%d)\n", err);
            goto prepare_dev_for_cr_dump;
        }

        SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_INIT_PERF_COUNTERS); /* successful sx_asic_perf_counter_init() */
    }

    /* Initialize health check after ASIC sysfs is created */
    err = sx_health_check_dev_init(dev);
    if (err) {
        sxd_log_pci_err(pdev, "failed to initialize health check (err=%d)\n", err);
        goto prepare_dev_for_cr_dump;
    }

    priv->generic_wq = sx_health_check_create_monitored_workqueue(dev, "generic");
    if (!priv->generic_wq) {
        sxd_log_err(dev, "failed to create generic work-queue\n");
        err = -ENOMEM;
        goto prepare_dev_for_cr_dump;
    }

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_INIT_HEALTH_CHECK); /* successful sx_health_check_dev_init() */

    /* This will call SW2HW_EQ */
    err = __sx_setup_sx(dev);
    if ((err == -EBUSY) && (priv->flags & SX_FLAG_MSI_X)) {
        priv->flags &= ~SX_FLAG_MSI_X;
        pci_disable_msix(dev->pdev);
        err = __sx_setup_sx(dev);
    }

    if (err) {
        sxd_log_pci_err(pdev, "failed to setup MSI-X interface (err=%d)\n", err);
        goto prepare_dev_for_cr_dump;
    }

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_SET_ASYNC_QUEUES); /* successful __sx_setup_sx() */

    /* This will call MGIR */
    sx_emad_dev_init(dev);
    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_INIT_EMAD); /* successful sx_emad_dev_init() */

    priv->global_flushing = false;
    priv->drop_ib_mads = false; /* During ISSU IB MADs should not be sent to FW */

    sx_sysfs_update_tec_support(dev);

    /* This will call MCAM, MGIR, MTUTC, MTPPS */
    err = sx_core_clock_dev_init(priv);
    if (err) {
        sxd_log_pci_err(pdev, "failed to initialize clock (err=%d)\n", err);
        goto prepare_dev_for_cr_dump;
    }

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_INIT_CLOCK); /* successful sx_core_clock_dev_init() */

    memset(&priv->cr_dump_info, 0, sizeof(priv->cr_dump_info));
    err = sx_core_cr_dump_init(priv);
    if (err) {
        sxd_log_pci_err(pdev, "failed to initialize CR-Space dump parameters (err=%d)\n", err);
        goto prepare_dev_for_cr_dump;
    }

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_INIT_CR_DUMP); /* successful sx_core_cr_dump_init() */

    /* must be called after priv->generic_wq is ready */
    sx_core_bulk_counters_db_init(priv);

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_INIT_BULK_COUNTERS); /* sx_core_bulk_counters_db_init() done */

    /* Initialize SyncE after ASIC sysfs is created */
    err = sx_core_synce_init(dev);
    if (err) {
        sxd_log_pci_err(pdev, "failed to initialize synce (err=%d)\n", err);
        goto prepare_dev_for_cr_dump;
    }

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_INIT_SYNCE); /* successful sx_core_synce_init() */

    /* initialize Ber-Monitor */
    err = sx_core_ber_monitor_dev_init(dev);
    if (err) {
        sxd_log_pci_err(pdev, "failed to initialize ber-monitor (err=%d)\n", err);
        goto prepare_dev_for_cr_dump;
    }

    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_INIT_BER_MONITOR); /* successful sx_core_synce_init() */

    if (saved_params) {
        fi.in_pci_restart = true;
    }

    err = sx_core_register_device(dev, &fi);
    if (err) {
        sxd_log_pci_err(pdev, "failed to register device (err=%d)\n", err);
        goto prepare_dev_for_cr_dump;
    }

    sx_dbg_init(dev, NULL);
    SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_INIT_DBG);


    __probe_params.successful_probes++;
    priv->registered = true;

    /* If this is a PCI restart, re-enable all file descriptors that were marked
     * as closing during device removal. This allows userspace to continue using
     * existing file descriptors with the restarted device.
     */
    if (saved_params) {
        sx_core_mark_all_files_reopened(dev);
    }

    sxd_log_pci_notice(pdev, "PCI probe completed successfully\n");
    return 0;

prepare_dev_for_cr_dump:
    orig_err = err;
    sxd_log_pci_err(pdev, "device is in FATAL state during probe! preparing it for CR-Space dump\n");
    priv->dev_info.dev_info_ro.chip_type = __pci_dev_id_to_chip_type(pdev->device);
    priv->dev_info.dev_info_ro.flags |= SX_DEV_INFO_F_PCI_PROBE_FAILURE;

    /* map CR-Space if it is not already mapped */
    if (!SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_MAP_CR_SPACE)) {
        err = __sx_map_cr_space_area(dev);
        if (err) {
            /* still, must return 0 at the end of the probe function so driver remains up.
             * user will have to unload the driver so deinitialization will be triggered.
             */
            sxd_log_pci_err(pdev, "failed to prepare device for CR-Space dump (err=%d)\n", err);
        } else {
            SX_PROBE_STEP_SET(priv, SX_PROBE_STEP_MAP_CR_SPACE); /* successful __sx_map_cr_space_area() */
        }
    }

    /* get secured FW data if we don't have it already (and if we're running in secured FW) */
    if (!SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_INIT_BOARD)) {
        if (priv->dev_specific_cb.prepare_dev_for_secured_dump_cb) {
            if (!sx_core_fw_is_ready(dev)) {
                sxd_log_pci_notice(pdev, "cannot prepare device for secured dump!\n");
            } else {
                sxd_log_pci_notice(pdev, "preparing device for secured dump\n");
                err = priv->dev_specific_cb.prepare_dev_for_secured_dump_cb(priv);
                if (err) {
                    sxd_log_pci_err(pdev, "failed to prepare device for secured dump (err=%d)\n", err);
                }
            }
        }
    }

    if (saved_params) { /*During PCI restart driver should fail and return the original error */
        return orig_err;
    }

    return 0; /* must return 0, so driver will stay loaded and device is still usable */

cq_threads_failed:
dpt_init_pci_failed:
    sx_dbg_dump_proc_fs_dev_deinit(dev);
    sx_internal_log_dev_deinit(dev);

add_char_dev_failed:
    sx_dev_db_remove_device(dev);

dev_db_add_failed:
    pci_set_drvdata(pdev, NULL);

set_dma_mask_failed:
    pci_release_region(pdev, 0);

pci_req_region_failed:
invalid_bar0:
    pci_disable_device(pdev);
pci_enable_failed:
    sx_core_remove_one(priv, false, &fi);

out:
    return err;
}

/* The purpose of keep_listeners is to let the function know whether to
 * deallocate the original listeners database or not. This flag is used in
 * the PCI restart flow.
 */
static void __pci_remove_common(struct pci_dev *pdev, struct pci_restart_saved_params *saved_params)
{
    struct sx_priv            *priv;
    struct sx_dev             *dev;
    fw_dump_completion_state_t cr_dump_state = FW_DUMP_COMPLETION_STATE_IDLE;
    int                        i, err = 0;
    struct sx_flow_info        fi = { .in_pci_restart = false};

    /* at this point 'pci_restart_lock' must be acquired (write lock)! */
    BUG_ON(!rwsem_is_locked(&sx_glb.pci_restart_lock));

    __probe_params.total_removes++;

    dev = pci_get_drvdata(pdev);
    if (!dev) {
        return;
    }

    priv = sx_priv(dev);

    if (saved_params) {
        fi.in_pci_restart = true;
    }
    if (sx_core_fw_is_faulty(dev)) {
        __remove_dummy_device(priv);
        return;
    }
/* For Palladium skip sysfs tree creation and module event registration */
#if !defined(PD_BU)
    sx_module_sysfs_unregister_module_event_handler(dev);
#endif

    /* must be called before any CQ/DQ shutdown so there will be no race with the
     * asynchronous health-check mechanism
     */
    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_INIT_HEALTH_CHECK)) {
        sx_health_check_dev_disable(dev);
    }

    /* Mark all open file descriptors as closing to prevent new ioctl operations.
     * This must be called BEFORE the device is removed from the database.
     * For PCI restart (saved_params != NULL), skip this as FDs will be reused.
     */
    sx_core_mark_all_files_closing(dev, (saved_params != NULL));

    pci_set_drvdata(pdev, NULL);

    if (priv->kobj_add_done) {
        /* udev event for system management purpose (only if chip was reset enable or warm boot) */
        sx_send_udev_event(pdev, priv, KOBJ_REMOVE, "Unregistering from PCI");
    }

    if (sx_core_sysfs_dev_is_enabled(dev)) { /* PCI-Restart/ISSU might have already disabled it */
        sx_core_sysfs_dev_disable(dev, "PCI-Remove-Common");
    }

    if (priv->registered) {
        sx_core_unregister_device(dev, &fi);
        priv->registered = false;
    }

    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_SET_ASYNC_QUEUES)) {
        /* ######################################################### */
        /* ########## Until now there was no access to FW ########## */

        /* 1. Disable RDQ usage monitor before disable SWID.
         * 2. Disable RDQ usage before setting 'global_flushing' to 'true' because if sx_QUERY_CQ()
         *    (that is called from the periodic handler in a work-queue) times out, it will
         *    trigger emergency reset while other command-interface calls in current context will
         *    also trigger it - causing a real mess.
         */
        sx_core_rdq_usage_monitor_deinit(dev);
    }

    priv->global_flushing = true; /* from here, emergency reset will be triggered for every FW timeout */

    /* kill long-cmd (if we see that FW is stuck on it) */
    err = sx_core_cr_dump_long_cmd_get(dev->device_id, &cr_dump_state);
    if (err) {
        sxd_log_warning(dev,
                        "device deinit: failed to get cr_dump state (err=%d). ignoring\n",
                        err);
        err = 0;
    }

    /* if FW is stuck on long-command, kill this command! */
    if (cr_dump_state == FW_DUMP_COMPLETION_STATE_REQUEST_SENT) {
        err = sx_core_cr_dump_long_cmd_set(dev->device_id, FW_DUMP_COMPLETION_STATE_DONE);
        if (err) {
            sxd_log_warning(dev,
                            "device deinit: failed to set cr_dump state (err=%d). ignoring\n",
                            err);
            err = 0;
        }
    }

    /*
     * Disconnect all trap groups before flush and destroy
     */
    __sx_core_disconnect_all_trap_groups(dev);

    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_INIT_EMAD)) {
        sx_emad_dev_deinit(dev);
    }

    for (i = 0; i < NUMBER_OF_SWIDS; i++) {
        if (sx_bitmap_test(&priv->swid_bitmap, i)) {
            sx_disable_swid(dev, i);
        }
    }

    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_SET_ASYNC_QUEUES)) {
        __sx_setup_sx_cleanup(dev);
    }

    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_MAP_CR_SPACE)) {
        __sx_cr_space_cleanup(dev);
    }

    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_MAP_DOORBELL)) {
        __sx_doorbell_cleanup(dev);
    }

    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_INIT_DBG)) {
        sx_dbg_deinit(dev, NULL);
    }

    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_INIT_PERF_COUNTERS)) {
        sx_asic_perf_counter_deinit(dev);
    }

    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_INIT_SYNCE)) {
        sx_core_synce_deinit(dev);
    }

    /* must be called before priv->generic_wq is destroyed */
    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_INIT_BULK_COUNTERS)) {
        sx_core_bulk_counters_db_deinit(priv);
    }

    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_INIT_BER_MONITOR)) {
        sx_core_ber_monitor_dev_deinit(dev);
    }

    sx_core_ptp_dev_cleanup(priv, PTP_CLEANUP_CONFIG_PTP1_MSG_TYPES);

    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_INIT_CR_DUMP)) {
        sx_core_cr_dump_deinit(priv);
    }

    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_INIT_CLOCK)) {
        sx_core_clock_dev_deinit(priv);
    }

    if (priv->flags & SX_FLAG_MSI_X) {
        pci_disable_msix(dev->pdev);
    }

    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_INIT_BOARD)) {
        __sx_close_board(dev);
    }

    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_CMD_POOL_CREATE)) {
        sx_cmd_pool_destroy(dev);
    }

    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_CMD_INIT_PCI)) {
        sx_cmd_unmap(dev);
    }

    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_PCI_SETTINGS)) {
        pci_release_region(pdev, 0);
        pci_disable_device(pdev);
    }

    /* should be as late as possible because it holds all workqueues */
    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_INIT_HEALTH_CHECK)) {
        sx_health_check_destroy_monitored_workqueue(dev, priv->generic_wq);
        sx_health_check_dev_deinit(dev);
    }

    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_INIT_SYSFS)) {
        sx_sysfs_asic_remove(dev);
    }

    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_ADD_DEV)) {
        sx_dev_db_remove_device(dev);
    }

    if (SX_PROBE_STEP_CHECK(priv, SX_PROBE_STEP_CREATE_CQ_THREADS)) {
        __delete_cq_threads(priv);
    }

    if (saved_params) {
        fi.in_pci_restart = true;
        saved_params->emergency_reset_done = sx_emergency_reset_done(dev);
        if (saved_params->emergency_reset_done) {
            saved_params->reset_duration_msec = priv->reset_info.duration_msec;
        }
    }

    sx_internal_log_dev_deinit(dev);
    sx_dbg_dump_proc_fs_dev_deinit(dev);

    sx_core_remove_one(priv, saved_params != NULL, &fi);
}

#ifndef SXD_KERNEL_DISABLE_PCI_DRV_SHUTDOWN
static void __pci_shutdown_common(struct pci_dev *pdev)
{
    struct sx_dev *dev = NULL;

    sxd_log_debug(SXD_DEV_NONE, "Shutdown PCI driver\n");

    down_write(&sx_glb.pci_restart_lock);

    dev = pci_get_drvdata(pdev);

    if (!dev) {
        sxd_log_err(SXD_DEV_NONE, "PCI shutdown - could not get SX device!\n");
    } else {
        __pci_remove_core(pdev);
    }

    up_write(&sx_glb.pci_restart_lock);
}
#endif

static int __pci_probe_core(struct pci_dev *pdev, const struct pci_device_id *id)
{
    sxd_log_notice(SXD_DEV_NONE, "found a PCI core device\n");
    return __pci_probe_common(pdev, NULL, NULL);
}

static void __pci_remove_core(struct pci_dev *pdev)
{
    sxd_log_notice(SXD_DEV_NONE, "remove a PCI core device\n");
    __pci_remove_common(pdev, NULL);
    sx_glb.pci_drivers_in_use &= ~PCI_DRIVER_F_SX_DRIVER;
}

#ifndef SXD_KERNEL_DISABLE_PCI_DRV_SHUTDOWN
static void __pci_shutdown_core(struct pci_dev *pdev)
{
    sxd_log_notice(SXD_DEV_NONE, "shutdown a PCI core device\n");
    __pci_shutdown_common(pdev);
}
#endif

static int __pci_probe_oob(struct pci_dev *pdev, const struct pci_device_id *id)
{
    sxd_log_notice(SXD_DEV_NONE, "found a PCI OOB backbone device\n");
    return __pci_probe_common(pdev, &sx_glb.oob_backbone_dev, NULL);
}

static void __pci_remove_oob(struct pci_dev *pdev)
{
    sxd_log_notice(SXD_DEV_NONE, "remove a PCI OOB backbone device\n");
    __pci_remove_common(pdev, NULL);
    sx_glb.pci_drivers_in_use &= ~PCI_DRIVER_F_SX_OOB_DRIVER;
}

#ifndef SXD_KERNEL_DISABLE_PCI_DRV_SHUTDOWN
static void __pci_shutdown_oob(struct pci_dev *pdev)
{
    sxd_log_notice(SXD_DEV_NONE, "shutdown a PCI OOB backbone device\n");
    __pci_shutdown_common(pdev);
}
#endif

static int __pci_register_common(struct pci_driver *driver,
                                 bool               do_reset,
                                 u32               *total_probes,
                                 u32               *successful_probes,
                                 u32                driver_flag)
{
    bool unreg_driver = false;
    int  ret = 0;

    /* at this point 'pci_restart_lock' must be acquired (write lock)! */
    BUG_ON(!rwsem_is_locked(&sx_glb.pci_restart_lock));

    memset(&__probe_params, 0, sizeof(__probe_params));
    __probe_params.do_reset = do_reset;
    __probe_params.is_oob = (driver_flag == PCI_DRIVER_F_SX_OOB_DRIVER) ? true : false;


    ret = pci_register_driver(driver);
    if (ret < 0) {
        sxd_log_err(SXD_DEV_NONE, "pci_register_driver() failed (err=%d)\n", ret);
        goto out;
    }

    if (__probe_params.total_probes == 0) {
        sxd_log_err(SXD_DEV_NONE, "pci_register_driver() returned successfully but probe function not called\n");
        unreg_driver = true;
        goto out;
    }

    if (__probe_params.total_probes != __probe_params.successful_probes + __probe_params.fw_boot_failures) {
        sxd_log_err(SXD_DEV_NONE, "pci_register_driver() returned successfully but probe function failed\n");
        /* probe failure that returned 0 from pci_register_driver() needs to keep the driver up for CR-Space dump */
    }

    sx_glb.pci_drivers_in_use |= driver_flag;

out:
    if (unreg_driver) {
        pci_unregister_driver(driver);
    }

    *total_probes = __probe_params.total_probes;
    *successful_probes = __probe_params.successful_probes;
    return ret;
}

static void __pci_unregister_common(struct pci_driver *driver, u32 *total_removes, u32 driver_flag)
{
    /* at this point 'pci_restart_lock' must be acquired (write lock)! */
    BUG_ON(!rwsem_is_locked(&sx_glb.pci_restart_lock));

    memset(&__probe_params, 0, sizeof(__probe_params));

    if (sx_glb.pci_drivers_in_use & driver_flag) {
        pci_unregister_driver(driver);
        *total_removes = __probe_params.total_removes;
    }
}

static void __sx_core_disconnect_all_trap_groups(struct sx_dev *dev)
{
    int err;

    if (dev == NULL) {
        sxd_log_err(SXD_DEV_NONE, "sx_core_disconnect_all_trap_groups: dev is NULL \n");
        return;
    }

    err = __sx_core_dev_specific_cb_get_reference(dev);
    if (err) {
        sxd_log_err(dev, " dev_specific_cb_get_ref failed. Failed disconnect trap groups.\n");
        return;
    }

    if (sx_priv(dev)->dev_specific_cb.sx_disconnect_all_trap_groups_cb != NULL) {
        sx_priv(dev)->dev_specific_cb.sx_disconnect_all_trap_groups_cb(dev);
    }
    __sx_core_dev_specific_cb_release_reference(dev);
}

/****************************************************************************************
 * NON-STATIC functions
 ***************************************************************************************/

void sx_send_udev_event(struct pci_dev *pdev, struct sx_priv *priv, enum kobject_action action, const char *origin)
{
    char        sdk_env[] = "SX_CORE_EVENT=1";
    char       *envp_ext[] = { sdk_env, NULL };
    const char *action_str = NULL;

    switch (action) {
    case KOBJ_ADD:
        if (!priv) {
            sxd_log_err(SXD_DEV_NONE, "Device private data is NULL, will not send UDEV KOBJ_ADD event\n");
            return;
        }

        priv->kobj_add_done = true;
        action_str = "KOBJ_ADD";
        break;

    case KOBJ_REMOVE:
        if (!priv) {
            sxd_log_err(SXD_DEV_NONE, "Device private data is NULL, will not send UDEV KOBJ_REMOVE event\n");
            return;
        }

        priv->kobj_add_done = false;
        action_str = "KOBJ_REMOVE";
        break;

    default:
        action_str = "N/A";
        break;
    }

    if (!origin) {
        origin = "N/A";
    }

    sxd_log_notice(SXD_PRIV_DEV(priv),
                   "sending UDEV event from sx-core driver [bus=%s, action=%d (%s), origin='%s']\n",
                   dev_name(&pdev->dev),
                   action,
                   action_str,
                   origin);

    kobject_uevent_env(&pdev->dev.kobj, action, envp_ext);
}

static int __cr_mode_get_mailboxes(struct sx_dev *dev)
{
    int ret;

    if (sx_i2c_mode()) {
        /* sx_dpt_init_default_dev() eventually calls get_local_mbox() of the I2C driver
         * and it will initialize the mailboxes in DPT and in the I2C driver itself */
        ret = sx_dpt_init_default_dev(dev);
    } else { /* MST mode */
        ret = sx_QUERY_FW_2(dev, dev->device_id, HCR2);
    }

    if (ret) {
        sxd_log_err(dev, "CR-Mode: Failed to get mailboxes (err=%d)\n", ret);
    }

    return ret;
}

const char * sx_get_chip_type_str(sxd_chip_types_t chip_type)
{
    const char *ret;

    switch (chip_type) {
    case SXD_CHIP_TYPE_SWITCHX_A2:
        ret = "SwitchX-A2";
        break;

    case SXD_CHIP_TYPE_SWITCHX_A1:
        ret = "SwitchX-A1";
        break;

    case SXD_CHIP_TYPE_SWITCHX_A0:
        ret = "SwitchX-A0";
        break;

    case SXD_CHIP_TYPE_SWITCH_IB:
        ret = "Switch-IB";
        break;

    case SXD_CHIP_TYPE_SPECTRUM:
        ret = "Spectrum-1";
        break;

    case SXD_CHIP_TYPE_SWITCH_IB2:
        ret = "Switch-IB/2";
        break;

    case SXD_CHIP_TYPE_SPECTRUM_A1:
        ret = "Spectrum-A1";
        break;

    case SXD_CHIP_TYPE_SPECTRUM2:
        ret = "Spectrum-2";
        break;

    case SXD_CHIP_TYPE_QUANTUM:
        ret = "Quantum";
        break;

    case SXD_CHIP_TYPE_SPECTRUM3:
        ret = "Spectrum-3";
        break;

    case SXD_CHIP_TYPE_QUANTUM2:
        ret = "Quantum-2";
        break;

    case SXD_CHIP_TYPE_SPECTRUM4:
        ret = "Spectrum-4";
        break;

    case SXD_CHIP_TYPE_SPECTRUM5:
        ret = "Spectrum-5";
        break;

    case SXD_CHIP_TYPE_SPECTRUM6:
        ret = "Spectrum-6";
        break;

    case SXD_CHIP_TYPE_SPECTRUM7:
        ret = "Spectrum-7";
        break;

    case SXD_CHIP_TYPE_QUANTUM3:
        ret = "Quantum-3";
        break;

    case SXD_CHIP_TYPE_QUANTUM4:
        ret = "Quantum-4";
        break;

    case SXD_CHIP_TYPE_QUANTUM5:
        ret = "Quantum-5";
        break;

    default:
        ret = "N/A";
    }

    return ret;
}

int sx_dev_init_cr_device(bool reset_chip)
{
    struct sx_priv     *priv = NULL;
    struct sx_dev      *dev = NULL;
    int                 ret = 0;
    struct sx_flow_info fi = {.in_pci_restart = false};

    sxd_log_info(SXD_DEV_NONE, "Running in CR-Space configuration, creating device\n");

    ret = sx_core_init_one(&priv, NULL);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "Failed to create CR-Space device (err=%d)\n", ret);
        goto out;
    }

    INIT_LIST_HEAD(&priv->cr_mode.tx_queue);
    spin_lock_init(&priv->cr_mode.tx_queue_lock);
    dev = &priv->dev;

    ret = sx_dev_db_add_device(dev); /* also allocated the device ID */
    if (ret) {
        sxd_log_err(dev, "Failed to add CR-Space device to database (err=%d)\n", ret);
        goto out_remove_one;
    }

    ret = __cr_mode_get_mailboxes(dev);
    if (ret) {
        sxd_log_err(dev, "Failed to get mailboxes before reset (err=%d)\n", ret);
        goto out_remove_from_db;
    }

    ret = __sx_init_chip_callbacks(dev);
    if (ret) {
        sxd_log_err(dev,
                    "Failed to initialize common board information on CR-Space device (err=%d)\n",
                    ret);
        goto out_remove_from_db;
    }

    ret = sx_reset(dev, (reset_chip) ? 1 : 0);
    if (ret) {
        sxd_log_err(dev, "Failed to reset CR-Space device (err=%d)\n", ret);
        goto out_remove_from_db;
    }

    ret = sx_add_char_dev(&sx_glb.cdev, 1 /* Use minor 1 for char device */);
    if (ret) {
        sxd_log_err(dev, "Failed to add character device (err=%d). Aborting.\n", ret);
        goto out_remove_from_db;
    }

    ret = sx_core_register_device(dev, &fi);
    if (ret) {
        sxd_log_err(dev, "Failed to register the device, aborting.\n");
        goto out_remove_from_db;
    }

    priv->registered = true;

    /*
     * after reset, we should ask for the mailboxes again
     * (maybe there is a new FW version loaded with different mailbox layout)
     */
    ret = __cr_mode_get_mailboxes(dev);
    if (ret) {
        sxd_log_err(dev, "Failed to get mailboxes after reset (err=%d)\n", ret);
        goto out_remove_from_db;
    }

    if (reset_chip) {
        if ((sx_glb.sx_dpt.dpt_info[dev->device_id].in_mb_size[HCR2] < 0x800) ||
            (sx_glb.sx_dpt.dpt_info[dev->device_id].out_mb_size[HCR2] < 0x800)) {
            sxd_log_err(dev,
                        "HCR2 (Dev %d): In CR-Space mode, mailboxes must be at least 0x800 in size!\n",
                        dev->device_id);
            sxd_log_err(dev, "If you just burned a new FW image, try to reload the I2C driver\n");
            ret = -ENOBUFS;
            goto out_remove_from_db;
        }
    }

    ret = sx_cr_polling_thread_init(dev);
    if (ret) {
        sxd_log_err(dev, "Failed to initialize CR-Space polling thread (err=%d)\n", ret);
        goto out_remove_from_db;
    }

    return 0;

out_remove_from_db:
    sx_dev_db_remove_device(dev);

out_remove_one:
    sx_core_remove_one(priv, false, &fi);

out:
    return ret;
}

void sx_dev_deinit_cr_device(void)
{
    struct sx_priv     *priv = NULL;
    struct sx_dev      *dev = NULL;
    struct sx_flow_info fi = {.in_pci_restart = false};

    dev = sx_dev_db_get_default_device();
    if (!dev) {
        sxd_log_err(SXD_DEV_NONE, "Did not find CR-Space device\n");
        return;
    }

    priv = sx_priv(dev);

    sx_cr_polling_thread_deinit(dev);

    if (priv->registered) {
        sx_core_unregister_device(dev, &fi);
        priv->registered = false;
    }

    sx_dev_db_remove_device(dev);
    sx_core_remove_one(priv, false, &fi);
}

int sx_dev_init_core_pci(bool do_reset, u32 *total_probes, u32 *successful_probes)
{
    int ret = 0;

    down_write(&sx_glb.pci_restart_lock);
    ret = __pci_register_common(&__sx_driver,
                                do_reset,
                                total_probes,
                                successful_probes,
                                PCI_DRIVER_F_SX_DRIVER);
    up_write(&sx_glb.pci_restart_lock);
    return ret;
}

void sx_dev_deinit_core_pci(u32 *total_removes)
{
    down_write(&sx_glb.pci_restart_lock);
    __pci_unregister_common(&__sx_driver, total_removes, PCI_DRIVER_F_SX_DRIVER);
    up_write(&sx_glb.pci_restart_lock);
}

int sx_dev_init_oob_pci(void)
{
    u32 total = 0, ok = 0;
    int err = 0;

    sxd_log_info(SXD_DEV_NONE, "OOB: initializing SwitchX device on PCI\n");

    down_write(&sx_glb.pci_restart_lock);

    err = __pci_register_common(&__sx_oob_driver,
                                true,
                                &total,
                                &ok,
                                PCI_DRIVER_F_SX_OOB_DRIVER);

    up_write(&sx_glb.pci_restart_lock);

    if (err) {
        sxd_log_err(SXD_DEV_NONE, "OOB: pci_register_driver() failed (err=%d)\n", err);
        goto out;
    }

    if (total == 0) {
        sxd_log_err(SXD_DEV_NONE, "OOB: SwitchX device was not found!\n");
        err = -ENODEV;
        goto out;
    }

    sxd_log_info(SXD_DEV_NONE, "OOB: SwitchX device was successfully initialized on PCI\n");

out:
    return err;
}


void sx_dev_deinit_oob_pci(void)
{
    u32 dummy = 0;

    sxd_log_info(SXD_DEV_NONE, "OOB: deinitializing SwitchX device on PCI\n");

    down_write(&sx_glb.pci_restart_lock);
    __pci_unregister_common(&__sx_oob_driver, &dummy, PCI_DRIVER_F_SX_OOB_DRIVER);
    up_write(&sx_glb.pci_restart_lock);
}

int sx_restart_one_pci(sxd_dev_id_t dev_id, bool do_reset)
{
    struct sx_dev                  *dev, *new_dev = NULL;
    struct sx_priv                 *priv = NULL;
    struct pci_dev                 *pdev = NULL;
    struct pci_restart_saved_params saved_params;
    int                             err = 0;

    sxd_log_notice(SXD_DEV_NONE, "RESTART PCI DEVICE %u (reset_asic=%s)\n", dev_id, ((do_reset) ? "yes" : "no"));

    down_write(&sx_glb.pci_restart_lock);

    dev = sx_dev_db_get_dev_by_id(dev_id);
    if (!dev) {
        sxd_log_err(SXD_DEV_NONE, "device not found for dev_id=%u\n", dev_id);
        err = -ENODEV;
        goto out;
    }

    if (!dev->pdev) {
        sxd_log_err(dev, "sx_restart_one_pci error: dev or pdev are NULL, exit\n");
        err = -ENODEV;
        goto out;
    }

    priv = sx_priv(dev);

    /* save old device's information for the new device */
    memset(&saved_params, 0, sizeof(saved_params));
    saved_params.dev_id = dev->device_id;
    saved_params.listeners = priv->listeners_and_rf_db.info;
    saved_params.mmap_info = priv->mmap_db.info;
    memcpy(&saved_params.dev_info_set, &priv->dev_info.dev_info_set, sizeof(saved_params.dev_info_set));

    /* must keep 'dev->pdev' because 'dev' is about to be deallocated and then allocated again */
    pdev = dev->pdev;

    /* enable chip reset from sx_restart_one_pci even
     * if driver was loaded with __perform_chip_reset = 0
     * for example in case of WARM boot*/


    memset(&__probe_params, 0, sizeof(__probe_params));
    __probe_params.do_reset = do_reset;

    priv->reset_info.in_pci_restart = true; /* marking old device */
    __pci_remove_common(pdev, &saved_params);
    err = __pci_probe_common(pdev, &new_dev, &saved_params);
    if (err) {
        sxd_log_err(SXD_DEV_NONE, "__pci_probe_common failed with err: %d \n", err);
        goto out;
    }

    priv = sx_priv(new_dev);
    priv->reset_info.in_pci_restart = false; /* marking new device */

out:
    up_write(&sx_glb.pci_restart_lock);
    return err;
}

/* this function is used only from /proc/mlx_sx/sx_core interface */
int sx_add_one_pci(struct pci_dev *pdev, bool do_reset)
{
    int err = 0;

    down_write(&sx_glb.pci_restart_lock);
    memset(&__probe_params, 0, sizeof(__probe_params));
    __probe_params.do_reset = do_reset;
    err = __pci_probe_common(pdev, NULL, NULL);
    up_write(&sx_glb.pci_restart_lock);

    return err;
}

/* this function is used only from /proc/mlx_sx/sx_core interface */
void sx_remove_one_pci(struct pci_dev *pdev)
{
    down_write(&sx_glb.pci_restart_lock);
    __pci_remove_common(pdev, NULL);
    up_write(&sx_glb.pci_restart_lock);
}

/* this function is used only from net-namespace destructor callback (in sx_netdev) and only on multi-ASIC system */
void sx_core_delete_pci_dev(struct sx_dev *dev)
{
    if (!dev->pdev) {
        sxd_log_err(dev, "Device is not PCI\n");
        return;
    }

    sx_remove_one_pci(dev->pdev);
}
EXPORT_SYMBOL(sx_core_delete_pci_dev);

void sx_core_reset_dm_err_counters(struct sx_priv *priv)
{
    atomic64_set(&priv->wjh_info.dm_err_counters.from_monitor_rdq, 0);
    atomic64_set(&priv->wjh_info.dm_err_counters.from_sw_queue, 0);
    atomic64_set(&priv->wjh_info.dm_err_counters.from_user_channel_dm, 0);
    atomic64_set(&priv->wjh_info.dm_err_counters.dm_disabled, 0);
    atomic64_set(&priv->wjh_info.dm_err_counters.dm_inactive, 0);
    atomic64_set(&priv->wjh_info.dm_err_counters.skb_clone_failed, 0);
    atomic64_set(&priv->wjh_info.dm_err_counters.skb_handling_failed, 0);
    atomic64_set(&priv->wjh_info.dm_err_counters.in_dm_hw_report_entry_point, 0);
    atomic64_set(&priv->wjh_info.dm_err_counters.invalid_trap, 0);
    atomic64_set(&priv->wjh_info.dm_err_counters.no_hw_report_cb, 0);
}

int sx_core_init_one(struct sx_priv **sx_priv, struct pci_restart_saved_params *saved_params)
{
    struct sx_priv *priv;
    struct sx_dev  *dev;
    int             i, j, err;

    priv = vmalloc(sizeof(struct sx_priv));
    if (!priv) {
        sxd_log_err(SXD_DEV_NONE, "Device struct alloc failed, aborting.\n");
        err = -ENOMEM;
        goto out;
    }
    memset(priv, 0, sizeof *priv);
    dev = &priv->dev;

    /* we start in good will. this will be set to false if FW's system status is not ready */
    priv->global_system_status_ready = true;

    /* Initialize TEC capability cache */
    priv->tec_support = false;

    /* Initialize local params with global values for Multi-ASIC systems */
    if (sx_core_has_pci_multi_dev()) {
        priv->local_params.rx_debug = rx_debug;
        priv->local_params.tx_debug = tx_debug;
        priv->local_params.chip_info_type = chip_info_type;
        priv->local_params.chip_info_revision = chip_info_revision;
        priv->local_params.enable_keep_packet_crc = enable_keep_packet_crc;
        priv->local_params.inited = true;
    }

    mutex_init(&priv->listeners_and_rf_db.lock);
    spin_lock_init(&priv->mmap_db.lock);

    if (saved_params) { /* PCI_RESTART flow */
        priv->reset_info.in_pci_restart = true; /* marking new device */
        priv->listeners_and_rf_db.info = saved_params->listeners;
        priv->mmap_db.info = saved_params->mmap_info;
        if (saved_params->emergency_reset_done) {
            priv->reset_info.last_chip_reset_type = SX_RESET_TYPE_EMERGENCY;
            priv->reset_info.duration_msec = saved_params->reset_duration_msec;
        }
        memcpy(&priv->dev_info.dev_info_set, &saved_params->dev_info_set, sizeof(priv->dev_info.dev_info_set));
    } else {
        priv->listeners_and_rf_db.info = kmalloc(sizeof(struct listeners_and_rf_info), GFP_KERNEL);
        if (!priv->listeners_and_rf_db.info) {
            sxd_log_err(dev, "Device listeners database allocation failed. aborting.\n");
            err = -ENOMEM;
            goto out_free_priv;
        }

        for (i = 0; i < NUM_HW_SYNDROMES + 1; i++) {
            INIT_LIST_HEAD(&priv->listeners_and_rf_db.info->per_synd_list[i]);
        }

        priv->mmap_db.info = kmalloc(sizeof(struct sx_mmap_info), GFP_KERNEL);
        if (!priv->mmap_db.info) {
            sxd_log_err(dev, "Device mmap database allocation failed. aborting.\n");
            err = -ENOMEM;
            goto out_free_priv;
        }

        __hash_init(priv->mmap_db.info->hash_tb, SX_MMAP_HASH_TABLE_BUCKETS);
        priv->mmap_db.info->seq = 0;
    }

    /* default pvid for all ports is 1 */
    for (i = 0; i < MAX_SYSPORT_NUM; i++) {
        if (i < MAX_LAG_NUM) {
            priv->pvid_lag_db[i] = 1;
        }

        priv->pvid_sysport_db[i] = 1;
    }

    /* initialize lag_filter_db with invalid value */
    for (i = 0; i < NUM_HW_SYNDROMES; i++) {
        for (j = 0; j < MAX_LAG_PORTS_IN_FILTER; j++) {
            priv->lag_filter_db[i][j] = LAG_ID_INVALID;
        }
    }

    /* initialize fid to hw_fid mapping with invalid value */
    for (i = 0; i < MAX_FIDS_NUM; i++) {
        priv->fid_to_hwfid[i] = INVALID_HW_FID_ID;
    }

    /* initialize rif to hw_fid mapping with invalid value */
    for (i = 0; i < MAX_RIFS_NUM; i++) {
        priv->rif_id_to_hwfid[i] = INVALID_HW_FID_ID;
        memset(&priv->rif_data[i], 0, sizeof(priv->rif_data[0]));
    }

    for (i = 0; i < MAX_SLOT_NUM; i++) {
        for (j = 0; j < MAX_MODULE_NUM; j++) {
            priv->module_data[i][j].power_mode_policy = SX_MODULE_POWER_MODE_POLICY_HIGH;
        }
    }

    if (sx_cr_mode()) {
        /* in CR mode we're polling for port state only on active ports:
         * active port: swid != 255
         * inactive port: swid = 255
         *
         * by default, keep all local ports to swid 0 because IB does not always
         * use this mapping. only in CR-Mode we explicitly inactivate all ports by default.
         */
        for (i = 0; i < MAX_PHYPORT_NUM + 1; i++) {
            priv->local_to_swid_db[i] = 255; /* invalid swid */
        }
    }

    mutex_init(&priv->module_access_mutex);

    for (i = 0; i < MAX_MONITOR_RDQ_NUM; i++) {
        priv->wjh_info.monitor_rdqs_arr[i] = RDQ_INVALID_ID;
    }
    priv->wjh_info.buffer_drop_trap_id = NUM_HW_SYNDROMES + 1;
    sx_core_reset_dm_err_counters(priv);

    err = sx_cmd_init(dev);
    if (err) {
        sx_err(dev, "Failed initializing command interface, aborting.\n");
        goto out_free_priv;
    }

    init_rwsem(&priv->sysfs_access_info.rw_lock);
    spin_lock_init(&priv->profile.profile_lock);
    priv->profile.pci_profile_set = false;
    priv->profile.dev_profile_set = false;
    priv->profile.first_ib_swid = true;
    priv->dev_sw_rst_flow = false;
    spin_lock_init(&priv->db_lock);
    spin_lock_init(&priv->accuflow.counters.lock);
    spin_lock_init(&priv->accuflow.debug.lock);
    INIT_LIST_HEAD(&priv->ctx_list);
    INIT_LIST_HEAD(&priv->dev_list);
    atomic_set(&priv->dev_specific_cb_refcnt, 0);
    init_waitqueue_head(&priv->dev_specific_cb_not_in_use);
    sema_init(&priv->debug_only.tx_packets_sem, SX_MAX_CPU_PORT_LOOPBACK_TX_PACKET_IN_FLIGHT);
    INIT_LIST_HEAD(&priv->cq_threads.tasklet_resched_list);
    INIT_LIST_HEAD(&priv->cq_threads.sp_list);
    spin_lock_init(&priv->cq_threads.lock);
    sx_cq_init_sp_work(priv);
    priv->cq_threads.paused_cq_bitmap = 0;

    err = sx_bitmap_init(&priv->swid_bitmap, NUMBER_OF_SWIDS);
    if (err) {
        sx_err(dev, "Failed to initialize SWIDs bitmap, aborting.\n");
        goto out_free_priv;
    }

    __set_default_capabilities(dev);
    memset(&priv->stats, 0, sizeof(priv->stats));

    if (sx_priv != NULL) {
        *sx_priv = priv;
    }

    return 0;

out_free_priv:
    if (priv->listeners_and_rf_db.info) {
        kfree(priv->listeners_and_rf_db.info);
        priv->listeners_and_rf_db.info = NULL;
    }

    if (priv->mmap_db.info) {
        kfree(priv->mmap_db.info);
        priv->mmap_db.info = NULL;
    }

    vfree(priv);

out:
    return err;
}

void sx_core_remove_one(struct sx_priv *priv, bool pci_restart, struct sx_flow_info *fi)
{
    struct sx_dev *dev = &priv->dev;

    if (priv->registered) {
        sx_core_unregister_device(dev, fi);
        priv->registered = false;
    }

    sx_dpt_remove_dev(dev->device_id);
    sx_core_dev_deinit_switchx_cb(dev);

    if (priv->listeners_and_rf_db.info && !pci_restart) {
        sx_core_listeners_cleanup(dev);
        kfree(priv->listeners_and_rf_db.info);
        priv->listeners_and_rf_db.info = NULL;
    }

    if (priv->mmap_db.info && !pci_restart) {
        kfree(priv->mmap_db.info);
        priv->mmap_db.info = NULL;
    }

    vfree(priv);
}

static struct sx_priv * __create_dummy_device(int chip_type, struct pci_dev *pdev)
{
    struct sx_priv     *priv = NULL;
    int                 ret = 0;
    struct sx_flow_info fi = {.in_pci_restart = false};

    if (chip_type == 0) {
        sxd_log_err(SXD_DEV_NONE, "Chip type is not defined for device.\n");
        goto out;
    }

    ret = sx_core_init_one(&priv, NULL);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "Couldn't initialize the device. Aborting...\n");
        goto out;
    }

    ret = sx_core_dev_init_switchx_cb(&priv->dev, chip_type, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "callback dev init failed for device (%u)\n",
                    priv->dev.device_id);
        goto out_remove_one;
    }

    ret = sx_dev_db_add_device(&priv->dev); /* also allocated the device ID */
    if (ret) {
        goto out_remove_one;
    }

    ret = sx_dbg_dump_proc_fs_dev_init(&priv->dev);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "Failed to initialize debug dump proc fs for dummy device, aborting.\n");
        goto out_remove_device;
    }

    ret = sx_dpt_init_dev_pci(&priv->dev);
    if (ret) {
        goto out_dump_proc_fs_dev_deinit;
    }

    ret = sx_health_check_dev_init(&priv->dev);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "Failed to initialize health check for dummy device, aborting.\n");
        goto out_dump_proc_fs_dev_deinit;
    }

    ret = sx_core_register_device(&priv->dev, &fi);
    if (ret) {
        sxd_log_err(SXD_PRIV_DEV(priv), "Failed to register the device, aborting.\n");
        goto out_health_check_dev_deinit;
    }

    priv->registered = true;

    if (pdev) {
        priv->dev.pdev = pdev;
        pci_set_drvdata(pdev, &priv->dev);
    }

    return priv;

out_health_check_dev_deinit:
    sx_health_check_dev_deinit(&priv->dev);

out_dump_proc_fs_dev_deinit:
    sx_dbg_dump_proc_fs_dev_deinit(&priv->dev);

out_remove_device:
    sx_dev_db_remove_device(&priv->dev);

out_remove_one:
    sx_core_remove_one(priv, false, &fi);

out:
    return NULL;
}

static void __remove_dummy_device(struct sx_priv *priv)
{
    struct sx_flow_info fi = {.in_pci_restart = false};

    if (priv->dev.pdev) {
        pci_set_drvdata(priv->dev.pdev, NULL);
        priv->dev.pdev = NULL;
    }

    if (priv->registered) {
        sx_core_unregister_device(&priv->dev, &fi);
        priv->registered = false;
    }

    sx_health_check_dev_deinit(&priv->dev);
    sx_dbg_dump_proc_fs_dev_deinit(&priv->dev);
    sx_dev_db_remove_device(&priv->dev);
    sx_core_remove_one(priv, false, &fi);
}

/*
 * when does the driver create a fake device:
 * 1. when a problem occurs and there is no PCI device detected on the bus.
 * 2. on OOB system.
 */
int sx_core_create_fake_device(struct sx_priv **priv_pp)
{
    struct sx_priv *priv = NULL;

    sxd_log_info(SXD_DEV_NONE, "creating fake device\n");

    if (g_chip_type == 0) {
        sxd_log_err(SXD_DEV_NONE, "Chip type is not defined for device.\n");
        return -EINVAL;
    }

    priv = __create_dummy_device(g_chip_type, NULL);
    if (!priv) {
        return -EFAULT;
    }

    sx_glb.fake_dev = &priv->dev;
    *priv_pp = priv;
    return 0;
}

void sx_core_remove_fake_device(struct sx_priv *priv)
{
    __remove_dummy_device(priv);
    sx_glb.fake_dev = NULL;
}

bool sx_core_fw_is_faulty(struct sx_dev *dev)
{
    return (sx_priv(dev)->dev_info.dev_info_ro.fw_boot_status != SXD_FW_BOOT_STATUS_OK_E);
}

bool sx_core_fw_is_ready(struct sx_dev *dev)
{
    return sx_priv(dev)->global_system_status_ready;
}

/* this function is not exported since it is only called from sx_core */
void sx_core_sysfs_dev_enable(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);

    if (!priv->dev.pdev) {
        sxd_log_info(dev, "sysfs enable is allowed only on a PCI device\n");
        return;
    }

    down_write(&priv->sysfs_access_info.rw_lock);

    /* from this point on, we have exclusive access to the 'sysfs_access_info' structure. */

    /* since writer (sx_core_sysfs_dev_disable/sx_core_sysfs_dev_enable) is only acquiring the
     * lock to change DB and not for the whole duration of the 'disable' operation, we have the
     * 'locked_by_writer' boolean to check if someone has already disabled sysfs. if not, we're not
     * going to enable again but just go out. */
    if (!priv->sysfs_access_info.locked_by_writer) {
        sxd_log_notice(dev,
                       "Tried to enable sysfs access to device %d while it is already enabled\n",
                       dev->device_id);
        goto out;
    }

    sxd_log_notice(dev, "Enabling sysfs access to device %d\n", dev->device_id);

    priv->sysfs_access_info.locker_ts = 0;
    priv->sysfs_access_info.writer_desc[0] = '\0';
    priv->sysfs_access_info.locked_by_writer = false;

out:
    up_write(&priv->sysfs_access_info.rw_lock);
}

/* this function is not exported since it is only called from sx_core */
void sx_core_sysfs_dev_disable(struct sx_dev *dev, const char *locker_name)
{
    struct sx_priv *priv = sx_priv(dev);

    if (!priv->dev.pdev) {
        sxd_log_info(dev, "sysfs disable is allowed only on a PCI device\n");
        return;
    }

    down_write(&priv->sysfs_access_info.rw_lock);

    /* from this point on, we have exclusive access to the 'sysfs_access_info' structure. */

    /* since writer (sx_core_sysfs_dev_disable/sx_core_sysfs_dev_enable) is only acquiring the
     * lock to change DB and not for the whole duration of the 'disable' operation, we have the
     * 'locked_by_writer' boolean to check if someone has already disabled sysfs. if so, we're not
     * going to disable again but just go out. */
    if (priv->sysfs_access_info.locked_by_writer) {
        sxd_log_err(dev,
                    "Tried to disable sysfs access to device %d while it is already disabled "
                    "[old_locker='%s', new_locker='%s']\n",
                    dev->device_id,
                    priv->sysfs_access_info.writer_desc,
                    locker_name);
        goto out;
    }

    sxd_log_notice(dev,
                   "Disabling sysfs access to device %d (locker: %s)\n",
                   dev->device_id,
                   locker_name);

    priv->sysfs_access_info.locker_ts = jiffies;
    priv->sysfs_access_info.locked_by_writer = true;
    atomic_set(&priv->sysfs_access_info.num_of_readers, 0);
    strncpy(priv->sysfs_access_info.writer_desc, locker_name, sizeof(priv->sysfs_access_info.writer_desc) - 1);

out:
    up_write(&priv->sysfs_access_info.rw_lock);
}

bool sx_core_sysfs_dev_is_enabled(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);
    bool            ret = false;

    down_write(&priv->sysfs_access_info.rw_lock);
    ret = !priv->sysfs_access_info.locked_by_writer;
    up_write(&priv->sysfs_access_info.rw_lock);

    return ret;
}

int sx_core_sysfs_dev_hold(struct sx_dev *dev)
{
    struct sx_priv *priv = NULL;
    int             ret = 0;

    if (unlikely(!dev)) {
        ret = -EINVAL;
        goto out;
    }

    priv = sx_priv(dev);

    if (!priv->dev.pdev) {
        sxd_log_info(dev, "sysfs hold is allowed only on a PCI device\n");
        goto out;
    }

    down_read(&priv->sysfs_access_info.rw_lock);

    /* if sysfs is disabled (locked_by_writer=true), tell the caller there is no device. */
    if (priv->sysfs_access_info.locked_by_writer) {
        up_read(&priv->sysfs_access_info.rw_lock);
        ret = -ENODEV;
        goto out;
    }

    if (atomic_inc_return(&priv->sysfs_access_info.num_of_readers) == 1) { /* first reader */
        priv->sysfs_access_info.locker_ts = jiffies; /* set the timestamp of the first reader */
    }

out:
    return ret;
}
EXPORT_SYMBOL(sx_core_sysfs_dev_hold);

void sx_core_sysfs_dev_release(struct sx_dev *dev)
{
    struct sx_priv *priv = NULL;

    if (unlikely(!dev)) {
        return;
    }

    priv = sx_priv(dev);

    if (!priv->dev.pdev) {
        sxd_log_info(dev, "sysfs release is allowed only on a PCI device\n");
        return;
    }

    if (atomic_dec_and_test(&priv->sysfs_access_info.num_of_readers)) {
        priv->sysfs_access_info.locker_ts = 0; /* when no more readers, reset the timestamp of oldest reader */
    }

    up_read(&priv->sysfs_access_info.rw_lock);
}
EXPORT_SYMBOL(sx_core_sysfs_dev_release);

int sx_get_cap_max_switch_ports(struct sx_dev *dev, u16 *max_switch_ports)
{
    struct ku_query_rsrc query_rsrc;
    int                  err = 0;

    memset(&query_rsrc, 0, sizeof(query_rsrc));
    query_rsrc.dev_id = dev->device_id;
    query_rsrc.rsrc_id = KU_RES_ID_CAP_MAX_SWITCH_PORTS;

    err = sx_QUERY_RSRC(dev, &query_rsrc);
    if (!err) {
        *max_switch_ports = (u16)query_rsrc.rsrc_val;
    }

    return err;
}

/* this function scans the PCI bus and fills 'info_arr' with devices information.
 * the function returns the number of devices filled in the array.
 * if arr_size is less than number of devices, error will be returned.
 */
int sx_pci_scan(struct sx_pci_device_info *info_arr, int arr_size)
{
    int             i, dev_count = 0;
    u16             vendor, device_id;
    struct pci_dev *pdev;

    for (i = 0; i < ARRAY_SIZE(__sx_pci_table); i++) {
        vendor = __sx_pci_table[i].vendor;
        device_id = __sx_pci_table[i].device;

        /* start iteration on the current type */
        pdev = pci_get_device(vendor, device_id, NULL);

        while (pdev) {
            /* pdev reference count is incremented */

            if (dev_count < arr_size) {
                info_arr[dev_count].pdev = pdev;
                info_arr[dev_count].bus_name = dev_name(&pdev->dev);
                info_arr[dev_count].device_id = pdev->device;
            } else {
                sxd_log_err(SXD_DEV_NONE, "PCI scan: number of devices on the bus is greater than array size\n");
                pci_dev_put(pdev); /* need to decrement back reference count */
                dev_count = -EINVAL;
                goto out;
            }

            dev_count++;
            pdev = pci_get_device(vendor, device_id, pdev); /* continue iteration from current place */
        }
    }

out:
    return dev_count;
}

bool sx_core_multi_asic_fs_mode(void)
{
    return multi_asic_fs_mode != 0;
}
EXPORT_SYMBOL(sx_core_multi_asic_fs_mode);
