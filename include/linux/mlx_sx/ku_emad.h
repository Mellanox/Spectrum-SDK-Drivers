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

#ifndef __KU_EMAD_H__
#define __KU_EMAD_H__

#ifdef __KERNEL__

#include <linux/types.h>
#define SXD_BOOL_TRUE  (1)
#define SXD_BOOL_FALSE (0)

/* on kernel side, SXD_DYNAMIC_REG_ON_STACK is not defined as stack size is quite limited */
#define SXD_DYNAMIC_REG_ALLOC(reg_struct, \
                              extra_size_bytes) ((reg_struct*)vmalloc(sizeof(reg_struct) + (extra_size_bytes)))

#else /* __KERNEL__ */

#include <complib/mlxsw_item.h>
#define SXD_BOOL_TRUE  (TRUE)
#define SXD_BOOL_FALSE (FALSE)

#define SXD_DYNAMIC_REG_ON_STACK(reg_struct, var_name, extra_size_bytes)            \
    uint8_t __reg_buff_ ## var_name ## __[sizeof(reg_struct) + (extra_size_bytes)]; \
    reg_struct *var_name = (reg_struct*)__reg_buff_ ## var_name ## __
#define SXD_DYNAMIC_REG_ALLOC(reg_struct, \
                              extra_size_bytes) ((reg_struct*)cl_calloc(1, sizeof(reg_struct) + (extra_size_bytes)))

#endif /* __KERNEL__ */

typedef uint8_t sxd_boolean_t;

#define SXD_EMAD_PORT_IDX_GLOBAL (0xfffe) /* PRM value */

typedef enum { /* PRM values */
    SXD_EMAD_METHOD_INVALID_E = 0,
    SXD_EMAD_METHOD_QUERY_E   = 1,
    SXD_EMAD_METHOD_WRITE_E   = 2,
    SXD_EMAD_METHOD_EVENT_E   = 5
} sxd_emad_method_e;

typedef enum { /* PRM values */
    SXD_EMAD_TLV_TYPE_END_E         = 0,
    SXD_EMAD_TLV_TYPE_OPERATION_E   = 1,
    SXD_EMAD_TLV_TYPE_STRING_E      = 2,
    SXD_EMAD_TLV_TYPE_REG_E         = 3,
    SXD_EMAD_TLV_TYPE_LATENCY_E     = 4,
    SXD_EMAD_TLV_TYPE_METADATA_E    = 6,
    SXD_EMAD_TLV_TYPE_INDEX_E       = 7,
    SXD_EMAD_TLV_TYPE_MULTI_INDEX_E = 8
} sxd_emad_tlv_types_e;

typedef enum { /* PRM values */
    SXD_EMAD_STATUS_OK_E                         = 0x00,
    SXD_EMAD_STATUS_BUSY_E                       = 0x01,
    SXD_EMAD_STATUS_BAD_BASE_VER_E               = 0x02,
    SXD_EMAD_STATUS_BAD_TLV_E                    = 0x03,
    SXD_EMAD_STATUS_NOT_SUPP_REG_E               = 0x04,
    SXD_EMAD_STATUS_NOT_SUPP_CLASS_E             = 0x05,
    SXD_EMAD_STATUS_NOT_SUPP_METHOD_E            = 0x06,
    SXD_EMAD_STATUS_BAD_PARAM_E                  = 0x07,
    SXD_EMAD_STATUS_RESOURCE_NOT_AVAILABLE_E     = 0x08,
    SXD_EMAD_STATUS_LONG_PROCESS_E               = 0x09,
    SXD_EMAD_STATUS_RETRY_E                      = 0x0a,
    SXD_EMAD_STATUS_PERIPHERAL_ERR_E             = 0x0b,
    SXD_EMAD_STATUS_PROFILE_NO_MEMORY_E          = 0x0c,
    SXD_EMAD_STATUS_ISSU_ONGOING_E               = 0x0d,
    SXD_EMAD_STATUS_NOT_SUPPORTED_BY_SECONDARY_E = 0x0e,
    SXD_EMAD_STATUS_BAD_CONFIG_E                 = 0x20,
    SXD_EMAD_STATUS_ERASE_EXCEEDED_E             = 0x21,
    SXD_EMAD_STATUS_ERROR_E                      = 0x22,
    SXD_EMAD_STATUS_REG_LENGTH_TOO_SMALL_E       = 0x24,
    SXD_EMAD_STATUS_REG_ERROR_E                  = 0x25,
    SXD_EMAD_STATUS_NOT_EXECUTED_E               = 0x26,
    SXD_EMAD_STATUS_REVERTED_E                   = 0x27,
    SXD_EMAD_STATUS_INTERNAL_ERR_E               = 0x70,
    SXD_EMAD_STATUS_DROP_E                       = 0x7f
} sxd_emad_status_e;

/**
 * sxd_emad_header_t structure is used to store EMAD header layout.
 */
typedef struct {
    uint8_t dmac[6];
    uint8_t smac[6];
    __be16  ethertype;
    uint8_t mlx_proto;
    uint8_t ver;
} __attribute__((packed)) sxd_emad_eth_header_t;

/**
 * sxd_emad_operation_t structure is used to store EMAD operation
 * layout.
 */
typedef struct {
    __be16  type_len;
    uint8_t dr_status;
    uint8_t reserved1;
    __be16  register_id;
    uint8_t r_method;
    uint8_t emad_class;
    __be64  tid;
} __attribute__((packed)) sxd_emad_operation_tlv_t;

typedef struct {
    sxd_emad_eth_header_t    eth_hdr;
    sxd_emad_operation_tlv_t emad_op;
} __attribute__((packed)) sxd_emad_mandatory_headers_t;

/**
 * sxd_emad_string_t structure is used to store EMAD string (in case of error)
 * layout.
 */
typedef struct {
    __be16  type_len;
    uint8_t reserved[2];
    char    string[128];
} __attribute__((packed)) sxd_emad_string_tlv_t;

/**
 * sxd_emad_latency_t structure is used to store EMAD latency layout
 */
typedef struct {
    __be16  type_len;
    uint8_t reserved1;
    uint8_t ccmc_v;
    __be32  latency_time;
    __be32  code_cache_miss_count;
    __be32  total_latency_time;
    __be32  code_cache_miss_latency_time;
    __be32  reserved[2];
} __attribute__((packed)) sxd_emad_latency_tlv_t;

/**
 * sxd_emad_index_t structure is used to store EMAD index layout
 * PRM: "Index TLV"
 */
typedef struct {
    __be16 type_len;
    __be16 index;
} __attribute__((packed)) sxd_emad_index_tlv_t;

/**
 * sxd_emad_multi_index_t structure is used to store EMAD index layout
 * PRM: "Multi Index TLV"
 */
typedef struct {
    __be16 type_len;
    __be16 base_index;
    __be32 bitmap[];
} __attribute__((packed)) sxd_emad_multi_index_tlv_t;

/**
 * sxd_emad_reg_t structure is used to store EMAD register layout.
 */
typedef struct {
    __be16  type_len;
    uint8_t reserved1;
    uint8_t elen;
    uint8_t reg_data[];
} __attribute__((packed)) sxd_emad_reg_tlv_t;

typedef struct {
    __be16  type_len;
    uint8_t reserved1;
    uint8_t metadata_id;
} __attribute__((packed)) sxd_emad_metadata_tlv_t;

typedef enum {
    SXD_EMAD_METADATA_ID_INVALID_E,
    SXD_EMAD_METADATA_ID_ITER_E,
    SXD_EMAD_METADATA_ID_ITER_TS_E
} sxd_emad_metadata_id_e;

typedef struct {
    __be32 iteration;
} __attribute__((packed)) sxd_emad_metadata_iter_t;

typedef struct {
    __be32 iteration;
    __be32 utc_sec;
    __be32 utc_nsec;
} __attribute__((packed)) sxd_emad_metadata_iter_ts_t;

/**
 * sxd_emad_end_t structure is used to store EMAD end layout.
 */
typedef struct {
    __be16 type_len;
    __be16 reserved1;
} __attribute__((packed)) sxd_emad_end_tlv_t;

/**
 * sxd_emad_tlv_t structure to represent generic TLV.
 */
typedef struct {
    __be16 type_len;
    __be16 reserved;
} __attribute__((packed)) sxd_emad_tlv_t;

/**
 * sxd_emad_tlv_headers_t structure is for TLV markers of an EMAD buffer
 */
typedef struct {
    sxd_emad_operation_tlv_t   *operation_tlv;
    sxd_emad_string_tlv_t      *string_tlv;
    sxd_emad_latency_tlv_t     *latency_tlv;
    sxd_emad_index_tlv_t       *index_tlv;
    sxd_emad_multi_index_tlv_t *multi_index_tlv;
    sxd_emad_reg_tlv_t         *reg_tlv;
    sxd_emad_end_tlv_t         *end_tlv;
    sxd_emad_metadata_tlv_t    *metadata_tlv;

    /* helper pointers */
    void *reg_data;                       /* register layout (right after REG_TLV) */
    void *indexation_tlv;                       /* the pointer to the indexation TLV:
                                                 *  index TLV or multi-index TLV (NULL if no indexation header) */
} sxd_emad_tlv_markers_t;

#define SXD_EMAD_TLV_MARKERS_F_ETH_HDR_EXISTS (1 << 0)

#define SXD_EMAD_TLV_MARKERS_F_END         (1 << SXD_EMAD_TLV_TYPE_END_E)
#define SXD_EMAD_TLV_MARKERS_F_OPERATION   (1 << SXD_EMAD_TLV_TYPE_OPERATION_E)
#define SXD_EMAD_TLV_MARKERS_F_STRING      (1 << SXD_EMAD_TLV_TYPE_STRING_E)
#define SXD_EMAD_TLV_MARKERS_F_REG         (1 << SXD_EMAD_TLV_TYPE_REG_E)
#define SXD_EMAD_TLV_MARKERS_F_LATENCY     (1 << SXD_EMAD_TLV_TYPE_LATENCY_E)
#define SXD_EMAD_TLV_MARKERS_F_METADATA    (1 << SXD_EMAD_TLV_TYPE_METADATA_E)
#define SXD_EMAD_TLV_MARKERS_F_INDEX       (1 << SXD_EMAD_TLV_TYPE_INDEX_E)
#define SXD_EMAD_TLV_MARKERS_F_MULTI_INDEX (1 << SXD_EMAD_TLV_TYPE_MULTI_INDEX_E)

#define SXD_EMAD_TLV_MARKERS_DEFAULT_MANDATORY_MASK \
    (SXD_EMAD_TLV_MARKERS_F_OPERATION |             \
     SXD_EMAD_TLV_MARKERS_F_REG)
#define SXD_EMAD_TLV_MARKERS_DEFAULT_OPTIONAL_MASK \
    (SXD_EMAD_TLV_MARKERS_F_STRING |               \
     SXD_EMAD_TLV_MARKERS_F_LATENCY |              \
     SXD_EMAD_TLV_MARKERS_F_INDEX |                \
     SXD_EMAD_TLV_MARKERS_F_MULTI_INDEX |          \
     SXD_EMAD_TLV_MARKERS_F_END)

typedef enum {
    SXD_EMAD_TLV_MARKERS_STATUS_SUCCESS_E,
    SXD_EMAD_TLV_MARKERS_STATUS_BUFFER_TOO_SHORT_E,
    SXD_EMAD_TLV_MARKERS_STATUS_NO_EMAD_FRAME_E,
    SXD_EMAD_TLV_MARKERS_STATUS_BAD_TLV_TYPE_E,
    SXD_EMAD_TLV_MARKERS_STATUS_MANDATORY_HEADER_MISSING_E,
    SXD_EMAD_TLV_MARKERS_STATUS_FOUND_TWICE_E,
    SXD_EMAD_TLV_MARKERS_STATUS_NOT_EXPECTED_E
} sxd_emad_tlv_markers_status_e;

typedef enum {
    SXD_EMAD_MULTI_REG_PARSER_STATUS_SUCCESS_E,
    SXD_EMAD_MULTI_REG_PARSER_STATUS_CB_STOPPED_ITER_E,
    SXD_EMAD_MULTI_REG_PARSER_STATUS_BAD_FRAME_E,
} sxd_emad_multi_reg_parser_status_e;

/*************************************************
 * FUNCTIONS
 ************************************************/

/* misc functions */
sxd_emad_tlv_markers_status_e sxd_emad_tlv_markers_get(const uint8_t          *buff,
                                                       uint32_t                buff_size,
                                                       uint32_t                flags,
                                                       uint32_t                mandatory_tlv_mask,
                                                       uint32_t                optional_tlv_mask,
                                                       sxd_emad_tlv_markers_t *tlv_markers);
#define SXD_EMAD_TLV_MARKERS_DEFAULT_GET(buff, buff_size, tlv_markers)    \
    sxd_emad_tlv_markers_get((buff),                                      \
                             (buff_size),                                 \
                             SXD_EMAD_TLV_MARKERS_F_ETH_HDR_EXISTS,       \
                             SXD_EMAD_TLV_MARKERS_DEFAULT_MANDATORY_MASK, \
                             SXD_EMAD_TLV_MARKERS_DEFAULT_OPTIONAL_MASK,  \
                             (tlv_markers))

typedef sxd_emad_multi_reg_parser_status_e (*sxd_emad_multi_reg_parser_cb_t)(sxd_emad_tlv_types_e tlv_type,
                                                                             uint16_t             tlv_len,
                                                                             const void          *tlv,
                                                                             void                *context);

sxd_emad_multi_reg_parser_status_e sxd_emad_multi_reg_parse(const uint8_t                 *buff,
                                                            uint32_t                       buff_size,
                                                            sxd_emad_multi_reg_parser_cb_t parser_cb,
                                                            void                          *parser_cb_context);

/* generic TLV functions */
uint8_t sxd_emad_tlv_type_get(const void *tlv);
uint16_t sxd_emad_tlv_len_get(const void *tlv);
void sxd_emad_tlv_build(void                *tlv,
                        sxd_emad_tlv_types_e type,
                        uint16_t             length);

/* EMAD Ethernet header functions */
void sxd_emad_eth_header_build(void *buff);

/* Operation TLV functions */
uint8_t sxd_emad_operation_tlv_status_get(const sxd_emad_operation_tlv_t *op_tlv);
uint16_t sxd_emad_operation_tlv_reg_id_get(const sxd_emad_operation_tlv_t *op_tlv);
uint8_t sxd_emad_operation_tlv_method_get(const sxd_emad_operation_tlv_t *op_tlv);
uint64_t sxd_emad_operation_tlv_tid_get(const sxd_emad_operation_tlv_t *op_tlv);
void sxd_emad_operation_tlv_build(sxd_emad_operation_tlv_t *op_tlv,
                                  sxd_emad_method_e         method,
                                  uint16_t                  reg_id,
                                  uint64_t                  tid);

/* Reg TLV functions */
uint16_t sxd_emad_reg_tlv_register_len_get(const sxd_emad_reg_tlv_t *reg_tlv);

/* Index TLV functions */
uint8_t sxd_emad_index_tlv_port_is_global(const sxd_emad_index_tlv_t *idx_tlv);
void sxd_emad_index_tlv_port_global_build(sxd_emad_index_tlv_t *index_tlv);
void sxd_emad_index_tlv_port_single_build(sxd_emad_index_tlv_t *index_tlv,
                                          uint8_t               lp_msb,
                                          uint8_t               lp_lsb);
void sxd_emad_index_tlv_port_single_deparse(const sxd_emad_index_tlv_t *idx_tlv,
                                            uint8_t                    *lp_msb,
                                            uint8_t                    *lp_lsb);

/* Multi-Index TLV functions */
void sxd_emad_multi_index_tlv_port_build(sxd_emad_multi_index_tlv_t *multi_index_tlv,
                                         uint16_t                    base_port,
                                         const uint32_t             *port_bitmap_p,
                                         uint32_t                    port_bitmap_size_in_dwords);
uint32_t sxd_emad_multi_index_tlv_port_deparse(const sxd_emad_multi_index_tlv_t *multi_idx_tlv,
                                               uint32_t                         *port_bitmap_p,
                                               uint32_t                          port_bitmap_size_in_dwords);

#endif /* __KU_EMAD_H__ */
