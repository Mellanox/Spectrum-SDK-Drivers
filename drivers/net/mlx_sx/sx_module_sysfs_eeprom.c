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
#include <linux/if_vlan.h>
#include <linux/poll.h>
#include <linux/mlx_sx/auto_registers/reg.h>
#include <linux/mlx_sx/cmd.h>
#include <linux/mlx_sx/skb_hook.h>
#include <linux/seq_file.h>
#include <linux/ktime.h>
#include <linux/mlx_sx/auto_registers/reg.h>
#include <linux/mlx_sx/driver.h>
#include <linux/hashtable.h>
#include <linux/jhash.h>

#include "linux/mlx_sx/device.h"
#include "linux/stddef.h"
#include "sx.h"
#include "alloc.h"
#include "dev_init.h"
#include "dev_db.h"
#include "emad.h"
#include "sx_dbg_dump_proc.h"

/************************************************
 *  Define
 ***********************************************/
#define EEPROM_UPPER_PAGE_OFFSET       (0x80)
#define EEPROM_MODULE_PAGE_SELECT_ADDR 127
#define MCIA_DWOARD_BYTE1(dword) (((dword) >> 24) & 0xFF)
#define MCIA_DWOARD_BYTE2(dword) (((dword) >> 16) & 0xFF)
#define MCIA_DWOARD_BYTE3(dword) (((dword) >> 8) & 0xFF)
#define MCIA_DWOARD_BYTE4(dword) (((dword)) & 0xFF)

#define MCIA_BYTE1_TO_DWOARD(byte) (((byte) << 24) & 0xFF000000)
#define MCIA_BYTE2_TO_DWOARD(byte) (((byte) << 16) & 0x00FF0000)
#define MCIA_BYTE3_TO_DWOARD(byte) (((byte) << 8) & 0x0000FF00)
#define MCIA_BYTE4_TO_DWOARD(byte) ((byte) & 0x000000FF)

#define MCIA_ACCESS_MAX_RETRIES 3

/* Confirmed with Arch:
 *  - 32 is selected, if parallel number reach the number, the following request would become sync till the parallel session number decrease the value.
 *  - For the time consumption in the worst case: the max timeout of 32 request to the same module could cost 320ms (=32*10ms), so the SDK timeout 10sec should be enough.
 */
#define MAX_PARALLEL_MCIA_ACCESS_NUM 32
#define PARALLEL_MCIA_HASH_BITS      9


/* max time for FW returning MCIA trap */
#define MAX_MCIA_ACCESS_DURING_MSECS 10000  /* confirmed by FW's calculation. */

/* waiting time-out for SDK to be blocking state to wait notification */
#define ASYNC_MCIA_ACCESS_WAITING_TIMEOUT_MSECS (MAX_MCIA_ACCESS_DURING_MSECS)

#define IS_VALID_EEPROM_MANAGEMENT_TYPE(type) \
    ((type >= EEPROM_MANAGEMENT_TYPE_SFP) &&  \
     (type <= EEPROM_MANAGEMENT_TYPE_CMIS))

/* FW team confirm the HW I2C limitation that MCIA writing max size is <=126 bytes even if MCAM.mng_feature_cap_mask.bit32 supports 128 bytes. */
#define MCIA_WRITE_MAX_SIZE (126)

static ssize_t __module_sysfs_eeprom_bin_read(struct file          *flip,
                                              struct kobject       *kobj,
                                              struct bin_attribute *attr,
                                              char                 *buf,
                                              loff_t                pos,
                                              size_t                count);
static ssize_t __module_sysfs_eeprom_bin_write(struct file          *flip,
                                               struct kobject       *kobj,
                                               struct bin_attribute *attr,
                                               char                 *buf,
                                               loff_t                pos,
                                               size_t                count);

static struct bin_attribute module_eeprom_page0_attribute = __BIN_ATTR(data,
                                                                       (S_IRUGO | S_IWUSR),
                                                                       __module_sysfs_eeprom_bin_read,
                                                                       __module_sysfs_eeprom_bin_write,
                                                                       EEPROM_PAGE0_SIZE);
static struct bin_attribute module_eeprom_pagex_attribute = __BIN_ATTR(data,
                                                                       (S_IRUGO | S_IWUSR),
                                                                       __module_sysfs_eeprom_bin_read,
                                                                       __module_sysfs_eeprom_bin_write,
                                                                       EEPROM_UPPER_PAGE_SIZE);
static int __stateful_access_mcia(struct sx_dev *dev, struct ku_access_mcia_reg *reg_data, bool check_pnv);
static int __sx_core_get_module_page_valid(struct sx_dev *dev,
                                           uint8_t        module,
                                           uint8_t        slot,
                                           uint8_t        page,
                                           uint8_t       *pnv_p);
/************************************************
 *  Enum
 ***********************************************/
enum eeprom_module_info_id {
    EEPROM_MODULE_INFO_ID_SFP            = 0x03,
    EEPROM_MODULE_INFO_ID_QSFP           = 0x0C,
    EEPROM_MODULE_INFO_ID_QSFP_PLUS      = 0x0D,
    EEPROM_MODULE_INFO_ID_QSFP28         = 0x11,
    EEPROM_MODULE_INFO_ID_QSFP_DD        = 0x18,
    EEPROM_MODULE_INFO_ID_QSFP_8X        = 0x19,
    EEPROM_MODULE_INFO_ID_SFP_DD         = 0x1A,
    EEPROM_MODULE_INFO_ID_DSFP           = 0x1B,
    EEPROM_MODULE_INFO_ID_QSFP_PLUS_CMIS = 0x1E,
    EEPROM_MODULE_INFO_ID_ELS            = 0x22,
    EEPROM_MODULE_INFO_ID_OE             = 0x80,
};

enum mcia_module_status {
    MCIA_MODULE_STATUS_GOOD                  = 0,
    MCIA_MODULE_STATUS_NO_EEPROM_MODULE      = 0x1,
    MCIA_MODULE_STATUS_MODULE_NOT_SUPPORTED  = 0x2,
    MCIA_MODULE_STATUS_MODULE_NOT_CONNECTED  = 0x3,
    MCIA_MODULE_STATUS_MODULE_TYPE_INVALID   = 0x4,
    MCIA_MODULE_STATUS_MODULE_NOT_ACCESSIBLE = 0x5,
    MCIA_MODULE_STATUS_I2C_ERROR             = 0x9,
    MCIA_MODULE_STATUS_MODULE_DISABLED       = 0x10,
    MCIA_MODULE_STATUS_PAGE_ACCESS_FAILED    = 0x11,
    MCIA_MODULE_STATUS_LAST_STATUS           = MCIA_MODULE_STATUS_PAGE_ACCESS_FAILED + 1,
};
enum mcia_module_access_type {
    MCIA_MODULE_ACCESS_TYPE_READ  = 1,
    MCIA_MODULE_ACCESS_TYPE_WRITE = 2,
};
typedef enum parallel_mcia_access_completion_flag {
    PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_UNUSED    = 0,
    PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_ALLOCATED = 1,
    PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_TRAPPED   = 2   /* MCIA trap is received and to be handled */
} parallel_mcia_access_completion_flag_t;

static const char * const mcia_module_status_str_s[] = {
    [MCIA_MODULE_STATUS_GOOD] = "MCIA_MODULE_STATUS_GOOD",                                   \
    [MCIA_MODULE_STATUS_NO_EEPROM_MODULE] = "MCIA_MODULE_STATUS_NO_EEPROM_MODULE",           \
    [MCIA_MODULE_STATUS_MODULE_NOT_SUPPORTED] = "MCIA_MODULE_STATUS_MODULE_NOT_SUPPORTED",   \
    [MCIA_MODULE_STATUS_MODULE_NOT_CONNECTED] = "MCIA_MODULE_STATUS_MODULE_NOT_CONNECTED",   \
    [MCIA_MODULE_STATUS_MODULE_TYPE_INVALID] = "MCIA_MODULE_STATUS_MODULE_TYPE_INVALID",     \
    [MCIA_MODULE_STATUS_MODULE_NOT_ACCESSIBLE] = "MCIA_MODULE_STATUS_MODULE_NOT_ACCESSIBLE", \
    [MCIA_MODULE_STATUS_I2C_ERROR] = "MCIA_MODULE_STATUS_I2C_ERROR",                         \
    [MCIA_MODULE_STATUS_MODULE_DISABLED] = "MCIA_MODULE_STATUS_MODULE_DISABLED",             \
    [MCIA_MODULE_STATUS_PAGE_ACCESS_FAILED] = "MCIA_MODULE_STATUS_PAGE_ACCESS_FAILED",
};
static const char * const mcia_module_access_type_str_s[] = {
    [MCIA_MODULE_ACCESS_TYPE_READ] = "MCIA_MODULE_ACCESS_TYPE_READ", \
    [MCIA_MODULE_ACCESS_TYPE_WRITE] = "MCIA_MODULE_ACCESS_TYPE_WRITE",
};
#define MCIA_MODULE_STATUS_ARR_SIZE (sizeof(mcia_module_status_str_s) / sizeof(mcia_module_status_str_s[0]))
#define MCIA_MODULE_ACCESS_TYPE_ARR_SIZE     \
    (sizeof(mcia_module_access_type_str_s) / \
     sizeof(mcia_module_access_type_str_s[0]))

static const char * mcia_status_to_str(enum mcia_module_status status)
{
    if (status >= MCIA_MODULE_STATUS_LAST_STATUS) {
        return "MCIA_MODULE_STATUS_UNKNOWN";
    } else {
        return mcia_module_status_str_s[status];
    }
};

static const char * mcia_access_type_to_str(enum mcia_module_access_type access_type)
{
    if (access_type >= MCIA_MODULE_ACCESS_TYPE_ARR_SIZE) {
        return "MCIA_MODULE_ACCESS_TYPE_UNKNOWN";
    } else {
        return mcia_module_access_type_str_s[access_type];
    }
};
/************************************************
 * Type declarations
 ***********************************************/
struct page_limit_bytes_segment {
    int page;
    int begin;
    int end;
};

struct page_access_segment {
    int begin;
    int end;
};

/* parallel mcia access key structure with packed unsigned fields */
typedef struct __attribute__((__packed__)) parallel_mcia_access_key_t {
    uint8_t  module;
    uint8_t  i2c_addr;
    uint8_t  page;
    uint8_t  bank;
    uint16_t size;
    uint16_t dev_addr;
} parallel_mcia_access_key_t;

typedef struct parallel_mcia_access_completion {
    struct completion                      event_ready;
    parallel_mcia_access_key_t             key;
    parallel_mcia_access_completion_flag_t flag;
    int                                    index;
    struct ku_access_mcia_reg              mcia_access_reg;
    pid_t                                  caller_tid;
    struct hlist_node                      hash_node;
} parallel_mcia_access_completion_t;

typedef struct parallel_mcia_access_db {
    bool                              is_initialized;
    parallel_mcia_access_completion_t completions[MAX_PARALLEL_MCIA_ACCESS_NUM];
    struct mutex                      async_db_mutex;
    uint32_t                          allocated_completions;
    uint32_t                          active_parallel_mcia_sessions; /* Number of active parallel MCIA access sessions at the moment. */
    uint32_t                          total_parallel_mcia_sessions; /* Number of total parallel MCIA access sessions till now. */
    uint64_t                          timeout_session_counter; /* Number of parallel MCIA access sessions that is timed out. */
    uint64_t                          async_to_sync_session_counter; /* Number of parallel MCIA access sessions that is changed to sync session due to parallel session limit. */
    uint64_t                          async_request_err_counter; /* Number of parallel MCIA access sessions that request fails. */
    uint64_t                          unmatched_trap_counter; /* Number of parallel MCIA access sessions that is trapped but without matched session. */
    uint64_t                          unused_trap_counter; /* Number of parallel MCIA access sessions that is trapped and matched but no more used. */
    uint64_t                          matched_trap_counter; /* Number of parallel MCIA access sessions that is trapped and matched and still used. */
    uint64_t                          duplicated_trap_counter; /* Number of parallel MCIA access sessions that is trapped more than once. */
    uint64_t                          unknown_trap_counter; /* Number of parallel MCIA access sessions that is trapped with unknown session flag. */
    DECLARE_HASHTABLE(key_hash_table, PARALLEL_MCIA_HASH_BITS);
} parallel_mcia_access_db_t;

struct mcia_work {
    struct ku_mcia_reg mcia;
    struct work_struct w;
};
/* Parallel MCIA access DB is global for all modules in 1 ASIC or multiple ASICs.
 * We support duplicated eeprom access, so it is OK for same local module in case
 * of multiple ASICs. */
static parallel_mcia_access_db_t __parallel_mcia_access_db;
static struct workqueue_struct  *s_parallel_mod_access_wq_p = NULL; /* For parallel module access workqueue.*/
static bool                      s_parallel_mod_access_wq_initialized = false;
static DEFINE_MUTEX(s_parallel_mod_access_wq_init_mutex);
/************************************************
 * Globals
 ***********************************************/
/* entries within same page should be added adjacently and in sequence */
static struct page_limit_bytes_segment cmis_rd_limit_arr[] = {
    {0, 126, 127},
    {11, 134, 135}
};

static struct page_limit_bytes_segment cmis_wr_limit_arr[] = {
    {0, 26, 26},
    {0, 31, 36},
    {0, 126, 127},
    {10, 0, 255}
};

static struct page_limit_bytes_segment sff8636_rd_limit_arr[] = {
    {0, 3, 21},
    {0, 127, 127}
};

static struct page_limit_bytes_segment sff8636_wr_limit_arr[] = {
    {0, 86, 88},
    {0, 93, 93},
    {0, 98, 99},
    {0, 100, 106},
    {0, 127, 127},
    {3, 230, 251}
};

/* SFF8472's limitation bytes are only for i2c 0x51 */
static struct page_limit_bytes_segment sff8472_rd_limit_arr[] = {
    {0, 112, 113},
    {0, 116, 117},
    {0, 127, 127}
};

static struct page_limit_bytes_segment sff8472_wr_limit_arr[] = {
    {0, 110, 110},
    {0, 114, 115},
    {0, 118, 118},
    {0, 127, 127}
};

static int cmis_rd_limit_arr_len = ARRAY_SIZE(cmis_rd_limit_arr);
static int cmis_wr_limit_arr_len = ARRAY_SIZE(cmis_wr_limit_arr);
static int sff8636_rd_limit_arr_len = ARRAY_SIZE(sff8636_rd_limit_arr);
static int sff8636_wr_limit_arr_len = ARRAY_SIZE(sff8636_wr_limit_arr);
static int sff8472_rd_limit_arr_len = ARRAY_SIZE(sff8472_rd_limit_arr);
static int sff8472_wr_limit_arr_len = ARRAY_SIZE(sff8472_wr_limit_arr);

/************************************************
 *  Functions
 ***********************************************/

/* Simplified hash function using direct key hashing */
static uint32_t parallel_mcia_access_hash_key(const parallel_mcia_access_key_t *key)
{
    return jhash(key, sizeof(parallel_mcia_access_key_t), 0);
}

/* Compare two parallel mcia access keys */
static bool __parallel_mcia_access_key_equal(const parallel_mcia_access_key_t *key1,
                                             const parallel_mcia_access_key_t *key2)
{
    return memcmp(key1, key2, sizeof(parallel_mcia_access_key_t)) == 0;
}

/* Copy parallel mcia access key */
static void __copy_parallel_mcia_access_key(parallel_mcia_access_key_t *dest, const parallel_mcia_access_key_t *src)
{
    memcpy(dest, src, sizeof(parallel_mcia_access_key_t));
}

/* Initialize parallel mcia access related DB. */
static void __init_parallel_mcia_access_db(void)
{
    int i;

    mutex_init(&__parallel_mcia_access_db.async_db_mutex);
    __parallel_mcia_access_db.allocated_completions = 0;
    __parallel_mcia_access_db.active_parallel_mcia_sessions = 0;
    __parallel_mcia_access_db.total_parallel_mcia_sessions = 0;
    /* Initialize hash table using hash_init */
    hash_init(__parallel_mcia_access_db.key_hash_table);

    /* Initialize completion slots */
    for (i = 0; i < MAX_PARALLEL_MCIA_ACCESS_NUM; i++) {
        init_completion(&__parallel_mcia_access_db.completions[i].event_ready);
        __parallel_mcia_access_db.completions[i].index = i;
        __parallel_mcia_access_db.completions[i].flag = PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_UNUSED;
        memset(&__parallel_mcia_access_db.completions[i].key, 0, sizeof(parallel_mcia_access_key_t));
        INIT_HLIST_NODE(&__parallel_mcia_access_db.completions[i].hash_node);
    }
    __parallel_mcia_access_db.is_initialized = true;
}

/* Cleanup function to remove all entries from hash table. */
static void __deinit_parallel_mcia_access_db(void)
{
    parallel_mcia_access_completion_t *comp;
    struct hlist_node                 *tmp;
    int                                bkt;

    mutex_lock(&__parallel_mcia_access_db.async_db_mutex);

    if (!__parallel_mcia_access_db.is_initialized) {
        sxd_log_err(SXD_DEV_NONE, "Parallel MCIA access DB is not initialized\n");
        goto out;
    }
    __parallel_mcia_access_db.is_initialized = false;

    /* Iterate through all hash table buckets */
    hash_for_each_safe(__parallel_mcia_access_db.key_hash_table, bkt, tmp, comp, hash_node) {
        hash_del(&comp->hash_node);
        comp->index = MAX_PARALLEL_MCIA_ACCESS_NUM;
        comp->flag = PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_UNUSED;
        memset(&comp->key, 0, sizeof(parallel_mcia_access_key_t));
    }

    __parallel_mcia_access_db.allocated_completions = 0;
    __parallel_mcia_access_db.active_parallel_mcia_sessions = 0;
    __parallel_mcia_access_db.total_parallel_mcia_sessions = 0;
    mutex_unlock(&__parallel_mcia_access_db.async_db_mutex);
out:
    return;
}

/* Find completion by parallel mcia access key using hash_for_each_possible_safe,
 *  Should be called with db lock held. */
static int find_completion_by_key(const parallel_mcia_access_key_t *key, int *index)
{
    uint32_t                           hash_key = parallel_mcia_access_hash_key(key);
    parallel_mcia_access_completion_t *comp;
    struct hlist_node                 *tmp;

    hash_for_each_possible_safe(__parallel_mcia_access_db.key_hash_table, comp, tmp, hash_node, hash_key) {
        if (__parallel_mcia_access_key_equal(&comp->key, key)) {
            if (comp->flag == PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_TRAPPED) {
                /* Ignore the duplicated key which is already trapped before and continue to find the next one. */
                continue;
            }
            if (index) {
                *index = comp->index;
            }
            return 0;
        }
    }
    return -ENOENT;
}

/* Allocate a new completion slot,
 *  Should be called with db lock held. */
static int allocate_completion_from_pool(int *index)
{
    int err = 0;
    int i;

    for (i = 0; i < MAX_PARALLEL_MCIA_ACCESS_NUM; i++) {
        if (__parallel_mcia_access_db.completions[i].flag == PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_UNUSED) {
            __parallel_mcia_access_db.completions[i].flag = PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_ALLOCATED;
            __parallel_mcia_access_db.completions[i].index = i;
            *index = i;
            __parallel_mcia_access_db.allocated_completions++;
            break;
        }
    }
    if (i == MAX_PARALLEL_MCIA_ACCESS_NUM) {
        err = -ENOMEM;
        sxd_log_notice(SXD_DEV_NONE,
                       "There are already max (%d) parallel MCIA access session on-going (weird case), not allowing more parallel access.\n",
                       i);
    }

    return err;
}

/* De-Allocate a completion slot,
 *  Should be called with db lock held.
 */
static void release_completion_to_pool(int comp_index)
{
    parallel_mcia_access_completion_t *comp = &__parallel_mcia_access_db.completions[comp_index];

    if (comp->flag != PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_UNUSED) {
        comp->flag = PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_UNUSED;
        reinit_completion(&comp->event_ready);
        comp->index = MAX_PARALLEL_MCIA_ACCESS_NUM;
        memset(&comp->key, 0, sizeof(parallel_mcia_access_key_t));
        __parallel_mcia_access_db.allocated_completions--;
    }
}

static inline void __hlist_add_tail(struct hlist_node *node, struct hlist_head *head)
{
    struct hlist_node *last;

    /* If the list is empty, add to head */
    if (hlist_empty(head)) {
        hlist_add_head(node, head);
        return;
    }

    /* Find the last node in the list */
    hlist_for_each(last, head) {
        if (last->next == NULL) {
            /* Add after the last node */
            hlist_add_behind(node, last);
            return;
        }
    }
}

/**
 * HASH_ADD_TAIL - add an object to the tail of a hashtable bucket
 * @hashtable: hashtable to add to
 * @node: the hash node of the object to be added
 * @hash_key: the hash key of the object to be added
 */
  #define HASH_ADD_TAIL(hashtable, node, hash_key) \
    __hlist_add_tail(node, &hashtable[hash_min(hash_key, HASH_BITS(hashtable))])


static bool __reach_max_parallel_mcia_access_session_number(void)
{
    bool reached = false;

    mutex_lock(&__parallel_mcia_access_db.async_db_mutex);

    if (!__parallel_mcia_access_db.is_initialized) {
        sxd_log_debug(SXD_DEV_NONE, "Parallel MCIA access DB is not initialized or has been deinitialized\n");
        reached = true;
        goto out;
    }

    if (__parallel_mcia_access_db.active_parallel_mcia_sessions >= MAX_PARALLEL_MCIA_ACCESS_NUM) {
        reached = true;
        __parallel_mcia_access_db.async_to_sync_session_counter++;
        goto out;
    }

    __parallel_mcia_access_db.active_parallel_mcia_sessions++;
    __parallel_mcia_access_db.total_parallel_mcia_sessions++;

out:
    mutex_unlock(&__parallel_mcia_access_db.async_db_mutex);
    return reached;
}

/* Register parallel mcia access key values.
 *   This function could be called by many parallel user callers and potentially be bottleneck.
 *   - It should be finished asap.
 *   - For one completion, the DB operation of its registration should always be done earlier than that by its later TRAP id handler.*/
static int __register_parallel_mcia_access_session(uint8_t  module,
                                                   uint8_t  i2c_addr,
                                                   uint8_t  page,
                                                   uint8_t  bank,
                                                   uint16_t size,
                                                   uint16_t dev_addr,
                                                   int     *comp_index)
{
    int                                err = 0;
    parallel_mcia_access_completion_t *comp = NULL;
    parallel_mcia_access_key_t         key;
    uint32_t                           hash_key = 0;

    /* Setup key structure */
    key.module = module;
    key.i2c_addr = i2c_addr;
    key.page = page;
    key.bank = bank;
    key.size = size;
    key.dev_addr = dev_addr;

    mutex_lock(&__parallel_mcia_access_db.async_db_mutex);

    if (!__parallel_mcia_access_db.is_initialized) {
        sxd_log_err(SXD_DEV_NONE, "Parallel MCIA access DB is not initialized or has been deinitialized\n");
        err = -EINVAL;
        goto out;
    }

    /* Allocate completion slot */
    err = allocate_completion_from_pool(comp_index);
    if (err) {
        goto out;
    }

    comp = &__parallel_mcia_access_db.completions[*comp_index];

    /* Setup completion */
    __copy_parallel_mcia_access_key(&comp->key, &key);
    comp->caller_tid = current->pid;

    /* Add to hash table, in FIFO order for duplicated eeprom access (with same 6 tuple key). */
    hash_key = parallel_mcia_access_hash_key(&key);
    HASH_ADD_TAIL(__parallel_mcia_access_db.key_hash_table, &comp->hash_node, hash_key);

out:
    mutex_unlock(&__parallel_mcia_access_db.async_db_mutex);
    return err;
}

/* Unregister parallel mcia access key values */
static void unregister_parallel_mcia_access_session(int comp_index)
{
    parallel_mcia_access_completion_t *comp;

    mutex_lock(&__parallel_mcia_access_db.async_db_mutex);

    if (!__parallel_mcia_access_db.is_initialized) {
        sxd_log_err(SXD_DEV_NONE, "Parallel MCIA access DB is not initialized or has been deinitialized\n");
        goto out;
    }

    comp = &__parallel_mcia_access_db.completions[comp_index];
    if (comp->flag == PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_UNUSED) {
        sxd_log_err(SXD_DEV_NONE, "Parallel MCIA access session %d is not registered\n", comp_index);
        goto out;
    }

    /* Remove from hash table */
    hash_del(&comp->hash_node);

    /* Free completion slot */
    release_completion_to_pool(comp_index);

    __parallel_mcia_access_db.active_parallel_mcia_sessions--;


out:

    mutex_unlock(&__parallel_mcia_access_db.async_db_mutex);
}

/*
 *  MCIA access task (process context) register its keys and enters to blocking state to wait for signal from trap handler.
 *  MCIA access task is signaled by trap handler within the specified time window, or timeout occurs.
 */
static int __async_mcia_access_session(struct sx_dev             *dev,
                                       bool                       check_pnv,
                                       uint8_t                    module,
                                       uint8_t                    i2c_addr,
                                       uint8_t                    page,
                                       uint8_t                    bank,
                                       uint16_t                   size,
                                       uint16_t                   dev_addr,
                                       unsigned long              timeout_ms,
                                       struct ku_access_mcia_reg *reg_data)
{
    parallel_mcia_access_completion_t *comp = NULL;
    int                                ret;
    int                                err = 0;
    unsigned long                      timeout_jiffies;
    int                                comp_index = 0;

    err = __register_parallel_mcia_access_session(module, i2c_addr, page, bank, size, dev_addr, &comp_index);
    if (err) {
        sxd_log_err(dev,
                    "Failed to register parallel MCIA access session: module=%u, i2c_addr=0x%x, page=%u, bank=%u, size=%u, dev_addr=0x%x, err=%d\n",
                    module,
                    i2c_addr,
                    page,
                    bank,
                    size,
                    dev_addr,
                    err);
        goto out;
    }
    comp = &__parallel_mcia_access_db.completions[comp_index];
    sx_int_log_debug(&sx_priv(
                         dev)->module_eeprom_log,
                     "Registered parallel MCIA access session %d (caller_tid=%d): module=%u, i2c_addr=0x%x, page=%u, bank=%u, size=%u, dev_addr=0x%x.\n",
                     comp_index,
                     comp->caller_tid,
                     module,
                     i2c_addr,
                     page,
                     bank,
                     size,
                     dev_addr);
    err = __stateful_access_mcia(dev, reg_data, check_pnv);
    if (err) {
        mutex_lock(&__parallel_mcia_access_db.async_db_mutex);
        __parallel_mcia_access_db.async_request_err_counter++;
        mutex_unlock(&__parallel_mcia_access_db.async_db_mutex);
        sxd_log_err(dev,
                    "mcia access failed in async mode, caller_tid=%d, key(module=%u,i2c_addr=%u,page=%u,bank=%u,size=%u,dev_addr=0x%x), total: %llu\n",
                    comp->caller_tid,
                    module,
                    i2c_addr,
                    page,
                    bank,
                    size,
                    dev_addr,
                    __parallel_mcia_access_db.async_request_err_counter);
        goto release_completion;
    }

    /* handle MCIA async access:
     *   - The calling task would wait for the completion event triggered by MCIA trap handler.
     *   The reg_data would be overwritten by the MCIA content from the MCIA trap.
     *   - if waiting timeout, the parallel mcia access should return -ETIMEDOUT.
     */

    timeout_jiffies = msecs_to_jiffies(timeout_ms);

    if (completion_done(&comp->event_ready)) {
        /* MCIA trap may be already received here. */
        sxd_log_debug(dev,
                      "MCIA trap is already received and no more waiting, caller_tid=%d, key(module=%u,i2c_addr=%u,page=%u,bank=%u,size=%u,dev_addr=0x%x)\n",
                      comp->caller_tid,
                      module,
                      i2c_addr,
                      page,
                      bank,
                      size,
                      dev_addr);
        goto async_complete;
    }
    ret = wait_for_completion_timeout(&comp->event_ready, timeout_jiffies);
    if (ret == 0) {
        /* check the completion flag again, to avoid the missed case */
        mutex_lock(&__parallel_mcia_access_db.async_db_mutex);
        if (comp->flag == PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_TRAPPED) {
            mutex_unlock(&__parallel_mcia_access_db.async_db_mutex);
            sxd_log_debug(dev,
                          "MCIA trap could already be received before or during waiting end, assumed as OK: caller_tid=%d, key(module=%u,i2c_addr=%u,page=%u,bank=%u,size=%u,dev_addr=0x%x)\n",
                          comp->caller_tid,
                          module,
                          i2c_addr,
                          page,
                          bank,
                          size,
                          dev_addr);
            goto async_complete;
        }
        __parallel_mcia_access_db.timeout_session_counter++;
        mutex_unlock(&__parallel_mcia_access_db.async_db_mutex);
        sxd_log_notice(dev,
                       "Async MCIA timeout, caller_tid=%d, key(module=%u,i2c_addr=%u,page=%u,bank=%u,size=%u,dev_addr=0x%x), total: %llu\n",
                       comp->caller_tid,
                       module,
                       i2c_addr,
                       page,
                       bank,
                       size,
                       dev_addr,
                       __parallel_mcia_access_db.timeout_session_counter);
        err = -ETIMEDOUT;
        goto release_completion;
    }

    /* Process the event */
    sx_int_log_debug(&sx_priv(
                         dev)->module_eeprom_log,
                     "Async MCIA access OK (session_index=%d, caller_tid=%d, key(module=%u,i2c_addr=%u,page=%u,bank=%u,size=%u,dev_addr=0x%x), with %d ms left.\n",
                     comp->index,
                     comp->caller_tid,
                     module,
                     i2c_addr,
                     page,
                     bank,
                     size,
                     dev_addr,
                     jiffies_to_msecs(ret));

async_complete:
    memcpy(&reg_data->mcia_reg, &comp->mcia_access_reg.mcia_reg, sizeof(reg_data->mcia_reg));

release_completion:
    sx_int_log_debug(&sx_priv(
                         dev)->module_eeprom_log,
                     "Unregistered parallel MCIA access session %d (caller_tid=%d): module=%u, i2c_addr=0x%x, page=%u, bank=%u, size=%u, dev_addr=0x%x.\n",
                     comp_index,
                     comp->caller_tid,
                     module,
                     i2c_addr,
                     page,
                     bank,
                     size,
                     dev_addr);
    unregister_parallel_mcia_access_session(comp_index);

out:
    return err;
}

/* Called in MCIA trap handler: lookup blocking parallel mcia access by key and signal it. */
static void __wakeup_async_mcia_access_session(struct mcia_work *mcia_w)
{
    int                                err = 0;
    parallel_mcia_access_completion_t *comp;
    int                                comp_index = 0;
    bool                               is_locked = false;
    struct ku_mcia_reg                *mcia_reg = &mcia_w->mcia;
    parallel_mcia_access_key_t         key;

    memset(&key, 0, sizeof(parallel_mcia_access_key_t));
    key.module = mcia_reg->module;
    key.i2c_addr = mcia_reg->i2c_device_address;
    key.page = mcia_reg->page_number;
    key.bank = mcia_reg->bank_number;
    key.size = mcia_reg->size;
    key.dev_addr = mcia_reg->device_address;


    mutex_lock(&__parallel_mcia_access_db.async_db_mutex);
    is_locked = true;

    if (!__parallel_mcia_access_db.is_initialized) {
        sxd_log_debug(SXD_DEV_NONE, "Parallel MCIA access DB is not initialized or has been deinitialized\n");
        goto out;
    }
    err = find_completion_by_key(&key, &comp_index);
    if (err) {
        __parallel_mcia_access_db.unmatched_trap_counter++;
        sxd_log_notice(SXD_DEV_NONE,
                       "Not found parallel MCIA access session: module=%u, i2c_addr=0x%x, page=%u, bank=%u, size=%u, dev_addr=0x%x, total: %llu\n",
                       key.module,
                       key.i2c_addr,
                       key.page,
                       key.bank,
                       key.size,
                       key.dev_addr,
                       __parallel_mcia_access_db.unmatched_trap_counter);
        goto out;
    }

    comp = &__parallel_mcia_access_db.completions[comp_index];

    switch (comp->flag) {
    case PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_UNUSED:
        __parallel_mcia_access_db.unused_trap_counter++;
        sxd_log_err(SXD_DEV_NONE,
                    "Parallel MCIA access session %d (caller_tid=%d) (module=%u,i2c_addr=0x%x,page=%u,bank=%u,size=%u,dev_addr=0x%x) is not registered, total: %llu\n",
                    comp_index,
                    comp->caller_tid,
                    key.module,
                    key.i2c_addr,
                    key.page,
                    key.bank,
                    key.size,
                    key.dev_addr,
                    __parallel_mcia_access_db.unused_trap_counter);
        break;

    case PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_ALLOCATED:
        __parallel_mcia_access_db.matched_trap_counter++;
        memcpy(&comp->mcia_access_reg.mcia_reg, mcia_reg, sizeof(struct ku_mcia_reg));
        sxd_log_debug(SXD_DEV_NONE,
                      "Parallel MCIA access session %d (caller_tid=%d) (module=%u,i2c_addr=0x%x,page=%u,bank=%u,size=%u,dev_addr=%d) trap is received.\n",
                      comp_index,
                      comp->caller_tid,
                      key.module,
                      key.i2c_addr,
                      key.page,
                      key.bank,
                      key.size,
                      key.dev_addr);
        comp->flag = PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_TRAPPED;
        /* Signal the parallel mcia access */
        complete(&comp->event_ready);
        break;

    case PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_TRAPPED:
        __parallel_mcia_access_db.duplicated_trap_counter++;
        sxd_log_notice(SXD_DEV_NONE,
                       "Parallel MCIA access session %d (caller_tid=%d) (module=%u,i2c_addr=0x%x,page=%u,bank=%u,size=%u,dev_addr=0x%x) has been trapped and handled, total: %llu\n",
                       comp_index,
                       comp->caller_tid,
                       key.module,
                       key.i2c_addr,
                       key.page,
                       key.bank,
                       key.size,
                       key.dev_addr,
                       __parallel_mcia_access_db.duplicated_trap_counter);
        break;

    default:
        __parallel_mcia_access_db.unknown_trap_counter++;
        sxd_log_err(SXD_DEV_NONE,
                    "Parallel MCIA access session %d (caller_tid=%d) (module=%u,i2c_addr=0x%x,page=%u,bank=%u,size=%u,dev_addr=0x%x) has unknown flag: %d, total: %llu\n",
                    comp_index,
                    comp->caller_tid,
                    key.module,
                    key.i2c_addr,
                    key.page,
                    key.bank,
                    key.size,
                    key.dev_addr,
                    comp->flag,
                    __parallel_mcia_access_db.unknown_trap_counter);
        goto out;
    }

out:
    if (is_locked) {
        mutex_unlock(&__parallel_mcia_access_db.async_db_mutex);
    }
    return;
}


static void __mcia_trap_handler_work(struct work_struct *work)
{
    struct mcia_work *mcia_w = container_of(work, struct mcia_work, w);


    __wakeup_async_mcia_access_session(mcia_w);
    kfree(mcia_w);
}

static void __enqueue_mcia_event(struct ku_mcia_reg *mcia_reg)
{
    struct mcia_work *mcia_w;

    /* must to allocate mcia_W because we move
     *  to new context from atomic (trap flow) to mcia trap handler. */
    mcia_w = kmalloc(sizeof(struct mcia_work), GFP_ATOMIC);
    if (mcia_w == NULL) {
        sxd_log_err(SXD_DEV_NONE, "Failed to allocate work\n");
        return;
    }

    INIT_WORK(&mcia_w->w, __mcia_trap_handler_work);
    memcpy(&mcia_w->mcia, mcia_reg, sizeof(struct ku_mcia_reg));
    queue_work(s_parallel_mod_access_wq_p, &mcia_w->w);
}

void sx_handle_mcia_event(struct completion_info *ci)
{
    struct ku_mcia_reg mcia_reg;
    int                err;
    u8               * mcia_outbox = (u8*)ci->skb->data + sizeof(struct sx_emad) +
                                     sizeof(struct sxd_emad_tlv_reg);

    err = __MCIA_decode(mcia_outbox, &mcia_reg, NULL);
    if (err) {
        sxd_log_err(SXD_DEV_FROM_COMP_INFO(ci), "__MCIA_decode returned with error %d\n", err);
        return;
    }
    sxd_log_debug(SXD_DEV_FROM_COMP_INFO(ci),
                  "MCIA event received: module=%u, i2c_addr=0x%x, page=%u, bank=%u, size=%u, dev_addr=0x%x\n",
                  mcia_reg.module,
                  mcia_reg.i2c_device_address,
                  mcia_reg.page_number,
                  mcia_reg.bank_number,
                  mcia_reg.size,
                  mcia_reg.device_address);
    __enqueue_mcia_event(&mcia_reg);
}

int sx_core_parallel_mod_access_wq_init(void)
{
    int  err = 0;
    bool is_locked = false;

    /* there should be only 1 parallel module access work queue for single asic or multi-asic environment.*/
    mutex_lock(&s_parallel_mod_access_wq_init_mutex);
    is_locked = true;
    if (s_parallel_mod_access_wq_initialized) {
        sxd_log_notice(SXD_DEV_NONE, "Parallel module access work queue already initialized.\n");
        goto out;
    }

    s_parallel_mod_access_wq_p = create_singlethread_workqueue("sx_parallel_mod_access_wq");
    if (!s_parallel_mod_access_wq_p) {
        sxd_log_err(SXD_DEV_NONE, "Failed to create parallel module access work queue.\n");
        err = -ENOMEM;
        goto out;
    }
    __init_parallel_mcia_access_db();
    s_parallel_mod_access_wq_initialized = true;

out:
    if (is_locked) {
        mutex_unlock(&s_parallel_mod_access_wq_init_mutex);
    }
    return err;
}

void sx_core_parallel_mod_access_wq_deinit(void)
{
    bool is_locked = false;

    mutex_lock(&s_parallel_mod_access_wq_init_mutex);
    is_locked = true;
    if (!s_parallel_mod_access_wq_initialized) {
        sxd_log_notice(SXD_DEV_NONE, "Parallel module access work queue already deinitialized.\n");
        goto out;
    }

    __deinit_parallel_mcia_access_db();
    destroy_workqueue(s_parallel_mod_access_wq_p);
    s_parallel_mod_access_wq_p = NULL;
    s_parallel_mod_access_wq_initialized = false;

out:
    if (is_locked) {
        mutex_unlock(&s_parallel_mod_access_wq_init_mutex);
    }
}

int sx_module_parallel_mcia_access_db_statistics_clear(int argc, const char *argv[], struct sx_dev *dev, void *context)
{
    int err = 0;

    if (argc != 1) {
        err = -EINVAL;
        goto out;
    }
    mutex_lock(&__parallel_mcia_access_db.async_db_mutex);
    __parallel_mcia_access_db.timeout_session_counter = 0;
    __parallel_mcia_access_db.async_to_sync_session_counter = 0;
    __parallel_mcia_access_db.async_request_err_counter = 0;
    __parallel_mcia_access_db.unmatched_trap_counter = 0;
    __parallel_mcia_access_db.unused_trap_counter = 0;
    __parallel_mcia_access_db.matched_trap_counter = 0;
    __parallel_mcia_access_db.duplicated_trap_counter = 0;
    __parallel_mcia_access_db.unknown_trap_counter = 0;
    mutex_unlock(&__parallel_mcia_access_db.async_db_mutex);
out:
    return err;
}


int sx_module_parallel_mcia_access_db_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_priv *priv = NULL;
    int             i;
    struct sx_dev  *dev = sx_dbg_dump_get_device(m);

    if (!dev) {
        return 0;
    }

    priv = sx_priv(dev);
    seq_printf(m, "Parallel MCIA Access DB\n");
    seq_printf(m, "-------------------------------------------------------------------------------\n\n");
    if (!priv->module_eeprom_async_access_mode) {
        seq_printf(m, "Parallel MCIA access: disabled\n");
        goto out;
    }
    seq_printf(m, "Parallel MCIA access: enabled\n");

    if (!__parallel_mcia_access_db.is_initialized) {
        seq_printf(m, "Parallel MCIA access: Uninitialized\n");
        goto out;
    }
    seq_printf(m, "Parallel MCIA access: Initialized\n");

    mutex_lock(&__parallel_mcia_access_db.async_db_mutex);
    seq_printf(m, "allocated_completions: %d\n", __parallel_mcia_access_db.allocated_completions);
    seq_printf(m, "active_parallel_mcia_sessions: %d\n", __parallel_mcia_access_db.active_parallel_mcia_sessions);
    seq_printf(m, "total_parallel_mcia_sessions: %d\n\n", __parallel_mcia_access_db.total_parallel_mcia_sessions);
    seq_printf(m, "timeout_session_counter: %llu\n", __parallel_mcia_access_db.timeout_session_counter);
    seq_printf(m, "async_to_sync_session_counter: %llu\n", __parallel_mcia_access_db.async_to_sync_session_counter);
    seq_printf(m,
               "async_request_err_counter: %llu\n",
               __parallel_mcia_access_db.async_request_err_counter);
    seq_printf(m, "unmatched_trap_counter: %llu\n", __parallel_mcia_access_db.unmatched_trap_counter);
    seq_printf(m, "unused_trap_counter: %llu\n", __parallel_mcia_access_db.unused_trap_counter);
    seq_printf(m, "matched_trap_counter: %llu\n", __parallel_mcia_access_db.matched_trap_counter);
    seq_printf(m, "duplicated_trap_counter: %llu\n", __parallel_mcia_access_db.duplicated_trap_counter);
    seq_printf(m, "unknown_trap_counter: %llu\n", __parallel_mcia_access_db.unknown_trap_counter);
    seq_printf(m,
               "\n%-14s   %-14s   %-14s   %-14s   %-14s   %-14s   %-14s   %-14s   %-14s\n",
               "Index",
               "Flag",
               "Module",
               "I2C",
               "Page",
               "Bank",
               "Size",
               "Dev_Addr",
               "Caller_TID");
    seq_printf(m,
               "..................................................................................................................................................\n");
    for (i = 0; i < MAX_PARALLEL_MCIA_ACCESS_NUM; i++) {
        if (__parallel_mcia_access_db.completions[i].flag == PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_UNUSED) {
            continue;
        }
        seq_printf(m, "%-14d   %-14d   %-14d   0x%-12x   %-14d   %-14d   %-14d   %-14d   %-14d\n",
                   i, __parallel_mcia_access_db.completions[i].flag,
                   __parallel_mcia_access_db.completions[i].key.module,
                   __parallel_mcia_access_db.completions[i].key.i2c_addr,
                   __parallel_mcia_access_db.completions[i].key.page,
                   __parallel_mcia_access_db.completions[i].key.bank,
                   __parallel_mcia_access_db.completions[i].key.size,
                   __parallel_mcia_access_db.completions[i].key.dev_addr,
                   __parallel_mcia_access_db.completions[i].caller_tid);
    }
    mutex_unlock(&__parallel_mcia_access_db.async_db_mutex);

out:
    seq_printf(m, "\n\n");
    return 0;
}

int sx_module_eeprom_access_db_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_priv *priv = NULL;
    int             i;
    struct sx_dev  *dev = sx_dbg_dump_get_device(m);

    if (!dev) {
        return 0;
    }

    priv = sx_priv(dev);
    seq_printf(m, "Module Eeprom Access DB\n");
    seq_printf(m, "-------------------------------------------------------------------------------\n");
    seq_printf(m,
               "module_eeprom_async_access_mode: %s\n",
               priv->module_eeprom_async_access_mode ? "enabled" : "disabled");
    seq_printf(m, "module_eeprom_max_access_size: %d\n", priv->module_eeprom_max_access_size);
    seq_printf(m, "-------------------------------------\n");

    seq_printf(m, "\n%-14s   %-14s\n", "Module", "EEPROM Type");
    seq_printf(m, ".....................................\n");

    mutex_lock(&priv->module_eeprom_access_db.access_mutex);
    for (i = 0; i < MAX_MODULE_NUM; i++) {
        if (priv->module_eeprom_access_db.eeprom_types[i] == EEPROM_MANAGEMENT_TYPE_INVALID) {
            continue;
        }
        seq_printf(m, "%-14d   0x%-12x\n", i, priv->module_eeprom_access_db.eeprom_types[i]);
    }
    mutex_unlock(&priv->module_eeprom_access_db.access_mutex);

    seq_printf(m, "\n\n");
    return 0;
}

static inline void __reg_mcia_pack(struct ku_mcia_reg *mcia_reg,
                                   uint8_t             module,
                                   uint8_t             lock,
                                   uint8_t             page,
                                   uint8_t             pnv,
                                   uint16_t            device_addr,
                                   uint16_t            size,
                                   uint8_t             i2c_addr,
                                   uint8_t             slot,
                                   bool                is_async)
{
    mcia_reg->l = lock;
    mcia_reg->pnv = pnv;
    mcia_reg->module = module;
    mcia_reg->slot_index = slot;
    mcia_reg->i2c_device_address = i2c_addr;
    mcia_reg->page_number = page;
    mcia_reg->device_address = device_addr;
    mcia_reg->size = size;
    mcia_reg->bg_mode = is_async ? 1 : 0;
}

static inline int __mcia_status_to_errno(enum mcia_module_status status)
{
    int err = 0;

    switch (status) {
    case MCIA_MODULE_STATUS_GOOD:
        err = 0;
        break;

    case MCIA_MODULE_STATUS_NO_EEPROM_MODULE:
        err = -ENXIO;
        break;

    case MCIA_MODULE_STATUS_MODULE_NOT_SUPPORTED:
        err = -EOPNOTSUPP;
        break;

    case MCIA_MODULE_STATUS_MODULE_NOT_CONNECTED:
        err = -ENOLINK;
        break;

    case MCIA_MODULE_STATUS_MODULE_TYPE_INVALID:
        err = -EPROTONOSUPPORT;
        break;

    case MCIA_MODULE_STATUS_MODULE_NOT_ACCESSIBLE:
        err = -EACCES;
        break;

    case MCIA_MODULE_STATUS_I2C_ERROR:
        err = -EIO;
        break;

    case MCIA_MODULE_STATUS_MODULE_DISABLED:
        err = -ENODEV;
        break;

    case MCIA_MODULE_STATUS_PAGE_ACCESS_FAILED:
        err = -EFAULT;
        break;

    default:
        err = -ENOSYS;
        break;
    }
    return err;
}

static int __handle_mcia_return_status(struct ku_mcia_reg mcia_reg, enum mcia_module_access_type access_type)
{
    switch (mcia_reg.status) {
    case MCIA_MODULE_STATUS_GOOD:
        break;

    case MCIA_MODULE_STATUS_MODULE_NOT_CONNECTED:
    case MCIA_MODULE_STATUS_I2C_ERROR:
    case MCIA_MODULE_STATUS_MODULE_DISABLED:
    case MCIA_MODULE_STATUS_MODULE_TYPE_INVALID:
        sxd_log_notice(SXD_DEV_NONE,
                       "MCIA access: %s to module: %d failed due to unavailable hw link, status: %d (%s).\n",
                       mcia_access_type_to_str(access_type),
                       mcia_reg.module,
                       mcia_reg.status,
                       mcia_status_to_str(mcia_reg.status));
        break;

    default:
        sxd_log_err(SXD_DEV_NONE, "MCIA access failed. module: %u, status: %d (%s), op: %s.\n", mcia_reg.module,
                    mcia_reg.status, mcia_status_to_str(mcia_reg.status), mcia_access_type_to_str(access_type));
    }

    return __mcia_status_to_errno(mcia_reg.status);
}

/* Only for independent module, check/update mcia.pnv before each MCIA access */
static int __update_pnv(struct sx_dev *dev, struct ku_access_mcia_reg *reg_data)
{
    int err = 0;

    /* For independent module, MGMT FW is not aware about module type.
     * Driver is also not aware of module type.
     * For flat / un paged / passive module it is an issue, since its EEPROM is
     * Read Only, but MCIA access performs write to page offset (byte 127 in page 0).
     * So, driver should avoid attempt to write to this offset by indicating MCIA.PNV=1.
     * This indication informs FW MGMT to use current page offset and don't performs
     * page offset write.
     * */
    if ((reg_data->mcia_reg.page_number == 0) &&
        ((reg_data->mcia_reg.device_address + reg_data->mcia_reg.size) < EEPROM_UPPER_PAGE_OFFSET)) {
        /* Allow page select writing for lower page 0 access (0-127) */
        reg_data->mcia_reg.pnv = 0;
        sx_int_log_debug(&sx_priv(dev)->module_eeprom_log,
                         "Module ID %d | Skip page selector query for page %d, offset %d, count %zu, pnv %d",
                         reg_data->mcia_reg.module,
                         reg_data->mcia_reg.page_number,
                         reg_data->mcia_reg.device_address,
                         reg_data->mcia_reg.size,
                         reg_data->mcia_reg.pnv);
    } else {
        err = __sx_core_get_module_page_valid(dev,
                                              reg_data->mcia_reg.module,
                                              reg_data->mcia_reg.slot_index,
                                              reg_data->mcia_reg.page_number,
                                              &reg_data->mcia_reg.pnv);
        if (err) {
            sxd_log_debug(dev, "Failed to get module page valid, err: %d\n", err);
            goto out;
        }
    }
out:
    return err;
}

/* Module eeprom access via MCIA may be blocked by other access via PMMP, etc., and FW will return busy to SDK. SDK should retry several times. */
static int __stateful_access_mcia(struct sx_dev *dev, struct ku_access_mcia_reg *reg_data, bool check_pnv)
{
    int  err = 0;
    int  try_loop = MCIA_ACCESS_MAX_RETRIES;/* FW would hold 20mSec per try. */
    bool lock_to_be_released = false;

    if (check_pnv) {
        /* For independent module, update pnv before each MCIA access */
        mutex_lock(&sx_priv(dev)->module_eeprom_mcia_access_mutex);
        lock_to_be_released = true;
        __update_pnv(dev, reg_data);
    }
    sx_int_log_info(&sx_priv(
                        dev)->module_eeprom_log,
                    "Module ID %d Bank %d (async_mode=%d, caller pid:%d) | MCIA pack: pnv=%d i2c_addr=0x%x page=%d offset=%d, size=%d, check_pnv=%d",
                    reg_data->mcia_reg.module,
                    reg_data->mcia_reg.bank_number,
                    reg_data->mcia_reg.bg_mode,
                    current->pid,
                    reg_data->mcia_reg.pnv,
                    reg_data->mcia_reg.i2c_device_address,
                    reg_data->mcia_reg.page_number,
                    reg_data->mcia_reg.device_address,
                    reg_data->mcia_reg.size,
                    check_pnv);

    while (try_loop-- > 0) {
        err = sx_ACCESS_REG_MCIA(dev, reg_data);
        if (err || reg_data->op_tlv.status) {
            if (reg_data->op_tlv.status == SX_EMAD_STATUS_BUSY_E) {
                err = -EBUSY;
                continue;
            }
            sxd_log_err(dev,
                        "Fails to access register MCIA, err: %d, status: %d.\n",
                        err,
                        reg_data->op_tlv.status);
            err = -EFAULT;
            goto out;
        }
        goto out;
    }
out:
    if (lock_to_be_released) {
        mutex_unlock(&sx_priv(dev)->module_eeprom_mcia_access_mutex);
        lock_to_be_released = false;
    }
    return err;
}

static int __sx_core_mcia_access(struct sx_dev             *dev,
                                 uint8_t                    access_type,
                                 uint8_t                    module,
                                 uint8_t                    page,
                                 bool                       check_pnv,
                                 uint16_t                   device_addr,
                                 uint16_t                   size,
                                 uint8_t                    i2c_addr,
                                 uint8_t                    slot,
                                 struct ku_access_mcia_reg *reg_data)
{
    int     err = 0;
    uint8_t lock = 0;  /* suggested to be 0 by FW team. */
    uint8_t method = EMAD_METHOD_QUERY;
    bool    is_async = false;

    if (access_type == MCIA_MODULE_ACCESS_TYPE_WRITE) {
        method = EMAD_METHOD_WRITE;
    }

    reg_data->dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&(reg_data->op_tlv), MCIA_REG_ID, method);

    is_async = sx_priv(dev)->module_eeprom_async_access_mode;
    /* In HW, the max parallel session number should be N (=I2C buses), which is < 32, while details is hidden to SW.
     * To avoid maintaining unlimited async MCIA sessions, SDK considers 32 (MAX_PARALLEL_MCIA_ACCESS_NUM) as the limit,
     * and change the following requests to sync mode until parallel session number becomes less.
     *  - the value cannot be too small, otherwise it can impact parallel access performance, i.e, FW may still have free I2C buses but SDK block the request;
     *  - it cannot be too big, otherwise it can impact the total time of one MCIA call which calls the module shared same I2C bus with previous requests, which would wait/be queued in FW.
     */
    if (is_async) {
        if (__reach_max_parallel_mcia_access_session_number()) {
            is_async = false;
            sx_int_log_info(&sx_priv(
                                dev)->module_eeprom_log,
                            "Change access mode to sync because too many parallel access sessions (%d) are on-going.\n",
                            __parallel_mcia_access_db.allocated_completions);
        }
    }

    __reg_mcia_pack(&(reg_data->mcia_reg), module, lock, page, 0, device_addr, size, i2c_addr, slot, is_async);

    if (is_async) {
        err = __async_mcia_access_session(dev,
                                          check_pnv,
                                          reg_data->mcia_reg.module,
                                          reg_data->mcia_reg.i2c_device_address,
                                          reg_data->mcia_reg.page_number,
                                          reg_data->mcia_reg.bank_number,
                                          size,
                                          reg_data->mcia_reg.device_address,
                                          ASYNC_MCIA_ACCESS_WAITING_TIMEOUT_MSECS,
                                          reg_data);
    } else {
        err = __stateful_access_mcia(dev, reg_data, check_pnv);
    }
    if (err) {
        sxd_log_err(dev,
                    "Fails to access (%d, async:%d) register MCIA (module:%d, page:%d, pnv:%d, device_addr:%d, size:%d, i2c_addr:0x%x, slot:%d, status:%d), err: %d.\n",
                    method,
                    is_async,
                    module,
                    page,
                    reg_data->mcia_reg.pnv,
                    device_addr,
                    size,
                    i2c_addr,
                    slot,
                    reg_data->mcia_reg.status,
                    err);
        goto out;
    }
out:
    sx_int_log_info(&sx_priv(
                        dev)->module_eeprom_log,
                    "async (%d) MCIA exit (module:%d, page:%d, pnv:%d, device_addr:%d, size:%d, i2c_addr:0x%x, slot:%d, status:%d), err: %d.\n",
                    is_async,
                    module,
                    page,
                    reg_data->mcia_reg.pnv,
                    device_addr,
                    size,
                    i2c_addr,
                    slot,
                    reg_data->mcia_reg.status,
                    err);
    return err;
}

/* This function is valid for independent module only.
 * It reads page value from EEPROM (byte 127 in lower page). */
int sx_core_get_module_page_from_eeprom(struct sx_dev *dev, uint8_t module, uint8_t slot, uint8_t *page_p)
{
    int                       err = 0;
    struct ku_access_mcia_reg reg_data;

    memset(&reg_data, 0, sizeof(reg_data));

    reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&reg_data.op_tlv, MCIA_REG_ID, EMAD_METHOD_QUERY);
    /* page is in byte 127, Due to Endianness issues we need to access addresses that are 4 bytes aligned */
    __reg_mcia_pack(&reg_data.mcia_reg, module, 0, 0, 1, 124, 4, 0x50, slot, false);

    sx_int_log_debug(&sx_priv(dev)->module_eeprom_log,
                     "Module ID %d | MCIA pack:pnv=%d i2c_addr=0x%x page=%d offset=%d",
                     reg_data.mcia_reg.module,
                     reg_data.mcia_reg.pnv,
                     reg_data.mcia_reg.i2c_device_address,
                     reg_data.mcia_reg.page_number,
                     reg_data.mcia_reg.device_address);

    err = __stateful_access_mcia(dev, &reg_data, false);
    if (err) {
        sxd_log_err(dev, "Fails to get module page, err: %d.\n", err);
        goto out;
    }

    if (reg_data.mcia_reg.status) {
        err = __handle_mcia_return_status(reg_data.mcia_reg, MCIA_MODULE_ACCESS_TYPE_READ);
        goto out;
    }
    /* location of bytes in word  : |124|125|126|127| */
    *page_p = MCIA_DWOARD_BYTE4(reg_data.mcia_reg.dword_0);
    sx_int_log_debug(&sx_priv(dev)->module_eeprom_log, "Module ID %d | Module page from eeprom %d", module, *page_p);

out:
    if (err != 0) {
        sx_int_log_error(&sx_priv(dev)->module_eeprom_log,
                         "Module ID %d | Get page from EEPROM failed, EMAD status %d MCIA status %d",
                         module,
                         err,
                         reg_data.mcia_reg.status);
    }
    return err;
}
EXPORT_SYMBOL(sx_core_get_module_page_from_eeprom);

/* This function is valid for independent module only.
 * It sets pnv=0 if last access page changed. */
static int __sx_core_get_module_page_valid(struct sx_dev *dev,
                                           uint8_t        module,
                                           uint8_t        slot,
                                           uint8_t        page,
                                           uint8_t       *pnv_p)
{
    int     err = 0;
    uint8_t last_page = 0;

    /* Read last page access from eeprom */
    err = sx_core_get_module_page_from_eeprom(dev,
                                              module,
                                              slot,
                                              &last_page);
    if (err) {
        sxd_log_debug(dev, "Failed to get module page from EEPROM, err: %d.\n", err);
        goto out;
    }

    if (last_page == page) {
        /* Accessing same page: no need to write page value to page offset in EEPROM.
         * Indicate FW-MGMT to skip this operation by setting MCIA.PNV=1. */
        *pnv_p = 1;
    } else {
        /* Accessing different page: need writing to page offset in EEPROM.
         * Use legacy MCIA.PNV=0. */
        *pnv_p = 0;
    }

    sx_int_log_debug(&sx_priv(dev)->module_eeprom_log,
                     "Module ID %d | access page %d, last page %d, pnv %d",
                     module,
                     page,
                     last_page,
                     *pnv_p);

out:
    return err;
}

/* This function is valid for dependent module only.
 * It gets module type from cached value. */
static bool __sx_core_get_cached_module_type(struct sx_dev *dev, uint8_t module, uint8_t *type)
{
    uint8_t cached_type = 0;

    mutex_lock(&sx_priv(dev)->module_eeprom_access_db.access_mutex);
    cached_type = sx_priv(dev)->module_eeprom_access_db.eeprom_types[module];
    mutex_unlock(&sx_priv(dev)->module_eeprom_access_db.access_mutex);

    if (IS_VALID_EEPROM_MANAGEMENT_TYPE(cached_type)) {
        *type = cached_type;
        return true;
    }
    return false;
}

/* This function is valid for dependent module only.
 * It sets module type to cached value. */
void sx_core_set_cached_module_type(struct sx_dev *dev, uint8_t module, uint8_t type)
{
    mutex_lock(&sx_priv(dev)->module_eeprom_access_db.access_mutex);
    sx_priv(dev)->module_eeprom_access_db.eeprom_types[module] = type;
    mutex_unlock(&sx_priv(dev)->module_eeprom_access_db.access_mutex);
    return;
}

/* This function is valid for dependent module only.
 * It gets module type from EEPROM. */
static int __sx_core_get_module_type(struct sx_dev *dev, uint8_t module, uint8_t slot, uint8_t *type)
{
    int                       err = 0;
    uint8_t                   byte0 = 0;
    struct ku_access_mcia_reg reg_data;

    if (__sx_core_get_cached_module_type(dev, module, type)) {
        sx_int_log_debug(&sx_priv(dev)->module_eeprom_log, "Module ID %d | Module type from cache %d", module, *type);
        goto out;
    }

    memset(&reg_data, 0, sizeof(reg_data));

    reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&reg_data.op_tlv, MCIA_REG_ID, EMAD_METHOD_QUERY);

    __reg_mcia_pack(&reg_data.mcia_reg, module, 0, 0, 0, 0, 1, 0x50, slot, false);

    sx_int_log_debug(&sx_priv(dev)->module_eeprom_log,
                     "Module ID %d | MCIA pack:pnv=%d i2c_addr=0x%x page=%d offset=%d",
                     reg_data.mcia_reg.module,
                     reg_data.mcia_reg.pnv,
                     reg_data.mcia_reg.i2c_device_address,
                     reg_data.mcia_reg.page_number,
                     reg_data.mcia_reg.device_address);
    err = __stateful_access_mcia(dev, &reg_data, false);
    if (err) {
        sxd_log_err(dev, "Fails to get module type, err: %d.\n", err);
        goto out;
    }

    if (reg_data.mcia_reg.status) {
        err = __handle_mcia_return_status(reg_data.mcia_reg, MCIA_MODULE_ACCESS_TYPE_READ);
        goto out;
    }

    byte0 = MCIA_DWOARD_BYTE1(reg_data.mcia_reg.dword_0);
    switch (byte0) {
    case EEPROM_MODULE_INFO_ID_SFP:
        *type = EEPROM_MANAGEMENT_TYPE_SFP;
        break;

    case EEPROM_MODULE_INFO_ID_QSFP:
    case EEPROM_MODULE_INFO_ID_QSFP_PLUS:
    case EEPROM_MODULE_INFO_ID_QSFP28:
        *type = EEPROM_MANAGEMENT_TYPE_QSFP;
        break;

    case EEPROM_MODULE_INFO_ID_QSFP_DD:
    case EEPROM_MODULE_INFO_ID_QSFP_8X:
    case EEPROM_MODULE_INFO_ID_SFP_DD:
    case EEPROM_MODULE_INFO_ID_DSFP:
    case EEPROM_MODULE_INFO_ID_QSFP_PLUS_CMIS:
    case EEPROM_MODULE_INFO_ID_ELS:
    case EEPROM_MODULE_INFO_ID_OE:
        *type = EEPROM_MANAGEMENT_TYPE_CMIS;
        break;

    default:
        sxd_log_err(dev, "Unknown EEPROM type: %d.\n", byte0);
        err = -EFAULT;
        goto out;
    }

    sx_core_set_cached_module_type(dev, module, *type);

    sx_int_log_debug(&sx_priv(dev)->module_eeprom_log, "Module ID %d | Eeprom byte0 0x%x type %d", module, byte0,
                     *type);
out:
    return err;
}

/*
 *  check whether buff range [buf_b, buf_e] is overlapping with a limitation byte range [limit_b, limit_e]:
 *   if yes, return true, with the overlapping range in [*hole_b, *hole_e]
 *   if no, return false
 */
static bool __range_is_overlap(loff_t buf_b, size_t buf_e, int limit_b, size_t limit_e, int *hole_b, int *hole_e)
{
    if ((buf_b > limit_e) || (buf_e < limit_b)) {
        return false;
    }

    *hole_b = buf_b;
    *hole_e = buf_e;

    if (buf_b < limit_b) {
        *hole_b = limit_b;
    }

    if (buf_e > limit_e) {
        *hole_e = limit_e;
    }

    return true;
}

/* Calculate whether any limitation holes exist in the page access range (start, len): *ret_arr_len == 0 means no holes exist. */
static int get_limit_bytes_holes_from_eeprom_page_access_range(struct page_limit_bytes_segment *limit_arr,
                                                               int                              limit_arr_len,
                                                               uint8_t                          page,
                                                               loff_t                           start,
                                                               size_t                           len,
                                                               struct page_access_segment     **ret_hole_arr,
                                                               int                             *ret_hole_arr_len)
{
    int                         err = 0;
    int                         i;
    bool                        found = false;
    int                         hole_b = 0;
    int                         hole_e = 0;
    int                         hole_num = 0;
    struct page_access_segment *holes = NULL;

    *ret_hole_arr_len = 0;

    for (i = 0; i < limit_arr_len; i++) {
        if (page == limit_arr[i].page) {
            found = true;
            break;
        }
    }
    if (!found) {
        goto out;
    }

    for (; i < limit_arr_len; i++) {
        if (page != limit_arr[i].page) {
            break;
        }
        if (__range_is_overlap(start, start + len - 1, limit_arr[i].begin, limit_arr[i].end, &hole_b, &hole_e)) {
            if (hole_b <= hole_e) {
                if (!holes) {
                    holes = kzalloc(sizeof(*holes) * limit_arr_len, GFP_KERNEL);
                    if (!holes) {
                        err = -ENOMEM;
                        goto out;
                    }
                }
                holes[hole_num].begin = hole_b;
                holes[hole_num].end = hole_e;
                hole_num++;
            }
        }
    }
    *ret_hole_arr_len = hole_num;
    *ret_hole_arr = holes;
out:
    return err;
}

/*
 *  The function should be called after get_limit_bytes_holes_from_eeprom_page_access_range(..),
 *  and it assumes that all holes should be within the page access range.
 */
static int __separate_eeprom_page_access_range_into_segments(struct page_access_segment  *hole_arr,
                                                             int                          hole_arr_len,
                                                             loff_t                       start,
                                                             size_t                       len,
                                                             struct page_access_segment **ret_arr,
                                                             int                         *ret_arr_len)
{
    int                         err = 0;
    int                         i;
    size_t                      hole_b = 0;
    size_t                      hole_e = 0;
    int                         max_segments = hole_arr_len + 1;
    int                         segments = 1;
    struct page_access_segment *segs = NULL;

    *ret_arr_len = segments;
    segs = kmalloc(sizeof(*segs) * max_segments, GFP_KERNEL);
    if (!(segs)) {
        err = -ENOMEM;
        goto out;
    }
    segs[0].begin = start;
    segs[0].end = start + len - 1;

    for (i = 0; i < hole_arr_len; i++) {
        hole_b = hole_arr[i].begin;
        hole_e = hole_arr[i].end;

        if (hole_e >= segs[segments - 1].end) {
            /* page access range is reached, and no need to compare more */
            if (hole_b == segs[segments - 1].begin) {
                segments--;
                break;
            }
            segs[segments - 1].end = hole_b - 1;
            break;
        }
        /* case: hole_e < segs[segments - 1].end */
        if (hole_b == segs[segments - 1].begin) {
            segs[segments - 1].begin = hole_e + 1;
            continue;
        }
        /* Extend a new segment */
        segs[segments].begin = hole_e + 1;
        segs[segments].end = segs[segments - 1].end;
        segs[segments - 1].end = hole_b - 1;

        segments++;
    }
    *ret_arr_len = segments;
    *ret_arr = segs;

out:
    return err;
}

static int __sx_core_get_slot_and_module(struct sx_dev *dev, uint16_t local_port, uint8_t *slot, uint8_t *module)
{
    struct sx_priv *priv = sx_priv(dev);
    int             err = 0;
    unsigned long   flags;

    if (local_port > MAX_PHYPORT_NUM) {
        sxd_log_err(dev, "Local port %d is invalid. (MAX %d).\n", local_port, MAX_PHYPORT_NUM);
        err = -EINVAL;
        goto out;
    }

    spin_lock_irqsave(&priv->db_lock, flags);
    *slot = priv->local_to_slot_map[local_port];
    *module = priv->local_to_module_map[local_port];
    spin_unlock_irqrestore(&priv->db_lock, flags);

    if ((*slot >= MAX_SLOT_NUM) || (*module >= MAX_MODULE_NUM)) {
        err = -EINVAL;
        sxd_log_err(dev, "slot id %d or module id %d is out of range.\n", *slot, *module);
        goto out;
    }

out:
    return err;
}

/* Fetch the required params for MCIA access/check */
static int __module_sysfs_eeprom_parse_params(const char                        *node_name,
                                              bool                               is_independent,
                                              loff_t                             pos,
                                              struct sx_dev                     *dev,
                                              uint8_t                            module,
                                              uint8_t                            slot,
                                              enum sx_module_sysfs_eeprom_access access,
                                              size_t                             count,
                                              uint8_t                           *i2c_addr,
                                              uint8_t                           *page,
                                              bool                              *check_pnv,
                                              uint16_t                          *device_addr,
                                              struct page_limit_bytes_segment  **limit_arr,
                                              int                               *limit_arr_len)
{
    int     err = 0;
    uint8_t mgmt_type = 0;

    *i2c_addr = 0x50;

    if (node_name[0] == 'i') {
        *page = 0;
        if (strcmp(node_name, "i2c-0x51") == 0) {
            *i2c_addr = 0x51;
        }
    } else {
        *page = (uint8_t)simple_strtol(node_name, NULL, 10);
    }

    *device_addr = pos;

    if (*page > 0) {
        /* add upper page offset for page 1~255 */
        *device_addr += EEPROM_UPPER_PAGE_OFFSET;
    }

    *check_pnv = false;
    if (is_independent) {
        *limit_arr_len = 0;
        /* 'pnv' will be updated before each MCIA access */
        *check_pnv = true;
    } else {
        /* For dependent module, we cache eeprom module type after 1st access to reduce MCIA access overhead.
         * We cannot do the same for independent module because its required page number cannot be
         * cached due to possible changes by external 3rd part tool. */
        err = __sx_core_get_module_type(dev, module, slot, &mgmt_type);
        if (err) {
            sxd_log_debug(dev, "Failed to get the module type, err: %d\n", err);
            goto out;
        }

        if ((*i2c_addr == 0x51) && (mgmt_type != EEPROM_MANAGEMENT_TYPE_SFP)) {
            sxd_log_notice(dev,
                           "Failed to access i2c address 0x51 because module type is not SFF-8472.\n");
            err = -EFAULT;
            goto out;
        }

        if ((*page > 0) && (mgmt_type == EEPROM_MANAGEMENT_TYPE_SFP)) {
            *i2c_addr = 0x51;
        }

        switch (mgmt_type) {
        case EEPROM_MANAGEMENT_TYPE_SFP:
            if (*i2c_addr == 0x51) {
                /* SFF8472's limitation bytes are only for i2c 0x51 */
                if (access == MODULE_SYSFS_EEPROM_READ) {
                    *limit_arr = sff8472_rd_limit_arr;
                    *limit_arr_len = sff8472_rd_limit_arr_len;
                } else {
                    *limit_arr = sff8472_wr_limit_arr;
                    *limit_arr_len = sff8472_wr_limit_arr_len;
                }
            }
            break;

        case EEPROM_MANAGEMENT_TYPE_QSFP:
            if (access == MODULE_SYSFS_EEPROM_READ) {
                *limit_arr = sff8636_rd_limit_arr;
                *limit_arr_len = sff8636_rd_limit_arr_len;
            } else {
                *limit_arr = sff8636_wr_limit_arr;
                *limit_arr_len = sff8636_wr_limit_arr_len;
            }
            break;

        case EEPROM_MANAGEMENT_TYPE_CMIS:
            if (access == MODULE_SYSFS_EEPROM_READ) {
                *limit_arr = cmis_rd_limit_arr;
                *limit_arr_len = cmis_rd_limit_arr_len;
            } else {
                *limit_arr = cmis_wr_limit_arr;
                *limit_arr_len = cmis_wr_limit_arr_len;
            }
            break;

        default:
            sxd_log_err(dev, "Invalid module type %d.\n", mgmt_type);
            err = -EINVAL;
            goto out;
            break;
        }
    }

out:
    return err;
}

static ssize_t __module_sysfs_eeprom_low_level_read(struct sx_dev *dev,
                                                    char          *buf,
                                                    ssize_t        count,
                                                    uint8_t        module,
                                                    uint8_t        slot,
                                                    uint8_t        page,
                                                    bool           check_pnv,
                                                    uint16_t       device_addr,
                                                    uint8_t        i2c_addr)
{
    int                       err = 0;
    ssize_t                   len, remainder;
    loff_t                    i, dw_end;
    uint16_t                  rd_len;
    uint16_t                  handled;
    uint32_t                 *dword = NULL;
    struct ku_access_mcia_reg reg_data;

    len = count;
    handled = 0;
    while (len > 0) {
        rd_len = len;
        if (len > sx_priv(dev)->module_eeprom_max_access_size) {
            rd_len = sx_priv(dev)->module_eeprom_max_access_size;
        }

        memset(&reg_data, 0, sizeof(reg_data));
        err = __sx_core_mcia_access(dev,
                                    MCIA_MODULE_ACCESS_TYPE_READ,
                                    module,
                                    page,
                                    check_pnv,
                                    device_addr + handled,
                                    rd_len,
                                    i2c_addr,
                                    slot,
                                    &reg_data);
        if (err) {
            sxd_log_err(dev, "Fails to read module eeprom, status: %d.\n", err);
            if (err > 0) {
                err = -err;
            }
            goto out;
        }
        if (reg_data.mcia_reg.status) {
            err = __handle_mcia_return_status(reg_data.mcia_reg, MCIA_MODULE_ACCESS_TYPE_READ);
            goto out;
        }

        dword = &(reg_data.mcia_reg.dword_0);
        remainder = rd_len % 4;
        dw_end = handled + rd_len - remainder;
        for (i = handled; i < dw_end; i += 4) {
            buf[i] = MCIA_DWOARD_BYTE1(*dword);
            buf[i + 1] = MCIA_DWOARD_BYTE2(*dword);
            buf[i + 2] = MCIA_DWOARD_BYTE3(*dword);
            buf[i + 3] = MCIA_DWOARD_BYTE4(*dword);
            dword++;
        }
        if (remainder) {
            if (remainder == 3) {
                buf[i + 2] = MCIA_DWOARD_BYTE3(*dword);
            }
            if (remainder >= 2) {
                buf[i + 1] = MCIA_DWOARD_BYTE2(*dword);
            }
            if (remainder >= 1) {
                buf[i] = MCIA_DWOARD_BYTE1(*dword);
            }
        }
        len -= rd_len;
        handled += rd_len;
    }

out:
    if (err < 0) {
        return err;
    }
    return count;
}

static ssize_t __module_sysfs_eeprom_low_level_write(struct sx_dev *dev,
                                                     char          *buf,
                                                     ssize_t        count,
                                                     uint8_t        module,
                                                     uint8_t        slot,
                                                     uint8_t        page,
                                                     bool           check_pnv,
                                                     uint16_t       device_addr,
                                                     uint8_t        i2c_addr)
{
    int                       err = 0;
    ssize_t                   len, remainder;
    loff_t                    i, dw_end;
    uint16_t                  wr_len;
    uint16_t                  handled;
    uint32_t                 *dword = NULL;
    struct ku_access_mcia_reg reg_data;

    len = count;
    handled = 0;
    while (len > 0) {
        wr_len = len;
        if (len > min((int)sx_priv(dev)->module_eeprom_max_access_size, MCIA_WRITE_MAX_SIZE)) {
            wr_len = min((int)sx_priv(dev)->module_eeprom_max_access_size, MCIA_WRITE_MAX_SIZE);
        }

        memset(&reg_data, 0, sizeof(reg_data));
        dword = &(reg_data.mcia_reg.dword_0);
        remainder = wr_len % 4;
        dw_end = handled + wr_len - remainder;
        for (i = handled; i < dw_end; i += 4) {
            *dword |= MCIA_BYTE1_TO_DWOARD(buf[i]);
            *dword |= MCIA_BYTE2_TO_DWOARD(buf[i + 1]);
            *dword |= MCIA_BYTE3_TO_DWOARD(buf[i + 2]);
            *dword |= MCIA_BYTE4_TO_DWOARD(buf[i + 3]);
            dword++;
        }
        if (remainder) {
            if (remainder == 3) {
                *dword |= MCIA_BYTE3_TO_DWOARD(buf[i + 2]);
            }
            if (remainder >= 2) {
                *dword |= MCIA_BYTE2_TO_DWOARD(buf[i + 1]);
            }
            if (remainder >= 1) {
                *dword |= MCIA_BYTE1_TO_DWOARD(buf[i]);
            }
        }

        err = __sx_core_mcia_access(dev,
                                    MCIA_MODULE_ACCESS_TYPE_WRITE,
                                    module,
                                    page,
                                    check_pnv,
                                    device_addr + handled,
                                    wr_len,
                                    i2c_addr,
                                    slot,
                                    &reg_data);
        if (err) {
            sxd_log_err(dev, "Fails to lower level write module eeprom, status: %d.\n", err);
            if (err > 0) {
                err = -err;
            }
            goto out;
        }
        if (reg_data.mcia_reg.status) {
            err = __handle_mcia_return_status(reg_data.mcia_reg, MCIA_MODULE_ACCESS_TYPE_WRITE);
            goto out;
        }

        len -= wr_len;
        handled += wr_len;
    }
out:
    if (err < 0) {
        return err;
    }
    return count;
}

static ssize_t __sx_core_module_sysfs_eeprom_access(struct sx_dev                     *dev,
                                                    const char                        *node_name,
                                                    uint8_t                            slot,
                                                    uint8_t                            module,
                                                    enum sx_module_sysfs_eeprom_access access,
                                                    char                              *buf,
                                                    loff_t                             pos,
                                                    size_t                             count)
{
    int                              err = 0;
    ssize_t                          offset, len;
    loff_t                           i;
    uint8_t                          page = 0;
    bool                             check_pnv = false;
    uint8_t                          i2c_addr = 0;
    uint16_t                         device_addr;
    struct page_limit_bytes_segment *limit_arr = NULL;
    int                              limit_arr_len = 0;
    struct page_access_segment      *hole_arr = NULL;
    int                              hole_arr_len = 0;
    struct page_access_segment      *access_segs = NULL;
    int                              access_segs_len = 0;
    bool                             is_independent = false;
    bool                             sysfs_dev_hold = false;

    err = sx_core_sysfs_dev_hold(dev);
    if (err) {
        goto err_out;
    }

    sysfs_dev_hold = true;

    err = sx_core_get_module_control(dev, slot, module, &is_independent);
    if (err) {
        goto err_out;
    }

    err = __module_sysfs_eeprom_parse_params(node_name,
                                             is_independent,
                                             pos,
                                             dev,
                                             module,
                                             slot,
                                             access,
                                             count,
                                             &i2c_addr,
                                             &page,
                                             &check_pnv,
                                             &device_addr,
                                             &limit_arr,
                                             &limit_arr_len);
    if (err) {
        sxd_log_debug(dev, "Fail to parse %s and cannot access it, err: %d\n", node_name, err);
        goto err_out;
    }

    err = get_limit_bytes_holes_from_eeprom_page_access_range(limit_arr,
                                                              limit_arr_len,
                                                              page,
                                                              device_addr,
                                                              count,
                                                              &hole_arr,
                                                              &hole_arr_len);
    if (err) {
        sxd_log_notice(dev,
                       "Fail to get limitation bytes for %s in range of (%u, %lu), err: %d\n",
                       node_name,
                       device_addr,
                       count,
                       err);
        goto err_out;
    }

    if (access == MODULE_SYSFS_EEPROM_READ) {
        memset(buf, 0, count);
    }

    if (hole_arr_len == 0) {
        /* No limitation bytes in the access scope, simply call low-level access once. */
        if (access == MODULE_SYSFS_EEPROM_READ) {
            err =
                __module_sysfs_eeprom_low_level_read(dev,
                                                     buf,
                                                     count,
                                                     module,
                                                     slot,
                                                     page,
                                                     check_pnv,
                                                     device_addr,
                                                     i2c_addr);
        } else {
            err =
                __module_sysfs_eeprom_low_level_write(dev,
                                                      buf,
                                                      count,
                                                      module,
                                                      slot,
                                                      page,
                                                      check_pnv,
                                                      device_addr,
                                                      i2c_addr);
        }
        if (err < 0) {
            sxd_log_err(dev, "Fails to access (%d) module eeprom, status: %d.\n", access, err);
            goto err_out;
        }
        goto out;
    }

    err = __separate_eeprom_page_access_range_into_segments(hole_arr,
                                                            hole_arr_len,
                                                            device_addr,
                                                            count,
                                                            &access_segs,
                                                            &access_segs_len);
    if (err) {
        sxd_log_notice(dev, "Fail to separate access range (%u, %lu) for %s, err: %d\n",
                       device_addr,
                       count,
                       node_name,
                       err);
        goto err_out;
    }

    /* call low-level access for each segments separated by limitation bytes */
    for (i = 0; i < access_segs_len; i++) {
        offset = access_segs[i].begin - pos;
        if (page > 0) {
            offset -= EEPROM_UPPER_PAGE_OFFSET;
        }
        len = access_segs[i].end - access_segs[i].begin + 1;
        if (access == MODULE_SYSFS_EEPROM_READ) {
            err = __module_sysfs_eeprom_low_level_read(dev,
                                                       buf + offset,
                                                       len,
                                                       module,
                                                       slot,
                                                       page,
                                                       check_pnv,
                                                       access_segs[i].begin,
                                                       i2c_addr);
        } else {
            err = __module_sysfs_eeprom_low_level_write(dev,
                                                        buf + offset,
                                                        len,
                                                        module,
                                                        slot,
                                                        page,
                                                        check_pnv,
                                                        access_segs[i].begin,
                                                        i2c_addr);
        }
        if (err < 0) {
            sxd_log_err(dev, "Fails to access (%d) module eeprom, status: %d.\n", access, err);
            goto err_out;
        }
    }
    kfree(access_segs);

out:
    if (hole_arr) {
        kfree(hole_arr);
    }

    if (sysfs_dev_hold) {
        sx_core_sysfs_dev_release(dev);
    }

    return count;

err_out:
    if (access_segs) {
        kfree(access_segs);
    }
    if (hole_arr) {
        kfree(hole_arr);
    }

    if (sysfs_dev_hold) {
        sx_core_sysfs_dev_release(dev);
    }

    return err;
}

ssize_t sx_core_port_module_sysfs_eeprom_access(struct sx_dev                     *dev,
                                                const char                        *node_name,
                                                uint16_t                           local_port,
                                                enum sx_module_sysfs_eeprom_access access,
                                                char                              *buf,
                                                loff_t                             pos,
                                                size_t                             count)
{
    int     err = 0;
    uint8_t module = 0;
    uint8_t slot = 0;

    err = __sx_core_get_slot_and_module(dev, local_port, &slot, &module);
    if (err) {
        sxd_log_err(dev, "Fails to get module id, status: %d.\n", err);
        err = -EINVAL;
        goto out;
    }

    err = __sx_core_module_sysfs_eeprom_access(dev,
                                               node_name,
                                               slot,
                                               module,
                                               access,
                                               buf,
                                               pos,
                                               count);
out:
    return err;
}
EXPORT_SYMBOL(sx_core_port_module_sysfs_eeprom_access);

/*
 *   sx_core module sysfs nodes are like:
 *       ./sx_core/$asic/$module_id/eeprom/pages/0/i2c-0x5x/data
 *       ./sx_core/$asic/$module_id/eeprom/pages/x/data
 *   Notice: local module id is returned in multi-asic system.
 */
static int __eeprom_sysfs_get_dev_slot_module(struct kobject *kobj, struct sx_dev **dev, uint8_t *slot,
                                              uint8_t *module)
{
    int             ret = 0;
    int             module_id = 0;
    int             module_pos = 0;
    struct kobject *kobj_parent = kobj->parent;
    uint8_t         local_module_id = 0;

    if (!kobj_parent) {
        sxd_log_err(SXD_DEV_NONE, "Invalid kobj %s\n", kobject_name(kobj));
        ret = -EINVAL;
        goto out;
    }

    module_pos = 2;
    if (kobj->name[0] == 'i') {
        /* page0 has 1 more level */
        module_pos = 3;
    }
    for ( ; module_pos > 0; module_pos--) {
        if (kobj_parent) {
            kobj_parent = kobj_parent->parent;
        }
    }
    if (!kobj_parent) {
        sxd_log_err(SXD_DEV_NONE, "Invalid sysfs node entry because of null parent: kobj %s\n", kobject_name(kobj));
        ret = -EINVAL;
        goto out;
    }

    ret = kstrtoint(kobj_parent->name + strlen(MODULE_NODE_SYSFS_PREFIX), 10, &module_id);
    if (ret || (module_id > MAX_MODULE_NUM)) {
        sxd_log_err(SXD_DEV_NONE, "sysfs entry got invalid module value for %s\n", kobject_name(kobj));
        ret = -EINVAL;
        goto out;
    }

    ret = sx_core_get_possible_local_module(module_id, &local_module_id);
    if (ret) {
        goto out;
    }

    kobj_parent = kobj_parent->parent;
    ret = sx_core_asic_get_dev_global(module_id, dev);
    if (ret) {
        sxd_log_err(*dev, "sysfs entry power_on got invalid value\n");
        goto out;
    }

    *module = local_module_id;
    *slot = sx_priv(*dev)->module_to_slot_map[*module];

out:
    return ret;
}

static int __create_module_sysfs_eeprom_page0(struct kobject           *parent,
                                              struct eeprom_page0_node *node,
                                              struct bin_attribute     *bin_attr)
{
    int err = 0, i;

    node->page = kobject_create_and_add("0", parent);
    if (!(node->page)) {
        err = -ENOMEM;
        goto out;
    }
    node->i2c[0] = kobject_create_and_add("i2c-0x50", node->page);
    if (!(node->page)) {
        err = -ENOMEM;
        goto phase1_err;
    }
    node->i2c[1] = kobject_create_and_add("i2c-0x51", node->page);
    if (!(node->page)) {
        err = -ENOMEM;
        goto phase2_err;
    }

    for (i = 0; i < MODULE_EEPROM_I2C_ADDR_NUM; i++) {
        err = sysfs_create_bin_file(node->i2c[i], bin_attr);
        if (err) {
            goto phase3_err;
        }
    }

    return err;

phase3_err:
    for (; i > 0; i--) {
        sysfs_remove_bin_file(node->i2c[i - 1], bin_attr);
    }
    kobject_put(node->i2c[1]);
phase2_err:
    kobject_put(node->i2c[0]);
phase1_err:
    kobject_put(node->page);
out:
    return err;
}

static void __delete_module_sysfs_eeprom_page0(struct eeprom_page0_node *node, struct bin_attribute *bin_attr)
{
    int i;

    for (i = 0; i < MODULE_EEPROM_I2C_ADDR_NUM; i++) {
        sysfs_remove_bin_file(node->i2c[i], bin_attr);
        kobject_put(node->i2c[i]);
    }
    kobject_put(node->page);
}

static int __create_module_sysfs_eeprom_pagex(struct kobject           *parent,
                                              struct eeprom_pagex_node *node,
                                              int                       page_id,
                                              struct bin_attribute     *bin_attr)
{
    int  err = 0;
    char num_str[8];

    snprintf(num_str, sizeof(num_str), "%d", page_id);
    node->page = kobject_create_and_add(num_str, parent);
    if (!(node->page)) {
        err = -ENOMEM;
        goto out;
    }

    err = sysfs_create_bin_file(node->page, bin_attr);
    if (err) {
        goto sysfs_file_err;
    }

    return err;

sysfs_file_err:
    kobject_put(node->page);
out:
    return err;
}

static void __delete_module_sysfs_eeprom_pagex(struct eeprom_pagex_node *node, struct bin_attribute *bin_attr)
{
    sysfs_remove_bin_file(node->page, bin_attr);
    kobject_put(node->page);
}

int sx_core_create_module_sysfs_eeprom_tree(struct kobject                  *parent,
                                            struct module_sysfs_eeprom_tree *root,
                                            struct bin_attribute            *page0_attr,
                                            struct bin_attribute            *pagex_attr)
{
    int err = 0, i;

    root->eeprom = kobject_create_and_add("eeprom", parent);
    if (!(root->eeprom)) {
        err = -ENOMEM;
        goto out;
    }
    root->pages = kobject_create_and_add("pages", root->eeprom);
    if (!(root->pages)) {
        err = -ENOMEM;
        goto phase1_err;
    }

    err = __create_module_sysfs_eeprom_page0(root->pages, &(root->page0), page0_attr);
    if (err < 0) {
        goto phase2_err;
    }

    for (i = 0; i < MODULE_EEPROM_UPPER_PAGE_NUM; i++) {
        err = __create_module_sysfs_eeprom_pagex(root->pages, &(root->page[i]), i + 1, pagex_attr);
        if (err < 0) {
            goto phase3_err;
        }
    }
    return err;

phase3_err:
    for (; i > 0; i--) {
        __delete_module_sysfs_eeprom_pagex(&(root->page[i - 1]), pagex_attr);
    }
    __delete_module_sysfs_eeprom_page0(&(root->page0), page0_attr);
phase2_err:
    kobject_put(root->pages);
phase1_err:
    kobject_put(root->eeprom);
out:
    return err;
}
EXPORT_SYMBOL(sx_core_create_module_sysfs_eeprom_tree);

void sx_core_delete_module_sysfs_eeprom_tree(struct module_sysfs_eeprom_tree *root,
                                             struct bin_attribute            *page0_attr,
                                             struct bin_attribute            *pagex_attr)
{
    int i;

    for (i = 0; i < MODULE_EEPROM_UPPER_PAGE_NUM; i++) {
        __delete_module_sysfs_eeprom_pagex(&(root->page[i]), pagex_attr);
    }
    __delete_module_sysfs_eeprom_page0(&(root->page0), page0_attr);
    kobject_put(root->pages);
    kobject_put(root->eeprom);
}
EXPORT_SYMBOL(sx_core_delete_module_sysfs_eeprom_tree);

int sx_core_create_module_sysfs_default_eeprom_tree(struct kobject *parent, struct module_sysfs_eeprom_tree *root)
{
    return sx_core_create_module_sysfs_eeprom_tree(parent,
                                                   root,
                                                   &module_eeprom_page0_attribute,
                                                   &module_eeprom_pagex_attribute);
}
EXPORT_SYMBOL(sx_core_create_module_sysfs_default_eeprom_tree);

void sx_core_delete_module_sysfs_default_eeprom_tree(struct module_sysfs_eeprom_tree *root)
{
    sx_core_delete_module_sysfs_eeprom_tree(root,
                                            &module_eeprom_page0_attribute,
                                            &module_eeprom_pagex_attribute);
}
EXPORT_SYMBOL(sx_core_delete_module_sysfs_default_eeprom_tree);

static ssize_t __module_sysfs_eeprom_bin_read(struct file          *flip,
                                              struct kobject       *kobj,
                                              struct bin_attribute *attr,
                                              char                 *buf,
                                              loff_t                pos,
                                              size_t                count)
{
    int            ret = 0;
    int            read_count = 0;
    struct sx_dev *dev = NULL;
    uint8_t        module = 0;
    uint8_t        slot = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    if (pos > attr->size) {
        return 0;
    }
    if (pos + count > attr->size) {
        count = attr->size - pos;
    }
    if (count == 0) {
        return 0;
    }

    ret = __eeprom_sysfs_get_dev_slot_module(kobj, &dev, &slot, &module);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "eeprom read: __eeprom_sysfs_get_dev_slot_module failed (%d) for %s\n", ret,
                    kobject_name(kobj));
        goto out;
    }
    sx_int_log_debug(&sx_priv(dev)->module_eeprom_log,
                     "Module ID %d (caller pid:%d) | Eeprom read start, pos: %lld, count: %zu",
                     module,
                     current->pid,
                     pos,
                     count);
    read_count = __sx_core_module_sysfs_eeprom_access(dev,
                                                      kobject_name(kobj),
                                                      slot,
                                                      module,
                                                      MODULE_SYSFS_EEPROM_READ,
                                                      buf,
                                                      pos,
                                                      count);
    if (read_count < 0) {
        sxd_log_debug(SXD_DEV_NONE, "Fails to read eeprom, status: %d.\n", read_count);
        ret = read_count;
        goto out;
    }
    sx_int_log_info(&sx_priv(dev)->module_eeprom_log,
                    "Module ID %d (caller pid:%d) | Eeprom read completed, pos: %lld, read %d bytes",
                    module,
                    current->pid,
                    pos,
                    read_count);
    return read_count;

out:
    return ret;
}

static ssize_t __module_sysfs_eeprom_bin_write(struct file          *flip,
                                               struct kobject       *kobj,
                                               struct bin_attribute *attr,
                                               char                 *buf,
                                               loff_t                pos,
                                               size_t                count)
{
    int            ret = 0;
    int            write_count = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot = 0;
    uint8_t        module = 0;

    if (pos > attr->size) {
        return 0;
    }
    if (pos + count > attr->size) {
        count = attr->size - pos;
    }
    if (count == 0) {
        return 0;
    }

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, buf, count);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__,
                    ret);
        ret = 0;
        /*continue*/
    }
    ret = __eeprom_sysfs_get_dev_slot_module(kobj, &dev, &slot, &module);
    if (ret) {
        sxd_log_err(dev,
                    "eeprom write: __eeprom_sysfs_get_dev_slot_module failed (%d) for %s\n",
                    ret,
                    kobject_name(kobj));
        goto out;
    }

    sx_int_log_debug(&sx_priv(dev)->module_eeprom_log,
                     "Module ID %d (caller pid:%d) | Eeprom write start, pos: %lld, count: %zu",
                     module,
                     current->pid,
                     pos,
                     count);
    write_count = __sx_core_module_sysfs_eeprom_access(dev,
                                                       kobject_name(kobj),
                                                       slot,
                                                       module,
                                                       MODULE_SYSFS_EEPROM_WRITE,
                                                       buf,
                                                       pos,
                                                       count);
    if (write_count < 0) {
        sxd_log_err(dev, "Fails to write eeprom, status: %d.\n", write_count);
        ret = write_count;
        goto out;
    }
    sx_int_log_info(&sx_priv(dev)->module_eeprom_log,
                    "Module ID %d (caller pid:%d) | Eeprom write completed, wrote %d bytes",
                    module,
                    current->pid,
                    write_count);
    return write_count;

out:
    return ret;
}
