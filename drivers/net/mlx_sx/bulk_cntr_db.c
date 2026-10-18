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

#include <linux/slab.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/hashtable.h>
#include <linux/jhash.h>
#include <linux/uaccess.h>

#include "mmap.h"
#include "bulk_cntr_db.h"
#include <linux/mlx_sx/device.h>
#include <linux/mlx_sx/cmd.h>
#include <linux/mlx_sx/auto_registers/cmd_auto.h>
#include <linux/mlx_sx/auto_registers/reg.h>
#include "sx.h"
#include "ioctl_internal.h"
#include "sx_dbg_dump_proc.h"

#ifndef BYTES_IN_MB
#define BYTES_IN_MB (1024 * 1204)
#endif

#define BULK_CNTR_GC_TOLERANCE_SECS (10)

/* HFT is not considered in this max progress limit. */
#define BULK_CNTR_MAX_IN_PROGRESS  (2)
#define BULK_CNTR_HFT_BUF_SIZE_MAX (350)

/* Global Generic Netlink family registry (keyed by family name). */
#define BULK_CNTR_GENL_FAMILY_HASH_BITS (4)

static DEFINE_HASHTABLE(g_bulk_cntr_genl_family_db, BULK_CNTR_GENL_FAMILY_HASH_BITS);
static DEFINE_SPINLOCK(g_bulk_cntr_genl_family_db_lock);

struct transaction_entry {
    struct {
        u32  updates;
        u32  count;
        bool got_mocs_done;                                   /* some of the key types do not wait only
                                                               *  for MOCS_DONE but for SDK approval to release
                                                               *  the bulk-counter transaction. If GC wakes up for
                                                               *  this transaction, at least we know if MOCS_DONE was
                                                               *  received from FW or not. */
    } gc;
    unsigned long            buffer_id;                   /* mmap pointer in user process */
    sxd_bulk_cntr_event_id_t event_id;
    unsigned long            start_time;                   /* jiffies */
    u8                       is_canceled;                   /* is transaction canceled by user */
    bool                     gc_exempt;                     /* This flag would exempt the transaction from the idle watchdog while session
                                                             * is on-going without expected FW events (except final MOCS_DONE event). */
    u32                                        refcnt;
    struct sxd_bulk_cntr_buffer_layout_common *common_layout; /* pointer to shared-memory from driver's perspective */
};
struct transaction_status {
    pid_t                            client_pid; /* user process ID */
    unsigned long                    buffer_id; /* mmap pointer in user process */
    enum sxd_bulk_cntr_done_status_e status; /* status of the transaction */
};

/* HFT work queue specific information.*/
struct sx_hft_session_ctx {
    struct workqueue_struct *hft_wq_p;
    spinlock_t               hft_wq_lock;
    bool                     is_hft_wq_active;
};

static struct kmem_cache *__event_parser_cache_p = NULL;
/* HFT specific context.*/
static struct sx_hft_session_ctx g_hft_session_ctx = {
    .hft_wq_p = NULL,
    .is_hft_wq_active = false,
};

/* should be called in DB lock */
static void __bulk_cntr_db_entry_ref_inc(struct transaction_entry *entry)
{
    entry->refcnt++;
}

/* should be called in DB lock */
static void __bulk_cntr_db_entry_ref_dec(struct transaction_entry *entry)
{
    if (--entry->refcnt == 0) {
        kfree(entry);
    }
}

/* this function gets user process ID + user shared memory pointer and returns the shared memory pointer
 * from driver's perspective */
static struct sxd_bulk_cntr_buffer_layout_common * __get_buffer(struct sx_dev *dev,
                                                                pid_t          client_pid,
                                                                unsigned long  buffer_id,
                                                                bool           is_streaming,
                                                                bool           get_ref)
{
    struct sx_priv *priv = sx_priv(dev);

    if (is_streaming) {
        if (buffer_id >= SXD_BULK_CNTR_STREAMING_BUFFER_ID_MAX_NUM) {
            return NULL;
        }
        return priv->bulk_counters.streaming_buffer_ids[buffer_id].layout;
    }

    return (struct sxd_bulk_cntr_buffer_layout_common*)sx_mmap_user_to_kernel(dev, client_pid, buffer_id, get_ref);
}

static void __bulk_cntr_streaming_try_alloc_buffer_id_buff(struct sx_dev *dev,
                                                           unsigned long  buffer_id,
                                                           bool           alloc_can_sleep)
{
    int ret = 0;

    ret = sx_bulk_cntr_streaming_buffer_id_alloc_and_fill_buff(dev, buffer_id, alloc_can_sleep);
    if (ret) {
        sxd_log_debug(dev,
                      "%s: sx_bulk_cntr_streaming_buffer_id_alloc_and_fill_buff() failed, err:%d\n",
                      __func__,
                      ret);
    }
}

static void __bulk_cntr_streaming_try_release_buffer_id_buff(struct sx_dev *dev, unsigned long buffer_id)
{
    int ret = 0;

    ret = sx_bulk_cntr_streaming_buffer_id_free_buff(dev, buffer_id);
    if (ret) {
        sxd_log_debug(dev,
                      "%s: sx_bulk_cntr_streaming_buffer_id_free_buff() failed, err:%d\n",
                      __func__,
                      ret);
    }
}

static void __init_elephant_layout(struct sxd_bulk_cntr_buffer_layout_elephant *layout_elephant_p,
                                   struct sxd_bulk_cntr_params_elephant        *elephant_params_p)
{
    int total_ports = 0, i = 0, j = 0;

    memcpy(&layout_elephant_p->port_mask, &elephant_params_p->port_mask,
           sizeof(layout_elephant_p->port_mask));

    for (i = 0; i < SXD_COS_ELEPHANT_PORT_MASK_NUM; i++) {
        for (j = 0; j < 32; j++) {
            if (layout_elephant_p->port_mask[i] & (1 << j)) {
                total_ports++;
            }
        }
    }

    memset(&layout_elephant_p->port_flows, 0, sizeof(layout_elephant_p->port_flows));
    memset(&layout_elephant_p->data, 0, sizeof(layout_elephant_p->data[0]) * total_ports *
           SXD_COS_ELEPHANT_FLOW_ID_NUM_MAX);
}
/* kernel cache functions */

void bulk_cntr_db_event_cache_delete(void)
{
    if (__event_parser_cache_p != NULL) {
        kmem_cache_destroy(__event_parser_cache_p);
    }
}

int bulk_cntr_db_event_cache_create(void)
{
    int err = 0;

    __event_parser_cache_p = kmem_cache_create("event_parser_cache", SXD_EMAD_BUFFER_MAX_SIZE, 0, 0, NULL);
    if (__event_parser_cache_p == NULL) {
        sxd_log_err(SXD_DEV_NONE, "Failed to allocate memory cache for bulk counter event cache\n");
        err = -ENOMEM;
    }

    return err;
}

void * bulk_cntr_db_event_record_allocate(void)
{
    void *reg_p = NULL;

    if (__event_parser_cache_p == NULL) {
        sxd_log_err(SXD_DEV_NONE, "Bulk counter event cache not created\n");
        goto out;
    }

    reg_p = kmem_cache_alloc(__event_parser_cache_p, GFP_ATOMIC);
    if (reg_p == NULL) {
        sxd_log_err(SXD_DEV_NONE, "Failed to allocate event record\n");
        goto out;
    }

    memset(reg_p, 0, SXD_EMAD_BUFFER_MAX_SIZE);
out:
    return reg_p;
}


void bulk_cntr_db_event_record_free(void *reg_p)
{
    kmem_cache_free(__event_parser_cache_p, reg_p);
}

static void __bulk_cntr_db_reset_entry_gc_internal(struct transaction_entry *entry, bool fw_update, bool mocs_done)
{
    entry->gc.count = 0; /* reset GC count (tell GC that this entry is not idle) */
    if (fw_update) {
        entry->gc.updates++; /* debug information if GC is mad at this entry */
    }
    if (mocs_done) {
        entry->gc.got_mocs_done = true;
    }
}

void bulk_cntr_db_reset_entry_gc(struct sx_dev                  *dev,
                                 const sxd_bulk_cntr_event_id_t *event_id_p,
                                 bool                            fw_update,
                                 bool                            mocs_done)
{
    struct sx_priv               *priv = sx_priv(dev);
    enum sxd_bulk_cntr_key_type_e type = (enum sxd_bulk_cntr_key_type_e)event_id_p->event_id_fields.type;
    struct transaction_entry     *entry;

    if (type >= SXD_BULK_CNTR_KEY_TYPE_LAST_E) {
        WARN_ON_ONCE(1);
        return;
    }

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    entry = priv->bulk_counters.in_progress[type];
    if (entry && (entry->event_id.event_id_value == event_id_p->event_id_value)) {
        __bulk_cntr_db_reset_entry_gc_internal(entry, fw_update, mocs_done);
    }
    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
}

/* add a new bulk-counter transaction */
int bulk_cntr_db_add(struct sx_dev* dev, struct ku_bulk_cntr_transaction_add *bulk_cntr_tr_add)
{
    struct sxd_bulk_cntr_buffer_layout_common         *layout_common;
    struct sxd_bulk_cntr_buffer_layout_flow           *layout_flow;
    struct sxd_bulk_cntr_buffer_layout_shared_buffer  *layout_shared_buffer;
    struct sxd_bulk_cntr_buffer_layout_headroom       *layout_headroom;
    struct sxd_bulk_cntr_buffer_layout_port           *layout_port;
    struct sxd_bulk_cntr_buffer_layout_elephant       *layout_elephant;
    struct sxd_bulk_cntr_buffer_layout_stateful_db    *layout_stateful_db;
    struct sxd_bulk_cntr_buffer_layout_flow_estimator *layout_flow_estimator;
    struct sxd_bulk_cntr_buffer_layout_hft            *layout_hft;
    struct sxd_bulk_cntr_buffer_layout_arft           *layout_arft;
    struct sxd_bulk_cntr_buffer_layout_trim           *layout_trim;
    struct transaction_entry                          *entry;
    int                                                i = 0, j = 0;
    int                                                err = 0;
    enum sxd_bulk_cntr_key_type_e                      type = bulk_cntr_tr_add->event_id.event_id_fields.type;
    uint16_t                                           max_packet_size = 0;
    struct sx_priv                                    *priv = sx_priv(dev);
    uint32_t                                           profile_id = 0;
    bool                                               is_streaming = bulk_cntr_tr_add->is_streaming_buffer;
    sxd_bulk_cntr_streaming_profile_attrs_t           *streaming_profiles = priv->bulk_counters.streaming_profiles;
    sxd_bulk_cntr_streaming_buffer_id_attrs_t         *streaming_buffer_ids =
        priv->bulk_counters.streaming_buffer_ids;

    if ((unsigned int)type >= SXD_BULK_CNTR_KEY_TYPE_LAST_E) {
        return -EINVAL;
    }
    if (is_streaming && (bulk_cntr_tr_add->buffer_id >= SXD_BULK_CNTR_STREAMING_BUFFER_ID_MAX_NUM)) {
        return -EINVAL;
    }

    sxd_log_debug(dev, "DBG: %s: type %d, is_streaming: %d.\n", __func__, type, is_streaming);

    /* so far only HFT supports bulk counter streaming mode. */
    if ((is_streaming) && (type != SXD_BULK_CNTR_KEY_TYPE_HFT_E)) {
        sxd_log_err(dev,
                    "%s: type %d does not support bulk counter streaming mode, (err=%d)\n",
                    __func__,
                    type,
                    err);
        return -EOPNOTSUPP;
    }

    /* buffer is retrieved here with reference count because if user process that allocated this
     * buffer suddenly dies, this pointer becomes stale and the rest of the flow here is dangerous.
     * we don't release the reference until the transaction is done or an error happened */
    layout_common = __get_buffer(dev,
                                 bulk_cntr_tr_add->event_id.event_id_fields.client_id,
                                 bulk_cntr_tr_add->buffer_id,
                                 is_streaming,
                                 true);
    if (is_streaming) {
        profile_id = streaming_buffer_ids[bulk_cntr_tr_add->buffer_id].profile_id;
    }

    if (!layout_common) {
        sxd_log_err(dev, "Bulk-Counter DB add - invalid buffer_id\n");
        return -EINVAL;
    }
    layout_common->is_streaming = is_streaming;
    sxd_log_debug(dev,
                  "DBG: %s: profile_id %u, buffer_id: %lu.\n",
                  __func__,
                  profile_id,
                  bulk_cntr_tr_add->buffer_id);

    if ((type == SXD_BULK_CNTR_KEY_TYPE_PORT_EXT_E) && (layout_common->type == SXD_BULK_CNTR_KEY_TYPE_PORT_E)) {
        /* Update the layout_common type to SXD_BULK_CNTR_KEY_TYPE_PORT_EXT_E */
        layout_common->type = SXD_BULK_CNTR_KEY_TYPE_PORT_EXT_E;
    }

    if (type != SXD_BULK_CNTR_KEY_TYPE_ACCUFLOW_E) {
        if (type != layout_common->type) {
            sxd_log_err(dev, "Bulk-Counter DB add - type mismatch\n");
            return -EINVAL;
        }
    }

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

    /* Only one HFT session is allowed at a time and it should not be accounted for max session in progress.
     * Two different bulk session types along with HFT is allowed to execute at the same time.*/
    if (type == SXD_BULK_CNTR_KEY_TYPE_HFT_E) {
        if (priv->bulk_counters.in_progress[type] != NULL) {
            err = -EBUSY;
            goto out;
        }
    } else if ((priv->bulk_counters.in_progress[type] != NULL) ||
               (priv->bulk_counters.in_progress_cnt == BULK_CNTR_MAX_IN_PROGRESS)) {
        err = -EBUSY;
        goto out;
    }

    entry = kzalloc(sizeof(struct transaction_entry), GFP_ATOMIC);
    if (!entry) {
        err = -ENOMEM;
        goto out;
    }

    layout_common->counters_received_so_far = 0; /* reset the numbers received for the new transaction */
    switch (type) {
    case SXD_BULK_CNTR_KEY_TYPE_ACCUFLOW_E:
    /* fall-through */
    case SXD_BULK_CNTR_KEY_TYPE_FLOW_E:
        layout_flow = (struct sxd_bulk_cntr_buffer_layout_flow*)layout_common;
        layout_flow->base_counter_id = bulk_cntr_tr_add->params.flow_params.base_counter_id;
        break;

    case SXD_BULK_CNTR_KEY_TYPE_FLOW_ESTIMATOR_E:
        layout_flow_estimator = (struct sxd_bulk_cntr_buffer_layout_flow_estimator*)layout_common;
        layout_flow_estimator->base_counter_id = bulk_cntr_tr_add->params.flow_estimator_params.base_counter_id;
        break;

    case SXD_BULK_CNTR_KEY_TYPE_SHARED_BUFFER_E:
        layout_shared_buffer = (struct sxd_bulk_cntr_buffer_layout_shared_buffer*)layout_common;
        layout_common->num_of_counters = bulk_cntr_tr_add->params.shared_buffer_params.num_of_counters;
        layout_shared_buffer->port_idx = 0;
        layout_shared_buffer->pg_tc_sp_idx = 0;
        layout_shared_buffer->port_pool_idx = 0;
        layout_shared_buffer->pool_idx = 0;
        layout_shared_buffer->last_type = SXD_BULK_CNTR_SB_TYPE_INVALID;
        layout_shared_buffer->mc_pool_id = bulk_cntr_tr_add->params.shared_buffer_params.mc_pool_id;
        memcpy(layout_shared_buffer->port_mask, bulk_cntr_tr_add->params.shared_buffer_params.port_mask,
               sizeof(uint32_t) * SXD_BULK_CNTR_PORT_MASK_NUM);
        memcpy(layout_shared_buffer->sw_pool_id, bulk_cntr_tr_add->params.shared_buffer_params.sw_pool_id,
               sizeof(uint32_t) * SXD_BULK_CNTR_TOTAL_POOL_MAX_NUM);
        memset(layout_shared_buffer->port, 0, sizeof(sxd_bulk_cntr_shared_buffer_port_t) * SXD_BULK_CNTR_PORT_NUM);
        for (i = 0; i < SXD_BULK_CNTR_PORT_NUM; i++) {
            for (j = 0; j < SXD_BULK_CNTR_POOL_NUM; j++) {
                layout_shared_buffer->port[i].rx_per_pool[j].sw_pool_id = SXD_BULK_CNTR_POOL_ID_INVALID;
                layout_shared_buffer->port[i].tx_per_pool[j].sw_pool_id = SXD_BULK_CNTR_POOL_ID_INVALID;
                layout_shared_buffer->port[i].rx_per_pool_desc[j].sw_pool_id = SXD_BULK_CNTR_POOL_ID_INVALID;
                layout_shared_buffer->port[i].tx_per_pool_desc[j].sw_pool_id = SXD_BULK_CNTR_POOL_ID_INVALID;
            }
        }
        memset(layout_shared_buffer->mc_port, 0, sizeof(sxd_bulk_cntr_shared_buffer_mc_port_t) *
               SXD_BULK_CNTR_PORT_NUM);
        memset(&layout_shared_buffer->mc_switch_prio, 0, sizeof(sxd_bulk_cntr_shared_buffer_mc_sp_t));
        memset(&layout_shared_buffer->pool, 0, sizeof(sxd_bulk_cntr_shared_buffer_pool_t));
        for (i = 0; i < SXD_BULK_CNTR_TOTAL_POOL_MAX_NUM; i++) {
            layout_shared_buffer->pool.statistics[i].sw_pool_id = SXD_BULK_CNTR_POOL_ID_INVALID;
        }

        break;

    case SXD_BULK_CNTR_KEY_TYPE_HEADROOM_E:
        layout_headroom = (struct sxd_bulk_cntr_buffer_layout_headroom*)layout_common;
        layout_common->num_of_counters = bulk_cntr_tr_add->params.headroom_params.num_of_counters;
        memcpy(layout_headroom->port_width, bulk_cntr_tr_add->params.headroom_params.port_width,
               sizeof(uint8_t) * SXD_BULK_CNTR_PORT_NUM);
        memcpy(layout_headroom->port_mask, bulk_cntr_tr_add->params.headroom_params.port_mask,
               sizeof(uint32_t) * SXD_BULK_CNTR_PORT_MASK_NUM);
        memset(layout_headroom->headroom, 0, sizeof(sxd_bulk_cntr_headroom_port_t) * SXD_BULK_CNTR_PORT_NUM);
        break;

    case SXD_BULK_CNTR_KEY_TYPE_PORT_E:
    /* fall-through */
    case SXD_BULK_CNTR_KEY_TYPE_PORT_EXT_E:
        layout_common->num_of_counters = bulk_cntr_tr_add->params.port_params.number_of_counters;
        layout_port = (struct sxd_bulk_cntr_buffer_layout_port*)layout_common;
        memcpy(&(layout_port->mappings),
               &(bulk_cntr_tr_add->params.port_params.mappings),
               sizeof(layout_port->mappings));
        memset(layout_port->counters, 0, sizeof(layout_port->counters[0]) * (layout_port->counters_size));
        break;

    case SXD_BULK_CNTR_KEY_TYPE_ELEPHANT_E:
        layout_elephant = (struct sxd_bulk_cntr_buffer_layout_elephant *)layout_common;
        __init_elephant_layout(layout_elephant, &bulk_cntr_tr_add->params.elephant_params);
        break;

    case SXD_BULK_CNTR_KEY_TYPE_STATEFUL_DB_E:
        layout_stateful_db = (struct sxd_bulk_cntr_buffer_layout_stateful_db *)layout_common;

        layout_stateful_db->meta_layout.number_of_entries = 0;
        layout_stateful_db->meta_layout.number_of_entries_translated = 0;
        memset(layout_stateful_db->data, 0,
               sizeof(layout_stateful_db->data[0]) * (bulk_cntr_tr_add->params.stateful_db_params.entries_num_max));

        break;

    case SXD_BULK_CNTR_KEY_TYPE_HFT_E:
        err = sx_core_get_rdq_param_max(dev, NULL, &max_packet_size);
        if (err) {
            sxd_log_err(dev, "could not get RDQ attributes (err=%d)\n", err);
            goto out;
        }
        layout_hft = (struct sxd_bulk_cntr_buffer_layout_hft*)layout_common;
        layout_hft->samples_expected =
            is_streaming ? streaming_profiles[profile_id].attrs.format_data.format_data_ipfix.bulk_size
                                                    : bulk_cntr_tr_add->params.hft_params.samples_expected;
        layout_hft->sample_buffer_size = bulk_cntr_tr_add->params.hft_params.sample_buffer_size;
        memcpy(&layout_hft->metadata_map,
               &bulk_cntr_tr_add->params.hft_params.metadata_map,
               sizeof(layout_hft->metadata_map));
        memcpy(&layout_hft->port_map,
               &bulk_cntr_tr_add->params.hft_params.port_map,
               sizeof(layout_hft->port_map));
        memcpy(&layout_hft->global_map,
               &bulk_cntr_tr_add->params.hft_params.global_map,
               sizeof(layout_hft->global_map));
        layout_hft->samples_received = 0;
        layout_hft->buffer_idx = 0;
        layout_hft->buffer_id = bulk_cntr_tr_add->buffer_id;
        layout_hft->moftd_max_entries = SXD_MAX_HFT_ENTRIES_SINGLE_EMAD(max_packet_size);
        layout_hft->moftd_reg_buf = kmalloc(max_packet_size, GFP_ATOMIC);
        if (layout_hft->moftd_reg_buf == NULL) {
            err = -ENOMEM;
            sxd_log_err(dev, "%s: failed to allocate memory for MOFTD buffer.\n",
                        __func__);
            goto out;
        }
        break;

    case SXD_BULK_CNTR_KEY_TYPE_AR_FLOW_E:
        layout_arft = (struct sxd_bulk_cntr_buffer_layout_arft *)layout_common;
        layout_arft->meta_layout.trap_status.status = SXD_BULK_CNTR_ARFT_TRAP_STATUS_OK_E;
        layout_arft->meta_layout.trap_status.sample_error = 0;
        layout_arft->meta_layout.trap_status.table_error = 0;
        /* Init per transaction metadata in the case user re-initiates
         *  the mocs transaction and does not allocate a new buffer */
        layout_arft->meta_layout.number_of_entries_received = 0;
        layout_arft->meta_layout.number_of_samples_received = 0;
        layout_arft->meta_layout.number_of_tables_received = 0;
        break;

    case SXD_BULK_CNTR_KEY_TYPE_TRIM_E:
        /* TRIM: mapping key is in mmap (trim_params); clear counter slots for this transaction. */
        layout_trim = (struct sxd_bulk_cntr_buffer_layout_trim *)layout_common;
        memset(layout_trim->counters,
               0,
               (size_t)layout_common->num_of_counters * sizeof(layout_trim->counters[0]));
        break;

    default:
        break;
    }

    /* save transaction data */
    entry->buffer_id = bulk_cntr_tr_add->buffer_id;
    entry->event_id = bulk_cntr_tr_add->event_id;
    entry->is_canceled = 0;
    entry->start_time = jiffies;
    entry->refcnt = 1;
    entry->common_layout = layout_common;
    /* Exempt streaming-to-network HFT from the idle watchdog: it has no per-sample host events
     * (no MOFTD trap), so gc.count would not be checked while session is on-going. */
    entry->gc_exempt = (is_streaming &&
                        (streaming_profiles[profile_id].attrs.protocol_type ==
                         SXD_BULK_CNTR_STREAMING_PROFILE_PROTOCOL_TYPE_UDP_TO_NETWORK));

    priv->bulk_counters.in_progress[type] = entry;

    /* hft session doesn't affect other bulk counter session */
    if (type != SXD_BULK_CNTR_KEY_TYPE_HFT_E) {
        priv->bulk_counters.in_progress_cnt++;
    }

    if (is_streaming) {
        __bulk_cntr_streaming_try_alloc_buffer_id_buff(dev, bulk_cntr_tr_add->buffer_id, false);
    }

out:
    if (err && !is_streaming) {
        /* on error we need to decrease reference from the buffer we hold */
        sx_mmap_ref_dec(dev,
                        bulk_cntr_tr_add->event_id.event_id_fields.client_id,
                        bulk_cntr_tr_add->buffer_id);
    }

    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    return err;
}

/* common delete (called from DEL/COMPLETE) of bulk-counter transaction from DB */
static void __bulk_cntr_db_del(struct sx_priv *priv, uint32_t type, struct transaction_entry *entry)
{
    u32 duration;

    priv->bulk_counters.in_progress[type] = NULL;

    /* Do it only for non HFT session as firmware support bulk session and hft. */
    if (type != SXD_BULK_CNTR_KEY_TYPE_HFT_E) {
        priv->bulk_counters.in_progress_cnt--;
    }

    duration = jiffies_to_msecs(jiffies - entry->start_time);
    if (duration > priv->bulk_counters.top_durations[type].duration) {
        priv->bulk_counters.top_durations[type].event_id = entry->event_id;
        priv->bulk_counters.top_durations[type].start_time = entry->start_time;
        priv->bulk_counters.top_durations[type].duration = duration;
    }

    __bulk_cntr_db_entry_ref_dec(entry);
}

/* must be called under lock */
static int __bulk_cntr_db_complete_internal(struct sx_dev                    *dev,
                                            struct transaction_entry         *entry,
                                            enum sxd_bulk_cntr_key_type_e     type,
                                            enum sxd_bulk_cntr_done_status_e *status)
{
    sxd_bulk_cntr_buffer_layout_stateful_db_t *layout_stateful_p = NULL;
    sxd_bulk_cntr_buffer_layout_hft_t         *layout_hft_p = NULL;
    sxd_bulk_cntr_buffer_layout_arft_t        *layout_arft_p = NULL;
    struct sx_priv                            *priv = sx_priv(dev);

    if (entry->common_layout->is_streaming && (type != SXD_BULK_CNTR_KEY_TYPE_HFT_E)) {
        sxd_log_err(dev, "type %d does not support bulk counter streaming mode\n", type);
        return -EOPNOTSUPP;
    }

    /* determine the status of the transaction to be reported to the user */
    if (entry->is_canceled) {
        *status = SXD_BULK_CNTR_DONE_STATUS_CANCELED;
    } else if ((entry->common_layout->num_of_counters != entry->common_layout->counters_received_so_far) &&
               (type != SXD_BULK_CNTR_KEY_TYPE_STATEFUL_DB_E) && (type != SXD_BULK_CNTR_KEY_TYPE_AR_FLOW_E) &&
               (type != SXD_BULK_CNTR_KEY_TYPE_TRIM_E)) {
        *status = SXD_BULK_CNTR_DONE_STATUS_ERROR;
        sxd_log_notice(dev, "type %d expected number of counters %d does not match number of counters received %d\n",
                       type,
                       entry->common_layout->num_of_counters,
                       entry->common_layout->counters_received_so_far);
    } else if (type == SXD_BULK_CNTR_KEY_TYPE_STATEFUL_DB_E) {
        layout_stateful_p = (struct sxd_bulk_cntr_buffer_layout_stateful_db *)entry->common_layout;
        if (layout_stateful_p->meta_layout.number_of_entries !=
            layout_stateful_p->meta_layout.number_of_entries_translated) {
            *status = SXD_BULK_CNTR_DONE_STATUS_ERROR;
        } else {
            *status = SXD_BULK_CNTR_DONE_STATUS_OK;
        }
    } else if (type == SXD_BULK_CNTR_KEY_TYPE_AR_FLOW_E) {
        layout_arft_p = (struct sxd_bulk_cntr_buffer_layout_arft *)entry->common_layout;
        if (layout_arft_p->meta_layout.trap_status.status != SXD_BULK_CNTR_ARFT_TRAP_STATUS_OK_E) {
            *status = SXD_BULK_CNTR_DONE_STATUS_ERROR;
        } else {
            *status = SXD_BULK_CNTR_DONE_STATUS_OK;
        }
    } else {
        *status = SXD_BULK_CNTR_DONE_STATUS_OK;
    }

    if (type == SXD_BULK_CNTR_KEY_TYPE_HFT_E) {
        layout_hft_p = (sxd_bulk_cntr_buffer_layout_hft_t*)entry->common_layout;
        if (layout_hft_p->moftd_reg_buf != NULL) {
            kfree(layout_hft_p->moftd_reg_buf);
            layout_hft_p->moftd_reg_buf = NULL;
        }
    }

    /* delete the transaction from DB. */
    __bulk_cntr_db_del(priv, type, entry);
    return 0;
}

static void __bulk_cntr_db_post_complete(struct sx_dev *dev,
                                         pid_t          client_id,
                                         unsigned long  buffer_id,
                                         bool           is_streaming)
{
    if (is_streaming) {
        __bulk_cntr_streaming_try_release_buffer_id_buff(dev, buffer_id);
    } else {
        /* Decrement reference to shared memory we hold from bulk_cntr_db_add() after lock dependency is removed.
         * so that memory is deallocated if application has exited, after bulk transaction is triggered. */
        sx_mmap_ref_dec(dev, client_id, buffer_id);
    }
}

/* complete a transaction. called in two cases:
 * 1. MOCS_DONE event handled by the driver and no ack from SDK is required.
 * 2. Ack from SDK upon MOCS_DONE event (in cases when SDK has to do
 *    some cleanups for the transaction, e.g. unlock counter range).
 */
int bulk_cntr_db_complete(struct sx_dev                    *dev,
                          enum sxd_bulk_cntr_key_type_e     type,
                          unsigned long                    *buffer_id_p,
                          enum sxd_bulk_cntr_done_status_e *status_p,
                          uint32_t                         *cookie_p)
{
    struct sx_priv           *priv = sx_priv(dev);
    struct transaction_entry *entry = NULL;
    int                       err = 0;
    unsigned long             buffer_id = 0;
    pid_t                     client_id = 0;
    bool                      is_streaming = false;

    if ((type < 0) || (type >= SXD_BULK_CNTR_KEY_TYPE_LAST_E)) {
        return -EINVAL;
    }

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

    /* check that there is an operation of type 'type' in progress */
    entry = priv->bulk_counters.in_progress[type];
    if (!entry) {
        err = -ENOENT;
        goto out;
    }

    buffer_id = entry->buffer_id;
    client_id = entry->event_id.event_id_fields.client_id;
    is_streaming = !!entry->common_layout->is_streaming;

    if (buffer_id_p) {
        *buffer_id_p = buffer_id;
    }

    if (cookie_p) {
        *cookie_p = entry->common_layout->cookie;
    }

    err = __bulk_cntr_db_complete_internal(dev, entry, type, status_p);

out:
    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

    if (!err) {
        __bulk_cntr_db_post_complete(dev, client_id, buffer_id, is_streaming);
    }

    return err;
}

int bulk_cntr_db_finalize(struct sx_dev *dev,
                          pid_t          client_pid,
                          unsigned long  buffer_id,
                          bool           is_streaming,
                          uint32_t       cookie)
{
    struct sx_priv                            *priv = sx_priv(dev);
    struct transaction_entry                  *entry;
    struct sxd_bulk_cntr_buffer_layout_common *layout_common;
    bool                                       transaction_completed = false;

    /* first check if the transaction exists */
    layout_common = __get_buffer(dev, client_pid, buffer_id, is_streaming, false);
    if (!layout_common) {
        /* transaction completed gracefully, no more actions required */
        return 0;
    }

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    entry = priv->bulk_counters.in_progress[layout_common->type];
    if (!entry || (entry->common_layout != layout_common)) {
        /* @entry is no longer there or it is a new one that belongs to another transaction
         * that just started with the same type */
        transaction_completed = true;
    }
    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

    if (transaction_completed) {
        /* transaction completed gracefully, no more actions required */
        return 0;
    }

    sxd_log_notice(dev, "deallocating a bulk-counter buffer while transaction is in progress "
                   "(client_pid=%d, buffer_id=0x%lx, type=%d, cookie=%u)\n",
                   client_pid, buffer_id, layout_common->type, cookie);

    return -EINPROGRESS;
}

/* delete a bulk-counter transaction from DB */
int bulk_cntr_db_del(struct sx_dev *dev, pid_t client_pid, unsigned long buffer_id, bool is_streaming)
{
    struct sxd_bulk_cntr_buffer_layout_common *layout_common;
    struct transaction_entry                  *entry;
    int                                        err = 0;
    enum sxd_bulk_cntr_key_type_e              type = SXD_BULK_CNTR_KEY_TYPE_LAST_E;
    sxd_bulk_cntr_buffer_layout_hft_t         *layout_hft_p = NULL;
    struct sx_priv                            *priv = sx_priv(dev);

    /* we don't need to get reference (send 'false' as last parameter) because this function is called
     * from SDK as a rollback or transaction_add. transaction_add already got reference on this,
     * so the pointer is safe. */
    layout_common = __get_buffer(dev, client_pid, buffer_id, is_streaming, false);
    if (!layout_common) {
        sxd_log_err(dev, "Bulk-Counter DB del - invalid buffer_id\n");
        return -EINVAL;
    }

    type = layout_common->type;
    if (type >= SXD_BULK_CNTR_KEY_TYPE_LAST_E) {
        return -EINVAL;
    }
    /* so far only HFT supports bulk counter streaming mode. */
    if (is_streaming && (type != SXD_BULK_CNTR_KEY_TYPE_HFT_E)) {
        sxd_log_err(dev,
                    "%s: type %d does not support bulk counter streaming mode, (err=%d)\n",
                    __func__,
                    type,
                    err);
        return -EOPNOTSUPP;
    }

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

    /* check if there requested buffer is indeed in progress */
    entry = priv->bulk_counters.in_progress[type];
    if (!entry) {
        err = -ENOENT;
        goto out;
    }

    if (entry->buffer_id != buffer_id) {
        /* If buffer IDs are not equal and the type of buffer is the FLOW COUNTERS type,
         *  we need to check also the ACCUFLOW COUNTERS type as by the design
         *  on the user level there is only the FLOW COUNTERS type. */
        if (type == SXD_BULK_CNTR_KEY_TYPE_FLOW_E) {
            entry = priv->bulk_counters.in_progress[SXD_BULK_CNTR_KEY_TYPE_ACCUFLOW_E];
            if (!entry || (entry->buffer_id != buffer_id)) {
                err = -ENOENT;
                goto out;
            }

            type = SXD_BULK_CNTR_KEY_TYPE_ACCUFLOW_E;
        } else {
            err = -ENOENT;
            sxd_log_err(dev, "buffer id %lu does not match entry->buffer_id %lu yet (err=%d)\n",
                        buffer_id,
                        entry->buffer_id,
                        err);
            goto out;
        }
    }

    /* check if the operation was canceled by the user */
    if (entry->is_canceled) { /* transaction is already canceled, waiting for MOCS_DONE to delete it */
        err = -EBUSY;
        goto out;
    }

    if (type == SXD_BULK_CNTR_KEY_TYPE_HFT_E) {
        layout_hft_p = (sxd_bulk_cntr_buffer_layout_hft_t*)layout_common;
        if (layout_hft_p->moftd_reg_buf != NULL) {
            kfree(layout_hft_p->moftd_reg_buf);
            layout_hft_p->moftd_reg_buf = NULL;
        }
    }

    /* delete the transaction from DB */
    __bulk_cntr_db_del(priv, type, entry);

out:
    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

    if (!err) {
        __bulk_cntr_db_post_complete(dev, client_pid, buffer_id, is_streaming);
    }

    return err;
}

static int __genl_reg_family(struct genl_family *family)
{
    int ret = genl_register_family(family);

    if (ret) {
        switch (ret) {
        case -ENOMEM:
            sxd_log_err(SXD_DEV_NONE, "Failed to register family: Out of memory\n");
            break;

        case -EEXIST:
            sxd_log_err(SXD_DEV_NONE, "Failed to register family: Name conflict\n");
            break;

        case -EINVAL:
            sxd_log_err(SXD_DEV_NONE, "Failed to register family: Invalid parameters\n");
            break;

        case -ENOSPC:
            sxd_log_err(SXD_DEV_NONE, "Failed to register family: Maximum number of families reached\n");
            break;

        default:
            sxd_log_err(SXD_DEV_NONE, "Failed to register family: Unknown error %d\n", ret);
            break;
        }
    }
    return ret;
}

/* get net namespace of current caller (pid) */
static void __get_net_safe(struct net **net_pp)
{
    rcu_read_lock();
    *net_pp = rcu_dereference(current->nsproxy)->net_ns;
    if (*net_pp) {
        get_net(*net_pp);   /* carry a ref out of RCU */
    }
    rcu_read_unlock();
}

static void __put_net_safe(struct net **net_pp)
{
    if (*net_pp) {
        put_net(*net_pp);   /* release the ref */
    }
    *net_pp = NULL;
}

static u32 __genl_family_name_hash(const char *family_name)
{
    return jhash(family_name,
                 strnlen(family_name, SXD_BULK_CNTR_STREAMING_GENETLINK_FAMILY_NAME_LEN),
                 0);
}

static struct sxd_bulk_cntr_genl_family_entry * __genl_family_entry_lookup_locked(const char *family_name)
{
    struct sxd_bulk_cntr_genl_family_entry *entry = NULL;
    u32                                     hash_key = __genl_family_name_hash(family_name);

    hash_for_each_possible(g_bulk_cntr_genl_family_db, entry, hnode, hash_key) {
        if (strncmp(entry->family_name,
                    family_name,
                    SXD_BULK_CNTR_STREAMING_GENETLINK_FAMILY_NAME_LEN) == 0) {
            return entry;
        }
    }

    return NULL;
}

static int __genl_family_entry_mcgrp_cmp(const struct sxd_bulk_cntr_genl_family_entry *entry,
                                         const char                                   *mcgrp_name,
                                         uint8_t                                       mcgrp_flags)
{
    if (strncmp(entry->mcgrp_name,
                mcgrp_name,
                SXD_BULK_CNTR_STREAMING_GENETLINK_MC_GROUP_NAME_LEN) != 0) {
        return -EINVAL;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0))
    if (entry->mcgrp_flags != mcgrp_flags) {
        return -EINVAL;
    }
#else
    (void)mcgrp_flags;
#endif

    return 0;
}

static void __genl_family_entry_fill(struct sxd_bulk_cntr_genl_family_entry      *entry,
                                     const sxd_bulk_cntr_streaming_profile_cfg_t *attrs)
{
    const sxd_bulk_cntr_streaming_profile_protocol_genetlink_t *genl_cfg =
        &attrs->protocol_data.protocol_data_genetlink;

    memset(entry, 0, sizeof(*entry));
    INIT_LIST_HEAD(&entry->holders);
    INIT_LIST_HEAD(&entry->flush_node);

    strncpy(entry->family_name,
            genl_cfg->family_name,
            sizeof(entry->family_name) - 1);
    strncpy(entry->mcgrp_name,
            genl_cfg->mcgrps[0].name,
            sizeof(entry->mcgrp_name) - 1);
    entry->mcgrp_flags = genl_cfg->mcgrps[0].flags;

    strncpy(entry->genl_family.name,
            entry->family_name,
            sizeof(entry->genl_family.name) - 1);
    entry->genl_family.version = attrs->format_data.format_data_ipfix.version;
    /* IPFIX message is handled as raw Generic Netlink payload. */
    entry->genl_family.maxattr = 0;
    entry->genl_family.netnsok = true;
    entry->genl_family.parallel_ops = true;

    strncpy(entry->genl_mcgrps[0].name,
            entry->mcgrp_name,
            sizeof(entry->genl_mcgrps[0].name) - 1);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0))
    entry->genl_mcgrps[0].flags = entry->mcgrp_flags;
#endif
    entry->genl_family.mcgrps = entry->genl_mcgrps;
    entry->genl_family.n_mcgrps = 1;
}

/**
 * Acquire a shared Generic Netlink family registry entry for (dev, profile_id).
 * Register with the kernel only on the 0-to-1 transition. Same family with a
 * different multicast group or MC group flags is rejected with -EINVAL.
 * Concurrent first-time registration or unregister of the same family returns -EBUSY.
 */
static int __genl_family_acquire(struct sx_dev                               *dev,
                                 uint32_t                                     profile_id,
                                 const sxd_bulk_cntr_streaming_profile_cfg_t *attrs,
                                 struct sxd_bulk_cntr_genl_family_entry     **entry_pp)
{
    struct sxd_bulk_cntr_genl_family_entry                     *entry = NULL;
    struct sxd_bulk_cntr_genl_family_entry                     *existing = NULL;
    struct sxd_bulk_cntr_genl_holder                           *holder = NULL;
    const sxd_bulk_cntr_streaming_profile_protocol_genetlink_t *genl_cfg =
        &attrs->protocol_data.protocol_data_genetlink;
    const char *family_name = genl_cfg->family_name;
    const char *mcgrp_name = genl_cfg->mcgrps[0].name;
    uint8_t     mcgrp_flags = genl_cfg->mcgrps[0].flags;
    int         err = 0;
    u32         hash_key = 0;

    if ((family_name[0] == 0) || (mcgrp_name[0] == 0)) {
        err = -EINVAL;
        goto out;
    }

    holder = kzalloc(sizeof(*holder), GFP_KERNEL);
    if (!holder) {
        err = -ENOMEM;
        goto out;
    }
    holder->dev = dev;
    holder->profile_id = profile_id;

    entry = kzalloc(sizeof(*entry), GFP_KERNEL);
    if (!entry) {
        err = -ENOMEM;
        goto out_free_holder;
    }
    __genl_family_entry_fill(entry, attrs);

    spin_lock_bh(&g_bulk_cntr_genl_family_db_lock);
    existing = __genl_family_entry_lookup_locked(family_name);
    if (existing) {
        if (existing->registering || existing->unregistering) {
            err = -EBUSY;
            spin_unlock_bh(&g_bulk_cntr_genl_family_db_lock);
            goto out_free_entry;
        }
        err = __genl_family_entry_mcgrp_cmp(existing, mcgrp_name, mcgrp_flags);
        if (err) {
            if (strncmp(existing->mcgrp_name,
                        mcgrp_name,
                        SXD_BULK_CNTR_STREAMING_GENETLINK_MC_GROUP_NAME_LEN) != 0) {
                sxd_log_err(dev,
                            "%s: family '%s' already registered with different MC group "
                            "(existing='%s', requested='%s')\n",
                            __func__,
                            family_name,
                            existing->mcgrp_name,
                            mcgrp_name);
            } else {
                sxd_log_err(dev,
                            "%s: family '%s' already registered with different MC group flags "
                            "(mcgrp='%s', existing=0x%x, requested=0x%x)\n",
                            __func__,
                            family_name,
                            mcgrp_name,
                            existing->mcgrp_flags,
                            mcgrp_flags);
            }
            spin_unlock_bh(&g_bulk_cntr_genl_family_db_lock);
            goto out_free_entry;
        }
        list_add_tail(&holder->list, &existing->holders);
        existing->refcnt++;
        *entry_pp = existing;
        spin_unlock_bh(&g_bulk_cntr_genl_family_db_lock);
        kfree(entry);
        entry = NULL;
        sxd_log_notice(dev,
                       "Bulk counter streaming profile shares Generic Netlink family %s "
                       "(refcnt=%u, profile_id=%u)\n",
                       existing->family_name,
                       existing->refcnt,
                       profile_id);
        goto out;
    }

    /* First reference: insert as registering so concurrent acquire of the same
     * family returns -EBUSY. Keep the entry local until genl_register_family()
     * succeeds; do not publish through entry_pp while registering is true.
     */
    entry->registering = true;
    entry->refcnt = 1;
    list_add_tail(&holder->list, &entry->holders);
    hash_key = __genl_family_name_hash(entry->family_name);
    hash_add(g_bulk_cntr_genl_family_db, &entry->hnode, hash_key);
    spin_unlock_bh(&g_bulk_cntr_genl_family_db_lock);

    sxd_log_debug(SXD_DEV_NONE,
                  "DBG: Try to register generic Netlink family %s, MC group: %s.\n",
                  entry->genl_family.name,
                  entry->genl_family.mcgrps[0].name);
    err = __genl_reg_family(&entry->genl_family);
    if (err) {
        sxd_log_err(SXD_DEV_NONE,
                    "Failed to register generic Netlink family with ret = %d\n",
                    err);
        spin_lock_bh(&g_bulk_cntr_genl_family_db_lock);
        list_del(&holder->list);
        hash_del(&entry->hnode);
        spin_unlock_bh(&g_bulk_cntr_genl_family_db_lock);
        kfree(holder);
        kfree(entry);
        goto out;
    }

    spin_lock_bh(&g_bulk_cntr_genl_family_db_lock);
    entry->registering = false;
    *entry_pp = entry;
    spin_unlock_bh(&g_bulk_cntr_genl_family_db_lock);

    sxd_log_notice(SXD_DEV_NONE,
                   "Bulk counter streaming profile registers generic Netlink family %s.\n",
                   entry->genl_family.name);
    goto out;

out_free_entry:
    kfree(entry);
out_free_holder:
    kfree(holder);
out:
    return err;
}

/**
 * Release one (dev, profile_id) reference. Unregister and free the entry only
 * when the reference count transitions to 0.
 */
static int __genl_family_release(struct sxd_bulk_cntr_genl_family_entry *entry,
                                 struct sx_dev                          *dev,
                                 uint32_t                                profile_id)
{
    struct sxd_bulk_cntr_genl_holder *holder = NULL;
    struct sxd_bulk_cntr_genl_holder *tmp = NULL;
    struct sxd_bulk_cntr_genl_holder *found = NULL;
    bool                              do_unregister = false;
    char                              family_name[SXD_BULK_CNTR_STREAMING_GENETLINK_FAMILY_NAME_LEN];
    int                               err = 0;

    if (!entry) {
        err = -EINVAL;
        goto out;
    }

    memset(family_name, 0, sizeof(family_name));

    spin_lock_bh(&g_bulk_cntr_genl_family_db_lock);
    if (entry->registering || entry->unregistering) {
        spin_unlock_bh(&g_bulk_cntr_genl_family_db_lock);
        err = -EBUSY;
        sxd_log_err(dev,
                    "%s: family '%s' is still %s, cannot release profile_id %u\n",
                    __func__,
                    entry->family_name,
                    entry->registering ? "registering" : "unregistering",
                    profile_id);
        goto out;
    }
    list_for_each_entry_safe(holder, tmp, &entry->holders, list) {
        if ((holder->dev == dev) && (holder->profile_id == profile_id)) {
            found = holder;
            break;
        }
    }
    if (!found) {
        spin_unlock_bh(&g_bulk_cntr_genl_family_db_lock);
        err = -ENOENT;
        sxd_log_err(dev,
                    "%s: no holder for profile_id %u on family '%s'\n",
                    __func__,
                    profile_id,
                    entry->family_name);
        goto out;
    }
    list_del(&found->list);

    if (entry->refcnt == 0) {
        spin_unlock_bh(&g_bulk_cntr_genl_family_db_lock);
        kfree(found);
        err = -EINVAL;
        sxd_log_err(dev, "%s: refcnt underflow for family '%s'\n", __func__, entry->family_name);
        goto out;
    }
    entry->refcnt--;
    if (entry->refcnt == 0) {
        /* Keep the entry hashed until genl_unregister_family() completes so a
         * concurrent acquire cannot start a new register while the family is
         * still present in the Generic Netlink subsystem.
         */
        entry->unregistering = true;
        strncpy(family_name, entry->family_name, sizeof(family_name) - 1);
        do_unregister = true;
    } else {
        sxd_log_notice(dev,
                       "Bulk counter streaming profile releases Generic Netlink family %s "
                       "(refcnt=%u, profile_id=%u)\n",
                       entry->family_name,
                       entry->refcnt,
                       profile_id);
    }
    spin_unlock_bh(&g_bulk_cntr_genl_family_db_lock);
    kfree(found);

    if (do_unregister) {
        err = genl_unregister_family(&entry->genl_family);
        if (err) {
            sxd_log_err(SXD_DEV_NONE,
                        "Failed to unregister generic Netlink family '%s' with ret = %d, "
                        "keeping it in the registry for reuse\n",
                        family_name,
                        err);
            /* The family is still registered in the Generic Netlink subsystem, so the
             * entry stays hashed. Clear unregistering, otherwise every future acquire
             * of this family name is rejected with -EBUSY for the module lifetime.
             */
            spin_lock_bh(&g_bulk_cntr_genl_family_db_lock);
            entry->unregistering = false;
            spin_unlock_bh(&g_bulk_cntr_genl_family_db_lock);
        } else {
            sxd_log_notice(SXD_DEV_NONE,
                           "Bulk counter streaming profile un-register generic Netlink family %s.\n",
                           family_name);
            spin_lock_bh(&g_bulk_cntr_genl_family_db_lock);
            hash_del(&entry->hnode);
            spin_unlock_bh(&g_bulk_cntr_genl_family_db_lock);
            kfree(entry);
        }
    }

out:
    return err;
}

/**
 * Drop this profile's shared Genl registry reference (via genl_entry_p) and
 * clear the per-device profile slot. Used by explicit delete and all deinit paths.
 */
static int __streaming_profile_clear(struct sx_dev *dev, uint32_t profile_id)
{
    struct sx_priv                          *priv = sx_priv(dev);
    sxd_bulk_cntr_streaming_profile_attrs_t *streaming_profiles = priv->bulk_counters.streaming_profiles;
    struct sxd_bulk_cntr_genl_family_entry  *entry_p = NULL;
    int                                      err = 0;

    if (profile_id >= SXD_BULK_CNTR_STREAMING_PROFILE_MAX_NUM) {
        err = -EINVAL;
        goto out;
    }

    if (!streaming_profiles[profile_id].is_allocated) {
        goto out;
    }

    if (streaming_profiles[profile_id].attrs.protocol_type ==
        SXD_BULK_CNTR_STREAMING_PROFILE_PROTOCOL_TYPE_GENETLINK) {
        if (!streaming_profiles[profile_id].genl_entry_p) {
            goto out;
        }
        entry_p = streaming_profiles[profile_id].genl_entry_p;
        if (entry_p) {
            err = __genl_family_release(entry_p, dev, profile_id);
            if (err) {
                sxd_log_err(dev,
                            "%s: failed to release Generic Netlink for profile_id %u (err=%d)\n",
                            __func__,
                            profile_id,
                            err);
                /* Still clear the local slot so deinit cannot leave a dangling pointer. */
            }
        } else if (streaming_profiles[profile_id].is_allocated) {
            sxd_log_err(dev,
                        "%s: profile_id %u is allocated but has no Generic Netlink registry entry\n",
                        __func__,
                        profile_id);
            err = -EINVAL;
        }

        __put_net_safe(&streaming_profiles[profile_id].net_ns);
    }
    memset(&streaming_profiles[profile_id], 0, sizeof(streaming_profiles[profile_id]));

out:
    return err;
}

/**
 * Force-drain the global Generic Netlink family registry. Used on module deinit
 * as a safety net after per-device profile cleanup.
 */
static void __genl_family_db_flush(void)
{
    struct sxd_bulk_cntr_genl_family_entry *entry = NULL;
    struct sxd_bulk_cntr_genl_family_entry *tmp_entry = NULL;
    struct sxd_bulk_cntr_genl_holder       *holder = NULL;
    struct sxd_bulk_cntr_genl_holder       *tmp = NULL;

    LIST_HEAD(to_free);
    int bkt = 0;
    int err = 0;

    spin_lock_bh(&g_bulk_cntr_genl_family_db_lock);
    hash_for_each(g_bulk_cntr_genl_family_db, bkt, entry, hnode) {
        list_for_each_entry_safe(holder, tmp, &entry->holders, list) {
            list_del(&holder->list);
            kfree(holder);
        }
        entry->refcnt = 0;
        entry->unregistering = true;
        list_add_tail(&entry->flush_node, &to_free);
    }
    spin_unlock_bh(&g_bulk_cntr_genl_family_db_lock);

    list_for_each_entry_safe(entry, tmp_entry, &to_free, flush_node) {
        list_del(&entry->flush_node);
        err = genl_unregister_family(&entry->genl_family);
        if (err) {
            sxd_log_err(SXD_DEV_NONE,
                        "%s: failed to unregister leftover family '%s' (err=%d), "
                        "keeping it in the registry for reuse\n",
                        __func__,
                        entry->family_name,
                        err);
            spin_lock_bh(&g_bulk_cntr_genl_family_db_lock);
            entry->unregistering = false;
            spin_unlock_bh(&g_bulk_cntr_genl_family_db_lock);
        } else {
            sxd_log_notice(SXD_DEV_NONE,
                           "%s: flushed leftover Generic Netlink family '%s'\n",
                           __func__,
                           entry->family_name);
            spin_lock_bh(&g_bulk_cntr_genl_family_db_lock);
            hash_del(&entry->hnode);
            spin_unlock_bh(&g_bulk_cntr_genl_family_db_lock);
            kfree(entry);
        }
    }
}

int bulk_cntr_streaming_profile_add(struct sx_dev* dev, ku_bulk_cntr_streaming_profile_t *profile_p)
{
    int                                      err = 0;
    uint32_t                                 profile_id = profile_p->profile_id;
    struct sx_priv                          *priv = sx_priv(dev);
    sxd_bulk_cntr_streaming_profile_attrs_t *streaming_profiles = priv->bulk_counters.streaming_profiles;

    if (profile_id >= SXD_BULK_CNTR_STREAMING_PROFILE_MAX_NUM) {
        return -EINVAL;
    }

    sxd_log_debug(dev, "DBG: %s: profile_id %u, is_allocated: %d, type=%d.\n",
                  __func__,
                  profile_id,
                  streaming_profiles[profile_id].is_allocated,
                  streaming_profiles[profile_id].attrs.protocol_type);

    if (streaming_profiles[profile_id].is_allocated) {
        err = -EEXIST;
        sxd_log_err(dev, "%s: profile_id %u has been added before.\n", __func__, profile_id);
        goto out;
    }
    memset(&streaming_profiles[profile_id], 0, sizeof(streaming_profiles[profile_id]));
    memcpy(&streaming_profiles[profile_id].attrs,
           &profile_p->attrs,
           sizeof(streaming_profiles[profile_id].attrs));
    streaming_profiles[profile_id].max_packet_size = profile_p->max_packet_size;

    if (streaming_profiles[profile_id].attrs.protocol_type ==
        SXD_BULK_CNTR_STREAMING_PROFILE_PROTOCOL_TYPE_GENETLINK) {
        err = __genl_family_acquire(dev,
                                    profile_id,
                                    &streaming_profiles[profile_id].attrs,
                                    &streaming_profiles[profile_id].genl_entry_p);
        if (err) {
            sxd_log_err(dev,
                        "%s: failed to acquire generic netlink for profile_id %u.\n",
                        __func__,
                        profile_id);
            memset(&streaming_profiles[profile_id], 0, sizeof(streaming_profiles[profile_id]));
            goto out;
        }
        __get_net_safe(&streaming_profiles[profile_id].net_ns);
        sxd_log_notice(dev,
                       "Bulk counter streaming profile_id %u using Generic Netlink family %s, netns inode: %u.\n",
                       profile_id,
                       streaming_profiles[profile_id].genl_entry_p->family_name,
                       streaming_profiles[profile_id].net_ns ?
                       streaming_profiles[profile_id].net_ns->ns.inum : 0);
    }
    streaming_profiles[profile_id].is_allocated = true;

out:
    return err;
}

int bulk_cntr_streaming_profile_del(struct sx_dev* dev, uint32_t profile_id)
{
    int                                      err = 0;
    struct sx_priv                          *priv = sx_priv(dev);
    sxd_bulk_cntr_streaming_profile_attrs_t *streaming_profiles = priv->bulk_counters.streaming_profiles;

    if (profile_id >= SXD_BULK_CNTR_STREAMING_PROFILE_MAX_NUM) {
        return -EINVAL;
    }

    sxd_log_debug(dev, "DBG: %s: profile_id %u, is_allocated: %d.\n",
                  __func__,
                  profile_id,
                  streaming_profiles[profile_id].is_allocated);

    if (!streaming_profiles[profile_id].is_allocated) {
        err = -ENOENT;
        sxd_log_err(dev, "%s: profile_id %u has not been added before.\n", __func__,
                    profile_id);
        goto out;
    }

    err = __streaming_profile_clear(dev, profile_id);
    if (err) {
        sxd_log_err(dev,
                    "%s: failed to clear streaming profile_id %u.\n",
                    __func__,
                    profile_id);
        goto out;
    }

out:
    return err;
}

/* in case of sdk failures, we must make sure to un-register generic netlink families. */
void sx_bulk_cntr_streaming_profile_unregister(struct sx_dev* dev)
{
    int i = 0;

    for (i = 0; i < SXD_BULK_CNTR_STREAMING_PROFILE_MAX_NUM; i++) {
        __streaming_profile_clear(dev, i);
    }

    return;
}

/* add a new streaming buffer struct */
int bulk_cntr_streaming_buffer_add(struct sx_dev                       * dev,
                                   ku_bulk_cntr_streaming_buffer_info_t *bulk_cntr_streaming_buffer)
{
    sxd_bulk_cntr_buffer_layout_common_t      *layout_common;
    int                                        err = 0;
    enum sxd_bulk_cntr_key_type_e              type = bulk_cntr_streaming_buffer->type;
    unsigned long                              buffer_id = bulk_cntr_streaming_buffer->buffer_id;
    uint32_t                                   profile_id = bulk_cntr_streaming_buffer->profile_id;
    struct sx_priv                            *priv = sx_priv(dev);
    sxd_bulk_cntr_streaming_profile_attrs_t   *streaming_profiles = priv->bulk_counters.streaming_profiles;
    sxd_bulk_cntr_streaming_buffer_id_attrs_t *streaming_buffer_ids = priv->bulk_counters.streaming_buffer_ids;

    if (buffer_id >= SXD_BULK_CNTR_STREAMING_BUFFER_ID_MAX_NUM) {
        return -EINVAL;
    }
    if (profile_id >= SXD_BULK_CNTR_STREAMING_PROFILE_MAX_NUM) {
        return -EINVAL;
    }

    sxd_log_debug(dev,
                  "DBG: %s: type %d, buffer_id %lu (allocated: %d), profile_id %u (allocated: %d), buff_size: %u.\n",
                  __func__,
                  type,
                  buffer_id,
                  streaming_buffer_ids[buffer_id].is_allocated,
                  profile_id,
                  streaming_profiles[profile_id].is_allocated,
                  bulk_cntr_streaming_buffer->buffer_size);

    switch (type) {
    case SXD_BULK_CNTR_KEY_TYPE_HFT_E:

        if (streaming_buffer_ids[buffer_id].is_allocated) {
            err = -EEXIST;
            sxd_log_err(dev, "%s: buffer_id %lu has been added before.\n", __func__, buffer_id);
            goto out;
        }
        if (streaming_profiles[profile_id].attrs.protocol_type ==
            SXD_BULK_CNTR_STREAMING_PROFILE_PROTOCOL_TYPE_GENETLINK) {
            if (!streaming_profiles[profile_id].is_allocated ||
                !streaming_profiles[profile_id].genl_entry_p) {
                err = -EINVAL;
                sxd_log_err(dev, "%s: profile_id %u has no Generic Netlink registry entry.\n",
                            __func__, profile_id);
                goto out;
            }
        }
        memset(&streaming_buffer_ids[buffer_id], 0, sizeof(streaming_buffer_ids[buffer_id]));
        layout_common = (sxd_bulk_cntr_buffer_layout_common_t *)kzalloc(sizeof(sxd_bulk_cntr_buffer_layout_hft_t),
                                                                        GFP_KERNEL);
        if (!layout_common) {
            err = -ENOMEM;
            goto out;
        }
        layout_common->type = type;
        layout_common->buff_size = bulk_cntr_streaming_buffer->buffer_size;
        layout_common->counters_received_so_far = 0;
        layout_common->cookie = bulk_cntr_streaming_buffer->cookie;

        streaming_buffer_ids[buffer_id].layout = layout_common;
        streaming_buffer_ids[buffer_id].type = bulk_cntr_streaming_buffer->type;
        streaming_buffer_ids[buffer_id].buffer_size = bulk_cntr_streaming_buffer->buffer_size;
        streaming_buffer_ids[buffer_id].profile_id = profile_id;
        streaming_buffer_ids[buffer_id].genl_family_p =
            &streaming_profiles[profile_id].genl_entry_p->genl_family;
        streaming_buffer_ids[buffer_id].ipfix_num =
            streaming_profiles[profile_id].attrs.format_data.format_data_ipfix.ipfix_msgs_num;
        /* First exported message uses sequence 1. Later sends continue from next_ipfix_sequence. */
        streaming_buffer_ids[buffer_id].next_ipfix_sequence = 1;
        streaming_buffer_ids[buffer_id].is_allocated = true;
        streaming_buffer_ids[buffer_id].net_ns = streaming_profiles[profile_id].net_ns;
        break;

    default:
        /* other types are to be extended in future. */
        err = -EOPNOTSUPP;
        sxd_log_err(dev, "%s: Not support type %d, (err=%d)\n", __func__, type, err);
        break;
    }

out:
    return err;
}

/* del a new streaming buffer struct */
int bulk_cntr_streaming_buffer_del(struct sx_dev                       * dev,
                                   ku_bulk_cntr_streaming_buffer_info_t *bulk_cntr_streaming_buffer)
{
    int                                        err = 0;
    unsigned long                              buffer_id = bulk_cntr_streaming_buffer->buffer_id;
    struct sx_priv                            *priv = sx_priv(dev);
    sxd_bulk_cntr_streaming_buffer_id_attrs_t *streaming_buffer_ids = priv->bulk_counters.streaming_buffer_ids;

    if (buffer_id >= SXD_BULK_CNTR_STREAMING_BUFFER_ID_MAX_NUM) {
        return -EINVAL;
    }

    sxd_log_debug(dev, "DBG: %s: buffer_id %lu, is_allocated: %d, is_buffer_alloc:%d.\n",
                  __func__,
                  buffer_id,
                  streaming_buffer_ids[buffer_id].is_allocated,
                  streaming_buffer_ids[buffer_id].is_buffer_allocated);

    if (!streaming_buffer_ids[buffer_id].is_allocated) {
        err = -ENOENT;
        sxd_log_err(dev, "%s: buffer_id %lu has not been added before.\n", __func__, buffer_id);
        goto out;
    }
    kfree(streaming_buffer_ids[buffer_id].layout);
    streaming_buffer_ids[buffer_id].is_allocated = false;
    streaming_buffer_ids[buffer_id].net_ns = NULL;

out:
    return err;
}

/* cancel an operation that is in progress */
int bulk_cntr_db_cancel(struct sx_dev *dev, pid_t client_pid, unsigned long buffer_id, bool is_streaming)
{
    struct sxd_bulk_cntr_buffer_layout_common *layout_common;
    struct transaction_entry                  *entry;
    int                                        err = 0;
    struct sx_priv                            *priv = sx_priv(dev);

    sxd_log_debug(dev,
                  "DBG: %s: buffer_id %lu, is_streaming: %d.\n",
                  __func__,
                  buffer_id,
                  is_streaming);

    layout_common = __get_buffer(dev, client_pid, buffer_id, is_streaming, false);
    if (!layout_common) {
        sxd_log_err(dev, "Bulk-Counter DB cancel - invalid buffer_id\n");
        return -EINVAL;
    }

    if (layout_common->type >= SXD_BULK_CNTR_KEY_TYPE_LAST_E) {
        return -EINVAL;
    }

    /* so far only HFT supports bulk counter streaming mode. */
    if (is_streaming && (layout_common->type != SXD_BULK_CNTR_KEY_TYPE_HFT_E)) {
        sxd_log_err(dev, "%s: type %d does not support bulk counter streaming mode, (err=%d)\n",
                    __func__,
                    layout_common->type,
                    err);
        return -EOPNOTSUPP;
    }

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

    /* check if there requested buffer is indeed in progress */
    entry = priv->bulk_counters.in_progress[layout_common->type];
    if (!entry) {
        err = -ENOENT;
        goto out;
    }

    if (entry->buffer_id != buffer_id) {
        /* If buffer IDs are not equal and the type of buffer is the FLOW COUNTERS type,
         *  we need to check also the ACCUFLOW COUNTERS type as by the design
         *  on the user level there is only the FLOW COUNTERS type. */
        if (layout_common->type == SXD_BULK_CNTR_KEY_TYPE_FLOW_E) {
            entry = priv->bulk_counters.in_progress[SXD_BULK_CNTR_KEY_TYPE_ACCUFLOW_E];
            if (!entry || (entry->buffer_id != buffer_id)) {
                err = -ENOENT;
                goto out;
            }
        } else {
            err = -ENOENT;
            goto out;
        }
    }

    /* mark the transaction as 'canceled' */
    entry->is_canceled = 1;

    __bulk_cntr_db_reset_entry_gc_internal(entry, false, false);

out:
    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    return err;
}

/* get a buffer from MOCS event_id field (upon MOCS parsing) */
bool bulk_cntr_db_event_id_to_buffer_get(struct sx_dev                              *dev,
                                         enum sxd_bulk_cntr_key_type_e               type,
                                         sxd_bulk_cntr_event_id_t                   *event_id_p,
                                         struct sxd_bulk_cntr_buffer_layout_common **layout_common,
                                         struct sxd_bulk_cntr_buff_ref_token        *ref_token_p)
{
    struct transaction_entry *entry;
    bool                      ret = false;
    struct sx_priv           *priv = sx_priv(dev);

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

    /* check that there is an operation of type 'type' in progress */
    entry = priv->bulk_counters.in_progress[type];
    if (!entry || (event_id_p && (entry->event_id.event_id_value != event_id_p->event_id_value))) {
        goto out;
    }

    __bulk_cntr_db_reset_entry_gc_internal(entry, true, false);

    __bulk_cntr_db_entry_ref_inc(entry);
    *layout_common = entry->common_layout;
    ref_token_p->client_id = entry->event_id.event_id_fields.client_id;
    ref_token_p->buffer_id = entry->buffer_id;
    ref_token_p->opaque = entry;
    ret = true;
out:
    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    return ret;
}

void bulk_cntr_db_buffer_put(struct sx_dev *dev, struct sxd_bulk_cntr_buff_ref_token *ref_token_p)
{
    struct sx_priv           *priv = sx_priv(dev);
    struct transaction_entry *entry = (struct transaction_entry*)ref_token_p->opaque;

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    __bulk_cntr_db_entry_ref_dec(entry);
    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
}

int bulk_cntr_db_per_prio_cache_set(struct sx_dev *dev, struct ku_bulk_cntr_per_prio_cache *bulk_cntr_per_prio_cache)
{
    int                                err = 0;
    sxd_port_per_prio_counter_cache_t* temp_per_prio_counter_cache = NULL;
    struct sx_priv                    *priv = sx_priv(dev);

    if (bulk_cntr_per_prio_cache->is_set) {
        temp_per_prio_counter_cache = (sxd_port_per_prio_counter_cache_t*)sx_mmap_user_to_kernel(
            dev,
            bulk_cntr_per_prio_cache->pid,
            bulk_cntr_per_prio_cache->user_ptr,
            false);

        if (!temp_per_prio_counter_cache) {
            sxd_log_err(dev, "Bulk-Counter DB per prio cache set - invalid buffer_id\n");
            err = -EINVAL;
            goto out;
        }
    }

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    priv->bulk_counters.per_prio_counter_cache = temp_per_prio_counter_cache;
    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

out:
    return err;
}

int bulk_cntr_db_per_prio_cache_entry_get(struct sx_dev         *dev,
                                          uint16_t               local_port,
                                          uint16_t               prio,
                                          sxd_port_cntr_prio_t **cache_entry)
{
    int             err = 0;
    struct sx_priv *priv = sx_priv(dev);

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

    if (!priv->bulk_counters.per_prio_counter_cache) {
        err = -ENOENT;
        goto out;
    }

    if ((local_port > MAX_PHYPORT_NUM) || (prio > SXD_PORT_PRIO_ID_MAX) || (!cache_entry)) {
        err = -EINVAL;
        goto out;
    }

    *cache_entry = &(priv->bulk_counters.per_prio_counter_cache->counters[local_port][prio]);

out:
    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    return err;
}

int bulk_cntr_db_in_progress(struct sx_dev *dev, struct ku_bulk_cntr_transaction *bulk_cntr_tr_in_progress, void *data)
{
    int err = 0;

    err = bulk_cntr_db_in_progress_get(dev, bulk_cntr_tr_in_progress->type, &bulk_cntr_tr_in_progress->in_progress);
    if (err) {
        goto out;
    }

    err = copy_to_user((void*)data, bulk_cntr_tr_in_progress, sizeof(*bulk_cntr_tr_in_progress));
    if (err) {
        goto out;
    }

out:
    return err;
}

int bulk_cntr_db_in_progress_get(struct sx_dev *dev, enum sxd_bulk_cntr_key_type_e type, u8 *in_progress_p)
{
    int             err = 0;
    struct sx_priv *priv = sx_priv(dev);

    if (type >= SXD_BULK_CNTR_KEY_TYPE_LAST_E) {
        err = -EINVAL;
        sxd_log_err(dev, "Bulk-Counter DB in progress - invalid type: %u\n", type);
        goto out;
    }

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    *in_progress_p = (priv->bulk_counters.in_progress[type] != NULL);
    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

out:
    return err;
}

int bulk_cntr_db_in_progress_info_get(struct sx_dev                                    *dev,
                                      struct ku_bulk_cntr_transaction_in_progress_info *bulk_cntr_tr_in_progress_info,
                                      void                                             *data)
{
    int                                      err = 0;
    struct sx_priv                          *priv = sx_priv(dev);
    enum sxd_bulk_cntr_key_type_e            type = bulk_cntr_tr_in_progress_info->type;
    struct sxd_bulk_cntr_buffer_layout_port *layout_port;
    struct transaction_entry                *entry = NULL;

    if ((type != SXD_BULK_CNTR_KEY_TYPE_PORT_E) && (type != SXD_BULK_CNTR_KEY_TYPE_PORT_EXT_E)) {
        err = -EINVAL;
        sxd_log_err(dev, "Bulk-Counter DB in progress info get - invalid type: %u\n", type);
        goto out;
    }

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    /* check that there is an operation of type 'type' in progress */
    entry = priv->bulk_counters.in_progress[type];
    if (entry != NULL) {
        layout_port = (struct sxd_bulk_cntr_buffer_layout_port *)entry->common_layout;
        memcpy(&(bulk_cntr_tr_in_progress_info->params.port_params.mappings),
               &(layout_port->mappings),
               sizeof(bulk_cntr_tr_in_progress_info->params.port_params.mappings));

        bulk_cntr_tr_in_progress_info->event_id = entry->event_id;
        bulk_cntr_tr_in_progress_info->in_progress = true;
    } else {
        bulk_cntr_tr_in_progress_info->in_progress = false;
    }

    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

    err = copy_to_user((void*)data, bulk_cntr_tr_in_progress_info, sizeof(*bulk_cntr_tr_in_progress_info));
    if (err) {
        goto out;
    }

out:
    return err;
}

static void __bulk_cntr_db_hft_mmap_dtor(struct sx_dev *dev, unsigned long user_ptr, pid_t pid, unsigned long size)
{
    struct sx_priv *priv = sx_priv(dev);

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

    if (priv->bulk_counters.bulk_counter_buffer_hft_utilization >= size) {
        priv->bulk_counters.bulk_counter_buffer_hft_utilization -= size;
    } else {
        sxd_log_err(dev,
                    "hft memory destructor failure hft total size(%llu), size freed(%lu).\n",
                    priv->bulk_counters.bulk_counter_buffer_hft_utilization,
                    size);
    }

    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
}

int bulk_cntr_db_buffer_stats_set(struct sx_dev *dev, struct ku_bulk_cntr_buffer_stats *bulk_cntr_buffer_stats_data_p)
{
    int             err = 0;
    struct sx_priv *priv = sx_priv(dev);

    /* is_allocated is set to true when NOS maps memory for bulk counters and
     * is set to false when NOS unmaps the memory, but we do accounting when memory
     * is actually freed through the destructor callback.
     */
    if (bulk_cntr_buffer_stats_data_p->is_allocated) {
        /* max buffer limit size 0 is considered as unlimited, so we just need to update utilization.
         * SDK imposes a hard limit on the buffer than can allocated when max buffer limit is non zero
         */
        spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
        if ((priv->bulk_counters.bulk_counter_max_buffer_hft_limit == 0) ||
            (priv->bulk_counters.bulk_counter_buffer_hft_utilization + bulk_cntr_buffer_stats_data_p->size <=
             (priv->bulk_counters.bulk_counter_max_buffer_hft_limit * BYTES_IN_MB))) {
            priv->bulk_counters.bulk_counter_buffer_hft_utilization += bulk_cntr_buffer_stats_data_p->size;
        } else {
            err = -ENOMEM;
        }
        spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
        if (!err) {
            sx_mmap_set_pfn_dtor(dev,
                                 bulk_cntr_buffer_stats_data_p->user_ptr,
                                 bulk_cntr_buffer_stats_data_p->pid,  bulk_cntr_buffer_stats_data_p->size,
                                 __bulk_cntr_db_hft_mmap_dtor);
        }
    }

    return err;
}

int bulk_cntr_db_buffer_info_get(struct sx_dev *dev, struct ku_bulk_cntr_buffer_info *bulk_cntr_buffer_info_p)
{
    struct sx_priv *priv = sx_priv(dev);

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    bulk_cntr_buffer_info_p->current_utilization = priv->bulk_counters.bulk_counter_buffer_hft_utilization;
    bulk_cntr_buffer_info_p->max_size = priv->bulk_counters.bulk_counter_max_buffer_hft_limit;
    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

    return 0;
}

int bulk_cntr_db_buffer_info_set(struct sx_dev *dev, struct ku_bulk_cntr_buffer_info *bulk_cntr_buffer_info_p)
{
    int             err = 0;
    struct sx_priv *priv = sx_priv(dev);

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    /* we need to make sure that the current utilization is less than the new max if the size is not unlimited. */
    if (bulk_cntr_buffer_info_p->max_size != 0) {
        if ((bulk_cntr_buffer_info_p->max_size * BYTES_IN_MB) <
            priv->bulk_counters.bulk_counter_buffer_hft_utilization) {
            err = -ENOMEM;
            goto out;
        }
    }
    priv->bulk_counters.bulk_counter_max_buffer_hft_limit = bulk_cntr_buffer_info_p->max_size;

out:
    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    return err;
}

int bulk_cntr_genl_family_db_dump(struct seq_file *m)
{
    struct sxd_bulk_cntr_genl_family_entry *entry = NULL;
    struct sxd_bulk_cntr_genl_holder       *holder = NULL;
    int                                     bkt = 0;

    seq_printf(m, "Global Generic Netlink family registry:\n");

    spin_lock_bh(&g_bulk_cntr_genl_family_db_lock);
    hash_for_each(g_bulk_cntr_genl_family_db, bkt, entry, hnode) {
        seq_printf(m, "\tFamily: %s\n", entry->family_name);
        seq_printf(m, "\t\tMC group: %s\n", entry->mcgrp_name);
        seq_printf(m, "\t\trefcnt: %u\n", entry->refcnt);
        seq_printf(m, "\t\tregistering: %d\n", entry->registering);
        seq_printf(m, "\t\tunregistering: %d\n", entry->unregistering);
        seq_printf(m, "\t\tholders:\n");
        list_for_each_entry(holder, &entry->holders, list) {
            seq_printf(m,
                       "\t\t\tdev_id=%u profile_id=%u\n",
                       holder->dev ? holder->dev->device_id : 0,
                       holder->profile_id);
        }
    }
    spin_unlock_bh(&g_bulk_cntr_genl_family_db_lock);

    return 0;
}

int sx_core_bulk_cntr_init(void)
{
    spin_lock_init(&g_hft_session_ctx.hft_wq_lock);
    g_hft_session_ctx.hft_wq_p = create_singlethread_workqueue("hft_wq");
    if (!g_hft_session_ctx.hft_wq_p) {
        sxd_log_err(SXD_DEV_NONE, "Failed to create HFT work queue.\n");
        return -ENOMEM;
    }

    if (bulk_cntr_db_event_cache_create()) {
        sxd_log_err(SXD_DEV_NONE, "Failed to create event cache.\n");
        return -ENOMEM;
    }
    spin_lock_bh(&g_hft_session_ctx.hft_wq_lock);
    g_hft_session_ctx.is_hft_wq_active = true;
    spin_unlock_bh(&g_hft_session_ctx.hft_wq_lock);
    return 0;
}


void sx_core_bulk_cntr_deinit(void)
{
    spin_lock_bh(&g_hft_session_ctx.hft_wq_lock);
    g_hft_session_ctx.is_hft_wq_active = false;
    spin_unlock_bh(&g_hft_session_ctx.hft_wq_lock);

    flush_workqueue(g_hft_session_ctx.hft_wq_p);
    destroy_workqueue(g_hft_session_ctx.hft_wq_p);
    g_hft_session_ctx.hft_wq_p = NULL;
    bulk_cntr_db_event_cache_delete();

    /* Safety net: drop any leftover shared Genl registrations after per-device cleanup. */
    __genl_family_db_flush();
}

bool sx_core_hft_queue_work(struct work_struct *w, struct sx_dev *dev)
{
    bool            ret = true;
    struct sx_priv *priv = sx_priv(dev);

    if (priv->bulk_counters.in_progress[SXD_BULK_CNTR_KEY_TYPE_HFT_E] == NULL) {
        sxd_log_rl_warning(dev, "sx_core_hft_queue_work"
                           "Err: HFT is not started");
        ret = false;
        goto out;
    }

    spin_lock_bh(&g_hft_session_ctx.hft_wq_lock);
    if (!g_hft_session_ctx.is_hft_wq_active) {
        ret = false;
        spin_unlock_bh(&g_hft_session_ctx.hft_wq_lock);
        goto out;
    }
    ret = queue_work(g_hft_session_ctx.hft_wq_p, w);
    spin_unlock_bh(&g_hft_session_ctx.hft_wq_lock);

out:
    return ret;
}

int bulk_cntr_db_mocs_session_acquire(struct sx_dev *dev, enum sxd_bulk_cntr_key_type_e type)
{
    int             err = 0;
    struct sx_priv *priv = sx_priv(dev);

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

    if (type == SXD_BULK_CNTR_KEY_TYPE_HFT_E) {
        if ((priv->bulk_counters.in_progress[type] != NULL) || (priv->bulk_counters.hft_reservation == true)) {
            sxd_log_err(dev, "HFT counters have already reserved a MOCS session.\n");
            err = -EBUSY;
            goto out;
        }
        priv->bulk_counters.hft_reservation = true;
        goto out;
    }

    if (priv->bulk_counters.in_progress_cnt >= BULK_CNTR_MAX_IN_PROGRESS) {
        sxd_log_err(dev, "There is no free MOCS session.\n");
        err = -EBUSY;
        goto out;
    }

    if (type == SXD_BULK_CNTR_KEY_TYPE_ACCUFLOW_E) {
        if (priv->bulk_counters.accuflow_reservation) {
            sxd_log_err(dev, "Accumulated counters have already reserved a MOCS session.\n");
            err = -EBUSY;
            goto out;
        }
        priv->bulk_counters.accuflow_reservation = true;
    }

    priv->bulk_counters.in_progress_cnt++;

out:
    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    return err;
}
EXPORT_SYMBOL(bulk_cntr_db_mocs_session_acquire);

int bulk_cntr_db_mocs_session_release(struct sx_dev *dev, enum sxd_bulk_cntr_key_type_e type)
{
    int             err = 0;
    struct sx_priv *priv = sx_priv(dev);

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

    if (type == SXD_BULK_CNTR_KEY_TYPE_HFT_E) {
        if (priv->bulk_counters.hft_reservation == false) {
            err = -EINVAL;
            sxd_log_err(dev, "Bulk-Counter DB hft session release - nothing to release \n");
            goto out;
        }
        priv->bulk_counters.hft_reservation = false;
        goto out;
    }

    if (priv->bulk_counters.in_progress_cnt == 0) {
        err = -EINVAL;
        sxd_log_err(dev, "Bulk-Counter DB session release - nothing to release \n");
        goto out;
    }

    if (type == SXD_BULK_CNTR_KEY_TYPE_ACCUFLOW_E) {
        if (priv->bulk_counters.accuflow_reservation == false) {
            sxd_log_err(dev, "Accumulated counters didn't reserve a MOCS session.\n");
            err = -EINVAL;
            goto out;
        }
        priv->bulk_counters.accuflow_reservation = false;
    }

    priv->bulk_counters.in_progress_cnt--;

out:
    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    return err;
}
EXPORT_SYMBOL(bulk_cntr_db_mocs_session_release);

int bulk_cntr_db_dump(struct seq_file *m, void *v, void *context)
{
    int               err = 0;
    int               type;
    struct sx_dev    *dev = sx_dbg_dump_get_device(m);
    struct sx_priv   *priv = sx_priv(dev);
    char              time_str[32];
    struct sx_timeval now;

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

    seq_printf(m, "Bulk counters DB:\n");
    seq_printf(m, "Device ID                        : %u \n", dev->device_id);
    seq_printf(m, "Transactions in progress         : %u \n", priv->bulk_counters.in_progress_cnt);
    seq_printf(m, "Max transactions in parallel     : %u \n", BULK_CNTR_MAX_IN_PROGRESS);
    seq_printf(m, "Accumulated counters reservation : %u \n", priv->bulk_counters.accuflow_reservation);
    seq_printf(m,
               "HFT bulk buffer max limit : %llu MB\n",
               priv->bulk_counters.bulk_counter_max_buffer_hft_limit);
    seq_printf(m,
               "HFT bulk buffer current utilization : %llu bytes\n",
               priv->bulk_counters.bulk_counter_buffer_hft_utilization);

    sx_dbg_get_time(&now);

    seq_printf(m, "Transactions:\n");
    for (type = 0; type < SXD_BULK_CNTR_KEY_TYPE_LAST_E; type++) {
        if (priv->bulk_counters.in_progress[type] == NULL) {
            seq_printf(m, "\tTransaction type #%d ............ Idle\n", type);
        } else {
            sx_dbg_jiffies_to_time_str(&now, priv->bulk_counters.in_progress[type]->start_time, time_str,
                                       sizeof(time_str));
            seq_printf(m,
                       "\tTransaction type #%d ............ EventID: 0x%llx (client_id=%u, id=%u), Started: %s, Duration (msec): %u\n",
                       type,
                       priv->bulk_counters.in_progress[type]->event_id.event_id_value,
                       priv->bulk_counters.in_progress[type]->event_id.event_id_fields.client_id,
                       priv->bulk_counters.in_progress[type]->event_id.event_id_fields.id,
                       time_str,
                       jiffies_to_msecs(jiffies - priv->bulk_counters.in_progress[type]->start_time));
        }
    }

    seq_printf(m, "Maximum duration:\n");
    for (type = 0; type < SXD_BULK_CNTR_KEY_TYPE_LAST_E; type++) {
        if (priv->bulk_counters.top_durations[type].duration == 0) {
            seq_printf(m, "\tTransaction type #%d ............ N/A\n", type);
        } else {
            sx_dbg_jiffies_to_time_str(&now, priv->bulk_counters.top_durations[type].start_time, time_str,
                                       sizeof(time_str));
            seq_printf(m,
                       "\tTransaction type #%d ............ EventID: 0x%llx (client_id=%u, id=%u), Started: %s, Duration (msec): %u\n",
                       type,
                       priv->bulk_counters.top_durations[type].event_id.event_id_value,
                       priv->bulk_counters.top_durations[type].event_id.event_id_fields.client_id,
                       priv->bulk_counters.top_durations[type].event_id.event_id_fields.id,
                       time_str,
                       priv->bulk_counters.top_durations[type].duration);
        }
    }

    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    return err;
}

static void __bulk_cntr_db_gc(struct work_struct *work)
{
    struct delayed_work      *dwork = to_delayed_work(work);
    struct sx_priv           *priv;
    struct transaction_entry *tr_entry;
    unsigned long             now = jiffies;
    int                       type;

    priv = container_of(dwork, struct sx_priv, bulk_counters.gc_dwork);

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    for (type = 0; type < SXD_BULK_CNTR_KEY_TYPE_LAST_E; type++) {
        tr_entry = priv->bulk_counters.in_progress[type];

        if (!tr_entry) {
            continue;
        }

        /* Streaming-to-network HFT has no host-delivered progress events until the session is cancelled and
         * awaits the MOCS_DONE completion event from FW, so exempt it while running. */
        if (tr_entry->gc_exempt && !tr_entry->is_canceled) {
            continue;
        }

        /*
         *  - When an entry is updated with FW events, it resets the gc.count to 0.
         *  - When an entry is cancelled, it resets gc.count to 0.
         *  - GC is running periodically for all bulk-counter entries (every X seconds).
         *  - Every periodic iteration GC increments gc.count for an entry.
         *  - So here in this function (after gc.count increment):
         *   1. if entry's gc.count is 1, it means that the entry was not updated 0-X seconds
         *      (depends on when the entry was added to the database). we'll keep monitoring it.
         *   2. if gc.count is 2, it means that the entry was not updated for >X seconds;
         *      we'll only put a big error message and if the transaction has been cancelled already,
         *      we'll also raise a fatal health event.
         */
        if (++tr_entry->gc.count == 2) {
            sxd_log_err(&priv->dev,
                        "Bulk-Counter GC: transaction is idle for too long "
                        "[type=%u, client_id=%u, id=%u, updates=%u, cancelled=%s, "
                        "got_mocs_done=%s, started (msecs ago)=%u]\n",
                        type,
                        tr_entry->event_id.event_id_fields.client_id,
                        tr_entry->event_id.event_id_fields.id,
                        tr_entry->gc.updates,
                        ((tr_entry->is_canceled) ? "yes" : "no"),
                        ((tr_entry->gc.got_mocs_done) ? "true" : "false"),
                        jiffies_to_msecs(now - tr_entry->start_time));

            if (tr_entry->is_canceled) {
                sx_health_report_error_generic(&priv->dev,
                                               SXD_HEALTH_SEVERITY_FATAL,
                                               "Bulk-Counter transaction is stuck");
            }
        }
    }
    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

    queue_delayed_work(priv->generic_wq, dwork, BULK_CNTR_GC_TOLERANCE_SECS * HZ);
}

void sx_core_bulk_counters_db_init(struct sx_priv *priv)
{
    size_t i;

    priv->bulk_counters.in_progress_cnt = 0;
    priv->bulk_counters.accuflow_reservation = false;
    priv->bulk_counters.hft_reservation = false;
    spin_lock_init(&priv->bulk_counters.bulk_cntr_db_lock);
    priv->bulk_counters.per_prio_counter_cache = NULL;
    priv->bulk_counters.bulk_counter_buffer_hft_utilization = 0;
    priv->bulk_counters.bulk_counter_max_buffer_hft_limit = BULK_CNTR_HFT_BUF_SIZE_MAX; /* 350MB default initialization. */
    for (i = 0; i < SXD_BULK_CNTR_KEY_TYPE_LAST_E; i++) {
        priv->bulk_counters.in_progress[i] = NULL;
    }

    INIT_DELAYED_WORK(&priv->bulk_counters.gc_dwork, __bulk_cntr_db_gc);
    queue_delayed_work(priv->generic_wq, &priv->bulk_counters.gc_dwork, 0 /* now */);
}

void sx_core_bulk_counters_db_deinit(struct sx_priv *priv)
{
    cancel_delayed_work_sync(&priv->bulk_counters.gc_dwork);

    /*
     * Device / probe-failure deinit must release every streaming profile's
     * shared Generic Netlink reference even if __sx_setup_sx_cleanup() was not run.
     */
    sx_bulk_cntr_streaming_profile_unregister(&priv->dev);
}
