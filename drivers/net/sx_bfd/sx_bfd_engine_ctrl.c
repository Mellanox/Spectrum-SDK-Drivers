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
#include <linux/kernel.h>

#include "sx_bfd_engine_ctrl.h"
#include "sx_bfd_cdev.h"
#include "sx_bfd_tx_session.h"
#include "sx_bfd_rx_session.h"
#include <linux/mlx_sx/device.h>

int sx_bfd_engine_ctrl_init(struct sx_bfd_dev_db *bfd_dev_db)
{
    int err;

    err = sx_bfd_cdev_init(bfd_dev_db);
    if (err < 0) {
        sxd_log_err("Kernel BFD chardevice failed to initialize for device %d.\n", bfd_dev_db->device_id);
        goto bail;
    }

    err = sx_bfd_workqueue_init(bfd_dev_db);
    if (err < 0) {
        sxd_log_err("Kernel BFD workqueue initialization failed.\n");
        goto bail;
    }

    err = sx_bfd_rx_session_init(bfd_dev_db);
    if (err < 0) {
        sxd_log_err("Kernel BFD RX session initialization failed.\n");
        goto bail;
    }

    err = sx_bfd_tx_session_init(bfd_dev_db);
    if (err < 0) {
        sxd_log_err("Kernel BFD TX session initialization failed.\n");
        goto bail;
    }

bail:
    if (err) {
        sx_bfd_engine_ctrl_deinit(bfd_dev_db);
    }
    return err;
}

void sx_bfd_engine_ctrl_deinit(struct sx_bfd_dev_db *bfd_dev_db)
{
    sx_bfd_rx_session_deinit(bfd_dev_db);

    sx_bfd_tx_session_deinit(bfd_dev_db);

    sx_bfd_workqueue_deinit(bfd_dev_db);
}
