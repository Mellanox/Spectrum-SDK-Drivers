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

#ifndef DQ_H_
#define DQ_H_

#define SX_WQE_SHIFT 5
#define SX_DESC_SIZE 32
/************************************************
 * Includes
 ***********************************************/

#include <linux/mlx_sx/device.h>
#include "sx.h"


/************************************************
 * Structs
 ***********************************************/

/* DQ (SDQ/RDQ context) */
struct sx_dq_context {
    u8     cq;
    u8     sdq_tclass;
    u8     reserved1;
    u8     log2_dq_sz;
    u32    reserved2;
    u32    reserved3;
    u32    reserved4;
    __be64 dma_addr[8];   /* DQE buffer dma addresses */
    /* Physical Address of Descriptor Queue page <i> (i=0,1,...,7) */
};

#define DONT_FORCE_SDQ_ID 0xff

/************************************************
 * Functions
 ***********************************************/
struct sx_wqe * sx_get_wqe_by_index(struct sx_dq *dq, int n);

int sx_core_post_recv(struct sx_dq *rdq);
void sx_core_repost_recv(struct sx_dq *rdq);
int sx_core_init_sdq_table(struct sx_dev *dev);
int sx_core_init_rdq_table(struct sx_dev *dev);
int sx_core_create_sdq(struct sx_dev *dev, int nent,
                       u8 dqn, struct sx_dq **sdq_p);
int sx_core_create_rdq(struct sx_dev *dev, int nent,
                       u8 dqn, struct sx_dq **rdq_p);

void sx_core_destroy_sdq_table(struct sx_dev *dev, u8 free_table);
void sx_core_destroy_rdq_table(struct sx_dev *dev, u8 free_table);
void sx_core_destroy_sdq(struct sx_dev *dev, struct sx_dq *dq);
void sx_core_destroy_rdq(struct sx_dev *dev, struct sx_dq *dq);
void sx_dq_notify_doorbell(struct sx_dq *dq);
int sx_add_pkts_to_sdq(struct sx_dq *sdq);
int sx_hw2sw_dq(struct sx_dev *dev, struct sx_dq *dq);
int sx_dq_modify_2err(struct sx_dev *dev, struct sx_dq *dq);
int sx_flush_dq(struct sx_dev *dev, struct sx_dq *dq, bool update_flushing_state);

int sx_core_add_rdq_to_monitor_rdq_list(struct sx_dq *dq);
void sx_core_del_rdq_from_monitor_rdq_list(struct sx_dq *dq);
void sx_dq_update_occupancy(struct sx_dq *dq);
int sx_core_rdq_usage_monitor_init(struct sx_dev *dev);
void sx_core_rdq_usage_monitor_deinit(struct sx_dev *dev);

#define SX_RDQ_TRACEPOINT_F_ENABLED          (1 << 0)
#define SX_RDQ_TRACEPOINT_F_FILTER_INSTALLED (1 << 1)
#define SX_RDQ_TRACEPOINT_F_DROP_SKB         (1 << 2)
u8 sx_run_rdq_tracepoint(struct sx_priv         *priv,
                         struct sx_dq           *rdq,
                         struct sx_cqe_params   *cqe_params,
                         struct sk_buff         *skb,
                         u16                     rdq_max_buff_size,
                         struct sx_rx_timestamp *rx_timestamp);
int sx_dq_info_dump(struct seq_file *m, void *v, void *context);

#endif /* DQ_H_ */
/************************************************
 *                  EOF                         *
 ***********************************************/
