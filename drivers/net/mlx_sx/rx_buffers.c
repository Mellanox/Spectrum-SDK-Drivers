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

#include "sx.h" /* include it first to set SX_PAGE_POOL_SUPPORTED value */

#if SX_PAGE_POOL_SUPPORTED
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
#include <net/page_pool/helpers.h>
#else /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)) */
#include <net/page_pool.h>
#endif /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)) */
#endif /* #if SX_PAGE_POOL_SUPPORTED */

#include <linux/seq_file.h>
#include "dq.h"
#include "rx_buffers.h"

#define SX_PCI_SKB_HEADROOM        (NET_SKB_PAD + NET_IP_ALIGN)
#define SX_PCI_RX_BUF_SW_OVERHEAD  (SX_PCI_SKB_HEADROOM + SKB_DATA_ALIGN(sizeof(struct skb_shared_info)))
#define SX_RX_MAX_WQE_LEN_IN_PAGES (3)

/******************************************************************/
/* HELPERS                                                        */
/******************************************************************/
static inline void __sync_sge_frag_for_cpu(struct sx_dq *rdq, struct sx_sge_internal *sge, int frag_idx, u16 frag_size)
{
    dma_sync_single_for_cpu(&rdq->dev->pdev->dev,
                            sge->u.rx.rx_frags[frag_idx].dma_addr,
                            frag_size,
                            DMA_FROM_DEVICE);
}

static void __sx_rx_buffers_deinit_wqes(struct sx_dq *rdq)
{
    struct sx_sge_internal *sge;
    int                     i, frag_num;

    for (i = 0; i < rdq->wqe_cnt; i++) {
        sge = &rdq->sge[i];

        if (sge->u.rx.mon_rdq_skb) { /* WJH RDQs */
            sx_skb_consume(sge->u.rx.mon_rdq_skb);
            sge->u.rx.mon_rdq_skb = NULL;
        }

        for (frag_num = 0; frag_num < SX_MAX_FRAGS_PER_WQE; frag_num++) {
            sx_rx_buffers_put(rdq, &sge->u.rx.rx_frags[frag_num]);
        }
    }
}

static int __sx_rx_buffers_init_wqes(struct sx_dq *rdq)
{
    int i, err = 0;

    for (i = 0; i < rdq->wqe_cnt; i++) {
        err = sx_core_post_recv(rdq); /* map pages to RDQ WQs */
        if (err) {
            sxd_log_err(rdq->dev, "failed to prepare RDQ buffers (dqn=%d)\n", rdq->dqn);
            break;
        }
    }

    if (err) {
        __sx_rx_buffers_deinit_wqes(rdq);
    }

    return err;
}

/******************************************************************/
/* COMMON                                                         */
/******************************************************************/
static int __sx_rx_buffers_prepare_wqe_frag(struct sx_dq *rdq,
                                            int           wqe_index,
                                            int           frag_index,
                                            dma_addr_t   *dma_addr_p,
                                            u16          *frag_len_p,
                                            bool         *skip_p);

int sx_rx_buffers_prepare_wqe(struct sx_dq *rdq, int wqe_index)
{
    struct sx_wqe *wqe = sx_get_wqe_by_index(rdq, wqe_index);
    dma_addr_t     dma_addr;
    u16            frag_len;
    int            frag_idx, err = 0;
    bool           skip;

    wqe->flags = 0;
    for (frag_idx = 0; frag_idx < SX_MAX_FRAGS_PER_WQE; frag_idx++) {
        err = __sx_rx_buffers_prepare_wqe_frag(rdq, wqe_index, frag_idx, &dma_addr, &frag_len, &skip);
        if (err) {
            break;
        }

        if (skip) {
            /* rx_frag[frag_idx] is not in use or already mapped and should not be updated */
            continue;
        }

        /* first fragment of the packet (SKB head / linear) holds 'skb_shared_info' that comes at the end */
        if (frag_idx == 0) {
            frag_len -= SX_PCI_RX_BUF_SW_OVERHEAD;
            dma_addr += SX_PCI_SKB_HEADROOM;
        }

        wqe->byte_count[frag_idx] = cpu_to_be16(frag_len);
        wqe->dma_addr[frag_idx] = cpu_to_be64(dma_addr);
    }

    /* if there was an error, we need to complete WQE settings */
    for (; frag_idx < SX_MAX_FRAGS_PER_WQE; frag_idx++) {
        wqe->byte_count[frag_idx] = 0;
        wqe->dma_addr[frag_idx] = 0;
    }

    return err;
}

#if SX_PAGE_POOL_SUPPORTED
/******************************************************************/
/* Page-Pool Implementation                                       */
/******************************************************************/

/* 2^order consecutive pages for each RX buffer allocation */
#define SX_PAGE_POOL_ORDER (2)

/* ------------------------------- */
/* DERIVED VALUES (DON'T TOUCH!)   */
/* ------------------------------- */
/* RX buffers allocation size in pages */
#define SX_PAGE_POOL_ALLOC_SIZE_IN_PAGES (1U << SX_PAGE_POOL_ORDER)

/* RX buffers allocation size in bytes */
#define SX_PAGE_POOL_ALLOC_SIZE_IN_BYTES (SX_PAGE_POOL_ALLOC_SIZE_IN_PAGES * PAGE_SIZE)

/* Maximum fragments per RX WQE according to configuration above */
#if (SX_RX_MAX_WQE_LEN_IN_PAGES > SX_PAGE_POOL_ALLOC_SIZE_IN_PAGES)
#define SX_PAGE_POOL_FRAGS_PER_WQE (SX_RX_MAX_WQE_LEN_IN_PAGES / SX_PAGE_POOL_ALLOC_SIZE_IN_PAGES)
#else
#define SX_PAGE_POOL_FRAGS_PER_WQE (1)
#endif

/* Validation that the configuration makes sense */
#if (SX_PAGE_POOL_FRAGS_PER_WQE < 1 || SX_PAGE_POOL_FRAGS_PER_WQE > SX_MAX_FRAGS_PER_WQE)
#error RX buffers configuration is invalid!
#endif

int sx_rx_buffers_init(struct sx_dq *rdq)
{
    struct page_pool_params params;
    int                     err = 0;

    memset(&params, 0, sizeof(params));
    params.flags = PP_FLAG_DMA_MAP;
    params.order = SX_PAGE_POOL_ORDER;
    params.pool_size = rdq->wqe_cnt * SX_PAGE_POOL_FRAGS_PER_WQE;
    params.dev = &rdq->dev->pdev->dev;
    params.nid = dev_to_node(params.dev);
    params.dma_dir = DMA_FROM_DEVICE;

    rdq->page_pool = page_pool_create(&params);
    if (IS_ERR(rdq->page_pool)) {
        err = PTR_ERR(rdq->page_pool);
        rdq->page_pool = NULL;
        sxd_log_err(rdq->dev, "failed to create page pool for RDQ %d (err=%d)\n", rdq->dqn, err);
        goto out;
    }

    err = __sx_rx_buffers_init_wqes(rdq);
    if (err) {
        page_pool_destroy(rdq->page_pool);
    }

out:
    return err;
}

void sx_rx_buffers_deinit(struct sx_dq *rdq)
{
    __sx_rx_buffers_deinit_wqes(rdq);
    page_pool_destroy(rdq->page_pool);
    rdq->page_pool = NULL;
}

void sx_rx_buffers_put(struct sx_dq *rdq, struct sx_rx_sge_data *rx_sge_data)
{
    if (rx_sge_data->page) {
        page_pool_put_full_page(rdq->page_pool, rx_sge_data->page, false);
        rx_sge_data->page = NULL;
        rx_sge_data->dma_addr = (dma_addr_t)0;
    }
}

struct sk_buff * sx_rx_buffers_build_skb(struct sx_dq *rdq, int wqe_index, u16 total_byte_count)
{
    struct sx_sge_internal *sge = &rdq->sge[wqe_index];
    struct sk_buff         *skb;
    struct page           * page;
    bool                    is_linear;
    u16                     frag_size;
    u16                     skb_initial_size;
    int                     i = 1;
    void                   *data;

    skb_initial_size = SKB_DATA_ALIGN(total_byte_count) + SX_PCI_RX_BUF_SW_OVERHEAD;
    if (likely(skb_initial_size <= SX_PAGE_POOL_ALLOC_SIZE_IN_BYTES)) {
        frag_size = total_byte_count;
        is_linear = true;
    } else {
        skb_initial_size = SX_PAGE_POOL_ALLOC_SIZE_IN_BYTES;
        frag_size = SX_PAGE_POOL_ALLOC_SIZE_IN_BYTES - SX_PCI_RX_BUF_SW_OVERHEAD;
        is_linear = false;
    }

    data = page_address(sge->u.rx.rx_frags[0].page);
    net_prefetch(data);

    skb = build_skb(data, skb_initial_size);
    if (unlikely(!skb)) {
        sx_health_report_error_generic(rdq->dev, SXD_HEALTH_SEVERITY_FATAL, "Failed to allocate RX skb");
        skb = NULL;
        goto out;
    }

    skb_mark_for_recycle(skb);

    __sync_sge_frag_for_cpu(rdq, sge, 0, SX_PCI_SKB_HEADROOM + frag_size);
    skb_reserve(skb, SX_PCI_SKB_HEADROOM);
    skb_put(skb, frag_size);

    if (likely(is_linear)) {
        goto recycle_pages;
    }

    total_byte_count -= frag_size;

    for ( ; total_byte_count > 0 && i < SX_PAGE_POOL_FRAGS_PER_WQE; i++) {
        page = sge->u.rx.rx_frags[i].page;
        frag_size = (total_byte_count < SX_PAGE_POOL_ALLOC_SIZE_IN_BYTES) ?
                    total_byte_count : SX_PAGE_POOL_ALLOC_SIZE_IN_BYTES;
        __sync_sge_frag_for_cpu(rdq, sge, i, frag_size);
        skb_add_rx_frag(skb, skb_shinfo(skb)->nr_frags, page, 0, frag_size, SX_PAGE_POOL_ALLOC_SIZE_IN_BYTES);
        total_byte_count -= frag_size;
    }

    WARN_ON_ONCE(total_byte_count > 0);

recycle_pages:

    /* all pages that we have attached to the skb will be returned to the page pool
     * once the skb is freed. now we just need to mark these pages as 'not in our control' so
     * sx_core_post_recv() will allocate new pages to these entries for the next cycle.
     */
    for (i--; i >= 0; i--) {
        sge->u.rx.rx_frags[i].page = NULL; /* page is consumed (need to allocate in next cycle) */
    }

out:
    return skb;
}

static int __sx_rx_buffers_prepare_wqe_frag(struct sx_dq *rdq,
                                            int           wqe_index,
                                            int           frag_index,
                                            dma_addr_t   *dma_addr_p,
                                            u16          *frag_len_p,
                                            bool         *skip_p)
{
    struct sx_rx_sge_data *sge_frag = &rdq->sge[wqe_index].u.rx.rx_frags[frag_index];

    if (sge_frag->page) {
        /* rx_frag[frag_idx] already has a page mapped and WQE is updated */
        *skip_p = true;
        return 0;
    }

    *skip_p = false;
    if (frag_index >= SX_PAGE_POOL_FRAGS_PER_WQE) {
        *dma_addr_p = (dma_addr_t)0;
        *frag_len_p = 0;
        return 0;
    }

    sge_frag->page = page_pool_dev_alloc_pages(rdq->page_pool);
    if (!sge_frag->page) {
        goto error;
    }

    sge_frag->dma_addr = page_pool_get_dma_addr(sge_frag->page);

    *dma_addr_p = sge_frag->dma_addr;
    *frag_len_p = SX_PAGE_POOL_ALLOC_SIZE_IN_BYTES;
    return 0;

error:
    sge_frag->dma_addr = (dma_addr_t)0;
    sx_health_report_error_generic(rdq->dev,
                                   SXD_HEALTH_SEVERITY_FATAL,
                                   "Failed to allocate page(s) from page pool");
    return -ENOMEM;
}

void sx_rx_buffers_config_dump(struct seq_file *m)
{
    seq_printf(m, "RX-Buffer configuration\n");
    seq_printf(m, "    Implementation .................................... Page-Pool\n");
    seq_printf(m, "    SX_RX_MAX_WQE_LEN_IN_PAGES ........................ %u\n", (u32)SX_RX_MAX_WQE_LEN_IN_PAGES);
    seq_printf(m, "    SX_PCI_SKB_HEADROOM ............................... %u\n", (u32)SX_PCI_SKB_HEADROOM);
    seq_printf(m, "    SX_PCI_RX_BUF_SW_OVERHEAD ......................... %u\n", (u32)SX_PCI_RX_BUF_SW_OVERHEAD);
    seq_printf(m, "    SX_PAGE_POOL_ORDER ................................ %u\n", (u32)SX_PAGE_POOL_ORDER);
    seq_printf(m, "    SX_PAGE_POOL_ALLOC_SIZE_IN_PAGES .................. %u\n",
               (u32)SX_PAGE_POOL_ALLOC_SIZE_IN_PAGES);
    seq_printf(m, "    SX_PAGE_POOL_ALLOC_SIZE_IN_BYTES .................. %u\n",
               (u32)SX_PAGE_POOL_ALLOC_SIZE_IN_BYTES);
    seq_printf(m, "    SX_PAGE_POOL_FRAGS_PER_WQE ........................ %u\n", (u32)SX_PAGE_POOL_FRAGS_PER_WQE);
}

#else /* #if SX_PAGE_POOL_SUPPORTED */
/******************************************************************/
/* Linear-DMA-Allocation Implementation                           */
/******************************************************************/

#define SX_RX_MAX_WQE_LEN_IN_BYTES (SX_RX_MAX_WQE_LEN_IN_PAGES * PAGE_SIZE + SX_PCI_RX_BUF_SW_OVERHEAD)

int sx_rx_buffers_init(struct sx_dq *rdq)
{
    return __sx_rx_buffers_init_wqes(rdq);
}

void sx_rx_buffers_deinit(struct sx_dq *rdq)
{
    __sx_rx_buffers_deinit_wqes(rdq);
}

void sx_rx_buffers_put(struct sx_dq *rdq, struct sx_rx_sge_data *rx_sge_data)
{
    if (rx_sge_data->dma_addr) {
        dma_unmap_single(&rdq->dev->pdev->dev,
                         rx_sge_data->dma_addr,
                         SX_RX_MAX_WQE_LEN_IN_BYTES,
                         DMA_FROM_DEVICE);
        rx_sge_data->dma_addr = (dma_addr_t)0;
    }

    if (rx_sge_data->skb) {
        consume_skb(rx_sge_data->skb);
        rx_sge_data->skb = NULL;
    }
}

struct sk_buff * sx_rx_buffers_build_skb(struct sx_dq *rdq, int wqe_index, u16 total_byte_count)
{
    struct sx_sge_internal *sge = &rdq->sge[wqe_index];
    struct sx_rx_sge_data  *sge_frag_0 = &sge->u.rx.rx_frags[0];
    struct sk_buff         *skb;

    skb = sge_frag_0->skb;
    __sync_sge_frag_for_cpu(rdq, sge, 0, SX_PCI_SKB_HEADROOM + total_byte_count);
    skb_reserve(skb, SX_PCI_SKB_HEADROOM);
    skb_put(skb, total_byte_count);

    sge_frag_0->skb = NULL; /* prevent following sx_rx_buffers_put() call from freeing the skb */
    sx_rx_buffers_put(rdq, sge_frag_0);

    return skb;
}

static int __sx_rx_buffers_prepare_wqe_frag(struct sx_dq *rdq,
                                            int           wqe_index,
                                            int           frag_index,
                                            dma_addr_t   *dma_addr_p,
                                            u16          *frag_len_p,
                                            bool         *skip_p)
{
    struct sx_rx_sge_data *sge_frag_0 = &rdq->sge[wqe_index].u.rx.rx_frags[0]; /* we care only if frag_index == 0 */
    bool                   skb_valid = false;

    /* when not using page-pool, we use a single allocation (non-fragmented skb) */

    *skip_p = false;
    if (frag_index > 0) {
        *dma_addr_p = (dma_addr_t)0;
        *frag_len_p = 0;
        return 0;
    }

    WARN_ON_ONCE(sge_frag_0->skb != NULL);

    sge_frag_0->skb = alloc_skb(SX_RX_MAX_WQE_LEN_IN_BYTES, GFP_ATOMIC);
    if (!sge_frag_0->skb) {
        WARN_ON_ONCE(1);
        goto error;
    }

    skb_valid = true;
    sge_frag_0->dma_addr = dma_map_single(&rdq->dev->pdev->dev,
                                          sge_frag_0->skb->data,
                                          SX_RX_MAX_WQE_LEN_IN_BYTES,
                                          DMA_FROM_DEVICE);
    if (unlikely(dma_mapping_error(&rdq->dev->pdev->dev, sge_frag_0->dma_addr))) {
        WARN_ON_ONCE(1);
        goto error;
    }

    *dma_addr_p = sge_frag_0->dma_addr;
    *frag_len_p = SX_RX_MAX_WQE_LEN_IN_BYTES;
    return 0;

error:
    if (skb_valid) {
        kfree_skb(sge_frag_0->skb);
    }
    sge_frag_0->skb = NULL;
    sge_frag_0->dma_addr = (dma_addr_t)0;
    sx_health_report_error_generic(rdq->dev, SXD_HEALTH_SEVERITY_FATAL, "Failed to allocate RDQ WQE buffer");
    return -ENOMEM;
}

void sx_rx_buffers_config_dump(struct seq_file *m)
{
    seq_printf(m, "RX-Buffer configuration\n");
    seq_printf(m, "    Implementation .................................... Linear-DMA-Allocation\n");
    seq_printf(m, "    SX_RX_MAX_WQE_LEN_IN_BYTES (with overhead)......... %u\n", (u32)SX_RX_MAX_WQE_LEN_IN_BYTES);
    seq_printf(m, "    SX_PCI_SKB_HEADROOM ............................... %u\n", (u32)SX_PCI_SKB_HEADROOM);
    seq_printf(m, "    SX_PCI_RX_BUF_SW_OVERHEAD ......................... %u\n", (u32)SX_PCI_RX_BUF_SW_OVERHEAD);
}

#endif /* #if SX_PAGE_POOL_SUPPORTED */
