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

#ifndef SX_DROP_MONITOR_H
#define SX_DROP_MONITOR_H

#include <net/drop_monitor.h>
#include <linux/mlx_sx/driver.h>
#include "sx.h"
#include "cq.h"

#ifndef SX_DROP_MONITOR_DISABLED
void sx_drop_monitor_destroy(struct sx_dev *dev);

void sx_drop_monitor_handle_monitor_rdq(struct sx_cq           *cq,
                                        int                     cqe_index,
                                        struct completion_info *ci,
                                        struct sk_buff         *skb);

int sx_drop_monitor_set_buffer_drop_params(unsigned long data, struct sx_dev *dev);

void sx_drop_monitor_handle_sw_queue(struct completion_info *comp_info, void *context);

int sx_drop_monitor_set_synd(unsigned int                     cmd,
                             const struct ku_synd_ioctl      *ku,
                             enum l2_type                     listener_type,
                             const union ku_filter_critireas *critireas,
                             struct sx_dev                   *dev);

#else
static inline void sx_drop_monitor_destroy(struct sx_dev *dev)
{
}

static inline void sx_drop_monitor_handle_monitor_rdq(struct sx_cq           *cq,
                                                      int                     cqe_index,
                                                      struct completion_info *ci,
                                                      struct sk_buff         *skb)
{
}

static inline int sx_drop_monitor_set_buffer_drop_params(unsigned long data, struct sx_dev *dev)
{
    return 0;
}

static inline void sx_drop_monitor_handle_sw_queue(struct completion_info *comp_info, void *context)
{
}

static inline int sx_drop_monitor_set_synd(unsigned int                     cmd,
                                           const struct ku_synd_ioctl      *ku,
                                           enum l2_type                     listener_type,
                                           const union ku_filter_critireas *critireas,
                                           struct sx_dev                   *dev)
{
    return 0;
}

#endif /* SX_DROP_MONITOR_DISABLED */

#endif /* SX_DROP_MONITOR_H */
