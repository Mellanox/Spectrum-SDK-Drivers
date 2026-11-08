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

#ifndef __BULK_CNTR_DB_H__
#define __BULK_CNTR_DB_H__

#include <linux/seq_file.h>
#include <linux/list.h>
#include <linux/types.h>
#include <net/genetlink.h>
#include <linux/mlx_sx/kernel_user.h>
#include <linux/mlx_sx/device.h>

struct sxd_bulk_cntr_buff_ref_token {
    pid_t         client_id;
    unsigned long buffer_id;
    void         *opaque;
};

void bulk_cntr_db_reset_entry_gc(struct sx_dev                  *dev,
                                 const sxd_bulk_cntr_event_id_t *event_id_p,
                                 bool                            fw_update,
                                 bool                            mocs_done);
int bulk_cntr_db_add(struct sx_dev *dev, struct ku_bulk_cntr_transaction_add *bulk_cntr_tr_add);
int bulk_cntr_db_finalize(struct sx_dev *dev,
                          pid_t          client_pid,
                          unsigned long  buffer_id,
                          bool           is_streaming,
                          uint32_t       cookie);
int bulk_cntr_db_del(struct sx_dev *dev, pid_t client_pid, unsigned long buffer_id, bool is_streaming);
int bulk_cntr_db_cancel(struct sx_dev *dev, pid_t client_pid, unsigned long buffer_id, bool is_streaming);

int bulk_cntr_db_ack(unsigned long                    buffer_id,
                     sxd_bulk_cntr_event_id_t        *ev_id,
                     enum sxd_bulk_cntr_done_status_e status);

int bulk_cntr_db_complete(struct sx_dev                    *dev,
                          enum sxd_bulk_cntr_key_type_e     type,
                          unsigned long                    *buffer_id,
                          enum sxd_bulk_cntr_done_status_e *status,
                          uint32_t                         *cookie);

int bulk_cntr_db_in_progress_get(struct sx_dev *dev, enum sxd_bulk_cntr_key_type_e type, u8 *in_progress_p);
int bulk_cntr_db_in_progress(struct sx_dev *dev, struct ku_bulk_cntr_transaction *bulk_cntr_tr_in_progress,
                             void *data);

int bulk_cntr_db_in_progress_info_get(struct sx_dev                                    *dev,
                                      struct ku_bulk_cntr_transaction_in_progress_info *bulk_cntr_tr_in_progress_info,
                                      void                                             *data);

bool bulk_cntr_db_event_id_to_buffer_get(struct sx_dev                              *dev,
                                         enum sxd_bulk_cntr_key_type_e               type,
                                         sxd_bulk_cntr_event_id_t                   *event_id_p,
                                         struct sxd_bulk_cntr_buffer_layout_common **layout_common,
                                         struct sxd_bulk_cntr_buff_ref_token        *ref_token_p);
void bulk_cntr_db_buffer_put(struct sx_dev *dev, struct sxd_bulk_cntr_buff_ref_token *ref_token_p);

int bulk_cntr_db_per_prio_cache_set(struct sx_dev *dev, struct ku_bulk_cntr_per_prio_cache *bulk_cntr_per_prio_cache);

int bulk_cntr_db_per_prio_cache_entry_get(struct sx_dev         *dev,
                                          uint16_t               local_port,
                                          uint16_t               prio,
                                          sxd_port_cntr_prio_t **cache_entry);

int bulk_cntr_db_mocs_session_acquire(struct sx_dev *dev, enum sxd_bulk_cntr_key_type_e type);
int bulk_cntr_db_mocs_session_release(struct sx_dev *dev, enum sxd_bulk_cntr_key_type_e type);
int bulk_cntr_db_buffer_stats_set(struct sx_dev *dev, struct ku_bulk_cntr_buffer_stats *bulk_cntr_buffer_stats_data_p);
int bulk_cntr_db_buffer_info_get(struct sx_dev *dev, struct ku_bulk_cntr_buffer_info *bulk_cntr_buffer_info_p);
int bulk_cntr_db_buffer_info_set(struct sx_dev *dev, struct ku_bulk_cntr_buffer_info *bulk_cntr_buffer_info_p);
int bulk_cntr_db_dump(struct seq_file *m, void *v, void *context);

int sx_core_bulk_cntr_init(void);
void sx_core_bulk_cntr_deinit(void);
bool sx_core_hft_queue_work(struct work_struct *w, struct sx_dev *dev);

void bulk_cntr_db_event_cache_delete(void);
int bulk_cntr_db_event_cache_create(void);
void * bulk_cntr_db_event_record_allocate(void);
void bulk_cntr_db_event_record_free(void *reg_p);
int bulk_cntr_streaming_profile_add(struct sx_dev* dev, ku_bulk_cntr_streaming_profile_t *profile_p);
int bulk_cntr_streaming_profile_del(struct sx_dev* dev, uint32_t profile_id);
int bulk_cntr_streaming_buffer_add(struct sx_dev                       * dev,
                                   ku_bulk_cntr_streaming_buffer_info_t *bulk_cntr_streaming_buffer);
int bulk_cntr_streaming_buffer_del(struct sx_dev                       * dev,
                                   ku_bulk_cntr_streaming_buffer_info_t *bulk_cntr_streaming_buffer);

/**
 * Holder back-pointer from a shared Generic Netlink family registry entry to a
 * per-device streaming profile that references it.
 */
struct sxd_bulk_cntr_genl_holder {
    struct list_head list;
    struct sx_dev   *dev;
    uint32_t         profile_id;
};

/**
 * Global Generic Netlink family registry entry, keyed by family name.
 * Protected by g_bulk_cntr_genl_family_db_lock in bulk_cntr_db.c.
 */
struct sxd_bulk_cntr_genl_family_entry {
    struct hlist_node           hnode;
    char                        family_name[SXD_BULK_CNTR_STREAMING_GENETLINK_FAMILY_NAME_LEN];
    char                        mcgrp_name[SXD_BULK_CNTR_STREAMING_GENETLINK_MC_GROUP_NAME_LEN];
    uint8_t                     mcgrp_flags;
    struct genl_family          genl_family;
    struct genl_multicast_group genl_mcgrps[SXD_BULK_CNTR_STREAMING_GENETLINK_MC_GROUP_NUM];
    u32                         refcnt;
    bool                        registering;   /* true while genl_register_family is in progress */
    bool                        unregistering; /* true while genl_unregister_family is in progress */
    struct list_head            holders;       /* list of sxd_bulk_cntr_genl_holder */
    struct list_head            flush_node;    /* temporary list node used by module flush */
};

int bulk_cntr_genl_family_db_dump(struct seq_file *m);

#endif /* __BULK_CNTR_DB_H__ */
