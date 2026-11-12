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

#ifndef SX_BFD_MAIN_H
#define SX_BFD_MAIN_H

#include <linux/mlx_sx/device.h>
#include <linux/hash.h>
#include <linux/hashtable.h>

struct sx_bfd_dev_db * sx_bfd_dev_db_find_device(sxd_dev_id_t device_id);

struct sockets_scheduler {
    struct list_head scheduler_list;
    spinlock_t       scheduler_lock;
    struct semaphore scheduler_sem;
    bool             scheduler_thread_active;
};

/*According to architecture we should support up to 1K sessions.
 * Keeping 75% load factor we need 1K/0.75 buckets = 1365 buckets.
 * rounding to power of 2 we get 2048 (11 bits) */
#define SX_BFD_SESSIONS_HASH_BITS 11

#define SX_RX_VRFS_HASH_BITS 8

struct sx_bfd_dev_db {
    bool                      initialized;
    struct sx_dev            *dev;
    sxd_dev_id_t              device_id;
    struct workqueue_struct * workqueue;
    spinlock_t                tx_sess_lock;
    spinlock_t                rx_sess_lock;
    struct hlist_head         tx_sessions[1 << SX_BFD_SESSIONS_HASH_BITS];
    struct hlist_head         rx_sessions[1 << SX_BFD_SESSIONS_HASH_BITS];
    struct hlist_head         rx_vrfs[1 << SX_RX_VRFS_HASH_BITS];
    struct mutex              rx_db_lock;
    struct sockets_scheduler  scheduler;
    struct task_struct      * bfd_thread;
};

struct sx_bfd_globals_db {
    dev_t       char_dev;
    struct cdev cdev;
};

extern struct sx_bfd_globals_db sx_bfd_globals;

#endif /* SX_BFD_MAIN_H */
