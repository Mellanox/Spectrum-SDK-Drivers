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

#ifndef __SX_BFD_WORKQUEUE_H
#define __SX_BFD_WORKQUEUE_H

#include "sx_bfd_main.h"

typedef struct sx_bfd_delayed_work sx_bfd_delayed_work_t;
typedef void (*handler_func)(struct sx_bfd_dev_db *bfd_dev_db, uint32_t data);

int sx_bfd_create_delayed_work(struct sx_bfd_dev_db   * bfd_dev_db,
                               sx_bfd_delayed_work_t ** dwork,
                               handler_func             func,
                               uint32_t                 data);

void sx_bfd_destroy_delayed_work(sx_bfd_delayed_work_t * dwork);
void sx_bfd_destroy_delayed_work_sync(sx_bfd_delayed_work_t * dwork);

void sx_bfd_delayed_work_dispatch(struct workqueue_struct * workqueue,
                                  sx_bfd_delayed_work_t   * dwork,
                                  unsigned long             delay);
void sx_bfd_delayed_work_cancel(sx_bfd_delayed_work_t * dwork);

int sx_bfd_workqueue_init(struct sx_bfd_dev_db *bfd_dev_db);

void sx_bfd_workqueue_deinit(struct sx_bfd_dev_db *bfd_dev_db);


#endif /* __sx_bfd_WORKQUEUE_H */
