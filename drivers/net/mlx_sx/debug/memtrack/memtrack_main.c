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
#include "memtrack.h"

MODULE_AUTHOR("NVIDIA");
MODULE_DESCRIPTION("Memory allocations tracking");
MODULE_LICENSE("GPL");

static int __init memtrack_init(void)
{
    int err = 0;

    sxd_log_mem_info("initializing module\n");

    err = memtrack_db_create();
    if (err) {
        sxd_log_mem_err("failed to create database (err=%d)\n", err);
        goto out;
    }

    err = memtrack_debugfs_create();
    if (err) {
        sxd_log_mem_err("failed to create debugfs entries (err=%d)\n", err);
        goto out_db_destroy;
    }

    return 0;

out_db_destroy:
    memtrack_db_destroy();

out:
    return err;
}

static void __exit memtrack_cleanup(void)
{
    memtrack_debugfs_destroy();
    memtrack_db_destroy();
    sxd_log_mem_info("cleanup module\n");
}

/************************************************
 *  MODULE init/exit
 ***********************************************/

module_init(memtrack_init);
module_exit(memtrack_cleanup);

/************************************************
 *                  EOF                         *
 ***********************************************/
