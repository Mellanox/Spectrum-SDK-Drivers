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

#include <linux/module.h>
#include <linux/kernel.h>

#include "sx_bfd_engine_data.h"
#include "sx_bfd_engine_ctrl.h"
#include "sx_bfd_cdev.h"
#include "sx_bfd_main.h"
#include "sx_bfd_rx_session.h"
#include "sx_bfd_tx_session.h"
#include <linux/mlx_sx/device.h>
#include <linux/mlx_sx/driver.h>

static struct sx_bfd_dev_db __bfd_dev_db[SX_MAX_PCI_DEVICES];

#define SX_BFD_CHAR_DEVICE_NAME "bfdcdev"

enum SX_BFD_CHAR_DEVICE {
    SX_BFD_MAJOR      = 232,
    SX_BFD_BASE_MINOR = 0,
};

struct sx_bfd_globals_db sx_bfd_globals;

static struct sx_bfd_dev_db * sx_bfd_dev_db_find_free(void)
{
    int i;

    for (i = 0; i < SX_MAX_PCI_DEVICES; i++) {
        if (__bfd_dev_db[i].initialized == false) {
            sxd_log_debug("sx_bfd_dev_db_find_free index %d\n", i);
            return &__bfd_dev_db[i];
        }
    }

    return NULL;
}

struct sx_bfd_dev_db * sx_bfd_dev_db_find_device(sxd_dev_id_t device_id)
{
    int i;

    for (i = 0; i < SX_MAX_PCI_DEVICES; i++) {
        if ((__bfd_dev_db[i].initialized == true) && (__bfd_dev_db[i].device_id == device_id)) {
            sxd_log_debug("sx_bfd_dev_db_find_device index %d\n", i);
            return &__bfd_dev_db[i];
        }
    }

    return NULL;
}

static int __bfd_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_dev                  * dev = sx_dbg_dump_get_device(m);
    struct sx_bfd_dev_db           * bfd_dev_db = NULL;
    int                              i;
    struct hlist_node              * tmp;
    struct sx_bfd_tx_session_entry * tx_entry;
    struct sx_bfd_rx_session_entry * rx_entry;


    sx_dbg_dump_print_header(m, "sx_bfd information dump\n");
    bfd_dev_db = sx_bfd_dev_db_find_device(dev->device_id);
    if (bfd_dev_db == NULL) {
        sxd_log_notice("Coudn't find device in BFD DB!!!!!\n");
        return 0;
    }

    spin_lock_bh(&bfd_dev_db->tx_sess_lock);

    hash_for_each_safe(bfd_dev_db->tx_sessions, i, tmp, tx_entry, node) {
        seq_printf(m, "bfd tx session: id %d (%p)\n", tx_entry->session_id, tx_entry->sess);
    }

    spin_unlock_bh(&bfd_dev_db->tx_sess_lock);

    spin_lock_bh(&bfd_dev_db->rx_sess_lock);

    hash_for_each_safe(bfd_dev_db->rx_sessions, i, tmp, rx_entry, node) {
        seq_printf(m, "bfd rx session: id %d (%p)\n", rx_entry->session_id, rx_entry->sess);
    }

    spin_unlock_bh(&bfd_dev_db->rx_sess_lock);

    return 0;
}

static void * sx_bfd_init_one(struct sx_dev *dev, struct sx_flow_info *fi)
{
    int                    err = 0;
    struct sx_bfd_dev_db * bfd_dev_db = NULL;

    bfd_dev_db = sx_bfd_dev_db_find_free();
    if (bfd_dev_db == NULL) {
        sxd_log_err("Couldn't find free index for device %d in BFD devices DB.\n", dev->device_id);
        return NULL;
    }

    memset(bfd_dev_db, 0, sizeof(struct sx_bfd_dev_db));
    bfd_dev_db->dev = dev;
    bfd_dev_db->device_id = dev->device_id;

    err = sx_bfd_engine_ctrl_init(bfd_dev_db);
    if (err < 0) {
        sxd_log_err("Kernel BFD engine control failed to initialize.\n");
        goto bail;
    }

    bfd_dev_db->initialized = true;

    sx_dbg_dump_read_handler_register("bfd_dump",
                                      __bfd_dump,
                                      NULL,
                                      dev,
                                      NULL);

    sxd_log_debug("Kernel BFD initialized device %d\n", dev->device_id);

    return bfd_dev_db;

bail:
    return NULL;
}
static void sx_bfd_remove_one(struct sx_dev *dev, void *rsc, struct sx_flow_info *fi)
{
    struct sx_bfd_dev_db * bfd_dev_db = (struct sx_bfd_dev_db*)rsc;

    sx_bfd_engine_ctrl_deinit(bfd_dev_db);
    bfd_dev_db->initialized = false;

    sx_dbg_dump_read_handler_unregister("bfd_dump", dev);

    sxd_log_debug("Kernel BFD uninitialized device %d\n", dev->device_id);
}
static int sx_bfd_event(struct sx_dev       *dev,
                        void                *resources,
                        enum sx_dev_event    event,
                        union sx_event_data *event_data)
{
    int err = 0;

    switch (event) {
    default:
        break;
    }
    return err;
}
static struct sx_interface sx_bfd_interface = {
    .add = sx_bfd_init_one,
    .remove = sx_bfd_remove_one,
    .event = sx_bfd_event,
    .name = "sx_bfd"
};

static int __init sx_bfd_init(void)
{
    int err = 0;

    sxd_log_notice("Kernel BFD start init.\n");

    sx_bfd_globals.char_dev = MKDEV(SX_BFD_MAJOR, SX_BFD_BASE_MINOR);
    err = register_chrdev_region(sx_bfd_globals.char_dev, SX_MAX_PCI_DEVICES,
                                 SX_BFD_CHAR_DEVICE_NAME);
    if (err) {
        sxd_log_info("Couldn't register the default device number. "
                     "Trying to allocate one dynamically %d\n", err);
        err = alloc_chrdev_region(&sx_bfd_globals.char_dev, SX_BFD_BASE_MINOR, SX_MAX_PCI_DEVICES,
                                  SX_BFD_CHAR_DEVICE_NAME);
        if (err) {
            sxd_log_err("Couldn't register device number. "
                        "Aborting...\n");
            goto bail;
        }
    }

    sx_bfd_init_char_dev();

    err = sx_register_interface(&sx_bfd_interface);
    if (err) {
        goto bail_cdev;
    }

    sxd_log_notice("Kernel BFD init success.\n");

    return 0;

bail_cdev:
    sx_bfd_deinit_char_dev();
    unregister_chrdev_region(sx_bfd_globals.char_dev, SX_MAX_PCI_DEVICES);
bail:
    sxd_log_debug("Kernel BFD init failed.\n");
    return err;
}

static void __exit sx_bfd_deinit(void)
{
    sxd_log_notice("Kernel BFD start deinit.\n");
    sx_bfd_deinit_char_dev();
    unregister_chrdev_region(sx_bfd_globals.char_dev, SX_MAX_PCI_DEVICES);
    sx_unregister_interface(&sx_bfd_interface);

    /* The BFD driver is using kernel UDP sockets. when driver unloads, it terminates all living
     * BFD sessions (thus, closing all sockets). UDP sockets run their destructor-callback in
     * RCU context and not inline [look at kernel function sk_destruct() and the comment above
     * __sk_destruct(), which is the wrapper of the real socket destructor that is running in RCU
     * context].
     * Since this driver installs a private socket destructor for every socket [sk_user_destruct() which
     * resides in the driver], this destructor must run BEFORE the driver is totally unloaded from the kernel.
     * This is why we need to call synchronize_rcu(), to let all destructors complete their task before
     * returning from this function.
     **/

    synchronize_rcu();

    sxd_log_notice("Kernel BFD deinit success.\n");
}

module_init(sx_bfd_init);
module_exit(sx_bfd_deinit);

MODULE_AUTHOR("Alla Segal, Dan Akunis");
MODULE_DESCRIPTION("NVIDIA Switch BFD driver");
MODULE_LICENSE("Dual BSD/GPL");
