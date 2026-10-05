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

#ifndef ALLOC_H_
#define ALLOC_H_

/************************************************
 * Includes
 ***********************************************/

#include "sx.h"

/************************************************
 * Function prototypes
 ***********************************************/
u32 sx_bitmap_num_bits(struct sx_bitmap *bitmap);
void sx_bitmap_clear_all(struct sx_bitmap *bitmap);
void sx_bitmap_copy(struct sx_bitmap *dst, struct sx_bitmap *src);
u32 sx_bitmap_test(struct sx_bitmap *bitmap, u32 obj);
u32 sx_bitmap_set(struct sx_bitmap *bitmap, u32 obj);
void sx_set_u64_to_bitmap(u64 obj, struct sx_bitmap *bitmap);
u32 sx_bitmap_alloc(struct sx_bitmap *bitmap);
void sx_bitmap_unset(struct sx_bitmap *bitmap, u32 obj);
int sx_buf_alloc(struct sx_dev *dev, int size, int max_direct,
                 struct sx_buf *buf, int init_val);
void sx_fill_page_list(__be64 *dma_addrs, struct sx_buf *buf);
void sx_buf_free(struct sx_dev *dev, int size, struct sx_buf *buf);
int sx_bitmap_init(struct sx_bitmap *bitmap, u32 num);

#endif /* ALLOC_H_ */

/************************************************
 *                  EOF                         *
 ***********************************************/
