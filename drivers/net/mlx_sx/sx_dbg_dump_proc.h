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

#ifndef __SX_DBG_DUMP_PROC_H__
#define __SX_DBG_DUMP_PROC_H__

#define DBG_DUMP_CMD_NAME "dbg_cmd"

int sx_dbg_dump_proc_fs_init(void);
void sx_dbg_dump_proc_fs_deinit(void);

int sx_dbg_dump_proc_fs_dev_init(struct sx_dev *dev);
void sx_dbg_dump_proc_fs_dev_deinit(struct sx_dev *dev);
struct sx_dev * sx_dbg_dump_get_device(struct seq_file *m);

#endif /* __SX_DBG_DUMP_PROC_H__ */
