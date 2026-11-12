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

#ifndef SX_ICM_H
#define SX_ICM_H

#include <linux/pci.h>

struct sx_dev;

#define SX_MAP_FA_INVALID_HANDLE (0)
typedef unsigned long sx_map_fa_handle_t;

int sx_map_fa_item_alloc(struct sx_dev *dev, u32 num_of_pages, sx_map_fa_handle_t *handle);
void sx_map_fa_item_dealloc(sx_map_fa_handle_t handle);
void * sx_map_fa_item_get_page(sx_map_fa_handle_t handle, u32 page_index);

typedef bool (*sx_map_fa_iterator_cb_t)(struct sx_dev *dev,
                                        void          *buff,
                                        dma_addr_t     dma_addr,
                                        u32            num_of_pages,
                                        bool           is_last,
                                        void          *context);
void sx_map_fa_item_iterate(sx_map_fa_handle_t handle, sx_map_fa_iterator_cb_t callback, void *context);

#endif /* SX_ICM_H */
