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
#include <linux/mlx_sx/device.h>

static int __init sx_bfd_init(void)
{
    int err = 0;

    err = sx_bfd_engine_ctrl_init();
    if (err < 0) {
        sxd_log_err("Kernel BFD engine control failed to initialize.\n");
        goto bail;
    }

    err = sx_bfd_engine_data_init();
    if (err < 0) {
        sxd_log_err("Kernel BFD engine data failed to initialize.\n");
        goto bail;
    }

    sxd_log_debug("Kernel BFD initialized\n");

bail:
    return err;
}

static void __exit sx_bfd_deinit(void)
{
    sxd_log_debug("Kernel BFD start deinit \n");

    sx_bfd_engine_ctrl_deinit();
    sx_bfd_engine_data_deinit();

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

    sxd_log_debug("Kernel BFD deinitialized\n");
}

module_init(sx_bfd_init);
module_exit(sx_bfd_deinit);

MODULE_AUTHOR("Alla Segal, Dan Akunis");
MODULE_DESCRIPTION("NVIDIA Switch BFD driver");
MODULE_LICENSE("Dual BSD/GPL");
