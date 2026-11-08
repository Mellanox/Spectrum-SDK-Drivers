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

#ifndef SX_CQ_H
#define SX_CQ_H

/************************************************
 * Includes
 ***********************************************/

#include <linux/types.h>
#include "sx.h"

#define CQN_INVALID 255

#define ETH_CRC_LENGTH 4

#define SX_CQ_TIMESTAMP_TYPE(cq) (sx_priv((cq)->dev)->cq_table.ts_type[(cq)->cqn])

#define MON_CQ_HANDLER_THREAD_CPU_UTIL_PERCENT_DEFAULT 20

/************************************************
 * Enums
 ***********************************************/

enum {
    SX_CQE_IS_ERR_MASK   = 0x80,
    SX_CQE_DQN_MASK      = 0x1f,
    SX_CQE_DQN_MSB_MASK  = 0x4000,
    SX_CQE_DQN_MSB_SHIFT = 5,
};

/************************************************
 * Structures
 ***********************************************/

struct sx_cq_context {
    u8     eq_num;   /* 0 or 1 */
    u8     reserved1;
    u8     flags;
    u8     log_cq_size;
    u16    reserved2;
    __be16 producer_counter;
    u16    reserved3;
    u16    reserved4;
    u32    reserved5;
    __be64 dma_addr[8];   /* CQE buffer dma addresses */
};
struct sx_cqe_v0 {
    u8     vlan2_lag_subport;
    u8     lag;
    __be16 system_port_lag_id;
    __be16 wqe_counter;
    __be16 dqn5_byte_count;
    __be16 ulp_crc_vlan_flow;
    __be16 trap_id;
    __be16 reserved1;
    u8     type_swid;
    u8     e_sr_dqn_owner;
};
struct sx_cqe_v1 {
    u8     version_e_sr_packet_ok_rp_lag;
    u8     rp_lag_subport;
    __be16 rp_system_port_lag_id;
    __be16 wqe_counter;
    __be16 isx_ulp_crc_byte_count;
    __be16 vlan;
    __be16 sma_check_id_trap_id;
    __be16 time_stamp;
    u8     type_swid_crc;
    u8     dqn_owner;
};
struct sx_cqe_v2 {
    u8     version_e_sr_packet_ok_rp_lag;
    u8     rp_lag_subport;
    __be16 rp_system_port_lag_id;
    __be16 wqe_counter;
    __be16 isx_ulp_crc_byte_count;
    __be16 vlan_mirror_cong2;
    __be16 sma_check_id_trap_id;
    __be16 time_stamp1;
    u8     type_swid_crc;
    u8     dqn;
    u8     mirror_tclass_mirror_elph_ep_lag;
    u8     ep_lag_subport;
    __be16 ep_system_port_lag_id;
    __be32 mirror_cong1_user_def_val_orig_pkt_len;
    __be32 mirror_reason_time_stamp_type_time_stamp2;
    __be16 mirror_latency2;
    u8     mirror_latency1;
    u8     pe2cqe_token_owner;
};
union sx_cqe {
    struct sx_cqe_v0 *v0;
    struct sx_cqe_v1 *v1;
    struct sx_cqe_v2 *v2;
};

void sx_cq_update_ts(struct sx_cq *cq, enum sx_cq_last_ts last_ts);

/************************************************
 * Globals
 ***********************************************/

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) || defined(CONFIG_SX_RHEL_8_6) || defined(CONFIG_SX_CENTOS_8_4)
/* kernel 5.10 and later do not have mmiowb() anymore. instead, this function is
 * called implicitly by spinlock_unlock() functions. make sure to wrap every place that called mmiowb()
 * with spinlock.
 */
    #define MMIOWB() do { } while(0)
#else
    #define MMIOWB() do { mmiowb(); } while(0)
#endif


/************************************************
 * Inline Functions
 ***********************************************/

static inline void sx_cq_arm(struct sx_cq *cq)
{
    sx_cq_update_ts(cq, SX_CQ_ARM_LAST);

    /* Make sure that descriptors are written before doorbell. */
    wmb();
    __raw_writel((__force u32)cpu_to_be32(cq->cons_index & 0xffff), cq->arm_db);
    MMIOWB();
}

/* Update the Consumer Index */
static inline void sx_cq_set_ci(struct sx_cq *cq)
{
    /* Make sure that descriptors are written before doorbell. */
    wmb();
    __raw_writel((__force u32)cpu_to_be32((cq->cons_index + cq->nent) & 0xffff), cq->set_ci_db);
    MMIOWB();
}

/************************************************
 * Functions
 ***********************************************/
int dispatch_pkt(struct sx_dev *dev, struct completion_info *ci, u16 hw_synd, int dispatch_default);
int sx_core_init_cq_table(struct sx_dev *dev);
void sx_core_destroy_cq_table(struct sx_dev *dev);
int sx_core_create_cq(struct sx_dev *dev, int nent, struct sx_cq **cq, u8 cqn, u8 send);
void sx_core_destroy_cq(struct sx_dev *dev, struct sx_cq *cq, struct sx_dq *dq);
void sx_core_dump_synd_tbl(struct sx_dev *dev);
int rx_skb(struct sx_dev          *dev,
           struct sk_buff         *skb,
           struct completion_info *ci,
           int                     is_from_monitor_rdq,
           struct listener_entry * force_listener,
           u8                      dev_id);

void sx_get_cqe_all_versions(struct sx_cq *cq, uint32_t n, union sx_cqe *cqe_p);
void sx_tx_wqe_sync_for_cpu(struct sx_dq *dq, int idx);
void sx_cq_show_cq(struct sx_dev *dev, int cqn);
void sx_cq_dump_cq(struct sx_dev *dev, int cqn);
void sx_cq_flush_rdq(struct sx_dev *my_dev, int idx);
void sx_init_cq_db_v0(struct sx_cq *tcq, u8 cqn, u8 *cqe_ver);
void sx_init_cq_db_spc(struct sx_cq *tcq, u8 cqn, u8 *cqe_ver);
void sx_init_cq_db_v2(struct sx_cq *tcq, u8 cqn, u8 *cqe_ver);

void sx_cq_pause(struct sx_cq *cq);
void sx_cq_resume(struct sx_cq *cq);
void sx_cq_trigger(struct sx_cq *cq);
void sx_cq_set_rx_monitoring(struct sx_cq *cq, bool is_monitoring);
void sx_cq_check_and_handle_strict_prio(struct sx_cq *cq);
void sx_cq_init_sp_work(struct sx_priv *priv);
void sx_cq_set_thread_param(struct sx_cq *cq, enum sx_cq_thread thread_id, int budget, bool strict_prio);

/* an interface for SGMII/CR-Mode driver */
void sx_fill_cqe_params_from_cqev0(union sx_cqe *u_cqe, struct sx_cqe_params *cqe_params);
void sx_fill_cqe_params_from_cqev2(union sx_cqe *u_cqe, struct sx_cqe_params *cqe_params);

void sx_duplicate_completion_info(const struct completion_info *src, struct completion_info *dst);

#endif /* SX_CQ_H */

/************************************************
 *                  EOF                         *
 ***********************************************/
