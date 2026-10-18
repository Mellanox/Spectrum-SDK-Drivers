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

#include <linux/mlx_sx/driver.h>
#include <linux/mlx_sx/auto_registers/cmd_auto.h>
#include <linux/mlx_sx/auto_registers/reg.h>
#include <linux/mlx_sx/cmd.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include "sx_spice_wrapper.h"
#include "sx_spice_access_reg_auto.h"
#include "sx_spice_asic_ports.h"

/************************************************
 * Definitions                             *
 ***********************************************/

#define PORTS_PRINT_BUFF_SIZE (32)

/************************************************
 * Type definitions                             *
 ***********************************************/

typedef struct port_data {
    uint16_t            local_port;
    dev_private_data_t *dev_data;
    struct list_head    port_data_list;
} port_data_t;

/************************************************
* Static Functions declaration                  *
************************************************/

static ssize_t __sx_spice_access_port_status_read(struct file *filp, char *buf, size_t lbuf, loff_t *ppos);

static ssize_t __sx_spice_access_port_mapping_read(struct file *filp, char *buf, size_t lbuf, loff_t *ppos);

static ssize_t __sx_spice_access_port_phy_bubble_supported_read(struct file *filp, char *buf, size_t lbuf,
                                                                loff_t *ppos);

static ssize_t __sx_spice_access_port_phy_bubble_status_read(struct file *filp, char *buf, size_t lbuf, loff_t *ppos);

static ssize_t __sx_spice_access_port_phy_neg_status_read(struct file *filp, char *buf, size_t lbuf, loff_t *ppos);

static ssize_t __sx_spice_access_port_phy_remote_dev_read(struct file *filp, char *buf, size_t lbuf, loff_t *ppos);

static ssize_t __sx_spice_access_port_phy_xlpn_valid_read(struct file *filp, char *buf, size_t lbuf, loff_t *ppos);

static ssize_t __sx_spice_access_port_phy_xlpn_rev_read(struct file *filp, char *buf, size_t lbuf, loff_t *ppos);

static ssize_t __sx_spice_access_port_phy_xlpn_p12_read(struct file *filp, char *buf, size_t lbuf, loff_t *ppos);


/************************************************
 *  Local variables
 ***********************************************/
static const struct file_operations port_status_fops = {
    .read = __sx_spice_access_port_status_read
};

static const struct file_operations port_mapping_fops = {
    .read = __sx_spice_access_port_mapping_read
};

static const struct file_operations port_phy_bubble_supported_fops = {
    .read = __sx_spice_access_port_phy_bubble_supported_read
};

static const struct file_operations port_phy_bubble_status_fops = {
    .read = __sx_spice_access_port_phy_bubble_status_read
};

static const struct file_operations port_phy_neg_status_fops = {
    .read = __sx_spice_access_port_phy_neg_status_read
};

static const struct file_operations port_phy_xlpn_valid_fops = {
    .read = __sx_spice_access_port_phy_xlpn_valid_read
};

static const struct file_operations port_phy_xlpn_rev_fops = {
    .read = __sx_spice_access_port_phy_xlpn_rev_read
};

static const struct file_operations port_phy_xlpn_p12_fops = {
    .read = __sx_spice_access_port_phy_xlpn_p12_read
};
static const struct file_operations port_phy_remote_dev_fops = {
    .read = __sx_spice_access_port_phy_remote_dev_read
};

/************************************************
 * Global variables                             *
 ***********************************************/


/************************************************
 * Static Functions definition                  *
 ***********************************************/


/************************************************
 *                     API                      *
 ***********************************************/
bool sx_spice_is_local_port_in_range(dev_private_data_t *dev_data, uint16_t local_port)
{
    return dev_data && dev_data->dev && (local_port != 0) && (local_port <= dev_data->max_local_port);
}

static bool __sx_spice_local_port_pmlp_valid_get(dev_private_data_t *dev_data, uint16_t local_port)
{
    /* One-time PMLP sweep to build the validity bitmap at tree-create time */

    struct ku_access_pmlp_reg reg_data;
    int                       err;
    u8                        fw_status = 0;

    if (!sx_spice_is_local_port_in_range(dev_data, local_port)) {
        return false;
    }

    memset(&reg_data, 0, sizeof(reg_data));
    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data.pmlp_reg.local_port, reg_data.pmlp_reg.lp_msb, local_port);
    reg_data.dev_id = dev_data->dev->device_id;
    sx_cmd_set_op_tlv(&reg_data.op_tlv, PMLP_REG_ID, 1);

    err = sx_ACCESS_REG_PMLP(dev_data->dev, &reg_data);
    fw_status = sxd_emad_operation_tlv_status_get(&reg_data.op_tlv);

    if (err || fw_status) {
        return false;
    }

    return reg_data.pmlp_reg.width != 0;
}

bool sx_spice_local_port_emad_allowed(dev_private_data_t *dev_data, uint16_t local_port)
{
    if (!sx_spice_is_local_port_in_range(dev_data, local_port)) {
        return false;
    }

    return test_bit(local_port, dev_data->valid_local_ports_bitmap);
}

void sx_spice_local_port_validity_set(dev_private_data_t *dev_data, uint16_t local_port, bool is_valid)
{
    if (!sx_spice_is_local_port_in_range(dev_data, local_port)) {
        return;
    }

    if (is_valid) {
        set_bit(local_port, dev_data->valid_local_ports_bitmap);
    } else {
        clear_bit(local_port, dev_data->valid_local_ports_bitmap);
    }
}

int sx_spice_local_port_validity_bitmap_init(dev_private_data_t *dev_data)
{
    uint16_t local_port;
    bool     is_valid;

    if (!dev_data || !dev_data->dev || (dev_data->max_local_port == 0)) {
        return -EINVAL;
    }

    bitmap_zero(dev_data->valid_local_ports_bitmap, MAX_PHYPORT_NUM + 1);
    for (local_port = 1; local_port <= dev_data->max_local_port; local_port++) {
        is_valid = __sx_spice_local_port_pmlp_valid_get(dev_data, local_port);
        sx_spice_local_port_validity_set(dev_data,
                                         local_port,
                                         is_valid);
    }

    return 0;
}

static ssize_t __sx_spice_access_port_status_read(struct file *filp, char *buf, size_t lbuf, loff_t *ppos)
{
    char               kbuffer[PORTS_PRINT_BUFF_SIZE] = {0};
    int                err = 0;
    ssize_t            size = 0;
    ssize_t            print_data_size = 0;
    struct ku_paos_reg reg_data;
    const port_data_t *port_data_p = (port_data_t*)filp->f_inode->i_private;

    if (*ppos != 0) {
        return 0;
    }

    memset(&reg_data, 0, sizeof(reg_data));
    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data.local_port, reg_data.lp_msb, port_data_p->local_port);

    err = SX_SPICE_EMAD_ACCESS_REG_LOCAL_PORT(PAOS, port_data_p->dev_data, &reg_data, port_data_p->local_port);
    if (err) {
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "N/A\n");
        size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);
        goto out;
    }

    if (reg_data.oper_status == 1) {
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "Up\n");
    } else {
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "Down\n");
    }

    size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);

out:
    return size;
}

static ssize_t __sx_spice_access_port_mapping_read(struct file *filp, char *buf, size_t lbuf, loff_t *ppos)
{
    char                      kbuffer[PORTS_PRINT_BUFF_SIZE] = {0};
    int                       err = 0;
    ssize_t                   size = 0;
    ssize_t                   print_data_size = 0;
    struct ku_access_pmlp_reg reg_data;
    const port_data_t        *port_data_p = (port_data_t*)filp->f_inode->i_private;

    if (*ppos != 0) {
        return 0;
    }

    memset(&reg_data, 0, sizeof(reg_data));

    /* Note PMLP register is called via command interface and not via
     * the regular SPICE. This is due to extra logic in the sxd_access_reg_pmlp
     * that prevents us from using auto_reg.
     *
     * File purpose: report Mapped/Unmapped status, so intentionally we skip checking
     * calling sx_spice_local_port_emad_allowed() - would hide "Unmapped" and show "N/A",
     * defeating its purpose.
     */
    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data.pmlp_reg.local_port, reg_data.pmlp_reg.lp_msb,
                                        port_data_p->local_port);
    reg_data.dev_id = port_data_p->dev_data->dev->device_id;
    sx_cmd_set_op_tlv(&reg_data.op_tlv, PMLP_REG_ID, 1);

    err = sx_ACCESS_REG_PMLP(port_data_p->dev_data->dev, &reg_data);
    if (err) {
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "N/A\n");
        size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);
        goto out;
    }

    if (reg_data.pmlp_reg.width != 0) {
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "Mapped\n");
    } else {
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "Unmapped\n");
    }

    size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);

out:
    return size;
}

static ssize_t __sx_spice_access_port_phy_bubble_supported_read(struct file *filp, char *buf, size_t lbuf,
                                                                loff_t *ppos)
{
    char               kbuffer[PORTS_PRINT_BUFF_SIZE] = {0};
    int                err = 0;
    ssize_t            size = 0;
    ssize_t            print_data_size = 0;
    struct ku_pddr_reg reg_data;
    const port_data_t *port_data_p = (port_data_t*)filp->f_inode->i_private;

    if (*ppos != 0) {
        return 0;
    }

    memset(&reg_data, 0, sizeof(reg_data));
    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data.local_port, reg_data.lp_msb, port_data_p->local_port);
    reg_data.page_select = SXD_PDDR_PAGE_SELECT_LINK_PARTNER_INFO_PAGE_E;

    err = SX_SPICE_EMAD_ACCESS_REG_LOCAL_PORT(PDDR, port_data_p->dev_data, &reg_data, port_data_p->local_port);
    if (err) {
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "N/A\n");
        size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);
        goto out;
    }

    if (reg_data.page_data.pddr_link_partner_info.info_supported_mask &
        SXD_PDDR_INFO_SUPPORTED_MASK_BUBBLE_AGREEMENT_SUPPORTED_E) {
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "Supported\n");
    } else {
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "Unsupported\n");
    }

    size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);

out:
    return size;
}

static ssize_t __sx_spice_access_port_phy_bubble_status_read(struct file *filp, char *buf, size_t lbuf, loff_t *ppos)
{
    char               kbuffer[PORTS_PRINT_BUFF_SIZE] = {0};
    int                err = 0;
    ssize_t            size = 0;
    ssize_t            print_data_size = 0;
    struct ku_pddr_reg reg_data;
    const port_data_t *port_data_p = (port_data_t*)filp->f_inode->i_private;

    if (*ppos != 0) {
        return 0;
    }
    memset(&reg_data, 0, sizeof(reg_data));
    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data.local_port, reg_data.lp_msb, port_data_p->local_port);
    reg_data.page_select = SXD_PDDR_PAGE_SELECT_LINK_PARTNER_INFO_PAGE_E;

    err = SX_SPICE_EMAD_ACCESS_REG_LOCAL_PORT(PDDR, port_data_p->dev_data, &reg_data, port_data_p->local_port);
    if (err) {
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "N/A\n");
        size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);
        goto out;
    }

    switch (reg_data.page_data.pddr_link_partner_info.bubble_agreement) {
    case 0:
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "No negotiation\n");
        break;

    case 1:
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "Bubble agreed\n");
        break;

    case 2:
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "Bubble not agreed\n");
        break;

    default:
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "N/A\n");
        break;
    }

    size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);

out:
    return size;
}

static ssize_t __sx_spice_access_port_phy_neg_status_read(struct file *filp, char *buf, size_t lbuf, loff_t *ppos)
{
    char               kbuffer[PORTS_PRINT_BUFF_SIZE] = {0};
    int                err = 0;
    ssize_t            size = 0;
    ssize_t            print_data_size = 0;
    struct ku_pddr_reg reg_data;
    const port_data_t *port_data_p = (port_data_t*)filp->f_inode->i_private;

    if (*ppos != 0) {
        return 0;
    }
    memset(&reg_data, 0, sizeof(reg_data));
    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data.local_port, reg_data.lp_msb, port_data_p->local_port);
    reg_data.page_select = SXD_PDDR_PAGE_SELECT_OPERATIONAL_INFO_PAGE_E;

    err = SX_SPICE_EMAD_ACCESS_REG_LOCAL_PORT(PDDR, port_data_p->dev_data, &reg_data, port_data_p->local_port);
    if (err) {
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "N/A\n");
        size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);
        goto out;
    }
    /* To do check if need to verify the bit 5/6/8/9 are set if so print num otherwise print error?*/

    print_data_size = snprintf(kbuffer,
                               PORTS_PRINT_BUFF_SIZE,
                               " %u\n",
                               reg_data.page_data.pddr_operation_info_page.neg_mode_active);

    size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);

out:
    return size;
}

static ssize_t __sx_spice_access_port_phy_remote_dev_read(struct file *filp, char *buf, size_t lbuf, loff_t *ppos)
{
    char               kbuffer[PORTS_PRINT_BUFF_SIZE] = {0};
    int                err = 0;
    ssize_t            size = 0;
    ssize_t            print_data_size = 0;
    struct ku_pddr_reg reg_data;
    const port_data_t *port_data_p = (port_data_t*)filp->f_inode->i_private;

    if (*ppos != 0) {
        return 0;
    }
    memset(&reg_data, 0, sizeof(reg_data));
    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data.local_port, reg_data.lp_msb, port_data_p->local_port);
    reg_data.page_select = SXD_PDDR_PAGE_SELECT_OPERATIONAL_INFO_PAGE_E;

    err = SX_SPICE_EMAD_ACCESS_REG_LOCAL_PORT(PDDR, port_data_p->dev_data, &reg_data, port_data_p->local_port);
    if (err) {
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "N/A\n");
        size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);
        goto out;
    }

    if (reg_data.page_data.pddr_operation_info_page.neg_mode_active == 0) {
        /* This error is returned according to the spec */
        size = -EFAULT;
        goto out;
    }

    memset(&reg_data, 0, sizeof(reg_data));
    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data.local_port, reg_data.lp_msb, port_data_p->local_port);
    reg_data.page_select = SXD_PDDR_PAGE_SELECT_PHY_INFO_PAGE_E;

    err = SX_SPICE_EMAD_ACCESS_REG_LOCAL_PORT(PDDR, port_data_p->dev_data, &reg_data, port_data_p->local_port);
    if (err) {
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "N/A\n");
        size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);
        goto out;
    }
    print_data_size = snprintf(kbuffer,
                               PORTS_PRINT_BUFF_SIZE,
                               " %u\n",
                               reg_data.page_data.pddr_phy_info_page.remote_device_type);
    size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);

out:
    return size;
}

static ssize_t __sx_spice_access_port_phy_xlpn_rev_read(struct file *filp, char *buf, size_t lbuf, loff_t *ppos)
{
    char                 kbuffer[PORTS_PRINT_BUFF_SIZE] = {0};
    int                  err = 0;
    ssize_t              size = 0;
    ssize_t              print_data_size = 0;
    struct ku_pnlpnr_reg reg_data;
    const port_data_t   *port_data_p = (port_data_t*)filp->f_inode->i_private;

    if (*ppos != 0) {
        return 0;
    }
    memset(&reg_data, 0, sizeof(reg_data));
    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data.local_port, reg_data.lp_msb, port_data_p->local_port);

    err = SX_SPICE_EMAD_ACCESS_REG_LOCAL_PORT(PNLPNR, port_data_p->dev_data, &reg_data, port_data_p->local_port);
    if (err) {
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "N/A\n");
        size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);
        goto out;
    }

    if (reg_data.nlpn_v == 0) {
        ;
        /* This error is returned according to the spec */
        size = -EFAULT;
        goto out;
    }

    print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, " %u \n", reg_data.nlpn_rev);
    size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);

out:
    return size;
}
static ssize_t __sx_spice_access_port_phy_xlpn_valid_read(struct file *filp, char *buf, size_t lbuf, loff_t *ppos)
{
    char                 kbuffer[PORTS_PRINT_BUFF_SIZE] = {0};
    int                  err = 0;
    ssize_t              size = 0;
    ssize_t              print_data_size = 0;
    struct ku_pnlpnr_reg reg_data;
    const port_data_t   *port_data_p = (port_data_t*)filp->f_inode->i_private;

    if (*ppos != 0) {
        return 0;
    }
    memset(&reg_data, 0, sizeof(reg_data));
    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data.local_port, reg_data.lp_msb, port_data_p->local_port);

    err = SX_SPICE_EMAD_ACCESS_REG_LOCAL_PORT(PNLPNR, port_data_p->dev_data, &reg_data, port_data_p->local_port);
    if (err) {
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "N/A\n");
        size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);
        goto out;
    }

    if (reg_data.link_up == 0) {
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "0\n");
        size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);
        goto out;
    }

    print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, " %u\n", reg_data.nlpn_rev);
    size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);

out:
    return size;
}

static ssize_t __sx_spice_access_port_phy_xlpn_p12_read(struct file *filp, char *buf, size_t lbuf, loff_t *ppos)
{
    char                 kbuffer[PORTS_PRINT_BUFF_SIZE] = {0};
    int                  err = 0;
    ssize_t              size = 0;
    ssize_t              print_data_size = 0;
    struct ku_pnlpnr_reg reg_data;
    const port_data_t   *port_data_p = (port_data_t*)filp->f_inode->i_private;

    if (*ppos != 0) {
        return 0;
    }
    memset(&reg_data, 0, sizeof(reg_data));
    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data.local_port, reg_data.lp_msb, port_data_p->local_port);
    reg_data.page_req = (1 << 12);

    err = SX_SPICE_EMAD_ACCESS_REG_LOCAL_PORT(PNLPNR, port_data_p->dev_data, &reg_data, port_data_p->local_port);
    if (err) {
        print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, "N/A\n");
        size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);
        goto out;
    }

    if ((reg_data.nlpn_v == 0) || ((reg_data.nlpn_v == 1) && !(reg_data.page_valid & (1 << 12)))) {
        /* This error is returned according to the spec */
        size = -EFAULT;
        goto out;
    }

    print_data_size = snprintf(kbuffer, PORTS_PRINT_BUFF_SIZE, " 0x%x\n", reg_data.pages[12]);
    size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);

out:
    return size;
}

static int sx_spice_port_status_file_create(struct dentry *parent, port_data_t *port_data_p)
{
    struct dentry * file_p = NULL;

    file_p = debugfs_create_file("oper_status", 0644, parent, (void *)port_data_p, &port_status_fops);
    if (file_p == NULL) {
        sxd_log_err(SXD_DEV_FROM_DEVDATA_CONT(port_data_p),
                    "Failed to create file port operation status for SPICE\n");
        return -EACCES;
    }

    return 0;
}

static int sx_spice_port_mapping_file_create(struct dentry *parent, port_data_t *port_data_p)
{
    struct dentry * file_p = NULL;

    file_p = debugfs_create_file("mapping", 0644, parent, (void *)port_data_p, &port_mapping_fops);
    if (file_p == NULL) {
        sxd_log_err(SXD_DEV_FROM_DEVDATA_CONT(port_data_p), "Failed to create file port mapping for SPICE\n");
        return -EACCES;
    }

    return 0;
}


static int sx_spice_port_phy_files_create(struct dentry *parent, port_data_t *port_data_p)
{
    struct dentry * file_p = NULL;

    /* The bubble info is available only for Spectrum 4 and above. */
    if ((port_data_p->dev_data->hw_device_id == SXD_MGIR_HW_DEV_ID_SPECTRUM4) ||
        (port_data_p->dev_data->hw_device_id == SXD_MGIR_HW_DEV_ID_SPECTRUM5) ||
        (port_data_p->dev_data->hw_device_id == SXD_MGIR_HW_DEV_ID_SPECTRUM6) ||
        (port_data_p->dev_data->hw_device_id == SXD_MGIR_HW_DEV_ID_SPECTRUM7)) {
        file_p = debugfs_create_file("bubble_supported",
                                     0644,
                                     parent,
                                     (void *)port_data_p,
                                     &port_phy_bubble_supported_fops);
        if (file_p == NULL) {
            sxd_log_err(SXD_DEV_FROM_DEVDATA_CONT(port_data_p),
                        "Failed to create file bubble supported for SPICE\n");
            return -EACCES;
        }

        file_p = debugfs_create_file("bubble_status", 0644, parent, (void *)port_data_p, &port_phy_bubble_status_fops);
        if (file_p == NULL) {
            sxd_log_err(SXD_DEV_FROM_DEVDATA_CONT(port_data_p),
                        "Failed to create file bubble status for SPICE\n");
            return -EACCES;
        }

        file_p = debugfs_create_file("neg_status", 0644, parent, (void *)port_data_p, &port_phy_neg_status_fops);
        if (file_p == NULL) {
            sxd_log_err(SXD_DEV_FROM_DEVDATA_CONT(port_data_p),
                        "Failed to create file negotiated protocol status for SPICE\n");
            return -EACCES;
        }

        file_p = debugfs_create_file("remote_dev", 0644, parent, (void *)port_data_p, &port_phy_remote_dev_fops);
        if (file_p == NULL) {
            sxd_log_err(SXD_DEV_FROM_DEVDATA_CONT(port_data_p),
                        "Failed to create file negotiated protocol status for SPICE\n");
            return -EACCES;
        }

        file_p = debugfs_create_file("xlpn_valid", 0644, parent, (void *)port_data_p, &port_phy_xlpn_valid_fops);
        if (file_p == NULL) {
            sxd_log_err(SXD_DEV_FROM_DEVDATA_CONT(port_data_p),
                        "Failed to create file nxlpn_valid for SPICE\n");
            return -EACCES;
        }
        file_p = debugfs_create_file("xlpn_rev", 0644, parent, (void *)port_data_p, &port_phy_xlpn_rev_fops);
        if (file_p == NULL) {
            sxd_log_err(SXD_DEV_FROM_DEVDATA_CONT(port_data_p), "Failed to create file xlpn_rev for SPICE\n");
            return -EACCES;
        }
        file_p = debugfs_create_file("xlpn_p12", 0644, parent, (void *)port_data_p, &port_phy_xlpn_p12_fops);
        if (file_p == NULL) {
            sxd_log_err(SXD_DEV_FROM_DEVDATA_CONT(port_data_p), "Failed to create file xlpn_p12 for SPICE\n");
            return -EACCES;
        }
    }

    return 0;
}

static void __port_data_local_port_free(dev_private_data_t *dev_data, uint16_t local_port)
{
    port_data_t *iter_port_data;

    list_for_each_entry(iter_port_data, &dev_data->port_data_list, port_data_list) {
        if (iter_port_data->local_port == local_port) {
            list_del(&iter_port_data->port_data_list);
            kfree(iter_port_data);
            break;
        }
    }
}

static int sx_spice_asic_ports_local_port_create(dev_private_data_t *dev_data, uint16_t local_port)
{
    int             err = 0;
    char            dname[60] = { 0 };
    struct dentry * port_dir_p = NULL;
    struct dentry * phy_p = NULL;
    port_data_t   * port_data_p = NULL;

    if ((dev_data == NULL) || (dev_data->dev == NULL) ||
        (dev_data->asic_counters_ports_root_inode_p == NULL)) {
        err = -EINVAL;
        goto err_out;
    }

    snprintf(dname, sizeof(dname), "local_port.%d", local_port);
    port_dir_p = debugfs_create_dir(dname, dev_data->asic_counters_ports_root_inode_p);
    if (port_dir_p == NULL) {
        err = -EACCES;
        goto err_out;
    }

    port_data_p = kzalloc(sizeof(port_data_t), GFP_KERNEL);
    if (port_data_p == NULL) {
        sxd_log_err(dev_data->dev, "Failed to allocate port_data for SPICE\n");
        err = -ENOMEM;
        goto err_out;
    }

    port_data_p->dev_data = dev_data;
    port_data_p->local_port = local_port;
    list_add_tail(&port_data_p->port_data_list, &dev_data->port_data_list);

    err = sx_spice_port_status_file_create(port_dir_p, port_data_p);
    if (err != 0) {
        sxd_log_err(dev_data->dev, "Failed to create SPICE port status\n");
        goto err_out;
    }

    err = sx_spice_port_mapping_file_create(port_dir_p, port_data_p);
    if (err != 0) {
        sxd_log_err(dev_data->dev, "Failed to create SPICE port mapping\n");
        goto err_out;
    }

    phy_p = debugfs_create_dir("phy", port_dir_p);
    if (phy_p == NULL) {
        err = -EACCES;
        goto err_out;
    }

    err = sx_spice_port_phy_files_create(phy_p, port_data_p);
    if (err != 0) {
        sxd_log_err(dev_data->dev, "Failed to create SPICE phy\n");
        goto err_out;
    }

    return 0;

err_out:
    /* Remove the debugfs subtree before freeing port_data_p: the files created above hold a raw
     * pointer to it as i_private, so debugfs entries must go first. */
    debugfs_remove_recursive(port_dir_p);
    __port_data_local_port_free(dev_data, local_port);
    return err;
}

int sx_spice_access_reg_asic_ports_file_create(struct dentry *parent, dev_private_data_t *dev_data)
{
    int             err = 0;
    struct dentry * ports_p = NULL;
    int             local_port_it = 0;

    sxd_log_info(dev_data->dev, "sx_spice_tree_asic_ports_create\n");

    ports_p = debugfs_create_dir("ports", parent);
    if (ports_p == NULL) {
        return -EACCES;
    }
    dev_data->asic_counters_ports_root_inode_p = ports_p;

    for (local_port_it = 1; local_port_it <= dev_data->max_local_port; local_port_it++) {
        err = sx_spice_asic_ports_local_port_create(dev_data, local_port_it);
        if (err != 0) {
            debugfs_remove_recursive(ports_p);
            dev_data->asic_counters_ports_root_inode_p = NULL;
            return err;
        }
    }

    return 0;
}

void sx_spice_asic_ports_data_list_free(dev_private_data_t *priv_data)
{
    port_data_t *iter_port_data, *tmp_port_data;

    list_for_each_entry_safe(iter_port_data, tmp_port_data, &priv_data->port_data_list, port_data_list) {
        list_del(&iter_port_data->port_data_list);
        kfree(iter_port_data);
    }
}
