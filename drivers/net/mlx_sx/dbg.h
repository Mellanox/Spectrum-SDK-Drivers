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

#ifndef __DBG_H__
#define __DBG_H__

struct sx_dev;
struct seq_file;

int sx_cmd_ifc_dump(struct seq_file *m, void *v, void *context);
int sx_dbg_host_ifc_init(struct sx_dev *dev);
int sx_dbg_db_init(struct sx_dev *dev);
void sx_dbg_db_deinit(struct sx_dev *dev);
void sx_dbg_host_ifc_deinit(struct sx_dev *dev);

#endif /* __DBG_H__ */
