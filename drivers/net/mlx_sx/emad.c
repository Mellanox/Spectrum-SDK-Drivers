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

#include <linux/module.h>
#include <linux/delay.h>

#include <linux/mlx_sx/kernel_user.h>
#include <linux/mlx_sx/skb_hook.h>
#include <linux/mlx_sx/driver.h>
#include <linux/mlx_sx/ku_emad.h>
#include <linux/mlx_sx/auto_registers/cmd_auto.h>
#include <linux/mlx_sx/auto_registers/reg.h>
#include <linux/mlx_sx/cmd.h>

#include "sx.h"
#include "emad.h"
#include "dev_db.h"
#include "sx_dbg_dump_proc.h"
#include "health_check.h"

extern int rx_dump;
extern int tx_dump;

#define SX_EMAD_TID_PREFIX   (0xc0ffee00)
#define SX_EMAD_TIMEOUT_MSEC (20000) /* 20 seconds */
#define SX_EMAD_TX_SKB_SIZE  ((12 * 1024) - ISX_HDR_SIZE)

typedef u64 sx_emad_tid_t;

typedef enum {
    SX_EMAD_TR_STATE_ALLOCATED,
    SX_EMAD_TR_STATE_IN_TX_QUEUE,
    SX_EMAD_TR_STATE_RX_RECEIVED,
} sx_emad_transaction_state_e;

struct sx_emad_completion_status {
    bool timedout; /* has transaction timed out? */

    /* following data is valid only if transaction has not timed out */
    sxd_emad_status_e fw_status;
    const char       *fw_error; /* valid only when fw_status is not OK and string_tlv is supported by the device */
    u32               fw_latency; /* valid only if latency_tlv is supported by the device */
};

static struct kmem_cache *__emad_transaction_cache = NULL;

typedef void (*sx_emad_reg_access_completion_t)(struct sx_dev                          *dev,
                                                const struct sx_emad_completion_status *emad_comp_status,
                                                sxd_emad_tlv_markers_t                 *tlv_markers,
                                                void                                   *context);

struct sx_emad_transaction {
    struct sx_dev                   *dev;
    struct list_head                 list;    /* transaction list */
    sxd_emad_method_e                method;
    sx_emad_tid_t                    tid;
    u16                              reg_id;
    const struct sx_emad_in_params  *in_params;
    struct sx_emad_completion_status status;
    sx_emad_reg_access_completion_t  completion_cb;
    void                            *completion_cb_context;
    struct sk_buff                  *skb_tx;
    emad_prio_e                      priority;
    sx_emad_transaction_state_e      tr_state;
    unsigned long                    since;    /* jiffies */
};

struct sx_sync_emad_context {
    bool                       timedout;
    struct completion          comp;
    struct sx_emad_out_params *out_params;
};

struct sx_sync_emad_read_context {
    struct sx_sync_emad_context sync_ctx;
    access_reg_decode_cb_t      decode_cb;
    void                       *ku_reg;
};

int fw_status_to_err(u32 fw_status)
{
    switch (fw_status) {
    case SXD_EMAD_STATUS_OK_E:
        return 0;

    case SXD_EMAD_STATUS_BUSY_E:
    case SXD_EMAD_STATUS_ISSU_ONGOING_E:
    case SXD_EMAD_STATUS_RESOURCE_NOT_AVAILABLE_E:
        return -EBUSY;

    case SXD_EMAD_STATUS_NOT_SUPP_REG_E:
    case SXD_EMAD_STATUS_NOT_SUPP_CLASS_E:
    case SXD_EMAD_STATUS_NOT_SUPP_METHOD_E:
        return -ENOTSUPP;

    case SXD_EMAD_STATUS_BAD_PARAM_E:
    case SXD_EMAD_STATUS_BAD_CONFIG_E:
        return -EINVAL;

    case SXD_EMAD_STATUS_BAD_BASE_VER_E:
    case SXD_EMAD_STATUS_BAD_TLV_E:
    case SXD_EMAD_STATUS_REG_LENGTH_TOO_SMALL_E:
    case SXD_EMAD_STATUS_RETRY_E:
        return -EPROTO;

    case SXD_EMAD_STATUS_PROFILE_NO_MEMORY_E:
        return -ENOMEM;

    case SXD_EMAD_STATUS_LONG_PROCESS_E:
        return -ETIME;

    case SXD_EMAD_STATUS_INTERNAL_ERR_E:
    case SXD_EMAD_STATUS_PERIPHERAL_ERR_E:
    case SXD_EMAD_STATUS_ERASE_EXCEEDED_E:
    case SXD_EMAD_STATUS_ERROR_E:
    case SXD_EMAD_STATUS_DROP_E:
        return -EIO;

    default:
        return -EIO;
    }

    return -EIO;
}

static struct sx_emad_transaction * __alloc_emad_transaction(void)
{
    return (struct sx_emad_transaction*)kmem_cache_zalloc(__emad_transaction_cache, GFP_KERNEL);
}

static void __dealloc_emad_transaction(struct sx_emad_transaction *emad_tr)
{
    kmem_cache_free(__emad_transaction_cache, emad_tr);
}

static void __complete_emad_transaction(struct sx_emad_transaction *emad_tr, sxd_emad_tlv_markers_t *tlv_markers)
{
    if (emad_tr->completion_cb) {
        emad_tr->completion_cb(emad_tr->dev,
                               &emad_tr->status,
                               tlv_markers,
                               emad_tr->completion_cb_context);
    }

    __dealloc_emad_transaction(emad_tr);
}

static bool __emad_transaction_in_range(sx_emad_tid_t tid, sx_emad_tid_t head_tid, sx_emad_tid_t tail_tid)
{
    /*
     * this function tries to figure out if a certain transaction ID 'tid' exists in a list of
     * in-flight transaction IDs. the first tid in the list is 'head_tid'. the last tid in the list
     * is 'tail_tid'.
     *
     * the most common and simple case is [head_tid <= tid <= tail_tid].
     *
     * the corner case is when we have a wrap-around in the list:
     * [head_tid, ..., MAX_TID_VALUE, ZERO_TID_VALUE, ..., tail_tid]
     * the tid is in this list if [head_tid <= tid <= MAX_TID_VALUE] or [ZERO_TID_VALUE <= tid <= tail_tid]
     * where ZERO_TID_VALUE=(0x<__emad_tid_base> | 0x00000000), MAX_TID_VALUE=(0x<__emad_tid_base> | 0xffffffff)
     */

    if (head_tid <= tail_tid) {
        /* most of the time, head_tid <= tail_tid */
        return (tid >= head_tid && tid <= tail_tid);
    }

    /* if we're here (tid wrap-around), head_tid >= tail_tid */
    return (tid >= head_tid || tid <= tail_tid);
}

/* must be called when transaction list is locked! */
static struct sx_emad_transaction * __tid_lookup(struct sx_priv *priv, sx_emad_tid_t tid)
{
    struct sx_emad_transaction *head_tr, *tail_tr, *emad_tr, *tmp;

    if (list_empty(&priv->emad.transactions_list)) {
        sxd_log_err(SXD_PRIV_DEV(priv),
                    "EMAD TID 0x%llx is unsolicited: there is no in-flight transaction (dev_id=%u)\n",
                    tid,
                    priv->dev.device_id);
        return NULL;
    }

    head_tr = list_first_entry(&priv->emad.transactions_list, struct sx_emad_transaction, list);
    tail_tr = list_last_entry(&priv->emad.transactions_list, struct sx_emad_transaction, list);

    if (!__emad_transaction_in_range(tid, head_tr->tid, tail_tr->tid)) {
        sxd_log_err(SXD_PRIV_DEV(priv),
                    "EMAD TID 0x%llx is unsolicited: transaction is out of in-flight range "
                    "(dev_id=%u, head_tid=0x%llx, tail_tid=0x%llx)\n",
                    tid,
                    priv->dev.device_id,
                    head_tr->tid,
                    tail_tr->tid);
        return NULL;
    }

    list_for_each_entry_safe(emad_tr, tmp, &priv->emad.transactions_list, list) {
        list_del(&emad_tr->list);

        emad_tr->tr_state = SX_EMAD_TR_STATE_RX_RECEIVED;

        /* we expect that requested tid will be at the beginning of the tid-list.
         * in this case, we just change the state of the transaction and go out.
         */
        if (emad_tr->tid == tid) {
            return emad_tr;
        }

        /* if we get here, it means that we have at least one unexpected transaction(s) in the
         * tid-list that comes before the expected one. it can be one of these cases:
         * 1) FW replies come in a different order than the requests we've sent (which is not good).
         * 2) user is working in transaction mode (no one waits for EMAD replies) and one or more
         *    of the requests did not get any reply for some reason.
         *
         * anyhow, we're going to ignore this reply and put an error for this.
         * */
        sxd_log_err(SXD_PRIV_DEV(priv), "EMAD transaction reply is out-of-order! "
                    "(curr_tid=0x%llx, expected_tid (head_tid)=0x%llx, tail_tid=0x%llx "
                    "reg_id=0x%x, dev_id=%u, method=%u)\n",
                    tid,
                    emad_tr->tid,
                    tail_tr->tid,
                    emad_tr->reg_id,
                    priv->dev.device_id,
                    emad_tr->method);

        /* if no one waits for the transaction (sync_event is NULL), let's deallocate the buffer.
         * if the main thread waited for this transaction and already timed out, the main thread
         * is responsible for deallocating the transaction.
         */
        emad_tr->status.timedout = true;
        __complete_emad_transaction(emad_tr, NULL);
    }

    return NULL;
}

static void __emad_rx(struct completion_info *ci, void *context)
{
    struct sx_priv             *priv = sx_priv((struct sx_dev*)context);
    struct sk_buff             *skb = ci->skb;
    struct sx_emad_transaction *emad_tr;
    sxd_emad_tlv_markers_t      tlv_markers;
    sx_emad_tid_t               tid;
    int                         err;

    if (rx_dump) {
        print_hex_dump_debug("RX-EMAD    ", DUMP_PREFIX_OFFSET, 16, 1, skb->data, skb->len, 0);
    }

    err = SXD_EMAD_TLV_MARKERS_DEFAULT_GET(skb->data, skb->len, &tlv_markers);
    if (err) {
        return;
    }

    tid = be64_to_cpu(tlv_markers.operation_tlv->tid);

    spin_lock_bh(&priv->emad.transactions_lock);
    emad_tr = __tid_lookup(priv, tid);
    spin_unlock_bh(&priv->emad.transactions_lock);

    if (!emad_tr) { /* transaction was not found ... */
        return;
    }

    emad_tr->status.timedout = false;
    emad_tr->status.fw_status = tlv_markers.operation_tlv->dr_status & 0x7f;
    emad_tr->status.fw_error = tlv_markers.string_tlv ? tlv_markers.string_tlv->string : NULL;
    emad_tr->status.fw_latency = tlv_markers.latency_tlv ? be32_to_cpu(tlv_markers.latency_tlv->latency_time) : 0;

    if ((emad_tr->status.fw_status != SXD_EMAD_STATUS_OK_E) && !emad_tr->in_params->silent) {
        sxd_log_err(emad_tr->dev,
                    "EMAD transaction FW error (tid=0x%llx, reg_id=0x%x, dev_id=%u, method=%u, priority=%u, fw_status=%u, fw_cause='%s')\n",
                    emad_tr->tid,
                    emad_tr->reg_id,
                    emad_tr->dev->device_id,
                    emad_tr->method,
                    emad_tr->priority,
                    emad_tr->status.fw_status,
                    ((emad_tr->status.fw_error) ? emad_tr->status.fw_error : "N/A"));
        print_hex_dump(KERN_NOTICE, "", DUMP_PREFIX_OFFSET, 16, 1, skb->data, skb->len, 0);
    }

    __complete_emad_transaction(emad_tr, &tlv_markers);
}

/* must be called within device's EMAD transactions lock */
static sx_emad_tid_t __create_transaction_id(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);
    sx_emad_tid_t   tid;

    /*
     * EMAD Transaction ID (64bit):
     *
     *       6         5         4         3         2         1
     *    3210987654321098765432109876543210987654321098765432109876543210
     *    |                               |       |
     *    |                               |       +--------- bits 00-23 (24bits) --> running counter
     *    |                               |                                          (no problem of wrap-around)
     *    |                               +----------------- bits 24-31 (8bits)  --> device-id
     *    +------------------------------------------------- bits 32-63 (32bits) --> SX_EMAD_TID_PREFIX
     */

    priv->emad.tid_lsb++;

    tid = ((u64)SX_EMAD_TID_PREFIX) << 32 |
          (((u32)dev->device_id) << 24) |
          (priv->emad.tid_lsb & 0xffffff);

    return tid;
}

static struct sk_buff * __alloc_emad_skb(gfp_t gfp)
{
    struct sk_buff *skb;
    unsigned int    headroom;

    skb = alloc_skb(SX_EMAD_TX_SKB_SIZE, gfp);
    if (!skb) {
        return NULL;
    }

    headroom = skb_headroom(skb);
    if (headroom < ISX_HDR_SIZE) {
        skb_reserve(skb, ISX_HDR_SIZE - headroom);
    }


    skb_put(skb, SX_EMAD_TX_SKB_SIZE - ISX_HDR_SIZE);
    return skb;
}

static u8 * __build_emad_skb_common(struct sk_buff *skb, sxd_emad_method_e method, u16 reg_id, u64 tid, u32 flags)
{
    u8 *curr_hdr = skb->data; /* @skb->data is now where EMAD Ethernet header should be */

    sxd_emad_eth_header_build(curr_hdr);
    curr_hdr += sizeof(sxd_emad_eth_header_t);

    sxd_emad_operation_tlv_build((sxd_emad_operation_tlv_t*)curr_hdr, method, reg_id, tid);
    curr_hdr += sizeof(sxd_emad_operation_tlv_t);

    if (likely(flags & SX_EMAD_F_STRING_TLV)) {
        sxd_emad_tlv_build(curr_hdr, SXD_EMAD_TLV_TYPE_STRING_E, sizeof(sxd_emad_string_tlv_t));
        curr_hdr += sizeof(sxd_emad_string_tlv_t);
    }

    if (likely(flags & SX_EMAD_F_LATECNY_TLV)) {
        sxd_emad_tlv_build(curr_hdr, SXD_EMAD_TLV_TYPE_LATENCY_E, sizeof(sxd_emad_latency_tlv_t));
        curr_hdr += sizeof(sxd_emad_latency_tlv_t);
    }

    return curr_hdr;
}

static void __build_emad_skb_raw(struct sk_buff   *skb,
                                 sxd_emad_method_e method,
                                 u16               reg_id,
                                 u64               tid,
                                 u32               flags,
                                 const void       *raw_reg_buff,
                                 u32               raw_reg_buff_size)
{
    sxd_emad_reg_tlv_t *reg_tlv;
    u8                 *end_tlv;
    u32                 total_frame_size;

    reg_tlv = (sxd_emad_reg_tlv_t*)__build_emad_skb_common(skb, method, reg_id, tid, flags);
    total_frame_size = ((u8*)reg_tlv) - skb->data;

    sxd_emad_tlv_build(reg_tlv, SXD_EMAD_TLV_TYPE_REG_E, sizeof(sxd_emad_reg_tlv_t) + raw_reg_buff_size);
    total_frame_size += sizeof(sxd_emad_reg_tlv_t) + raw_reg_buff_size;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0))
    memcpy(reg_tlv->reg_data, raw_reg_buff, raw_reg_buff_size);
#else
    memcpy(reg_tlv->reg_data, raw_reg_buff, flex_array_size(reg_tlv, reg_data, raw_reg_buff_size));
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0)) */

    end_tlv = ((u8*)reg_tlv) + sxd_emad_tlv_len_get(reg_tlv);
    sxd_emad_tlv_build(end_tlv, SXD_EMAD_TLV_TYPE_END_E, sizeof(sxd_emad_end_tlv_t));
    total_frame_size += sizeof(sxd_emad_end_tlv_t);

    __skb_trim(skb, total_frame_size);
}

static void __build_emad_skb_ku_reg(struct sk_buff        *skb,
                                    sxd_emad_method_e      method,
                                    u16                    reg_id,
                                    u64                    tid,
                                    u32                    flags,
                                    void                  *ku_reg,
                                    access_reg_encode_cb_t encode_cb)
{
    u32 total_frame_size;
    u32 encoder_bytes = 0;
    u8 *encoder_buff;

    encoder_buff = __build_emad_skb_common(skb, method, reg_id, tid, flags);
    total_frame_size = encoder_buff - skb->data;

    encode_cb(encoder_buff, ku_reg, NULL, flags, &encoder_bytes);
    encoder_buff += encoder_bytes;
    total_frame_size += encoder_bytes;

    sxd_emad_tlv_build(encoder_buff, SXD_EMAD_TLV_TYPE_END_E, sizeof(sxd_emad_end_tlv_t));
    total_frame_size += sizeof(sxd_emad_end_tlv_t);

    __skb_trim(skb, total_frame_size);
}

static void __init_meta(sxd_dev_id_t dev_id, struct isx_meta *meta)
{
    memset(meta, 0, sizeof(*meta));

    meta->dev_id = dev_id;
    meta->etclass = 6; /* EMAD ETCLASS */
    meta->rdq = 0x1f;
    meta->lp = 1;
    meta->type = SX_PKT_TYPE_EMAD_CTL;
}

int sx_emad_build(sxd_dev_id_t      dev_id,
                  const u8         *reg_buff,
                  u32               reg_buff_len,
                  struct sk_buff  **skb,
                  struct isx_meta  *meta,
                  u32               tid_high,
                  u32               tid_low,
                  gfp_t             gfp,
                  u16               reg_id,
                  sxd_emad_method_e method)
{
    u64 tid = (((u64)tid_high) << 32) | tid_low;

    *skb = __alloc_emad_skb(gfp);
    if (!(*skb)) {
        return -ENOMEM;
    }

    __init_meta(dev_id, meta);
    __build_emad_skb_raw(*skb, method, reg_id, tid, 0, reg_buff, reg_buff_len);
    return 0;
}

static struct sx_emad_transaction * __create_emad_transaction(struct sx_dev                  *dev,
                                                              emad_prio_e                     priority,
                                                              sxd_emad_method_e               method,
                                                              u16                             reg_id,
                                                              const struct sx_emad_in_params *in_params,
                                                              sx_emad_reg_access_completion_t completion_cb,
                                                              void                           *completion_cb_context,
                                                              struct sk_buff                 *skb)
{
    struct sx_emad_transaction *emad_tr;

    emad_tr = __alloc_emad_transaction();
    if (!emad_tr) {
        goto out;
    }

    emad_tr->dev = dev;
    emad_tr->method = method;
    emad_tr->reg_id = reg_id;
    emad_tr->tid = 0; /* will be filled later */
    INIT_LIST_HEAD(&emad_tr->list);
    emad_tr->skb_tx = skb;
    emad_tr->in_params = in_params;
    emad_tr->completion_cb = completion_cb;
    emad_tr->completion_cb_context = completion_cb_context;
    emad_tr->priority = priority;
    emad_tr->tr_state = SX_EMAD_TR_STATE_ALLOCATED;
    emad_tr->since = jiffies;

out:
    return emad_tr;
}

static void __delete_emad_transaction(struct sx_emad_transaction *emad_tr)
{
    list_del(&emad_tr->list);
    __dealloc_emad_transaction(emad_tr);
}

static int __send_emad(struct sx_dev *dev, struct sx_emad_transaction *emad_tr)
{
    struct sx_priv           *priv = sx_priv(dev);
    struct isx_meta           meta;
    struct sk_buff           *skb = emad_tr->skb_tx;
    sxd_emad_operation_tlv_t *op_tlv = (sxd_emad_operation_tlv_t*)(skb->data + sizeof(sxd_emad_eth_header_t));
    int                       err;

    __init_meta(dev->device_id, &meta);

    spin_lock_bh(&priv->emad.transactions_lock);

    emad_tr->tid = __create_transaction_id(dev);
    op_tlv->tid = cpu_to_be64(emad_tr->tid);

    list_add_tail(&emad_tr->list, &priv->emad.transactions_list);

    if (tx_dump) {
        print_hex_dump_debug("TX-EMAD    ", DUMP_PREFIX_OFFSET, 16, 1, skb->data, skb->len, 0);
    }

    err = sx_qos_emad_push_to_queue(dev, skb, &meta, emad_tr->priority);
    if (err) {
        kfree_skb(emad_tr->skb_tx);
        __delete_emad_transaction(emad_tr);
    } else {
        emad_tr->tr_state = SX_EMAD_TR_STATE_IN_TX_QUEUE;
    }

    spin_unlock_bh(&priv->emad.transactions_lock);

    return err;
}

static
struct sx_emad_transaction * __emad_reg_send(struct sx_dev                  *dev,
                                             emad_prio_e                     priority,
                                             sxd_emad_method_e               method,
                                             u16                             reg_id,
                                             void                           *ku_reg,
                                             const struct sx_emad_in_params *in_params,
                                             access_reg_encode_cb_t          encode_cb,
                                             sx_emad_reg_access_completion_t completion_cb,
                                             void                           *completion_cb_context)
{
    struct sx_priv             *priv = sx_priv(dev);
    struct sx_emad_transaction *emad_tr = NULL;
    struct sk_buff             *skb;
    int                         err;

    skb = __alloc_emad_skb(GFP_KERNEL);
    if (!skb) {
        goto out;
    }

    __build_emad_skb_ku_reg(skb,
                            method,
                            reg_id,
                            0,
                            priv->emad.flags,
                            ku_reg,
                            encode_cb);

    emad_tr = __create_emad_transaction(dev,
                                        priority,
                                        method,
                                        reg_id,
                                        in_params,
                                        completion_cb,
                                        completion_cb_context,
                                        skb);
    if (!emad_tr) {
        kfree_skb(skb);
        goto out;
    }

    err = __send_emad(dev, emad_tr);
    if (err) { /* EMAD transaction and skb are already freed in this case */
        emad_tr = NULL;
        goto out;
    }

out:
    return emad_tr;
}

static int __sync_wait(struct sx_priv              *priv,
                       struct sx_emad_transaction  *emad_tr,
                       struct sx_sync_emad_context *ctx,
                       u32                          timeout_usec)
{
    bool should_wait;
    long wait_ret;
    int  err = 0;

    do {
        wait_ret = wait_for_completion_interruptible_timeout(&ctx->comp, msecs_to_jiffies(timeout_usec / 1000));
    } while (wait_ret == -ERESTARTSYS);

    if (wait_ret > 0) { /* RX flow signaled us */
        /* if context timed-out, it means that a reply for later transaction has arrived */
        err = (ctx->timedout) ? -ETIMEDOUT : fw_status_to_err(ctx->out_params->fw_status);
        goto out;
    }

    /* if we got here, the transaction has timed out! */
    err = -ETIMEDOUT;

    spin_lock_bh(&priv->emad.transactions_lock);
    should_wait = (emad_tr->tr_state == SX_EMAD_TR_STATE_RX_RECEIVED);

    sxd_log_err(SXD_PRIV_DEV(priv), "EMAD TIMEOUT [tid:%llx, method=%u, reg_id=%04x]\n",
                emad_tr->tid,
                emad_tr->method,
                emad_tr->reg_id);

    sx_health_report_error_emad_timeout(&priv->dev, emad_tr->reg_id, timeout_usec, "SXD_driver");

    if (!should_wait) {
        __delete_emad_transaction(emad_tr);
    }

    spin_unlock_bh(&priv->emad.transactions_lock);

    /* we might get into a situation where the timeout expired for an EMAD transaction but the
     * reply from FW received a very-short time after.
     * in this case, we MUST wait for it to complete because RX thread uses some data that
     * was allocated on our own thread's stack (the completion context).
     * we will wait on the very same completion object that was timed-out.
     */

    if (should_wait) {
        do {
            wait_ret = wait_for_completion_interruptible(&ctx->comp);
        } while (wait_ret == -ERESTARTSYS);
    }

out:
    return err;
}

static void __sync_complete(const struct sx_emad_completion_status *emad_comp_status, struct sx_sync_emad_context *ctx)
{
    ctx->timedout = emad_comp_status->timedout;
    if (!emad_comp_status->timedout) {
        ctx->out_params->fw_status = emad_comp_status->fw_status;
        ctx->out_params->fw_latency = emad_comp_status->fw_latency;
    }

    complete(&ctx->comp);
}

static void __sync_init(struct sx_emad_out_params *out_params, u16 reg_id, struct sx_sync_emad_context *ctx)
{
    memset(ctx, 0, sizeof(*ctx));

    init_completion(&ctx->comp);
    ctx->out_params = out_params;
    ctx->out_params->reg_id = reg_id;
}

static void __emad_write_completion(struct sx_dev                          *dev,
                                    const struct sx_emad_completion_status *emad_comp_status,
                                    sxd_emad_tlv_markers_t                 *tlv_markers,
                                    void                                   *context)
{
    struct sx_sync_emad_context *ctx = (struct sx_sync_emad_context*)context;

    __sync_complete(emad_comp_status, ctx);
}

static int __sx_emad_reg_write_sync(struct sx_dev                  *dev,
                                    emad_prio_e                     priority,
                                    u16                             reg_id,
                                    void                           *ku_reg,
                                    const struct sx_emad_in_params *in_params,
                                    struct sx_emad_out_params      *out_params,
                                    access_reg_encode_cb_t          encode_cb,
                                    u32                             timeout_usec)
{
    struct sx_priv             *priv = sx_priv(dev);
    struct sx_emad_transaction *emad_tr;
    struct sx_sync_emad_context ctx;
    int                         err = 0;

    __sync_init(out_params, reg_id, &ctx);

    emad_tr = __emad_reg_send(dev,
                              priority,
                              SXD_EMAD_METHOD_WRITE_E,
                              reg_id,
                              ku_reg,
                              in_params,
                              encode_cb,
                              __emad_write_completion,
                              &ctx);

    if (!emad_tr) {
        err = -ENOMEM;
        goto out;
    }

    err = __sync_wait(priv, emad_tr, &ctx, timeout_usec);

out:
    return err;
}

static void __emad_read_completion(struct sx_dev                          *dev,
                                   const struct sx_emad_completion_status *emad_comp_status,
                                   sxd_emad_tlv_markers_t                 *tlv_markers,
                                   void                                   *context)
{
    struct sx_sync_emad_read_context *ctx = (struct sx_sync_emad_read_context*)context;

    if (!emad_comp_status->timedout && (emad_comp_status->fw_status == SXD_EMAD_STATUS_OK_E)) {
        ctx->decode_cb(tlv_markers, ctx->ku_reg, NULL);
    }

    __sync_complete(emad_comp_status, &ctx->sync_ctx);
}

static int __sx_emad_reg_read_sync(struct sx_dev                  *dev,
                                   emad_prio_e                     priority,
                                   u16                             reg_id,
                                   void                           *ku_reg,
                                   const struct sx_emad_in_params *in_params,
                                   struct sx_emad_out_params      *out_params,
                                   access_reg_encode_cb_t          encode_cb,
                                   access_reg_decode_cb_t          decode_cb,
                                   u32                             timeout_usec)
{
    struct sx_priv                  *priv = sx_priv(dev);
    struct sx_emad_transaction      *emad_tr;
    struct sx_sync_emad_read_context ctx;
    int                              err = 0;

    __sync_init(out_params, reg_id, &ctx.sync_ctx);
    ctx.decode_cb = decode_cb;
    ctx.ku_reg = ku_reg;

    emad_tr = __emad_reg_send(dev,
                              priority,
                              SXD_EMAD_METHOD_QUERY_E,
                              reg_id,
                              ku_reg,
                              in_params,
                              encode_cb,
                              __emad_read_completion,
                              &ctx);

    if (!emad_tr) {
        err = -ENOMEM;
        goto out;
    }

    err = __sync_wait(priv, emad_tr, &ctx.sync_ctx, timeout_usec);

out:
    return err;
}

int sx_emad_access_reg_common(struct sx_dev                  *dev,
                              emad_prio_e                     priority,
                              sxd_emad_method_e               method,
                              u16                             reg_id,
                              void                           *ku_reg,
                              const struct sx_emad_in_params *in_params,
                              struct sx_emad_out_params      *out_params,
                              access_reg_encode_cb_t          encode_cb,
                              access_reg_decode_cb_t          decode_cb)
{
    struct sx_priv           *priv;
    int                       err = 0;
    struct sx_emad_in_params  backup_in_params;
    struct sx_emad_out_params backup_out_params;

    if (!dev || !reg_id || !ku_reg || !encode_cb ||
        ((method != SXD_EMAD_METHOD_WRITE_E) && (method != SXD_EMAD_METHOD_QUERY_E)) ||
        ((method == SXD_EMAD_METHOD_QUERY_E) && !decode_cb)) {
        return -EINVAL;
    }

    priv = sx_priv(dev);
    if (!(priv->emad.flags & SX_EMAD_F_SUPPORTED)) {
        return -EOPNOTSUPP;
    }

    if (!in_params) {
        memset(&backup_in_params, 0, sizeof(backup_in_params));
        in_params = &backup_in_params;
    }

    if (!out_params) {
        memset(&backup_out_params, 0, sizeof(backup_out_params));
        out_params = &backup_out_params;
    }


    if (method == SXD_EMAD_METHOD_WRITE_E) {
        err = __sx_emad_reg_write_sync(dev,
                                       priority,
                                       reg_id,
                                       ku_reg,
                                       in_params,
                                       out_params,
                                       encode_cb,
                                       sx_emad_timeout_get(dev));
    } else { /* method == SXD_EMAD_METHOD_QUERY_E */
        err = __sx_emad_reg_read_sync(dev,
                                      priority,
                                      reg_id,
                                      ku_reg,
                                      in_params,
                                      out_params,
                                      encode_cb,
                                      decode_cb,
                                      sx_emad_timeout_get(dev));
    }

    return err;
}

int sx_emad_dev_init(struct sx_dev *dev)
{
    struct sx_priv           *priv = sx_priv(dev);
    struct ku_access_mgir_reg mgir;
    union ku_filter_critireas crit;
    int                       err = 0;

    INIT_LIST_HEAD(&priv->emad.transactions_list);
    spin_lock_init(&priv->emad.transactions_lock);
    priv->emad.flags = 0;
    priv->emad.tid_lsb = 0;
    priv->emad.timeout_usec = SX_EMAD_TIMEOUT_MSEC * 1000; /* Convert milliseconds to microseconds */

    memset(&mgir, 0, sizeof(mgir));
    mgir.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mgir.op_tlv, MLXSW_MGIR_ID, SXD_EMAD_METHOD_QUERY_E);

    err = sx_ACCESS_REG_MGIR(dev, &mgir);
    if (err) {
        sxd_log_err(dev, "failed to initialize EMAD on device %u\n", dev->device_id);
        goto out;
    }

    switch (mgir.mgir_reg.hw_info.device_id) {
    case SXD_MGIR_HW_DEV_ID_SWITCH_IB:
    case SXD_MGIR_HW_DEV_ID_SWITCH_IB2:
        sxd_log_notice(dev, "device %d does not support EMAD\n", dev->device_id);
        goto out; /* without error (keep err = 0) */

    default:
        priv->emad.flags |= SX_EMAD_F_SUPPORTED;
        break;
    }

    priv->emad.flags |= (mgir.mgir_reg.fw_info.string_tlv) ? SX_EMAD_F_STRING_TLV : 0;
    priv->emad.flags |= (mgir.mgir_reg.fw_info.latency_tlv) ? SX_EMAD_F_LATECNY_TLV : 0;
    priv->emad.flags |= (mgir.mgir_reg.fw_info.index_tlv) ? SX_EMAD_F_INDEXATION_TLV : 0;

    sxd_log_info(dev, "EMAD is ready on device %u [string_tlv=%s, latency_tlv=%s, index_tlv=%s]\n",
                 dev->device_id,
                 ((mgir.mgir_reg.fw_info.string_tlv) ? "true" : "false"),
                 ((mgir.mgir_reg.fw_info.latency_tlv) ? "true" : "false"),
                 ((mgir.mgir_reg.fw_info.index_tlv) ? "true" : "false"));

    memset(&crit, 0, sizeof(crit));
    crit.eth.emad_tid = SX_EMAD_TID_PREFIX;
    err = sx_core_add_synd(0, /* swid */
                           SXD_TRAP_ID_GENERAL_ETH_EMAD,
                           L2_TYPE_ETH,
                           "EMAD (driver)",
                           0, /* is default? */
                           &crit,
                           "sx-core EMAD RX handler",
                           __emad_rx,
                           dev,
                           CHECK_DUP_ENABLED_E,
                           dev,
                           NULL,
                           1 /* is register? */);
    if (err) {
        sxd_log_err(dev, "failed to register EMAD handler on device %u\n", dev->device_id);
        goto out;
    }

out:
    return err;
}

void sx_emad_dev_deinit(struct sx_dev *dev)
{
    struct sx_priv           *priv = sx_priv(dev);
    union ku_filter_critireas crit;
    bool                      busy;
    int                       err = 0;
    unsigned long             end_wait = jiffies + 5 * HZ; /* we'll wait up to 5 seconds */

    if (!(priv->emad.flags & SX_EMAD_F_SUPPORTED)) {
        return;
    }

    do {
        spin_lock_bh(&priv->emad.transactions_lock);
        busy = !list_empty(&priv->emad.transactions_list);
        spin_unlock_bh(&priv->emad.transactions_lock);

        if (!busy) {
            sxd_log_info(dev, "device %u does not have pending EMADs\n", dev->device_id);
            break;
        }

        sxd_log_warning(dev, "device %u has pending EMADs, waiting ...\n", dev->device_id);
        msleep(100);
    } while (time_before(jiffies, end_wait));

    memset(&crit, 0, sizeof(crit));
    crit.eth.emad_tid = SX_EMAD_TID_PREFIX;
    err = sx_core_remove_synd(0, /* swid */
                              SXD_TRAP_ID_GENERAL_ETH_EMAD,
                              L2_TYPE_ETH,
                              0, /* is default? */
                              &crit,
                              dev,
                              dev,
                              __emad_rx,
                              NULL,
                              1, /* is register */
                              NULL);
    if (err) {
        sxd_log_err(dev, "failed to unregister EMAD handler on device %u\n", dev->device_id);
    }
}

static const char * __state_to_str(sx_emad_transaction_state_e state)
{
    switch (state) {
    case SX_EMAD_TR_STATE_ALLOCATED:
        return "Allocated";

    case SX_EMAD_TR_STATE_IN_TX_QUEUE:
        return "TX-Queue";

    case SX_EMAD_TR_STATE_RX_RECEIVED:
        return "Received";

    default:
        break;
    }

    return "N/A";
}

static const char * __method_to_str(sxd_emad_method_e method)
{
    switch (method) {
    case SXD_EMAD_METHOD_QUERY_E:
        return "Query";

    case SXD_EMAD_METHOD_WRITE_E:
        return "Write";

    case SXD_EMAD_METHOD_EVENT_E:
        return "Event";

    default:
        break;
    }

    return "N/A";
}

static const char * __prio_to_str(emad_prio_e prio)
{
    switch (prio) {
    case EMAD_PRIO_HIGH:
        return "High";

    case EMAD_PRIO_LOW:
        return "Low";

    default:
        break;
    }

    return "N/A";
}

static int __emad_from_driver_dump(struct seq_file* m, void *v, void* context)
{
    struct sx_priv             *priv = NULL;
    struct sx_emad_transaction *emad_tr;
    struct sx_dev             * dev = sx_dbg_dump_get_device(m);

    if (!dev) {
        return 0;
    }
    priv = sx_priv(dev);
    /* EMAD-from-driver is not supported on non-PCI (OOB) systems */
    if (!dev->pdev) {
        return 0;
    }

    seq_printf(m, "EMAD from driver - Device %u\n", dev->device_id);
    sx_dbg_dump_print_empty_line(m);
    seq_printf(m, "Supports String TLV: %s\n", ((priv->emad.flags & SX_EMAD_F_STRING_TLV) ? "True" : "False"));
    seq_printf(m, "Supports Latency TLV: %s\n", ((priv->emad.flags & SX_EMAD_F_LATECNY_TLV) ? "True" : "False"));
    seq_printf(m, "Supports Index/Multi-Index TLV: %s\n",
               ((priv->emad.flags & SX_EMAD_F_INDEXATION_TLV) ? "True" : "False"));
    sx_dbg_dump_print_empty_line(m);

    seq_printf(m, "%-18s    %-6s    %-6s    %-9s    %-8s    %-15s\n",
               "TID", "RegId", "Method", "State", "Priority", "Msecs-In-Flight");
    sx_dbg_dump_print_separate_line(m);

    spin_lock_bh(&priv->emad.transactions_lock);
    list_for_each_entry(emad_tr, &priv->emad.transactions_list, list) {
        seq_printf(m, "0x%016llx    0x%04x    %-6s    %-9s    %-8s    %-15lu\n",
                   emad_tr->tid,
                   emad_tr->reg_id,
                   __method_to_str(emad_tr->method),
                   __state_to_str(emad_tr->tr_state),
                   __prio_to_str(emad_tr->priority),
                   (unsigned long)jiffies_to_msecs(jiffies - emad_tr->since));
    }
    spin_unlock_bh(&priv->emad.transactions_lock);
    return 0;
}

int sx_emad_init(void)
{
    int err = 0;

    __emad_transaction_cache = kmem_cache_create("emad_transaction", sizeof(struct sx_emad_transaction), 0, 0, NULL);
    if (!__emad_transaction_cache) {
        sxd_log_err(SXD_DEV_NONE, "failed to create EMAD transaction cache\n");
        err = -ENOMEM;
        goto out;
    }

out:
    return err;
}

void sx_emad_deinit(void)
{
    if (__emad_transaction_cache) {
        kmem_cache_destroy(__emad_transaction_cache);
        __emad_transaction_cache = NULL;
    }
}

bool sx_emad_dump_hanlders_per_device_init(struct sx_dev *dev, void *context)
{
    sx_dbg_dump_read_handler_register("emad_from_driver", __emad_from_driver_dump, NULL, dev, NULL);
    return true;
}

bool sx_emad_dump_hanlders_per_device_deinit(struct sx_dev *dev, void *context)
{
    sx_dbg_dump_read_handler_unregister("emad_from_driver", dev);
    return true;
}

/**
 * sx_emad_timeout_get - Get the current EMAD timeout value for a specific device
 * @dev: Pointer to sx_dev structure
 *
 * Returns: EMAD timeout in microseconds (e.g., 20000000 for 20 seconds)
 */
int sx_emad_timeout_get(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);

    return priv->emad.timeout_usec;
}

/**
 * sx_emad_timeout_set - Set the EMAD timeout value for a specific device
 * @dev: Pointer to sx_dev structure
 * @timeout_usec: Timeout in microseconds (e.g., 20000000 for 20 seconds)
 *
 * Returns: 0 on success
 */
int sx_emad_timeout_set(struct sx_dev *dev, u32 timeout_usec)
{
    struct sx_priv *priv = sx_priv(dev);

    priv->emad.timeout_usec = timeout_usec;
    sxd_log_info(dev, "EMAD timeout set to %u usec for device %u\n", timeout_usec,
                 dev->device_id);

    /* Update health check SDQ timeout to stay synchronized with EMAD timeout */
    sx_health_update_sdq_timeout(dev);

    return 0;
}

/**
 * sx_emad_timeout_dbg_cmd_handler - Debug CLI handler for EMAD timeout commands
 * Handles: emad_timeout_set <timeout_usec>
 */
int sx_emad_timeout_dbg_cmd_handler(int argc, const char *argv[], struct sx_dev *dev, void *context)
{
    u32 timeout_usec;
    int err = 0;

    if (argc < 2) {
        sxd_log_notice(dev,
                       "EMAD: invalid number of arguments. Usage: emad_timeout_set <timeout_usec>\n");
        return -EINVAL;
    }

    /* Parse timeout_usec */
    err = kstrtou32(argv[1], 10, &timeout_usec);
    if (err) {
        sxd_log_notice(dev, "EMAD: invalid timeout_usec [%s]\n", argv[1]);
        return -EINVAL;
    }

    /* Set the timeout */
    err = sx_emad_timeout_set(dev, timeout_usec);
    if (err) {
        sxd_log_err(dev, "EMAD: failed to set timeout for device %u\n", dev->device_id);
        return err;
    }

    sxd_log_notice(dev,
                   "EMAD: timeout set successfully for device %u: %u usec\n",
                   dev->device_id,
                   timeout_usec);
    return 0;
}
