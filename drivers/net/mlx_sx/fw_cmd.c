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

#include "fw.h"
#include "sx.h"
#include "sgmii.h"
#include "cq.h"
#include "fw_internal.h"
#include "dev_init.h"
#include <linux/mlx_sx/cmd.h>
#include <linux/mlx_sx/driver.h>

static void get_board_id(void *vsd, char *board_id)
{
#define VSD_OFFSET_SX_BOARD_ID 0xd0

    memset(board_id, 0, SX_BOARD_ID_LEN);
    memcpy(board_id, vsd + VSD_OFFSET_SX_BOARD_ID, SX_PSID_SIZE);
}


int sx_QUERY_RSRC(struct sx_dev *dev, struct ku_query_rsrc* query_rsrc)
{
    struct sx_cmd_mailbox *mailbox;
    u32                   *outbox;
    u16                    resource_id;
    u64                    resource_val;
    int                    err = 0;
    int                    index, i;

#define MLXSW_CMD_QUERY_RESOURCES_TABLE_END_ID 0xffff
#define MLXSW_CMD_QUERY_RESOURCES_MAX_QUERIES  100
#define MLXSW_CMD_QUERY_RESOURCES_PER_QUERY    32
#define QUERY_RSRC_RESOURCE_ID_OFFSET          0x00 /* index * rsrc entry + 2 */
#define QUERY_RSRC_RESOURCE_VAL_OFFSET         0x00 /* pull the 64 bits and extract the 40 bits*/
#define QUERY_RSRC_RESOURCE_ENTRY_SIZE         0x08
#define QUERY_RSRC_RESOURCE_VAL_WIDTH_BITS     40

    mailbox = sx_alloc_cmd_mailbox(dev, dev->device_id);
    if (IS_ERR(mailbox)) {
        return PTR_ERR(mailbox);
    }

    for (index = 0; index < MLXSW_CMD_QUERY_RESOURCES_MAX_QUERIES;
         index++) {
        memset(mailbox->buf, 0, SX_MAILBOX_SIZE);
        outbox = mailbox->buf;

        err = sx_cmd_box(dev, dev->device_id, 0, mailbox, index, 0,
                         SX_CMD_QUERY_RSRC, SX_CMD_TIME_CLASS_A, 0, 0);
        if (err) {
            goto out;
        }

        for (i = 0; i < MLXSW_CMD_QUERY_RESOURCES_PER_QUERY; i++) {
            SX_GET(resource_id, outbox,
                   (i * QUERY_RSRC_RESOURCE_ENTRY_SIZE) + QUERY_RSRC_RESOURCE_ID_OFFSET);
            if (resource_id == MLXSW_CMD_QUERY_RESOURCES_TABLE_END_ID) {
                goto out;
            }

            if (resource_id == query_rsrc->rsrc_id) {
                SX_GET(resource_val, outbox, (i * QUERY_RSRC_RESOURCE_ENTRY_SIZE) + QUERY_RSRC_RESOURCE_VAL_OFFSET);
                query_rsrc->rsrc_val = ((1ull << QUERY_RSRC_RESOURCE_VAL_WIDTH_BITS) - 1) & resource_val;
                goto out;
            }
        }
    }

out:
    sx_free_cmd_mailbox(dev, mailbox);
    return err;
}
EXPORT_SYMBOL(sx_QUERY_RSRC);

static void sx_fw_2_ku_query_fw(struct sx_priv *priv, const struct sx_fw *fw, struct ku_query_fw *query_fw)
{
    memset(query_fw, 0, sizeof(*query_fw));

    query_fw->core_clk = fw->core_clock;
    query_fw->dt = fw->debug_trace;
    query_fw->smkey = fw->smkey;
    query_fw->fw_year = fw->fw_year;
    query_fw->fw_month = fw->fw_month;
    query_fw->fw_day = fw->fw_day;
    query_fw->fw_hour = fw->fw_hour;
    query_fw->fw_minutes = fw->fw_minutes;
    query_fw->fw_seconds = fw->fw_seconds;
    query_fw->fw_cap_mod_support = SX_GET_FW_CAP(priv, SX_FW_CAP_MOD_SUPPORT_MASK, SX_FW_CAP_MOD_SUPPORT_OFFSET);
    query_fw->fw_rev = fw->fw_ver;
    query_fw->frc_offset = fw->frc_offset;
    query_fw->frc_bar = fw->frc_bar;
    query_fw->utc_sec_offset = fw->utc_sec_offset;
    query_fw->utc_sec_bar = fw->utc_sec_bar;
    query_fw->utc_nsec_offset = fw->utc_nsec_offset;
    query_fw->utc_nsec_bar = fw->utc_nsec_bar;
    query_fw->cr_dump_offset = fw->cr_dump_offset;
    query_fw->cr_dump_bar = fw->cr_dump_bar;
    query_fw->cap_dump_host_size_flat = fw->cap_dump_host_size_flat;
    query_fw->cap_dump_host_size_reduced_flat = fw->cap_dump_host_size_reduced_flat;
    query_fw->cap_dump_host_size_gw = fw->cap_dump_host_size_gw;
    query_fw->cap_dump_host_size_gdb = fw->cap_dump_host_size_gdb;
}

#define QUERY_FW_IN_MB_SIZE 0x5c
int sx_QUERY_FW(struct sx_dev *dev, struct ku_query_fw* query_fw)
{
    struct sx_priv        *priv = sx_priv(dev);
    struct sx_fw           fw;
    struct sx_cmd         *cmd = &priv->cmd;
    struct sx_cmd_mailbox *mailbox;
    u32                   *outbox;
    int                    err = 0, i;
    sxd_dev_id_t           target_dev_id;

    memset(&fw, 0, sizeof(fw));

    fw.local_in_mb_size = 0;
    fw.local_out_mb_offset = 0;
    fw.local_in_mb_offset = 0;
    fw.local_out_mb_size = 0;

    for (i = 0; i < ARRAY_SIZE(fw.map_fa_items); i++) {
        fw.map_fa_items[i] = SX_MAP_FA_INVALID_HANDLE;
    }

    if (NULL == query_fw) {
        target_dev_id = dev->device_id;
    } else {
        target_dev_id = query_fw->dev_id;
    }

#define QUERY_FW_VER_OFFSET                  0x00
#define QUERY_FW_CORE_CLOCK_OFFSET           0x08
#define QUERY_FW_DEBUG_TRACE_OFFSET          0x0c
#define QUERY_FW_SMKEY_OFFSET                0x0c
#define QUERY_FW_FW_HOUR_OFFSET              0x10
#define QUERY_FW_FW_MINUTES_OFFSET           0x11
#define QUERY_FW_FW_SECONDS_OFFSET           0x12
#define QUERY_FW_FW_YEAR_OFFSET              0x14
#define QUERY_FW_FW_MONTH_OFFSET             0x16
#define QUERY_FW_FW_DAY_OFFSET               0x17
#define QUERY_FW_CAP_OFFSET                  0x18
#define QUERY_FW_ERR_START_OFFSET            0x30
#define QUERY_FW_ERR_SIZE_OFFSET             0x38
#define QUERY_FW_ERR_BAR_OFFSET              0x3c
#define QUERY_FW_CLR_INT_BASE_OFFSET         0x20
#define QUERY_FW_CLR_INT_BAR_OFFSET          0x28
#define QUERY_FW_DB_PAGE_OFFSET_OFFSET       0x40
#define QUERY_FW_DB_PAGE_BAR_OFFSET          0x48
#define QUERY_FW_FRC_OFFSET                  0x50
#define QUERY_FW_FRC_BAR_OFFSET              0x58
#define QUERY_FW_UTC_SEC_OFFSET              0x70
#define QUERY_FW_UTC_SEC_BAR_OFFSET          0x78
#define QUERY_FW_UTC_NSEC_OFFSET             0x80
#define QUERY_FW_UTC_NSEC_BAR_OFFSET         0x88
#define QUERY_FW_CR_DUMP_OFFSET              0x90
#define QUERY_FW_CR_DUMP_BAR_OFFSET          0x98
#define QUERY_FW_DUMP_HOST_SIZE_FLAT         0x9c
#define QUERY_FW_DUMP_HOST_SIZE_REDUCED_FLAT 0xb4
#define QUERY_FW_DUMP_HOST_SIZE_GW           0xa0
#define QUERY_FW_DUMP_HOST_SIZE_GDB          0xa4
#define QUERY_FW_SIZE_OFFSET                 0x00

    mailbox = sx_alloc_cmd_mailbox(dev, target_dev_id);
    if (IS_ERR(mailbox)) {
        return PTR_ERR(mailbox);
    }
    outbox = mailbox->buf;
    err = sx_cmd_box(dev, target_dev_id, 0, mailbox, 0, 0,
                     SX_CMD_QUERY_FW, SX_CMD_TIME_CLASS_A,
                     QUERY_FW_IN_MB_SIZE, 0);
    if (err) {
        goto out;
    }

    SX_GET(fw.fw_pages, outbox, QUERY_FW_SIZE_OFFSET);
    SX_GET(fw.fw_ver,   outbox, QUERY_FW_VER_OFFSET);
    /*
     * FW subminor version is at more significant bits than minor
     * version, so swap here.
     */
    fw.fw_ver = (fw.fw_ver & 0xffff00000000ull) |
                ((fw.fw_ver & 0xffff0000ull) >> 16) |
                ((fw.fw_ver & 0x0000ffffull) << 16);
    dev->fw_ver = fw.fw_ver;
    SX_GET(fw.core_clock,  outbox, QUERY_FW_CORE_CLOCK_OFFSET);
    SX_GET(fw.debug_trace, outbox, QUERY_FW_DEBUG_TRACE_OFFSET);
    fw.debug_trace = fw.debug_trace >> 7;
    SX_GET(fw.smkey, outbox, QUERY_FW_SMKEY_OFFSET);
    fw.smkey = (fw.smkey >> 6) & 1;
    cmd->max_cmds = 1;
    SX_GET(fw.catas_offset, outbox, QUERY_FW_ERR_START_OFFSET);
    SX_GET(fw.catas_size,   outbox, QUERY_FW_ERR_SIZE_OFFSET);
    SX_GET(fw.catas_bar,    outbox, QUERY_FW_ERR_BAR_OFFSET);
    fw.catas_bar = (fw.catas_bar >> 6) * 2;
    SX_GET(fw.clr_int_base, outbox, QUERY_FW_CLR_INT_BASE_OFFSET);
    SX_GET(fw.clr_int_bar,  outbox, QUERY_FW_CLR_INT_BAR_OFFSET);
    fw.clr_int_bar = (fw.clr_int_bar >> 6) * 2;
    SX_GET(fw.doorbell_page_offset, outbox,
           QUERY_FW_DB_PAGE_OFFSET_OFFSET);
    SX_GET(fw.doorbell_page_bar,    outbox, QUERY_FW_DB_PAGE_BAR_OFFSET);
    SX_GET(fw.frc_offset, outbox, QUERY_FW_FRC_OFFSET);
    SX_GET(fw.frc_bar,    outbox, QUERY_FW_FRC_BAR_OFFSET);
    SX_GET(fw.utc_sec_offset, outbox, QUERY_FW_UTC_SEC_OFFSET);
    SX_GET(fw.utc_sec_bar, outbox, QUERY_FW_UTC_SEC_BAR_OFFSET);
    SX_GET(fw.utc_nsec_offset, outbox, QUERY_FW_UTC_NSEC_OFFSET);
    SX_GET(fw.utc_nsec_bar, outbox, QUERY_FW_UTC_NSEC_BAR_OFFSET);
    SX_GET(fw.cr_dump_offset, outbox, QUERY_FW_CR_DUMP_OFFSET);
    SX_GET(fw.cr_dump_bar, outbox, QUERY_FW_CR_DUMP_BAR_OFFSET);
    SX_GET(fw.fw_hour,    outbox, QUERY_FW_FW_HOUR_OFFSET);
    SX_GET(fw.fw_minutes, outbox, QUERY_FW_FW_MINUTES_OFFSET);
    SX_GET(fw.fw_seconds, outbox, QUERY_FW_FW_SECONDS_OFFSET);
    SX_GET(fw.fw_year,    outbox, QUERY_FW_FW_YEAR_OFFSET);
    SX_GET(fw.fw_month,   outbox, QUERY_FW_FW_MONTH_OFFSET);
    SX_GET(fw.fw_day,     outbox, QUERY_FW_FW_DAY_OFFSET);
    SX_GET(fw.fw_capabilities, outbox, QUERY_FW_CAP_OFFSET);
    SX_GET(fw.cap_dump_host_size_flat, outbox, QUERY_FW_DUMP_HOST_SIZE_FLAT);
    SX_GET(fw.cap_dump_host_size_reduced_flat, outbox, QUERY_FW_DUMP_HOST_SIZE_REDUCED_FLAT);
    SX_GET(fw.cap_dump_host_size_gw,   outbox, QUERY_FW_DUMP_HOST_SIZE_GW);
    SX_GET(fw.cap_dump_host_size_gdb,  outbox, QUERY_FW_DUMP_HOST_SIZE_GDB);

    if (NULL != query_fw) {
        sx_fw_2_ku_query_fw(priv, &fw, query_fw);
    }

    if ((!priv->is_fw_initialized) &&
        ((DPT_PATH_PCI_E == sx_glb.sx_dpt.dpt_info[target_dev_id].cmd_path) ||
         is_sgmii_device(target_dev_id))) {
        memcpy(&priv->fw, &fw, sizeof(fw));
        sx_fw_2_ku_query_fw(priv, &fw, &priv->dev_info.dev_info_ro.q_fw);
        priv->is_fw_initialized = 1;
    }

out:
    sx_free_cmd_mailbox(dev, mailbox);
    return err;
}
EXPORT_SYMBOL(sx_QUERY_FW);

int sx_QUERY_FW_2(struct sx_dev *dev, int sx_dev_id, u8 hcr_number)
{
    struct sx_fw          *fw = &sx_priv(dev)->fw;
    struct sx_cmd_mailbox *mailbox;
    int                    err = 0;
    u32                    out_mb_info, in_mb_info;
    int                    sx_cmd = (hcr_number == HCR1) ? SX_CMD_QUERY_FW_HCR1 : SX_CMD_QUERY_FW;

#define QUERY_FW_OUT_MB_INFO_OFFSET 0x00
#define QUERY_FW_IN_MB_INFO_OFFSET  0x04
    mailbox = sx_alloc_cmd_mailbox(dev, dev->device_id);
    if (IS_ERR(mailbox)) {
        return PTR_ERR(mailbox);
    }
    mailbox->imm_data = 0ULL;

    err = sx_cmd_imm(dev, sx_dev_id, 0, mailbox, 0, 1, sx_cmd, SX_CMD_TIME_CLASS_A, QUERY_FW_IN_MB_SIZE);
    if (err) {
        goto out;
    }

    in_mb_info = mailbox->imm_data >> 32;
    out_mb_info = mailbox->imm_data & 0xFFFFFFFFUL;

    if (dev->device_id == sx_dev_id) {
        fw->local_in_mb_offset = (in_mb_info & 0x000FFFFF);
        fw->local_in_mb_size = in_mb_info >> 20;
        fw->local_out_mb_offset = (out_mb_info & 0x000FFFFF);
        fw->local_out_mb_size = out_mb_info >> 20;
    }

    sx_glb.sx_dpt.dpt_info[sx_dev_id].in_mb_offset[hcr_number] = (in_mb_info & 0x000FFFFF);
    sx_glb.sx_dpt.dpt_info[sx_dev_id].in_mb_size[hcr_number] = in_mb_info >> 20;
    sx_glb.sx_dpt.dpt_info[sx_dev_id].out_mb_offset[hcr_number] = (out_mb_info & 0x000FFFFF);
    sx_glb.sx_dpt.dpt_info[sx_dev_id].out_mb_size[hcr_number] = out_mb_info >> 20;

    sxd_log_info(dev,
                 "HCR%d (Dev %d): In-Mailbox [offset=0x%x, size=0x%x], Out-Mailbox [offset=0x%x, size=0x%x]\n",
                 hcr_number + 1,
                 sx_dev_id,
                 sx_glb.sx_dpt.dpt_info[sx_dev_id].in_mb_offset[hcr_number],
                 sx_glb.sx_dpt.dpt_info[sx_dev_id].in_mb_size[hcr_number],
                 sx_glb.sx_dpt.dpt_info[sx_dev_id].out_mb_offset[hcr_number],
                 sx_glb.sx_dpt.dpt_info[sx_dev_id].out_mb_size[hcr_number]);

out:
    sx_free_cmd_mailbox(dev, mailbox);
    return err;
}

#define SX_MAX_VPA_IN_CMD 32

struct sx_map_fa_cb_ctx {
    struct sx_cmd_mailbox *mailbox;
    sxd_map_memory_type_e  type;
    u32                    num_of_vpa;
    __be64                *vpa;
    int                    err;
};

static bool __map_fa_desc_cb(struct sx_dev *dev,
                             void          *buff,
                             dma_addr_t     dma_addr,
                             u32            num_of_pages,
                             bool           is_last,
                             void          *context)
{
    struct sx_map_fa_cb_ctx *map_fa_ctx = (struct sx_map_fa_cb_ctx*)context;
    int                      err = 0;

    map_fa_ctx->vpa[map_fa_ctx->num_of_vpa++] = cpu_to_be64(dma_addr | (map_fa_ctx->type << 8) | ilog2(num_of_pages));
    if (!is_last && (map_fa_ctx->num_of_vpa < SX_MAX_VPA_IN_CMD)) {
        return false; /* continue iteration */
    }

    err = sx_cmd(dev,
                 dev->device_id,
                 map_fa_ctx->mailbox,
                 map_fa_ctx->num_of_vpa,
                 0,
                 SX_CMD_MAP_FA,
                 SX_CMD_TIME_CLASS_B,
                 sx_priv(dev)->fw.local_in_mb_size,
                 0);

    if (!err) {
        map_fa_ctx->num_of_vpa = 0;
        return false; /* continue iteration */
    }

    map_fa_ctx->err = err;
    return true; /* stop iteration */
}

int sx_MAP_FA(struct sx_dev *dev, sxd_map_memory_type_e type, sx_map_fa_handle_t handle)
{
    struct sx_cmd_mailbox  *mailbox;
    struct sx_map_fa_cb_ctx ctx;

    mailbox = sx_alloc_cmd_mailbox(dev, dev->device_id);
    if (IS_ERR(mailbox)) {
        return PTR_ERR(mailbox);
    }
    memset(mailbox->buf, 0, SX_MAILBOX_SIZE);

    ctx.mailbox = mailbox;
    ctx.type = type;
    ctx.num_of_vpa = 0;
    ctx.vpa = mailbox->buf;
    ctx.err = 0;

    sx_map_fa_item_iterate(handle, __map_fa_desc_cb, &ctx);

    sx_free_cmd_mailbox(dev, mailbox);
    return ctx.err;
}

int sx_UNMAP_FA(struct sx_dev *dev)
{
    return sx_cmd(dev, dev->device_id, 0, 0, 0, SX_CMD_UNMAP_FA,
                  SX_CMD_TIME_CLASS_B,
                  sx_priv(dev)->fw.local_in_mb_size, 0);
}

int sx_QUERY_AQ_CAP(struct sx_dev *dev)
{
    struct sx_cmd_mailbox *mailbox;
    u32                   *outbox;
    u8                     field;
    int                    err;
    struct sx_dev_cap     *dev_cap = &sx_priv(dev)->dev_cap;

#define QUERY_DEV_CAP_MAX_SDQ_SZ_OFFSET 0x0
#define QUERY_DEV_CAP_MAX_SDQ_OFFSET    0x3
#define QUERY_DEV_CAP_MAX_RDQ_SZ_OFFSET 0x4
#define QUERY_DEV_CAP_MAX_RDQ_OFFSET    0x7
#define QUERY_DEV_CAP_MAX_CQ_SZ_OFFSET  0x8
#define QUERY_DEV_CAP_MAX_CQ_OFFSET     0xb
#define QUERY_DEV_CAP_MAX_EQ_SZ_OFFSET  0xc
#define QUERY_DEV_CAP_MAX_EQ_OFFSET     0xf
#define QUERY_DEV_CAP_MAX_SG_SQ_OFFSET  0x12
#define QUERY_DEV_CAP_MAX_SG_RQ_OFFSET  0x13

    mailbox = sx_alloc_cmd_mailbox(dev, dev->device_id);
    if (IS_ERR(mailbox)) {
        return PTR_ERR(mailbox);
    }
    outbox = mailbox->buf;

    err = sx_cmd_box(dev, dev->device_id, 0, mailbox, 0, 0,
                     SX_CMD_QUERY_AQ_CAP, SX_CMD_TIME_CLASS_A, 0, 0);

    if (err) {
        goto out;
    }

    SX_GET(field, outbox, QUERY_DEV_CAP_MAX_SDQ_SZ_OFFSET);
    dev_cap->log_max_sdq_sz = min((int)field, SX_MAX_LOG_DQ_SIZE);
    SX_GET(field, outbox, QUERY_DEV_CAP_MAX_SDQ_OFFSET);
    dev_cap->max_num_sdqs = field;
    SX_GET(field, outbox, QUERY_DEV_CAP_MAX_RDQ_SZ_OFFSET);
    dev_cap->log_max_rdq_sz = min((int)field, SX_MAX_LOG_DQ_SIZE);
    SX_GET(field, outbox, QUERY_DEV_CAP_MAX_RDQ_OFFSET);
    dev_cap->max_num_rdqs = field;
    SX_GET(field, outbox, QUERY_DEV_CAP_MAX_CQ_SZ_OFFSET);
    dev_cap->log_max_cq_sz = min((int)field, SX_MAX_LOG_CQ_SIZE);
    SX_GET(field, outbox, QUERY_DEV_CAP_MAX_CQ_OFFSET);
    dev_cap->max_num_cqs = field;
    SX_GET(field, outbox, QUERY_DEV_CAP_MAX_EQ_SZ_OFFSET);
    dev_cap->log_max_eq_sz = field;
    SX_GET(field, outbox, QUERY_DEV_CAP_MAX_EQ_OFFSET);
    dev_cap->max_num_eqs = field;
    SX_GET(field, outbox, QUERY_DEV_CAP_MAX_SG_SQ_OFFSET);
    dev_cap->max_sg_sq = field;
    SX_GET(field, outbox, QUERY_DEV_CAP_MAX_SG_RQ_OFFSET);
    dev_cap->max_sg_rq = field;

out:
    sx_free_cmd_mailbox(dev, mailbox);
    return err;
}
EXPORT_SYMBOL(sx_QUERY_AQ_CAP);

int sx_QUERY_BOARDINFO(struct sx_dev *dev, struct ku_query_board_info *board)
{
    struct sx_cmd_mailbox *mailbox;
    u32                   *outbox;
    int                    err;

#define QUERY_ADAPTER_INTA_PIN_OFFSET      0x10
#define QUERY_ADAPTER_VSD_VENDOR_ID_OFFSET 0x1e
#define QUERY_ADAPTER_VSD_OFFSET           0x20

    memset(board, 0, sizeof(*board));
    mailbox = sx_alloc_cmd_mailbox(dev, dev->device_id);
    if (IS_ERR(mailbox)) {
        return PTR_ERR(mailbox);
    }

    outbox = mailbox->buf;
    err = sx_cmd_box(dev, dev->device_id, 0, mailbox, 0, 0,
                     SX_CMD_QUERY_BOARDINFO, SX_CMD_TIME_CLASS_A, 0, 0);
    if (err) {
        goto out;
    }


    SX_GET(board->vsd_vendor_id, outbox,
           QUERY_ADAPTER_VSD_VENDOR_ID_OFFSET);
    SX_GET(board->inta_pin, outbox, QUERY_ADAPTER_INTA_PIN_OFFSET);

    if (board->vsd_vendor_id == PCI_VENDOR_ID_MELLANOX) {
        get_board_id(outbox + QUERY_ADAPTER_VSD_OFFSET / 4,
                     board->board_id);
    }

out:
    sx_free_cmd_mailbox(dev, mailbox);
    return err;
}
EXPORT_SYMBOL(sx_QUERY_BOARDINFO);

#define PROFILE_SET_MASK_H_OFFSET                  0x00
#define PROFILE_SET_MASK_L_OFFSET                  0x08
#define PROFILE_MAX_VEPA_OFFSET                    0x13
#define PROFILE_PORT_PROFILE_NUM_OFFSET            0x14
#define PROFILE_MAX_LAG_OFFSET                     0x16
#define PROFILE_MAX_PORT_OFFSET                    0x1a
#define PROFILE_MAX_MID_OFFSET                     0x1e
#define PROFILE_MAX_PGT_OFFSET                     0x22
#define PROFILE_MAX_SYSPORT_OFFSET                 0x26
#define PROFILE_MAX_VLANS_OFFSET                   0x2a
#define PROFILE_MAX_REGIONS_OFFSET                 0x2e
#define PROFILE_MAX_FLOOD_TABLES_OFFSET            0x31
#define PROFILE_MAX_PER_VID_FLOOD_TABLES_OFFSET    0x32
#define PROFILE_FLOOD_MODE_OFFSET                  0x33
#define PROFILE_MAX_FID_OFFSET_FLOOD_TABLES_OFFSET 0x34
#define PROFILE_FID_OFFSET_TABLE_SIZE_OFFSET       0x36
#define PROFILE_MAX_PER_FID_FLOOD_TABLE_OFFSET     0x38
#define PROFILE_PER_FID_TABLE_SIZE_OFFSET          0x3A
#define PROFILE_MAX_FID_OFFSET                     0x3E
#define PROFILE_MAX_IB_MC_OFFSET                   0x42
#define PROFILE_MAX_PKEY_OFFSET                    0x46
#define PROFILE_HSBA_STATUS                        0x4b
#define PROFILE_AR_SEC_OFFSET                      0x4c
#define PROFILE_AR_GRP_CAP_OFFSET                  0x4e
#define PROFILE_ARN_OFFSET                         0x50
#define PROFILE_ARN_BIT_N                          7
#define PROFILE_MODULE_OFFSET                      0x51
#define PROFILE_KVH_XLT_CACHE_MODE                 0x52
#define PROFILE_UBRIDGE_SPLIT_READY_OFFSET         0x53
#define PROFILE_SPLIT_READY_BIT_N                  1
#define PROFILE_LAG_MODE_BIT_N                     3
#define PROFILE_UBRIDGE_BIT_N                      4
#define PROFILE_UMLABEL_BIT_N                      5
#define PROFILE_KVD_LINEAR_SIZE_OFFSET             0x54
#define PROFILE_KVD_HASH_SINGLE_SIZE_OFFSET        0x58
#define PROFILE_KVD_HASH_DOUBLE_SIZE_OFFSET        0x5C
#define PROFILE_SWID_0_CONF_OFFSET                 0x60
#define PROFILE_SWID_1_CONF_OFFSET                 0x68
#define PROFILE_SWID_2_CONF_OFFSET                 0x70
#define PROFILE_SWID_3_CONF_OFFSET                 0x78
#define PROFILE_SWID_4_CONF_OFFSET                 0x80
#define PROFILE_SWID_5_CONF_OFFSET                 0x88
#define PROFILE_SWID_6_CONF_OFFSET                 0x90
#define PROFILE_SWID_7_CONF_OFFSET                 0x98
#define PROFILE_IB_RTR_FLAGS_OFFSET                0xa0
#define PROFILE_IB_RTR_EN_BIT_N                    7
#define PROFILE_IB_RTR_ECMP_BIT_N                  6
#define PROFILE_IB_RTR_MCF_BIT_N                   5
#define PROFILE_IB_RTR_ECMP_LID_RANGE_OFFSET       0xa2
#define PROFILE_CQE_TIME_STAMP_TYPE_OFFSET         0xb2
#define PROFILE_CQE_TIME_STAMP_TYPE_BIT_N          0
#define PROFILE_E2E_TIME_BIT_N                     2
#define PROFILE_CQE_VERSION_OFFSET                 0xb3
#define PROFILE_RESERVED1_OFFSET                   0xb4
#define PROFILE_SUPPORTED_FEATURE_SET_OFFSET       0xb8
#define CONFIG_PROFILE_MB_SIZE                     0xc0
#define SUPPORTED_FEATURE_SET_MOD_IND_MASK         ((u32)(0x1))


int sx_GET_PROFILE(struct sx_dev *dev, struct ku_profile *profile, struct profile_driver_params *params_p)
{
    struct sx_cmd_mailbox       *mailbox;
    u32                         *outbox;
    int                          err;
    u8                           tmp;
    struct profile_driver_params params;

    memset(profile, 0, sizeof(*profile));
    memset(&params, 0, sizeof(params));
    mailbox = sx_alloc_cmd_mailbox(dev, profile->dev_id);
    if (IS_ERR(mailbox)) {
        return PTR_ERR(mailbox);
    }

    outbox = mailbox->buf;
    err = sx_cmd_box(dev, dev->device_id, 0, mailbox, 0, 0x2,
                     SX_CMD_CONFIG_PROFILE, SX_CMD_TIME_CLASS_A,
                     CONFIG_PROFILE_MB_SIZE, 0);
    if (err) {
        goto out;
    }

    SX_GET(profile->max_vepa_channels, outbox, PROFILE_MAX_VEPA_OFFSET);
    SX_GET(profile->max_lag, outbox, PROFILE_MAX_LAG_OFFSET);
    SX_GET(profile->max_port_per_lag, outbox, PROFILE_MAX_PORT_OFFSET);
    SX_GET(profile->max_mid, outbox, PROFILE_MAX_MID_OFFSET);
    SX_GET(profile->max_pgt, outbox, PROFILE_MAX_PGT_OFFSET);
    SX_GET(profile->max_system_port, outbox, PROFILE_MAX_SYSPORT_OFFSET);
    SX_GET(profile->max_active_vlans, outbox, PROFILE_MAX_VLANS_OFFSET);
    SX_GET(profile->max_regions, outbox, PROFILE_MAX_REGIONS_OFFSET);
    SX_GET(profile->max_flood_tables, outbox, PROFILE_MAX_FLOOD_TABLES_OFFSET);
    SX_GET(profile->max_per_vid_flood_tables, outbox,
           PROFILE_MAX_PER_VID_FLOOD_TABLES_OFFSET);
    SX_GET(profile->flood_mode, outbox, PROFILE_FLOOD_MODE_OFFSET);
    SX_GET(profile->max_fid_offset_flood_tables, outbox, PROFILE_MAX_FID_OFFSET_FLOOD_TABLES_OFFSET);
    SX_GET(profile->fid_offset_table_size, outbox, PROFILE_FID_OFFSET_TABLE_SIZE_OFFSET);
    SX_GET(profile->max_per_fid_flood_table, outbox, PROFILE_MAX_PER_FID_FLOOD_TABLE_OFFSET);
    SX_GET(profile->per_fid_table_size, outbox, PROFILE_PER_FID_TABLE_SIZE_OFFSET);
    SX_GET(profile->max_ib_mc, outbox, PROFILE_MAX_IB_MC_OFFSET);
    SX_GET(profile->max_pkey, outbox, PROFILE_MAX_PKEY_OFFSET);
    SX_GET(profile->hsba_status, outbox, PROFILE_HSBA_STATUS);
    SX_GET(profile->ar_sec, outbox, PROFILE_AR_SEC_OFFSET);
    SX_GET(profile->adaptive_routing_group_cap, outbox, PROFILE_AR_GRP_CAP_OFFSET);
    SX_GET(profile->arn, outbox, PROFILE_ARN_OFFSET);
    profile->arn = profile->arn >> PROFILE_ARN_BIT_N;
    SX_GET(profile->module_master_fw_default, outbox, PROFILE_MODULE_OFFSET);
    SX_GET(tmp, outbox, PROFILE_UBRIDGE_SPLIT_READY_OFFSET);
    profile->lag_mode = (tmp >> PROFILE_LAG_MODE_BIT_N) & 0x1;
    profile->split_ready = (tmp >> PROFILE_SPLIT_READY_BIT_N) & 0x3;
    profile->ubridge_mode = (tmp >> PROFILE_UBRIDGE_BIT_N) & 0x1;
    profile->umlabel = (tmp >> PROFILE_UMLABEL_BIT_N) & 0x1;
    SX_GET(profile->kvd_linear_size, outbox, PROFILE_KVD_LINEAR_SIZE_OFFSET);
    SX_GET(profile->kvd_hash_single_size, outbox, PROFILE_KVD_HASH_SINGLE_SIZE_OFFSET);
    SX_GET(profile->kvd_hash_double_size, outbox, PROFILE_KVD_HASH_DOUBLE_SIZE_OFFSET);
    SX_GET(tmp, outbox, PROFILE_SWID_0_CONF_OFFSET + 1);
    profile->swid0_config_type.type = tmp >> 4;
    SX_GET(tmp, outbox, PROFILE_SWID_1_CONF_OFFSET + 1);
    profile->swid1_config_type.type = tmp >> 4;
    SX_GET(tmp, outbox, PROFILE_SWID_2_CONF_OFFSET + 1);
    profile->swid2_config_type.type = tmp >> 4;
    SX_GET(tmp, outbox, PROFILE_SWID_3_CONF_OFFSET + 1);
    profile->swid3_config_type.type = tmp >> 4;
    SX_GET(tmp, outbox, PROFILE_SWID_4_CONF_OFFSET + 1);
    profile->swid4_config_type.type = tmp >> 4;
    SX_GET(tmp, outbox, PROFILE_SWID_5_CONF_OFFSET + 1);
    profile->swid5_config_type.type = tmp >> 4;
    SX_GET(tmp, outbox, PROFILE_SWID_6_CONF_OFFSET + 1);
    profile->swid6_config_type.type = tmp >> 4;
    SX_GET(tmp, outbox, PROFILE_SWID_7_CONF_OFFSET + 1);
    profile->swid7_config_type.type = tmp >> 4;
    SX_GET(profile->swid0_config_type.properties, outbox,
           PROFILE_SWID_0_CONF_OFFSET + 3);
    SX_GET(profile->swid1_config_type.properties, outbox,
           PROFILE_SWID_1_CONF_OFFSET + 3);
    SX_GET(profile->swid2_config_type.properties, outbox,
           PROFILE_SWID_2_CONF_OFFSET + 3);
    SX_GET(profile->swid3_config_type.properties, outbox,
           PROFILE_SWID_3_CONF_OFFSET + 3);
    SX_GET(profile->swid4_config_type.properties, outbox,
           PROFILE_SWID_4_CONF_OFFSET + 3);
    SX_GET(profile->swid5_config_type.properties, outbox,
           PROFILE_SWID_5_CONF_OFFSET + 3);
    SX_GET(profile->swid6_config_type.properties, outbox,
           PROFILE_SWID_6_CONF_OFFSET + 3);
    SX_GET(profile->swid7_config_type.properties, outbox,
           PROFILE_SWID_7_CONF_OFFSET + 3);
    SX_GET(tmp, outbox, PROFILE_IB_RTR_FLAGS_OFFSET);
    profile->ib_router_en = (tmp >> PROFILE_IB_RTR_EN_BIT_N) & 0x1;
    profile->ib_router_ecmp = (tmp >> PROFILE_IB_RTR_ECMP_BIT_N) & 0x1;
    profile->ib_router_mcf = (tmp >> PROFILE_IB_RTR_MCF_BIT_N) & 0x1;
    SX_GET(profile->ib_router_ecmp_lid_range, outbox, PROFILE_IB_RTR_ECMP_LID_RANGE_OFFSET);
    SX_GET(tmp, outbox, PROFILE_CQE_TIME_STAMP_TYPE_OFFSET);
    params.cqe_time_stamp_type = (tmp >> PROFILE_CQE_TIME_STAMP_TYPE_BIT_N) & 0x3;
    profile->e2e_time = (tmp >> PROFILE_E2E_TIME_BIT_N) & 0x3;
    SX_GET(params.cqe_version, outbox, PROFILE_CQE_VERSION_OFFSET);
    SX_GET(params.supported_feature_set, outbox, PROFILE_SUPPORTED_FEATURE_SET_OFFSET);
    SX_GET(profile->reserved1, outbox, PROFILE_RESERVED1_OFFSET);

    if (params_p != NULL) {
        memcpy(params_p, &params, sizeof(*params_p));
    }

out:
    sx_free_cmd_mailbox(dev, mailbox);
    return err;
}
EXPORT_SYMBOL(sx_GET_PROFILE);

int sx_SET_PROFILE(struct sx_dev *dev, struct ku_profile *profile, struct profile_driver_params *params)
{
    struct sx_cmd_mailbox *mailbox;
    u32                   *inbox;
    int                    err;
    u8                     temp_u8 = 0;

    mailbox = sx_alloc_cmd_mailbox(dev, profile->dev_id);
    if (IS_ERR(mailbox)) {
        return PTR_ERR(mailbox);
    }

    params->supported_feature_set |= SUPPORTED_FEATURE_SET_MOD_IND_MASK;

    inbox = mailbox->buf;
    memset(inbox, 0, SX_MAILBOX_SIZE);
    SX_PUT(inbox, profile->set_mask_0_63, PROFILE_SET_MASK_L_OFFSET);
    SX_PUT(inbox, profile->set_mask_64_127, PROFILE_SET_MASK_H_OFFSET);
    SX_PUT(inbox, profile->max_vepa_channels, PROFILE_MAX_VEPA_OFFSET);
    temp_u8 = profile->port_profile_num << 4;
    SX_PUT(inbox, temp_u8, PROFILE_PORT_PROFILE_NUM_OFFSET);
    SX_PUT(inbox, profile->max_lag, PROFILE_MAX_LAG_OFFSET);
    SX_PUT(inbox, profile->max_port_per_lag, PROFILE_MAX_PORT_OFFSET);
    SX_PUT(inbox, profile->max_mid, PROFILE_MAX_MID_OFFSET);
    SX_PUT(inbox, profile->max_pgt, PROFILE_MAX_PGT_OFFSET);
    SX_PUT(inbox, profile->max_system_port,
           PROFILE_MAX_SYSPORT_OFFSET);
    SX_PUT(inbox, profile->max_active_vlans, PROFILE_MAX_VLANS_OFFSET);
    SX_PUT(inbox, profile->max_regions, PROFILE_MAX_REGIONS_OFFSET);
    SX_PUT(inbox, profile->max_flood_tables, PROFILE_MAX_FLOOD_TABLES_OFFSET);
    SX_PUT(inbox, profile->max_per_vid_flood_tables,
           PROFILE_MAX_PER_VID_FLOOD_TABLES_OFFSET);
    SX_PUT(inbox, profile->flood_mode, PROFILE_FLOOD_MODE_OFFSET);
    SX_PUT(inbox, profile->max_fid_offset_flood_tables, PROFILE_MAX_FID_OFFSET_FLOOD_TABLES_OFFSET);
    SX_PUT(inbox, profile->fid_offset_table_size, PROFILE_FID_OFFSET_TABLE_SIZE_OFFSET);
    SX_PUT(inbox, profile->max_per_fid_flood_table, PROFILE_MAX_PER_FID_FLOOD_TABLE_OFFSET);
    SX_PUT(inbox, profile->per_fid_table_size, PROFILE_PER_FID_TABLE_SIZE_OFFSET);
    SX_PUT(inbox, profile->max_fid, PROFILE_MAX_FID_OFFSET);
    SX_PUT(inbox, profile->max_ib_mc, PROFILE_MAX_IB_MC_OFFSET);
    SX_PUT(inbox, profile->max_pkey, PROFILE_MAX_PKEY_OFFSET);
    SX_PUT(inbox, profile->hsba_status, PROFILE_HSBA_STATUS);
    SX_PUT(inbox, profile->ar_sec, PROFILE_AR_SEC_OFFSET);
    SX_PUT(inbox, profile->adaptive_routing_group_cap, PROFILE_AR_GRP_CAP_OFFSET);
    SX_PUT(inbox, (u8)(profile->arn << PROFILE_ARN_BIT_N), PROFILE_ARN_OFFSET);
    SX_PUT(inbox, profile->module_master_fw_default, PROFILE_MODULE_OFFSET);
    temp_u8 = (u8)((profile->split_ready) << PROFILE_SPLIT_READY_BIT_N);
    temp_u8 |= (u8)((!!(profile->lag_mode)) << PROFILE_LAG_MODE_BIT_N);
    temp_u8 |= (u8)((!!(profile->ubridge_mode)) << PROFILE_UBRIDGE_BIT_N);
    temp_u8 |= (u8)((!!(profile->umlabel)) << PROFILE_UMLABEL_BIT_N);
    SX_PUT(inbox, temp_u8, PROFILE_UBRIDGE_SPLIT_READY_OFFSET);
    SX_PUT(inbox, profile->kvd_linear_size, PROFILE_KVD_LINEAR_SIZE_OFFSET);
    SX_PUT(inbox, profile->kvd_hash_single_size, PROFILE_KVD_HASH_SINGLE_SIZE_OFFSET);
    SX_PUT(inbox, profile->kvd_hash_double_size, PROFILE_KVD_HASH_DOUBLE_SIZE_OFFSET);
    SX_PUT(inbox, profile->swid0_config_type.mask,
           PROFILE_SWID_0_CONF_OFFSET);
    SX_PUT(inbox, profile->swid0_config_type.properties,
           PROFILE_SWID_0_CONF_OFFSET + 3);
    temp_u8 = (u8)(profile->swid0_config_type.type << 4);
    SX_PUT(inbox, temp_u8, PROFILE_SWID_0_CONF_OFFSET + 1);
    SX_PUT(inbox, profile->swid1_config_type.mask,
           PROFILE_SWID_1_CONF_OFFSET);
    SX_PUT(inbox, profile->swid1_config_type.properties,
           PROFILE_SWID_1_CONF_OFFSET + 3);
    temp_u8 = (u8)(profile->swid1_config_type.type << 4);
    SX_PUT(inbox, temp_u8, PROFILE_SWID_1_CONF_OFFSET + 1);
    SX_PUT(inbox, profile->swid2_config_type.mask,
           PROFILE_SWID_2_CONF_OFFSET);
    SX_PUT(inbox, profile->swid2_config_type.properties,
           PROFILE_SWID_2_CONF_OFFSET + 3);
    temp_u8 = (u8)(profile->swid2_config_type.type << 4);
    SX_PUT(inbox, temp_u8, PROFILE_SWID_2_CONF_OFFSET + 1);
    SX_PUT(inbox, profile->swid3_config_type.mask,
           PROFILE_SWID_3_CONF_OFFSET);
    SX_PUT(inbox, profile->swid3_config_type.properties,
           PROFILE_SWID_3_CONF_OFFSET + 3);
    temp_u8 = (u8)(profile->swid3_config_type.type << 4);
    SX_PUT(inbox, temp_u8, PROFILE_SWID_3_CONF_OFFSET + 1);
    SX_PUT(inbox, profile->swid4_config_type.mask,
           PROFILE_SWID_4_CONF_OFFSET);
    SX_PUT(inbox, profile->swid4_config_type.properties,
           PROFILE_SWID_4_CONF_OFFSET + 3);
    temp_u8 = (u8)(profile->swid4_config_type.type << 4);
    SX_PUT(inbox, temp_u8, PROFILE_SWID_4_CONF_OFFSET + 1);
    SX_PUT(inbox, profile->swid5_config_type.mask,
           PROFILE_SWID_5_CONF_OFFSET);
    SX_PUT(inbox, profile->swid5_config_type.properties,
           PROFILE_SWID_5_CONF_OFFSET + 3);
    temp_u8 = (u8)(profile->swid5_config_type.type << 4);
    SX_PUT(inbox, temp_u8, PROFILE_SWID_5_CONF_OFFSET + 1);
    SX_PUT(inbox, profile->swid6_config_type.mask,
           PROFILE_SWID_6_CONF_OFFSET);
    SX_PUT(inbox, profile->swid6_config_type.properties,
           PROFILE_SWID_6_CONF_OFFSET + 3);
    temp_u8 = (u8)(profile->swid6_config_type.type << 4);
    SX_PUT(inbox, temp_u8, PROFILE_SWID_6_CONF_OFFSET + 1);
    SX_PUT(inbox, profile->swid7_config_type.mask,
           PROFILE_SWID_7_CONF_OFFSET);
    SX_PUT(inbox, profile->swid7_config_type.properties,
           PROFILE_SWID_7_CONF_OFFSET + 3);
    temp_u8 = (u8)(profile->swid7_config_type.type << 4);
    SX_PUT(inbox, temp_u8, PROFILE_SWID_7_CONF_OFFSET + 1);
    SX_PUT(inbox, profile->reserved1, PROFILE_RESERVED1_OFFSET);
    temp_u8 = (u8)((profile->ib_router_en & 0x1) << PROFILE_IB_RTR_EN_BIT_N |
                   (profile->ib_router_ecmp & 0x1) << PROFILE_IB_RTR_ECMP_BIT_N |
                   (profile->ib_router_mcf & 0x1) << PROFILE_IB_RTR_MCF_BIT_N);
    SX_PUT(inbox, temp_u8, PROFILE_IB_RTR_FLAGS_OFFSET);
    SX_PUT(inbox, profile->ib_router_ecmp_lid_range, PROFILE_IB_RTR_ECMP_LID_RANGE_OFFSET);
    temp_u8 = (u8)((params->cqe_time_stamp_type & 0x3) << PROFILE_CQE_TIME_STAMP_TYPE_BIT_N);
    temp_u8 |= (u8)((profile->e2e_time & 0x3) << PROFILE_E2E_TIME_BIT_N);
    SX_PUT(inbox, temp_u8, PROFILE_CQE_TIME_STAMP_TYPE_OFFSET);
    SX_PUT(inbox, params->cqe_version, PROFILE_CQE_VERSION_OFFSET);
    SX_PUT(inbox, params->supported_feature_set, PROFILE_SUPPORTED_FEATURE_SET_OFFSET);

    err = sx_cmd(dev, profile->dev_id, mailbox, 0, 1, SX_CMD_CONFIG_PROFILE,
                 SX_CMD_TIME_CLASS_A, CONFIG_PROFILE_MB_SIZE, 0);

    sx_free_cmd_mailbox(dev, mailbox);
    return err;
}

int sx_SET_KVH_CACHE_PARAMS(struct sx_dev *dev, struct profile_kvh_params *kvh_params)
{
    struct sx_cmd_mailbox *mailbox;
    struct ku_profile      profile;
    u32                   *inbox;
    int                    err;


    mailbox = sx_alloc_cmd_mailbox(dev, dev->device_id);
    if (IS_ERR(mailbox)) {
        return PTR_ERR(mailbox);
    }

    inbox = mailbox->buf;
    memset(inbox, 0, SX_MAILBOX_SIZE);
    memset(&profile, 0, sizeof(struct ku_profile));

    profile.set_mask_0_63 = 0x800000000;   /* Bit35 set_kvh_xlt_cache_mode*/
    SX_PUT(inbox, profile.set_mask_0_63, PROFILE_SET_MASK_L_OFFSET);
    SX_PUT(inbox, kvh_params->kvh_xlt_cache_mode, PROFILE_KVH_XLT_CACHE_MODE);


    err = sx_cmd(dev, dev->device_id, mailbox, 0, 1, SX_CMD_CONFIG_PROFILE,
                 SX_CMD_TIME_CLASS_A, CONFIG_PROFILE_MB_SIZE, 0);

    sx_free_cmd_mailbox(dev, mailbox);
    return err;
}

int sx_GET_KVH_CACHE_PARAMS(struct sx_dev *dev, struct profile_kvh_params *kvh_params_p)
{
    struct sx_cmd_mailbox *mailbox;
    u32                   *outbox;
    int                    err;

    mailbox = sx_alloc_cmd_mailbox(dev, dev->device_id);
    if (IS_ERR(mailbox)) {
        return PTR_ERR(mailbox);
    }

    memset(kvh_params_p, 0, sizeof(struct profile_kvh_params));

    outbox = mailbox->buf;
    err = sx_cmd_box(dev, dev->device_id, 0, mailbox, 0, 0x2,
                     SX_CMD_CONFIG_PROFILE, SX_CMD_TIME_CLASS_A,
                     CONFIG_PROFILE_MB_SIZE, 0);
    if (err) {
        goto out;
    }
    SX_GET(kvh_params_p->kvh_xlt_cache_mode, outbox, PROFILE_KVH_XLT_CACHE_MODE);

out:
    sx_free_cmd_mailbox(dev, mailbox);
    return err;
}
EXPORT_SYMBOL(sx_GET_KVH_CACHE_PARAMS);

int sx_SET_SYSTEM_M_KEY(struct sx_dev *dev, struct ku_system_m_key *system_m_key)
{
    struct sx_cmd_mailbox *mailbox;
    int                    err;

#define INIT_SYSTEM_M_KEY_MB_SIZE 0x08

    mailbox = sx_alloc_cmd_mailbox(dev, system_m_key->dev_id);
    if (IS_ERR(mailbox)) {
        return PTR_ERR(mailbox);
    }

    mailbox->imm_data = system_m_key->system_m_key;
    mailbox->is_in_param_imm = 1;

    err = sx_cmd(dev, system_m_key->dev_id, mailbox, 0, 1, SX_CMD_INIT_SYSTEM_M_KEY,
                 SX_CMD_TIME_CLASS_A, INIT_SYSTEM_M_KEY_MB_SIZE, 0);

    sx_free_cmd_mailbox(dev, mailbox);
    return err;
}
EXPORT_SYMBOL(sx_SET_SYSTEM_M_KEY);

int sx_GET_SYSTEM_M_KEY(struct sx_dev *dev, struct ku_system_m_key *system_m_key)
{
    struct sx_cmd_mailbox *mailbox;
    int                    err;

#define INIT_SYSTEM_M_KEY_MB_SIZE 0x08

    mailbox = sx_alloc_cmd_mailbox(dev, system_m_key->dev_id);
    if (IS_ERR(mailbox)) {
        return PTR_ERR(mailbox);
    }

    err = sx_cmd_imm(dev, system_m_key->dev_id, 0, mailbox, 0, 2, SX_CMD_INIT_SYSTEM_M_KEY,
                     SX_CMD_TIME_CLASS_A, INIT_SYSTEM_M_KEY_MB_SIZE);
    if (err) {
        goto out;
    }

    system_m_key->system_m_key = mailbox->imm_data;

out:
    sx_free_cmd_mailbox(dev, mailbox);
    return err;
}
EXPORT_SYMBOL(sx_GET_SYSTEM_M_KEY);

int sx_ISSU_FW_START(struct sx_dev *dev, uint8_t fw_cmd)
{
    return sx_cmd(dev, dev->device_id, 0, 0, fw_cmd, SX_CMD_ISSU_FW,
                  SX_CMD_TIME_CLASS_A, sx_priv(dev)->fw.local_in_mb_size, 0);
}

int sx_ISSU_FW_STATUS_GET(struct sx_dev *dev, u8 *issu_fw_status)
{
    struct sx_cmd_mailbox *mailbox;
    int                    err = 0;

#define INIT_ISSU_FW_MB_SIZE 0x08

    mailbox = sx_alloc_cmd_mailbox(dev, dev->device_id);
    if (IS_ERR(mailbox)) {
        return PTR_ERR(mailbox);
    }

    mailbox->imm_data = 0ULL;
    err = sx_cmd_imm(dev, dev->device_id, 0, mailbox, 0, 2 /* get status */, SX_CMD_ISSU_FW,
                     SX_CMD_TIME_CLASS_A, INIT_ISSU_FW_MB_SIZE);
    if (err) {
        goto out;
    }

    *issu_fw_status = mailbox->imm_data & 0xF;

out:
    sx_free_cmd_mailbox(dev, mailbox);
    return err;
}


int sx_MAD_DEMUX(struct sx_dev *sx_dev, int dev_id, u8 enable)
{
    static const uint8_t   mad_demux_enable_cmd[] = {
        0x00, 0x02, 0x10, 0x00, /* qp0_method_mask=[trap], qp0_attribute_mask_all_ports=[sm_info]           */
        0x00, 0x00, 0x00, 0x00, /* reserved                                                                 */
        0x00, 0x00, 0x00, 0x03, /* qp0_attribute_mask_port_zero=[port_info,pkey_table]                      */
        0x00, 0x00, 0x09, 0x70, /* other_class_flags=[sa,qp0_res,vs13,vs14,vs15]                            */
        0x80, 0x01, 0x00, 0x00, /* perf_flag=[1], perf_method_mask=[get_response_method]                    */
        0x00, 0x00, 0x00, 0x00, /* reserved                                                                 */
        0x80, 0x03, 0x00, 0x00, /* bm_flag=[1], bm_method_mask=[get_response_method,trap_method]            */
        0x00, 0x00, 0x00, 0x00, /* reserved                                                                 */
        0x80, 0x03, 0x00, 0x00, /* vs9_flag=[1], vs9_method_mask=[get_response_method,trap_method]          */
        0x00, 0x00, 0x00, 0x00, /* reserved                                                                 */
        0x80, 0x01, 0x00, 0x00, /* vs10_flag=[1], vs10_method_mask=[get_response_method]                    */
        0x00, 0x00, 0x00, 0x00, /* reserved                                                                 */
        0x80, 0x03, 0x00, 0x00, /* cc_flag=[1], cc_method_mask=[get_response_method,trap_method]            */
        0x00, 0x00, 0x00, 0x00, /* reserved                                                                 */
        0x00, 0x03, 0x00, 0x00, /* vs11_method_mask=[get_response_method,trap_method]                       */
        0x00, 0x00, 0x00, 0x00, /* reserved                                                                 */
        0x00, 0x03, 0x00, 0x00, /* vs12_method_mask=[get_response_method,trap_method]                       */
        0x00, 0x00, 0x00, 0x00, /* reserved                                                                 */
        0x00, 0x00, 0x00, 0x00, /* vs13 covered by 'other_class_flags'                                      */
        0x00, 0x00, 0x00, 0x00, /* reserved                                                                 */
        0x00, 0x00, 0x00, 0x00, /* vs14 covered by 'other_class_flags'                                      */
        0x00, 0x00, 0x00, 0x00, /* reserved                                                                 */
        0x00, 0x00, 0x00, 0x00, /* vs15 covered by 'other_class_flags'                                      */
    };
    struct sx_cmd_mailbox *mailbox;
    int                    err;

    mailbox = sx_alloc_cmd_mailbox(sx_dev, dev_id);
    if (IS_ERR(mailbox)) {
        return PTR_ERR(mailbox);
    }

    if (enable) {
        memcpy(mailbox->buf, mad_demux_enable_cmd, sizeof(mad_demux_enable_cmd));
        memset(mailbox->buf + sizeof(mad_demux_enable_cmd), 0, 256 - sizeof(mad_demux_enable_cmd)); /* padding */
    } else { /* disable */
        memset(mailbox->buf, 0, 256); /* disable command is all zeros */
    }

    err = sx_cmd(sx_dev, dev_id, mailbox, 0, 0, SX_CMD_INIT_MAD_DEMUX, SX_CMD_TIME_CLASS_C, 256, 0);

    sx_free_cmd_mailbox(sx_dev, mailbox);
    return err;
}

int sx_QUERY_CQ(struct sx_dev *dev, u8 cqn, struct ku_query_cq *cq_context_p)
{
    u32                   *outbox;
    int                    err;
    u32                    in_modifier = cqn & 0xffffff;
    u8                     cqe_ver_eqn, st;
    u16                    last_notified_counter;
    struct sx_cmd_mailbox *mailbox;

#define REG_MODULE_OFFSET                 0x15
#define QUERY_CQ_LEN                      0x50
#define QUERY_CQ_CQE_VER_EQN_OFFSET       0x0
#define QUERY_CQ_ST_OFFSET                0x2
#define QUERY_CQ_LOG_CQ_SIZE_OFFSET       0x3
#define QUERY_CQ_PROD_PTR_OFFSET          0x6
#define QUERY_CQ_LAST_NOTIFIED_PRT_OFFSET 0xa

    mailbox = sx_alloc_cmd_mailbox(dev, dev->device_id);
    if (IS_ERR(mailbox)) {
        return PTR_ERR(mailbox);
    }
    outbox = mailbox->buf;
    err = sx_cmd_box(dev, dev->device_id, 0, mailbox, in_modifier, 0,
                     SX_CMD_QUERY_CQ, SX_CMD_TIME_CLASS_A,
                     QUERY_CQ_LEN, 0);
    if (err) {
        goto out;
    }

    SX_GET(cqe_ver_eqn, outbox, QUERY_CQ_CQE_VER_EQN_OFFSET);
    cq_context_p->eq_num = cqe_ver_eqn & 0x1;

    SX_GET(st, outbox, QUERY_CQ_ST_OFFSET);
    SX_GET(cq_context_p->log_cq_size, outbox, QUERY_CQ_LOG_CQ_SIZE_OFFSET);
    SX_GET(cq_context_p->producer_counter, outbox, QUERY_CQ_PROD_PTR_OFFSET);
    SX_GET(last_notified_counter, outbox, QUERY_CQ_LAST_NOTIFIED_PRT_OFFSET);

out:
    sx_free_cmd_mailbox(dev, mailbox);
    return err;
}
EXPORT_SYMBOL(sx_QUERY_CQ);
