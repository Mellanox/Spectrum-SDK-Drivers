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

#ifndef __SX_RX_BUFFERS__
#define __SX_RX_BUFFERS__

#include "sx.h"

int sx_rx_buffers_init(struct sx_dq *rdq);
void sx_rx_buffers_deinit(struct sx_dq *rdq);

void sx_rx_buffers_put(struct sx_dq *rdq, struct sx_rx_sge_data *rx_sge_data);

struct sk_buff * sx_rx_buffers_build_skb(struct sx_dq *rdq, int wqe_index, u16 total_byte_count);
int sx_rx_buffers_prepare_wqe(struct sx_dq *rdq, int wqe_index);

void sx_rx_buffers_config_dump(struct seq_file *m);

#endif /* __SX_RX_BUFFERS__ */
