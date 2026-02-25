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

#ifndef __SX_INTERNAL_LOG_H__
#define __SX_INTERNAL_LOG_H__

#include <linux/types.h>
#include <linux/spinlock.h>

#define SX_INTERNAL_LOG_MAX_NAME (64)

enum sx_internal_log_severity {
    SX_INTERNAL_LOG_SEVERITY_CRITICAL_E,
    SX_INTERNAL_LOG_SEVERITY_ERROR_E,
    SX_INTERNAL_LOG_SEVERITY_WARNING_E,
    SX_INTERNAL_LOG_SEVERITY_NOTICE_E,
    SX_INTERNAL_LOG_SEVERITY_INFO_E,
    SX_INTERNAL_LOG_SEVERITY_DEBUG_E,
    SX_INTERNAL_LOG_SEVERITY_INVALID_E
};

#define sx_int_log(ilog, severity, fmt, args ...) sx_internal_log_add((ilog), (severity), (fmt), ## args)
#define sx_int_log_critical(ilog, fmt, args ...)    \
    sx_int_log((ilog),                              \
               SX_INTERNAL_LOG_SEVERITY_CRITICAL_E, \
               (fmt),                               \
               ## args)
#define sx_int_log_error(ilog, fmt, args ...) sx_int_log((ilog), SX_INTERNAL_LOG_SEVERITY_ERROR_E, (fmt), ## args)
#define sx_int_log_warning(ilog, fmt, args ...)                   \
    sx_int_log((ilog), SX_INTERNAL_LOG_SEVERITY_WARNING_E, (fmt), \
               ## args)
#define sx_int_log_notice(ilog, fmt, args ...) sx_int_log((ilog), SX_INTERNAL_LOG_SEVERITY_NOTICE_E, (fmt), ## args)
#define sx_int_log_info(ilog, fmt, args ...)   sx_int_log((ilog), SX_INTERNAL_LOG_SEVERITY_INFO_E, (fmt), ## args)
#define sx_int_log_debug(ilog, fmt, args ...)  sx_int_log((ilog), SX_INTERNAL_LOG_SEVERITY_DEBUG_E, (fmt), ## args)

struct sx_internal_log_entry;
struct sx_internal_log {
    struct list_head              list;
    struct sx_internal_log_entry *cyc_buff;
    spinlock_t                    lock;
    u32                           curr_num_of_entries;
    u32                           next;
    u64                           running_counter;
    u32                           max_entries;
    enum sx_internal_log_severity min_severity;
    char                          name[SX_INTERNAL_LOG_MAX_NAME];
};

int sx_internal_log_dev_init(struct sx_dev *dev);
void sx_internal_log_dev_deinit(struct sx_dev *dev);

int sx_internal_log_init(struct sx_internal_log       *ilog,
                         struct sx_dev                *dev,
                         u32                           max_entries,
                         enum sx_internal_log_severity min_severity,
                         const char                   *fmt,
                         ...);
void sx_internal_log_deinit(struct sx_internal_log *ilog,
                            struct sx_dev          *dev);
int sx_internal_log_add(struct sx_internal_log       *ilog,
                        enum sx_internal_log_severity severity,
                        const char                   *fmt,
                        ...);
void sx_internal_log_dump(struct sx_internal_log *ilog,
                          struct seq_file        *m);

#endif /* __SX_INTERNAL_LOG_H__ */
