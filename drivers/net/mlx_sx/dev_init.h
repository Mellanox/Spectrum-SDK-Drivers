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

#ifndef __SX_DEV_INIT_H__
#define __SX_DEV_INIT_H__

#include <linux/types.h>

struct sx_pci_device_info {
    struct pci_dev *pdev;
    const char     *bus_name;
    u16             device_id;
};

struct sx_dev;
struct sx_priv;
struct pci_restart_saved_params;

extern int rx_debug;
extern int tx_debug;
extern int chip_info_type;
extern int chip_info_revision;

void sx_send_udev_event(struct pci_dev     *pdev,
                        struct sx_priv     *priv,
                        enum kobject_action action,
                        const char         *origin);
int sx_dev_init_cr_device(bool reset_chip);
void sx_dev_deinit_cr_device(void);
int sx_dev_init_core_pci(bool do_reset, u32 *total_probes, u32 *successful_probes);
void sx_dev_deinit_core_pci(u32 *total_removes);
int sx_dev_init_oob_pci(void);
void sx_dev_deinit_oob_pci(void);
int sx_restart_one_pci(struct sx_dev *dev, bool do_reset);
int sx_add_one_pci(struct pci_dev *pdev, bool do_reset);
void sx_remove_one_pci(struct pci_dev *pdev);
int sx_core_init_one(struct sx_priv **sx_priv, struct pci_restart_saved_params *saved_params);
void sx_core_remove_one(struct sx_priv *priv, bool keep_listeners, struct sx_flow_info *fi);
int sx_core_create_fake_device(struct sx_priv **priv_pp);
void sx_core_remove_fake_device(struct sx_priv *priv);
bool sx_core_fw_is_faulty(struct sx_dev *dev);
int sx_pci_scan(struct sx_pci_device_info *info_arr, int arr_size);

#endif /* __SX_DEV_INIT_H__ */
