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

#include <linux/fs.h>

#include <linux/mlx_sx/kernel_user.h>

#include "sx.h"
#include "ioctl_internal.h"
#include "mmap.h"

long ctrl_cmd_add_dev_path(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_dpt_path_add dpt_path_add_data;
    struct sx_dev         *dev;
    int                    err;

    err = copy_from_user(&dpt_path_add_data, (void*)data, sizeof(dpt_path_add_data));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &dpt_path_add_data.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    err = sx_dpt_add_dev_path(dpt_path_add_data.dev_id,
                              dpt_path_add_data.path_type,
                              dpt_path_add_data.path_info,
                              dpt_path_add_data.is_local);

out:
    return err;
}


long ctrl_cmd_remove_dev_path(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_dpt_path_add dpt_path_add_data;
    struct sx_dev         *dev;
    int                    err;

    err = copy_from_user(&dpt_path_add_data, (void*)data, sizeof(dpt_path_add_data));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &dpt_path_add_data.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    err = sx_dpt_remove_dev_path(dpt_path_add_data.dev_id, dpt_path_add_data.path_type);

out:
    return err;
}


long ctrl_cmd_remove_dev(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_dpt_path_add dpt_path_add_data;
    struct sx_dev         *dev;
    int                    err;

    err = copy_from_user(&dpt_path_add_data, (void*)data, sizeof(dpt_path_add_data));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &dpt_path_add_data.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    err = sx_dpt_remove_dev(dpt_path_add_data.dev_id);

out:
    return err;
}


long ctrl_cmd_set_cmd_path(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_dpt_path_modify dpt_path_modify_data;
    struct sx_dev            *dev;
    int                       err;

    err = copy_from_user(&dpt_path_modify_data, (void*)data, sizeof(dpt_path_modify_data));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &dpt_path_modify_data.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    err = sx_dpt_set_cmd_path(dpt_path_modify_data.dev_id, dpt_path_modify_data.path_type);

    /*
     * If we set DPT_PATH to be I2C - we still need to make sure we have the PCI's HCR mailbox addresses.
     * That is because now FW has two different mailboxes: one for PCI (HCR1) and one for I2C (HCR2).
     * Even if we use I2C, there are a few commands that must be sent on HCR1. Thus, we need to get the mailbox of this HCR.
     */
    if (dpt_path_modify_data.path_type == DPT_PATH_I2C) {
        sxd_log_notice(dev, "path type is I2C, going to get also PCI's mailbox\n");
        sx_QUERY_FW_2(dev, dpt_path_modify_data.dev_id, HCR1); /* true is to enforce getting HCR1 mailbox addresses */
    }

out:
    return err;
}


long ctrl_cmd_set_mad_path(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_dpt_path_modify dpt_path_modify_data;
    struct sx_dev            *dev;
    int                       err;

    err = copy_from_user(&dpt_path_modify_data, (void*)data, sizeof(dpt_path_modify_data));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &dpt_path_modify_data.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    err = sx_dpt_set_mad_path(dpt_path_modify_data.dev_id, dpt_path_modify_data.path_type);

out:
    return err;
}


long ctrl_cmd_set_emad_path(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_dpt_path_modify dpt_path_modify_data;
    struct sx_dev            *dev;
    int                       err;

    err = copy_from_user(&dpt_path_modify_data, (void*)data, sizeof(dpt_path_modify_data));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &dpt_path_modify_data.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    err = sx_dpt_set_emad_path(dpt_path_modify_data.dev_id, dpt_path_modify_data.path_type);

out:
    return err;
}


long ctrl_cmd_set_cr_access_path(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_dpt_path_modify dpt_path_modify_data;
    struct sx_dev            *dev;
    int                       err;

    err = copy_from_user(&dpt_path_modify_data, (void*)data, sizeof(dpt_path_modify_data));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &dpt_path_modify_data.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    err = sx_dpt_set_cr_access_path(dpt_path_modify_data.dev_id, dpt_path_modify_data.path_type);

out:
    return err;
}


long ctrl_cmd_cr_space_read(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_cr_space_read read_data;
    unsigned char          *buf = NULL;
    int                     err;

    err = copy_from_user(&read_data, (void*)data, sizeof(read_data));
    if (err) {
        goto out;
    }

    buf = kmalloc(read_data.size, GFP_KERNEL);
    if (!buf) {
        err = -ENOMEM;
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, NULL, &read_data.dev_id);

    err = sx_dpt_cr_space_read(read_data.dev_id, read_data.address, buf, read_data.size);
    if (err) {
        goto out_kfree;
    }

    err = copy_to_user(read_data.data, buf, read_data.size);

out_kfree:
    kfree(buf);

out:
    return err;
}


long ctrl_cmd_cr_space_write(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_cr_space_write write_data;
    unsigned char           *buf = NULL;
    int                      err;

    err = copy_from_user(&write_data, (void*)data, sizeof(write_data));
    if (err) {
        goto out;
    }

    buf = kmalloc(write_data.size, GFP_KERNEL);
    if (!buf) {
        err = -ENOMEM;
        goto out;
    }

    err = copy_from_user(buf, write_data.data, write_data.size);
    if (err) {
        goto out_kfree;
    }

    sx_core_get_dev_and_id_from_fd(file, NULL, &write_data.dev_id);

    err = sx_dpt_cr_space_write(write_data.dev_id, write_data.address, buf, write_data.size);

out_kfree:
    kfree(buf);

out:
    return err;
}

long ctrl_cmd_cr_dump(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_cr_dump read_data;
    unsigned char    *buf = NULL;
    int               err;
    struct sx_dev    *dev;

    err = copy_from_user(&read_data, (void*)data, sizeof(read_data));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &read_data.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    memset(&(read_data.ret), 0, sizeof(read_data.ret));

    if (read_data.opcode == SX_CR_DUMP_OP_GET_GDB_DUMP_LIMIT) {
        err = sx_core_cr_dump_get_cap_dump_host_size(read_data.opcode, dev, &(read_data.ret));
        if (err) {
            err = -EAGAIN;
            sxd_log_err(dev, "Cannot get valid param for the op_code (%d).\n", read_data.opcode);
            goto out;
        }
        err = copy_to_user(&(((struct ku_cr_dump *)data)->ret), &(read_data.ret), sizeof(read_data.ret));
        goto out;
    }

    buf = (unsigned char *)sx_mmap_user_to_kernel(dev, read_data.pid, (unsigned long)(read_data.data), false);
    if (!buf) {
        err = -ENOMEM;
        sxd_log_err(dev, "Cr_dump memory block is not valid - size: %u, usr buff: 0x%p, pid:%d.\n",
                    read_data.size,
                    read_data.data,
                    read_data.pid);
        goto out;
    }
    err = sx_core_cr_dump_handler(&read_data, buf);
    if (err) {
        goto out;
    }
    err = copy_to_user(&(((struct ku_cr_dump *)data)->ret), &(read_data.ret), sizeof(read_data.ret));

out:
    return err;
}

long ctrl_cmd_cr_dump_notify_dump_completion(struct file *file, unsigned int cmd, unsigned long data)
{
    struct ku_cr_dump_completion_state cr_dump_completion_state;
    struct sx_dev                     *dev;
    int                                err;

    err = copy_from_user(&cr_dump_completion_state, (void*)data, sizeof(cr_dump_completion_state));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, &cr_dump_completion_state.dev_id);
    if (!dev) {
        return -ENODEV;
    }

    sx_core_cr_dump_notify_dump_completion(cr_dump_completion_state.dev_id,
                                           cr_dump_completion_state.query,
                                           &cr_dump_completion_state.state);
    err = copy_to_user(&(((struct ku_cr_dump_completion_state *)data)->state),
                       &(cr_dump_completion_state.state),
                       sizeof(cr_dump_completion_state.state));

out:
    return err;
}

long ctrl_cmd_drop_ib_mads(struct file *file, unsigned int cmd, unsigned long data)
{
    struct sx_dev  *dev = NULL;
    struct sx_priv *priv = NULL;
    int             err = 0;
    bool            drop_mads;


    err = copy_from_user(&drop_mads, (void*)data, sizeof(bool));
    if (err) {
        goto out;
    }

    sx_core_get_dev_and_id_from_fd(file, &dev, NULL);
    if (!dev) {
        return -ENODEV;
    }

    priv = sx_priv(dev);
    priv->drop_ib_mads = drop_mads;

out:
    return err;
}
