/*
 * SPDX-FileCopyrightText: Copyright (c) 2010-2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/wait.h>
#include <net/sock.h>
#include <asm/pgtable.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/kthread.h>
#include <linux/mlx_sx/cmd.h>
#include <linux/mlx_sx/device.h>
#include <linux/mlx_sx/kernel_user.h>
#include <linux/mlx_sx/auto_registers/cmd_auto.h>
#include <linux/mlx_sx/auto_registers/reg.h>
#include <linux/netdevice.h>
#include <linux/vmalloc.h>
#include "sx_proc.h"
#include "cq.h"
#include "fw.h"
#include "sx.h"
#include "alloc.h"
#include "sx_dpt.h"
#include "dev_db.h"
#include "dev_init.h"
#include "sx_af_counters.h"
#include "health_check.h"

#define DEF_SX_CORE_PROC_MAX_SIZE 256

#define PROC_DUMP(dev, fmt, args ...)     sxd_log_err(dev, fmt, ## args)
#define PROC_DUMP_DBG(dev, fmt, args ...) sxd_log_debug(dev, fmt, ## args)

#define IGNORE_IF_NO_PCI(dev, cmd, ret_val)                                      \
    if (!(sx_glb.pci_drivers_in_use & PCI_DRIVER_F_SX_DRIVER)) {                 \
        PROC_DUMP(dev, "(%s): Driver has not detected any PCI device\n", (cmd)); \
        return (ret_val);                                                        \
    }

void __sx_proc_dump_fw_icm(struct sx_dev *dev);
void __sx_proc_dump_swids(struct sx_dev *dev);
void __sx_proc_dump_sdq(struct sx_dev *dev);
void __sx_proc_dump_rdq(struct sx_dev *dev);
void __sx_proc_dump_eq(struct sx_dev *dev);
void __sx_proc_dump_trap_groups_attr(struct sx_dev *dev);
void __sx_proc_dump_rdq_single(struct sx_dev *dev, int rdq);
void __sx_proc_rdq_start(struct sx_dev *dev, uint8_t rdq);
void __sx_proc_rdq_stop(struct sx_dev *dev, uint8_t rdq);
void __sx_proc_set_dev_profile(struct sx_dev *dev);
extern void sx_core_dump_synd_tbl(struct sx_dev *dev);

void __sx_proc_dump_emad(struct sx_dev *my_dev, u32 emad_type, u32 max_cnt, u32 direction);

void __sx_proc_show_cq(struct sx_dev *dev, int cq);
void __sx_proc_dump_cq(struct sx_dev *dev, int cqn);
void __sx_proc_flush_rdq(struct sx_dev *my_dev, int idx);
void __debug_dump_netdev(struct sx_dev *dev);
void __debug_netdev_traps(struct sx_dev *dev, u32 trap_id, u32 is_add);
void __set_net_dev_oper_state(char *dev_name, char *oper_state);
void __dump_net_dev_oper_state(void);
static ssize_t sx_proc_write(struct file *file, const char __user *buffer,
                             size_t count, loff_t *pos);
static void print_sx_proc_help(void);

struct sx_proc_dev_private_data {
    sxd_dev_id_t dev_id;
};

static struct proc_dir_entry          *__root_entry = NULL;
static struct proc_dir_entry          *__dir_proc_entry[SX_MAX_PCI_DEVICES] = { NULL };
static struct proc_dir_entry          *__proc_entry[SX_MAX_PCI_DEVICES] = { NULL };
static struct proc_dir_entry          *__driver_version_info_entry = NULL;
static struct sx_proc_dev_private_data __priv_data[SX_MAX_PCI_DEVICES];

/* Buffer to store driver version info appended content */
static char   __driver_version_info_buffer[DRIVER_VERSION_INFO_MAX_SIZE];
static size_t __driver_version_info_len = 0;
static DEFINE_SPINLOCK(__driver_version_info_lock);

extern int rx_debug;
extern int rx_debug_metadata;
extern int rx_debug_pkt_type;
extern int rx_debug_emad_type;
extern int rx_cqev2_dbg;
extern int rx_dump;
extern int rx_dump_cnt;
extern int tx_debug;
extern int tx_debug_metadata;
extern int tx_debug_pkt_type;
extern int tx_debug_emad_type;
extern int tx_dump;
extern int tx_dump_cnt;

const char *ku_pkt_type_str[] = {
    "SX_PKT_TYPE_ETH_CTL_UC", /**< Eth control unicast */
    "SX_PKT_TYPE_ETH_CTL_MC", /**< Eth control multicast */
    "SX_PKT_TYPE_ETH_DATA", /**< Eth data */
    "SX_PKT_TYPE_DROUTE_EMAD_CTL", /**< Directed route emad */
    "SX_PKT_TYPE_EMAD_CTL", /**< Emad */
    "SX_PKT_TYPE_FC_CTL_UC", /**< FC control unicast */
    "SX_PKT_TYPE_FC_CTL_MC", /**< FC control multicast */
    "SX_PKT_TYPE_FCOE_CTL_UC", /**< FC over Eth control unicast */
    "SX_PKT_TYPE_FCOE_CTL_MC", /**< FC over Eth control multicast */
    "SX_PKT_TYPE_IB_RAW_CTL", /**< IB raw control */
    "SX_PKT_TYPE_IB_TRANSPORT_CTL", /**< IB transport control */
    "SX_PKT_TYPE_IB_RAW_DATA", /**< IB raw data */
    "SX_PKT_TYPE_IB_TRANSPORT_DATA", /**< IB transport data */
    "SX_PKT_TYPE_EOIB_CTL", /**< Eth over IB control */
    "SX_PKT_TYPE_FCOIB_CTL",  /**< FC over IB control */
};

void __dump_kdbs(void);

static void __trim(char *buff)
{
    char *tmp = buff;

#define IS_CHAR_TO_TRIM(ch) ((ch) <= 0x20 || (ch) >= 0x7f)

    while (*tmp && IS_CHAR_TO_TRIM(*tmp)) {
        tmp++;
    }

    while (*tmp && !IS_CHAR_TO_TRIM(*tmp)) {
        *buff = *tmp;
        buff++;
        tmp++;
    }

    *buff = '\0';
}

static char * __sx_proc_str_get_ulong(char *buffer, unsigned long *val)
{
    const char delimiters[] = " .,;:!-";
    char      *running;
    char      *token;

    running = buffer;

    token = strsep(&running, delimiters);
    if (token == NULL) {
        *val = 0;
        return NULL;
    }

    if (!running) { /* no more tokens after this one */
        __trim(token);
    }

    if (strstr(token, "0x")) {
        *val = simple_strtoul(token, NULL, 16);
    } else {
        *val = simple_strtoul(token, NULL, 10);
    }

    return running;
}

char * sx_proc_str_get_u32(char *buffer, u32 *val32)
{
    const char delimiters[] = " .,;:!-";
    char      *running;
    char      *token;

    running = (char*)buffer;
    token = strsep(&running, delimiters);
    if (token == NULL) {
        *val32 = 0;
        return NULL;
    }

    if (!running) { /* no more tokens after this one */
        __trim(token);
    }

    if (strstr(token, "0x")) {
        *val32 = simple_strtol(token, NULL, 16);
    } else {
        *val32 = simple_strtol(token, NULL, 10);
    }

    return running;
}
EXPORT_SYMBOL(sx_proc_str_get_u32);

static char * __sx_proc_str_get_str(char *buffer, char **str)
{
    const char delimiters[] = " ,;:!-";
    char      *running;
    char      *token;

    running = buffer;
    token = strsep(&running, delimiters);
    if (token == NULL) {
        *str = 0;
        return NULL;
    }

    if (!running) { /* no more tokens after this one */
        __trim(token);
    }

    *str = token;
    return running;
}

static int sx_proc_show(struct seq_file *m, void *v)
{
    const struct sx_proc_dev_private_data *priv_data = PDE_DATA_DEF(file_inode(m->file));
    struct sx_dev                         *my_dev = NULL;
    struct sx_fw                          *fw = NULL;

    IGNORE_IF_NO_PCI(SXD_DEV_NONE, __FUNCTION__, 0);

    my_dev = sx_dev_db_get_dev_by_id(priv_data->dev_id);
    if (!my_dev) {
        PROC_DUMP(my_dev, "sx_proc_read: Invalid device received Null\n");
        return -ENODEV;
    }
    fw = &sx_priv(my_dev)->fw;

    seq_printf(m, "\nPROC Version: %08x\n", 0x2);

    seq_printf(m,
               "fw_ver: %d.%d.%04d\nfw_date: %02x:%02x %02x.%02x.%04x\n",
               (int)(fw->fw_ver >> 32),
               (int)((fw->fw_ver >> 16) & 0xffff),
               (int)(fw->fw_ver & 0xffff),
               fw->fw_hour,
               fw->fw_minutes,
               fw->fw_day,
               fw->fw_month,
               fw->fw_year);

    seq_printf(m, "out_mb: offset:0x%x, size:0x%x ; in_mb offset:0x%x, size:0x%x\n",
               fw->local_out_mb_offset, fw->local_out_mb_size,
               fw->local_in_mb_offset, fw->local_in_mb_size);

    return 0;
}

static int sx_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, sx_proc_show, NULL);
}

static int driver_version_info_show(struct seq_file *m, void *v)
{
    seq_printf(m, "=== Driver Version Information ===\n");
    seq_printf(m, "Version: %s\n", SX_SDK_VERSION);

#ifdef PD_BU
    seq_printf(m, "Compiled for Palladium: Yes\n");
#else
    seq_printf(m, "Compiled for Palladium: No\n");
#endif

    if (sizeof(long) == 8) {
        seq_printf(m, "Compiled as 64bit: Yes (sizeof(long)=%zu)\n", sizeof(long));
    } else {
        seq_printf(m, "Compiled as 64bit: No (sizeof(long)=%zu)\n", sizeof(long));
    }

    seq_printf(m, "CTRL_CMD_MAX_VAL: 0x%x (%d)\n", CTRL_CMD_MAX_VAL, CTRL_CMD_MAX_VAL);
    seq_printf(m, "CTRL_CMD_ACCESS_REG_MAX: 0x%x (%d)\n", CTRL_CMD_ACCESS_REG_MAX, CTRL_CMD_ACCESS_REG_MAX);

    seq_printf(m, "\n=== Compiler Information ===\n");
#ifdef __clang__
    seq_printf(m, "Compiler: Clang %d.%d.%d\n", __clang_major__, __clang_minor__, __clang_patchlevel__);
#ifdef __clang_version__
    seq_printf(m, "Clang version string: %s\n", __clang_version__);
#endif
#elif defined(__GNUC__)
    seq_printf(m, "Compiler: GCC %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#ifdef __VERSION__
    seq_printf(m, "GCC version string: %s\n", __VERSION__);
#endif
#else
    seq_printf(m, "Compiler: Unknown\n");
#endif

    seq_printf(m, "\nStructure sizes:\n");
    seq_printf(m, "  ku_dev_info:            %zu bytes\n",
               sizeof(struct ku_dev_info));
    seq_printf(m, "  ku_access_common:       %zu bytes\n",
               sizeof(struct ku_access_common));
    seq_printf(m, "  ku_write:               %zu bytes\n",
               sizeof(struct ku_write));

    /* Display appended content if any */
    {
        unsigned long flags;
        spin_lock_irqsave(&__driver_version_info_lock, flags);
        if (__driver_version_info_len > 0) {
            seq_printf(m, "\n=== Appended Information ===\n");
            seq_write(m, __driver_version_info_buffer, __driver_version_info_len);
        }
        spin_unlock_irqrestore(&__driver_version_info_lock, flags);
    }

    return 0;
}

static int driver_version_info_open(struct inode *inode, struct file *file)
{
    return single_open(file, driver_version_info_show, NULL);
}

static ssize_t driver_version_info_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    unsigned long flags;
    size_t        write_len;
    size_t        available_space;
    char         *temp_buffer = NULL;

    if (count == 0) {
        return 0;
    }

    /* Allocate temporary buffer for user data */
    temp_buffer = kmalloc(count, GFP_KERNEL);
    if (!temp_buffer) {
        sxd_log_err(SXD_DEV_NONE, "Failed to allocate temporary buffer for driver version info\n");
        return -ENOMEM;
    }

    /* Copy from user space BEFORE acquiring spinlock (copy_from_user can sleep) */
    if (copy_from_user(temp_buffer, buffer, count)) {
        kfree(temp_buffer);
        return -EFAULT;
    }

    /* Now acquire spinlock to update protected buffer */
    spin_lock_irqsave(&__driver_version_info_lock, flags);

    /* Calculate available space (leave room for null terminator) */
    available_space = DRIVER_VERSION_INFO_MAX_SIZE - __driver_version_info_len - 1;

    if (available_space == 0) {
        spin_unlock_irqrestore(&__driver_version_info_lock, flags);
        kfree(temp_buffer);
        sxd_log_warning(SXD_DEV_NONE, "Driver version info buffer full, cannot append\n");
        return -ENOSPC;
    }

    /* Limit write size to available space */
    write_len = min(count, available_space);

    /* Copy from temporary buffer to protected buffer (this is safe in atomic context) */
    memcpy(__driver_version_info_buffer + __driver_version_info_len, temp_buffer, write_len);

    /* Update length */
    __driver_version_info_len += write_len;

    /* Null terminate */
    __driver_version_info_buffer[__driver_version_info_len] = '\0';

    spin_unlock_irqrestore(&__driver_version_info_lock, flags);

    /* Free temporary buffer */
    kfree(temp_buffer);

    sxd_log_info(SXD_DEV_NONE, "Driver version info appended: %zu bytes written (total: %zu)\n",
                 write_len, __driver_version_info_len);

    return write_len;
}

static void sx_proc_handle_access_reg(struct sx_dev *dev, char *p, u32 dev_id, u32 reg_id, char *running)
{
    int err = 0;
    u32 local_port;
    u32 lp_msb;

    switch (reg_id) {
    case MGIR_REG_ID:
    {
        struct ku_access_mgir_reg reg_data;

        memset(&reg_data, 0, sizeof(reg_data));
        reg_data.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data.op_tlv, reg_id, 1);
        PROC_DUMP(dev, "Executing ACCESS_REG_MGIR Command\n");
        err = sx_ACCESS_REG_MGIR(dev, &reg_data);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "Finished Executing Query ACCESS_REG_MGIR Command, PSID = %d%d%d%d\n",
                  reg_data.mgir_reg.fw_info.psid[0],
                  reg_data.mgir_reg.fw_info.psid[1],
                  reg_data.mgir_reg.fw_info.psid[2],
                  reg_data.mgir_reg.fw_info.psid[3]);
        break;
    }

    case PSPA_REG_ID:
    {
        struct ku_access_pspa_reg reg_data;
        u32                       swid_id;
        memset(&reg_data, 0, sizeof(reg_data));
        reg_data.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data.op_tlv, reg_id, 2);
        /* local port + sub port are inputs */
        running = sx_proc_str_get_u32(running, &swid_id);
        reg_data.pspa_reg.swid = swid_id;
        running = sx_proc_str_get_u32(running, &local_port);
        reg_data.pspa_reg.local_port = local_port;
        running = sx_proc_str_get_u32(running, &lp_msb);
        reg_data.pspa_reg.lp_msb = lp_msb;
        PROC_DUMP(dev, "Executing ACCESS_REG_PSPA Command, "
                  "swid_id:%d, local port:%d, lp_msb:%d\n",
                  reg_data.pspa_reg.swid,
                  reg_data.pspa_reg.local_port,
                  reg_data.pspa_reg.lp_msb);
        err = sx_ACCESS_REG_PSPA(dev, &reg_data);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "Finished Executing Write ACCESS_REG_PSPA Command."
                  "Query not implemented yet\n");
        break;
    }

    case PTYS_REG_ID:
    {
        struct ku_access_ptys_reg reg_data, reg_data2;
        memset(&reg_data, 0, sizeof(reg_data));
        reg_data.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data.op_tlv, reg_id, 2);

        running = sx_proc_str_get_u32(running, &local_port);
        reg_data.ptys_reg.local_port = local_port;
        running = sx_proc_str_get_u32(running, &lp_msb);
        reg_data.ptys_reg.lp_msb = lp_msb;
        running = sx_proc_str_get_u32(running,
                                      &reg_data.ptys_reg.eth_proto_admin);
        running = sx_proc_str_get_u32(running,
                                      &reg_data.ptys_reg.eth_proto_oper);
        reg_data.ptys_reg.proto_mask = (1 << 2);
        PROC_DUMP(dev, "Executing ACCESS_REG_PTYS Command: local port:%d, lp_msb: %d"
                  "eth_proto_adm:0x%x, proto_oper:0x%x, "
                  "proto_mask:%d\n",
                  reg_data.ptys_reg.local_port,
                  reg_data.ptys_reg.lp_msb,
                  reg_data.ptys_reg.eth_proto_admin,
                  reg_data.ptys_reg.eth_proto_oper,
                  reg_data.ptys_reg.proto_mask);
        err = sx_ACCESS_REG_PTYS(dev, &reg_data);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "Finished Executing Write ACCESS_REG_PTYS Command. "
                  "Now executing Query:\n");
        memset(&reg_data, 0, sizeof(reg_data));
        memset(&reg_data2, 0, sizeof(reg_data2));
        reg_data2.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data2.op_tlv, reg_id, 1);
        reg_data2.ptys_reg.local_port = 1;
        reg_data2.ptys_reg.proto_mask = (1 << 2);
        err = sx_ACCESS_REG_PTYS(dev, &reg_data2);
        if (err) {
            goto print_err;
        }
        PROC_DUMP(dev, "ext_eth_proto_capability = %x\n",
                  reg_data2.ptys_reg.ext_eth_proto_capability);
        PROC_DUMP(dev, "eth_proto_capability = %x\n",
                  reg_data2.ptys_reg.eth_proto_capability);
        PROC_DUMP(dev, "ib_proto_capability = %x\n",
                  reg_data2.ptys_reg.ib_proto_capability);
        PROC_DUMP(dev, "ext_eth_proto_admin = %x\n",
                  reg_data2.ptys_reg.ext_eth_proto_admin);
        PROC_DUMP(dev, "eth_proto_admin = %x\n",
                  reg_data2.ptys_reg.eth_proto_admin);
        PROC_DUMP(dev, "ib_proto_admin = %x\n",
                  reg_data2.ptys_reg.ib_proto_admin);
        PROC_DUMP(dev, "ext_eth_proto_oper = %x\n",
                  reg_data2.ptys_reg.ext_eth_proto_oper);
        PROC_DUMP(dev, "eth_proto_oper = %x\n",
                  reg_data2.ptys_reg.eth_proto_oper);
        PROC_DUMP(dev, "ib_proto_oper = %x\n",
                  reg_data2.ptys_reg.ib_proto_oper);
        break;
    }

    case PMLP_REG_ID:
    {
        struct ku_access_pmlp_reg reg_data, reg_data2;
        int                       i;
        memset(&reg_data, 0, sizeof(reg_data));
        reg_data.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data.op_tlv, reg_id, 2);
        /* local port is the input */
        reg_data.pmlp_reg.local_port = 1;
        reg_data.pmlp_reg.width = 4;
        for (i = 0; i < 4; i++) {
            reg_data.pmlp_reg.lane_module_mapping[i].tx_lane = i;
            reg_data.pmlp_reg.lane_module_mapping[i].module = 3;
        }
        PROC_DUMP(dev, "Executing ACCESS_REG_PMLP Command\n");
        err = sx_ACCESS_REG_PMLP(dev, &reg_data);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "Finished Executing Write ACCESS_REG_PMLP Command. "
                  "Now executing Query:\n");
        memset(&reg_data2, 0, sizeof(reg_data2));
        reg_data2.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data2.op_tlv, reg_id, 1);
        reg_data2.pmlp_reg.local_port = 1;
        err = sx_ACCESS_REG_PMLP(dev, &reg_data2);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "width = %u\n", reg_data2.pmlp_reg.width);
        PROC_DUMP(dev, "lane0 = %u\n", reg_data2.pmlp_reg.lane_module_mapping[0].tx_lane);
        PROC_DUMP(dev, "module0 = %u\n", reg_data2.pmlp_reg.lane_module_mapping[0].module);
        PROC_DUMP(dev, "lane1 = %u\n", reg_data2.pmlp_reg.lane_module_mapping[1].tx_lane);
        PROC_DUMP(dev, "module1 = %u\n", reg_data2.pmlp_reg.lane_module_mapping[1].module);
        PROC_DUMP(dev, "lane2 = %u\n", reg_data2.pmlp_reg.lane_module_mapping[2].tx_lane);
        PROC_DUMP(dev, "module2 = %u\n", reg_data2.pmlp_reg.lane_module_mapping[2].module);
        PROC_DUMP(dev, "lane3 = %u\n", reg_data2.pmlp_reg.lane_module_mapping[3].tx_lane);
        PROC_DUMP(dev, "module3 = %u\n", reg_data2.pmlp_reg.lane_module_mapping[3].module);
        break;
    }

    case PLIB_REG_ID:
    {
        struct ku_access_plib_reg reg_data;
        memset(&reg_data, 0, sizeof(reg_data));
        reg_data.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data.op_tlv, reg_id, 2);

        running = sx_proc_str_get_u32(running, &local_port);
        reg_data.plib_reg.local_port = local_port;
        running = sx_proc_str_get_u32(running, &lp_msb);
        reg_data.plib_reg.lp_msb = lp_msb;
        running = sx_proc_str_get_u32(running, &local_port);
        reg_data.plib_reg.ib_port = local_port;
        PROC_DUMP(dev, "Executing ACCESS_REG_PLIB SET Command, local_port = %u, lp_msb = %u, ib_port = %u\n",
                  reg_data.plib_reg.local_port, reg_data.plib_reg.lp_msb, reg_data.plib_reg.ib_port);
        err = sx_ACCESS_REG_PLIB(dev, &reg_data);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "Finished Executing ACCESS_REG_PLIB SET Command:\n");
        sx_cmd_set_op_tlv(&reg_data.op_tlv, reg_id, 1);
        reg_data.plib_reg.ib_port = 0;
        PROC_DUMP(dev, "Executing ACCESS_REG_PLIB GET Command, local_port = %u, lp_msb = %u\n",
                  reg_data.plib_reg.local_port,
                  reg_data.plib_reg.lp_msb);
        err = sx_ACCESS_REG_PLIB(dev, &reg_data);
        PROC_DUMP(dev, "ib_port = %u\n", reg_data.plib_reg.ib_port);
        break;
    }

    case PMPR_REG_ID:
    {
        struct ku_access_pmpr_reg reg_data, reg_data2;
        memset(&reg_data, 0, sizeof(reg_data));
        reg_data.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data.op_tlv, reg_id, 2);
        reg_data.pmpr_reg.module = 1;
        reg_data.pmpr_reg.attenuation5g = 2;
        PROC_DUMP(dev, "Executing ACCESS_REG_PMPR Command\n");
        err = sx_ACCESS_REG_PMPR(dev, &reg_data);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "Finished Executing Write ACCESS_REG_PMPR Command. "
                  "Now executing query:\n");
        memset(&reg_data2, 0, sizeof(reg_data2));
        reg_data2.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data2.op_tlv, reg_id, 1);
        reg_data2.pmpr_reg.module = 1;
        err = sx_ACCESS_REG_PMPR(dev, &reg_data2);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "module = %u\n",
                  reg_data2.pmpr_reg.module);
        PROC_DUMP(dev, "Attenuation5G = %u\n",
                  reg_data2.pmpr_reg.attenuation5g);
        break;
    }

    case PMTU_REG_ID:
    {
        struct ku_access_pmtu_reg reg_data, reg_data2;
        memset(&reg_data, 0, sizeof(reg_data));
        reg_data.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data.op_tlv, reg_id, 2);
        SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data.pmtu_reg.local_port, reg_data.pmtu_reg.lp_msb, 1);
        reg_data.pmtu_reg.admin_mtu = 638;
        PROC_DUMP(dev, "Executing ACCESS_REG_PMTU Command\n");
        err = sx_ACCESS_REG_PMTU(dev, &reg_data);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "Finished Executing Write ACCESS_REG_PMTU Command. "
                  "Now executing Query:\n");
        memset(&reg_data2, 0, sizeof(reg_data2));
        reg_data2.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data2.op_tlv, reg_id, 1);
        SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data2.pmtu_reg.local_port, reg_data2.pmtu_reg.lp_msb, 1);
        err = sx_ACCESS_REG_PMTU(dev, &reg_data2);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "max_mtu = %u\n", reg_data2.pmtu_reg.max_mtu);
        PROC_DUMP(dev, "admin_mtu = %u\n", reg_data2.pmtu_reg.admin_mtu);
        PROC_DUMP(dev, "oper_mtu = %u\n", reg_data2.pmtu_reg.oper_mtu);
        break;
    }

    case PELC_REG_ID:
    {
        u32                       operation; /* 1 = QUERY 2 = WRITE*/
        u32                       op = 0;
        unsigned long             admin, request;
        struct ku_access_pelc_reg reg_data;
        memset(&reg_data, 0, sizeof(reg_data));
        reg_data.dev_id = dev_id;
        running = sx_proc_str_get_u32(running, &operation);
        sx_cmd_set_op_tlv(&reg_data.op_tlv, reg_id, operation);

        running = sx_proc_str_get_u32(running, &local_port);
        running = sx_proc_str_get_u32(running, &lp_msb);
        running = sx_proc_str_get_u32(running, &op);
        reg_data.pelc_reg.local_port = local_port;
        reg_data.pelc_reg.lp_msb = lp_msb;
        reg_data.pelc_reg.op = op;
        if (operation == 2) {
            running = __sx_proc_str_get_ulong(running, &admin);
            running = __sx_proc_str_get_ulong(running, &request);
            reg_data.pelc_reg.admin = 0x10ULL;
            reg_data.pelc_reg.request = 0x10ULL;
            PROC_DUMP(dev,
                      "Read admin = 0x%llx, request = 0x%llx\n",
                      reg_data.pelc_reg.admin,
                      reg_data.pelc_reg.request);
            PROC_DUMP(dev, "Executing ACCESS_REG_PELC Set Command\n");
            err = sx_ACCESS_REG_PELC(dev, &reg_data);
            if (err) {
                goto print_err;
            }
        } else {
            PROC_DUMP(dev, "Executing ACCESS_REG_PELC Get Command\n");
            err = sx_ACCESS_REG_PELC(dev, &reg_data);
            if (err) {
                goto print_err;
            }

            PROC_DUMP(dev, "Finished Executing ACCESS_REG_PELC Get Command:\n");
            PROC_DUMP(dev, "admin = %llx\n", reg_data.pelc_reg.admin);
            PROC_DUMP(dev, "capability = %llx\n", reg_data.pelc_reg.capability);
            PROC_DUMP(dev, "request = %llx\n", reg_data.pelc_reg.request);
            PROC_DUMP(dev, "active = %llx\n", reg_data.pelc_reg.active);
        }
        break;
    }

    case PVLC_REG_ID:
    {
        struct ku_access_pvlc_reg reg_data, reg_data2;
        memset(&reg_data, 0, sizeof(reg_data));
        reg_data.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data.op_tlv, reg_id, 2);
        /* local port is the input */
        reg_data.pvlc_reg.local_port = 1;
        reg_data.pvlc_reg.vl_admin = 3;
        PROC_DUMP(dev, "Executing ACCESS_REG_PVLC Command\n");
        err = sx_ACCESS_REG_PVLC(dev, &reg_data);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "Finished Executing Write ACCESS_REG_PVLC Command. "
                  "Now executing Query:\n");
        memset(&reg_data2, 0, sizeof(reg_data2));
        reg_data2.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data2.op_tlv, reg_id, 1);
        reg_data2.pvlc_reg.local_port = 1;
        err = sx_ACCESS_REG_PVLC(dev, &reg_data2);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "vl_cap = %u\n", reg_data2.pvlc_reg.vl_cap);
        PROC_DUMP(dev, "vl_admin = %u\n", reg_data2.pvlc_reg.vl_admin);
        PROC_DUMP(dev, "vl_oper = %u\n", reg_data2.pvlc_reg.vl_operational);
        break;
    }

    case MLXSW_HPKT_ID:
    {
        struct ku_access_hpkt_reg reg_data, reg_data2;
        memset(&reg_data, 0, sizeof(reg_data));
        reg_data.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data.op_tlv, reg_id, 2);
        /* trap_id is the input */
        reg_data.hpkt_reg.action = 1;
        reg_data.hpkt_reg.trap_group = 2;
        reg_data.hpkt_reg.trap_id = 5;
        PROC_DUMP(dev, "Executing ACCESS_REG_HPKT Command\n");
        err = sx_ACCESS_REG_HPKT(dev, &reg_data);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "Finished Executing Write ACCESS_REG_HPKT Command. "
                  "Now executing Query:\n");
        memset(&reg_data2, 0, sizeof(reg_data2));
        reg_data2.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data2.op_tlv, reg_id, 1);
        reg_data2.hpkt_reg.trap_id = 0x10;
        err = sx_ACCESS_REG_HPKT(dev, &reg_data2);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "action = %u\n", reg_data2.hpkt_reg.action);
        PROC_DUMP(dev, "trap_group = %u\n", reg_data2.hpkt_reg.trap_group);
        break;
    }

    case MFSC_REG_ID:
    {
        struct ku_access_mfsc_reg reg_data;
        u32                       fan;
        memset(&reg_data, 0, sizeof(reg_data));
        reg_data.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data.op_tlv, reg_id, 1);
        /* fan is the input */
        running = sx_proc_str_get_u32(running, &fan);
        reg_data.mfsc_reg.pwm = (u8)(fan & 0x7);
        PROC_DUMP(dev, "Executing ACCESS_REG_MFSC Command , dev_id: %d \n", reg_data.dev_id);
        err = sx_ACCESS_REG_MFSC(dev, &reg_data);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "Finished Executing ACCESS_REG_MFSC Command:\n");
        PROC_DUMP(dev, "pwm_duty_cycle = %u\n",
                  reg_data.mfsc_reg.pwm_duty_cycle);
        break;
    }

    case MFSM_REG_ID:
    {
        struct ku_access_mfsm_reg reg_data;
        u32                       tacho;
        memset(&reg_data, 0, sizeof(reg_data));
        reg_data.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data.op_tlv, reg_id, 1);
        /* tacho is the input */
        running = sx_proc_str_get_u32(running, &tacho);
        reg_data.mfsm_reg.tacho = (u8)(tacho & 0x7);
        PROC_DUMP(dev, "Executing ACCESS_REG_MFSM Command, dev_id: %d\n", dev_id);
        err = sx_ACCESS_REG_MFSM(dev, &reg_data);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "Finished Executing ACCESS_REG_MFSM Command:\n");
        PROC_DUMP(dev, "n = %u\n", reg_data.mfsm_reg.n);
        PROC_DUMP(dev, "rpm = %u\n", reg_data.mfsm_reg.rpm);
        break;
    }

    case MFSL_REG_ID:
    {
        struct ku_access_mfsl_reg reg_data;
        u32                       tacho;
        memset(&reg_data, 0, sizeof(reg_data));
        reg_data.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data.op_tlv, reg_id, 1);
        /* tacho is the input */
        running = sx_proc_str_get_u32(running, &tacho);
        reg_data.mfsl_reg.fan = (u8)(tacho & 0x7);
        PROC_DUMP(dev, "Executing ACCESS_REG_MFSL Command\n");
        err = sx_ACCESS_REG_MFSL(dev, &reg_data);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "Finished Executing ACCESS_REG_MFSL Command:\n");
        PROC_DUMP(dev, "ee = %u\n", reg_data.mfsl_reg.ee);
        PROC_DUMP(dev, "ie = %u\n", reg_data.mfsl_reg.ie);
        PROC_DUMP(dev, "tach_min = %u\n", reg_data.mfsl_reg.tach_min);
        PROC_DUMP(dev, "tach_max = %u\n", reg_data.mfsl_reg.tach_max);
        break;
    }

    case SPZR_REG_ID:
    {
        struct ku_access_spzr_reg reg_data;
        memset(&reg_data, 0, sizeof(reg_data));
        reg_data.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data.op_tlv, reg_id, 1);
        reg_data.spzr_reg.enh_sw_p0 = 1;
        reg_data.spzr_reg.enh_sw_p0_mask = 1;
        PROC_DUMP(dev, "Executing ACCESS_REG_SPZR Command\n");
        err = sx_ACCESS_REG_SPZR(dev, &reg_data);
        break;
    }

    case MLXSW_HTGT_ID:
    {
        struct ku_access_htgt_reg reg_data, reg_data2;
        memset(&reg_data, 0, sizeof(reg_data));
        reg_data.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data.op_tlv, reg_id, 2);
        PROC_DUMP(dev, "Executing ACCESS_REG_HTGT Command\n");
        reg_data.htgt_reg.swid = 254;
        reg_data.htgt_reg.type = 0;
        reg_data.htgt_reg.trap_group = 2;
        reg_data.htgt_reg.path.rdq = 21;
        reg_data.htgt_reg.path.cpu_tclass = 1;
        err = sx_ACCESS_REG_HTGT(dev, &reg_data);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "Finished Executing Write ACCESS_REG_HTGT Command. "
                  "Now executing Query:\n");
        memset(&reg_data2, 0, sizeof(reg_data2));
        reg_data2.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data2.op_tlv, reg_id, 1);
        reg_data2.htgt_reg.swid = 0;
        reg_data2.htgt_reg.trap_group = 2;
        err = sx_ACCESS_REG_HTGT(dev, &reg_data2);
        if (err) {
            goto print_err;
        }

        PROC_DUMP(dev, "swid = %u\n", reg_data2.htgt_reg.swid);
        PROC_DUMP(dev, "type = %u\n", reg_data2.htgt_reg.type);
        PROC_DUMP(dev, "group = %u\n", reg_data2.htgt_reg.trap_group);
        PROC_DUMP(dev, "rdq = %u\n",
                  reg_data2.htgt_reg.path.rdq);
        PROC_DUMP(dev, "cpu_tclass = %u\n",
                  reg_data2.htgt_reg.path.cpu_tclass);
        break;
    }

    case MCIA_REG_ID:
        sxd_log_warning(dev, "%s() reg_id %u not implemented yet\n",
                        __func__, reg_id);
        break;

    case MPSC_REG_ID:
    {
        struct ku_access_mpsc_reg reg_data, reg_data2;
        u32                       rate;
        u32                       enable;
        memset(&reg_data, 0, sizeof(reg_data));
        reg_data.dev_id = dev_id;

        running = sx_proc_str_get_u32(running, &local_port);
        reg_data.mpsc_reg.local_port = local_port;
        running = sx_proc_str_get_u32(running, &lp_msb);
        reg_data.mpsc_reg.lp_msb = lp_msb;
        running = sx_proc_str_get_u32(running, &rate);
        reg_data.mpsc_reg.rate = rate;
        running = sx_proc_str_get_u32(running, &enable);
        reg_data.mpsc_reg.enable = (u8)enable;

        sx_cmd_set_op_tlv(&reg_data.op_tlv, reg_id, 2);
        PROC_DUMP(dev, "Executing ACCESS_REG_MPSC Write Command\n");

        err = sx_ACCESS_REG_MPSC(dev, &reg_data);
        PROC_DUMP(dev, );

        PROC_DUMP(dev, "Finished Executing Write ACCESS_REG_MPSC Command. "
                  "Now executing Query:\n");
        memset(&reg_data2, 0, sizeof(reg_data2));
        reg_data2.dev_id = dev_id;
        sx_cmd_set_op_tlv(&reg_data2.op_tlv, reg_id, 1);

        err = sx_ACCESS_REG_MPSC(dev, &reg_data2);
        if (err) {
            goto print_err;
        }
        PROC_DUMP(dev,
                  "Read rate = 0x%08u, count_drops = %lld\n",
                  reg_data2.mpsc_reg.rate,
                  reg_data2.mpsc_reg.count_drops);
        break;
    }

    default:
        err = sx_proc_handle_access_reg_auto(dev, p, dev_id, reg_id, running);
        if (err == EINVAL) {
            sxd_log_warning(dev, "%s() reg_id %u doesn't exist\n",
                            __func__, reg_id);
        } else if (err) {
            goto print_err;
        }
    }

    return;

print_err:
    sxd_log_warning(dev, "%s() execution of ACCESS_REG on reg_id %x Failed."
                    "error code = %d \n", __func__, reg_id, err);
}

static void __query_fw(struct sx_dev *my_dev, char *running)
{
    struct sx_dev     *dev;
    struct ku_query_fw fw;
    u32                dev_id;

    running = sx_proc_str_get_u32(running, &dev_id);
    if (dev_id == 0) {
        dev_id = my_dev->device_id;
    }

    dev = sx_dev_db_get_dev_by_id(dev_id);
    if (dev) {
        my_dev = dev;
    }
    fw.dev_id = dev_id;

    PROC_DUMP(dev, "Executing QUERY_FW Command to dev %d !\n", dev_id);
    sx_QUERY_FW(my_dev, &fw);
    PROC_DUMP(dev, "fw_rev = 0x%llx\n", fw.fw_rev);
    PROC_DUMP(dev, "debug_trace = 0x%x\n", fw.dt);
    PROC_DUMP(dev, "core_clock = 0x%x\n", fw.core_clk);
    PROC_DUMP(dev, "fw_hour = 0x%x\n", fw.fw_hour);
    PROC_DUMP(dev, "fw_minutes = 0x%x\n", fw.fw_minutes);
    PROC_DUMP(dev, "fw_seconds = 0x%x\n", fw.fw_seconds);
    PROC_DUMP(dev, "fw_year = 0x%x\n", fw.fw_year);
    PROC_DUMP(dev, "fw_month = 0x%x\n", fw.fw_month);
    PROC_DUMP(dev, "fw_day = 0x%x\n", fw.fw_day);
    PROC_DUMP(dev, "fw revision = %d.%d.%d\n", (int)(fw.fw_rev >> 32),
              (int)((fw.fw_rev >> 16) & 0xffff),
              (int)(fw.fw_rev & 0xffff));
    PROC_DUMP(dev, "fw release date = %x:%x %x.%x.%x\n", fw.fw_hour,
              fw.fw_minutes, fw.fw_day, fw.fw_month, fw.fw_year);

    PROC_DUMP(dev, "Executing QUERY_FW_2 Command\n");
    sx_QUERY_FW_2(my_dev, dev_id, HCR1);
    PROC_DUMP(dev, "Finished Executing QUERY_FW_2 Command:\n");
    PROC_DUMP(dev, "local_out_mb_offset = 0x%x\n",
              sx_glb.sx_dpt.dpt_info[dev_id].out_mb_offset[HCR1]);
    PROC_DUMP(dev, "local_out_mb_size = 0x%x\n",
              sx_glb.sx_dpt.dpt_info[dev_id].out_mb_size[HCR1]);
    PROC_DUMP(dev, "local_in_mb_offset = 0x%x\n",
              sx_glb.sx_dpt.dpt_info[dev_id].in_mb_offset[HCR1]);
    PROC_DUMP(dev, "local_in_mb_size = 0x%x\n",
              sx_glb.sx_dpt.dpt_info[dev_id].in_mb_size[HCR1]);
}

static void __query_cq(struct sx_dev *my_dev, char *running)
{
    struct ku_query_cq cq_context;
    struct sx_dev     *dev;
    u32                cqn, dev_id;

    running = sx_proc_str_get_u32(running, &cqn);

    dev_id = my_dev->device_id;

    dev = sx_dev_db_get_dev_by_id(dev_id);
    if (dev) {
        my_dev = dev;
    }

    PROC_DUMP(dev, "Executing QUERY_CQ Command to dev %d !\n", dev_id);
    sx_QUERY_CQ(my_dev, cqn, &cq_context);
    PROC_DUMP(dev, "cqn:%d, eqn:%d, log_cq_size:%u, producer_counter:%u \n",
              cqn, cq_context.eq_num, cq_context.log_cq_size, cq_context.producer_counter);
}

static void __query_board_info(struct sx_dev *my_dev, char *running)
{
    struct ku_query_board_info board;

    memset(&board, 0, sizeof(board));
    PROC_DUMP(my_dev, "Executing QUERY_BOARDINFO Command\n");
    sx_QUERY_BOARDINFO(my_dev, &board);
    PROC_DUMP(my_dev, "Finished Executing QUERY_BOARDINFO Command:\n");

    PROC_DUMP(my_dev, "vsd_vendor_id = 0x%x\n", board.vsd_vendor_id);
    PROC_DUMP(my_dev, "board_id = %s\n", board.board_id);
    PROC_DUMP(my_dev, "inta_pin = 0x%x\n", board.inta_pin);
}

static void __mkey_set(struct sx_dev *my_dev, char *running)
{
    struct ku_system_m_key system_m_key;
    u32                    dev_id = 0;
    u32                    m_key_h = 0, m_key_l = 0;
    int                    err = 0;

    running = sx_proc_str_get_u32(running, &dev_id);
    running = sx_proc_str_get_u32(running, &m_key_h);
    running = sx_proc_str_get_u32(running, &m_key_l);

    system_m_key.dev_id = dev_id;
    system_m_key.system_m_key = ((u64)m_key_h << 32) | m_key_l;
    PROC_DUMP(my_dev, "Executing set system_m_key: 0x%x%x to dev %d\n",
              m_key_h, m_key_l, dev_id);
    err = sx_SET_SYSTEM_M_KEY(my_dev, &system_m_key);
    PROC_DUMP(my_dev, "Finished Executing set_m_key. err = %d\n", err);
}

static void __mkey_get(struct sx_dev *my_dev, char *running)
{
    struct ku_system_m_key system_m_key;
    u32                    dev_id = 0;
    int                    err = 0;

    running = sx_proc_str_get_u32(running, &dev_id);

    memset(&system_m_key, 0, sizeof(system_m_key));
    system_m_key.dev_id = dev_id;
    PROC_DUMP(my_dev, "Executing get system_m_key from dev %d\n",
              dev_id);
    err = sx_GET_SYSTEM_M_KEY(my_dev, &system_m_key);
    PROC_DUMP(my_dev, "Finished Executing get_m_key. err = %d, system_m_key:0x%llx\n",
              err, system_m_key.system_m_key);
}

static ssize_t sx_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    int                              cmd = 0;
    int                              err = 0;
    char                            *p;
    char                            *running;
    char                            *cmd_str;
    size_t                           buf_len = 0;
    struct sx_proc_dev_private_data *priv_data = (struct sx_proc_dev_private_data*)PDE_DATA_DEF(file_inode(file));
    struct sx_dev                   *my_dev = sx_dev_db_get_dev_by_id(priv_data->dev_id);
    int                              dev_id;
    char                             proc_buf[DEF_SX_CORE_PROC_MAX_SIZE] = { 0 };

    buf_len = min(count, sizeof(proc_buf) - 1);
    if (copy_from_user(proc_buf, buffer, buf_len) != 0) {
        PROC_DUMP(my_dev, "copy_from_user() failed\n");
        return -EFAULT;
    }
    proc_buf[buf_len] = '\0';
    running = (char*)proc_buf;

    running = __sx_proc_str_get_str(running, &cmd_str);

    p = strstr(cmd_str, "dump_dpt");
    if (p != NULL) {
        sx_dpt_dump();
        cmd++;
    }

    p = strstr(cmd_str, "dpt_setp");
    if (p != NULL) {
        int path_type;
        running = sx_proc_str_get_u32(running, &dev_id);
        running = sx_proc_str_get_u32(running, &path_type);
        PROC_DUMP(my_dev, "Change dpt dev_id:%d , path_type: %d  \n",
                  dev_id, path_type);
        sx_dpt_set_cmd_dbg(dev_id, path_type);
        cmd++;
    }

    p = strstr(cmd_str, "dpt_set_cr");
    if (p != NULL) {
        int path_type;
        running = sx_proc_str_get_u32(running, &dev_id);
        running = sx_proc_str_get_u32(running, &path_type);
        PROC_DUMP(my_dev, "Change dpt dev_id:%d , CR path_type: %d  \n",
                  dev_id, path_type);
        sx_dpt_set_cr_access_path(dev_id, path_type);
        cmd++;
    }

    p = strstr(cmd_str, "cr_space_write");
    if (p != NULL) {
        unsigned int address;
        unsigned int value;
        running = sx_proc_str_get_u32(running, &dev_id);
        running = sx_proc_str_get_u32(running, &address);
        running = sx_proc_str_get_u32(running, &value);
        PROC_DUMP(my_dev, "write dword to cr_space dev_id:%d , address: 0x%x , value: %d \n",
                  dev_id, address, value);
        err = sx_dpt_cr_space_write(dev_id, address, (unsigned char*)&value, 4);
        if (err) {
            PROC_DUMP(my_dev,
                      "cr_space write: dev_id:%d, address: 0x%x, write failed, error: %d \n",
                      dev_id,
                      address,
                      err);
        }
        cmd++;
    }

    p = strstr(cmd_str, "cr_space_read");
    if (p != NULL) {
        unsigned int address;
        unsigned int value = 0;
        running = sx_proc_str_get_u32(running, &dev_id);
        running = sx_proc_str_get_u32(running, &address);
        PROC_DUMP(my_dev, "read dword from cr_space dev_id:%d , address: 0x%x\n",
                  dev_id, address);
        err = sx_dpt_cr_space_read(dev_id, address, (unsigned char*)&value, 4);
        if (err) {
            PROC_DUMP(my_dev,
                      "cr_space read: dev_id:%d, address: 0x%x, read failed, error: %d \n",
                      dev_id,
                      address,
                      err);
        } else {
            PROC_DUMP(my_dev, "cr_space read result %x", value);
        }
        cmd++;
    }

    p = strstr(cmd_str, "q_fw");
    if (p != NULL) {
        IGNORE_IF_NO_PCI(SXD_DEV_NONE, "q_fw", count);

        __query_fw(my_dev, running);
        cmd++;
    }

    p = strstr(cmd_str, "q_cq");
    if (p != NULL) {
        IGNORE_IF_NO_PCI(SXD_DEV_NONE, "q_cq", count);

        __query_cq(my_dev, running);
        cmd++;
    }

    p = strstr(cmd_str, "q_board_info");
    if (p != NULL) {
        IGNORE_IF_NO_PCI(SXD_DEV_NONE, "q_board_info", count);

        __query_board_info(my_dev, running);
        cmd++;
    }

    p = strstr(cmd_str, "q_aq_cap");
    if (p != NULL) {
        struct sx_dev_cap *dev_cap = &sx_priv(my_dev)->dev_cap;

        PROC_DUMP(my_dev, "Executing QUERY_AQ_CAP Command\n");
        sx_QUERY_AQ_CAP(my_dev);
        PROC_DUMP(my_dev, "Finished Executing QUERY_AQ_CAP Command:\n");
        PROC_DUMP(my_dev, "log_max_rdq_sz = %d\n", dev_cap->log_max_rdq_sz);
        PROC_DUMP(my_dev, "log_max_sdq_sz = %d\n", dev_cap->log_max_sdq_sz);
        PROC_DUMP(my_dev, "log_max_cq_sz = %d\n", dev_cap->log_max_cq_sz);
        PROC_DUMP(my_dev, "log_max_eq_sz = %d\n", dev_cap->log_max_eq_sz);
        PROC_DUMP(my_dev, "max_num_rdqs = %u\n", dev_cap->max_num_rdqs);
        PROC_DUMP(my_dev, "max_num_sdqs = %u\n", dev_cap->max_num_sdqs);
        PROC_DUMP(my_dev, "max_num_cqs = %u\n", dev_cap->max_num_cqs);
        PROC_DUMP(my_dev, "max_num_eqs = %u\n", dev_cap->max_num_eqs);
        PROC_DUMP(my_dev, "max_sg_sq = %u\n", dev_cap->max_sg_sq);
        PROC_DUMP(my_dev, "max_sg_rq = %u\n", dev_cap->max_sg_rq);
        cmd++;
    }

    p = strstr(cmd_str, "get_profile");
    if (p != NULL) {
        struct ku_profile            *profile;
        struct profile_driver_params *params;

        profile = vmalloc(sizeof(*profile));
        if (profile == NULL) {
            sxd_log_err(my_dev, "Error allocate mem for profile\n");
            goto out;
        }
        params = vmalloc(sizeof(*params));
        if (params == NULL) {
            sxd_log_err(my_dev, "Error allocate mem for params\n");
            vfree(profile);
            goto out;
        }
        memset(profile, 0, sizeof(*profile));
        memset(params, 0, sizeof(*params));
        PROC_DUMP(my_dev, "Executing GET_PROFILE Command\n");
        sx_GET_PROFILE(my_dev, profile, params);

        PROC_DUMP(my_dev, "Finished Executing GET_PROFILE Command:\n");
        PROC_DUMP(my_dev, "max_vepa_channels = %u\n", profile->max_vepa_channels);
        PROC_DUMP(my_dev, "max_lag = %u\n", profile->max_lag);
        PROC_DUMP(my_dev, "max_port_per_lag = %u\n", profile->max_port_per_lag);
        PROC_DUMP(my_dev, "max_mid = %u\n", profile->max_mid);
        PROC_DUMP(my_dev, "max_pgt = %u\n", profile->max_pgt);
        PROC_DUMP(my_dev, "max_system_port = %u\n", profile->max_system_port);
        PROC_DUMP(my_dev, "max_active_vlans = %u\n", profile->max_active_vlans);
        PROC_DUMP(my_dev, "max_regions = %u\n", profile->max_regions);
        PROC_DUMP(my_dev, "max_flood_tables = %u\n", profile->max_flood_tables);
        PROC_DUMP(my_dev, "max_per_vid_flood_tables = %u\n", profile->max_per_vid_flood_tables);
        PROC_DUMP(my_dev, "flood mode = %u\n", profile->flood_mode);
        PROC_DUMP(my_dev,
                  "max_fid_offset_flood_tables = %u\n",
                  profile->max_fid_offset_flood_tables);
        PROC_DUMP(my_dev, "fid_offset_table_size = %u\n", profile->fid_offset_table_size);
        PROC_DUMP(my_dev, "max_per_fid_flood_table = %u\n", profile->max_per_fid_flood_table);
        PROC_DUMP(my_dev, "per_fid_table_size = %u\n", profile->per_fid_table_size);
        PROC_DUMP(my_dev, "max_fid = %u\n", profile->max_fid);
        PROC_DUMP(my_dev, "max_ib_mc = %u\n", profile->max_ib_mc);
        PROC_DUMP(my_dev, "max_pkey = %u\n", profile->max_pkey);
        PROC_DUMP(my_dev, "ar_sec = %u\n", profile->ar_sec);
        PROC_DUMP(my_dev,
                  "adaptive_routing_group_cap = %u\n",
                  profile->adaptive_routing_group_cap);
        PROC_DUMP(my_dev, "adaptive-routing-notification = %u\n", profile->arn);
        PROC_DUMP(my_dev, "kvd_linear_size = %u\n", profile->kvd_linear_size);
        PROC_DUMP(my_dev, "kvd_hash_single_size = %u\n", profile->kvd_hash_single_size);
        PROC_DUMP(my_dev, "kvd_hash_double_size = %u\n", profile->kvd_hash_double_size);
        PROC_DUMP(my_dev, "switch_0_type = %u\n", profile->swid0_config_type.type);
        PROC_DUMP(my_dev, "switch_1_type = %u\n", profile->swid1_config_type.type);
        PROC_DUMP(my_dev, "switch_2_type = %u\n", profile->swid2_config_type.type);
        PROC_DUMP(my_dev, "switch_3_type = %u\n", profile->swid3_config_type.type);
        PROC_DUMP(my_dev, "switch_4_type = %u\n", profile->swid4_config_type.type);
        PROC_DUMP(my_dev, "switch_5_type = %u\n", profile->swid5_config_type.type);
        PROC_DUMP(my_dev, "switch_6_type = %u\n", profile->swid6_config_type.type);
        PROC_DUMP(my_dev, "switch_7_type = %u\n", profile->swid7_config_type.type);
        PROC_DUMP(my_dev, "ib_router_en = %u\n", profile->ib_router_en);
        PROC_DUMP(my_dev, "ib_router_ecmp = %u\n", profile->ib_router_ecmp);
        PROC_DUMP(my_dev, "ib_router_mcf = %u\n", profile->ib_router_mcf);
        PROC_DUMP(my_dev, "ib_router_ecmp_lid_range = %u\n", profile->ib_router_ecmp_lid_range);
        PROC_DUMP(my_dev, "split_ready = %u\n", profile->split_ready);
        PROC_DUMP(my_dev, "cqe_version = %u\n", params->cqe_version);
        PROC_DUMP(my_dev, "umlabel = %u\n", profile->umlabel);
        PROC_DUMP(my_dev, "cqe_time_stamp_type = %u\n", params->cqe_time_stamp_type);

        vfree(profile);
        vfree(params);
        cmd++;
    }

    p = strstr(cmd_str, "access_reg");
    if (p != NULL) {
        u32 reg_id = 0;
        sx_cmd_use_polling(my_dev);
        running = sx_proc_str_get_u32(running, &dev_id);
        running = sx_proc_str_get_u32(running, &reg_id);
        sx_proc_handle_access_reg(my_dev, p, dev_id, reg_id, running);
        cmd++;
    }

    p = strstr(cmd_str, "mem_wr");
    if (p != NULL) {
        unsigned long width;
        unsigned long addr, val;
        running = __sx_proc_str_get_ulong(running, &width);
        running = __sx_proc_str_get_ulong(running, &addr);
        running = __sx_proc_str_get_ulong(running, &val);
        sxd_log_info(my_dev, "Write value %lx to addr: 0x%lx  \n", val, addr);
        sx_proc_dbg_wr(width, addr, val);
        cmd++;
    }

    p = strstr(cmd_str, "mem_rd ");
    if (p != NULL) {
        unsigned long width;
        unsigned long val, addr;
        running = __sx_proc_str_get_ulong(running, &width);
        running = __sx_proc_str_get_ulong(running, &addr);
        sx_proc_dbg_rd(width, addr, &val);
        sxd_log_info(my_dev, "Read value addr: 0x%lx  val: 0x%lx \n",
                     addr, val);
        cmd++;
    }

    p = strstr(cmd_str, "sw_reset");
    if (p != NULL) {
        IGNORE_IF_NO_PCI(my_dev, "sw_reset", count);

        if (!my_dev->pdev) {
            PROC_DUMP(my_dev, "Executing sw_reset is only possible "
                      "when PCI is available\n");
            goto out;
        }

        PROC_DUMP_DBG(my_dev, "Executing sw_reset\n");
        err = sx_reset(my_dev, 1);
        if (err) {
            PROC_DUMP_DBG(my_dev, "Failed in Executing sw_reset. err = %d\n", err);
        } else {
            PROC_DUMP_DBG(my_dev, "Finished Executing sw_reset. err = %d\n", err);
        }

        cmd++;
    }
    p = strstr(cmd_str, "sys_status");
    if (p != NULL) {
        u16 sys_status = 0;

        IGNORE_IF_NO_PCI(my_dev, "sys_status", count);

        if (!my_dev->pdev) {
            PROC_DUMP(my_dev, "Executing sys_status is only possible "
                      "when PCI is available\n");
            goto out;
        }

        PROC_DUMP_DBG(my_dev, "Executing sys_status\n");
        err = get_system_status(my_dev, &sys_status);
        if (err) {
            PROC_DUMP_DBG(my_dev, "Failed in Executing get_system_status. err = %d\n", err);
        } else {
            PROC_DUMP_DBG(my_dev, "Finished Executing get_system_status. err = %d\n", err);
            PROC_DUMP_DBG(my_dev, "System Status: 0x%x\n", sys_status);
        }
        cmd++;
    }
    p = strstr(cmd_str, "enable_swid");
    if (p != NULL) {
        struct sx_priv *priv = sx_priv(my_dev);
        u32             swid;

        IGNORE_IF_NO_PCI(my_dev, "enable_swid", count);

        running = sx_proc_str_get_u32(running, &swid);
        spin_lock(&priv->profile.profile_lock);
        if (!priv->profile.pci_profile_set || (swid >= NUMBER_OF_SWIDS)
            || (priv->profile.pci_profile.swid_type[swid] ==
                SX_KU_L2_TYPE_DONT_CARE) ||
            sx_bitmap_test(&priv->swid_bitmap,
                           swid)) {
            PROC_DUMP(my_dev, "Err in param, not executing enable_swid\n");
            spin_unlock(&priv->profile.profile_lock);
            goto out;
        }

        spin_unlock(&priv->profile.profile_lock);
        PROC_DUMP_DBG(my_dev, "Executing enable_swid. dev_id = %u, "
                      "swid num = %u\n",
                      my_dev->device_id, swid);
        err = sx_enable_swid(my_dev, my_dev->device_id, swid, 0x1c0 + swid, 0);
        if (err) {
            PROC_DUMP_DBG(my_dev, "Failed in Executing enable_swid. err = %d\n", err);
        } else {
            PROC_DUMP_DBG(my_dev, "Finished Executing enable_swid. err = %d\n", err);
        }
        cmd++;
    }

    p = strstr(cmd_str, "disable_swid");
    if (p != NULL) {
        struct sx_priv *priv = sx_priv(my_dev);
        u32             swid;

        IGNORE_IF_NO_PCI(my_dev, "disable_swid", count);

        running = sx_proc_str_get_u32(running, &swid);
        spin_lock(&priv->profile.profile_lock);
        if (!priv->profile.pci_profile_set || (swid >= NUMBER_OF_SWIDS)
            || (priv->profile.pci_profile.swid_type[swid] ==
                SX_KU_L2_TYPE_DONT_CARE) ||
            !sx_bitmap_test(&sx_priv(my_dev)->swid_bitmap,
                            swid)) {
            PROC_DUMP(my_dev, "Err in param not executing disable_swid\n");
            spin_unlock(&priv->profile.profile_lock);
            goto out;
        }
        spin_unlock(&priv->profile.profile_lock);
        PROC_DUMP_DBG(my_dev, "Executing disable_swid. swid num = %u\n", swid);
        sx_disable_swid(my_dev, swid);
        PROC_DUMP_DBG(my_dev, "Finished Executing disable_swid\n");
        cmd++;
    }

    p = strstr(cmd_str, "change_conf");
    if (p != NULL) {
        IGNORE_IF_NO_PCI(my_dev, "change_conf", count);

        PROC_DUMP_DBG(my_dev, "Executing change_configuration. \n");
        err = sx_change_configuration(my_dev);
        if (err) {
            PROC_DUMP_DBG(my_dev, "Failed in Executing change_configuration. err = %d\n", err);
        } else {
            PROC_DUMP_DBG(my_dev, "Finished Executing change_configuration. err = %d\n", err);
        }
        cmd++;
    }

    p = strstr(cmd_str, "pcidrv_restart");
    if (p != NULL) {
        bool         do_reset = true;
        sxd_dev_id_t dev_id = priv_data->dev_id;
        char        *arg_reset = NULL;

        IGNORE_IF_NO_PCI(my_dev, "pcidrv_restart", count);

        running = __sx_proc_str_get_str(running, &arg_reset);
        if (arg_reset) {
            if (strcmp(arg_reset, "no_reset") == 0) {
                do_reset = false;
            } else if (strcmp(arg_reset, "reset") == 0) {
                do_reset = true;
            } else {
                PROC_DUMP(my_dev, "pcidrv_restart: wrong argument to pcidrv_restart command\n");
                goto out;
            }
        }

        if (!my_dev) {
            PROC_DUMP(my_dev, "pcidrv_restart: device %u was not found\n", dev_id);
            goto out;
        }

        sxd_log_notice(my_dev, "pcidrv_restart: restart PCI device %u\n", dev_id);
        err = sx_restart_one_pci(dev_id, do_reset);
        if (err) {
            PROC_DUMP_DBG(my_dev,
                          "pcidrv_restart: Failed in restarting pci driver. err = %d\n",
                          err);
        } else {
            PROC_DUMP_DBG(my_dev,
                          "pcidrv_restart: Finished restarting pci driver. err = %d\n",
                          err);
        }
        cmd++;
    }

    p = strstr(cmd_str, "pcidrv_add");
    if (p != NULL) {
        bool            do_reset = true;
        char           *arg_reset = NULL;
        struct pci_dev *pdev = NULL;
        sxd_dev_id_t    dev_id = priv_data->dev_id;

        IGNORE_IF_NO_PCI(my_dev, "pcidrv_add", count);

        running = __sx_proc_str_get_str(running, &arg_reset);
        if (arg_reset) {
            if (strcmp(arg_reset, "no_reset") == 0) {
                do_reset = false;
            } else if (strcmp(arg_reset, "reset") == 0) {
                do_reset = true;
            } else {
                PROC_DUMP(my_dev, "pcidrv_add: wrong argument to pcidrv_restart command\n");
                goto out;
            }
        }

        if (my_dev) {
            sxd_log_notice(my_dev, "pcidrv_add: device %u already exists, no need to create one\n", dev_id);
            goto out;
        }

        if (sx_dev_db_dev_id_to_pdev(dev_id, &pdev) != 0) {
            PROC_DUMP(my_dev, "pcidrv_add: device %u was not found\n", dev_id);
            goto out;
        }

        sxd_log_notice(my_dev, "pcidrv_add: add device %u\n", dev_id);
        err = sx_add_one_pci(pdev, do_reset);
        if (err) {
            PROC_DUMP_DBG(my_dev, "pcidrv_add: failed (err=%d)\n", err);
        } else {
            PROC_DUMP_DBG(my_dev, "pcidrv_add: Finished successfully\n");
        }
        cmd++;
    }

    p = strstr(cmd_str, "pcidrv_del");
    if (p != NULL) {
        sxd_dev_id_t    dev_id = priv_data->dev_id;
        struct pci_dev *pdev = NULL;

        IGNORE_IF_NO_PCI(my_dev, "pcidrv_del", count);

        if (!my_dev) {
            sxd_log_notice(my_dev, "pcidrv_del: device %u does not exist, no need to delete it\n", dev_id);
            goto out;
        }

        if (sx_dev_db_dev_id_to_pdev(dev_id, &pdev) != 0) {
            PROC_DUMP(my_dev, "pcidrv_del: device %u was not found\n", dev_id);
            goto out;
        }

        sxd_log_notice(my_dev, "pcidrv_del: del device %u\n", dev_id);
        sx_remove_one_pci(pdev);
        PROC_DUMP_DBG(my_dev, "pcidrv_del: Finished successfully\n");
        cmd++;
    }

    p = strstr(cmd_str, "dump_buf");
    if (p != NULL) {
        unsigned long val, addr, cnt;
        int           i;
        running = __sx_proc_str_get_ulong(running, &cnt);
        __sx_proc_str_get_ulong(running, &addr);
        for (i = 0; i < cnt; i++) {
            if ((i == 0) || (i % 4 == 0)) {
                sxd_log_info(my_dev, "\n 0x%04x : ", i);
            }

            sx_proc_dbg_rd(1, addr + i, &val);
            sxd_log_info(my_dev, " 0x%02x", (u8)val);
        }
        sxd_log_info(my_dev, "\n");
        cmd++;
    }

    p = strstr(cmd_str, "dump_fw");
    if (p != NULL) {
        PROC_DUMP(my_dev, "Dump fw: \n");
        __sx_proc_dump_fw_icm(my_dev);
        cmd++;
    }

    p = strstr(cmd_str, "dump_swid");
    if (p != NULL) {
        PROC_DUMP_DBG(my_dev, "Dump active swids: \n");
        __sx_proc_dump_swids(my_dev);
        cmd++;
    }

    p = strstr(cmd_str, "dump_sdq");
    if (p != NULL) {
        IGNORE_IF_NO_PCI(my_dev, "dump_sdq", count);

        PROC_DUMP_DBG(my_dev, "Dump active sdqs: \n");
        __sx_proc_dump_sdq(my_dev);
        cmd++;
    }

    p = strstr(cmd_str, "dump_rdq");
    if (p != NULL) {
        IGNORE_IF_NO_PCI(my_dev, "dump_rdq", count);

        PROC_DUMP_DBG(my_dev, "Dump active rdqs: \n");
        __sx_proc_dump_rdq(my_dev);
        cmd++;
    }

    p = strstr(cmd_str, "dump_eq");
    if (p != NULL) {
        IGNORE_IF_NO_PCI(my_dev, "dump_eq", count);

        PROC_DUMP_DBG(my_dev, "Dump active eqs: \n");
        __sx_proc_dump_eq(my_dev);
        cmd++;
    }

    p = strstr(cmd_str, "dump_tg_attr");
    if (p != NULL) {
        IGNORE_IF_NO_PCI(my_dev, "dump_tg_attr", count);

        PROC_DUMP_DBG(my_dev, "Dump trap groups attributes: \n");
        __sx_proc_dump_trap_groups_attr(my_dev);
        cmd++;
    }

    p = strstr(cmd_str, "show_rdq_post");
    if (p != NULL) {
        u32 rdq_idx;

        IGNORE_IF_NO_PCI(my_dev, "show_rdq_post", count);

        running = sx_proc_str_get_u32(running, &rdq_idx);

        PROC_DUMP_DBG(my_dev, "Show rdqs data: \n");
        __sx_proc_dump_rdq_single(my_dev, rdq_idx);
        cmd++;
    }

    p = strstr(cmd_str, "dev_prof");
    if (p != NULL) {
        IGNORE_IF_NO_PCI(my_dev, "dev_prof", count);

        PROC_DUMP_DBG(my_dev, "Set dev profile: \n");
        __sx_proc_set_dev_profile(my_dev);
        cmd++;
    }

    p = strstr(cmd_str, "dump_synd");
    if (p != NULL) {
        PROC_DUMP_DBG(my_dev, "Dump syndromes: \n");
        sx_core_dump_synd_tbl(my_dev);
        cmd++;
    }

    p = strstr(cmd_str, "dump_stats");
    if (p != NULL) {
        PROC_DUMP_DBG(my_dev, "Dump statistics: \n");
        __dump_stats(my_dev);
        cmd++;
    }
    p = strstr(cmd_str, "trace_pkt");
    if (p != NULL) {
        u32 emad_type;
        u32 max_cnt;
        u32 direction;
        running = sx_proc_str_get_u32(running, &emad_type);
        running = sx_proc_str_get_u32(running, &max_cnt);
        running = sx_proc_str_get_u32(running, &direction);

        PROC_DUMP(my_dev, "trace_emad type:0x%x max_cnt:0x%x direction:%d\n",
                  emad_type, max_cnt, direction);
        __sx_proc_dump_emad(my_dev, emad_type, max_cnt, direction);
        cmd++;
    }

    p = strstr(cmd_str, "dump_netdev");
    if (p != NULL) {
        PROC_DUMP(my_dev, "dump netdev info.\n");
        __debug_dump_netdev(my_dev);
        cmd++;
    }

    p = strstr(cmd_str, "dbg_trap");
    if (p != NULL) {
        u32 trap_id;
        u32 is_add;
        running = sx_proc_str_get_u32(running, &trap_id);
        running = sx_proc_str_get_u32(running, &is_add);

        PROC_DUMP(my_dev, "debug netdev trap: trap_id: %d (0-all 5 traps), is_add; %d \n",
                  trap_id, is_add);
        __debug_netdev_traps(my_dev, trap_id, is_add);
        cmd++;
    }

    p = strstr(cmd_str, "oper_dev");
    if (p != NULL) {
        char *dev_name;
        char *oper_state;
        running = __sx_proc_str_get_str(running, &dev_name);
        running = __sx_proc_str_get_str(running, &oper_state);

        PROC_DUMP(my_dev, "debug set dev %s oper state: %s \n",
                  dev_name, oper_state);
        __set_net_dev_oper_state(dev_name, oper_state);
        cmd++;
    }

    p = strstr(cmd_str, "dump_dev");
    if (p != NULL) {
        PROC_DUMP(my_dev, "Dump netdev :\n");
        __dump_net_dev_oper_state();
        cmd++;
    }

    p = strstr(cmd_str, "dump_kdb");
    if (p != NULL) {
        IGNORE_IF_NO_PCI(my_dev, "dump_kdb", count);

        PROC_DUMP(my_dev, "Dump kernel DBs:\n");
        __dump_kdbs();
        cmd++;
    }

    p = strstr(cmd_str, "set_rdq_rl");
    if (p != NULL) {
        PROC_DUMP(my_dev, "set_rdq_rl: rate limiter is obsolete\n");
        cmd++;
    }

    p = strstr(cmd_str, "get_rdq_rl");
    if (p != NULL) {
        PROC_DUMP(my_dev, "get_rdq_rl: rate limiter is obsolete\n");
        cmd++;
    }

    p = strstr(cmd_str, "show_cq");
    if (p != NULL) {
        u32 cq_idx;

        IGNORE_IF_NO_PCI(my_dev, "show_cq", count);

        running = sx_proc_str_get_u32(running, &cq_idx);

        PROC_DUMP(my_dev, "Show CQ data: \n");
        __sx_proc_show_cq(my_dev, cq_idx);
        cmd++;
    }

    p = strstr(cmd_str, "flush_rdq");
    if (p != NULL) {
        u32 idx;

        IGNORE_IF_NO_PCI(my_dev, "flush_rdq", count);
        running = sx_proc_str_get_u32(running, &idx);

        PROC_DUMP(my_dev, "Flush RDQ %d: \n", idx);
        __sx_proc_flush_rdq(my_dev, idx);
        cmd++;
    }

    p = strstr(cmd_str, "set_mkey");
    if (p != NULL) {
        __mkey_set(my_dev, running);
        cmd++;
    }

    p = strstr(cmd_str, "get_mkey");
    if (p != NULL) {
        __mkey_get(my_dev, running);
        cmd++;
    }

    p = strstr(cmd_str, "accuflows");
    if (p != NULL) {
        unsigned int op;
        running = sx_proc_str_get_u32(running, &op);
        sx_af_counters_proc_fs_handler(my_dev, (sx_af_counters_proc_fs_handler_op_e)op);
        cmd++;
    }

    p = strstr(cmd_str, "ptp_max_diff_clear");
    if (p != NULL) {
        char *op = NULL;

        running = __sx_proc_str_get_str(running, &op);
        if (op) {
            if ((strcmp(op, "RX") == 0) || (strcmp(op, "RX_TX") == 0)) {
                sx_priv(my_dev)->ptp.hwd.spc1.max_diff[0] = 0;
                sx_priv(my_dev)->ptp.hwd.spc1.max_diff_ts[0] = 0;
            }
            if ((strcmp(op, "TX") == 0) || (strcmp(op, "RX_TX") == 0)) {
                sx_priv(my_dev)->ptp.hwd.spc1.max_diff[1] = 0;
                sx_priv(my_dev)->ptp.hwd.spc1.max_diff_ts[1] = 0;
            }
        }
        cmd++;
    }

    p = strstr(cmd_str, "sdq_max_diff_clear");
    if (p != NULL) {
        unsigned int sdqn = (unsigned int)-1;
        running = sx_proc_str_get_u32(running, &sdqn);
        if ((sdqn < NUMBER_OF_SDQS) && sx_priv(my_dev)->sdq_table.dq[sdqn]) {
            sx_priv(my_dev)->sdq_table.dq[sdqn]->max_comp_time = 0;
            sx_priv(my_dev)->sdq_table.dq[sdqn]->max_comp_ts = 0;
        }

        cmd++;
    }

    p = strstr(cmd_str, "health_check_trigger");
    if (p) {
        struct sx_health_check_trigger_params params;
        char                                 *op = NULL;

        memset(&params, 0, sizeof(params));

        running = __sx_proc_str_get_str(running, &op);
        if (!op) {
            PROC_DUMP(my_dev, "health-check-trigger: missing operation\n");
            goto out_health_check_trigger;
        }

        if (strcmp(op, "add_dev") == 0) {
            params.op = SX_HEALTH_CHECK_TRIGGER_OP_ADD_DEV;
        } else if (strcmp(op, "del_dev") == 0) {
            params.op = SX_HEALTH_CHECK_TRIGGER_OP_DEL_DEV;
        } else if (strcmp(op, "sysfs") == 0) {
            params.op = SX_HEALTH_CHECK_TRIGGER_OP_SYSFS;
        } else if (strcmp(op, "catas") == 0) {
            params.op = SX_HEALTH_CHECK_TRIGGER_OP_CATAS;
        } else if (strcmp(op, "cmd_ifc") == 0) {
            params.op = SX_HEALTH_CHECK_TRIGGER_OP_CMD_IFC;
        } else if (strcmp(op, "sdq") == 0) {
            params.op = SX_HEALTH_CHECK_TRIGGER_OP_SDQ;
        } else if (strcmp(op, "rdq") == 0) {
            params.op = SX_HEALTH_CHECK_TRIGGER_OP_RDQ;
        } else if (strcmp(op, "cancel_all") == 0) {
            params.op = SX_HEALTH_CHECK_TRIGGER_OP_CANCEL_ALL;
        } else if (strcmp(op, "kernel_threads") == 0) {
            params.op = SX_HEALTH_CHECK_TRIGGER_OP_KERNEL_THREADS;
        } else if (strcmp(op, "sx_dbg_test_fw_fatal_event") == 0) {
            params.op = SX_HEALTH_CHECK_TRIGGER_OP_FW_FATAL_EVENT_TEST;
        } else if (strcmp(op, "sx_dbg_test_fw_fatal_cause") == 0) {
            params.op = SX_HEALTH_CHECK_TRIGGER_OP_FW_CAUSE_TEST;
        } else if (strcmp(op, "sx_dbg_test_fw_assert") == 0) {
            params.op = SX_HEALTH_CHECK_TRIGGER_OP_FW_ASSERT_TEST;
        } else if (strcmp(op, "sx_dbg_test_fw_tile_assert") == 0) {
            params.op = SX_HEALTH_CHECK_TRIGGER_OP_FW_TILE_ASSERT_TEST;
        } else if (strcmp(op, "sx_dbg_test_fw_tile_fatal_cause") == 0) {
            params.op = SX_HEALTH_CHECK_TRIGGER_OP_FW_TILE_FATAL_CAUSE_TEST;
        } else if (strcmp(op, "sx_dbg_test_pll_lock_cause") == 0) {
            params.op = SX_HEALTH_CHECK_TRIGGER_OP_FW_PLL_FATAL_CAUSE_TEST;
        } else if (strcmp(op, "sx_dbg_extend_thread_monitor_threshold") == 0) {
            params.op = SX_HEALTH_CHECK_TRIGGER_OP_EXTEND_THRESHOLD_THREAD_MONITOR;
        } else if (strcmp(op, "sx_dbg_set_new_thread_monitor_threshold") == 0) {
            params.op = SX_HEALTH_CHECK_TRIGGER_OP_SET_NEW_THRESHOLD_THREAD_MONITOR;
        } else {
            PROC_DUMP(my_dev, "health-check-trigger: invalid operation\n");
            goto out_health_check_trigger;
        }

        if (params.op == SX_HEALTH_CHECK_TRIGGER_OP_KERNEL_THREADS) {
            running = __sx_proc_str_get_str(running, &params.params.kthread_params.name);

            if (!params.params.kthread_params.name) {
                PROC_DUMP(my_dev, "health-check-trigger: missing kernel thread name\n");
                goto out_health_check_trigger;
            }
        }
        if ((params.op != SX_HEALTH_CHECK_TRIGGER_OP_SYSFS) &&
            (params.op != SX_HEALTH_CHECK_TRIGGER_OP_KERNEL_THREADS)) {
            running = sx_proc_str_get_u32(running, &dev_id);
            if (dev_id == 0) {
                PROC_DUMP(my_dev, "health-check-trigger: missing dev-id\n");
                goto out_health_check_trigger;
            }

            params.dev_id = (u8)dev_id;

            if ((params.op == SX_HEALTH_CHECK_TRIGGER_OP_SDQ) || (params.op == SX_HEALTH_CHECK_TRIGGER_OP_RDQ)) {
                if (!running) {
                    PROC_DUMP(my_dev, "health-check-trigger: missing dqn\n");
                    goto out_health_check_trigger;
                }

                running = sx_proc_str_get_u32(running, &params.params.dq_params.dqn);
            }
        }

        if (params.op == SX_HEALTH_CHECK_TRIGGER_OP_EXTEND_THRESHOLD_THREAD_MONITOR) {
            running = sx_proc_str_get_u32(running, &params.params.sdk_threshold_debug_params.mult_value);
            /* This is the last argument, Running will be NULL At this point - Don't validate it, only the mult_value */
            if (params.params.sdk_threshold_debug_params.mult_value == 0) {
                PROC_DUMP(my_dev,
                          "health-check-trigger: sdk_threshold_debug_params.mult_value is 0\n");
                goto out_health_check_trigger;
            }
        }
        if (params.op == SX_HEALTH_CHECK_TRIGGER_OP_SET_NEW_THRESHOLD_THREAD_MONITOR) {
            running = sx_proc_str_get_u32(running, &params.params.sdk_threshold_debug_params.new_debug_threshold_time);
            /* This is the last argument, Running will be NULL At this point - Don't validate it, only the new_debug_threshold_time
             * we choose 200 as a max value because the longest time for thread is 200  */
            if ((params.params.sdk_threshold_debug_params.new_debug_threshold_time > 200) ||
                (params.params.sdk_threshold_debug_params.new_debug_threshold_time == 0)) {
                PROC_DUMP(my_dev,
                          "health-check-trigger: sdk_threshold_debug_params.new_debug_threshold_time [%d] is out of range 1-200 0\n",
                          params.params.sdk_threshold_debug_params.new_debug_threshold_time);
                goto out_health_check_trigger;
            }
        }

        sx_health_check_set_debug_trigger(my_dev, &params);

out_health_check_trigger:
        cmd++;
    }

    if (cmd == 0) {
        print_sx_proc_help();
    }

out:
    return count;
}

static void print_sx_proc_help(void)
{
    PROC_DUMP(SXD_DEV_NONE, "Available Commands:\n");
    if (sx_core_has_pci_multi_dev()) {
        PROC_DUMP(SXD_DEV_NONE,
                  "This is a multi asic system - every command should have [dev_id] as the first parameter!\n");
    }
    PROC_DUMP(SXD_DEV_NONE, "  add_port_netdev [sys_port] [name] - Create a port netdevice\n");
    PROC_DUMP(SXD_DEV_NONE, "  rem_port_netdev [sys_port] - Destroy a port netdevice\n");
    PROC_DUMP(SXD_DEV_NONE, "  dump_dpt - dump DPT table\n");
    PROC_DUMP(SXD_DEV_NONE, "  dpt_setp [dev_id] [path type] - set DPT table entry path\n");
    PROC_DUMP(SXD_DEV_NONE, "  dpt_set_cr [dev_id] [path type] - set DPT path for cr_space\n");
    PROC_DUMP(SXD_DEV_NONE, "  cr_space_write [dev_id] [address] [value] - write dword to the cr_space\n");
    PROC_DUMP(SXD_DEV_NONE, "  cr_space_read [dev_id] [address] - read dword from the cr_space\n");
    PROC_DUMP(SXD_DEV_NONE, "  q_fw - query fw\n");
    PROC_DUMP(SXD_DEV_NONE, "  q_board_info - query board info\n");
    PROC_DUMP(SXD_DEV_NONE, "  q_aq_cap - query async queues capabilities\n");
    PROC_DUMP(SXD_DEV_NONE, "  get_profile - "
              "runs CONFIG_PROFILE with get modifier\n");
    PROC_DUMP(SXD_DEV_NONE, "  access_reg [dev_id] [reg_id] [additional info]- runs "
              "ACCESS_REG (get) for the register with the "
              "given reg_id\n");
    PROC_DUMP(SXD_DEV_NONE, "  mem_wr [width 1 2 4] [address] [value] - "
              "write memory\n");
    PROC_DUMP(SXD_DEV_NONE, "  mem_rd [width 1 2 4] [address] - read memory\n");
    PROC_DUMP(SXD_DEV_NONE, "  sw_reset - run SW reset\n");
    PROC_DUMP(SXD_DEV_NONE, "  sys_status - get system status from FW\n");
    PROC_DUMP(SXD_DEV_NONE, "  enable_swid [swid_num] - enable the swid\n");
    PROC_DUMP(SXD_DEV_NONE, "  disable_swid [swid_num] - disable the swid\n");
    PROC_DUMP(SXD_DEV_NONE, "  change_conf [swid_num] - execute "
              "change_configuration\n");
    PROC_DUMP(SXD_DEV_NONE, "  pcidrv_restart - restart pci driver\n");
    PROC_DUMP(SXD_DEV_NONE, "  dump_buf [size] [address] - dumps a buffer\n");
    PROC_DUMP(SXD_DEV_NONE, "  dump_swid - dumps active swids\n");
    PROC_DUMP(SXD_DEV_NONE, "  dump_fw_icm - dumps ICM\n");
    PROC_DUMP(SXD_DEV_NONE, "  dump_sdq - dumps opened sdqs\n");
    PROC_DUMP(SXD_DEV_NONE, "  dump_rdq - dumps opened rdqs\n");
    PROC_DUMP(SXD_DEV_NONE, "  dump_eq - dumps opened eqs\n");
    PROC_DUMP(SXD_DEV_NONE, "  show_rdq_post <rdq> - dumps one rdq\n");
    PROC_DUMP(SXD_DEV_NONE, "  rdq_start <rdq> - enable handling events for RDQ\n");
    PROC_DUMP(SXD_DEV_NONE, "  rdq_stop <rdq> - disable handling events for RDQ\n");
    PROC_DUMP(SXD_DEV_NONE, "  dev_prof - set dev_profile\n");
    PROC_DUMP(SXD_DEV_NONE, "  dump_synd - dump syndromes\n");
    PROC_DUMP(SXD_DEV_NONE, "  dump_stats - dump statistics\n");
    PROC_DUMP(SXD_DEV_NONE, "  trace_pkt [type] [max_cnt (0:disable)] [direction rx:1, tx:2, both:3]\n");
    PROC_DUMP(SXD_DEV_NONE, "  dump_netdev - dump netdev info\n");
    PROC_DUMP(SXD_DEV_NONE,
              "  dbg_trap [trap_id] [is_add] - debug trap add/rem (trap_id = 0 add/rem all traps).\n");
    PROC_DUMP(SXD_DEV_NONE, "  oper_dev [up | down] - set oper state of netdev \n");
    PROC_DUMP(SXD_DEV_NONE, "  dump_dev - set oper state of netdev \n");
    PROC_DUMP(SXD_DEV_NONE, "  dump_kdb - dump kernel dbs \n");
    PROC_DUMP(SXD_DEV_NONE, "  show_cq [CQ] - dump CQ \n");
    PROC_DUMP(SXD_DEV_NONE, "  flush_rdq [RDQ] - flush RDQ\n");
    PROC_DUMP(SXD_DEV_NONE, "  backup_poll [disable(0)/enable(1)/query(3)] - Enable CQ backup polling mechanism\n");
    PROC_DUMP(SXD_DEV_NONE, "  debug_cq_bckp_poll_cqn [cqn] - trigger backup polling for CQN\n");
    PROC_DUMP(SXD_DEV_NONE, "  set_mkey [dev_id] [m_key_h] [m_key_l] - set system mkey\n");
    PROC_DUMP(SXD_DEV_NONE, "  get_mkey [dev_id] - get system mkey\n");
    PROC_DUMP(SXD_DEV_NONE, "  dump_ptp - dump PTP internal DB's\n");
    PROC_DUMP(SXD_DEV_NONE, "  accuflows [op] - query the Accumulated counters, op: [0 - clear the dbg DB]\n");
    PROC_DUMP(SXD_DEV_NONE,
              "  health_check_trigger <add_dev|del_dev|sysfs|catas|cmd_ifc|sdq|rdq> [dev-id] [dqn]\n");
    PROC_DUMP(SXD_DEV_NONE, "  ptp_max_diff_clear <RX | TX | RX_TX>\n");
    PROC_DUMP(SXD_DEV_NONE, "  sdq_max_diff_clear <sdq>\n");
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static const struct proc_ops sx_proc_ops = {
    .proc_open = sx_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = sx_proc_write,
};

static const struct proc_ops driver_version_info_proc_ops = {
    .proc_open = driver_version_info_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_write = driver_version_info_write,
};
#else
static const struct file_operations sx_proc_ops = {
    .owner = THIS_MODULE,
    .open = sx_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
    .write = sx_proc_write,
};

static const struct file_operations driver_version_info_proc_ops = {
    .owner = THIS_MODULE,
    .open = driver_version_info_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
    .write = driver_version_info_write,
};
#endif

static void __destroy_tree(void)
{
    char entry_name[32];
    int  i;

    for (i = 0; i < SX_MAX_PCI_DEVICES; i++) {
        if (__proc_entry[i]) {
            remove_proc_entry("sx_core", __dir_proc_entry[i]);
        }

        if (sx_core_has_pci_multi_dev()) {
            if (__dir_proc_entry[i]) {
                snprintf(entry_name, sizeof(entry_name) - 1, "asic%d", i);
                remove_proc_entry(entry_name, __root_entry);
            }
        }
    }

    if (__driver_version_info_entry) {
        remove_proc_entry(SX_DRIVER_VERSION_INFO, NULL);
    }

    remove_proc_entry("mlx_sx", NULL);
}

int sx_core_init_proc_fs(int num_of_pci_devices)
{
    char entry_name[32];
    int  i;

    WARN_ON(num_of_pci_devices > SX_MAX_PCI_DEVICES);

    __root_entry = proc_mkdir("mlx_sx", NULL);
    if (!__root_entry) {
        sxd_log_err(SXD_DEV_NONE, "SX proc_fs: failed to create root directory\n");
        return -EINVAL;
    }

    if (sx_core_has_pci_multi_dev()) {
        for (i = 0; i < num_of_pci_devices; i++) {
            snprintf(entry_name, sizeof(entry_name) - 1, "asic%d", i);
            __dir_proc_entry[i] = proc_mkdir(entry_name, __root_entry);
            if (!__dir_proc_entry[i]) {
                sxd_log_err(SXD_DEV_NONE, "SX proc_fs: failed to create directory entry '%s'\n", entry_name);
                goto failure;
            }
        }
    } else { /* legacy mode */
        __dir_proc_entry[0] = __root_entry;
        num_of_pci_devices = 1;
    }

    for (i = 0; i < num_of_pci_devices; i++) {
        __priv_data[i].dev_id = (sxd_dev_id_t)i + 1;

        __proc_entry[i] = proc_create_data("sx_core",
                                           S_IFREG | S_IRUGO | S_IWUGO,
                                           __dir_proc_entry[i],
                                           &sx_proc_ops,
                                           &__priv_data[i]);
        if (!__proc_entry[i]) {
            sxd_log_err(SXD_DEV_NONE, "SX proc_fs: failed to create 'sx_core' entry for device %d\n", i + 1);
            goto failure;
        }
    }

    __driver_version_info_entry = proc_create(SX_DRIVER_VERSION_INFO,
                                              S_IFREG | S_IRUGO | S_IWUGO,
                                              NULL,
                                              &driver_version_info_proc_ops);
    if (!__driver_version_info_entry) {
        sxd_log_err(SXD_DEV_NONE, "SX proc_fs: failed to create '" SX_DRIVER_VERSION_INFO "' entry\n");
        goto failure;
    }

    return 0;

failure:
    __destroy_tree();
    return -ENOMEM;
}

void sx_core_close_proc_fs(void)
{
    __destroy_tree();
}

void sx_proc_dbg_wr(int width, unsigned long addr, unsigned long val)
{
    if (addr == 0) {
        sxd_log_warning(SXD_DEV_NONE, "%s() unsupported addr NULL \n", __func__);
    }

    switch (width) {
    case 1:
        *(u8*)addr = (u8)val;
        break;

    case 2:
        *(u16*)addr = (u16)val;
        break;

    case 4:
        *(u32*)addr = (u32)val;
        break;

    case 8:
        *(u64*)addr = val;
        break;

    default:
        sxd_log_warning(SXD_DEV_NONE, "%s() unsupported width %d \n",
                        __func__, width);
    }
}

void sx_proc_dbg_rd(int width, unsigned long addr, unsigned long *val)
{
    if (addr == 0) {
        sxd_log_warning(SXD_DEV_NONE, "%s() unsupported addr NULL \n", __func__);
    }

    switch (width) {
    case 1:
        *val = *(u8*)addr;
        break;

    case 2:
        *val = *(u16*)addr;
        break;

    case 4:
        *val = *(u32*)addr;
        break;

    case 8:
        *val = *(u64*)addr;
        break;

    default:
        sxd_log_warning(SXD_DEV_NONE, "%s() unsupported width %d \n",
                        __func__, width);
    }
}


void __sx_proc_dump_swids(struct sx_dev *dev)
{
    int i;

    for (i = 0; i < NUMBER_OF_SWIDS; i++) {
        if (sx_bitmap_test(&sx_priv(dev)->swid_bitmap, i)) {
            sxd_log_info(dev, "Swid %i is ACTIVE.\n", i);
        } else {
            sxd_log_info(dev, "Swid %i is DISABLED.\n", i);
        }
    }
}

static bool __map_fa_desc_dump(struct sx_dev *dev,
                               void          *buff,
                               dma_addr_t     dma_addr,
                               u32            num_of_pages,
                               bool           is_last,
                               void          *context)
{
    sxd_log_info(dev, "icm_addr:%lx icm_size (pages):%u",
                 (unsigned long)dma_addr,
                 num_of_pages);
    return false; /* continue iteration */
}

void __sx_proc_dump_fw_icm(struct sx_dev *dev)
{
    struct sx_priv    *priv = sx_priv(dev);
    sx_map_fa_handle_t fw_load = priv->fw.map_fa_items[SXD_MAP_MEMORY_TYPE_FW_LOAD_E];

    if (fw_load == SX_MAP_FA_INVALID_HANDLE) {
        sxd_log_info(dev, "No FW memory\n");
        return;
    }

    sxd_log_info(dev, "FW_ICM");
    sx_map_fa_item_iterate(fw_load, __map_fa_desc_dump, NULL);
}

void __sx_proc_dump_sdq(struct sx_dev *dev)
{
    struct sx_priv     *priv = sx_priv(dev);
    struct sx_dq_table *sdq_table = &priv->sdq_table;
    int                 i;

    if (sdq_table == NULL) {
        sxd_log_info(dev, "sdq table is empty \n");
        return;
    }

    for (i = 0; i < priv->dev_cap.max_num_sdqs; i++) {
        if (sdq_table->dq[i]) {
            sxd_log_info(dev, "[sdq %d]: dqn:%d, is_send:%d, "
                         "head:%d, tail:%d,"
                         " wqe_cnt:%d, wqe_shift:%d,is_flush:%d,"
                         " state:%d, ref_cnt: %d\n cqn:%d, "
                         "cq_con_idx:%d, cq_nent:%d, "
                         "cq_ref_cnt: %d \n",
                         i,
                         sdq_table->dq[i]->dqn,
                         sdq_table->dq[i]->is_send,
                         sdq_table->dq[i]->head,
                         sdq_table->dq[i]->tail,
                         sdq_table->dq[i]->wqe_cnt,
                         sdq_table->dq[i]->wqe_shift,
                         sdq_table->dq[i]->is_flushing,
                         sdq_table->dq[i]->state,
                         atomic_read(
                             &sdq_table->dq[i]->refcount),

                         sdq_table->dq[i]->cq->cqn,
                         sdq_table->dq[i]->cq->cons_index,
                         sdq_table->dq[i]->cq->nent,
                         atomic_read(
                             &sdq_table->dq[i]->cq->refcount)
                         );
        }
    }
}

void __sx_proc_dump_rdq(struct sx_dev *dev)
{
    struct sx_priv     *priv = sx_priv(dev);
    struct sx_dq_table *rdq_table = &priv->rdq_table;
    struct sx_cq_table *cq_table = &priv->cq_table;
    int                 i;
    int                 cqn;

    if (rdq_table == NULL) {
        sxd_log_info(dev, "rdq table is empty \n");
        return;
    }

    if (cq_table == NULL) {
        sxd_log_info(dev, "cq table is empty \n");
        return;
    }

    for (i = 0; i < priv->dev_cap.max_num_rdqs; i++) {
        if (rdq_table->dq[i]) {
            cqn = i + NUMBER_OF_SDQS;
            sxd_log_info(dev, "[rdq %d]: dqn:%d, is_send:%d, "
                         "head:%d, tail:%d, wqe_cnt:%d, "
                         "wqe_shift:%d,is_flush:%d, state:%d, "
                         "ref_cnt: %d e_sz:%d \n cqn:%d, cq_con_idx:%d, "
                         "cq_nent:%d, cq_ref_cnt: %d, cq_on_hold:%d, "
                         "ts_en:%d, is_monitor:%d \n",
                         i,
                         rdq_table->dq[i]->dqn,
                         rdq_table->dq[i]->is_send,
                         rdq_table->dq[i]->head,
                         rdq_table->dq[i]->tail,
                         rdq_table->dq[i]->wqe_cnt,
                         rdq_table->dq[i]->wqe_shift,
                         rdq_table->dq[i]->is_flushing,
                         rdq_table->dq[i]->state,
                         atomic_read(&rdq_table->dq[i]->refcount),
                         priv->profile.pci_profile.rdq_properties[rdq_table->dq[i]->dqn].entry_size,
                         rdq_table->dq[i]->cq->cqn,
                         rdq_table->dq[i]->cq->cons_index,
                         rdq_table->dq[i]->cq->nent,
                         atomic_read(&rdq_table->dq[i]->cq->refcount),
                         cq_table->cq[cqn]->paused,
                         (SX_CQ_TIMESTAMP_TYPE(rdq_table->dq[i]->cq) != SXD_TS_TYPE_CQE_NONE),
                         (int)rdq_table->dq[i]->is_monitor);
        }
    }
}

void __sx_proc_dump_eq(struct sx_dev *dev)
{
    struct sx_priv     *priv = sx_priv(dev);
    struct sx_eq_table *eq_table = &priv->eq_table;
    int                 i;

    if (eq_table == NULL) {
        sxd_log_info(dev, "eq_table is empty \n");
        return;
    }

    for (i = 0; i < SX_NUM_EQ; i++) {
        sxd_log_info(dev, "[eq %d]: eqn:%d, nent:%d, cons_index:%d \n",
                     i, eq_table->eq[i].eqn, eq_table->eq[i].nent,
                     eq_table->eq[i].cons_index);
    }
}

void __sx_proc_dump_trap_groups_attr(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);
    int             i;

    for (i = 0; i < NUM_OF_TRAP_GROUPS; i++) {
        sxd_log_info(dev, "[trap group %d]: is_tac_v:%d, is_tac:%d \n",
                     i,
                     priv->trap_group_attr[i].skip_disconnect_on_restart_valid,
                     priv->trap_group_attr[i].skip_disconnect_on_restart);
    }
}

void __sx_proc_dump_rdq_single(struct sx_dev *dev, int rdq)
{
    struct sx_priv     *priv = sx_priv(dev);
    struct sx_dq_table *rdq_table = &priv->rdq_table;
    int                 i = rdq;

    sxd_log_info(dev, "Dump rdq index %d \n", rdq);

    if (rdq_table == NULL) {
        sxd_log_info(dev, "rdq table is empty \n");
        return;
    }

    if (rdq_table->dq[i]) {
        sxd_log_info(dev, "[rdq %d]: dqn:%d, is_send:%d, "
                     "head:%d, tail:%d, wqe_cnt:%d, "
                     "wqe_shift:%d,is_flush:%d, state:%d, "
                     "ref_cnt: %d e_sz:%d \n cqn:%d, cq_con_idx:%d, "
                     "cq_nent:%d, cq_ref_cnt: %d \n",
                     i,
                     rdq_table->dq[i]->dqn,
                     rdq_table->dq[i]->is_send,
                     rdq_table->dq[i]->head,
                     rdq_table->dq[i]->tail,
                     rdq_table->dq[i]->wqe_cnt,
                     rdq_table->dq[i]->wqe_shift,
                     rdq_table->dq[i]->is_flushing,
                     rdq_table->dq[i]->state,
                     atomic_read(
                         &rdq_table->dq[i]->refcount),
                     priv->profile.pci_profile.rdq_properties[rdq_table->dq[i]->dqn].entry_size,
                     rdq_table->dq[i]->cq->cqn,
                     rdq_table->dq[i]->cq->cons_index,
                     rdq_table->dq[i]->cq->nent,
                     atomic_read(
                         &rdq_table->dq[i]->cq->refcount)
                     );
    } else {
        sxd_log_info(dev, "rdq %d doesn't exist \n", i);
    }
}

/* Current values are for IB single swid with AR enabled */
void __sx_proc_set_dev_profile(struct sx_dev *dev)
{
    struct ku_profile            single_part_eth_device_profile = {
        .dev_id = 255,
        .set_mask_0_63 = 0xf3ff,
        .set_mask_64_127 = 0,
        .max_vepa_channels = 0,
        .max_lag = 0,
        .max_port_per_lag = 0,
        .max_mid = 0,
        .max_pgt = 0,
        .max_system_port = 0,
        .max_active_vlans = 0,
        .max_regions = 0,
        .max_flood_tables = 0,
        .max_per_vid_flood_tables = 0,
        .flood_mode = 0,
        .max_fid_offset_flood_tables = 0,
        .fid_offset_table_size = 0,
        .max_per_fid_flood_table = 0,
        .per_fid_table_size = 0,
        .max_ib_mc = 27,
        .max_pkey = 126,
        .ar_sec = 1,
        .adaptive_routing_group_cap = 2048,
        .arn = 0,
        .kvd_linear_size = 0,
        .kvd_hash_single_size = 0,
        .kvd_hash_double_size = 0,
        .swid0_config_type = {
            .mask = 1,
            .type = KU_SWID_TYPE_INFINIBAND
        },
        .swid1_config_type = {
            .mask = 1,
            .type = KU_SWID_TYPE_DISABLED
        },
        .swid2_config_type = {
            .mask = 1,
            .type = KU_SWID_TYPE_DISABLED
        },
        .swid3_config_type = {
            .mask = 1,
            .type = KU_SWID_TYPE_DISABLED
        },
        .swid4_config_type = {
            .mask = 1,
            .type = KU_SWID_TYPE_DISABLED
        },
        .swid5_config_type = {
            .mask = 1,
            .type = KU_SWID_TYPE_DISABLED
        },
        .swid6_config_type = {
            .mask = 1,
            .type = KU_SWID_TYPE_DISABLED
        },
        .swid7_config_type = {
            .mask = 1,
            .type = KU_SWID_TYPE_DISABLED
        }
    };
    struct profile_driver_params addition_params;

    addition_params.cqe_version = 0;
    addition_params.cqe_time_stamp_type = 0;

    sx_SET_PROFILE(dev, &single_part_eth_device_profile, &addition_params);
}


void __dump_stats(struct sx_dev* sx_dev)
{
    struct sx_priv *priv = sx_priv(sx_dev);
    int             swid, pkt_type, synd, rdq;

    for (swid = 0; swid < NUMBER_OF_SWIDS + 1; swid++) {
        sxd_log_info(sx_dev, "=========================\n");

        if (NUMBER_OF_SWIDS == swid) {
            sxd_log_info(sx_dev, "Packets stats for swid 254 \n");
        } else {
            sxd_log_info(sx_dev, "Packets stats for swid %d \n", swid);
        }

        for (pkt_type = 0; pkt_type < PKT_TYPE_NUM; pkt_type++) {
            if (0 != priv->stats.rx_by_pkt_type[swid][pkt_type]) {
                sxd_log_info(sx_dev, "rx pkt of type [%s (%d)]: %llu \n",
                             sx_cqe_packet_type_str[pkt_type], pkt_type,
                             priv->stats.rx_by_pkt_type[swid][pkt_type]);
            }

            if (0 != priv->stats.tx_by_pkt_type[swid][pkt_type]) {
                sxd_log_info(sx_dev, "tx pkt of type [%s (%d)]: %llu (%llu bytes)\n",
                             ku_pkt_type_str[pkt_type], pkt_type,
                             priv->stats.tx_by_pkt_type[swid][pkt_type],
                             priv->stats.tx_by_pkt_type_bytes[swid][pkt_type]);
            }
        }     /* for (pkt_type=0; pkt_type<PKT_TYPE_NUM; pkt_type++) { */

        for (synd = 0; synd < NUM_HW_SYNDROMES; synd++) {
            if (0 != priv->stats.rx_by_synd[swid][synd]) {
                sxd_log_info(sx_dev, "rx pkt on synd [%d]: %llu (%llu bytes)\n", synd,
                             priv->stats.rx_by_synd[swid][synd],
                             priv->stats.rx_by_synd_bytes[swid][synd]);
            }

            /* this counters aren't per swid so print them for swid 0 only */
            if (swid == 0) {
                if (0 != priv->stats.rx_eventlist_by_synd[synd]) {
                    sxd_log_info(sx_dev, "events on synd [%d]: %llu\n", synd,
                                 priv->stats.rx_eventlist_by_synd[synd]);
                }

                if (0 != priv->stats.tx_loopback_ok_by_synd[synd]) {
                    sxd_log_info(sx_dev, "tx loopback_ok on synd [%d]: %llu\n", synd,
                                 priv->stats.tx_loopback_ok_by_synd[synd]);
                }

                if (0 != priv->stats.tx_loopback_dropped_by_synd[synd]) {
                    sxd_log_info(sx_dev, "tx loopback_dropped on synd [%d]: %llu\n", synd,
                                 priv->stats.tx_loopback_dropped_by_synd[synd]);
                }
            }
        }     /* for (synd=0; synd<PKT_TYPE_NUM; synd++) { */

        for (rdq = 0; rdq < NUMBER_OF_RDQS; rdq++) {
            if (0 != priv->stats.rx_by_rdq[swid][rdq]) {
                sxd_log_info(sx_dev, "rx pkt on rdq [%d]: %llu (%llu bytes)\n", rdq,
                             priv->stats.rx_by_rdq[swid][rdq],
                             priv->stats.rx_by_rdq_bytes[swid][rdq]);
            }
        }    /* for (rdq = 0; rdq < NUMBER_OF_RDQS; rdq++) { */
    }

    sxd_log_err(sx_dev, "=========================\n");
    for (synd = 0; synd < NUM_HW_SYNDROMES; synd++) {
        for (pkt_type = 0; pkt_type < PKT_TYPE_NUM; pkt_type++) {
            if (priv->stats.rx_unconsumed_by_synd[synd][pkt_type] != 0) {
                sxd_log_info(sx_dev, "rx unconsumed on synd [%d] type [%s]: %llu \n",
                             synd, sx_cqe_packet_type_str[pkt_type],
                             priv->stats.rx_unconsumed_by_synd[synd][pkt_type]);
            }
        }
    }
    /* rx_eventlist_drops_by_synd */
    sxd_log_info(sx_dev, "=========================\n");
    for (synd = 0; synd < NUM_HW_SYNDROMES; synd++) {
        if (priv->stats.rx_eventlist_drops_by_synd[synd] != 0) {
            sxd_log_info(sx_dev, "rx event list drops on synd [%d]: %llu \n",
                         synd, priv->stats.rx_eventlist_drops_by_synd[synd]);
        }
    }

    sxd_log_info(sx_dev, "=========================\n");
    sxd_log_info(sx_dev, "event list_drops_counter: %llu \n"
                 "unconsumed_packets_counter: %llu \n"
                 "filtered_lag_packets_counter: %llu \n"
                 "filtered_port_packets_counter: %llu \n"
                 "loopback_packets_counter: %llu \n",
                 priv->stats.eventlist_drops_counter,
                 priv->stats.unconsumed_packets_counter,
                 priv->stats.filtered_lag_packets_counter,
                 priv->stats.filtered_port_packets_counter,
                 priv->stats.loopback_packets_counter);
}

void __sx_proc_dump_emad(struct sx_dev *my_dev, u32 emad_type, u32 max_cnt, u32 direction)
{
    struct sx_priv *priv = sx_priv(my_dev);

    if ((SX_PROC_DUMP_PKT_DISABLE == direction) ||
        (0 == max_cnt)) {
        MODULE_PARAM_INSTANCE_SET(priv, rx_debug, SX_DBG_DISABLE);
        rx_debug_metadata = SX_DBG_DISABLE;
        rx_debug_pkt_type = SX_DBG_PACKET_TYPE_ANY;
        rx_debug_emad_type = SX_DBG_EMAD_TYPE_ANY;
        rx_dump = SX_DBG_DISABLE;
        rx_dump_cnt = SX_DBG_COUNT_UNLIMITED;
        rx_cqev2_dbg = SX_DBG_DISABLE;

        MODULE_PARAM_INSTANCE_SET(priv, tx_debug, SX_DBG_DISABLE);
        tx_debug_metadata = SX_DBG_DISABLE;
        tx_debug_pkt_type = SX_DBG_PACKET_TYPE_ANY;
        tx_debug_emad_type = SX_DBG_EMAD_TYPE_ANY;
        tx_dump = SX_DBG_DISABLE;
        tx_dump_cnt = SX_DBG_COUNT_UNLIMITED;
        return;
    }

    if (direction & SX_PROC_DUMP_PKT_RX) {
        MODULE_PARAM_INSTANCE_SET(priv, rx_debug, SX_DBG_ENABLE);
        rx_debug_metadata = SX_DBG_ENABLE;
        rx_debug_pkt_type = 5; /* emad trap */
        rx_debug_emad_type = emad_type;
        rx_dump = SX_DBG_ENABLE;
        rx_dump_cnt = max_cnt;
    }

    if (direction & SX_PROC_DUMP_PKT_TX) {
        MODULE_PARAM_INSTANCE_SET(priv, tx_debug, SX_DBG_ENABLE);
        tx_debug_metadata = SX_DBG_ENABLE;
        tx_debug_pkt_type = SX_PKT_TYPE_EMAD_CTL;
        tx_debug_emad_type = emad_type;
        tx_dump = SX_DBG_ENABLE;
        tx_dump_cnt = max_cnt;
    }
}


void __debug_dump_netdev(struct sx_dev *dev)
{
    sxd_log_err(dev,
                "'debug_dump_netdev' is obsolete. use debug-dump proc entry 'netdev_dump' instead.\n");
}

void __debug_netdev_traps(struct sx_dev *dev, u32 trap_id, u32 is_add)
{
    int                  cmd;
    union sx_event_data *event_data = NULL;

    event_data = kzalloc(sizeof(union sx_event_data), GFP_KERNEL);
    if (!event_data) {
        sxd_log_err(dev, "Failed to allocate memory for event data.\n");
        return;
    }

    if (is_add == 1) {
        cmd = SX_DEV_EVENT_ADD_SYND_NETDEV;
    } else {
        cmd = SX_DEV_EVENT_REMOVE_SYND_NETDEV;
    }

    event_data->eth_l3_synd.swid = 0;
    event_data->eth_l3_synd.hw_synd = trap_id;

    sx_core_dispatch_event(dev, cmd, event_data);
    kfree(event_data);
}


void __set_net_dev_oper_state(char *dev_name, char *oper_state)
{
    struct net_device *dev;

    dev = first_net_device(&init_net);
    while (dev) {
        if ((dev_name != NULL) &&
            (!strcmp(dev->name, dev_name))) {
            sxd_log_rl_info(SXD_DEV_NONE, "Device [%s] set oper state to %s \n",
                            dev->name, oper_state);


            if (strstr(oper_state, "up")) {
                netif_dormant_off(dev);
                netif_carrier_on(dev);
                netif_start_queue(dev);
            } else if (strstr(oper_state, "down")) {
                netif_dormant_on(dev);
                netif_carrier_off(dev);
                netif_stop_queue(dev);
            } else {
                sxd_log_rl_err(SXD_DEV_NONE, "Unsupported oper_state: %s \n", oper_state);
            }
            break;
        }

        dev = next_net_device(dev);
    }
}

void __dump_net_dev_oper_state(void)
{
    struct net_device *dev;

    dev = first_net_device(&init_net);
    while (dev) {
        sxd_log_debug(SXD_DEV_NONE, "Found dev [%s]: dormant:%d, carrier:%d  \n",
                      dev->name, netif_dormant(dev), netif_carrier_ok(dev));

        dev = next_net_device(dev);
    }
}

void __dump_kdbs(void)
{
    int             dev_id = SX_SINGLE_ASIC_DEFAULT_DEV_ID;
    struct sx_dev  *my_dev = NULL;
    struct sx_priv *priv = NULL;
    int             i, j, is_valid = 0, is_printed = 0;
    uint16_t        phy_port_max = 0, rif_id = 0, fid = 0;
    uint16_t        lag_max = 0, lag_member_max = 0;

    my_dev = sx_dev_db_get_dev_by_id(dev_id);
    priv = sx_priv(my_dev);

    if (sx_core_get_phy_port_max(my_dev, &phy_port_max)) {
        sxd_log_info(my_dev, "Failed to get max number of phy ports.\n");
    }

    /* IB only */
    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "IB to LOCAL :\n");
    for (i = 0; i < MAX_IBPORT_NUM + 1; i++) {
        if (priv->ib_to_local_db[i] != 0) {
            sxd_log_info(my_dev, "[ib%d : local %d] \n",
                         i, priv->ib_to_local_db[i]);
        }
    }

    /* ETH only */
    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "ETH SYS to LOCAL :\n");
    for (i = 0; i < MAX_SYSPORT_NUM; i++) {
        if (priv->system_to_local_db[i] != 0) {
            sxd_log_info(my_dev, "[system 0x%x : local %d : pvid %d] \n",
                         i, priv->system_to_local_db[i], priv->pvid_sysport_db[i]);
        }
    }

    /* ETH only */
    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "ETH LOCAL to SYSTEM :\n");
    for (i = 0; i < MAX_PHYPORT_NUM + 1; i++) {
        if (priv->local_to_system_db[i] != 0) {
            sxd_log_info(my_dev, "[local 0x%x : system %d (0x%x) ] \n",
                         i, (int)priv->local_to_system_db[i], (int)priv->local_to_system_db[i]);
        }
    }

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "ETH LAG to PVID :\n");

    if (sx_core_get_lag_max(my_dev, &lag_max, &lag_member_max)) {
        sxd_log_info(my_dev, "Failed to get max number of LAGs.\n");
    }

    for (i = 0; i < lag_max; i++) {
        if (priv->pvid_lag_db[i] != 0) {
            sxd_log_info(my_dev, "[lid %d : pvid %d] \n",
                         i, priv->pvid_lag_db[i]);
        }
    }

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "ETH LAG MEMBER to LOCAL :\n");
    for (i = 0; i < lag_max; i++) { /* lid */
        for (j = 0; j < lag_member_max; j++) { /* lag member id */
            if (priv->lag_member_to_local_db[i][j] != 0) {
                if (is_printed == 0) {
                    sxd_log_info(my_dev, "LAG 0x%x: ", i);
                    is_printed = 1;
                }

                sxd_log_info(my_dev, "[%d:local %d], ",
                             j, priv->lag_member_to_local_db[i][j]);
            }
        }

        if (is_printed == 1) {
            sxd_log_info(my_dev, "\n");
        }

        is_printed = 0;
    }

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "ETH LAG_RP :\n");
    for (i = 0; i < lag_max; i++) { /* lid */
        if (priv->lag_is_rp[i] != 0) {
            sxd_log_info(my_dev, "%d, ", i);
        }
    }
    sxd_log_info(my_dev, "\n");

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "FID to HW_FID mapping dump :\n");
    sxd_log_info(my_dev, "%-7s| %-7s| %-7s\n", "#", "FID", "HW_FID");
    sxd_log_info(my_dev, "------------------------------------------\n");
    i = 0;
    for (fid = 0; fid < MAX_FIDS_NUM; fid++) {
        if (priv->fid_to_hwfid[fid] != INVALID_HW_FID_ID) {
            i++;
            sxd_log_info(my_dev, "%-7u| %-7u| %-7u\n",
                         i, fid, priv->fid_to_hwfid[fid]);
        }
    }
    sxd_log_info(my_dev, "\n");

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "RIF to HW_FID mapping dump :\n");
    sxd_log_info(my_dev, "%-7s| %-7s| %-7s\n", "#", "RIF", "HW_FID");
    sxd_log_info(my_dev, "------------------------------------------\n");
    i = 0;
    for (rif_id = 0; rif_id < MAX_RIFS_NUM; rif_id++) {
        if (priv->rif_id_to_hwfid[rif_id] != INVALID_HW_FID_ID) {
            i++;
            sxd_log_info(my_dev, "%-7u| %-7u| %-7u\n",
                         i, rif_id, priv->rif_id_to_hwfid[rif_id]);
        }
    }
    sxd_log_info(my_dev, "\n");

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "RIF data dump :\n");
    sxd_log_info(my_dev, "%-7s| %-7s| %-15s| %-7s|\n", "RIF", "IS_LAG", "LAG/PORT", "VLAN");
    sxd_log_info(my_dev, "------------------------------------------\n");
    i = 0;
    for (rif_id = 0; rif_id < MAX_RIFS_NUM; rif_id++) {
        if (priv->rif_data[rif_id].is_valid) {
            sxd_log_info(my_dev, "%-7u| %-7u| %-15u| %-7u|\n",
                         rif_id, priv->rif_data[rif_id].is_lag,
                         priv->rif_data[rif_id].is_lag ?
                         priv->rif_data[rif_id].lag_id : priv->rif_data[rif_id].local_port,
                         priv->rif_data[rif_id].vlan_id);
        }
    }
    sxd_log_info(my_dev, "\n");

    /* common */
    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "COMMON LOCAL TO SWID :\n");
    sxd_log_info(my_dev, "LOCAL\t| SWID\t|\n");
    sxd_log_info(my_dev, "------------------------------------------\n");
    for (i = 0; i < (phy_port_max + 1); i++) { /* system port */
        sxd_log_info(my_dev, "%02d \t| %d\t|  \n",
                     i, priv->local_to_swid_db[i]);
    }

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "COMMON TRUNCATE SIZE:\n");
    sxd_log_info(my_dev, "RDQ\t| tr_sz\t|\n");
    for (i = 0; i < (NUMBER_OF_RDQS); i++) { /* rdq */
        if (priv->truncate_size_db[i] != 0) {
            sxd_log_info(my_dev, "%d\t| %d\t|\n", i, priv->truncate_size_db[i]);
        }
    }

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "SYSPORT FILTER DB:\n");
    for (i = 0; i < (NUM_HW_SYNDROMES); i++) { /* trap id */
        for (j = 0; j < (MAX_SYSTEM_PORTS_IN_FILTER); j++) { /* system port */
            if (priv->sysport_filter_db[i][j] != 0) {
                sxd_log_info(my_dev, "HW_SYND 0x%x SYSPORT 0x%x\n",
                             i, priv->sysport_filter_db[i][j]);
            }
        }
    }

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "LAG FILTER DB:\n");
    for (i = 0; i < (NUM_HW_SYNDROMES); i++) { /* trap id */
        for (j = 0; j < (MAX_LAG_PORTS_IN_FILTER); j++) { /* system port */
            if (priv->lag_filter_db[i][j] != LAG_ID_INVALID) {
                sxd_log_info(my_dev, "HW_SYND 0x%x LAG_ID %d\n",
                             i, priv->lag_filter_db[i][j]);
            }
        }
    }

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "port_prio2tc:\n");

    for (i = 0; i < (phy_port_max + 1); i++) { /* system port */
        for (j = 0; j < (MAX_PRIO_NUM + 1); j++) { /* prio */
            is_valid += priv->port_prio2tc[i][j];
        }

        if (is_valid) {
            sxd_log_info(my_dev, "sysport 0x%x : ", i);
            for (j = 0; j < (MAX_PRIO_NUM + 1); j++) { /* prio */
                sxd_log_info(my_dev, "p%d:tc%d, ",
                             j, priv->port_prio2tc[i][j]);
            }
            sxd_log_info(my_dev, "\n");
        }
        is_valid = 0;
    }

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "lag_prio2tc:\n");
    for (i = 0; i < (lag_max + 1); i++) { /* lid */
        for (j = 0; j < (MAX_PRIO_NUM + 1); j++) { /* prio */
            is_valid += priv->lag_prio2tc[i][j];
        }

        if (is_valid) {
            sxd_log_info(my_dev, "lag_id %d : ", i);
            for (j = 0; j < (MAX_PRIO_NUM + 1); j++) { /* prio */
                sxd_log_info(my_dev, "p%d:tc%d, ",
                             j, priv->port_prio2tc[i][j]);
            }
            sxd_log_info(my_dev, "\n");
        }
        is_valid = 0;
    }

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "port_vtag_mode:\n");

    for (i = 0; i < (phy_port_max + 1); i++) { /* system port */
        for (j = 0; j < (SXD_MAX_VLAN_NUM); j++) {   /* vlan */
            is_valid += priv->port_vtag_mode[i][j];
        }

        if (is_valid) {
            sxd_log_info(my_dev, "sys_port 0x%x tagged on vlans: ", i);
            for (j = 0; j < (SXD_MAX_VLAN_NUM); j++) { /* vlan */
                if (priv->port_vtag_mode[i][j]) {
                    sxd_log_info(my_dev, "%d, ", j);
                }
            }
            sxd_log_info(my_dev, "\n");
        }
        is_valid = 0;
    }

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "lag_vtag_mode:\n");
    for (i = 0; i < (lag_max + 1); i++) { /* lid */
        for (j = 0; j < (SXD_MAX_VLAN_NUM); j++) { /* vlan */
            is_valid += priv->lag_vtag_mode[i][j];
        }

        if (is_valid) {
            sxd_log_info(my_dev, "lag 0x%x tagged on vlans: ", i);
            for (j = 0; j < (SXD_MAX_VLAN_NUM); j++) { /* vlan */
                if (priv->lag_vtag_mode[i][j]) {
                    sxd_log_info(my_dev, "%d, ", j);
                }
            }
            sxd_log_info(my_dev, "\n");
        }
        is_valid = 0;
    }

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "prio_tag_sysports:\n");
    for (i = 0; i < (phy_port_max + 1); i++) { /* system port */
        if (priv->port_prio_tagging_mode[i]) {
            sxd_log_info(my_dev, "0x%x, ", i);
        }
    }
    sxd_log_info(my_dev, "\n");


    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "prio_tag_lags:\n");
    for (i = 0; i < (lag_max + 1); i++) { /* lid */
        if (priv->lag_prio_tagging_mode[i]) {
            sxd_log_info(my_dev, "lag%d, ", i);
        }
    }
    sxd_log_info(my_dev, "\n");

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "vid2ip:\n");
    for (i = 0; i < (SXD_MAX_VLAN_NUM - 1); i++) { /* vlan */
        if (priv->icmp_vlan2ip_db[i]) {
            sxd_log_info(my_dev, "vlan%d: 0x%x, ", i, priv->icmp_vlan2ip_db[i]);
        }
    }
    sxd_log_info(my_dev, "\n");

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "port_rp_rif:\n");
    sxd_log_info(my_dev, "LOCAL\t| VLAN\t| RIF\t|\n");
    sxd_log_info(my_dev, "-------------------------\n");
    for (i = 0; i < (phy_port_max + 1); i++) {
        for (j = 0; j < SXD_MAX_VLAN_NUM; j++) {
            if (priv->port_rp_rif_valid[i][j]) {
                sxd_log_info(my_dev, "%u\t| %u\t| %u\t|\n", i, j, priv->port_rp_rif[i][j]);
            }
        }
    }

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "lag_rp_rif:\n");
    sxd_log_info(my_dev, "LID\t| VLAN\t| RIF\t|\n");
    sxd_log_info(my_dev, "-------------------------\n");
    for (i = 0; i < lag_max; i++) {
        for (j = 0; j < SXD_MAX_VLAN_NUM; j++) {
            if (priv->lag_rp_rif_valid[i][j]) {
                sxd_log_info(my_dev, "%u\t| %u\t| %u\t|\n", i, j, priv->lag_rp_rif[i][j]);
            }
        }
    }

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "port_vid_to_fid:\n");
    sxd_log_info(my_dev, "PORT\t| VLAN\t| FID\t|\n");
    sxd_log_info(my_dev, "-------------------------\n");
    for (i = 0; i < phy_port_max; i++) {
        for (j = 0; j < SXD_MAX_VLAN_NUM; j++) {
            if (priv->port_vid_to_fid[i][j]) {
                sxd_log_info(my_dev, "%u\t| %u\t| %u\t|\n", i, j, priv->port_vid_to_fid[i][j]);
            }
        }
    }

    sxd_log_info(my_dev, "============================\n");
    sxd_log_info(my_dev, "lag_oper_state:\n");
    sxd_log_info(my_dev, "LID\t| oper_s\n");
    sxd_log_info(my_dev, "------------------\n");
    for (i = 0; i < lag_max; i++) {
        if (priv->lag_oper_state[i]) {
            sxd_log_info(my_dev, "%u\t| UP\t|\n", i);
        } else {
            sxd_log_info(my_dev, "%u\t| DOWN\t|\n", i);
        }
    }
}


void __sx_proc_show_cq(struct sx_dev *dev, int cqn)
{
    sx_cq_show_cq(dev, cqn);
}

void __sx_proc_flush_rdq(struct sx_dev *my_dev, int idx)
{
    sx_cq_flush_rdq(my_dev, idx);
}
