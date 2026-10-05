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

#include <linux/mlx_sx/kernel_user.h>

#include "sx.h"
#include "ioctl_internal.h"
#include "dev_init.h"

long ctrl_cmd_set_pci_profile(struct file *file, unsigned int cmd, unsigned long data)
{
    struct sx_dev *dev;

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    return sx_core_ioctl_set_pci_profile(dev, data, 1);
}


long ctrl_cmd_set_pci_profile_driver_only(struct file *file, unsigned int cmd, unsigned long data)
{
    struct sx_dev *dev;

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    return sx_core_ioctl_set_pci_profile(dev, data, 0);
}


long ctrl_cmd_pci_device_restart(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_set_pci_device_restart pci_restart;
    int                              err = 0;
    struct sx_dev                   *dev;

    err = copy_from_user(&pci_restart, (void*)data, sizeof(pci_restart));
    if (err) {
        return err;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &pci_restart.dev_id);

    if (!dev) {
        sxd_log_err(SXD_DEV_NONE, "device %u wasn't found\n", pci_restart.dev_id);
        return -ENODEV;
    }
    sxd_log_info(dev, "ioctl PCI device restart called for device %u\n", pci_restart.dev_id);

    return sx_restart_one_pci(pci_restart.dev_id, pci_restart.is_chip_reset);
}


long ctrl_cmd_get_pci_profile(struct file *file, unsigned int cmd, unsigned long data)
{
    struct sx_dev            *dev;
    struct ku_get_pci_profile pci_prof;
    int                       err = 0;

    err = copy_from_user(&pci_prof,
                         (void*)data, sizeof(pci_prof));
    if (err) {
        return -ENOMEM;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &pci_prof.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    return sx_core_ioctl_get_pci_profile(dev, data);
}


long ctrl_cmd_reset(struct file *file, unsigned int cmd, unsigned long data)
{
    sxd_dev_id_t   dev_id = (sxd_dev_id_t)data;
    struct sx_dev *dev;

    sx_core_get_dev_and_id_from_fd(file, &dev, &dev_id);
    if (!dev) {
        return -ENODEV;
    }

    return sx_change_configuration(dev);
}

long ctrl_cmd_get_system_status(struct file *file, unsigned int cmd, unsigned long data)
{
    struct sx_dev *dev;
    u16            system_status = 0;
    int            err = 0;

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        err = -ENODEV;
        goto out;
    }

    err = get_system_status(dev, &system_status);
    if (err) {
        sxd_log_err(dev, "get-system-status: failed to read status from CR\n");
        goto out;
    }

    err = copy_to_user((void*)data, &system_status, sizeof(system_status));
    if (err) {
        goto out;
    }

out:
    return err;
}
