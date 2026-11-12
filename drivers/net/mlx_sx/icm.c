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

#include <linux/init.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/scatterlist.h>
#include <linux/mlx_sx/cmd.h>
#include "sx.h"
#include "icm.h"
#include "fw.h"
#include <linux/vmalloc.h>

#define SX_MAX_PAGES_PER_DESC (512)

struct sx_map_fa_desc {
    struct list_head list;
    u32              num_of_pages;
    void            *buff;
    dma_addr_t       dma_addr;
};

struct sx_map_fa_item {
    struct list_head desc_list;
    struct sx_dev   *dev;
};

/* returns the number of pages associated with the new descriptor, 0 if failed */
static u32 __sx_map_fa_alloc_desc(struct sx_dev *dev, struct sx_map_fa_item *item, u32 num_of_pages)
{
    struct sx_map_fa_desc *desc;
    bool                   ok = false;

    desc = vzalloc(sizeof(struct sx_map_fa_desc));
    if (!desc) {
        sxd_log_err(dev, "MAP_FA: failed to allocate item descriptor\n");
        return 0;
    }

    INIT_LIST_HEAD(&desc->list);
    desc->num_of_pages = (num_of_pages >= SX_MAX_PAGES_PER_DESC) ? SX_MAX_PAGES_PER_DESC :
                         rounddown_pow_of_two(num_of_pages);

    /* start allocating a bulk of maximum allowed pages. if we fail to do that, we decrease the number by
     * half (increase allocation probability to succeed) until allocation done.
     * if we decrement to 0 pages, we fail.
     */
    do {
        desc->buff = dma_alloc_coherent(&dev->pdev->dev, desc->num_of_pages * PAGE_SIZE, &desc->dma_addr, GFP_KERNEL);
        if (desc->buff) {
            ok = true;
            break;
        }

        sxd_log_debug(dev, "MAP_FA: failed to allocate %u continuous pages\n",
                      desc->num_of_pages);
        desc->num_of_pages >>= 1;
    } while (desc->num_of_pages > 0);

    if (ok) {
        list_add_tail(&desc->list, &item->desc_list);
        return desc->num_of_pages;
    }

    vfree(desc);
    return 0;
}

int sx_map_fa_item_alloc(struct sx_dev *dev, u32 num_of_pages, sx_map_fa_handle_t *handle_p)
{
    struct sx_map_fa_item *item;
    sx_map_fa_handle_t     handle = SX_MAP_FA_INVALID_HANDLE;
    u32                    pages_allocated;

    if (num_of_pages == 0) {
        sxd_log_err(dev, "MAP_FA: number of pages must not be 0\n");
        return -EINVAL;
    }

    item = vzalloc(sizeof(*item));
    if (!item) {
        sxd_log_err(dev, "MAP_FA: cannot allocate item\n");
        return -ENOMEM;
    }

    handle = (sx_map_fa_handle_t)item;
    INIT_LIST_HEAD(&item->desc_list);
    item->dev = dev;

    while (num_of_pages > 0) {
        pages_allocated = __sx_map_fa_alloc_desc(dev, item, num_of_pages);
        if (pages_allocated == 0) {
            goto cleanup;
        }

        num_of_pages -= pages_allocated;
    }

    *handle_p = handle;
    return 0;

cleanup:
    sx_map_fa_item_dealloc(handle);
    return -ENOMEM;
}

void sx_map_fa_item_dealloc(sx_map_fa_handle_t handle)
{
    struct sx_map_fa_item *item = (struct sx_map_fa_item*)handle;
    struct sx_map_fa_desc *desc, *tmp;

    if (!item) {
        return;
    }

    list_for_each_entry_safe(desc, tmp, &item->desc_list, list) {
        dma_free_coherent(&item->dev->pdev->dev, desc->num_of_pages * PAGE_SIZE, desc->buff, desc->dma_addr);
        list_del(&desc->list);
        vfree(desc);
    }

    vfree(item);
}

struct sx_map_fa_get_page_ctx {
    u32   page_index;
    void* ret_buff;
};

static bool __map_fa_get_page_cb(struct sx_dev *dev,
                                 void          *buff,
                                 dma_addr_t     dma_addr,
                                 u32            num_of_pages,
                                 bool           is_last,
                                 void          *context)
{
    struct sx_map_fa_get_page_ctx *get_page_ctx = (struct sx_map_fa_get_page_ctx*)context;

    if (get_page_ctx->page_index < num_of_pages) {
        get_page_ctx->ret_buff = buff + (get_page_ctx->page_index * PAGE_SIZE);
        return true; /* stop iteration */
    }

    get_page_ctx->page_index -= num_of_pages;
    return false; /* continue iteration */
}

void * sx_map_fa_item_get_page(sx_map_fa_handle_t handle, u32 page_index)
{
    struct sx_map_fa_get_page_ctx ctx = {
        .page_index = page_index,
        .ret_buff = NULL
    };

    sx_map_fa_item_iterate(handle, __map_fa_get_page_cb, &ctx);
    return ctx.ret_buff;
}

void sx_map_fa_item_iterate(sx_map_fa_handle_t handle, sx_map_fa_iterator_cb_t callback, void *context)
{
    struct sx_map_fa_item *item = (struct sx_map_fa_item*)handle;
    struct sx_map_fa_desc *desc;
    bool                   is_last;
    bool                   stop;

    if (!item) {
        return;
    }

    list_for_each_entry(desc, &item->desc_list, list) {
        is_last = list_is_last(&desc->list, &item->desc_list);
        stop = callback(item->dev, desc->buff, desc->dma_addr, desc->num_of_pages, is_last, context);
        if (stop) {
            break;
        }
    }
}
