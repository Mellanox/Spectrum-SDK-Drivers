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

#ifndef __DEV_DB_H__
#define __DEV_DB_H__

#include <linux/mlx_sx/device.h>
#include <linux/mlx_sx/kernel_user.h>

#define SX_SINGLE_ASIC_DEFAULT_DEV_ID (1)

int sx_dev_db_pdev_to_dev_id(struct pci_dev *pdev, sxd_dev_id_t *dev_id);
int sx_dev_db_set_dev_by_id(sxd_dev_id_t dev_id, struct sx_dev *dev);
struct sx_dev * sx_dev_db_get_dev_by_id(sxd_dev_id_t dev_id);
struct sx_dev * sx_dev_db_get_default_device(void);
int sx_dev_db_add_device(struct sx_dev *dev);
int sx_dev_db_remove_device(struct sx_dev *dev);
int sx_dev_db_reassign_dev_id_to_default_device(sxd_dev_id_t dev_id);

int dev_db_init(void);
void dev_db_deinit(void);

int sx_dev_db_dump(struct seq_file *m, void *v, void *context);

struct sx_dev_db_iterator_seq_file_context {
    struct seq_file *m;
    void            *v;
    void            *context;
};

typedef bool (*sx_dev_db_iterator_cb_t)(struct sx_dev *dev, void *context);
int sx_dev_db_iterate(sx_dev_db_iterator_cb_t iter_cb, void *context);

#endif /* __DEV_DB_H__ */
