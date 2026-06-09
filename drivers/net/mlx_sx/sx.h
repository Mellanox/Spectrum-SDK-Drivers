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

#ifndef SX_H
#define SX_H


/************************************************
 * Includes
 ***********************************************/
#include <asm/atomic.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/skbuff.h>
#include <linux/mlx_sx/device.h>
#include <linux/mlx_sx/driver.h>
#include <linux/mlx_sx/sx_i2c_if.h>
#include <linux/timer.h>
#include <linux/mlx_sx/map.h>
#include "eq.h"
#include "fw.h"
#include "icm.h"
#include "sx_dpt.h"
#include "counter.h"
#include "internal_log.h"
#include "health_check.h"
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/timecounter.h>
#include <linux/clocksource.h>
#include <linux/net_tstamp.h>
#include <linux/ptp_clock_kernel.h>
#include <linux/bpf.h>
#include <net/genetlink.h>
#include <linux/netdevice.h>

/************************************************
 *  Defines
 ***********************************************/
#ifdef PD_BU
#define INCREASED_TIMEOUT
#endif

#define DRV_NAME    "sx_core"
#define DRV_RELDATE "May, 2011"

#define PCI_DRIVER_F_SX_DRIVER     (1 << 0) /* PCI driver initialized successfully */
#define PCI_DRIVER_F_SX_OOB_DRIVER (1 << 1) /* OOB PCI driver initialized successfully */


/* SwitchX PCI device ID */
#define SWITCHX_PCI_DEV_ID 0xc738
/* SwitchX in flash recovery mode */
#define SWITCHX_FLASH_MODE_PCI_DEV_ID 0x0246

/* Spectrum PCI device ID */
#define SPECTRUM_PCI_DEV_ID 0xcb84
/* Spectrum in flash recovery mode */
#define SPECTRUM_FLASH_MODE_PCI_DEV_ID 0x0249

/* Spectrum2 PCI device ID */
#define SPECTRUM2_PCI_DEV_ID 0xcf6c
/* Spectrum2 in flash recovery mode */
#define SPECTRUM2_FLASH_MODE_PCI_DEV_ID 0x024e

/* Spectrum3 PCI device ID */
#define SPECTRUM3_PCI_DEV_ID 0xcf70
/* Spectrum3 in flash recovery mode */
#define SPECTRUM3_FLASH_MODE_PCI_DEV_ID 0x0250

/* Spectrum4 PCI device ID */
#define SPECTRUM4_PCI_DEV_ID 0xcf80
/* Spectrum4 'in flash recovery mode' PCI device ID */
#define SPECTRUM4_FLASH_MODE_PCI_DEV_ID 0x0254
/* Spectrum4 'RMA' PCI device ID */
#define SPECTRUM4_RMA_PCI_DEV_ID 0x0255

/* Spectrum5 PCI device ID */
#define SPECTRUM5_PCI_DEV_ID 0xcf82
/* Spectrum5 'in flash recovery mode' PCI device ID */
#define SPECTRUM5_FLASH_MODE_PCI_DEV_ID 0x0270
/* Spectrum5 'RMA' PCI device ID */
#define SPECTRUM5_RMA_PCI_DEV_ID 0x0271

/* Spectrum6 PCI device ID */
#define SPECTRUM6_PCI_DEV_ID 0xcf84
/* Spectrum6 'in flash recovery mode' PCI device ID */
#define SPECTRUM6_FLASH_MODE_PCI_DEV_ID 0x0274
/* Spectrum6 'RMA' PCI device ID */
#define SPECTRUM6_RMA_PCI_DEV_ID 0x0275

/* SwitchIB PCI device ID */
#define SWITCH_IB_PCI_DEV_ID 0xcb20
/* SwitchIB in flash recovery mode */
#define SWITCH_IB_FLASH_MODE_PCI_DEV_ID 0x0247

/* SwitchIB2 PCI device ID */
#define SWITCH_IB2_PCI_DEV_ID 0xcf08

/* SwitchIB2 in flash recovery mode */
#define SWITCH_IB2_FLASH_MODE_PCI_DEV_ID 0x024B

/* Quantum PCI device ID */
#define QUANTUM_PCI_DEV_ID 0xd2f0

/* Quantum in flash recovery mode */
#define QUANTUM_FLASH_MODE_PCI_DEV_ID 0x024D

/* Quantum2 PCI device ID */
#define QUANTUM2_PCI_DEV_ID 0xd2f2
/* Quantum2 in flash recovery mode */
#define QUANTUM2_FLASH_MODE_PCI_DEV_ID 0x257
/* Quantum2 'RMA' PCI device ID */
#define QUANTUM2_RMA_PCI_DEV_ID 0x0258

/* Quantum3 PCI device ID */
#define QUANTUM3_PCI_DEV_ID 0xd2f4
/* Quantum3 in flash recovery mode */
#define QUANTUM3_FLASH_MODE_PCI_DEV_ID 0x25b
/* Quantum3 'RMA' PCI device ID */
#define QUANTUM3_RMA_PCI_DEV_ID 0x025C

/* Quantum4 PCI device ID */
#define QUANTUM4_PCI_DEV_ID 0xd2f8
/* Quantum4 in flash recovery mode */
#define QUANTUM4_FLASH_MODE_PCI_DEV_ID 0x278
/* Quantum4 'RMA' PCI device ID */
#define QUANTUM4_RMA_PCI_DEV_ID 0x0279

/* Quantum5 PCI device ID */
#define QUANTUM5_PCI_DEV_ID 0xd2fA
/* Quantum5 in flash recovery mode */
#define QUANTUM5_FLASH_MODE_PCI_DEV_ID 0x27C
/* Quantum5 'RMA' PCI device ID */
#define QUANTUM5_RMA_PCI_DEV_ID 0x027D

#define TO_FIELD(mask, shift, value) \
    (value & mask) << shift;

#define sx_err(mdev, format, arg ...)                  \
    if ((mdev)->pdev) {                                \
        sxd_log_pci_err((mdev->pdev), format, ## arg); \
    } else {                                           \
        sxd_log_err(mdev, format, ## arg);             \
    }
#define sx_warn(mdev, format, arg ...)                     \
    if ((mdev)->pdev) {                                    \
        sxd_log_pci_warning((mdev->pdev), format, ## arg); \
    } else {                                               \
        sxd_log_warning(mdev, format, ## arg);             \
    }
#define sx_notice(mdev, format, arg ...)                  \
    if ((mdev)->pdev) {                                   \
        sxd_log_pci_notice((mdev->pdev), format, ## arg); \
    } else {                                              \
        sxd_log_notice(mdev, format, ## arg);             \
    }
#define sx_info(mdev, format, arg ...)                  \
    if ((mdev)->pdev) {                                 \
        sxd_log_pci_info((mdev->pdev), format, ## arg); \
    } else {                                            \
        sxd_log_info(mdev, format, ## arg);             \
    }

#define SX_WRITE_LIMIT       2000
#define SX_TRUNCATE_SIZE_MIN 4
#define LAG_ID_INVALID       (MAX_LAG_NUM)

#define SX_CORE_UNUSED_PARAM(P)
#define MAX_SYSTEM_PORTS_IN_FILTER 256
#define MAX_LAG_PORTS_IN_FILTER    256
#define FROM_BITS_TO_U64(bits) \
    (((bits) %                 \
      64) ? ((bits) / 64) + 1 : ((bits) / 64))

#define SX_AGG_EBPF_PROG_NUM_PER_RDQ 7

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 17, 0)) || defined(CONFIG_SX_RHEL_9_4)
#define PDE_DATA_DEF pde_data
#else
#define PDE_DATA_DEF PDE_DATA
#endif

#define SX_DBG_PACKET_TYPE_ANY 0xFFFF
#define SX_DBG_EMAD_TYPE_ANY   0xFFFF
#define SX_DBG_CMD_OP_TYPE_ANY 0xFFFF
#define SX_DBG_REG_TYPE_ANY    0xFFFF

#define SX_DBG_COUNT_UNLIMITED 0xFFFF

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)) && IS_ENABLED(CONFIG_PAGE_POOL)
#define SX_PAGE_POOL_SUPPORTED 1
#else
#define SX_PAGE_POOL_SUPPORTED 0
#endif

/* PRM definition: Maximum of 3 fragments per WQE */
#define SX_MAX_FRAGS_PER_WQE (3)

/************************************************
 *  Enums
 ***********************************************/
enum {
    SX_FLAG_MSI_X = 1
};

enum TX_BASE_HEADER_DEFS {
    TX_HDR_VER_MASK_V1           = 0xF,
    TX_HDR_VER_SHIFT_V1          = 4,
    TX_HDR_CTL_MASK              = 0x3,
    TX_HDR_CTL_SHIFT             = 2,
    TX_HDR_MC_MASK               = 0x1,
    TX_HDR_MC_SHIFT              = 0,
    TX_HDR_PROTOCOL_MASK         = 0x7,
    TX_HDR_PROTOCOL_SHIFT        = 5,
    TX_HDR_ETCLASS_MASK          = 0x7,
    TX_HDR_ETCLASS_SHIFT         = 2,
    TX_HDR_SWID_MASK             = 0x7,
    TX_HDR_SWID_SHIFT            = 12,
    TX_HDR_SYSTEM_PORT_MID_MASK  = 0xFFFF,
    TX_HDR_SYSTEM_PORT_MID_SHIFT = 0,
    TX_HDR_CTCLASS3_MASK         = 0x1,
    TX_HDR_CTCLASS3_SHIFT        = 14,
    TX_HDR_RDQ_MASK              = 0x1F,
    TX_HDR_RDQ_SHIFT             = 9,
    TX_HDR_CPU_SIGNATURE_MASK    = 0x1FF,
    TX_HDR_CPU_SIGNATURE_SHIFT   = 0,
    TX_HDR_SIGNATURE_MASK        = 0xFFFF,
    TX_HDR_SIGNATURE_SHIFT       = 0,
    TX_HDR_STCLASS_MASK          = 0x7,
    TX_HDR_STCLASS_SHIFT         = 13,
    TX_HDR_EMAD_MASK             = 0x1,
    TX_HDR_EMAD_SHIFT            = 5,
    TX_HDR_TYPE_MASK             = 0xF,
    TX_HDR_TYPE_SHIFT            = 0,
    TX_HDR_RX_IS_ROUTER_MASK_V1  = 0x1,
    TX_HDR_RX_IS_ROUTER_SHIFT_V1 = 3,
    TX_HDR_RX_IS_TUNNEL_MASK_V1  = 0x1,
    TX_HDR_RX_IS_TUNNEL_SHIFT_V1 = 2,
    TX_HDR_FID_VALID_MASK_V1     = 0x1,
    TX_HDR_FID_VALID_SHIFT_V1    = 0,
    TX_HDR_CONTROL_MASK_V1       = 0x1,
    TX_HDR_CONTROL_SHIFT_V1      = 6,
    TX_HDR_ETCLASS_MASK_V1       = 0xF,
    TX_HDR_ETCLASS_SHIFT_V1      = 0,
    TX_HDR_FID_MASK_V1           = 0xFFFF,
    TX_HDR_FID_SHIFT_V1          = 0
};
enum tele_dir_ing {
    TELE_DIR_ING_EGRESS_E  = 0,
    TELE_DIR_ING_INGRESS_E = 1,
    TELE_DIR_ING_MAX_E     = 1,
    TELE_DIR_ING_NUM_E     = 2,
};

enum asic_ga_index {
    GA_ASIC_0 = 0,
    GA_ASIC_1,
    GA_ASIC_2,
    GA_ASIC_3,
    GA_MIN_INDEX = GA_ASIC_0,
    GA_MAX_INDEX = GA_ASIC_3
};

enum eeprom_management_type {
    EEPROM_MANAGEMENT_TYPE_INVALID = 0x00,
    EEPROM_MANAGEMENT_TYPE_SFP     = 0x01,
    EEPROM_MANAGEMENT_TYPE_QSFP    = 0x02,
    EEPROM_MANAGEMENT_TYPE_CMIS    = 0x03,
};

/************************************************
 *  Structs
 ***********************************************/
struct event_data {
    struct list_head       list;
    struct sk_buff        *skb;
    u16                    system_port;
    u16                    trap_id;
    u8                     dev_id;
    u8                     is_lag;
    u16                    lag_sub_port;
    u8                     swid;
    struct sx_dev         *dev;
    u32                    original_packet_size;
    struct sx_rx_timestamp rx_timestamp;
    u32                    user_def_val;
    u16                    dest_sysport;
    u8                     dest_is_lag;
    u16                    dest_lag_subport;
    u8                     mirror_reason;
    u8                     mirror_tclass;
    u16                    mirror_cong;
    u32                    mirror_lantency;
    u8                     mirror_elephant;
    u8                     channel_experienced_drop;
};
struct sx_rsc { /* sx  resource */
    struct event_data           evlist; /* event list           */
    int                         evlist_size; /* the current size     */
    spinlock_t                  lock; /* event list lock	*/
    wait_queue_head_t           poll_wait;
    atomic_t                    multi_packet_read_enable;
    atomic_t                    read_blocking_state;
    struct semaphore            write_sem;
    struct sx_dq               *bound_monitor_rdq;
    struct file               * owner;
    int                         queue_type;
    int                         channel_experienced_drop;
    atomic_t                    refcnt;
    atomic_t                    mmap_refcnt;
    ku_mmap_mem_alloc_scheme_e  mmap_alloc_scheme;
    struct list_head            list;
    struct ku_client_pid_get    client_pid;
    char                        name[32];
    struct ku_fd_set_owner_info owner_info;
    unsigned long               since;
    bool                        closing;
    bool                        tmp_device_restart_closing; /* this flag indicate that device is during remove or restart */
    sxd_dev_id_t                health_dev_id;
};
struct tx_base_header_v0 {
    u8  ctl_mc;
    u8  protocol_etclass;
    u16 swid;
    u16 system_port_mid;
    u16 ctclass3_rdq_cpu_signature;
    u32 reserved;
    u16 signature;
    u16 stclass_emad_type;
};
struct tx_base_header_v1 {
    u8  version_ctl;
    u8  protocol_rx_is_router_rx_is_tunnel_fid_valid;
    u16 swid_control_etclass;
    u16 system_port_mid;
    u8  ext_fwd_mode;
    u8  reserved1;
    u16 fid;
    u8  reserved2;
    u8  host_based_mirror_reason_mirror_agent;
    u8  reserved3[3];
    u8  type;
};
struct sx_bitmap {
    u32           max;
    spinlock_t    lock;   /* bitmap lock */
    unsigned long table[4];
};
struct sx_buf_list {
    void      *buf;
    dma_addr_t map;
};
struct sx_buf {
    struct {
        struct sx_buf_list  direct;
        struct sx_buf_list *page_list;
    } u;
    int nbufs;
    int npages;
    int page_shift;
};


/************************************************
 * EQ - Structs
 ***********************************************/
struct sx_eq {
    struct sx_dev      *dev;
    void __iomem       *ci_db;
    void __iomem       *arm_db;
    int                 eqn;
    u32                 cons_index;
    u16                 irq;
    u16                 have_irq;
    int                 nent;
    struct sx_buf_list *page_list;
};
struct sx_eq_table {
    struct sx_bitmap bitmap;
    void __iomem    *clr_int;
    u32              clr_mask;
    struct sx_eq     eq[SX_NUM_EQ];
    int              have_irq;
    u8               inta_pin;
};

/************************************************
 * CQ - Structs
 ***********************************************/

struct sx_cq;
union sx_cq_poll_context;

struct sx_cq_schedule_callbacks {
    void (*sx_cq_trigger_cb)(struct sx_cq *cq);
    void (*sx_cq_reschedule_cb)(struct sx_cq *cq);
};

struct sx_cq_poll_callbacks {
    int  (*sx_cq_poll_init_context_cb)(struct sx_cq *cq, union sx_cq_poll_context *context);
    void (*sx_cq_poll_deinit_context_cb)(struct sx_cq *cq, union sx_cq_poll_context *context);
    int  (*sx_cq_poll_one_cb)(struct sx_cq *cq, union sx_cq_poll_context *context);
};

union sx_cqe;

enum sx_cq_last_ts {
    SX_CQ_ARM_LAST, /*The last timestamp is arm ts*/
    SX_CQ_EVENT_LAST,/*The last timestamp is event ts*/
    SX_CQ_EVENT_MAX, /* for triggering failure*/
};

const char * sx_cq_thread_name(enum sx_cq_thread cq_thread);

struct sx_cq {
    u32               cons_index;
    u64               cons_index_dbg;
    u64               cons_index_dbg_snapshot;
    __be32           *set_ci_db;
    __be32           *arm_db;
    int               cqn;
    atomic_t          refcount;
    struct completion free;
    spinlock_t        lock;     /* sx_cq lock */
    struct sx_buf     buf;
    int               nent;
    struct sx_dev    *dev;
    struct sx_dq     *dq;      /* the matching dq */
    u8                cqe_version;
    size_t            cqe_sizeof;
    void              (*sx_next_cqe_cb)(struct sx_cq *cq, union sx_cqe *cqe_p);
    void              (*sx_fill_poll_one_params_from_cqe_cb)(union sx_cqe         *u_cqe,
                                                             struct sx_cqe_params *cqe_params);
    u8                                     (*sx_get_cqe_owner_cb)(struct sx_cq *cq, int n);
    void                                   (*sx_cqe_owner_init_cb)(struct sx_cq *cq);
    u32                                    wqe_counter;
    bool                                   paused;
    struct sx_rx_timestamp               * cqe_ts_arr;
    unsigned long                          armed_ts;
    unsigned long                          event_ts;
    enum sx_cq_last_ts                     last_ts; /* the last timestamp is event or arm*/
    u64                                    event_cntr; /* running counter to count the events number*/
    const struct sx_cq_schedule_callbacks *sched_callbacks;
    const struct sx_cq_poll_callbacks     *poll_callbacks;
    struct work_struct                     work;
    struct workqueue_struct               *cq_thread;
    enum sx_cq_thread                      cq_thread_id;
    bool                                   strict_prio; /* Relevant to RDQs only ==> true = strict priority, false = DWRR */
    bool                                   user_strict_prio; /* store the original user configuration in case the RDQ switch to be monitored*/
    int                                    budget; /* (depends on 'strict_prio') ==>
                                                    *  matching TC weight/index for DWRR/SP manners, V1 and V2 */
    int              user_budget;                       /* store the original user configuration in case the RDQ switch to be monitored*/
    struct list_head tasklet_resched_list;
    struct list_head sp_list;                       /* list of strict-priority CQs that has CQEs to consume */
    bool             in_sp_list;                       /* is this CQ in SP list? */
};

struct sx_cq_table {
    spinlock_t    lock;     /* cq_table lock */
    struct sx_cq *cq[NUMBER_OF_SDQS + NUMBER_OF_RDQS];

    /* following info is in CQ-table because when configured, CQ may not be created yet */
    u8 ts_type[NUMBER_OF_SDQS + NUMBER_OF_RDQS];            /* SXD_TS_TYPE_CQE_NONE, SXD_TS_TYPE_CQE_UTC, SXD_TS_TYPE_LINUX */
};

/************************************************
 * DQ - Structs
 ***********************************************/

struct sx_rx_sge_data {
#if SX_PAGE_POOL_SUPPORTED
    struct page *page; /* consecutive page(s) allocation (depends on SX_PAGE_POOL_ORDER) */
#else
    struct sk_buff *skb; /* skb is always linear (no fragments) */
#endif
    dma_addr_t dma_addr;
};

struct sx_tx_sge_data {
    u16        frag_len;
    void      *vaddr;
    dma_addr_t dma_addr;
};

struct sx_sge_internal {
    union {
        struct {
            struct sx_rx_sge_data rx_frags[SX_MAX_FRAGS_PER_WQE];
            struct sk_buff       *mon_rdq_skb; /* used only by monitoring RDQs */
        } rx;
        struct {
            struct sx_tx_sge_data tx_frags[SX_MAX_FRAGS_PER_WQE];
            struct sk_buff       *skb;
        } tx;
    } u;
};

struct sx_wqe {
    __be16 flags;
    __be16 byte_count[SX_MAX_FRAGS_PER_WQE];
    __be64 dma_addr[SX_MAX_FRAGS_PER_WQE];
};

struct sx_pkt {
    struct sk_buff  *skb;
    struct list_head list_send_to_sdq;
    u8               set_lp;
    enum ku_pkt_type type;
    struct list_head list_wait_for_completion;
    unsigned long    since;
    u16              idx;
};
enum dq_state {
    DQ_STATE_FREE,
    DQ_STATE_RESET,
    DQ_STATE_RTS,
    DQ_STATE_ERROR,
};

struct sx_dq {
    void                    (*event)(struct sx_dq *, enum sx_event);
    struct sx_dev          *dev;
    int                     dqn;
    int                     is_send;
    struct sx_buf           buf;
    spinlock_t              lock; /* sx_dq lock */
    u16                     head;
    u16                     tail; /* same type as wqe_counter in cqe */
    int                     wqe_cnt;
    int                     wqe_shift;
    struct sx_cq           *cq;  /* the matching cq */
    struct sx_sge_internal *sge;
    wait_queue_head_t       tx_full_wait; /* Not sure we need it */
    __be32                 *db;
    int                     is_flushing;
    struct sx_pkt           pkts_list; /* Send only */
    struct sx_pkt           pkts_comp_list; /* Send only */
    uint64_t                pkts_sent_to_sdq; /* Send only */
    uint64_t                pkts_recv_completion; /* completion received within time frame (Send only) */
    uint64_t                pkts_late_completion; /* completion received out of time frame (Send only) */
    uint64_t                pkts_sw_queue_drops; /* Send only */
    uint32_t                pkts_in_sw_queue; /* Send only */
    uint64_t                max_comp_time; /* Send only */
    unsigned long           max_comp_ts; /* Send only */
    unsigned long           send_watermark; /* Send only */
    unsigned long           send_occupancy; /* Send only */
    unsigned long           recv_watermark; /* Receive only */
    unsigned long           recv_occupancy; /* Receive only */
#if SX_PAGE_POOL_SUPPORTED
    struct page_pool *page_pool;       /* Receive only */
#endif /* #if SX_PAGE_POOL_SUPPORTED */
    enum dq_state     state;
    atomic_t          refcount;
    struct completion free;
    unsigned long     last_completion;       /* time stamp that marks when last completion received */
    unsigned long     last_full_queue;       /* time stamp that marks when was last time the queue was full */
    int               is_monitor;            /* rdq is cyclic monitor rdq. non valid for sdq */
    uint32_t          mon_rx_count;
    uint32_t          mon_rx_start;
    uint32_t          mon_rx_start_total;       /* start point to calculate total number of discarded packets per RDQ */
    struct sx_rsc   * file_priv_p;
    uint8_t           cpu_tclass;

    /* SW queue used for storing discarded packets which may be consumed by NOS,
     * since current HW doesn't allow to store such packet in multiple RDQs. */
    struct event_data *sw_dup_evlist_p;
    uint32_t           sw_dup_evlist_cnt;            /* number of discarded packets in the SW cyclic buffer */
    uint32_t           sw_dup_evlist_total_cnt;      /* total number of discarded packets per SW cyclic buffer (could be greater than size of the cyclic buffer) */
};
struct sx_dq_table {
    spinlock_t    lock;       /* dq_table lock */
    struct sx_dq *dq[NUMBER_OF_SDQS + NUMBER_OF_RDQS];
};
struct sx_cmd {
    struct dma_pool       *pool;
    void __iomem          *hcr;
    struct mutex           hcr_mutex;  /* the HCR's mutex */
    struct semaphore       pci_poll_sem;
    struct semaphore       i2c_poll_sem;
    struct semaphore       event_sem;
    int                    max_cmds;
    spinlock_t             context_lock;  /* the context lock */
    int                    free_head;
    struct sx_cmd_context *context;
    u16                    token_mask;
    u16                    last_reg_id;
    u8                     use_events;
    u8                     toggle;
    unsigned long          last_cmd_ts; /* timestamp in jiffies of the last command */
};

#define SX_FW_CAP_UBRIDGE            (1 << 0)
#define SX_FW_CAP_LAG_MODE           (1 << 1)
#define SX_FW_CAP_CFF                (1 << 2)
#define SX_FW_CAP_UMLABEL            (1 << 3)
#define SX_FW_CAP_MOD_SUPPORT_MASK   (0x3 << 4)
#define SX_FW_CAP_MOD_SUPPORT_OFFSET (4)
#define SX_IS_FW_CAP(priv, cap)           ((priv)->fw.fw_capabilities & (cap))
#define SX_GET_FW_CAP(priv, mask, offset) (((priv)->fw.fw_capabilities & (mask)) >> (offset))

struct sx_fw {
    u64                clr_int_base;
    u64                catas_offset;
    u32                catas_size;
    u8                 clr_int_bar;
    u8                 catas_bar;
    u8                 debug_trace;
    u8                 smkey;
    u64                fw_ver;
    u64                doorbell_page_offset;
    u8                 doorbell_page_bar;
    u64                frc_offset;
    u8                 frc_bar;
    u64                utc_sec_offset;
    u8                 utc_sec_bar;
    u64                utc_nsec_offset;
    u8                 utc_nsec_bar;
    u64                cr_dump_offset;
    u8                 cr_dump_bar;
    u16                core_clock;
    sx_map_fa_handle_t map_fa_items[SXD_MAP_MEMORY_TYPE_MAX_E + 1];
    u16                fw_pages;
    u8                 fw_hour;
    u8                 fw_minutes;
    u8                 fw_seconds;
    u16                fw_year;
    u8                 fw_month;
    u8                 fw_day;
    u32                fw_capabilities;
    u32                local_out_mb_offset;
    u32                local_out_mb_size;
    u32                local_in_mb_offset;
    u32                local_in_mb_size;
    u32                cap_dump_host_size_flat;
    u32                cap_dump_host_size_reduced_flat;
    u32                cap_dump_host_size_gw;
    u32                cap_dump_host_size_gdb;
};
union sx_cmd_ifc_registers {
    struct ku_access_pspa_reg  pspa_reg_data;
    struct ku_access_ptys_reg  ptys_reg_data;
    struct ku_access_pmlp_reg  pmlp_reg_data;
    struct ku_access_plib_reg  plib_reg_data;
    struct ku_access_spzr_reg  spzr_reg_data;
    struct ku_access_paos_reg  paos_reg_data;
    struct ku_access_pmpc_reg  pmpc_reg_data;
    struct ku_access_pmpr_reg  pmpr_reg_data;
    struct ku_access_pmtu_reg  pmtu_reg_data;
    struct ku_access_pelc_reg  pelc_reg_data;
    struct ku_access_htgt_reg  htgt_reg_data;
    struct ku_access_mfsc_reg  mfsc_reg_data;
    struct ku_access_mfsm_reg  mfsm_reg_data;
    struct ku_access_mfsl_reg  mfsl_reg_data;
    struct ku_access_pvlc_reg  pvlc_reg_data;
    struct ku_access_mcia_reg  mcia_reg_data;
    struct ku_access_mfcr_reg  mfcr_reg_data;
    struct ku_access_fore_reg  fore_reg_data;
    struct ku_access_mtcap_reg mtcap_reg_data;
    struct ku_access_mtbr_reg  mtbr_reg_data;
    struct ku_access_mmdio_reg mmdio_reg_data;
    struct ku_access_mmia_reg  mmia_reg_data;
    struct ku_access_mfpa_reg  mfpa_reg_data;
    struct ku_access_mfbe_reg  mfbe_reg_data;
    struct ku_access_mfba_reg  mfba_reg_data;
    struct ku_access_mjtag_reg mjtag_reg_data;
    struct ku_access_pmaos_reg pmaos_reg_data;
    struct ku_access_mfm_reg   mfm_reg_data;
    struct ku_access_spad_reg  spad_reg_data;
    struct ku_access_sspr_reg  sspr_reg_data;
    struct ku_access_ppad_reg  ppad_reg_data;
    struct ku_access_spmcr_reg spmcr_reg_data;
    struct ku_access_smid_reg  smid_reg_data;
    struct ku_access_mgir_reg  mgir_reg_data;
    struct ku_access_plbf_reg  plbf_reg_data;
    struct ku_access_mhsr_reg  mhsr_reg_data;
    struct ku_access_mpsc_reg  mpsc_reg_data;
};
/************************************************
 * Private data struct
 ***********************************************/
union swid_data {
    struct {
        int synd;
        u64 mac;
    } eth_swid_data;
    struct {
    } ib_swid_data;
};

struct sx_priv;
struct sx_ptp_packet_metadata;
/* Note - all these callbacks are called when the db_lock spinlock is locked! */
struct dev_specific_cb {
    int (*get_hw_etclass_cb)(struct isx_meta *meta, u8* hw_etclass);
    int (*sx_build_isx_header_cb)(struct isx_meta *meta, struct sk_buff *skb, u8 stclass,  u8 hw_etclass);
    u8  (*max_cpu_etclass_for_unlimited_mtu)(void);
    int (*sx_get_sdq_cb)(struct sx_dev *dev, enum ku_pkt_type type,
                         u8 swid, u8 etclass, u8 *stclass, u8 *sdq);
    int (*sx_get_sdq_num_cb)(struct sx_dev *dev, u8 swid, u8 etclass, u8 *sdq);
    int (*get_send_to_port_as_data_supported_cb)(u8 *send_to_port_as_data_supported);
    int (*get_rp_vid_cb)(struct sx_dev *dev, struct completion_info *comp_info, u16 *vid);
    int (*get_swid_cb)(struct sx_dev *dev, struct completion_info *comp_info, u8 *swid);
    int (*get_lag_mid_cb)(struct sx_dev *dev, u16 lag_id, u16 *mid);
    int (*get_ib_system_port_mid)(struct sx_dev *dev, u16 ib_port, u16* sys_port_mid);
    int (*sx_ptp_dev_mtpcpc_config)(struct sx_priv *priv);
    int (*sx_ptp_dev_init)(struct sx_priv *priv, ptp_mode_t ptp_mode);
    int (*sx_ptp_dev_cleanup)(struct sx_priv *priv);
    int (*sx_ptp_dump)(struct sx_priv *priv, struct seq_file *m, void *v, void *context);
    int (*sx_ptp_get_mtptpt_message_types)(u16* ptp0_msg_types, u16* ptp1_msg_types);
    int (*sx_ptp_rx_handler)(struct sx_priv                      *priv,
                             struct completion_info              *ci,
                             ptp_mode_t                           ptp_mode,
                             const struct sx_ptp_packet_metadata *pkt_meta);
    int (*sx_ptp_tx_handler)(struct sx_priv                      *priv,
                             struct sk_buff                      *skb,
                             const struct sx_ptp_packet_metadata *pkt_meta);
    int (*sx_ptp_tx_ts_handler)(struct sx_priv *priv,
                                struct sk_buff *skb,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
                                const struct timespec64 *tx_ts);
#else
                                const struct timespec *tx_ts);
#endif
    int (*sx_ptp_get_supported_tx_types)(struct sx_priv *priv, u32 *tx_types);
    int (*sx_ptp_set_tx_type)(struct sx_priv *priv, int tx_type);
    int (*sx_ptp_tx_control_to_data)(struct sx_priv  *priv,
                                     struct sk_buff **orig_skb,
                                     struct isx_meta *meta,
                                     u16              port,
                                     u8               is_lag,
                                     u8              *is_tagged);
    void (*sx_set_device_profile_update_cb)(struct ku_profile* profile, struct profile_driver_params *driver_params);
    void (*sx_init_cq_db_cb)(struct sx_cq *cq, u8 cqn, u8 *cqe_ver);
    u8   (*is_sw_rate_limiter_supported)(void);
    void (*sx_disconnect_all_trap_groups_cb)(struct sx_dev *dev);
    int  (*sx_get_phy_port_max_cb)(uint16_t *port);
    int  (*sx_get_lag_max_cb)(uint16_t *max_lag_p, uint16_t *max_port_lags_p);
    void (*sx_get_rdq_max_cb)(uint8_t *max_rdqs, uint16_t *max_entry_size);
    int  (*is_eqn_cmd_ifc_only_cb)(int eqn, u8 *is_cmd_ifc_only);
    int  (*sx_clock_dev_init)(struct sx_priv *priv);
    int  (*sx_clock_dev_cleanup)(struct sx_priv *priv);
    int  (*sx_clock_cqe_ts_to_utc)(struct sx_priv *priv,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
                                   const struct timespec64 *cqe_ts,
                                   struct timespec64       *utc);
#else
                                   const struct timespec *cqe_ts,
                                   struct timespec       *utc);
#endif

    int (*sx_clock_dump)(struct sx_priv *priv, struct seq_file *m, void *v, void *context);
    int (*sx_clock_ptp_pps_init)(struct sx_priv *priv);
    u16 (*cap_max_mtu_get_cb)(bool to_fw);  /* to_fw ==> false: packet is destined to the network,
                                             *            true:  packet is destined to FW */
    int  (*synce_init_cb)(struct sx_dev *dev);
    void (*synce_deinit_cb)(struct sx_dev *dev);
    int  (*chip_reset_cb)(struct sx_dev *dev);
    int  (*chip_emergency_reset_cb)(struct sx_dev *dev);
    int  (*chip_post_reset_cb)(struct sx_dev *dev);
    u32  (*get_system_status_mem_offset_cb)(void);
    u8   (*is_mirror_header_v2_ext_cb)(void);
    void (*multi_asic_sysfs_env_init)(struct sx_dev *dev);
    int  (*prepare_dev_for_secured_dump_cb)(struct sx_priv *priv);
    u64  (*get_base_info_pointer_mem_cb)(void);
    bool (*support_loopback_modules_cb)(void);
    bool (*is_system_hidden_port)(u16 local_port);
};

struct rif_info {
    u16 local_port;
    u16 vlan_id;
    u8  is_lag;
    u16 lag_id;
    u8  is_valid;
};

enum ptp_spc1_counters {
    PTP_COUNTER_SPC1_FIFO_TRAP,
    PTP_COUNTER_SPC1_GC_EVENTS,
    PTP_COUNTER_SPC1_GC_RECORDS,
    PTP_COUNTER_SPC1_FIFO_OVERFLOW,
    PTP_COUNTER_SPC1_OUT_OF_MEMORY,
    PTP_COUNTER_SPC1_PENDING_EVENTS,
    PTP_COUNTER_SPC1_PENDING_RECORDS,
    PTP_COUNTER_SPC1_EVENT_ARRIVED_FIRST,
    PTP_COUNTER_SPC1_RECORD_ARRIVED_FIRST,
    PTP_COUNTER_SPC1_EMPTY_TS,
    PTP_COUNTER_SPC1_REG_ACCESS_SUCCEEDED,
    PTP_COUNTER_SPC1_REG_ACCESS_FAILED,
    PTP_COUNTER_SPC1_LAST
};
struct ptp_record {
    struct list_head     list;
    unsigned long        since;
    struct mtpptr_record mtpptr;
};
struct sx_ptp_packet_fields {
    u16 seqid;
    u8  domain;
    u8  msg_type;
};
struct sx_ptp_packet_metadata {
    u16                         sysport_lag_id;
    u8                          timestamp_required;
    struct sx_ptp_packet_fields pkt_fields;
};
struct ptp_common_event_data {
    struct list_head            list;
    u8                          need_timestamp;
    unsigned long               since;
    struct sx_ptp_packet_fields pkt_fields;
};
struct ptp_rx_event_data {
    struct ptp_common_event_data common;
    struct completion_info       ci;
};
struct ptp_tx_event_data {
    struct ptp_common_event_data common;
    struct sk_buff              *skb;
};

typedef void (*ptp_db_handle_cb_t)(struct sx_priv *priv, struct ptp_common_event_data *ced, u64 frc);
typedef void (*ptp_db_gc_cb_t)(struct ptp_common_event_data *ced);

#define PTP_MAX_PORTS   (MAX_PHYPORT_NUM + MAX_LAG_NUM)
#define PTP_MAX_RECORDS (4)

struct ptp_db {
    struct list_head   sysport_events_list[PTP_MAX_PORTS];
    struct list_head   sysport_records_list[PTP_MAX_PORTS];
    spinlock_t         sysport_lock[PTP_MAX_PORTS];
    u8                 direction;
    ptp_db_handle_cb_t handle_cb;
    ptp_db_gc_cb_t     gc_cb;
};

enum ptp_spc2_counters {
    PTP_COUNTER_SPC2_TIMESTAMP_ARRIVED,
    PTP_COUNTER_SPC2_ONE_STEP_TIMESTAMP_ARRIVED,
    PTP_COUNTER_SPC2_EMPTY_TIMESTAMP,
    PTP_COUNTER_SPC2_NO_TIMESTAMP,
    PTP_COUNTER_SPC2_TX_WA,
    PTP_COUNTER_SPC2_LAST
};

enum ptp_common_counters {
    PTP_COUNTER_COMMON_TOTAL,
    PTP_COUNTER_COMMON_NEED_TIMESTAMP,
    PTP_COUNTER_COMMON_RATE_LIMIT,
    PTP_COUNTER_COMMON_LAST
};
enum ptp_trap_counters {
    PTP_COUNTER_TRAP_PTP0,
    PTP_COUNTER_TRAP_PTP1,
    PTP_COUNTER_TRAP_OTHER,
    PTP_COUNTER_TRAP_PTP0_ERROR,
    PTP_COUNTER_TRAP_PTP1_ERROR,
    PTP_COUNTER_TRAP_LAST
};

#define PTP_PPS_MAX_PIN_NUM 8
typedef enum pps_pin_cap_mode {
    PPS_PIN_CAP_MODE_SUPPORT_PPS_IN_E      = 0x1,
    PPS_PIN_CAP_MODE_SUPPORT_PPS_OUT_E     = 0x2,
    PPS_PIN_CAP_MODE_SUPPORT_VIRTUAL_PIN_E = 0x4
} pps_pin_cap_mode_t;
struct ptp_pps {
    pps_pin_cap_mode_t pin_caps[PTP_PPS_MAX_PIN_NUM];
    bool               enabled;
};

struct listener_register_filter_entry {
    int is_global_register;
    int is_global_filter;
    u64 ports_registers[FROM_BITS_TO_U64((MAX_PHYPORT_NUM + 1))];
    u64 vlans_registers[FROM_BITS_TO_U64((SXD_MAX_VLAN_NUM + 1))];
    u64 lags_registers[FROM_BITS_TO_U64((MAX_LAG_NUM + 1))];
    u64 ports_filters[FROM_BITS_TO_U64((MAX_PHYPORT_NUM + 1))];
    u64 vlans_filters[FROM_BITS_TO_U64((SXD_MAX_VLAN_NUM + 1))];
    u64 lags_filters[FROM_BITS_TO_U64((MAX_LAG_NUM + 1))];
};

/* TARGET_PID_DONT_CARE defines the don't care value for target-process-id. */
#define TARGET_PID_DONT_CARE 0

struct listener_entry {
    u8                                    swid;
    enum l2_type                          listener_type;
    struct ku_client_pid_get              client_pid;
    char                                  pname[64];  /* process name that registered the trap */
    u8                                    is_default; /* is a default listener  */
    union ku_filter_critireas             critireas;  /* more filter critireas  */
    cq_handler                            handler;    /* The completion handler */
    char                                  handler_str[SX_LISTENER_HANDLER_STR_MAX_LEN]; /* (for debug only) */
    void                                 *context;      /* to pass to the handler */
    u64                                   rx_pkts;
    struct listener_register_filter_entry listener_register_filter;
    struct list_head                      list;
    struct list_head                      gc_list;
};

struct listeners_and_rf_info {
    struct list_head per_synd_list[NUM_HW_SYNDROMES + 1];
};

struct sx_mmap_info {
#define SX_MMAP_HASH_TABLE_BUCKETS (64)
    struct hlist_head hash_tb[SX_MMAP_HASH_TABLE_BUCKETS];
    u64               seq;
};

typedef struct sx_module_independent_params {
    enum sx_module_hw_present_state hw_present;
    enum sx_module_power_good_state power_good;
    enum sx_module_interrupt_state  interrupt;
} sx_module_independent_params_t;

struct module_info {
    enum sx_module_power_mode_policy power_mode_policy;
    enum sx_module_power_mode        power_mode;
    uint16_t                         num_ports_up;
    enum sx_module_present_state     present;
    sx_module_independent_params_t   independent_params;
};

enum sx_kobject_type {
    SX_KOBJECT_ASIC           = 0,
    SX_KOBJECT_ASIC_TEMP      = 1,
    SX_KOBJECT_ASIC_PERF_CNTR = 2,
    SX_KOBJECT_MAX            = SX_KOBJECT_ASIC_PERF_CNTR,
};

enum sx_probe_steps {
    SX_PROBE_STEP_PCI_SETTINGS,
    SX_PROBE_STEP_ADD_DEV,
    SX_PROBE_STEP_INIT_DPT,
    SX_PROBE_STEP_CREATE_CQ_THREADS,
    SX_PROBE_STEP_CHAR_DEV,
    SX_PROBE_STEP_CMD_POOL_CREATE,
    SX_PROBE_STEP_CMD_INIT_PCI,
    SX_PROBE_STEP_INIT_CALLBACKS,
    SX_PROBE_STEP_RESET,
    SX_PROBE_STEP_INIT_BOARD,
    SX_PROBE_STEP_MAP_DOORBELL,
    SX_PROBE_STEP_MAP_CR_SPACE,
    SX_PROBE_STEP_SET_ASYNC_QUEUES,
    SX_PROBE_STEP_INIT_EMAD,
    SX_PROBE_STEP_INIT_CLOCK,
    SX_PROBE_STEP_INIT_SYNCE,
    SX_PROBE_STEP_INIT_BER_MONITOR,
    SX_PROBE_STEP_INIT_CR_DUMP,
    SX_PROBE_STEP_INIT_BULK_COUNTERS,
    SX_PROBE_STEP_INIT_PERF_COUNTERS,
    SX_PROBE_STEP_INIT_HEALTH_CHECK,
    SX_PROBE_STEP_INIT_SYSFS,
    SX_PROBE_STEP_INIT_DBG,
};

#define SX_PROBE_STEP_SET(priv, step)                      \
    do { (priv)->pci_probe_steps_bitmap |= 1ULL << (step); \
    } while (0)
#define SX_PROBE_STEP_CHECK(priv, step)               ((priv)->pci_probe_steps_bitmap & (1ULL << (step)))
#define MULTI_ASIC_MODULE_SYSFS_IS_SUPPORT(sx_priv_p) ((sx_priv_p)->mmam_support)

#define RDQ_USAGE_MONITOR_TIMER_INTERVAL_DEFAULT_MS 2000
#define RDQ_USAGE_MONITOR_TIMER_INTERVAL_MIN_MS     1000
#define RDQ_USAGE_MONITOR_TIMER_INTERVAL_MAX_MS     100000


enum sx_reset_type {
    SX_RESET_TYPE_NONE,
    SX_RESET_TYPE_EMERGENCY, /* when FW is dead: legacy / only pci-toggle */
    SX_RESET_TYPE_NORMAL     /* MRSR(1) or MRSR(6) with pci-toggle        */
};

struct sx_rdq_use_monitor {
    struct workqueue_struct *wq;
    struct delayed_work      ws;
    bool                     stopped;
    bool                     disabled; /* for debug purpose only. timers will be active but periodic work is NOP */
    u32                      interval_ms;
};

typedef struct sxd_bulk_cntr_streaming_profile_attrs {
    sxd_boolean_t                         is_allocated;
    sxd_bulk_cntr_streaming_profile_cfg_t attrs;
    struct genl_family                    genl_family;
    struct genl_multicast_group           genl_mcgrps[SXD_BULK_CNTR_STREAMING_GENETLINK_MC_GROUP_NUM];
    struct net                           *net_ns;
} sxd_bulk_cntr_streaming_profile_attrs_t;

typedef struct sxd_bulk_cntr_streaming_buffer_id_attrs {
    sxd_boolean_t                         is_allocated;     /* whether the buffer is created. */
    sxd_boolean_t                         is_buffer_allocated;  /* whether its steaming buffer is allocated. */
    sxd_boolean_t                         in_use;
    sxd_boolean_t                         curr_sample_err;
    uint32_t                              type;
    uint32_t                              buffer_size;
    uint32_t                              profile_id;
    uint32_t                              buffer_alloc_err;
    uint32_t                              sample_err;
    struct sk_buff                       *genl_skb;
    void                                 *genl_payload;
    struct genl_family                   *genl_family_p;
    struct net                           *net_ns;
    uint16_t                              ipfix_num;
    uint16_t                              running_ipfix_idx;
    uint16_t                              ipfix_received_bytes[SXD_BULK_CNTR_STREAMING_IPFIX_MSG_LIST_MAX_NUM];
    uint16_t                              ipfix_expected_bytes[SXD_BULK_CNTR_STREAMING_IPFIX_MSG_LIST_MAX_NUM];
    uint8_t                              *ipfix_running_offset[SXD_BULK_CNTR_STREAMING_IPFIX_MSG_LIST_MAX_NUM];
    sxd_bulk_cntr_buffer_layout_common_t *layout; /* the real allocated size should also consider its type size, e.g, size of sxd_bulk_cntr_buffer_layout_hft_t for HFT type */
} sxd_bulk_cntr_streaming_buffer_id_attrs_t;

struct sx_health_public_info {
    bool          is_fatal;
    u64           tasklet_start_cnt;
    u64           tasklet_end_cnt;
    unsigned long tasklet_last_start;                /* timestamp in jiffies */
    u64           cmd_ifc_num_of_pck_received;
    u32           tasklet_max_duration;    /* in jiffies */
};

typedef struct sx_mcam_reg_data {
    struct ku_access_mcam_reg group_0X9100;
} sx_mcam_reg_data_t;

struct sx_dq_info {
    u32 log2_dq_sz;
    u64 dma_addr[8];
};

#define PARALLEL_MCIA_HASH_BITS 9

typedef enum parallel_mcia_access_completion_flag {
    PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_UNUSED    = 0,
    PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_ALLOCATED = 1,
    PARALLEL_MCIA_ACCESS_COMPLETION_FLAG_TRAPPED   = 2   /* MCIA trap is received and to be handled */
} parallel_mcia_access_completion_flag_t;

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
    parallel_mcia_access_completion_t *completions;
    struct mutex                       async_db_mutex;
    uint32_t                           completions_num;
    uint32_t                           allocated_completions;
    uint32_t                           max_allocated_completions;
    uint32_t                           active_parallel_mcia_sessions; /* Number of active parallel MCIA access sessions at the moment. */
    uint32_t                           total_parallel_mcia_sessions; /* Number of total parallel MCIA access sessions till now. */
    uint64_t                           timeout_session_counter; /* Number of parallel MCIA access sessions that is timed out. */
    uint64_t                           async_request_err_counter; /* Number of parallel MCIA access sessions that request fails. */
    uint64_t                           unmatched_trap_counter; /* Number of parallel MCIA access sessions that is trapped but without matched session. */
    uint64_t                           unused_trap_counter; /* Number of parallel MCIA access sessions that is trapped and matched but no more used. */
    uint64_t                           matched_trap_counter; /* Number of parallel MCIA access sessions that is trapped and matched and still used. */
    uint64_t                           duplicated_trap_counter; /* Number of parallel MCIA access sessions that is trapped more than once. */
    uint64_t                           unknown_trap_counter; /* Number of parallel MCIA access sessions that is trapped with unknown session flag. */
    DECLARE_HASHTABLE(key_hash_table, PARALLEL_MCIA_HASH_BITS);
} parallel_mcia_access_db_t;

/* Initial value for I2C bus semaphore - allows 2 concurrent accesses per I2C bus */
#define I2C_BUS_SEM_INIT_COUNT 2

/* Number of access hash nodes and access statuses per I2C bus */
#define I2C_BUS_ACCESS_HASH_NUM 2

/* Number of access hash nodes and access statuses per I2C bus */
#define MAX_MODULE_NUM_IN_I2C_BUS 16

#define MAX_I2C_BUS_NUM 64

/* I2C bus entry - per I2C bus */
typedef struct i2c_bus_entry {
    uint8_t          num_modules_mapped;                          /* Number of modules mapped to this bus, set at init time */
    uint8_t          mapped_modules[MAX_MODULE_NUM_IN_I2C_BUS];              /* List of module IDs mapped to this I2C bus, set at init time */
    struct semaphore sem;                                         /* Semaphore for I2C bus access, init to I2C_BUS_SEM_INIT_COUNT */
    uint64_t         tx_counter;
    uint64_t         rx_counter;
    uint64_t         timeout_counter;
    struct mutex     i2c_counters_mutex;
} i2c_bus_entry_t;

/* I2C bus mapping database */
typedef struct i2c_bus_mapping_db {
    uint8_t         num_i2c_buses;                            /* Number of unique I2C buses in the system, set at init time */
    uint8_t         num_modules;                            /* Number of modules in the system, set at init time */
    uint8_t         module_to_i2c_bus[MAX_MODULE_NUM];        /* Per-module: I2C bus index it is mapped to, set at init time */
    i2c_bus_entry_t i2c_buses_db[MAX_I2C_BUS_NUM];              /* Per-I2C bus entry array */
} i2c_bus_mapping_db_t;

typedef struct parallel_mod_access_db {
    parallel_mcia_access_db_t parallel_mcia_access_db;
    i2c_bus_mapping_db_t      i2c_bus_mapping_db;             /* I2C bus to module mapping DB */
    struct workqueue_struct  *parallel_mod_access_wq_p;       /* For parallel module access workqueue.*/
    bool                      parallel_mod_access_initialized;
    struct mutex              parallel_mod_access_wq_init_mutex;
} async_mode_access_db_t;

struct sx_priv {
    struct sx_dev              dev;
    u64                        pci_probe_steps_bitmap;
    bool                       kobj_add_done;
    struct list_head           dev_list;
    struct list_head           ctx_list;
    spinlock_t                 ctx_lock;       /* the ctx_list's lock */
    struct sx_cmd              cmd;
    struct sx_eq_table         eq_table;
    struct sx_cq_table         cq_table;
    struct sx_dq_table         sdq_table;
    struct sx_dq_table         rdq_table;
    struct sx_rdq_use_monitor  rdq_use_monitor;
    struct sx_bitmap           swid_bitmap;
    struct sx_fw               fw;
    int                        is_fw_initialized;
    bool                       registered;
    void __iomem              *clr_base;
    union swid_data            swid_data[NUMBER_OF_SWIDS];
    struct ku_l2_tunnel_params l2_tunnel_params;
    bool                       dev_specific_cb_init_done;
    struct sx_device_info      dev_info;
    bool                       dev_sw_rst_flow;
    unsigned long              flags;
    char                       board_id[SX_BOARD_ID_LEN];
    u16                        vsd_vendor_id;
    bool                       global_flushing;
    u64                        bar0_dbregs_offset;
    u8                         bar0_dbregs_bar;
    void __iomem              *db_base;
    struct sx_perf_cntr        perf_cntr;
    bool                       sniffer_enable;
    bool                       sniffer_read_enable;
    bool                       prm_sniffer_enable;
    bool                       drop_ib_mads;  /* During ISSU IB MADs should not be sent to FW */
    struct workqueue_struct   *generic_wq;

    struct {
        struct workqueue_struct *wq[SX_CQ_THREAD_MAX];
        struct work_struct       sp_work;
        struct list_head         tasklet_resched_list; /* lock is not needed, accessed only in tasklet */
        spinlock_t               lock; /* use to access sp_list, paused_cq_bitmap and cq->sp_scheduled */
        struct list_head         sp_list;
        uint64_t                 paused_cq_bitmap;
    } cq_threads;

    struct {
        struct listeners_and_rf_info *info;
        struct mutex                  lock;
    } listeners_and_rf_db;

    /* cr space address and size */
    void __iomem *cr_space_start;
    u32           cr_space_size;

    struct {
        spinlock_t            profile_lock;   /* the profile's lock */
        struct ku_profile     dev_profile;
        struct sx_pci_profile pci_profile;
        bool                  pci_profile_set;
        bool                  dev_profile_set;
        bool                  first_ib_swid;
    } profile;

    struct {
        u32 vsc_start;
        u32 running_counter;
    } secured_cr;

    struct {
        bool                          valid;
        struct ku_loopback_ports_info info; /* all system ports that have loopback modules */
    } loopback_ports;

    struct {
        struct proc_dir_entry *root_dir;
        struct mutex           lock;
        struct list_head       read_entries;
        struct list_head       write_entries;
    } dbg_dump_proc;

    struct {
        struct list_head list;
        struct mutex     lock;
        uint32_t         num_of_logs;
    } internal_logs;

    /* Health check */
    struct {
        void                        *priv_info;
        struct sx_health_public_info public_info;
    } health_check;

    /* BER monitor */
    struct {
        spinlock_t               lock;
        struct workqueue_struct *wq;
        u8                       port_ber_monitor_bitmask[MAX_PHYPORT_NUM + 1];
        u8                       port_ber_monitor_state[MAX_PHYPORT_NUM + 1];
    } bm_info;

    struct {
        struct sx_mmap_info *info;
        spinlock_t           lock;
    } mmap_db;

    /* AccuFlow */
    struct {
        struct {
            spinlock_t lock;
            bool       counters_initialized;
            bool       deinit_in_progress;
            void      *priv_counters_ctx;
            void      *priv_stats;
        } counters;
        struct {
            spinlock_t lock;
            bool       dbg_inited;
            bool       init_in_progress;
            bool       deinit_in_progress;
            void      *priv_dbg_db;
        } debug;
    } accuflow;

    /* HW clock and PTP */
    struct {
        struct ptp_clock        *clock;
        struct ptp_clock_info    clock_info;
        struct workqueue_struct *wq;
        struct sx_core_map       clock_setters; /* database of processes that touched the clock */
        struct list_head         clock_setters_lru; /* least-recent-used list */
        struct mutex             clock_setters_lock;
        u32                      clock_setters_num_entries;
        struct sx_internal_log   log_settime;
        struct sx_internal_log   log_adjtime;
        struct sx_internal_log   log_adjfreq;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0))
        struct sx_internal_log log_adjphase;
#endif
        struct sx_internal_log log_activity;
        struct sx_internal_log log_synce;
        bool                   initialized;
        union {
            struct {
                void __iomem       *frc_base;
                struct delayed_work overflow_dwork;
                unsigned long       overflow_period;
                struct cyclecounter cycles;     /* hardware abstraction for a clock */
                struct timecounter  clock;     /* layer above a %struct cyclecounter which counts nanoseconds */
                u32                 nominal_c_mult;     /* Hardware nominal mult */
                int                 freq_adj;     /* diff of frequency adjustment (relevant to SPC1 only) */
                bool                time_adj;     /* is there any time adjustment recently? (relevant to SPC1 only) */
                spinlock_t          clock_lock;       /* R/W lock */
                struct delayed_work set_hw_time_dwork;
            } spc1;
            struct {
                void __iomem  *utc_sec;
                void __iomem  *utc_nsec;
                struct ptp_pps pps_info;
                s32            adjtime_range; /* + and - */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0))
                bool is_phase_adj_supported;
                s32  min_phase_adj_ns;
                s32  max_phase_adj_ns;
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0))
                bool is_scaled_ppm_freq_adj_supported;
                long max_scaled_ppm_freq_adj_ppb;
                long max_freq_adj_ppb;
#endif
            } spc2; /* and SPC3 */
        } hwd;
    } hw_clock;

    /* EMAD management */
    struct {
        u32              flags;
        struct list_head transactions_list;
        spinlock_t       transactions_lock; /* must be spinlock because __emad_rx() is running in interrupt context */
        u32              tid_lsb; /* running counter */
        u32              timeout_usec;
    } emad;

    struct {
        ptp_mode_t             ptp_mode;
        bool                   hw_cleanup_required; /* Set when user explicitly disables PTP via IOCTL */
        struct delayed_work    per_sec_dwork;
        atomic64_t             rx_budget[PTP_MAX_PORTS];
        atomic64_t             tx_budget[PTP_MAX_PORTS];
        atomic64_t             common_counters[2][PTP_COUNTER_COMMON_LAST];
        atomic64_t             traps_counters[PTP_COUNTER_TRAP_LAST];
        int                    ptp_cqn;
        struct sx_internal_log log;  /* log of errors and major events */
        union {
            struct {
                atomic64_t          counters[2][PTP_COUNTER_SPC1_LAST];
                atomic64_t          records_dist[2][PTP_MAX_RECORDS + 1];
                u64                 max_diff[2];    /* max time diff between event and record */
                unsigned long       max_diff_ts[2];    /* max time difference timestamp */
                struct ptp_db       rx_db;
                struct ptp_db       tx_db;
                struct delayed_work gc_dwork;
            } spc1;
            struct {
                atomic64_t             counters[2][PTP_COUNTER_SPC2_LAST];
                unsigned long          max_tx_ts_diff; /* max time between packet tx and its timestamp in jiffies */
                struct sx_internal_log log_rx;
                struct sx_internal_log log_tx;
                struct sx_internal_log log_pps_events;

                /* TX type is unique for all net-devices (no hybrid mode) */
                int global_tx_type;
                u8  global_tx_msg_type_one_step_bitmap;

                /* we need cap_fid in favor of the PTP WA */
                u16 cap_fid;
            } spc2; /* and SPC3 */
        } hwd;
    } ptp;

    /* multi-dev support */
    struct {
        u64 rx_by_pkt_type[NUMBER_OF_SWIDS + 1][SX_PKT_TYPE_MAX];
        u64 tx_by_pkt_type[NUMBER_OF_SWIDS + 1][SX_PKT_TYPE_MAX];
        u64 tx_by_pkt_type_bytes[NUMBER_OF_SWIDS + 1][SX_PKT_TYPE_MAX];
        u64 rx_by_synd[NUMBER_OF_SWIDS + 1][NUM_HW_SYNDROMES + 1];
        u64 rx_by_synd_bytes[NUMBER_OF_SWIDS + 1][NUM_HW_SYNDROMES + 1];
        u64 rx_unconsumed_by_synd[NUM_HW_SYNDROMES + 1][PKT_TYPE_NUM];
        u64 rx_eventlist_by_synd[NUM_HW_SYNDROMES + 1];
        u64 rx_eventlist_drops_by_synd[NUM_HW_SYNDROMES + 1];
        u64 rx_by_rdq[NUMBER_OF_SWIDS + 1][NUMBER_OF_RDQS];
        u64 rx_by_rdq_bytes[NUMBER_OF_SWIDS + 1][NUMBER_OF_RDQS];
        u64 tx_loopback_ok_by_synd[NUM_HW_SYNDROMES + 1];
        u64 tx_loopback_dropped_by_synd[NUM_HW_SYNDROMES + 1];
        u64 eventlist_drops_counter;
        u64 unconsumed_packets_counter;
        u64 filtered_lag_packets_counter;
        u64 filtered_port_packets_counter;
        u64 loopback_packets_counter;
    } stats;

    /* cr_dump info */
    struct {
        u32        cap_dump_host_size_flat;
        u32        cap_dump_host_size_reduced_flat;
        u32        cap_dump_host_size_gw;
        u32        cap_dump_host_size_gdb;
        u32        cap_current_dump_host_size;
        u8         dumped_sn;
        u16        trans_sn;
        u32        dumped_bytes;
        u32        cr_dump_memblk_size;
        dma_addr_t dma_addr;
        bool       dma_buff_used;
        int        dma_size;
        bool       gdb_dump_mode;
    } cr_dump_info;

    /* reset info */
    struct {
        enum sx_reset_type last_chip_reset_type;
        u32                duration_msec;
        u32                system_status_mem_offset;
        bool               in_pci_restart;
        bool               emergency_reset_done;
    } reset_info;

    /* reset capability */
    struct {
        bool reset_cap_initialized;
        bool mrsr6_with_sbr_is_supported;
        bool mrsr6_is_supported;
        bool mrsrft_is_supported;
    } reset_cap;

    struct sx_dev_cap dev_cap;

    /* sysfs/external-tool access to device */
    struct {
        struct rw_semaphore rw_lock;        /* grant read/write access to this structure */
        atomic_t            num_of_readers; /* current number of readers (for debug only)
                                             *  [only readers update it] */
        unsigned long locker_ts;            /* timestamp of oldest reader/writer in jiffies (for debug only) */
        bool          locked_by_writer;       /* since writer will not hold the lock in the whole duration of the 'disable'
                                               *  operation (sx_core_sysfs_dev_disable and sx_core_sysfs_dev_enable are
                                               *  called from different contexts), need this boolean to mark it is used by
                                               *  a writer. */
        char writer_desc[64];                /* in case sysfs is disabled, it contains the description of the one who
                                              *  did it (for debug only) */
    } sysfs_access_info;

    /* bulk counters */
    struct {
        struct transaction_entry          *in_progress[SXD_BULK_CNTR_KEY_TYPE_LAST_E];
        u32                                in_progress_cnt;
        bool                               accuflow_reservation;
        bool                               hft_reservation;
        spinlock_t                         bulk_cntr_db_lock;
        struct delayed_work                gc_dwork;
        sxd_port_per_prio_counter_cache_t *per_prio_counter_cache;
        u64                                bulk_counter_buffer_hft_utilization;
        u64                                bulk_counter_max_buffer_hft_limit;
        struct {
            sxd_bulk_cntr_event_id_t event_id;
            unsigned long            start_time;
            u32                      duration; /* msecs */
        } top_durations[SXD_BULK_CNTR_KEY_TYPE_LAST_E]; /* debug info */
        sxd_bulk_cntr_streaming_profile_attrs_t   streaming_profiles[SXD_BULK_CNTR_STREAMING_PROFILE_MAX_NUM];
        sxd_bulk_cntr_streaming_buffer_id_attrs_t streaming_buffer_ids[SXD_BULK_CNTR_STREAMING_BUFFER_ID_MAX_NUM];
    } bulk_counters;

    /* internal debug (testing, monitoring) - no operational impact! */
    struct {
        atomic_t          tx_packets_in_flight;
        bool              enable_cpu_port_loopback;
        struct sx_dq_info sdq_info[NUMBER_OF_SDQS];
        struct sx_dq_info rdq_info[NUMBER_OF_RDQS];
    } debug_only;

    /* IB only */
    u16 ib_to_local_db[MAX_IBPORT_NUM + 1];
    /* ETH only */
    u16                       system_to_local_db[MAX_SYSPORT_NUM];
    u16                       local_to_system_db[MAX_PHYPORT_NUM + 1];
    u16                       lag_member_to_local_db[MAX_LAG_NUM][MAX_LAG_MEMBERS_NUM];
    u8                        local_is_rp[MAX_PHYPORT_NUM + 1];
    u16                       local_rp_vid[MAX_PHYPORT_NUM + 1];
    u8                        lag_is_rp[MAX_LAG_NUM];
    u16                       lag_rp_vid[MAX_LAG_NUM];
    u16                       fid_to_hwfid[MAX_FIDS_NUM];
    u16                       rif_id_to_hwfid[MAX_RIFS_NUM];
    struct rif_info           rif_data[MAX_RIFS_NUM];
    u8                        local_to_slot_map[MAX_PHYPORT_NUM + 1];
    u8                        local_to_module_map[MAX_PHYPORT_NUM + 1];
    u8                        local_to_submodule_map[MAX_PHYPORT_NUM + 1];
    u8                        module_to_slot_map[MAX_MODULE_NUM];
    u16                       modules_to_port_map_count[MAX_SLOT_NUM][MAX_MODULE_NUM];
    u16                       module_to_ports_map[MAX_SLOT_NUM][MAX_MODULE_NUM][MAX_LANE_NUM_PER_MODULE];
    sxd_paos_admin_status_t   local_port_state[MAX_PHYPORT_NUM + 1];
    struct module_info        module_data[MAX_SLOT_NUM][MAX_MODULE_NUM];
    struct mutex              module_access_mutex;
    struct kobject           *kobj[SX_KOBJECT_MAX + 1];
    struct module_sysfs_node *module_sysfs_arr;
    bool                      apply_im_supported;
    struct sx_internal_log    module_log;        /* Log of errors and major events */
    struct sx_internal_log    module_eeprom_log; /* Log of eeprom accesses */
    bool                      module_log_init;
    struct {
        sxd_module_master_mode_e module_support_type;
        bool                     frequency_support;
    } independent_module_params;
    /* TEC capability cache */
    bool tec_support;
    u8   module_eeprom_max_access_size;
    bool module_eeprom_async_access_mode;
    bool is_i2c_utl_supported;

    struct {
        struct mutex           access_mutex;
        u8                     eeprom_types[MAX_MODULE_NUM];
        async_mode_access_db_t async_mod_access_db;
    } module_eeprom_access_db;
    struct mutex module_eeprom_mcia_access_mutex;  /* sync for consecutive MCIA accesses in parallel EEPROM access mode, e.g., for pnv check for independent module. */
    /* common */
    u8                        local_to_swid_db[MAX_PHYPORT_NUM + 1];
    spinlock_t                db_lock;   /* Lock for all DBs */
    u16                       pvid_sysport_db[MAX_SYSPORT_NUM];
    u16                       pvid_lag_db[MAX_LAG_NUM];
    u16                       truncate_size_db[NUMBER_OF_RDQS];
    u16                       sysport_filter_db[NUM_HW_SYNDROMES][MAX_SYSTEM_PORTS_IN_FILTER];
    u16                       lag_filter_db[NUM_HW_SYNDROMES][MAX_LAG_PORTS_IN_FILTER];
    u8                        lag_oper_state[MAX_LAG_NUM];
    u8                        tele_thrs_state[MAX_PHYPORT_NUM + 1][TELE_DIR_ING_NUM_E];
    u64                       tele_thrs_tc_vec[MAX_PHYPORT_NUM + 1][TELE_DIR_ING_NUM_E];
    struct ku_trap_group_attr trap_group_attr[NUM_OF_TRAP_GROUPS];
    /* RP helper dbs */
    u8                     port_prio2tc[MAX_PHYPORT_NUM + 1][MAX_PRIO_NUM + 1];
    u8                     lag_prio2tc[MAX_LAG_NUM + 1][MAX_PRIO_NUM + 1];
    u8                     port_vtag_mode[MAX_PHYPORT_NUM + 1][SXD_MAX_VLAN_NUM];
    u8                     lag_vtag_mode[MAX_LAG_NUM + 1][SXD_MAX_VLAN_NUM];
    u8                     port_rp_rif_valid[MAX_PHYPORT_NUM + 1][SXD_MAX_VLAN_NUM];
    u8                     lag_rp_rif_valid[MAX_LAG_NUM][SXD_MAX_VLAN_NUM];
    u16                    port_rp_rif[MAX_PHYPORT_NUM + 1][SXD_MAX_VLAN_NUM];
    u16                    lag_rp_rif[MAX_LAG_NUM][SXD_MAX_VLAN_NUM];
    u8                     port_prio_tagging_mode[MAX_PHYPORT_NUM + 1];
    u8                     lag_prio_tagging_mode[MAX_LAG_NUM + 1];
    u16                    port_vid_to_fid[MAX_PHYPORT_NUM + 1][SXD_MAX_VLAN_NUM];
    u64                    port_mac_addr[MAX_PHYPORT_NUM + 1];
    struct dev_specific_cb dev_specific_cb;
    atomic_t               dev_specific_cb_refcnt;
    wait_queue_head_t      dev_specific_cb_not_in_use;
    /* ECMP redirect IP override */
    u32                   icmp_vlan2ip_db[SXD_MAX_VLAN_NUM];
    struct tasklet_struct intr_tasklet;
    struct {
        u32                      monitor_rdqs_arr[MAX_MONITOR_RDQ_NUM];
        u32                      monitor_rdqs_count;
        struct bpf_prog        * filter_ebpf_progs[NUMBER_OF_RDQS];
        struct bpf_prog        * agg_ebpf_progs[NUMBER_OF_RDQS][SX_AGG_EBPF_PROG_NUM_PER_RDQ];
        bool                     enable_monitor_rdq_trace_points;
        bool                     enable_drop_monitor_report;
        sx_net_dm_hw_report_func sx_net_dm_hw_report;
        struct listener_entry   *drop_monitor_listener;
        u16                      buffer_drop_trap_id;
    } wjh_info;
#ifdef SW_PUDE_EMULATION
    /* PUDE WA for NOS (PUDE events are handled by SDK). Needed for BU. */
    u8 admin_status_sysport_db[MAX_SYSPORT_NUM];
#endif /* SW_PUDE_EMULATION */
    void __iomem *cr_dump_base;
    struct {
        struct task_struct *polling_thread;
        struct list_head    tx_queue;
        spinlock_t          tx_queue_lock;
    } cr_mode;
    bool mmam_support;
    bool crsdump_in_process;

    /* Per-ASIC local parameters (default values are the global module parameters) */
    struct {
        int  tx_debug;
        int  rx_debug;
        int  chip_info_type;
        int  chip_info_revision;
        int  enable_keep_packet_crc;
        bool inited;
    } local_params;

    sx_mcam_reg_data_t mcam_reg_data;
};

struct sx_globals {
    struct rw_semaphore      pci_restart_lock;
    u8                       pci_drivers_in_use; /* should be used under pci_restart_lock */
    struct workqueue_struct *global_wq;
    struct sx_dev           *fake_dev;           /* running on OOB systems */
    struct sx_dev           *oob_backbone_dev;        /* SwitchX that interconnects all OOB devices */
    struct sx_dpt_s          sx_dpt;
    struct sx_i2c_ifc        sx_i2c;
    struct sx_mst_ifc        sx_mst;
    struct cdev              cdev;
};

extern struct sx_globals sx_glb;

struct isx_specific_data {
    u8  version;
    u8  ctl;
    u8  mc;
    u8  protocol;
    u8  ctclass;
    u16 cpu_signature;
    u16 signature;
    u8  emad;
    u8  type;
    u8  use_control_tclass;
    u8  rx_is_router;
    u8  fid_valid;
    u16 fid;
    u8  etclass;
    u8  rx_is_tunnel;
};

struct sx_timeval {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    struct timespec64 t;
#else
    struct timeval t;
#endif
    unsigned long j; /* jiffies */
};

struct sx_tm {
    struct tm tm;
    u32       msec;
};

/************************************************
 * Inline Functions
 ***********************************************/
static inline struct sx_priv * sx_priv(struct sx_dev *p_dev)
{
    return container_of(p_dev, struct sx_priv, dev);
}

static inline int sx_core_get_device_id_from_fd(struct file *file)
{
    return iminor(file->f_path.dentry->d_inode);
}

void sx_core_get_dev_and_id_from_fd(struct file *file, struct sx_dev **dev_pp, sxd_dev_id_t *dev_id_p);

void sx_core_file_add_ref(struct sx_rsc *file);
void sx_core_file_dec_ref(struct sx_rsc *file);

void inc_unconsumed_packets_global_counter(u16 hw_synd, enum sx_packet_type pkt_type);
void inc_filtered_lag_packets_global_counter(void);
void inc_filtered_port_packets_global_counter(void);
void inc_unconsumed_packets_counter(struct sx_priv *priv, u16 hw_synd, enum sx_packet_type pkt_type);
void sx_core_listeners_cleanup(struct sx_dev *dev);
void sx_core_mark_all_files_closing(struct sx_dev *dev, bool is_restart);
void sx_core_mark_all_files_reopened(struct sx_dev *dev);
void inc_filtered_lag_packets_counter(struct sx_priv *priv);
void inc_filtered_port_packets_counter(struct sx_priv *priv);
int get_system_status(struct sx_dev *dev, u16 *system_status);

int sx_reset(struct sx_dev *dev, u8 perform_chip_reset);
int sx_reset_with_mrsr_1(struct sx_dev *dev);
int sx_reset_by_capability(struct sx_dev *dev);
int sx_emergency_reset_with_cr_space(struct sx_dev *dev);
int sx_emergency_reset_by_capability(struct sx_dev *dev);
int sx_post_reset_mrsr_6(struct sx_dev *dev);
void sx_emergency_reset(struct sx_dev *dev, bool force);
bool sx_emergency_reset_done(struct sx_dev *dev);
int sx_emergency_reset_proc_handler(int argc, const char *argv[], struct sx_dev *dev, void *context);
const char * sx_reset_type_str(enum sx_reset_type type);

int sx_intf_dump(struct seq_file *m, void *v, void *context);
int sx_core_register_device(struct sx_dev *dev, struct sx_flow_info *fi);
void sx_core_unregister_device(struct sx_dev *dev, struct sx_flow_info *fi);
int sx_cmd_init(struct sx_dev *dev);
int sx_cmd_init_pci(struct sx_dev *dev);
bool sx_dbg_init(struct sx_dev *dev, void *context);
bool sx_dbg_deinit(struct sx_dev *dev, void *context);
int sx_cmd_pool_create(struct sx_dev *dev);
void sx_cmd_pool_destroy(struct sx_dev *dev);
void sx_cmd_unmap(struct sx_dev *dev);
int sx_core_cr_dump_init(struct sx_priv *priv);
void sx_core_cr_dump_deinit(struct sx_priv *priv);
void sx_core_bulk_counters_db_init(struct sx_priv *priv);
void sx_core_bulk_counters_db_deinit(struct sx_priv *priv);
int sx_enable_swid(struct sx_dev *dev, int sx_dev_id, u8 swid, int synd, u64 mac);
void sx_disable_swid(struct sx_dev *dev, u8 swid);
int sx_change_configuration(struct sx_dev *dev);
int sx_handle_set_profile(struct sx_dev *dev);
int sx_build_isx_header(struct isx_meta *meta, struct sk_buff *skb, u8 stclass);
int sx_build_isx_header_v0(struct isx_meta *meta, struct sk_buff *skb, u8 stclass,  u8 hw_etclass);
int sx_build_isx_header_v1(struct isx_meta *meta, struct sk_buff *skb, u8 stclass,  u8 hw_etclass);
int sx_get_sdq(struct isx_meta *meta, struct sx_dev *dev, enum ku_pkt_type type, u8 swid,
               u8 etclass, u8 lp, u8 *stclass, u8 *sdq, u8 *max_cpu_etclass_for_unlimited_mtu, u16 *cap_max_mtu);
int sx_core_get_rdq_param_max(struct sx_dev *dev, uint8_t *rdq_num_max_p, uint16_t *rdq_max_entry_size_p);
int sx_init_reset_capabilities(struct sx_dev *dev);
int sx_fd_dump_handler(struct seq_file* m, void *v, void* context);
void sx_ci_set_timestamp(const struct sx_cq *cq, const struct sx_rx_timestamp *linux_ts, struct completion_info *ci);

/**
 * The following function is a wrapper for chip-specific
 * callbacks, which are used for getting relevant SDQ numbers
 * (IDs) which we want to create/delete.
 */
int sx_get_sqd_num(struct sx_dev *dev, u8 swid, u8 etclass, u8 *sdq);

int sx_get_sdq_per_traffic_type(struct sx_dev *dev, enum ku_pkt_type type, u8 swid, u8 etclass,
                                u8 *stclass, u8 *sdq);
int sx_core_dispatch_event(struct sx_dev *dev, enum sx_dev_event type,
                           union sx_event_data *event_data);

/**
 * The following function is used as a callback for devices
 * where we are getting SDQ number for creation based on the
 * specified egress TC.
 * Please note that the further SDQ handling will be based on
 * traffic type and handled by 'sx_get_sdq_cb'
 */
int sx_get_sdq_num_per_etclasss(struct sx_dev *dev, u8 swid, u8 etclass, u8 *sdq);

int disable_monitor_rdq(struct sx_dev *dev, int dqn, u8 del_all_fd_listeners);
void unset_monitor_rdq(struct sx_dq *dq);
void sx_set_monitor_rdq_lock(void);
void sx_set_monitor_rdq_unlock(void);

/**
 * Used to send "SDK health" event (SXD_TRAP_ID_SDK_HEALTH_EVENT) to the registered users
 */
int sx_send_health_event(uint8_t dev_id, sxd_health_cause_t cause, sxd_health_severity_t severity,
                         uint8_t irisc_id, sxd_event_health_data_t *event_extra_data, struct listener_entry *listener);
void sx_health_handle_new_listener(struct sx_dev *dev, struct listener_entry *listener);

bool sdk_health_get(uint8_t dev_id);
void sdk_health_set(uint8_t dev_id, bool is_enabled);
bool sdk_health_test_and_disable(uint8_t dev_id);

void sx_dbg_get_time(struct sx_timeval *sxtv);
void sx_dbg_jiffies_to_time(const struct sx_timeval *now,
                            unsigned long            jiffies_ts,
                            struct sx_tm            *sxtm);
void sx_dbg_jiffies_to_time_str(const struct sx_timeval *now,
                                unsigned long            jiffies_ts,
                                char                    *buff,
                                uint32_t                 bufflen);

int sx_sysfs_asic_create(struct sx_dev *dev);

void sx_sysfs_asic_remove(struct sx_dev *dev);

/* Multi-ASIC sysfs helper functions */
struct kobject * sx_core_get_sysfs_parent(struct sx_dev *dev);

/**
 * Get the sx_dev device pointer associated with a given kobject.
 *
 * Traverses the kobject hierarchy to find an "asic$id" kobject under the specified
 * module folder (e.g., "sx_core" or "sx_netdev"). Extracts the ASIC ID from the
 * kobject name and retrieves the corresponding sx_dev pointer. The device ID is
 * calculated as (asic_id + 1). If no valid "asic<id>" is found, defaults to asic_id = 0.
 *
 * @param kobj_p           Pointer to the kobject to resolve.
 * @param dev_pp           Address of pointer to store the resulting sx_dev pointer.
 * @param module_folder_p  Name of the parent module folder ("sx_core" or "sx_netdev").
 *
 * @return 0 on success, negative error code on failure (-EINVAL for invalid parameters,
 *         -ENODEV if device not found).
 */
int sx_core_asic_get_dev_from_kobj(struct kobject *kobj_p, struct sx_dev **dev_pp, const char *module_folder_p);

/**
 * Retrieve the sx_dev pointer for a given global module index in a multi-ASIC system.
 *
 * For multi-ASIC systems, uses the global module index to determine the correct device.
 * For single-ASIC systems, returns the default device. Returns 0 on success or a negative
 * error code on failure.
 *
 * @param global_module  Global module index to resolve.
 * @param dev_pp         Address of pointer to store the resulting sx_dev pointer.
 *
 * @return 0 on success, negative error code on failure.
 */
int sx_core_asic_get_dev_global(uint8_t global_module, struct sx_dev **dev);

int sx_sysfs_independent_module_db_dump(struct seq_file *m, void *v, void *context);
int sx_core_send_sniffer_event(struct kobject   *kobj,
                               struct attribute *attr,
                               const char       *cb_name,
                               const char       *buf,
                               size_t            size);
int sx_sysfs_asic_init_tempeature(struct sx_dev *dev);
int sx_sysfs_asic_independent_init(struct sx_dev *dev);

int sx_sysfs_get_mcam_reg_data(struct sx_dev *dev);
void sx_sysfs_update_tec_support(struct sx_dev *dev);

ssize_t sx_asic_perf_counter_init(struct sx_dev *dev);
void sx_asic_perf_counter_deinit(struct sx_dev *dev);

bool sx_i2c_mode(void);
bool sx_mst_mode(void);
bool sx_cr_mode(void);
int sx_cr_polling_thread_init(struct sx_dev *dev);
void sx_cr_polling_thread_deinit(struct sx_dev *dev);
int sx_cr_send_packet(struct sx_dev *dev, struct sk_buff *skb, struct isx_meta *meta);

const char * sx_get_chip_type_str(sxd_chip_types_t chip_type);
const char * sx_get_ga_asic_str(enum asic_ga_index ga);

int sx_core_get_possible_local_module(uint8_t global_module, uint8_t *local_module);
int sx_multi_asic_module_sysfs_module_is_inited(uint8_t global_module, bool *inited);
void sx_multi_asic_module_sysfs_env_init(struct sx_dev *dev);
int sx_multi_asic_module_sysfs_map_ga_to_devid(u8 ga, sxd_dev_id_t dev_id);
void sx_multi_asic_module_unmap_ga_devid(u8 ga, sxd_dev_id_t dev_id);
int sx_multi_asic_module_sysfs_get_devid_via_ga(u8 ga, sxd_dev_id_t *dev_id);
int sx_multi_asic_module_sysfs_get_global_module(uint8_t ga, uint8_t local_module, uint8_t *global_module);
int sx_multi_asic_module_sysfs_dump(struct seq_file *m, void *v, void *context);
int sx_module_sysfs_dump(struct seq_file *m, void *v, void *context);
int sx_multi_asic_module_sysfs_need_skip(uint8_t module, bool *need_skip);
int sx_multi_asic_module_ind_type(uint8_t module, uint8_t *ind_type);
int sx_module_get_local_port_for_els(uint8_t module, uint8_t *local_port);
int sx_module_get_module_type(uint8_t module_id, uint8_t *module_type);
uint8_t sx_module_get_max_submodule_num(void);

/* enable/disable access to the device from external tools (such as ethtool) or sysfs */
void sx_core_sysfs_dev_enable(struct sx_dev *dev);
void sx_core_sysfs_dev_disable(struct sx_dev *dev, const char *locker_name);
bool sx_core_sysfs_dev_is_enabled(struct sx_dev *dev);

int sx_core_secured_cr_init(struct sx_dev *dev);
int sx_core_secured_cr_read(struct sx_dev *dev, u32 offset, u32 *data);
int sx_bulk_cntr_streaming_buffer_id_alloc_and_fill_buff(struct sx_dev *dev,
                                                         unsigned long  buffer_id,
                                                         bool           alloc_can_sleep);
int sx_bulk_cntr_streaming_buffer_id_free_buff(struct sx_dev *dev, unsigned long buffer_id);
void sx_bulk_cntr_streaming_profile_unregister(struct sx_dev* dev);

void sx_core_set_cached_module_type(struct sx_dev *dev, uint8_t module, uint8_t type);
void sx_handle_mcia_event(struct completion_info *ci);
int sx_module_parallel_mcia_access_db_statistics_clear(int argc, const char *argv[], struct sx_dev *dev,
                                                       void *context);
int sx_module_parallel_mcia_access_i2c_statistics_read(struct sx_dev *dev, char *buf);
int sx_module_parallel_mcia_access_i2c_statistics_clear(struct sx_dev *dev);

int sx_core_parallel_mod_access_wq_init(struct sx_dev *dev);
void sx_core_parallel_mod_access_wq_deinit(struct sx_dev *dev);

#endif  /* SX_H */

/************************************************
 *                  EOF                         *
 ***********************************************/
