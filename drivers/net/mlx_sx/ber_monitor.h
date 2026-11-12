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

#ifndef __BER_MONITOR_H__
#define __BER_MONITOR_H__

struct completion_info;

int sx_core_ber_monitor_dev_init(struct sx_dev *dev);
void sx_core_ber_monitor_dev_deinit(struct sx_dev *dev);

void sx_core_ber_monitor_handle_ppbme_event(struct completion_info *ci);
int sx_core_ber_monitor_set_bitmask(struct sx_dev *dev, u16 local_port, u8 bitmask);

int sx_core_ber_monitor_dump(struct seq_file *m, void *v, void *context);

/* The functions below MUST be called under priv->db_lock lock! */
int sx_core_ber_monitor_set_state(struct sx_dev *dev, u16 local_port, u8 state);

#endif /* __BER_MONITOR_H__ */
