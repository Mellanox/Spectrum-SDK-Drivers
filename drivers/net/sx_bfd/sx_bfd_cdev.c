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

#include "sx_bfd_cdev.h"
#include "sx_bfd_main.h"

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sx_bfd/sx_bfd_ctrl_cmds.h>
#include "sx_bfd_tx_session.h"
#include "sx_bfd_rx_session.h"

static long __sx_bfd_ioctl(struct file *fp __attribute__((unused)), unsigned int cmd, unsigned long data)
{
    int                    err = 0;
    int                    minor = iminor(file_inode(fp));
    struct sx_bfd_dev_db * bfd_dev_db = NULL;

    bfd_dev_db = sx_bfd_dev_db_find_device((sxd_dev_id_t)minor);
    if (bfd_dev_db == NULL) {
        sxd_log_err(SXD_DEV_NONE, "Couldn't find device %d in BFD devices DB.\n", minor);
        err = -ENOTTY;
        return err;
    }

    switch (cmd) {
    case SX_BFD_CMD_START_TX_OFFLOAD:
        return sx_bfd_tx_sess_add(bfd_dev_db, (char*)data, NULL);

    case SX_BFD_CMD_UPDATE_TX_OFFLOAD:
        return sx_bfd_tx_sess_update(bfd_dev_db, (char*)data);

    case SX_BFD_CMD_STOP_TX_OFFLOAD:
        return sx_bfd_tx_sess_del(bfd_dev_db, (char*)data, NULL);

    case SX_BFD_CMD_START_RX_OFFLOAD:
        return sx_bfd_rx_sess_add(bfd_dev_db, (char*)data);

    case SX_BFD_CMD_UPDATE_RX_OFFLOAD:
        return sx_bfd_rx_sess_update(bfd_dev_db, (char*)data);

    case SX_BFD_CMD_STOP_RX_OFFLOAD:
        return sx_bfd_rx_sess_del(bfd_dev_db, (char*)data);

    case SX_BFD_CMD_GET_RX_STATS:
        return sx_bfd_get_rx_sess_stats(bfd_dev_db, (char*)data, false);

    case SX_BFD_CMD_GET_TX_STATS:
        return sx_bfd_get_tx_sess_stats(bfd_dev_db, (char*)data, false);

    case SX_BFD_CMD_GET_AND_CLEAR_RX_STATS:
        return sx_bfd_get_rx_sess_stats(bfd_dev_db, (char*)data, true);

    case SX_BFD_CMD_GET_AND_CLEAR_TX_STATS:
        return sx_bfd_get_tx_sess_stats(bfd_dev_db, (char*)data, true);

    default:
        sxd_log_debug(SXD_DEV_FROM_BFD(bfd_dev_db), "Unsupported sx bfd command %d", cmd);
        err = -ENOTTY;
    }

    return err;
}

static const struct file_operations __sx_bfd_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = __sx_bfd_ioctl      /* As of Linux kernel 2.6.11 need to use unlocked_ioctl */
};

void sx_bfd_init_char_dev(void)
{
    cdev_init(&sx_bfd_globals.cdev, &__sx_bfd_fops);
    sx_bfd_globals.cdev.owner = THIS_MODULE;
}

void sx_bfd_deinit_char_dev(void)
{
    cdev_del(&sx_bfd_globals.cdev);
}

int sx_bfd_cdev_init(struct sx_bfd_dev_db *bfd_dev_db)
{
    int err = 0;
    int devno, major, minor;

    if (!bfd_dev_db->initialized) {
        major = MAJOR(sx_bfd_globals.char_dev);
        minor = MINOR(sx_bfd_globals.char_dev) + bfd_dev_db->device_id;
        devno = MKDEV(major, minor);

        err = cdev_add(&sx_bfd_globals.cdev, devno, 1);
        sxd_log_debug(SXD_DEV_FROM_BFD(bfd_dev_db), "Create char dev with major:%d minor:%d \n", major, minor);
        if (err) {
            sxd_log_err(SXD_DEV_FROM_BFD(bfd_dev_db),
                        "Couldn't add char device for device %d. Aborting... err: %d\n",
                        bfd_dev_db->device_id,
                        err);
            return err;
        }

        sxd_log_debug(SXD_DEV_FROM_BFD(bfd_dev_db),
                      "BFD char-device initialized for device %d.\n",
                      bfd_dev_db->device_id);
    }

    return err;
}
