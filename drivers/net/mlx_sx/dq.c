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
 * Includes                                     *
 ***********************************************/

#include <linux/skbuff.h>
#include <linux/pci.h>
#include <linux/if_ether.h>
#include <linux/delay.h>
#include <linux/mlx_sx/cmd.h>
#include <linux/mlx_sx/kernel_user.h>
#include <linux/mlx_sx/skb_hook.h>
#include <linux/mlx_sx/device.h>
#include "dq.h"
#include "rx_buffers.h"
#include "cq.h"
#include "alloc.h"
#include "sx.h"
#include "sx_dpt.h"
#include "sx_proc.h"
#include "sgmii.h"
#include "dev_db.h"
#include "trace_func.h"

#define SDQ_WATCHDOG_FULL_MSECS 300000

#define SX_MAX_SDQ_SW_QUEUE_LEN (10 * (1 << (SX_MAX_LOG_DQ_SIZE)))

/************************************************
 *  Globals
 ***********************************************/
static int (*tx_stub_func)(struct sk_buff *skb, u8 sdqn);
extern int tx_debug;
extern int tx_debug_metadata;
extern int tx_debug_pkt_type;
extern int tx_debug_emad_type;
extern int tx_dump;
extern int tx_dump_cnt;

/************************************************
 * Functions                            *
 ***********************************************/

static int sx_nent_to_4k(int nent)
{
    return ALIGN(nent * SX_DESC_SIZE, 4096) / 4096;
}

static inline void * sx_buf_offset(struct sx_buf *buf, int offset)
{
    if (buf->nbufs == 1) {
        return buf->u.direct.buf + offset;
    } else {
        return buf->u.page_list[offset >> PAGE_SHIFT].buf +
               (offset & (PAGE_SIZE - 1));
    }
}

struct sx_wqe * sx_get_wqe_by_index(struct sx_dq *dq, int n)
{
    return (struct sx_wqe*)sx_buf_offset(&dq->buf, n << SX_WQE_SHIFT);
}

static __be16 sx_set_wqe_flags(u8 set_lp, enum ku_pkt_type type)
{
    u16 flags = 0;

    flags |= (1 << 15); /* Generate CQE for this WQE */
    if (set_lp) {
        flags |= (1 << 14);
    }
    switch (type) {
    case SX_PKT_TYPE_ETH_CTL_UC:
    case SX_PKT_TYPE_ETH_CTL_MC:
    case SX_PKT_TYPE_ETH_DATA:
    case SX_PKT_TYPE_DROUTE_EMAD_CTL:
    case SX_PKT_TYPE_EMAD_CTL:
        flags |= (WQE_PKT_TYPE_ETH << 7);
        break;

    case SX_PKT_TYPE_FCOE_CTL_UC:
    case SX_PKT_TYPE_FCOE_CTL_MC:
        flags |= (WQE_PKT_TYPE_FCOE << 7);
        break;

    case SX_PKT_TYPE_IB_RAW_CTL:
    case SX_PKT_TYPE_IB_RAW_DATA:
        flags |= (WQE_PKT_TYPE_RAW_IB << 7);
        break;

    case SX_PKT_TYPE_IB_TRANSPORT_CTL:
    case SX_PKT_TYPE_IB_TRANSPORT_DATA:
        flags |= (WQE_PKT_TYPE_IB_TRANSPORT << 7);
        break;

    case SX_PKT_TYPE_IB_CTL_2:
        flags |= (WQE_PKT_TYPE_IBGen2 << 7);
        break;

    case SX_PKT_TYPE_FCOIB_CTL:
        flags |= (WQE_PKT_TYPE_FCOIB << 7);
        break;

    case SX_PKT_TYPE_IB_NVLINK:
        flags |= (WQE_PKT_TYPE_NVLoIB << 7);
        break;

    case SX_PKT_TYPE_FC_CTL_UC:
    case SX_PKT_TYPE_FC_CTL_MC:
    case SX_PKT_TYPE_EOIB_CTL:
    default:
        break; /* unsupported */
    }

    return cpu_to_be16(flags);
}

static int sx_build_send_packet(struct sx_dq *sdq, struct sk_buff *skb, struct sx_wqe *wqe, int idx)
{
    int             i, num_frags;
    struct sk_buff *orig_skb = NULL;

    num_frags = skb_shinfo(skb)->nr_frags;
    if (num_frags > 2) { /* no support for more than 2 fragments ... */
        orig_skb = skb;
        skb = skb_copy(skb, GFP_ATOMIC);

        if (!skb) {
            return -ENOMEM;
        }

        num_frags = 0;
    }

    sdq->sge[idx].u.tx.skb = skb;
    sdq->sge[idx].u.tx.tx_frags[0].vaddr = skb->data;
    sdq->sge[idx].u.tx.tx_frags[0].frag_len = skb_headlen(skb);
    sdq->sge[idx].u.tx.tx_frags[0].dma_addr = dma_map_single(&sdq->dev->pdev->dev,
                                                             sdq->sge[idx].u.tx.tx_frags[0].vaddr,
                                                             sdq->sge[idx].u.tx.tx_frags[0].frag_len,
                                                             DMA_TO_DEVICE);
    if (dma_mapping_error(&sdq->dev->pdev->dev, sdq->sge[idx].u.tx.tx_frags[0].dma_addr)) {
        if (orig_skb) {
            sx_skb_free(skb); /* delete the copy. the original skb will be freed elsewhere */
        }

        return -ENOMEM;
    }

    if (orig_skb) {
        skb->destructor = orig_skb->destructor;
        orig_skb->destructor = NULL;
    }

    wqe->dma_addr[0] = cpu_to_be64(sdq->sge[idx].u.tx.tx_frags[0].dma_addr);
    wqe->byte_count[0] = cpu_to_be16(sdq->sge[idx].u.tx.tx_frags[0].frag_len);

    dma_sync_single_for_device(&sdq->dev->pdev->dev,
                               sdq->sge[idx].u.tx.tx_frags[0].dma_addr,
                               sdq->sge[idx].u.tx.tx_frags[0].frag_len,
                               DMA_TO_DEVICE);

    for (i = 1; i <= num_frags; i++) {
        skb_frag_t            *frag = &skb_shinfo(skb)->frags[i - 1];
        struct sx_tx_sge_data *sge_data = &sdq->sge[idx].u.tx.tx_frags[i];

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
        sge_data->vaddr = skb_frag_address(frag);
        sge_data->frag_len = skb_frag_size(frag);
#else
        sge_data->vaddr = lowmem_page_address(frag->page.p) + frag->page_offset;
        sge_data->frag_len = frag->size;
#endif

        sge_data->dma_addr = dma_map_single(&sdq->dev->pdev->dev,
                                            sge_data->vaddr,
                                            sge_data->frag_len,
                                            DMA_TO_DEVICE);
        if (dma_mapping_error(&sdq->dev->pdev->dev, sge_data->dma_addr)) {
            return -ENOMEM;
        }

        wqe->dma_addr[i] = cpu_to_be64(sge_data->dma_addr);
        wqe->byte_count[i] = cpu_to_be16(sge_data->frag_len);
        dma_sync_single_for_device(&sdq->dev->pdev->dev,
                                   sge_data->dma_addr,
                                   sge_data->frag_len,
                                   DMA_TO_DEVICE);
    }

    for (; i < SX_MAX_FRAGS_PER_WQE; i++) {
        wqe->byte_count[i] = 0;
    }

    if (orig_skb) {
        sx_skb_consume(orig_skb); /* delete orig_skb instead of completion handler. the completion handler will
                                   *  delete the copy */
    }

    return 0;
}
/* DQ must be locked here!!! */
static int sx_dq_overflow(struct sx_dq *sdq)
{
    int q_size;

    if (sdq->head >= sdq->tail) {
        q_size = sdq->head - sdq->tail;
    } else {
        q_size = 0x10000 - (sdq->tail - sdq->head);
    }

    if (sdq->send_watermark < q_size) {
        sdq->send_watermark = q_size;
    }

    return (q_size + 1 >= sdq->wqe_cnt);
}


static int sx_is_overflow(unsigned head, unsigned tail, int n)
{
    if ((int)(head - tail) >= n) {
        return -1;
    } else {
        return 0;
    }
}

void sx_dq_update_occupancy(struct sx_dq *sdq)
{
    int q_size;

    if (sdq->head >= sdq->tail) {
        q_size = sdq->head - sdq->tail;
    } else {
        q_size = 0x10000 - (sdq->tail - sdq->head);
    }
    sdq->send_occupancy = q_size;
}

int sx_flush_dq(struct sx_dev *dev, struct sx_dq *dq, bool update_flushing_state)
{
    int           err;
    unsigned long flags;
    unsigned long end;

    if (update_flushing_state) {
        dq->is_flushing = 1;
    }

    err = sx_dq_modify_2err(dev, dq);
    if (err) {
        /* failed to mark DQ as erroneous (that will eventually send completions for all buffers).
         * don't worry, sx_free_dq_sges() will do that in another way */
        sx_warn(dev, "Failed to modify DQ (dqn=%d) to error\n", dq->dqn);
        goto out_mod;
    }

    end = jiffies + 5 * HZ;
    spin_lock_irqsave(&dq->lock, flags);
    while (dq->head != dq->tail) {
        spin_unlock_irqrestore(&dq->lock, flags);
        msleep(1000 / HZ);
        if (time_after(jiffies, end)) {
            spin_lock_irqsave(&dq->lock, flags);

            sx_err(dev,
                   "sx_core: Error: DQ flush timeout [dqn=%d, head=%u, tail=%u, is_monitor=%s]\n",
                   dq->dqn,
                   dq->head,
                   dq->tail,
                   (dq->is_monitor ? "true" : "false"));
            break;
        }

        spin_lock_irqsave(&dq->lock, flags);
    }

    if (update_flushing_state) {
        dq->is_flushing = 0;
    }

    spin_unlock_irqrestore(&dq->lock, flags);

    /* take control from hw */
    err = sx_hw2sw_dq(dev, dq);
    if (err) {
        sx_warn(dev, "hw2sw_dq failed (%d) "
                "for DQN %06x\n", err, dq->dqn);
    }


out_mod:
    return err;
}

void sx_dq_notify_doorbell(struct sx_dq *dq)
{
    /* Make sure that descriptors are written before doorbell. */
    wmb();
    __raw_writel((__force u32)cpu_to_be32(dq->head & 0xffff), dq->db);
    MMIOWB();
}

int sx_add_pkts_to_sdq(struct sx_dq *sdq)
{
    struct sx_wqe    *wqe;
    int               err = 0;
    int               wqe_idx;
    struct sx_pkt    *curr_pkt;
    struct list_head *pos, *q;
    u8                arm = 0;

    list_for_each_safe(pos, q, &sdq->pkts_list.list_send_to_sdq) {
        curr_pkt = list_entry(pos, struct sx_pkt, list_send_to_sdq);
        list_del(pos);
        wqe_idx = sdq->head & (sdq->wqe_cnt - 1);
        wqe = sx_get_wqe_by_index(sdq, wqe_idx);

        wqe->flags = sx_set_wqe_flags(curr_pkt->set_lp, curr_pkt->type);
        err = sx_build_send_packet(sdq, curr_pkt->skb, wqe, wqe_idx);
        if (err) {
            sx_tx_skb_free(curr_pkt->skb, curr_pkt->type); /* drop packet flow, use kfree_skb */
            break;
        }

        curr_pkt->idx = wqe_idx;
        curr_pkt->since = jiffies;
        list_add_tail(&curr_pkt->list_wait_for_completion, &sdq->pkts_comp_list.list_wait_for_completion);
        sdq->pkts_sent_to_sdq++;
        sdq->pkts_in_sw_queue--;

        ++sdq->head;

        arm = 1;
        if (sx_dq_overflow(sdq)) {
            break; /* go to arm the db */
        }
    }

    if (arm) {
        sx_dq_notify_doorbell(sdq);
    }

    return err;
}

int __sx_core_post_send(struct sx_dev *dev, struct sk_buff *skb, struct isx_meta *meta, u8 sdq_id)
{
    struct sx_priv *priv = sx_priv(dev);
    unsigned long   flags = 0;
    int             err = 0;
    struct sx_pkt  *new_pkt;
    struct sx_dq   *sdq = NULL;
    u8              sdqn;
    u8              stclass;
    u8              max_cpu_etclass_for_unlimited_mtu;
    u16             cap_max_mtu = 0;

    if (!dev || !priv->profile.pci_profile_set) {
        sxd_log_rl_err(dev, "Cannot send because the profile is not set\n");
        sx_tx_skb_free(skb, meta->type); /* drop packet flow, use kfree_skb */
        return -ENODEV;
    }

    if (priv->global_flushing) {
        sxd_log_rl_debug(dev, "Cannot send packet while in global_flushing mode\n");
        sx_tx_skb_free(skb, meta->type); /* drop packet flow, use kfree_skb */
        return 0; /* this is not an error */
    }

    if (NUMBER_OF_SWIDS <= meta->swid) {
        sxd_log_rl_err(dev, "Cannot send packet on swid %u\n", meta->swid);
        sx_tx_skb_free(skb, meta->type); /* drop packet flow, use kfree_skb */
        return -EINVAL;
    }

    err = sx_get_sdq(meta, dev, meta->type, meta->swid,
                     meta->etclass, meta->lp, &stclass, &sdqn, &max_cpu_etclass_for_unlimited_mtu,
                     &cap_max_mtu);

    if (err) {
        sxd_log_rl_err(dev, "Error in callback structure sx_get_sdq for selecting SDQ\n");
        sx_tx_skb_free(skb, meta->type); /* drop packet flow, use kfree_skb */
        return err;
    }
    if (DONT_FORCE_SDQ_ID == sdq_id) {
        sdq = sx_priv(dev)->sdq_table.dq[sdqn];
    } else {
        /* used for Health event mechanism (SDQ monitor)*/
        sdq = sx_priv(dev)->sdq_table.dq[sdq_id];
    }

    if (!sdq) {
        sxd_log_rl_err(dev, "SDQ was not allocated\n");
        sx_tx_skb_free(skb, meta->type); /* drop packet flow, use kfree_skb */
        return -ENOENT;
    }

    if (sdq->is_flushing == 1) {
        sxd_log_rl_debug(dev, "Cannot send packet on dqn [%u] while in flushing mode\n", sdqn);
        sx_tx_skb_free(skb, meta->type); /* drop packet flow, use kfree_skb */
        return 0; /* this is not an error */
    }

    if (skb->len > cap_max_mtu) {
        sxd_log_rl_err(dev, "Cannot send packet of size %u, larger than max MTU %u\n",
                       skb->len, cap_max_mtu);
        sx_tx_skb_free(skb, meta->type); /* drop packet flow, use kfree_skb */
        return -EINVAL;
    }

    err = sx_build_isx_header(meta, skb, stclass);
    if (err) {
        sxd_log_rl_err(dev, "Error in build header/stub\n");
        sx_tx_skb_free(skb, meta->type); /* drop packet flow, use kfree_skb */
        return err;
    }

    if ((skb->len > 2048) && (priv->profile.pci_profile.cpu_egress_tclass[sdqn] > max_cpu_etclass_for_unlimited_mtu)) {
        sxd_log_rl_err(dev,
                       "Cannot send packet of size %u from SDQ %u since it's bounded to cpu_tclass %u\n",
                       skb->len,
                       sdqn,
                       priv->profile.pci_profile.cpu_egress_tclass[sdqn]);
        sx_tx_skb_free(skb, meta->type); /* drop packet flow, use kfree_skb */
        return -EINVAL;
    }

    /* send the packet also to the verification stub.
     * Since the verification environment doesn't yet support ETH L3
     * packets we don't pass them such packets. */
    if (tx_stub_func &&
        ((meta->type != SX_PKT_TYPE_ETH_DATA) &&
         (meta->type != SX_PKT_TYPE_ETH_CTL_UC))) {
        tx_stub_func(skb, 0);
    }

    new_pkt = kmalloc(sizeof(*new_pkt), GFP_ATOMIC);
    if (!new_pkt) {
        sxd_log_rl_err(dev, "Cannot send because packet allocation has failed\n");
        sx_tx_skb_free(skb, meta->type); /* drop packet flow, use kfree_skb */
        return -ENOMEM;
    }

    new_pkt->skb = skb;
    new_pkt->set_lp = meta->lp;
    new_pkt->type = meta->type;

    spin_lock_irqsave(&sdq->lock, flags);
    if (sdq->pkts_in_sw_queue < SX_MAX_SDQ_SW_QUEUE_LEN) {
        list_add_tail(&new_pkt->list_send_to_sdq, &sdq->pkts_list.list_send_to_sdq);
        sdq->pkts_in_sw_queue++;
    } else {
        sx_tx_skb_free(skb, meta->type);   /* drop packet flow, use kfree_skb */
        kfree(new_pkt);
        sdq->pkts_sw_queue_drops++;

        sxd_log_rl_notice(dev, "SW queue for SDQ %u is full. dropping the packet!\n", sdq->dqn);
        err = -EBUSY;
        goto out;
    }

    if (sx_dq_overflow(sdq)) {
        if ((sdq->last_full_queue != sdq->last_completion) &&
            (time_after_eq(jiffies, (sdq->last_completion + msecs_to_jiffies(SDQ_WATCHDOG_FULL_MSECS))))) {
            sdq->last_full_queue = sdq->last_completion;
            sxd_log_rl_err(dev, "sdq[%d] buffer is full for at least %d seconds\n",
                           sdq->dqn,
                           SDQ_WATCHDOG_FULL_MSECS / 1000);
        }
        goto out;
    }

    if (dev->pdev) {
        err = sx_add_pkts_to_sdq(sdq);
    }

out:
    spin_unlock_irqrestore(&sdq->lock, flags);
    return err;
}

static void __print_tx_packet(struct sk_buff *skb, const struct isx_meta *meta)
{
    struct sx_dev  *dev = sx_dev_db_get_dev_by_id(meta->dev_id);
    struct sx_priv *priv = sx_priv(dev);
    u16             reg_id = 0xffff; /* invalid register-id */
    int             i;

    /* if packet type is not interesting, go out */
    if ((tx_debug_pkt_type != SX_DBG_PACKET_TYPE_ANY) && (tx_debug_pkt_type != meta->type)) {
        return;
    }

    /* if packet's register-id (packet is EMAD) is not interesting, go out */
    if (meta->type == SX_PKT_TYPE_EMAD_CTL) {
        reg_id = be16_to_cpu(((struct sx_emad *)skb->data)->emad_op.register_id);

        if ((tx_debug_emad_type != SX_DBG_EMAD_TYPE_ANY) && (tx_debug_emad_type != reg_id)) {
            return;
        }
    }

    if (tx_debug_metadata) {
        sxd_log_debug(SXD_DEV_NONE, "TX_META: et=%d, swid=%d, sysport=0x%x, rdq=%d, to_cpu=%d, lp=%d, type=%d, "
                      "dev_id=%d, rx_is_router=%d, fid_valid=%d, fid=%d, skb_headlen(skb)=%d, "
                      "skb_shinfo(skb)->nr_frags=%d\n",
                      meta->etclass,
                      meta->swid,
                      meta->system_port_mid,
                      meta->rdq,
                      meta->to_cpu,
                      meta->lp,
                      meta->type,
                      meta->dev_id,
                      meta->rx_is_router,
                      meta->fid_valid,
                      meta->fid,
                      skb_headlen(skb),
                      skb_shinfo(skb)->nr_frags);
    }

    if (tx_dump) {
        print_hex_dump_debug("TX-SKB    ",
                             DUMP_PREFIX_OFFSET,
                             16,
                             1,
                             skb->data,
                             skb_headlen(skb),
                             false);

        for (i = 0; i < 2 && i < skb_shinfo(skb)->nr_frags; i++) {
            skb_frag_t  *frag = &skb_shinfo(skb)->frags[i];
            void        *buf;
            unsigned int cnt;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
            buf = skb_frag_address(frag);
            cnt = skb_frag_size(frag);
#else
            buf = (void*)(lowmem_page_address(frag->page.p) + frag->page_offset);
            cnt = frag->size;
#endif

            print_hex_dump_debug("TX-SKB (Frag)   ",
                                 DUMP_PREFIX_OFFSET,
                                 16,
                                 1,
                                 buf,
                                 cnt,
                                 false);
        }

        if ((tx_dump_cnt != 0xFFFF) && (tx_dump_cnt > 0)) {
            tx_dump_cnt--;
        }

        if (tx_dump_cnt == 0) {
            tx_dump = 0;
            MODULE_PARAM_INSTANCE_SET(priv, tx_debug, 0);
            tx_debug_metadata = 0;
            tx_debug_pkt_type = 0xFF;
        }
    }
}

int sx_core_post_send(struct sx_dev *dev, struct sk_buff *skb, struct isx_meta *meta)
{
    struct sx_priv *priv = (dev != NULL) ? sx_priv(dev) : NULL;
    int             err = 0;

    /* WA for pad TX packets with size less than ETH_ZLEN */
    if (((meta->type == SX_PKT_TYPE_ETH_DATA) ||
         (meta->type == SX_PKT_TYPE_ETH_CTL_UC)) &&
        (skb->len < ETH_ZLEN)) {
        err = skb_pad(skb, ETH_ZLEN - skb->len);
        if (err) {
            sx_tx_skb_free(skb, meta->type);   /* drop packet flow, use kfree_skb */
            return err;
        }
        skb->len = ETH_ZLEN;
        skb_set_tail_pointer(skb, ETH_ZLEN);
    }

    sx_core_skb_hook_tx_call(dev, skb);

    if (MODULE_PARAM_INSTANCE_GET(priv, tx_debug)) {
        __print_tx_packet(skb, meta);
    }

    if ((dev != NULL) && (meta->swid < NUMBER_OF_SWIDS)) {
        priv->stats.tx_by_pkt_type[meta->swid][meta->type]++;
        priv->stats.tx_by_pkt_type_bytes[meta->swid][meta->type] += skb->len;
    } else if (dev != NULL) {
        priv->stats.tx_by_pkt_type[NUMBER_OF_SWIDS][meta->type]++;
        priv->stats.tx_by_pkt_type_bytes[NUMBER_OF_SWIDS][meta->type] += skb->len;
    }

    if ((meta->type == SX_PKT_TYPE_DROUTE_EMAD_CTL) || /* emad */
        (meta->type == SX_PKT_TYPE_EMAD_CTL)) { /* emad */
        err = sx_dpt_send_emad(meta->dev_id, skb, meta);
    } else if ((meta->type == SX_PKT_TYPE_IB_TRANSPORT_CTL) ||
               (meta->type == SX_PKT_TYPE_IB_TRANSPORT_DATA)) { /* mad */
        err = sx_dpt_send_mad(meta->dev_id, skb, meta);
    } else if (is_sgmii_device(meta->dev_id)) {
        err = sgmii_send_misc(meta->dev_id, skb, meta);
        if (err) {
            sx_tx_skb_free(skb, meta->type);   /* drop packet flow, use kfree_skb */
        }
    } else if (dev && dev->pdev) {
        err = __sx_core_post_send(dev, skb, meta, DONT_FORCE_SDQ_ID);
    } else if (sx_cr_mode()) {
        err = sx_cr_send_packet(dev, skb, meta);
    } else {
        return -EFAULT;
    }

    return err;
}
EXPORT_SYMBOL(sx_core_post_send);

/*
 * Posts a buffer to the HW RDQ
 * The skb contains the kernel buffer address and length of the buffer.
 * Post recv maps the buffer to DMA memory and adds it to RDQ.
 */
int sx_core_post_recv(struct sx_dq *rdq)
{
    struct sx_priv *priv = sx_priv(rdq->dev);
    int             idx, err = 0;

    if (!priv->profile.pci_profile_set) {
        sxd_log_rl_warning(SXD_PRIV_DEV(priv),
                           "sx_core_post_recv() cannot execute because the profile is not set\n");
        return -EPERM;
    }

    spin_lock_bh(&rdq->lock);

    /* Sanity check - overflow should never happen here */
    if (sx_is_overflow(rdq->head, rdq->tail, rdq->wqe_cnt)) {
        sxd_log_rl_err(SXD_PRIV_DEV(priv), "ERR: overflow in rdq, " \
                       "rdq->tail = %u, "
                       "rdq->head = %u\n", rdq->tail, rdq->head);
        err = -EBUSY;
        goto out;
    }

    idx = rdq->head & (rdq->wqe_cnt - 1);
    err = sx_rx_buffers_prepare_wqe(rdq, idx);
    if (err) {
        goto out;
    }

    rdq->head++;

    if (rdq->is_monitor) {
        rdq->mon_rx_count++;

        /*
         *  the window [mon_rx_start, mon_rx_start+mon_rx_cnt] should
         *  contains maximum wqe_cnt packets
         */
        if (rdq->mon_rx_count - rdq->mon_rx_start > rdq->wqe_cnt) {
            rdq->mon_rx_start++;
        }
    }

    sx_dq_notify_doorbell(rdq);

out:
    spin_unlock_bh(&rdq->lock);
    return err;
}

/*
 * This function is used because we can't call kfree_skb on TX IB packets
 * that hold info in the nonlinear part of the SKB. The TX IB skb was not
 * allocated by us, but was given to us by the user (OFED drivers and tools) */
static void __sx_prepare_tx_ib_skb_free(struct sk_buff *skb)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
    int i;
#endif

    if (!skb) {
        return;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    skb_shinfo(skb)->nr_frags = 0;
    skb_shinfo(skb)->frag_list = NULL;
#else
    for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
        skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
        frag->page.p = NULL;
        frag->page_offset = 0;
        frag->size = 0;
    }

    if (i) {
        skb_shinfo(skb)->nr_frags = 0;
        skb_shinfo(skb)->frag_list = NULL;
    }
#endif
}

#define SX_IS_IB_PKT_TYPE(pkt_type)                  \
    (((pkt_type) == SX_PKT_TYPE_IB_TRANSPORT_CTL) || \
     ((pkt_type) == SX_PKT_TYPE_IB_TRANSPORT_DATA))

void sx_tx_skb_free(struct sk_buff *skb, enum ku_pkt_type pkt_type)
{
    if (unlikely(SX_IS_IB_PKT_TYPE(pkt_type))) {
        __sx_prepare_tx_ib_skb_free(skb);
    }

    sx_skb_free(skb);
}
EXPORT_SYMBOL(sx_tx_skb_free);

void sx_tx_skb_consume(struct sk_buff *skb, enum ku_pkt_type pkt_type)
{
    if (unlikely(SX_IS_IB_PKT_TYPE(pkt_type))) {
        __sx_prepare_tx_ib_skb_free(skb);
    }

    sx_skb_consume(skb);
}
EXPORT_SYMBOL(sx_tx_skb_consume);

void sx_skb_free(struct sk_buff *skb)
{
    kfree_skb(skb);
}
EXPORT_SYMBOL(sx_skb_free);

void sx_skb_consume(struct sk_buff *skb)
{
    consume_skb(skb);
}
EXPORT_SYMBOL(sx_skb_consume);

/************************************************
 * Create Functions                             *
 ***********************************************/

static int __sx_init_dq_table(struct sx_dev *dev, unsigned int ndqs, int send)
{
    struct sx_priv     *priv = sx_priv(dev);
    struct sx_dq_table *dq_table = NULL;

    dq_table = ((send == true) ? &(priv->sdq_table) : &(priv->rdq_table));
    spin_lock_init(&dq_table->lock);
    return 0;
}

int sx_core_init_sdq_table(struct sx_dev *dev)
{
    return __sx_init_dq_table(dev, sx_priv(dev)->dev_cap.max_num_sdqs, true);
}

int sx_core_init_rdq_table(struct sx_dev *dev)
{
    return __sx_init_dq_table(dev, sx_priv(dev)->dev_cap.max_num_rdqs, false);
}

int sx_hw2sw_dq(struct sx_dev *dev, struct sx_dq *dq)
{
    int err;

    err = sx_cmd(dev, dev->device_id, 0, dq->dqn & 0xffffff, !dq->is_send,
                 SX_CMD_HW2SW_DQ, SX_CMD_TIME_CLASS_A,
                 sx_priv(dev)->fw.local_in_mb_size, 0);
    if (!err) {
        dq->state = DQ_STATE_ERROR;
    }

    return err;
}

int sx_dq_modify_2err(struct sx_dev *dev, struct sx_dq *dq)
{
    int err;

    err = sx_cmd(dev, dev->device_id, 0, dq->dqn, !dq->is_send,
                 SX_CMD_2ERR_DQ, SX_CMD_TIME_CLASS_A,
                 sx_priv(dev)->fw.local_in_mb_size, 0);
    if (!err) {
        dq->state = DQ_STATE_ERROR;
    }

    return err;
}

static int sx_sw2hw_dq(struct sx_dev *dev, struct sx_dq *dq)
{
    struct sx_priv        *priv = sx_priv(dev);
    struct sx_cmd_mailbox *mailbox;
    struct sx_dq_context  *context;
    struct sx_dq_info     *dq_debug_info;
    int                    i, err = 0;

    mailbox = sx_alloc_cmd_mailbox(dev, dev->device_id);
    if (IS_ERR(mailbox)) {
        err = PTR_ERR(mailbox);
        mailbox = NULL;
        goto out;
    }

    memset(mailbox->buf, 0, SX_MAILBOX_SIZE);
    context = mailbox->buf;
    context->cq = dq->cq->cqn;
    context->log2_dq_sz = ilog2(sx_nent_to_4k(dq->wqe_cnt));
    if (dq->is_send) {
        context->sdq_tclass = priv->profile.pci_profile.cpu_egress_tclass[dq->dqn];
    }

    if (dev->pdev) {
        sx_fill_page_list(context->dma_addr, &dq->buf);
    }


    dq_debug_info = (dq->is_send) ? &priv->debug_only.sdq_info[dq->dqn] : &priv->debug_only.rdq_info[dq->dqn];
    dq_debug_info->log2_dq_sz = context->log2_dq_sz;
    for (i = 0; i < ARRAY_SIZE(context->dma_addr); i++) {
        dq_debug_info->dma_addr[i] = be64_to_cpu(context->dma_addr[i]);
    }

    err = sx_cmd(dev, dev->device_id, mailbox, dq->dqn & 0xffffff,
                 !dq->is_send, SX_CMD_SW2HW_DQ, SX_CMD_TIME_CLASS_A,
                 sx_priv(dev)->fw.local_in_mb_size, 0);
    if (!err) {
        if (dev->pdev && !dq->is_send) {
            err = sx_rx_buffers_init(dq);
            if (err) {
                goto out;
            }
        }

        dq->state = DQ_STATE_RTS;
    }

out:
    if (mailbox) {
        sx_free_cmd_mailbox(dev, mailbox);
    }

    return err;
}

static int sx_dq_alloc(struct sx_dev *dev, u8 send, struct sx_dq *dq, u8 dqn)
{
    int                 err;
    int                 dq_base;
    unsigned long       flags;
    struct sx_priv     *priv = sx_priv(dev);
    struct sx_dq_table *dq_table = send ?
                                   &priv->sdq_table : &priv->rdq_table;

    dq->is_send = send;
    dq->dqn = dqn;
    dq->sge = kzalloc(dq->wqe_cnt * sizeof *dq->sge, GFP_KERNEL);
    if (!dq->sge) {
        err = -ENOMEM;
        goto err_out;
    }

    spin_lock_irqsave(&dq_table->lock, flags);
    dq_table->dq[dq->dqn] = dq;
    spin_unlock_irqrestore(&dq_table->lock, flags);
    atomic_set(&dq->refcount, 1);
    init_completion(&dq->free);
    init_waitqueue_head(&dq->tx_full_wait);
    dq_base = send ? SX_SEND_DQ_DB_BASE : SX_RECV_DQ_DB_BASE;
    dq->db = priv->db_base + dq_base + dq->dqn * 4;
    spin_lock_init(&dq->lock);
    dq->state = DQ_STATE_RESET;
    dq->dev = dev;
    if (send) {
        INIT_LIST_HEAD(&dq->pkts_list.list_send_to_sdq);
        INIT_LIST_HEAD(&dq->pkts_comp_list.list_wait_for_completion);
        dq->pkts_sent_to_sdq = 0;
        dq->pkts_recv_completion = 0;
        dq->pkts_late_completion = 0;
        dq->pkts_in_sw_queue = 0;
        dq->pkts_sw_queue_drops = 0;
        dq->max_comp_time = 0;
    }

    return 0;

err_out:
    kfree(dq->sge);
    return err;
}

static int sx_create_dq(struct sx_dev *dev, int nent, u8 dqn, struct sx_dq **dq, u8 send)
{
    struct sx_priv *priv = sx_priv(dev);
    int             ret;
    struct sx_dq   *tdq;
    struct sx_cq   *cq;
    int             cqn = send ? dqn : dqn + NUMBER_OF_SDQS;

    tdq = kzalloc(sizeof *tdq, GFP_KERNEL);
    if (!tdq) {
        sxd_log_err(dev, "failed to allocate DQ (dqn=%d, is_send=%u)\n", dqn, send);
        return -ENOMEM;
    }

    ret = sx_core_create_cq(dev, (1 << priv->dev_cap.log_max_cq_sz), &cq, cqn, send);
    if (ret < 0) {
        sxd_log_err(dev,
                    "failed to allocate CQ for DQ (dqn=%d, is_send=%u, err=%d)\n",
                    dqn,
                    send,
                    ret);
        goto free_dq;
    }

    cq->dq = tdq;
    tdq->cq = cq;

    ret = sx_buf_alloc(dev, nent * sizeof(struct sx_wqe),
                       PAGE_SIZE, &tdq->buf, 0);
    if (ret) {
        sxd_log_err(dev,
                    "failed to allocate buffers for DQ (dqn=%d, is_send=%u, err=%d)\n",
                    dqn,
                    send,
                    ret);
        ret = -ENOMEM;
        goto free_cq;
    }

    tdq->wqe_cnt = nent;
    ret = sx_dq_alloc(dev, send, tdq, dqn);
    if (ret) {
        sxd_log_err(dev,
                    "failed to allocate DQ elements (dqn=%d, is_send=%u, err=%d)\n",
                    dqn,
                    send,
                    ret);
        goto free_buf;
    }

    tdq->state = DQ_STATE_RESET;
    tdq->is_flushing = 0;
    ret = sx_sw2hw_dq(dev, tdq);
    if (ret) {
        sxd_log_err(dev, "failed to create DQ in HW (err=%d)\n", ret);
        goto free_buf;
    }

    *dq = tdq;
    return 0;

free_buf:
    sx_buf_free(dev, nent * sizeof(struct sx_wqe), &tdq->buf);
free_cq:
    sx_core_destroy_cq(dev, cq, tdq);
free_dq:
    kfree(tdq);
    return ret;
}

int sx_core_create_sdq(struct sx_dev *dev, int nent, u8 dqn, struct sx_dq **sdq_p)
{
    return sx_create_dq(dev, nent, dqn, sdq_p, 1);
}

int sx_core_create_rdq(struct sx_dev *dev, int nent, u8 dqn, struct sx_dq **rdq_p)
{
    return sx_create_dq(dev, nent, dqn, rdq_p, 0);
}

int sx_core_add_rdq_to_monitor_rdq_list(struct sx_dq *dq)
{
    struct sx_dev *dev = dq->dev;
    int            i;

    /* add monitor rdq to the list if it wasn't existed in the list before */
    for (i = 0; i < sx_priv(dev)->wjh_info.monitor_rdqs_count; i++) {
        if (sx_priv(dev)->wjh_info.monitor_rdqs_arr[i] == dq->dqn) { /* already in the list */
            return 0;
        }
    }

    if (sx_priv(dev)->wjh_info.monitor_rdqs_count == MAX_MONITOR_RDQ_NUM) { /* no more space */
        return -EINVAL;
    }

    sx_priv(dev)->wjh_info.monitor_rdqs_arr[sx_priv(dev)->wjh_info.monitor_rdqs_count++] = dq->dqn;
    return 0;
}

void sx_core_del_rdq_from_monitor_rdq_list(struct sx_dq *dq)
{
    int            i;
    struct sx_dev *dev = dq->dev;
    u32            monitor_rdqs_count = sx_priv(dev)->wjh_info.monitor_rdqs_count;

    /* remove monitor rdq from the list */
    for (i = 0; i < monitor_rdqs_count; i++) {
        if (sx_priv(dev)->wjh_info.monitor_rdqs_arr[i] == dq->dqn) {
            sx_priv(dev)->wjh_info.monitor_rdqs_arr[i] =
                sx_priv(dev)->wjh_info.monitor_rdqs_arr[monitor_rdqs_count - 1];
            sx_priv(dev)->wjh_info.monitor_rdqs_count--;
            sx_priv(dev)->wjh_info.monitor_rdqs_arr[monitor_rdqs_count - 1] = RDQ_INVALID_ID;
        }
    }
}


/************************************************
 * Destroy Functions                            *
 ***********************************************/
static void __sx_free_sdq_sges(struct sx_dq *sdq)
{
    int i;

    for (i = 0; i < sdq->wqe_cnt; ++i) {
        if (sdq->sge[i].u.tx.tx_frags[0].vaddr) {
            sx_tx_wqe_sync_for_cpu(sdq, i);
            sx_skb_consume(sdq->sge[i].u.tx.skb); /* free unused skb, use consume_skb */
        }
    }
}

static void sx_dq_free(struct sx_dev *dev, struct sx_dq *dq)
{
    struct list_head *pos, *q;
    struct sx_pkt    *tmp_pkt;

    if (atomic_dec_and_test(&dq->refcount)) {
        complete(&dq->free);
    }

    wait_for_completion(&dq->free);

    if (dq->is_send && !list_empty(&dq->pkts_list.list_send_to_sdq)) {
        list_for_each_safe(pos, q, &dq->pkts_list.list_send_to_sdq) {
            tmp_pkt = list_entry(pos, struct sx_pkt, list_send_to_sdq);
            list_del(pos);
            sx_skb_consume(tmp_pkt->skb);   /* free unused skb, use consume_skb */
            kfree(tmp_pkt);
        }
    }

    if (dq->is_send && !list_empty(&dq->pkts_comp_list.list_wait_for_completion)) {
        list_for_each_safe(pos, q, &dq->pkts_comp_list.list_wait_for_completion) {
            tmp_pkt = list_entry(pos, struct sx_pkt, list_wait_for_completion);
            list_del(pos);
            kfree(tmp_pkt);
        }
    }

    if (dq->is_send) {
        __sx_free_sdq_sges(dq);
    } else {
        sx_rx_buffers_deinit(dq);
    }

    kfree(dq->sge);
}

static void sx_dq_remove(struct sx_dev *dev, struct sx_dq *dq)
{
    struct sx_dq_table *dq_table = dq->is_send ?
                                   &sx_priv(dev)->sdq_table : &sx_priv(dev)->rdq_table;
    unsigned long flags;

    spin_lock_irqsave(&dq_table->lock, flags);
    dq_table->dq[dq->dqn] = NULL;
    spin_unlock_irqrestore(&dq_table->lock, flags);
}

static void sx_destroy_dq(struct sx_dev *dev, struct sx_dq *dq)
{
    /* set dq flushing state true for the entire flow to stop packets sending */
    dq->is_flushing = 1;

    /* before flushing a RDQ, increment its CQ reference count.
     * The reference count will be decremented when the CQ is done handling all buffers that were
     * punted to CPU on sx_dq_modify_2err().
     *
     * The waiting on sx_flush_dq() is not enough because when dq->head == dq->tail, the CQ may still
     * be iterating in sx_cq_completion().
     */
    if (!dq->is_send) {
        atomic_inc(&dq->cq->refcount);
    }

    sx_flush_dq(dev, dq, false);
    sx_core_destroy_cq(dev, dq->cq, dq);
    sx_dq_remove(dev, dq);
    sx_dq_free(dev, dq);
    sx_buf_free(dev, dq->wqe_cnt * sizeof(struct sx_wqe), &dq->buf);
    kfree(dq);
}

void sx_core_destroy_sdq(struct sx_dev *dev, struct sx_dq *dq)
{
    sx_destroy_dq(dev, dq);
}

void sx_core_destroy_rdq(struct sx_dev *dev, struct sx_dq *dq)
{
    sx_destroy_dq(dev, dq);
}

void sx_core_destroy_sdq_table(struct sx_dev *dev, u8 free_table)
{
    struct sx_priv     *priv = sx_priv(dev);
    struct sx_dq_table *sdq_table = &priv->sdq_table;
    int                 i;

    for (i = 0; i < priv->dev_cap.max_num_sdqs; i++) {
        if (sdq_table->dq[i]) {
            if (atomic_read(&sdq_table->dq[i]->refcount) == 1) {
                sx_core_destroy_sdq(dev, sdq_table->dq[i]);
            } else {
                atomic_dec(&sdq_table->dq[i]->refcount);
            }
        }
    }

    if (free_table) {
        memset(&sdq_table->dq, 0, sizeof(sdq_table->dq));
    }
}

void sx_core_destroy_rdq_table(struct sx_dev *dev, u8 free_table)
{
    struct sx_priv     *priv = sx_priv(dev);
    struct sx_dq_table *rdq_table = &priv->rdq_table;
    int                 i;

    for (i = 0; i < priv->dev_cap.max_num_rdqs; i++) {
        if (rdq_table->dq[i]) {
            if (atomic_read(&rdq_table->dq[i]->refcount) == 1) {
                disable_monitor_rdq(dev, rdq_table->dq[i]->dqn, 1);
                sx_core_destroy_rdq(dev, rdq_table->dq[i]);
            } else {
                atomic_dec(&rdq_table->dq[i]->refcount);
            }
        }
    }

    if (free_table) {
        memset(rdq_table->dq, 0, sizeof(rdq_table->dq));
    }
}

static bool __sx_core_rdq_usage_monitor_queue_delayed_work(struct sx_priv *priv, unsigned long delay)
{
    return queue_delayed_work(priv->rdq_use_monitor.wq, &priv->rdq_use_monitor.ws, delay);
}

static void __rdq_usage_info_update(struct work_struct *work_p)
{
    struct delayed_work *dwork = to_delayed_work(work_p);
    struct sx_priv      *priv = container_of(dwork, struct sx_priv, rdq_use_monitor.ws);
    struct sx_dq_table  *rdq_table;
    struct sx_dq        *rdq;
    u32                  cq_cons = 0;
    u32                  cq_prod = 0;
    u32                  cq_occu = 0;
    struct ku_query_cq   cq_context;
    unsigned long        flags;
    int                  rc, cqn = 0;
    u32                  i = 0;
    u32                  interval_ms = 0;
    bool                 error = false;
    unsigned long        running_time_limit = jiffies + HZ / 2;
    static u32           rdq_begin_idx = 0;
    u32                  rdq_end_idx = 0;

    if (priv->rdq_use_monitor.disabled) {
        goto reschedule;
    }

    /* In general, iteration of all rdq (max_num_rdqs) would take about ~1 ms. But it might be slower in some extreme
     * cases, mainly because of slow sx_QUERY_CQ due to slow CPU/FW response, and then trigger health check errors.
     * We would release CPU if the whole time costs 0.5+ second, and restart from the next rdq in next running. */
    rdq_end_idx = rdq_begin_idx + priv->dev_cap.max_num_rdqs;
    for (; rdq_begin_idx < rdq_end_idx; rdq_begin_idx++) {
        i = rdq_begin_idx % priv->dev_cap.max_num_rdqs;
        if (priv->rdq_use_monitor.stopped || (jiffies > running_time_limit)) {
            break;
        }

        spin_lock_irqsave(&priv->rdq_table.lock, flags);
        rdq_table = &priv->rdq_table;
        if (!rdq_table->dq[i]) {
            spin_unlock_irqrestore(&priv->rdq_table.lock, flags);
            continue;
        }
        cqn = rdq_table->dq[i]->cq->cqn;
        rdq = rdq_table->dq[i];
        spin_unlock_irqrestore(&priv->rdq_table.lock, flags);

        if (priv->cq_table.cq[cqn] == NULL) {
            continue;
        }

        error = false;
        rc = sx_QUERY_CQ(&priv->dev, cqn, &cq_context);
        if (rc) {
            error = true;
        }

        spin_lock_irqsave(&rdq->lock, flags);
        if (error) {
            rdq->recv_occupancy = 0;
        } else {
            cq_cons = (priv->cq_table.cq[cqn]->wqe_counter & 0xffff);
            cq_prod = (cq_context.producer_counter);
            cq_occu = (cq_cons > cq_prod) ? (cq_cons - cq_prod) : (cq_prod - cq_cons);
            rdq->recv_occupancy = cq_occu % rdq->wqe_cnt;
        }

        if (rdq->recv_watermark < rdq->recv_occupancy) {
            rdq->recv_watermark = rdq->recv_occupancy;
        }
        spin_unlock_irqrestore(&rdq->lock, flags);
    }
    rdq_begin_idx = rdq_begin_idx % priv->dev_cap.max_num_rdqs;

reschedule:
    spin_lock_irqsave(&priv->ctx_lock, flags);
    interval_ms = priv->rdq_use_monitor.interval_ms;
    spin_unlock_irqrestore(&priv->ctx_lock, flags);

    if (!priv->rdq_use_monitor.stopped) {
        __sx_core_rdq_usage_monitor_queue_delayed_work(priv, msecs_to_jiffies(interval_ms));
    }
}

u8 sx_run_rdq_tracepoint(struct sx_priv         *priv,
                         struct sx_dq           *rdq,
                         struct sx_cqe_params   *cqe_params,
                         struct sk_buff         *skb,
                         u16                     rdq_max_buff_size,
                         struct sx_rx_timestamp *rx_timestamp)
{
    bool should_drop = false;
    u8   ret = 0;

    if (likely(!priv->wjh_info.enable_monitor_rdq_trace_points)) {
        goto out;
    }

    ret |= SX_RDQ_TRACEPOINT_F_ENABLED;

    spin_lock_bh(&priv->rdq_table.lock);

    if (unlikely(priv->wjh_info.filter_ebpf_progs[rdq->dqn])) {
        ret |= SX_RDQ_TRACEPOINT_F_FILTER_INSTALLED;
        should_drop = sx_core_call_rdq_filter_trace_point_func(&priv->dev,
                                                               priv->wjh_info.filter_ebpf_progs[rdq->dqn],
                                                               skb,
                                                               cqe_params,
                                                               rdq_max_buff_size);
        if (should_drop) {
            ret |= SX_RDQ_TRACEPOINT_F_DROP_SKB;
            goto unlock;
        }
    }

    if (unlikely(rdq->is_monitor)) {
        sx_core_call_rdq_agg_trace_point_func(priv,
                                              rdq->dqn,
                                              rdq_max_buff_size,
                                              skb,
                                              cqe_params,
                                              (rx_timestamp != NULL) ? &rx_timestamp->timestamp : NULL);
    }

unlock:
    spin_unlock_bh(&priv->rdq_table.lock);

out:
    return ret;
}

int sx_core_rdq_usage_monitor_init(struct sx_dev *dev)
{
    int             err = 0;
    struct sx_priv *priv = NULL;
    char            wq_name[64] = { 0 };

    if (!dev) {
        sxd_log_err(SXD_DEV_NONE, "%s: Could not find a valid device\n", __func__);
        err = -ENODEV;
        goto out;
    }

    snprintf(wq_name, sizeof(wq_name) - 1, "rdq_mon_wq@%u", dev->device_id);

    priv = sx_priv(dev);

    /* This thread not needed in health check monitor because thread sending ICMD that already covered by CMD_IFC monitor*/
    priv->rdq_use_monitor.wq = create_singlethread_workqueue(wq_name);
    if (!priv->rdq_use_monitor.wq) {
        sxd_log_err(dev, "Failed to create RDQ usage monitor work queue.\n");
        err = -ENOMEM;
        goto out;
    }

    priv->rdq_use_monitor.interval_ms = RDQ_USAGE_MONITOR_TIMER_INTERVAL_DEFAULT_MS;
    INIT_DELAYED_WORK(&priv->rdq_use_monitor.ws, __rdq_usage_info_update);
    __sx_core_rdq_usage_monitor_queue_delayed_work(priv, 0);

out:
    return err;
}


void sx_core_rdq_usage_monitor_deinit(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);

    /* RDQ usage deinit can be called from sx_core remove flow, or from
     * ISSU start flow. */
    if (priv->rdq_use_monitor.stopped == false) {
        priv->rdq_use_monitor.stopped = true;
        cancel_delayed_work_sync(&priv->rdq_use_monitor.ws);
    }

    if (priv->rdq_use_monitor.wq) {
        destroy_workqueue(priv->rdq_use_monitor.wq);
        priv->rdq_use_monitor.wq = NULL;
    }
}

int sx_dq_info_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_priv    *priv = NULL;
    u16                dqn = 0;
    struct sx_dq      *dq = NULL;
    struct sx_dev    * dev = sx_dbg_dump_get_device(m);
    struct sx_dq_info *dq_info;
    struct sx_wqe     *wqe;
    bool               is_send;
    int                i, iter;

    if (!dev) {
        return 0;
    }
    priv = sx_priv(dev);

    sx_dbg_dump_print_header(m, "DQ info dump");
    seq_printf(m, "%-3s   %-10s   %s\n"
               "------------------------------------------------------------------------\n",
               "DQ#",
               "log2_dq_sz",
               "dma_addr");

    for (iter = 0; iter < NUMBER_OF_SDQS + NUMBER_OF_RDQS; iter++) {
        is_send = (iter < NUMBER_OF_SDQS);
        dqn = (is_send) ? iter : iter - NUMBER_OF_SDQS;

        if (!is_send && (dqn == 0)) {
            seq_printf(m, "------------------------------------------------------------------------\n");
        }

        dq = (is_send) ? priv->sdq_table.dq[dqn] : priv->rdq_table.dq[dqn];
        if (!dq) {
            continue;
        }

        dq_info = (is_send) ? &priv->debug_only.sdq_info[dqn] : &priv->debug_only.rdq_info[dqn];
        seq_printf(m, "%c%02u   %-10u   ", (is_send ? 'S' : 'R'), dqn, dq_info->log2_dq_sz);
        for (i = 0; i < ARRAY_SIZE(dq_info->dma_addr); i++) {
            seq_printf(m, "%lx, ", (long)dq_info->dma_addr[i]);
        }

        seq_printf(m, "\n");
    }

    sx_dbg_dump_print_empty_line(m);

    for (iter = 0; iter < NUMBER_OF_SDQS + NUMBER_OF_RDQS; iter++) {
        is_send = (iter < NUMBER_OF_SDQS);
        dqn = (is_send) ? iter : iter - NUMBER_OF_SDQS;

        dq = (is_send) ? priv->sdq_table.dq[dqn] : priv->rdq_table.dq[dqn];
        if (!dq) {
            continue;
        }

        seq_printf(m, "%c%02u\n", (is_send ? 'S' : 'R'), dqn);
        spin_lock_bh(&dq->lock);
        for (i = 0; i < dq->wqe_cnt; i++) {
            wqe = sx_get_wqe_by_index(dq, i);
            seq_printf(m, "    %04u) flags=0x%x, byte_count[%u,%u,%u], dma_addr=[0x%llx,0x%llx,0x%llx]\n",
                       i,
                       be16_to_cpu(wqe->flags),
                       be16_to_cpu(wqe->byte_count[0]),
                       be16_to_cpu(wqe->byte_count[1]),
                       be16_to_cpu(wqe->byte_count[2]),
                       be64_to_cpu(wqe->dma_addr[0]),
                       be64_to_cpu(wqe->dma_addr[1]),
                       be64_to_cpu(wqe->dma_addr[2]));
        }
        spin_unlock_bh(&dq->lock);
    }

    sx_dbg_dump_print_empty_line(m);
    return 0;
}

/************************************************
 *                  EOF                         *
 ***********************************************/
