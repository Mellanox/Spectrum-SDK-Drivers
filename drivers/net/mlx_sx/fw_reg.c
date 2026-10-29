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
#include "emad.h"
#include "dev_init.h"
#include <linux/mlx_sx/cmd.h>
#include <linux/mlx_sx/driver.h>
#include <linux/mlx_sx/auto_registers/cmd_auto.h>

#include <linux/types.h>
#include <linux/string.h>
#include <linux/bitops.h>
#include <linux/module.h>

static int raw_reg_access_verbose = 0;
module_param_named(raw_reg_access_verbose, raw_reg_access_verbose, int, 0644);
MODULE_PARM_DESC(raw_reg_access_verbose, "Force error-level logging for RAW register access failures (default: 0)");

#define REG_LOCAL_PORT_OFFSET 0x01

/**
 * Send CMD-IFC register sniffer notification to userspace.
 * This function sends register data to userspace for PRM sniffer recording.
 *
 * @param dev          Device pointer
 * @param reg_id       Register ID
 * @param method       EMAD method (1=query, 2=write)
 * @param is_tx        1 for TX (before cmd), 0 for RX (after cmd)
 * @param reg_data     Register data buffer (PRM layout)
 * @param reg_len      Register data length in bytes
 */
static void __cmd_ifc_sniffer_send(struct sx_dev *dev,
                                   u16            reg_id,
                                   u8             method,
                                   u8             is_tx,
                                   const void    *reg_data,
                                   u16            reg_len)
{
    sxd_cmd_ifc_reg_sniffer_notification_t *notif_p = NULL;
    struct sx_timeval                       now;
    size_t                                  alloc_size;
    u16                                     copy_len;

    copy_len = min_t(u16, reg_len, SXD_CMD_IFC_REG_DATA_MAX);
    alloc_size = sizeof(*notif_p) + copy_len;

    notif_p = kzalloc(alloc_size, GFP_KERNEL);
    if (!notif_p) {
        goto out;
    }

    notif_p->device_id = dev->device_id;
    notif_p->reg_id = reg_id;
    notif_p->method = method;
    notif_p->is_tx = is_tx;
    notif_p->reg_len = copy_len;

    sx_dbg_get_time(&now);
    notif_p->timestamp.tv_sec = now.t.tv_sec;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    notif_p->timestamp.tv_nsec = now.t.tv_nsec;
#else
    notif_p->timestamp.tv_nsec = 1000 * now.t.tv_usec;
#endif

    if (reg_data && (copy_len > 0)) {
        memcpy(notif_p->reg_data, reg_data, copy_len);
    }

    send_trap(notif_p,
              alloc_size,
              SXD_TRAP_ID_CMD_IFC_REG_SNIFFER,
              0,
              dev->device_id,
              TARGET_PID_DONT_CARE,
              GFP_KERNEL);

out:
    kfree(notif_p);
}

/*
 * Create a contiguous bitmask starting at bit position @l and ending at
 * position @h. For example
 * GEN_MASK_ULL(39, 21) gives us the 64bit vector 0x000000ffffe00000.
 */
#define GEN_MASK(h, l) \
    (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#define GEN_MASK_ULL(h, l) \
    (((~0ULL) - (1ULL << (l)) + 1) & (~0ULL >> (BITS_PER_LONG_LONG - 1 - (h))))

struct mlxsw_item {
    unsigned short offset;           /* bytes in container */
    unsigned short step;           /* step in bytes for indexed items */
    unsigned short in_step_offset;    /* offset within one step */
    unsigned char  shift;          /* shift in bits */
    unsigned char  element_size;      /* size of element in bit array */
    union {
        unsigned char  bits;
        unsigned short bytes;
    } size;
    const char *name;
};

/* returns 0 for big endian, 1 for little endian. */
static inline unsigned int __little_endian_order(void)
{
    volatile u32 i = 0xff;

    return (*((u8*)(&i))) == 0xff;
}

static inline unsigned int __mlxsw_item_offset(const struct mlxsw_item *item,
                                               unsigned short           outer_index,
                                               unsigned short           index,
                                               size_t                   typesize)
{
    BUG_ON(index && !item->step);
    if ((item->offset % typesize != 0) ||
        (item->step % typesize != 0) ||
        (item->in_step_offset % typesize != 0)) {
        sxd_log_err(SXD_DEV_NONE, "mlxsw: item bug (name=%s,offset=%x,step=%x,in_step_offset=%x,typesize=%zx)\n",
                    item->name, item->offset, item->step,
                    item->in_step_offset, typesize);
        BUG();
    }

    return ((item->offset + outer_index * typesize + (unsigned int)item->step * index + item->in_step_offset) /
            typesize);
}

static inline unsigned int __mlxsw_item_offset_no_checks(const struct mlxsw_item *item,
                                                         unsigned short           outer_index,
                                                         unsigned short           index,
                                                         size_t                   typesize)
{
    BUG_ON(index && !item->step);

    return ((item->offset + outer_index * typesize + (unsigned int)item->step * index + item->in_step_offset) /
            typesize);
}

static inline u8 __mlxsw_item_get8(const char              *buf,
                                   const struct mlxsw_item *item,
                                   unsigned short           outer_index,
                                   unsigned short           index)
{
    unsigned int offset = __mlxsw_item_offset(item, outer_index, index, sizeof(u8));
    u8          *b = (u8 *)buf;
    u8           tmp;

    tmp = b[offset];
    tmp >>= item->shift;
    tmp &= GEN_MASK(item->size.bits - 1, 0);
    return tmp;
}

static inline void __mlxsw_item_set8(char                    *buf,
                                     const struct mlxsw_item *item,
                                     unsigned short           outer_index,
                                     unsigned short           index,
                                     u8                       val)
{
    unsigned int offset = __mlxsw_item_offset(item, outer_index, index, sizeof(u8));
    u8          *b = (u8 *)buf;
    u8           mask = GEN_MASK(item->size.bits - 1, 0) << item->shift;
    u8           tmp;

    val <<= item->shift;
    val &= mask;
    tmp = b[offset];
    tmp &= ~mask;
    tmp |= val;
    b[offset] = tmp;
}

static inline u16 __mlxsw_item_get16(const char              *buf,
                                     const struct mlxsw_item *item,
                                     unsigned short           outer_index,
                                     unsigned short           index)
{
    unsigned int offset = __mlxsw_item_offset(item, outer_index, index, sizeof(u16));
    __be16      *b = (__be16 *)buf;
    u16          tmp;

    tmp = be16_to_cpu(b[offset]);
    tmp >>= item->shift;
    tmp &= GEN_MASK(item->size.bits - 1, 0);
    return tmp;
}

static inline void __mlxsw_item_set16(char                    *buf,
                                      const struct mlxsw_item *item,
                                      unsigned short           outer_index,
                                      unsigned short           index,
                                      u16                      val)
{
    unsigned int offset = __mlxsw_item_offset(item, outer_index, index, sizeof(u16));
    __be16      *b = (__be16 *)buf;
    u16          mask = GEN_MASK(item->size.bits - 1, 0) << item->shift;
    u16          tmp;

    val <<= item->shift;
    val &= mask;
    tmp = be16_to_cpu(b[offset]);
    tmp &= ~mask;
    tmp |= val;
    b[offset] = cpu_to_be16(tmp);
}

static inline u32 __mlxsw_item_get32(const char              *buf,
                                     const struct mlxsw_item *item,
                                     unsigned short           outer_index,
                                     unsigned short           index)
{
    unsigned int offset = __mlxsw_item_offset(item, outer_index, index, sizeof(u32));
    __be32      *b = (__be32 *)buf;
    u32          tmp;

    tmp = be32_to_cpu(b[offset]);
    tmp >>= item->shift;
    tmp &= GEN_MASK(item->size.bits - 1, 0);
    return tmp;
}

static inline void __mlxsw_item_set32(char                    *buf,
                                      const struct mlxsw_item *item,
                                      unsigned short           outer_index,
                                      unsigned short           index,
                                      u32                      val)
{
    unsigned int offset = __mlxsw_item_offset(item, outer_index, index, sizeof(u32));
    __be32      *b = (__be32 *)buf;
    u32          mask = GEN_MASK(item->size.bits - 1, 0) << item->shift;
    u32          tmp;

    val <<= item->shift;
    val &= mask;
    tmp = be32_to_cpu(b[offset]);
    tmp &= ~mask;
    tmp |= val;
    b[offset] = cpu_to_be32(tmp);
}

static inline u64 __mlxsw_item_get64(const char              *buf,
                                     const struct mlxsw_item *item,
                                     unsigned short           outer_index,
                                     unsigned short           index)
{
    size_t       typesize = sizeof(u64);
    unsigned int offset = __mlxsw_item_offset_no_checks(item, outer_index, index, typesize);
    __be64      *b = (__be64 *)buf;
    u64          tmp;
    u32          tmp32;

    if ((item->step % typesize != 0) ||
        (item->in_step_offset % typesize != 0)) {
        sxd_log_err(SXD_DEV_NONE, "mlxsw: item bug (name=%s,offset=%x,step=%x,in_step_offset=%x,typesize=%zx)\n",
                    item->name, item->offset, item->step,
                    item->in_step_offset, typesize);
        BUG();
    }

    if (item->offset % typesize == 0) {
        tmp = be64_to_cpu(b[offset]);
        tmp >>= item->shift;
        tmp &= GEN_MASK_ULL(item->size.bits - 1, 0);
    } else if (item->offset % sizeof(u32) != 0) {
        /* If not aligned to 4 bytes then raise error. */
        sxd_log_err(SXD_DEV_NONE, "mlxsw: item bug (name=%s,offset=%x,step=%x,in_step_offset=%x,typesize=%zx)\n",
                    item->name, item->offset, item->step,
                    item->in_step_offset, typesize);
        BUG();
    } else {
        /* This is a hack to work around not aligned u64 fields.
         *  Emulate access to __be64 *b as it was pair of __be32.
         *  Expect that this is a full 8 byte long field.
         *  Thus no shift is required. */
        offset = __mlxsw_item_offset_no_checks(item, outer_index, index, sizeof(u32));
        if (__little_endian_order()) {
            tmp32 = be32_to_cpu(((__be32 *)b)[offset]);
            tmp = (u64)tmp32 << 32;
            tmp += be32_to_cpu(((__be32 *)b)[offset + 1]);
        } else {
            tmp32 = be32_to_cpu(((__be32 *)b)[offset + 1]);
            tmp = (u64)tmp32 << 32;
            tmp += be32_to_cpu(((__be32 *)b)[offset]);
        }
    }
    return tmp;
}

static inline void __mlxsw_item_set64(char                    *buf,
                                      const struct mlxsw_item *item,
                                      unsigned short           outer_index,
                                      unsigned short           index,
                                      u64                      val)
{
    size_t       typesize = sizeof(u64);
    unsigned int offset = __mlxsw_item_offset_no_checks(item, outer_index, index, typesize);
    __be64      *b = (__be64 *)buf;
    u64          mask = GEN_MASK_ULL(item->size.bits - 1, 0) << item->shift;
    u64          tmp;

    if ((item->step % typesize != 0) ||
        (item->in_step_offset % typesize != 0)) {
        sxd_log_err(SXD_DEV_NONE, "mlxsw: item bug (name=%s,offset=%x,step=%x,in_step_offset=%x,typesize=%zx)\n",
                    item->name, item->offset, item->step,
                    item->in_step_offset, typesize);
        BUG();
    }

    if (item->offset % typesize == 0) {
        val <<= item->shift;
        val &= mask;
        tmp = be64_to_cpu(b[offset]);
        tmp &= ~mask;
        tmp |= val;
        b[offset] = cpu_to_be64(tmp);
    } else if (item->offset % sizeof(u32) != 0) {
        /* If not aligned to 4 bytes then raise error. */
        sxd_log_err(SXD_DEV_NONE, "mlxsw: item bug (name=%s,offset=%x,step=%x,in_step_offset=%x,typesize=%zx)\n",
                    item->name, item->offset, item->step,
                    item->in_step_offset, typesize);
        BUG();
    } else {
        /* This is a hack to work around not aligned u64 fields.
         *  Emulate access to __be64 *b as it was pair of __be32.
         *  Expect that this is a full 8 byte long field.
         *  Thus no shift or mask is required. */
        offset = __mlxsw_item_offset_no_checks(item, outer_index, index, sizeof(u32));
        if (__little_endian_order()) {
            ((__be32 *)b)[offset] = cpu_to_be32(((uint32_t *)&val)[1]);
            ((__be32 *)b)[offset + 1] = cpu_to_be32((uint32_t)val);
        } else {
            ((__be32 *)b)[offset] = cpu_to_be32((uint32_t)val);
            ((__be32 *)b)[offset + 1] = cpu_to_be32(((uint32_t *)&val)[1]);
        }
    }
}

static inline void __mlxsw_item_memcpy_from(const char              *buf,
                                            char                    *dst,
                                            const struct mlxsw_item *item,
                                            unsigned short           outer_index,
                                            unsigned short           index)
{
    unsigned int offset = __mlxsw_item_offset(item, outer_index, index, sizeof(char));

    memcpy(dst, &buf[offset], item->size.bytes);
}

static inline void __mlxsw_item_memcpy_to(char                    *buf,
                                          const char              *src,
                                          const struct mlxsw_item *item,
                                          unsigned short           outer_index,
                                          unsigned short           index)
{
    unsigned int offset = __mlxsw_item_offset(item, outer_index, index, sizeof(char));

    memcpy(&buf[offset], src, item->size.bytes);
}

static inline char * __mlxsw_item_data(char *buf, const struct mlxsw_item *item, unsigned short index)
{
    unsigned int offset = __mlxsw_item_offset(item, 0, index, sizeof(char));

    return &buf[offset];
}

static inline u16 __mlxsw_item_bit_array_offset(const struct mlxsw_item *item, u16 outer_index, u16 index, u8 *shift)
{
    u16 items_per_byte;
    u16 offset;        /* byte offset inside the array */
    u8  in_byte_index;

    BUG_ON(index && !item->element_size);
    if ((item->offset % sizeof(u32) != 0) ||
        (BITS_PER_BYTE % item->element_size != 0)) {
        sxd_log_err(SXD_DEV_NONE, "mlxsw: item bug (name=%s,offset=%x,element_size=%x)\n",
                    item->name, item->offset, item->element_size);
        BUG();
    }

    items_per_byte = BITS_PER_BYTE / item->element_size;
    offset = index / items_per_byte;
    in_byte_index = items_per_byte - index % items_per_byte - 1;
    *shift = in_byte_index * item->element_size;

    return item->offset + offset;
}

static inline u8 __mlxsw_item_bit_array_get(const char *buf, const struct mlxsw_item *item, u16 outer_index, u16 index)
{
    u8  shift, tmp;
    u16 offset = __mlxsw_item_bit_array_offset(item, outer_index, index, &shift);

    tmp = buf[offset];
    tmp >>= shift;
    tmp &= GEN_MASK(item->element_size - 1, 0);
    return tmp;
}

static inline void __mlxsw_item_bit_array_set(char                    *buf,
                                              const struct mlxsw_item *item,
                                              u16                      outer_index,
                                              u16                      index,
                                              u8                       val)
{
    u8  shift, tmp;
    u16 offset = __mlxsw_item_bit_array_offset(item, outer_index, index, &shift);
    u8  mask = GEN_MASK(item->element_size - 1, 0) << shift;

    val <<= shift;
    val &= mask;
    tmp = buf[offset];
    tmp &= ~mask;
    tmp |= val;
    buf[offset] = tmp;
}

#define __ITEM_NAME(_type, _cname, _iname) \
    mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _item

/* _type: cmd_mbox, reg, etc.
 * _cname: container name (e.g. command name, register name)
 * _iname: item name within the container
 */

#define MLXSW_ITEM8(_type, _cname, _iname, _offset, _shift, _sizebits)                               \
    static struct mlxsw_item __ITEM_NAME(_type, _cname, _iname) = {                                  \
        .offset = _offset,                                                                           \
        .shift = _shift,                                                                             \
        .size = {.bits = _sizebits, },                                                               \
        .name = #_type "_" #_cname "_" #_iname,                                                      \
    };                                                                                               \
    u8                       mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _get(const char *buf); \
    u8                       mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _get(const char *buf)  \
    {                                                                                                \
        return __mlxsw_item_get8(buf, &__ITEM_NAME(_type, _cname, _iname), 0, 0);                    \
    }                                                                                                \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _set(char *buf, u8 val);                   \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _set(char *buf, u8 val)                    \
    {                                                                                                \
        __mlxsw_item_set8(buf, &__ITEM_NAME(_type, _cname, _iname), 0, 0, val);                      \
    }

/*
 *  _type = "reg"
 *  _cname = register name
 *  _iname = field name
 *  _offset = field offset or node offset if node is indexed (bytes)
 *  _shift =
 *  _sizebits = field size in bits
 *  _step = field size or node size if node is indexed
 *  _instepoffset = 0 if one field is indexed
 *                 sizeof(node) if node is indexed
 */
#define MLXSW_ITEM8_INDEXED(_type, _cname, _iname, _offset, _shift, _sizebits,                                 \
                            _step, _instepoffset)                                                              \
    static struct mlxsw_item __ITEM_NAME(_type, _cname, _iname) = {                                            \
        .offset = _offset,                                                                                     \
        .step = _step,                                                                                         \
        .in_step_offset = _instepoffset,                                                                       \
        .shift = _shift,                                                                                       \
        .size = {.bits = _sizebits, },                                                                         \
        .name = #_type "_" #_cname "_" #_iname,                                                                \
    };                                                                                                         \
    u8                       mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _get(const char *buf,            \
                                                                                   unsigned short outer_index, \
                                                                                   unsigned short index);      \
    u8 mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _get(const char *buf,                                  \
                                                             unsigned short outer_index,                       \
                                                             unsigned short index)                             \
    {                                                                                                          \
        return __mlxsw_item_get8(buf, &__ITEM_NAME(_type, _cname, _iname), outer_index, index);                \
    }                                                                                                          \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _set(char *buf, unsigned short outer_index,          \
                                                               unsigned short index, u8 val);                  \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _set(char *buf, unsigned short outer_index,          \
                                                               unsigned short index, u8 val)                   \
    {                                                                                                          \
        __mlxsw_item_set8(buf, &__ITEM_NAME(_type, _cname, _iname), outer_index, index, val);                  \
    }

#define MLXSW_ITEM16(_type, _cname, _iname, _offset, _shift, _sizebits)                              \
    static struct mlxsw_item __ITEM_NAME(_type, _cname, _iname) = {                                  \
        .offset = _offset,                                                                           \
        .shift = _shift,                                                                             \
        .size = {.bits = _sizebits, },                                                               \
        .name = #_type "_" #_cname "_" #_iname,                                                      \
    };                                                                                               \
    u16                      mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _get(const char *buf); \
    u16                      mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _get(const char *buf)  \
    {                                                                                                \
        return __mlxsw_item_get16(buf, &__ITEM_NAME(_type, _cname, _iname), 0, 0);                   \
    }                                                                                                \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _set(char *buf, u16 val);                  \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _set(char *buf, u16 val)                   \
    {                                                                                                \
        __mlxsw_item_set16(buf, &__ITEM_NAME(_type, _cname, _iname), 0, 0, val);                     \
    }

/*
 *  _type = "reg"
 *  _cname = register name
 *  _iname = field name
 *  _offset = field offset or node offset if node is indexed (bytes)
 *  _shift =
 *  _sizebits = field size in bits
 *  _step = field size or node size if node is indexed
 *  _instepoffset = 0 if one field is indexed
 *                 sizeof(node) if node is indexed
 */
#define MLXSW_ITEM16_INDEXED(_type, _cname, _iname, _offset, _shift, _sizebits,                                \
                             _step, _instepoffset)                                                             \
    static struct mlxsw_item __ITEM_NAME(_type, _cname, _iname) = {                                            \
        .offset = _offset,                                                                                     \
        .step = _step,                                                                                         \
        .in_step_offset = _instepoffset,                                                                       \
        .shift = _shift,                                                                                       \
        .size = {.bits = _sizebits, },                                                                         \
        .name = #_type "_" #_cname "_" #_iname,                                                                \
    };                                                                                                         \
    u16                      mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _get(const char *buf,            \
                                                                                   unsigned short outer_index, \
                                                                                   unsigned short index);      \
    u16 mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _get(const char *buf,                                 \
                                                              unsigned short outer_index,                      \
                                                              unsigned short index)                            \
    {                                                                                                          \
        return __mlxsw_item_get16(buf, &__ITEM_NAME(_type, _cname, _iname), outer_index, index);               \
    }                                                                                                          \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _set(char *buf, unsigned short outer_index,          \
                                                               unsigned short index, u16 val);                 \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _set(char *buf, unsigned short outer_index,          \
                                                               unsigned short index, u16 val)                  \
    {                                                                                                          \
        __mlxsw_item_set16(buf, &__ITEM_NAME(_type, _cname, _iname), outer_index, index, val);                 \
    }

#define MLXSW_ITEM32(_type, _cname, _iname, _offset, _shift, _sizebits)                              \
    static struct mlxsw_item __ITEM_NAME(_type, _cname, _iname) = {                                  \
        .offset = _offset,                                                                           \
        .shift = _shift,                                                                             \
        .size = {.bits = _sizebits, },                                                               \
        .name = #_type "_" #_cname "_" #_iname,                                                      \
    };                                                                                               \
    u32                      mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _get(const char *buf); \
    u32                      mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _get(const char *buf)  \
    {                                                                                                \
        return __mlxsw_item_get32(buf, &__ITEM_NAME(_type, _cname, _iname), 0, 0);                   \
    }                                                                                                \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _set(char *buf, u32 val);                  \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _set(char *buf, u32 val)                   \
    {                                                                                                \
        __mlxsw_item_set32(buf, &__ITEM_NAME(_type, _cname, _iname), 0, 0, val);                     \
    }

/*
 *  _type = "reg"
 *  _cname = register name
 *  _iname = field name
 *  _offset = field offset or node offset if node is indexed (bytes)
 *  _shift =
 *  _sizebits = field size in bits
 *  _step = field size or node size if node is indexed
 *  _instepoffset = 0 if one field is indexed
 *                 sizeof(node) if node is indexed
 */
#define MLXSW_ITEM32_INDEXED(_type, _cname, _iname, _offset, _shift, _sizebits,                                \
                             _step, _instepoffset)                                                             \
    static struct mlxsw_item __ITEM_NAME(_type, _cname, _iname) = {                                            \
        .offset = _offset,                                                                                     \
        .step = _step,                                                                                         \
        .in_step_offset = _instepoffset,                                                                       \
        .shift = _shift,                                                                                       \
        .size = {.bits = _sizebits, },                                                                         \
        .name = #_type "_" #_cname "_" #_iname,                                                                \
    };                                                                                                         \
    u32                      mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _get(const char *buf,            \
                                                                                   unsigned short outer_index, \
                                                                                   unsigned short index);      \
    u32 mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _get(const char *buf,                                 \
                                                              unsigned short outer_index,                      \
                                                              unsigned short index)                            \
    {                                                                                                          \
        return __mlxsw_item_get32(buf, &__ITEM_NAME(_type, _cname, _iname), outer_index, index);               \
    }                                                                                                          \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _set(char *buf, unsigned short outer_index,          \
                                                               unsigned short index, u32 val);                 \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _set(char *buf, unsigned short outer_index,          \
                                                               unsigned short index, u32 val)                  \
    {                                                                                                          \
        __mlxsw_item_set32(buf, &__ITEM_NAME(_type, _cname, _iname), outer_index, index, val);                 \
    }

#define MLXSW_ITEM64(_type, _cname, _iname, _offset, _shift, _sizebits)                              \
    static struct mlxsw_item __ITEM_NAME(_type, _cname, _iname) = {                                  \
        .offset = _offset,                                                                           \
        .shift = _shift,                                                                             \
        .size = {.bits = _sizebits, },                                                               \
        .name = #_type "_" #_cname "_" #_iname,                                                      \
    };                                                                                               \
    u64                      mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _get(const char *buf); \
    u64                      mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _get(const char *buf)  \
    {                                                                                                \
        return __mlxsw_item_get64(buf, &__ITEM_NAME(_type, _cname, _iname), 0, 0);                   \
    }                                                                                                \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _set(char *buf, u64 val);                  \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _set(char *buf, u64 val)                   \
    {                                                                                                \
        __mlxsw_item_set64(buf, &__ITEM_NAME(_type, _cname, _iname), 0, 0, val);                     \
    }

/*
 *  _type = "reg"
 *  _cname = register name
 *  _iname = field name
 *  _offset = field offset or node offset if node is indexed (bytes)
 *  _shift =
 *  _sizebits = field size in bits
 *  _step = field size or node size if node is indexed
 *  _instepoffset = 0 if one field is indexed
 *                 sizeof(node) if node is indexed
 */
#define MLXSW_ITEM64_INDEXED(_type, _cname, _iname, _offset, _shift,                                           \
                             _sizebits, _step, _instepoffset)                                                  \
    static struct mlxsw_item __ITEM_NAME(_type, _cname, _iname) = {                                            \
        .offset = _offset,                                                                                     \
        .step = _step,                                                                                         \
        .in_step_offset = _instepoffset,                                                                       \
        .shift = _shift,                                                                                       \
        .size = {.bits = _sizebits, },                                                                         \
        .name = #_type "_" #_cname "_" #_iname,                                                                \
    };                                                                                                         \
    u64                      mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _get(const char *buf,            \
                                                                                   unsigned short outer_index, \
                                                                                   unsigned short index);      \
    u64 mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _get(const char *buf,                                 \
                                                              unsigned short outer_index,                      \
                                                              unsigned short index)                            \
    {                                                                                                          \
        return __mlxsw_item_get64(buf, &__ITEM_NAME(_type, _cname, _iname), outer_index, index);               \
    }                                                                                                          \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _set(char *buf, unsigned short outer_index,          \
                                                               unsigned short index, u64 val);                 \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _set(char *buf, unsigned short outer_index,          \
                                                               unsigned short index, u64 val)                  \
    {                                                                                                          \
        __mlxsw_item_set64(buf, &__ITEM_NAME(_type, _cname, _iname), outer_index, index, val);                 \
    }

#define MLXSW_ITEM_BUF(_type, _cname, _iname, _offset, _sizebytes)                                                      \
    static struct mlxsw_item __ITEM_NAME(_type, _cname, _iname) = {                                                     \
        .offset = _offset,                                                                                              \
        .size = {.bytes = _sizebytes, },                                                                                \
        .name = #_type "_" #_cname "_" #_iname,                                                                         \
    };                                                                                                                  \
    void                     mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _memcpy_from(const char *buf, char *dst); \
    void                     mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _memcpy_from(const char *buf, char *dst)  \
    {                                                                                                                   \
        __mlxsw_item_memcpy_from(buf, dst, &__ITEM_NAME(_type, _cname, _iname), 0, 0);                                  \
    }                                                                                                                   \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _memcpy_to(char *buf, const char *src);                       \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _memcpy_to(char *buf, const char *src)                        \
    {                                                                                                                   \
        __mlxsw_item_memcpy_to(buf, src, &__ITEM_NAME(_type, _cname, _iname), 0, 0);                                    \
    }                                                                                                                   \
    char * mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _data(char *buf);                                           \
    char * mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _data(char *buf)                                            \
    {                                                                                                                   \
        return __mlxsw_item_data(buf, &__ITEM_NAME(_type, _cname, _iname), 0);                                          \
    }

#define MLXSW_ITEM_BUF_INDEXED(_type, _cname, _iname, _offset, _sizebytes, _step, _instepoffset)                       \
    static struct mlxsw_item __ITEM_NAME(_type, _cname, _iname) = {                                                    \
        .offset = _offset,                                                                                             \
        .step = _step,                                                                                                 \
        .in_step_offset = _instepoffset,                                                                               \
        .size = {.bytes = _sizebytes, },                                                                               \
        .name = #_type "_" #_cname "_" #_iname,                                                                        \
    };                                                                                                                 \
    void                     mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _memcpy_from(const char *buf,            \
                                                                                           unsigned short outer_index, \
                                                                                           unsigned short index,       \
                                                                                           char *dst);                 \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _memcpy_from(const char *buf,                                \
                                                                       unsigned short outer_index,                     \
                                                                       unsigned short index,                           \
                                                                       char *dst)                                      \
    {                                                                                                                  \
        __mlxsw_item_memcpy_from(buf, dst, &__ITEM_NAME(_type, _cname, _iname), outer_index, index);                   \
    }                                                                                                                  \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _memcpy_to(char *buf,                                        \
                                                                     unsigned short outer_index,                       \
                                                                     unsigned short index,                             \
                                                                     const char *src);                                 \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _memcpy_to(char *buf,                                        \
                                                                     unsigned short outer_index,                       \
                                                                     unsigned short index,                             \
                                                                     const char *src)                                  \
    {                                                                                                                  \
        __mlxsw_item_memcpy_to(buf, src, &__ITEM_NAME(_type, _cname, _iname), outer_index, index);                     \
    }                                                                                                                  \
    char * mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _data(char *buf, unsigned short index);                    \
    char * mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _data(char *buf, unsigned short index)                     \
    {                                                                                                                  \
        return __mlxsw_item_data(buf, &__ITEM_NAME(_type, _cname, _iname), index);                                     \
    }

/*
 *  _type = "reg"
 *  _cname = register name
 *  _iname = field name
 *  _offset = array offset in bytes
 *  _sizebytes = size of the array in bytes
 *  _element_size = size in bits of single element of the array
 */
#define MLXSW_ITEM_BIT_ARRAY(_type, _cname, _iname, _offset, _sizebytes, _element_size)                        \
    static struct mlxsw_item __ITEM_NAME(_type, _cname, _iname) = {                                            \
        .offset = _offset,                                                                                     \
        .element_size = _element_size,                                                                         \
        .size = {.bytes = _sizebytes, },                                                                       \
        .name = #_type "_" #_cname "_" #_iname,                                                                \
    };                                                                                                         \
    u8                       mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _get(const char *buf,            \
                                                                                   u16 outer_index,            \
                                                                                   u16 index)                  \
    {                                                                                                          \
        return __mlxsw_item_bit_array_get(buf,                                                                 \
                                          &__ITEM_NAME(_type, _cname, _iname),                                 \
                                          outer_index, index);                                                 \
    }                                                                                                          \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _set(char *buf, u16 outer_index, u16 index, u8 val); \
    void mlxsw_ ## _type ## _ ## _cname ## _ ## _iname ## _set(char *buf, u16 outer_index, u16 index, u8 val)  \
    {                                                                                                          \
        return __mlxsw_item_bit_array_set(buf,                                                                 \
                                          &__ITEM_NAME(_type, _cname, _iname),                                 \
                                          outer_index, index, val);                                            \
    }                                                                                                          \


/* old implementation defined all register field's offsets from the beginning of the buffer (inbox/outbox)
 * and not from the beginning of the register area itself. 'auto-reg' infrastructure works with functions that
 * get the pointer to the beginning of the register area. the two definitions below adapts old implementation to
 * new one so we don't have to recalculate all offsets.
 */
#define SX_PUT_REG_FIELD(dest, source, offset) SX_PUT((dest), (source), ((offset) + sizeof(sxd_emad_reg_tlv_t)))
#define SX_GET_REG_FIELD(dest, source, offset) SX_GET((dest), (source), (offset))

int sx_ACCESS_REG_internal(struct sx_dev            *dev,
                           uint8_t                   dev_id,
                           uint32_t                  flags,
                           sxd_emad_operation_tlv_t *op_tlv,
                           access_reg_encode_cb_t    reg_encode_cb,
                           access_reg_decode_cb_t    reg_decode_cb,
                           void                     *ku_reg,
                           void                     *context)
{
    sxd_emad_tlv_markers_t        tlv_markers;
    sxd_emad_tlv_markers_status_e tlv_markers_st;
    struct sx_cmd_mailbox        *in_mailbox;
    struct sx_cmd_mailbox        *out_mailbox;
    u8                           *inbox;
    u8                           *encoder_buff_start;
    u8                           *outbox;
    int                           err;
    u32                           cmd_flags = 0;
    u32                           encoder_buff_bytes = 0;
    u32                           total_in_frame_size = 0;
    u32                           total_out_frame_size = 0;
    u8                            fw_status = 0;

    if (!dev || !op_tlv || !ku_reg) {
        sxd_log_err(SXD_DEV_NONE, "Access Reg invalid param dev:%p op_tlv:%p ku_reg:%p \n", dev, op_tlv, ku_reg);
        return -EINVAL;
    }

    /* if flags are 0, we check whether there are flags that were passed in op_tlv->tid (32bit-msb) field.
     * see sx_cmd_set_op_tlv_ext() */
    if (!flags) {
        flags = op_tlv->tid >> 32;
    }

    if (flags & SX_ACCESS_REG_F_FORCE_WAITING) {
        cmd_flags |= SX_CMD_F_FORCE_WAITING;
    }

    if (sx_core_fw_is_faulty(dev)) {
        sxd_log_notice(SXD_DEV_NONE, "Access Reg: Faulty FW - ignoring\n");
        return 0;
    }

    if (!sx_core_fw_is_ready(dev)) {
        sxd_log_notice(dev, "Access Reg: FW is not ready - ignoring\n");
        return 0;
    }

    in_mailbox = sx_alloc_cmd_mailbox(dev, dev_id);
    if (IS_ERR(in_mailbox)) {
        return PTR_ERR(in_mailbox);
    }

    out_mailbox = sx_alloc_cmd_mailbox(dev, dev_id);
    if (IS_ERR(out_mailbox)) {
        err = PTR_ERR(out_mailbox);
        goto out_free;
    }

    inbox = in_mailbox->buf;
    memset(inbox, 0, SX_MAILBOX_SIZE);
    outbox = out_mailbox->buf;

    set_operation_tlv(inbox, op_tlv);
    encoder_buff_start = inbox + sizeof(sxd_emad_operation_tlv_t);

    if (reg_encode_cb) {
        /* encoder will build INDEXATION_TLV (if needed) and REG_TLV,
         * encoder will return @encoder_buff_bytes (total number of bytes written to the buffer) */
        err = reg_encode_cb(encoder_buff_start, ku_reg, context, sx_priv(dev)->emad.flags, &encoder_buff_bytes);
        if (err) {
            goto out;
        }
    }

    total_in_frame_size = sizeof(sxd_emad_operation_tlv_t) + encoder_buff_bytes;

    /* Send TX notification to CMD-IFC sniffer (before command) */
    if (sx_priv(dev)->prm_sniffer_enable) {
        __cmd_ifc_sniffer_send(dev,
                               sxd_emad_operation_tlv_reg_id_get(op_tlv),
                               sxd_emad_operation_tlv_method_get(op_tlv),
                               1,
                               encoder_buff_start,
                               encoder_buff_bytes);
    }

    err = sx_cmd_box(dev, dev_id, in_mailbox, out_mailbox, 0, 0,
                     SX_CMD_ACCESS_REG, SX_CMD_TIME_CLASS_A,
                     total_in_frame_size, cmd_flags);
    if (err) {
        if (flags & SX_ACCESS_REG_F_IGNORE_FW_RET_CODE) {
            err = 0;
        } else if (err != -ECONNRESET) {
            if (flags & SX_ACCESS_REG_F_QUIET_ERRORS) {
                sxd_log_debug(dev,
                              "Register access failed (reg_id=0x%04x, err=%d [from CMD-IFC status])\n",
                              sxd_emad_operation_tlv_reg_id_get(op_tlv), err);
            } else {
                sxd_log_err(dev,
                            "Register access failed (reg_id=0x%04x, err=%d [from CMD-IFC status])\n",
                            sxd_emad_operation_tlv_reg_id_get(op_tlv), err);
                print_hex_dump(KERN_NOTICE, "RegInbox ", DUMP_PREFIX_OFFSET, 16, 1, inbox,
                               total_in_frame_size, 0);
                dump_stack();
            }
        }

        goto out;
    }

    get_operation_tlv(outbox, op_tlv);

    tlv_markers_st = sxd_emad_tlv_markers_get(outbox,
                                              SX_MAILBOX_SIZE,
                                              0,
                                              SXD_EMAD_TLV_MARKERS_F_OPERATION | SXD_EMAD_TLV_MARKERS_F_REG,
                                              SXD_EMAD_TLV_MARKERS_F_INDEX | SXD_EMAD_TLV_MARKERS_F_MULTI_INDEX |
                                              SXD_EMAD_TLV_MARKERS_F_END,
                                              &tlv_markers);
    if (tlv_markers_st != SXD_EMAD_TLV_MARKERS_STATUS_SUCCESS_E) {
        sxd_log_err(dev, "failed to parse EMAD frame (status=%d)\n", tlv_markers_st);
        print_hex_dump(KERN_NOTICE, "RegOutbox ", DUMP_PREFIX_OFFSET, 16, 1, outbox,
                       SX_MAILBOX_SIZE, 0);
        dump_stack();
        err = -EINVAL;
        goto out;
    }

    total_out_frame_size = sizeof(sxd_emad_operation_tlv_t) +
                           sxd_emad_tlv_len_get(tlv_markers.reg_tlv);

    if (unlikely(tlv_markers.indexation_tlv)) {
        total_out_frame_size += sxd_emad_tlv_len_get(tlv_markers.indexation_tlv);
    }

    fw_status = sxd_emad_operation_tlv_status_get(op_tlv);
    if (fw_status) {
        err = fw_status_to_err(fw_status);
        if ((fw_status == SXD_EMAD_STATUS_BUSY_E) || (fw_status == SXD_EMAD_STATUS_ISSU_ONGOING_E)) {
            /* these status codes are not real errors, log it as 'info' */
            sxd_log_notice(SXD_DEV_NONE,
                           "Register access notice (reg_id=0x%04x, status=0x%x, err=%d [from OP-TLV status])\n",
                           sxd_emad_operation_tlv_reg_id_get(op_tlv),
                           fw_status,
                           err);
        } else if (flags & SX_ACCESS_REG_F_QUIET_ERRORS) {
            sxd_log_debug(SXD_DEV_NONE,
                          "Register access failed (reg_id=0x%04x, status=0x%x, err=%d [from OP-TLV status])\n",
                          sxd_emad_operation_tlv_reg_id_get(op_tlv),
                          fw_status,
                          err);
        } else {
            sxd_log_err(SXD_DEV_NONE,
                        "Register access failed (reg_id=0x%04x, status=0x%x, err=%d [from OP-TLV status])\n",
                        sxd_emad_operation_tlv_reg_id_get(op_tlv),
                        fw_status,
                        err);
            print_hex_dump(KERN_NOTICE, "RegInbox ", DUMP_PREFIX_OFFSET, 16, 1, inbox, total_in_frame_size, 0);
            dump_stack();
        }

        goto out;
    }

    if (reg_decode_cb &&
        ((flags & SX_ACCESS_REG_F_SET_AND_GET) ||
         (sxd_emad_operation_tlv_method_get(op_tlv) == SXD_EMAD_METHOD_QUERY_E))) {
        err = reg_decode_cb(&tlv_markers, ku_reg, context);
        if (err) {
            goto out;
        }

        /* Send RX notification to CMD-IFC sniffer (after successful query/decode) */
        if (sx_priv(dev)->prm_sniffer_enable) {
            __cmd_ifc_sniffer_send(dev,
                                   sxd_emad_operation_tlv_reg_id_get(op_tlv),
                                   sxd_emad_operation_tlv_method_get(op_tlv),
                                   0,
                                   outbox + sizeof(sxd_emad_operation_tlv_t),
                                   total_out_frame_size - sizeof(sxd_emad_operation_tlv_t));
        }
    }

out:
    sx_free_cmd_mailbox(dev, out_mailbox);

out_free:
    sx_free_cmd_mailbox(dev, in_mailbox);
    return err;
}

/****************************************************
 * SPZR (wrapper)
 *
 * sx_ib module is calling the SPZR wrapper.
 * on OOB system, it will call the register by EMAD
 * Otherwise, it will call it with Command Interface
 ***************************************************/
int sx_ACCESS_REG_SPZR_wrapper(struct sx_dev *dev, struct ku_access_spzr_reg *reg_data)
{
    if (is_sgmii_supported()) {
        /* SPZR is called in bootstrap and when working with OOB, we don't have PCI to work with, only I2C.
         * The problem is that I2C is enabled only on one management board. We must call this register from both
         * management boards and the only way to do that is with SGMII.
         */
        return sgmii_emad_access_spzr(reg_data->dev_id, &reg_data->spzr_reg);
    }

    return sx_ACCESS_REG_SPZR(dev, reg_data);
}
EXPORT_SYMBOL(sx_ACCESS_REG_SPZR_wrapper);

/************************************************
 * PSPA
 ***********************************************/
#define REG_SWID_OFFSET                (0)
#define REG_SUB_PORT_OFFSET            (2)
#define PSPA_REG_LOCAL_PORT_MSB_OFFSET (3)
#define PSPA_REG_LEN_IN_BYTES          (0x8)

static int __PSPA_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_pspa_reg   *pspa_reg = (struct ku_pspa_reg*)ku_reg;
    sxd_emad_index_tlv_t *index_tlv;
    u8                   *reg_tlv;

    if (flags & SX_EMAD_F_INDEXATION_TLV) {
        index_tlv = (sxd_emad_index_tlv_t*)inbox;
        reg_tlv = inbox + sizeof(sxd_emad_index_tlv_t);
        sxd_emad_index_tlv_port_single_build(index_tlv, pspa_reg->lp_msb, pspa_reg->local_port);
        *bytes_written += sizeof(sxd_emad_index_tlv_t);
    } else {
        reg_tlv = inbox;
        SX_PUT_REG_FIELD(reg_tlv, pspa_reg->local_port, REG_LOCAL_PORT_OFFSET);
        SX_PUT_REG_FIELD(reg_tlv, pspa_reg->lp_msb, PSPA_REG_LOCAL_PORT_MSB_OFFSET);
    }

    SX_PUT_REG_FIELD(reg_tlv, pspa_reg->swid, REG_SWID_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, pspa_reg->sub_port, REG_SUB_PORT_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + PSPA_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + PSPA_REG_LEN_IN_BYTES;
    return 0;
}

static int __PSPA_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_pspa_reg *pspa_reg = (struct ku_pspa_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;

    if (tlv_markers->index_tlv) {
        sxd_emad_index_tlv_port_single_deparse(tlv_markers->index_tlv,
                                               &pspa_reg->lp_msb,
                                               &pspa_reg->local_port);
    } else {
        SX_GET_REG_FIELD(pspa_reg->local_port, outbox, REG_LOCAL_PORT_OFFSET);
        SX_GET_REG_FIELD(pspa_reg->lp_msb, outbox, PSPA_REG_LOCAL_PORT_MSB_OFFSET);
        pspa_reg->lp_msb &= 0x3;
    }

    SX_GET_REG_FIELD(pspa_reg->swid, outbox, REG_SWID_OFFSET);
    SX_GET_REG_FIELD(pspa_reg->sub_port, outbox, REG_SUB_PORT_OFFSET);
    return 0;
}

int sx_ACCESS_REG_PSPA(struct sx_dev *dev, struct ku_access_pspa_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __PSPA_encode,
                                  __PSPA_decode,
                                  &reg_data->pspa_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_PSPA);

/************************************************
 * MJTAG
 ***********************************************/
#define MJTAG_REG_CMD_OFFSET               0x00
#define MJTAG_REG_CMD_BIT_N                6
#define MJTAG_REG_SEQ_NUM_BIT_N            0x0f
#define MJTAG_REG_SEQ_NUM_OFFSET           0x00
#define MJTAG_REG_TRANSACTIONS_SIZE_OFFSET 0x03
#define MJTAG_REG_JTAG_TRANSACTION_OFFSET  0x04
#define MJTAG_REG_TRANSACTION_TDO_BIT_N    0x03
#define MJTAG_REG_TRANSACTION_TDI_BIT_N    0x01
#define MJTAG_REG_LEN_IN_BYTES             (0x30)

static int __MJTAG_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_mjtag_reg *mjtag_reg = (struct ku_mjtag_reg*)ku_reg;
    u8                  *reg_tlv = inbox;
    u8                   tmp_val_u8 = 0;
    int                  counter;

    tmp_val_u8 |= mjtag_reg->cmd << MJTAG_REG_CMD_BIT_N;
    tmp_val_u8 |= mjtag_reg->seq_num;
    SX_PUT_REG_FIELD(reg_tlv, tmp_val_u8, MJTAG_REG_CMD_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mjtag_reg->size, MJTAG_REG_TRANSACTIONS_SIZE_OFFSET);

    for (counter = 0; counter < mjtag_reg->size; counter++) {
        tmp_val_u8 = 0;
        tmp_val_u8 |= (mjtag_reg->jtag_transaction_sets[counter].tdo & 0x01) << MJTAG_REG_TRANSACTION_TDO_BIT_N;
        tmp_val_u8 |= (mjtag_reg->jtag_transaction_sets[counter].tdi & 0x01) << MJTAG_REG_TRANSACTION_TDI_BIT_N;
        tmp_val_u8 |= (mjtag_reg->jtag_transaction_sets[counter].tms & 0x01);
        SX_PUT_REG_FIELD(reg_tlv, tmp_val_u8, MJTAG_REG_JTAG_TRANSACTION_OFFSET + counter);
    }

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + MJTAG_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + MJTAG_REG_LEN_IN_BYTES;
    return 0;
}

static int __MJTAG_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_mjtag_reg *mjtag_reg = (struct ku_mjtag_reg*)ku_reg;
    u8                  *outbox = tlv_markers->reg_tlv->reg_data;
    u8                   tmp_val_u8 = 0;
    int                  counter;

    SX_GET_REG_FIELD(tmp_val_u8, outbox, MJTAG_REG_CMD_OFFSET);
    mjtag_reg->cmd = tmp_val_u8 & (1 << MJTAG_REG_CMD_BIT_N) ? 1 : 0;
    SX_GET_REG_FIELD(tmp_val_u8, outbox, MJTAG_REG_SEQ_NUM_OFFSET);
    mjtag_reg->seq_num = tmp_val_u8 & MJTAG_REG_SEQ_NUM_BIT_N ? 1 : 0;
    SX_GET_REG_FIELD(mjtag_reg->size, outbox, MJTAG_REG_TRANSACTIONS_SIZE_OFFSET);

    for (counter = 0; counter < mjtag_reg->size; counter++) {
        SX_GET_REG_FIELD(tmp_val_u8, outbox, MJTAG_REG_JTAG_TRANSACTION_OFFSET);
        mjtag_reg->jtag_transaction_sets[counter].tdi = tmp_val_u8 & (1 << MJTAG_REG_TRANSACTION_TDI_BIT_N) ? 1 : 0;
        mjtag_reg->jtag_transaction_sets[counter].tdo = tmp_val_u8 & (1 << MJTAG_REG_TRANSACTION_TDO_BIT_N) ? 1 : 0;
        mjtag_reg->jtag_transaction_sets[counter].tms = tmp_val_u8 & 1 ? 1 : 0;
    }

    return 0;
}

int sx_ACCESS_REG_MJTAG(struct sx_dev *dev, struct ku_access_mjtag_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __MJTAG_encode,
                                  __MJTAG_decode,
                                  &reg_data->mjtag_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_MJTAG);


/************************************************
 * PPLM
 ***********************************************/
#define REG_PLR_OVERRIDE_FIELDS_OFFSET               0x00
#define REG_LOCAL_PORT_OFFSET                        0x01
#define REG_LOCAL_PORT_MSB_OFFSET                    0x02
#define REG_PLR_VALID_FIELDS_OFFSET                  0x03
#define REG_PLR_OPER_FIELDS_OFFSET                   0x04
#define REG_PLR_SUPPORT_FIELDS_OFFSET                0x05
#define REG_PLR_FORCE_PEER_OFFSET                    0x06
#define REG_PORT_PROFILE_MODE_OFFSET                 0x08
#define REG_STATIC_PORT_PROFILE_OFFSET               0x09
#define REG_ACTIVE_PORT_PROFILE_OFFSET               0x0A
#define REG_PLR_FEC_MODE_ACTIVE_OFFSET               0x0C
#define REG_IB_FEC_OVERRIDE_ADMIN_HDR                0x38
#define REG_IB_FEC_OVERRIDE_ADMIN_EDR                0x3A
#define REG_IB_FEC_OVERRIDE_ADMIN_FDR                0x3C
#define REG_IB_FEC_OVERRIDE_ADMIN_NDR                0x42
#define REG_IB_FEC_OVERRIDE_ADMIN_XDR                0x4E
#define REG_NVLINK_FEC_OVERRIDE_CAP_200G_1X_MODE_A   0x68
#define REG_NVLINK_FEC_OVERRIDE_ADMIN_200G_1X_MODE_A 0x6A
#define REG_NVLINK_FEC_OVERRIDE_CAP_400G_2X_MODE_A   0x6C
#define REG_NVLINK_FEC_OVERRIDE_ADMIN_400G_2X_MODE_A 0x6E
#define REG_NVLINK_FEC_OVERRIDE_CAP_400G_2X_MODE_B   0x70
#define REG_NVLINK_FEC_OVERRIDE_ADMIN_400G_2X_MODE_B 0x72
#define REG_NVLINK_FEC_OVERRIDE_CAP_360G_2X_MODE_B   0x74
#define REG_NVLINK_FEC_OVERRIDE_ADMIN_360G_2X_MODE_B 0x76
#define REG_NVLINK_FEC_OVERRIDE_CAP_328G_2X_MODE_B   0x78
#define REG_NVLINK_FEC_OVERRIDE_ADMIN_328G_2X_MODE_B 0x7A
#define REG_NVLINK_FEC_OVERRIDE_CAP_200G_2X_MODE_A   0x7C
#define REG_NVLINK_FEC_OVERRIDE_ADMIN_200G_2X_MODE_A 0x7E
#define PPLM_REG_LEN_IN_BYTES                        (0x80)

static int __PPLM_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_pplm_reg   *pplm_reg = (struct ku_pplm_reg*)ku_reg;
    sxd_emad_index_tlv_t *index_tlv;
    u8                   *reg_tlv;
    u8                    tmp_u8 = 0;
    u16                   tmp_u16 = 0;
    u32                   plr_fields_and_fec_mode_active;

    if (flags & SX_EMAD_F_INDEXATION_TLV) {
        index_tlv = (sxd_emad_index_tlv_t*)inbox;
        reg_tlv = inbox + sizeof(sxd_emad_index_tlv_t);
        sxd_emad_index_tlv_port_single_build(index_tlv, pplm_reg->lp_msb, pplm_reg->local_port);
        *bytes_written += sizeof(sxd_emad_index_tlv_t);
    } else {
        reg_tlv = inbox;
        SX_PUT_REG_FIELD(reg_tlv, pplm_reg->local_port, REG_LOCAL_PORT_OFFSET);
        tmp_u8 = (pplm_reg->lp_msb & 0x03) << 4;
        SX_PUT_REG_FIELD(reg_tlv, tmp_u8, REG_LOCAL_PORT_MSB_OFFSET);
    }

    tmp_u8 = (pplm_reg->plr_reject_mode_vld & 0x1);
    tmp_u8 |= (pplm_reg->plr_margin_th_override_to_default & 0x1) << 1;
    tmp_u8 |= (pplm_reg->tx_crc_plr_override_to_default & 0x1) << 2;
    tmp_u8 |= (pplm_reg->plr_reject_mode_override_to_default & 0x1) << 3;
    tmp_u8 |= (pplm_reg->plr_force_peer_oper & 0x3) << 6;
    SX_PUT_REG_FIELD(inbox, tmp_u8, REG_PLR_OVERRIDE_FIELDS_OFFSET);
    SX_PUT_REG_FIELD(inbox, pplm_reg->local_port, REG_LOCAL_PORT_OFFSET);
    tmp_u8 = (pplm_reg->lp_msb & 0x03) << 4;
    SX_PUT_REG_FIELD(inbox, tmp_u8, REG_LOCAL_PORT_MSB_OFFSET);
    tmp_u8 = (pplm_reg->plr_vld & 0x1) << 2;
    tmp_u8 |= (pplm_reg->tx_crc_plr_vld & 0x1) << 3;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u8, REG_PLR_VALID_FIELDS_OFFSET);
    tmp_u8 = (pplm_reg->plr_margin_th_oper & 0xF) << 4;
    tmp_u8 |= (pplm_reg->tx_crc_plr_oper & 0x1) << 3;
    tmp_u8 |= (pplm_reg->plr_reject_mode_oper & 0x3) << 1;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u8, REG_PLR_OPER_FIELDS_OFFSET);
    tmp_u16 = (pplm_reg->plr_reject_mode_support & 0xF) << 11;
    tmp_u16 |= (pplm_reg->tx_crc_plr_support & 0x1) << 10;
    tmp_u16 |= (pplm_reg->plr_margin_th_support & 0xFF) << 1;
    SX_PUT_REG_FIELD(inbox, tmp_u16, REG_PLR_SUPPORT_FIELDS_OFFSET);
    tmp_u16 = (pplm_reg->plr_force_peer_admin & 0x3) << 7;
    tmp_u16 |= (pplm_reg->plr_force_peer_cap & 0x1) << 6;
    SX_PUT_REG_FIELD(inbox, tmp_u16, REG_PLR_FORCE_PEER_OFFSET);
    SX_PUT_REG_FIELD(inbox, pplm_reg->port_profile_mode, REG_PORT_PROFILE_MODE_OFFSET);
    SX_PUT_REG_FIELD(inbox, pplm_reg->static_port_profile, REG_STATIC_PORT_PROFILE_OFFSET);
    plr_fields_and_fec_mode_active = (pplm_reg->fec_mode_active & 0xFFFFFF);
    plr_fields_and_fec_mode_active |= (pplm_reg->plr_reject_mode & 0x3) << 24;
    plr_fields_and_fec_mode_active |= (pplm_reg->tx_crc_plr & 0x1) << 27;
    plr_fields_and_fec_mode_active |= (pplm_reg->plr_margin_th & 0xF) << 28;
    SX_PUT_REG_FIELD(reg_tlv, plr_fields_and_fec_mode_active, REG_PLR_FEC_MODE_ACTIVE_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, pplm_reg->ib_fec_override_admin_edr, REG_IB_FEC_OVERRIDE_ADMIN_EDR);
    SX_PUT_REG_FIELD(reg_tlv, pplm_reg->ib_fec_override_admin_fdr, REG_IB_FEC_OVERRIDE_ADMIN_FDR);
    SX_PUT_REG_FIELD(reg_tlv, pplm_reg->ib_fec_override_admin_hdr, REG_IB_FEC_OVERRIDE_ADMIN_HDR);
    SX_PUT_REG_FIELD(reg_tlv, pplm_reg->ib_fec_override_admin_ndr, REG_IB_FEC_OVERRIDE_ADMIN_NDR);
    SX_PUT_REG_FIELD(reg_tlv, pplm_reg->ib_fec_override_admin_xdr, REG_IB_FEC_OVERRIDE_ADMIN_XDR);
    SX_PUT_REG_FIELD(reg_tlv,
                     pplm_reg->nvl6_fec_page_data.nvlink_fec_override_cap_200g_1x_mode_a,
                     REG_NVLINK_FEC_OVERRIDE_CAP_200G_1X_MODE_A);
    SX_PUT_REG_FIELD(reg_tlv,
                     pplm_reg->nvl6_fec_page_data.nvlink_fec_override_admin_200g_1x_mode_a,
                     REG_NVLINK_FEC_OVERRIDE_ADMIN_200G_1X_MODE_A);
    SX_PUT_REG_FIELD(reg_tlv,
                     pplm_reg->nvl6_fec_page_data.nvlink_fec_override_cap_400g_2x_mode_a,
                     REG_NVLINK_FEC_OVERRIDE_CAP_400G_2X_MODE_A);
    SX_PUT_REG_FIELD(reg_tlv,
                     pplm_reg->nvl6_fec_page_data.nvlink_fec_override_admin_400g_2x_mode_a,
                     REG_NVLINK_FEC_OVERRIDE_ADMIN_400G_2X_MODE_A);
    SX_PUT_REG_FIELD(reg_tlv,
                     pplm_reg->nvl6_fec_page_data.nvlink_fec_override_cap_400g_2x_mode_b,
                     REG_NVLINK_FEC_OVERRIDE_CAP_400G_2X_MODE_B);
    SX_PUT_REG_FIELD(reg_tlv,
                     pplm_reg->nvl6_fec_page_data.nvlink_fec_override_admin_400g_2x_mode_b,
                     REG_NVLINK_FEC_OVERRIDE_ADMIN_400G_2X_MODE_B);
    SX_PUT_REG_FIELD(reg_tlv,
                     pplm_reg->nvl6_fec_page_data.nvlink_fec_override_cap_360g_2x_mode_b,
                     REG_NVLINK_FEC_OVERRIDE_CAP_360G_2X_MODE_B);
    SX_PUT_REG_FIELD(reg_tlv,
                     pplm_reg->nvl6_fec_page_data.nvlink_fec_override_admin_360g_2x_mode_b,
                     REG_NVLINK_FEC_OVERRIDE_ADMIN_360G_2X_MODE_B);
    SX_PUT_REG_FIELD(reg_tlv,
                     pplm_reg->nvl6_fec_page_data.nvlink_fec_override_cap_328g_2x_mode_b,
                     REG_NVLINK_FEC_OVERRIDE_CAP_328G_2X_MODE_B);
    SX_PUT_REG_FIELD(reg_tlv,
                     pplm_reg->nvl6_fec_page_data.nvlink_fec_override_admin_328g_2x_mode_b,
                     REG_NVLINK_FEC_OVERRIDE_ADMIN_328G_2X_MODE_B);
    SX_PUT_REG_FIELD(reg_tlv,
                     pplm_reg->nvl6_fec_page_data.nvlink_fec_override_cap_200g_2x_mode_a,
                     REG_NVLINK_FEC_OVERRIDE_CAP_200G_2X_MODE_A);
    SX_PUT_REG_FIELD(reg_tlv,
                     pplm_reg->nvl6_fec_page_data.nvlink_fec_override_admin_200g_2x_mode_a,
                     REG_NVLINK_FEC_OVERRIDE_ADMIN_200G_2X_MODE_A);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + PPLM_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + PPLM_REG_LEN_IN_BYTES;
    return 0;
}

static int __PPLM_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_pplm_reg *pplm_reg = (struct ku_pplm_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u32                 plr_fields_and_fec_mode_active;
    u8                  tmp_u8;
    u16                 tmp_u16;

    SX_GET_REG_FIELD(tmp_u8, outbox, REG_PLR_OVERRIDE_FIELDS_OFFSET);
    pplm_reg->plr_reject_mode_vld = tmp_u8 & 0x1;
    pplm_reg->plr_margin_th_override_to_default = (tmp_u8 >> 1) & 0x1;
    pplm_reg->tx_crc_plr_override_to_default = (tmp_u8 >> 2) & 0x1;
    pplm_reg->plr_reject_mode_override_to_default = (tmp_u8 >> 3) & 0x1;
    pplm_reg->plr_force_peer_oper = (tmp_u8 >> 6) & 0x3;
    if (tlv_markers->index_tlv) {
        sxd_emad_index_tlv_port_single_deparse(tlv_markers->index_tlv,
                                               &pplm_reg->lp_msb,
                                               &pplm_reg->local_port);
    } else {
        SX_GET_REG_FIELD(pplm_reg->local_port, outbox, REG_LOCAL_PORT_OFFSET);
        SX_GET_REG_FIELD(pplm_reg->lp_msb, outbox, REG_LOCAL_PORT_MSB_OFFSET);
        pplm_reg->lp_msb >>= 4;
        pplm_reg->lp_msb &= 0x3;
    }
    SX_GET_REG_FIELD(tmp_u8, outbox, REG_PLR_VALID_FIELDS_OFFSET);
    pplm_reg->plr_vld = (tmp_u8 >> 2) & 0x1;
    pplm_reg->tx_crc_plr_vld = (tmp_u8 >> 3) & 0x1;
    SX_GET_REG_FIELD(tmp_u8, outbox, REG_PLR_OPER_FIELDS_OFFSET);
    pplm_reg->plr_margin_th_oper = (tmp_u8 >> 4) & 0xF;
    pplm_reg->tx_crc_plr_oper = (tmp_u8 >> 3) & 0x1;
    pplm_reg->plr_reject_mode_oper = (tmp_u8 >> 1) & 0x3;
    SX_GET_REG_FIELD(tmp_u16, outbox, REG_PLR_FORCE_PEER_OFFSET);
    pplm_reg->plr_force_peer_admin = (tmp_u16 >> 7) & 0x3;
    pplm_reg->plr_force_peer_cap = (tmp_u16 >> 6) & 0x1;
    SX_GET_REG_FIELD(tmp_u16, outbox, REG_PLR_SUPPORT_FIELDS_OFFSET);
    pplm_reg->plr_reject_mode_support = (tmp_u16 >> 11) & 0xF;
    pplm_reg->tx_crc_plr_support = (tmp_u16 >> 10) & 0x1;
    pplm_reg->plr_margin_th_support = (tmp_u16 >> 1) & 0xFF;
    SX_GET_REG_FIELD(pplm_reg->port_profile_mode, outbox, REG_PORT_PROFILE_MODE_OFFSET);
    SX_GET_REG_FIELD(pplm_reg->static_port_profile, outbox, REG_STATIC_PORT_PROFILE_OFFSET);
    SX_GET_REG_FIELD(pplm_reg->active_port_profile, outbox, REG_ACTIVE_PORT_PROFILE_OFFSET);
    SX_GET_REG_FIELD(plr_fields_and_fec_mode_active, outbox, REG_PLR_FEC_MODE_ACTIVE_OFFSET);
    pplm_reg->fec_mode_active = plr_fields_and_fec_mode_active & 0xFFFFFF;
    pplm_reg->plr_reject_mode = (plr_fields_and_fec_mode_active >> 24) & 0x3;
    pplm_reg->tx_crc_plr = (plr_fields_and_fec_mode_active >> 27) & 0x1;
    pplm_reg->plr_margin_th = (plr_fields_and_fec_mode_active >> 28) & 0xF;
    SX_GET_REG_FIELD(pplm_reg->ib_fec_override_admin_edr, outbox, REG_IB_FEC_OVERRIDE_ADMIN_EDR);
    SX_GET_REG_FIELD(pplm_reg->ib_fec_override_admin_fdr, outbox, REG_IB_FEC_OVERRIDE_ADMIN_FDR);
    SX_GET_REG_FIELD(pplm_reg->ib_fec_override_admin_hdr, outbox, REG_IB_FEC_OVERRIDE_ADMIN_HDR);
    SX_GET_REG_FIELD(pplm_reg->ib_fec_override_admin_ndr, outbox, REG_IB_FEC_OVERRIDE_ADMIN_NDR);
    SX_GET_REG_FIELD(pplm_reg->ib_fec_override_admin_xdr, outbox, REG_IB_FEC_OVERRIDE_ADMIN_XDR);
    SX_GET_REG_FIELD(pplm_reg->nvl6_fec_page_data.nvlink_fec_override_cap_200g_1x_mode_a,
                     outbox,
                     REG_NVLINK_FEC_OVERRIDE_CAP_200G_1X_MODE_A);
    SX_GET_REG_FIELD(pplm_reg->nvl6_fec_page_data.nvlink_fec_override_admin_200g_1x_mode_a,
                     outbox,
                     REG_NVLINK_FEC_OVERRIDE_ADMIN_200G_1X_MODE_A);
    SX_GET_REG_FIELD(pplm_reg->nvl6_fec_page_data.nvlink_fec_override_cap_400g_2x_mode_a,
                     outbox,
                     REG_NVLINK_FEC_OVERRIDE_CAP_400G_2X_MODE_A);
    SX_GET_REG_FIELD(pplm_reg->nvl6_fec_page_data.nvlink_fec_override_admin_400g_2x_mode_a,
                     outbox,
                     REG_NVLINK_FEC_OVERRIDE_ADMIN_400G_2X_MODE_A);
    SX_GET_REG_FIELD(pplm_reg->nvl6_fec_page_data.nvlink_fec_override_cap_400g_2x_mode_b,
                     outbox,
                     REG_NVLINK_FEC_OVERRIDE_CAP_400G_2X_MODE_B);
    SX_GET_REG_FIELD(pplm_reg->nvl6_fec_page_data.nvlink_fec_override_admin_400g_2x_mode_b,
                     outbox,
                     REG_NVLINK_FEC_OVERRIDE_ADMIN_400G_2X_MODE_B);
    SX_GET_REG_FIELD(pplm_reg->nvl6_fec_page_data.nvlink_fec_override_cap_360g_2x_mode_b,
                     outbox,
                     REG_NVLINK_FEC_OVERRIDE_CAP_360G_2X_MODE_B);
    SX_GET_REG_FIELD(pplm_reg->nvl6_fec_page_data.nvlink_fec_override_admin_360g_2x_mode_b,
                     outbox,
                     REG_NVLINK_FEC_OVERRIDE_ADMIN_360G_2X_MODE_B);
    SX_GET_REG_FIELD(pplm_reg->nvl6_fec_page_data.nvlink_fec_override_cap_328g_2x_mode_b,
                     outbox,
                     REG_NVLINK_FEC_OVERRIDE_CAP_328G_2X_MODE_B);
    SX_GET_REG_FIELD(pplm_reg->nvl6_fec_page_data.nvlink_fec_override_admin_328g_2x_mode_b,
                     outbox,
                     REG_NVLINK_FEC_OVERRIDE_ADMIN_328G_2X_MODE_B);
    SX_GET_REG_FIELD(pplm_reg->nvl6_fec_page_data.nvlink_fec_override_cap_200g_2x_mode_a,
                     outbox,
                     REG_NVLINK_FEC_OVERRIDE_CAP_200G_2X_MODE_A);
    SX_GET_REG_FIELD(pplm_reg->nvl6_fec_page_data.nvlink_fec_override_admin_200g_2x_mode_a,
                     outbox,
                     REG_NVLINK_FEC_OVERRIDE_ADMIN_200G_2X_MODE_A);
    return 0;
}

int sx_ACCESS_REG_PPLM(struct sx_dev *dev, struct ku_access_pplm_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __PPLM_encode,
                                  __PPLM_decode,
                                  &reg_data->pplm_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_PPLM);


/************************************************
 * PLPC
 ***********************************************/
#define REG_PROFILE_ID_OFFSET                   0x00
#define REG_PLPC_PROTO_MASK_OFFSET              0x02
#define REG_LANE_SPEED_OFFSET                   0x06
#define REG_LPBF_OFFSET                         0x0A
#define REG_FEC_MODE_POLICY_OFFSET              0x0B
#define REG_RETRANSMISSION_CAPABILITY_OFFSET    0x0C
#define REG_FEC_MODE_CAPABILITY_OFFSET          0x0D
#define REG_RETRANSMISSION_SUPPORT_ADMIN_OFFSET 0x10
#define REG_FEC_MODE_SUPPORT_ADMIN_OFFSET       0x11
#define REG_RETRANSMISSION_REQUEST_ADMIN_OFFSET 0x14
#define REG_FEC_MODE_REQUEST_ADMIN_OFFSET       0x15
#define PLPC_REG_LEN_IN_BYTES                   (0x28)

static int __PLPC_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_plpc_reg *plpc_reg = (struct ku_plpc_reg*)ku_reg;
    u8                 *reg_tlv = inbox;

    SX_PUT_REG_FIELD(reg_tlv, plpc_reg->profile_id, REG_PROFILE_ID_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, plpc_reg->proto_mask, REG_PLPC_PROTO_MASK_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, plpc_reg->lane_speed, REG_LANE_SPEED_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, plpc_reg->lpbf, REG_LPBF_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, plpc_reg->fec_mode_policy, REG_FEC_MODE_POLICY_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, plpc_reg->retransmission_capability, REG_RETRANSMISSION_CAPABILITY_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, plpc_reg->fec_mode_capability, REG_FEC_MODE_CAPABILITY_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, plpc_reg->retransmission_support_admin, REG_RETRANSMISSION_SUPPORT_ADMIN_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, plpc_reg->fec_mode_support_admin, REG_FEC_MODE_SUPPORT_ADMIN_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, plpc_reg->retransmission_request_admin, REG_RETRANSMISSION_REQUEST_ADMIN_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, plpc_reg->fec_mode_request_admin, REG_FEC_MODE_REQUEST_ADMIN_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + PLPC_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + PLPC_REG_LEN_IN_BYTES;
    return 0;
}

static int __PLPC_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_plpc_reg *plpc_reg = (struct ku_plpc_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;

    SX_GET_REG_FIELD(plpc_reg->profile_id, outbox, REG_PROFILE_ID_OFFSET);
    SX_GET_REG_FIELD(plpc_reg->proto_mask, outbox, REG_PLPC_PROTO_MASK_OFFSET);
    SX_GET_REG_FIELD(plpc_reg->lane_speed, outbox, REG_LANE_SPEED_OFFSET);
    SX_GET_REG_FIELD(plpc_reg->lpbf, outbox, REG_LPBF_OFFSET);
    SX_GET_REG_FIELD(plpc_reg->fec_mode_policy, outbox, REG_FEC_MODE_POLICY_OFFSET);
    SX_GET_REG_FIELD(plpc_reg->retransmission_capability, outbox, REG_RETRANSMISSION_CAPABILITY_OFFSET);
    SX_GET_REG_FIELD(plpc_reg->fec_mode_capability, outbox, REG_FEC_MODE_CAPABILITY_OFFSET);
    SX_GET_REG_FIELD(plpc_reg->retransmission_support_admin, outbox, REG_RETRANSMISSION_SUPPORT_ADMIN_OFFSET);
    SX_GET_REG_FIELD(plpc_reg->fec_mode_support_admin, outbox, REG_FEC_MODE_SUPPORT_ADMIN_OFFSET);
    SX_GET_REG_FIELD(plpc_reg->retransmission_request_admin, outbox, REG_RETRANSMISSION_REQUEST_ADMIN_OFFSET);
    SX_GET_REG_FIELD(plpc_reg->fec_mode_request_admin, outbox, REG_FEC_MODE_REQUEST_ADMIN_OFFSET);
    return 0;
}

int sx_ACCESS_REG_PLPC(struct sx_dev *dev, struct ku_access_plpc_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __PLPC_encode,
                                  __PLPC_decode,
                                  &reg_data->plpc_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_PLPC);


/************************************************
 * PMPC
 ***********************************************/
#define REG_MODULE_STATE_UPDATED_OFFSET 0x00
#define PMPC_REG_LEN_IN_BYTES           (0x20)

static int __PMPC_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_pmpc_reg *pmpc_reg = (struct ku_pmpc_reg*)ku_reg;
    u8                 *reg_tlv = inbox;
    int                 i;

    for (i = 0; i < 8; i++) {
        SX_PUT_REG_FIELD(reg_tlv, pmpc_reg->module_state_updated_bitmap[i], REG_MODULE_STATE_UPDATED_OFFSET + (i * 4));
    }

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + PMPC_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + PMPC_REG_LEN_IN_BYTES;
    return 0;
}

static int __PMPC_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_pmpc_reg *pmpc_reg = (struct ku_pmpc_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    int                 i;

    for (i = 0; i < 8; i++) {
        SX_GET_REG_FIELD(pmpc_reg->module_state_updated_bitmap[i], outbox, REG_MODULE_STATE_UPDATED_OFFSET + (i * 4));
    }

    return 0;
}

int sx_ACCESS_REG_PMPC(struct sx_dev *dev, struct ku_access_pmpc_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __PMPC_encode,
                                  __PMPC_decode,
                                  &reg_data->pmpc_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_PMPC);


/************************************************
 * PMPR
 ***********************************************/
#define PMPR_REG_MODULE_OFFSET          0x01
#define PMPR_REG_ATTENUATION_5G_OFFSET  0x07
#define PMPR_REG_ATTENUATION_7G_OFFSET  0x0B
#define PMPR_REG_ATTENUATION_12G_OFFSET 0x0F
#define PMPR_REG_LEN_IN_BYTES           (0x10)

static int __PMPR_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_pmpr_reg *pmpr_reg = (struct ku_pmpr_reg*)ku_reg;
    u8                 *reg_tlv = inbox;

    SX_PUT_REG_FIELD(reg_tlv, pmpr_reg->module, PMPR_REG_MODULE_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, pmpr_reg->attenuation5g, PMPR_REG_ATTENUATION_5G_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, pmpr_reg->attenuation7g, PMPR_REG_ATTENUATION_7G_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, pmpr_reg->attenuation12g, PMPR_REG_ATTENUATION_12G_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + PMPR_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + PMPR_REG_LEN_IN_BYTES;
    return 0;
}

static int __PMPR_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_pmpr_reg *pmpr_reg = (struct ku_pmpr_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;

    SX_GET_REG_FIELD(pmpr_reg->module, outbox, PMPR_REG_MODULE_OFFSET);
    SX_GET_REG_FIELD(pmpr_reg->attenuation5g, outbox, PMPR_REG_ATTENUATION_5G_OFFSET);
    SX_GET_REG_FIELD(pmpr_reg->attenuation7g, outbox, PMPR_REG_ATTENUATION_7G_OFFSET);
    SX_GET_REG_FIELD(pmpr_reg->attenuation12g, outbox, PMPR_REG_ATTENUATION_12G_OFFSET);
    return 0;
}

int sx_ACCESS_REG_PMPR(struct sx_dev *dev, struct ku_access_pmpr_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __PMPR_encode,
                                  __PMPR_decode,
                                  &reg_data->pmpr_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_PMPR);

/************************************************
 *
 * PELC
 ***********************************************/
#define REG_PELC_OP_OFFSET             0x00
#define REG_PELC_LOCAL_PORT_OFFSET     0x01
#define REG_PELC_LOCAL_PORT_MSB_OFFSET 0x02
/* 0x17 reserved not in use */

#define REG_PELC_OP_ADMIN_OFFSET      0x04
#define REG_PELC_OP_CAPABILITY_OFFSET 0x05
#define REG_PELC_OP_REQUEST_OFFSET    0x06
#define REG_PELC_OP_ACTIVE_OFFSET     0x07

#define REG_PELC_ADMIN_OFFSET      0x08
#define REG_PELC_CAPABILITY_OFFSET 0x10
#define REG_PELC_REQUEST_OFFSET    0x18
#define REG_PELC_ACTIVE_OFFSET     0x20
#define PELC_REG_LEN_IN_BYTES      (0x38)

static int __PELC_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_pelc_reg   *pelc_reg = (struct ku_pelc_reg*)ku_reg;
    sxd_emad_index_tlv_t *index_tlv;
    u8                   *reg_tlv;
    u8                    tmp_u8 = 0;

    if (flags & SX_EMAD_F_INDEXATION_TLV) {
        index_tlv = (sxd_emad_index_tlv_t*)inbox;
        reg_tlv = inbox + sizeof(sxd_emad_index_tlv_t);
        sxd_emad_index_tlv_port_single_build(index_tlv, pelc_reg->lp_msb, pelc_reg->local_port);
        *bytes_written += sizeof(sxd_emad_index_tlv_t);
    } else {
        reg_tlv = inbox;
        SX_PUT_REG_FIELD(reg_tlv, pelc_reg->local_port, REG_PELC_LOCAL_PORT_OFFSET);
        tmp_u8 = (pelc_reg->lp_msb & 0x03) << 4;
        SX_PUT_REG_FIELD(reg_tlv, tmp_u8, REG_PELC_LOCAL_PORT_MSB_OFFSET);
    }

    tmp_u8 = pelc_reg->op << 4;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u8, REG_PELC_OP_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, pelc_reg->op_admin, REG_PELC_OP_ADMIN_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, pelc_reg->op_capability, REG_PELC_OP_CAPABILITY_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, pelc_reg->op_request, REG_PELC_OP_REQUEST_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, pelc_reg->op_active, REG_PELC_OP_ACTIVE_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, pelc_reg->admin, REG_PELC_ADMIN_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, pelc_reg->capability, REG_PELC_CAPABILITY_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, pelc_reg->request, REG_PELC_REQUEST_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, pelc_reg->active, REG_PELC_ACTIVE_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + PELC_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + PELC_REG_LEN_IN_BYTES;
    return 0;
}

static int __PELC_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_pelc_reg *pelc_reg = (struct ku_pelc_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;

    SX_GET_REG_FIELD(pelc_reg->op, outbox, REG_PELC_OP_OFFSET);
    pelc_reg->op >>= 4;
    if (tlv_markers->index_tlv) {
        sxd_emad_index_tlv_port_single_deparse(tlv_markers->index_tlv,
                                               &pelc_reg->lp_msb,
                                               &pelc_reg->local_port);
    } else {
        SX_GET_REG_FIELD(pelc_reg->local_port, outbox, REG_PELC_LOCAL_PORT_OFFSET);
        SX_GET_REG_FIELD(pelc_reg->lp_msb, outbox, REG_PELC_LOCAL_PORT_MSB_OFFSET);
        pelc_reg->lp_msb >>= 4;
        pelc_reg->lp_msb &= 0x3;
    }
    SX_GET_REG_FIELD(pelc_reg->op_admin, outbox, REG_PELC_OP_ADMIN_OFFSET);
    SX_GET_REG_FIELD(pelc_reg->op_capability, outbox, REG_PELC_OP_CAPABILITY_OFFSET);
    SX_GET_REG_FIELD(pelc_reg->op_request, outbox, REG_PELC_OP_REQUEST_OFFSET);
    SX_GET_REG_FIELD(pelc_reg->op_active, outbox, REG_PELC_OP_ACTIVE_OFFSET);
    SX_GET_REG_FIELD(pelc_reg->admin, outbox, REG_PELC_ADMIN_OFFSET);
    SX_GET_REG_FIELD(pelc_reg->capability, outbox, REG_PELC_CAPABILITY_OFFSET);
    SX_GET_REG_FIELD(pelc_reg->request, outbox, REG_PELC_REQUEST_OFFSET);
    SX_GET_REG_FIELD(pelc_reg->active, outbox, REG_PELC_ACTIVE_OFFSET);
    return 0;
}

int sx_ACCESS_REG_PELC(struct sx_dev *dev, struct ku_access_pelc_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __PELC_encode,
                                  __PELC_decode,
                                  &reg_data->pelc_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_PELC);


/************************************************
 * MCIA
 ***********************************************/
#define REG_L_OFFSET                  0x00
#define REG_MODULE_OFFSET             0x01
#define REG_SLOT_INDEX_OFFSET         0x02
#define REG_STATUS_OFFSET             0x03
#define REG_I2C_DEVICE_ADDRESS_OFFSET 0x04
#define REG_PAGE_NUMBER_OFFSET        0x05
#define REG_DEVICE_ADDRESS_OFFSET     0x06
#define REG_BG_MODE_OFFSET            0x08
#define REG_BANK_NUMBER_OFFSET        0x09
#define REG_SIZE_OFFSET               0x0a
#define REG_DWORD_0_OFFSET            0x10
#define REG_DWORD_1_OFFSET            0x14
#define REG_DWORD_2_OFFSET            0x18
#define REG_DWORD_3_OFFSET            0x1C
#define REG_DWORD_4_OFFSET            0x20
#define REG_DWORD_5_OFFSET            0x24
#define REG_DWORD_6_OFFSET            0x28
#define REG_DWORD_7_OFFSET            0x2C
#define REG_DWORD_8_OFFSET            0x30
#define REG_DWORD_9_OFFSET            0x34
#define REG_DWORD_10_OFFSET           0x38
#define REG_DWORD_11_OFFSET           0x3C
#define REG_DWORD_12_OFFSET           0x40
#define REG_DWORD_13_OFFSET           0x44
#define REG_DWORD_14_OFFSET           0x48
#define REG_DWORD_15_OFFSET           0x4C
#define REG_DWORD_16_OFFSET           0x50
#define REG_DWORD_17_OFFSET           0x54
#define REG_DWORD_18_OFFSET           0x58
#define REG_DWORD_19_OFFSET           0x5C
#define REG_DWORD_20_OFFSET           0x60
#define REG_DWORD_21_OFFSET           0x64
#define REG_DWORD_22_OFFSET           0x68
#define REG_DWORD_23_OFFSET           0x6C
#define REG_DWORD_24_OFFSET           0x70
#define REG_DWORD_25_OFFSET           0x74
#define REG_DWORD_26_OFFSET           0x78
#define REG_DWORD_27_OFFSET           0x7C
#define REG_DWORD_28_OFFSET           0x80
#define REG_DWORD_29_OFFSET           0x84
#define REG_DWORD_30_OFFSET           0x88
#define REG_DWORD_31_OFFSET           0x8c
#define MCIA_REG_LEN_IN_BYTES         (0x94)

static int __MCIA_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_mcia_reg *mcia_reg = (struct ku_mcia_reg*)ku_reg;
    u8                 *reg_tlv = inbox;
    u8                  tmp_u8 = 0;

    tmp_u8 = (mcia_reg->l & 0x1) << 7;
    tmp_u8 |= (mcia_reg->pnv & 0x1) << 5;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u8, REG_L_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->module, REG_MODULE_OFFSET);
    tmp_u8 = (mcia_reg->slot_index & 0x0F) << 4;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u8, REG_SLOT_INDEX_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->i2c_device_address, REG_I2C_DEVICE_ADDRESS_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->page_number, REG_PAGE_NUMBER_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->device_address, REG_DEVICE_ADDRESS_OFFSET);
    tmp_u8 = (mcia_reg->bg_mode & 0x1) << 3;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u8, REG_BG_MODE_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->bank_number, REG_BANK_NUMBER_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->size, REG_SIZE_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_0, REG_DWORD_0_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_1, REG_DWORD_1_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_2, REG_DWORD_2_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_3, REG_DWORD_3_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_4, REG_DWORD_4_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_5, REG_DWORD_5_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_6, REG_DWORD_6_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_7, REG_DWORD_7_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_8, REG_DWORD_8_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_9, REG_DWORD_9_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_10, REG_DWORD_10_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_11, REG_DWORD_11_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_12, REG_DWORD_12_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_13, REG_DWORD_13_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_14, REG_DWORD_14_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_15, REG_DWORD_15_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_16, REG_DWORD_16_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_17, REG_DWORD_17_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_18, REG_DWORD_18_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_19, REG_DWORD_19_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_20, REG_DWORD_20_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_21, REG_DWORD_21_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_22, REG_DWORD_22_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_23, REG_DWORD_23_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_24, REG_DWORD_24_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_25, REG_DWORD_25_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_26, REG_DWORD_26_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_27, REG_DWORD_27_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_28, REG_DWORD_28_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_29, REG_DWORD_29_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_30, REG_DWORD_30_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mcia_reg->dword_31, REG_DWORD_31_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + MCIA_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + MCIA_REG_LEN_IN_BYTES;
    return 0;
}

int __MCIA_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_mcia_reg *mcia_reg = (struct ku_mcia_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u8                  tmp_u8 = 0;

    SX_GET_REG_FIELD(tmp_u8, outbox, REG_L_OFFSET);
    mcia_reg->l = tmp_u8 >> 7;
    SX_GET_REG_FIELD(mcia_reg->module, outbox, REG_MODULE_OFFSET);
    SX_GET_REG_FIELD(tmp_u8, outbox, REG_SLOT_INDEX_OFFSET);
    mcia_reg->slot_index = tmp_u8 >> 4;
    SX_GET_REG_FIELD(mcia_reg->status, outbox, REG_STATUS_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->i2c_device_address, outbox, REG_I2C_DEVICE_ADDRESS_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->page_number, outbox, REG_PAGE_NUMBER_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->device_address, outbox, REG_DEVICE_ADDRESS_OFFSET);
    SX_GET_REG_FIELD(tmp_u8, outbox, REG_BG_MODE_OFFSET);
    mcia_reg->bg_mode = tmp_u8 >> 3;
    SX_GET_REG_FIELD(mcia_reg->bank_number, outbox, REG_BANK_NUMBER_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->size, outbox, REG_SIZE_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_0, outbox, REG_DWORD_0_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_1, outbox, REG_DWORD_1_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_2, outbox, REG_DWORD_2_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_3, outbox, REG_DWORD_3_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_4, outbox, REG_DWORD_4_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_5, outbox, REG_DWORD_5_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_6, outbox, REG_DWORD_6_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_7, outbox, REG_DWORD_7_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_8, outbox, REG_DWORD_8_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_9, outbox, REG_DWORD_9_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_10, outbox, REG_DWORD_10_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_11, outbox, REG_DWORD_11_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_12, outbox, REG_DWORD_12_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_13, outbox, REG_DWORD_13_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_14, outbox, REG_DWORD_14_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_15, outbox, REG_DWORD_15_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_16, outbox, REG_DWORD_16_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_17, outbox, REG_DWORD_17_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_18, outbox, REG_DWORD_18_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_19, outbox, REG_DWORD_19_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_20, outbox, REG_DWORD_20_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_21, outbox, REG_DWORD_21_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_22, outbox, REG_DWORD_22_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_23, outbox, REG_DWORD_23_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_24, outbox, REG_DWORD_24_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_25, outbox, REG_DWORD_25_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_26, outbox, REG_DWORD_26_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_27, outbox, REG_DWORD_27_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_28, outbox, REG_DWORD_28_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_29, outbox, REG_DWORD_29_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_30, outbox, REG_DWORD_30_OFFSET);
    SX_GET_REG_FIELD(mcia_reg->dword_31, outbox, REG_DWORD_31_OFFSET);
    return 0;
}

static const char * __mcia_err_str(u8 status)
{
    switch (status) {
    case 0:
        return "Ok";

    case 1:
        return "No EEPROM Module";

    case 2:
        return "Module not supported";

    case 3:
        return "Module not connected";

    case 4:
        return "Invalid module type";

    case 5:
        return "Module not accessible";

    case 9:
        return "I2C error";

    case 16:
        return "Module disabled";

    case 17:
        return "Page access failed";

    default:
        break;
    }

    return "N/A";
}

int sx_ACCESS_REG_MCIA(struct sx_dev *dev, struct ku_access_mcia_reg *reg_data)
{
    const char *err_msg = NULL;
    uint8_t     fw_status = 0;
    int         err = sx_ACCESS_REG_internal(dev,
                                             reg_data->dev_id,
                                             SX_ACCESS_REG_F_SET_AND_GET, /* need decode mcia during writing operation.*/
                                             &reg_data->op_tlv,
                                             __MCIA_encode,
                                             __MCIA_decode,
                                             &reg_data->mcia_reg,
                                             NULL);

    fw_status = sxd_emad_operation_tlv_status_get(&reg_data->op_tlv);
    if (err && fw_status &&
        (fw_status != SXD_EMAD_STATUS_BUSY_E) &&
        (fw_status != SXD_EMAD_STATUS_ISSU_ONGOING_E)) {
        err_msg = __mcia_err_str(reg_data->mcia_reg.status);

        sxd_log_warning(dev, "MCIA failed (fw_err=%u, mcia_status=%u ['%s'])\n",
                        fw_status,
                        reg_data->mcia_reg.status,
                        err_msg);
    }

    return err;
}
EXPORT_SYMBOL(sx_ACCESS_REG_MCIA);

/************************************************
 * MFSL
 ***********************************************/
#define REG_FAN_OFFSET        0x00
#define REG_EE_SHIFT          2
#define REG_FLAGS_OFFSET      0x03
#define REG_TACH_MIN_OFFSET   0x06
#define REG_TACH_MAX_OFFSET   0x0a
#define MFSL_REG_LEN_IN_BYTES (0xc)

static int __MFSL_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_mfsl_reg *mfsl_reg = (struct ku_mfsl_reg*)ku_reg;
    u8                 *reg_tlv = inbox;
    u8                  flag;

    SX_PUT_REG_FIELD(reg_tlv, mfsl_reg->fan, REG_FAN_OFFSET);
    flag = (mfsl_reg->ee << REG_EE_SHIFT);
    flag |= mfsl_reg->ie;
    SX_PUT_REG_FIELD(reg_tlv, flag, REG_FLAGS_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mfsl_reg->tach_min, REG_TACH_MIN_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mfsl_reg->tach_max, REG_TACH_MAX_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + MFSL_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + MFSL_REG_LEN_IN_BYTES;
    return 0;
}

static int __MFSL_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_mfsl_reg *mfsl_reg = (struct ku_mfsl_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u8                  flags;

    SX_GET_REG_FIELD(mfsl_reg->fan, outbox, REG_FAN_OFFSET);
    SX_GET_REG_FIELD(flags, outbox, REG_FLAGS_OFFSET);
    mfsl_reg->ee = (flags >> REG_EE_SHIFT) & 0x3;
    mfsl_reg->ie = flags & 0x1;
    SX_GET_REG_FIELD(mfsl_reg->tach_min, outbox, REG_TACH_MIN_OFFSET);
    SX_GET_REG_FIELD(mfsl_reg->tach_max, outbox, REG_TACH_MAX_OFFSET);
    return 0;
}

int sx_ACCESS_REG_MFSL(struct sx_dev *dev, struct ku_access_mfsl_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __MFSL_encode,
                                  __MFSL_decode,
                                  &reg_data->mfsl_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_MFSL);

/************************************************
 * MTBR
 ***********************************************/
#define REG_BASE_SENSOR_INDEX_OFFSET 0x02
#define REG_MTBR_SLOT_INDEX_OFFSET   0x01
#define REG_NUM_REC_OFFSET           0x07
#define REG_TEMP_RECORD_OFFSET       0x10
#define REG_TEMP_IN_REC_OFFSET       0x12

static int __MTBR_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_mtbr_reg *mtbr_reg = (struct ku_mtbr_reg*)ku_reg;
    u8                 *reg_tlv = inbox;
    u8                  slot_idx;
    u16                 tmp_idx;
    u16                 mtbr_reg_len;

    slot_idx = mtbr_reg->slot_index & 0xF;
    tmp_idx = mtbr_reg->base_sensor_index & 0xFFF;
    SX_PUT_REG_FIELD(reg_tlv, slot_idx, REG_MTBR_SLOT_INDEX_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, tmp_idx, REG_BASE_SENSOR_INDEX_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mtbr_reg->num_rec, REG_NUM_REC_OFFSET);
    mtbr_reg_len = 4 + 4 * mtbr_reg->num_rec;

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + mtbr_reg_len);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + mtbr_reg_len;
    return 0;
}

static int __MTBR_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_mtbr_reg *mtbr_reg = (struct ku_mtbr_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    int                 counter;
    u8                  slot_idx;
    u16                 tmp_idx;

    SX_GET_REG_FIELD(slot_idx, outbox, REG_MTBR_SLOT_INDEX_OFFSET);
    mtbr_reg->slot_index = slot_idx & 0xF;
    SX_GET_REG_FIELD(tmp_idx, outbox, REG_BASE_SENSOR_INDEX_OFFSET);
    mtbr_reg->base_sensor_index = tmp_idx & 0xFFF;
    SX_GET_REG_FIELD(mtbr_reg->num_rec, outbox, REG_NUM_REC_OFFSET);
    if (mtbr_reg->num_rec > MTBR_MAX_TEMPERATURE_RECORDS) {
        sxd_log_err(SXD_DEV_NONE, "response MTBR num_rec %d is greater than max records %d.\n",
                    mtbr_reg->num_rec, MTBR_MAX_TEMPERATURE_RECORDS);
        return -EINVAL;
    }
    for (counter = 0; counter < mtbr_reg->num_rec; counter++) {
        SX_GET_REG_FIELD(mtbr_reg->temperature_record[counter].max_temperature,
                         outbox,
                         REG_TEMP_RECORD_OFFSET + (counter * sizeof(mtbr_reg->temperature_record[0])));
        SX_GET_REG_FIELD(mtbr_reg->temperature_record[counter].temperature,
                         outbox,
                         REG_TEMP_IN_REC_OFFSET + (counter * sizeof(mtbr_reg->temperature_record[0])));
    }

    return 0;
}

int sx_ACCESS_REG_MTBR(struct sx_dev *dev, struct ku_access_mtbr_reg *reg_data)
{
    if (reg_data->mtbr_reg.num_rec > MTBR_MAX_TEMPERATURE_RECORDS) {
        sxd_log_err(dev, "request MTBR num_rec %d is greater than max records %d.\n",
                    reg_data->mtbr_reg.num_rec, MTBR_MAX_TEMPERATURE_RECORDS);
        return -EINVAL;
    }

    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __MTBR_encode,
                                  __MTBR_decode,
                                  &reg_data->mtbr_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_MTBR);


/************************************************
 * MMDIO
 ***********************************************/
#define REG_MDIO_INDEX_OFFSET  0x01
#define REG_OPERATION_OFFSET   0x03
#define REG_ADDRESS_OFFSET     0x04
#define REG_DATA_OFFSET        0x08
#define MMDIO_REG_LEN_IN_BYTES (0xc)

static int __MMDIO_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_mmdio_reg *mmdio_reg = (struct ku_mmdio_reg*)ku_reg;
    u8                  *reg_tlv = inbox;

    SX_PUT_REG_FIELD(reg_tlv, mmdio_reg->mdio_index, REG_MDIO_INDEX_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mmdio_reg->operation, REG_OPERATION_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mmdio_reg->address, REG_ADDRESS_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mmdio_reg->data, REG_DATA_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + MMDIO_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + MMDIO_REG_LEN_IN_BYTES;
    return 0;
}

static int __MMDIO_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_mmdio_reg *mmdio_reg = (struct ku_mmdio_reg*)ku_reg;
    u8                  *outbox = tlv_markers->reg_tlv->reg_data;

    SX_GET_REG_FIELD(mmdio_reg->mdio_index, outbox, REG_MDIO_INDEX_OFFSET);
    SX_GET_REG_FIELD(mmdio_reg->operation, outbox, REG_OPERATION_OFFSET);
    SX_GET_REG_FIELD(mmdio_reg->address, outbox, REG_ADDRESS_OFFSET);
    SX_GET_REG_FIELD(mmdio_reg->data, outbox, REG_DATA_OFFSET);
    return 0;
}

int sx_ACCESS_REG_MMDIO(struct sx_dev *dev, struct ku_access_mmdio_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __MMDIO_encode,
                                  __MMDIO_decode,
                                  &reg_data->mmdio_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_MMDIO);


/************************************************
 * MMIA
 ***********************************************/
#define REG_OPERATION_OFFSET  0x03
#define REG_DATA_OFFSET       0x08
#define MMIA_REG_LEN_IN_BYTES (0x10)

static int __MMIA_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_mmia_reg *mmia_reg = (struct ku_mmia_reg*)ku_reg;
    u8                 *reg_tlv = inbox;

    SX_PUT_REG_FIELD(reg_tlv, mmia_reg->operation, REG_OPERATION_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mmia_reg->data, REG_DATA_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + MMIA_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + MMIA_REG_LEN_IN_BYTES;
    return 0;
}

static int __MMIA_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_mmia_reg *mmia_reg = (struct ku_mmia_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;

    SX_GET_REG_FIELD(mmia_reg->operation, outbox, REG_OPERATION_OFFSET);
    SX_GET_REG_FIELD(mmia_reg->data, outbox, REG_DATA_OFFSET);
    return 0;
}

int sx_ACCESS_REG_MMIA(struct sx_dev *dev, struct ku_access_mmia_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __MMIA_encode,
                                  __MMIA_decode,
                                  &reg_data->mmia_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_MMIA);


/************************************************
 * MFPA
 ***********************************************/
#define REG_P_OFFSET                0x02
#define REG_FS_OFFSET               0x03
#define REG_FS_BITS_SHIFT           4
#define REG_BOOT_ADDRESS_OFFSET     0x04
#define REG_FLASH_NUM_OFFSET        0x13
#define REG_JEDEC_ID_OFFSET         0x14
#define REG_BLOCK_ALLIGNMENT_OFFSET 0x19
#define REG_SECTOR_SIZE_OFFSET      0x1b
#define REG_CAPABILITY_MASK_OFFSET  0x1C
#define REG_CAPABILITY_MASK_BIT_N   7
#define MFPA_REG_LEN_IN_BYTES       (0x20)

static int __MFPA_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_mfpa_reg *mfpa_reg = (struct ku_mfpa_reg*)ku_reg;
    u8                 *reg_tlv = inbox;
    u8                  flag;

    SX_PUT_REG_FIELD(reg_tlv, mfpa_reg->p, REG_P_OFFSET);
    flag = (u8)(mfpa_reg->fs << REG_FS_BITS_SHIFT);
    SX_PUT_REG_FIELD(reg_tlv, flag, REG_FS_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mfpa_reg->boot_address, REG_BOOT_ADDRESS_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + MFPA_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + MFPA_REG_LEN_IN_BYTES;
    return 0;
}

static int __MFPA_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_mfpa_reg *mfpa_reg = (struct ku_mfpa_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u8                  flag;

    SX_GET_REG_FIELD(mfpa_reg->p, outbox, REG_P_OFFSET);
    SX_GET_REG_FIELD(flag, outbox, REG_FS_OFFSET);
    mfpa_reg->fs = (u8)(flag >> REG_FS_BITS_SHIFT);
    SX_GET_REG_FIELD(mfpa_reg->boot_address, outbox, REG_BOOT_ADDRESS_OFFSET);
    SX_GET_REG_FIELD(mfpa_reg->flash_num, outbox, REG_FLASH_NUM_OFFSET);
    SX_GET_REG_FIELD(mfpa_reg->jedec_id, outbox, REG_JEDEC_ID_OFFSET);
    SX_GET_REG_FIELD(mfpa_reg->block_allignment, outbox, REG_BLOCK_ALLIGNMENT_OFFSET);
    SX_GET_REG_FIELD(mfpa_reg->sector_size, outbox, REG_SECTOR_SIZE_OFFSET);
    SX_GET_REG_FIELD(flag, outbox, REG_CAPABILITY_MASK_OFFSET);
    mfpa_reg->capability_mask = (u8)(flag >> REG_CAPABILITY_MASK_BIT_N);
    return 0;
}

int sx_ACCESS_REG_MFPA(struct sx_dev *dev, struct ku_access_mfpa_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __MFPA_encode,
                                  __MFPA_decode,
                                  &reg_data->mfpa_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_MFPA);


/************************************************
 * MFBE
 ***********************************************/
#define REG_P_OFFSET            0x02
#define REG_FS_OFFSET           0x03
#define REG_FS_BITS_SHIFT       4
#define REG_MFBE_ADDRESS_OFFSET 0x08
#define MFBE_REG_LEN_IN_BYTES   (0xc)

static int __MFBE_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_mfbe_reg *mfbe_reg = (struct ku_mfbe_reg*)ku_reg;
    u8                 *reg_tlv = inbox;
    u8                  flag;

    SX_PUT_REG_FIELD(reg_tlv, mfbe_reg->p, REG_P_OFFSET);
    flag = (u8)(mfbe_reg->fs << REG_FS_BITS_SHIFT);
    SX_PUT_REG_FIELD(reg_tlv, flag, REG_FS_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mfbe_reg->address, REG_MFBE_ADDRESS_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + MFBE_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + MFBE_REG_LEN_IN_BYTES;
    return 0;
}

static int __MFBE_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_mfbe_reg *mfbe_reg = (struct ku_mfbe_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u8                  flag;

    SX_GET_REG_FIELD(mfbe_reg->p, outbox, REG_P_OFFSET);
    SX_GET_REG_FIELD(flag, outbox, REG_FS_OFFSET);
    mfbe_reg->fs = (u8)(flag >> REG_FS_BITS_SHIFT);
    SX_GET_REG_FIELD(mfbe_reg->address, outbox, REG_MFBE_ADDRESS_OFFSET);
    return 0;
}

int sx_ACCESS_REG_MFBE(struct sx_dev *dev, struct ku_access_mfbe_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __MFBE_encode,
                                  __MFBE_decode,
                                  &reg_data->mfbe_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_MFBE);


/************************************************
 * MFBA
 ***********************************************/
#define REG_P_OFFSET            0x02
#define REG_FS_OFFSET           0x03
#define REG_FS_BITS_SHIFT       4
#define REG_MFBA_SIZE_OFFSET    0x06
#define REG_MFBA_ADDRESS_OFFSET 0x08
#define REG_MFBA_DATA_OFFSET    0x0C
#define MFBA_REG_LEN_IN_BYTES   (0x10c)

static int __MFBA_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_mfba_reg *mfba_reg = (struct ku_mfba_reg*)ku_reg;
    u8                 *reg_tlv = inbox;
    u8                  flag;
    int                 i;

    SX_PUT_REG_FIELD(reg_tlv, mfba_reg->p, REG_P_OFFSET);
    flag = (u8)(mfba_reg->fs << REG_FS_BITS_SHIFT);
    SX_PUT_REG_FIELD(reg_tlv, flag, REG_FS_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mfba_reg->size, REG_MFBA_SIZE_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mfba_reg->address, REG_MFBA_ADDRESS_OFFSET);
    for (i = 0; i < 192; i++) {
        SX_PUT_REG_FIELD(reg_tlv, mfba_reg->data[i], REG_MFBA_DATA_OFFSET + i);
    }

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + MFBA_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + MFBA_REG_LEN_IN_BYTES;
    return 0;
}

static int __MFBA_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_mfba_reg *mfba_reg = (struct ku_mfba_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u8                  flag;
    int                 i;

    SX_GET_REG_FIELD(mfba_reg->p, outbox, REG_P_OFFSET);
    SX_GET_REG_FIELD(flag, outbox, REG_FS_OFFSET);
    mfba_reg->fs = (u8)(flag >> REG_FS_BITS_SHIFT);
    SX_GET_REG_FIELD(mfba_reg->size, outbox, REG_MFBA_SIZE_OFFSET);
    SX_GET_REG_FIELD(mfba_reg->address, outbox, REG_MFBA_ADDRESS_OFFSET);
    for (i = 0; i < 192; i++) {
        SX_GET_REG_FIELD(mfba_reg->data[i], outbox, REG_MFBA_DATA_OFFSET + i);
    }

    return 0;
}

int sx_ACCESS_REG_MFBA(struct sx_dev *dev, struct ku_access_mfba_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __MFBA_encode,
                                  __MFBA_decode,
                                  &reg_data->mfba_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_MFBA);


/************************************************
 * RAW
 ***********************************************/
#define OP_TLV_SIZE 0x10

static int __RAW_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_raw_reg  *raw_reg = (struct ku_raw_reg*)ku_reg;
    sxd_emad_reg_tlv_t *reg_tlv;
    u8                 *dst;

    if (raw_reg->is_bootstrap) {
        /* bootstrap already framed the buffer as [Index?][REG TLV][payload] */
        dst = inbox;
        *bytes_written = raw_reg->size;
    } else {
        /* direct RAW callers pass payload only — wrap it with a REG TLV */
        reg_tlv = (sxd_emad_reg_tlv_t*)inbox;
        sxd_emad_tlv_build(reg_tlv,
                           SXD_EMAD_TLV_TYPE_REG_E,
                           sizeof(sxd_emad_reg_tlv_t) + raw_reg->size);
        dst = reg_tlv->reg_data;
        *bytes_written = sizeof(sxd_emad_reg_tlv_t) + raw_reg->size;
    }

    return copy_from_user(dst, raw_reg->buff, raw_reg->size);
}

static int __RAW_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_raw_reg *raw_reg = (struct ku_raw_reg*)ku_reg;
    u8                *outbox;

    if (raw_reg->is_bootstrap) {
        /* return the framed response ([Index?][REG TLV][payload]) as bootstrap deparse expects */
        outbox = (tlv_markers->index_tlv) ? (u8*)tlv_markers->index_tlv :
                 (tlv_markers->multi_index_tlv) ? (u8*)tlv_markers->multi_index_tlv :
                 (u8*)tlv_markers->reg_tlv;
    } else {
        /* return payload only — strip the REG TLV we added on encode */
        if (!tlv_markers->reg_data) {
            return -EINVAL;
        }
        outbox = tlv_markers->reg_data;
    }

    return copy_to_user(raw_reg->buff, outbox, raw_reg->size);
}

int sx_ACCESS_REG_RAW(struct sx_dev *dev, struct ku_access_raw_reg *reg_data)
{
    /* worst case: treat raw_reg.size as payload only (REG TLV added on encode) */
    u32 mailbox_size_needed = OP_TLV_SIZE + sizeof(sxd_emad_reg_tlv_t) +
                              reg_data->raw_reg.size;

    if (SX_MAILBOX_SIZE < mailbox_size_needed) {
        sx_warn(dev, "Cannot send the raw register access request "
                "since the mailbox size %u is too small (need %u)\n",
                (u32)SX_MAILBOX_SIZE, mailbox_size_needed);
        return -ENOMEM;
    }

    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  SX_ACCESS_REG_F_SET_AND_GET |
                                  (raw_reg_access_verbose ? 0 : SX_ACCESS_REG_F_QUIET_ERRORS),
                                  &reg_data->op_tlv,
                                  __RAW_encode,
                                  __RAW_decode,
                                  &reg_data->raw_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_RAW);


/************************************************
 * FORE
 ***********************************************/
#define REG_FAN_UNDER_LIMIT_OFFSET 0x00
#define REG_FAN_OVER_LIMIT_OFFSET  0x04
#define FORE_REG_LEN_IN_BYTES      (0xc)

static int __FORE_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_fore_reg *fore_reg = (struct ku_fore_reg*)ku_reg;
    u8                 *reg_tlv = inbox;

    SX_PUT_REG_FIELD(reg_tlv, fore_reg->fan_under_limit, REG_FAN_UNDER_LIMIT_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, fore_reg->fan_over_limit, REG_FAN_OVER_LIMIT_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + FORE_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + FORE_REG_LEN_IN_BYTES;
    return 0;
}

static int __FORE_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_fore_reg *fore_reg = (struct ku_fore_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;

    SX_GET_REG_FIELD(fore_reg->fan_under_limit, outbox, REG_FAN_UNDER_LIMIT_OFFSET);
    SX_GET_REG_FIELD(fore_reg->fan_over_limit, outbox, REG_FAN_OVER_LIMIT_OFFSET);
    return 0;
}

int sx_ACCESS_REG_FORE(struct sx_dev *dev, struct ku_access_fore_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __FORE_encode,
                                  __FORE_decode,
                                  &reg_data->fore_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_FORE);


/************************************************
 * MFCR
 ***********************************************/
#define REG_PWM_FREQ_OFFSET     0x03
#define REG_TACHO_ACTIVE_OFFSET 0x04
#define REG_PWM_ACTIVE_OFFSET   0x07
#define MFCR_REG_LEN_IN_BYTES   (0x8)

static int __MFCR_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_mfcr_reg *mfcr_reg = (struct ku_mfcr_reg*)ku_reg;
    u8                 *reg_tlv = inbox;

    SX_PUT_REG_FIELD(reg_tlv, mfcr_reg->pwm_frequency, REG_PWM_FREQ_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mfcr_reg->tacho_active, REG_TACHO_ACTIVE_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mfcr_reg->pwm_active, REG_PWM_ACTIVE_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + MFCR_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + MFCR_REG_LEN_IN_BYTES;
    return 0;
}

static int __MFCR_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_mfcr_reg *mfcr_reg = (struct ku_mfcr_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;

    SX_GET_REG_FIELD(mfcr_reg->pwm_frequency, outbox, REG_PWM_FREQ_OFFSET);
    SX_GET_REG_FIELD(mfcr_reg->tacho_active, outbox, REG_TACHO_ACTIVE_OFFSET);
    SX_GET_REG_FIELD(mfcr_reg->pwm_active, outbox, REG_PWM_ACTIVE_OFFSET);
    return 0;
}

int sx_ACCESS_REG_MFCR(struct sx_dev *dev, struct ku_access_mfcr_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __MFCR_encode,
                                  __MFCR_decode,
                                  &reg_data->mfcr_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_MFCR);

/************************************************
 * SPMCR
 ***********************************************/
#define SPMCR_REG_SWID_OFFSET          0x00
#define SPMCR_REG_LOCAL_PORT_OFFSET    0x01
#define SPMCR_REG_MAX_SUB_PORT_OFFSET  0x02
#define SPMCR_REG_BASE_STAG_VID_OFFSET 0x06
#define SPMCR_REG_LEN_IN_BYTES         (0xc)

static int __SPMCR_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_spmcr_reg  *spmcr_reg = (struct ku_spmcr_reg*)ku_reg;
    sxd_emad_index_tlv_t *index_tlv;
    u8                   *reg_tlv;

    if (flags & SX_EMAD_F_INDEXATION_TLV) {
        index_tlv = (sxd_emad_index_tlv_t*)inbox;
        reg_tlv = inbox + sizeof(sxd_emad_index_tlv_t);
        sxd_emad_index_tlv_port_single_build(index_tlv, 0, spmcr_reg->local_port);
        *bytes_written += sizeof(sxd_emad_index_tlv_t);
    } else {
        reg_tlv = inbox;
        SX_PUT_REG_FIELD(reg_tlv, spmcr_reg->local_port, SPMCR_REG_LOCAL_PORT_OFFSET);
    }

    SX_PUT_REG_FIELD(reg_tlv, spmcr_reg->swid, SPMCR_REG_SWID_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, spmcr_reg->max_sub_port, SPMCR_REG_MAX_SUB_PORT_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, spmcr_reg->base_stag_vid, SPMCR_REG_BASE_STAG_VID_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + SPMCR_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + SPMCR_REG_LEN_IN_BYTES;
    return 0;
}

static int __SPMCR_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_spmcr_reg *spmcr_reg = (struct ku_spmcr_reg*)ku_reg;
    u8                  *outbox = tlv_markers->reg_tlv->reg_data;
    u8                   lp_msb_dummy;

    if (tlv_markers->index_tlv) {
        sxd_emad_index_tlv_port_single_deparse(tlv_markers->index_tlv,
                                               &lp_msb_dummy,
                                               &spmcr_reg->local_port);
    } else {
        SX_GET_REG_FIELD(spmcr_reg->local_port, outbox, SPMCR_REG_LOCAL_PORT_OFFSET);
    }

    SX_GET_REG_FIELD(spmcr_reg->swid, outbox, SPMCR_REG_SWID_OFFSET);
    SX_GET_REG_FIELD(spmcr_reg->max_sub_port, outbox, SPMCR_REG_MAX_SUB_PORT_OFFSET);
    SX_GET_REG_FIELD(spmcr_reg->swid, outbox, SPMCR_REG_BASE_STAG_VID_OFFSET);
    return 0;
}

int sx_ACCESS_REG_SPMCR(struct sx_dev *dev, struct ku_access_spmcr_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __SPMCR_encode,
                                  __SPMCR_decode,
                                  &reg_data->spmcr_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_SPMCR);

/************************************************
 * SMID
 ***********************************************/
#define SMID_REG_SWID_OFFSET 0x00
#define SMID_REG_MID_OFFSET  0x02

#define SMID_REG_PORTS_255_224_OFFSET 0x20
#define SMID_REG_PORTS_223_192_OFFSET 0x24
#define SMID_REG_PORTS_191_160_OFFSET 0x28
#define SMID_REG_PORTS_159_128_OFFSET 0x2C
#define SMID_REG_PORTS_127_96_OFFSET  0x30
#define SMID_REG_PORTS_95_64_OFFSET   0x34
#define SMID_REG_PORTS_63_32_OFFSET   0x38
#define SMID_REG_PORTS_31_0_OFFSET    0x3C

#define SMID_REG_PORTS_255_224_MASK_OFFSET 0x40
#define SMID_REG_PORTS_223_192_MASK_OFFSET 0x44
#define SMID_REG_PORTS_191_160_MASK_OFFSET 0x48
#define SMID_REG_PORTS_159_128_MASK_OFFSET 0x4C
#define SMID_REG_PORTS_127_96_MASK_OFFSET  0x50
#define SMID_REG_PORTS_95_64_MASK_OFFSET   0x54
#define SMID_REG_PORTS_63_32_MASK_OFFSET   0x58
#define SMID_REG_PORTS_31_0_MASK_OFFSET    0x5C

#define SMID_REG_LEN_IN_BYTES (0x240)

static int __SMID_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_smid_reg *smid_reg = (struct ku_smid_reg*)ku_reg;
    u8                 *reg_tlv = inbox;
    u32                 tmp_val_u32;
    u32                 tmp_mask_u32;
    int                 i, j;

    SX_PUT_REG_FIELD(reg_tlv, smid_reg->swid, SMID_REG_SWID_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, smid_reg->mid, SMID_REG_MID_OFFSET);

    for (j = 0; j < 256; j += 32) {
        tmp_val_u32 = 0;
        tmp_mask_u32 = 0;

        for (i = 0; i < 32; i++) {
            tmp_val_u32 |= (smid_reg->ports_bitmap[j + i] ? (1 << i) : 0);
            tmp_mask_u32 |= (smid_reg->mask_bitmap[j + i] ? (1 << i) : 0);
        }

        if (j == 0) {
            SX_PUT_REG_FIELD(reg_tlv, tmp_val_u32, SMID_REG_PORTS_31_0_OFFSET);
            SX_PUT_REG_FIELD(reg_tlv, tmp_mask_u32, SMID_REG_PORTS_31_0_MASK_OFFSET);
        } else if (j == 32) {
            SX_PUT_REG_FIELD(reg_tlv, tmp_val_u32, SMID_REG_PORTS_63_32_OFFSET);
            SX_PUT_REG_FIELD(reg_tlv, tmp_mask_u32, SMID_REG_PORTS_63_32_MASK_OFFSET);
        } else if (j == 64) {
            SX_PUT_REG_FIELD(reg_tlv, tmp_val_u32, SMID_REG_PORTS_95_64_OFFSET);
            SX_PUT_REG_FIELD(reg_tlv, tmp_mask_u32, SMID_REG_PORTS_95_64_MASK_OFFSET);
        } else if (j == 96) {
            SX_PUT_REG_FIELD(reg_tlv, tmp_val_u32, SMID_REG_PORTS_127_96_OFFSET);
            SX_PUT_REG_FIELD(reg_tlv, tmp_mask_u32, SMID_REG_PORTS_127_96_MASK_OFFSET);
        } else if (j == 128) {
            SX_PUT_REG_FIELD(reg_tlv, tmp_val_u32, SMID_REG_PORTS_159_128_OFFSET);
            SX_PUT_REG_FIELD(reg_tlv, tmp_mask_u32, SMID_REG_PORTS_159_128_MASK_OFFSET);
        } else if (j == 160) {
            SX_PUT_REG_FIELD(reg_tlv, tmp_val_u32, SMID_REG_PORTS_191_160_OFFSET);
            SX_PUT_REG_FIELD(reg_tlv, tmp_mask_u32, SMID_REG_PORTS_191_160_MASK_OFFSET);
        } else if (j == 192) {
            SX_PUT_REG_FIELD(reg_tlv, tmp_val_u32, SMID_REG_PORTS_223_192_OFFSET);
            SX_PUT_REG_FIELD(reg_tlv, tmp_mask_u32, SMID_REG_PORTS_223_192_MASK_OFFSET);
        } else if (j == 224) {
            SX_PUT_REG_FIELD(reg_tlv, tmp_val_u32, SMID_REG_PORTS_255_224_OFFSET);
            SX_PUT_REG_FIELD(reg_tlv, tmp_mask_u32, SMID_REG_PORTS_255_224_MASK_OFFSET);
        }
    }

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + SMID_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + SMID_REG_LEN_IN_BYTES;
    return 0;
}

static int __SMID_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_smid_reg *smid_reg = (struct ku_smid_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u32                 tmp_val_u32;
    int                 i;

    SX_GET_REG_FIELD(smid_reg->swid, outbox, SMID_REG_SWID_OFFSET);
    SX_GET_REG_FIELD(smid_reg->mid, outbox, SMID_REG_MID_OFFSET);
    /*
     *  SX_GET_REG_FIELD(tmp_val_u32, outbox,SMID_REG_PORTS_255_224_OFFSET);
     *  SX_GET_REG_FIELD(tmp_val_u32, outbox,SMID_REG_PORTS_223_192_OFFSET);
     *  SX_GET_REG_FIELD(tmp_val_u32, outbox,SMID_REG_PORTS_191_160_OFFSET);
     *  SX_GET_REG_FIELD(tmp_val_u32, outbox,SMID_REG_PORTS_159_128_OFFSET);
     *  SX_GET_REG_FIELD(tmp_val_u32, outbox,SMID_REG_PORTS_127_96_OFFSET);
     *  SX_GET_REG_FIELD(tmp_val_u32, outbox,SMID_REG_PORTS_95_64_OFFSET);
     */
    SX_GET_REG_FIELD(tmp_val_u32, outbox, SMID_REG_PORTS_63_32_OFFSET);
    for (i = 0; i < 32; i++) {
        if (tmp_val_u32 & (1 << i)) {
            smid_reg->ports_bitmap[32 + i] = 1;
        }
    }
    SX_GET_REG_FIELD(tmp_val_u32, outbox, SMID_REG_PORTS_31_0_OFFSET);
    for (i = 0; i < 32; i++) {
        if (tmp_val_u32 & (1 << i)) {
            smid_reg->ports_bitmap[i] = 1;
        }
    }

    return 0;
}

int sx_ACCESS_REG_SMID(struct sx_dev *dev, struct ku_access_smid_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __SMID_encode,
                                  __SMID_decode,
                                  &reg_data->smid_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_SMID);


/************************************************
 * SPMS
 ***********************************************/
#define SPMS_DWORD0_OFFSET    0x00
#define SPMS_DWORD1_OFFSET    0x04
#define SPMS_REG_LEN_IN_BYTES (0x404)

static int __SPMS_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_spms_reg   *spms_reg = (struct ku_spms_reg*)ku_reg;
    sxd_emad_index_tlv_t *index_tlv;
    u8                   *reg_tlv;
    u32                   tmp_u32;
    int                   i, j, vlan = 4095;

    if (flags & SX_EMAD_F_INDEXATION_TLV) {
        index_tlv = (sxd_emad_index_tlv_t*)inbox;
        reg_tlv = inbox + sizeof(sxd_emad_index_tlv_t);
        sxd_emad_index_tlv_port_single_build(index_tlv, spms_reg->lp_msb, spms_reg->local_port);
        *bytes_written += sizeof(sxd_emad_index_tlv_t);
    } else {
        reg_tlv = inbox;
        tmp_u32 = (((u32)(spms_reg->local_port & 0xff)) << 16) |
                  (((u32)(spms_reg->lp_msb & 0x3)) << 12);
        SX_PUT_REG_FIELD(reg_tlv, tmp_u32, SPMS_DWORD0_OFFSET);
    }

    for (i = 0; i < 256; i++) { /* dwords */
        tmp_u32 = 0;
        for (j = 30; j >= 0; j -= 2) { /* states per dword */
            tmp_u32 |= ((u32)(spms_reg->state[vlan--] & 0x3)) << j;
        }
        SX_PUT_REG_FIELD(reg_tlv, tmp_u32, (SPMS_DWORD1_OFFSET + (i * 4)));
    }

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + SPMS_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + SPMS_REG_LEN_IN_BYTES;
    return 0;
}

static int __SPMS_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_spms_reg *spms_reg = (struct ku_spms_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u32                 tmp_u32;
    int                 i, j, vlan = 4095;

    if (tlv_markers->index_tlv) {
        sxd_emad_index_tlv_port_single_deparse(tlv_markers->index_tlv,
                                               &spms_reg->lp_msb,
                                               &spms_reg->local_port);
    } else {
        SX_GET_REG_FIELD(tmp_u32, outbox, SPMS_DWORD0_OFFSET);
        spms_reg->local_port = (tmp_u32 >> 16) & 0xff;
        spms_reg->lp_msb = (tmp_u32 >> 12) & 0x3;
    }

    for (i = 0; i < 256; i++) { /* dwords */
        SX_GET_REG_FIELD(tmp_u32, outbox, (SPMS_DWORD1_OFFSET + (i * 4)));
        for (j = 30; j >= 0; j -= 2) { /* states per dword */
            spms_reg->state[vlan--] = (tmp_u32 >> j) & 0x3;
        }
    }

    return 0;
}

int sx_ACCESS_REG_SPMS(struct sx_dev *dev, struct ku_access_spms_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __SPMS_encode,
                                  __SPMS_decode,
                                  &reg_data->spms_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_SPMS);

/************************************************
 * PLBF
 ***********************************************/
#define PLBF_LOCAL_PORT_OFFSET     0x01
#define PLBF_LOCAL_PORT_MSB_OFFSET 0x02
#define PLBF_LBF_MODE_OFFSET       0x03
#define PLBF_REG_LEN_IN_BYTES      (0x8)

static int __PLBF_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_plbf_reg   *plbf_reg = (struct ku_plbf_reg*)ku_reg;
    sxd_emad_index_tlv_t *index_tlv;
    u8                   *reg_tlv;

    if (flags & SX_EMAD_F_INDEXATION_TLV) {
        index_tlv = (sxd_emad_index_tlv_t*)inbox;
        reg_tlv = inbox + sizeof(sxd_emad_index_tlv_t);
        sxd_emad_index_tlv_port_single_build(index_tlv, plbf_reg->lp_msb, plbf_reg->port);
        *bytes_written += sizeof(sxd_emad_index_tlv_t);
    } else {
        reg_tlv = inbox;
        SX_PUT_REG_FIELD(reg_tlv, ((plbf_reg->lp_msb & 0x3) << 4), PLBF_LOCAL_PORT_MSB_OFFSET);
    }

    SX_PUT_REG_FIELD(reg_tlv, plbf_reg->port, PLBF_LOCAL_PORT_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, plbf_reg->lbf_mode, PLBF_LBF_MODE_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + PLBF_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + PLBF_REG_LEN_IN_BYTES;
    return 0;
}

static int __PLBF_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_plbf_reg *plbf_reg = (struct ku_plbf_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u8                  temp = 0;

    if (tlv_markers->index_tlv) {
        sxd_emad_index_tlv_port_single_deparse(tlv_markers->index_tlv,
                                               &plbf_reg->lp_msb,
                                               &plbf_reg->port);
    } else {
        SX_GET_REG_FIELD(plbf_reg->port, outbox, PLBF_LOCAL_PORT_OFFSET);
        SX_GET_REG_FIELD(temp, outbox, PLBF_LOCAL_PORT_MSB_OFFSET);
        plbf_reg->lp_msb = (temp >> 4) & 0x3;
    }
    SX_GET_REG_FIELD(plbf_reg->lbf_mode, outbox, PLBF_LBF_MODE_OFFSET);
    return 0;
}

int sx_ACCESS_REG_PLBF(struct sx_dev *dev, struct ku_access_plbf_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __PLBF_encode,
                                  __PLBF_decode,
                                  &reg_data->plbf_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_PLBF);

/************************************************
 * MSCI
 ***********************************************/
#define MSCI_INDEX_OFFSET     0x03
#define MSCI_VERSION_OFFSET   0x04
#define MSCI_REG_LEN_IN_BYTES (0xc)

static int __MSCI_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_msci_reg *msci_reg = (struct ku_msci_reg*)ku_reg;
    u8                 *reg_tlv = inbox;

    SX_PUT_REG_FIELD(reg_tlv, msci_reg->index, MSCI_INDEX_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + MSCI_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + MSCI_REG_LEN_IN_BYTES;
    return 0;
}

static int __MSCI_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_msci_reg *msci_reg = (struct ku_msci_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;

    SX_GET_REG_FIELD(msci_reg->index, outbox, MSCI_INDEX_OFFSET);
    SX_GET_REG_FIELD(msci_reg->version, outbox, MSCI_VERSION_OFFSET);
    return 0;
}

int sx_ACCESS_REG_MSCI(struct sx_dev *dev, struct ku_access_msci_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __MSCI_encode,
                                  __MSCI_decode,
                                  &reg_data->msci_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_MSCI);


/************************************************
 * MRSR
 ***********************************************/
#define MRSR_COMMAND_OFFSET   0x03
#define MRSR_REG_LEN_IN_BYTES (0x8)

static int __MRSR_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_mrsr_reg *mrsr_reg = (struct ku_mrsr_reg*)ku_reg;
    u8                 *reg_tlv = inbox;
    u8                  tmp;

    tmp = (u8)mrsr_reg->command;
    SX_PUT_REG_FIELD(reg_tlv, tmp, MRSR_COMMAND_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + MRSR_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + MRSR_REG_LEN_IN_BYTES;
    return 0;
}

int sx_ACCESS_REG_MRSR(struct sx_dev *dev, struct ku_access_mrsr_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  /* Ignore the error from FW regarding MRSR register */
                                  SX_ACCESS_REG_F_IGNORE_FW_RET_CODE,
                                  &reg_data->op_tlv,
                                  __MRSR_encode,
                                  NULL,
                                  &reg_data->mrsr_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_MRSR);


/************************************************
 * RGCR
 ***********************************************/
#define RGCR_DWORD0_OFFSET    0x00
#define RGCR_REG_LEN_IN_BYTES (0x28)

static int __RGCR_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_rgcr_reg *rgcr_reg = (struct ku_rgcr_reg*)ku_reg;
    u8                 *reg_tlv = inbox;
    u32                 tmp_u32;

    tmp_u32 = (((u32)(rgcr_reg->ipv4_enable & 0x1)) << 31) |
              (((u32)(rgcr_reg->ipv6_enable & 0x1)) << 30) |
              (((u32)(rgcr_reg->mpls_enable & 0x1)) << 29);
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, RGCR_DWORD0_OFFSET);

    tmp_u32 = rgcr_reg->max_vlan_router_interfaces & 0xffff;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, RGCR_DWORD0_OFFSET + 4);

    tmp_u32 = rgcr_reg->max_port_router_interfaces & 0xffff;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, RGCR_DWORD0_OFFSET + 8);

    tmp_u32 = rgcr_reg->max_pkey_router_interfaces & 0xffff;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, RGCR_DWORD0_OFFSET + 12);

    tmp_u32 = rgcr_reg->max_router_interfaces & 0xffff;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, RGCR_DWORD0_OFFSET + 16);

    tmp_u32 = rgcr_reg->max_virtual_routers & 0xffff;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, RGCR_DWORD0_OFFSET + 20);

    tmp_u32 = (((u32)(rgcr_reg->grht & 0xf)) << 24) |
              (((u32)(rgcr_reg->usp & 0x1)) << 20) |
              (((u32)(rgcr_reg->pcp_rw & 0x3)) << 16) |
              (((u32)(rgcr_reg->ipb & 0x1)) << 9) |
              (((u32)(rgcr_reg->allr & 0x1)) << 8) |
              (((u32)(rgcr_reg->mcsi & 0x1)) << 4) |
              (rgcr_reg->rpf & 0x1);
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, RGCR_DWORD0_OFFSET + 24);

    tmp_u32 = (((u32)(rgcr_reg->ipv6_op_type & 0x3)) << 24) |
              (((u32)(rgcr_reg->ipv6_packet_rate & 0x1f)) << 16) |
              (((u32)(rgcr_reg->ipv4_op_type & 0x3)) << 8) |
              (rgcr_reg->ipv4_packet_rate & 0x1f);
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, RGCR_DWORD0_OFFSET + 28);

    tmp_u32 = (((u32)(rgcr_reg->grh_hop_limit & 0xff)) << 16);
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, RGCR_DWORD0_OFFSET + 32);

    tmp_u32 = rgcr_reg->expected_irif_list_index_base & 0xffffff;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, RGCR_DWORD0_OFFSET + 36);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + RGCR_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + RGCR_REG_LEN_IN_BYTES;
    return 0;
}

static int __RGCR_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_rgcr_reg *rgcr_reg = (struct ku_rgcr_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u32                 tmp_u32;

    SX_GET_REG_FIELD(tmp_u32, outbox, RGCR_DWORD0_OFFSET);
    rgcr_reg->ipv4_enable = (tmp_u32 >> 31) & 0x1;
    rgcr_reg->ipv6_enable = (tmp_u32 >> 30) & 0x1;
    rgcr_reg->mpls_enable = (tmp_u32 >> 29) & 0x1;

    SX_GET_REG_FIELD(tmp_u32, outbox, RGCR_DWORD0_OFFSET + 4);
    rgcr_reg->max_vlan_router_interfaces = tmp_u32 & 0xffff;

    SX_GET_REG_FIELD(tmp_u32, outbox, RGCR_DWORD0_OFFSET + 8);
    rgcr_reg->max_port_router_interfaces = tmp_u32 & 0xffff;

    SX_GET_REG_FIELD(tmp_u32, outbox, RGCR_DWORD0_OFFSET + 12);
    rgcr_reg->max_pkey_router_interfaces = tmp_u32 & 0xffff;

    SX_GET_REG_FIELD(tmp_u32, outbox, RGCR_DWORD0_OFFSET + 16);
    rgcr_reg->max_router_interfaces = tmp_u32 & 0xffff;

    SX_GET_REG_FIELD(tmp_u32, outbox, RGCR_DWORD0_OFFSET + 20);
    rgcr_reg->max_virtual_routers = tmp_u32 & 0xffff;

    SX_GET_REG_FIELD(tmp_u32, outbox, RGCR_DWORD0_OFFSET + 24);
    rgcr_reg->grht = (tmp_u32 >> 24) & 0xf;
    rgcr_reg->usp = (tmp_u32 >> 20) & 0x1;
    rgcr_reg->pcp_rw = (tmp_u32 >> 16) & 0x3;
    rgcr_reg->ipb = (tmp_u32 >> 9) & 0x1;
    rgcr_reg->allr = (tmp_u32 >> 8) & 0x1;
    rgcr_reg->mcsi = (tmp_u32 >> 4) & 0x1;
    rgcr_reg->rpf = tmp_u32 & 0x1;

    SX_GET_REG_FIELD(tmp_u32, outbox, RGCR_DWORD0_OFFSET + 28);
    rgcr_reg->ipv6_op_type = (tmp_u32 >> 24) & 0x3;
    rgcr_reg->ipv6_packet_rate = (tmp_u32 >> 16) & 0x1f;
    rgcr_reg->ipv4_op_type = (tmp_u32 >> 8) & 0x3;
    rgcr_reg->ipv4_packet_rate = tmp_u32 & 0x1f;

    SX_GET_REG_FIELD(tmp_u32, outbox, RGCR_DWORD0_OFFSET + 32);
    rgcr_reg->grh_hop_limit = (tmp_u32 >> 16) & 0xff;

    SX_GET_REG_FIELD(tmp_u32, outbox, RGCR_DWORD0_OFFSET + 36);
    rgcr_reg->expected_irif_list_index_base = tmp_u32 & 0xffffff;

    return 0;
}

int sx_ACCESS_REG_RGCR(struct sx_dev *dev, struct ku_access_rgcr_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __RGCR_encode,
                                  __RGCR_decode,
                                  &reg_data->rgcr_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_RGCR);


/************************************************
 * RTPS
 ***********************************************/
#define RTPS_REG_SWID_OFFSET   0x00
#define REG_TCA_PHY_LOG_OFFSET 0x03
#define REG_SW_PHY_LOG_OFFSET  0x07
#define RTPS_REG_LEN_IN_BYTES  (0x8)

static int __RTPS_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_rtps_reg *rtps_reg = (struct ku_rtps_reg*)ku_reg;
    u8                 *reg_tlv = inbox;
    u8                  tmp_val_u8;

    SX_PUT_REG_FIELD(reg_tlv, rtps_reg->swid, RTPS_REG_SWID_OFFSET);
    tmp_val_u8 = (rtps_reg->tca_log_pstate & 0xF) << 4;
    tmp_val_u8 |= rtps_reg->tca_phy_pstate & 0xF;
    SX_PUT_REG_FIELD(reg_tlv, tmp_val_u8, REG_TCA_PHY_LOG_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + RTPS_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + RTPS_REG_LEN_IN_BYTES;
    return 0;
}

static int __RTPS_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_rtps_reg *rtps_reg = (struct ku_rtps_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u8                  tmp_val_u8;

    SX_GET_REG_FIELD(rtps_reg->swid, outbox, RTPS_REG_SWID_OFFSET);
    SX_GET_REG_FIELD(tmp_val_u8, outbox, REG_TCA_PHY_LOG_OFFSET);
    rtps_reg->tca_log_pstate = (tmp_val_u8 >> 4) & 0xF;
    rtps_reg->tca_phy_pstate = tmp_val_u8 & 0xF;
    SX_GET_REG_FIELD(tmp_val_u8, outbox, REG_SW_PHY_LOG_OFFSET);
    rtps_reg->switch_log_pstate = (tmp_val_u8 >> 4) & 0xF;
    rtps_reg->switch_phy_pstate = tmp_val_u8 & 0xF;
    return 0;
}

int sx_ACCESS_REG_RTPS(struct sx_dev *dev, struct ku_access_rtps_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __RTPS_encode,
                                  __RTPS_decode,
                                  &reg_data->rtps_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_RTPS);


/************************************************
 * RTCA
 ***********************************************/
#define RTCA_REG_SWID_OFFSET  0x00
#define REG_LMC_OFFSET        0x01
#define REG_LID_OFFSET        0x02
#define REG_GID_OFFSET        0x04
#define GID_SIZE              16
#define RTCA_REG_LEN_IN_BYTES (0x14)

static int __RTCA_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_rtca_reg *rtca_reg = (struct ku_rtca_reg*)ku_reg;
    u8                 *reg_tlv = inbox;
    int                 i;

    SX_PUT_REG_FIELD(reg_tlv, rtca_reg->swid, RTCA_REG_SWID_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, rtca_reg->lmc, REG_LMC_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, rtca_reg->lid, REG_LID_OFFSET);
    for (i = 0; i < GID_SIZE; i++) {
        SX_PUT_REG_FIELD(reg_tlv, rtca_reg->gid.addr_octet[i], i + REG_GID_OFFSET);
    }

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + RTCA_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + RTCA_REG_LEN_IN_BYTES;
    return 0;
}

static int __RTCA_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_rtca_reg *rtca_reg = (struct ku_rtca_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    int                 i;

    SX_GET_REG_FIELD(rtca_reg->swid, outbox, RTCA_REG_SWID_OFFSET);
    SX_GET_REG_FIELD(rtca_reg->lmc, outbox, REG_LMC_OFFSET);
    SX_GET_REG_FIELD(rtca_reg->lid, outbox, REG_LID_OFFSET);

    for (i = 0; i < GID_SIZE; i++) {
        SX_GET_REG_FIELD(rtca_reg->gid.addr_octet[i], outbox, i + REG_GID_OFFSET);
    }

    return 0;
}

int sx_ACCESS_REG_RTCA(struct sx_dev *dev, struct ku_access_rtca_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __RTCA_encode,
                                  __RTCA_decode,
                                  &reg_data->rtca_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_RTCA);

/************************************************
 * RUFT
 ***********************************************/
#define REG_VALID_TYPE_OFFSET     0x00
#define REG_OP_ACT_OFFSET         0x01
#define REG_OFFSET                0x02
#define REG_RUFT_VRID_OFFSET      0x04
#define REG_DIP_OFFSET            0x10
#define REG_DIP_MASK_OFFSET       0x20
#define REG_ECMP_HASH_OFFSET      0x54
#define REG_ECMP_HASH_MASK_OFFSET 0x58
#define REG_TRAP_ACTION_OFFSET    0x60
#define REG_TRAP_GROUP_OFFSET     0x61
#define REG_TRAP_ID_OFFSET        0x62
#define REG_MIRROR_DST_OFFSET     0x64
#define REG_PRIO_QOS_OFFSET       0x65
#define REG_EGRESS_RIF_OFFSET     0x72
#define REG_ECMP_SIZE_OFFSET      0x74
#define REG_TABLE_OFFSET          0x75
#define REG_ADJ_IDX_OFFSET        0x76
#define RUFT_REG_LEN_IN_BYTES     (0x78)

static int __RUFT_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_ruft_reg *ruft_reg = (struct ku_ruft_reg*)ku_reg;
    u8                 *reg_tlv = inbox;
    u8                  tmp_val_u8;
    u16                 tmp_val_u16;
    int                 i;

    tmp_val_u8 = 0;
    tmp_val_u8 |= (ruft_reg->valid & 1) << 7;
    tmp_val_u8 |= (ruft_reg->route_type & 3);
    SX_PUT_REG_FIELD(reg_tlv, tmp_val_u8, REG_VALID_TYPE_OFFSET);
    tmp_val_u8 = 0;
    tmp_val_u8 |= (ruft_reg->operation & 7) << 4;
    tmp_val_u8 |= (ruft_reg->activity & 1);
    SX_PUT_REG_FIELD(reg_tlv, tmp_val_u8, REG_OP_ACT_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, ruft_reg->offset, REG_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, ruft_reg->router, REG_RUFT_VRID_OFFSET);
    for (i = 0; i <= 3; i++) {
        SX_PUT_REG_FIELD(reg_tlv, ruft_reg->destination_ip[i], (REG_DIP_OFFSET + (i * 4)));
        SX_PUT_REG_FIELD(reg_tlv, ruft_reg->destination_ip_mask[i], (REG_DIP_MASK_OFFSET + (i * 4)));
    }
    SX_PUT_REG_FIELD(reg_tlv, ruft_reg->ecmp_hash, REG_ECMP_HASH_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, ruft_reg->ecmp_hash_mask, REG_ECMP_HASH_MASK_OFFSET);
    tmp_val_u8 = 0;
    tmp_val_u8 |= (ruft_reg->route_action & 0xf) << 4;
    SX_PUT_REG_FIELD(reg_tlv, tmp_val_u8, REG_TRAP_ACTION_OFFSET);
    tmp_val_u8 = 0;
    tmp_val_u8 |= (ruft_reg->trap_group & 0xf);
    SX_PUT_REG_FIELD(reg_tlv, tmp_val_u8, REG_TRAP_GROUP_OFFSET);
    tmp_val_u16 = 0;
    tmp_val_u16 |= (ruft_reg->trap_id & 0x01ff);
    SX_PUT_REG_FIELD(reg_tlv, tmp_val_u16, REG_TRAP_ID_OFFSET);
    tmp_val_u8 = 0;
    tmp_val_u8 |= (ruft_reg->mirror_enable & 1) << 7;
    tmp_val_u8 |= (ruft_reg->mirror_dst & 7);
    SX_PUT_REG_FIELD(reg_tlv, tmp_val_u8, REG_MIRROR_DST_OFFSET);
    tmp_val_u8 = 0;
    tmp_val_u8 |= (ruft_reg->prio & 7) << 4;
    tmp_val_u8 |= (ruft_reg->qos & 3);
    SX_PUT_REG_FIELD(reg_tlv, tmp_val_u8, REG_PRIO_QOS_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, ruft_reg->egress_rif, REG_EGRESS_RIF_OFFSET);
    tmp_val_u8 = ruft_reg->ecmp_size;
    SX_PUT_REG_FIELD(reg_tlv, tmp_val_u8, REG_ECMP_SIZE_OFFSET);
    tmp_val_u8 = 0;
    tmp_val_u8 |= (ruft_reg->table & 7);
    SX_PUT_REG_FIELD(reg_tlv, tmp_val_u8, REG_TABLE_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, ruft_reg->adjacency_index, REG_ADJ_IDX_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + RUFT_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + RUFT_REG_LEN_IN_BYTES;
    return 0;
}

static int __RUFT_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_ruft_reg *ruft_reg = (struct ku_ruft_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u8                  tmp_val_u8;
    u16                 tmp_val_u16;
    int                 i;

    SX_GET_REG_FIELD(tmp_val_u8, outbox, REG_VALID_TYPE_OFFSET);
    ruft_reg->valid = (tmp_val_u8 >> 7) & 1;
    ruft_reg->route_type = tmp_val_u8 & 3;
    SX_GET_REG_FIELD(tmp_val_u8, outbox, REG_OP_ACT_OFFSET);
    ruft_reg->operation = (tmp_val_u8 >> 4) & 7;
    ruft_reg->activity = tmp_val_u8 & 1;
    SX_GET_REG_FIELD(ruft_reg->offset, outbox, REG_OFFSET);
    SX_GET_REG_FIELD(ruft_reg->router, outbox, REG_RUFT_VRID_OFFSET);
    for (i = 0; i <= 3; i++) {
        SX_GET_REG_FIELD(ruft_reg->destination_ip[i], outbox, (REG_DIP_OFFSET + (i * 4)));
        SX_GET_REG_FIELD(ruft_reg->destination_ip_mask[i], outbox, (REG_DIP_MASK_OFFSET + (i * 4)));
    }
    SX_GET_REG_FIELD(ruft_reg->ecmp_hash, outbox, REG_ECMP_HASH_OFFSET);
    SX_GET_REG_FIELD(ruft_reg->ecmp_hash_mask, outbox, REG_ECMP_HASH_MASK_OFFSET);
    SX_GET_REG_FIELD(tmp_val_u8, outbox, REG_TRAP_ACTION_OFFSET);
    ruft_reg->route_action = (tmp_val_u8 >> 4) & 0xf;
    SX_GET_REG_FIELD(tmp_val_u8, outbox, REG_TRAP_GROUP_OFFSET);
    ruft_reg->trap_group = tmp_val_u8 & 0xf;
    SX_GET_REG_FIELD(tmp_val_u16, outbox, REG_TRAP_ID_OFFSET);
    ruft_reg->trap_id = tmp_val_u16 & 0x01ff;
    SX_GET_REG_FIELD(tmp_val_u8, outbox, REG_MIRROR_DST_OFFSET);
    ruft_reg->mirror_enable = (tmp_val_u8 >> 7) & 1;
    ruft_reg->mirror_dst = tmp_val_u8 & 7;
    SX_GET_REG_FIELD(tmp_val_u8, outbox, REG_PRIO_QOS_OFFSET);
    ruft_reg->prio = (tmp_val_u8 >> 4) & 7;
    ruft_reg->qos = tmp_val_u8 & 3;
    SX_GET_REG_FIELD(ruft_reg->egress_rif, outbox, REG_EGRESS_RIF_OFFSET);
    SX_GET_REG_FIELD(tmp_val_u8, outbox, REG_ECMP_SIZE_OFFSET);
    ruft_reg->ecmp_size = tmp_val_u8;
    SX_GET_REG_FIELD(tmp_val_u8, outbox, REG_TABLE_OFFSET);
    ruft_reg->table = tmp_val_u8 & 7;
    SX_GET_REG_FIELD(ruft_reg->adjacency_index, outbox, REG_ADJ_IDX_OFFSET);
    return 0;
}

int sx_ACCESS_REG_RUFT(struct sx_dev *dev, struct ku_access_ruft_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __RUFT_encode,
                                  __RUFT_decode,
                                  &reg_data->ruft_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_RUFT);


/************************************************
 * MPSC
 ***********************************************/
#define REG_LOCAL_PORT_MSB_OFFSET 0x02
#define REG_C_E_CONG_OFFSET       0x04
#define REG_C_BIT_N               7
#define REG_E_BIT_N               6
#define REG_CONG_BIT_N            5
#define REG_RATE_OFFSET           0x08
#define REG_COUNT_DROP_OFFSET     0x0C
#define MPSC_REG_LEN_IN_BYTES     (0x14)

static int __MPSC_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_mpsc_reg   *mpsc_reg = (struct ku_mpsc_reg*)ku_reg;
    sxd_emad_index_tlv_t *index_tlv;
    u8                   *reg_tlv;
    u8                    tmp = 0;

    if (flags & SX_EMAD_F_INDEXATION_TLV) {
        index_tlv = (sxd_emad_index_tlv_t*)inbox;
        reg_tlv = inbox + sizeof(sxd_emad_index_tlv_t);
        sxd_emad_index_tlv_port_single_build(index_tlv, mpsc_reg->lp_msb, mpsc_reg->local_port);
        *bytes_written += sizeof(sxd_emad_index_tlv_t);
    } else {
        reg_tlv = inbox;
        SX_PUT_REG_FIELD(reg_tlv, mpsc_reg->local_port, REG_LOCAL_PORT_OFFSET);
        tmp = (mpsc_reg->lp_msb & 0x03) << 4;
        SX_PUT_REG_FIELD(reg_tlv, tmp, REG_LOCAL_PORT_MSB_OFFSET);
    }

    tmp = mpsc_reg->clear_count ? (1 << REG_C_BIT_N) : 0;
    tmp |= mpsc_reg->enable ? (1 << REG_E_BIT_N) : 0;
    tmp |= mpsc_reg->cong ? (1 << REG_CONG_BIT_N) : 0;
    SX_PUT_REG_FIELD(reg_tlv, tmp, REG_C_E_CONG_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mpsc_reg->rate, REG_RATE_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + MPSC_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + MPSC_REG_LEN_IN_BYTES;
    return 0;
}

static int __MPSC_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_mpsc_reg *mpsc_reg = (struct ku_mpsc_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u8                  tmp;

    if (tlv_markers->index_tlv) {
        sxd_emad_index_tlv_port_single_deparse(tlv_markers->index_tlv,
                                               &mpsc_reg->lp_msb,
                                               &mpsc_reg->local_port);
    } else {
        SX_GET_REG_FIELD(mpsc_reg->local_port, outbox, REG_LOCAL_PORT_OFFSET);
        SX_GET_REG_FIELD(mpsc_reg->lp_msb, outbox, REG_LOCAL_PORT_MSB_OFFSET);
        mpsc_reg->lp_msb = (mpsc_reg->lp_msb >> 4) & 0x03;
    }
    SX_GET_REG_FIELD(tmp, outbox, REG_C_E_CONG_OFFSET);
    mpsc_reg->enable = tmp & (1 << REG_E_BIT_N) ? 1 : 0;
    mpsc_reg->cong = tmp & (1 << REG_CONG_BIT_N) ? 1 : 0;
    SX_GET_REG_FIELD(mpsc_reg->rate, outbox, REG_RATE_OFFSET);
    SX_GET_REG_FIELD(mpsc_reg->count_drops, outbox, REG_COUNT_DROP_OFFSET);
    return 0;
}

int sx_ACCESS_REG_MPSC(struct sx_dev *dev, struct ku_access_mpsc_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __MPSC_encode,
                                  __MPSC_decode,
                                  &reg_data->mpsc_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_MPSC);


/************************************************
 * MLCR
 ***********************************************/
#define REG_LP_MSB_OFFSET               0x00
#define REG_LED_TYPE_OFFSET             0x03
#define REG_BEACON_DURATION_OFFSET      0x06
#define REG_BEACON_REMAIN_OFFSET        0x0a
#define REG_CAP_LOCAL_OR_UID_ONLY_BIT_N 4
#define MLCR_REG_LEN_IN_BYTES           (0xc)

static int __MLCR_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_mlcr_reg   *mlcr_reg = (struct ku_mlcr_reg*)ku_reg;
    sxd_emad_index_tlv_t *index_tlv;
    u8                   *reg_tlv;
    u8                    tmp_u8 = 0;

    if (flags & SX_EMAD_F_INDEXATION_TLV) {
        index_tlv = (sxd_emad_index_tlv_t*)inbox;
        reg_tlv = inbox + sizeof(sxd_emad_index_tlv_t);
        sxd_emad_index_tlv_port_single_build(index_tlv, mlcr_reg->lp_msb, mlcr_reg->local_port);
        *bytes_written += sizeof(sxd_emad_index_tlv_t);
    } else {
        reg_tlv = inbox;
        SX_PUT_REG_FIELD(reg_tlv, mlcr_reg->local_port, REG_LOCAL_PORT_OFFSET);
        tmp_u8 = mlcr_reg->lp_msb & 0x3;
        SX_PUT_REG_FIELD(reg_tlv, tmp_u8, REG_LP_MSB_OFFSET);
    }

    SX_PUT_REG_FIELD(reg_tlv, mlcr_reg->led_type, REG_LED_TYPE_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mlcr_reg->beacon_duration, REG_BEACON_DURATION_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mlcr_reg->beacon_remain, REG_BEACON_REMAIN_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + MLCR_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + MLCR_REG_LEN_IN_BYTES;
    return 0;
}

static int __MLCR_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_mlcr_reg *mlcr_reg = (struct ku_mlcr_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u8                  tmp_u8;

    if (tlv_markers->index_tlv) {
        sxd_emad_index_tlv_port_single_deparse(tlv_markers->index_tlv,
                                               &mlcr_reg->lp_msb,
                                               &mlcr_reg->local_port);
    } else {
        SX_GET_REG_FIELD(mlcr_reg->local_port, outbox, REG_LOCAL_PORT_OFFSET);
        SX_GET_REG_FIELD(mlcr_reg->lp_msb, outbox, REG_LP_MSB_OFFSET);
        mlcr_reg->lp_msb &= 0x3;
    }
    SX_GET_REG_FIELD(tmp_u8, outbox, REG_LED_TYPE_OFFSET);
    mlcr_reg->cap_local_or_uid_only = tmp_u8 & (1 << REG_CAP_LOCAL_OR_UID_ONLY_BIT_N) ? 1 : 0;
    mlcr_reg->led_type = tmp_u8 & 0xf;
    SX_GET_REG_FIELD(mlcr_reg->beacon_duration, outbox, REG_BEACON_DURATION_OFFSET);
    SX_GET_REG_FIELD(mlcr_reg->beacon_remain, outbox, REG_BEACON_REMAIN_OFFSET);
    return 0;
}

int sx_ACCESS_REG_MLCR(struct sx_dev *dev, struct ku_access_mlcr_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __MLCR_encode,
                                  __MLCR_decode,
                                  &reg_data->mlcr_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_MLCR);

/************************************************
 * MTPTPT
 ***********************************************/
#define REG_MTPTPT_TRAP_ID_OFFSET  0x03
#define REG_MTPTPT_MSG_TYPE_OFFSET 0x06
#define MTPTPT_REG_LEN_IN_BYTES    (0x8)

static int __MTPTPT_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_mtptpt_reg *mtptpt_reg = (struct ku_mtptpt_reg*)ku_reg;
    u8                   *reg_tlv = inbox;

    SX_PUT_REG_FIELD(reg_tlv, mtptpt_reg->trap_id, REG_MTPTPT_TRAP_ID_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, mtptpt_reg->message_type, REG_MTPTPT_MSG_TYPE_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + MTPTPT_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + MTPTPT_REG_LEN_IN_BYTES;
    return 0;
}

static int __MTPTPT_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_mtptpt_reg *mtptpt_reg = (struct ku_mtptpt_reg*)ku_reg;
    u8                   *outbox = tlv_markers->reg_tlv->reg_data;

    SX_GET_REG_FIELD(mtptpt_reg->trap_id, outbox, REG_MTPTPT_TRAP_ID_OFFSET);
    SX_GET_REG_FIELD(mtptpt_reg->message_type, outbox, REG_MTPTPT_MSG_TYPE_OFFSET);
    return 0;
}

int sx_ACCESS_REG_MTPTPT(struct sx_dev *dev, struct ku_access_mtptpt_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __MTPTPT_encode,
                                  __MTPTPT_decode,
                                  &reg_data->mtptpt_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_MTPTPT);


/************************************************
 * SBCTC
 ***********************************************/
#define REG_SBCTC_DIR_ING_OFFSET        0x00
#define REG_SBCTC_LOCAL_PORT_OFFSET     0x01
#define REG_SBCTC_LOCAL_PORT_MSB_OFFSET 0x02
#define REG_SBCTC_MODE_OFFSET           0x03
#define REG_SBCTC_EN_CONFIG_OFFSET      0x04
#define REG_SBCTC_EVENT_OFFSET          0x07
#define REG_SBCTC_TCLASS_EN_OFFSET      0x08
#define REG_SBCTC_THR_MAX_OFFSET        0x10
#define REG_SBCTC_THR_MIN_OFFSET        0x14
#define SBCTC_REG_LEN_IN_BYTES          (0x20)

static int __SBCTC_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_sbctc_reg  *sbctc_reg = (struct ku_sbctc_reg*)ku_reg;
    sxd_emad_index_tlv_t *index_tlv;
    u8                   *reg_tlv;
    u8                    en_config;
    u8                    tmp_val_u8 = 0;

    if (flags & SX_EMAD_F_INDEXATION_TLV) {
        index_tlv = (sxd_emad_index_tlv_t*)inbox;
        reg_tlv = inbox + sizeof(sxd_emad_index_tlv_t);
        sxd_emad_index_tlv_port_single_build(index_tlv, sbctc_reg->lp_msb, sbctc_reg->local_port);
        *bytes_written += sizeof(sxd_emad_index_tlv_t);
    } else {
        reg_tlv = inbox;
        SX_PUT_REG_FIELD(reg_tlv, sbctc_reg->local_port, REG_SBCTC_LOCAL_PORT_OFFSET);
        tmp_val_u8 |= ((sbctc_reg->lp_msb & 0x3) << 4);
        SX_PUT_REG_FIELD(reg_tlv, tmp_val_u8, REG_SBCTC_LOCAL_PORT_MSB_OFFSET);
    }

    SX_PUT_REG_FIELD(reg_tlv, sbctc_reg->dir_ing, REG_SBCTC_DIR_ING_OFFSET);
    tmp_val_u8 = 0;
    tmp_val_u8 |= (sbctc_reg->res & 1) << 4;
    tmp_val_u8 |= (sbctc_reg->mode & 1);
    SX_PUT_REG_FIELD(reg_tlv, tmp_val_u8, REG_SBCTC_MODE_OFFSET);
    en_config = sbctc_reg->en_config << 7;
    SX_PUT_REG_FIELD(reg_tlv, en_config, REG_SBCTC_EN_CONFIG_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, sbctc_reg->event, REG_SBCTC_EVENT_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, sbctc_reg->tclass_en, REG_SBCTC_TCLASS_EN_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, sbctc_reg->thr_max, REG_SBCTC_THR_MAX_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, sbctc_reg->thr_min, REG_SBCTC_THR_MIN_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + SBCTC_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + SBCTC_REG_LEN_IN_BYTES;
    return 0;
}

static int __SBCTC_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_sbctc_reg *sbctc_reg = (struct ku_sbctc_reg*)ku_reg;
    u8                  *outbox = tlv_markers->reg_tlv->reg_data;
    u8                   tmp_val_u8 = 0;

    SX_GET_REG_FIELD(tmp_val_u8, outbox, REG_SBCTC_MODE_OFFSET);
    sbctc_reg->res = (tmp_val_u8 >> 4) & 1;
    sbctc_reg->mode = tmp_val_u8 & 1;
    SX_GET_REG_FIELD(sbctc_reg->event, outbox, REG_SBCTC_EVENT_OFFSET);
    SX_GET_REG_FIELD(sbctc_reg->tclass_en, outbox, REG_SBCTC_TCLASS_EN_OFFSET);
    SX_GET_REG_FIELD(sbctc_reg->thr_max, outbox, REG_SBCTC_THR_MAX_OFFSET);
    SX_GET_REG_FIELD(sbctc_reg->thr_min, outbox, REG_SBCTC_THR_MIN_OFFSET);
    return 0;
}

int sx_ACCESS_REG_SBCTC(struct sx_dev *dev, struct ku_access_sbctc_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __SBCTC_encode,
                                  __SBCTC_decode,
                                  &reg_data->sbctc_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_SBCTC);


/************************************************
 * PPBMC
 ***********************************************/
#define REG_PPBMC_CLR_MONITOR_E_CTRL_IND_OFFSET 0x00
#define REG_PPBMC_LOCAL_PORT_OFFSET             0x01
#define REG_PPBMC_PNAT_MSB_MONITOR_TYPE_OFFSET  0x02
#define REG_PPBMC_E_EVENT_CTRL_OFFSET           0x04
#define REG_PPBMC_MONITOR_CTRL_OFFSET           0x05
#define REG_PPBMC_MONITOR_E_CTRL_IND_CAP_OFFSET 0x06
#define REG_PPBMC_MONITOR_STATE_OFFSET          0x07
#define PPBMC_REG_LEN_IN_BYTES                  (0x10)
int __PPBMC_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_ppbmc_reg  *ppbmc_reg = (struct ku_ppbmc_reg*)ku_reg;
    sxd_emad_index_tlv_t *index_tlv;
    u8                   *reg_tlv;
    u8                    e_event_ctrl;
    u8                    pnat_msb_monitor_type = 0;
    u8                    clr_monitor_e_ctrl_ind;
    u8                    monitor_type_ext_e_ctrl_ind_cap;

    if (flags & SX_EMAD_F_INDEXATION_TLV) {
        index_tlv = (sxd_emad_index_tlv_t*)inbox;
        reg_tlv = inbox + sizeof(sxd_emad_index_tlv_t);
        sxd_emad_index_tlv_port_single_build(index_tlv, ppbmc_reg->lp_msb, ppbmc_reg->local_port);
        *bytes_written += sizeof(sxd_emad_index_tlv_t);
    } else {
        reg_tlv = inbox;
        SX_PUT_REG_FIELD(reg_tlv, ppbmc_reg->local_port, REG_PPBMC_LOCAL_PORT_OFFSET);
        pnat_msb_monitor_type = ((ppbmc_reg->lp_msb & 0x3) << 4);
    }

    clr_monitor_e_ctrl_ind = ppbmc_reg->clr << 0x7;
    clr_monitor_e_ctrl_ind |= ppbmc_reg->monitor_e_ctrl_ind & 0xF;
    SX_PUT_REG_FIELD(reg_tlv, clr_monitor_e_ctrl_ind, REG_PPBMC_CLR_MONITOR_E_CTRL_IND_OFFSET);
    pnat_msb_monitor_type |= (ppbmc_reg->monitor_type & 0xF);
    pnat_msb_monitor_type |= ((ppbmc_reg->pnat & 0x3) << 6);
    SX_PUT_REG_FIELD(reg_tlv, pnat_msb_monitor_type, REG_PPBMC_PNAT_MSB_MONITOR_TYPE_OFFSET);
    e_event_ctrl = ppbmc_reg->e << 6 | ((ppbmc_reg->event_ctrl) & 0xF);
    SX_PUT_REG_FIELD(reg_tlv, e_event_ctrl, REG_PPBMC_E_EVENT_CTRL_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, ppbmc_reg->monitor_ctrl, REG_PPBMC_MONITOR_CTRL_OFFSET);
    monitor_type_ext_e_ctrl_ind_cap = ppbmc_reg->monitor_e_ctrl_ind_cap << 0x2;
    monitor_type_ext_e_ctrl_ind_cap |= ((ppbmc_reg->monitor_type_ext) & 0xF) << 4;
    SX_PUT_REG_FIELD(reg_tlv, monitor_type_ext_e_ctrl_ind_cap, REG_PPBMC_MONITOR_E_CTRL_IND_CAP_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + PPBMC_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + PPBMC_REG_LEN_IN_BYTES;
    return 0;
}

int __PPBMC_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_ppbmc_reg *ppbmc_reg = (struct ku_ppbmc_reg*)ku_reg;
    u8                  *outbox = tlv_markers->reg_tlv->reg_data;
    u8                   e_event_ctrl;
    u8                   pnat_msb_monitor_type;
    u8                   clr_monitor_e_ctrl_ind;
    u8                   monitor_type_ext_e_ctrl_ind_cap;

    SX_GET_REG_FIELD(clr_monitor_e_ctrl_ind, outbox, REG_PPBMC_CLR_MONITOR_E_CTRL_IND_OFFSET);
    ppbmc_reg->clr = (clr_monitor_e_ctrl_ind >> 0x7) & 0x1;
    ppbmc_reg->monitor_e_ctrl_ind = clr_monitor_e_ctrl_ind & 0xF;
    SX_GET_REG_FIELD(pnat_msb_monitor_type, outbox, REG_PPBMC_PNAT_MSB_MONITOR_TYPE_OFFSET);
    ppbmc_reg->monitor_type = pnat_msb_monitor_type & 0xF;
    SX_GET_REG_FIELD(e_event_ctrl, outbox, REG_PPBMC_E_EVENT_CTRL_OFFSET);
    ppbmc_reg->e = (e_event_ctrl >> 6) & 0x3;
    ppbmc_reg->event_ctrl = e_event_ctrl & 0xF;
    SX_GET_REG_FIELD(ppbmc_reg->monitor_ctrl, outbox, REG_PPBMC_MONITOR_CTRL_OFFSET);
    SX_GET_REG_FIELD(monitor_type_ext_e_ctrl_ind_cap, outbox, REG_PPBMC_MONITOR_E_CTRL_IND_CAP_OFFSET);
    ppbmc_reg->monitor_e_ctrl_ind_cap = (monitor_type_ext_e_ctrl_ind_cap >> 0x2) & 0x1;
    ppbmc_reg->monitor_type_ext = (monitor_type_ext_e_ctrl_ind_cap >> 0x4) & 0xF;
    SX_GET_REG_FIELD(ppbmc_reg->monitor_state, outbox, REG_PPBMC_MONITOR_STATE_OFFSET);
    return 0;
}

int sx_ACCESS_REG_PPBMC(struct sx_dev *dev, struct ku_access_ppbmc_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __PPBMC_encode,
                                  __PPBMC_decode,
                                  &reg_data->ppbmc_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_PPBMC);

/****************************************************
 * PPBMC (wrapper)
 *
 * on OOB system, it will call the register by EMAD
 * Otherwise, it will call it with Command Interface
 ***************************************************/
int sx_ACCESS_REG_PPBMC_wrapper(struct sx_dev *dev, u8 method, struct ku_access_ppbmc_reg *reg_data)
{
    if (is_sgmii_supported()) {
        /* PPBMC is common for all switch types. Some switches support command-interface over PCI and
         * Mantaray, that is using OOB, is using SGMII. Since the common call (sx_ACCESS_REG_PPBMC) is
         * auto generated, we must use a wrapper to decide which path to take.
         */
        return sgmii_emad_access_ppbmc(reg_data->dev_id, method, PPBMC_REG_LEN_IN_BYTES, &reg_data->ppbmc_reg);
    }

    return sx_ACCESS_REG_PPBMC(dev, reg_data);
}
EXPORT_SYMBOL(sx_ACCESS_REG_PPBMC_wrapper);

/************************************************
 * QPEM
 ***********************************************/
#define QPEM_LOCAL_PORT_OFFSET     0x01
#define QPEM_LOCAL_PORT_MSB_OFFSET 0x02
#define QPEM_RECORD0_OFFSET        0x04
#define QPEM_REG_LEN_IN_BYTES      (0x104)

static int __QPEM_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_qpem_reg   *qpem_reg = (struct ku_qpem_reg*)ku_reg;
    sxd_emad_index_tlv_t *index_tlv;
    u8                   *reg_tlv;
    u8                   *curr_switch_prio, *curr_ecn, *curr_color;
    u8                    tmp_u8 = 0;
    u32                   i, j, k;

    if (flags & SX_EMAD_F_INDEXATION_TLV) {
        index_tlv = (sxd_emad_index_tlv_t*)inbox;
        reg_tlv = inbox + sizeof(sxd_emad_index_tlv_t);
        sxd_emad_index_tlv_port_single_build(index_tlv, qpem_reg->lp_msb, qpem_reg->local_port);
        *bytes_written += sizeof(sxd_emad_index_tlv_t);
    } else {
        reg_tlv = inbox;
        SX_PUT_REG_FIELD(reg_tlv, qpem_reg->local_port, QPEM_LOCAL_PORT_OFFSET);
        tmp_u8 = (qpem_reg->lp_msb & 0x3) << 4;
        SX_PUT_REG_FIELD(reg_tlv, tmp_u8, QPEM_LOCAL_PORT_MSB_OFFSET);
    }

    for (i = 0; i < 16; i++) {
        curr_switch_prio = inbox + 4 + i * 16; /* 16 is the size of switch_prio record */
        for (j = 0; j < 4; j++) {
            curr_ecn = curr_switch_prio + j * 4; /* 4 is the size of ecn record */
            for (k = 0; k < 3; k++) {
                curr_color = curr_ecn + k; /* size of color record is 1 byte */
                *curr_color = ((qpem_reg->switch_prio[i].ecn[j].color[k].enable_exp & 0x1) << 7) |
                              (qpem_reg->switch_prio[i].ecn[j].color[k].exp & 0x7);
            }
        }
    }

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + QPEM_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + QPEM_REG_LEN_IN_BYTES;
    return 0;
}

static int __QPEM_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_qpem_reg *qpem_reg = (struct ku_qpem_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u8                 *curr_switch_prio, *curr_ecn, *curr_color;
    u8                  tmp_u8 = 0;
    u32                 i, j, k;

    if (tlv_markers->index_tlv) {
        sxd_emad_index_tlv_port_single_deparse(tlv_markers->index_tlv,
                                               &qpem_reg->lp_msb,
                                               &qpem_reg->local_port);
    } else {
        SX_GET_REG_FIELD(qpem_reg->local_port, outbox, QPEM_LOCAL_PORT_OFFSET);
        SX_GET_REG_FIELD(tmp_u8, outbox, QPEM_LOCAL_PORT_MSB_OFFSET);
        qpem_reg->lp_msb = (tmp_u8 >> 4) & 0x3;
    }

    for (i = 0; i < 16; i++) {
        curr_switch_prio = outbox + 4 + i * 16; /* 16 is the size of switch_prio record */
        for (j = 0; j < 4; j++) {
            curr_ecn = curr_switch_prio + j * 4; /* 4 is the size of ecn record */
            for (k = 0; k < 3; k++) {
                curr_color = curr_ecn + k; /* size of color record is 1 byte */
                qpem_reg->switch_prio[i].ecn[j].color[k].enable_exp = (*curr_color >> 7) & 0x1;
                qpem_reg->switch_prio[i].ecn[j].color[k].exp = (*curr_color) & 0x7;
            }
        }
    }

    return 0;
}

int sx_ACCESS_REG_QPEM(struct sx_dev *dev, struct ku_access_qpem_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __QPEM_encode,
                                  __QPEM_decode,
                                  &reg_data->qpem_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_QPEM);


/************************************************
 * SLRP
 ***********************************************/

/* slrp
 * ----
 *
 */
/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, slrp, status, 0x0, 4, 4);

/* version
 * Access: RO
 */
MLXSW_ITEM8(reg, slrp, version, 0x0, 0, 4);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slrp, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slrp, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slrp, lp_msb, 0x2, 4, 2);

/* lane
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slrp, lane, 0x2, 0, 4);

/* port_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slrp, port_type, 0x3, 4, 4);

/* page_data_slrp_40nm_28nm_ib_sel
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_ib_sel, 0x4, 6, 2);

/* page_data_slrp_40nm_28nm_dp_sel
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_dp_sel, 0x6, 4, 1);

/* page_data_slrp_40nm_28nm_dp90sel
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_dp90sel, 0x6, 0, 4);

/* page_data_slrp_40nm_28nm_mix90phase
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_mix90phase, 0x7, 0, 8);

/* page_data_slrp_40nm_28nm_ffe_tap0
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_ffe_tap0, 0x8, 0, 8);

/* page_data_slrp_40nm_28nm_ffe_tap1
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_ffe_tap1, 0x9, 0, 8);

/* page_data_slrp_40nm_28nm_ffe_tap2
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_ffe_tap2, 0xa, 0, 8);

/* page_data_slrp_40nm_28nm_ffe_tap3
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_ffe_tap3, 0xb, 0, 8);

/* page_data_slrp_40nm_28nm_ffe_tap4
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_ffe_tap4, 0xc, 0, 8);

/* page_data_slrp_40nm_28nm_ffe_tap5
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_ffe_tap5, 0xd, 0, 8);

/* page_data_slrp_40nm_28nm_ffe_tap6
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_ffe_tap6, 0xe, 0, 8);

/* page_data_slrp_40nm_28nm_ffe_tap7
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_ffe_tap7, 0xf, 0, 8);

/* page_data_slrp_40nm_28nm_ffe_tap8
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_ffe_tap8, 0x10, 0, 8);

/* page_data_slrp_40nm_28nm_mixerbias_tap_amp
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_mixerbias_tap_amp, 0x11, 0, 8);

/* page_data_slrp_40nm_28nm_ffe_tap_en
 * Access: RW
 */
MLXSW_ITEM16(reg, slrp, page_data_slrp_40nm_28nm_ffe_tap_en, 0x12, 0, 9);

/* page_data_slrp_40nm_28nm_ffe_tap_offset0
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_ffe_tap_offset0, 0x14, 0, 8);

/* page_data_slrp_40nm_28nm_ffe_tap_offset1
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_ffe_tap_offset1, 0x15, 0, 8);

/* page_data_slrp_40nm_28nm_slicer_offset0
 * Access: RW
 */
MLXSW_ITEM16(reg, slrp, page_data_slrp_40nm_28nm_slicer_offset0, 0x16, 0, 16);

/* page_data_slrp_40nm_28nm_mixer_offset0
 * Access: RW
 */
MLXSW_ITEM16(reg, slrp, page_data_slrp_40nm_28nm_mixer_offset0, 0x18, 0, 16);

/* page_data_slrp_40nm_28nm_mixer_offset1
 * Access: RW
 */
MLXSW_ITEM16(reg, slrp, page_data_slrp_40nm_28nm_mixer_offset1, 0x1a, 0, 16);

/* page_data_slrp_40nm_28nm_mixerbgn_inp
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_mixerbgn_inp, 0x1c, 0, 8);

/* page_data_slrp_40nm_28nm_mixerbgn_inn
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_mixerbgn_inn, 0x1d, 0, 8);

/* page_data_slrp_40nm_28nm_mixerbgn_refp
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_mixerbgn_refp, 0x1e, 0, 8);

/* page_data_slrp_40nm_28nm_mixerbgn_refn
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_mixerbgn_refn, 0x1f, 0, 8);

/* page_data_slrp_40nm_28nm_sel_slicer_lctrl_h
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_sel_slicer_lctrl_h, 0x20, 7, 1);

/* page_data_slrp_40nm_28nm_sel_slicer_lctrl_l
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_sel_slicer_lctrl_l, 0x20, 6, 1);

/* page_data_slrp_40nm_28nm_ref_mixer_vreg
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_ref_mixer_vreg, 0x20, 0, 5);

/* page_data_slrp_40nm_28nm_slicer_gctrl
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_slicer_gctrl, 0x21, 0, 8);

/* page_data_slrp_40nm_28nm_lctrl_input
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_lctrl_input, 0x22, 0, 8);

/* page_data_slrp_40nm_28nm_mixer_offset_cm1
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_mixer_offset_cm1, 0x23, 0, 8);

/* page_data_slrp_40nm_28nm_common_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_40nm_28nm_common_mode, 0x24, 2, 6);

/* page_data_slrp_40nm_28nm_mixer_offset_cm0
 * Access: RW
 */
MLXSW_ITEM16(reg, slrp, page_data_slrp_40nm_28nm_mixer_offset_cm0, 0x24, 0, 9);

/* page_data_slrp_40nm_28nm_slicer_offset_cm
 * Access: RW
 */
MLXSW_ITEM16(reg, slrp, page_data_slrp_40nm_28nm_slicer_offset_cm, 0x26, 0, 9);

/* page_data_slrp_16nm_mixer_offset_up
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_mixer_offset_up, 0x4, 0, 8);

/* page_data_slrp_16nm_mixer_offset_down
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_mixer_offset_down, 0x5, 0, 8);

/* page_data_slrp_16nm_sel_enc
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_enc, 0x6, 0, 8);

/* page_data_slrp_16nm_dp_sel
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_dp_sel, 0x7, 0, 2);

/* page_data_slrp_16nm_sel_ref_in_mixer_biasgen_up
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_ref_in_mixer_biasgen_up, 0x8, 4, 4);

/* page_data_slrp_16nm_sel_ref_in_mixer_biasgen_mid
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_ref_in_mixer_biasgen_mid, 0x8, 0, 4);

/* page_data_slrp_16nm_sel_ref_in_mixer_biasgen_down
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_ref_in_mixer_biasgen_down, 0x9, 4, 4);

/* page_data_slrp_16nm_sel_ref_in_mixer_biasgen_eo
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_ref_in_mixer_biasgen_eo, 0x9, 0, 4);

/* page_data_slrp_16nm_sel_ref_out_mixer_biasgen_up
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_ref_out_mixer_biasgen_up, 0xa, 4, 4);

/* page_data_slrp_16nm_sel_ref_out_mixer_biasgen_mid
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_ref_out_mixer_biasgen_mid, 0xa, 0, 4);

/* page_data_slrp_16nm_sel_ref_out_mixer_biasgen_down
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_ref_out_mixer_biasgen_down, 0xb, 4, 4);

/* page_data_slrp_16nm_sel_ref_out_mixer_biasgen_eo
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_ref_out_mixer_biasgen_eo, 0xb, 0, 4);

/* page_data_slrp_16nm_sel_amp_mixer_biasgen_up
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_amp_mixer_biasgen_up, 0xc, 6, 2);

/* page_data_slrp_16nm_sel_amp_mixer_biasgen_mid
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_amp_mixer_biasgen_mid, 0xc, 4, 2);

/* page_data_slrp_16nm_sel_amp_mixer_biasgen_down
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_amp_mixer_biasgen_down, 0xc, 2, 2);

/* page_data_slrp_16nm_sel_amp_mixer_biasgen_eo
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_amp_mixer_biasgen_eo, 0xc, 0, 2);

/* page_data_slrp_16nm_mixer_offset_eye_scan
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_mixer_offset_eye_scan, 0xd, 0, 8);

/* page_data_slrp_16nm_gctrl_bin_bgn0_n
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_gctrl_bin_bgn0_n, 0xe, 4, 4);

/* page_data_slrp_16nm_gctrl_bin_bgn0_p
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_gctrl_bin_bgn0_p, 0xe, 0, 4);

/* page_data_slrp_16nm_gctrl_bin_bgn1_n
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_gctrl_bin_bgn1_n, 0xf, 4, 4);

/* page_data_slrp_16nm_gctrl_bin_bgn1_p
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_gctrl_bin_bgn1_p, 0xf, 0, 4);

/* page_data_slrp_16nm_sel_digital_gctrl
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_digital_gctrl, 0x10, 7, 1);

/* page_data_slrp_16nm_sel_bias2ctle
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_bias2ctle, 0x10, 0, 7);

/* page_data_slrp_16nm_selgc_ref_op0_alev_ctle
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_selgc_ref_op0_alev_ctle, 0x11, 3, 5);

/* page_data_slrp_16nm_selgc_ref_op1_alev_ctle
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_selgc_ref_op1_alev_ctle, 0x12, 6, 5);

/* page_data_slrp_16nm_sel_bias_en_0
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_bias_en_0, 0x12, 2, 4);

/* page_data_slrp_16nm_sel_bias_en_1
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_bias_en_1, 0x13, 6, 4);

/* page_data_slrp_16nm_sel_bias_en_2
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_bias_en_2, 0x13, 2, 4);

/* page_data_slrp_16nm_sel_bias_en_3
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_bias_en_3, 0x14, 4, 4);

/* page_data_slrp_16nm_sel_bias_en_4
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_bias_en_4, 0x14, 0, 4);

/* page_data_slrp_16nm_sel_bias_en_5
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_bias_en_5, 0x15, 4, 4);

/* page_data_slrp_16nm_sel_bias_en_6
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_bias_en_6, 0x16, 7, 5);

/* page_data_slrp_16nm_sel_gctrln_en_0
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_gctrln_en_0, 0x16, 3, 4);

/* page_data_slrp_16nm_sel_gctrln_en_1
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_gctrln_en_1, 0x17, 7, 4);

/* page_data_slrp_16nm_sel_gctrln_en_2
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_gctrln_en_2, 0x17, 3, 4);

/* page_data_slrp_16nm_sel_gctrln_en_3
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_gctrln_en_3, 0x18, 4, 4);

/* page_data_slrp_16nm_sel_gctrln_en_4
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_gctrln_en_4, 0x18, 0, 4);

/* page_data_slrp_16nm_sel_gctrln_en_5
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_gctrln_en_5, 0x19, 4, 4);

/* page_data_slrp_16nm_sel_gctrln_en_6
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_gctrln_en_6, 0x19, 0, 4);

/* page_data_slrp_16nm_sel_gctrlp_en_0
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_gctrlp_en_0, 0x1a, 4, 4);

/* page_data_slrp_16nm_sel_gctrlp_en_1
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_gctrlp_en_1, 0x1a, 0, 4);

/* page_data_slrp_16nm_sel_gctrlp_en_2
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_gctrlp_en_2, 0x1b, 4, 4);

/* page_data_slrp_16nm_sel_gctrlp_en_3
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_gctrlp_en_3, 0x1b, 0, 4);

/* page_data_slrp_16nm_sel_gctrlp_en_4
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_gctrlp_en_4, 0x1c, 4, 4);

/* page_data_slrp_16nm_sel_gctrlp_en_5
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_gctrlp_en_5, 0x1c, 0, 4);

/* page_data_slrp_16nm_sel_gctrlp_en_6
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_sel_gctrlp_en_6, 0x1d, 4, 4);

/* page_data_slrp_16nm_crnt_bgn_offset_p1
 * Access: RO
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_crnt_bgn_offset_p1, 0x1e, 7, 5);

/* page_data_slrp_16nm_crnt_bgn_offset_n1
 * Access: RO
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_crnt_bgn_offset_n1, 0x1e, 2, 5);

/* page_data_slrp_16nm_crnt_bgn_offset_p0
 * Access: RO
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_crnt_bgn_offset_p0, 0x1f, 5, 5);

/* page_data_slrp_16nm_crnt_bgn_offset_n0
 * Access: RO
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_16nm_crnt_bgn_offset_n0, 0x1f, 0, 5);

/* page_data_slrp_gearbox_ffe_tap_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_gearbox_ffe_tap_index, 0x6, 0, 4);

/* page_data_slrp_gearbox_ffe_sub_channel
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_gearbox_ffe_sub_channel, 0x7, 0, 5);

/* page_data_slrp_gearbox_snr_en
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_gearbox_snr_en, 0x8, 7, 1);

/* page_data_slrp_gearbox_dsp_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_gearbox_dsp_mode, 0x9, 7, 4);

/* page_data_slrp_gearbox_afe_trim
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_gearbox_afe_trim, 0x9, 0, 4);

/* page_data_slrp_gearbox_ctle
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_gearbox_ctle, 0xb, 0, 8);

/* page_data_slrp_gearbox_raw_snr_value
 * Access: RO
 */
MLXSW_ITEM16(reg, slrp, page_data_slrp_gearbox_raw_snr_value, 0xc, 0, 16);

/* page_data_slrp_gearbox_dsp_err_gen
 * Access: WO
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_gearbox_dsp_err_gen, 0xe, 0, 2);

/* page_data_slrp_gearbox_snr_sym_cnt
 * Access: WO
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_gearbox_snr_sym_cnt, 0xf, 0, 3);

/* page_data_slrp_gearbox_db_snr_value
 * Access: RO
 */
MLXSW_ITEM32(reg, slrp, page_data_slrp_gearbox_db_snr_value, 0x10, 0, 32);

/* page_data_slrp_gearbox_ffe_tap_value
 * Access: RO
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_gearbox_ffe_tap_value, 0x17, 0, 8);

/* page_data_slrp_usr_mixer00
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_mixer00, 0x4, 1, 7);

/* page_data_slrp_usr_mixer90
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_mixer90, 0x5, 2, 7);

/* page_data_slrp_usr_termination
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_termination, 0x6, 6, 4);

/* page_data_slrp_usr_neg_cap_up
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_neg_cap_up, 0x8, 6, 2);

/* page_data_slrp_usr_neg_cap_mid
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_neg_cap_mid, 0x8, 4, 2);

/* page_data_slrp_usr_neg_cap_dn
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_neg_cap_dn, 0x8, 2, 2);

/* page_data_slrp_usr_offset_coarse_polarity_up
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_offset_coarse_polarity_up, 0x8, 1, 1);

/* page_data_slrp_usr_offset_coarse_polarity_mid
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_offset_coarse_polarity_mid, 0x8, 0, 1);

/* page_data_slrp_usr_offset_coarse_polarity_dn
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_offset_coarse_polarity_dn, 0x9, 7, 1);

/* page_data_slrp_usr_offset_fine_polarity_up
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_offset_fine_polarity_up, 0x9, 6, 1);

/* page_data_slrp_usr_offset_fine_polarity_mid
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_offset_fine_polarity_mid, 0x9, 5, 1);

/* page_data_slrp_usr_offset_fine_polarity_dn
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_offset_fine_polarity_dn, 0x9, 4, 1);

/* page_data_slrp_usr_amp_vga
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_amp_vga, 0xa, 0, 5);

/* page_data_slrp_usr_amp_vga_last_stage
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_amp_vga_last_stage, 0xb, 0, 5);

/* page_data_slrp_usr_offset_coarse_up
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_offset_coarse_up, 0xc, 2, 6);

/* page_data_slrp_usr_offset_coarse_mid
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_offset_coarse_mid, 0xd, 4, 6);

/* page_data_slrp_usr_offset_coarse_dn
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_offset_coarse_dn, 0xe, 6, 6);

/* page_data_slrp_usr_offset_fine_up
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_offset_fine_up, 0xe, 2, 4);

/* page_data_slrp_usr_offset_fine_mid
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_offset_fine_mid, 0xf, 6, 4);

/* page_data_slrp_usr_offset_fine_dn
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_usr_offset_fine_dn, 0xf, 2, 4);

/* page_data_slrp_7nm_adc_recording_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_adc_recording_admin, 0x4, 4, 2);

/* page_data_slrp_7nm_adc_recording_status
 * Access: RO
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_adc_recording_status, 0x4, 0, 2);

/* page_data_slrp_7nm_adc_rocording_lanes
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_adc_rocording_lanes, 0x5, 0, 2);

/* page_data_slrp_7nm_edge_vos_ccal_en
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_edge_vos_ccal_en, 0x6, 1, 1);

/* page_data_slrp_7nm_adc_gain_shift_auto
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_adc_gain_shift_auto, 0x6, 0, 1);

/* page_data_slrp_7nm_ccal_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_ccal_mode, 0x7, 4, 3);

/* page_data_slrp_7nm_ctle_override_ctrl
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_ctle_override_ctrl, 0x8, 6, 2);

/* page_data_slrp_7nm_vga_override_ctrl
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_vga_override_ctrl, 0x8, 4, 2);

/* page_data_slrp_7nm_adc_vos_override_ctrl
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_adc_vos_override_ctrl, 0x8, 2, 2);

/* page_data_slrp_7nm_adc_gain_override_ctrl
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_adc_gain_override_ctrl, 0x8, 0, 2);

/* page_data_slrp_7nm_phos_override_ctrl
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_phos_override_ctrl, 0x9, 6, 2);

/* page_data_slrp_7nm_cal_error_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_cal_error_cnt, 0xa, 0, 8);

/* page_data_slrp_7nm_ccal_state
 * Access: RO
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_ccal_state, 0xb, 4, 2);

/* page_data_slrp_7nm_ccal_op
 * Access: RO
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_ccal_op, 0xb, 0, 2);

/* page_data_slrp_7nm_phos
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_phos, 0xc, 1, 5);

/* page_data_slrp_7nm_vga_vos1
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_vga_vos1, 0xd, 0, 6);

/* page_data_slrp_7nm_vga_vos0
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_vga_vos0, 0xe, 0, 6);

/* page_data_slrp_7nm_ctle_vos0
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_ctle_vos0, 0xf, 0, 7);

/* page_data_slrp_7nm_adc_vos
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, slrp, page_data_slrp_7nm_adc_vos, 0x10, 64);

/* page_data_slrp_7nm_adc_gos
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, slrp, page_data_slrp_7nm_adc_gos, 0x50, 64);

/* page_data_slrp_7nm_phos3
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_phos3, 0x90, 0, 5);

/* page_data_slrp_7nm_phos2
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_phos2, 0x91, 0, 5);

/* page_data_slrp_7nm_phos1
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_phos1, 0x92, 0, 5);

/* page_data_slrp_7nm_phos0
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_phos0, 0x93, 0, 5);

/* page_data_slrp_7nm_phos7
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_phos7, 0x94, 0, 5);

/* page_data_slrp_7nm_phos6
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_phos6, 0x95, 0, 5);

/* page_data_slrp_7nm_phos5
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_phos5, 0x96, 0, 5);

/* page_data_slrp_7nm_phos4
 * Access: RW
 */
MLXSW_ITEM8(reg, slrp, page_data_slrp_7nm_phos4, 0x97, 0, 5);

/* slrp
 * ----
 *
 */

#define MLXSW_SLRP_ID  0x5026
#define MLXSW_SLRP_LEN 0x98

uint8_t mlxsw_reg_slrp_status_get(const char *buf);

uint8_t mlxsw_reg_slrp_version_get(const char *buf);

uint8_t mlxsw_reg_slrp_local_port_get(const char *buf);

void mlxsw_reg_slrp_local_port_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_pnat_get(const char *buf);

void mlxsw_reg_slrp_pnat_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_lp_msb_get(const char *buf);

void mlxsw_reg_slrp_lp_msb_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_lane_get(const char *buf);

void mlxsw_reg_slrp_lane_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_port_type_get(const char *buf);

void mlxsw_reg_slrp_port_type_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ib_sel_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ib_sel_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_dp_sel_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_dp_sel_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_dp90sel_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_dp90sel_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mix90phase_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mix90phase_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap0_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap0_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap1_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap1_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap2_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap2_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap3_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap3_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap4_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap4_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap5_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap5_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap6_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap6_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap7_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap7_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap8_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap8_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbias_tap_amp_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbias_tap_amp_set(char *buf, uint8_t val);

uint16_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap_en_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap_en_set(char *buf, uint16_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap_offset0_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap_offset0_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap_offset1_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap_offset1_set(char *buf, uint8_t val);

uint16_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_slicer_offset0_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_slicer_offset0_set(char *buf, uint16_t val);

uint16_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixer_offset0_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixer_offset0_set(char *buf, uint16_t val);

uint16_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixer_offset1_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixer_offset1_set(char *buf, uint16_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbgn_inp_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbgn_inp_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbgn_inn_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbgn_inn_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbgn_refp_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbgn_refp_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbgn_refn_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbgn_refn_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_sel_slicer_lctrl_h_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_sel_slicer_lctrl_h_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_sel_slicer_lctrl_l_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_sel_slicer_lctrl_l_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ref_mixer_vreg_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ref_mixer_vreg_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_slicer_gctrl_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_slicer_gctrl_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_lctrl_input_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_lctrl_input_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixer_offset_cm1_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixer_offset_cm1_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_common_mode_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_common_mode_set(char *buf, uint8_t val);

uint16_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixer_offset_cm0_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixer_offset_cm0_set(char *buf, uint16_t val);

uint16_t mlxsw_reg_slrp_page_data_slrp_40nm_28nm_slicer_offset_cm_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_40nm_28nm_slicer_offset_cm_set(char *buf, uint16_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_mixer_offset_up_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_mixer_offset_up_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_mixer_offset_down_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_mixer_offset_down_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_enc_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_enc_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_dp_sel_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_dp_sel_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_in_mixer_biasgen_up_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_in_mixer_biasgen_up_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_in_mixer_biasgen_mid_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_in_mixer_biasgen_mid_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_in_mixer_biasgen_down_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_in_mixer_biasgen_down_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_in_mixer_biasgen_eo_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_in_mixer_biasgen_eo_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_out_mixer_biasgen_up_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_out_mixer_biasgen_up_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_out_mixer_biasgen_mid_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_out_mixer_biasgen_mid_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_out_mixer_biasgen_down_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_out_mixer_biasgen_down_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_out_mixer_biasgen_eo_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_out_mixer_biasgen_eo_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_amp_mixer_biasgen_up_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_amp_mixer_biasgen_up_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_amp_mixer_biasgen_mid_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_amp_mixer_biasgen_mid_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_amp_mixer_biasgen_down_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_amp_mixer_biasgen_down_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_amp_mixer_biasgen_eo_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_amp_mixer_biasgen_eo_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_mixer_offset_eye_scan_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_mixer_offset_eye_scan_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_gctrl_bin_bgn0_n_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_gctrl_bin_bgn0_n_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_gctrl_bin_bgn0_p_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_gctrl_bin_bgn0_p_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_gctrl_bin_bgn1_n_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_gctrl_bin_bgn1_n_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_gctrl_bin_bgn1_p_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_gctrl_bin_bgn1_p_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_digital_gctrl_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_digital_gctrl_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias2ctle_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias2ctle_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_selgc_ref_op0_alev_ctle_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_selgc_ref_op0_alev_ctle_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_selgc_ref_op1_alev_ctle_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_selgc_ref_op1_alev_ctle_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_0_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_0_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_1_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_1_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_2_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_2_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_3_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_3_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_4_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_4_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_5_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_5_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_6_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_6_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_0_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_0_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_1_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_1_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_2_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_2_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_3_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_3_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_4_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_4_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_5_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_5_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_6_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_6_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_0_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_0_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_1_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_1_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_2_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_2_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_3_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_3_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_4_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_4_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_5_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_5_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_6_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_6_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_crnt_bgn_offset_p1_get(const char *buf);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_crnt_bgn_offset_n1_get(const char *buf);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_crnt_bgn_offset_p0_get(const char *buf);

uint8_t mlxsw_reg_slrp_page_data_slrp_16nm_crnt_bgn_offset_n0_get(const char *buf);

uint8_t mlxsw_reg_slrp_page_data_slrp_gearbox_ffe_tap_index_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_gearbox_ffe_tap_index_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_gearbox_ffe_sub_channel_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_gearbox_ffe_sub_channel_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_gearbox_snr_en_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_gearbox_snr_en_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_gearbox_dsp_mode_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_gearbox_dsp_mode_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_gearbox_afe_trim_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_gearbox_afe_trim_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_gearbox_ctle_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_gearbox_ctle_set(char *buf, uint8_t val);

uint16_t mlxsw_reg_slrp_page_data_slrp_gearbox_raw_snr_value_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_gearbox_dsp_err_gen_set(char *buf, uint8_t val);

void mlxsw_reg_slrp_page_data_slrp_gearbox_snr_sym_cnt_set(char *buf, uint8_t val);

uint32_t mlxsw_reg_slrp_page_data_slrp_gearbox_db_snr_value_get(const char *buf);

uint8_t mlxsw_reg_slrp_page_data_slrp_gearbox_ffe_tap_value_get(const char *buf);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_mixer00_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_mixer00_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_mixer90_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_mixer90_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_termination_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_termination_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_neg_cap_up_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_neg_cap_up_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_neg_cap_mid_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_neg_cap_mid_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_neg_cap_dn_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_neg_cap_dn_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_polarity_up_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_polarity_up_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_polarity_mid_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_polarity_mid_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_polarity_dn_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_polarity_dn_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_polarity_up_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_polarity_up_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_polarity_mid_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_polarity_mid_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_polarity_dn_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_polarity_dn_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_amp_vga_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_amp_vga_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_amp_vga_last_stage_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_amp_vga_last_stage_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_up_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_up_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_mid_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_mid_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_dn_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_dn_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_up_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_up_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_mid_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_mid_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_dn_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_dn_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_adc_recording_admin_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_adc_recording_admin_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_adc_recording_status_get(const char *buf);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_adc_rocording_lanes_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_adc_rocording_lanes_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_edge_vos_ccal_en_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_edge_vos_ccal_en_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_adc_gain_shift_auto_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_adc_gain_shift_auto_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_ccal_mode_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_ccal_mode_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_ctle_override_ctrl_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_ctle_override_ctrl_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_vga_override_ctrl_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_vga_override_ctrl_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_adc_vos_override_ctrl_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_adc_vos_override_ctrl_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_adc_gain_override_ctrl_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_adc_gain_override_ctrl_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_phos_override_ctrl_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_phos_override_ctrl_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_cal_error_cnt_get(const char *buf);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_ccal_state_get(const char *buf);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_ccal_op_get(const char *buf);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_phos_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_phos_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_vga_vos1_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_vga_vos1_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_vga_vos0_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_vga_vos0_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_ctle_vos0_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_ctle_vos0_set(char *buf, uint8_t val);

void mlxsw_reg_slrp_page_data_slrp_7nm_adc_vos_memcpy_from(const char *buf, char *dst);

void mlxsw_reg_slrp_page_data_slrp_7nm_adc_vos_memcpy_to(char *buf, const char *src);

void mlxsw_reg_slrp_page_data_slrp_7nm_adc_gos_memcpy_from(const char *buf, char *dst);

void mlxsw_reg_slrp_page_data_slrp_7nm_adc_gos_memcpy_to(char *buf, const char *src);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_phos3_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_phos3_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_phos2_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_phos2_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_phos1_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_phos1_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_phos0_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_phos0_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_phos7_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_phos7_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_phos6_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_phos6_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_phos5_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_phos5_set(char *buf, uint8_t val);

uint8_t mlxsw_reg_slrp_page_data_slrp_7nm_phos4_get(const char *buf);

void mlxsw_reg_slrp_page_data_slrp_7nm_phos4_set(char *buf, uint8_t val);

static int __SLRP_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_slrp_reg   *slrp_reg = (struct ku_slrp_reg*)ku_reg;
    sxd_emad_index_tlv_t *index_tlv;
    u8                   *reg_tlv;

    if (flags & SX_EMAD_F_INDEXATION_TLV) {
        index_tlv = (sxd_emad_index_tlv_t*)inbox;
        reg_tlv = inbox + sizeof(sxd_emad_index_tlv_t);
        sxd_emad_index_tlv_port_single_build(index_tlv, slrp_reg->lp_msb, slrp_reg->local_port);
        *bytes_written += sizeof(sxd_emad_index_tlv_t);
    } else {
        reg_tlv = inbox;
        mlxsw_reg_slrp_local_port_set(reg_tlv, slrp_reg->local_port);
        mlxsw_reg_slrp_lp_msb_set(reg_tlv, slrp_reg->lp_msb);
    }

    mlxsw_reg_slrp_pnat_set(reg_tlv, slrp_reg->pnat);
    mlxsw_reg_slrp_lane_set(reg_tlv, slrp_reg->lane);
    mlxsw_reg_slrp_port_type_set(reg_tlv, slrp_reg->port_type);
    switch (slrp_reg->port_type) {
    case 0: /* Network port */
    case 2: /* Internal IC LR Port */
        switch (slrp_reg->version) {
        case 0: /* 40nm */
        case 1: /* 28nm */
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ib_sel_set(reg_tlv, slrp_reg->page_data.slrp_40nm_28nm.ib_sel);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_dp_sel_set(reg_tlv, slrp_reg->page_data.slrp_40nm_28nm.dp_sel);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_dp90sel_set(reg_tlv, slrp_reg->page_data.slrp_40nm_28nm.dp90sel);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mix90phase_set(reg_tlv,
                                                                   slrp_reg->page_data.slrp_40nm_28nm.mix90phase);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap0_set(reg_tlv, slrp_reg->page_data.slrp_40nm_28nm.ffe_tap0);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap1_set(reg_tlv, slrp_reg->page_data.slrp_40nm_28nm.ffe_tap1);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap2_set(reg_tlv, slrp_reg->page_data.slrp_40nm_28nm.ffe_tap2);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap3_set(reg_tlv, slrp_reg->page_data.slrp_40nm_28nm.ffe_tap3);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap4_set(reg_tlv, slrp_reg->page_data.slrp_40nm_28nm.ffe_tap4);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap5_set(reg_tlv, slrp_reg->page_data.slrp_40nm_28nm.ffe_tap5);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap6_set(reg_tlv, slrp_reg->page_data.slrp_40nm_28nm.ffe_tap6);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap7_set(reg_tlv, slrp_reg->page_data.slrp_40nm_28nm.ffe_tap7);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap8_set(reg_tlv, slrp_reg->page_data.slrp_40nm_28nm.ffe_tap8);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbias_tap_amp_set(reg_tlv,
                                                                          slrp_reg->page_data.slrp_40nm_28nm.mixerbias_tap_amp);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap_en_set(reg_tlv,
                                                                   slrp_reg->page_data.slrp_40nm_28nm.ffe_tap_en);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap_offset0_set(reg_tlv,
                                                                        slrp_reg->page_data.slrp_40nm_28nm.ffe_tap_offset0);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap_offset1_set(reg_tlv,
                                                                        slrp_reg->page_data.slrp_40nm_28nm.ffe_tap_offset1);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_slicer_offset0_set(reg_tlv,
                                                                       slrp_reg->page_data.slrp_40nm_28nm.slicer_offset0);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixer_offset0_set(reg_tlv,
                                                                      slrp_reg->page_data.slrp_40nm_28nm.mixer_offset0);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixer_offset1_set(reg_tlv,
                                                                      slrp_reg->page_data.slrp_40nm_28nm.mixer_offset1);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbgn_inp_set(reg_tlv,
                                                                     slrp_reg->page_data.slrp_40nm_28nm.mixerbgn_inp);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbgn_inn_set(reg_tlv,
                                                                     slrp_reg->page_data.slrp_40nm_28nm.mixerbgn_inn);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbgn_refp_set(reg_tlv,
                                                                      slrp_reg->page_data.slrp_40nm_28nm.mixerbgn_refp);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbgn_refn_set(reg_tlv,
                                                                      slrp_reg->page_data.slrp_40nm_28nm.mixerbgn_refn);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_sel_slicer_lctrl_h_set(reg_tlv,
                                                                           slrp_reg->page_data.slrp_40nm_28nm.sel_slicer_lctrl_h);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_sel_slicer_lctrl_l_set(reg_tlv,
                                                                           slrp_reg->page_data.slrp_40nm_28nm.sel_slicer_lctrl_l);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ref_mixer_vreg_set(reg_tlv,
                                                                       slrp_reg->page_data.slrp_40nm_28nm.ref_mixer_vreg);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_slicer_gctrl_set(reg_tlv,
                                                                     slrp_reg->page_data.slrp_40nm_28nm.slicer_gctrl);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_lctrl_input_set(reg_tlv,
                                                                    slrp_reg->page_data.slrp_40nm_28nm.lctrl_input);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixer_offset_cm1_set(reg_tlv,
                                                                         slrp_reg->page_data.slrp_40nm_28nm.mixer_offset_cm1);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_common_mode_set(reg_tlv,
                                                                    slrp_reg->page_data.slrp_40nm_28nm.common_mode);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixer_offset_cm0_set(reg_tlv,
                                                                         slrp_reg->page_data.slrp_40nm_28nm.mixer_offset_cm0);
            mlxsw_reg_slrp_page_data_slrp_40nm_28nm_slicer_offset_cm_set(reg_tlv,
                                                                         slrp_reg->page_data.slrp_40nm_28nm.slicer_offset_cm);
            break;

        case 3: /* 16nm */
            mlxsw_reg_slrp_page_data_slrp_16nm_mixer_offset_up_set(reg_tlv,
                                                                   slrp_reg->page_data.slrp_16nm.mixer_offset_up);
            mlxsw_reg_slrp_page_data_slrp_16nm_mixer_offset_down_set(reg_tlv,
                                                                     slrp_reg->page_data.slrp_16nm.mixer_offset_down);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_enc_set(reg_tlv, slrp_reg->page_data.slrp_16nm.sel_enc);
            mlxsw_reg_slrp_page_data_slrp_16nm_dp_sel_set(reg_tlv, slrp_reg->page_data.slrp_16nm.dp_sel);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_in_mixer_biasgen_up_set(reg_tlv,
                                                                               slrp_reg->page_data.slrp_16nm.sel_ref_in_mixer_biasgen_up);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_in_mixer_biasgen_mid_set(reg_tlv,
                                                                                slrp_reg->page_data.slrp_16nm.sel_ref_in_mixer_biasgen_mid);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_in_mixer_biasgen_down_set(reg_tlv,
                                                                                 slrp_reg->page_data.slrp_16nm.sel_ref_in_mixer_biasgen_down);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_in_mixer_biasgen_eo_set(reg_tlv,
                                                                               slrp_reg->page_data.slrp_16nm.sel_ref_in_mixer_biasgen_eo);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_out_mixer_biasgen_up_set(reg_tlv,
                                                                                slrp_reg->page_data.slrp_16nm.sel_ref_out_mixer_biasgen_up);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_out_mixer_biasgen_mid_set(reg_tlv,
                                                                                 slrp_reg->page_data.slrp_16nm.sel_ref_out_mixer_biasgen_mid);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_out_mixer_biasgen_down_set(reg_tlv,
                                                                                  slrp_reg->page_data.slrp_16nm.sel_ref_out_mixer_biasgen_down);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_out_mixer_biasgen_eo_set(reg_tlv,
                                                                                slrp_reg->page_data.slrp_16nm.sel_ref_out_mixer_biasgen_eo);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_amp_mixer_biasgen_up_set(reg_tlv,
                                                                            slrp_reg->page_data.slrp_16nm.sel_amp_mixer_biasgen_up);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_amp_mixer_biasgen_mid_set(reg_tlv,
                                                                             slrp_reg->page_data.slrp_16nm.sel_amp_mixer_biasgen_mid);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_amp_mixer_biasgen_down_set(reg_tlv,
                                                                              slrp_reg->page_data.slrp_16nm.sel_amp_mixer_biasgen_down);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_amp_mixer_biasgen_eo_set(reg_tlv,
                                                                            slrp_reg->page_data.slrp_16nm.sel_amp_mixer_biasgen_eo);
            mlxsw_reg_slrp_page_data_slrp_16nm_mixer_offset_eye_scan_set(reg_tlv,
                                                                         slrp_reg->page_data.slrp_16nm.mixer_offset_eye_scan);
            mlxsw_reg_slrp_page_data_slrp_16nm_gctrl_bin_bgn0_n_set(reg_tlv,
                                                                    slrp_reg->page_data.slrp_16nm.gctrl_bin_bgn0_n);
            mlxsw_reg_slrp_page_data_slrp_16nm_gctrl_bin_bgn0_p_set(reg_tlv,
                                                                    slrp_reg->page_data.slrp_16nm.gctrl_bin_bgn0_p);
            mlxsw_reg_slrp_page_data_slrp_16nm_gctrl_bin_bgn1_n_set(reg_tlv,
                                                                    slrp_reg->page_data.slrp_16nm.gctrl_bin_bgn1_n);
            mlxsw_reg_slrp_page_data_slrp_16nm_gctrl_bin_bgn1_p_set(reg_tlv,
                                                                    slrp_reg->page_data.slrp_16nm.gctrl_bin_bgn1_p);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_digital_gctrl_set(reg_tlv,
                                                                     slrp_reg->page_data.slrp_16nm.sel_digital_gctrl);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias2ctle_set(reg_tlv, slrp_reg->page_data.slrp_16nm.sel_bias2ctle);
            mlxsw_reg_slrp_page_data_slrp_16nm_selgc_ref_op0_alev_ctle_set(reg_tlv,
                                                                           slrp_reg->page_data.slrp_16nm.selgc_ref_op0_alev_ctle);
            mlxsw_reg_slrp_page_data_slrp_16nm_selgc_ref_op1_alev_ctle_set(reg_tlv,
                                                                           slrp_reg->page_data.slrp_16nm.selgc_ref_op1_alev_ctle);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_0_set(reg_tlv, slrp_reg->page_data.slrp_16nm.sel_bias_en_0);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_1_set(reg_tlv, slrp_reg->page_data.slrp_16nm.sel_bias_en_1);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_2_set(reg_tlv, slrp_reg->page_data.slrp_16nm.sel_bias_en_2);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_3_set(reg_tlv, slrp_reg->page_data.slrp_16nm.sel_bias_en_3);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_4_set(reg_tlv, slrp_reg->page_data.slrp_16nm.sel_bias_en_4);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_5_set(reg_tlv, slrp_reg->page_data.slrp_16nm.sel_bias_en_5);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_6_set(reg_tlv, slrp_reg->page_data.slrp_16nm.sel_bias_en_6);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_0_set(reg_tlv,
                                                                   slrp_reg->page_data.slrp_16nm.sel_gctrln_en_0);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_1_set(reg_tlv,
                                                                   slrp_reg->page_data.slrp_16nm.sel_gctrln_en_1);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_2_set(reg_tlv,
                                                                   slrp_reg->page_data.slrp_16nm.sel_gctrln_en_2);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_3_set(reg_tlv,
                                                                   slrp_reg->page_data.slrp_16nm.sel_gctrln_en_3);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_4_set(reg_tlv,
                                                                   slrp_reg->page_data.slrp_16nm.sel_gctrln_en_4);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_5_set(reg_tlv,
                                                                   slrp_reg->page_data.slrp_16nm.sel_gctrln_en_5);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_6_set(reg_tlv,
                                                                   slrp_reg->page_data.slrp_16nm.sel_gctrln_en_6);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_0_set(reg_tlv,
                                                                   slrp_reg->page_data.slrp_16nm.sel_gctrlp_en_0);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_1_set(reg_tlv,
                                                                   slrp_reg->page_data.slrp_16nm.sel_gctrlp_en_1);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_2_set(reg_tlv,
                                                                   slrp_reg->page_data.slrp_16nm.sel_gctrlp_en_2);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_3_set(reg_tlv,
                                                                   slrp_reg->page_data.slrp_16nm.sel_gctrlp_en_3);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_4_set(reg_tlv,
                                                                   slrp_reg->page_data.slrp_16nm.sel_gctrlp_en_4);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_5_set(reg_tlv,
                                                                   slrp_reg->page_data.slrp_16nm.sel_gctrlp_en_5);
            mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_6_set(reg_tlv,
                                                                   slrp_reg->page_data.slrp_16nm.sel_gctrlp_en_6);
            break;

        case 4: /* 7nm */
            mlxsw_reg_slrp_page_data_slrp_7nm_adc_recording_admin_set(reg_tlv,
                                                                      slrp_reg->page_data.slrp_7nm.adc_recording_admin);
            mlxsw_reg_slrp_page_data_slrp_7nm_adc_rocording_lanes_set(reg_tlv,
                                                                      slrp_reg->page_data.slrp_7nm.adc_rocording_lanes);
            mlxsw_reg_slrp_page_data_slrp_7nm_edge_vos_ccal_en_set(reg_tlv,
                                                                   slrp_reg->page_data.slrp_7nm.edge_vos_ccal_en);
            mlxsw_reg_slrp_page_data_slrp_7nm_adc_gain_shift_auto_set(reg_tlv,
                                                                      slrp_reg->page_data.slrp_7nm.adc_gain_shift_auto);
            mlxsw_reg_slrp_page_data_slrp_7nm_ccal_mode_set(reg_tlv, slrp_reg->page_data.slrp_7nm.ccal_mode);
            mlxsw_reg_slrp_page_data_slrp_7nm_ctle_override_ctrl_set(reg_tlv,
                                                                     slrp_reg->page_data.slrp_7nm.ctle_override_ctrl);
            mlxsw_reg_slrp_page_data_slrp_7nm_vga_override_ctrl_set(reg_tlv,
                                                                    slrp_reg->page_data.slrp_7nm.vga_override_ctrl);
            mlxsw_reg_slrp_page_data_slrp_7nm_adc_vos_override_ctrl_set(reg_tlv,
                                                                        slrp_reg->page_data.slrp_7nm.adc_vos_override_ctrl);
            mlxsw_reg_slrp_page_data_slrp_7nm_adc_gain_override_ctrl_set(reg_tlv,
                                                                         slrp_reg->page_data.slrp_7nm.adc_gain_override_ctrl);
            mlxsw_reg_slrp_page_data_slrp_7nm_phos_override_ctrl_set(reg_tlv,
                                                                     slrp_reg->page_data.slrp_7nm.phos_override_ctrl);
            mlxsw_reg_slrp_page_data_slrp_7nm_phos_set(reg_tlv, slrp_reg->page_data.slrp_7nm.phos);
            mlxsw_reg_slrp_page_data_slrp_7nm_vga_vos1_set(reg_tlv, slrp_reg->page_data.slrp_7nm.vga_vos1);
            mlxsw_reg_slrp_page_data_slrp_7nm_vga_vos0_set(reg_tlv, slrp_reg->page_data.slrp_7nm.vga_vos0);
            mlxsw_reg_slrp_page_data_slrp_7nm_ctle_vos0_set(reg_tlv, slrp_reg->page_data.slrp_7nm.ctle_vos0);
            mlxsw_reg_slrp_page_data_slrp_7nm_adc_vos_memcpy_to(inbox, (char *)slrp_reg->page_data.slrp_7nm.adc_vos);
            mlxsw_reg_slrp_page_data_slrp_7nm_adc_gos_memcpy_to(inbox, (char *)slrp_reg->page_data.slrp_7nm.adc_gos);
            mlxsw_reg_slrp_page_data_slrp_7nm_phos3_set(reg_tlv, slrp_reg->page_data.slrp_7nm.phos3);
            mlxsw_reg_slrp_page_data_slrp_7nm_phos2_set(reg_tlv, slrp_reg->page_data.slrp_7nm.phos2);
            mlxsw_reg_slrp_page_data_slrp_7nm_phos1_set(reg_tlv, slrp_reg->page_data.slrp_7nm.phos1);
            mlxsw_reg_slrp_page_data_slrp_7nm_phos0_set(reg_tlv, slrp_reg->page_data.slrp_7nm.phos0);
            mlxsw_reg_slrp_page_data_slrp_7nm_phos7_set(reg_tlv, slrp_reg->page_data.slrp_7nm.phos7);
            mlxsw_reg_slrp_page_data_slrp_7nm_phos6_set(reg_tlv, slrp_reg->page_data.slrp_7nm.phos6);
            mlxsw_reg_slrp_page_data_slrp_7nm_phos5_set(reg_tlv, slrp_reg->page_data.slrp_7nm.phos5);
            mlxsw_reg_slrp_page_data_slrp_7nm_phos4_set(reg_tlv, slrp_reg->page_data.slrp_7nm.phos4);
            break;

        default:
            break;
        }
        break;

    case 1: /* Near-End Port (For Retimer/Gearbox - Host side) */
    case 3: /* Far-End Port (For Retimer/Gearbox - Line side) */
        mlxsw_reg_slrp_page_data_slrp_gearbox_ffe_tap_index_set(reg_tlv,
                                                                slrp_reg->page_data.slrp_gearbox.ffe_tap_index);
        mlxsw_reg_slrp_page_data_slrp_gearbox_ffe_sub_channel_set(reg_tlv,
                                                                  slrp_reg->page_data.slrp_gearbox.ffe_sub_channel);
        mlxsw_reg_slrp_page_data_slrp_gearbox_snr_en_set(reg_tlv, slrp_reg->page_data.slrp_gearbox.snr_en);
        mlxsw_reg_slrp_page_data_slrp_gearbox_dsp_mode_set(reg_tlv, slrp_reg->page_data.slrp_gearbox.dsp_mode);
        mlxsw_reg_slrp_page_data_slrp_gearbox_afe_trim_set(reg_tlv, slrp_reg->page_data.slrp_gearbox.afe_trim);
        mlxsw_reg_slrp_page_data_slrp_gearbox_ctle_set(reg_tlv, slrp_reg->page_data.slrp_gearbox.ctle);
        mlxsw_reg_slrp_page_data_slrp_gearbox_dsp_err_gen_set(reg_tlv, slrp_reg->page_data.slrp_gearbox.dsp_err_gen);
        mlxsw_reg_slrp_page_data_slrp_gearbox_snr_sym_cnt_set(reg_tlv, slrp_reg->page_data.slrp_gearbox.snr_sym_cnt);
        break;

    case 4: /* USR Main */
    case 5: /* USR Tile */
        mlxsw_reg_slrp_page_data_slrp_usr_mixer00_set(reg_tlv, slrp_reg->page_data.slrp_usr.mixer00);
        mlxsw_reg_slrp_page_data_slrp_usr_mixer90_set(reg_tlv, slrp_reg->page_data.slrp_usr.mixer90);
        mlxsw_reg_slrp_page_data_slrp_usr_termination_set(reg_tlv, slrp_reg->page_data.slrp_usr.termination);
        mlxsw_reg_slrp_page_data_slrp_usr_neg_cap_up_set(reg_tlv, slrp_reg->page_data.slrp_usr.neg_cap_up);
        mlxsw_reg_slrp_page_data_slrp_usr_neg_cap_mid_set(reg_tlv, slrp_reg->page_data.slrp_usr.neg_cap_mid);
        mlxsw_reg_slrp_page_data_slrp_usr_neg_cap_dn_set(reg_tlv, slrp_reg->page_data.slrp_usr.neg_cap_dn);
        mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_polarity_up_set(reg_tlv,
                                                                        slrp_reg->page_data.slrp_usr.offset_coarse_polarity_up);
        mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_polarity_mid_set(reg_tlv,
                                                                         slrp_reg->page_data.slrp_usr.offset_coarse_polarity_mid);
        mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_polarity_dn_set(reg_tlv,
                                                                        slrp_reg->page_data.slrp_usr.offset_coarse_polarity_dn);
        mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_polarity_up_set(reg_tlv,
                                                                      slrp_reg->page_data.slrp_usr.offset_fine_polarity_up);
        mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_polarity_mid_set(reg_tlv,
                                                                       slrp_reg->page_data.slrp_usr.offset_fine_polarity_mid);
        mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_polarity_dn_set(reg_tlv,
                                                                      slrp_reg->page_data.slrp_usr.offset_fine_polarity_dn);
        mlxsw_reg_slrp_page_data_slrp_usr_amp_vga_set(reg_tlv, slrp_reg->page_data.slrp_usr.amp_vga);
        mlxsw_reg_slrp_page_data_slrp_usr_amp_vga_last_stage_set(reg_tlv,
                                                                 slrp_reg->page_data.slrp_usr.amp_vga_last_stage);
        mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_up_set(reg_tlv, slrp_reg->page_data.slrp_usr.offset_coarse_up);
        mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_mid_set(reg_tlv,
                                                                slrp_reg->page_data.slrp_usr.offset_coarse_mid);
        mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_dn_set(reg_tlv, slrp_reg->page_data.slrp_usr.offset_coarse_dn);
        mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_up_set(reg_tlv, slrp_reg->page_data.slrp_usr.offset_fine_up);
        mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_mid_set(reg_tlv, slrp_reg->page_data.slrp_usr.offset_fine_mid);
        mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_dn_set(reg_tlv, slrp_reg->page_data.slrp_usr.offset_fine_dn);
        break;

    default:
        break;
    }

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + MLXSW_SLRP_LEN);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + MLXSW_SLRP_LEN;
    return 0;
}

static int __SLRP_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_slrp_reg *slrp_reg = (struct ku_slrp_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;

    slrp_reg->status = mlxsw_reg_slrp_status_get(outbox);
    slrp_reg->version = mlxsw_reg_slrp_version_get(outbox);
    if (tlv_markers->index_tlv) {
        sxd_emad_index_tlv_port_single_deparse(tlv_markers->index_tlv,
                                               &slrp_reg->lp_msb,
                                               &slrp_reg->local_port);
    } else {
        slrp_reg->local_port = mlxsw_reg_slrp_local_port_get(outbox);
        slrp_reg->lp_msb = mlxsw_reg_slrp_lp_msb_get(outbox);
    }
    slrp_reg->pnat = mlxsw_reg_slrp_pnat_get(outbox);
    slrp_reg->lane = mlxsw_reg_slrp_lane_get(outbox);
    slrp_reg->port_type = mlxsw_reg_slrp_port_type_get(outbox);

    switch (slrp_reg->port_type) {
    case 0:     /* Network port */
    case 2:     /* Internal IC LR Port */
        switch (slrp_reg->version) {
        case 0:         /* 40nm */
        case 1:         /* 28nm */
            slrp_reg->page_data.slrp_40nm_28nm.ib_sel = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ib_sel_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.dp_sel = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_dp_sel_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.dp90sel = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_dp90sel_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.mix90phase = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mix90phase_get(
                outbox);
            slrp_reg->page_data.slrp_40nm_28nm.ffe_tap0 = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap0_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.ffe_tap1 = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap1_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.ffe_tap2 = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap2_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.ffe_tap3 = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap3_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.ffe_tap4 = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap4_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.ffe_tap5 = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap5_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.ffe_tap6 = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap6_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.ffe_tap7 = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap7_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.ffe_tap8 = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap8_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.mixerbias_tap_amp =
                mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbias_tap_amp_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.ffe_tap_en = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap_en_get(
                outbox);
            slrp_reg->page_data.slrp_40nm_28nm.ffe_tap_offset0 =
                mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap_offset0_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.ffe_tap_offset1 =
                mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ffe_tap_offset1_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.slicer_offset0 =
                mlxsw_reg_slrp_page_data_slrp_40nm_28nm_slicer_offset0_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.mixer_offset0 =
                mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixer_offset0_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.mixer_offset1 =
                mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixer_offset1_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.mixerbgn_inp = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbgn_inp_get(
                outbox);
            slrp_reg->page_data.slrp_40nm_28nm.mixerbgn_inn = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbgn_inn_get(
                outbox);
            slrp_reg->page_data.slrp_40nm_28nm.mixerbgn_refp =
                mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbgn_refp_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.mixerbgn_refn =
                mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixerbgn_refn_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.sel_slicer_lctrl_h =
                mlxsw_reg_slrp_page_data_slrp_40nm_28nm_sel_slicer_lctrl_h_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.sel_slicer_lctrl_l =
                mlxsw_reg_slrp_page_data_slrp_40nm_28nm_sel_slicer_lctrl_l_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.ref_mixer_vreg =
                mlxsw_reg_slrp_page_data_slrp_40nm_28nm_ref_mixer_vreg_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.slicer_gctrl = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_slicer_gctrl_get(
                outbox);
            slrp_reg->page_data.slrp_40nm_28nm.lctrl_input = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_lctrl_input_get(
                outbox);
            slrp_reg->page_data.slrp_40nm_28nm.mixer_offset_cm1 =
                mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixer_offset_cm1_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.common_mode = mlxsw_reg_slrp_page_data_slrp_40nm_28nm_common_mode_get(
                outbox);
            slrp_reg->page_data.slrp_40nm_28nm.mixer_offset_cm0 =
                mlxsw_reg_slrp_page_data_slrp_40nm_28nm_mixer_offset_cm0_get(outbox);
            slrp_reg->page_data.slrp_40nm_28nm.slicer_offset_cm =
                mlxsw_reg_slrp_page_data_slrp_40nm_28nm_slicer_offset_cm_get(outbox);
            break;

        case 3:         /* 16nm */
            slrp_reg->page_data.slrp_16nm.mixer_offset_up = mlxsw_reg_slrp_page_data_slrp_16nm_mixer_offset_up_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.mixer_offset_down = mlxsw_reg_slrp_page_data_slrp_16nm_mixer_offset_down_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.sel_enc = mlxsw_reg_slrp_page_data_slrp_16nm_sel_enc_get(outbox);
            slrp_reg->page_data.slrp_16nm.dp_sel = mlxsw_reg_slrp_page_data_slrp_16nm_dp_sel_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_ref_in_mixer_biasgen_up =
                mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_in_mixer_biasgen_up_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_ref_in_mixer_biasgen_mid =
                mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_in_mixer_biasgen_mid_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_ref_in_mixer_biasgen_down =
                mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_in_mixer_biasgen_down_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_ref_in_mixer_biasgen_eo =
                mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_in_mixer_biasgen_eo_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_ref_out_mixer_biasgen_up =
                mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_out_mixer_biasgen_up_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_ref_out_mixer_biasgen_mid =
                mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_out_mixer_biasgen_mid_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_ref_out_mixer_biasgen_down =
                mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_out_mixer_biasgen_down_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_ref_out_mixer_biasgen_eo =
                mlxsw_reg_slrp_page_data_slrp_16nm_sel_ref_out_mixer_biasgen_eo_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_amp_mixer_biasgen_up =
                mlxsw_reg_slrp_page_data_slrp_16nm_sel_amp_mixer_biasgen_up_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_amp_mixer_biasgen_mid =
                mlxsw_reg_slrp_page_data_slrp_16nm_sel_amp_mixer_biasgen_mid_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_amp_mixer_biasgen_down =
                mlxsw_reg_slrp_page_data_slrp_16nm_sel_amp_mixer_biasgen_down_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_amp_mixer_biasgen_eo =
                mlxsw_reg_slrp_page_data_slrp_16nm_sel_amp_mixer_biasgen_eo_get(outbox);
            slrp_reg->page_data.slrp_16nm.mixer_offset_eye_scan =
                mlxsw_reg_slrp_page_data_slrp_16nm_mixer_offset_eye_scan_get(outbox);
            slrp_reg->page_data.slrp_16nm.gctrl_bin_bgn0_n = mlxsw_reg_slrp_page_data_slrp_16nm_gctrl_bin_bgn0_n_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.gctrl_bin_bgn0_p = mlxsw_reg_slrp_page_data_slrp_16nm_gctrl_bin_bgn0_p_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.gctrl_bin_bgn1_n = mlxsw_reg_slrp_page_data_slrp_16nm_gctrl_bin_bgn1_n_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.gctrl_bin_bgn1_p = mlxsw_reg_slrp_page_data_slrp_16nm_gctrl_bin_bgn1_p_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.sel_digital_gctrl = mlxsw_reg_slrp_page_data_slrp_16nm_sel_digital_gctrl_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.sel_bias2ctle = mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias2ctle_get(outbox);
            slrp_reg->page_data.slrp_16nm.selgc_ref_op0_alev_ctle =
                mlxsw_reg_slrp_page_data_slrp_16nm_selgc_ref_op0_alev_ctle_get(outbox);
            slrp_reg->page_data.slrp_16nm.selgc_ref_op1_alev_ctle =
                mlxsw_reg_slrp_page_data_slrp_16nm_selgc_ref_op1_alev_ctle_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_bias_en_0 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_0_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_bias_en_1 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_1_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_bias_en_2 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_2_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_bias_en_3 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_3_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_bias_en_4 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_4_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_bias_en_5 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_5_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_bias_en_6 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_bias_en_6_get(outbox);
            slrp_reg->page_data.slrp_16nm.sel_gctrln_en_0 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_0_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.sel_gctrln_en_1 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_1_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.sel_gctrln_en_2 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_2_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.sel_gctrln_en_3 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_3_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.sel_gctrln_en_4 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_4_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.sel_gctrln_en_5 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_5_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.sel_gctrln_en_6 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrln_en_6_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.sel_gctrlp_en_0 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_0_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.sel_gctrlp_en_1 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_1_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.sel_gctrlp_en_2 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_2_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.sel_gctrlp_en_3 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_3_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.sel_gctrlp_en_4 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_4_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.sel_gctrlp_en_5 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_5_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.sel_gctrlp_en_6 = mlxsw_reg_slrp_page_data_slrp_16nm_sel_gctrlp_en_6_get(
                outbox);
            slrp_reg->page_data.slrp_16nm.crnt_bgn_offset_p1 =
                mlxsw_reg_slrp_page_data_slrp_16nm_crnt_bgn_offset_p1_get(outbox);
            slrp_reg->page_data.slrp_16nm.crnt_bgn_offset_n1 =
                mlxsw_reg_slrp_page_data_slrp_16nm_crnt_bgn_offset_n1_get(outbox);
            slrp_reg->page_data.slrp_16nm.crnt_bgn_offset_p0 =
                mlxsw_reg_slrp_page_data_slrp_16nm_crnt_bgn_offset_p0_get(outbox);
            slrp_reg->page_data.slrp_16nm.crnt_bgn_offset_n0 =
                mlxsw_reg_slrp_page_data_slrp_16nm_crnt_bgn_offset_n0_get(outbox);
            break;

        case 4:         /* 7nm */
            slrp_reg->page_data.slrp_7nm.adc_recording_admin =
                mlxsw_reg_slrp_page_data_slrp_7nm_adc_recording_admin_get(outbox);
            slrp_reg->page_data.slrp_7nm.adc_recording_status =
                mlxsw_reg_slrp_page_data_slrp_7nm_adc_recording_status_get(outbox);
            slrp_reg->page_data.slrp_7nm.adc_rocording_lanes =
                mlxsw_reg_slrp_page_data_slrp_7nm_adc_rocording_lanes_get(outbox);
            slrp_reg->page_data.slrp_7nm.edge_vos_ccal_en = mlxsw_reg_slrp_page_data_slrp_7nm_edge_vos_ccal_en_get(
                outbox);
            slrp_reg->page_data.slrp_7nm.adc_gain_shift_auto =
                mlxsw_reg_slrp_page_data_slrp_7nm_adc_gain_shift_auto_get(outbox);
            slrp_reg->page_data.slrp_7nm.ccal_mode = mlxsw_reg_slrp_page_data_slrp_7nm_ccal_mode_get(outbox);
            slrp_reg->page_data.slrp_7nm.ctle_override_ctrl = mlxsw_reg_slrp_page_data_slrp_7nm_ctle_override_ctrl_get(
                outbox);
            slrp_reg->page_data.slrp_7nm.vga_override_ctrl = mlxsw_reg_slrp_page_data_slrp_7nm_vga_override_ctrl_get(
                outbox);
            slrp_reg->page_data.slrp_7nm.adc_vos_override_ctrl =
                mlxsw_reg_slrp_page_data_slrp_7nm_adc_vos_override_ctrl_get(outbox);
            slrp_reg->page_data.slrp_7nm.adc_gain_override_ctrl =
                mlxsw_reg_slrp_page_data_slrp_7nm_adc_gain_override_ctrl_get(outbox);
            slrp_reg->page_data.slrp_7nm.phos_override_ctrl = mlxsw_reg_slrp_page_data_slrp_7nm_phos_override_ctrl_get(
                outbox);
            slrp_reg->page_data.slrp_7nm.cal_error_cnt = mlxsw_reg_slrp_page_data_slrp_7nm_cal_error_cnt_get(outbox);
            slrp_reg->page_data.slrp_7nm.ccal_state = mlxsw_reg_slrp_page_data_slrp_7nm_ccal_state_get(outbox);
            slrp_reg->page_data.slrp_7nm.ccal_op = mlxsw_reg_slrp_page_data_slrp_7nm_ccal_op_get(outbox);
            slrp_reg->page_data.slrp_7nm.phos = mlxsw_reg_slrp_page_data_slrp_7nm_phos_get(outbox);
            slrp_reg->page_data.slrp_7nm.vga_vos1 = mlxsw_reg_slrp_page_data_slrp_7nm_vga_vos1_get(outbox);
            slrp_reg->page_data.slrp_7nm.vga_vos0 = mlxsw_reg_slrp_page_data_slrp_7nm_vga_vos0_get(outbox);
            slrp_reg->page_data.slrp_7nm.ctle_vos0 = mlxsw_reg_slrp_page_data_slrp_7nm_ctle_vos0_get(outbox);
            mlxsw_reg_slrp_page_data_slrp_7nm_adc_vos_memcpy_from(outbox,
                                                                  (char *)slrp_reg->page_data.slrp_7nm.adc_vos);
            mlxsw_reg_slrp_page_data_slrp_7nm_adc_gos_memcpy_from(outbox,
                                                                  (char *)slrp_reg->page_data.slrp_7nm.adc_gos);
            slrp_reg->page_data.slrp_7nm.phos3 = mlxsw_reg_slrp_page_data_slrp_7nm_phos3_get(outbox);
            slrp_reg->page_data.slrp_7nm.phos2 = mlxsw_reg_slrp_page_data_slrp_7nm_phos2_get(outbox);
            slrp_reg->page_data.slrp_7nm.phos1 = mlxsw_reg_slrp_page_data_slrp_7nm_phos1_get(outbox);
            slrp_reg->page_data.slrp_7nm.phos0 = mlxsw_reg_slrp_page_data_slrp_7nm_phos0_get(outbox);
            slrp_reg->page_data.slrp_7nm.phos7 = mlxsw_reg_slrp_page_data_slrp_7nm_phos7_get(outbox);
            slrp_reg->page_data.slrp_7nm.phos6 = mlxsw_reg_slrp_page_data_slrp_7nm_phos6_get(outbox);
            slrp_reg->page_data.slrp_7nm.phos5 = mlxsw_reg_slrp_page_data_slrp_7nm_phos5_get(outbox);
            slrp_reg->page_data.slrp_7nm.phos4 = mlxsw_reg_slrp_page_data_slrp_7nm_phos4_get(outbox);
            break;

        default:
            break;
        }
        break;

    case 1:     /* Near-End Port (For Retimer/Gearbox - Host side) */
    case 3:     /* Far-End Port (For Retimer/Gearbox - Line side) */
        slrp_reg->page_data.slrp_gearbox.ffe_tap_index =
            mlxsw_reg_slrp_page_data_slrp_gearbox_ffe_tap_index_get(outbox);
        slrp_reg->page_data.slrp_gearbox.ffe_sub_channel = mlxsw_reg_slrp_page_data_slrp_gearbox_ffe_sub_channel_get(
            outbox);
        slrp_reg->page_data.slrp_gearbox.snr_en = mlxsw_reg_slrp_page_data_slrp_gearbox_snr_en_get(outbox);
        slrp_reg->page_data.slrp_gearbox.dsp_mode = mlxsw_reg_slrp_page_data_slrp_gearbox_dsp_mode_get(outbox);
        slrp_reg->page_data.slrp_gearbox.afe_trim = mlxsw_reg_slrp_page_data_slrp_gearbox_afe_trim_get(outbox);
        slrp_reg->page_data.slrp_gearbox.ctle = mlxsw_reg_slrp_page_data_slrp_gearbox_ctle_get(outbox);
        slrp_reg->page_data.slrp_gearbox.raw_snr_value =
            mlxsw_reg_slrp_page_data_slrp_gearbox_raw_snr_value_get(outbox);
        slrp_reg->page_data.slrp_gearbox.db_snr_value = mlxsw_reg_slrp_page_data_slrp_gearbox_db_snr_value_get(outbox);
        slrp_reg->page_data.slrp_gearbox.ffe_tap_value =
            mlxsw_reg_slrp_page_data_slrp_gearbox_ffe_tap_value_get(outbox);
        break;

    case 4:     /* USR Main */
    case 5:     /* USR Tile */
        slrp_reg->page_data.slrp_usr.mixer00 = mlxsw_reg_slrp_page_data_slrp_usr_mixer00_get(outbox);
        slrp_reg->page_data.slrp_usr.mixer90 = mlxsw_reg_slrp_page_data_slrp_usr_mixer90_get(outbox);
        slrp_reg->page_data.slrp_usr.termination = mlxsw_reg_slrp_page_data_slrp_usr_termination_get(outbox);
        slrp_reg->page_data.slrp_usr.neg_cap_up = mlxsw_reg_slrp_page_data_slrp_usr_neg_cap_up_get(outbox);
        slrp_reg->page_data.slrp_usr.neg_cap_mid = mlxsw_reg_slrp_page_data_slrp_usr_neg_cap_mid_get(outbox);
        slrp_reg->page_data.slrp_usr.neg_cap_dn = mlxsw_reg_slrp_page_data_slrp_usr_neg_cap_dn_get(outbox);
        slrp_reg->page_data.slrp_usr.offset_coarse_polarity_up =
            mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_polarity_up_get(outbox);
        slrp_reg->page_data.slrp_usr.offset_coarse_polarity_mid =
            mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_polarity_mid_get(outbox);
        slrp_reg->page_data.slrp_usr.offset_coarse_polarity_dn =
            mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_polarity_dn_get(outbox);
        slrp_reg->page_data.slrp_usr.offset_fine_polarity_up =
            mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_polarity_up_get(outbox);
        slrp_reg->page_data.slrp_usr.offset_fine_polarity_mid =
            mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_polarity_mid_get(outbox);
        slrp_reg->page_data.slrp_usr.offset_fine_polarity_dn =
            mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_polarity_dn_get(outbox);
        slrp_reg->page_data.slrp_usr.amp_vga = mlxsw_reg_slrp_page_data_slrp_usr_amp_vga_get(outbox);
        slrp_reg->page_data.slrp_usr.amp_vga_last_stage = mlxsw_reg_slrp_page_data_slrp_usr_amp_vga_last_stage_get(
            outbox);
        slrp_reg->page_data.slrp_usr.offset_coarse_up = mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_up_get(outbox);
        slrp_reg->page_data.slrp_usr.offset_coarse_mid =
            mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_mid_get(outbox);
        slrp_reg->page_data.slrp_usr.offset_coarse_dn = mlxsw_reg_slrp_page_data_slrp_usr_offset_coarse_dn_get(outbox);
        slrp_reg->page_data.slrp_usr.offset_fine_up = mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_up_get(outbox);
        slrp_reg->page_data.slrp_usr.offset_fine_mid = mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_mid_get(outbox);
        slrp_reg->page_data.slrp_usr.offset_fine_dn = mlxsw_reg_slrp_page_data_slrp_usr_offset_fine_dn_get(outbox);
        break;

    default:
        break;
    }

    return 0;
}

int sx_ACCESS_REG_SLRP(struct sx_dev *dev, struct ku_access_slrp_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __SLRP_encode,
                                  __SLRP_decode,
                                  &reg_data->slrp_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_SLRP);

int sx_EMAD_ACCESS_REG_SLRP(struct sx_dev                  *dev,
                            emad_prio_e                     priority,
                            sxd_emad_method_e               method,
                            const struct sx_emad_in_params *in_params,
                            struct sx_emad_out_params      *out_params,
                            struct ku_slrp_reg             *reg)
{
    return sx_emad_access_reg_common(dev,
                                     priority,
                                     method,
                                     MLXSW_SLRP_ID,
                                     reg,
                                     in_params,
                                     out_params,
                                     __SLRP_encode,
                                     __SLRP_decode);
}
EXPORT_SYMBOL(sx_EMAD_ACCESS_REG_SLRP);


/************************************************
 * SLCR
 ***********************************************/
#define SLCR_LOCAL_PORT_OFFEST 0x15
#define SLCR_LP_MSB_SH_OFFSET  0x02
#define SLCR_HASH_TYPE_OFFSET  0x03
#define SLCR_LAG_HASH_OFFSET   0x04
#define SLCR_SEED_OFFSET       0x08
#define SLCR_REG_LEN_IN_BYTES  (0x10)

static int __SLCR_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_slcr_reg   *slcr_reg = (struct ku_slcr_reg*)ku_reg;
    sxd_emad_index_tlv_t *index_tlv;
    u8                   *reg_tlv;
    u8                    lp_msb_sh = ((slcr_reg->lp_msb & 0x3) << 4) | (slcr_reg->sh & 0x1);
    u8                    type = slcr_reg->hash_type & 0xf;
    u32                   lag_hash = slcr_reg->hash_configuration & 0xfffff;

    if (flags & SX_EMAD_F_INDEXATION_TLV) {
        index_tlv = (sxd_emad_index_tlv_t*)inbox;
        reg_tlv = inbox + sizeof(sxd_emad_index_tlv_t);
        sxd_emad_index_tlv_port_single_build(index_tlv, slcr_reg->lp_msb, slcr_reg->local_port);
        *bytes_written += sizeof(sxd_emad_index_tlv_t);
    } else {
        reg_tlv = inbox;
        SX_PUT_REG_FIELD(reg_tlv, slcr_reg->local_port, SLCR_LOCAL_PORT_OFFEST);
        SX_PUT_REG_FIELD(reg_tlv, lp_msb_sh, SLCR_LP_MSB_SH_OFFSET);
    }

    SX_PUT_REG_FIELD(reg_tlv, type, SLCR_HASH_TYPE_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, lag_hash, SLCR_LAG_HASH_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, slcr_reg->seed, SLCR_SEED_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + SLCR_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + SLCR_REG_LEN_IN_BYTES;
    return 0;
}

static int __SLCR_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_slcr_reg *slcr_reg = (struct ku_slcr_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u8                  lp_msb_sh;

    SX_GET_REG_FIELD(lp_msb_sh, outbox, SLCR_LP_MSB_SH_OFFSET);

    if (tlv_markers->index_tlv) {
        sxd_emad_index_tlv_port_single_deparse(tlv_markers->index_tlv,
                                               &slcr_reg->lp_msb,
                                               &slcr_reg->local_port);
    } else {
        SX_GET_REG_FIELD(slcr_reg->local_port, outbox, SLCR_LOCAL_PORT_OFFEST);
        slcr_reg->lp_msb = (lp_msb_sh >> 4) & 0x3;
    }
    slcr_reg->sh = lp_msb_sh & 0x1;
    SX_GET_REG_FIELD(slcr_reg->hash_type, outbox, SLCR_HASH_TYPE_OFFSET);
    slcr_reg->hash_type &= 0xf;
    SX_GET_REG_FIELD(slcr_reg->hash_configuration, outbox, SLCR_LAG_HASH_OFFSET);
    slcr_reg->hash_configuration &= 0xfffff;
    SX_GET_REG_FIELD(slcr_reg->seed, outbox, SLCR_SEED_OFFSET);

    return 0;
}

int sx_ACCESS_REG_SLCR(struct sx_dev *dev, struct ku_access_slcr_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __SLCR_encode,
                                  __SLCR_decode,
                                  &reg_data->slcr_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_SLCR);

/************************************************
 * SFN
 ***********************************************/
#define SFN_SWID_OFFSET     0x00
#define SFN_REC_TYPE_OFFSET 0x01
#define SFN_END_OFFSET      0x05
#define SFN_NUM_REC_OFFSET  0x07
#define SFN_RECORD0_OFFSET  0x10

static int __SFN_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_sfn_reg *sfn_reg = (struct ku_sfn_reg*)ku_reg;
    u8                *reg_tlv = inbox;
    u8                 tmp_u8;
    u16                rec_size;
    u16                sfn_reg_len;

    SX_PUT_REG_FIELD(reg_tlv, sfn_reg->swid, SFN_SWID_OFFSET);
    tmp_u8 = (sfn_reg->rec_type & 0x1) << 4;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u8, SFN_REC_TYPE_OFFSET);
    tmp_u8 = (sfn_reg->end & 0x1) << 4;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u8, SFN_END_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, sfn_reg->num_records, SFN_NUM_REC_OFFSET);

    rec_size = (sfn_reg->rec_type == 0) ? 4 : 8;
    sfn_reg_len = 4 + sfn_reg->num_records * rec_size;

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + sfn_reg_len);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + sfn_reg_len;
    return 0;
}

static int __SFN_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_sfn_reg      *sfn_reg = (struct ku_sfn_reg*)ku_reg;
    u8                     *outbox = tlv_markers->reg_tlv->reg_data;
    struct sfn_record_data *curr_rec_usr = NULL;
    const u8               *curr_rec_prm = outbox + SFN_RECORD0_OFFSET;
    int                     i;

    SX_GET_REG_FIELD(sfn_reg->swid, outbox, SFN_SWID_OFFSET);
    SX_GET_REG_FIELD(sfn_reg->rec_type, outbox, SFN_REC_TYPE_OFFSET);
    sfn_reg->rec_type = (sfn_reg->rec_type >> 4) & 0x1;
    SX_GET_REG_FIELD(sfn_reg->num_records, outbox, SFN_NUM_REC_OFFSET);

    for (i = 0; i < sfn_reg->num_records; i++) {
        curr_rec_usr = &sfn_reg->records[i];

        curr_rec_usr->sfn_type = (curr_rec_prm[1] >> 4) & 0xf;
        switch (curr_rec_usr->sfn_type) {
        case 5: /* Learned MAC notification */
            memcpy(curr_rec_usr->sx_sfn_type.lrnt_mac.mac.ether_addr_octet, &curr_rec_prm[2], 6);
            curr_rec_usr->sx_sfn_type.lrnt_mac.fid = be16_to_cpu(*(u16*)&curr_rec_prm[10]);
            curr_rec_usr->sx_sfn_type.lrnt_mac.system_port = be16_to_cpu(*(u16*)&curr_rec_prm[14]);

            if (sfn_reg->rec_type == 1) { /* case of 32-byte record */
                curr_rec_usr->sx_sfn_type.lrnt_mac.counter_set.type = curr_rec_prm[28];
                curr_rec_usr->sx_sfn_type.lrnt_mac.counter_set.index = be32_to_cpu(*(u32*)&curr_rec_prm[28]) &
                                                                       0xffffff;
            }

            break;

        case 6: /* Learned MAC LAG notification */
            memcpy(curr_rec_usr->sx_sfn_type.lrnt_mac_lag.mac.ether_addr_octet, &curr_rec_prm[2], 6);
            curr_rec_usr->sx_sfn_type.lrnt_mac_lag.fid = be16_to_cpu(*(u16*)&curr_rec_prm[10]);
            curr_rec_usr->sx_sfn_type.lrnt_mac_lag.lag_id = be16_to_cpu(*(u16*)&curr_rec_prm[14]) & 0x1ff;
            if (sfn_reg->rec_type == 1) { /* case of 32-byte record */
                curr_rec_usr->sx_sfn_type.lrnt_mac_lag.counter_set.type = curr_rec_prm[28];
                curr_rec_usr->sx_sfn_type.lrnt_mac_lag.counter_set.index = be32_to_cpu(*(u32*)&curr_rec_prm[28]) &
                                                                           0xffffff;
            }

            break;

        case 0xd: /* Learned Unicast Tunnel FDB notification */
            memcpy(curr_rec_usr->sx_sfn_type.lrnt_uc_tunnel.mac.ether_addr_octet, &curr_rec_prm[2], 6);
            curr_rec_usr->sx_sfn_type.lrnt_uc_tunnel.udip_msb = curr_rec_prm[8];
            curr_rec_usr->sx_sfn_type.lrnt_uc_tunnel.fid = be16_to_cpu(*(u16*)&curr_rec_prm[10]);
            curr_rec_usr->sx_sfn_type.lrnt_uc_tunnel.udip_lsb = be32_to_cpu(*(u32*)&curr_rec_prm[12]) & 0xffffff;

            if (sfn_reg->rec_type == 1) { /* case of 32-byte record */
                curr_rec_usr->sx_sfn_type.lrnt_uc_tunnel.counter_set.type = curr_rec_prm[28];
                curr_rec_usr->sx_sfn_type.lrnt_uc_tunnel.counter_set.index = be32_to_cpu(*(u32*)&curr_rec_prm[28]) &
                                                                             0xffffff;
            }

            break;

        case 7: /* Aged-out MAC record */
            memcpy(curr_rec_usr->sx_sfn_type.aged_mac.mac.ether_addr_octet, &curr_rec_prm[2], 6);
            curr_rec_usr->sx_sfn_type.aged_mac.fid = be16_to_cpu(*(u16*)&curr_rec_prm[10]);
            curr_rec_usr->sx_sfn_type.aged_mac.system_port = be16_to_cpu(*(u16*)&curr_rec_prm[14]);

            if (sfn_reg->rec_type == 1) { /* case of 32-byte record */
                curr_rec_usr->sx_sfn_type.aged_mac.counter_set.type = curr_rec_prm[28];
                curr_rec_usr->sx_sfn_type.aged_mac.counter_set.index = be32_to_cpu(*(u32*)&curr_rec_prm[28]) &
                                                                       0xffffff;
            }

            break;

        case 8: /* Aged-out MAC LAG record */
            memcpy(curr_rec_usr->sx_sfn_type.aged_mac_lag.mac.ether_addr_octet, &curr_rec_prm[2], 6);
            curr_rec_usr->sx_sfn_type.aged_mac_lag.fid = be16_to_cpu(*(u16*)&curr_rec_prm[10]);
            curr_rec_usr->sx_sfn_type.aged_mac_lag.lag_id = be16_to_cpu(*(u16*)&curr_rec_prm[14]) & 0x1ff;

            if (sfn_reg->rec_type == 1) { /* case of 32-byte record */
                curr_rec_usr->sx_sfn_type.aged_mac_lag.counter_set.type = curr_rec_prm[28];
                curr_rec_usr->sx_sfn_type.aged_mac_lag.counter_set.index = be32_to_cpu(*(u32*)&curr_rec_prm[28]) &
                                                                           0xffffff;
            }

            break;

        case 0xe: /* Aged-out Unicast Tunnel FDB record */
            memcpy(curr_rec_usr->sx_sfn_type.aged_uc_tunnel.mac.ether_addr_octet, &curr_rec_prm[2], 6);
            curr_rec_usr->sx_sfn_type.aged_uc_tunnel.udip_msb = curr_rec_prm[8];
            curr_rec_usr->sx_sfn_type.aged_uc_tunnel.fid = be16_to_cpu(*(u16*)&curr_rec_prm[10]);
            curr_rec_usr->sx_sfn_type.aged_uc_tunnel.protocol = (curr_rec_prm[12] >> 3) & 0x1;
            curr_rec_usr->sx_sfn_type.aged_uc_tunnel.udip_lsb = be32_to_cpu(*(u32*)&curr_rec_prm[12]) & 0xffffff;

            if (sfn_reg->rec_type == 1) { /* case of 32-byte record */
                curr_rec_usr->sx_sfn_type.aged_uc_tunnel.counter_set.type = curr_rec_prm[28];
                curr_rec_usr->sx_sfn_type.aged_uc_tunnel.counter_set.index = be32_to_cpu(*(u32*)&curr_rec_prm[28]) &
                                                                             0xffffff;
            }

            break;

        default:
            sxd_log_err(SXD_DEV_NONE,
                        "SFN type (decode) is unsupported (rec_index=%d, type=0x%x)\n",
                        i,
                        curr_rec_usr->sfn_type);
            break;
        }

        curr_rec_prm += (sfn_reg->rec_type == 0) ? 16 : 32;
    }

    return 0;
}

int sx_ACCESS_REG_SFN(struct sx_dev *dev, struct ku_access_sfn_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __SFN_encode,
                                  __SFN_decode,
                                  &reg_data->sfn_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_SFN);

/************************************************
 * SPVTR
 ***********************************************/
#define SPVTR_TPORT_OFFSET                  0x00
#define SPVTR_LOCAL_PORT_OFFSET             0x01
#define SPVTR_LP_MSB_OFFSET                 0x02
#define SPVTR_IPPE_IPVE_EPVE_OFFSET         0x04
#define SPVTR_IPPRIO_MODE_IPVID_MODE_OFFSET 0x05
#define SPVTR_EPVID_MODE_OFFSET             0x07
#define SPVTR_REG_LEN_IN_BYTES              (0x10)

static int __SPVTR_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_spvtr_reg  *spvtr_reg = (struct ku_spvtr_reg*)ku_reg;
    sxd_emad_index_tlv_t *index_tlv;
    u8                   *reg_tlv;
    u8                    tmp_u8;

    if (flags & SX_EMAD_F_INDEXATION_TLV) {
        index_tlv = (sxd_emad_index_tlv_t*)inbox;
        reg_tlv = inbox + sizeof(sxd_emad_index_tlv_t);
        sxd_emad_index_tlv_port_single_build(index_tlv, spvtr_reg->lp_msb, spvtr_reg->local_port);
        *bytes_written += sizeof(sxd_emad_index_tlv_t);
    } else {
        reg_tlv = inbox;
        SX_PUT_REG_FIELD(reg_tlv, spvtr_reg->local_port, SPVTR_LOCAL_PORT_OFFSET);
        tmp_u8 = spvtr_reg->lp_msb << 4;
        SX_PUT_REG_FIELD(reg_tlv, tmp_u8, SPVTR_LP_MSB_OFFSET);
    }

    tmp_u8 = spvtr_reg->tport & 0x1;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u8, SPVTR_TPORT_OFFSET);

    tmp_u8 = 0;
    tmp_u8 |= spvtr_reg->ipprio_enable ? (1 << 7) : 0;
    tmp_u8 |= spvtr_reg->ipvid_enable ? (1 << 6) : 0;
    tmp_u8 |= spvtr_reg->epvid_enable ? (1 << 5) : 0;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u8, SPVTR_IPPE_IPVE_EPVE_OFFSET);

    tmp_u8 = ((spvtr_reg->ipprio_mode & 0xf) << 4) | (spvtr_reg->ipvid_mode & 0xf);
    SX_PUT_REG_FIELD(reg_tlv, tmp_u8, SPVTR_IPPRIO_MODE_IPVID_MODE_OFFSET);

    tmp_u8 = spvtr_reg->epvid_mode & 0xf;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u8, SPVTR_EPVID_MODE_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + SPVTR_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + SPVTR_REG_LEN_IN_BYTES;
    return 0;
}

static int __SPVTR_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_spvtr_reg *spvtr_reg = (struct ku_spvtr_reg*)ku_reg;
    u8                  *outbox = tlv_markers->reg_tlv->reg_data;
    u8                   tmp_u8;

    SX_GET_REG_FIELD(tmp_u8, outbox, SPVTR_TPORT_OFFSET);
    spvtr_reg->tport = tmp_u8 & 0x1;

    if (tlv_markers->index_tlv) {
        sxd_emad_index_tlv_port_single_deparse(tlv_markers->index_tlv,
                                               &spvtr_reg->lp_msb,
                                               &spvtr_reg->local_port);
    } else {
        SX_GET_REG_FIELD(spvtr_reg->local_port, outbox, SPVTR_LOCAL_PORT_OFFSET);
        SX_GET_REG_FIELD(tmp_u8, outbox, SPVTR_LP_MSB_OFFSET);
        spvtr_reg->lp_msb = (tmp_u8 >> 4) & 0x3;
    }

    SX_GET_REG_FIELD(tmp_u8, outbox, SPVTR_IPPE_IPVE_EPVE_OFFSET);
    spvtr_reg->ipprio_enable = (tmp_u8 >> 7) & 0x1;
    spvtr_reg->ipvid_enable = (tmp_u8 >> 6) & 0x1;
    spvtr_reg->epvid_enable = (tmp_u8 >> 5) & 0x1;

    SX_GET_REG_FIELD(tmp_u8, outbox, SPVTR_IPPRIO_MODE_IPVID_MODE_OFFSET);
    spvtr_reg->ipprio_mode = (tmp_u8 >> 4) & 0xf;
    spvtr_reg->ipvid_mode = tmp_u8 & 0xf;

    SX_GET_REG_FIELD(tmp_u8, outbox, SPVTR_EPVID_MODE_OFFSET);
    spvtr_reg->epvid_mode = tmp_u8 & 0xf;

    return 0;
}

int sx_ACCESS_REG_SPVTR(struct sx_dev *dev, struct ku_access_spvtr_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __SPVTR_encode,
                                  __SPVTR_decode,
                                  &reg_data->spvtr_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_SPVTR);

/************************************************
 * CWTP
 ***********************************************/
#define CWTP_LOCAL_PORT_OFFSET              0x01
#define CWTP_LP_MSB_OFFSET                  0x02
#define CWTP_TRAFIC_CLASS_OFFSET            0x23
#define CWTP_MODE_OFFSET                    0x24
#define CWTP_PROFILE_MIN_OFFSET             0x28
#define CWTP_PROFILE_PERCENT_AND_MAX_OFFSET 0x2C
#define CWTP_REG_LEN_IN_BYTES               (0x40)

static int __CWTP_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_cwtp_reg   *cwtp_reg = (struct ku_cwtp_reg*)ku_reg;
    sxd_emad_index_tlv_t *index_tlv;
    u8                   *reg_tlv;
    u32                   tmp_u32;
    u8                    tmp_u8;
    int                   i;

    if (flags & SX_EMAD_F_INDEXATION_TLV) {
        index_tlv = (sxd_emad_index_tlv_t*)inbox;
        reg_tlv = inbox + sizeof(sxd_emad_index_tlv_t);
        sxd_emad_index_tlv_port_single_build(index_tlv, cwtp_reg->lp_msb, cwtp_reg->local_port);
        *bytes_written += sizeof(sxd_emad_index_tlv_t);
    } else {
        reg_tlv = inbox;
        SX_PUT_REG_FIELD(reg_tlv, cwtp_reg->local_port, CWTP_LOCAL_PORT_OFFSET);
        tmp_u8 = cwtp_reg->lp_msb << 4;
        SX_PUT_REG_FIELD(reg_tlv, tmp_u8, CWTP_LP_MSB_OFFSET);
    }


    SX_PUT_REG_FIELD(reg_tlv, cwtp_reg->traffic_class, CWTP_TRAFIC_CLASS_OFFSET);

    tmp_u8 = cwtp_reg->mode & 0x1;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u8, CWTP_MODE_OFFSET);

    for (i = 0; i < 3; i++) {
        tmp_u32 = cwtp_reg->profiles[i].profile_i_min & 0xfffff;
        SX_PUT_REG_FIELD(reg_tlv, tmp_u32, (CWTP_PROFILE_MIN_OFFSET + (i * 8)));

        tmp_u32 = (((u32)(cwtp_reg->profiles[i].profile_i_percent & 0x7f)) << 24) |
                  (cwtp_reg->profiles[i].profile_i_max & 0xfffff);
        SX_PUT_REG_FIELD(reg_tlv, tmp_u32, (CWTP_PROFILE_PERCENT_AND_MAX_OFFSET + (i * 8)));
    }

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + CWTP_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + CWTP_REG_LEN_IN_BYTES;
    return 0;
}

static int __CWTP_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_cwtp_reg *cwtp_reg = (struct ku_cwtp_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u32                 tmp_u32;
    u8                  tmp_u8;
    int                 i;

    if (tlv_markers->index_tlv) {
        sxd_emad_index_tlv_port_single_deparse(tlv_markers->index_tlv,
                                               &cwtp_reg->lp_msb,
                                               &cwtp_reg->local_port);
    } else {
        SX_GET_REG_FIELD(cwtp_reg->local_port, outbox, CWTP_LOCAL_PORT_OFFSET);
        SX_GET_REG_FIELD(tmp_u8, outbox, CWTP_LP_MSB_OFFSET);
        cwtp_reg->lp_msb = (tmp_u8 >> 4) & 0x3;
    }

    SX_GET_REG_FIELD(cwtp_reg->traffic_class, outbox, CWTP_TRAFIC_CLASS_OFFSET);

    SX_GET_REG_FIELD(tmp_u8, outbox, CWTP_MODE_OFFSET);
    cwtp_reg->mode = tmp_u8 & 0x1;

    for (i = 0; i < 3; i++) {
        SX_GET_REG_FIELD(tmp_u32, outbox, (CWTP_PROFILE_MIN_OFFSET + (i * 8)));
        cwtp_reg->profiles[i].profile_i_min = tmp_u32 & 0xfffff;

        SX_GET_REG_FIELD(tmp_u32, outbox, (CWTP_PROFILE_PERCENT_AND_MAX_OFFSET + (i * 8)));
        cwtp_reg->profiles[i].profile_i_percent = (tmp_u32 >> 24) & 0x7f;
        cwtp_reg->profiles[i].profile_i_max = tmp_u32 & 0xfffff;
    }

    return 0;
}

int sx_ACCESS_REG_CWTP(struct sx_dev *dev, struct ku_access_cwtp_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __CWTP_encode,
                                  __CWTP_decode,
                                  &reg_data->cwtp_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_CWTP);

/************************************************
 * PFCC
 ***********************************************/
#define PFCC_DWORD0_OFFSET    0x00
#define PFCC_DWORD1_OFFSET    0x04
#define PFCC_DWORD2_OFFSET    0x08
#define PFCC_DWORD3_OFFSET    0x0C
#define PFCC_DWORD4_OFFSET    0x10
#define PFCC_DWORD5_OFFSET    0x14
#define PFCC_DWORD6_OFFSET    0x18
#define PFCC_REG_LEN_IN_BYTES (0x20)

static int __PFCC_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_pfcc_reg   *pfcc_reg = (struct ku_pfcc_reg*)ku_reg;
    sxd_emad_index_tlv_t *index_tlv;
    u8                   *reg_tlv;
    u32                   tmp_u32;

    if (flags & SX_EMAD_F_INDEXATION_TLV) {
        index_tlv = (sxd_emad_index_tlv_t*)inbox;
        reg_tlv = inbox + sizeof(sxd_emad_index_tlv_t);
        sxd_emad_index_tlv_port_single_build(index_tlv, pfcc_reg->lp_msb, pfcc_reg->local_port);
        *bytes_written += sizeof(sxd_emad_index_tlv_t);
    } else {
        reg_tlv = inbox;
        tmp_u32 = (((u32)(pfcc_reg->local_port & 0xff)) << 16) |
                  (((u32)(pfcc_reg->lp_msb & 0x3)) << 12);
        SX_PUT_REG_FIELD(reg_tlv, tmp_u32, PFCC_DWORD0_OFFSET);
    }


    tmp_u32 = (((u32)(pfcc_reg->prio_mask_tx & 0xff)) << 16) |
              (pfcc_reg->prio_mask_rx & 0xff);
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, PFCC_DWORD1_OFFSET);

    tmp_u32 = (((u32)(pfcc_reg->pause_policy_tx & 0x1)) << 31) |
              (((u32)(pfcc_reg->patx & 0x3)) << 27) |
              (((u32)(pfcc_reg->prio_policy_tx & 0xff)) << 16) |
              (pfcc_reg->cb_policy_tx & 0xff);
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, PFCC_DWORD2_OFFSET);

    tmp_u32 = (((u32)(pfcc_reg->pause_policy_rx & 0x1)) << 31) |
              (((u32)(pfcc_reg->parx & 0x3)) << 27) |
              (((u32)(pfcc_reg->prio_policy_rx & 0xff)) << 16) |
              (pfcc_reg->cb_policy_rx & 0xff);
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, PFCC_DWORD3_OFFSET);
    /* DWORD4 & DWORD5 are reserved*/
    tmp_u32 = 0;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, PFCC_DWORD4_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, PFCC_DWORD5_OFFSET);

    tmp_u32 = (((u32)(pfcc_reg->pfc_rx_timer_mode & 0x3) << 16) |
               (pfcc_reg->pfc_rx_timer_val & 0xffff));
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, PFCC_DWORD6_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + PFCC_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + PFCC_REG_LEN_IN_BYTES;
    return 0;
}

static int __PFCC_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_pfcc_reg *pfcc_reg = (struct ku_pfcc_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u32                 tmp_u32;

    if (tlv_markers->index_tlv) {
        sxd_emad_index_tlv_port_single_deparse(tlv_markers->index_tlv,
                                               &pfcc_reg->lp_msb,
                                               &pfcc_reg->local_port);
    } else {
        SX_GET_REG_FIELD(tmp_u32, outbox, PFCC_DWORD0_OFFSET);
        pfcc_reg->local_port = (tmp_u32 >> 16) & 0xff;
        pfcc_reg->lp_msb = (tmp_u32 >> 12) & 0x3;
    }

    SX_GET_REG_FIELD(tmp_u32, outbox, PFCC_DWORD1_OFFSET);
    pfcc_reg->prio_mask_tx = (tmp_u32 >> 16) & 0xff;
    pfcc_reg->prio_mask_rx = tmp_u32 & 0xff;

    SX_GET_REG_FIELD(tmp_u32, outbox, PFCC_DWORD2_OFFSET);
    pfcc_reg->pause_policy_tx = (tmp_u32 >> 31) & 0x1;
    pfcc_reg->patx = (tmp_u32 >> 27) & 0x3;
    pfcc_reg->prio_policy_tx = (tmp_u32 >> 16) & 0xff;
    pfcc_reg->cb_policy_tx = tmp_u32 & 0xff;

    SX_GET_REG_FIELD(tmp_u32, outbox, PFCC_DWORD3_OFFSET);
    pfcc_reg->pause_policy_rx = (tmp_u32 >> 31) & 0x1;
    pfcc_reg->parx = (tmp_u32 >> 27) & 0x3;
    pfcc_reg->prio_policy_rx = (tmp_u32 >> 16) & 0xff;
    pfcc_reg->cb_policy_rx = tmp_u32 & 0xff;

    SX_GET_REG_FIELD(tmp_u32, outbox, PFCC_DWORD6_OFFSET);
    pfcc_reg->pfc_rx_timer_mode = (tmp_u32 >> 16) & 0x3;
    pfcc_reg->pfc_rx_timer_val = (tmp_u32 & 0xffff);

    return 0;
}

int sx_ACCESS_REG_PFCC(struct sx_dev *dev, struct ku_access_pfcc_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __PFCC_encode,
                                  __PFCC_decode,
                                  &reg_data->pfcc_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_PFCC);

/************************************************
 * QPPM
 ***********************************************/
#define QPPM_LOCAL_PORT_OFFSET   0x01
#define QPPM_LP_MSB_OFFSET       0x02
#define QPPM_SWITCH_PRIO0_OFFSET 0x04
#define QPPM_REG_LEN_IN_BYTES    (0x44)

static int __QPPM_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_qppm_reg   *qppm_reg = (struct ku_qppm_reg*)ku_reg;
    sxd_emad_index_tlv_t *index_tlv;
    u8                   *reg_tlv;
    u32                   tmp_u32;
    u8                    tmp_u8;
    int                   i, color_idx;

    if (flags & SX_EMAD_F_INDEXATION_TLV) {
        index_tlv = (sxd_emad_index_tlv_t*)inbox;
        reg_tlv = inbox + sizeof(sxd_emad_index_tlv_t);
        sxd_emad_index_tlv_port_single_build(index_tlv, qppm_reg->lp_msb, qppm_reg->local_port);
        *bytes_written += sizeof(sxd_emad_index_tlv_t);
    } else {
        reg_tlv = inbox;
        SX_PUT_REG_FIELD(reg_tlv, qppm_reg->local_port, QPPM_LOCAL_PORT_OFFSET);
        tmp_u8 = qppm_reg->lp_msb << 4;
        SX_PUT_REG_FIELD(reg_tlv, tmp_u8, QPPM_LP_MSB_OFFSET);
    }


    for (i = 0; i < 16; i++) {
        tmp_u32 = 0;

        for (color_idx = 0; color_idx < 3; color_idx++) {
            tmp_u8 = ((qppm_reg->switch_prio[i].color[color_idx].enable_pcp & 0x1) << 7) |
                     ((qppm_reg->switch_prio[i].color[color_idx].dei & 0x1) << 4) |
                     (qppm_reg->switch_prio[i].color[color_idx].pcp & 0x7);

            tmp_u32 |= ((u32)tmp_u8) << (8 * (3 - color_idx));
        }

        SX_PUT_REG_FIELD(reg_tlv, tmp_u32, (QPPM_SWITCH_PRIO0_OFFSET + i * 4));
    }

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + QPPM_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + QPPM_REG_LEN_IN_BYTES;
    return 0;
}

static int __QPPM_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_qppm_reg *qppm_reg = (struct ku_qppm_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u32                 tmp_u32;
    u8                  tmp_u8;
    int                 i, color_idx;

    if (tlv_markers->index_tlv) {
        sxd_emad_index_tlv_port_single_deparse(tlv_markers->index_tlv,
                                               &qppm_reg->lp_msb,
                                               &qppm_reg->local_port);
    } else {
        SX_GET_REG_FIELD(qppm_reg->local_port, outbox, QPPM_LOCAL_PORT_OFFSET);
        SX_GET_REG_FIELD(tmp_u8, outbox, QPPM_LP_MSB_OFFSET);
        qppm_reg->lp_msb = (tmp_u8 >> 4) & 0x3;
    }

    for (i = 0; i < 16; i++) {
        SX_GET_REG_FIELD(tmp_u32, outbox, (QPPM_SWITCH_PRIO0_OFFSET + i * 4));

        for (color_idx = 0; color_idx < 3; color_idx++) {
            tmp_u8 = (tmp_u32 >> (8 * (3 - color_idx))) & 0xff;

            qppm_reg->switch_prio[i].color[color_idx].enable_pcp = (tmp_u8 >> 7) & 0x1;
            qppm_reg->switch_prio[i].color[color_idx].dei = (tmp_u8 >> 4) & 0x1;
            qppm_reg->switch_prio[i].color[color_idx].pcp = tmp_u8 & 0x7;
        }
    }

    return 0;
}

int sx_ACCESS_REG_QPPM(struct sx_dev *dev, struct ku_access_qppm_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __QPPM_encode,
                                  __QPPM_decode,
                                  &reg_data->qppm_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_QPPM);

/************************************************
 * SBMM
 ***********************************************/
#define SBMM_DWORD_OFFSET(x) (4 * (x))
#define SBMM_REG_LEN_IN_BYTES (0x28)

static int __SBMM_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_sbmm_reg *sbmm_reg = (struct ku_sbmm_reg*)ku_reg;
    u8                 *reg_tlv = inbox;
    u32                 tmp_u32;

    tmp_u32 = (sbmm_reg->prio & 0xf) << 8;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, SBMM_DWORD_OFFSET(0));

    tmp_u32 = (sbmm_reg->buff_occupancy) & 0xffffff;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, SBMM_DWORD_OFFSET(4));

    tmp_u32 = (((u32)(sbmm_reg->clr & 0x1)) << 31) | (sbmm_reg->max_buff_occupancy & 0xffffff);
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, SBMM_DWORD_OFFSET(5));

    tmp_u32 = (sbmm_reg->min_buff) & 0xffffff;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, SBMM_DWORD_OFFSET(6));

    tmp_u32 = (((u32)(sbmm_reg->infinite_size & 0x1)) << 31) | (sbmm_reg->max_buff & 0xffffff);
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, SBMM_DWORD_OFFSET(7));

    tmp_u32 = (sbmm_reg->pool) & 0xf;
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, SBMM_DWORD_OFFSET(9));

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + SBMM_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + SBMM_REG_LEN_IN_BYTES;
    return 0;
}

static int __SBMM_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_sbmm_reg *sbmm_reg = (struct ku_sbmm_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u32                 tmp_u32;

    SX_GET_REG_FIELD(tmp_u32, outbox, SBMM_DWORD_OFFSET(0));
    sbmm_reg->prio = (tmp_u32 >> 8) & 0xf;

    SX_GET_REG_FIELD(tmp_u32, outbox, SBMM_DWORD_OFFSET(4));
    sbmm_reg->buff_occupancy = tmp_u32 & 0xffffff;

    SX_GET_REG_FIELD(tmp_u32, outbox, SBMM_DWORD_OFFSET(5));
    sbmm_reg->clr = (tmp_u32 >> 31) & 0x1;
    sbmm_reg->max_buff_occupancy = tmp_u32 & 0xffffff;

    SX_GET_REG_FIELD(tmp_u32, outbox, SBMM_DWORD_OFFSET(6));
    sbmm_reg->min_buff = tmp_u32 & 0xffffff;

    SX_GET_REG_FIELD(tmp_u32, outbox, SBMM_DWORD_OFFSET(7));
    sbmm_reg->infinite_size = (tmp_u32 >> 31) & 0x1;
    sbmm_reg->max_buff = tmp_u32 & 0xffffff;

    SX_GET_REG_FIELD(tmp_u32, outbox, SBMM_DWORD_OFFSET(9));
    sbmm_reg->pool = tmp_u32 & 0xf;

    return 0;
}

int sx_ACCESS_REG_SBMM(struct sx_dev *dev, struct ku_access_sbmm_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __SBMM_encode,
                                  __SBMM_decode,
                                  &reg_data->sbmm_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_SBMM);

/************************************************
 * SLECR
 ***********************************************/
#define SLECR_DWORD0_OFFSET 0x00
#define SLECR_REG_LEN       4

static int __SLECR_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_slecr_reg *slecr_reg = (struct ku_slecr_reg*)ku_reg;
    u32                  tmp_u32;

    tmp_u32 = (((u32)(slecr_reg->swid & 0xff)) << 24) |
              (((u32)(slecr_reg->independent_learning & 0x1)) << 3) |
              (((u32)(slecr_reg->roaming_enable & 0x1)) << 2);
    SX_PUT_REG_FIELD(inbox, tmp_u32, SLECR_DWORD0_OFFSET);

    return 0;
}

static int __SLECR_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_slecr_reg *slecr_reg = (struct ku_slecr_reg*)ku_reg;
    u8                  *outbox = tlv_markers->reg_tlv->reg_data;
    u32                  tmp_u32;

    SX_GET_REG_FIELD(tmp_u32, outbox, SLECR_DWORD0_OFFSET);
    slecr_reg->swid = (tmp_u32 >> 24) & 0xff;
    slecr_reg->independent_learning = (tmp_u32 >> 3) & 0x1;
    slecr_reg->roaming_enable = (tmp_u32 >> 2) & 0x1;

    return 0;
}

int sx_ACCESS_REG_SLECR(struct sx_dev *dev, struct ku_access_slecr_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __SLECR_encode,
                                  __SLECR_decode,
                                  &reg_data->slecr_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_SLECR);

/************************************************
 * SPVC
 ***********************************************/
#define SPVC_LOCAL_PORT_OFFSET 0x01
#define SPVC_LP_MSB_OFFSET     0x02
#define SPVC_INFO_OFFSET       0x09
#define SPVC_REG_LEN_IN_BYTES  (0xc)

static int __SPVC_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_spvc_reg   *spvc_reg = (struct ku_spvc_reg*)ku_reg;
    sxd_emad_index_tlv_t *index_tlv;
    u8                   *reg_tlv;
    u8                    tmp_u8;
    int                   i;

    if (flags & SX_EMAD_F_INDEXATION_TLV) {
        index_tlv = (sxd_emad_index_tlv_t*)inbox;
        reg_tlv = inbox + sizeof(sxd_emad_index_tlv_t);
        sxd_emad_index_tlv_port_single_build(index_tlv, spvc_reg->lp_msb, spvc_reg->local_port);
        *bytes_written += sizeof(sxd_emad_index_tlv_t);
    } else {
        reg_tlv = inbox;
        SX_PUT_REG_FIELD(reg_tlv, spvc_reg->local_port, SPVC_LOCAL_PORT_OFFSET);
        tmp_u8 = spvc_reg->lp_msb << 4;
        SX_PUT_REG_FIELD(reg_tlv, tmp_u8, SPVC_LP_MSB_OFFSET);
    }

    for (i = 0; i < 3; i++) {
        tmp_u8 = ((spvc_reg->inner_et[i] & 0x1) << 1) |
                 (spvc_reg->et[i] & 0x1);
        SX_PUT_REG_FIELD(reg_tlv, tmp_u8, (SPVC_INFO_OFFSET + 2 - i));
    }

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + SPVC_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + SPVC_REG_LEN_IN_BYTES;
    return 0;
}

static int __SPVC_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_spvc_reg *spvc_reg = (struct ku_spvc_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u8                  tmp_u8;
    int                 i;

    if (tlv_markers->index_tlv) {
        sxd_emad_index_tlv_port_single_deparse(tlv_markers->index_tlv,
                                               &spvc_reg->lp_msb,
                                               &spvc_reg->local_port);
    } else {
        SX_GET_REG_FIELD(spvc_reg->local_port, outbox, SPVC_LOCAL_PORT_OFFSET);
        SX_GET_REG_FIELD(tmp_u8, outbox, SPVC_LP_MSB_OFFSET);
        spvc_reg->lp_msb = (tmp_u8 >> 4) & 0x3;
    }

    for (i = 0; i < 3; i++) {
        SX_GET_REG_FIELD(tmp_u8, outbox, (SPVC_INFO_OFFSET + 2 - i));
        spvc_reg->inner_et[i] = (tmp_u8 >> 1) & 0x1;
        spvc_reg->et[i] = tmp_u8 & 0x1;
    }

    return 0;
}

int sx_ACCESS_REG_SPVC(struct sx_dev *dev, struct ku_access_spvc_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __SPVC_encode,
                                  __SPVC_decode,
                                  &reg_data->spvc_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_SPVC);

/************************************************
 * MOPIR_V2
 ***********************************************/
#define MOPIR_V2_DWORD0_OFFSET    0x00
#define MOPIR_V2_DWORD1_OFFSET    0x04
#define MOPIR_V2_PACKET_OFFSET    0x50
#define MOPIR_V2_REG_LEN_IN_BYTES (0x100)

static int __MOPIR_V2_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_mopir_v2_reg *mopir_v2_reg = (struct ku_mopir_v2_reg*)ku_reg;
    u8                     *reg_tlv = inbox;
    u32                     tmp_u32;

    tmp_u32 = ((u32)(1 << 24)) /* num_pckt=1 */ |
              ((u32)(1 << 20)) /* protocol=Ethernet */ |
              (((u32)(mopir_v2_reg->is_control & 0x1)) << 18) |
              (mopir_v2_reg->dst_port & 0xffff);
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, MOPIR_V2_DWORD0_OFFSET);

    tmp_u32 = (mopir_v2_reg->packet_length & 0x3fff);
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, MOPIR_V2_DWORD1_OFFSET);

    memcpy(inbox + MOPIR_V2_PACKET_OFFSET, mopir_v2_reg->packet, mopir_v2_reg->packet_length);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + MOPIR_V2_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + MOPIR_V2_REG_LEN_IN_BYTES;
    return 0;
}

static int __MOPIR_V2_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    /* nothing to do here */
    return 0;
}

int sx_ACCESS_REG_MOPIR_V2(struct sx_dev *dev, struct ku_access_mopir_v2_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __MOPIR_V2_encode,
                                  __MOPIR_V2_decode,
                                  &reg_data->mopir_v2_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_MOPIR_V2);

/************************************************
 * MOPR
 ***********************************************/
#define MOPR_DWORD0_OFFSET    0x00
#define MOPR_DWORD1_OFFSET    0x04
#define MOPR_CQE_V2_OFFSET    0x10
#define MOPR_PACKET_OFFSET    0x50
#define MOPR_REG_LEN_IN_BYTES (0x100)

static int __MOPR_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_mopr_reg *mopr_reg = (struct ku_mopr_reg*)ku_reg;
    u8                 *reg_tlv = inbox;
    u32                 tmp_u32;

    tmp_u32 = (((u32)(mopr_reg->op & 0x7)) << 24) | 13;  /* buffer_id=tester (13)*/
    SX_PUT_REG_FIELD(reg_tlv, tmp_u32, MOPR_DWORD0_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + MOPR_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + MOPR_REG_LEN_IN_BYTES;
    return 0;
}

static int __MOPR_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_mopr_reg *mopr_reg = (struct ku_mopr_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    u32                 tmp_u32;

    SX_GET_REG_FIELD(tmp_u32, outbox, MOPR_DWORD0_OFFSET);
    mopr_reg->op = (tmp_u32 >> 24) & 0x7;

    SX_GET_REG_FIELD(tmp_u32, outbox, MOPR_DWORD1_OFFSET);
    mopr_reg->packet_length = tmp_u32 & 0x3fff;

    if (mopr_reg->packet != NULL) {
        memcpy(mopr_reg->cqe_v2, outbox + MOPR_CQE_V2_OFFSET, sizeof(mopr_reg->cqe_v2));
        memcpy(mopr_reg->packet, outbox + MOPR_PACKET_OFFSET, mopr_reg->packet_length);
    }

    return 0;
}

int sx_ACCESS_REG_MOPR(struct sx_dev *dev, struct ku_access_mopr_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __MOPR_encode,
                                  __MOPR_decode,
                                  &reg_data->mopr_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_MOPR);

/************************************************
 * RDPM
 ***********************************************/
#define RDPM_REG_LEN_IN_BYTES (0x40)

static int __RDPM_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_rdpm_reg *rdpm_reg = (struct ku_rdpm_reg*)ku_reg;
    u8                 *reg_tlv = inbox;
    u8                 *dscp;
    int                 i;

    for (dscp = inbox, i = 63; i >= 0; i--, dscp++) {
        *dscp = ((rdpm_reg->dscp_update[i] & 0x1) << 7) |
                ((rdpm_reg->color[i] & 0x3) << 5) |
                (rdpm_reg->priority[i] & 0xf);
    }

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + RDPM_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + RDPM_REG_LEN_IN_BYTES;
    return 0;
}

static int __RDPM_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_rdpm_reg *rdpm_reg = (struct ku_rdpm_reg*)ku_reg;
    u8                 *outbox = tlv_markers->reg_tlv->reg_data;
    const u8           *dscp;
    int                 i;

    for (dscp = outbox, i = 63; i >= 0; i--, dscp++) {
        rdpm_reg->dscp_update[i] = ((*dscp) >> 7) & 0x1;
        rdpm_reg->color[i] = ((*dscp) >> 5) & 0x3;
        rdpm_reg->priority[i] = (*dscp) & 0xf;
    }

    return 0;
}

int sx_ACCESS_REG_RDPM(struct sx_dev *dev, struct ku_access_rdpm_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __RDPM_encode,
                                  __RDPM_decode,
                                  &reg_data->rdpm_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_RDPM);

/************************************************
 * MNVDA_NV_SWITCH_CONF
 ***********************************************/
#define NV_SWITCH_CONF_VALID_HEADER_TYPE_OFFSET 0x00
#define NV_SWITCH_CONF_LENGTH_OFFSET            0x03
#define NV_SWITCH_CONF_TYPE_CLASS_INDEX_OFFSET  0x04
#define NV_SWITCH_CONF_CRC16_OFFSET             0x0a
#define NV_SWITCH_CONF_DAS_OFFSET               0x0C
#define NV_SWITCH_CONF_IS_NVLINK_OFFSET         0x0D
#define NV_SWITCH_CONF_SPLIT_MODE_OFFSET        0x0F
#define NV_SWITCH_CONF_REG_LEN_IN_BYTES         (0x8)

static int __NV_SWITCH_CONF_encode(u8 *inbox, void *ku_reg, void *context, u32 flags, u32 *bytes_written)
{
    struct ku_mnvda_nv_switch_conf_reg *nv_switch_conf_reg = (struct ku_mnvda_nv_switch_conf_reg*)ku_reg;
    u8                                 *reg_tlv = inbox;
    u8                                  das = (nv_switch_conf_reg->nv_switch_conf.das) << 4;
    u8                                  valid = nv_switch_conf_reg->valid << 6;

    SX_PUT_REG_FIELD(reg_tlv, valid, NV_SWITCH_CONF_VALID_HEADER_TYPE_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, nv_switch_conf_reg->length, NV_SWITCH_CONF_LENGTH_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, nv_switch_conf_reg->type_class_index, NV_SWITCH_CONF_TYPE_CLASS_INDEX_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, nv_switch_conf_reg->crc16, NV_SWITCH_CONF_CRC16_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, das, NV_SWITCH_CONF_DAS_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, nv_switch_conf_reg->nv_switch_conf.is_nvlink, NV_SWITCH_CONF_IS_NVLINK_OFFSET);
    SX_PUT_REG_FIELD(reg_tlv, nv_switch_conf_reg->nv_switch_conf.split_mode, NV_SWITCH_CONF_SPLIT_MODE_OFFSET);

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + NV_SWITCH_CONF_REG_LEN_IN_BYTES);
    *bytes_written += sizeof(sxd_emad_reg_tlv_t) + NV_SWITCH_CONF_REG_LEN_IN_BYTES;
    return 0;
}

static int __NV_SWITCH_CONF_decode(sxd_emad_tlv_markers_t *tlv_markers, void *ku_reg, void *context)
{
    struct ku_mnvda_nv_switch_conf_reg *nv_switch_conf_reg = (struct ku_mnvda_nv_switch_conf_reg*)ku_reg;
    u8                                 *outbox = tlv_markers->reg_tlv->reg_data;
    u8                                  valid;
    u8                                  das;

    SX_GET_REG_FIELD(valid, outbox, NV_SWITCH_CONF_VALID_HEADER_TYPE_OFFSET);
    nv_switch_conf_reg->valid = valid >> 6;
    SX_GET_REG_FIELD(nv_switch_conf_reg->length, outbox, NV_SWITCH_CONF_LENGTH_OFFSET);
    SX_GET_REG_FIELD(nv_switch_conf_reg->type_class_index, outbox, NV_SWITCH_CONF_TYPE_CLASS_INDEX_OFFSET);
    SX_GET_REG_FIELD(nv_switch_conf_reg->crc16, outbox, NV_SWITCH_CONF_CRC16_OFFSET);
    SX_GET_REG_FIELD(das, outbox, NV_SWITCH_CONF_DAS_OFFSET);
    nv_switch_conf_reg->nv_switch_conf.das = das >> 4;
    SX_GET_REG_FIELD(nv_switch_conf_reg->nv_switch_conf.is_nvlink, outbox, NV_SWITCH_CONF_IS_NVLINK_OFFSET);
    SX_GET_REG_FIELD(nv_switch_conf_reg->nv_switch_conf.split_mode, outbox, NV_SWITCH_CONF_SPLIT_MODE_OFFSET);

    return 0;
}

int sx_ACCESS_REG_MNVDA_NV_SWITCH_CONF(struct sx_dev *dev, struct ku_access_mnvda_nv_switch_conf_reg *reg_data)
{
    return sx_ACCESS_REG_internal(dev,
                                  reg_data->dev_id,
                                  0,
                                  &reg_data->op_tlv,
                                  __NV_SWITCH_CONF_encode,
                                  __NV_SWITCH_CONF_decode,
                                  &reg_data->mnvda_nv_switch_conf_reg,
                                  NULL);
}
EXPORT_SYMBOL(sx_ACCESS_REG_MNVDA_NV_SWITCH_CONF);

/************************************************
 * RAW_BUFF
 ***********************************************/

int sx_ACCESS_REG_RAW_BUFF(struct sx_dev *dev, struct ku_access_reg_raw_buff *raw_data)
{
    struct sx_cmd_mailbox *in_mailbox;
    struct sx_cmd_mailbox *out_mailbox;
    u32                   *inbox;
    u32                   *outbox;
    int                    err;

    if (SX_MAILBOX_SIZE < raw_data->raw_buff.size) {
        sx_warn(dev, "Cannot send the raw register access request "
                "since the mailbox size is too small\n");
        return -ENOMEM;
    }

    in_mailbox = sx_alloc_cmd_mailbox(dev, raw_data->dev_id);
    if (IS_ERR(in_mailbox)) {
        return PTR_ERR(in_mailbox);
    }

    out_mailbox = sx_alloc_cmd_mailbox(dev, raw_data->dev_id);
    if (IS_ERR(out_mailbox)) {
        err = PTR_ERR(out_mailbox);
        goto out_free;
    }

    inbox = in_mailbox->buf;
    memset(inbox, 0, SX_MAILBOX_SIZE);
    outbox = out_mailbox->buf;

    err = copy_from_user(inbox, raw_data->raw_buff.buff,
                         raw_data->raw_buff.size);
    if (err) {
        goto out;
    }

    err = sx_cmd_box(dev, raw_data->dev_id, in_mailbox, out_mailbox, 0, 0,
                     SX_CMD_ACCESS_REG, SX_CMD_TIME_CLASS_A,
                     raw_data->raw_buff.size, 0);
    if (err) {
        goto out;
    }

    err = copy_to_user(raw_data->raw_buff.buff, outbox,
                       raw_data->raw_buff.size);

out:
    sx_free_cmd_mailbox(dev, out_mailbox);
out_free:
    sx_free_cmd_mailbox(dev, in_mailbox);
    return err;
}
