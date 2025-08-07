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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/mlx_sx/driver.h>
#include <linux/mlx_sx/kernel_user.h>
#include "sx.h"
#include "dev_db.h"
#include "alloc.h"
#include "mmap.h"
#include "ber_monitor.h"
#include "sx_dbg_dump_proc.h"
#include "bulk_cntr_db.h"
#include "sx_af_counters.h"
#include "health_check.h"

/************************************************
 *  MACROS
 ***********************************************/

#define SEQ_FILE_SIZE (64 * 1024)

/************************************************
 *  LOCAL VARIABLES
 ***********************************************/
struct proc_dir_entry *__root_dir = NULL;

struct sx_read_handler_info {
    struct list_head           list;
    sx_dbg_dump_proc_show_cb_t show_cb;
    sx_dbg_dump_proc_size_cb_t size_cb;
    void                      *context;
    const char                *name;
    struct sx_dev             *dev;
};

struct sx_write_handler_info {
    struct list_head          list;
    sx_dbg_dump_proc_cmd_cb_t cmd_cb;
    char                     *cmd;
    char                     *usage;
    char                     *description;
    void                     *context;
    struct sx_dev            *dev;
};

void sx_dbg_dump_print_header(struct seq_file *m, const char* header)
{
    seq_printf(m,
               "\n========================================================================================================================\n");
    seq_printf(m, "%s", header);
    seq_printf(m,
               "\n========================================================================================================================\n\n");
}
EXPORT_SYMBOL(sx_dbg_dump_print_header);

static struct sx_read_handler_info * __name_to_info(struct sx_priv *priv, const char *name)
{
    struct sx_read_handler_info *iter = NULL;

    if (!name) {
        return NULL;
    }

    list_for_each_entry(iter, &priv->dbg_dump_proc.read_entries, list) {
        if (strcmp(name, iter->name) == 0) {
            return iter;
        }
    }

    return NULL;
}

static struct sx_write_handler_info * __cmd_to_info(struct sx_priv *priv, const char *cmd)
{
    struct sx_write_handler_info *iter = NULL;

    list_for_each_entry(iter, &priv->dbg_dump_proc.write_entries, list) {
        if (strcmp(cmd, iter->cmd) == 0) {
            return iter;
        }
    }

    return NULL;
}

/* ******************************************************************************* */
/* PROC DUMP INFRA FUNCTIONS                                                       */
/* ******************************************************************************* */

static int __sx_db_dump_proc_show(struct seq_file* m, void* v)
{
    struct sx_read_handler_info *info = (struct sx_read_handler_info*)m->private;

    seq_printf(m, "******************** DEBUG PROC FILE: %s ********************\n\n", info->name);
    return info->show_cb(m, v, info->context);
}

static int __sx_dbg_dump_proc_open(struct inode *inode, struct file *file)
{
    struct sx_read_handler_info *iter = (struct sx_read_handler_info*)PDE_DATA_DEF(inode);
    struct sx_priv              *priv = sx_priv(iter->dev);
    int                          ret = 0;

    mutex_lock(&priv->dbg_dump_proc.lock);
    ret = single_open_size(file, __sx_db_dump_proc_show, iter, iter->size_cb ? iter->size_cb() : SEQ_FILE_SIZE);
    mutex_unlock(&priv->dbg_dump_proc.lock);

    return ret;
}

int sx_dbg_dump_read_handler_register(const char                *name,
                                      sx_dbg_dump_proc_show_cb_t show_cb,
                                      sx_dbg_dump_proc_size_cb_t size_cb,
                                      struct sx_dev             *dev,
                                      void                      *context)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    static const struct proc_ops __sx_dbg_dump_proc_handler = {
        .proc_open = __sx_dbg_dump_proc_open,
        .proc_read = seq_read,
        .proc_lseek = seq_lseek,
        .proc_release = single_release
    };
#else
    static const struct file_operations __sx_dbg_dump_proc_handler = {
        .owner = THIS_MODULE,
        .open = __sx_dbg_dump_proc_open,
        .read = seq_read,
        .llseek = seq_lseek,
        .release = single_release
    };
#endif

    struct proc_dir_entry       *pde = NULL;
    struct sx_read_handler_info *read_handler_info = NULL;
    struct sx_priv              *priv = NULL;
    int                          err = 0;

    if (!dev || !name || !show_cb) {
        sxd_log_err("no device, proc file name is empty or no show handler\n");
        return -EINVAL;
    }

    priv = sx_priv(dev);

    mutex_lock(&priv->dbg_dump_proc.lock);

    if (__name_to_info(priv, name)) {
        sxd_log_err("proc file %s already exists\n", name);
        err = -EEXIST;
        goto error;
    }

    err = -ENOMEM;

    read_handler_info = kzalloc(sizeof(struct sx_read_handler_info), GFP_KERNEL);
    if (read_handler_info == NULL) {
        sxd_log_warning("create info for proc %s failed\n", name);
        goto error;
    }

    read_handler_info->name = kstrdup(name, GFP_KERNEL);
    if (read_handler_info->name == NULL) {
        sxd_log_warning("allocate name of info for proc %s failed\n", name);
        goto error;
    }
    read_handler_info->dev = dev;
    INIT_LIST_HEAD(&read_handler_info->list);
    read_handler_info->show_cb = show_cb;
    read_handler_info->size_cb = size_cb;
    read_handler_info->context = context;

    pde = proc_create_data(name,
                           S_IFREG | S_IRUGO,
                           priv->dbg_dump_proc.root_dir,
                           &__sx_dbg_dump_proc_handler,
                           read_handler_info);
    if (pde == NULL) {
        sxd_log_warning("create proc %s failed\n", name);
        goto error;
    }

    list_add(&read_handler_info->list, &priv->dbg_dump_proc.read_entries);
    err = 0;

out:
    mutex_unlock(&priv->dbg_dump_proc.lock);
    return err;

error:
    if (read_handler_info) {
        if (read_handler_info->name) {
            kfree(read_handler_info->name);
        }

        kfree(read_handler_info);
    }

    goto out;
}
EXPORT_SYMBOL(sx_dbg_dump_read_handler_register);

static void __sx_dbg_dump_read_handler_unregister(struct sx_read_handler_info *read_handler_info, struct sx_priv *priv)
{
    list_del(&read_handler_info->list);
    remove_proc_entry(read_handler_info->name, priv->dbg_dump_proc.root_dir);
    kfree(read_handler_info->name);
    kfree(read_handler_info);
}

void sx_dbg_dump_read_handler_unregister(const char *name, struct sx_dev *dev)
{
    struct sx_read_handler_info *read_handler_info = NULL;
    struct sx_priv              *priv = NULL;

    if (!dev || !name) {
        sxd_log_err("no device or proc file name is empty\n");
        return;
    }

    priv = sx_priv(dev);

    mutex_lock(&priv->dbg_dump_proc.lock);
    read_handler_info = __name_to_info(priv, name);
    if (read_handler_info == NULL) {
        sxd_log_err("proc file %s does not exist\n", name);
    } else {
        __sx_dbg_dump_read_handler_unregister(read_handler_info, priv);
    }

    mutex_unlock(&priv->dbg_dump_proc.lock);
}
EXPORT_SYMBOL(sx_dbg_dump_read_handler_unregister);

int sx_dbg_dump_write_handler_register(const char               *cmd,
                                       const char               *usage,
                                       const char               *description,
                                       sx_dbg_dump_proc_cmd_cb_t cmd_cb,
                                       struct sx_dev            *dev,
                                       void                     *context)
{
    struct sx_write_handler_info *write_handler_info = NULL;
    struct sx_priv               *priv = NULL;
    int                           err = 0;

    if (!dev || !cmd || !usage || !description || !cmd_cb) {
        sxd_log_err("write handler invalid arguments\n");
        return -EINVAL;
    }

    priv = sx_priv(dev);

    mutex_lock(&priv->dbg_dump_proc.lock);

    if (__cmd_to_info(priv, cmd)) {
        sxd_log_err("command handler '%s' already exists\n", cmd);
        err = -EEXIST;
        goto error;
    }

    err = -ENOMEM;

    write_handler_info = kzalloc(sizeof(struct sx_write_handler_info), GFP_KERNEL);
    if (write_handler_info == NULL) {
        sxd_log_warning("create cmd proc '%s' failed\n", cmd);
        goto error;
    }

    write_handler_info->cmd = kstrdup(cmd, GFP_KERNEL);
    if (write_handler_info->cmd == NULL) {
        sxd_log_warning("allocate command of info for proc %s failed\n", cmd);
        goto error;
    }

    write_handler_info->usage = kstrdup(usage, GFP_KERNEL);
    if (write_handler_info->usage == NULL) {
        sxd_log_warning("allocate usage of info for proc %s failed\n", cmd);
        goto error;
    }

    write_handler_info->description = kstrdup(description, GFP_KERNEL);
    if (write_handler_info->description == NULL) {
        sxd_log_warning("allocate description of info for proc %s failed\n", cmd);
        goto error;
    }

    INIT_LIST_HEAD(&write_handler_info->list);
    write_handler_info->cmd_cb = cmd_cb;
    write_handler_info->context = context;
    write_handler_info->dev = dev;

    list_add(&write_handler_info->list, &priv->dbg_dump_proc.write_entries);
    err = 0;

out:
    mutex_unlock(&priv->dbg_dump_proc.lock);
    return err;

error:
    if (write_handler_info) {
        if (write_handler_info->cmd) {
            kfree(write_handler_info->cmd);
        }

        if (write_handler_info->usage) {
            kfree(write_handler_info->usage);
        }

        if (write_handler_info->description) {
            kfree(write_handler_info->description);
        }

        kfree(write_handler_info);
    }

    goto out;
}
EXPORT_SYMBOL(sx_dbg_dump_write_handler_register);

static void __sx_dbg_dump_write_handler_unregister(struct sx_write_handler_info *write_handler_info)
{
    list_del(&write_handler_info->list);
    kfree(write_handler_info->cmd);
    kfree(write_handler_info->usage);
    kfree(write_handler_info->description);
    kfree(write_handler_info);
}


void sx_dbg_dump_write_handler_unregister(const char *cmd, struct sx_dev *dev)
{
    struct sx_write_handler_info *write_handler_info = NULL;
    struct sx_priv               *priv = NULL;

    if (!cmd || !dev) {
        sxd_log_err("write handler unregister: invalid arguments\n");
        return;
    }

    priv = sx_priv(dev);

    mutex_lock(&priv->dbg_dump_proc.lock);

    write_handler_info = __cmd_to_info(priv, cmd);
    if (write_handler_info == NULL) {
        sxd_log_err("cmd handler '%s' does not exist\n", cmd);
    } else {
        __sx_dbg_dump_write_handler_unregister(write_handler_info);
    }

    mutex_unlock(&priv->dbg_dump_proc.lock);
}
EXPORT_SYMBOL(sx_dbg_dump_write_handler_unregister);

static ssize_t __sx_dbg_dump_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
#define SX_WRITE_PROC_MAX_ARGS (10)

    char                          buff[1024] = { 0 };
    const char                   *argv[SX_WRITE_PROC_MAX_ARGS] = { 0 };
    char                         *token = buff, *tmp = buff;
    size_t                        buff_len = sizeof(buff) - 1;
    int                           argc = 0, rc = 0;
    struct inode                 *inode = file_inode(file);
    struct sx_dev                *dev = PDE_DATA_DEF(inode); /* proc data for write-entry is the sx_dev */
    struct sx_priv               *priv = sx_priv(dev);
    struct sx_write_handler_info *info = NULL;

    if (count < buff_len) {
        buff_len = count;
    }

    if (copy_from_user(buff, buffer, buff_len) != 0) {
        return -EFAULT;
    }

    while (tmp && argc < SX_WRITE_PROC_MAX_ARGS) {
        token = strsep(&tmp, " ");
        argv[argc++] = strim(token);
    }

    if (tmp) {
        sxd_log_err("too many arguments for a single command!\n");
        goto out;
    }

    mutex_lock(&priv->dbg_dump_proc.lock);
    info = __cmd_to_info(priv, argv[0]);
    if (info) {
        rc = info->cmd_cb(argc, argv, info->context);
        if (rc) {
            sxd_log_err("command '%s' failed (err=%d)\n", argv[0], rc);
        }
    } else {
        sxd_log_err("unknown command '%s'\n", argv[0]);
    }
    mutex_unlock(&priv->dbg_dump_proc.lock);

out:
    return count;
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static const struct proc_ops __sx_command_proc_handler = {
    .proc_write = __sx_dbg_dump_write
};
#else
static const struct file_operations __sx_command_proc_handler = {
    .write = __sx_dbg_dump_write
};
#endif

static int __help_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_write_handler_info *iter_w = NULL, *tmp_w = NULL;
    char                         *usage = NULL, *next = NULL;
    struct sx_dev                *dev = sx_dbg_dump_get_device(m);
    struct sx_priv               *priv = sx_priv(dev);

    sx_dbg_dump_print_header(m, "Commands list");

    mutex_lock(&priv->dbg_dump_proc.lock);

    seq_printf(m, "*** All commands should be written to /proc/dbg_dump/dev_X/dbg_cmd ***\n\n");

    sx_dbg_dump_print_separate_line(m);
    list_for_each_entry_safe(iter_w, tmp_w, &priv->dbg_dump_proc.write_entries, list) {
        seq_printf(m, "Command: %s\n", iter_w->cmd);
        seq_printf(m, "Description: %s\n", iter_w->description);
        seq_printf(m, "Usage:\n");
        usage = iter_w->usage;
        next = iter_w->usage;
        do {
            usage = strsep(&next, "\n");
            seq_printf(m, "    %s\n", usage);
            if (next) {
                next[-1] = '\n';
            }
        } while (next);
        sx_dbg_dump_print_separate_line(m);
    }

    mutex_unlock(&priv->dbg_dump_proc.lock);
    return 0;
}

int sx_dbg_dump_proc_fs_init(void)
{
    __root_dir = proc_mkdir(SX_DBG_DUMP_PROC_DIR, NULL);
    if (!__root_dir) {
        sxd_log_err("failed to create proc debug dump root directory\n");
        return -ENOMEM;
    }

    return 0;
}

void sx_dbg_dump_proc_fs_deinit(void)
{
    remove_proc_entry(SX_DBG_DUMP_PROC_DIR, NULL);
    __root_dir = NULL;
}

int sx_dbg_dump_proc_fs_dev_init(struct sx_dev *dev)
{
    char            dev_string[DEVICE_ID_STRING_MAX_LEN] = {0};
    struct sx_priv *priv = NULL;

    WARN_ON(!dev);
    WARN_ON(!__root_dir);

    priv = sx_priv(dev);
    snprintf(dev_string, DEVICE_ID_STRING_MAX_LEN - 1, "dev_%u", dev->device_id);

    /* Create sub folder for device */
    priv->dbg_dump_proc.root_dir = proc_mkdir(dev_string, __root_dir);
    if (priv->dbg_dump_proc.root_dir == NULL) {
        sxd_log_warning("create proc dir for device %u - %s failed\n", dev->device_id, dev_string);
        return -ENOMEM;
    }

    mutex_init(&priv->dbg_dump_proc.lock);
    INIT_LIST_HEAD(&priv->dbg_dump_proc.read_entries);
    INIT_LIST_HEAD(&priv->dbg_dump_proc.write_entries);

    sx_dbg_dump_read_handler_register(DBG_DUMP_CMD_NAME "_help", __help_dump, NULL, dev, NULL);
    proc_create_data(DBG_DUMP_CMD_NAME,
                     S_IFREG | S_IWUSR,
                     priv->dbg_dump_proc.root_dir,
                     &__sx_command_proc_handler,
                     dev);
    return 0;
}

void sx_dbg_dump_proc_fs_dev_deinit(struct sx_dev *dev)
{
    struct sx_read_handler_info  *iter_r = NULL, *tmp_r = NULL;
    struct sx_write_handler_info *iter_w = NULL, *tmp_w = NULL;
    char                          dev_string[DEVICE_ID_STRING_MAX_LEN] = { 0 };
    struct sx_priv               *priv = NULL;

    WARN_ON(!dev);
    WARN_ON(!__root_dir);

    priv = sx_priv(dev);

    mutex_lock(&priv->dbg_dump_proc.lock);

    list_for_each_entry_safe(iter_r, tmp_r, &priv->dbg_dump_proc.read_entries, list) {
        __sx_dbg_dump_read_handler_unregister(iter_r, priv);
    }

    list_for_each_entry_safe(iter_w, tmp_w, &priv->dbg_dump_proc.write_entries, list) {
        __sx_dbg_dump_write_handler_unregister(iter_w);
    }

    mutex_unlock(&priv->dbg_dump_proc.lock);

    remove_proc_entry(DBG_DUMP_CMD_NAME, priv->dbg_dump_proc.root_dir);

    /* Remove dbg dump proc directory */
    snprintf(dev_string, DEVICE_ID_STRING_MAX_LEN - 1, "dev_%u", dev->device_id);
    remove_proc_entry(dev_string, __root_dir);
}

struct sx_dev * sx_dbg_dump_get_device(struct seq_file *m)
{
    struct sx_read_handler_info *info = (struct sx_read_handler_info*)m->private;

    return info->dev;
}
