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

#ifndef SX_FW_INTERNAL_H
#define SX_FW_INTERNAL_H

#define SX_PSID_SIZE 16

#define SX_GET(dest, source, offset)            \
    do {                                        \
        void *__p = (char*)(source) + (offset); \
        switch (sizeof(dest)) {                 \
        case 1:                                 \
            (dest) = *(u8*)__p;                 \
            break;                              \
        case 2:                                 \
            (dest) = be16_to_cpup(__p);         \
            break;                              \
        case 4:                                 \
            (dest) = be32_to_cpup(__p);         \
            break;                              \
        case 8:                                 \
            (dest) = be64_to_cpup(__p);         \
            break;                              \
        default:                                \
            break;                              \
        }                                       \
    } while (0)

#define SX_PUT(dest, source, offset)             \
    do {                                         \
        void *__d = ((char*)(dest) + (offset));  \
        switch (sizeof(source)) {                \
        case 1:                                  \
            *(u8*)__d = (source);                \
            break;                               \
        case 2:                                  \
            *(__be16*)__d = cpu_to_be16(source); \
            break;                               \
        case 4:                                  \
            *(__be32*)__d = cpu_to_be32(source); \
            break;                               \
        case 8:                                  \
            *(__be64*)__d = cpu_to_be64(source); \
            break;                               \
        default:                                 \
            break;                               \
        }                                        \
    } while (0)

struct ku_operation_tlv;

typedef int (*access_reg_encode_cb_t)(u8   *inbox,
                                      void *ku_reg,
                                      void *context,
                                      u32   flags,
                                      u32  *bytes_written);
typedef int (*access_reg_decode_cb_t)(sxd_emad_tlv_markers_t *tlv_markers,
                                      void                   *ku_reg,
                                      void                   *context);

#define SX_ACCESS_REG_F_IGNORE_FW_RET_CODE (1 << 0)
#define SX_ACCESS_REG_F_SET_AND_GET        (1 << 1)
#define SX_ACCESS_REG_F_FORCE_WAITING      (1 << 2)
#define SX_ACCESS_REG_F_QUIET_ERRORS       (1 << 3)

#define SX_EMAD_F_SUPPORTED      (1 << 0) /* does device support EMAD? */
#define SX_EMAD_F_STRING_TLV     (1 << 1) /* include string TLV?       */
#define SX_EMAD_F_LATECNY_TLV    (1 << 2) /* include latency TLV?      */
#define SX_EMAD_F_INDEXATION_TLV (1 << 3) /* index/multi-index TLV supported? */

int sx_ACCESS_REG_internal(struct sx_dev            *dev,
                           uint8_t                   dev_id,
                           uint32_t                  flags,
                           sxd_emad_operation_tlv_t *op_tlv,
                           access_reg_encode_cb_t    reg_encode_cb,
                           access_reg_decode_cb_t    reg_decode_cb,
                           void                     *ku_reg,
                           void                     *context);

void set_operation_tlv(void *inbox, const sxd_emad_operation_tlv_t *op_tlv);
void get_operation_tlv(const void *outbox, sxd_emad_operation_tlv_t *op_tlv);

#endif /* SX_FW_INTERNAL_H */
