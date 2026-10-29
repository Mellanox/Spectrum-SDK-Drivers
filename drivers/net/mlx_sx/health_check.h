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

#ifndef __HEALTH_CHECK_H__
#define __HEALTH_CHECK_H__

#include <linux/types.h>
#include <linux/mlx_sx/kernel_user.h>

struct sx_cq;

enum sx_health_check_trigger_op {
    SX_HEALTH_CHECK_TRIGGER_OP_ADD_DEV,
    SX_HEALTH_CHECK_TRIGGER_OP_DEL_DEV,
    SX_HEALTH_CHECK_TRIGGER_OP_SYSFS,
    SX_HEALTH_CHECK_TRIGGER_OP_CATAS,
    SX_HEALTH_CHECK_TRIGGER_OP_CMD_IFC,
    SX_HEALTH_CHECK_TRIGGER_OP_SDQ,
    SX_HEALTH_CHECK_TRIGGER_OP_RDQ,
    SX_HEALTH_CHECK_TRIGGER_OP_CANCEL_ALL,
    SX_HEALTH_CHECK_TRIGGER_OP_CANCEL_ONE,
    SX_HEALTH_CHECK_TRIGGER_OP_KERNEL_THREADS,
    SX_HEALTH_CHECK_TRIGGER_OP_FW_FATAL_EVENT_TEST,
    SX_HEALTH_CHECK_TRIGGER_OP_FW_CAUSE_TEST,
    SX_HEALTH_CHECK_TRIGGER_OP_FW_ASSERT_TEST,
    SX_HEALTH_CHECK_TRIGGER_OP_FW_TILE_ASSERT_TEST,
    SX_HEALTH_CHECK_TRIGGER_OP_FW_TILE_FATAL_CAUSE_TEST,
    SX_HEALTH_CHECK_TRIGGER_OP_FW_PLL_FATAL_CAUSE_TEST,
    SX_HEALTH_CHECK_TRIGGER_OP_EXTEND_THRESHOLD_THREAD_MONITOR,
    SX_HEALTH_CHECK_TRIGGER_OP_SET_NEW_THRESHOLD_THREAD_MONITOR,
    SX_HEALTH_CHECK_TRIGGER_OP_EXTEND_THRESHOLD_KERNEL_THREAD_MONITOR
};

#define SX_HEALTH_CHECK_ADD_THREAD    1
#define SX_HEALTH_CHECK_DELETE_THREAD 0

#define NUM_OF_SDK_THREADS_MAP_BY_BITS 64
#define SX_PER_DEV_CHECKS \
    "0-tasklet, 1-catas, 2-sdq, 3-rdq, 4-sdk_threads, 5-cmd_ifc, 6-sysfs_ts, 7-kernel_threads"
#define SX_HEALTH_DEBUG_STATE_FLAGS \
    "0-fatal_on_warn"

struct sx_health_check_trigger_params {
    enum sx_health_check_trigger_op op;
    u8                              dev_id;
    union {
        struct dq_params {
            int dqn;
        } dq_params;
        struct kthread_params {
            char* name;
        } kthread_params;
        /* This filed is for debug purpose for cases we want
         * to increase the thread threshold time (debug kernel setup for example)*/
        struct sdk_thread_threshold_params {
            int mult_value;
            int new_debug_threshold_time;
        } sdk_threshold_debug_params;
        struct cancel_one_params {
            sxd_health_cause_t cause;
        } cancel_one_params;
        struct cmd_ifc_trigger_params {
            sxd_health_cause_t cause;
        } cmd_ifc_trigger_params;
    } params;
    bool fatal_error_mode_active;
};

int sx_health_check_dev_init(struct sx_dev *dev);
void sx_health_check_dev_deinit(struct sx_dev *dev);
void sx_health_check_dev_disable(struct sx_dev *dev);
int sx_health_check_configure(struct sx_dev *dev, ku_dbg_health_check_params_t *params);
void sx_health_check_report_cmd_ifc_ok(struct sx_dev *dev);

int sx_health_check_dump(struct seq_file *m, void *v, void *context);

void sx_health_check_set_debug_trigger(struct sx_dev *dev, struct sx_health_check_trigger_params *params);

/**
 * sx_health_check_get_first_kthread_name - Get first kthread name from health DB
 * @dev: Pointer to sx_dev structure
 * @buf: Buffer to copy the kthread name into
 * @buf_size: Size of buf (should be at least 64 to hold any monitored wq name)
 *
 * Returns: 0 if a name was copied, -ENODEV if health not initialized,
 *          -ENOENT if no kernel threads are registered in the health DB.
 */
int sx_health_check_get_first_kthread_name(struct sx_dev *dev, char *buf, size_t buf_size);

/**
 * sx_health_check_get_first_rdq_dqn - Get first RDQ dqn from health DB config
 * @dev: Pointer to sx_dev structure
 * @dqn: Output parameter for the first RDQ dqn in the health DB config
 *
 * Returns: 0 if dqn was set, -ENODEV if health not initialized,
 *          -ENOENT if no RDQ is configured in the health DB.
 */
int sx_health_check_get_first_rdq_dqn(struct sx_dev *dev, int *dqn);

/**
 * sx_health_check_get_first_sdq_dqn - Get first SDQ dqn from health DB config
 * @dev: Pointer to sx_dev structure
 * @dqn: Output parameter for the first SDQ dqn in the health DB config
 *
 * Returns: 0 if dqn was set, -ENODEV if health not initialized,
 *          -ENOENT if no SDQ is configured in the health DB.
 */
int sx_health_check_get_first_sdq_dqn(struct sx_dev *dev, int *dqn);

int sx_health_update_tg(struct sx_dev *dev, int hw_trap_group, bool is_add, bool is_wjh_rdq_update);

/**
 * sx_health_update_sdq_timeout - Update SDQ monitor timeout based on current EMAD timeout
 * @dev: Pointer to sx_dev structure
 *
 * This function should be called when EMAD timeout is changed to synchronize
 * the SDQ monitor timeout with the new EMAD timeout value.
 */
void sx_health_update_sdq_timeout(struct sx_dev *dev);

void sx_health_report_error_mfde(struct sx_dev *dev, struct ku_mfde_reg *mfde_reg);
void sx_health_report_error_meccc(struct sx_dev *dev, struct ku_meccc_reg *meccc_reg);
void sx_health_report_error_fshe(struct sx_dev *dev, struct ku_fshe_reg *fshe_reg);
void sx_health_report_error_emad_timeout(struct sx_dev *dev,
                                         u16            reg_id,
                                         u32            usecs,
                                         const char    *origin);
void sx_health_report_error_cmdifc_timeout(struct sx_dev     *dev,
                                           sxd_health_cause_t cause,
                                           u16                op,
                                           u16                reg_id);
void sx_health_report_error_generic(struct sx_dev        *dev,
                                    sxd_health_severity_t severity,
                                    const char           *error_msg);

void sx_health_sdk_terminated(struct sx_dev *dev);

int sx_health_is_debug_trigger(struct sx_dev *dev, bool is_send, int dqn);

int sx_health_check_dbg_cmd_handler(int argc, const char *argv[], struct sx_dev *dev, void *context);

void sx_health_cq_ok(struct sx_cq *cq);

#endif /* __HEALTH_CHECK_H__ */
