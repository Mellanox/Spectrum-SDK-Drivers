/*
 * SPDX-FileCopyrightText: Copyright (c) 2010-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/hashtable.h>
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
/* HFT is not considered in this max progress limit. */
#define BULK_CNTR_MAX_IN_PROGRESS  (2)
#define BULK_CNTR_HFT_BUF_SIZE_MAX (350)

struct transaction_entry {
    unsigned long                              buffer_id; /* mmap pointer in user process */
    sxd_bulk_cntr_event_id_t                   event_id;
    unsigned long                              start_time; /* jiffies */
    u8                                         is_canceled; /* is transaction canceled by user */
    bool                                       is_finalize_buff_flow; /* deallocating a buffer of ongoing transaction */
    u32                                        refcnt;
    struct completion                          comp;
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
};

static struct kmem_cache        *__event_parser_cache_p = NULL;
static struct sx_hft_session_ctx g_hft_session_ctx;         /* HFT specific context.*/

/* should be called in DB lock */
static void __bulk_cntr_db_entry_ref_inc(struct transaction_entry *entry)
{
    entry->refcnt++;
}

/* should be called in DB lock */
static void __bulk_cntr_db_entry_ref_dec(struct transaction_entry *entry)
{
    if (--entry->refcnt == 0) {
        if (entry->is_finalize_buff_flow) {
            complete(&entry->comp);
        } else {
            kfree(entry);
        }
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

    sxd_log_debug(dev, "DBG: %s: type %d, is_streaming: %d.\n", __func__, type, is_streaming);

    /* so far only HFT supports bulk counter streaming mode. */
    if ((is_streaming) && (type != SXD_BULK_CNTR_KEY_TYPE_HFT_E)) {
        err = -EOPNOTSUPP;
        sxd_log_err(dev,
                    "%s: type %d does not support bulk counter streaming mode, (err=%d)\n",
                    __func__,
                    type,
                    err);
        goto out;
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
        /* TRIM counters use MOCS/MOTCNT mechanism with flow counters.
         * No additional buffer layout initialization needed - counter data
         * is parsed in SDK layer after MOCS completion. */
        break;

    default:
        break;
    }

    /* save transaction data */
    entry->buffer_id = bulk_cntr_tr_add->buffer_id;
    entry->event_id = bulk_cntr_tr_add->event_id;
    entry->is_canceled = 0;
    entry->start_time = jiffies;
    init_completion(&entry->comp);
    entry->refcnt = 1;
    entry->common_layout = layout_common;

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
               (type != SXD_BULK_CNTR_KEY_TYPE_STATEFUL_DB_E) && (type != SXD_BULK_CNTR_KEY_TYPE_AR_FLOW_E)) {
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
        /* Decrement reference to shared memory we hold from bulk_cntr_db_add()after lock dependency is removed.
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
                          uint32_t                         *cookie_p,
                          bool                             *send_notification_p)
{
    struct sx_priv           *priv = sx_priv(dev);
    struct transaction_entry *entry = NULL;
    int                       err = 0;
    unsigned long             buffer_id = 0;
    pid_t                     client_id = 0;
    bool                      is_streaming = false;

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

    if (send_notification_p) {
        /* if driver owns the transaction user should not get any notification */
        *send_notification_p = !entry->is_finalize_buff_flow;
    }

    err = __bulk_cntr_db_complete_internal(dev, entry, type, status_p);

out:
    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

    if (!err) {
        __bulk_cntr_db_post_complete(dev, client_id, buffer_id, is_streaming);
    }

    return err;
}

int bulk_cntr_db_finalize(struct sx_dev                *dev,
                          pid_t                         client_pid,
                          unsigned long                 buffer_id,
                          bool                          is_streaming,
                          enum sxd_bulk_cntr_key_type_e type,
                          uint32_t                      cookie)
{
    struct sx_priv                            *priv = sx_priv(dev);
    struct transaction_entry                  *entry;
    struct ku_access_mocs_reg                  mocs;
    struct sxd_bulk_cntr_buffer_layout_common *layout_common;
    enum sxd_bulk_cntr_done_status_e           tr_status;
    int                                        err, comp_err = 0;
    bool                                       transaction_completed = false;
    unsigned long                              start_time = 0;
    u32                                        duration = 0;

    /* first check if the transaction exists */
    layout_common = __get_buffer(dev, client_pid, buffer_id, is_streaming, false);
    if (!layout_common) {
        /* transaction completed gracefully, no more actions required */
        return 0;
    }

    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    entry = priv->bulk_counters.in_progress[type];
    if (entry && (entry->common_layout == layout_common)) {
        entry->is_finalize_buff_flow = true;
        start_time = entry->start_time;
    } else {
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
                   client_pid, buffer_id, type, cookie);

    /* OK, transaction did not complete. send MOCS(cancel) to FW */
    memset(&mocs, 0, sizeof(mocs));
    mocs.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mocs.op_tlv, MLXSW_MOCS_ID, EMAD_METHOD_WRITE);
    mocs.mocs_reg.opcode = SXD_MOCS_OPCODE_CANCEL_E;
    mocs.mocs_reg.type = (sxd_mocs_type_t)type;
    err = sx_ACCESS_REG_MOCS(dev, &mocs);
    if (err) {
        sxd_log_err(dev, "failed to cancel on-going transaction in FW (type=%u)\n", layout_common->type);
        /* continue the flow regardless of @err value */
    }

    /* Let's do a a quick check if transaction still exists after MOCS(cancel) */
    spin_lock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    if (entry == priv->bulk_counters.in_progress[type]) {
        /* now we complete the transaction (decrement our reference count from mmap and deallocate @entry.
         * any late MOCS_DONE for this transaction will be ignored because we remove it.
         */
        comp_err = __bulk_cntr_db_complete_internal(dev, entry, type, &tr_status);
    } else {
        /* Great! current [type] transaction is NULL or belongs to another transaction that just came in.
         * ours is already gone after all we've gone through... anyhow, we are good here. */
        transaction_completed = true;
    }
    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);

    if (transaction_completed) {
        sxd_log_notice(dev, "transaction completed gracefully after MOCS(cancel)"
                       "(client_pid=%d, buffer_id=0x%lx, type=%d, cookie=%u)\n",
                       client_pid, buffer_id, type, cookie);
        WARN_ON_ONCE(!entry->is_finalize_buff_flow);
        kfree(entry); /* entry->is_finalize_buff_flow == true, it means that we own the entry */
        return 0;
    }

    duration = jiffies_to_msecs(jiffies - start_time);
    sxd_log_warning(dev, "transaction still exists after MOCS(cancel), duration=%u msecs, complete_status=%d\n",
                    duration, comp_err);
    if (!comp_err) {
        wait_for_completion(&entry->comp);
        kfree(entry);

        __bulk_cntr_db_post_complete(dev, client_pid, buffer_id, is_streaming);
    }

    sx_health_report_error_generic(dev, SXD_HEALTH_SEVERITY_WARN, "Bulk transaction is stuck");
    return err;
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
    /* so far only HFT supports bulk counter streaming mode. */
    if (is_streaming && (type != SXD_BULK_CNTR_KEY_TYPE_HFT_E)) {
        err = -EOPNOTSUPP;
        sxd_log_err(dev,
                    "%s: type %d does not support bulk counter streaming mode, (err=%d)\n",
                    __func__,
                    type,
                    err);
        goto out;
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

static int __streaming_profile_register_genl(sxd_bulk_cntr_streaming_profile_attrs_t *profile_attrs)
{
    int ret = 0;
    int i = 0;

    strncpy(profile_attrs->genl_family.name,
            profile_attrs->attrs.protocol_data.protocol_data_genetlink.family_name,
            sizeof(profile_attrs->genl_family.name) - 1);
    profile_attrs->genl_family.version = profile_attrs->attrs.format_data.format_data_ipfix.version;
    /* we handle ipfix message as raw data of generic netlink message. */
    profile_attrs->genl_family.maxattr = 0;
    for (i = 0; i < SXD_BULK_CNTR_STREAMING_GENETLINK_MC_GROUP_NUM; i++) {
        if (profile_attrs->attrs.protocol_data.protocol_data_genetlink.mcgrps[i].name[0] == 0) {
            break;
        }
        strncpy(profile_attrs->genl_mcgrps[i].name,
                profile_attrs->attrs.protocol_data.protocol_data_genetlink.mcgrps[i].name,
                sizeof(profile_attrs->genl_mcgrps[i].name) - 1);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0))
        profile_attrs->genl_mcgrps[i].flags =
            profile_attrs->attrs.protocol_data.protocol_data_genetlink.mcgrps[i].flags;
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)) */
    }
    profile_attrs->genl_family.mcgrps = profile_attrs->genl_mcgrps;
    profile_attrs->genl_family.n_mcgrps = i;
    sxd_log_debug(SXD_DEV_NONE, "DBG: Try to register generic Netlink family %s, MC group[0]: %s.\n",
                  profile_attrs->genl_family.name,
                  profile_attrs->genl_family.mcgrps[0].name);
    ret = __genl_reg_family(&profile_attrs->genl_family);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "Failed to register generic Netlink family with ret = %d\n", ret);
        goto out;
    }
    sxd_log_notice(SXD_DEV_NONE, "Bulk counter streaming profile registers generic Netlink family %s.\n",
                   profile_attrs->genl_family.name);

out:
    return ret;
}

static int __streaming_profile_unregister_genl(sxd_bulk_cntr_streaming_profile_attrs_t *profile_attrs)
{
    int ret = 0;

    ret = genl_unregister_family(&profile_attrs->genl_family);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "Failed to unregister generic Netlink family with ret = %d\n", ret);
        goto out;
    }
    sxd_log_notice(SXD_DEV_NONE, "Bulk counter streaming profile un-register generic Netlink family %s.\n",
                   profile_attrs->genl_family.name);

out:
    return ret;
}

int bulk_cntr_streaming_profile_add(struct sx_dev* dev, ku_bulk_cntr_streaming_profile_t *profile_p)
{
    int                                      err = 0;
    uint32_t                                 profile_id = profile_p->profile_id;
    struct sx_priv                          *priv = sx_priv(dev);
    sxd_bulk_cntr_streaming_profile_attrs_t *streaming_profiles = priv->bulk_counters.streaming_profiles;

    sxd_log_debug(dev, "DBG: %s: profile_id %u, is_allocated: %d.\n",
                  __func__,
                  profile_id,
                  streaming_profiles[profile_id].is_allocated);

    if (streaming_profiles[profile_id].is_allocated) {
        err = -EEXIST;
        sxd_log_err(dev, "%s: profile_id %u has been added before.\n", __func__, profile_id);
        goto out;
    }
    memset(&streaming_profiles[profile_id], 0, sizeof(streaming_profiles[profile_id]));
    memcpy(&streaming_profiles[profile_id].attrs,
           &profile_p->attrs,
           sizeof(streaming_profiles[profile_id].attrs));
    err = __streaming_profile_register_genl(&streaming_profiles[profile_id]);
    if (err) {
        sxd_log_err(dev,
                    "%s: failed to register generic netlink for profile_id %u.\n",
                    __func__,
                    profile_id);
        goto out;
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

    err = __streaming_profile_unregister_genl(&streaming_profiles[profile_id]);
    if (err) {
        sxd_log_err(dev,
                    "%s: failed to un-register generic netlink for profile_id %u.\n",
                    __func__,
                    profile_id);
        goto out;
    }

    streaming_profiles[profile_id].is_allocated = false;

out:
    return err;
}

/* in case of sdk failures, we must make sure to un-register generic netlink families. */
void sx_bulk_cntr_streaming_profile_unregister(struct sx_dev* dev)
{
    int                                      i = 0;
    struct sx_priv                          *priv = sx_priv(dev);
    sxd_bulk_cntr_streaming_profile_attrs_t *streaming_profiles = priv->bulk_counters.streaming_profiles;

    for (i = 0; i < SXD_BULK_CNTR_STREAMING_PROFILE_MAX_NUM; i++) {
        if (streaming_profiles[i].is_allocated) {
            __streaming_profile_unregister_genl(&streaming_profiles[i]);
        }
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
        streaming_buffer_ids[buffer_id].genl_family_p = &streaming_profiles[profile_id].genl_family;
        streaming_buffer_ids[buffer_id].ipfix_num =
            streaming_profiles[profile_id].attrs.format_data.format_data_ipfix.ipfix_msgs_num;
        streaming_buffer_ids[buffer_id].is_allocated = true;
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

    /* so far only HFT supports bulk counter streaming mode. */
    if (is_streaming && (layout_common->type != SXD_BULK_CNTR_KEY_TYPE_HFT_E)) {
        err = -EOPNOTSUPP;
        sxd_log_err(dev, "%s: type %d does not support bulk counter streaming mode, (err=%d)\n",
                    __func__,
                    layout_common->type,
                    err);
        goto out;
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
    if (!entry ||
        entry->is_finalize_buff_flow ||
        (event_id_p && (entry->event_id.event_id_value != event_id_p->event_id_value))) {
        goto out;
    }

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

int sx_core_bulk_cntr_init(void)
{
    g_hft_session_ctx.hft_wq_p = create_singlethread_workqueue("hft_wq");
    if (!g_hft_session_ctx.hft_wq_p) {
        sxd_log_err(SXD_DEV_NONE, "Failed to create HFT work queue.\n");
        return -ENOMEM;
    }

    if (bulk_cntr_db_event_cache_create()) {
        sxd_log_err(SXD_DEV_NONE, "Failed to create event cache.\n");
        return -ENOMEM;
    }

    return 0;
}


void sx_core_bulk_cntr_deinit(void)
{
    flush_workqueue(g_hft_session_ctx.hft_wq_p);
    destroy_workqueue(g_hft_session_ctx.hft_wq_p);
    g_hft_session_ctx.hft_wq_p = NULL;

    bulk_cntr_db_event_cache_delete();
}

bool sx_core_hft_queue_work(struct work_struct *w, struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);

    if (priv->bulk_counters.in_progress[SXD_BULK_CNTR_KEY_TYPE_HFT_E] == NULL) {
        sxd_log_rl_warning(dev, "sx_core_hft_queue_work"
                           "Err: HFT is not started");
        return -ENOENT;
    }

    return queue_work(g_hft_session_ctx.hft_wq_p, w);
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

    seq_printf(m, "Transactions:\n");
    for (type = 0; type < SXD_BULK_CNTR_KEY_TYPE_LAST_E; type++) {
        if (priv->bulk_counters.in_progress[type] == NULL) {
            seq_printf(m, "\tTransaction type #%d ............ Idle\n", type);
        } else {
            seq_printf(m, "\tTransaction type #%d ............ EventID: 0x%llx, BufferID: %lu, IsCanceled: %u\n",
                       type,
                       priv->bulk_counters.in_progress[type]->event_id.event_id_value,
                       priv->bulk_counters.in_progress[type]->buffer_id,
                       priv->bulk_counters.in_progress[type]->is_canceled);
        }
    }

    sx_dbg_get_time(&now);
    seq_printf(m, "Maximum duration:\n");
    for (type = 0; type < SXD_BULK_CNTR_KEY_TYPE_LAST_E; type++) {
        if (priv->bulk_counters.top_durations[type].duration == 0) {
            seq_printf(m, "\tTransaction type #%d ............ N/A\n", type);
        } else {
            sx_dbg_jiffies_to_time_str(&now, priv->bulk_counters.top_durations[type].start_time, time_str,
                                       sizeof(time_str));
            seq_printf(m, "\tTransaction type #%d ............ EventID: 0x%llx, Started: %s, Duration (msec): %u\n",
                       type,
                       priv->bulk_counters.top_durations[type].event_id.event_id_value,
                       time_str,
                       priv->bulk_counters.top_durations[type].duration);
        }
    }

    spin_unlock_bh(&priv->bulk_counters.bulk_cntr_db_lock);
    return err;
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
}
