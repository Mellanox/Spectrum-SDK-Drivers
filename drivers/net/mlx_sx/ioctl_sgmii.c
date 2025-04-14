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

#include <linux/fs.h>

#include <linux/mlx_sx/kernel_user.h>

#include "sx.h"
#include "sgmii.h"
#include "dev_init.h"
#include "ioctl_internal.h"

long ctrl_cmd_set_sgmii_system_cfg(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_sgmii_system_cfg sgmii_system_cfg;
    int                        err;

    err = copy_from_user(&sgmii_system_cfg, (void*)data, sizeof(sgmii_system_cfg));
    if (err) {
        goto out;
    }

    err = sgmii_set_system_cfg(&sgmii_system_cfg);
    if (err) {
        goto out;
    }

    err = sx_dev_init_oob_pci(); /* detect the SwitchX on PCI */

out:
    return err;
}


long ctrl_cmd_init_sgmii_dev(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_sgmii_init_dev sgmii_init_dev;
    int                      err;

    err = copy_from_user(&sgmii_init_dev, (void*)data, sizeof(sgmii_init_dev));
    if (err) {
        goto out;
    }

    err = sgmii_dev_init(sgmii_init_dev.dev_id, sgmii_init_dev.init_hopf);

out:
    return err;
}


long ctrl_cmd_set_sgmii_default_dev(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_sgmii_default_dev default_dev_cfg;
    int                         err;

    err = copy_from_user(&default_dev_cfg, (void*)data, sizeof(default_dev_cfg));
    if (err) {
        goto out;
    }

    err = sgmii_default_dev_set(default_dev_cfg.dev_id,
                                default_dev_cfg.hopf_on_remote_mgmt ? true : false);
    if (err) {
        sxd_log_err("failed to set SGMII default device (err=%d)\n", err);
    }

out:
    return err;
}


long ctrl_cmd_set_sgmii_mft_info(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_sgmii_mft_info mft_info;
    int                      err;

    err = copy_from_user(&mft_info, (void*)data, sizeof(mft_info));
    if (err) {
        goto out;
    }

    err = sgmii_dev_set_mft_info(&mft_info);
    if (err) {
        sxd_log_err("failed to set SGMII MFT info (err=%d)\n", err);
    }

out:
    return err;
}
