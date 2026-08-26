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

#include <linux/seq_file.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/mlx_sx/cmd.h>

#include "sx.h"
#include "sx_dbg_dump_proc.h"
#include "alloc.h"
#include "mmap.h"
#include "ber_monitor.h"
#include "health_check.h"
#include "sx_af_counters.h"
#include "bulk_cntr_db.h"
#include "dev_db.h"
#include "sx_clock.h"
#include "ptp.h"
#include "synce.h"
#include "qos_emad.h"
#include "health_check.h"
#include "emad.h"
#include "dq.h"
#include "rx_buffers.h"
#include "dbg.h"

void sx_dbg_get_time(struct sx_timeval *sxtv)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    ktime_get_real_ts64(&sxtv->t);
#else
    do_gettimeofday(&sxtv->t);
#endif

    sxtv->t.tv_sec -= sys_tz.tz_minuteswest * 60;
    sxtv->j = jiffies;
}

void sx_dbg_jiffies_to_time(const struct sx_timeval *now, unsigned long jiffies_ts, struct sx_tm *sxtm)
{
    struct sx_timeval _now, past;
    u64               diff_usec, diff_msec;

    if (!now) { /* user did not supply current time, we'll get it alone */
        sx_dbg_get_time(&_now);
        now = &_now;
    }

    diff_msec = jiffies_to_msecs(now->j - jiffies_ts);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    diff_usec = (now->t.tv_sec * 1000000 + now->t.tv_nsec / 1000) - diff_msec * 1000;
    past.t.tv_sec = diff_usec / 1000000;
    past.t.tv_nsec = (diff_usec % 1000000) * 1000;
    time64_to_tm(past.t.tv_sec, 0, &sxtm->tm);
    sxtm->msec = (u32)(past.t.tv_nsec / 1000000);
#else
    diff_usec = (now->t.tv_sec * 1000000 + now->t.tv_usec) - diff_msec * 1000;
    past.t.tv_sec = diff_usec / 1000000;
    past.t.tv_usec = diff_usec % 1000000;
    time_to_tm(past.t.tv_sec, 0, &sxtm->tm);
    sxtm->msec = (u32)(past.t.tv_usec / 1000);
#endif
}

void sx_dbg_jiffies_to_time_str(const struct sx_timeval *now, unsigned long jiffies_ts, char *buff, u32 bufflen)
{
    struct sx_tm sxtm;

    sx_dbg_jiffies_to_time(now, jiffies_ts, &sxtm);

    snprintf(buff, bufflen, "%04d-%02d-%02d %02d:%02d:%02d.%03u",
             (int)(1900 + sxtm.tm.tm_year),
             sxtm.tm.tm_mon + 1,
             sxtm.tm.tm_mday,
             sxtm.tm.tm_hour,
             sxtm.tm.tm_min,
             sxtm.tm.tm_sec,
             sxtm.msec);
}

static int __module_i2c_mapping_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_priv       *priv;
    char                  header[SX_DBG_DUMP_HEADER_MAX_LEN];
    struct sx_dev        *dev = sx_dbg_dump_get_device(m);
    uint32_t              i2c_index = 0;
    i2c_bus_mapping_db_t *i2c_db_p = NULL;
    uint32_t              i;

    if (!dev) {
        goto out;
    }
    priv = sx_priv(dev);

    i2c_db_p = &(priv->module_eeprom_access_db.async_mod_access_db.i2c_bus_mapping_db);

    seq_printf(m,
               "Number of I2c modules buses %d\n", i2c_db_p->num_i2c_buses);

    seq_printf(m,
               "Number of modules %d\n", i2c_db_p->num_modules);

    sx_dbg_dump_print_empty_line(m);

    snprintf(header, SX_DBG_DUMP_HEADER_MAX_LEN, "Module I2c mapping dump - device_id %u", dev->device_id);
    sx_dbg_dump_print_header(m, header);

    if (!priv->module_eeprom_access_db.async_mod_access_db.parallel_mod_access_initialized) {
        seq_printf(m, "Parallel MCIA access DB is not initialized or has been deinitialized\n");
        goto out;
    }

    for (i2c_index = 0; i2c_index < i2c_db_p->num_i2c_buses; i2c_index++) {
        seq_printf(m, "I2C index %d, Modules Number %d, Modules Connected: ",
                   i2c_index,  i2c_db_p->i2c_buses_db[i2c_index].num_modules_mapped);
        for (i = 0; i < i2c_db_p->i2c_buses_db[i2c_index].num_modules_mapped; i++) {
            seq_printf(m, "[%d]", i2c_db_p->i2c_buses_db[i2c_index].mapped_modules[i]);
        }
        sx_dbg_dump_print_empty_line(m);
    }

    sx_dbg_dump_print_empty_line(m);
out:
    return 0;
}


static int __module_i2c_utilization_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_priv         *priv;
    char                    header[SX_DBG_DUMP_HEADER_MAX_LEN];
    struct sx_dev          *dev = sx_dbg_dump_get_device(m);
    uint32_t                i2c_index = 0;
    i2c_bus_mapping_db_t   *i2c_db_p = NULL;
    async_mode_access_db_t *async_mod_access_db_p = NULL;

    if (!dev) {
        goto out;
    }
    priv = sx_priv(dev);

    i2c_db_p = &(priv->module_eeprom_access_db.async_mod_access_db.i2c_bus_mapping_db);

    async_mod_access_db_p = &(priv
                              ->module_eeprom_access_db.async_mod_access_db);

    snprintf(header, SX_DBG_DUMP_HEADER_MAX_LEN, "Module I2c utilization dump - device_id %u", dev->device_id);
    sx_dbg_dump_print_header(m, header);

    if (!priv->module_eeprom_access_db.async_mod_access_db.parallel_mod_access_initialized) {
        seq_printf(m, "Parallel MCIA access DB is not initialized or has been deinitialized\n");
        goto out;
    }

    sx_dbg_dump_print_table_header(m, "| %-9s | %-9s | %-9s | %-9s |", "I2C index", "RX", "TX", "Timeout");

    for (i2c_index = 0; i2c_index < i2c_db_p->num_i2c_buses; i2c_index++) {
        mutex_lock(&i2c_db_p->i2c_buses_db[i2c_index].i2c_counters_mutex);
        seq_printf(m, "| %-9d | %-9lld | %-9lld  | %-9lld |\n",
                   i2c_index,
                   i2c_db_p->i2c_buses_db[i2c_index].rx_counter,
                   i2c_db_p->i2c_buses_db[i2c_index].tx_counter,
                   i2c_db_p->i2c_buses_db[i2c_index].timeout_counter);

        mutex_unlock(&i2c_db_p->i2c_buses_db[i2c_index].i2c_counters_mutex);
    }

    sx_dbg_dump_print_empty_line(m);

    seq_printf(
        m,
        "Watermark stateful MCIA db entries in SDK: %d%%\n",
        async_mod_access_db_p->parallel_mcia_access_db.completions_num ?
        (async_mod_access_db_p->parallel_mcia_access_db.max_allocated_completions * 100 /
         async_mod_access_db_p->parallel_mcia_access_db.completions_num) : 0);

    sx_dbg_dump_print_empty_line(m);
out:
    return 0;
}


static int __module_parallel_mcia_access_db_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_priv         *priv = NULL;
    int                     i;
    struct sx_dev          *dev = sx_dbg_dump_get_device(m);
    char                    header[SX_DBG_DUMP_HEADER_MAX_LEN];
    async_mode_access_db_t *async_mod_access_db_p = NULL;

    if (!dev) {
        goto out;
    }
    priv = sx_priv(dev);

    async_mod_access_db_p = &(priv
                              ->module_eeprom_access_db.async_mod_access_db);

    snprintf(header, SX_DBG_DUMP_HEADER_MAX_LEN, "Parallel (async) MCIA Access dump - device_id %u", dev->device_id);
    sx_dbg_dump_print_header(m, header);

    if (!priv->module_eeprom_access_db.async_mod_access_db.parallel_mod_access_initialized) {
        seq_printf(m, "Parallel MCIA access DB is not initialized or has been deinitialized\n");
        goto out;
    }

    seq_printf(
        m,
        "-------------------------------------------------------------------------------\n\n");
    if (!priv->module_eeprom_async_access_mode) {
        seq_printf(m, "Parallel MCIA access: disabled\n");
        goto out;
    }
    seq_printf(m, "Parallel MCIA access: enabled\n");

    seq_printf(m, "Parallel MCIA access: Initialized\n");

    mutex_lock(&async_mod_access_db_p->parallel_mcia_access_db.async_db_mutex);
    seq_printf(
        m,
        "allocated_completions: %d\n",
        async_mod_access_db_p->parallel_mcia_access_db.allocated_completions);
    seq_printf(
        m,
        "active_parallel_mcia_sessions: %d\n",
        async_mod_access_db_p->parallel_mcia_access_db
        .active_parallel_mcia_sessions);
    seq_printf(
        m,
        "total_parallel_mcia_sessions: %d\n\n",
        async_mod_access_db_p->parallel_mcia_access_db
        .total_parallel_mcia_sessions);
    seq_printf(
        m,
        "timeout_session_counter: %llu\n",
        async_mod_access_db_p->parallel_mcia_access_db
        .timeout_session_counter);
    seq_printf(
        m,
        "async_request_err_counter: %llu\n",
        async_mod_access_db_p->parallel_mcia_access_db
        .async_request_err_counter);
    seq_printf(
        m,
        "unmatched_trap_counter: %llu\n",
        async_mod_access_db_p->parallel_mcia_access_db
        .unmatched_trap_counter);
    seq_printf(
        m, "unused_trap_counter: %llu\n",
        async_mod_access_db_p->parallel_mcia_access_db.unused_trap_counter);
    seq_printf(
        m,
        "matched_trap_counter: %llu\n",
        async_mod_access_db_p->parallel_mcia_access_db.matched_trap_counter);
    seq_printf(
        m,
        "duplicated_trap_counter: %llu\n",
        async_mod_access_db_p->parallel_mcia_access_db
        .duplicated_trap_counter);
    seq_printf(
        m,
        "unknown_trap_counter: %llu\n",
        async_mod_access_db_p->parallel_mcia_access_db.unknown_trap_counter);
    seq_printf(
        m,
        "\n%-14s   %-14s   %-14s   %-14s   %-14s   %-14s   %-14s   %-14s   %-14s\n",
        "Index", "Flag", "Module", "I2C", "Page", "Bank", "Size",
        "Dev_Addr", "Caller_TID");
    seq_printf(
        m,
        "..................................................................................................................................................\n");
    for (i = 0; i < async_mod_access_db_p->parallel_mcia_access_db.completions_num; i++) {
        if (async_mod_access_db_p->parallel_mcia_access_db.completions[i].flag ==
            PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_UNUSED) {
            continue;
        }
        seq_printf(
            m,
            "%-14d   %-14d   %-14d   0x%-12x   %-14d   %-14d   %-14d   %-14d   %-14d\n",
            i,
            async_mod_access_db_p->parallel_mcia_access_db.completions[i]
            .flag,
            async_mod_access_db_p->parallel_mcia_access_db.completions[i]
            .key.module,
            async_mod_access_db_p->parallel_mcia_access_db.completions[i]
            .key.i2c_addr,
            async_mod_access_db_p->parallel_mcia_access_db.completions[i]
            .key.page,
            async_mod_access_db_p->parallel_mcia_access_db.completions[i]
            .key.bank,
            async_mod_access_db_p->parallel_mcia_access_db.completions[i]
            .key.size,
            async_mod_access_db_p->parallel_mcia_access_db.completions[i]
            .key.dev_addr,
            async_mod_access_db_p->parallel_mcia_access_db.completions[i]
            .caller_tid);
    }
    mutex_unlock(
        &async_mod_access_db_p->parallel_mcia_access_db.async_db_mutex);

out:  sx_dbg_dump_print_empty_line(m);
    return 0;
}

static int __module_eeprom_access_db_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_priv *priv = NULL;
    int             i;
    char            header[SX_DBG_DUMP_HEADER_MAX_LEN];
    struct sx_dev  *dev = sx_dbg_dump_get_device(m);


    if (!dev) {
        goto out;
    }
    priv = sx_priv(dev);

    snprintf(header, SX_DBG_DUMP_HEADER_MAX_LEN, "Module eeprom access dump - device_id %u", dev->device_id);
    sx_dbg_dump_print_header(m, header);

    seq_printf(
        m,
        "-------------------------------------------------------------------------------\n");
    seq_printf(m, "module_eeprom_async_access_mode: %s\n",
               priv->module_eeprom_async_access_mode ? "enabled" : "disabled");
    seq_printf(m, "module_eeprom_max_access_size: %d\n",
               priv->module_eeprom_max_access_size);
    seq_printf(m, "-------------------------------------\n");

    seq_printf(m, "\n%-14s   %-14s\n", "Module", "EEPROM Type");
    seq_printf(m, ".....................................\n");

    mutex_lock(&priv->module_eeprom_access_db.access_mutex);
    for (i = 0; i < MAX_MODULE_NUM; i++) {
        if (priv->module_eeprom_access_db.eeprom_types[i] == EEPROM_MANAGEMENT_TYPE_INVALID) {
            continue;
        }
        seq_printf(m, "%-14d   0x%-12x\n", i,
                   priv->module_eeprom_access_db.eeprom_types[i]);
    }
    mutex_unlock(&priv->module_eeprom_access_db.access_mutex);

    sx_dbg_dump_print_empty_line(m);
out:
    return 0;
}


static int __sp_list_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_priv *priv;
    char            header[SX_DBG_DUMP_HEADER_MAX_LEN];
    struct sx_dev  *dev = sx_dbg_dump_get_device(m);
    struct sx_cq   *iter_cq;
    uint32_t        index = 0;

    if (!dev) {
        goto out;
    }
    priv = sx_priv(dev);
    if (!priv) {
        goto out;
    }

    snprintf(header, SX_DBG_DUMP_HEADER_MAX_LEN, "Strict prio list dump - device_id %u", dev->device_id);
    sx_dbg_dump_print_header(m, header);

    sx_dbg_dump_print_table_header(m, "| %-9s | %-9s | %-9s |", "#", "CQ #", "budget");

    spin_lock_bh(&priv->cq_threads.lock);
    list_for_each_entry(iter_cq, &priv->cq_threads.sp_list, sp_list) {
        seq_printf(m, "| %-9d | %-9d | %-9d |\n",
                   index,
                   iter_cq->cqn,
                   iter_cq->budget);
        index++;
    }
    spin_unlock_bh(&priv->cq_threads.lock);

    sx_dbg_dump_print_empty_line(m);

out:
    return 0;
}

static int __cq_dump(struct seq_file *m, void *v, void *context)
{
    u8                  cqn = 0;
    unsigned long       flags;
    struct sx_cq_table *cq_table;
    long                now = jiffies;
    struct sx_priv     *priv;
    struct sx_cq       *cq;
    u64                 cons_index, diff;
    char                header[SX_DBG_DUMP_HEADER_MAX_LEN];
    struct sx_dev     * dev = sx_dbg_dump_get_device(m);
    char                flags_str[10] = { 0 };
    char                arm_str[20] = { 0 };
    char                event_str[20] = { 0 };
    char                tc_weight_str[20] = { 0 };
    bool                is_sdq;
    int                 num_flags;

    if (!dev) {
        goto out;
    }
    priv = sx_priv(dev);
    if (!priv) {
        goto out;
    }

    cq_table = &priv->cq_table;
    if (!cq_table) {
        goto out;
    }

    snprintf(header, SX_DBG_DUMP_HEADER_MAX_LEN, "CQ dump - device_id %u", dev->device_id);
    sx_dbg_dump_print_header(m, header);

    sx_dbg_dump_print_table_header(m,
                                   "%-14s | %-5s | %-12s | %-9s | %-12s | %-12s | %-5s | %-9s | %-4s | %s",
                                   "CQ #",
                                   "flags",
                                   "thread",
                                   "tc/weight",
                                   "cons_idx",
                                   "diff",
                                   "nent",
                                   "buff_size",
                                   "CQEv",
                                   "arm/event (sec)");

    spin_lock_irqsave(&cq_table->lock, flags);

    for (cqn = 0; cqn < priv->dev_cap.max_num_cqs; cqn++) {
        if (cqn == NUMBER_OF_SDQS) {
            sx_dbg_dump_print_separate_line(m);
        }

        cq = cq_table->cq[cqn];
        if (!cq) {
            continue;
        }

        is_sdq = (cqn < NUMBER_OF_SDQS);
        cons_index = cq->cons_index_dbg;
        diff = cons_index - cq->cons_index_dbg_snapshot;
        cq->cons_index_dbg_snapshot = cons_index;
        num_flags = 0;

        if (cq->dq->is_monitor) {
            flags_str[num_flags++] = 'M';
        }
        if (cq->paused) {
            flags_str[num_flags++] = 'P';
        }
        flags_str[num_flags] = '\0';

        if (cq->strict_prio) {
            snprintf(tc_weight_str, sizeof(tc_weight_str) - 1, "SP TC%02d", cq->budget);
        } else {
            snprintf(tc_weight_str, sizeof(tc_weight_str) - 1, "DWRR %d", cq->budget);
        }

        if (cq->armed_ts > 0) {
            snprintf(arm_str, sizeof(arm_str) - 1, "%u", jiffies_to_msecs(now - cq->armed_ts) / 1000);
        } else {
            snprintf(arm_str, sizeof(arm_str) - 1, "NA");
        }

        if (cq->event_ts > 0) {
            snprintf(event_str, sizeof(event_str) - 1, "%u", jiffies_to_msecs(now - cq->event_ts) / 1000);
        } else {
            snprintf(event_str, sizeof(event_str) - 1, "NA");
        }

        seq_printf(m,
                   "CQ %02u (%s %02u) | %-5s | %-12s | %-9s | %-12llu | %-12llu | %-5u | %-9u | %-4u | %s/%s\n",
                   cqn,
                   (is_sdq ? "SDQ" : "RDQ"),
                   cq->dq->dqn,
                   flags_str,
                   sx_cq_thread_name(cq->cq_thread_id),
                   tc_weight_str,
                   cons_index,
                   diff,
                   cq->nent,
                   (is_sdq ? 0 : priv->profile.pci_profile.rdq_properties[cq->dq->dqn].entry_size),
                   cq->cqe_version,
                   arm_str, event_str);
    }

    spin_unlock_irqrestore(&cq_table->lock, flags);
    sx_dbg_dump_print_empty_line(m);

out:
    return 0;
}

static int __ber_monitor_dump(struct seq_file *m, void *v, void *context)
{
    sx_core_ber_monitor_dump(m, v, context);
    return 0;
}

static int __tele_thrs_dump(struct seq_file *m, void *v, void *context)
{
    u16             i = 0;
    unsigned long   flags;
    char            header[SX_DBG_DUMP_HEADER_MAX_LEN];
    struct sx_dev * dev = sx_dbg_dump_get_device(m);

    if (!dev) {
        return 0;
    }
    snprintf(header, SX_DBG_DUMP_HEADER_MAX_LEN, "TELE threshold dump - device_id %u", dev->device_id);
    sx_dbg_dump_print_header(m, header);

    sx_dbg_dump_print_table_header(m, "%-11s| %-11s| %s", "local_port", "state", "tc_pg_vec");
    spin_lock_irqsave(&sx_priv(dev)->db_lock, flags);

    for (i = 0; i < MAX_PHYPORT_NUM + 1; i++) {
        if (sx_priv(dev)->tele_thrs_state[i][TELE_DIR_ING_EGRESS_E]) {
            seq_printf(m, "%-11u| %-11u| [tc] %llx\n",
                       i,
                       sx_priv(dev)->tele_thrs_state[i][TELE_DIR_ING_EGRESS_E],
                       sx_priv(dev)->tele_thrs_tc_vec[i][TELE_DIR_ING_EGRESS_E]);
        }
        if (sx_priv(dev)->tele_thrs_state[i][TELE_DIR_ING_INGRESS_E]) {
            seq_printf(m, "%-11u| %-11u| [pg] %llx\n",
                       i,
                       sx_priv(dev)->tele_thrs_state[i][TELE_DIR_ING_INGRESS_E],
                       sx_priv(dev)->tele_thrs_tc_vec[i][TELE_DIR_ING_INGRESS_E]);
        }
    }
    spin_unlock_irqrestore(&sx_priv(dev)->db_lock, flags);

    sx_dbg_dump_print_empty_line(m);
    return 0;
}

static void __monitor_rdq_dump_one(struct seq_file *m, struct sx_dev *dev, int rdq_n)
{
    struct sx_priv     *priv = sx_priv(dev);
    struct sx_dq_table *rdq_table = &priv->rdq_table;
    struct sx_dq       *rdq;

    if (rdq_table == NULL) {
        seq_printf(m, "rdq table is empty\n");
        return;
    }

    if (rdq_n > (priv->dev_cap.max_num_rdqs - 1)) {
        seq_printf(m, "rdq_n %d is out of range [0..%d]\n",
                   rdq_n, priv->dev_cap.max_num_rdqs);
        return;
    }


    if (!rdq_table->dq[rdq_n]) {
        return;
    }

    rdq = rdq_table->dq[rdq_n];

    if (!rdq->is_monitor) {
        return;
    }

    seq_printf(m, "rdq %d data:  is_mon:%d, rx_cnt:%u, rx_start:%u, cpu_tc:%d \n",
               rdq_n,
               rdq->is_monitor,                  /* rdq is cyclic monitor rdq. non valid for sdq */
               rdq->mon_rx_count,
               rdq->mon_rx_start,
               rdq->cpu_tclass);
}

static int __monitor_rdq_dump(struct seq_file *m, void *v, void *context)
{
    int             rdq_n;
    struct sx_priv *priv = NULL;
    char            header[SX_DBG_DUMP_HEADER_MAX_LEN];
    struct sx_dev * dev = sx_dbg_dump_get_device(m);

    if (!dev) {
        return 0;
    }
    priv = sx_priv(dev);

    snprintf(header, SX_DBG_DUMP_HEADER_MAX_LEN, "Monitor RDQ dump - device_id %u", dev->device_id);
    sx_dbg_dump_print_header(m, header);

    for (rdq_n = 0; rdq_n < priv->dev_cap.max_num_rdqs; rdq_n++) {
        __monitor_rdq_dump_one(m, dev, rdq_n);
    }

    sx_dbg_dump_print_empty_line(m);
    return 0;
}

static void __rdq_usage_dump_one(struct seq_file *m, struct sx_dev *dev, int rdq_n)
{
    struct sx_priv     *priv = sx_priv(dev);
    struct sx_dq_table *rdq_table = &priv->rdq_table;
    struct sx_dq       *rdq;

    if (rdq_table == NULL) {
        seq_printf(m, "rdq table is empty\n");
        return;
    }

    if (rdq_n > (priv->dev_cap.max_num_rdqs - 1)) {
        seq_printf(m, "rdq_n %d is out of range [0..%d]\n",
                   rdq_n, priv->dev_cap.max_num_rdqs);
        return;
    }


    if (!rdq_table->dq[rdq_n]) {
        return;
    }

    rdq = rdq_table->dq[rdq_n];
    seq_printf(m, "rdq %d usage: occupancy:%lu, watermark:%lu \n",
               rdq_n, rdq->recv_occupancy, rdq->recv_watermark);
}

static int __rdq_usage_dump(struct seq_file *m, void *v, void *context)
{
    int             rdq_n;
    struct sx_priv *priv = NULL;
    char            header[SX_DBG_DUMP_HEADER_MAX_LEN];
    struct sx_dev * dev = sx_dbg_dump_get_device(m);

    if (!dev) {
        return 0;
    }
    priv = sx_priv(dev);

    snprintf(header,
             SX_DBG_DUMP_HEADER_MAX_LEN,
             "RDQ usage dump (monitor interval %u ms) - device_id %u",
             priv->rdq_use_monitor.interval_ms,
             dev->device_id);
    sx_dbg_dump_print_header(m, header);

    for (rdq_n = 0; rdq_n < priv->dev_cap.max_num_rdqs; rdq_n++) {
        __rdq_usage_dump_one(m, dev, rdq_n);
    }

    sx_dbg_dump_print_empty_line(m);
    return 0;
}

static int __rdq_usage_clear(int argc, const char *argv[], struct sx_dev *dev, void *context)
{
    struct sx_priv *priv = sx_priv(dev);
    int             rc;
    int             rdqn;
    unsigned long   flags;

    if (argc != 3) {
        return -EINVAL;
    }

    /* dev_id (argv[1]) is obsolete. to maintain backwards compatibility, we just ignore it */

    rc = kstrtoint(argv[2], 10, &rdqn);
    if (rc) {
        return -EINVAL;
    }

    if ((rdqn < 0) || (rdqn >= NUMBER_OF_RDQS)) {
        return -EINVAL;
    }

    sxd_log_notice(dev,
                   "%s: clear rdq %d usage info, dev_id: %u\n",
                   __func__,
                   rdqn,
                   dev->device_id);

    if (priv->rdq_table.dq[rdqn]) {
        spin_lock_irqsave(&(priv->rdq_table.dq[rdqn]->lock), flags);
        priv->rdq_table.dq[rdqn]->recv_watermark = 0;
        priv->rdq_table.dq[rdqn]->recv_occupancy = 0;
        spin_unlock_irqrestore(&(priv->rdq_table.dq[rdqn]->lock), flags);
    }

    return 0;
}

static int __rdq_usage_timer_interval_set(int argc, const char *argv[], struct sx_dev *dev, void *context)
{
    struct sx_priv *priv = sx_priv(dev);
    int             rc;
    int             interval;
    unsigned long   flags;

    if (argc != 3) {
        return -EINVAL;
    }

    /* dev_id (argv[1]) is obsolete. to maintain backwards compatibility, we just ignore it */

    rc = kstrtoint(argv[2], 10, &interval);
    if (rc) {
        return -EINVAL;
    }

    if ((interval < RDQ_USAGE_MONITOR_TIMER_INTERVAL_MIN_MS) || (interval > RDQ_USAGE_MONITOR_TIMER_INTERVAL_MAX_MS)) {
        sxd_log_notice(dev, "%s: rdq usage monitor timer value %u is out of range [%d ~ %d].\n",
                       __func__,
                       interval,
                       RDQ_USAGE_MONITOR_TIMER_INTERVAL_MIN_MS,
                       RDQ_USAGE_MONITOR_TIMER_INTERVAL_MAX_MS);
        return -ERANGE;
    }

    sxd_log_notice(dev, "%s: try to set rdq usage monitor timer value to %u, dev_id: %u\n",
                   __func__,
                   interval,
                   dev->device_id);

    spin_lock_irqsave(&priv->ctx_lock, flags);
    priv->rdq_use_monitor.interval_ms = interval;
    spin_unlock_irqrestore(&priv->ctx_lock, flags);


    return 0;
}

static int __unreg_hft_streaming_genl_family(int argc, const char *argv[], struct sx_dev *dev, void *context)
{
    struct sx_priv *priv = sx_priv(dev);
    int             ret = 0;
    int             i = 0;
    int             found = 0;

    if (argc != 3) {
        sxd_log_err(dev, "unreg_hft_streaming_genl_family: invalid number of arguments\n");
        ret = -EINVAL;
        goto out;
    }

    /* dev_id (argv[1]) is obsolete. to maintain backwards compatibility, we just ignore it */

    sxd_log_notice(dev,
                   "%s: To un-register dev(%u) generic netlink family '%s'.\n",
                   __func__,
                   dev->device_id,
                   argv[2]);
    for (i = 0; i < SXD_BULK_CNTR_STREAMING_PROFILE_MAX_NUM; i++) {
        if (priv->bulk_counters.streaming_profiles[i].is_allocated) {
            if (strncmp(priv->bulk_counters.streaming_profiles[i].genl_family.name,
                        argv[2],
                        sizeof(priv->bulk_counters.streaming_profiles[i].genl_family.name)) == 0) {
                found = 1;
                break;
            }
        }
    }

    if (found == 0) {
        sxd_log_err(dev,
                    "unreg_hft_streaming_genl_family: no generic netlink family '%s\' is registered.\n",
                    argv[2]);
        ret = -EINVAL;
        goto out;
    }
    ret = genl_unregister_family(&priv->bulk_counters.streaming_profiles[i].genl_family);
    if (ret) {
        sxd_log_err(dev, "Failed to unregister generic Netlink family (%s) with ret = %d\n",
                    argv[2], ret);
        goto out;
    }

    sxd_log_notice(dev, "OK to un-register generic Netlink family (%s).\n", argv[2]);

out:
    return ret;
}

static int __rdq_usage_toggle(int argc, const char *argv[], struct sx_dev *dev, void *context)
{
    bool disable = false;

    if (argc < 3) {
        sxd_log_notice(dev, "RDQ-monitoring: debug command - invalid number of arguments\n");
        return -EINVAL;
    }

    /* dev_id (argv[1]) is obsolete. to maintain backwards compatibility, we just ignore it */

    /* argv[2] is the command */
    if (strcmp(argv[2], "enable") == 0) {
        disable = false;
    } else if (strcmp(argv[2], "disable") == 0) {
        disable = true;
    } else {
        sxd_log_notice(dev, "RDQ-monitoring: debug command - invalid command [%s]\n", argv[2]);
        return -EINVAL;
    }

    sx_priv(dev)->rdq_use_monitor.disabled = disable;
    return 0;
}

static int __dbg_wjh_counters_clear(int argc, const char *argv[], struct sx_dev *dev, void *context)
{
    struct sx_priv      *priv = sx_priv(dev);
    union sx_event_data *event_data = NULL;

    event_data = vzalloc(sizeof(*event_data));
    if (!event_data) {
        sxd_log_warning(dev, "failed to allocate event data for WJH clear counters function\n");
        return 0;
    }

    sx_core_reset_dm_err_counters(priv);
    sx_core_dispatch_event(dev, SX_DEV_EVENT_CLEAR_WJH_COUNTERS, event_data);
    return 0;
}

static int __cpu_port_loopback_toggle_write(int argc, const char *argv[], struct sx_dev *dev, void *context)
{
    struct sx_priv *priv = sx_priv(dev);

    if (argc != 2) {
        sxd_log_notice(dev, "CPU-port-loopback: debug command - invalid number of arguments\n");
        return -EINVAL;
    }

    /* argv[1] is the command */
    if (strcmp(argv[1], "enable") == 0) {
        priv->debug_only.enable_cpu_port_loopback = true;
    } else if (strcmp(argv[1], "disable") == 0) {
        priv->debug_only.enable_cpu_port_loopback = false;
    } else {
        sxd_log_notice(dev, "CPU-port-loopback: debug command - invalid command [%s]\n", argv[1]);
        return -EINVAL;
    }

    sxd_log_info(dev,
                 "setting CPU port loopback (debug feature) to %s on device %u\n",
                 argv[1],
                 dev->device_id);

    return 0;
}

static int __cpu_port_loopback_toggle_read(struct seq_file *m, void *v, void *context)
{
    struct sx_dev  *dev = sx_dbg_dump_get_device(m);
    struct sx_priv *priv = sx_priv(dev);

    seq_printf(m, "%d\n", (priv->debug_only.enable_cpu_port_loopback ? 1 : 0));
    return 0;
}


static int __reset_info_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_priv *priv = NULL;
    char            header[SX_DBG_DUMP_HEADER_MAX_LEN];
    struct sx_dev  *dev = sx_dbg_dump_get_device(m);

    if (!dev) {
        return 0;
    }
    priv = sx_priv(dev);

    snprintf(header, SX_DBG_DUMP_HEADER_MAX_LEN, "Reset info dump - device_id %u", dev->device_id);
    sx_dbg_dump_print_header(m, header);

    seq_printf(m, "Last chip reset type ........ %s\n", sx_reset_type_str(priv->reset_info.last_chip_reset_type));
    seq_printf(m, "Last reset duration ......... %u msec\n", priv->reset_info.duration_msec);
    seq_printf(m, "System status offset ........ 0x%x\n", priv->reset_info.system_status_mem_offset);
    sx_dbg_dump_print_empty_line(m);
    return 0;
}

static int __sysfs_dev_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_priv *priv = NULL;
    char            header[SX_DBG_DUMP_HEADER_MAX_LEN];
    struct sx_dev  *dev = sx_dbg_dump_get_device(m);

    if (!dev) {
        return 0;
    }
    priv = sx_priv(dev);

    snprintf(header, SX_DBG_DUMP_HEADER_MAX_LEN, "sysfs device dump - device_id %u", dev->device_id);
    sx_dbg_dump_print_header(m, header);

    down_read(&priv->sysfs_access_info.rw_lock);
    seq_printf(m, "Number of holders .............. %d\n", atomic_read(&priv->sysfs_access_info.num_of_readers));
    seq_printf(m, "Disable command (if any) ....... %s\n",
               ((priv->sysfs_access_info.locked_by_writer) ? priv->sysfs_access_info.writer_desc : "N/A"));

    if (priv->sysfs_access_info.locker_ts) {
        seq_printf(m, "Oldest locker time ............. %u (msecs ago)\n",
                   jiffies_to_msecs(jiffies - priv->sysfs_access_info.locker_ts));
    }
    up_read(&priv->sysfs_access_info.rw_lock);

    sx_dbg_dump_print_empty_line(m);
    return 0;
}

extern int fast_boot;
extern int skip_reset;
extern int enable_keep_packet_crc;
static int __sx_core_info_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_dev  *dev = sx_dbg_dump_get_device(m);
    struct sx_priv *priv = dev ? sx_priv(dev) : NULL;

    sx_dbg_dump_print_header(m, "sx_core information dump");

    seq_printf(m, "Kernel version ........................................ %d.%d.%d\n",
               ((LINUX_VERSION_CODE >> 16) & 0xff), /* major */
               ((LINUX_VERSION_CODE >> 8) & 0xff),  /* minor */
               (LINUX_VERSION_CODE & 0xff));        /* sub-minor */
    seq_printf(m, "Kconfig\n");
    seq_printf(m, "    CONFIG_NET_DEVLINK ................................ %s\n",
               IS_ENABLED(CONFIG_NET_DEVLINK) ? "yes" : "no");
    seq_printf(m,
               "    CONFIG_NET_SWITCHDEV ................................. %s\n",
               IS_ENABLED(CONFIG_NET_SWITCHDEV) ? "yes" : "no");
    seq_printf(m, "sx_core parameters\n");
    seq_printf(m, "    fast_boot ......................................... %d\n", fast_boot);
    seq_printf(m, "    skip_reset ........................................ %d\n", skip_reset);
    seq_printf(m, "    enable_keep_packet_crc ............................ %d\n",
               MODULE_PARAM_INSTANCE_GET(priv, enable_keep_packet_crc));

    sx_rx_buffers_config_dump(m);

    if (priv) {
        seq_printf(m, "sx_priv\n");
        seq_printf(m, "    drop_ib_mads ................................ %d\n", priv->drop_ib_mads);
    }

    sx_dbg_dump_print_empty_line(m);
    return 0;
}

static int __sdq_dump(struct seq_file *m, void *v, void *context)
{
    u16             sdqn = 0;
    unsigned long   now = jiffies;
    struct sx_dq   *sdq = NULL;
    struct sx_priv *priv = NULL;
    char            max_comp_ts[20];
    char            last_full[20];
    char            last_comp[20];
    char            header[SX_DBG_DUMP_HEADER_MAX_LEN];
    struct sx_dev * dev = sx_dbg_dump_get_device(m);

    if (!dev) {
        return 0;
    }
    priv = sx_priv(dev);

    snprintf(header, SX_DBG_DUMP_HEADER_MAX_LEN, "SDQ dump - device_id %u", dev->device_id);
    sx_dbg_dump_print_header(m, header);

    seq_printf(m,
               "%-5s   %-12s   %-19s   %-27s   %-51s   %-27s\n"
               "%-5s   %-12s   %-8s   %-8s   %-12s   %-12s   %-6s   %-12s   %-12s   %-12s   %-12s   %-12s\n"
               "---------------------------------------------------------------------------------"
               "------------------------------------------------------------------------\n",
               "|SDQ#",
               "|Sent",
               "|SW Queue",
               "|Completion Count",
               "|Max Diff",
               "|Last",
               "|    ",
               "|    ",
               "|Level",
               "Drops",
               "|OK",
               "Late",
               "|Msec",
               "TS (sec ago)",
               "Watermark",
               "Occupancy",
               "|Full",
               "Comp");

    for (sdqn = 0; sdqn < priv->dev_cap.max_num_sdqs; sdqn++) {
        sdq = priv->sdq_table.dq[sdqn];

        if (sdq) {
            spin_lock_bh(&sdq->lock);

            if (sdq->last_full_queue != 0) {
                snprintf(last_full, sizeof(last_full), "%u", jiffies_to_msecs(now - sdq->last_full_queue) / 1000);
            } else {
                snprintf(last_full, sizeof(last_full), "N/A");
            }

            if (sdq->last_completion != 0) {
                snprintf(last_comp, sizeof(last_comp), "%u", jiffies_to_msecs(now - sdq->last_completion) / 1000);
            } else {
                snprintf(last_comp, sizeof(last_comp), "N/A");
            }

            if (sdq->max_comp_ts != 0) {
                snprintf(max_comp_ts, sizeof(max_comp_ts), "%u", jiffies_to_msecs(now - sdq->max_comp_ts) / 1000);
            } else {
                snprintf(max_comp_ts, sizeof(max_comp_ts), "N/A");
            }

            seq_printf(m,
                       "|%-5u  |%-12llu  |%-8u  %-8llu   |%-12llu  %-12llu   |%-6u  %-12s   %-14lu  %-12ld  |%-12s  %-12s\n",
                       sdqn,
                       sdq->pkts_sent_to_sdq,
                       sdq->pkts_in_sw_queue,
                       sdq->pkts_sw_queue_drops,
                       sdq->pkts_recv_completion,
                       sdq->pkts_late_completion,
                       jiffies_to_msecs(sdq->max_comp_time),
                       max_comp_ts,
                       sdq->send_watermark,
                       sdq->send_occupancy,
                       last_full,
                       last_comp);

            spin_unlock_bh(&sdq->lock);
        }
    }

    sx_dbg_dump_print_empty_line(m);
    return 0;
}

static int __sdq_max_diff_clear(int argc, const char *argv[], struct sx_dev *dev, void *context)
{
    struct sx_priv *priv = sx_priv(dev);
    int             rc;
    int             sdqn;

    if (argc != 3) {
        return -EINVAL;
    }

    /* dev_id (argv[1]) is obsolete. to maintain backwards compatibility, we just ignore it */

    rc = kstrtoint(argv[2], 10, &sdqn);
    if (rc) {
        return -EINVAL;
    }

    if ((sdqn < 0) || (sdqn >= NUMBER_OF_SDQS)) {
        return -EINVAL;
    }

    if (priv->sdq_table.dq[sdqn]) {
        priv->sdq_table.dq[sdqn]->max_comp_time = 0;
        priv->sdq_table.dq[sdqn]->max_comp_ts = 0;
    }

    return 0;
}

static int __rdq_to_filter_ebpf_prog_dump(struct seq_file *m, void *v, void *context)
{
    u16             i = 0;
    u16             rdq = 0;
    struct sx_priv *priv = NULL;
    unsigned long   flags;
    struct sx_dev * dev = sx_dbg_dump_get_device(m);

    if (!dev) {
        return 0;
    }
    priv = sx_priv(dev);

    seq_printf(m, "RDQ to filter eBPF program mapping dump\n");
    seq_printf(m,
               "------------------------------------------------------------------------------------------------------------------------\n");
    sx_dbg_dump_print_table_header(m, "%-7s| %-7s| %-20s", "#", "RDQ", "Filter eBPF program");

    spin_lock_irqsave(&sx_priv(dev)->rdq_table.lock, flags);

    i = 0;
    for (rdq = 0; rdq < (NUMBER_OF_RDQS); rdq++) {
        if (priv->wjh_info.filter_ebpf_progs[rdq] != NULL) {
            i++;
            seq_printf(m, "%-7u| %-7u| %-20p\n",
                       i,
                       rdq,
                       priv->wjh_info.filter_ebpf_progs[rdq]);
        }
    }

    spin_unlock_irqrestore(&sx_priv(dev)->rdq_table.lock, flags);

    sx_dbg_dump_print_empty_line(m);
    return 0;
}

static int __rdq_to_aggregation_ebpf_prog_dump(struct seq_file *m, void *v, void *context)
{
    u16             i = 0;
    u16             rdq = 0;
    u16             ebpf_prog_num = 0;
    struct sx_priv *priv = NULL;
    unsigned long   flags;
    struct sx_dev * dev = sx_dbg_dump_get_device(m);

    if (!dev) {
        return 0;
    }
    priv = sx_priv(dev);

    seq_printf(m, "RDQ to aggregation eBPF program mapping dump\n");
    seq_printf(m,
               "------------------------------------------------------------------------------------------------------------------------\n");
    sx_dbg_dump_print_table_header(m, "%-7s| %-7s| %-20s", "#", "RDQ", "Aggregation eBPF program");

    spin_lock_irqsave(&sx_priv(dev)->rdq_table.lock, flags);

    i = 0;
    for (rdq = 0; rdq < (NUMBER_OF_RDQS); rdq++) {
        for (ebpf_prog_num = 0; ebpf_prog_num < SX_AGG_EBPF_PROG_NUM_PER_RDQ; ebpf_prog_num++) {
            if (priv->wjh_info.agg_ebpf_progs[rdq][ebpf_prog_num] != NULL) {
                i++;
                seq_printf(m, "%-7u| %-7u| %-20p\n",
                           i,
                           rdq,
                           priv->wjh_info.agg_ebpf_progs[rdq][ebpf_prog_num]);
            }
        }
    }

    spin_unlock_irqrestore(&sx_priv(dev)->rdq_table.lock, flags);

    sx_dbg_dump_print_empty_line(m);
    return 0;
}

static int __wjh_info_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_priv      *priv = NULL;
    char                 header[SX_DBG_DUMP_HEADER_MAX_LEN];
    struct sx_dev       *dev = sx_dbg_dump_get_device(m);
    union sx_event_data *event_data = NULL;

    if (!dev) {
        return 0;
    }

    event_data = vzalloc(sizeof(*event_data));
    if (!event_data) {
        sxd_log_warning(dev, "failed to allocate event data for WJH dump function\n");
        return 0;
    }

    event_data->wjh_dump_info.m = m;
    priv = sx_priv(dev);

    snprintf(header, SX_DBG_DUMP_HEADER_MAX_LEN, "WJH info dump - device_id %u", dev->device_id);
    sx_dbg_dump_print_header(m, header);

    seq_printf(m,
               "Enable monitor RDQ trace points ........ %s\n",
               priv->wjh_info.enable_monitor_rdq_trace_points ? "yes" : "no");
    seq_printf(m,
               "Enable drop monitor report ............. %s\n",
               priv->wjh_info.enable_drop_monitor_report ? "yes" : "no");
    seq_printf(m, "Drop monitor HW report callback ........ %p\n", priv->wjh_info.sx_net_dm_hw_report);
    seq_printf(m, "Drop monitor listener .................. %p\n", priv->wjh_info.drop_monitor_listener);
    seq_printf(m, "Buffer drop trap ID .................... %u\n", priv->wjh_info.buffer_drop_trap_id);
    sx_dbg_dump_print_empty_line(m);

    seq_printf(m, "Drop monitor counters\n");
    seq_printf(m, "    Drop monitor is disabled ................................. %llu\n",
               atomic64_read(&priv->wjh_info.dm_err_counters.dm_disabled));
    seq_printf(m, "    Drop monitor is inactive ................................. %llu\n",
               atomic64_read(&priv->wjh_info.dm_err_counters.dm_inactive));
    seq_printf(m, "    Total packets got to drop-monitor entry point ............ %llu\n",
               atomic64_read(&priv->wjh_info.dm_err_counters.in_dm_hw_report_entry_point));
    seq_printf(m, "        From monitor RDQ handler ............................. %llu\n",
               atomic64_read(&priv->wjh_info.dm_err_counters.from_monitor_rdq));
    seq_printf(m, "        From SW queue handler ................................ %llu\n",
               atomic64_read(&priv->wjh_info.dm_err_counters.from_sw_queue));
    seq_printf(m, "        From user-channel drop-monitor ....................... %llu\n",
               atomic64_read(&priv->wjh_info.dm_err_counters.from_user_channel_dm));
    seq_printf(m, "    Buffer clone failed ...................................... %llu\n",
               atomic64_read(&priv->wjh_info.dm_err_counters.skb_clone_failed));
    seq_printf(m, "    Buffer handling failed ................................... %llu\n",
               atomic64_read(&priv->wjh_info.dm_err_counters.skb_handling_failed));
    seq_printf(m, "    Invalid trap ............................................. %llu\n",
               atomic64_read(&priv->wjh_info.dm_err_counters.invalid_trap));
    seq_printf(m, "    No HW report callback .................................... %llu\n",
               atomic64_read(&priv->wjh_info.dm_err_counters.no_hw_report_cb));

    sx_core_dispatch_event(dev, SX_DEV_EVENT_DUMP_WJH_COUNTERS, event_data);
    sx_dbg_dump_print_empty_line(m);
    __rdq_to_filter_ebpf_prog_dump(m, v, context);
    __rdq_to_aggregation_ebpf_prog_dump(m, v, context);
    sx_dbg_dump_print_empty_line(m);

    vfree(event_data);
    return 0;
}

static const char* __get_mod_support_type_str(u32 type)
{
    switch (type) {
    case SXD_MODULE_MASTER_MODE_FW_CONTROL_E:
        return "dependent";

    case SXD_MODULE_MASTER_MODE_SW_CONTROL_E:
        return "independent";

    case SXD_MODULE_MASTER_MODE_SW_CONTROL_NO_I2C_CONNECTIVITY_E:
        return "standalone";

    default:
        return "unknown";
    }
}

struct map_fa_dump_ctx {
    struct seq_file *m;
    u32              index;
};

static bool __map_fa_dump_cb(struct sx_dev *dev,
                             void          *buff,
                             dma_addr_t     dma_addr,
                             u32            num_of_pages,
                             bool           is_last,
                             void          *context)
{
    struct map_fa_dump_ctx *dump_ctx = (struct map_fa_dump_ctx*)context;

    seq_printf(dump_ctx->m, "    %u) dma_addr=%lx, pages=%u\n",
               dump_ctx->index,
               (unsigned long)dma_addr,
               num_of_pages);

    dump_ctx->index++;
    return false; /* continue iteration */
}

static int __fw_and_board_info_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_priv        *priv;
    struct sx_fw          *fw;
    char                   header[SX_DBG_DUMP_HEADER_MAX_LEN];
    struct sx_dev        * dev = sx_dbg_dump_get_device(m);
    const char            *fw_branch_tag = "N/A";
    struct map_fa_dump_ctx ctx;

    if (!dev) {
        return 0;
    }
    priv = sx_priv(dev);
    fw = &priv->fw;

    if (((char)priv->dev_info.dev_info_ro.mgir.dev_info.dev_branch_tag[0]) != '\0') {
        fw_branch_tag = (const char*)priv->dev_info.dev_info_ro.mgir.dev_info.dev_branch_tag;
    }

    snprintf(header, SX_DBG_DUMP_HEADER_MAX_LEN, "FW and BoardInfo dump - device_id %u", dev->device_id);
    sx_dbg_dump_print_header(m, header);

    seq_printf(m, "PCI bus ............................. %s\n", dev_name(&priv->dev.pdev->dev));
    seq_printf(m, "chip-type ........................... %d\n", priv->dev_info.dev_info_ro.chip_type);
    seq_printf(m, "fw-status ........................... %d\n", priv->dev_info.dev_info_ro.fw_boot_status);
    seq_printf(m, "MGIR (chip/rev) ..................... %u (0x%04x)/0x%x\n",
               priv->dev_info.dev_info_ro.mgir.hw_info.device_id,
               priv->dev_info.dev_info_ro.mgir.hw_info.device_id,
               priv->dev_info.dev_info_ro.mgir.hw_info.device_hw_revision);
    seq_printf(m, "PSID ................................ %s\n", priv->board_id);
    seq_printf(m, "SDK version ......................... %s\n",
#if defined(SX_SDK_VERSION)
               (strlen(SX_SDK_VERSION) > 0) ? SX_SDK_VERSION : "<Empty>");
#else
               "N/A");
#endif

    seq_printf(m, "FW release version .................. %d.%d.%04d [dev_branch_tag: %s]\n",
               (int)(priv->dev_info.dev_info_ro.mgir.fw_info.extended_major),
               (int)(priv->dev_info.dev_info_ro.mgir.fw_info.extended_minor),
               (int)(priv->dev_info.dev_info_ro.mgir.fw_info.extended_sub_minor),
               fw_branch_tag);
    seq_printf(m, "FW release date ..................... %02x/%02x/%04x %02x:%02x\n",
               fw->fw_day,
               fw->fw_month,
               fw->fw_year,
               fw->fw_hour,
               fw->fw_minutes);
    seq_printf(m, "mod_support_cap ..................... %s\n",
               __get_mod_support_type_str(SX_GET_FW_CAP(priv, SX_FW_CAP_MOD_SUPPORT_MASK,
                                                        SX_FW_CAP_MOD_SUPPORT_OFFSET)));
    seq_printf(m, "mod_support_use ..................... %s\n",
               __get_mod_support_type_str(priv->independent_module_params.module_support_type));
    seq_printf(m, "clear_interrupt_base ................ 0x%llx\n", fw->clr_int_base);
    seq_printf(m, "catas_offset ........................ 0x%llx\n", fw->catas_offset);
    seq_printf(m, "catas_size .......................... 0x%x\n", fw->catas_size);
    seq_printf(m, "debug_trace ......................... %u\n", fw->debug_trace);
    seq_printf(m, "smkey ............................... %u\n", fw->smkey);
    seq_printf(m, "doorbell_page_offset ................ 0x%llx\n", fw->doorbell_page_offset);
    seq_printf(m, "frc_offset .......................... 0x%llx\n", fw->frc_offset);
    seq_printf(m, "utc_sec_offset ...................... 0x%llx\n", fw->utc_sec_offset);
    seq_printf(m, "utc_nsec_offset ..................... 0x%llx\n", fw->utc_nsec_offset);
    seq_printf(m, "core_clock .......................... %u\n", fw->core_clock);
    seq_printf(m, "fw_pages ............................ %u\n", fw->fw_pages);
    seq_printf(m, "local_out_mb_offset ................. 0x%x\n", fw->local_out_mb_offset);
    seq_printf(m, "local_out_mb_size ................... %u\n", fw->local_out_mb_size);
    seq_printf(m, "local_in_mb_offset .................. 0x%x\n", fw->local_in_mb_offset);
    seq_printf(m, "local_in_mb_size .................... %u\n", fw->local_in_mb_size);
    seq_printf(m, "system_status_mem offset ............ 0x%x\n", priv->reset_info.system_status_mem_offset);
    seq_printf(m, "cr_dump_offset ...................... 0x%llx\n", fw->cr_dump_offset);
    seq_printf(m, "cap_dump_host_size_flat ............. %u\n", fw->cap_dump_host_size_flat);
    seq_printf(m, "cap_dump_host_size_reduced_flat ..... %u\n", fw->cap_dump_host_size_reduced_flat);
    seq_printf(m, "cap_dump_host_size_gdb .............. %u\n", fw->cap_dump_host_size_gdb);
    seq_printf(m, "cap_dump_host_size_gw ............... %u\n", fw->cap_dump_host_size_gw);
    seq_printf(m, "mmam_support ........................ %u\n", !!priv->mmam_support);
    seq_printf(m, "cpo_indication ...................... %s\n",
               priv->dev_info.dev_info_ro.mgir.hw_info.cpo_indication ? "TRUE" : "FALSE");
    seq_printf(m, "tec_support ......................... %u\n", !!priv->tec_support);
    seq_printf(m, "apply_im_supported .................. %u\n", priv->apply_im_supported);
    seq_printf(m, "mrsr6_is_supported .................. %u\n", priv->reset_cap.mrsr6_is_supported);
    seq_printf(m, "mrsr6_with_sbr_is_supported by FW ... %u\n", priv->reset_cap.mrsr6_with_sbr_is_supported);
    seq_printf(m, "mrsrft_is_supported by FW ........... %u\n", priv->reset_cap.mrsrft_is_supported);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
    seq_printf(m, "SBR is supported by kernel .......... 1\n");
#else /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) */
    seq_printf(m, "SBR is supported by kernel .......... 0\n");
#endif /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) */

    ctx.m = m;

    ctx.index = 0;
    seq_printf(m, "MAP_FA (FW trace)\n");
    sx_map_fa_item_iterate(priv->fw.map_fa_items[SXD_MAP_MEMORY_TYPE_FW_LOAD_E], __map_fa_dump_cb, &ctx);
    if (ctx.index == 0) {
        seq_printf(m, "    N/A\n");
    }

    ctx.index = 0;
    seq_printf(m, "MAP_FA (Port Profile)\n");
    sx_map_fa_item_iterate(priv->fw.map_fa_items[SXD_MAP_MEMORY_TYPE_PORT_PROFILE_MEM_E], __map_fa_dump_cb, &ctx);
    if (ctx.index == 0) {
        seq_printf(m, "    N/A\n");
    }

    ctx.index = 0;
    seq_printf(m, "MAP_FA (S2S Latency)\n");
    sx_map_fa_item_iterate(priv->fw.map_fa_items[SXD_MAP_MEMORY_TYPE_SWITCH_TO_SWITCH_LATENCY_E],
                           __map_fa_dump_cb,
                           &ctx);
    if (ctx.index == 0) {
        seq_printf(m, "    N/A\n");
    }

    ctx.index = 0;
    seq_printf(m, "MAP_FA (BFD TX)\n");
    sx_map_fa_item_iterate(priv->fw.map_fa_items[SXD_MAP_MEMORY_TYPE_BFD_TX_OFFLOAD_E],
                           __map_fa_dump_cb,
                           &ctx);
    if (ctx.index == 0) {
        seq_printf(m, "    N/A\n");
    }

    ctx.index = 0;
    seq_printf(m, "MAP_FA (BFD RX)\n");
    sx_map_fa_item_iterate(priv->fw.map_fa_items[SXD_MAP_MEMORY_TYPE_BFD_RX_OFFLOAD_E],
                           __map_fa_dump_cb,
                           &ctx);
    if (ctx.index == 0) {
        seq_printf(m, "    N/A\n");
    }

    sx_dbg_dump_print_empty_line(m);
    return 0;
}

static int __loopback_ports_dump(struct seq_file *m, void *v, void *context)
{
    char            header[SX_DBG_DUMP_HEADER_MAX_LEN];
    struct sx_dev * dev = sx_dbg_dump_get_device(m);
    struct sx_priv *priv;
    int             i;

    if (!dev) {
        return 0;
    }
    priv = sx_priv(dev);

    snprintf(header,
             SX_DBG_DUMP_HEADER_MAX_LEN,
             "Loopback ports dump - device_id %u",
             dev->device_id);

    if (!priv->loopback_ports.valid) {
        seq_printf(m, "The loopback-ports database has not been initialized yet\n");
        return 0;
    }

    sx_dbg_dump_print_header(m, header);
    sx_dbg_dump_print_table_header(m, "%-10s   %-9s", "local_port", "module_id");

    for (i = 0; i < priv->loopback_ports.info.num_of_ports; i++) {
        seq_printf(m, "%-10u   %-9u\n",
                   priv->loopback_ports.info.loopback_ports[i].local_port,
                   priv->loopback_ports.info.loopback_ports[i].module_id);
    }

    return 0;
}

static int __sx_sysfs_dev_set_proc_handler(int argc, const char *argv[], struct sx_dev *dev, void *context)
{
    int         hold_msecs, err = 0;
    const char *status_str = "success";

    if (argc < 3) {
        return -EINVAL;
    }

    /* dev_id (argv[1]) is obsolete. to maintain backwards compatibility, we just ignore it */

    if (strcmp(argv[2], "enable") == 0) {
        sx_core_sysfs_dev_enable(dev);
    } else if (strcmp(argv[2], "disable") == 0) {
        sx_core_sysfs_dev_disable(dev, "debug (/proc/dbg_dump/sysfs_dev_set)");
    } else if (strcmp(argv[2], "hold_and_release") == 0) {
        if (argc < 4) {
            status_str = "expecting param: hold_msecs";
            err = -EINVAL;
            goto out;
        }

        err = kstrtoint(argv[3], 10, &hold_msecs);
        if (err) {
            status_str = "invalid hold_msecs value";
            err = -EINVAL;
            goto out;
        }

        err = sx_core_sysfs_dev_hold(dev);
        if (err) {
            status_str = "the device is currently disabled, operation failed";
            goto out;
        }

        msleep(hold_msecs);

        sx_core_sysfs_dev_release(dev);
    } else {
        err = -EINVAL;
        status_str = "invalid argument (operation)";
    }

out:
    sxd_log_notice(dev, "sysfs-dev proc handler: dev=%d, op=%s, err=%d, status='%s'\n",
                   dev->device_id, argv[2], err, status_str);
    return err;
}

static int __sx_cr_dump_cmd_handler(int argc, const char *argv[], struct sx_dev *dev, void *context)
{
    bool disable = false;
    bool flat_disable = false;
    bool reduced_flat_disable = false;

    if (argc < 3) {
        sxd_log_notice(dev, "CR-Dump: debug command - invalid number of arguments\n");
        return -EINVAL;
    }

    /* argv[1] is the command */
    if (strcmp(argv[1], "enable") == 0) {
        disable = false;
    } else if (strcmp(argv[1], "disable") == 0) {
        disable = true;
    } else if (strcmp(argv[1], "flat_disable") == 0) {
        flat_disable = true;
    } else if (strcmp(argv[1], "reduced_flat_disable") == 0) {
        reduced_flat_disable = true;
    } else {
        sxd_log_notice(dev, "CR-Dump: debug command - invalid command [%s]\n", argv[1]);
        return -EINVAL;
    }

    /* dev_id (argv[2]) is obsolete. to maintain backwards compatibility, we just ignore it */

    if (disable) {
        sx_priv(dev)->dev_info.dev_info_ro.flags |= SX_DEV_INFO_F_DEBUG_CR_DUMP_DISABLED;
    } else if (flat_disable) {
        sx_priv(dev)->dev_info.dev_info_ro.flags |= SX_DEV_INFO_F_DEBUG_CR_DUMP_FLAT_DISABLED;
    } else if (reduced_flat_disable) {
        sx_priv(dev)->dev_info.dev_info_ro.flags |= SX_DEV_INFO_F_DEBUG_CR_DUMP_FLAT_REDUCED_DISABLED;
    } else {
        sx_priv(dev)->dev_info.dev_info_ro.flags &= ~SX_DEV_INFO_F_DEBUG_CR_DUMP_DISABLED;
        sx_priv(dev)->dev_info.dev_info_ro.flags &= ~SX_DEV_INFO_F_DEBUG_CR_DUMP_FLAT_DISABLED;
        sx_priv(dev)->dev_info.dev_info_ro.flags &= ~SX_DEV_INFO_F_DEBUG_CR_DUMP_FLAT_REDUCED_DISABLED;
    }

    return 0;
}

static int __hft_streaming_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_dev                             *dev = sx_dbg_dump_get_device(m);
    struct sx_priv                            *priv = sx_priv(dev);
    int                                        i = 0;
    int                                        j = 0;
    sxd_bulk_cntr_streaming_profile_attrs_t   *streaming_profiles = priv->bulk_counters.streaming_profiles;
    sxd_bulk_cntr_streaming_buffer_id_attrs_t *streaming_buffer_ids = priv->bulk_counters.streaming_buffer_ids;

    sx_dbg_dump_print_header(m, "hft streaming DB dump");

    seq_printf(m, "Profiles:\n");
    for (i = 0; i < SXD_BULK_CNTR_STREAMING_PROFILE_MAX_NUM; i++) {
        if (!streaming_profiles[i].is_allocated) {
            continue;
        }
        seq_printf(m, "Profile ID %d\n", i);
        seq_printf(m, "\tProtocol type: %d\n", streaming_profiles[i].attrs.protocol_type);
        seq_printf(m, "\tNet namespace inode: %u\n", streaming_profiles[i].net_ns->ns.inum);
        seq_printf(m, "\tGeneric Netlink family: %s\n", streaming_profiles[i].genl_family.name);
        seq_printf(m, "\tGeneric Netlink MC groups:\n");
        for (j = 0; j < SXD_BULK_CNTR_STREAMING_GENETLINK_MC_GROUP_NUM; j++) {
            if (streaming_profiles[i].genl_mcgrps[j].name[0] != 0) {
                seq_printf(m, "\t\t%s\n", streaming_profiles[i].genl_mcgrps[j].name);
            }
        }
        seq_printf(m, "\t Format type: %d\n", streaming_profiles[i].attrs.format_type);
        seq_printf(m, "\t Format data:\n");
        seq_printf(m, "\t\t version: %d\n", streaming_profiles[i].attrs.format_data.format_data_ipfix.version);
        seq_printf(m,
                   "\t\t observation_domain_id: %d\n",
                   streaming_profiles[i].attrs.format_data.format_data_ipfix.observation_domain_id);
        seq_printf(m, "\t\t bulk_size: %d\n", streaming_profiles[i].attrs.format_data.format_data_ipfix.bulk_size);
        seq_printf(m,
                   "\t\t ipfix_msgs_num: %d\n",
                   streaming_profiles[i].attrs.format_data.format_data_ipfix.ipfix_msgs_num);
        for (j = 0; j < streaming_profiles[i].attrs.format_data.format_data_ipfix.ipfix_msgs_num; j++) {
            seq_printf(m, "\t\t\t template_id : %d, length: %d\n",
                       streaming_profiles[i].attrs.format_data.format_data_ipfix.ipfix_msgs_list[j].template_id,
                       streaming_profiles[i].attrs.format_data.format_data_ipfix.ipfix_msgs_list[j].length);
        }
    }

    seq_printf(m, "Buffers:\n");
    for (i = 0; i < SXD_BULK_CNTR_STREAMING_BUFFER_ID_MAX_NUM; i++) {
        if (!streaming_buffer_ids[i].is_allocated) {
            continue;
        }
        seq_printf(m, "Buffer ID %d\n", i);
        seq_printf(m, "\t is_buffer_allocated: %d\n", streaming_buffer_ids[i].is_buffer_allocated);
        seq_printf(m, "\t in_use: %d\n", streaming_buffer_ids[i].in_use);
        seq_printf(m, "\t curr_sample_err: %u\n", streaming_buffer_ids[i].curr_sample_err);
        seq_printf(m, "\t type: %u\n", streaming_buffer_ids[i].type);
        seq_printf(m, "\t buffer_size: %u\n", streaming_buffer_ids[i].buffer_size);
        seq_printf(m, "\t profile_id: %u\n", streaming_buffer_ids[i].profile_id);
        seq_printf(m, "\t alloc_err: %u\n", streaming_buffer_ids[i].buffer_alloc_err);
        seq_printf(m, "\t sample_err: %u\n", streaming_buffer_ids[i].sample_err);
        seq_printf(m, "\t ipfix_num: %u\n", streaming_buffer_ids[i].ipfix_num);
        seq_printf(m, "\t running_ipfix_idx: %u\n", streaming_buffer_ids[i].running_ipfix_idx);
    }

    sx_dbg_dump_print_empty_line(m);
    return 0;
}

bool sx_dbg_init(struct sx_dev *dev, void *context)
{
    sx_dbg_dump_write_handler_register("sdq_max_diff_clear",
                                       "sdq_max_diff_clear <dev_id> <sdq_num>",
                                       "Clear max diff between transmitted packet time and completion time",
                                       __sdq_max_diff_clear,
                                       dev,
                                       context);
    sx_dbg_dump_write_handler_register("qos_counters_clear",
                                       "<no parameters>",
                                       "Clear EMAD Quality of Service counters",
                                       sx_qos_emad_counter_clear,
                                       dev,
                                       context);
    sx_dbg_dump_write_handler_register("emergency_chip_reset",
                                       "emergency_chip_reset <dev_id>",
                                       "perform emergency reset (legacy / pci-toggle)",
                                       sx_emergency_reset_proc_handler,
                                       dev,
                                       context);
    sx_dbg_dump_write_handler_register("sysfs_dev_set",
                                       "<dev_id> <enable|disable|hold_and_release> [hold_msecs]",
                                       "enables/disables sysfs access to a device",
                                       __sx_sysfs_dev_set_proc_handler,
                                       dev,
                                       context);
    sx_dbg_dump_write_handler_register("health_check",
                                       "health_check <mute|dont_stop_on_fatal|stop|normal> <dev_id>\n"
                                       "health_check <set_debug_state_flags|unset_debug_state_flags> <dev_id> <flags_in_hex>\n"
                                       "  * debug_state_flags: " SX_HEALTH_DEBUG_STATE_FLAGS "\n"
                                       "health_check <set_dev_checks|unset_dev_checks> <dev_id> <checks_bitmask_in_hex>\n"
                                       "health_check <set_global_checks|unset_global_checks> <checks_bitmask_in_hex>\n"
                                       "  * Per-dev check bits: " SX_PER_DEV_CHECKS "\n",
                                       "health check debug commands",
                                       sx_health_check_dbg_cmd_handler,
                                       dev,
                                       context);
    sx_dbg_dump_write_handler_register("emad_timeout_set",
                                       "<timeout_usec>",
                                       "Set EMAD timeout for device (in microseconds, e.g., 20000000 for 20 sec)",
                                       sx_emad_timeout_dbg_cmd_handler,
                                       dev,
                                       context);
    sx_dbg_dump_write_handler_register("cr_dump",
                                       "cr_dump <enable|disable|flat_disable|reduced_flat_disable> <dev_id>\n",
                                       "enable all / disable all / disable flat only / disable reduced only CR dump",
                                       __sx_cr_dump_cmd_handler,
                                       dev,
                                       context);
    sx_dbg_dump_write_handler_register("rdq_usage_clear",
                                       "rdq_usage_clear <dev_id> <rdq_num>\n",
                                       "Clear rdq usage for specific rdq",
                                       __rdq_usage_clear,
                                       dev,
                                       context);
    sx_dbg_dump_write_handler_register("rdq_usage_timer_interval",
                                       "rdq_usage_timer_interval <dev_id> <timer_ms_value>\n",
                                       "Set interval for rdq usage monitor timer",
                                       __rdq_usage_timer_interval_set,
                                       dev,
                                       context);
    sx_dbg_dump_write_handler_register("unreg_hft_streaming_genl_family",
                                       "unreg_hft_streaming_genl_family <dev_id> <family_name>\n",
                                       "Unregister a hft streaming generic netlink family",
                                       __unreg_hft_streaming_genl_family,
                                       dev,
                                       context);
    sx_dbg_dump_write_handler_register("rdq_usage_toggle",
                                       "rdq_usage_toggle <dev_id> <enable|disable>\n",
                                       "enable/disable RDQ usage monitor",
                                       __rdq_usage_toggle,
                                       dev,
                                       context);
    sx_dbg_dump_write_handler_register("cpu_port_loopback_toggle",
                                       "cpu_port_loopback_toggle <enable|disable>\n",
                                       "enable/disable CPU port loopback (debug feature)",
                                       __cpu_port_loopback_toggle_write,
                                       dev,
                                       context);
    sx_dbg_dump_write_handler_register("parallel_mcia_access_db_statistics_clear",
                                       "<no parameters>",
                                       "clear parallel MCIA access DB statistics",
                                       sx_module_parallel_mcia_access_db_statistics_clear,
                                       dev,
                                       context);
    sx_dbg_dump_write_handler_register("wjh_counters_clear",
                                       "<no parameters>",
                                       "clear WJH counters",
                                       __dbg_wjh_counters_clear,
                                       dev,
                                       context);
    sx_dbg_dump_read_handler_register("cq_dump", __cq_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("sp_list_dump", __sp_list_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("cmd_ifc_dump", sx_cmd_ifc_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("ber_monitor_dump", __ber_monitor_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("tele_thrs_dump", __tele_thrs_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("ptp_dump", sx_dbg_ptp_dump_proc_show, NULL, dev, context);
    sx_dbg_dump_read_handler_register("clock_dump", sx_dbg_clock_dump_proc_show, NULL, dev, context);
    sx_dbg_dump_read_handler_register("clock_and_ptp_log_dump",
                                      sx_dbg_clock_and_ptp_log_dump_proc_show,
                                      NULL,
                                      dev,
                                      context);
    sx_dbg_dump_read_handler_register("monitor_rdq_dump", __monitor_rdq_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("sdq_dump", __sdq_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("dq_info_dump", sx_dq_info_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("rdq_usage_dump", __rdq_usage_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("wjh_info_dump", __wjh_info_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("fw_and_board_info_dump", __fw_and_board_info_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("mmap_dump", sx_mmap_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("bulk_cntr_dump", bulk_cntr_db_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("accuflow_dump", sx_af_counters_dump, sx_af_counters_dump_size, dev, context);
    sx_dbg_dump_read_handler_register("dev_db_dump", sx_dev_db_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("health_check_dump", sx_health_check_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("interface_dump", sx_intf_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("synce_dump", sx_core_synce_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("qos_emad_dump", sx_qos_emad_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("reset_info_dump", __reset_info_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("sysfs_dev_dump", __sysfs_dev_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("multi_asic_module_sysfs_dump", sx_multi_asic_module_sysfs_dump, NULL, dev,
                                      context);
    sx_dbg_dump_read_handler_register("module_sysfs_dump", sx_module_sysfs_dump, NULL, dev, NULL);
    sx_dbg_dump_read_handler_register("independent_module_db_dump",
                                      sx_sysfs_independent_module_db_dump,
                                      NULL,
                                      dev,
                                      context);
    sx_dbg_dump_read_handler_register("loopback_ports_dump", __loopback_ports_dump, NULL, dev, context);
    sx_dbg_dump_read_handler_register("sx_core_info_dump",
                                      __sx_core_info_dump,
                                      NULL,
                                      dev,
                                      context);
    sx_dbg_dump_read_handler_register("hft_streaming_dump",
                                      __hft_streaming_dump,
                                      NULL,
                                      dev,
                                      context);
    sx_dbg_dump_read_handler_register("cpu_port_loopback_toggle",
                                      __cpu_port_loopback_toggle_read,
                                      NULL,
                                      dev,
                                      context);

    sx_dbg_dump_read_handler_register("parallel_mcia_access_db_dump",
                                      __module_parallel_mcia_access_db_dump,
                                      NULL,
                                      dev,
                                      NULL);
    sx_dbg_dump_read_handler_register("module_eeprom_access_dump", __module_eeprom_access_db_dump, NULL, dev, NULL);
    sx_dbg_dump_read_handler_register("module_i2c_utilization_dump", __module_i2c_utilization_dump, NULL, dev, NULL);
    sx_dbg_dump_read_handler_register("module_i2c_mapping_dump", __module_i2c_mapping_dump, NULL, dev, NULL);

    sx_dbg_host_ifc_init(dev);
    sx_dbg_db_init(dev);
    sx_core_counters_init(dev);
    sx_emad_dump_hanlders_per_device_init(dev, context);
    sx_dpt_dump_hanlders_per_device_init(dev, context);

    return true;
}

bool sx_dbg_deinit(struct sx_dev *dev, void *context)
{
    sx_dpt_dump_hanlders_per_device_deinit(dev, context);
    sx_emad_dump_hanlders_per_device_deinit(dev, context);
    sx_core_counters_deinit(dev);
    sx_dbg_db_deinit(dev);
    sx_dbg_host_ifc_deinit(dev);

    sx_dbg_dump_read_handler_unregister("cq_dump", dev);
    sx_dbg_dump_read_handler_unregister("sp_list_dump", dev);
    sx_dbg_dump_read_handler_unregister("cmd_ifc_dump", dev);
    sx_dbg_dump_read_handler_unregister("ber_monitor_dump", dev);
    sx_dbg_dump_read_handler_unregister("tele_thrs_dump", dev);
    sx_dbg_dump_read_handler_unregister("ptp_dump", dev);
    sx_dbg_dump_read_handler_unregister("clock_dump", dev);
    sx_dbg_dump_read_handler_unregister("clock_and_ptp_log_dump", dev);
    sx_dbg_dump_read_handler_unregister("monitor_rdq_dump", dev);
    sx_dbg_dump_read_handler_unregister("sdq_dump", dev);
    sx_dbg_dump_read_handler_unregister("dq_info_dump", dev);
    sx_dbg_dump_read_handler_unregister("rdq_usage_dump", dev);
    sx_dbg_dump_read_handler_unregister("wjh_info_dump", dev);
    sx_dbg_dump_read_handler_unregister("fw_and_board_info_dump", dev);
    sx_dbg_dump_read_handler_unregister("mmap_dump", dev);
    sx_dbg_dump_read_handler_unregister("bulk_cntr_dump", dev);
    sx_dbg_dump_read_handler_unregister("accuflow_dump", dev);
    sx_dbg_dump_read_handler_unregister("dev_db_dump", dev);
    sx_dbg_dump_read_handler_unregister("health_check_dump", dev);
    sx_dbg_dump_read_handler_unregister("interface_dump", dev);
    sx_dbg_dump_read_handler_unregister("synce_dump", dev);
    sx_dbg_dump_read_handler_unregister("qos_emad_dump", dev);
    sx_dbg_dump_read_handler_unregister("reset_info_dump", dev);
    sx_dbg_dump_read_handler_unregister("sysfs_dev_dump", dev);
    sx_dbg_dump_write_handler_unregister("sdq_max_diff_clear", dev);
    sx_dbg_dump_write_handler_unregister("qos_counters_clear", dev);
    sx_dbg_dump_write_handler_unregister("emergency_chip_reset", dev);
    sx_dbg_dump_write_handler_unregister("sysfs_dev_set", dev);
    sx_dbg_dump_write_handler_unregister("health_check", dev);
    sx_dbg_dump_write_handler_unregister("cr_dump", dev);
    sx_dbg_dump_write_handler_unregister("rdq_usage_clear", dev);
    sx_dbg_dump_write_handler_unregister("rdq_usage_timer_interval", dev);
    sx_dbg_dump_write_handler_unregister("rdq_usage_toggle", dev);
    sx_dbg_dump_write_handler_unregister("cpu_port_loopback_toggle", dev);
    sx_dbg_dump_write_handler_unregister("parallel_mcia_access_db_statistics_clear", dev);
    sx_dbg_dump_read_handler_unregister("multi_asic_module_sysfs_dump", dev);
    sx_dbg_dump_read_handler_unregister("module_sysfs_dump", dev);
    sx_dbg_dump_read_handler_unregister("independent_module_db_dump", dev);
    sx_dbg_dump_read_handler_unregister("loopback_ports_dump", dev);
    sx_dbg_dump_read_handler_unregister("sx_core_info_dump", dev);
    sx_dbg_dump_read_handler_unregister("hft_streaming_dump", dev);
    sx_dbg_dump_read_handler_unregister("cpu_port_loopback_toggle", dev);
    sx_dbg_dump_read_handler_unregister("parallel_mcia_access_db_dump", dev);
    sx_dbg_dump_read_handler_unregister("module_eeprom_access_dump", dev);
    sx_dbg_dump_read_handler_unregister("module_i2c_utilization_dump", dev);
    sx_dbg_dump_read_handler_unregister("module_i2c_mapping_dump", dev);

    return true;
}
