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

bool sx_dbg_dump_proc_fs_init(struct sx_dev *dev, void *context);
bool sx_dbg_dump_proc_fs_deinit(struct sx_dev *dev, void *context);
struct sx_dev * sx_dbg_dump_get_device(struct seq_file *m);
void sx_dbg_dump_proc_fs_remove_parent_proc_folder(void);

struct sx_name_device_tuple {
    const char  *name;
    sxd_dev_id_t dev_id;
};

struct read_handler_info {
    struct list_head            list;
    sx_dbg_dump_proc_show_cb_t  show_cb;
    sx_dbg_dump_proc_size_cb_t  size_cb;
    void                       *context;
    struct sx_name_device_tuple read_handler_struct_key;
};


#endif /* __SX_DBG_DUMP_PROC_H__ */
