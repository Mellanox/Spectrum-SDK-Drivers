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
#include <linux/kernel.h>

#include "sx_bfd_engine_data.h"
#include "sx_bfd_engine_ctrl.h"
#include "sx_bfd_cdev.h"
#include "sx_bfd_main.h"
#include "sx_bfd_rx_session.h"
#include "sx_bfd_tx_session.h"
#include <linux/mlx_sx/device.h>
#include <linux/mlx_sx/driver.h>
#include <linux/in.h>
#include <linux/inet.h>

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
            sxd_log_debug(__bfd_dev_db[i].dev, "sx_bfd_dev_db_find_free index %d\n", i);
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
            sxd_log_debug(__bfd_dev_db[i].dev, "sx_bfd_dev_db_find_device index %d\n", i);
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
    struct ip_addr                   ip_addr;
    struct list_vrfs               * vrf_entry = NULL;

    sx_dbg_dump_print_header(m, "sx_bfd tx session information dump:\n");
    bfd_dev_db = sx_bfd_dev_db_find_device(dev->device_id);
    if (bfd_dev_db == NULL) {
        sxd_log_notice(SXD_DEV_NONE, "Coudn't find device in BFD DB!!!!!\n");
        return 0;
    }

    spin_lock_bh(&bfd_dev_db->tx_sess_lock);

    sx_dbg_dump_print_table_header(m,
                                   "%-28s | %-12s | %-12s | %-12s | %-20s | %-50s | %-12s | %-12s | %-12s | %-12s | %-12s | %-12s",
                                   "tx session id #",
                                   "vrf_id",
                                   "user_vrf_id",
                                   "hash_key",
                                   "port",
                                   "ip",
                                   "scope_id",
                                   "packet_len",
                                   "interval",
                                   "ref_count",
                                   "tx_counter",
                                   "dropped_packets");

    hash_for_each_safe(bfd_dev_db->tx_sessions, i, tmp, tx_entry, node) {
        seq_printf(m,
                   "%-8d (%p)    %-12d   %-12d   ",
                   tx_entry->session_id,
                   tx_entry->sess,
                   tx_entry->sess->vrf_id,
                   tx_entry->sess->user_vrf_id);
        if (((struct sockaddr *)&tx_entry->sess->peer_addr)->sa_family == AF_INET) {
            ip_addr.family = AF_INET;
            memcpy(&ip_addr.ipv4, &tx_entry->sess->peer_addr.peer_in.sin_addr, sizeof(struct in_addr));
            seq_printf(m, "%-12lx   %-20hu   ", ip_addr_scope_id_hash(&ip_addr, &tx_entry->sess->scope_id),
                       ntohs(tx_entry->sess->peer_addr.peer_in.sin_port));
            seq_printf(m, "%-50pI4   ", &tx_entry->sess->peer_addr.peer_in.sin_addr.s_addr);
        } else {
            ip_addr.family = AF_INET6;
            memcpy(&ip_addr.ipv6, &tx_entry->sess->peer_addr.peer_in6.sin6_addr, sizeof(struct in6_addr));
            seq_printf(m, "%-12lx   %-20hu   ", ip_addr_scope_id_hash(&ip_addr, &tx_entry->sess->scope_id),
                       ntohs(tx_entry->sess->peer_addr.peer_in6.sin6_port));
            seq_printf(m, "%-50pI6   ", &tx_entry->sess->peer_addr.peer_in6.sin6_addr.s6_addr);
        }
        seq_printf(m,
                   "%-12d   %-12d   %-12d   %-12d   ",
                   tx_entry->sess->scope_id.scope_id,
                   tx_entry->sess->packet_len,
                   tx_entry->sess->interval,
                   tx_entry->sess->ref_count);
        seq_printf(m, "%-12lld   %-12lld\n", atomic64_read(&tx_entry->sess->tx_counter),
                   atomic64_read(&tx_entry->sess->dropped_packets));
    }

    sx_dbg_dump_print_empty_line(m);

    spin_unlock_bh(&bfd_dev_db->tx_sess_lock);

    sx_dbg_dump_print_header(m, "sx_bfd rx session information dump:\n");
    spin_lock_bh(&bfd_dev_db->rx_sess_lock);

    sx_dbg_dump_print_table_header(m,
                                   "%-28s | %-12s | %-12s | %-12s | %-20s | %-50s | %-12s | %-12s | %-12s | %-12s | %-12s | %-12s",
                                   "rx session id #",
                                   "vrf_id",
                                   "user_vrf_id",
                                   "hash_key",
                                   "session_opaque_data",
                                   "ip",
                                   "scope_id",
                                   "packet_len",
                                   "interval",
                                   "ref_count",
                                   "rx_counter",
                                   "dropped_packets");

    hash_for_each_safe(bfd_dev_db->rx_sessions, i, tmp, rx_entry, node) {
        seq_printf(m,
                   "%-8d (%p)    %-12d   %-12d   %-12lx   %-20llu   ",
                   rx_entry->session_id,
                   rx_entry->sess,
                   rx_entry->sess->vrf_id, rx_entry->sess->user_vrf_id,
                   ip_addr_scope_id_hash(&rx_entry->sess->ip_addr, &rx_entry->sess->scope_id),
                   rx_entry->sess->session_opaque_data);
        if (rx_entry->sess->ip_addr.family == AF_INET) {
            seq_printf(m, "%-50pI4   ", &rx_entry->sess->ip_addr.ipv4.s_addr);
        } else {
            seq_printf(m, "%-50pI6   ", &rx_entry->sess->ip_addr.ipv6.s6_addr);
        }
        seq_printf(m,
                   "%-12d   %-12d   %-12d   %-12d   ",
                   rx_entry->sess->scope_id.scope_id,
                   rx_entry->sess->packet_len,
                   rx_entry->sess->interval,
                   rx_entry->sess->ref_count);
        seq_printf(m, "%-12lld   %-12lld\n", atomic64_read(&rx_entry->sess->rx_counter),
                   atomic64_read(&rx_entry->sess->dropped_packets));
    }

    sx_dbg_dump_print_empty_line(m);
    spin_unlock_bh(&bfd_dev_db->rx_sess_lock);

    sx_dbg_dump_print_header(m, "sx_bfd managed VRF link-list information dump:\n");

    sx_dbg_dump_print_table_header(m,
                                   "%-12s | %-20s | %-12s | %-20s",
                                   "vrf_id #",
                                   "netns",
                                   "ref_count",
                                   "linux_vrf_name");

    /* search in link-list vrf */
    list_for_each_entry(vrf_entry, &bfd_dev_db->vrfs_list, list) {
        seq_printf(m,
                   "%-12d   (%p)     %-12d   %-20s\n",
                   vrf_entry->vrf_id,
                   vrf_entry->netns,
                   vrf_entry->ref_count,
                   vrf_entry->linux_vrf_name);
    }

    return 0;
}

static void * sx_bfd_init_one(struct sx_dev *dev, struct sx_flow_info *fi)
{
    int                    err = 0;
    struct sx_bfd_dev_db * bfd_dev_db = NULL;

    bfd_dev_db = sx_bfd_dev_db_find_free();
    if (bfd_dev_db == NULL) {
        sxd_log_err(dev,
                    "Couldn't find free index for device %d in BFD devices DB.\n",
                    dev->device_id);
        return NULL;
    }

    memset(bfd_dev_db, 0, sizeof(struct sx_bfd_dev_db));
    bfd_dev_db->dev = dev;
    bfd_dev_db->device_id = dev->device_id;
    bfd_dev_db->vrf_id_running_counter = SX_BFD_LINK_LOCAL_VRFS_LIST_BEGIN;

    err = sx_bfd_engine_ctrl_init(bfd_dev_db);
    if (err < 0) {
        sxd_log_err(dev, "Kernel BFD engine control failed to initialize.\n");
        goto bail;
    }

    bfd_dev_db->initialized = true;

    sx_dbg_dump_read_handler_register("bfd_dump",
                                      __bfd_dump,
                                      NULL,
                                      dev,
                                      NULL);

    sxd_log_debug(dev, "Kernel BFD initialized device %d\n", dev->device_id);

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

    sxd_log_debug(dev, "Kernel BFD uninitialized device %d\n", dev->device_id);
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

    sxd_log_notice(SXD_DEV_NONE, "Kernel BFD start init.\n");

    sx_bfd_globals.char_dev = MKDEV(SX_BFD_MAJOR, SX_BFD_BASE_MINOR);
    err = register_chrdev_region(sx_bfd_globals.char_dev, SX_MAX_PCI_DEVICES,
                                 SX_BFD_CHAR_DEVICE_NAME);
    if (err) {
        sxd_log_info(SXD_DEV_NONE, "Couldn't register the default device number. "
                     "Trying to allocate one dynamically %d\n", err);
        err = alloc_chrdev_region(&sx_bfd_globals.char_dev, SX_BFD_BASE_MINOR, SX_MAX_PCI_DEVICES,
                                  SX_BFD_CHAR_DEVICE_NAME);
        if (err) {
            sxd_log_err(SXD_DEV_NONE, "Couldn't register device number. "
                        "Aborting...\n");
            goto bail;
        }
    }

    sx_bfd_init_char_dev();

    err = sx_register_interface(&sx_bfd_interface);
    if (err) {
        goto bail_cdev;
    }

    sxd_log_notice(SXD_DEV_NONE, "Kernel BFD init success.\n");

    return 0;

bail_cdev:
    sx_bfd_deinit_char_dev();
    unregister_chrdev_region(sx_bfd_globals.char_dev, SX_MAX_PCI_DEVICES);
bail:
    sxd_log_debug(SXD_DEV_NONE, "Kernel BFD init failed.\n");
    return err;
}

static void __exit sx_bfd_deinit(void)
{
    sxd_log_notice(SXD_DEV_NONE, "Kernel BFD start deinit.\n");
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

    sxd_log_notice(SXD_DEV_NONE, "Kernel BFD deinit success.\n");
}

module_init(sx_bfd_init);
module_exit(sx_bfd_deinit);

MODULE_AUTHOR("Alla Segal, Dan Akunis");
MODULE_DESCRIPTION("NVIDIA Switch BFD driver");
MODULE_LICENSE("Dual BSD/GPL");
