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

/*****************************************************
 * NOTE:
 * This file is compiled both in userspace and kernel
 ****************************************************/

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/mlx_sx/ku_emad.h>
#include <linux/mlx_sx/device.h>
#else /* __KERNEL__ */
#include <errno.h>
#include <sx/sxd/ku_emad.h>
#include <sx/sxd/kernel_user.h>
#endif /* __KERNEL */

#define SXD_EMAD_CHECK_TLV_FLAGS(ret, type_flag, tlv_expected_mask, tlv_found_mask) \
    do {                                                                            \
        if ((type_flag) & (tlv_expected_mask)) {                                    \
            if ((type_flag) & (tlv_found_mask)) {                                   \
                ret = SXD_EMAD_TLV_MARKERS_STATUS_FOUND_TWICE_E;                    \
                goto out;                                                           \
            }                                                                       \
            (tlv_found_mask) |= (type_flag);                                        \
        }                                                                           \
    } while (0)

sxd_emad_tlv_markers_status_e sxd_emad_tlv_markers_get(const uint8_t          *buff,
                                                       uint32_t                buff_size,
                                                       uint32_t                flags,
                                                       uint32_t                mandatory_tlv_mask,
                                                       uint32_t                optional_tlv_mask,
                                                       sxd_emad_tlv_markers_t *tlv_markers)
{
    const uint8_t                *curr_buff;
    uint32_t                      curr_buff_size;
    uint32_t                      tlv_len;
    uint32_t                      mandatory_found_mask = 0;
    uint32_t                      optional_found_mask = 0;
    uint32_t                      emad_tlv_type;
    uint32_t                      type_flag;
    sxd_emad_tlv_markers_status_e ret = SXD_EMAD_TLV_MARKERS_STATUS_SUCCESS_E;

    memset(tlv_markers, 0, sizeof(*tlv_markers));

    curr_buff = buff;
    curr_buff_size = buff_size;
    if (flags & SXD_EMAD_TLV_MARKERS_F_ETH_HDR_EXISTS) {
        if (curr_buff_size < sizeof(sxd_emad_eth_header_t)) {
            ret = SXD_EMAD_TLV_MARKERS_STATUS_BUFFER_TOO_SHORT_E;
            goto out;
        }

        curr_buff += sizeof(sxd_emad_eth_header_t);
        curr_buff_size -= sizeof(sxd_emad_eth_header_t);
    }

    while (!tlv_markers->end_tlv && curr_buff_size >= sizeof(sxd_emad_tlv_t)) {
        emad_tlv_type = sxd_emad_tlv_type_get(curr_buff);
        tlv_len = sxd_emad_tlv_len_get(curr_buff);

        switch (emad_tlv_type) {
        case SXD_EMAD_TLV_TYPE_OPERATION_E:
            tlv_markers->operation_tlv = (sxd_emad_operation_tlv_t*)curr_buff;
            if (tlv_len < sizeof(sxd_emad_operation_tlv_t)) {
                ret = SXD_EMAD_TLV_MARKERS_STATUS_BUFFER_TOO_SHORT_E;
                goto out;
            }
            break;

        case SXD_EMAD_TLV_TYPE_STRING_E:
            tlv_markers->string_tlv = (sxd_emad_string_tlv_t*)curr_buff;
            if (tlv_len < sizeof(sxd_emad_string_tlv_t)) {
                ret = SXD_EMAD_TLV_MARKERS_STATUS_BUFFER_TOO_SHORT_E;
                goto out;
            }
            break;

        case SXD_EMAD_TLV_TYPE_LATENCY_E:
            tlv_markers->latency_tlv = (sxd_emad_latency_tlv_t*)curr_buff;
            if (tlv_len < sizeof(sxd_emad_latency_tlv_t)) {
                ret = SXD_EMAD_TLV_MARKERS_STATUS_BUFFER_TOO_SHORT_E;
                goto out;
            }
            break;

        case SXD_EMAD_TLV_TYPE_INDEX_E:
            tlv_markers->index_tlv = (sxd_emad_index_tlv_t*)curr_buff;
            tlv_markers->indexation_tlv = (void*)curr_buff;
            if (tlv_len < sizeof(sxd_emad_index_tlv_t)) {
                ret = SXD_EMAD_TLV_MARKERS_STATUS_BUFFER_TOO_SHORT_E;
                goto out;
            }
            break;

        case SXD_EMAD_TLV_TYPE_MULTI_INDEX_E:
            tlv_markers->multi_index_tlv = (sxd_emad_multi_index_tlv_t*)curr_buff;
            tlv_markers->indexation_tlv = (void*)curr_buff;
            if (tlv_len < sizeof(sxd_emad_multi_index_tlv_t)) {
                ret = SXD_EMAD_TLV_MARKERS_STATUS_BUFFER_TOO_SHORT_E;
                goto out;
            }
            break;

        case SXD_EMAD_TLV_TYPE_REG_E:
            tlv_markers->reg_tlv = (sxd_emad_reg_tlv_t*)curr_buff;
            tlv_markers->reg_data = (void*)tlv_markers->reg_tlv->reg_data;
            if (tlv_len < sizeof(sxd_emad_reg_tlv_t)) {
                ret = SXD_EMAD_TLV_MARKERS_STATUS_BUFFER_TOO_SHORT_E;
                goto out;
            }
            break;

        case SXD_EMAD_TLV_TYPE_END_E:
            if (tlv_len == 0) { /* sometimes END_TLV is missing (instead, there are trailing 0s
                                 *  [END_TLV type is 0]). */
                goto out;
            }

            tlv_markers->end_tlv = (sxd_emad_end_tlv_t*)curr_buff;
            if (tlv_len < sizeof(sxd_emad_end_tlv_t)) {
                ret = SXD_EMAD_TLV_MARKERS_STATUS_BUFFER_TOO_SHORT_E;
                goto out;
            }
            break;

        default:
            ret = SXD_EMAD_TLV_MARKERS_STATUS_BAD_TLV_TYPE_E;
            goto out;
        }

        type_flag = 1 << emad_tlv_type;

        if (!(type_flag & (mandatory_tlv_mask | optional_tlv_mask))) {
            ret = SXD_EMAD_TLV_MARKERS_STATUS_NOT_EXPECTED_E;
            goto out;
        }

        SXD_EMAD_CHECK_TLV_FLAGS(ret, type_flag, mandatory_tlv_mask, mandatory_found_mask);
        SXD_EMAD_CHECK_TLV_FLAGS(ret, type_flag, optional_tlv_mask, optional_found_mask);

        if (tlv_len > curr_buff_size) {
            ret = SXD_EMAD_TLV_MARKERS_STATUS_BUFFER_TOO_SHORT_E;
            goto out;
        }

        curr_buff += tlv_len;
        curr_buff_size -= tlv_len;
    }

    /* check that all mandatory TLVs are there */
    if (mandatory_tlv_mask != mandatory_found_mask) {
        return SXD_EMAD_TLV_MARKERS_STATUS_MANDATORY_HEADER_MISSING_E;
    }

out:
    return ret;
}
#ifdef __KERNEL__
EXPORT_SYMBOL(sxd_emad_tlv_markers_get);
#endif

sxd_emad_multi_reg_parser_status_e sxd_emad_multi_reg_parse(const uint8_t                 *buff,
                                                            uint32_t                       buff_size,
                                                            sxd_emad_multi_reg_parser_cb_t parser_cb,
                                                            void                          *parser_cb_context)
{
    const uint8_t                     *curr_buff;
    uint32_t                           curr_buff_size;
    sxd_emad_tlv_types_e               tlv_type;
    uint16_t                           tlv_len;
    sxd_emad_multi_reg_parser_status_e ret = SXD_EMAD_MULTI_REG_PARSER_STATUS_SUCCESS_E;

    if (buff_size < sizeof(sxd_emad_eth_header_t) + sizeof(sxd_emad_operation_tlv_t)) {
        ret = SXD_EMAD_MULTI_REG_PARSER_STATUS_BAD_FRAME_E;
        goto out;
    }

    curr_buff = buff + sizeof(sxd_emad_eth_header_t);
    curr_buff_size = buff_size - sizeof(sxd_emad_eth_header_t);

    ret = parser_cb(SXD_EMAD_TLV_TYPE_OPERATION_E,
                    sizeof(sxd_emad_operation_tlv_t),
                    curr_buff,
                    parser_cb_context);
    if (ret != SXD_EMAD_MULTI_REG_PARSER_STATUS_SUCCESS_E) {
        goto out;
    }

    curr_buff += sizeof(sxd_emad_operation_tlv_t);
    curr_buff_size -= sizeof(sxd_emad_operation_tlv_t);

    /* time to parse the registers until END_TLV */
    while (curr_buff_size >= sizeof(sxd_emad_tlv_t)) {
        tlv_type = sxd_emad_tlv_type_get(curr_buff);
        tlv_len = sxd_emad_tlv_len_get(curr_buff);

        if (tlv_len > curr_buff_size) {
            ret = SXD_EMAD_MULTI_REG_PARSER_STATUS_BAD_FRAME_E;
            goto out;
        }

        if (tlv_type == SXD_EMAD_TLV_TYPE_END_E) {
            break;
        }

        ret = parser_cb(tlv_type, tlv_len, curr_buff, parser_cb_context);
        if (ret != SXD_EMAD_MULTI_REG_PARSER_STATUS_SUCCESS_E) {
            return ret;
        }

        curr_buff += tlv_len;
        curr_buff_size -= tlv_len;
    }

out:
    return ret;
}
#ifdef __KERNEL__
EXPORT_SYMBOL(sxd_emad_multi_reg_parse);
#endif

/* get TLV type */
uint8_t sxd_emad_tlv_type_get(const void *tlv)
{
    uint32_t tlv_dword = be32_to_cpu(*(uint32_t*)tlv);

    return (tlv_dword >> 27); /* 5 MSBs */
}
#ifdef __KERNEL__
EXPORT_SYMBOL(sxd_emad_tlv_type_get);
#endif

/* get TLV length in bytes */
uint16_t sxd_emad_tlv_len_get(const void *tlv)
{
    uint32_t tlv_dword = be32_to_cpu(*(uint32_t*)tlv);
    uint8_t  tlv_type = tlv_dword >> 27;
    uint16_t tlv_len = (tlv_dword >> 16) & 0x7ff;

    if (tlv_type == SXD_EMAD_TLV_TYPE_REG_E) { /* need to add the 'elen' field of REG_TLV */
        tlv_len |= ((uint16_t)(tlv_dword & 0xf)) << 11;
    }

    return tlv_len * 4;
}
#ifdef __KERNEL__
EXPORT_SYMBOL(sxd_emad_tlv_len_get);
#endif

/* build a TLV header */
void sxd_emad_tlv_build(void *tlv, sxd_emad_tlv_types_e type, uint16_t length)
{
    uint32_t *tlv_dword = (uint32_t*)tlv;

    length >>= 2; /* moving from bytes to dwords */

    /* constructing type, length LSBs (11bits), length MSBs (elen, 4bits) */
    *tlv_dword = cpu_to_be32((((uint32_t)type) << 27) |
                             (((uint32_t)length & 0x7ff) << 16) |
                             (((uint32_t)length >> 11)));
}
#ifdef __KERNEL__
EXPORT_SYMBOL(sxd_emad_tlv_build);
#endif

/* build EMAD Ethernet header */
void sxd_emad_eth_header_build(void *buff)
{
    uint64_t *hdr = (uint64_t*)buff;

    hdr[0] = 0x0200010000c90201ULL;
    hdr[1] = 0x00003289030201c9ULL;
}
#ifdef __KERNEL__
EXPORT_SYMBOL(sxd_emad_eth_header_build);
#endif

uint8_t sxd_emad_operation_tlv_status_get(const sxd_emad_operation_tlv_t *op_tlv)
{
    return op_tlv->dr_status & 0x7f;
}
#ifdef __KERNEL__
EXPORT_SYMBOL(sxd_emad_operation_tlv_status_get);
#endif

uint16_t sxd_emad_operation_tlv_reg_id_get(const sxd_emad_operation_tlv_t *op_tlv)
{
    return be16_to_cpu(op_tlv->register_id);
}
#ifdef __KERNEL__
EXPORT_SYMBOL(sxd_emad_operation_tlv_reg_id_get);
#endif

uint8_t sxd_emad_operation_tlv_method_get(const sxd_emad_operation_tlv_t *op_tlv)
{
    return op_tlv->r_method & 0x7f;
}
#ifdef __KERNEL__
EXPORT_SYMBOL(sxd_emad_operation_tlv_method_get);
#endif

uint64_t sxd_emad_operation_tlv_tid_get(const sxd_emad_operation_tlv_t *op_tlv)
{
    return be64_to_cpu(op_tlv->tid);
}
#ifdef __KERNEL__
EXPORT_SYMBOL(sxd_emad_operation_tlv_tid_get);
#endif

void sxd_emad_operation_tlv_build(sxd_emad_operation_tlv_t *op_tlv,
                                  sxd_emad_method_e         method,
                                  uint16_t                  reg_id,
                                  uint64_t                  tid)
{
    memset(op_tlv, 0, sizeof(*op_tlv));
    sxd_emad_tlv_build(op_tlv, SXD_EMAD_TLV_TYPE_OPERATION_E, sizeof(*op_tlv));
    op_tlv->r_method = method;
    op_tlv->emad_class = 0x1;
    op_tlv->register_id = cpu_to_be16(reg_id);
    op_tlv->tid = cpu_to_be64(tid);
}
#ifdef __KERNEL__
EXPORT_SYMBOL(sxd_emad_operation_tlv_build);
#endif

/* get register length in bytes (net register size, without reg_tlv length) */
uint16_t sxd_emad_reg_tlv_register_len_get(const sxd_emad_reg_tlv_t *reg_tlv)
{
    return sxd_emad_tlv_len_get(reg_tlv) - sizeof(sxd_emad_reg_tlv_t);
}
#ifdef __KERNEL__
EXPORT_SYMBOL(sxd_emad_reg_tlv_register_len_get);
#endif

uint8_t sxd_emad_index_tlv_port_is_global(const sxd_emad_index_tlv_t *idx_tlv)
{
    uint16_t index = be16_to_cpu(idx_tlv->index);

    return index == SXD_EMAD_PORT_IDX_GLOBAL;
}
#ifdef __KERNEL__
EXPORT_SYMBOL(sxd_emad_index_tlv_port_is_global);
#endif

void sxd_emad_index_tlv_port_global_build(sxd_emad_index_tlv_t *index_tlv)
{
    sxd_emad_tlv_build(index_tlv, SXD_EMAD_TLV_TYPE_INDEX_E, sizeof(sxd_emad_index_tlv_t));
    index_tlv->index = cpu_to_be16(SXD_EMAD_PORT_IDX_GLOBAL);
}
#ifdef __KERNEL__
EXPORT_SYMBOL(sxd_emad_index_tlv_port_global_build);
#endif

void sxd_emad_index_tlv_port_single_build(sxd_emad_index_tlv_t *index_tlv, uint8_t lp_msb, uint8_t lp_lsb)
{
    uint16_t local_port;

    SX_PORT_BUILD_PHY_ID_FROM_LSB_MSB(local_port, lp_lsb, lp_msb);
    sxd_emad_tlv_build(index_tlv, SXD_EMAD_TLV_TYPE_INDEX_E, sizeof(sxd_emad_index_tlv_t));
    index_tlv->index = cpu_to_be16(local_port);
}
#ifdef __KERNEL__
EXPORT_SYMBOL(sxd_emad_index_tlv_port_single_build);
#endif

void sxd_emad_index_tlv_port_single_deparse(const sxd_emad_index_tlv_t *idx_tlv, uint8_t *lp_msb, uint8_t *lp_lsb)
{
    uint16_t index = be16_to_cpu(idx_tlv->index);

    *lp_msb = index >> 8;
    *lp_lsb = index & 0xff;
}
#ifdef __KERNEL__
EXPORT_SYMBOL(sxd_emad_index_tlv_port_single_deparse);
#endif

void sxd_emad_multi_index_tlv_port_build(sxd_emad_multi_index_tlv_t *multi_index_tlv,
                                         uint16_t                    base_port,
                                         const uint32_t             *port_bitmap_p,
                                         uint32_t                    port_bitmap_size_in_dwords)
{
    uint32_t i;

    sxd_emad_tlv_build(multi_index_tlv,
                       SXD_EMAD_TLV_TYPE_MULTI_INDEX_E,
                       sizeof(sxd_emad_multi_index_tlv_t) + port_bitmap_size_in_dwords * sizeof(uint32_t));
    multi_index_tlv->base_index = cpu_to_be16(base_port);

    for (i = 0; i < port_bitmap_size_in_dwords; i++) {
        multi_index_tlv->bitmap[i] = cpu_to_be32(port_bitmap_p[i]);
    }
}
#ifdef __KERNEL__
EXPORT_SYMBOL(sxd_emad_multi_index_tlv_port_build);
#endif

uint32_t sxd_emad_multi_index_tlv_port_deparse(const sxd_emad_multi_index_tlv_t *multi_idx_tlv,
                                               uint32_t                         *port_bitmap_p,
                                               uint32_t                          port_bitmap_size_in_dwords)
{
    uint32_t tlv_dwords = 0;
    uint32_t bitmap_dwords = 0;
    uint32_t copy_dwords = 0;
    uint32_t i;

    tlv_dwords = sxd_emad_tlv_len_get(multi_idx_tlv) / sizeof(uint32_t);
    bitmap_dwords = tlv_dwords - sizeof(sxd_emad_multi_index_tlv_t) / sizeof(uint32_t);
    if (bitmap_dwords < port_bitmap_size_in_dwords) {
        copy_dwords = bitmap_dwords;
    } else {
        copy_dwords = port_bitmap_size_in_dwords;
    }

    for (i = 0; i < copy_dwords; i++) {
        port_bitmap_p[i] = be32_to_cpu(multi_idx_tlv->bitmap[i]);
    }

    return copy_dwords;
}
#ifdef __KERNEL__
EXPORT_SYMBOL(sxd_emad_multi_index_tlv_port_deparse);
#endif
