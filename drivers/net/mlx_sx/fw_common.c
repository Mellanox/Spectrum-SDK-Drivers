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
#include "fw_internal.h"
#include <linux/mlx_sx/kernel_user.h>
#include <linux/mlx_sx/ku_emad.h>

void set_operation_tlv(void *inbox, const sxd_emad_operation_tlv_t *op_tlv)
{
    memcpy(inbox, op_tlv, sizeof(*op_tlv));
}

void get_operation_tlv(const void *outbox, sxd_emad_operation_tlv_t *op_tlv)
{
    u8  tlv_type;
    u16 tlv_len;

    memcpy(op_tlv, outbox, sizeof(*op_tlv));

    /* validate TLV header integrity */
    tlv_len = sxd_emad_tlv_len_get(op_tlv);
    if (tlv_len != sizeof(*op_tlv)) {
        sxd_log_err(SXD_DEV_NONE, "ERROR: Operation TLV is invalid: length=%u\n", tlv_len);
        print_hex_dump(KERN_ERR, "OP_TLV ", DUMP_PREFIX_OFFSET, 16, 1, outbox, 16, 0);
        dump_stack();
        return;
    }

    /* validate TLV header integrity */
    tlv_type = sxd_emad_tlv_type_get(op_tlv);
    if (tlv_type != SXD_EMAD_TLV_TYPE_OPERATION_E) {
        sxd_log_err(SXD_DEV_NONE, "ERROR: Operation TLV is invalid: type=%u\n", tlv_type);
        print_hex_dump(KERN_ERR, "OP_TLV ", DUMP_PREFIX_OFFSET, 16, 1, outbox, 16, 0);
        dump_stack();
        return;
    }
}
