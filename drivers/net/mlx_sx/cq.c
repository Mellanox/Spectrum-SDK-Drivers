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

/************************************************
 * Includes
 ***********************************************/

#include <linux/sched.h>
#include <linux/skbuff.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/hardirq.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/if_vlan.h>
#include <linux/filter.h>
#include <linux/mlx_sx/device.h>
#include <linux/mlx_sx/driver.h>
#include <linux/mlx_sx/cmd.h>
#include <linux/mlx_sx/kernel_user.h>
#include <linux/workqueue.h>
#include <linux/vmalloc.h>
#include <linux/mlx_sx/skb_hook.h>
#include <linux/ktime.h>
#include "sx.h"
#include "cq.h"
#include "dq.h"
#include "ib.h"
#include "alloc.h"
#include "sx_proc.h"
#include "sx_clock.h"
#include "ptp.h"
#include "sgmii.h"
#include "bulk_cntr_event.h"
#include "ber_monitor.h"
#include "drop_monitor.h"
#include "sx_af_counters.h"
#include "health_check.h"
#include "rx_buffers.h"

#include <linux/module.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/mlx_sx/auto_registers/cmd_auto.h>
#include <linux/mlx_sx/auto_registers/reg.h>
/************************************************
 *  Definitions
 ***********************************************/
#define IB_CRC_LENGTH  (6) /* CRC length of all IB packets except IB_Raw */
#define IB_VCRC_LENGTH (2) /* CRC length of IB_Raw packets */

#define HETT_TRAP_ID                 269
#define MAX_MATCHING_LISTENERS       100
#define SX_LATE_COMPLETION_THRESHOLD (3 * HZ) /* completion later than 3 seconds means 'late completion' */
#define MONITOR_RDQ_BUDGET           10
#define DEFAULT_BUDGET               64

static inline int PORT_VLAN_BIT_IS_SET(u64* arr, u64 num)
{
    u64 tmp;

    tmp = (arr)[(num) / 64];
    return tmp & (1ULL << ((num) % 64)) ? 1 : 0;
}

/************************************************
 * Globals
 ***********************************************/

extern int rx_debug;
extern int rx_debug_metadata;
extern int rx_debug_pkt_type;
extern int rx_debug_emad_type;
extern int rx_cqev2_dbg;
extern int rx_dump;
extern int rx_dump_cnt;
extern int rx_debug_rarftbr_event;
extern int enable_keep_packet_crc;
extern int mon_cq_thread_cpu_util_percent;

struct handler_entry {
    cq_handler handler;
    void      *context;
};

union sx_cq_poll_context {
    struct {
        /* empty for now */
    } tx;
    struct {
        struct sx_rx_timestamp linux_ts;
        u16                    rdq_max_buff_size;
        u64                    ktime_before; /* WJH only */
        int                    handled; /* WJH only */
    } rx;
};

struct sx_pause_resume_work {
    struct work_struct work;
    struct sx_cq      *cq;
};

/************************************************
 *  Functions
 ***********************************************/

static void __sx_cq_dec_refcnt(struct sx_cq *cq)
{
    /*
     * decrement the reference count and if we reach 0, signal the 'free' completion
     * that tells the CQ is no longer in use and can be deleted
     */
    if (atomic_dec_and_test(&cq->refcount)) {
        complete(&cq->free);
    }
}

static void __print_cqe_params(const struct sx_dev *dev, const struct sx_cqe_params *cqe_params)
{
    sxd_log_debug(dev,
                  "CQE_PARAMS: trap_id=%u, is_err=%u, is_send=%u, dqn=%u, wqe_counter=%u, is_isx=%u, is_crc=%u, byte_count=%u,"
                  "user_def_val=%u, is_lag=%u, lag_support=%u, sysport_lag_id=%u, mirror_reason=%u, dest_sysport_lag_id=%u, "
                  "dest_is_lag=%u, dest_lag_subport=%u, mirror_tclass=%u, mirror_cong=%u, mirror_latency=%u, mirror_elephant=%u, "
                  "cqe_ts=%lu.%09lu (type=%u), pkt_type=%u\n",
                  cqe_params->trap_id,
                  cqe_params->is_err,
                  cqe_params->is_send,
                  cqe_params->dqn,
                  cqe_params->wqe_counter,
                  cqe_params->is_isx,
                  cqe_params->is_crc,
                  cqe_params->byte_count,
                  cqe_params->user_def_val,
                  cqe_params->is_lag,
                  cqe_params->lag_subport,
                  cqe_params->sysport_lag_id,
                  cqe_params->mirror_reason,
                  cqe_params->dest_sysport_lag_id,
                  cqe_params->dest_is_lag,
                  cqe_params->dest_lag_subport,
                  cqe_params->mirror_tclass,
                  cqe_params->mirror_cong,
                  cqe_params->mirror_lantency,
                  cqe_params->mirror_elephant,
                  (unsigned long)cqe_params->cqe_ts.timestamp.tv_sec,
                  (unsigned long)cqe_params->cqe_ts.timestamp.tv_nsec,
                  cqe_params->cqe_ts.ts_type,
                  cqe_params->pkt_type);
}


static u16 get_truncate_size_from_db(struct sx_dev *dev, int dqn)
{
    u16           ret = 0;
    unsigned long flags;

    spin_lock_irqsave(&sx_priv(dev)->db_lock, flags);
    ret = sx_priv(dev)->truncate_size_db[dqn];
    spin_unlock_irqrestore(&sx_priv(dev)->db_lock, flags);

    return ret;
}

/* Returns 1 if the port/lag id is found in the trap filter DB and the packet should be dropped */
static u8 check_trap_port_in_filter_db(struct sx_dev *dev, u16 hw_synd, u8 is_lag, u16 sysport_lag_id)
{
    struct sx_priv *priv = sx_priv(dev);
    int             i;
    u8              ret = 0;
    unsigned long   flags;

    if (is_lag) {
        spin_lock_irqsave(&priv->db_lock, flags);
        for (i = 0; i < MAX_LAG_PORTS_IN_FILTER; i++) {
            if (priv->lag_filter_db[hw_synd][i] == sysport_lag_id) {
                inc_filtered_lag_packets_counter(priv);
                ret = 1;
                break;
            }
        }

        spin_unlock_irqrestore(&priv->db_lock, flags);
        return ret;
    }

    /* EMADs can be received with sysport==0 */
    if (sysport_lag_id == 0) {
        return 0;
    }

    spin_lock_irqsave(&priv->db_lock, flags);
    for (i = 0; i < MAX_SYSTEM_PORTS_IN_FILTER; i++) {
        if (priv->sysport_filter_db[hw_synd][i] == sysport_lag_id) {
            inc_filtered_port_packets_counter(priv);
            ret = 1;
            break;
        }
    }

    spin_unlock_irqrestore(&priv->db_lock, flags);
    return ret;
}

struct sx_cpu_port_loopback_data {
    struct sx_priv *priv;
    u32             rx_port_idx;
};

static void __sx_cpu_port_loopback_skb_dtor(struct sk_buff *skb)
{
    struct sx_cpu_port_loopback_data *data = (struct sx_cpu_port_loopback_data*)skb->cb;

    atomic_dec(&data->priv->debug_only.tx_packets_in_flight);
}

#define SX_MAX_CPU_PORT_LOOPBACK_TX_PACKET_IN_FLIGHT (900) /* SDQ size is 1024, we fill up to ~90% */
static void sx_cpu_port_loopback(struct completion_info *ci)
{
    struct isx_meta                   meta;
    struct sk_buff                   *skb = ci->skb;
    struct sk_buff                   *tmp_skb = NULL;
    struct sx_priv                   *priv = sx_priv(ci->dev);
    int                               err = 0;
    struct sx_cpu_port_loopback_data *data = NULL;
    u32                               rx_port_idx = ci->cqe_params.sysport_lag_id - 1;

    /* to prevent starvation from some ports, each port that send a packet will set its bit in the bitmap.
     * if the bit is set, we'll skip this round. if not, we'll send the packet and set the bit.
     * the SKB destructor (upon SDQ completion) will clear the bit to enable another transmission.
     */
    if (atomic_read(&priv->debug_only.tx_packets_in_flight) >= SX_MAX_CPU_PORT_LOOPBACK_TX_PACKET_IN_FLIGHT) {
        goto out;
    }

    memset(&meta, 0, sizeof(meta));
    meta.dev_id = ci->dev_id;
    meta.type = SX_PKT_TYPE_ETH_CTL_UC;

    /* No LAG support */
    meta.system_port_mid = ci->cqe_params.sysport_lag_id;

    tmp_skb = skb_copy(skb, GFP_KERNEL);
    if (!tmp_skb) {
        sxd_log_rl_info(SXD_DEV_FROM_COMP_INFO(ci), "%s: fail to alloc_skb\n", __func__);
        err = -ENOMEM;
        goto out;
    }

    tmp_skb->destructor = __sx_cpu_port_loopback_skb_dtor;
    data = (struct sx_cpu_port_loopback_data*)tmp_skb->cb;
    data->priv = priv;
    data->rx_port_idx = rx_port_idx;

    err = sx_core_post_send(ci->dev, tmp_skb, &meta);
    if (err) {
        sxd_log_rl_info(SXD_DEV_FROM_COMP_INFO(ci), "%s: sx_core_post_send FAILED\n", __func__);
        goto out;
    }

    priv->stats.tx_loopback_ok_by_synd[ci->cqe_params.trap_id]++;
    atomic_inc(&priv->debug_only.tx_packets_in_flight);

out:
    if (err) {
        priv->stats.tx_loopback_dropped_by_synd[ci->cqe_params.trap_id]++;
    }

    /* Free memory */
    sx_skb_free(skb);
}

static void sx_handle_ppbme_event(struct completion_info *ci)
{
    if (!sx_validate_emad_format(ci, PPBME_REG_ID, 4, 0)) {
        return;
    }

    sx_core_ber_monitor_handle_ppbme_event(ci);
}

static inline void sx_reg_sbctr_tclass_set(void *reg_data, u64 tc_vec)
{
    struct sxd_emad_sbctr_reg* sbctr = (struct sxd_emad_sbctr_reg *)reg_data;

    sbctr->tc_vec = cpu_to_be64(tc_vec);
}

static void sx_handle_sbctr_event(struct completion_info *ci)
{
    struct ku_sbctr_reg sbctr_reg;
    u8                 *sbctr_outbox = (u8*)ci->skb->data + sizeof(struct sx_emad) +
                                       sizeof(struct sxd_emad_tlv_reg);
    u64             old_tc_vec = 0, tc_vec = 0, new_tc_vec = 0;
    u8              entity = 0, fp = 0, maybe_flag = 0, port_state = 0;
    u8              dir_ing = 0;
    u16             local_port = 0;
    struct sx_priv *priv = sx_priv((struct sx_dev *)ci->dev);
    unsigned long   flags;

    if (!sx_validate_emad_format(ci, SBCTR_REG_ID, 4, 0)) {
        return;
    }

    __SBCTR_decode(sbctr_outbox, &sbctr_reg, NULL);

    SX_PORT_BUILD_PHY_ID_FROM_LSB_MSB(local_port,
                                      sbctr_reg.local_port,
                                      sbctr_reg.lp_msb);

    if (local_port > MAX_PHYPORT_NUM) {
        sxd_log_err(SXD_DEV_FROM_COMP_INFO(ci), "Received local_port %d is invalid (max. %d) \n",
                    sbctr_reg.local_port, MAX_PHYPORT_NUM);
        return;
    }

    dir_ing = sbctr_reg.dir_ing;
    entity = sbctr_reg.entity;
    fp = sbctr_reg.fp;
    tc_vec = (((u64)sbctr_reg.tclass_vector_high << 32) | sbctr_reg.tclass_vector_low);

    spin_lock_irqsave(&priv->db_lock, flags);
    if (entity == 0) { /* entity is port tc */
        /* Update DB only upon port_tc event */
        old_tc_vec = priv->tele_thrs_tc_vec[local_port][dir_ing];
        priv->tele_thrs_tc_vec[local_port][dir_ing] = tc_vec;
        port_state = priv->tele_thrs_state[local_port][dir_ing];

        if (tc_vec == old_tc_vec) {
            /* No change in tc_vec state */
            if (tc_vec != 0) {
                /* If port is congested: drop event */
                ci->info.dont_care.drop_enable = 1;
            } else {
                /* No congestion - keep event in those cases:
                 * 1. This is the first event on port
                 * 2. Previous event was "port maybe congested" event */
                if ((SX_TELE_THRS_MAYBE_GET(port_state) == 1) || (SX_TELE_THRS_FIRST_EVENT_GET(port_state) == 0)) {
                    ci->info.dont_care.drop_enable = 0;
                } else {
                    ci->info.dont_care.drop_enable = 1;
                }
            }
        } else {
            /* Set new tc_vec:
             * High 32 bits marks the changed TCs
             * Low 32 bits keeps TCs value
             * Valid bits [0..15] */
            new_tc_vec = ((old_tc_vec ^ tc_vec) << 32) | (tc_vec & 0xFFFF);

            /* re-write the event */
            sx_reg_sbctr_tclass_set(ci->skb->data, new_tc_vec);
        }
    } else if ((entity == 1) && (fp == 1)) {
        /* Port maybe congested event */
        maybe_flag = 1;
    }

    /* Update port state field */
    if (maybe_flag) {
        SX_TELE_THRS_MAYBE_SET(priv->tele_thrs_state[local_port][dir_ing]);
    } else {
        SX_TELE_THRS_MAYBE_CLR(priv->tele_thrs_state[local_port][dir_ing]);
    }
    SX_TELE_THRS_FIRST_EVENT_SET(priv->tele_thrs_state[local_port][dir_ing]);
    spin_unlock_irqrestore(&priv->db_lock, flags);
}

static void sx_handle_fshe_event(struct completion_info *ci)
{
    struct ku_fshe_reg fshe_reg;
    int                err;
    u8               * fshe_outbox = (u8*)ci->skb->data + sizeof(struct sx_emad) +
                                     sizeof(struct sxd_emad_tlv_reg);

    err = __FSHE_decode(fshe_outbox, &fshe_reg, NULL);
    if (err) {
        sxd_log_err(SXD_DEV_FROM_COMP_INFO(ci), "__FSHE_decode returned with error %d\n", err);
        return;
    }
    sx_health_report_error_fshe(ci->dev, &fshe_reg);
}

static void sx_handle_meccc_event(struct completion_info *ci)
{
    struct ku_meccc_reg meccc_reg;
    int                 err;
    u8                * meccc_outbox = (u8*)ci->skb->data + sizeof(struct sx_emad) +
                                       sizeof(struct sxd_emad_tlv_reg);

    err = __MECCC_decode(meccc_outbox, &meccc_reg, NULL);
    if (err) {
        sxd_log_err(SXD_DEV_FROM_COMP_INFO(ci), "__MECCC_decode returned with error %d\n", err);
        return;
    }
    sx_health_report_error_meccc(ci->dev, &meccc_reg);
}

static void sx_handle_mfde_event(struct completion_info *ci)
{
    struct ku_mfde_reg mfde_reg;
    int                err;
    u8               * mfde_outbox = (u8*)ci->skb->data + sizeof(struct sx_emad) +
                                     sizeof(struct sxd_emad_tlv_reg);

    err = __MFDE_decode(mfde_outbox, &mfde_reg, NULL);
    if (err) {
        sxd_log_err(SXD_DEV_FROM_COMP_INFO(ci), "__MFDE_decode returned with error %d\n", err);
        return;
    }
    sx_health_report_error_mfde(ci->dev, &mfde_reg);
}

static void sx_parse_event(struct completion_info *ci)
{
    ci->info.dont_care.drop_enable = 0;

    switch (ci->cqe_params.trap_id) {
    case SXD_TRAP_ID_PPCNT:
        sx_bulk_cntr_handle_ppcnt(ci);
        break;

    case SXD_TRAP_ID_MGPCB:
        sx_bulk_cntr_handle_mgpcb(ci);
        break;

    case SXD_TRAP_ID_MOFRB:
        sx_bulk_cntr_handle_mofrb(ci);
        break;

    case SXD_TRAP_ID_PBSR:
        sx_bulk_cntr_handle_pbsr(ci);
        break;

    case SXD_TRAP_ID_SBSRD:
        sx_bulk_cntr_handle_sbsrd(ci);
        break;

    case SXD_TRAP_ID_CEER:
        sx_bulk_cntr_handle_ceer(ci);
        break;

    case SXD_TRAP_ID_MOCS_DONE:
        if (rx_debug_rarftbr_event) {
            sxd_log_debug(SXD_DEV_FROM_COMP_INFO(ci), "MOCS DONE trap received \n");
        }
        sx_bulk_cntr_handle_mocs_done(ci);
        break;

    case SXD_TRAP_ID_PPBME:
        sx_handle_ppbme_event(ci);
        break;

    case SXD_TRAP_ID_SB_CONG_TX_PORT:
        sx_handle_sbctr_event(ci);
        break;

    case SXD_TRAP_ID_MAFBI:
    /* fall-through */
    case SXD_TRAP_ID_MAFRI:
    /* fall-through */
    case SXD_TRAP_ID_ACCU_FLOW_INC:
        sx_af_counters_event_job_schedule(ci);
        break;

    case SXD_TRAP_ID_MTPPST:
        sx_core_ptp_handle_mtppst_event(ci);
        break;

    case SXD_TRAP_ID_FSED:
        sx_bulk_cntr_handle_fsed(ci);
        break;

    case SXD_TRAP_ID_MFDE:
        sx_handle_mfde_event(ci);
        break;

    case SXD_TRAP_ID_FSHE:
        sx_handle_fshe_event(ci);
        break;

    case SXD_TRAP_ID_MECCC:
        sx_handle_meccc_event(ci);
        break;

    case SXD_TRAP_ID_MOFTD:
        sx_bulk_cntr_handle_moftd(ci);
        break;

    case SXD_TRAP_ID_RARFTBR:
        if (rx_debug_rarftbr_event) {
            sxd_log_debug(SXD_DEV_FROM_COMP_INFO(ci), "RARFTBR trap received \n");
        }
        sx_bulk_cntr_handle_rarftbr(ci);
        break;

    case SXD_TRAP_ID_MCIA:
        sx_handle_mcia_event(ci);
        break;

    default:
        break;
    }
}

static u16 get_vid_from_db(struct sx_dev *dev, u8 is_lag, u16 sysport_lag_id)
{
    u16           ret = 1;
    unsigned long flags;

    spin_lock_irqsave(&sx_priv(dev)->db_lock, flags);
    if (is_lag) {
        ret = sx_priv(dev)->pvid_lag_db[sysport_lag_id];
    } else {
        ret = sx_priv(dev)->pvid_sysport_db[sysport_lag_id];
    }

    spin_unlock_irqrestore(&sx_priv(dev)->db_lock, flags);

    return ret;
}

static u32 get_qpn(u16 hw_synd, struct sk_buff *skb)
{
    u8  lnh = 0;
    u32 qpn;

    lnh = ((struct ib_header_lrh *)skb->data)->sl_lnh & 0x3;
    if (lnh == 3) {
        qpn = be32_to_cpu(((struct ib_header_multicast *)
                           skb->data)->bth.dest_qp) & 0xffffff;
    } else if (lnh == 2) {
        qpn = be32_to_cpu(((struct ib_header_unicast *)
                           skb->data)->bth.dest_qp) & 0xffffff;
    } else {
        qpn = 0xffffffff;
    }

    return qpn;
}

void sx_duplicate_completion_info(const struct completion_info *src, struct completion_info *dst)
{
    memcpy(dst, src, sizeof(*dst));

    if (src->active_timestamp == &src->cqe_params.cqe_ts) {
        dst->active_timestamp = &dst->cqe_params.cqe_ts;
    } else if (src->active_timestamp == &src->rx_linux_timestamp) {
        dst->active_timestamp = &dst->rx_linux_timestamp;
    } else {
        WARN_ON_ONCE(src->active_timestamp != NULL);
        dst->active_timestamp = NULL;
    }
}

static int listener_register_filter_entry_match_dispatch(struct completion_info                *ci,
                                                         struct listener_register_filter_entry *listener_register_filter)
{
    if ((listener_register_filter->is_global_filter) ||
        PORT_VLAN_BIT_IS_SET(listener_register_filter->ports_filters, ci->cqe_params.sysport_lag_id) ||
        ((ci->cqe_params.is_lag) &&
         PORT_VLAN_BIT_IS_SET(listener_register_filter->lags_filters, ci->cqe_params.sysport_lag_id)) ||
        (PORT_VLAN_BIT_IS_SET(listener_register_filter->vlans_filters, ci->vid))) {
        return 0;
    }
    if ((listener_register_filter->is_global_register) ||
        (PORT_VLAN_BIT_IS_SET(listener_register_filter->ports_registers, ci->cqe_params.sysport_lag_id) ||
         ((ci->cqe_params.is_lag) &&
          PORT_VLAN_BIT_IS_SET(listener_register_filter->lags_registers, ci->cqe_params.sysport_lag_id)) ||
         (PORT_VLAN_BIT_IS_SET(listener_register_filter->vlans_registers, ci->vid)))) {
        return 1;
    }
    return 0;
}

static int is_matching(struct completion_info *ci, struct listener_entry *listener)
{
    if ((listener->swid != ci->swid) &&
        (listener->swid != SWID_NUM_DONT_CARE)) {
        return 0;
    }
    /* If the packet came from a user (loopback), don't return it to the same user */
    if ((ci->context != NULL) && (listener->context == ci->context)) {
        return 0;
    }

    /* in case we want to send the trap to a certain process, other processes won't get it */
    if ((ci->target_pid != TARGET_PID_DONT_CARE) && (listener->client_pid.gpid != ci->target_pid)) {
        return 0;
    }

    switch (listener->listener_type) {
    case L2_TYPE_DONT_CARE:
        if (listener->critireas.dont_care.drop_enable && ci->info.dont_care.drop_enable) {
            break;
        }
        return 1;

    case L2_TYPE_ETH:
        if ((ci->cqe_params.pkt_type != PKT_TYPE_ETH) &&
            (ci->cqe_params.pkt_type != PKT_TYPE_FCoETH)) {
            break;
        }

        if ((ci->info.eth.ethtype != listener->critireas.eth.ethtype) &&
            (listener->critireas.eth.ethtype !=
             ETHTYPE_DONT_CARE_VALUE)) {
            break;
        }

        if ((ci->info.eth.dmac != listener->critireas.eth.dmac)
            && (listener->critireas.eth.dmac !=
                DMAC_DONT_CARE_VALUE)) {
            break;
        }

        if ((listener->critireas.eth.emad_tid != TID_DONT_CARE_VALUE) &&
            (ci->info.eth.emad_tid !=
             listener->critireas.eth.emad_tid)) {
            break;
        }

        if ((listener->critireas.eth.from_rp != IS_RP_DONT_CARE_E) &&
            (ci->info.eth.from_rp != listener->critireas.eth.from_rp)) {
            break;
        }

        if ((listener->critireas.eth.from_bridge != IS_BRIDGE_DONT_CARE_E) &&
            (ci->info.eth.from_bridge != listener->critireas.eth.from_bridge)) {
            break;
        }
        return 1;

    case L2_TYPE_IB:
        if ((ci->cqe_params.pkt_type == PKT_TYPE_IB_Raw) ||
            (ci->cqe_params.pkt_type == PKT_TYPE_IB_non_Raw) ||
            (ci->cqe_params.pkt_type == PKT_TYPE_FCoIB) ||
            (ci->cqe_params.pkt_type == PKT_TYPE_ETHoIB)) {
            if ((ci->info.ib.is_oob_originated_mad != listener->critireas.ib.is_oob_originated_mad) &&
                (listener->critireas.ib.is_oob_originated_mad != 255) /* don't care */) {
                break;
            }
            if ((ci->info.ib.qpn == listener->critireas.ib.qpn) ||
                (listener->critireas.ib.qpn == QPN_DONT_CARE_VALUE)) {
                return 1;
            }
        }
        break;

    default:
        break;
    }

    return 0;
}

/*
 * filter the listeners table, and call all relevant handlers
 */
int dispatch_pkt(struct sx_dev *dev, struct completion_info *ci, u16 entry, int dispatch_default)
{
    struct sx_priv               *priv = sx_priv(dev);
    struct listeners_and_rf_info *info = NULL;
    struct listener_entry        *listener;
    cq_handler                    listener_handler;    /*The completion handler*/
    void                         *listener_context;    /*to pass to the handler*/
    int                           num_found = 0;
    u8                            is_default;

    /* validate the syndrome range */
    if (entry > NUM_HW_SYNDROMES) {
        sxd_log_err(dev, "Error: arrived synd %d is out of range (1..%d) \n",
                    entry, NUM_HW_SYNDROMES);
        return -1;
    }

    info = priv->listeners_and_rf_db.info;

    if (!info) {
        goto out;
    }

    rcu_read_lock();

    /* Checking syndrome registration and NUM_HW_SYNDROMES callback iff dispatch_default set */
    /* I don't like the syndrome dispatchers at all, but it's too late to change */
    while (1) {
        list_for_each_entry_rcu(listener, &info->per_synd_list[entry], list) {
            if (listener->is_default && (num_found == 0)) {
                is_default = 1;
            } else {
                is_default = 0;
            }
            if (listener_register_filter_entry_match_dispatch(ci, &(listener->listener_register_filter))) {
                if (is_default || is_matching(ci, listener)) {
                    listener_handler = listener->handler;
                    listener_context = listener->context;
                    listener_handler(ci, listener_context);
                    listener->rx_pkts++;
                    ++num_found;
                }
            }
        }

        if (!dispatch_default || (entry == NUM_HW_SYNDROMES)) {
            break;
        }
        entry = NUM_HW_SYNDROMES;
    }

    rcu_read_unlock();

    if (num_found == 0) {
        inc_unconsumed_packets_counter(priv, ci->cqe_params.trap_id, ci->cqe_params.pkt_type);
    }

out:
    return num_found;
}

#ifdef SW_PUDE_EMULATION
/* PUDE WA for NOS (PUDE events are handled by SDK). Needed for BU. */
static int __verify_hw_synd(struct sx_priv *priv, struct completion_info *ci)
{
    int                       err = 0;
    struct sxd_emad_pude_reg* pude = NULL;

    if (!priv || !ci) {
        err = -EINVAL;
        goto out;
    }

    if (ci->hw_synd == SXD_TRAP_ID_PUDE) {
        pude = (struct sxd_emad_pude_reg *)ci->skb->data;

        if ((pude->oper_status == PORT_OPER_STATUS_UP) &&
            (priv->admin_status_sysport_db[pude->local_port] == 0x2)) { /* SXD_PORT_ADMIN_STATUS_DOWN_BY_CONF = 0x2 */
            sxd_log_debug(SXD_DEV_FROM_COMP_INFO(ci),
                          "Skipping PUDE [%d] event for port [%d] - admin status: %d \n",
                          pude->oper_status,
                          pude->local_port,
                          priv->admin_status_sysport_db[pude->local_port]);
            err = -EBADMSG;
            goto out;
        }
    }

out:
    return err;
}
#endif /* SW_PUDE_EMULATION */

static void __print_rx_packet(struct completion_info *ci)
{
    const struct ethhdr *eth_h = (struct ethhdr*)(ci->skb->data);
    char                 reg_id_str[20] = "";
    u16                  reg_id;
    struct sx_priv      *priv = sx_priv(ci->dev);

    /* if packet's trap-id is not interesting, go out */
    if ((rx_debug_pkt_type != SX_DBG_PACKET_TYPE_ANY) && (rx_debug_pkt_type != ci->cqe_params.trap_id)) {
        return;
    }

    /* if packet holds a PRM register (EMAD, PUDE, etc) and register-id is not interesting, go out */
    if (be16_to_cpu(eth_h->h_proto) == ETHTYPE_EMAD) {
        reg_id = be16_to_cpu(((struct sx_emad *)ci->skb->data)->emad_op.register_id);

        if ((rx_debug_emad_type != SX_DBG_EMAD_TYPE_ANY) && (rx_debug_emad_type != reg_id)) {
            return;
        }

        snprintf(reg_id_str, sizeof(reg_id_str) - 1, " (reg_id=0x%04x)", reg_id);
    }

    if (rx_cqev2_dbg) {
        __print_cqe_params(ci->dev, &ci->cqe_params);
    }

    if (rx_debug_metadata) {
        sxd_log_debug(SXD_DEV_FROM_COMP_INFO(ci),
                      "RX_META: swid=%d, sysport=%d, hw_synd=%d%s, skb_len=%d, pkt_type=%d, byte_count=%d, "
                      "is_lag=%d, src_lag_subport=%d, dest sysport=%#x, dest_is_lag=%d, dest_lag_subport=%d, "
                      "user_def_val=%#08x, mirror_tclass=%d, mirror_cong=%d, mirror_latency=%d, mirror_reason=%d\n",
                      ci->swid,
                      ci->cqe_params.sysport_lag_id,
                      ci->cqe_params.trap_id,
                      reg_id_str,
                      ci->skb->len,
                      ci->cqe_params.pkt_type,
                      ci->skb->len,
                      ci->cqe_params.is_lag,
                      ci->cqe_params.lag_subport,
                      ci->cqe_params.dest_sysport_lag_id,
                      ci->cqe_params.dest_is_lag,
                      ci->cqe_params.dest_lag_subport,
                      ci->cqe_params.user_def_val,
                      ci->cqe_params.mirror_tclass,
                      ci->cqe_params.mirror_cong,
                      ci->cqe_params.mirror_lantency,
                      ci->cqe_params.mirror_reason);
    }

    if (rx_dump) {
        print_hex_dump_debug("RX-SKB    ",
                             DUMP_PREFIX_OFFSET,
                             16,
                             1,
                             ci->skb->data,
                             ci->skb->len,
                             false);
    }

    if ((rx_dump_cnt != SX_DBG_COUNT_UNLIMITED) && (rx_dump_cnt > 0)) {
        rx_dump_cnt--;
    }

    if (rx_dump_cnt == 0) {
        rx_dump = 0;
        MODULE_PARAM_INSTANCE_SET(priv, rx_debug, 0);
        rx_debug_metadata = 0;
        rx_debug_pkt_type = 0xFF;
    }
}

/*
 * extracts the needed data from the cqe and from the packet, calls
 * the filter listeners with that info
 */
int rx_skb(struct sx_dev          *dev,
           struct sk_buff         *skb,
           struct completion_info *ci,
           int                     is_from_monitor_rdq,
           struct listener_entry * force_listener,
           u8                      dev_id)
{
    struct sx_priv *priv = sx_priv(dev);
    int             err = 0;
    u8              cqn = 0;
    u16             truncate_size = 0;
    u8              remove_crc = 0;
    u8              swid = 0;
    u8              is_from_rp = IS_RP_DONT_CARE_E;
    u16             fid = 0;
    u16             mad_attr_id;
    u8              counter_swid_index;

    if (ci->cqe_params.trap_id == HETT_TRAP_ID) {
        goto out; /* we don't want to count HETT_TRAP_ID */
    }

    remove_crc = (ci->cqe_params.is_crc && !MODULE_PARAM_INSTANCE_GET(priv, enable_keep_packet_crc));
    cqn = ci->cqe_params.dqn + NUMBER_OF_SDQS;

    if (ci->cqe_params.is_isx) {
        skb->data += ISX_HDR_SIZE;
        skb->len -= ISX_HDR_SIZE;
    }

    ci->skb = skb;
    ci->context = NULL;
    ci->dev = dev;
    ci->dev_id = dev_id;

#ifdef SW_PUDE_EMULATION
    /* PUDE WA for NOS (PUDE events are handled by SDK). Needed for BU. */
    if (__verify_hw_synd(priv, ci) != 0) {
        goto out;
    }
#endif /* SW_PUDE_EMULATION */

    /* If packet arrived from external port then port != 0 or is_lag != 0 */
    if (SX_CQE_RX_FROM_NETWORK_PORT(&ci->cqe_params)) {
        err = sx_core_get_swid(dev, ci, &swid);
        if (err) {
            err = -EINVAL;
            goto out;
        }

        if (swid == 255) { /* port is in migration between 2 swids */
            if (MODULE_PARAM_INSTANCE_GET(priv, rx_debug)) {
                sxd_log_err(dev,
                            "port (sysport=%u, is_lag=%u) is temporarily disabled! dropping packet!\n",
                            ci->cqe_params.sysport_lag_id,
                            ci->cqe_params.is_lag);
            }

            err = -EINVAL;
            goto out;
        }

        ci->swid = swid;
    } else {
        ci->swid = 0;
    }

    sx_core_skb_hook_rx_call(dev, skb);

    if (MODULE_PARAM_INSTANCE_GET(priv, rx_debug)) {
        __print_rx_packet(ci);
    }

    counter_swid_index = (ci->swid < NUMBER_OF_SWIDS) ? ci->swid : NUMBER_OF_SWIDS;
    priv->stats.rx_by_pkt_type[counter_swid_index][ci->cqe_params.pkt_type]++;
    priv->stats.rx_by_synd[counter_swid_index][ci->cqe_params.trap_id]++;
    priv->stats.rx_by_synd_bytes[counter_swid_index][ci->cqe_params.trap_id] += skb->len;
    priv->stats.rx_by_rdq[counter_swid_index][ci->cqe_params.dqn]++;
    priv->stats.rx_by_rdq_bytes[counter_swid_index][ci->cqe_params.dqn] += skb->len;

    /* put the original packet size before truncation in the completion info */
    ci->original_packet_size = skb->len;

    truncate_size = get_truncate_size_from_db(dev, ci->cqe_params.dqn);
    if ((truncate_size > 0) && (truncate_size < skb->len)) {
        skb->len = truncate_size;
    }

    if (remove_crc &&
        (ci->cqe_params.pkt_type != PKT_TYPE_IB_Raw) &&
        (ci->cqe_params.pkt_type != PKT_TYPE_IB_non_Raw)) {
        ci->original_packet_size -= ETH_CRC_LENGTH;

        if (!truncate_size) {
            skb->len -= ETH_CRC_LENGTH;
        }
    }

    if (unlikely(priv->debug_only.enable_cpu_port_loopback) && SX_CQE_RX_FROM_NETWORK_PORT(&ci->cqe_params)) {
        /* it's a DEBUG feature */
        sx_cpu_port_loopback(ci);
        return 0;
    }

    /* Check if the port/lag is in the traps filter DB. If so we silently drop the packet */
    if (check_trap_port_in_filter_db(dev, ci->cqe_params.trap_id, ci->cqe_params.is_lag,
                                     ci->cqe_params.sysport_lag_id)) {
        goto out;
    }

    /* Some events need to be silently drop in driver */
    sx_parse_event(ci);

    switch (ci->cqe_params.pkt_type) {
    case PKT_TYPE_ETH:
    case PKT_TYPE_FCoETH:
        ci->info.eth.ethtype = be16_to_cpu(((struct sx_eth_hdr *)
                                            skb->data)->ethertype);
        ci->info.eth.dmac = be64_to_cpu(((struct sx_eth_hdr *)
                                         skb->data)->dmac_smac1) >> 16;
        if (ci->info.eth.ethtype == ETHTYPE_EMAD) {
            ci->info.eth.emad_tid = be64_to_cpu(((struct sx_emad *)skb->data)->emad_op.tid) >> 32;
        } else if (ci->info.eth.ethtype == ETHTYPE_VLAN) {
            ci->is_tagged = VLAN_TAGGED_E;
            ci->vid = be16_to_cpu(((struct vlan_ethhdr*)skb->data)->h_vlan_TCI) & 0xfff;
            if (ci->vid == 0) { /* VLAN header with tag 0 ==> we should use pvid instead */
                ci->is_tagged = VLAN_PRIO_TAGGED_E;
                ci->vid = get_vid_from_db(dev, ci->cqe_params.is_lag, ci->cqe_params.sysport_lag_id);
            }
        } else {
            ci->is_tagged = VLAN_UNTAGGED_E;
            ci->vid = get_vid_from_db(dev, ci->cqe_params.is_lag, ci->cqe_params.sysport_lag_id);
        }

        if (SX_CQE_RX_FROM_NETWORK_PORT(&ci->cqe_params)) {
            err = sx_core_get_rp_mode(dev, ci->cqe_params.is_lag, ci->cqe_params.sysport_lag_id, ci->vid, &is_from_rp);
            if (err) {
                sxd_log_rl_err(dev, "Failed sx_core_get_rp_mode(). err: %d \n", err);
                goto out;
            }

            ci->info.eth.from_rp = (is_from_rp) ? IS_RP_FROM_RP_E : IS_RP_NOT_FROM_RP_E;
            err = sx_core_get_fid_by_port_vid(dev, ci, &fid);
            if (err) {
                sxd_log_rl_err(dev, "Failed sx_core_get_bridge(). err: %d \n", err);
                goto out;
            }

            ci->bridge_id = fid;
            ci->info.eth.from_bridge = (fid) ? IS_BRIDGE_FROM_BRIDGE_E : IS_BRIDGE_NOT_FROM_BRIDGE_E;
        }
        break;

    case PKT_TYPE_IB_non_Raw:
    case PKT_TYPE_FCoIB:
    case PKT_TYPE_ETHoIB:
        ci->info.ib.qpn = get_qpn(ci->cqe_params.trap_id, skb);
        if (ci->info.ib.qpn == 0xffffffff) {
            sxd_log_rl_warning(dev, "Received IB packet "
                               "is not valid. Dropping the packet\n");

            err = -EINVAL;
            goto out;
        }
        FALL_THROUGH;

    /* fall-through */
    case PKT_TYPE_IB_Raw:
        if (unlikely(is_sgmii_supported())) {
            err = sgmii_get_mad_header_info(skb->data,
                                            skb->len,
                                            NULL,
                                            &ci->info.ib.mad_tid,
                                            &mad_attr_id);
            if (!err) {
                ci->info.ib.is_oob_originated_mad = ((ci->info.ib.mad_tid >> 32) == SGMII_TR_ID_PREFIX);
            }
        }

        /* Extract the IB port from the sysport */
        ci->cqe_params.sysport_lag_id = (ci->cqe_params.sysport_lag_id >> 4) & 0xfff;
        if (remove_crc && !truncate_size) {
            skb->len -= (ci->cqe_params.pkt_type == PKT_TYPE_IB_Raw) ? IB_VCRC_LENGTH : IB_CRC_LENGTH;
        }
        break;

    default:
        sxd_log_rl_warning(dev, "Received packet type is FC, "
                           "and therefore unsupported right now\n");

        err = -EINVAL;
        goto out;
    }

    if (!is_from_monitor_rdq && sx_ptp_is_enabled(priv)) {
        /*
         * if (sx_core_ptp_rx_handler() != 0) it is not an error, it just means that either it is not a
         * PTP packet/event or it is a PTP packet that got its timestamp and should continue the packet
         * flow as usual [dispatch_pkt(), free skb].
         *
         * if (sx_core_ptp_rx_handler() == 0) it means that packet is being handled by PTP and processing
         * is done here (only on SPC1)
         *
         * */

        if (sx_core_ptp_rx_handler(priv, ci, cqn) == 0) {
            return 0;
        }
    }

    if (force_listener == NULL) {
        dispatch_pkt(dev, ci, ci->cqe_params.trap_id, 1);
    } else {
        force_listener->handler(ci, force_listener->context);
    }

out:
    sx_skb_consume(skb);
    return err;
}
EXPORT_SYMBOL(rx_skb);
static void * sx_get_cqe_from_buf(struct sx_buf *buf, int n, size_t cqe_size)
{
    int offset = n * cqe_size;

    if (buf->nbufs == 1) {
        return buf->u.direct.buf + offset;
    } else {
        return buf->u.page_list[offset >> PAGE_SHIFT].buf +
               (offset & (PAGE_SIZE - 1));
    }
}

static void * sx_get_cqe(struct sx_cq *cq, int n)
{
    return sx_get_cqe_from_buf(&cq->buf, n, cq->cqe_sizeof);
}

static u8 __sx_get_cqe_owner_v0(struct sx_cq *cq, int n)
{
    struct sx_cqe_v0 *cqe = sx_get_cqe(cq, n & (cq->nent - 1));

    return cqe->e_sr_dqn_owner;
}

static u8 __sx_get_cqe_owner_v1(struct sx_cq *cq, int n)
{
    struct sx_cqe_v1 *cqe = sx_get_cqe(cq, n & (cq->nent - 1));

    return cqe->dqn_owner;
}

static u8 __sx_get_cqe_owner_v2(struct sx_cq *cq, int n)
{
    struct sx_cqe_v2 *cqe = sx_get_cqe(cq, n & (cq->nent - 1));

    return cqe->pe2cqe_token_owner;
}

static void __sx_cqe_owner_init_v0(struct sx_cq *cq)
{
    int               i = 0;
    struct sx_cqe_v0 *cqe = NULL;

    for (i = 0; i < cq->nent; i++) {
        cqe = sx_get_cqe(cq, i);
        cqe->e_sr_dqn_owner = 1;
    }
}

static void __sx_cqe_owner_init_v1(struct sx_cq *cq)
{
    int               i = 0;
    struct sx_cqe_v1 *cqe = NULL;

    for (i = 0; i < cq->nent; i++) {
        cqe = sx_get_cqe(cq, i);
        cqe->dqn_owner = 1;
    }
}

static void __sx_cqe_owner_init_v2(struct sx_cq *cq)
{
    int               i = 0;
    struct sx_cqe_v2 *cqe = NULL;

    for (i = 0; i < cq->nent; i++) {
        cqe = sx_get_cqe(cq, i);
        cqe->pe2cqe_token_owner = 1;
    }
}

static void * sx_get_sw_cqe_v0(struct sx_cq *cq, int n)
{
    struct sx_cqe_v0 *cqe = sx_get_cqe(cq, n & (cq->nent - 1));

    return (cqe->e_sr_dqn_owner & 0x1) ^ !!(n & cq->nent) ? NULL : cqe;
}

static void * sx_get_sw_cqe_v2(struct sx_cq *cq, int n)
{
    struct sx_cqe_v2 *cqe = sx_get_cqe(cq, n & (cq->nent - 1));

    return (cqe->pe2cqe_token_owner & 0x1) ^ !!(n & cq->nent) ? NULL : cqe;
}

static void * sx_get_sw_cqe_v1(struct sx_cq *cq, int n)
{
    struct sx_cqe_v1 *cqe = sx_get_cqe(cq, n & (cq->nent - 1));

    return (cqe->dqn_owner & 0x1) ^ !!(n & cq->nent) ? NULL : cqe;
}

static void __sx_next_cqe_v0(struct sx_cq *cq, union sx_cqe *cqe_p)
{
    cqe_p->v0 = sx_get_sw_cqe_v0(cq, cq->cons_index);
}

static void __sx_next_cqe_v1(struct sx_cq *cq, union sx_cqe *cqe_p)
{
    cqe_p->v1 = sx_get_sw_cqe_v1(cq, cq->cons_index);
}

static void __sx_next_cqe_v2(struct sx_cq *cq, union sx_cqe *cqe_p)
{
    cqe_p->v2 = sx_get_sw_cqe_v2(cq, cq->cons_index);
}

void sx_tx_wqe_sync_for_cpu(struct sx_dq *sdq, int idx)
{
    struct sx_tx_sge_data *tx_frag;
    int                    i;

    for (i = 0; i < SX_MAX_FRAGS_PER_WQE; i++) {
        tx_frag = &sdq->sge[idx].u.tx.tx_frags[i];

        if (tx_frag->frag_len > 0) {
            dma_sync_single_for_cpu(&sdq->dev->pdev->dev, tx_frag->dma_addr, tx_frag->frag_len, DMA_TO_DEVICE);
            dma_unmap_single(&sdq->dev->pdev->dev, tx_frag->dma_addr, tx_frag->frag_len, DMA_TO_DEVICE);

            tx_frag->vaddr = NULL;
            tx_frag->frag_len = 0;
        }
    }
}

static void __sx_fill_poll_one_params_from_cqe_v0(union sx_cqe *u_cqe, struct sx_cqe_params *cqe_params)
{
    cqe_params->dqn = (u_cqe->v0->e_sr_dqn_owner >> 1) & SX_CQE_DQN_MASK;
    if (be16_to_cpu(u_cqe->v0->dqn5_byte_count) & SX_CQE_DQN_MSB_MASK) {
        cqe_params->dqn |= (1 << SX_CQE_DQN_MSB_SHIFT);
    }
    cqe_params->is_err = !!(u_cqe->v0->e_sr_dqn_owner & SX_CQE_IS_ERR_MASK);
    cqe_params->is_send = !!(u_cqe->v0->e_sr_dqn_owner >> 6);
    cqe_params->wqe_counter = be16_to_cpu(u_cqe->v0->wqe_counter);
    cqe_params->trap_id = be16_to_cpu(u_cqe->v0->trap_id) & 0x01FF;
    cqe_params->is_isx = be16_to_cpu(u_cqe->v0->dqn5_byte_count) >> 15;
    cqe_params->is_crc = u_cqe->v0->type_swid & 0x1;
    cqe_params->byte_count = be16_to_cpu(u_cqe->v0->dqn5_byte_count) & 0x3FFF;
    cqe_params->user_def_val = 0;
    cqe_params->is_lag = !!(u_cqe->v0->lag & 0x80);
    if (cqe_params->is_lag) {
        cqe_params->sysport_lag_id = (cqe_params->sysport_lag_id >> 4) & 0xFFF;
        cqe_params->lag_subport = u_cqe->v0->vlan2_lag_subport & 0x1F;
    } else {
        cqe_params->sysport_lag_id = be16_to_cpu(u_cqe->v0->system_port_lag_id);
    }
    cqe_params->mirror_reason = 0;
    cqe_params->cqe_ts.ts_type = SXD_TS_TYPE_NONE;
    cqe_params->pkt_type = (u_cqe->v0->type_swid >> 5) & 0x7;
    cqe_params->swid = (u_cqe->v0->type_swid >> 1) & 0x7;
}

static void __sx_fill_poll_one_params_from_cqe_v1(union sx_cqe *u_cqe, struct sx_cqe_params *cqe_params)
{
    cqe_params->dqn = (u_cqe->v1->dqn_owner >> 1) & 0x3F;
    cqe_params->is_err = !!(u_cqe->v1->version_e_sr_packet_ok_rp_lag & 0x8);
    cqe_params->is_send = !!(u_cqe->v1->version_e_sr_packet_ok_rp_lag & 0x4);
    cqe_params->wqe_counter = be16_to_cpu(u_cqe->v1->wqe_counter);
    cqe_params->trap_id = be16_to_cpu(u_cqe->v1->sma_check_id_trap_id) & 0x3FF;
    cqe_params->byte_count = be16_to_cpu(u_cqe->v1->isx_ulp_crc_byte_count) & 0x3FFF;
    cqe_params->user_def_val = 0;
    cqe_params->is_lag = u_cqe->v1->version_e_sr_packet_ok_rp_lag & 0x1;
    cqe_params->lag_subport = u_cqe->v1->rp_lag_subport & 0xFF;
    cqe_params->sysport_lag_id = be16_to_cpu(u_cqe->v1->rp_system_port_lag_id);
    cqe_params->mirror_reason = 0;
    cqe_params->cqe_ts.ts_type = SXD_TS_TYPE_NONE;
    cqe_params->swid = (u_cqe->v1->type_swid_crc >> 2) & 0x7;
}

static void __sx_fill_poll_one_params_from_cqe_v2(union sx_cqe *u_cqe, struct sx_cqe_params *cqe_params)
{
    cqe_params->dqn = (u_cqe->v2->dqn >> 1) & 0x3F;
    cqe_params->is_err = !!(u_cqe->v2->version_e_sr_packet_ok_rp_lag & 0x8);
    cqe_params->is_send = !!(u_cqe->v2->version_e_sr_packet_ok_rp_lag & 0x4);
    cqe_params->wqe_counter = be16_to_cpu(u_cqe->v2->wqe_counter);
    cqe_params->trap_id = be16_to_cpu(u_cqe->v2->sma_check_id_trap_id) & 0x3FF;
    cqe_params->is_isx = be16_to_cpu(u_cqe->v2->isx_ulp_crc_byte_count) >> 15;
    cqe_params->is_crc = (u_cqe->v2->type_swid_crc >> 1) & 0x1;
    cqe_params->byte_count = be16_to_cpu(u_cqe->v2->isx_ulp_crc_byte_count) & 0x3FFF;
    cqe_params->user_def_val = be32_to_cpu(u_cqe->v2->mirror_cong1_user_def_val_orig_pkt_len) & 0xFFFFF;
    cqe_params->is_lag = u_cqe->v2->version_e_sr_packet_ok_rp_lag & 0x1;
    cqe_params->lag_subport = u_cqe->v2->rp_lag_subport & 0xFF;
    cqe_params->sysport_lag_id = be16_to_cpu(u_cqe->v2->rp_system_port_lag_id);
    cqe_params->mirror_reason = (be32_to_cpu(u_cqe->v2->mirror_reason_time_stamp_type_time_stamp2) >> 24) & 0xFF;
    cqe_params->mirror_tclass = (u_cqe->v2->mirror_tclass_mirror_elph_ep_lag >> 3) & 0x1F;
    cqe_params->mirror_cong = ((be16_to_cpu(u_cqe->v2->vlan_mirror_cong2) & 0xF) << 12) |
                              ((be32_to_cpu(u_cqe->v2->mirror_cong1_user_def_val_orig_pkt_len) & 0xFFF00000) >> 20);
    cqe_params->mirror_lantency = (be16_to_cpu(u_cqe->v2->mirror_latency2) << 8) | u_cqe->v2->mirror_latency1;
    cqe_params->dest_is_lag = u_cqe->v2->mirror_tclass_mirror_elph_ep_lag & 0x1;
    cqe_params->dest_lag_subport = u_cqe->v2->ep_lag_subport;
    cqe_params->dest_sysport_lag_id = be16_to_cpu(u_cqe->v2->ep_system_port_lag_id);
    cqe_params->cqe_ts.timestamp.tv_sec =
        (be32_to_cpu(u_cqe->v2->mirror_reason_time_stamp_type_time_stamp2) >> 14) & 0xFF;
    cqe_params->cqe_ts.timestamp.tv_nsec =
        ((be32_to_cpu(u_cqe->v2->mirror_reason_time_stamp_type_time_stamp2) & 0x3FFF) << 16) |
        be16_to_cpu(u_cqe->v2->time_stamp1);
    cqe_params->cqe_ts.ts_type = (be32_to_cpu(u_cqe->v2->mirror_reason_time_stamp_type_time_stamp2) >> 22) & 0x3;
    cqe_params->pkt_type = (u_cqe->v2->type_swid_crc >> 5) & 0x7;
    cqe_params->mirror_elephant = (u_cqe->v2->mirror_tclass_mirror_elph_ep_lag >> 1) & 0x3;
    cqe_params->swid = (u_cqe->v2->type_swid_crc >> 2) & 0x7;
}

void sx_fill_cqe_params_from_cqev0(union sx_cqe *u_cqe, struct sx_cqe_params *cqe_params)
{
    __sx_fill_poll_one_params_from_cqe_v0(u_cqe, cqe_params);
}

void sx_fill_cqe_params_from_cqev2(union sx_cqe *u_cqe, struct sx_cqe_params *cqe_params)
{
    __sx_fill_poll_one_params_from_cqe_v2(u_cqe, cqe_params);
}

static inline bool __cq_has_cqe(struct sx_cq *cq)
{
    union sx_cqe u_cqe;

    cq->sx_next_cqe_cb(cq, &u_cqe);

    return u_cqe.v2 != NULL;
}

static int sx_SW2HW_CQ(struct sx_dev *dev, struct sx_cmd_mailbox *mailbox, int cq_num)
{
    return sx_cmd(dev, dev->device_id, mailbox, cq_num, 0, SX_CMD_SW2HW_CQ,
                  SX_CMD_TIME_CLASS_A, sx_priv(dev)->fw.local_in_mb_size, 0);
}

static int sx_HW2SW_CQ(struct sx_dev *dev, int cq_num)
{
    return sx_cmd_box(dev, dev->device_id, 0, 0, cq_num, 0,
                      SX_CMD_HW2SW_CQ, SX_CMD_TIME_CLASS_A,
                      sx_priv(dev)->fw.local_in_mb_size, 0);
}

void sx_init_cq_db_v0(struct sx_cq *tcq, u8 cqn, u8 *cqe_ver)
{
    tcq->cqe_version = 0;
    tcq->cqe_sizeof = sizeof(struct sx_cqe_v0);
    tcq->sx_next_cqe_cb = __sx_next_cqe_v0;
    tcq->sx_fill_poll_one_params_from_cqe_cb = __sx_fill_poll_one_params_from_cqe_v0;
    tcq->sx_get_cqe_owner_cb = __sx_get_cqe_owner_v0;
    tcq->sx_cqe_owner_init_cb = __sx_cqe_owner_init_v0;
    *cqe_ver = 0;
}

void sx_init_cq_db_spc(struct sx_cq *tcq, u8 cqn, u8 *cqe_ver)
{
    if (cqn < NUMBER_OF_SDQS) {
        tcq->cqe_version = 1;
        tcq->cqe_sizeof = sizeof(struct sx_cqe_v1);
        tcq->sx_next_cqe_cb = __sx_next_cqe_v1;
        tcq->sx_fill_poll_one_params_from_cqe_cb = __sx_fill_poll_one_params_from_cqe_v1;
        tcq->sx_get_cqe_owner_cb = __sx_get_cqe_owner_v1;
        tcq->sx_cqe_owner_init_cb = __sx_cqe_owner_init_v1;
        *cqe_ver = 0;
    } else {
        tcq->cqe_sizeof = sizeof(struct sx_cqe_v2);
        tcq->cqe_version = 2;
        tcq->sx_next_cqe_cb = __sx_next_cqe_v2;
        tcq->sx_fill_poll_one_params_from_cqe_cb = __sx_fill_poll_one_params_from_cqe_v2;
        tcq->sx_get_cqe_owner_cb = __sx_get_cqe_owner_v2;
        tcq->sx_cqe_owner_init_cb = __sx_cqe_owner_init_v2;
        *cqe_ver = 1;
    }
}

void sx_init_cq_db_v2(struct sx_cq *tcq, u8 cqn, u8 *cqe_ver)
{
    tcq->cqe_version = 2;
    tcq->cqe_sizeof = sizeof(struct sx_cqe_v2);
    tcq->sx_next_cqe_cb = __sx_next_cqe_v2;
    tcq->sx_fill_poll_one_params_from_cqe_cb = __sx_fill_poll_one_params_from_cqe_v2;
    tcq->sx_get_cqe_owner_cb = __sx_get_cqe_owner_v2;
    tcq->sx_cqe_owner_init_cb = __sx_cqe_owner_init_v2;
    *cqe_ver = 1;
}

void sx_ci_set_timestamp(const struct sx_cq *cq, const struct sx_rx_timestamp *linux_ts, struct completion_info *ci)
{
    struct sx_priv *priv = sx_priv(cq->dev);

    switch (SX_CQ_TIMESTAMP_TYPE(cq)) {
    case SXD_TS_TYPE_LINUX:
        memcpy(&ci->rx_linux_timestamp, linux_ts, sizeof(ci->rx_linux_timestamp));
        ci->active_timestamp = &ci->rx_linux_timestamp;
        break;

    case SXD_TS_TYPE_CQE_UTC:
        sx_core_clock_cqe_ts_to_utc(priv, &ci->cqe_params.cqe_ts.timestamp, &ci->cqe_params.cqe_ts.timestamp);
        ci->active_timestamp = &ci->cqe_params.cqe_ts;
        break;

    default:
        ci->active_timestamp = NULL;
    }
}

static int __sx_cq_poll_internal(struct sx_cq *cq, int budget)
{
    union sx_cq_poll_context context;
    int                      handled = 0;
    int                      err = 0;

    if (cq->poll_callbacks->sx_cq_poll_init_context_cb) {
        err = cq->poll_callbacks->sx_cq_poll_init_context_cb(cq, &context);
        if (err) {
            sxd_log_err(SXD_CONT_DEV(cq), "failed to initialize context (cq=%d)\n", cq->cqn);
            goto out;
        }
    }

    do {
        err = cq->poll_callbacks->sx_cq_poll_one_cb(cq, &context);
    } while (!err && ++handled < budget);

    if (cq->poll_callbacks->sx_cq_poll_deinit_context_cb) {
        cq->poll_callbacks->sx_cq_poll_deinit_context_cb(cq, &context);
    }

    if (err == -ENOBUFS) { /* may be returned from sx_cq_poll_one_cb() */
        __sx_cq_dec_refcnt(cq); /* this will signal sx_core_destroy_cq() that CQ can be deleted */
    }

out:
    return handled;
}

void sx_cq_update_ts(struct sx_cq *cq, enum sx_cq_last_ts last_ts)
{
    cq->last_ts = last_ts;
    if (last_ts == SX_CQ_EVENT_LAST) {
        cq->event_ts = jiffies;
    } else {
        cq->armed_ts = jiffies;
    }

    sx_health_cq_ok(cq);
}

/* this function should run only from SoftIRQ (tasklet) */
void sx_cq_trigger(struct sx_cq *cq)
{
    sx_cq_update_ts(cq, SX_CQ_EVENT_LAST);
    cq->sched_callbacks->sx_cq_trigger_cb(cq);
}

static inline void __sx_cq_reschedule(struct sx_cq *cq)
{
    if (likely(cq->sched_callbacks->sx_cq_reschedule_cb)) {
        cq->sched_callbacks->sx_cq_reschedule_cb(cq);
    } else {
        WARN_ONCE(1, "reschedule callback is NULL (maybe SP?) ... SP=%s!\n", (cq->strict_prio ? "true" : "false"));
    }
}

static int __sx_cq_poll_ex(struct sx_cq *cq, bool must_arm)
{
    int budget = cq->budget;
    int handled;

    if (unlikely(cq->paused)) {
        return 0;
    }

    handled = __sx_cq_poll_internal(cq, budget);

    if (!must_arm && (handled == budget) && __cq_has_cqe(cq)) {
        __sx_cq_reschedule(cq); /* another round */
    } else {
        sx_cq_arm(cq);
    }

    return handled;
}

static inline int __sx_cq_poll(struct sx_cq *cq)
{
    return __sx_cq_poll_ex(cq, false);
}

static inline bool __should_run_in_softirq(struct sx_cq *cq)
{
    return (cq->cq_thread_id == SX_CQ_THREAD_SDK_INTERNAL_HIGH);
}

static void __sx_cq_trigger_tasklet(struct sx_cq *cq)
{
    __sx_cq_poll(cq);
}

static void __sx_cq_reschedule_tasklet(struct sx_cq *cq)
{
    struct sx_priv *priv = sx_priv(cq->dev);

    list_add_tail(&cq->tasklet_resched_list, &priv->cq_threads.tasklet_resched_list);
    tasklet_schedule(&priv->intr_tasklet);
}

static void __sx_cq_trigger_workqueue(struct sx_cq *cq)
{
    if (WARN_ON_ONCE(!cq->cq_thread)) {
        return;
    }

    queue_work(cq->cq_thread, &cq->work);
}

static void __sx_cq_reschedule_workqueue(struct sx_cq *cq)
{
    __sx_cq_trigger_workqueue(cq);
}

#define STRICT_PRIO_BUDGET 10

static void __sx_cq_work_handler(struct work_struct *work)
{
    struct sx_cq   *cq = container_of(work, struct sx_cq, work);
    struct sx_priv *priv = sx_priv(cq->dev);
    bool            sp_pending = false;

    WARN_ON_ONCE(cq->strict_prio);

    if (cq->cq_thread_id == SX_CQ_THREAD_USER) {
        spin_lock_bh(&priv->cq_threads.lock);
        sp_pending = !list_empty(&priv->cq_threads.sp_list);
        spin_unlock_bh(&priv->cq_threads.lock);

        if (sp_pending) {
            __sx_cq_reschedule(cq);
            return;
        }
    }

    __sx_cq_poll(cq);
}

static int __sx_cq_poll_init_context_rx(struct sx_cq *cq, union sx_cq_poll_context *context)
{
    int err = 0;

    if (SX_CQ_TIMESTAMP_TYPE(cq) == SXD_TS_TYPE_LINUX) {
        sx_get_linux_timestamp(&context->rx.linux_ts);
    }

    err = sx_core_get_rdq_param_max(cq->dev, NULL, &context->rx.rdq_max_buff_size);
    if (err) {
        sxd_log_err(SXD_CONT_DEV(cq), "failed to get monitor RDQ attributes (err=%d)\n", err);
        goto out;
    }

out:
    return err;
}

static int __pre_poll_one_common(struct sx_cq             *cq,
                                 struct sx_cqe_params     *cqe_params,
                                 union sx_cq_poll_context *context,
                                 bool                      inc_cons_index)
{
    union sx_cqe u_cqe;

    spin_lock_bh(&cq->lock);
    cq->sx_next_cqe_cb(cq, &u_cqe);
    if (!u_cqe.v0) { /* it is a union: v0 == v1 == v2 */
        spin_unlock_bh(&cq->lock);
        return -EAGAIN;
    }

    if (likely(inc_cons_index)) {
        ++cq->cons_index;
        ++cq->cons_index_dbg;
    }

    spin_unlock_bh(&cq->lock);

    /* Make sure we read CQ entry contents after we've checked the ownership bit. */
    rmb();
    cq->sx_fill_poll_one_params_from_cqe_cb(&u_cqe, cqe_params);
    return 0;
}

static int __sx_cq_poll_one_rx(struct sx_cq *cq, union sx_cq_poll_context *context)
{
    struct sx_priv        *priv = sx_priv(cq->dev);
    struct sx_dq          *rdq = cq->dq;
    struct completion_info ci;
    struct sk_buff        *skb;
    u8                     tp_flags;
    int                    idx;
    int                    ret;

    memset(&ci, 0, sizeof(ci));

    ret = __pre_poll_one_common(cq, &ci.cqe_params, context, true);
    if (ret) {
        if (unlikely(ret != -EAGAIN)) {
            sxd_log_rl_err(SXD_PRIV_DEV(priv), "failed to get CQE from CQ #%d (err=%d)\n", cq->cqn, ret);
        }

        return ret;
    }

    /* get the skb from the right rdq entry, unmap the buffers and call rx_skb with the cqe and skb
     *   this while is temporary, we need it because FW doesn't send us error CQes for wqe too short. */

    idx = rdq->tail & (rdq->wqe_cnt - 1);
    ++rdq->tail;

    skb = sx_rx_buffers_build_skb(rdq, idx, ci.cqe_params.byte_count);
    if (!skb) {
        sxd_log_err(rdq->dev, "failed to allocate skb for RDQ\n");
        return -ENOMEM;
    }

    spin_lock_bh(&rdq->lock);
    cq->wqe_counter = ci.cqe_params.wqe_counter;
    spin_unlock_bh(&rdq->lock);

    sx_ci_set_timestamp(cq, &context->rx.linux_ts, &ci);

    tp_flags =
        sx_run_rdq_tracepoint(priv, rdq, &ci.cqe_params, skb, context->rx.rdq_max_buff_size, ci.active_timestamp);

    if (!ci.cqe_params.is_err && !(tp_flags & SX_RDQ_TRACEPOINT_F_DROP_SKB)) {
        rx_skb(cq->dev, skb, &ci, 0, NULL, cq->dev->device_id);
    } else {
        sx_skb_free(skb);   /* drop packet flow, use kfree_skb */
    }

    sx_cq_set_ci(cq);

    if (rdq->is_flushing) {
        if (rdq->head == rdq->tail) {
            /* when RDQ flushed all its buffers, return -ENOBUFS to tell the caller that CQ is no longer in use */
            ret = -ENOBUFS;
        }

        goto out;
    }

    ret = sx_core_post_recv(rdq);
    if (ret) {
        sxd_log_err(rdq->dev, "failed to prepare RDQ buffers (dqn=%d)\n", rdq->dqn);
    }

out:
    return ret;
}

static int __sx_cq_poll_one_tx(struct sx_cq *cq, union sx_cq_poll_context *context)
{
    struct sx_priv      *priv = sx_priv(cq->dev);
    struct sx_dq        *sdq = cq->dq;
    struct sx_cqe_params cqe_params;
    struct sx_pkt       *curr_pkt;
    struct list_head    *pos;
    unsigned long        now, comp_time;
    u16                  wqe_ctr;
    bool                 found;
    int                  idx;
    int                  ret;

    ret = __pre_poll_one_common(cq, &cqe_params, context, true);
    if (ret) {
        if (unlikely(ret != -EAGAIN)) {
            sxd_log_rl_err(SXD_PRIV_DEV(priv), "failed to get CQE from CQ #%d (err=%d)\n", cq->cqn, ret);
        }

        return ret;
    }

    spin_lock_bh(&sdq->lock);

    wqe_ctr = cqe_params.wqe_counter & (sdq->wqe_cnt - 1);

    /* find the wqe and unmap the DMA buffer.
     * sx_skb_free to call  the destructor which will signal
     * the write-semaphore for ETH packets, or call gen_completion for IB packets. */
    do {
        idx = sdq->tail++ & (sdq->wqe_cnt - 1);
        sdq->last_completion = jiffies;
        sx_tx_wqe_sync_for_cpu(sdq, idx);

        /* update occupancy when completion is received for SDQ.*/
        sx_dq_update_occupancy(sdq);

        /* if it is a TX PTP packet, put the timestamp skb */
        if ((cqe_params.cqe_ts.ts_type != SXD_TS_TYPE_NONE) && /* can be true only on SPC2 and above */
            (sdq->sge[idx].u.tx.skb != NULL) &&
            (skb_shinfo(sdq->sge[idx].u.tx.skb)->tx_flags & SKBTX_IN_PROGRESS)) {
            sx_core_clock_cqe_ts_to_utc(priv, &cqe_params.cqe_ts.timestamp, &cqe_params.cqe_ts.timestamp);
            sx_core_ptp_tx_ts_handler(priv, sdq->sge[idx].u.tx.skb, &cqe_params.cqe_ts.timestamp);
        }

        found = false;
        list_for_each(pos, &sdq->pkts_comp_list.list_wait_for_completion) {
            curr_pkt = list_entry(pos, struct sx_pkt, list_wait_for_completion);
            if (curr_pkt->idx == idx) {
                found = true;
                break;
            }
        }

        if (!found) {
            sxd_log_err(SXD_PRIV_DEV(priv),
                        "SDQ [%u]: completion for an unknown index (idx=%u)\n",
                        cqe_params.dqn,
                        idx);
            continue;
        }

        list_del(pos);
        sx_tx_skb_consume(sdq->sge[idx].u.tx.skb, curr_pkt->type);   /* free unused skb, use consume_skb */
        sdq->sge[idx].u.tx.skb = NULL;
        if (time_after(jiffies, curr_pkt->since + SX_LATE_COMPLETION_THRESHOLD)) {
            sdq->pkts_late_completion++;
            sxd_log_rl_warning(SXD_PRIV_DEV(priv),
                               "Receive late completion for dqn (%d) idx (%d)\n",
                               cqe_params.dqn,
                               idx);
        } else {
            sdq->pkts_recv_completion++;
        }

        now = jiffies;
        comp_time = now - curr_pkt->since;
        if (comp_time > sdq->max_comp_time) {
            sdq->max_comp_time = comp_time;
            sdq->max_comp_ts = now;
        }

        kfree(curr_pkt);

        if (idx != wqe_ctr) {
            sxd_log_err(SXD_CONT_DEV(cq), "should not be here!!! (idx=%d, wqe_ctr=%d)\n", idx,
                        wqe_ctr);
        }
    } while (idx != wqe_ctr);

    if ((!priv->global_flushing) && (!sdq->is_flushing)) {
        sx_add_pkts_to_sdq(sdq);
    }

    spin_unlock_bh(&sdq->lock);

    sx_cq_set_ci(cq);
    return 0;
}

static int __sx_cq_poll_init_monitoring_context(struct sx_cq *cq, union sx_cq_poll_context *context)
{
    int err = __sx_cq_poll_init_context_rx(cq, context);

    if (!err) {
        context->rx.ktime_before = ktime_to_ns(ktime_get());
        context->rx.handled = 0;
    }

    return err;
}

static inline u64 __calculate_sleep_time(u64 ktime)
{
    int percent;
    u64 sleep_time;

    if (mon_cq_thread_cpu_util_percent == 0) {
        percent = MON_CQ_HANDLER_THREAD_CPU_UTIL_PERCENT_DEFAULT;
    } else if (mon_cq_thread_cpu_util_percent > 100) {
        percent = 100;
    } else {
        percent = mon_cq_thread_cpu_util_percent;
    }

    /* Calculate the time to sleep, in nanoseconds */
    if (((ktime * (100 - percent)) % percent) == 0) {
        sleep_time = ktime * (100 - percent) / percent;
    } else {
        sleep_time = ktime * (100 - percent) / percent + 1;
    }

    /* Transform the time to sleep to microseconds */
    if ((sleep_time % 1000) == 0) {
        sleep_time = sleep_time / 1000;
    } else {
        sleep_time = sleep_time / 1000 + 1;
    }

    /* According to https://www.kernel.org/doc/Documentation/timers/timers-howto.txt,
     * if the sleep time is less than 10us, we should use udelay instead of usleep.
     * As udelay is doing busy-wait which is a waste of CPU cycles, we limit the minimum
     * sleep time to 10us here.
     */
    if (sleep_time < 10) {
        return 10;
    } else {
        return sleep_time;
    }
}

static void __sx_cq_poll_deinit_monitoring_context(struct sx_cq *cq, union sx_cq_poll_context *context)
{
    u64 ktime_after, sleep_time;

    if (context->rx.handled >= cq->budget) {
        ktime_after = ktime_to_ns(ktime_get());
        sleep_time = __calculate_sleep_time(ktime_after - context->rx.ktime_before);
        usleep_range(sleep_time, sleep_time + 5);
    }
}

/* WJH polling */
static int __sx_cq_poll_one_monitoring(struct sx_cq *cq, union sx_cq_poll_context *context)
{
    struct sx_dev          *dev = cq->dev;
    struct sx_priv         *priv = sx_priv(dev);
    struct sx_dq           *rdq = cq->dq;
    struct sx_sge_internal *sge;
    struct completion_info  ci;
    struct sk_buff         *skb;
    u16                     cons_index;
    u16                     idx;
    int                     ret = 0;
    u8                      tp_flags;

    context->rx.handled++;

    memset(&ci, 0, sizeof(ci));

    ret = __pre_poll_one_common(cq, &ci.cqe_params, context, false);
    if (ret) {
        if (unlikely(ret != -EAGAIN)) {
            sxd_log_rl_err(SXD_PRIV_DEV(priv), "failed to get CQE from CQ #%d (err=%d)\n", cq->cqn, ret);
        }

        goto out;
    }

    cons_index = cq->cons_index & 0xffff;
    sx_ci_set_timestamp(cq, &context->rx.linux_ts, &ci);
    if (ci.active_timestamp) {
        memcpy(&cq->cqe_ts_arr[cons_index % cq->nent], ci.active_timestamp, sizeof(cq->cqe_ts_arr[0]));
    } else {
        cq->cqe_ts_arr[cons_index % cq->nent].ts_type = SXD_TS_TYPE_CQE_NONE;
    }

    idx = cons_index % rdq->wqe_cnt;
    sge = &rdq->sge[idx];

    if (sge->u.rx.mon_rdq_skb) { /* skb was not consumed by WJH packet reader (we're cyclic), need to recycle */
        sx_skb_consume(sge->u.rx.mon_rdq_skb);
        sge->u.rx.mon_rdq_skb = NULL;
    }

    if (ci.cqe_params.is_err) { /* RDQ is terminating */
        return 0;
    }

    skb = sx_rx_buffers_build_skb(rdq, idx, ci.cqe_params.byte_count);
    if (unlikely(!skb)) {
        sxd_log_err(dev, "failed to allocate skb for monitoring RDQ\n");
        return -ENOMEM;
    }

    sge->u.rx.mon_rdq_skb = skb;
    ret = sx_rx_buffers_prepare_wqe(rdq, idx); /* preparing new pages instead those that were consumed by the new skb */
    if (ret) {
        goto out;
    }

    sx_drop_monitor_handle_monitor_rdq(cq, cons_index, &ci, skb);

    tp_flags =
        sx_run_rdq_tracepoint(priv, rdq, &ci.cqe_params, skb, context->rx.rdq_max_buff_size, ci.active_timestamp);
    if (!(tp_flags & SX_RDQ_TRACEPOINT_F_ENABLED) || !(tp_flags & SX_RDQ_TRACEPOINT_F_FILTER_INSTALLED)) {
        skb->mark = 0;
    }

    cq->cons_index++;
    cq->cons_index_dbg++;
    sx_cq_set_ci(cq);

    /* update rdq head and tail */
    rdq->head++;
    rdq->tail++;

    sx_dq_notify_doorbell(rdq);

    rdq->mon_rx_count++;

    /*
     *  the window [mon_rx_start, mon_rx_start+mon_rx_cnt] should
     *  contains maximum wqe_cnt packets
     */
    if (rdq->mon_rx_count - rdq->mon_rx_start > rdq->wqe_cnt) {
        rdq->mon_rx_start = rdq->mon_rx_count - rdq->wqe_cnt;
    }

out:
    return ret;
}

static void __cq_sp_list_add(struct sx_cq *cq)
{
    struct sx_priv *priv = sx_priv(cq->dev);
    struct sx_cq   *iter_cq;

    WARN_ON_ONCE(cq->in_sp_list);
    cq->in_sp_list = true;

    list_for_each_entry(iter_cq, &priv->cq_threads.sp_list, sp_list) {
        /* There may be a case when TC changed from strict priority to
         * DWRR while CQ is in the SP list.
         * This CQ will be removed from SP list when resuming CQ.
         * But we can have a case where CQ is in SP list with out-of-order budget.
         * In that case, we ignore this CQ when adding new CQ to the SP list. */
        if ((cq->budget > iter_cq->budget) && iter_cq->strict_prio) {
            /* higher values go first */
            list_add_tail(&cq->sp_list, &iter_cq->sp_list);
            return;
        }
    }

    /* If not inserted before any node, add at the end */
    list_add_tail(&cq->sp_list, &priv->cq_threads.sp_list);
}

static void __cq_sp_list_del(struct sx_cq *cq)
{
    WARN_ON_ONCE(!cq->in_sp_list);

    list_del(&cq->sp_list);
    cq->in_sp_list = false;
}

static void __sx_cq_sp_work_handler(struct work_struct *work)
{
    struct sx_priv *priv;
    struct sx_cq   *cq = NULL;
    int             handled;

    priv = container_of(work, struct sx_priv, cq_threads.sp_work);

    spin_lock_bh(&priv->cq_threads.lock);
    if (priv->cq_threads.paused_cq_bitmap) {
        goto resched;
    }

    if (!list_empty(&priv->cq_threads.sp_list)) {
        cq = list_first_entry(&priv->cq_threads.sp_list, struct sx_cq, sp_list);
    }

    spin_unlock_bh(&priv->cq_threads.lock);

    if (!cq) { /* SP list is empty. someone pulled at least one CQ from the list until we got here
                *  (probably user changed trap_group priority) */
        return; /* without rescheduling the work */
    }

    handled = __sx_cq_poll_internal(cq, STRICT_PRIO_BUDGET);

    spin_lock_bh(&priv->cq_threads.lock);
    if (handled < STRICT_PRIO_BUDGET) { /* CQ has no more entries to handle */
        if (cq->in_sp_list) { /* during __sx_cq_poll_internal() we were out of the cq_threads.lock scope.
                               *  the CQ might have already removed from the SP list so we need to make
                               *  sure it is still there before we remove it */
            __cq_sp_list_del(cq);
        }

        sx_cq_arm(cq);
    }

resched:
    if (!list_empty(&priv->cq_threads.sp_list)) {
        queue_work(priv->cq_threads.wq[SX_CQ_THREAD_USER], work);
    }

    spin_unlock_bh(&priv->cq_threads.lock);
}

void sx_cq_init_sp_work(struct sx_priv *priv)
{
    INIT_WORK(&priv->cq_threads.sp_work, __sx_cq_sp_work_handler);
}

/* check if the cq has SP scheduler and if so, add the cq to the sorted SP list */
static void __sx_cq_trigger_workqueue_sp(struct sx_cq *cq)
{
    struct sx_priv *priv = sx_priv(cq->dev);
    bool            trigger_sp_work = false;

    WARN_ON_ONCE(!cq->strict_prio);

    spin_lock_bh(&priv->cq_threads.lock);
    trigger_sp_work = list_empty(&priv->cq_threads.sp_list);
    __cq_sp_list_add(cq);
    spin_unlock_bh(&priv->cq_threads.lock);

    if (trigger_sp_work) {
        queue_work(priv->cq_threads.wq[SX_CQ_THREAD_USER], &priv->cq_threads.sp_work);
    }
}

static const struct sx_cq_schedule_callbacks __sx_cq_schedule_cb_tasklet = {
    .sx_cq_trigger_cb = __sx_cq_trigger_tasklet,
    .sx_cq_reschedule_cb = __sx_cq_reschedule_tasklet
};

static const struct sx_cq_schedule_callbacks __sx_cq_schedule_cb_workqueue = {
    .sx_cq_trigger_cb = __sx_cq_trigger_workqueue,
    .sx_cq_reschedule_cb = __sx_cq_reschedule_workqueue
};

static const struct sx_cq_schedule_callbacks __sx_cq_schedule_cb_workqueue_sp = {
    .sx_cq_trigger_cb = __sx_cq_trigger_workqueue_sp,
    .sx_cq_reschedule_cb = NULL
};

static const struct sx_cq_poll_callbacks __sx_cq_poll_cb_tx = {
    .sx_cq_poll_init_context_cb = NULL,
    .sx_cq_poll_deinit_context_cb = NULL,
    .sx_cq_poll_one_cb = __sx_cq_poll_one_tx
};

static const struct sx_cq_poll_callbacks __sx_cq_poll_cb_rx = {
    .sx_cq_poll_init_context_cb = __sx_cq_poll_init_context_rx,
    .sx_cq_poll_deinit_context_cb = NULL,
    .sx_cq_poll_one_cb = __sx_cq_poll_one_rx
};

static const struct sx_cq_poll_callbacks __sx_cq_poll_cb_monitoring = {
    .sx_cq_poll_init_context_cb = __sx_cq_poll_init_monitoring_context,
    .sx_cq_poll_deinit_context_cb = __sx_cq_poll_deinit_monitoring_context,
    .sx_cq_poll_one_cb = __sx_cq_poll_one_monitoring
};

static void __sx_cq_store_user_param(struct sx_cq *cq)
{
    cq->user_strict_prio = cq->strict_prio;
    cq->user_budget = cq->budget;
}

static void __sx_cq_get_user_param(struct sx_cq *cq, bool *is_sp, int *budget)
{
    *is_sp = cq->user_strict_prio;
    *budget = cq->user_budget;
}

void sx_cq_set_thread_param(struct sx_cq *cq, enum sx_cq_thread thread_id, int budget, bool strict_prio)
{
    struct sx_priv *priv = sx_priv(cq->dev);

    if (thread_id == SX_CQ_THREAD_WJH) {
        cq->strict_prio = false;
        cq->budget = MONITOR_RDQ_BUDGET;
    } else { /*USER thread */
        cq->strict_prio = strict_prio;
        cq->budget = budget; /* For USER threads the budget is the WRR weight or the tclass index in SP */
    }

    /* in case user sends 'EDIT' command, thread ID is SX_CQ_THREAD_MAX so only DWRR/SP settings are changed
     * but CQ thread remains as is */
    if (thread_id != SX_CQ_THREAD_MAX) {
        cq->cq_thread_id = thread_id;
        cq->cq_thread = priv->cq_threads.wq[cq->cq_thread_id];
    }

    if (__should_run_in_softirq(cq)) {
        cq->sched_callbacks = &__sx_cq_schedule_cb_tasklet;
    } else if (cq->strict_prio) {
        cq->sched_callbacks = &__sx_cq_schedule_cb_workqueue_sp;
    } else {
        cq->sched_callbacks = &__sx_cq_schedule_cb_workqueue;
    }
}

void sx_cq_set_rx_monitoring(struct sx_cq *cq, bool is_monitoring)
{
    bool is_sp = 0;
    int  budget = 0;

    /* CQ is of RDQ and must be paused when calling this function! */
    WARN_ON_ONCE(!cq->paused);

    if (is_monitoring) {
        __sx_cq_store_user_param(cq);
        cq->poll_callbacks = &__sx_cq_poll_cb_monitoring;
        sx_cq_set_thread_param(cq, SX_CQ_THREAD_WJH, MONITOR_RDQ_BUDGET, false);
    } else {
        __sx_cq_get_user_param(cq, &is_sp, &budget);
        cq->poll_callbacks = &__sx_cq_poll_cb_rx;
        sx_cq_set_thread_param(cq, SX_CQ_THREAD_USER, budget, is_sp);
    }
}

int sx_core_create_cq(struct sx_dev *dev, int nent, struct sx_cq **cq, u8 cqn, u8 send)
{
    struct sx_priv        *priv = sx_priv(dev);
    struct sx_cq_table    *cq_table = &priv->cq_table;
    struct sx_cmd_mailbox *mailbox;
    struct sx_cq_context  *cq_context;
    int                    err;
    unsigned long          flags;
    struct sx_cq          *tcq;
    u8                     cqe_ver = 0;

    if ((nent <= 0) || (dev == NULL)) {
        return -EINVAL;
    }

    tcq = kzalloc(sizeof *tcq, GFP_KERNEL);
    if (!tcq) {
        return -ENOMEM;
    }

    err = __sx_core_dev_specific_cb_get_reference(dev);
    if (err) {
        err = -EINVAL;
        goto free_from_cq_table;
    }

    priv->dev_specific_cb.sx_init_cq_db_cb(tcq, cqn, &cqe_ver);

    __sx_core_dev_specific_cb_release_reference(dev);

    spin_lock_irqsave(&cq_table->lock, flags);
    cq_table->cq[cqn] = tcq;
    spin_unlock_irqrestore(&cq_table->lock, flags);
    mailbox = sx_alloc_cmd_mailbox(dev, dev->device_id);
    if (IS_ERR(mailbox)) {
        err = PTR_ERR(mailbox);
        goto free_from_cq_table;
    }

    cq_context = mailbox->buf;
    memset(cq_context, 0, sizeof *cq_context);
    err = sx_buf_alloc(dev, nent * tcq->cqe_sizeof, PAGE_SIZE, &tcq->buf, 0);
    if (err) {
        goto err_mbox;
    }

    tcq->cqn = cqn;
    tcq->nent = nent;
    cq_context->eq_num = priv->eq_table.eq[SX_EQ_COMP].eqn | (cqe_ver << SX_CQE_VER_OFFSET);

    tcq->cqe_ts_arr = vmalloc(nent * sizeof(*tcq->cqe_ts_arr));
    if (tcq->cqe_ts_arr == NULL) {
        err = -ENOMEM;
        goto err_buf;
    }

    cq_context->log_cq_size = (u8)ilog2(nent);
    if (dev->pdev) {
        sx_fill_page_list(cq_context->dma_addr, &tcq->buf);
    }

    tcq->sx_cqe_owner_init_cb(tcq);
    err = sx_SW2HW_CQ(dev, mailbox, tcq->cqn);
    if (err) {
        goto err_ts_alloc;
    }

    sx_free_cmd_mailbox(dev, mailbox);
    tcq->set_ci_db = priv->db_base + SX_DBELL_CQ_CI_OFFSET + 4 * tcq->cqn;
    tcq->arm_db = priv->db_base + SX_DBELL_CQ_ARM_OFFSET + 4 * tcq->cqn;
    tcq->dev = dev;
    tcq->cons_index = 0;
    tcq->cons_index_dbg = 0;
    tcq->cons_index_dbg_snapshot = 0;
    atomic_set(&tcq->refcount, 1);
    init_completion(&tcq->free);
    spin_lock_init(&tcq->lock);
    sx_cq_set_ci(tcq);

    if (send) { /* CQ of SDQ */
        tcq->poll_callbacks = &__sx_cq_poll_cb_tx;
        sx_cq_set_thread_param(tcq, SX_CQ_THREAD_SDQ, DEFAULT_BUDGET, false);
    } else { /* CQ of RDQ */
        tcq->poll_callbacks = &__sx_cq_poll_cb_rx;
        sx_cq_set_thread_param(tcq, SX_CQ_THREAD_USER, DEFAULT_BUDGET, false);
    }
    INIT_LIST_HEAD(&tcq->tasklet_resched_list);
    INIT_LIST_HEAD(&tcq->sp_list);
    tcq->in_sp_list = false;
    INIT_WORK(&tcq->work, __sx_cq_work_handler);
    sx_cq_arm(tcq);
    *cq = tcq;

    return 0;

err_ts_alloc:
    if (tcq->cqe_ts_arr) {
        vfree(tcq->cqe_ts_arr);
    }

err_buf:
    sx_buf_free(dev, tcq->nent * tcq->cqe_sizeof, &tcq->buf);

err_mbox:
    sx_free_cmd_mailbox(dev, mailbox);

free_from_cq_table:
    spin_lock_irqsave(&cq_table->lock, flags);
    cq_table->cq[tcq->cqn] = NULL;
    spin_unlock_irqrestore(&cq_table->lock, flags);

    kfree(tcq);
    return err;
}

void sx_core_destroy_cq(struct sx_dev *dev, struct sx_cq *cq, struct sx_dq *dq)
{
    struct sx_priv     *priv = sx_priv(dev);
    struct sx_cq_table *cq_table = &priv->cq_table;
    unsigned long       flags;
    int                 err;

    if (!cq) {
        sx_err(dev, "sx_core_destroy_cq  : cq(%p) == NULL\n", cq);
        return;
    }

    spin_lock_irqsave(&cq_table->lock, flags);
    cq_table->cq[cq->cqn] = NULL;
    spin_unlock_irqrestore(&cq_table->lock, flags);

    /* at this point CQ is idle for sure */

    err = sx_HW2SW_CQ(dev, cq->cqn);
    if (err) {
        sx_warn(dev, "HW2SW_CQ failed (%d) for CQN %06x\n", err, cq->cqn);
    }

    synchronize_irq(priv->eq_table.eq[SX_EQ_COMP].irq);

    /* in case there are works for the specific cq in the work queue,
     * it will cancel the work or will wait for it to finish if the work already started
     */
    cancel_work_sync(&cq->work);

    __sx_cq_dec_refcnt(cq);
    if (!err && (wait_for_completion_timeout(&cq->free, msecs_to_jiffies(10000)) == 0)) {
        sxd_log_err(dev, "sx_core: Error - trying to delete CQ but it is still in use! "
                    "[cqn=%d, is_monitor=%s, DQ_head=%u, DQ_tail=%u\n",
                    cq->cqn,
                    (dq->is_monitor ? "true" : "false"),
                    dq->head,
                    dq->tail);
    }

    if (cq->cqe_ts_arr) {
        vfree(cq->cqe_ts_arr);
    }

    sx_buf_free(dev, cq->nent * cq->cqe_sizeof, &cq->buf);
    kfree(cq);
}

int sx_core_init_cq_table(struct sx_dev *dev)
{
    struct sx_priv     *priv = sx_priv(dev);
    struct sx_cq_table *cq_table = &priv->cq_table;

    spin_lock_init(&cq_table->lock);
    return 0;
}

void sx_core_destroy_cq_table(struct sx_dev *dev)
{
    sx_drop_monitor_destroy(dev);
}

void sx_core_dump_synd_tbl(struct sx_dev *dev)
{
    struct sx_priv               *priv = sx_priv(dev);
    struct listeners_and_rf_info *info = NULL;
    struct listener_entry        *listener;
    struct list_head             *pos;
    u16                           entry = 0;

    mutex_lock(&priv->listeners_and_rf_db.lock);
    info = priv->listeners_and_rf_db.info;

    if (!info) {
        goto out;
    }

    for (entry = 0; entry < NUM_HW_SYNDROMES + 1; entry++) {
        if (entry % 100 == 0) {
            udelay(1);
        }
        if (!list_empty(&info->per_synd_list[entry])) {
            list_for_each(pos, &info->per_synd_list[entry]) {
                listener = list_entry(pos,
                                      struct listener_entry, list);
                sxd_log_info(dev,
                             "=============================\n");
                sxd_log_info(dev,
                             "synd=%d, swid=%d,is_def:%d, "
                             "handler:%p, rx_pkt:%llu \n",
                             entry,
                             listener->swid,
                             listener->is_default,
                             listener->handler,
                             listener->rx_pkts);

                switch (listener->listener_type) {
                case L2_TYPE_DONT_CARE:
                    sxd_log_info(dev, "list_type: "
                                 "DONT_CARE, crit [port:0x%x] \n",
                                 listener->critireas.dont_care.sysport);
                    break;

                case L2_TYPE_IB:
                    sxd_log_info(dev, "list_type: IB, crit"
                                 " [qpn:0x%x (%d)] \n",
                                 listener->critireas.ib.qpn,
                                 listener->critireas.ib.qpn);
                    break;

                case L2_TYPE_ETH:
                    sxd_log_info(dev, "list_type: ETH, crit "
                                 "[ethtype:0x%x, dmac:%llx, "
                                 "emad_tid:0x%x, from_rp:%u, from_bridge:%u ] \n",
                                 listener->critireas.eth.ethtype,
                                 listener->critireas.eth.dmac,
                                 listener->critireas.eth.emad_tid,
                                 listener->critireas.eth.from_rp,
                                 listener->critireas.eth.from_bridge);
                    break;

                case L2_TYPE_FC:
                    sxd_log_info(dev, "list_type: FC \n");
                    break;

                default:
                    sxd_log_info(dev, "list_type: UNKNOWN \n");
                    break;
                }
            }
        }
    }

out:
    mutex_unlock(&priv->listeners_and_rf_db.lock);
}


void sx_cq_show_cq(struct sx_dev *dev, int cqn)
{
    struct sx_priv                     *priv = sx_priv(dev);
    struct sx_cq_table * __maybe_unused cq_table = &priv->cq_table;
    struct sx_cq                       *cq = priv->cq_table.cq[cqn];
    int                                 iii, jjj;
    u8                                  cqe_owner[16];

    if (!cq) {
        sxd_log_err(dev, "cq %d doesn't exist \n", cqn);
        return;
    }

    sxd_log_info(dev, "[cq %d]: cqn:%d, cons_index:%d, nent:%d cons_index&(nent-1):%d"
                 " ref_cnt:%d, cqe_version:%d, on_hold:%d \n",
                 cqn,
                 cq->cqn,
                 cq->cons_index,
                 cq->nent,
                 (cq->cons_index & (cq->nent - 1)),
                 atomic_read(&cq->refcount),
                 cq->cqe_version,
                 cq_table->cq[cqn]->paused);

    sxd_log_info(dev, "CQ %d owner:\n", cqn);
    for (iii = 0; iii < cq->nent / 16; iii++) {
        for (jjj = 0; (jjj < 16) && ((iii * 16 + jjj) < cq->nent); jjj++) {
            cqe_owner[jjj] = cq->sx_get_cqe_owner_cb(cq, (iii * 16 + jjj));
        }
        sxd_log_info(dev, "[%5.5d]: %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x "
                     "%2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x\n",
                     iii * 16,
                     cqe_owner[0], cqe_owner[1], cqe_owner[2], cqe_owner[3],
                     cqe_owner[4], cqe_owner[5], cqe_owner[6], cqe_owner[7],
                     cqe_owner[8], cqe_owner[9], cqe_owner[10], cqe_owner[11],
                     cqe_owner[12], cqe_owner[13], cqe_owner[14], cqe_owner[15]);
    }
    sxd_log_info(dev, "\n");
}

/* Get CQE despite the cqe version */
void sx_get_cqe_all_versions(struct sx_cq *cq, uint32_t n, union sx_cqe *cqe_p)
{
    switch (cq->cqe_version) {
    case 0:
        cqe_p->v0 = sx_get_cqe(cq, n & (cq->nent - 1));
        break;

    case 1:
        cqe_p->v1 = sx_get_cqe(cq, n & (cq->nent - 1));
        break;

    case 2:
        cqe_p->v2 = sx_get_cqe(cq, n & (cq->nent - 1));
        break;
    }

    /* In case the CQ version isn't 0-2 return NULL */
}

void sx_cq_flush_rdq(struct sx_dev *dev, int dqn)
{
    struct sx_priv     *priv = sx_priv(dev);
    struct sx_dq_table *rdq_table = &priv->rdq_table;
    struct sx_dq       *dq = rdq_table->dq[dqn];
    struct sx_cq       *cq = dq->cq;
    int                 num_of_packets;

    sx_cq_pause(cq);
    num_of_packets =
        __sx_cq_poll_internal(cq, cq->nent + 1 /* budget is larger than queue size, consume it in one step */);
    sx_cq_resume(cq);

    sxd_log_notice(dev, "RDQ flushed %d packets\n", num_of_packets);
}


static void __sx_cq_push_pause_resume_work_and_wait(struct sx_cq *cq, work_func_t work_func)
{
    struct sx_pause_resume_work pr_work;

    INIT_WORK(&pr_work.work, work_func);
    pr_work.cq = cq;

    queue_work(cq->cq_thread, &pr_work.work);
    flush_work(&pr_work.work);
}

static void __sx_cq_pause_work_handler(struct work_struct *work)
{
    struct sx_pause_resume_work *pr_work;

    pr_work = container_of(work, struct sx_pause_resume_work, work);
    pr_work->cq->paused = true;

    /* 'pause_work' is on the stack of sx_cq_pause() caller */
}

void sx_cq_pause(struct sx_cq *cq)
{
    struct sx_priv *priv = sx_priv(cq->dev);

    WARN_ON_ONCE(__should_run_in_softirq(cq));

    if (cq->paused) {
        WARN_ONCE(1, "CQ %d is already paused!\n", cq->cqn);
        return;
    }

    spin_lock_bh(&priv->cq_threads.lock);
    priv->cq_threads.paused_cq_bitmap |= (1ULL << cq->dq->dqn);
    spin_unlock_bh(&priv->cq_threads.lock);

    __sx_cq_push_pause_resume_work_and_wait(cq, __sx_cq_pause_work_handler);
}

static void __sx_cq_resume_work_handler(struct work_struct *work)
{
    struct sx_pause_resume_work *pr_work;
    struct sx_cq                *cq;
    struct sx_priv              *priv;

    pr_work = container_of(work, struct sx_pause_resume_work, work);
    cq = pr_work->cq;
    priv = sx_priv(cq->dev);

    spin_lock_bh(&priv->cq_threads.lock);
    priv->cq_threads.paused_cq_bitmap &= ~(1ULL << cq->dq->dqn);

    if (cq->in_sp_list) {
        __cq_sp_list_del(cq);
    }

    spin_unlock_bh(&priv->cq_threads.lock);

    /* we're running in CQ's workqueue context:
     * 1. set 'paused' to false, so __sx_cq_poll_ex() will run and not skip CQ.
     * __sx_cq_poll_ex() will consume some CQEs from the CQ (because if CQ is full, arm it will
     * not help to schedule polling). after that it will NOT schedule CQ's work but only arm CQ.
     * In this way, we make sure there is no double-scheduling (bug) for the CQ work (one from
     * tasklet and one from the current polling).
     */
    cq->paused = false;
    __sx_cq_poll_ex(cq, true);

    /* 'resume_work' is on the stack of sx_cq_resume() caller */
}

void sx_cq_resume(struct sx_cq *cq)
{
    if (!cq->paused) {
        WARN_ONCE(1, "CQ %d is not paused!\n", cq->cqn);
        return;
    }

    /* wait for CQ work (if it is running by chance) because once we resume the CQ,
     * EQ may trigger (schedule) the CQ's work and it must be idle at this time. so
     * we make sure it is idle. */
    flush_work(&cq->work);

    /* we just want to 'resume' a CQ that maybe already has some pending CQEs,
     * so we want to handle them and not count on an interrupt from this CQ */
    __sx_cq_push_pause_resume_work_and_wait(cq, __sx_cq_resume_work_handler);
}

/************************************************
 *                  EOF                         *
 ***********************************************/
