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

#include <linux/workqueue.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/ethtool.h>
#include <linux/etherdevice.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/netdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/tcp.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <net/ip.h>
#include <net/addrconf.h>
#include <linux/vmalloc.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/rtnetlink.h>
#include <linux/version.h>
#include <net/drop_monitor.h>
#include <net/flow_offload.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) || defined(CONFIG_SX_RHEL_8_6)
#include <net/devlink.h>
#include <trace/events/devlink.h>
#endif

#include <linux/mlx_sx/cmd.h>
#include <linux/mlx_sx/device.h>
#include <linux/mlx_sx/driver.h>
#include <linux/mlx_sx/cmd.h>
#include <linux/mlx_sx/kernel_user.h>
#include <linux/mlx_sx/auto_registers/reg.h>
#include <linux/mlx_sx/auto_registers/cmd_auto.h>
#include <linux/mlx_sx/auto_registers/sx_core_reg_buffer_print_auto.h>
#if defined(CONFIG_NET_SWITCHDEV)
#include <net/switchdev.h>
#endif
#include "sx_netdev.h"
#include "sx_psample.h"
#include "sx_netdev_module_sysfs_eeprom.h"
#include "synce.h"
#include "sx_netdev_module_sysfs_s3ip.h"

#define CREATE_TRACE_POINTS
#include "sx_netdev_trace.h"

/*
 * CONFIG_NET_PORT is private kernel define for a generic netlink channel
 * between the kernel and userspace switch drivers.
 * It is not defined in upstream kernel.
 */
#if defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
#include <net/fib_notifier.h>
#include <net/ip_fib.h>
#include <net/ip6_fib.h>
#include <net/nexthop.h>
#include "sx_netdev_addons.h"
#endif

/*
 * Local define in order to allocate skb IP aligned.
 * In header asm/processor.h NET_IP_ALIGN is defined as 0.
 */
#ifdef NET_IP_ALIGN
#undef NET_IP_ALIGN
#endif
#define NET_IP_ALIGN (2)

#define INVALID_LINUX_CHANNEL_MIRROR_LATENCY 0xFFFFFF
#define INVALID_LINUX_CHANNEL_MIRROR_TCLASS  0x1F
#define INVALID_LINUX_CHANNEL_MIRROR_CONG    0xFFFF
#define MULTI_PORT_TYPE_OF_LOCAL_PORT        0xFFFE
#define GET_BIT_IN_MASK(mask, offset) (((mask) & (1 << (offset))) >> (offset))

/*============================================================================
 *                       NV Records - Internal Definitions
 *============================================================================
 */

/* NV Records - Metadata in SKB headroom for upper layer access */

enum sx_nv_record_type_e {
    SX_NV_RECORD_TYPE_INVALID_E = 0x0,  /* Invalid/unused */
    SX_NV_RECORD_TYPE_TRAP_ID_E = 0x1,  /* Trap ID from CQE */
};

#define SX_NV_RECORD_IS_TYPE_ENABLED(bitmap, type) ((bitmap) & (1 << (type)))

#define SX_NV_RECORDS_ALIGNMENT 4      /* 4-byte alignment */
#define SX_NV_RECORDS_FLAG      0x07E9 /* Magic identifier (decimal 2025) */

/**
 * struct sx_nv_record_header_s - Header for each NV record
 * @type: Type of record (4 bits) - identifies the record type
 * @reserved: Reserved bits (4 bits) - must be zero
 * @length: Length (in bytes) of the record value only (not including this header)
 *
 * This header precedes the actual record value data.
 * Each record type has its own value structure that follows this header.
 */
typedef struct sx_nv_record_header_s {
    u8 type : 4;
    u8 reserved : 4;
    u8 length;
} __packed sx_nv_record_header_t;

/**
 * struct sx_nv_record_trap_id_value_s - Value structure for trap_id record
 * @trap_id: The trap ID value from CQE
 *
 * This structure represents only the value portion of a trap_id record.
 */
typedef struct sx_nv_record_trap_id_value_s {
    u16 trap_id;
} __packed sx_nv_record_trap_id_value_t;

/**
 * struct sx_nv_record_trap_id_s - Complete trap_id record
 * @header: Record header with type and length
 * @value: Trap ID value
 *
 * This structure represents a complete trap_id NV record by composing
 * the header and value structures. It's the complete record type.
 */
typedef struct sx_nv_record_trap_id_s {
    sx_nv_record_header_t        header;
    sx_nv_record_trap_id_value_t value;
} __packed sx_nv_record_trap_id_t;

/**
 * struct sx_nv_records_terminator_s - NV records block terminator
 * @nv_flag: Magic identifier (SX_NV_RECORDS_FLAG) to identify NV records block
 * @total_length: Total size of all records + terminator (unaligned, no padding, in bytes)
 * @reserved: Reserved byte for alignment (must be zero)
 *
 * This structure is placed at the end of the NV records block in headroom.
 */
typedef struct sx_nv_records_terminator_s {
    u16 nv_flag;
    u8  total_length;
    u8  reserved;
} __packed sx_nv_records_terminator_t;

MODULE_AUTHOR("Amos Hersch");
MODULE_DESCRIPTION("NVIDIA Switch Network Device Driver");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION(DRV_VERSION);

/*============================================================================
 *                             module parameters
 *============================================================================
 */
int sx_netdev_rx_debug = 0;
module_param_named(sx_netdev_rx_debug, sx_netdev_rx_debug, int, 0644);
MODULE_PARM_DESC(sx_netdev_rx_debug, "en/dis rx debug for sx_netdev module");

int sx_netdev_tx_debug = 0;
module_param_named(sx_netdev_tx_debug, sx_netdev_tx_debug, int, 0644);
MODULE_PARM_DESC(sx_netdev_tx_debug, "en/dis tx debug for sx_netdev module");

/* NV Records feature bitmap - each bit corresponds to record type enum value
 * Bit position matches sx_nv_record_type_e values:
 *   Bit 0: unused (corresponds to INVALID type)
 *   Bit 1: trap_id (corresponds to type value 1)
 *   Bits 2-31: reserved for future record types
 */
unsigned int nv_records_enable = 0;
module_param_named(nv_records_enable, nv_records_enable, uint, 0644);
MODULE_PARM_DESC(nv_records_enable, "Bitmap to enable NV record types (bit 1=trap_id, bits 2-31 reserved)");

int carrier_set_on_pude_disable = CARRIER_SET_ON_PUDE_ADMIN_UP_DOWN;
static int set_carrier_set_on_pude_disable(const char *val, const struct kernel_param *kp);
static const struct kernel_param_ops carrier_set_on_pude_disable_ops = {
    .set = set_carrier_set_on_pude_disable,
    .get = param_get_int,
};

module_param_cb(carrier_set_on_pude_disable, &carrier_set_on_pude_disable_ops, &carrier_set_on_pude_disable, 0644);
__MODULE_PARM_TYPE(carrier_set_on_pude_disable, "int");
MODULE_PARM_DESC(carrier_set_on_pude_disable, "0 - carrier is set by sx_netdev on PUDE event/admin up/admin down, "
                 "1 - carrier is set by sx_netdev on admin down, "
                 "2 - carrier is fully controlled by user");

/* per-ASIC local parameters for sx_netdev */
static ssize_t __show_netdev_local_param_int(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t __store_netdev_local_param_int(struct kobject        *kobj,
                                              struct kobj_attribute *attr,
                                              const char            *buf,
                                              size_t                 len);

static struct kobj_attribute lp_sx_netdev_tx_debug_attr = __ATTR(sx_netdev_tx_debug,
                                                                 (S_IRUGO | S_IWUSR),
                                                                 __show_netdev_local_param_int,
                                                                 __store_netdev_local_param_int);
static struct kobj_attribute lp_sx_netdev_rx_debug_attr = __ATTR(sx_netdev_rx_debug,
                                                                 (S_IRUGO | S_IWUSR),
                                                                 __show_netdev_local_param_int,
                                                                 __store_netdev_local_param_int);

static struct attribute *local_params_attrs[] = {
    &lp_sx_netdev_tx_debug_attr.attr,
    &lp_sx_netdev_rx_debug_attr.attr,
    NULL,
};

static struct attribute_group local_params_group = {
    .name = "local_parameters",
    .attrs = local_params_attrs,
};

int sx_netdev_tc_psample_enable = 0;
#if defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
module_param_named(sx_netdev_tc_psample_enable, sx_netdev_tc_psample_enable, int, 0644);
MODULE_PARM_DESC(sx_netdev_tc_psample_enable, "en/dis tc psample support");
#endif /* if defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) */

int arp_nd_mcast_sw_forwarding_enable = 0;
module_param_named(arp_nd_mcast_sw_forwarding_enable, arp_nd_mcast_sw_forwarding_enable, int, 0644);
MODULE_PARM_DESC(arp_nd_mcast_sw_forwarding_enable, "en/dis sw forwarding of mcast traffic");

int untagged_pkt_pvid_tag_disable = 0;
module_param_named(untagged_pkt_pvid_tag_disable, untagged_pkt_pvid_tag_disable, int, 0644);
MODULE_PARM_DESC(untagged_pkt_pvid_tag_disable, "en/dis PVID VLAN tag on untagged packet");

int single_netdev_per_port_enable = 0;
module_param_named(single_netdev_per_port_enable, single_netdev_per_port_enable, int, 0644);
MODULE_PARM_DESC(single_netdev_per_port_enable, "en/dis single netdevice per port");

int mirror_device_en = 0;
int change_proto_down_enable = 0;
int route_offload_flags = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) || defined(CONFIG_SX_RHEL_8_6)
#if defined(CONFIG_NET_PORT)
module_param_named(mirror_device_en, mirror_device_en, int, 0644);
MODULE_PARM_DESC(mirror_device_en, "en/dis mirror device for span to cpu");
#endif

module_param_named(change_proto_down_enable, change_proto_down_enable, int, 0644);
MODULE_PARM_DESC(change_proto_down_enable, "en/dis proto-down netdev flag");

module_param_named(route_offload_flags, route_offload_flags, int, 0644);
MODULE_PARM_DESC(route_offload_flags, "set route offload flags: 0 - off, 1 - set OFFLOAD and TRAP");
#endif

int offload_fwd_mark_en = 1;
#if defined(CONFIG_NET_SWITCHDEV)
module_param_named(offload_fwd_mark_en, offload_fwd_mark_en, int, 0644);
MODULE_PARM_DESC(offload_fwd_mark_en, "1 - enable offload of packets fwd in HW, 0 - disable offload");
#endif

int devlink_port_en = 0;
#if !defined(SX_DROP_MONITOR_DISABLED) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, \
                                                                                 0)) || defined(CONFIG_SX_RHEL_8_6))
module_param_named(devlink_port_en, devlink_port_en, int, 0644);
MODULE_PARM_DESC(devlink_port_en, "1 - enable devlink port support for sx_netdev interfaces, 0 - disable devlink");
#endif


/*============================================================================
 *                             defines
 *============================================================================
 */
#define BUILD_64BIT_COUNTER(high, low) ((u64)(high) << 32 | (low))

#define SX_LOCAL_PORT_TO_LOG_PORT(local_port) (0x10000 + ((local_port) << SX_PORT_PHY_ID_OFFS))
#define SX_MAX_PORT_TOGGLE_PER_SEC (10)

struct sx_ethtool_link_ext_state_opcode_mapping {
    u32                    status_opcode;
    enum sx_link_ext_state link_ext_state;
    u8                     link_ext_substate;
};

static const struct sx_ethtool_link_ext_state_opcode_mapping
    sx_link_ext_state_opcode_map[] = {
    {2, SX_LINK_EXT_STATE_AUTONEG,
     SX_LINK_EXT_SUBSTATE_AN_NO_PARTNER_DETECTED},
    {3, SX_LINK_EXT_STATE_AUTONEG,
     SX_LINK_EXT_SUBSTATE_AN_ACK_NOT_RECEIVED},
    {4, SX_LINK_EXT_STATE_AUTONEG,
     SX_LINK_EXT_SUBSTATE_AN_NEXT_PAGE_EXCHANGE_FAILED},
    {36, SX_LINK_EXT_STATE_AUTONEG,
     SX_LINK_EXT_SUBSTATE_AN_NO_PARTNER_DETECTED_FORCE_MODE},
    {38, SX_LINK_EXT_STATE_AUTONEG,
     SX_LINK_EXT_SUBSTATE_AN_FEC_MISMATCH_DURING_OVERRIDE},
    {39, SX_LINK_EXT_STATE_AUTONEG,
     SX_LINK_EXT_SUBSTATE_AN_NO_HCD},

    {5, SX_LINK_EXT_STATE_LINK_TRAINING_FAILURE,
     SX_LINK_EXT_SUBSTATE_LT_KR_FRAME_LOCK_NOT_ACQUIRED},
    {6, SX_LINK_EXT_STATE_LINK_TRAINING_FAILURE,
     SX_LINK_EXT_SUBSTATE_LT_KR_LINK_INHIBIT_TIMEOUT},
    {7, SX_LINK_EXT_STATE_LINK_TRAINING_FAILURE,
     SX_LINK_EXT_SUBSTATE_LT_KR_LINK_PARTNER_DID_NOT_SET_RECEIVER_READY},
    {8, SX_LINK_EXT_STATE_LINK_TRAINING_FAILURE, 0},
    {14, SX_LINK_EXT_STATE_LINK_TRAINING_FAILURE,
     SX_LINK_EXT_SUBSTATE_LT_REMOTE_FAULT},

    {9, SX_LINK_EXT_STATE_LINK_LOGICAL_MISMATCH,
     SX_LINK_EXT_SUBSTATE_LLM_PCS_DID_NOT_ACQUIRE_BLOCK_LOCK},
    {10, SX_LINK_EXT_STATE_LINK_LOGICAL_MISMATCH,
     SX_LINK_EXT_SUBSTATE_LLM_PCS_DID_NOT_ACQUIRE_AM_LOCK},
    {11, SX_LINK_EXT_STATE_LINK_LOGICAL_MISMATCH,
     SX_LINK_EXT_SUBSTATE_LLM_PCS_DID_NOT_GET_ALIGN_STATUS},
    {12, SX_LINK_EXT_STATE_LINK_LOGICAL_MISMATCH,
     SX_LINK_EXT_SUBSTATE_LLM_FC_FEC_IS_NOT_LOCKED},
    {13, SX_LINK_EXT_STATE_LINK_LOGICAL_MISMATCH,
     SX_LINK_EXT_SUBSTATE_LLM_RS_FEC_IS_NOT_LOCKED},

    {15, SX_LINK_EXT_STATE_BAD_SIGNAL_INTEGRITY, 0},
    {17, SX_LINK_EXT_STATE_BAD_SIGNAL_INTEGRITY,
     SX_LINK_EXT_SUBSTATE_BSI_LARGE_NUMBER_OF_PHYSICAL_ERRORS},
    {42, SX_LINK_EXT_STATE_BAD_SIGNAL_INTEGRITY,
     SX_LINK_EXT_SUBSTATE_BSI_UNSUPPORTED_RATE},

    {1024, SX_LINK_EXT_STATE_NO_CABLE, 0},

    {16, SX_LINK_EXT_STATE_CABLE_ISSUE,
     SX_LINK_EXT_SUBSTATE_CI_UNSUPPORTED_CABLE},
    {20, SX_LINK_EXT_STATE_CABLE_ISSUE,
     SX_LINK_EXT_SUBSTATE_CI_UNSUPPORTED_CABLE},
    {29, SX_LINK_EXT_STATE_CABLE_ISSUE,
     SX_LINK_EXT_SUBSTATE_CI_UNSUPPORTED_CABLE},
    {1025, SX_LINK_EXT_STATE_CABLE_ISSUE,
     SX_LINK_EXT_SUBSTATE_CI_UNSUPPORTED_CABLE},
    {1029, SX_LINK_EXT_STATE_CABLE_ISSUE,
     SX_LINK_EXT_SUBSTATE_CI_UNSUPPORTED_CABLE},
    {1031, SX_LINK_EXT_STATE_CABLE_ISSUE, 0},

    {1027, SX_LINK_EXT_STATE_EEPROM_ISSUE, 0},

    {23, SX_LINK_EXT_STATE_CALIBRATION_FAILURE, 0},

    {1032, SX_LINK_EXT_STATE_POWER_BUDGET_EXCEEDED, 0},

    {1030, SX_LINK_EXT_STATE_OVERHEAT, 0},

    {1042, SX_LINK_EXT_STATE_MODULE,
     SX_LINK_EXT_SUBSTATE_MODULE_CMIS_NOT_READY},
};

static char sx_netdev_version[] =
    "NVIDIA Switch Network Device Driver "
    DRV_VERSION " (" DRV_RELDATE ")\n";

#define VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size)            \
    do {                                                                  \
        if ((snp_res == 0) || (snp_res >= buffer_length - buffer_size)) { \
            return buffer_size;                                           \
        }                                                                 \
    } while (0)

static LIST_HEAD(__priv_db);
static DEFINE_RWLOCK(__priv_db_lock);

static int __sx_net_dm_hw_report(struct completion_info *comp_info,
                                 const char             *trap_name,
                                 const char             *trap_grp_name,
                                 const void             *fa_cookie,
                                 u8                      copy_skb);

/*============================================================================
 *                            sysfs infra
 *============================================================================
 */

static ssize_t skip_tunnel_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t skip_tunnel_cb(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len);
static void __bind_unbind_sx_core(bool is_bind, struct sx_dev_netdev_priv *priv, bool in_pci_restart);
static struct kobj_attribute skip_netdev_attr =
    __ATTR(skip_tunnel, S_IWUSR | S_IRUGO, skip_tunnel_show, skip_tunnel_cb);
static ssize_t store_bind_sx_core(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len);
static struct kobj_attribute bind_sx_core_attr = __ATTR(bind_sx_core, S_IWUSR, NULL, store_bind_sx_core);

static ssize_t show_module_id(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t reset_module(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len);
static ssize_t store_module_power_mode_policy(struct kobject        *kobj,
                                              struct kobj_attribute *attr,
                                              const char            *buf,
                                              size_t                 len);
static ssize_t show_module_power_mode_policy(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_power_mode(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_link_ext_state(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_temp_input(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_temp_label(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_temp_threshold_hi(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_temp_threshold_lo(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_temp_threshold_critical_hi(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_temp_threshold_critical_lo(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_id_attr = __ATTR(id, S_IRUGO, show_module_id, NULL);
static struct kobj_attribute module_reset_attr = __ATTR(reset, S_IWUSR, NULL, reset_module);
static struct kobj_attribute module_power_mode_policy_attr = __ATTR(power_mode_policy,
                                                                    (S_IRUGO | S_IWUSR),
                                                                    show_module_power_mode_policy,
                                                                    store_module_power_mode_policy);
static struct kobj_attribute module_power_mode_attr = __ATTR(power_mode, S_IRUGO, show_module_power_mode, NULL);
static struct kobj_attribute link_ext_state_attr = __ATTR(link_ext_state, S_IRUGO, show_link_ext_state, NULL);
static struct kobj_attribute module_temp_input_attr = __ATTR(input, S_IRUGO, show_module_temp_input, NULL);
static struct kobj_attribute module_temp_label_attr = __ATTR(label, S_IRUGO, show_module_temp_label, NULL);
static struct kobj_attribute module_temp_threshold_hi_attr =
    __ATTR(threshold_hi, S_IRUGO, show_module_temp_threshold_hi, NULL);
static struct kobj_attribute module_temp_threshold_lo_attr =
    __ATTR(threshold_lo, S_IRUGO, show_module_temp_threshold_lo, NULL);
static struct kobj_attribute module_temp_threshold_critical_hi_attr =
    __ATTR(threshold_critical_hi, S_IRUGO, show_module_temp_threshold_critical_hi, NULL);
static struct kobj_attribute module_temp_threshold_critical_lo_attr =
    __ATTR(threshold_critical_lo, S_IRUGO, show_module_temp_threshold_critical_lo, NULL);
/* TEC Module*/
static ssize_t show_module_tec_cooling_level(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_cooling_level_attr = __ATTR(cooling_level,
                                                                    S_IRUGO,
                                                                    show_module_tec_cooling_level,
                                                                    NULL);

static ssize_t show_module_tec_max_power(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_max_power_attr = __ATTR(max_power,
                                                                S_IRUGO,
                                                                show_module_tec_max_power,
                                                                NULL);

static ssize_t show_module_tec_power(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_power_attr = __ATTR(power, S_IRUGO, show_module_tec_power, NULL);

static ssize_t show_module_tec_power_warning_high(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_power_warning_high_attr = __ATTR(power_threshold_warning_hi,
                                                                         S_IRUGO,
                                                                         show_module_tec_power_warning_high,
                                                                         NULL);

static ssize_t show_module_tec_power_warning_low(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_power_warning_low_attr = __ATTR(power_threshold_warning_lo,
                                                                        S_IRUGO,
                                                                        show_module_tec_power_warning_low,
                                                                        NULL);

static ssize_t show_module_tec_power_alarm_high(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_power_alarm_high_attr = __ATTR(power_threshold_alarm_hi,
                                                                       S_IRUGO,
                                                                       show_module_tec_power_alarm_high,
                                                                       NULL);

static ssize_t show_module_tec_power_alarm_low(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_power_alarm_low_attr = __ATTR(power_threshold_alarm_lo,
                                                                      S_IRUGO,
                                                                      show_module_tec_power_alarm_low,
                                                                      NULL);

static ssize_t show_module_tec_max_set_point_temperature(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_max_set_point_temperature_attr = __ATTR(max_laser_temperature,
                                                                                S_IRUGO,
                                                                                show_module_tec_max_set_point_temperature,
                                                                                NULL);

static ssize_t show_module_tec_set_point_temperature(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_set_point_temperature_attr = __ATTR(laser_temperature,
                                                                            S_IRUGO,
                                                                            show_module_tec_set_point_temperature,
                                                                            NULL);

static ssize_t show_module_tec_set_point_temperature_warning_high(struct kobject        *kobj,
                                                                  struct kobj_attribute *attr,
                                                                  char                  *buf);
static struct kobj_attribute module_tec_set_point_temperature_warning_high_attr = __ATTR(
    laser_temperature_threshold_warning_hi,
    S_IRUGO,
    show_module_tec_set_point_temperature_warning_high,
    NULL);

static ssize_t show_module_tec_set_point_temperature_warning_low(struct kobject        *kobj,
                                                                 struct kobj_attribute *attr,
                                                                 char                  *buf);
static struct kobj_attribute module_tec_set_point_temperature_warning_low_attr = __ATTR(
    laser_temperature_threshold_warning_lo,
    S_IRUGO,
    show_module_tec_set_point_temperature_warning_low,
    NULL);

static ssize_t show_module_tec_set_point_temperature_alarm_high(struct kobject        *kobj,
                                                                struct kobj_attribute *attr,
                                                                char                  *buf);
static struct kobj_attribute module_tec_set_point_temperature_alarm_high_attr = __ATTR(
    laser_temperature_threshold_alarm_hi,
    S_IRUGO,
    show_module_tec_set_point_temperature_alarm_high,
    NULL);

static ssize_t show_module_tec_set_point_temperature_alarm_low(struct kobject        *kobj,
                                                               struct kobj_attribute *attr,
                                                               char                  *buf);
static struct kobj_attribute module_tec_set_point_temperature_alarm_low_attr = __ATTR(
    laser_temperature_threshold_alarm_lo,
    S_IRUGO,
    show_module_tec_set_point_temperature_alarm_low,
    NULL);

static ssize_t show_module_tec_max_cooling_level(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_max_cooling_level_attr = __ATTR(max_cooling_level,
                                                                        S_IRUGO,
                                                                        show_module_tec_max_cooling_level,
                                                                        NULL);

static ssize_t show_module_tec_min_cooling_level(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_min_cooling_level_attr = __ATTR(min_cooling_level,
                                                                        S_IRUGO,
                                                                        show_module_tec_min_cooling_level,
                                                                        NULL);


static ssize_t show_pddr_operation_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_pddr_troubleshooting_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_pddr_module_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_pddr_link_down_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_pddr_latched_flag_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_netdev_log_port_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_netdev_pipeline_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf);


static struct kobj_attribute pddr_operation_attr = __ATTR(port_oper_info, S_IRUGO, show_pddr_operation_info, NULL);
static struct kobj_attribute pddr_troubleshooting_attr = __ATTR(port_troubleshooting_info,
                                                                S_IRUGO,
                                                                show_pddr_troubleshooting_info,
                                                                NULL);
static struct kobj_attribute pddr_module_attr = __ATTR(port_module_info, S_IRUGO, show_pddr_module_info, NULL);
static struct kobj_attribute pddr_link_down_attr =
    __ATTR(port_link_down_info, S_IRUGO, show_pddr_link_down_info, NULL);
static struct kobj_attribute pddr_latched_flag_attr = __ATTR(port_latched_flag_info,
                                                             S_IRUGO,
                                                             show_pddr_latched_flag_info,
                                                             NULL);

static struct kobj_attribute netdev_log_port_attr = __ATTR(log_port, S_IRUGO, show_netdev_log_port_info, NULL);
static struct kobj_attribute netdev_pipeline_attr = __ATTR(pipeline, S_IRUGO, show_netdev_pipeline_info, NULL);

static int __netdev_rx_debug(struct sx_dev *dev)
{
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);
    int                        ret = 0;

    if (priv) {
        ret = MODULE_PARAM_INSTANCE_GET(priv, sx_netdev_rx_debug);
        dev_netdev_priv_put(priv);
    }

    return ret;
}

static int __netdev_tx_debug(struct sx_dev *dev)
{
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);
    int                        ret = 0;

    if (priv) {
        ret = MODULE_PARAM_INSTANCE_GET(priv, sx_netdev_tx_debug);
        dev_netdev_priv_put(priv);
    }

    return ret;
}

/*============================================================================
 *                          sysfs function implementation
 *============================================================================
 */
#define SX_NETDEV_PREFIX "/sys/module/sx_netdev/"

static int __sx_netdev_sysfs_asic_get_dev(struct kobject *kobj, sxd_dev_id_t *asic_id_p)
{
    struct kobject *kobj_module = kobj;
    unsigned long   tmp = 0;
    int             ret = 0;

    while (strstr(kobject_name(kobj_module), "asic") == NULL) {
        if (kobj_module->parent == NULL) {
            ret = -EINVAL;
            goto out;
        }
        kobj_module = kobj_module->parent;
    }

    ret = kstrtol(kobj_module->name + strlen(ASIC_SYSFS_PREFIX), 0, &tmp);

    *asic_id_p = (sxd_dev_id_t)tmp;

out:
    return ret;
}

int sx_netdev_sysfs_handler_start(struct kobject *kobj, enum sx_net_kobject_type kobj_type, struct net_device **netdev)
{
    int                        ret = 0;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_curr_net_namespace();
    sxd_dev_id_t               asic_id = 0;

    if (!priv) {
        return -ENODEV;
    }

    /* In Multi ASIC we should validate tht the user access the sysfs file from the right container and not from the host or another container*/
    if (sx_core_has_pci_multi_dev()) {
        ret = __sx_netdev_sysfs_asic_get_dev(kobj, &asic_id);
        if (ret) {
            sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(priv), "sysfs-handler-start: failed to get asic_id (ret=%d)\n",
                        ret);
            goto out;
        }

        /* if priv is NULL, we are NOT inside the container */
        if (!priv) {
            sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(priv),
                        "sysfs-handler-start: access sysfs file from outside the container is not allowed\n");
            ret = -EPERM;
            goto out;
        }

        /* Check that the user access the sysfs file from the right container it belongs to. */
        if (priv->dev->device_id != (asic_id + 1)) {
            sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(priv),
                        "sysfs-handler-start: trying to access files in asic[%u], from asic [%u]\n",
                        (asic_id + 1),
                        priv->dev->device_id);
            ret = -EPERM;
            goto out;
        }
    }

    ret = sx_core_sysfs_dev_hold(priv->dev);
    if (ret) {
        sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(priv),
                      "sysfs-handler-start: failed to get reference on device (ret=%d)\n",
                      ret);
        goto out;
    }

    ret = sx_netdev_get_net_device_by_kobj(kobj, netdev, kobj_type);
    if (ret) {
        sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(priv),
                      "sysfs-handler-start: failed to get net-device from kobj (ret=%d)\n",
                      ret);
        sx_core_sysfs_dev_release(priv->dev);
        goto out;
    }

out:
    dev_netdev_priv_put(priv);
    return ret;
}

void sx_netdev_sysfs_handler_end(struct net_device *netdev)
{
    struct sx_net_priv        *net_priv = (struct sx_net_priv*)netdev_priv(netdev);
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return;
    }

    sx_core_sysfs_dev_release(net_priv->dev);
    dev_netdev_priv_put(priv);
}

int send_netdev_sniffer_event(struct net_device *netdev,
                              struct kobject    *kobj,
                              struct attribute  *attr,
                              const char        *cb_name,
                              const char        *buf,
                              size_t             size)
{
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    int                        ret = 0;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return -ENODEV;
    }

    ret = send_sysfs_common_sniffer_event(kobj,
                                          attr,
                                          net_priv->dev,
                                          cb_name,
                                          buf,
                                          size,
                                          SYSFS_NETDEV);

    dev_netdev_priv_put(priv);
    return ret;
}

int sx_netdev_carrier_set_on_pude_disable_get(void)
{
    return carrier_set_on_pude_disable;
}

static ssize_t _get_pddr(struct net_device *netdev, char *buf, sxd_pddr_page_select_t page, ssize_t consumed_buf_space)
{
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    struct ku_access_pddr_reg  reg_data;
    int                        err = 0;
    bool                       is_independent = false;
    struct sx_dev_netdev_priv *priv;

    if (!net_priv->is_port_netdev) {
        return -ENODATA;
    }

    if (SX_PORT_TYPE_ID_GET(net_priv->log_port) != SX_PORT_TYPE_NETWORK) {
        return -ENODATA;
    }

    priv = dev_netdev_priv_from_netdev(netdev);
    if (!priv) {
        return -ENODEV;
    }

    err = sx_core_get_port_module_independent(net_priv->dev, net_priv->port, &is_independent);
    if (err) {
        goto out;
    }

    if (is_independent) {
        sxd_log_err(net_priv->dev,
                    "%s: Failed to read PDDR: port module is independent (SW control).\n",
                    __func__);
        err = -EFAULT;
        goto out;
    }

    memset(&reg_data, 0, sizeof(reg_data));

    reg_data.dev_id = net_priv->dev->device_id;
    sx_cmd_set_op_tlv(&reg_data.op_tlv, PDDR_REG_ID, EMAD_METHOD_QUERY);

    reg_data.pddr_reg.page_select = page;
    if (page == SXD_PDDR_PAGE_SELECT_TROUBLESHOOTING_INFO_PAGE_E) {
        reg_data.pddr_reg.page_data.pddr_troubleshooting_page.group_opcode =
            SXD_PDDR_GROUP_OPCODE_MONITOR_OPCODES_E;
    }
    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data.pddr_reg.local_port,
                                        reg_data.pddr_reg.lp_msb,
                                        net_priv->port);
    reg_data.pddr_reg.pnat = SXD_PDDR_PNAT_LOCAL_PORT_NUMBER_E;

    err = sx_ACCESS_REG_PDDR(net_priv->dev, &reg_data);
    if (err) {
        goto out;
    }

    err = sx_kernel_access_reg_pddr_data_to_buffer_print(buf + consumed_buf_space,
                                                         PAGE_SIZE - consumed_buf_space,
                                                         &reg_data.pddr_reg);
    if (err) {
        goto out;
    }

out:
    dev_netdev_priv_put(priv);
    return err;
}

static void __get_tec_support(struct net_device *netdev, bool *tec_support)
{
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    struct sx_dev             *dev = net_priv->dev;
    int                        err = 0;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    *tec_support = false; /* Default value */
    if (!priv) {
        return;
    }

    err = sx_core_get_tec_support(dev, tec_support);
    if (err) {
        sxd_log_notice(dev, "Failed to get TEC support, err=%d\n", err);
        *tec_support = false;
    }

    dev_netdev_priv_put(priv);
}

static ssize_t show_netdev_log_port_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                 ret = 0, len = 0;
    struct net_device  *netdev = NULL;
    struct sx_net_priv *net_priv = NULL;

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_PORT, &netdev);
    if (ret) {
        return ret;
    }

    net_priv = netdev_priv(netdev);
    if (!net_priv->is_port_netdev) {
        ret = -ENODATA;
        goto out;
    }

    len = snprintf(buf, PAGE_SIZE, "0x%x\n", net_priv->log_port);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_netdev_pipeline_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                 ret = 0, len = 0;
    struct net_device  *netdev = NULL;
    struct sx_net_priv *net_priv = NULL;

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_PORT, &netdev);
    if (ret) {
        return ret;
    }

    net_priv = netdev_priv(netdev);
    if (!net_priv->is_port_netdev) {
        ret = -ENODATA;
        goto out;
    }

    if (net_priv->pipeline == SX_NETDEV_PIPELINE_ADAPTIVE) {
        len = snprintf(buf, PAGE_SIZE, "adaptive\n");
    } else if (net_priv->pipeline == SX_NETDEV_PIPELINE_BYPASS) {
        len = snprintf(buf, PAGE_SIZE, "bypass\n");
    } else {
        sxd_log_err(net_priv->dev, "net_priv->pipeline is invalid %d\n", net_priv->pipeline);
        len = snprintf(buf, PAGE_SIZE, "invalid\n");
    }

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}


static ssize_t show_pddr_operation_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device *netdev = NULL;
    int                ret = 0;

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_PORT, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = _get_pddr(netdev, buf, SXD_PDDR_PAGE_SELECT_OPERATIONAL_INFO_PAGE_E, 0);
    sx_netdev_sysfs_handler_end(netdev);

    return ret;
}

static ssize_t show_pddr_troubleshooting_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device *netdev = NULL;
    int                ret = 0;

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_PORT, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = _get_pddr(netdev, buf, SXD_PDDR_PAGE_SELECT_TROUBLESHOOTING_INFO_PAGE_E, 0);
    sx_netdev_sysfs_handler_end(netdev);

    return ret;
}
static ssize_t show_pddr_link_down_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device *netdev = NULL;
    int                ret = 0;

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_PORT, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = _get_pddr(netdev, buf, SXD_PDDR_PAGE_SELECT_LINK_DOWN_INFO_E, 0);
    sx_netdev_sysfs_handler_end(netdev);

    return ret;
}

static ssize_t show_pddr_module_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device *netdev = NULL;
    int                ret = 0;

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_PORT, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = _get_pddr(netdev, buf, SXD_PDDR_PAGE_SELECT_MODULE_INFO_PAGE_E, 0);
    if (ret > 0) {
        ret += _get_pddr(netdev,
                         buf,
                         SXD_PDDR_PAGE_SELECT_TROUBLESHOOTING_INFO_PAGE_E, ret);
    }
    sx_netdev_sysfs_handler_end(netdev);

    return ret;
}
static ssize_t show_pddr_latched_flag_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device *netdev = NULL;
    int                ret = 0;

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_PORT, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = _get_pddr(netdev, buf, SXD_PDDR_PAGE_SELECT_MODULE_LATCHED_FLAG_INFO_PAGE_E, 0);
    sx_netdev_sysfs_handler_end(netdev);

    return ret;
}

static ssize_t __show_netdev_local_param_int(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int            ret = 0;
    struct sx_dev *dev = NULL;
    int            val = 0;

    ret = sx_sysfs_asic_get_dev_from_kobj(kobj, &dev, NETDEV_DRV_NAME);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "Failed to get device from kobject, err=%d\n", ret);
        return ret;
    }

    if (attr == &lp_sx_netdev_tx_debug_attr) {
        val = __netdev_tx_debug(dev);
    } else if (attr == &lp_sx_netdev_rx_debug_attr) {
        val = __netdev_rx_debug(dev);
    } else {
        sxd_log_err(dev, "sx_netdev unknown local param %s\n", attr->attr.name);
        return -EINVAL;
    }

    return sprintf(buf, "%d\n", val);
}

static ssize_t __store_netdev_local_param_int(struct kobject        *kobj,
                                              struct kobj_attribute *attr,
                                              const char            *buf,
                                              size_t                 len)
{
    struct sx_dev             *dev = NULL;
    struct sx_dev_netdev_priv *priv = NULL;
    int                        ret = 0;
    int                        val = 0;

    ret = sx_sysfs_asic_get_dev_from_kobj(kobj, &dev, NETDEV_DRV_NAME);
    if (ret) {
        sxd_log_err(dev, "Failed to get device from kobject, err=%d\n", ret);
        return ret;
    }

    priv = dev_netdev_priv_from_dev(dev);
    if (!priv) {
        sxd_log_err(dev, "sysfs error while getting net_priv from dev ret=%d\n", ret);
        return -ENODEV;
    }

    ret = kstrtoint(buf, 10, &val);
    if (ret) {
        goto out;
    }

    if (attr == &lp_sx_netdev_tx_debug_attr) {
        MODULE_PARAM_INSTANCE_SET(priv, sx_netdev_tx_debug, val);
    } else if (attr == &lp_sx_netdev_rx_debug_attr) {
        MODULE_PARAM_INSTANCE_SET(priv, sx_netdev_rx_debug, val);
    } else {
        sxd_log_err(dev, "sx_netdev unknown local param %s\n", attr->attr.name);
        ret = -EINVAL;
        goto out;
    }

    ret = len;

out:
    dev_netdev_priv_put(priv);
    return ret;
}


typedef struct pude_dwdata {
    struct sx_dev_netdev_priv *priv;
    u16                        sysport;
    int                        is_up;
    struct delayed_work        dwork;
} pude_dwdata_t;

u64 sx_netdev_mac_to_u64(u8 *addr)
{
    u64 mac = 0;
    int i;

    for (i = 0; i < ETH_ALEN; i++) {
        mac <<= 8;
        mac |= addr[i];
    }
    return mac;
}

void sx_netdev_u64_to_mac(u8* addr, u64 mac)
{
    int i;

    for (i = ETH_ALEN - 1; i >= 0; i--) {
        addr[i] = (u8)(mac & 0xFF);
        mac >>= 8;
    }
}


/*============================================================================
 *                          span2cpu mirror function implementation
 *============================================================================
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) || defined(CONFIG_SX_RHEL_8_6)
#define SPAN2CPU_IFNAME "mirror"


static bool sx_span2cpu_is_span2cpu_pkt(struct completion_info *comp_info)
{
    return comp_info->cqe_params.trap_id == SPAN_TO_CPU_TRAP_ID_ING ||
           comp_info->cqe_params.trap_id == SPAN_TO_CPU_TRAP_ID_EGR;
}

static void sx_span2cpu_send_to_mirror_taps(struct sk_buff *skb,  struct sx_dev *dev)
{
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);
    bool                       need_bh_off = !(hardirq_count() | softirq_count());

    if (!priv) {
        return;
    }

    skb_reset_network_header(skb);
    skb_reset_mac_len(skb);
    skb_push(skb, skb->mac_len);
    if (need_bh_off) {
        local_bh_disable();
    }
    dev_queue_xmit_nit(skb, priv->span2cpu_dev);
    if (need_bh_off) {
        local_bh_enable();
    }
    skb_pull(skb, skb->mac_len);

    dev_netdev_priv_put(priv);
}

static int sx_span2cpu_create_mirror_netdev(const struct net_device_ops *netdev_ops,
                                            const struct ethtool_ops    *ethtool_ops,
                                            struct sx_dev               *dev)
{
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);
    struct sx_net_priv        *net_priv;
    int                        err = 0;

    if (!priv) {
        return -ENODEV;
    }

    priv->span2cpu_dev = alloc_netdev(sizeof(struct sx_net_priv),
                                      SPAN2CPU_IFNAME, NET_NAME_UNKNOWN,
                                      sx_netdev_setup);

    if (!priv->span2cpu_dev) {
        err = -ENOMEM;
        goto out;
    }
    dev_net_set(priv->span2cpu_dev, priv->netns);

    priv->span2cpu_dev->features |= NETIF_F_HW_VLAN_CTAG_FILTER |
                                    NETIF_F_HW_TC;
    priv->span2cpu_dev->mtu = DEFAULT_FRAME_SIZE;

    priv->span2cpu_dev->netdev_ops = netdev_ops;
    priv->span2cpu_dev->ethtool_ops = ethtool_ops;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0))
    eth_hw_addr_random(priv->span2cpu_dev);
#else
    random_ether_addr(priv->span2cpu_dev->dev_addr);
#endif
    net_priv = netdev_priv(priv->span2cpu_dev);
    net_priv->dev = dev;

    sx_netdev_rtnl_lock();
    if (register_netdevice(priv->span2cpu_dev)) {
        sx_netdev_rtnl_unlock();
        sxd_log_err(dev, "Error when registering SPAN-to-CPU netdevice");
        free_netdev(priv->span2cpu_dev);
        priv->span2cpu_dev = NULL;

        err = -ENXIO;
        goto out;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) || defined(CONFIG_SX_RHEL_8_6)
    dev_open(priv->span2cpu_dev, NULL);
#else
    dev_open(priv->span2cpu_dev);
#endif

    netif_carrier_off(priv->span2cpu_dev);

    sx_netdev_rtnl_unlock();

out:
    dev_netdev_priv_put(priv);
    return err;
}

static void __sx_span2cpu_free_mirror_netdev(struct sx_dev *dev)
{
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return;
    }

    if (!priv->span2cpu_dev) {
        sxd_log_err(dev,
                    "trying to free span2cpu netdev for device [%u] but it is already NULL\n",
                    dev->device_id);
        return;
    }

    sx_netdev_rtnl_lock();
    dev_close(priv->span2cpu_dev);
    unregister_netdevice(priv->span2cpu_dev);
    sx_netdev_rtnl_unlock();
    /*We don't need to call free_netdev() here because we set needs_free_netdev = true in sx_netdev_setup */
    priv->span2cpu_dev = NULL;

    dev_netdev_priv_put(priv);
}

#endif /* if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) */


/*============================================================================
 *                          netdev dump function implementation
 *============================================================================
 */
static void __sx_netdev_dump_per_dev(struct seq_file   *m,
                                     const char        *port_type,
                                     const char        *ext_info,
                                     struct net_device *netdev)
{
    struct sx_net_priv *net_priv = netdev_priv(netdev);
    char                id[10] = "N/A";

    if (net_priv->is_port_netdev) {
        snprintf(id, sizeof(id), "%u", net_priv->port);
    } else if (net_priv->is_lag) {
        snprintf(id, sizeof(id), "%u", net_priv->mid);
    } else if (net_priv->is_bridge) {
        snprintf(id, sizeof(id), "%u", net_priv->bridge_id);
    }

    seq_printf(m, "%-7d    %-16s    0x%-8x    %-8s    %-5s    %s\n",
               netdev->ifindex,
               netdev->name,
               net_priv->log_port,
               port_type,
               id,
               ext_info);
}

extern int two_phase_port_init;
static int __netdev_dbg_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_dev             *dev = sx_dbg_dump_get_device(m);
    int                        i = 0, port = 0, port_type = 0;
    int                        max_ports[] = {
        [PORT_TYPE_SINGLE] = MAX_SYSPORT_NUM,
        [PORT_TYPE_LAG] = MAX_LAG_NUM
    };
    const char                *port_type_str[] = {
        [PORT_TYPE_SINGLE] = "Physical",
        [PORT_TYPE_LAG] = "LAG"
    };
    char                       info[128];
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return 0;
    }

    sx_dbg_dump_print_header(m, "sx_netdev information dump");

    seq_printf(m, "Kconfig\n");
    seq_printf(m, "    CONFIG_PSAMPLE ........................... %s\n", IS_ENABLED(CONFIG_PSAMPLE) ? "yes" : "no");
#ifdef PSAMPLE_MD_EXTENDED_ATTR
    seq_printf(m, "    PSAMPLE_MD_EXTENDED_ATTR ................. yes\n");
#else
    seq_printf(m, "    PSAMPLE_MD_EXTENDED_ATTR ................. no\n");
#endif
#ifdef CONFIG_NET_PORT
    seq_printf(m, "    CONFIG_NET_PORT .......................... yes\n");
#else
    seq_printf(m, "    CONFIG_NET_PORT .......................... no\n");
#endif
#ifdef SX_DROP_MONITOR_DISABLED
    seq_printf(m, "    SX_DROP_MONITOR_DISABLED ................. yes\n");
#else
    seq_printf(m, "    SX_DROP_MONITOR_DISABLED.................. no\n");
#endif
#ifdef SX_EXTEND_WJH_BUFFER_LINUX_CHANNEL
    seq_printf(m, "    SX_EXTEND_WJH_BUFFER_LINUX_CHANNEL ....... yes\n");
#else
    seq_printf(m, "    SX_EXTEND_WJH_BUFFER_LINUX_CHANNEL ....... no\n");
#endif
#ifdef SX_FLOW_OFFLOAD_DISABLED
    seq_printf(m, "    SX_FLOW_OFFLOAD_DISABLED ................. yes\n");
#else
    seq_printf(m, "    SX_FLOW_OFFLOAD_DISABLED ................. no\n");
#endif
    seq_printf(m, "sx_netdev parameters\n");
    seq_printf(m, "    arp_nd_mcast_sw_forwarding_enable ........ %d\n", arp_nd_mcast_sw_forwarding_enable);
    seq_printf(m, "    carrier_set_on_pude_disable .............. %d\n", carrier_set_on_pude_disable);
    seq_printf(m, "    change_proto_down_enable ................. %d\n", change_proto_down_enable);
    seq_printf(m, "    devlink_port_en .......................... %d\n", devlink_port_en);
    seq_printf(m, "    mirror_device_en ......................... %d\n", mirror_device_en);
    seq_printf(m, "    offload_fwd_mark_en ...................... %d\n", offload_fwd_mark_en);
    seq_printf(m, "    route_offload_flags ...................... %d\n", route_offload_flags);
    seq_printf(m, "    single_netdev_per_port_enable ............ %d\n", single_netdev_per_port_enable);
    seq_printf(m, "    sx_netdev_tc_psample_enable .............. %d\n", sx_netdev_tc_psample_enable);
    seq_printf(m, "    two_phase_port_init ...................... %d\n", two_phase_port_init);
    seq_printf(m, "    untagged_pkt_pvid_tag_disable ............ %d\n", untagged_pkt_pvid_tag_disable);

    sx_dbg_dump_print_empty_line(m);

    sx_dbg_dump_print_table_header(m, "%-7s    %-16s    %-10s    %-8s    %-5s    %s",
                                   "Ifindex",
                                   "Name",
                                   "LogPort",
                                   "Type",
                                   "ID",
                                   "Ext-Info");

    mutex_lock(&priv->netdev_resources.rsc_lock);

    /* go over all per swid devices */
    for (i = 0; i < NUMBER_OF_SWIDS; i++) {
        if (priv->netdev_resources.sx_netdevs[i] != NULL) {
            snprintf(info, sizeof(info), "sx_netdevs[index=%d]", i);
            __sx_netdev_dump_per_dev(m, "SWID", info, (priv->netdev_resources.sx_netdevs[i]));
        }
    }

    /* go over all per port devices */
    for (port_type = 0; port_type < PORT_TYPE_NUM; port_type++) {
        for (port = 0; port < max_ports[port_type]; port++) {
            for (i = 0; i < MAX_PORT_NETDEV_NUM; i++) {
                if (priv->netdev_resources.port_netdev[port_type][port][i] != NULL) {
                    snprintf(info, sizeof(info), "port_netdev[port_type=%d][port=%d][index=%d]", port_type, port, i);
                    __sx_netdev_dump_per_dev(m,
                                             port_type_str[port_type],
                                             info,
                                             (priv->netdev_resources.port_netdev[port_type][port][i]));
                }
            }
        }
    }

    mutex_unlock(&priv->netdev_resources.rsc_lock);

    /* go over all bridge devices */
    for (i = 0; i < MAX_BRIDGE_NUM; i++) {
        if (priv->bridge_netdev_db[i] != NULL) {
            snprintf(info, sizeof(info), "bridge_netdev_db[index=%d]", i);
            __sx_netdev_dump_per_dev(m, "Bridge", info, priv->bridge_netdev_db[i]);
        }
    }

    psample_rate_dump(m, priv->dev);
    dev_netdev_priv_put(priv);
    return 0;
}

/*
 * NETDEV is loaded with admin state down. User can change the admin state to UP and not the driver
 *
 *  static void netdev_adminstate_set(struct net_device *netdev, int should_rtnl_lock)
 *  {
 *   struct sx_net_priv *net_priv = netdev_priv(netdev);
 *
 *   if (should_rtnl_lock)
 *       rtnl_lock();
 *
 *   if (net_priv->admin_state) {
 *       dev_change_flags(netdev, netdev->flags | IFF_UP);
 *   } else {
 *       dev_change_flags(netdev, netdev->flags & ~IFF_UP);
 *   }
 *
 *   if (should_rtnl_lock)
 *       rtnl_unlock();
 *  }
 */

void netdev_linkstate_set(struct net_device *netdev)
{
    struct sx_net_priv *net_priv = netdev_priv(netdev);

    if (carrier_set_on_pude_disable && net_priv->is_port_netdev && !net_priv->is_lag) {
        return;
    }

    if (net_priv->is_oper_state_up) {
        netif_carrier_on(netdev);
        netif_start_queue(netdev);
    } else {
        netif_tx_disable(netdev);
        netif_carrier_off(netdev);
    }
}

static u32 sx_netdev_calc_vlan_extra_allocation_bytes(u8 vlan_child_exist, u8 is_tagged)
{
    if (!untagged_pkt_pvid_tag_disable &&
        (is_tagged == VLAN_UNTAGGED_E) && vlan_child_exist) {
        return VLAN_HLEN;
    }
    return 0;
}

static int sx_netdev_rx_apply_vlan_tag_logic(u8              vlan_child_exist,
                                             u8              is_tagged,
                                             struct sk_buff *old_skb_p,
                                             struct sk_buff *new_skb_p,
                                             u16             pvid)
{
    u32              vlan_tag = 0;
    u32              vlan_extra_bytes = 0;
    struct vlan_hdr *vhdr_p = NULL;
    u8              *skb_data_p = NULL;

    /* Calculate if we need extra bytes for VLAN tag */
    vlan_extra_bytes = sx_netdev_calc_vlan_extra_allocation_bytes(vlan_child_exist, is_tagged);

    /* Handle VLAN tag insertion or modification */
    if (vlan_extra_bytes > 0) {
        /* Need to add VLAN tag */
        skb_data_p = skb_put(new_skb_p, old_skb_p->len + VLAN_HLEN);
        if (!skb_data_p) {
            return -ENOMEM;
        }

        vlan_tag = ETH_P_8021Q << 16;
        vlan_tag |= (pvid & 0x3fff);
        vlan_tag = cpu_to_be32(vlan_tag);
        /* Copy dest + source MAC addresses, insert VLAN tag, copy ethertype + payload */
        memcpy(skb_data_p, old_skb_p->data, (2 * ETH_ALEN));
        memcpy(skb_data_p + (2 * ETH_ALEN), &vlan_tag, VLAN_HLEN);
        memcpy(skb_data_p + (2 * ETH_ALEN) + VLAN_HLEN, old_skb_p->data + (2 * ETH_ALEN),
               old_skb_p->len - (2 * ETH_ALEN));
    } else {
        /* No VLAN tag addition needed, but may need to modify existing tag */
        if ((is_tagged == VLAN_PRIO_TAGGED_E) && vlan_child_exist) {
            vhdr_p = (struct vlan_hdr *)(old_skb_p->data + ETH_HLEN);
            vhdr_p->h_vlan_TCI = (vhdr_p->h_vlan_TCI & htons(~VLAN_VID_MASK)) | htons(pvid);
        }
        memcpy(skb_put(new_skb_p, old_skb_p->len), old_skb_p->data, old_skb_p->len);
    }

    return 0;
}

static void sx_netdev_write_nv_records(u8 *buffer_p, u16 size, u16 trap_id)
{
    u16                         unaligned_size = 0;
    u16                         records_written = 0;
    u16                         padding_size = 0;
    sx_nv_records_terminator_t *terminator_p = NULL;
    sx_nv_record_trap_id_t     *trap_id_record_p = NULL;
    u8                         *current_p = NULL;

    /* Calculate unaligned size (records + terminator) */
    if (SX_NV_RECORD_IS_TYPE_ENABLED(nv_records_enable, SX_NV_RECORD_TYPE_TRAP_ID_E)) {
        unaligned_size += sizeof(sx_nv_record_trap_id_t);
        records_written++;
    }

    if (records_written > 0) {
        unaligned_size += sizeof(sx_nv_records_terminator_t);
    }

    /*
     * Write padding at BEGINNING to ensure terminator at fixed offset from skb->data.
     * This allows upper layer to read the terminator at a fixed offset (-sizeof(terminator))
     * for efficient parsing without variable offset calculations.
     */
    padding_size = size - unaligned_size;
    current_p = buffer_p;

    if (padding_size > 0) {
        memset(current_p, 0, padding_size);
        current_p += padding_size;
    }

    /* Write records after padding */
    if (SX_NV_RECORD_IS_TYPE_ENABLED(nv_records_enable, SX_NV_RECORD_TYPE_TRAP_ID_E)) {
        trap_id_record_p = (sx_nv_record_trap_id_t *)current_p;
        trap_id_record_p->header.type = SX_NV_RECORD_TYPE_TRAP_ID_E;
        trap_id_record_p->header.reserved = 0;
        trap_id_record_p->header.length = sizeof(sx_nv_record_trap_id_value_t);
        trap_id_record_p->value.trap_id = trap_id;
        current_p += sizeof(sx_nv_record_trap_id_t);
    }

    /* Write terminator at end (always at -sizeof(terminator) from skb->data) */
    if (records_written > 0) {
        terminator_p = (sx_nv_records_terminator_t *)current_p;
        terminator_p->nv_flag = SX_NV_RECORDS_FLAG;
        terminator_p->total_length = (u8)unaligned_size;  /* Unaligned size (no padding) */
        terminator_p->reserved = 0;
    }
}

static u16 sx_netdev_calc_nv_records_size(void)
{
    u16 unaligned_size = 0;

    /* Check if trap_id feature is enabled */
    if (SX_NV_RECORD_IS_TYPE_ENABLED(nv_records_enable, SX_NV_RECORD_TYPE_TRAP_ID_E)) {
        /* For trap_id record: header + value */
        unaligned_size += sizeof(sx_nv_record_trap_id_t);
    }

    /* Add terminator if any records are enabled */
    if (unaligned_size > 0) {
        unaligned_size += sizeof(sx_nv_records_terminator_t);
    }

    /* Align to 4-byte boundary */
    return ALIGN(unaligned_size, SX_NV_RECORDS_ALIGNMENT);
}

static int sx_netdev_rx_apply_vlan_and_nv_records(u8                            vlan_child_exist,
                                                  const struct completion_info *comp_info_p,
                                                  struct sk_buff              **new_skb_pp)
{
    u16             nv_records_size = 0;
    u16             vlan_extra_bytes = 0;
    int             err = 0;
    struct sk_buff *old_skb_p = NULL;
    struct sk_buff *new_skb_p = NULL;
    u8             *headroom_p = NULL;

    old_skb_p = comp_info_p->skb;

    /* Calculate NV records headroom size (aligned, 0 if disabled) */
    nv_records_size = sx_netdev_calc_nv_records_size();

    /* Calculate extra packet data space needed for VLAN tag insertion (0 or VLAN_HLEN) */
    vlan_extra_bytes = sx_netdev_calc_vlan_extra_allocation_bytes(vlan_child_exist, comp_info_p->is_tagged);

    /* Allocate new SKB: packet_len + VLAN_space + IP_align + NV_headroom */
    new_skb_p = netdev_alloc_skb(NULL, old_skb_p->len + vlan_extra_bytes + NET_IP_ALIGN + nv_records_size);
    if (!new_skb_p) {
        sxd_log_err(SXD_DEV_FROM_COMP_INFO(comp_info_p), "Failed to allocate new SKB\n");
        err = -ENOMEM;
        goto out;
    }

    /* Reserve NET_IP_ALIGN first (for IP header alignment) */
    skb_reserve(new_skb_p, NET_IP_ALIGN);

    /* Write NV records at current data pointer if enabled */
    if (nv_records_size > 0) {
        headroom_p = new_skb_p->data;
        sx_netdev_write_nv_records(headroom_p, nv_records_size, comp_info_p->cqe_params.trap_id);
        /* Reserve NV records space - moves data pointer past NV records */
        skb_reserve(new_skb_p, nv_records_size);
    }

    /* Get reference to old SKB before passing to VLAN logic */
    skb_get(old_skb_p);

    /* Apply VLAN tag logic (insertion/modification) - writes packet data at current skb->data */
    err = sx_netdev_rx_apply_vlan_tag_logic(vlan_child_exist, comp_info_p->is_tagged,
                                            old_skb_p, new_skb_p, comp_info_p->vid);
    if (err) {
        sxd_log_err(SXD_DEV_FROM_COMP_INFO(comp_info_p), "Failed to apply VLAN tag logic\n");
        kfree_skb(new_skb_p);
        consume_skb(old_skb_p);
        goto out;
    }

    *new_skb_pp = new_skb_p;

    /* Consume old SKB */
    consume_skb(old_skb_p);

out:
    return err;
}

static inline bool sx_is_arp_nd_on_network_port(struct completion_info *comp_info)
{
    return (comp_info->cqe_params.trap_id == SXD_TRAP_ID_NVE_DECAP_ARP ||
            comp_info->cqe_params.trap_id == SXD_TRAP_ID_OVERLAY_ICMPV6_CONF_TYPE);
}

#if defined(CONFIG_NET_SWITCHDEV)
static void sx_skb_set_mr_fwd_mark(struct sk_buff *skb, u8 fwd_mark)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)) || defined(CONFIG_SX_RHEL_8_6) || defined(CONFIG_SX_CENTOS_8_4)
    skb->offload_l3_fwd_mark = fwd_mark;
#else
    skb->offload_mr_fwd_mark = fwd_mark;
#endif
}
#endif /* defined(CONFIG_NET_SWITCHDEV) */

static void sx_netdev_fix_sx_skb(struct completion_info *comp_info, struct sk_buff *skb, struct net_device *netdev)
{
    struct ethhdr *eth_h = (struct ethhdr *)(skb->data);


    if (comp_info->cqe_params.trap_id == SXD_TRAP_ID_ETH_L3_MTUERROR) {
        struct iphdr *ipptr;
        u32           new_checksum;
        u16           old_frag_off;

        if (be16_to_cpu(eth_h->h_proto) == ETH_P_8021Q) {
            ipptr = (struct iphdr *)(skb->data + VLAN_ETH_HLEN);
        } else {
            ipptr = (struct iphdr *)(skb->data + ETH_HLEN);
        }

        if (ipptr->version == IPVERSION) {
            /* RFC 1624 describes how to recalculate the checksum via incremental update */
            old_frag_off = be16_to_cpu(ipptr->frag_off); /* save old flags */
            if (!(old_frag_off & 0x4000)) { /* If don't fragment bit is not set */
                ipptr->frag_off = cpu_to_be16(old_frag_off | 0x4000); /* set Don't-Fragment bit */
                /* calculate the new checksum */
                new_checksum = (~old_frag_off & 0xffff) + (be16_to_cpu(ipptr->frag_off) & 0xffff); /* checksum' = ~m+m' */
                new_checksum += (~be16_to_cpu(ipptr->check) & 0xffff); /* checksum' += ~checksum */
                new_checksum = (new_checksum & 0xffff) + (new_checksum >> 16); /* Add carry if such exists */
                ipptr->check = cpu_to_be16(~(new_checksum + (new_checksum >> 16))); /* checksum' = ~checksum' */
            }
        }
    } else if (comp_info->cqe_params.trap_id == SXD_TRAP_ID_ETH_L3_LBERROR) {
        memcpy(eth_h->h_dest, netdev->dev_addr, netdev->addr_len);
    }

    skb->dev = netdev;
    skb->protocol = eth_type_trans(skb, netdev);
    skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */

/* set offload mark of skb */
#if defined(CONFIG_NET_SWITCHDEV)
    if (offload_fwd_mark_en) {
        skb->offload_fwd_mark = 1;
        if (arp_nd_mcast_sw_forwarding_enable) {
            if (sx_is_arp_nd_on_network_port(comp_info)) {
                sx_skb_set_mr_fwd_mark(skb, 0);
            } else {
                sx_skb_set_mr_fwd_mark(skb, 1);
            }
        }
    } else {
        skb->offload_fwd_mark = 0;
        sx_skb_set_mr_fwd_mark(skb, 0);
    }
#endif
}

static void sx_netdev_handle_rx(struct completion_info *comp_info, struct net_device *netdev)
{
    struct sx_net_priv         *net_priv = NULL;
    struct sk_buff             *skb;
    unsigned long               last_bit = 0;
    u8                          vlan_child_exist = 0;
    int                         ret = 0;
    struct skb_shared_hwtstamps hwts = *skb_hwtstamps(comp_info->skb);

    if (!netdev) {
        return;
    }

    if (!(netdev->flags & IFF_UP)) {
        if (__netdev_rx_debug(comp_info->dev)) {
            sxd_log_err(SXD_DEV_FROM_COMP_INFO(comp_info),
                        "[rx_debug] %s: netdev %s is down! dropping packet!\n",
                        __func__,
                        netdev->name);
        }
        return;
    }

    net_priv = netdev_priv(netdev);
    if (!net_priv) {
        if (sx_netdev_rx_debug) {
            sxd_log_err(SXD_DEV_FROM_COMP_INFO(comp_info), "[rx_debug] %s: net_priv is NULL\n", __func__);
        }
        return;
    }

    last_bit = find_last_bit(net_priv->active_vlans, VLAN_N_VID);
    vlan_child_exist = last_bit < VLAN_N_VID && last_bit != 0;
    ret = sx_netdev_rx_apply_vlan_and_nv_records(vlan_child_exist, comp_info, &skb);
    if (ret) {
        net_priv->stats.rx_dropped++;
        if (__netdev_rx_debug(comp_info->dev)) {
            sxd_log_err(SXD_DEV_FROM_COMP_INFO(comp_info), "[rx_debug] %s: failed to apply vlan logic\n", __func__);
        }
        return;
    }

    sx_netdev_fix_sx_skb(comp_info, skb, netdev);

    skb_hwtstamps(skb)->hwtstamp = hwts.hwtstamp;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) || defined(CONFIG_SX_RHEL_8_6)
    if (mirror_device_en && sx_span2cpu_is_span2cpu_pkt(comp_info)) {
        sx_span2cpu_send_to_mirror_taps(skb, comp_info->dev);
        dev_kfree_skb(skb);
        return;
    }
#endif

    net_priv->stats.rx_packets++;
    net_priv->stats.rx_bytes += skb->len;


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)) || defined(CONFIG_SX_RHEL_9_4)
    netif_rx(skb);
#else
    if (in_interrupt()) {
        netif_rx(skb);
    } else {
        netif_rx_ni(skb);
    }
#endif
}

static void sx_netdev_log_port_rx_pkt(struct completion_info *comp_info, void *context)
{
    /* Get routed netdev */
    u8                         i = 0, port_type = 0;
    int                        num_netdevs = single_netdev_per_port_enable ? 1 : MAX_PORT_NETDEV_NUM;
    struct net_device         *netdev;
    struct sx_dev             *dev = comp_info->dev;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return;
    }

    port_type = comp_info->cqe_params.is_lag ? PORT_TYPE_LAG : PORT_TYPE_SINGLE;

    rcu_read_lock();

    for (i = 0; i < num_netdevs; i++) {
        netdev =
            rcu_dereference(priv->netdev_resources.port_netdev[port_type][comp_info->cqe_params.sysport_lag_id][i]);
        if (netdev) {
            sx_netdev_handle_rx(comp_info, netdev);
        } else {
            if (MODULE_PARAM_INSTANCE_GET(priv, sx_netdev_rx_debug)) {
                sxd_log_err(SXD_DEV_FROM_COMP_INFO(comp_info),
                            "[rx_debug] %s: %s %d index %d netdev is NULL! dropping packet!\n",
                            __func__,
                            comp_info->cqe_params.is_lag ? "lag" : "port",
                            comp_info->cqe_params.sysport_lag_id,
                            i);
            }
        }
    }

    rcu_read_unlock();
    dev_netdev_priv_put(priv);
}

static void sx_netdev_phy_port_rx_pkt(struct completion_info *comp_info, void *context)
{
    /* Get routed netdev */
    struct net_device         *netdev = context, *port_netdev;
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    struct sx_dev             *dev = comp_info->dev;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);
    uint16_t                   local = comp_info->cqe_params.sysport_lag_id;
    int                        num_netdevs;
    int                        ret = 0;
    u8                         i = 0;

    if (!priv) {
        return;
    }

    if (comp_info->cqe_params.is_lag) {
        ret = sx_core_get_local(net_priv->dev,
                                comp_info->cqe_params.sysport_lag_id,
                                comp_info->cqe_params.lag_subport,
                                &local);
        if (ret) {
            sxd_log_err(SXD_DEV_FROM_COMP_INFO(comp_info), "Fail to get local port from lag_id (%d) "
                        "and port index (%d)\n",
                        comp_info->cqe_params.sysport_lag_id,
                        comp_info->cqe_params.lag_subport);
            goto out;
        }
    }

    num_netdevs = single_netdev_per_port_enable ? 1 : MAX_PORT_NETDEV_NUM;

    rcu_read_lock();

    for (i = 0; i < num_netdevs; i++) {
        port_netdev = rcu_dereference(priv->netdev_resources.port_netdev[PORT_TYPE_SINGLE][local][i]);
        if (port_netdev) {
            sx_netdev_handle_rx(comp_info, port_netdev);
        } else {
            if (MODULE_PARAM_INSTANCE_GET(priv, sx_netdev_rx_debug)) {
                sxd_log_err(SXD_DEV_FROM_COMP_INFO(comp_info),
                            "[rx_debug] %s: port %d index %d netdev is NULL! dropping packet!\n",
                            __func__,
                            local,
                            i);
            }
        }
    }

    rcu_read_unlock();

out:
    dev_netdev_priv_put(priv);
}


static void __sx_netdev_handle_rx_pkt_user_netdev(struct completion_info *comp_info, void *context)
{
    struct net_device          *netdev = context;
    struct sk_buff             *old_skb = comp_info->skb;
    struct sk_buff             *new_skb = NULL;
    struct skb_shared_hwtstamps hwts = *skb_hwtstamps(comp_info->skb);


    rcu_read_lock();
    skb_get(old_skb);

    if (netdev != NULL) {
        if (!(netdev->flags & IFF_UP)) {
            if (__netdev_rx_debug(comp_info->dev)) {
                sxd_log_err(SXD_DEV_FROM_COMP_INFO(comp_info),
                            "[rx_debug] %s: netdev %s is down! dropping packet!\n",
                            __func__,
                            netdev->name);
            }
            goto out;
        }
        new_skb = netdev_alloc_skb(NULL, old_skb->len + NET_IP_ALIGN);
        if (!new_skb) {
            sxd_log_err(SXD_DEV_FROM_COMP_INFO(comp_info), "%s: out of memory!\n", __func__);
            goto out;
        }
        skb_reserve(new_skb, NET_IP_ALIGN);
        memcpy(skb_put(new_skb, old_skb->len), old_skb->data, old_skb->len);

        sx_netdev_fix_sx_skb(comp_info, new_skb, netdev);

        skb_hwtstamps(new_skb)->hwtstamp = hwts.hwtstamp;

        netdev->stats.rx_packets++;
        netdev->stats.rx_bytes += new_skb->len;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)) || defined(CONFIG_SX_RHEL_9_4)
        netif_rx(new_skb);
#else
        if (in_interrupt()) {
            netif_rx(new_skb);
        } else {
            netif_rx_ni(new_skb);
        }
#endif
    } else if (__netdev_rx_debug(comp_info->dev)) {
        sxd_log_err(SXD_DEV_FROM_COMP_INFO(comp_info), "[rx_debug] %s: netdev is NULL! dropping packet!\n", __func__);
    }
out:
    consume_skb(old_skb);   /* free unused skb, use consume_skb */
    rcu_read_unlock();
}

static void sx_netdev_handle_rx_pkt(struct completion_info *comp_info, void *context)
{
    struct net_device *netdev = context;

    if (netdev != NULL) {
        sx_netdev_handle_rx(comp_info, netdev);
    } else if (__netdev_rx_debug(comp_info->dev)) {
        sxd_log_err(SXD_DEV_FROM_COMP_INFO(comp_info), "[rx_debug] %s: netdev is NULL! dropping packet!\n",
                    __func__);
    }
}

static void __sx_netdev_pude_dwork_handler(struct work_struct *dwork_p)
{
    struct net_device         *netdev = NULL;
    struct sx_net_priv        *net_priv = NULL;
    int                        i = 0;
    pude_dwdata_t             *dwdata_p = container_of(dwork_p, pude_dwdata_t, dwork.work);
    int                        is_up = dwdata_p->is_up;
    u16                        sysport = dwdata_p->sysport;
    struct sx_dev_netdev_priv *priv = dwdata_p->priv;

    mutex_lock(&priv->netdev_resources.rsc_lock);

    /* Change port status for port netdev */
    for (i = 0; i < MAX_PORT_NETDEV_NUM; i++) {
        netdev = priv->netdev_resources.port_netdev[PORT_TYPE_SINGLE][sysport][i];
        if (netdev) {
            net_priv = netdev_priv(netdev);
            net_priv->is_oper_state_up = is_up;
            netdev_linkstate_set(netdev);
        }
    }

    mutex_unlock(&priv->netdev_resources.rsc_lock);

    dev_netdev_priv_put(priv); /* reference acquired in sx_netdev_handle_pude_event() */
    kfree(dwdata_p);
}

/* For Palladium skip event registration which invoke sysfs_notify as sysfs init is skipped */
#if !defined(PD_BU)

static void __sx_netdev_pmpe_dwork_handler(struct work_struct *dwork_p)
{
    pmpe_dwdata_t *dwdata_p = container_of(dwork_p, pmpe_dwdata_t, dwork.work);

    sysfs_notify(dwdata_p->module, NULL, "present");

    kfree(dwdata_p);
}

static void sx_netdev_handle_pmpe_event(struct completion_info *comp_info, void *context)
{
    struct net_device         *netdev = NULL;
    struct sx_net_priv        *net_priv = NULL;
    struct sx_dev             *dev = comp_info->dev;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);
    struct ku_pmpe_reg         pmpe_reg;
    u8                        *pmpe_outbox = (u8*)comp_info->skb->data + sizeof(struct sx_emad) +
                                             sizeof(struct sxd_emad_tlv_reg);
    uint16_t                     local_ports[MAX_LANE_NUM_PER_MODULE];
    uint16_t                     sysports[MAX_LANE_NUM_PER_MODULE];
    uint8_t                      ports_cnt = 0;
    int                          i = 0;
    int                          j = 0;
    enum sx_module_present_state present = SX_MODULE_PRESENT_INVALID;
    pmpe_dwdata_t               *dwdata_p;

    if (!priv) {
        return;
    }

    if (!sx_validate_emad_format(comp_info, MLXSW_PMPE_ID, 4, 0)) {
        goto out;
    }

    __PMPE_decode(pmpe_outbox, &pmpe_reg, NULL);

    sx_core_get_ports_by_module(dev, pmpe_reg.slot_index, pmpe_reg.module,
                                local_ports, sysports, &ports_cnt);

    for (i = 0; i < ports_cnt; i++) {
        for (j = 0; j < MAX_PORT_NETDEV_NUM; j++) {
            netdev = priv->netdev_resources.port_netdev[PORT_TYPE_SINGLE][sysports[i]][j];
            if (netdev) {
                net_priv = netdev_priv(netdev);
                present =
                    PMPE_MODULE_PLUGGED(pmpe_reg.module_status) ? SX_MODULE_PRESENT_PLUGGED :
                    SX_MODULE_PRESENT_UNPLUGGED;

                if (present != net_priv->module_present) {
                    net_priv->module_present = present;
                    dwdata_p = kzalloc(sizeof(*dwdata_p), GFP_ATOMIC);
                    if (!dwdata_p) {
                        sxd_log_err(SXD_DEV_FROM_COMP_INFO(comp_info), "PMPE handler failed to allocated memory\n");
                        return;
                    }
                    dwdata_p->module = net_priv->kobj[SX_NET_KOBJECT_MODULE];
                    INIT_DELAYED_WORK(&dwdata_p->dwork, __sx_netdev_pmpe_dwork_handler);
                    /* The delayed queueing used to change context */
                    queue_delayed_work(priv->netdev_wq, &dwdata_p->dwork, 0);
                }
            }
        }
    }

out:
    dev_netdev_priv_put(priv);
}
#endif /* #if !defined(PD_BU) */


/* this function is to protect against rapid port flapping.
 * we want to log every port operational state change. however, if port is
 * flapping rapidly, each print to the log will consume its CPU and exhaust it.
 *
 * The print will take place only if port is not marked as 'rapid flap' and there
 * were less than SX_MAX_PORT_TOGGLE_PER_SEC state updates per second.
 *
 * There is a timer that runs once per second to reset the counters and/or 'rapid flap'
 * marking.
 */
static void __conditional_log_pude_event(struct sx_dev_netdev_priv *priv, u16 local_port, u32 logical_port, bool is_up)
{
    spin_lock_bh(&priv->local_port_toggle_monitor_lock);

    /* always store the latest port state, so when port is not flapping anymore
     *  we will print the updated state of the port. */
    priv->local_port_toggle_monitor[local_port - 1].is_up = is_up;

    if (priv->local_port_toggle_monitor[local_port - 1].toggle_count < SX_MAX_PORT_TOGGLE_PER_SEC) {
        /* increment the PUDE counter for the port if it did not reach SX_MAX_PORT_TOGGLE_PER_SEC yet. */
        priv->local_port_toggle_monitor[local_port - 1].toggle_count++;

        /* if the port is not marked as 'rapid flap', put a log message for the port oper-state update */
        if (!priv->local_port_toggle_monitor[local_port - 1].rapid_flap) {
            sxd_log_notice(priv->dev, "Called for logical port - %05X status %s\n",
                           logical_port, is_up ? "UP" : "DOWN");
            goto out;
        }
    }

    /* if port is marked as 'rapid flap', just go out */
    if (priv->local_port_toggle_monitor[local_port - 1].rapid_flap) {
        goto out;
    }

    /* port is not marked as 'rapid flap' and we reached a PUDE counter of SX_MAX_PORT_TOGGLE_PER_SEC.
     * this is where we mark the port as 'rapid flap'. Further PUDE events on the port will not be
     * printed to log.
     * The case to uncheck 'rapid flap' is when there are less than SX_MAX_PORT_TOGGLE_PER_SEC PUDE
     * events on the port per second (periodic work is responsible for this).
     */
    sxd_log_warning(priv->dev,
                    "Logical port %05X is flapping rapidly! "
                    "No more status reports for this port until it stops flapping!\n",
                    logical_port);
    priv->local_port_toggle_monitor[local_port - 1].rapid_flap = true;

out:
    spin_unlock_bh(&priv->local_port_toggle_monitor_lock);
}

/* Called on every PUDE event */
static void sx_netdev_handle_pude_event(struct completion_info *comp_info, void *context)
{
    struct ku_pude_reg pude_reg;
    u8                *pude_outbox = (u8*)comp_info->skb->data + sizeof(struct sx_emad) +
                                     sizeof(struct sxd_emad_tlv_reg);
    unsigned int               logical_port = 0;
    uint16_t                   local_port = 0;
    int                        sysport = 0;
    int                        is_up = 0, is_lag = 0;
    pude_dwdata_t             *dwdata_p;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(comp_info->dev);

    if (!priv) {
        return;
    }

    if (!sx_validate_emad_format(comp_info, PUDE_REG_ID, 4, 0)) {
        goto out_err;
    }

    __PUDE_decode(pude_outbox, &pude_reg, NULL);

    SX_PORT_BUILD_PHY_ID_FROM_LSB_MSB(local_port, pude_reg.local_port, pude_reg.lp_msb);
    logical_port = SX_LOCAL_PORT_TO_LOG_PORT(local_port);
    sysport = translate_user_port_to_sysport(comp_info->dev,
                                             logical_port,
                                             &is_lag);
    if (sysport < 0) { /* error */
        goto out_err;
    }

    /* if port is a LAG port do nothing */
    if (is_lag) {
        goto out_err;
    }
    is_up = pude_reg.oper_status == SXD_PAOS_OPER_STATUS_UP_E;

    if (!carrier_set_on_pude_disable) {
        __conditional_log_pude_event(priv, local_port, logical_port, !!is_up);
    }

    dwdata_p = kzalloc(sizeof(*dwdata_p), GFP_ATOMIC);
    if (!dwdata_p) {
        sxd_log_err(SXD_DEV_FROM_COMP_INFO(comp_info), "PUDE handler failed to allocated memory\n");
        goto out_err;
    }
    dwdata_p->is_up = is_up;
    dwdata_p->sysport = sysport;
    dwdata_p->priv = priv;
    INIT_DELAYED_WORK(&dwdata_p->dwork, __sx_netdev_pude_dwork_handler);
    /* The delayed queueing used to change context */
    queue_delayed_work(priv->netdev_wq, &dwdata_p->dwork, 0);

    return; /* don't release 'priv' here (sx_dev_netdev_priv_put()). it will be released in __sx_netdev_pude_dwork_handler() */

out_err:
    dev_netdev_priv_put(priv);
}

static void sx_netdev_remove_vlan_header(struct sk_buff *skb)
{
    struct vlan_ethhdr *veth = NULL;

    veth = (struct vlan_ethhdr *)(skb->data);

    if (ntohs(veth->h_vlan_proto) != ETH_P_8021Q) {
        /* No vlan header */
        return;
    }

    /* set the protocol in ETH header */
    veth->h_vlan_proto = veth->h_vlan_encapsulated_proto;

    /* remove the VLAN header */
    memmove(skb->data + VLAN_HLEN, skb->data, ETH_HLEN);
    skb->mac_header += VLAN_HLEN;
    skb_pull(skb, VLAN_HLEN);

    skb_reset_network_header(skb);
    skb_reset_transport_header(skb);
    /* skb_reset_mac_len */
    skb->mac_len = skb->network_header - skb->mac_header;
}

static void __sx_netdev_handle_global_pkt(struct net_device *netdev, struct completion_info *comp_info)
{
    struct sx_net_priv *net_priv = NULL;
    int                 err = 0;
    u16                 vlan = 0;

    if (!netdev) {
        if (__netdev_rx_debug(comp_info->dev)) {
            sxd_log_err(SXD_DEV_FROM_COMP_INFO(comp_info),
                        "[rx_debug] %s: netdev is NULL! dropping packet!\n",
                        __FUNCTION__);
        }
        return;
    }

    /* SX: if RP netdev existed than pass the packet up with this netdev */
    net_priv = netdev_priv(netdev);
    err = sx_core_get_rp_vlan(net_priv->dev, comp_info, &vlan);
    if (err) {
        sxd_log_err(SXD_DEV_FROM_COMP_INFO(comp_info), "Failed sx_core_get_rp_vlan(). err: %d \n", err);
    }
    comp_info->vid = vlan;
    sx_netdev_handle_rx(comp_info, netdev);
}

static void sx_netdev_handle_global_pkt(struct completion_info *comp_info, void *context)
{
    struct net_device         *netdev = NULL;
    u8                         port_type = 0, i = 0;
    struct sx_dev             *dev = comp_info->dev;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return;
    }

    if (comp_info->bridge_id) {
        if ((comp_info->bridge_id < MIN_BRIDGE_ID)
            || (comp_info->bridge_id > MAX_BRIDGE_ID)) {
            sxd_log_err(SXD_DEV_FROM_COMP_INFO(comp_info), "Bridge ID %u is out of range [%u,%u]\n",
                        comp_info->bridge_id, MIN_BRIDGE_ID, MAX_BRIDGE_ID);
            goto out;
        }
        netdev = priv->bridge_netdev_db[comp_info->bridge_id - MIN_BRIDGE_ID];
        __sx_netdev_handle_global_pkt(netdev, comp_info);
    } else {
        port_type = comp_info->cqe_params.is_lag ? PORT_TYPE_LAG : PORT_TYPE_SINGLE;

        rcu_read_lock();

        for (i = 0; i < MAX_PORT_NETDEV_NUM; i++) {
            netdev = rcu_dereference(
                priv->netdev_resources.port_netdev[port_type][comp_info->cqe_params.sysport_lag_id][i]);
            if (netdev) {
                __sx_netdev_handle_global_pkt(netdev, comp_info);
            }
        }

        rcu_read_unlock();
    }

out:
    dev_netdev_priv_put(priv);
}

static int sx_netdev_register_global_event_handler(struct sx_dev *dev)
{
    int                        err = 0;
    union ku_filter_critireas  crit;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return -ENODEV;
    }

    /* Register listener for Ethernet SWID */
    memset(&crit, 0, sizeof(crit));
    err = sx_core_add_synd(ROUTER_PORT_SWID,
                           NUM_HW_SYNDROMES,
                           L2_TYPE_DONT_CARE,
                           NETDEV_DRV_NAME,
                           0,
                           &crit,
                           "NETDEV (Global)",
                           sx_netdev_handle_global_pkt,
                           NULL,
                           CHECK_DUP_DISABLED_E,
                           dev,
                           NULL,
                           1);
    if (err) {
        sxd_log_err(dev, "%s: Failed registering global rx_handler", __func__);
        goto out;
    }
    err = sx_core_add_synd(0,
                           SXD_TRAP_ID_PUDE,
                           L2_TYPE_DONT_CARE,
                           NETDEV_DRV_NAME,
                           0,
                           &crit,
                           "NETDEV (PUDE-event)",
                           sx_netdev_handle_pude_event,
                           NULL,
                           CHECK_DUP_DISABLED_E,
                           dev,
                           NULL,
                           1);
    if (err) {
        sxd_log_err(dev, "%s: Failed registering PUDE event rx_handler", __func__);
        goto out;
    }

/* For Palladium skip event registration which invoke sysfs_notify as sysfs init is skipped */
#if !defined(PD_BU)
    err = sx_core_add_synd(0,
                           SXD_TRAP_ID_PMPE,
                           L2_TYPE_DONT_CARE,
                           NETDEV_DRV_NAME,
                           0,
                           &crit,
                           "NETDEV (PMPE-event)",
                           sx_netdev_handle_pmpe_event,
                           NULL,
                           CHECK_DUP_ENABLED_E,
                           dev,
                           NULL,
                           1);
    if (err) {
        sxd_log_err(dev, "%s: Failed registering PMPE event rx_handler", __func__);
        goto out;
    }
#endif  /* !defined(PD_BU) */

out:
    dev_netdev_priv_put(priv);
    return err;
}

static int sx_netdev_unregister_global_event_handler(struct sx_dev *dev)
{
    int                        err = 0;
    union ku_filter_critireas  crit;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return -ENODEV;
    }

    memset(&crit, 0, sizeof(crit));
    err = sx_core_remove_synd(ROUTER_PORT_SWID, NUM_HW_SYNDROMES, L2_TYPE_DONT_CARE, 0,
                              &crit, NULL, dev, NULL, NULL, 1, NULL);
    if (err) {
        sxd_log_err(dev, "error: Failed de registering on "
                    "0x%x syndrome.\n", NUM_HW_SYNDROMES);
        goto out;
    }

    err = sx_core_remove_synd(0, SXD_TRAP_ID_PUDE, L2_TYPE_DONT_CARE, 0,
                              &crit, NULL, dev, NULL, NULL, 1, NULL);
    if (err) {
        sxd_log_err(dev, "error: Failed de registering on "
                    "0x%x syndrome.\n", SXD_TRAP_ID_PUDE);
        goto out;
    }

/* For Palladium skip event registration which invoke sysfs_notify as sysfs init is skipped */
#if !defined(PD_BU)
    err = sx_core_remove_synd(0, SXD_TRAP_ID_PMPE, L2_TYPE_DONT_CARE, 0,
                              &crit, NULL, dev, NULL, NULL, 1, NULL);
    if (err) {
        sxd_log_err(dev, "error: Failed de registering on "
                    "0x%x syndrome.\n", SXD_TRAP_ID_PMPE);
        goto out;
    }
#endif /* !defined(PD_BU) */

    sx_core_flush_synd_by_handler(dev, sx_netdev_handle_global_pkt);

out:
    dev_netdev_priv_put(priv);
    return err;
}

static int __sx_netdev_uc_type_get_data(enum sx_netdev_user_channel_type uc_type,
                                        union ku_filter_critireas       *crit_p,
                                        cq_handler                      *netdev_callback_p,
                                        char                            *synd_str,
                                        size_t                           synd_str_size)
{
    int                       err = 0;
    union ku_filter_critireas crit;
    cq_handler                netdev_callback = 0;

    memset(&crit, 0, sizeof(crit)); /* sets all ETH values to default */
    switch (uc_type) {
    case USER_CHANNEL_L3_NETDEV:
        netdev_callback = sx_netdev_handle_rx_pkt;
        if (synd_str_size > 0) {
            snprintf(synd_str, synd_str_size - 1, "NETDEV (RX)");
        }
        crit.eth.from_rp = IS_RP_NOT_FROM_RP_E;
        crit.eth.from_bridge = IS_BRIDGE_NOT_FROM_BRIDGE_E;
        break;

    case USER_CHANNEL_LOG_PORT_NETDEV:
        netdev_callback = sx_netdev_log_port_rx_pkt;
        if (synd_str_size > 0) {
            snprintf(synd_str, synd_str_size - 1, "NETDEV (Log-Port-RX)");
        }
        crit.eth.from_rp = IS_RP_DONT_CARE_E;
        crit.eth.from_bridge = IS_BRIDGE_DONT_CARE_E;
        break;

    case USER_CHANNEL_PHY_PORT_NETDEV:
        netdev_callback = sx_netdev_phy_port_rx_pkt;
        if (synd_str_size > 0) {
            snprintf(synd_str, synd_str_size - 1, "NETDEV (Phy-Port-RX)");
        }
        crit.eth.from_rp = IS_RP_DONT_CARE_E;
        crit.eth.from_bridge = IS_BRIDGE_DONT_CARE_E;
        break;

    case USER_CHANNEL_USER_NETDEV:
        netdev_callback = __sx_netdev_handle_rx_pkt_user_netdev;
        if (synd_str_size > 0) {
            snprintf(synd_str, synd_str_size - 1, "NETDEV (User-Netdev-RX)");
        }
        crit.eth.from_rp = IS_RP_DONT_CARE_E;
        crit.eth.from_bridge = IS_BRIDGE_DONT_CARE_E;
        break;

    default:
        sxd_log_err(SXD_DEV_NONE, "unsupported user channel type (%d).\n", uc_type);
        err = -ERANGE;
        goto out;
        break;
    }

    memcpy(crit_p, &crit, sizeof(crit));
    memcpy(netdev_callback_p, &netdev_callback, sizeof(netdev_callback));

out:
    return err;
}


static void sx_timestamp_init_config(struct hwtstamp_config *hwtstamp_config)
{
    hwtstamp_config->tx_type = HWTSTAMP_TX_OFF;
    hwtstamp_config->rx_filter = HWTSTAMP_FILTER_NONE;
}

static int sx_netdev_open(struct net_device *netdev)
{
    int                        err = 0;
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    struct sx_dev             *dev = net_priv->dev;
    union ku_filter_critireas  crit;
    cq_handler                 netdev_callback = 0;
    int                        i;
    u8                         uc_type = 0;
    struct ku_port_vlan_params port_vlan;
    char                       synd_str[SX_LISTENER_HANDLER_STR_MAX_LEN] = { 0 };
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return -ENODEV;
    }

    sxd_log_debug(dev, "%s: called for %s\n", __func__, netdev->name);

    netdev_linkstate_set(netdev);

    for (uc_type = USER_CHANNEL_L3_NETDEV; uc_type < NUM_OF_NET_DEV_TYPE; uc_type++) {
        err = __sx_netdev_uc_type_get_data(uc_type, &crit, &netdev_callback, synd_str, sizeof(synd_str));
        if (err) {
            sxd_log_err(dev, "%s: Failed get crit and handler for "
                        "user channel (%d).\n", __func__, uc_type);
            goto out;
        }

        for (i = 0; i < net_priv->num_of_traps[uc_type]; i++) {
            port_vlan.port_vlan_type = (ku_port_vlan_params_type_t)net_priv->trap_ids[uc_type][i].info_type;
            port_vlan.sysport = net_priv->trap_ids[uc_type][i].sysport;
            port_vlan.lag_id = net_priv->trap_ids[uc_type][i].lag_id;
            port_vlan.vlan = net_priv->trap_ids[uc_type][i].vlan;
            err = sx_core_add_synd(net_priv->swid,
                                   net_priv->trap_ids[uc_type][i].synd,
                                   L2_TYPE_ETH,
                                   NETDEV_DRV_NAME,
                                   0,
                                   &crit,
                                   synd_str,
                                   netdev_callback,
                                   netdev,
                                   CHECK_DUP_DISABLED_E,
                                   net_priv->dev,
                                   &port_vlan,
                                   1);
            if (err) {
                sxd_log_err(dev, "error %s, User channel (%d) failed registering on "
                            "0x%x syndrome.\n", netdev->name, uc_type, net_priv->trap_ids[uc_type][i].synd);
                goto out;
            }
        }
    }

    sx_timestamp_init_config(&net_priv->hwtstamp_config);

#if defined(CONFIG_NET_PORT) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)))
    port_init_ethtool_stats(netdev);
#endif

out:
    sxd_log_debug(dev, "%s: exit for %s\n", __func__, netdev->name);
    dev_netdev_priv_put(priv);

    return err;
}

static int sx_netdev_stop(struct net_device *netdev)
{
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    union ku_filter_critireas  crit;
    cq_handler                 netdev_callback = 0;
    int                        i;
    u8                         uc_type = 0;
    struct ku_port_vlan_params port_vlan;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return -ENODEV;
    }

    sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: called for %s\n", __func__, netdev->name);

    for (uc_type = USER_CHANNEL_L3_NETDEV; uc_type < NUM_OF_NET_DEV_TYPE; uc_type++) {
        if (__sx_netdev_uc_type_get_data(uc_type, &crit, &netdev_callback, NULL, 0)) {
            sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: Failed get crit and handler for "
                        "user channel (%d).\n", __func__, uc_type);
            continue;
        }

        for (i = 0; i < net_priv->num_of_traps[uc_type]; i++) {
            port_vlan.port_vlan_type = (ku_port_vlan_params_type_t)net_priv->trap_ids[uc_type][i].info_type;
            port_vlan.sysport = net_priv->trap_ids[uc_type][i].sysport;
            port_vlan.lag_id = net_priv->trap_ids[uc_type][i].lag_id;
            port_vlan.vlan = net_priv->trap_ids[uc_type][i].vlan;
            sx_core_remove_synd(net_priv->swid,
                                net_priv->trap_ids[uc_type][i].synd,
                                L2_TYPE_ETH, 0, &crit, netdev, net_priv->dev,
                                netdev_callback, &port_vlan, 1, NULL);
        }
    }

    if ((carrier_set_on_pude_disable != CARRIER_SET_BY_USER) || !net_priv->is_port_netdev || net_priv->is_lag) {
        netif_tx_disable(netdev);
        netif_carrier_off(netdev);
    }

    sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: exit for %s\n", __func__, netdev->name);
    dev_netdev_priv_put(priv);

    return 0;
}

static void __sx_netdev_set_lag_oper_state(struct sx_dev *dev, u16 lag_id, u8 oper_state)
{
    struct net_device         *netdev = NULL;
    struct sx_net_priv        *net_priv;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);
    uint16_t                   lag_max = 0, lag_member_max = 0;
    int                        err = 0;
    u8                         i = 0;

    if (!priv) {
        return;
    }

    err = sx_core_get_lag_max(dev, &lag_max, &lag_member_max);
    if (err) {
        sxd_log_err(dev, "Failed to get max number of LAGs. err = %u.\n", err);
        goto out;
    }

    if (lag_id >= lag_max) {
        sxd_log_rl_err(dev, "%s: lag_id %d is invalid. (MAX %d).\n", __func__,
                       lag_id, lag_max);
        goto out;
    }

    sxd_log_debug(dev, "%s: Called for lag_id %d  status %s\n", __func__,
                  lag_id, oper_state ? "UP" : "DOWN");

    mutex_lock(&priv->netdev_resources.rsc_lock);

    /* Change port netdev status */
    for (i = 0; i < MAX_PORT_NETDEV_NUM; i++) {
        netdev = priv->netdev_resources.port_netdev[PORT_TYPE_LAG][lag_id][i];
        if (netdev) {
            net_priv = netdev_priv(netdev);
            net_priv->is_oper_state_up = oper_state;
            netdev_linkstate_set(netdev);
        }
    }

    mutex_unlock(&priv->netdev_resources.rsc_lock);

out:
    dev_netdev_priv_put(priv);
}

static int sx_netdev_get_port_prio_to_tc(struct net_device *netdev, struct sk_buff *skb, uint8_t *tc)
{
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    uint8_t                    pcp;
    u16                        vlan_tci;
    struct vlan_ethhdr        *veth = NULL;
    int                        err = 0;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return -ENODEV;
    }

    *tc = 0;

    veth = (struct vlan_ethhdr *)(skb->data);
    if (ntohs(veth->h_vlan_proto) != ETH_P_8021Q) {
        /* We can reach this case if pkt was sent from phys. port netdev */
        if (__netdev_tx_debug(net_priv->dev)) {
            sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "[tx_debug] VETH proto (0x%x) is not ETH_P_8021Q !\n",
                          veth->h_vlan_proto);
        }
        *tc = SX_PACKET_DEFAULT_TC;
        goto out;
    }

    vlan_tci = ntohs(veth->h_vlan_TCI);
    pcp = (vlan_tci & SX_VLAN_PRIO_MASK) >> SX_VLAN_PRIO_SHIFT;

    err = sx_core_get_prio2tc(net_priv->dev, net_priv->port, net_priv->is_lag, pcp, tc);

out:
    dev_netdev_priv_put(priv);
    return err;
}

static int sx_netdev_set_vlan_according_hw(struct net_device *netdev, struct sk_buff *skb)
{
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    uint16_t                   vlan;
    uint8_t                    is_tagged, is_prio_tagged;
    struct vlan_ethhdr        *veth = NULL;
    int                        err = 0;
    unsigned long              last_bit = 0;
    u8                         vlan_child_exist = 0;
    int                        tx_debug = __netdev_tx_debug(net_priv->dev);
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return -ENODEV;
    }

    veth = (struct vlan_ethhdr *)(skb->data);
    if (ntohs(veth->h_vlan_proto) != ETH_P_8021Q) {
        /* We can reach this case if packet was sent from port netdev */
        if (tx_debug) {
            sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "[tx_debug] %s: No vlan header.\n", __func__);
        }
        goto out;
    }

    last_bit = find_last_bit(net_priv->active_vlans, VLAN_N_VID);
    vlan_child_exist = last_bit < VLAN_N_VID && last_bit != 0;

    /* We apply vlan tagging logic only if netdev has vlan child */
    if (!vlan_child_exist) {
        if (tx_debug) {
            sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(net_priv),
                          "[tx_debug] %s: netdev (%s) No vlan child.\n",
                          __func__,
                          netdev->name);
        }
        goto out;
    }

    vlan = ntohs(veth->h_vlan_TCI) & VLAN_VID_MASK;
    if (vlan > 4094) {
        sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: VETH vlan (%d) is very big. MAX: %d.\n",
                    __func__, vlan, 4094);
        err = -EINVAL;
        goto out;
    }

    err = sx_core_get_vlan_tagging(net_priv->dev, net_priv->port, net_priv->is_lag, vlan, &is_tagged);
    if (err) {
        sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "sx_core_get_vlan_tagging is failed. err: %d\n", err);
        goto out;
    }

    err = sx_core_get_prio_tagging(net_priv->dev, net_priv->port, net_priv->is_lag, &is_prio_tagged);
    if (err) {
        sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "sx_core_get_prio_tagging is failed. err: %d\n", err);
        goto out;
    }

    if (tx_debug) {
        sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(net_priv),
                      "[tx_debug] is_lag:%d, port:%d, vlan (from header):%d port is_tagged:%d, port is_prio_tag:%d.\n",
                      net_priv->is_lag,
                      net_priv->port,
                      vlan,
                      is_tagged,
                      is_prio_tagged);
    }

    /*
     * If port is VLAN tagged than send the packet with TAG
     * Else if port is Prio-tagged send the packet with Prio tag (zero the VLAN id)
     * Else send packet untagged
     */
    if (!is_tagged && is_prio_tagged) {
        /* port is PRIO tagged (but not VLAN tagged): send the packet prio tagged */
        veth->h_vlan_TCI &= ~(htons(VLAN_VID_MASK));
        if (tx_debug) {
            sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(net_priv),
                          "[tx_debug] Port is not vlan tag but prio tag, clear the vlan: vlan_tci:0x%x.\n",
                          veth->h_vlan_TCI);
        }
    } else if (!is_tagged && !is_prio_tagged) {
        if (tx_debug) {
            sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(net_priv),
                          "[tx_debug] Port is not vtag and not prio tag, remove vlan header.\n");
        }
        /* send the packet untagged */
        sx_netdev_remove_vlan_header(skb);
    } else {
        /* else port is VLAN tagged and send the packet as is */
        if (tx_debug) {
            sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "[tx_debug] Port is vlan tagged. do nothing.\n");
        }
    }

out:
    dev_netdev_priv_put(priv);
    return err;
}


static int sx_netdev_override_icmp_ip(struct net_device *netdev, struct sk_buff *skb)
{
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    uint16_t                   vlan;
    struct vlan_ethhdr        *veth = NULL;
    int                        err = 0;
    uint32_t                   ip_addr;
    struct iphdr             * iph = NULL;
    struct icmphdr           * icmph = NULL;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return -ENODEV;
    }

    veth = (struct vlan_ethhdr *)(skb->data);
    if (ntohs(veth->h_vlan_proto) != ETH_P_8021Q) {
        /* We can reach this case if pkt was sent from phys. port netdev */
        if (__netdev_tx_debug(net_priv->dev)) {
            sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "[tx_debug] VETH proto (0x%x) is not ETH_P_8021Q !\n",
                          veth->h_vlan_proto);
        }
        goto out;
    }

    /* is packet icmp type 5 */

    vlan = ntohs(veth->h_vlan_TCI) & VLAN_VID_MASK;
    if (vlan > 4094) {
        sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "VETH vlan (%d) is very big . MAX: %d !\n",
                    vlan, 4094);
        err = -EINVAL;
        goto out;
    }

    err = sx_core_get_vlan2ip(net_priv->dev, vlan, &ip_addr);
    if (err) {
        sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "sx_core_get_vlan2ip is failed. err: %d\n", err);
        goto out;
    }

    if (ip_addr == 0) {
        goto out;
    }

    iph = (struct iphdr*)(skb->data + sizeof(struct vlan_ethhdr));
    if (iph->protocol != IPPROTO_ICMP) {
        goto out;
    }

    icmph = (struct icmphdr *)skb_transport_header(skb);
    if (icmph->type != ICMP_REDIRECT) {
        goto out;
    }

    /* override SIP with DB IP */
    iph->saddr = htonl(ip_addr);

    /* recalc checksum */
    ip_send_check(iph);

out:
    dev_netdev_priv_put(priv);
    return err;
}


/* change skb to hold 'control' traffic (rather than 'data' traffic) */
static int __set_as_control_traffic(struct sk_buff *skb, struct net_device *netdev, struct isx_meta *meta)
{
    struct sx_net_priv *net_priv = netdev_priv(netdev);
    uint8_t             tc;
    int                 err;

    meta->system_port_mid = net_priv->is_lag ? net_priv->mid : net_priv->port;
    meta->type = SX_PKT_TYPE_ETH_CTL_UC;

    /* Set etclass
     * SX: get tc from pcp.
     * SPC: etclass 0 (overwritten in post_send) */
    err = sx_netdev_get_port_prio_to_tc(netdev, skb, &tc);
    if (err) {
        sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: Err: can't get prio to tc\n", __func__);
        goto error;
    }

    meta->etclass = tc; /* High priority */
    err = sx_netdev_set_vlan_according_hw(netdev, skb);
    if (err) {
        sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: Err: can't set vlan according hw.\n", __func__);
        goto error;
    }

    return 0;

error:

    net_priv->stats.tx_dropped++;
    kfree_skb(skb); /* drop packet flow, use kfree_skb */
    return err;
}

static int __is_ifc_router_port(struct sk_buff    *skb,
                                struct net_device *netdev,
                                u8                *is_ifc_router_port,
                                uint16_t          *ifc_vlan)
{
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    int                        err = 0;
    uint16_t                   vlan = 0;
    u8                         is_from_rp = 0;
    struct vlan_ethhdr        *veth = NULL;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return -ENODEV;
    }

    if ((skb == NULL) || (netdev == NULL)) {
        err = -EINVAL;
        sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: skb (%p) or netdev (%p) is NULL\n",
                    __func__, skb, netdev);
        goto error;
    }

    veth = (struct vlan_ethhdr *)(skb->data);
    if (ntohs(veth->h_vlan_proto) == ETH_P_8021Q) {
        vlan = ntohs(veth->h_vlan_TCI) & VLAN_VID_MASK;
        if (vlan > 4094) {
            err = -EINVAL;
            sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: VETH vlan (%d) is very big . MAX: %d.\n",
                        __func__, vlan, 4094);
            goto error;
        }
    }
    if (vlan == 0) {
        /* packet is untagged or prio tagged */
        err = sx_core_get_pvid(net_priv->dev, net_priv->port, net_priv->is_lag, &vlan);
        if (err) {
            sxd_log_rl_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: can't get pvid for port\n", __func__);
            goto error;
        }
    }

    err = sx_core_get_rp_mode(net_priv->dev, net_priv->is_lag, net_priv->port, vlan, &is_from_rp);
    if (err) {
        sxd_log_rl_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: Can't get rp mode for port\n", __func__);
        goto error;
    }

    if (is_ifc_router_port != NULL) {
        *is_ifc_router_port = is_from_rp;
    }

    if (ifc_vlan != NULL) {
        *ifc_vlan = vlan;
    }

    dev_netdev_priv_put(priv);
    return err;

error:
    kfree_skb(skb); /* drop packet flow, use kfree_skb */
    net_priv->stats.tx_dropped++;
    dev_netdev_priv_put(priv);
    return err;
}

static int __send_to_port_as_data(struct sk_buff *skb, struct net_device *netdev, struct isx_meta *meta, uint16_t vlan)
{
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    int                        err = 0;
    uint16_t                   hw_efid = 0;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return -ENODEV;
    }

    meta->type = SX_PKT_TYPE_ETH_DATA;
    meta->rx_is_router = 1;
    meta->fid_valid = 1;
    err = sx_core_get_rp_rfid(net_priv->dev, net_priv->port, net_priv->is_lag, vlan, &hw_efid);
    if (err) {
        sxd_log_rl_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: Can't get rp fid\n", __func__);
        goto error;
    }
    meta->fid = hw_efid;

    dev_netdev_priv_put(priv);
    return 0;

error:
    net_priv->stats.tx_dropped++;
    kfree_skb(skb); /* drop packet flow, use kfree_skb */
    dev_netdev_priv_put(priv);
    return err;
}

struct ptp_tx_skb_info {
    bool         valid;
    u8           shinfo_tx_flags;
    struct sock *skb_sk;
    char         skb_cb[sizeof(((struct sk_buff*)NULL)->cb)];
};

static int sx_netdev_hard_start_xmit(struct sk_buff *skb, struct net_device *netdev)
{
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    int                        err = 0;
    int                        len = 0;
    struct isx_meta            meta;
    struct sk_buff            *tmp_skb = NULL;
    struct vlan_ethhdr        *veth = NULL;
    u16                        pcp = 0;
    uint16_t                   ifc_vlan = 0;
    u8                         is_ifc_rp = 0;
    struct ptp_tx_skb_info     ptp_skb_info = {
        .valid = false
    };
    u8                         is_tagged;
    bool                       is_ptp_pkt = false;
    int                        tx_debug = __netdev_tx_debug(net_priv->dev);
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return -ENODEV;
    }

    if (net_priv->dev == NULL) {
        sxd_log_rl_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "sx_netdev_hard_start_"
                       "xmit: netdev %s net_priv->dev is NULL.\n", netdev->name);

        net_priv->stats.tx_dropped++;
        kfree_skb(skb); /* drop packet flow, use kfree_skb */
        err = -ENXIO;
        goto out;
    }

    veth = (struct vlan_ethhdr *)(skb->data);
    is_tagged = (ntohs(veth->h_vlan_proto) == ETH_P_8021Q);

    sx_netdev_override_icmp_ip(netdev, skb);

    if (tx_debug) {
        sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s called for (%s)\n", __func__, netdev->name);
    }

    netif_trans_update(netdev); /* save the timestamp */

    memset(&meta, 0, sizeof(meta));
    meta.dev_id = net_priv->dev->device_id;
    meta.lp = 0;
    meta.rdq = 0;
    meta.swid = net_priv->swid;
    meta.to_cpu = 0;

    if (net_priv->is_bridge) {
        /*
         * Bridge: send as data
         */
        meta.type = SX_PKT_TYPE_ETH_DATA;
        meta.rx_is_router = 0;
        meta.fid_valid = 1;
        meta.fid = net_priv->bridge_id;
    } else if (!net_priv->is_port_netdev) {
        /*
         * swid0_eth: send as data
         */
        meta.type = SX_PKT_TYPE_ETH_DATA;
        meta.system_port_mid = 0;
    } else if (!net_priv->send_to_port_as_data_supported) {
        /*
         * SX port: send as control
         */
        err = __set_as_control_traffic(skb, netdev, &meta);
        if (err) {
            goto out;
        }
    } else { /* net_priv->send_to_port_as_data_supported == true */
             /*
              * Spc port
              */
        if (net_priv->pipeline == SX_NETDEV_PIPELINE_ADAPTIVE) {
            err = __is_ifc_router_port(skb, netdev, &is_ifc_rp, &ifc_vlan);
            if (err) {
                goto out;
            }
        }
        /*
         * If Port or LAG is router port and netdev pipeline is adaptive, then send as data
         * else send as Control
         */
        if (is_ifc_rp) {
            err = __send_to_port_as_data(skb, netdev, &meta, ifc_vlan);
        } else {
            err = __set_as_control_traffic(skb, netdev, &meta);
        }
        if (err) {
            goto out;
        }
    }

    if (meta.type == SX_PKT_TYPE_ETH_DATA) {
        meta.rx_is_tunnel = net_priv->skip_tunnel;
    }

    if (net_priv->hwtstamp_config.tx_type != HWTSTAMP_TX_OFF) {
        err = sx_core_ptp_tx_handler(net_priv->dev,
                                     skb,
                                     net_priv->port,
                                     net_priv->is_lag,
                                     &is_ptp_pkt);
        if (err) {
            sxd_log_rl_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "failed to handle PTP TX packet (err=%d)\n", err);
        } else if (is_ptp_pkt) {
            /* we should save these local parameters in case skb will be replaced in this function (a VLAN
             * header should be added or not enough headroom */
            ptp_skb_info.valid = true;
            ptp_skb_info.shinfo_tx_flags = skb_shinfo(skb)->tx_flags;
            ptp_skb_info.skb_sk = skb->sk;
            memcpy(ptp_skb_info.skb_cb, skb->cb, sizeof(ptp_skb_info.skb_cb));

            /* *****************************************/
            /* Apply SPC2/3 PTP WA                     */

            err = sx_core_ptp_tx_control_to_data(net_priv->dev,
                                                 &skb,
                                                 &meta,
                                                 net_priv->port,
                                                 net_priv->is_lag,
                                                 &is_tagged);
            if (err) {
                kfree_skb(skb); /* drop packet flow, use kfree_skb */
                goto out;
            }
        }
    }

    /* *****************************************/
    if (tx_debug) {
        sxd_log_info(SXD_DEV_FROM_NETDEV_PRIV(net_priv),
                     "is_tagged:%d , skb->tx_flags:0x%x, skb->priority:%d meta.type:%d ,is_ifc_rp:%d, ifc_vlan: %d\n",
                     is_tagged,
                     skb_shinfo(skb)->tx_flags,
                     skb->priority,
                     meta.type,
                     is_ifc_rp,
                     ifc_vlan);
    }

    if (!is_tagged && (((skb->priority != 0) && (meta.type == SX_PKT_TYPE_ETH_DATA)) || is_ifc_rp)) {
        /* If it is a DATA packet and it is untagged, add prio tag (PCP according to skb->priority) */
        if (skb->priority > SX_VLAN_PRIO_MAX) {
            if (tx_debug) {
                sxd_log_info(SXD_DEV_FROM_NETDEV_PRIV(net_priv),
                             "%s: skb priority is to big. set max pcp %d\n",
                             __func__,
                             SX_VLAN_PRIO_MAX);
            }
            pcp = SX_VLAN_PRIO_MAX;
        } else {
            pcp = skb->priority;
        }

        err = sx_core_skb_add_vlan(&skb, ifc_vlan, pcp); /* skb may change pointer here ... */
        if (err) {
            sxd_log_rl_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: Fail to alloc skb with vlan header\n",
                           __func__);


            kfree_skb(skb); /* drop packet flow, use kfree_skb */
            net_priv->stats.tx_dropped++;
            goto out;
        }

        is_tagged = 1;
    }

    /* If there's no place for the ISX header
     * need to alloc a new skb and use it instead */
    tmp_skb = skb;
    len = skb->len;
    if (skb_headroom(skb) < ISX_HDR_SIZE) {
        if (tx_debug) {
            sxd_log_info(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "sx_netdev_hard_start_xmit: "
                         "No place for the ISX header, allocating a new "
                         "SKB\n");
        }
        tmp_skb = alloc_skb(ISX_HDR_SIZE + len, GFP_ATOMIC);
        if (!tmp_skb) {
            sxd_log_rl_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "sx_netdev_hard_start_"
                           "xmit: Err: failed allocating "
                           "SKB\n");

            net_priv->stats.tx_dropped++;
            kfree_skb(skb); /* drop packet flow, use kfree_skb */
            err = -ENOMEM;
            goto out;
        }

        skb_reserve(tmp_skb, ISX_HDR_SIZE);
        memcpy(skb_put(tmp_skb, len), skb->data, len);
        consume_skb(skb);   /* free unused skb, use consume_skb */
        len += ISX_HDR_SIZE;
    }

    if (ptp_skb_info.valid) {
        /* need to override these parameters with the ones we saved in this function. skb may have replaced
         * (allocated again) and these old parameters does not exist there anymore */
        skb_shinfo(tmp_skb)->tx_flags = ptp_skb_info.shinfo_tx_flags;
        tmp_skb->sk = ptp_skb_info.skb_sk;
        memcpy(tmp_skb->cb, ptp_skb_info.skb_cb, sizeof(skb->cb));
    }

    err = sx_core_post_send(net_priv->dev, tmp_skb, &meta);
    if (err) {
        if (err == -ENXIO) { /* sx_core interface was not available */
            kfree_skb(tmp_skb); /* drop packet flow, use kfree_skb */
        }

        /*
         *  We don't free the packet because sx_core_post_send free the packet in case of an error.
         *  We return NETDEV_TX_OK in case of ERROR because a nonzero return value indicates that the packet
         *  could not be transmitted at this time and the kernel will retry later.
         */


        sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "error %s, Failed sending packet\n", netdev->name);
        net_priv->stats.tx_dropped++;
    } else {
        net_priv->stats.tx_bytes += len;
        net_priv->stats.tx_packets++;
    }

    err = NETDEV_TX_OK;

out:
    dev_netdev_priv_put(priv);
    return err;
}

static struct net_device_stats * sx_netdev_get_stats(struct net_device *netdev)
{
    struct sx_net_priv *net_priv = netdev_priv(netdev);

    return &net_priv->stats;
}

static void sx_netdev_set_multicast(struct net_device *netdev)
{
    return;
}

static int sx_netdev_set_mac(struct net_device *netdev, void *p)
{
    struct sx_net_priv *net_priv = netdev_priv(netdev);
    struct sockaddr    *addr = p;

    if (!is_valid_ether_addr(addr->sa_data)) {
        sxd_log_info(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "address not given.");
        return -EADDRNOTAVAIL;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0))
    eth_hw_addr_set(netdev, addr->sa_data);
#else
    memcpy(netdev->dev_addr, addr->sa_data, netdev->addr_len);
#endif
    net_priv->mac = sx_netdev_mac_to_u64(addr->sa_data);

    return 0;
}

#if defined(CONFIG_NET_SWITCHDEV) && \
    (LINUX_VERSION_CODE < KERNEL_VERSION(5, 1, 0)) && !defined(CONFIG_SX_RHEL_8_6) && !defined(CONFIG_SX_CENTOS_8_4)
/* SWITCHDEV_ATTR_ID_PORT_PARENT_ID removed in kernel 5.1.0 */
int sx_netdev_attr_get(struct net_device *netdev, struct switchdev_attr *attr)
{
    struct sx_dev_netdev_priv* priv = dev_netdev_priv_from_netdev(netdev);
    u8                         sx_ppid[ETH_ALEN];
    int                        ret = 0;

    /*if we get here during PCI_RESTART when the netdevs are still up*/
    if (!priv) {
        return -EOPNOTSUPP;
    }

    memset(sx_ppid, 0xff, ETH_ALEN);
    sx_ppid[5] = priv->dev->device_id;

    switch (attr->id) {
    case SWITCHDEV_ATTR_ID_PORT_PARENT_ID:
        attr->u.ppid.id_len = ETH_ALEN;
        /* The attr->u.ppid.id is needed to distinguish between port that belong
         *  to SW1 and port that belong to SW2 on multi-chip platforms.
         *  Our platform is single switch so FF..FF id will be used */
        memcpy(attr->u.ppid.id, sx_ppid, attr->u.ppid.id_len);
        break;

    default:
        ret = -EOPNOTSUPP;
        break;
    }

    dev_netdev_priv_put(priv);
    return ret;
}

static const struct switchdev_ops sx_netdev_switchdev_ops = {
    .switchdev_port_attr_get = sx_netdev_attr_get,
};
#endif

#if defined(CONFIG_NET_SWITCHDEV) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)) || defined(CONFIG_SX_RHEL_8_6))
/* ndo_get_port_parent_id() replaced SWITCHDEV_ATTR_ID_PORT_PARENT_ID in kernel 5.1.0 */
static int __sx_netdev_get_port_parent_id(struct net_device *dev, struct netdev_phys_item_id *ppid)
{
    struct sx_dev_netdev_priv* priv = dev_netdev_priv_from_netdev(dev);
    u8                         sx_ppid[ETH_ALEN];

    /*if we get here during PCI_RESTART when the netdevs are still up*/
    if (!priv) {
        return -EOPNOTSUPP;
    }

    memset(sx_ppid, 0xff, ETH_ALEN);
    sx_ppid[5] = priv->dev->device_id;

    ppid->id_len = ETH_ALEN;

    /* The ppid id is needed to distinguish between port that belong
     *  to SW1 and port that belong to SW2 on multi-chip platforms.
     *  Our platform is single switch so FF..FF id will be used */
    memcpy(ppid->id, sx_ppid, ppid->id_len);

    dev_netdev_priv_put(priv);
    return 0;
}
#endif

static int sx_netdev_change_mtu(struct net_device *netdev, int new_mtu)
{
    if ((new_mtu < ETH_ZLEN) || (new_mtu > MAX_JUMBO_FRAME_SIZE)) {
        return -EINVAL;
    }

    netdev->mtu = new_mtu;

    return 0;
}

static int sx_hwstamp_set(struct net_device *dev, struct ifreq *ifr)
{
    struct sx_net_priv        *net_priv = netdev_priv(dev);
    struct hwtstamp_config     config;
    int                        err = 0;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(dev);

    if (!priv) {
        return -ENODEV;
    }

    if (copy_from_user(&config, ifr->ifr_data, sizeof(config))) {
        err = -EFAULT;
        goto out;
    }

    /* TX HW timestamp */
    err = sx_core_ptp_set_tx_type(net_priv->dev,
                                  config.tx_type);
    if (err) {
        goto out;
    }

    /* RX HW timestamp */
    switch (config.rx_filter) {
    case HWTSTAMP_FILTER_NONE:
        break;

    case HWTSTAMP_FILTER_ALL:
    case HWTSTAMP_FILTER_SOME:
    case HWTSTAMP_FILTER_PTP_V1_L4_EVENT:
    case HWTSTAMP_FILTER_PTP_V1_L4_SYNC:
    case HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ:
    case HWTSTAMP_FILTER_PTP_V2_L4_EVENT:
    case HWTSTAMP_FILTER_PTP_V2_L4_SYNC:
    case HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ:
    case HWTSTAMP_FILTER_PTP_V2_L2_EVENT:
    case HWTSTAMP_FILTER_PTP_V2_L2_SYNC:
    case HWTSTAMP_FILTER_PTP_V2_L2_DELAY_REQ:
    case HWTSTAMP_FILTER_PTP_V2_EVENT:
    case HWTSTAMP_FILTER_PTP_V2_SYNC:
    case HWTSTAMP_FILTER_PTP_V2_DELAY_REQ:
        config.rx_filter = HWTSTAMP_FILTER_ALL;
        break;

    default:
        err = -ERANGE;
        goto out;
    }

    memcpy(&net_priv->hwtstamp_config, &config, sizeof(config));

    err = copy_to_user(ifr->ifr_data, &config,
                       sizeof(config)) ? -EFAULT : 0;

out:
    dev_netdev_priv_put(priv);
    return err;
}

static int sx_netdev_do_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
    switch (cmd) {
    case SIOCSHWTSTAMP:
        return sx_hwstamp_set(netdev, ifr);

    default:
        return -EOPNOTSUPP;
    }
}

static int sx_netdev_vlan_rx_add_vid(struct net_device *netdev, __be16 proto, u16 vid)
{
    struct sx_net_priv *net_priv = netdev_priv(netdev);

    set_bit(vid, net_priv->active_vlans);

    if (__netdev_rx_debug(net_priv->dev)) {
        sxd_log_info(SXD_DEV_FROM_NETDEV_PRIV(net_priv),
                     "[rx_debug] adding VLAN:%d on netdev %s\n",
                     vid,
                     netdev->name);
    }
    return 0;
}

static int sx_netdev_vlan_rx_kill_vid(struct net_device *netdev, __be16 proto, u16 vid)
{
    struct sx_net_priv *net_priv = netdev_priv(netdev);

    if (__netdev_rx_debug(net_priv->dev)) {
        sxd_log_info(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "[rx_debug] Killing VID:%d netdev %s\n", vid,
                     netdev->name);
    }
    clear_bit(vid, net_priv->active_vlans);
    return 0;
}

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) && \
    (LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)) &&   \
    !defined(CONFIG_SX_RHEL_9_4)) ||                     \
    defined(CONFIG_SX_RHEL_8_6)
static int sx_change_proto_down(struct net_device *dev, bool proto_down)
{
    if (change_proto_down_enable) {
        dev->proto_down = proto_down;
    }

    return 0;
}
#endif

static bool __is_local_port_bound_to_swid(struct sx_dev *dev, u16 local_port)
{
    struct ku_access_pspa_reg  pspa_reg_data;
    int                        err = 0;
    bool                       ret = false;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return -ENODEV;
    }

    if (!dev || (local_port == 0)) {
        goto out;
    }

    memset(&pspa_reg_data, 0, sizeof(pspa_reg_data));

    sx_cmd_set_op_tlv(&pspa_reg_data.op_tlv, PSPA_REG_ID, EMAD_METHOD_QUERY);
    pspa_reg_data.dev_id = dev->device_id;
    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(pspa_reg_data.pspa_reg.local_port,
                                        pspa_reg_data.pspa_reg.lp_msb,
                                        local_port);
    err = sx_ACCESS_REG_PSPA(dev, &pspa_reg_data);
    if (err) {
        sxd_log_err(dev, "failed to get local_port <--> swid binding on local port %u (err=%d)\n", local_port, ret);
        goto out;
    }

    ret = pspa_reg_data.pspa_reg.swid == SWID_NUM_DONT_CARE ? false : true;

out:
    dev_netdev_priv_put(priv);
    return ret;
}

/* Assume log_port is a system port (not LAG) */
int sx_netdev_oper_state_get(struct sx_dev *dev, u32 log_port, u8 *is_oper_state_up_p)
{
    int                        err = 0;
    struct ku_access_paos_reg  paos_reg_data;
    u16                        local_port = SX_PORT_PHY_ID_GET(log_port);
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return -ENODEV;
    }

    if (!__is_local_port_bound_to_swid(dev, local_port)) {
        err = -EINVAL;
        goto out;
    }

    memset(&paos_reg_data, 0, sizeof(struct ku_access_paos_reg));

    sx_cmd_set_op_tlv(&paos_reg_data.op_tlv, MLXSW_PAOS_ID, EMAD_METHOD_QUERY);
    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(paos_reg_data.paos_reg.local_port,
                                        paos_reg_data.paos_reg.lp_msb,
                                        local_port);
    paos_reg_data.dev_id = dev->device_id;

    err = sx_ACCESS_REG_PAOS(dev, &paos_reg_data);
    if (err) {
        sxd_log_info(dev,
                     "%s: Unable to get port state, port = 0x%x err = %d\n",
                     __func__,
                     log_port,
                     err);
        paos_reg_data.paos_reg.oper_status = 0;
    }
    sxd_log_info(dev,
                 "%s: Port state for 0x%x = %d\n",
                 __func__,
                 log_port,
                 paos_reg_data.paos_reg.oper_status);
    *is_oper_state_up_p = (paos_reg_data.paos_reg.oper_status == SXD_PAOS_OPER_STATUS_UP_E);

out:
    dev_netdev_priv_put(priv);
    return err;
}

struct set_carrier_set_on_pude_disable_wdata {
    const char               * val;
    const struct kernel_param* kp;
    int                        ret_code;
    struct completion          wq_completion;
    struct work_struct         work;
    struct sx_dev_netdev_priv *priv;
};

static void __set_carrier_set_on_pude_disable_dwork_handler(struct work_struct *work_p)
{
    struct set_carrier_set_on_pude_disable_wdata *wdata_p =
        container_of(work_p, struct set_carrier_set_on_pude_disable_wdata, work);
    const char               * val = wdata_p->val;
    const struct kernel_param* kp = wdata_p->kp;
    int                        old_val = carrier_set_on_pude_disable;
    int                        err;
    int                        port;
    int                        i;
    struct net_device        * netdev;
    struct sx_net_priv        *net_priv;
    struct sx_dev_netdev_priv *priv = wdata_p->priv;

    /* carrier_set_on_pude_disable is set here. */
    err = param_set_int(val, kp);
    if (err) {
        wdata_p->ret_code = err;
        goto out;
    }

    /* When carrier_set_on_pude_disable is being changed from true to false,
     *  we need to set carrier for all port netdevs, in case new PUDEs
     *  are missed. Since the PUDE handler and this handler are executed
     *  in the same single thread workqueue, there won't be race.
     */
    if (old_val && !carrier_set_on_pude_disable) {
        mutex_lock(&priv->netdev_resources.rsc_lock);
        for (port = 0; port < MAX_SYSPORT_NUM; port++) {
            for (i = 0; i < MAX_PORT_NETDEV_NUM; i++) {
                netdev = priv->netdev_resources.port_netdev[PORT_TYPE_SINGLE][port][i];
                if (netdev == NULL) {
                    continue;
                }
                net_priv = netdev_priv(netdev);
                /* Retrieve the current operational state for this port, as it may not have been obtained during newlink */
                err = sx_netdev_oper_state_get(net_priv->dev, net_priv->log_port, &net_priv->is_oper_state_up);
                if (err) {
                    sxd_log_info(SXD_DEV_FROM_NETDEV_PRIV(net_priv),
                                 "%s: Unable to get port state, log_port = 0x%x, err = %d\n",
                                 __func__,
                                 net_priv->log_port,
                                 err);
                    net_priv->is_oper_state_up = 0;
                }
                if (net_priv->is_oper_state_up) {
                    netif_carrier_on(netdev);
                    netif_start_queue(netdev);
                } else {
                    netif_tx_disable(netdev);
                    netif_carrier_off(netdev);
                }
            }
        }
        mutex_unlock(&priv->netdev_resources.rsc_lock);
    }

out:
    complete(&wdata_p->wq_completion);
}

static int set_carrier_set_on_pude_disable(const char *val, const struct kernel_param *kp)
{
    struct set_carrier_set_on_pude_disable_wdata wdata;
    int                                          ret = 0;
    struct sx_dev_netdev_priv                   *priv = dev_netdev_priv_from_curr_net_namespace();
    int                                          n = 0;

    ret = kstrtoint(val, 0, &n);
    if (ret) {
        goto out;
    }

    switch (n) {
    case CARRIER_SET_ON_PUDE_ADMIN_UP_DOWN:
    case CARRIER_SET_ON_ADMIN_DOWN:
    case CARRIER_SET_BY_USER:
        break;

    default:
        ret = -EINVAL;
        goto out;
    }

    /* If the user specifies this parameter while loading the kernel module, we just set
     *  the parameter directly without pushing the job into workqueue, since the module parameter
     *  set callback is executed prior to module init function in such scenario, the workqueue
     *  and other stuff are created in module init function.
     */
    if (!priv) {
        ret = param_set_int(val, kp);
    } else {
        memset(&wdata, 0, sizeof(wdata));
        wdata.val = val;
        wdata.kp = kp;
        wdata.ret_code = 0;
        wdata.priv = priv;

        init_completion(&wdata.wq_completion);
        INIT_WORK(&wdata.work, __set_carrier_set_on_pude_disable_dwork_handler);
        ret = sx_core_sysfs_dev_hold(priv->dev);
        if (ret == 0) {
            queue_work(priv->netdev_wq, &wdata.work);
            do {
                ret = wait_for_completion_interruptible(&wdata.wq_completion);
            } while (ret == -ERESTARTSYS);
            sx_core_sysfs_dev_release(priv->dev);
            ret = wdata.ret_code;
        }
    }

out:
    if (priv) {
        dev_netdev_priv_put(priv);
    }
    return ret;
}

struct change_carrier_wdata {
    struct net_device *net_dev;
    bool               new_carrier;
    int                ret_code;
    struct completion  wq_completion;
    struct work_struct work;
};

static void __sx_netdev_change_carrier_dwork_handler(struct work_struct *work_p)
{
    struct change_carrier_wdata *wdata_p = container_of(work_p, struct change_carrier_wdata, work);
    struct net_device          * netdev = wdata_p->net_dev;
    struct sx_net_priv         * net_priv = netdev_priv(netdev);
    bool                         new_carrier = wdata_p->new_carrier;

    if (!carrier_set_on_pude_disable || !net_priv->is_port_netdev || net_priv->is_lag) {
        wdata_p->ret_code = -EPERM;
        goto out;
    }

    if (new_carrier) {
        netif_carrier_on(netdev);
        netif_start_queue(netdev);
    } else {
        netif_tx_disable(netdev);
        netif_carrier_off(netdev);
    }

out:
    complete(&wdata_p->wq_completion);
}

static int sx_netdev_change_carrier(struct net_device *netdev, bool new_carrier)
{
    struct change_carrier_wdata wdata;
    struct sx_dev_netdev_priv  *priv = dev_netdev_priv_from_netdev(netdev);
    int                         ret = 0;

    if (!priv) {
        return -ENODEV;
    }

    memset(&wdata, 0, sizeof(wdata));
    wdata.net_dev = netdev;
    wdata.new_carrier = new_carrier;
    wdata.ret_code = 0;

    init_completion(&wdata.wq_completion);
    INIT_WORK(&wdata.work, __sx_netdev_change_carrier_dwork_handler);
    queue_work(priv->netdev_wq, &wdata.work);
    do {
        ret = wait_for_completion_interruptible(&wdata.wq_completion);
    } while (ret == -ERESTARTSYS);

    dev_netdev_priv_put(priv);
    return wdata.ret_code;
}

static const struct net_device_ops sx_netdev_ops = {
    .ndo_open = sx_netdev_open,
    .ndo_stop = sx_netdev_stop,
    .ndo_start_xmit = sx_netdev_hard_start_xmit,
    .ndo_set_mac_address = sx_netdev_set_mac,
    .ndo_set_rx_mode = sx_netdev_set_multicast,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
    .ndo_eth_ioctl = sx_netdev_do_ioctl,
#else
    .ndo_do_ioctl = sx_netdev_do_ioctl,
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) */
    .ndo_change_carrier = sx_netdev_change_carrier,
    .ndo_change_mtu = sx_netdev_change_mtu,
    .ndo_get_stats = sx_netdev_get_stats,
    .ndo_vlan_rx_add_vid = sx_netdev_vlan_rx_add_vid,
    .ndo_vlan_rx_kill_vid = sx_netdev_vlan_rx_kill_vid,
    .ndo_tx_timeout = NULL, /* Disable the transmit timeout function.  */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) || defined(CONFIG_SX_RHEL_8_6)
#if defined(CONFIG_NET_PORT)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)) || defined(CONFIG_SX_RHEL_8_6)
    .ndo_setup_tc = sx_psample_setup_tc,
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)) */
    .ndo_get_stats64 = sx_get_stats64,
#endif /* if defined(CONFIG_NET_PORT) */
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) && \
    (LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)) &&   \
    !defined(CONFIG_SX_RHEL_9_4)) ||                     \
    defined(CONFIG_SX_RHEL_8_6)
    .ndo_change_proto_down = sx_change_proto_down,
#endif /* ((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0))) || defined(CONFIG_SX_RHEL_8_6)*/
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) || defined(CONFIG_SX_RHEL_8_6) */
#if defined(CONFIG_NET_SWITCHDEV) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)) || defined(CONFIG_SX_RHEL_8_6))
    /* ndo_get_port_parent_id() replaced SWITCHDEV_ATTR_ID_PORT_PARENT_ID in kernel 5.1.0 */
    .ndo_get_port_parent_id = __sx_netdev_get_port_parent_id,
#endif
};

/* if called from a sysfs handler, must be called within the context of sx_netdev_sysfs_handler_start() */
static int sx_netdev_module_reset(struct net_device *dev)
{
    int                        ret;
    struct sx_net_priv        *net_priv = netdev_priv(dev);
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(dev);

    if (!priv) {
        return -ENODEV;
    }

    ret = sx_core_reset_port_module(net_priv->dev, net_priv->port);
    dev_netdev_priv_put(priv);

    return ret;
}

static int sx_netdev_module_power_mode_policy_set(struct net_device               *dev,
                                                  enum sx_module_power_mode_policy power_mode_policy)
{
    int                        ret;
    struct sx_net_priv        *net_priv = netdev_priv(dev);
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(dev);

    if (!priv) {
        return -ENODEV;
    }

    ret = sx_core_set_port_module_power_mode_policy(net_priv->dev,
                                                    net_priv->port,
                                                    power_mode_policy);

    dev_netdev_priv_put(priv);
    return ret;
}

/* must be called within the context of sx_netdev_sysfs_handler_start() */
static int sx_netdev_module_power_mode_get(struct net_device *dev, struct sx_module_power_mode_params *params)
{
    int                        ret;
    struct sx_net_priv        *net_priv = netdev_priv(dev);
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(dev);

    if (!priv) {
        return -ENODEV;
    }

    ret = sx_core_get_port_module_power_mode(net_priv->dev,
                                             net_priv->port,
                                             params);

    dev_netdev_priv_put(priv);
    return ret;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0))
static int sx_get_ts_info(struct net_device *dev, struct ethtool_ts_info *info)
#else
static int sx_get_ts_info(struct net_device *dev, struct kernel_ethtool_ts_info *info)
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0)) */
{
    uint8_t                    ptp_clock_index = 0, ptp_enable = 0;
    int                        ret;
    struct sx_net_priv        *net_priv = netdev_priv(dev);
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(dev);

    if (!priv) {
        return -ENODEV;
    }

    ret = sx_core_sysfs_dev_hold(net_priv->dev);
    if (ret) {
        goto out;
    }

    ret = ethtool_op_get_ts_info(dev, info);
    if (ret) {
        goto out_release;
    }
    if ((!net_priv->is_port_netdev && !net_priv->is_lag) || !net_priv->dev) {
        goto out_release; /* ret = 0 */
    }

    sx_core_get_ptp_state(net_priv->dev, &ptp_enable);
    if (ptp_enable) {
        sx_core_get_ptp_clock_index(net_priv->dev, &ptp_clock_index);
        info->phc_index = ptp_clock_index;

        info->so_timestamping |= SOF_TIMESTAMPING_TX_HARDWARE |
                                 SOF_TIMESTAMPING_RX_HARDWARE |
                                 SOF_TIMESTAMPING_RAW_HARDWARE;

        sx_core_ptp_get_supported_tx_types(net_priv->dev,
                                           &info->tx_types);

        info->rx_filters = (BIT(1) << HWTSTAMP_FILTER_NONE) |
                           (BIT(1) << HWTSTAMP_FILTER_ALL);
    }

out_release:
    sx_core_sysfs_dev_release(net_priv->dev);
out:
    dev_netdev_priv_put(priv);
    return ret;
}

struct sx_ethtool_ppcnt_stats {
    char            str[ETH_GSTRING_LEN];
    sxd_ppcnt_grp_t grp;
    u64             (*getter)(const union sxd_ppcnt_counter_set_u* counter_set);
};

static u64 sx_reg_ppcnt_a_frame_check_sequence_errors_get(const union sxd_ppcnt_counter_set_u* counter_set)
{
    return BUILD_64BIT_COUNTER(counter_set->eth_802_3_cntrs_grp_data_layout.a_frame_check_sequence_errors_high,
                               counter_set->eth_802_3_cntrs_grp_data_layout.a_frame_check_sequence_errors_low);
}

static u64 sx_reg_ppcnt_a_symbol_error_during_carrier_get(const union sxd_ppcnt_counter_set_u* counter_set)
{
    return BUILD_64BIT_COUNTER(counter_set->eth_802_3_cntrs_grp_data_layout.a_symbol_error_during_carrier_high,
                               counter_set->eth_802_3_cntrs_grp_data_layout.a_symbol_error_during_carrier_low);
}

static u64 sx_reg_ppcnt_a_in_range_length_errors_get(const union sxd_ppcnt_counter_set_u* counter_set)
{
    return BUILD_64BIT_COUNTER(counter_set->eth_802_3_cntrs_grp_data_layout.a_in_range_length_errors_high,
                               counter_set->eth_802_3_cntrs_grp_data_layout.a_in_range_length_errors_low);
}

static u64 sx_reg_ppcnt_a_out_of_range_length_field_get(const union sxd_ppcnt_counter_set_u* counter_set)
{
    return BUILD_64BIT_COUNTER(counter_set->eth_802_3_cntrs_grp_data_layout.a_out_of_range_length_field_high,
                               counter_set->eth_802_3_cntrs_grp_data_layout.a_out_of_range_length_field_low);
}

static u64 sx_reg_ppcnt_a_frame_too_long_errors_get(const union sxd_ppcnt_counter_set_u* counter_set)
{
    return BUILD_64BIT_COUNTER(counter_set->eth_802_3_cntrs_grp_data_layout.a_frame_too_long_errors_high,
                               counter_set->eth_802_3_cntrs_grp_data_layout.a_frame_too_long_errors_low);
}

static u64 sx_reg_ppcnt_symbol_errors(const union sxd_ppcnt_counter_set_u* counter_set)
{
    return BUILD_64BIT_COUNTER(counter_set->phys_layer_cntrs.symbol_errors_high,
                               counter_set->phys_layer_cntrs.symbol_errors_low);
}

static struct sx_ethtool_ppcnt_stats ppcnt_grp_802_3_stats[] = {
    {
        .str = "a_symbol_error_during_carrier",
        .grp = SXD_PPCNT_GRP_IEEE_802_3_COUNTERS_E,
        .getter = sx_reg_ppcnt_a_symbol_error_during_carrier_get,
    },
    {
        .str = "a_frame_check_sequence_errors",
        .grp = SXD_PPCNT_GRP_IEEE_802_3_COUNTERS_E,
        .getter = sx_reg_ppcnt_a_frame_check_sequence_errors_get,
    },
    {
        .str = "a_in_range_length_errors",
        .grp = SXD_PPCNT_GRP_IEEE_802_3_COUNTERS_E,
        .getter = sx_reg_ppcnt_a_in_range_length_errors_get,
    },
    {
        /* triggered when an ethernet frame's length/type field value is between 1501 and 1535 */
        .str = "a_out_of_range_length_field",
        .grp = SXD_PPCNT_GRP_IEEE_802_3_COUNTERS_E,
        .getter = sx_reg_ppcnt_a_out_of_range_length_field_get,
    },
    {
        .str = "a_frame_too_long_errors",
        .grp = SXD_PPCNT_GRP_IEEE_802_3_COUNTERS_E,
        .getter = sx_reg_ppcnt_a_frame_too_long_errors_get,
    },
};

static struct sx_ethtool_ppcnt_stats ppcnt_grp_phys_stats[] = {
    {
        .str = "symbol_errors",
        .grp = SXD_PPCNT_GRP_PHYSICAL_LAYER_COUNTERS_E,
        .getter = sx_reg_ppcnt_symbol_errors,
    },
};

#define PPCNT_GRP_802_3_STATS_LEN ARRAY_SIZE(ppcnt_grp_802_3_stats)
#define PPCNT_GRP_PHYS_STATS_LEN  ARRAY_SIZE(ppcnt_grp_phys_stats)
#define SX_ETHTOOL_STATS_LEN      (PPCNT_GRP_802_3_STATS_LEN + PPCNT_GRP_PHYS_STATS_LEN)

static void sx_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
    u8 *p = data;
    int i = 0;

    switch (stringset) {
    case ETH_SS_STATS:
        for (i = 0; i < PPCNT_GRP_802_3_STATS_LEN; i++) {
            memcpy(p, ppcnt_grp_802_3_stats[i].str, ETH_GSTRING_LEN);
            p += ETH_GSTRING_LEN;
        }
        for (i = 0; i < PPCNT_GRP_PHYS_STATS_LEN; i++) {
            memcpy(p, ppcnt_grp_phys_stats[i].str, ETH_GSTRING_LEN);
            p += ETH_GSTRING_LEN;
        }

        break;
    }
}

static int sx_get_sset_count(struct net_device *dev, int sset)
{
    switch (sset) {
    case ETH_SS_STATS:
        return SX_ETHTOOL_STATS_LEN;

    default:
        return -EOPNOTSUPP;
    }
}

static void sx_get_ethtool_stats(struct net_device *dev, struct ethtool_stats *stats, u64 *data)
{
    int                        err = 0, i = 0, index = 0;
    struct sx_net_priv        *net_priv = netdev_priv(dev);
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(dev);
    struct ku_access_ppcnt_reg reg_data;

    if (!priv) {
        return;
    }

    if (!net_priv->is_port_netdev || net_priv->is_lag) {
        goto out;
    }

    memset(&reg_data, 0, sizeof(reg_data));

    err = sx_core_sysfs_dev_hold(net_priv->dev);
    if (err) {
        goto out;
    }

    if (!__is_local_port_bound_to_swid(priv->dev, net_priv->port)) {
        err = -EINVAL;
        goto out_release;
    }

    reg_data.dev_id = net_priv->dev->device_id;
    sx_cmd_set_op_tlv(&reg_data.op_tlv, MLXSW_PPCNT_ID,
                      EMAD_METHOD_QUERY);

    reg_data.ppcnt_reg.clr = 0;
    reg_data.ppcnt_reg.lp_gl = 1;
    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data.ppcnt_reg.local_port,
                                        reg_data.ppcnt_reg.lp_msb,
                                        net_priv->port);
    reg_data.ppcnt_reg.swid = 0;
    reg_data.ppcnt_reg.pnat = SXD_PPCNT_PNAT_LOCAL_PORT_NUMBER_E;

    reg_data.ppcnt_reg.grp = SXD_PPCNT_GRP_IEEE_802_3_COUNTERS_E;
    err = sx_ACCESS_REG_PPCNT(net_priv->dev, &reg_data);
    if (err) {
        goto out_release;
    }
    for (i = 0; i < PPCNT_GRP_802_3_STATS_LEN; i++) {
        data[index++] = ppcnt_grp_802_3_stats[i].getter(&reg_data.ppcnt_reg.counter_set);
    }

    memset(&reg_data.ppcnt_reg.counter_set, 0, sizeof(reg_data.ppcnt_reg.counter_set));
    reg_data.ppcnt_reg.grp = SXD_PPCNT_GRP_PHYSICAL_LAYER_COUNTERS_E;
    err = sx_ACCESS_REG_PPCNT(net_priv->dev, &reg_data);
    if (err) {
        goto out_release;
    }
    for (i = 0; i < PPCNT_GRP_PHYS_STATS_LEN; i++) {
        data[index++] = ppcnt_grp_phys_stats[i].getter(&reg_data.ppcnt_reg.counter_set);
    }

out_release:
    sx_core_sysfs_dev_release(net_priv->dev);
out:
    dev_netdev_priv_put(priv);
}

static void __set_link_ext_state(struct sx_ethtool_link_ext_state_opcode_mapping link_ext_state_mapping,
                                 struct sx_link_ext_state_info                  *link_ext_state_info)
{
    switch (link_ext_state_mapping.link_ext_state) {
    case SX_LINK_EXT_STATE_AUTONEG:
        link_ext_state_info->autoneg =
            link_ext_state_mapping.link_ext_substate;
        break;

    case SX_LINK_EXT_STATE_LINK_TRAINING_FAILURE:
        link_ext_state_info->link_training =
            link_ext_state_mapping.link_ext_substate;
        break;

    case SX_LINK_EXT_STATE_LINK_LOGICAL_MISMATCH:
        link_ext_state_info->link_logical_mismatch =
            link_ext_state_mapping.link_ext_substate;
        break;

    case SX_LINK_EXT_STATE_BAD_SIGNAL_INTEGRITY:
        link_ext_state_info->bad_signal_integrity =
            link_ext_state_mapping.link_ext_substate;
        break;

    case SX_LINK_EXT_STATE_CABLE_ISSUE:
        link_ext_state_info->cable_issue =
            link_ext_state_mapping.link_ext_substate;
        break;

    default:
        break;
    }

    link_ext_state_info->link_ext_state = link_ext_state_mapping.link_ext_state;
}

static int __get_link_ext_state(struct net_device *dev, struct sx_link_ext_state_info *link_ext_state_info)
{
    struct sx_net_priv        *net_priv = netdev_priv(dev);
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(dev);
    struct ku_access_pddr_reg  reg_data;
    int                        err = 0, i = 0;
    uint32_t                   status_opcode = 0;
    bool                       is_independent = false;

    if (!priv) {
        return -ENODEV;
    }

    if (!net_priv->is_port_netdev || net_priv->is_lag) {
        err = -ENODATA;
        goto out;
    }

    if (netif_carrier_ok(dev)) {
        err = -ENODATA;
        goto out;
    }

    err = sx_core_sysfs_dev_hold(net_priv->dev);
    if (err) {
        goto out;
    }

    err = sx_core_get_port_module_independent(net_priv->dev, net_priv->port, &is_independent);
    if (err) {
        goto out_release;
    }

    if (is_independent) {
        sxd_log_warning(SXD_DEV_FROM_NETDEV_PRIV(net_priv),
                        "%s: Failed to read link ext state: port module is independent (SW control).\n",
                        __func__);
        err = -ENODATA;
        goto out_release;
    }

    if (!__is_local_port_bound_to_swid(priv->dev, net_priv->port)) {
        err = -EINVAL;
        goto out_release;
    }

    memset(&reg_data, 0, sizeof(reg_data));

    reg_data.dev_id = net_priv->dev->device_id;
    sx_cmd_set_op_tlv(&reg_data.op_tlv, PDDR_REG_ID, EMAD_METHOD_QUERY);

    reg_data.pddr_reg.page_select = SXD_PDDR_PAGE_SELECT_TROUBLESHOOTING_INFO_PAGE_E;
    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data.pddr_reg.local_port,
                                        reg_data.pddr_reg.lp_msb,
                                        net_priv->port);
    reg_data.pddr_reg.pnat = SXD_PDDR_PNAT_LOCAL_PORT_NUMBER_E;

    err = sx_ACCESS_REG_PDDR(net_priv->dev, &reg_data);
    if (err) {
        goto out_release;
    }

    status_opcode =
        reg_data.pddr_reg.page_data.pddr_troubleshooting_page.status_opcode.pddr_monitor_opcode.monitor_opcode;

    err = -ENODATA;
    if (!status_opcode) {
        goto out_release;
    }

    for (i = 0; i < ARRAY_SIZE(sx_link_ext_state_opcode_map); i++) {
        if (sx_link_ext_state_opcode_map[i].status_opcode == status_opcode) {
            __set_link_ext_state(sx_link_ext_state_opcode_map[i], link_ext_state_info);
            err = 0;
            goto out_release;
        }
    }

    /* if you get here, err = -ENODATA */

out_release:
    sx_core_sysfs_dev_release(net_priv->dev);

out:
    dev_netdev_priv_put(priv);
    return err;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)) || defined(CONFIG_SX_RHEL_8_6)
static int sx_get_link_ext_state(struct net_device *dev, struct ethtool_link_ext_state_info *link_ext_state_info)
{
    int                           err = 0;
    struct sx_link_ext_state_info link_ext_state;

    memset(&link_ext_state, 0, sizeof(link_ext_state));

    err = __get_link_ext_state(dev, &link_ext_state);
    if (err) {
        return err;
    }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 16, 0))
    if (link_ext_state.link_ext_state == SX_LINK_EXT_STATE_MODULE) {
        return -ENODATA;
    }
#endif

    link_ext_state_info->link_ext_state = (enum ethtool_link_ext_state)link_ext_state.link_ext_state;
    link_ext_state_info->__link_ext_substate = link_ext_state.__link_ext_substate;

    return 0;
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0))
static int sx_reset_module(struct net_device *dev, u32 *flags)
{
    u32 req = *flags;
    int err = 0;

    if (!(req & ETH_RESET_PHY) &&
        !(req & (ETH_RESET_PHY << ETH_RESET_SHARED_SHIFT))) {
        goto out;
    }

    err = sx_netdev_module_reset(dev);
    if (err) {
        sxd_log_err(SXD_DEV_NONE, "Failed to reset module on %s\n", dev->name);
        goto out;
    }

    *flags &= ~(ETH_RESET_PHY | (ETH_RESET_PHY << ETH_RESET_SHARED_SHIFT));

out:
    return err;
}

static int sx_get_module_power_mode(struct net_device                       *dev,
                                    struct ethtool_module_power_mode_params *params,
                                    struct netlink_ext_ack                  *extack)
{
    int                                err = 0;
    struct sx_module_power_mode_params power_mode_params;

    memset(&power_mode_params, 0, sizeof(power_mode_params));

    err = sx_netdev_module_power_mode_get(dev, &power_mode_params);

    if (err) {
        NL_SET_ERR_MSG_MOD(extack, "Failed to retrieve module's power mode");
        goto out;
    }

    params->mode = (enum ethtool_module_power_mode)power_mode_params.mode;
    params->policy = (enum ethtool_module_power_mode_policy)power_mode_params.policy;

out:
    return err;
}

static int sx_set_module_power_mode(struct net_device                             *dev,
                                    const struct ethtool_module_power_mode_params *params,
                                    struct netlink_ext_ack                        *extack)
{
    int err = 0;

    err = sx_netdev_module_power_mode_policy_set(dev, (enum sx_module_power_mode_policy)params->policy);

    if (err) {
        NL_SET_ERR_MSG_MOD(extack, "Failed to set module's power mode policy");
        goto out;
    }

out:
    return err;
}
#endif /* if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)) */

const struct ethtool_ops sx_ethtool_ops = {
    .get_link = ethtool_op_get_link,
    .get_sset_count = sx_get_sset_count,
    .get_strings = sx_get_strings,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)) || defined(CONFIG_SX_RHEL_8_6)
    .get_link_ext_state = sx_get_link_ext_state,
#endif
    .get_ts_info = sx_get_ts_info,
    .get_ethtool_stats = sx_get_ethtool_stats,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0))
    .set_module_power_mode = sx_set_module_power_mode,
    .get_module_power_mode = sx_get_module_power_mode,
    .reset = sx_reset_module,
#endif
};

int sx_netdev_create_module_sysfs_nodes_for_tec(struct kobject *parent)
{
    int err = 0;

    if (parent == NULL) {
        err = -ENOMEM;
        goto out;
    }
    err = sysfs_create_file(parent, &(module_tec_cooling_level_attr.attr));
    if (err) {
        goto clean_up_module_temp_tec_kobj;
    }
    err = sysfs_create_file(parent, &(module_tec_max_power_attr.attr));
    if (err) {
        goto clean_up_module_temp_tec_cooling_level;
    }
    err = sysfs_create_file(parent, &(module_tec_power_attr.attr));
    if (err) {
        goto clean_up_module_temp_tec_max_power;
    }
    err = sysfs_create_file(parent, &(module_tec_power_warning_high_attr.attr));
    if (err) {
        goto clean_up_module_temp_tec_power;
    }
    err = sysfs_create_file(parent, &(module_tec_power_warning_low_attr.attr));
    if (err) {
        goto clean_up_module_temp_tec_power_warning_high;
    }
    err = sysfs_create_file(parent, &(module_tec_power_alarm_high_attr.attr));
    if (err) {
        goto clean_up_module_temp_tec_power_warning_low;
    }
    err = sysfs_create_file(parent, &(module_tec_power_alarm_low_attr.attr));
    if (err) {
        goto clean_up_module_temp_tec_power_alarm_high;
    }
    err = sysfs_create_file(parent, &(module_tec_max_set_point_temperature_attr.attr));
    if (err) {
        goto clean_up_module_temp_tec_power_alarm_low;
    }
    err = sysfs_create_file(parent, &(module_tec_set_point_temperature_attr.attr));
    if (err) {
        goto clean_up_module_temp_tec_max_set_point_temperature;
    }
    err = sysfs_create_file(parent, &(module_tec_set_point_temperature_warning_high_attr.attr));
    if (err) {
        goto clean_up_module_temp_tec_set_point_temperature;
    }
    err = sysfs_create_file(parent, &(module_tec_set_point_temperature_warning_low_attr.attr));
    if (err) {
        goto clean_up_module_temp_tec_set_point_temperature_warning_high;
    }
    err = sysfs_create_file(parent, &(module_tec_set_point_temperature_alarm_high_attr.attr));
    if (err) {
        goto clean_up_module_temp_tec_set_point_temperature_warning_low;
    }
    err = sysfs_create_file(parent, &(module_tec_set_point_temperature_alarm_low_attr.attr));
    if (err) {
        goto clean_up_module_temp_tec_set_point_temperature_alarm_high;
    }
    err = sysfs_create_file(parent, &(module_tec_max_cooling_level_attr.attr));
    if (err) {
        goto clean_up_module_temp_tec_set_point_temperature_alarm_low;
    }
    err = sysfs_create_file(parent, &(module_tec_min_cooling_level_attr.attr));
    if (err) {
        goto clean_up_module_temp_tec_max_cooling_level;
    }

    return 0;

clean_up_module_temp_tec_max_cooling_level:
    sysfs_remove_file(parent, &(module_tec_max_cooling_level_attr.attr));
clean_up_module_temp_tec_set_point_temperature_alarm_low:
    sysfs_remove_file(parent, &(module_tec_set_point_temperature_alarm_low_attr.attr));
clean_up_module_temp_tec_set_point_temperature_alarm_high:
    sysfs_remove_file(parent, &(module_tec_set_point_temperature_alarm_high_attr.attr));
clean_up_module_temp_tec_set_point_temperature_warning_low:
    sysfs_remove_file(parent, &(module_tec_set_point_temperature_warning_low_attr.attr));
clean_up_module_temp_tec_set_point_temperature_warning_high:
    sysfs_remove_file(parent, &(module_tec_set_point_temperature_warning_high_attr.attr));
clean_up_module_temp_tec_set_point_temperature:
    sysfs_remove_file(parent, &(module_tec_set_point_temperature_attr.attr));
clean_up_module_temp_tec_max_set_point_temperature:
    sysfs_remove_file(parent, &(module_tec_max_set_point_temperature_attr.attr));
clean_up_module_temp_tec_power_alarm_low:
    sysfs_remove_file(parent, &(module_tec_power_alarm_low_attr.attr));
clean_up_module_temp_tec_power_alarm_high:
    sysfs_remove_file(parent, &(module_tec_power_alarm_high_attr.attr));
clean_up_module_temp_tec_power_warning_low:
    sysfs_remove_file(parent, &(module_tec_power_warning_low_attr.attr));
clean_up_module_temp_tec_power_warning_high:
    sysfs_remove_file(parent, &(module_tec_power_warning_high_attr.attr));
clean_up_module_temp_tec_power:
    sysfs_remove_file(parent, &(module_tec_power_attr.attr));
clean_up_module_temp_tec_max_power:
    sysfs_remove_file(parent, &(module_tec_max_power_attr.attr));
clean_up_module_temp_tec_cooling_level:
    sysfs_remove_file(parent, &(module_tec_cooling_level_attr.attr));
clean_up_module_temp_tec_kobj:
    kobject_put(parent);
    parent = NULL;

out:
    return err;
}

void sx_netdev_delete_module_sysfs_nodes_for_tec(struct kobject *parent)
{
    if (parent == NULL) {
        return;
    }

    sysfs_remove_file(parent, &(module_tec_cooling_level_attr.attr));
    sysfs_remove_file(parent, &(module_tec_max_power_attr.attr));
    sysfs_remove_file(parent, &(module_tec_power_attr.attr));
    sysfs_remove_file(parent, &(module_tec_power_warning_high_attr.attr));
    sysfs_remove_file(parent, &(module_tec_power_warning_low_attr.attr));
    sysfs_remove_file(parent, &(module_tec_power_alarm_high_attr.attr));
    sysfs_remove_file(parent, &(module_tec_power_alarm_low_attr.attr));
    sysfs_remove_file(parent, &(module_tec_max_set_point_temperature_attr.attr));
    sysfs_remove_file(parent, &(module_tec_set_point_temperature_attr.attr));
    sysfs_remove_file(parent, &(module_tec_set_point_temperature_warning_high_attr.attr));
    sysfs_remove_file(parent, &(module_tec_set_point_temperature_warning_low_attr.attr));
    sysfs_remove_file(parent, &(module_tec_set_point_temperature_alarm_high_attr.attr));
    sysfs_remove_file(parent, &(module_tec_set_point_temperature_alarm_low_attr.attr));
    sysfs_remove_file(parent, &(module_tec_max_cooling_level_attr.attr));
    sysfs_remove_file(parent, &(module_tec_min_cooling_level_attr.attr));
}

int sx_netdev_create_module_sysfs_nodes(struct net_device *netdev)
{
    int                 err = 0;
    struct sx_net_priv *net_priv = netdev_priv(netdev);
    bool                tec_support = false;

    if (SX_PORT_TYPE_ID_GET(net_priv->log_port) != SX_PORT_TYPE_NETWORK) {
        goto out;
    }

    /* Although the PDDR related sysfs entries are under port directory, they are related to the modules.*/
    err = sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(link_ext_state_attr.attr));
    if (err) {
        goto out;
    }
    err = sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(netdev_log_port_attr.attr));
    if (err) {
        goto remove_link_ext_state;
    }
    err = sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(netdev_pipeline_attr.attr));
    if (err) {
        goto remove_netdev_log_port_attr;
    }
    err = sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(pddr_operation_attr.attr));
    if (err) {
        goto remove_netdev_pipeline_attr;
    }
    err = sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(pddr_troubleshooting_attr.attr));
    if (err) {
        goto remove_pddr_operation;
    }
    err = sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(pddr_module_attr.attr));
    if (err) {
        goto remove_pddr_troubleshooting;
    }
    err = sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(pddr_link_down_attr.attr));
    if (err) {
        goto remove_pddr_module;
    }
    err = sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(pddr_latched_flag_attr.attr));
    if (err) {
        goto remove_pddr_link_down;
    }

    net_priv->kobj[SX_NET_KOBJECT_MODULE] = kobject_create_and_add("module", net_priv->kobj[SX_NET_KOBJECT_PORT]);
    if (net_priv->kobj[SX_NET_KOBJECT_MODULE] == NULL) {
        err = -ENOMEM;
        goto remove_pddr_latched_flag;
    }
    err = sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_MODULE], &(module_id_attr.attr));
    if (err) {
        goto clean_up_module_kobj;
    }

    err = sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_MODULE], &(module_reset_attr.attr));
    if (err) {
        goto clean_up_module_kobj;
    }
    err = sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_MODULE], &(module_power_mode_policy_attr.attr));
    if (err) {
        goto clean_up_module_kobj;
    }
    err = sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_MODULE], &(module_power_mode_attr.attr));
    if (err) {
        goto clean_up_module_kobj;
    }

    net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP] =
        kobject_create_and_add("temperature", net_priv->kobj[SX_NET_KOBJECT_MODULE]);
    if (net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP] == NULL) {
        err = -ENOMEM;
        goto clean_up_module_kobj;
    }
    err = sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP], &(module_temp_input_attr.attr));
    if (err) {
        goto clean_up_module_temp_kobj;
    }
    err = sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP], &(module_temp_label_attr.attr));
    if (err) {
        goto clean_up_module_temp_input;
    }
    err = sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP], &(module_temp_threshold_hi_attr.attr));
    if (err) {
        goto clean_up_module_temp_label;
    }
    err = sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP], &(module_temp_threshold_lo_attr.attr));
    if (err) {
        goto clean_up_module_temp_threshold_hi;
    }
    err =
        sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP], &(module_temp_threshold_critical_hi_attr.attr));
    if (err) {
        goto clean_up_module_temp_threshold_lo;
    }
    err =
        sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP], &(module_temp_threshold_critical_lo_attr.attr));
    if (err) {
        goto clean_up_module_temp_threshold_critical_hi;
    }

    __get_tec_support(netdev, &tec_support);
    if (tec_support) {
        net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP_TEC] =
            kobject_create_and_add("tec", net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP]);
        err = sx_netdev_create_module_sysfs_nodes_for_tec(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP_TEC]);
        if (err) {
            goto clean_up_module_temp_kobj;
        }
    }

    /* for module eeprom sysfs */
    net_priv->eeprom_kobjs = kzalloc(sizeof(*(net_priv->eeprom_kobjs)), GFP_KERNEL);
    if (!(net_priv->eeprom_kobjs)) {
        sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "port sysfs nodes handler failed to allocated memory.\n");
        err = -ENOMEM;
        goto clean_up_module_temp_threshold_critical_lo;
    }
    err = sx_netdev_create_module_sysfs_eeprom_tree(net_priv->kobj[SX_NET_KOBJECT_MODULE],
                                                    net_priv->eeprom_kobjs);
    if (err) {
        sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv),
                    "port sysfs nodes handler failed to create eeprom sysfs sub-tree.\n");
        goto free_eeprom_kobjs;
    }

    err =
        sx_netdev_create_module_sysfs_extension_for_s3ip(net_priv->kobj[SX_NET_KOBJECT_MODULE]);
    if (err) {
        sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv),
                    "port sysfs nodes handler failed to create sysfs extension for S3IP.\n");
        goto delete_module_sysfs_eeprom_tree;
    }

    return 0;

delete_module_sysfs_eeprom_tree:
    sx_netdev_delete_module_sysfs_eeprom_tree(net_priv->eeprom_kobjs);

free_eeprom_kobjs:
    kfree(net_priv->eeprom_kobjs);
    net_priv->eeprom_kobjs = NULL;

clean_up_module_temp_threshold_critical_lo:
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP], &(module_temp_threshold_critical_lo_attr.attr));

clean_up_module_temp_threshold_critical_hi:
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP], &(module_temp_threshold_critical_hi_attr.attr));

clean_up_module_temp_threshold_lo:
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP], &(module_temp_threshold_lo_attr.attr));

clean_up_module_temp_threshold_hi:
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP], &(module_temp_threshold_hi_attr.attr));

clean_up_module_temp_label:
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP], &(module_temp_label_attr.attr));

clean_up_module_temp_input:
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP], &(module_temp_input_attr.attr));

clean_up_module_temp_kobj:
    kobject_put(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP]);
    net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP] = NULL;

clean_up_module_kobj:
    kobject_put(net_priv->kobj[SX_NET_KOBJECT_MODULE]);
    net_priv->kobj[SX_NET_KOBJECT_MODULE] = NULL;

remove_pddr_latched_flag:
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(pddr_latched_flag_attr.attr));

remove_pddr_link_down:
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(pddr_link_down_attr.attr));

remove_pddr_module:
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(pddr_module_attr.attr));

remove_pddr_troubleshooting:
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(pddr_troubleshooting_attr.attr));

remove_pddr_operation:
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(pddr_operation_attr.attr));

remove_netdev_pipeline_attr:
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(netdev_pipeline_attr.attr));

remove_netdev_log_port_attr:
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(netdev_log_port_attr.attr));

remove_link_ext_state:
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(link_ext_state_attr.attr));

out:
    return err;
}

void sx_netdev_delete_module_sysfs_nodes(struct net_device *netdev)
{
    struct sx_net_priv *net_priv = netdev_priv(netdev);

    if (SX_PORT_TYPE_ID_GET(net_priv->log_port) != SX_PORT_TYPE_NETWORK) {
        return;
    }

    sx_netdev_delete_module_sysfs_extension_for_s3ip(net_priv->kobj[SX_NET_KOBJECT_MODULE]);
    sx_netdev_delete_module_sysfs_eeprom_tree(net_priv->eeprom_kobjs);
    kfree(net_priv->eeprom_kobjs);
    net_priv->eeprom_kobjs = NULL;
    sx_netdev_delete_module_sysfs_nodes_for_tec(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP_TEC]); /* check if tec exist inside the function */
    kobject_put(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP_TEC]);
    net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP_TEC] = NULL;
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP], &(module_temp_input_attr.attr));
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP], &(module_temp_label_attr.attr));
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP], &(module_temp_threshold_hi_attr.attr));
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP], &(module_temp_threshold_lo_attr.attr));
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP], &(module_temp_threshold_critical_hi_attr.attr));
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP], &(module_temp_threshold_critical_lo_attr.attr));
    kobject_put(net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP]);
    net_priv->kobj[SX_NET_KOBJECT_MODULE_TEMP] = NULL;
    kobject_put(net_priv->kobj[SX_NET_KOBJECT_MODULE]);
    net_priv->kobj[SX_NET_KOBJECT_MODULE] = NULL;

    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(pddr_latched_flag_attr.attr));
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(pddr_link_down_attr.attr));
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(pddr_module_attr.attr));
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(pddr_troubleshooting_attr.attr));
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(pddr_operation_attr.attr));
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(link_ext_state_attr.attr));
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(netdev_log_port_attr.attr));
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(netdev_pipeline_attr.attr));
}

static int __sx_netdev_create_lag_sysfs_nodes(struct net_device *netdev)
{
    int                        err = 0;
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return -ENODEV;
    }

    net_priv->kobj[SX_NET_KOBJECT_PORT] = kobject_create_and_add(netdev->name, priv->kobj[SX_KOBJECT_ASIC]);
    if (net_priv->kobj[SX_NET_KOBJECT_PORT] == NULL) {
        err = -ENOMEM;
        goto out;
    }

    err = sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(netdev_log_port_attr.attr));
    if (err) {
        goto clean_up_port_kobj;
    }

    err = sysfs_create_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(netdev_pipeline_attr.attr));
    if (err) {
        goto remove_netdev_log_port_attr;
    }

    goto out;

remove_netdev_log_port_attr:
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(netdev_log_port_attr.attr));

clean_up_port_kobj:
    kobject_put(net_priv->kobj[SX_NET_KOBJECT_PORT]);
    net_priv->kobj[SX_NET_KOBJECT_PORT] = NULL;

out:
    dev_netdev_priv_put(priv);
    return err;
}

int sx_netdev_create_port_sysfs_nodes(struct net_device *netdev)
{
    int                        err = 0;
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return -ENODEV;
    }

    if (SX_PORT_TYPE_ID_GET(net_priv->log_port) == SX_PORT_TYPE_LAG) {
        err = __sx_netdev_create_lag_sysfs_nodes(netdev);
        goto out;
    }

    if (SX_PORT_TYPE_ID_GET(net_priv->log_port) != SX_PORT_TYPE_NETWORK) {
        goto out;
    }

    net_priv->kobj[SX_NET_KOBJECT_PORT] = kobject_create_and_add(netdev->name, priv->kobj[SX_KOBJECT_ASIC]);
    if (net_priv->kobj[SX_NET_KOBJECT_PORT] == NULL) {
        err = -ENOMEM;
        goto out;
    }

    if (priv->netdev_resources.create_module_sysfs && (net_priv->kobj[SX_NET_KOBJECT_MODULE] == NULL)) {
        err = sx_netdev_create_module_sysfs_nodes(netdev);
        if (err) {
            goto clean_up_port_kobj;
        }
    }

    err = sx_netdev_synce_sysfs_init(net_priv->kobj[SX_NET_KOBJECT_PORT]);
    if (err) {
        goto clean_up_module_sysfs_nodes;
    }

    goto out;

clean_up_module_sysfs_nodes:
    if (priv->netdev_resources.create_module_sysfs) {
        sx_netdev_delete_module_sysfs_nodes(netdev);
    }

clean_up_port_kobj:
    kobject_put(net_priv->kobj[SX_NET_KOBJECT_PORT]);
    net_priv->kobj[SX_NET_KOBJECT_PORT] = NULL;

out:
    dev_netdev_priv_put(priv);
    return err;
}

static int __sx_netdev_remove_lag_sysfs_nodes(struct net_device *netdev)
{
    struct sx_net_priv *net_priv = netdev_priv(netdev);

    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(netdev_log_port_attr.attr));
    sysfs_remove_file(net_priv->kobj[SX_NET_KOBJECT_PORT], &(netdev_pipeline_attr.attr));

    kobject_put(net_priv->kobj[SX_NET_KOBJECT_PORT]);
    net_priv->kobj[SX_NET_KOBJECT_PORT] = NULL;
    return 0;
}
int sx_netdev_remove_port_sysfs_nodes(struct net_device *netdev)
{
    int                 err = 0;
    struct sx_net_priv *net_priv = netdev_priv(netdev);

    if (net_priv->kobj[SX_NET_KOBJECT_PORT] == NULL) {
        goto out;
    }

    if (SX_PORT_TYPE_ID_GET(net_priv->log_port) == SX_PORT_TYPE_LAG) {
        return __sx_netdev_remove_lag_sysfs_nodes(netdev);
    }

    if (SX_PORT_TYPE_ID_GET(net_priv->log_port) != SX_PORT_TYPE_NETWORK) {
        goto out;
    }

    sx_netdev_synce_sysfs_deinit(net_priv->kobj[SX_NET_KOBJECT_PORT]);
    if (net_priv->kobj[SX_NET_KOBJECT_MODULE] != NULL) {
        sx_netdev_delete_module_sysfs_nodes(netdev);
    }
    kobject_put(net_priv->kobj[SX_NET_KOBJECT_PORT]);
    net_priv->kobj[SX_NET_KOBJECT_PORT] = NULL;

out:
    return err;
}

#if !defined(SX_DROP_MONITOR_DISABLED) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, \
                                                                                 0)) || defined(CONFIG_SX_RHEL_8_6))
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)) || defined(CONFIG_SX_RHEL_9_4)
static int sx_netdev_port_split(struct devlink         *devlink,
                                struct devlink_port    *port,
                                unsigned int            count,
                                struct netlink_ext_ack *extack)
#else /* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0) || defined(CONFIG_SX_RHEL_9_4) */
static int sx_netdev_port_split(struct devlink         *devlink,
                                unsigned int            port_index,
                                unsigned int            count,
                                struct netlink_ext_ack *extack)
#endif/* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0) || defined(CONFIG_SX_RHEL_9_4) */
{
#if defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)) || defined(CONFIG_SX_RHEL_9_4)
    return port_devlink_port_split(devlink, port->index, count, extack);
#else/* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0) || defined(CONFIG_SX_RHEL_9_4) */
    return port_devlink_port_split(devlink, port_index, count, extack);
#endif/* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0) || defined(CONFIG_SX_RHEL_9_4) */
#else /* defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) */
    return -ENOTSUPP;
#endif /* defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) */
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)) || defined(CONFIG_SX_RHEL_9_4)
static int sx_netdev_port_unsplit(struct devlink *devlink, struct devlink_port *port, struct netlink_ext_ack *extack)
#else /* (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)) || defined(CONFIG_SX_RHEL_9_4) */
static int sx_netdev_port_unsplit(struct devlink *devlink, unsigned int port_index, struct netlink_ext_ack *extack)
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)) || defined(CONFIG_SX_RHEL_9_4) */
{
#if defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)) || defined(CONFIG_SX_RHEL_9_4)
    return port_devlink_port_unsplit(devlink, port->index, extack);
#else /* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0) || defined(CONFIG_SX_RHEL_9_4) */
    return port_devlink_port_unsplit(devlink, port_index, extack);
#endif/* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)  || defined(CONFIG_SX_RHEL_9_4) */
#else /* defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) */
    return -ENOTSUPP;
#endif /* defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) */
}

#endif /* !defined(SX_DROP_MONITOR_DISABLED) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) || defined(CONFIG_SX_RHEL_8_6)) */

#if !defined(SX_DROP_MONITOR_DISABLED) && \
    ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) || defined(CONFIG_SX_RHEL_8_6))
#define DEVLINK_DUMMY_NAME "devlink_dummy"

static struct bus_type devlink_dummy_bus = {
    .name = DEVLINK_DUMMY_NAME,
    .dev_name = DEVLINK_DUMMY_NAME,
};

static struct device_driver devlink_dummy_driver = {
    .name = DEVLINK_DUMMY_NAME,
    .bus = &devlink_dummy_bus,
    .owner = THIS_MODULE,
};

static void devlink_dummy_dev_release(struct device *dev)
{
}

static struct device devlink_dummy_dev = {
    .bus = &devlink_dummy_bus,
    .release = devlink_dummy_dev_release,
};

#if !((LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)) || defined(CONFIG_SX_RHEL_9_4))
static int __sx_netdev_devlink_reload_down(struct devlink            *devlink,
                                           bool                       netns_change,
                                           enum devlink_reload_action action,
                                           enum devlink_reload_limit  limit,
                                           struct netlink_ext_ack    *extack)
{
    return 0;
}

static int __sx_netdev_devlink_reload_up(struct devlink            *devlink,
                                         enum devlink_reload_action action,
                                         enum devlink_reload_limit  limit,
                                         u32                       *actions_performed,
                                         struct netlink_ext_ack    *extack)
{
    return 0;
}
#endif /* !((LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)) || defined(CONFIG_SX_RHEL_9_4)) */

static const struct devlink_ops devlink_dummy_ops = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)) || defined(CONFIG_SX_RHEL_9_4)
    /* for now, ops is empty. split/unsplit callbacks moved to 'struct devlink_port_ops' */
#else /* (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5 ,0)) || defined(CONFIG_SX_RHEL_9_4) */
    .reload_actions = BIT(DEVLINK_RELOAD_ACTION_DRIVER_REINIT) | BIT(DEVLINK_RELOAD_ACTION_FW_ACTIVATE),
    .reload_down = __sx_netdev_devlink_reload_down,
    .reload_up = __sx_netdev_devlink_reload_up,
    .port_split = sx_netdev_port_split,
    .port_unsplit = sx_netdev_port_unsplit,
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5 ,0)) || defined(CONFIG_SX_RHEL_9_4) */
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)) || defined(CONFIG_SX_RHEL_9_4)
static const struct devlink_port_ops devlink_port_dummy_ops = {
    .port_split = sx_netdev_port_split,
    .port_unsplit = sx_netdev_port_unsplit,
};
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5 ,0)) || defined(CONFIG_SX_RHEL_9_4) */

static int __devlink_dummy_init(struct sx_dev_netdev_priv *priv)
{
    int err = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) || defined(CONFIG_SX_RHEL_9_4)
    priv->devlink_dummy_instance = devlink_alloc_ns(&devlink_dummy_ops,
                                                    0, priv->netns,
                                                    &devlink_dummy_dev);

#else /* (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) || defined(CONFIG_SX_RHEL_9_4) */
    priv->devlink_dummy_instance = devlink_alloc(&devlink_dummy_ops, 0);
    if (priv->devlink_dummy_instance) {
        write_pnet(&(priv->devlink_dummy_instance)->_net, priv->netns);
    }
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) || defined(CONFIG_SX_RHEL_9_4) */
    if (!priv->devlink_dummy_instance) {
        err = -ENOMEM;
        goto out;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) || defined(CONFIG_SX_RHEL_9_4)
    devlink_register(priv->devlink_dummy_instance);
#else /* (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) || defined(CONFIG_SX_RHEL_9_4) */
    err = devlink_register(priv->devlink_dummy_instance, &devlink_dummy_dev);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) || defined(CONFIG_SX_RHEL_9_4) */
    if (err) {
        goto err_devlink_free;
    }
    goto out;

err_devlink_free:
    devlink_free(priv->devlink_dummy_instance);
    priv->devlink_dummy_instance = NULL;

out:
    return err;
}

static void __devlink_dummy_exit(struct sx_dev_netdev_priv *priv)
{
    devlink_unregister(priv->devlink_dummy_instance);
    devlink_free(priv->devlink_dummy_instance);
    priv->devlink_dummy_instance = NULL;
}
#endif /* if !defined(SX_DROP_MONITOR_DISABLED) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) */

int sx_netdev_devlink_port_register(struct net_device *netdev)
{
#if !defined(SX_DROP_MONITOR_DISABLED) && \
    ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) || defined(CONFIG_SX_RHEL_8_6))
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    struct devlink_port_attrs  attrs = {};
    struct devlink_port       *devlink_port;
    int                        err = 0;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return -ENODEV;
    }

    if (!devlink_port_en) {
        goto out; /* err = 0 */
    }

    devlink_port = &net_priv->devlink_port;
    memset(devlink_port, 0, sizeof(*devlink_port));

    attrs.split = false;
#define SX_NETDEV_DEVLINK_ATTR_LANES_MAX 16
    attrs.lanes = SX_NETDEV_DEVLINK_ATTR_LANES_MAX;
    attrs.splittable = true;
    devlink_port_attrs_set(devlink_port, &attrs);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)) || defined(CONFIG_SX_RHEL_9_4)
    err = devlink_port_register_with_ops(priv->devlink_dummy_instance,
                                         devlink_port,
                                         net_priv->log_port,
                                         &devlink_port_dummy_ops);
#else /* (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5 ,0)) || defined(CONFIG_SX_RHEL_9_4) */
    err = devlink_port_register(priv->devlink_dummy_instance, devlink_port, net_priv->log_port);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5 ,0)) || defined(CONFIG_SX_RHEL_9_4) */

    if (err) {
        sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv),
                    "Error (%d) when registering devlink for netdev %s\n",
                    err,
                    netdev->name);
        goto out;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 2, 0)) || defined(CONFIG_SX_RHEL_9_4)
    devlink_port_type_eth_set(devlink_port);
#else /* (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 2 ,0)) || defined(CONFIG_SX_RHEL_9_4) */
    devlink_port_type_eth_set(devlink_port, netdev);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 2 ,0)) || defined(CONFIG_SX_RHEL_9_4) */

out:
    dev_netdev_priv_put(priv);
    return err;
#else  /* if !defined(SX_DROP_MONITOR_DISABLED) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) || defined(CONFIG_SX_RHEL_8_6)) */
    return 0;
#endif /* if !defined(SX_DROP_MONITOR_DISABLED) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) || defined(CONFIG_SX_RHEL_8_6)) */
}

void sx_netdev_devlink_port_unregister(struct net_device *netdev)
{
#if !defined(SX_DROP_MONITOR_DISABLED) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, \
                                                                                 0)) || defined(CONFIG_SX_RHEL_8_6))
    struct sx_net_priv *net_priv = netdev_priv(netdev);

    if (devlink_port_en) {
        devlink_port_unregister(&net_priv->devlink_port);
    }
#endif
}


/* ---------------------------------    USER NETDEV --------------------------*/

/* Add a netdev to the per-ASIC user listener set's netdev list */
static int __sx_user_netdev_list_add(struct sx_dev_netdev_priv *priv, struct net_device *netdev)
{
    struct sx_user_netdev_node *node;
    struct sx_user_netdev_node *iter;
    int                         err = 0;

    if (!priv || !netdev) {
        return -EINVAL;
    }

    mutex_lock(&priv->user_netdev_listener_set.netdev_list_lock);

    /* Prevent duplicates */
    list_for_each_entry(iter, &priv->user_netdev_listener_set.netdev_list, list) {
        if (iter->netdev == netdev) {
            err = -EEXIST;
            goto out_unlock;
        }
    }

    node = kmalloc(sizeof(*node), GFP_KERNEL);
    if (!node) {
        err = -ENOMEM;
        goto out_unlock;
    }

    INIT_LIST_HEAD(&node->list);
    node->netdev = netdev;
    list_add_tail(&node->list, &priv->user_netdev_listener_set.netdev_list);

out_unlock:
    mutex_unlock(&priv->user_netdev_listener_set.netdev_list_lock);
    return err;
}

/* Remove a netdev from the per-ASIC user listener set's netdev list */
static void __sx_user_netdev_list_remove(struct sx_dev_netdev_priv *priv, struct net_device *netdev)
{
    struct sx_user_netdev_node *iter, *tmp;

    if (!priv || !netdev) {
        return;
    }

    mutex_lock(&priv->user_netdev_listener_set.netdev_list_lock);
    list_for_each_entry_safe(iter, tmp, &priv->user_netdev_listener_set.netdev_list, list) {
        if (iter->netdev == netdev) {
            list_del(&iter->list);
            kfree(iter);

            break;
        }
    }
    mutex_unlock(&priv->user_netdev_listener_set.netdev_list_lock);
}

/* Iterate over all netdevs in the user listener set and do dev_put()*/
static void __sx_user_netdev_list_dev_put_for_each(struct sx_dev_netdev_priv *priv)
{
    struct sx_user_netdev_node *iter, *tmp;
    struct net_device          *nd = NULL;

    if (!priv) {
        return;
    }

    mutex_lock(&priv->user_netdev_listener_set.netdev_list_lock);
    list_for_each_entry_safe(iter, tmp, &priv->user_netdev_listener_set.netdev_list, list) {
        nd = iter->netdev;

        dev_put(nd);
        list_del(&iter->list);
        kfree(iter);
    }
    mutex_unlock(&priv->user_netdev_listener_set.netdev_list_lock);
}

/* Add a node for the current task and a given netdev */
static int __sx_user_ctx_add(struct sx_dev_netdev_priv *priv, struct net_device *netdev)
{
    struct sx_caller_ctx_node *node;

    node = kmalloc(sizeof(*node), GFP_KERNEL);
    if (!node) {
        return -ENOMEM;
    }

    INIT_LIST_HEAD(&node->list);
    node->netdev = netdev;
    node->caller = current;

    mutex_lock(&priv->user_netdev_listener_set.user_netdev_set_lock);
    list_add_tail(&node->list, &priv->user_netdev_listener_set.caller_ctx_list);
    mutex_unlock(&priv->user_netdev_listener_set.user_netdev_set_lock);

    return 0;
}


/* Find the netdev for a given task (optionally remove a ref). */
static struct net_device * __sx_user_ctx_find_netdev_by_task(struct sx_dev_netdev_priv *priv,
                                                             struct task_struct        *task,
                                                             bool                       remove_netdev,
                                                             bool                       put_ref)
{
    struct sx_caller_ctx_node *node;
    struct net_device         *nd = NULL;

    mutex_lock(&priv->user_netdev_listener_set.user_netdev_set_lock);
    list_for_each_entry(node, &priv->user_netdev_listener_set.caller_ctx_list, list) {
        if (node->caller == task) {
            nd = node->netdev;
            if (remove_netdev) {
                list_del(&node->list);
                kfree(node);
            }
            if (put_ref && nd) {
                dev_put(nd);
            }
            break;
        }
    }
    mutex_unlock(&priv->user_netdev_listener_set.user_netdev_set_lock);

    return nd;
}

static struct net_device * __sx_user_ctx_find_and_remove_netdev_for_current(struct sx_dev_netdev_priv *priv)
{
    return __sx_user_ctx_find_netdev_by_task(priv, current, true, false);
}

static struct net_device * __sx_user_ctx_find_remove_and_put_netdev_for_current(struct sx_dev_netdev_priv *priv)
{
    return __sx_user_ctx_find_netdev_by_task(priv, current, true, true);
}

static void __sx_user_ctx_drain(struct sx_dev_netdev_priv *priv)
{
    struct sx_caller_ctx_node *node, *tmp;
    struct net_device         *nd = NULL;

    mutex_lock(&priv->user_netdev_listener_set.user_netdev_set_lock);
    list_for_each_entry_safe(node, tmp,
                             &priv->user_netdev_listener_set.caller_ctx_list, list) {
        nd = node->netdev;
        dev_put(nd); /*release reference acquire on lookup */
        list_del(&node->list);
        kfree(node);
    }
    mutex_unlock(&priv->user_netdev_listener_set.user_netdev_set_lock);
}

static int __sx_netdev_lookup_netdev_by_ifindex_in_netns(struct sx_dev *dev, int ifindex)
{
    struct net_device         *netdev;
    int                        err = 0;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return -ENODEV;
    }
    /* lookup for the netdev and acquires a reference. */
    netdev = dev_get_by_index(current->nsproxy->net_ns, ifindex);
    if (!netdev) {
        sxd_log_err(dev, "No net device found in for ifindex %d\n", ifindex);
        err = -ENODEV;
        goto out;
    }

    sxd_log_debug(dev, "Found net_device: name= %s ifindex=%d netns=%p\n",
                  netdev->name,
                  netdev->ifindex,
                  current->nsproxy->net_ns);

    err = __sx_user_ctx_add(priv, netdev);
    if (err) {
        sxd_log_err(dev, "Couldn't store netdevice for ifindex %d\n", ifindex);
    }

out:
    dev_netdev_priv_put(priv);
    return err;
}

/*----------------------------- sx_dev_netdev_priv ------------------------------*/

struct sx_dev_netdev_priv * dev_netdev_priv_from_dev(struct sx_dev *dev)
{
    struct sx_dev_netdev_priv *priv = NULL, *priv_iter;

    if (!dev) {
        return NULL;
    }

    read_lock_bh(&__priv_db_lock);

    list_for_each_entry(priv_iter, &__priv_db, priv_sx_netdev_list) {
        if ((priv_iter->dev == dev) && priv_iter->sx_core_if.bound) {
            atomic_inc(&priv_iter->sx_core_if.refcount);
            priv = priv_iter;
            break;
        }
    }

    read_unlock_bh(&__priv_db_lock);

    if (!priv) {
        sxd_log_debug(dev,
                      "dev_netdev_priv_from_dev: cannot find priv for dev [%u]\n",
                      dev->device_id);
    }

    return priv;
}

static struct sx_dev_netdev_priv* __dev_netdev_priv_from_namespace(struct net *net)
{
    struct sx_dev_netdev_priv *priv = NULL, *priv_iter;

    read_lock_bh(&__priv_db_lock);

    if (!sx_core_has_pci_multi_dev()) {
        priv = list_first_entry_or_null(&__priv_db, struct sx_dev_netdev_priv, priv_sx_netdev_list);
        goto out;
    }

    list_for_each_entry(priv_iter, &__priv_db, priv_sx_netdev_list) {
        if (priv_iter->netns == net) {
            priv = priv_iter;
            break;
        }
    }

out:
    if (priv && priv->sx_core_if.bound) {
        atomic_inc(&priv->sx_core_if.refcount);
    } else {
        priv = NULL;
    }

    read_unlock_bh(&__priv_db_lock);
    return priv;
}

struct sx_dev_netdev_priv* dev_netdev_priv_from_curr_net_namespace(void)
{
    return __dev_netdev_priv_from_namespace(get_curr_net_namespace());
}

void dev_netdev_priv_put(struct sx_dev_netdev_priv *priv)
{
    if (atomic_dec_and_test(&priv->sx_core_if.refcount)) {
        complete(&priv->sx_core_if.free);
    }
}

struct net* get_curr_net_namespace(void)
{
    return current->nsproxy->net_ns;
}


static void sx_netdev_unregister_device(struct net_device *netdev)
{
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return;
    }

    netif_tx_disable(netdev);
    netif_carrier_off(netdev);

    sx_core_flush_synd_by_context(net_priv->dev, netdev);
    unregister_netdev(netdev); /* It will call our stop function */

    free_netdev(netdev);
    dev_netdev_priv_put(priv);
}

#define ATTACH_ONE_NETDEV(netdev, sx_dev)                                                                    \
    net_priv = netdev_priv(netdev);                                                                          \
    net_priv->dev = sx_dev;                                                                                  \
    for (uc_type = USER_CHANNEL_L3_NETDEV; uc_type < NUM_OF_NET_DEV_TYPE; uc_type++) {                       \
        err = __sx_netdev_uc_type_get_data(uc_type, &crit, &netdev_callback, synd_str, sizeof(synd_str));    \
        if (err) {                                                                                           \
            sxd_log_err(sx_dev, "%s: Failed get crit and handler for "                                       \
                        "user channel (%d).\n", __func__, uc_type);                                          \
            continue;                                                                                        \
        }                                                                                                    \
                                                                                                             \
        for (i = 0; i < net_priv->num_of_traps[uc_type]; i++) {                                              \
            port_vlan.port_vlan_type = (ku_port_vlan_params_type_t)net_priv->trap_ids[uc_type][i].info_type; \
            port_vlan.sysport = net_priv->trap_ids[uc_type][i].sysport;                                      \
            port_vlan.lag_id = net_priv->trap_ids[uc_type][i].lag_id;                                        \
            port_vlan.vlan = net_priv->trap_ids[uc_type][i].vlan;                                            \
            err = sx_core_add_synd(net_priv->swid, net_priv->trap_ids[uc_type][i].synd, L2_TYPE_ETH,         \
                                   NETDEV_DRV_NAME, 0, &crit, synd_str, netdev_callback, netdev,             \
                                   CHECK_DUP_ENABLED_E, net_priv->dev, &port_vlan, 1);                       \
            if (err) {                                                                                       \
                sxd_log_err(sx_dev, "error %s, User channel (%d) failed registering on "                     \
                            "0x%x syndrome.\n", netdev->name, uc_type, net_priv->trap_ids[uc_type][i].synd); \
            }                                                                                                \
        }                                                                                                    \
    }

#define SX_DEV_ATTACH_ONE_NETDEV(netdev, sx_dev) \
    net_priv = netdev_priv(netdev);              \
    net_priv->dev = sx_dev;

#define DETACH_ONE_NETDEV(netdev)                                                                  \
    netif_tx_disable(netdev);                                                                      \
    netif_carrier_off(netdev);                                                                     \
    net_priv = netdev_priv(netdev);                                                                \
    for (uc_type = USER_CHANNEL_L3_NETDEV; uc_type < NUM_OF_NET_DEV_TYPE; uc_type++) {             \
        if (__sx_netdev_uc_type_get_data(uc_type, &crit, &netdev_callback, NULL, 0)) {             \
            sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: Failed get crit and handler for " \
                        "user channel (%d).\n", __func__, uc_type);                                \
            continue;                                                                              \
        }                                                                                          \
                                                                                                   \
        for (i = 0; i < net_priv->num_of_traps[uc_type]; i++) {                                    \
            port_vlan.port_vlan_type =                                                             \
                (ku_port_vlan_params_type_t)net_priv->trap_ids[uc_type][i].info_type;              \
            port_vlan.sysport = net_priv->trap_ids[uc_type][i].sysport;                            \
            port_vlan.lag_id = net_priv->trap_ids[uc_type][i].lag_id;                              \
            port_vlan.vlan = net_priv->trap_ids[uc_type][i].vlan;                                  \
            sx_core_remove_synd(net_priv->swid,                                                    \
                                net_priv->trap_ids[uc_type][i].synd,                               \
                                L2_TYPE_ETH, 0, &crit, netdev, net_priv->dev,                      \
                                netdev_callback, &port_vlan, 1, NULL);                             \
        }                                                                                          \
    }                                                                                              \
    net_priv->dev = NULL;                                                                          \

static void attach_netdevs(struct sx_dev *dev)
{
    int                        err = 0;
    struct net_device         *netdev = NULL;
    struct sx_net_priv        *net_priv = NULL;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);
    union ku_filter_critireas  crit;
    cq_handler                 netdev_callback = 0;
    int                        i, j;
    u8                         uc_type = 0;
    struct ku_port_vlan_params port_vlan;
    int                        port_type = 0;
    int                        port = 0, br_id = 0;
    int                        swid = 0;
    int                        max_ports[] = {
        [PORT_TYPE_SINGLE] = MAX_SYSPORT_NUM,
        [PORT_TYPE_LAG] = MAX_LAG_NUM
    };
    char                       synd_str[SX_LISTENER_HANDLER_STR_MAX_LEN] = { 0 };

    if (!priv) {
        return;
    }

    mutex_lock(&priv->netdev_resources.rsc_lock);
    priv->netdev_resources.in_attach_detach = 1;

    for (swid = 0; swid < NUMBER_OF_SWIDS; swid++) {
        if (!priv->netdev_resources.allocated[swid]) {
            continue;
        }

        netdev = priv->netdev_resources.sx_netdevs[swid];
        if (netdev == NULL) {
            continue;
        }

        ATTACH_ONE_NETDEV(netdev, dev);
        netdev_linkstate_set(netdev);
    }

    for (port_type = 0; port_type < PORT_TYPE_NUM; port_type++) {
        for (port = 0; port < max_ports[port_type]; port++) {
            if (priv->netdev_resources.port_allocated[port_type][port] == 0) {
                continue;
            }
            for (j = 0; j < MAX_PORT_NETDEV_NUM; j++) {
                netdev = priv->netdev_resources.port_netdev[port_type][port][j];
                if (netdev != NULL) {
                    ATTACH_ONE_NETDEV(netdev, dev);
                    netdev_linkstate_set(netdev);
                }
            }
        }
    }

    for (br_id = 0; br_id < MAX_BRIDGE_NUM; br_id++) {
        netdev = priv->bridge_netdev_db[br_id];
        if (netdev != NULL) {
            SX_DEV_ATTACH_ONE_NETDEV(netdev, dev);
            netdev_linkstate_set(netdev);
        }
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) || defined(CONFIG_SX_RHEL_8_6)
    if (priv->span2cpu_dev) {
        SX_DEV_ATTACH_ONE_NETDEV(priv->span2cpu_dev, dev);
    }
#endif

    priv->netdev_resources.in_attach_detach = 0;
    mutex_unlock(&priv->netdev_resources.rsc_lock);

    dev_netdev_priv_put(priv);
}


static void sx_netdev_phy_port_remove(struct net_device *netdev)
{
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return;
    }

    /* Remove sysfs node for physical port */
    sx_netdev_remove_port_sysfs_nodes(netdev);

    sx_core_flush_synd_by_context(net_priv->dev, netdev);

    /*unregister netdev*/
    synchronize_rcu();

    sx_netdev_devlink_port_unregister(netdev);
    unregister_netdev(netdev); /* It will call our stop function */
    dev_netdev_priv_put(priv);
}


static void sx_netdev_remove_netdevs(struct sx_dev *dev)
{
    struct net_device         *netdev = NULL;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);
    struct sx_net_priv        *net_priv = NULL;
    int                        j;
    int                        port_type = 0;
    int                        port = 0, br_id = 0;
    int                        max_ports[] = {
        [PORT_TYPE_SINGLE] = MAX_SYSPORT_NUM,
        [PORT_TYPE_LAG] = MAX_LAG_NUM
    };

    sxd_log_info(dev, "removing all netdevs for dev [%u]\n", dev->device_id);

    if (!priv) {
        return;
    }

    mutex_lock(&priv->netdev_resources.rsc_lock);
    priv->netdev_resources.in_attach_detach = 1;

    for (port_type = 0; port_type < PORT_TYPE_NUM; port_type++) {
        for (port = 0; port < max_ports[port_type]; port++) {
            if (priv->netdev_resources.port_allocated[port_type][port] == 0) {
                continue;
            }
            for (j = 0; j < MAX_PORT_NETDEV_NUM; j++) {
                netdev = priv->netdev_resources.port_netdev[port_type][port][j];
                if (netdev != NULL) {
                    sx_netdev_phy_port_remove(netdev);
                    priv->netdev_resources.port_netdev[port_type][port][j] = NULL;
                    priv->netdev_resources.port_allocated[port_type][port]--;
                }
            }
        }
    }

    for (br_id = 0; br_id < MAX_BRIDGE_NUM; br_id++) {
        netdev = priv->bridge_netdev_db[br_id];
        if (netdev != NULL) {
            net_priv = netdev_priv(netdev);
            sx_netdev_unregister_device(netdev);
            priv->bridge_netdev_db[net_priv->bridge_id - MIN_BRIDGE_ID] = NULL;
        }
    }

    priv->netdev_resources.in_attach_detach = 0;
    mutex_unlock(&priv->netdev_resources.rsc_lock);

    dev_netdev_priv_put(priv);
}


static void detach_netdevs(struct sx_dev *dev)
{
    struct net_device         *netdev = NULL;
    struct sx_net_priv        *net_priv = NULL;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);
    union ku_filter_critireas  crit;
    cq_handler                 netdev_callback = 0;
    int                        i;
    u8                         uc_type = 0;
    struct ku_port_vlan_params port_vlan;
    int                        j;
    int                        port_type = 0;
    int                        port = 0, br_id = 0;
    int                        swid = 0;
    int                        max_ports[] = {
        [PORT_TYPE_SINGLE] = MAX_SYSPORT_NUM,
        [PORT_TYPE_LAG] = MAX_LAG_NUM
    };

    if (!priv) {
        return;
    }

    mutex_lock(&priv->netdev_resources.rsc_lock);
    priv->netdev_resources.in_attach_detach = 1;

    for (swid = 0; swid < NUMBER_OF_SWIDS; swid++) {
        if (!priv->netdev_resources.allocated[swid]) {
            continue;
        }

        netdev = priv->netdev_resources.sx_netdevs[swid];
        if (netdev == NULL) {
            continue;
        }

        DETACH_ONE_NETDEV(netdev);
    }

    for (port_type = 0; port_type < PORT_TYPE_NUM; port_type++) {
        for (port = 0; port < max_ports[port_type]; port++) {
            if (priv->netdev_resources.port_allocated[port_type][port] == 0) {
                continue;
            }
            for (j = 0; j < MAX_PORT_NETDEV_NUM; j++) {
                netdev = priv->netdev_resources.port_netdev[port_type][port][j];
                if (netdev != NULL) {
                    DETACH_ONE_NETDEV(netdev);
                }
            }
        }
    }

    for (br_id = 0; br_id < MAX_BRIDGE_NUM; br_id++) {
        netdev = priv->bridge_netdev_db[br_id];
        if (netdev != NULL) {
            DETACH_ONE_NETDEV(netdev);
        }
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) || defined(CONFIG_SX_RHEL_8_6)
    if (priv->span2cpu_dev) {
        SX_DEV_ATTACH_ONE_NETDEV(priv->span2cpu_dev, NULL);
    }
#endif

    priv->netdev_resources.in_attach_detach = 0;
    mutex_unlock(&priv->netdev_resources.rsc_lock);

    dev_netdev_priv_put(priv);
}

static void sx_netdev_attach_global_event_handler(struct sx_dev *dev)
{
    int                        err = 0;
    union ku_filter_critireas  crit;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return;
    }

    /* Register listener for Ethernet SWID */
    memset(&crit, 0, sizeof(crit));

    err = sx_core_add_synd(ROUTER_PORT_SWID,
                           NUM_HW_SYNDROMES,
                           L2_TYPE_DONT_CARE,
                           NETDEV_DRV_NAME,
                           0,
                           &crit,
                           "NETDEV (Global)",
                           sx_netdev_handle_global_pkt,
                           NULL,
                           CHECK_DUP_ENABLED_E,
                           dev,
                           NULL,
                           1);
    if (err) {
        sxd_log_err(dev, "%s: Failed registering global rx_handler", __func__);
    }
    err = sx_core_add_synd(0,
                           SXD_TRAP_ID_PUDE,
                           L2_TYPE_DONT_CARE,
                           NETDEV_DRV_NAME,
                           0,
                           &crit,
                           "NETDEV (PUDE-Event)",
                           sx_netdev_handle_pude_event,
                           NULL,
                           CHECK_DUP_ENABLED_E,
                           dev,
                           NULL,
                           1);
    if (err) {
        sxd_log_err(dev, "%s: Failed registering PUDE event rx_handler", __func__);
    }

/* For Palladium skip event registration which invoke sysfs_notify as sysfs init is skipped */
#if !defined(PD_BU)
    err = sx_core_add_synd(0,
                           SXD_TRAP_ID_PMPE,
                           L2_TYPE_DONT_CARE,
                           NETDEV_DRV_NAME,
                           0,
                           &crit,
                           "NETDEV (PMPE-Event)",
                           sx_netdev_handle_pmpe_event,
                           NULL,
                           CHECK_DUP_ENABLED_E,
                           dev,
                           NULL,
                           1);
    if (err) {
        sxd_log_err(dev, "%s: Failed registering PMPE event rx_handler", __func__);
    }
#endif /*if !defined(PD_BU)*/

    dev_netdev_priv_put(priv);
}


int sx_netdev_register_device(struct net_device *netdev, int should_rtnl_lock, int admin_state)
{
    int                 err = 0;
    struct sx_net_priv *net_priv = netdev_priv(netdev);
    struct inet6_dev   *idev = NULL;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0))
    u8 dev_addr[ETH_ALEN] = {0};
#endif
    sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: called for device %s\n", __func__, netdev->name);

#if defined(CONFIG_NET_SWITCHDEV) && \
    (LINUX_VERSION_CODE < KERNEL_VERSION(5, 1, 0)) && !defined(CONFIG_SX_RHEL_8_6) && !defined(CONFIG_SX_CENTOS_8_4)
    /* switchdev_ops was removed in kernel 5.1 */
    netdev->switchdev_ops = &sx_netdev_switchdev_ops;
#endif

    /* set netdev defaults and function callbacks */
    netdev->base_addr = 0;
    netdev->irq = 0;
    netdev->features |= NETIF_F_HW_VLAN_CTAG_FILTER;
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 17, 0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(6, 12, 0)))
    netdev->priv_flags |= IFF_CHANGE_PROTO_DOWN;
#endif

#if defined(CONFIG_NET_PORT) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)))
    if (sx_netdev_tc_psample_enable) {
        netdev->features |= NETIF_F_HW_TC;
    }
#endif /* if defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0))
    sx_netdev_u64_to_mac(dev_addr, net_priv->mac);
    eth_hw_addr_set(netdev, dev_addr);
#else
    sx_netdev_u64_to_mac(netdev->dev_addr, net_priv->mac);
#endif

    netdev->mtu = DEFAULT_FRAME_SIZE;

    netdev->netdev_ops = &sx_netdev_ops;
#if defined(CONFIG_NET_PORT) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)))
    netdev->ethtool_ops = &sx_cl_ethtool_ops;
#else
    netdev->ethtool_ops = &sx_ethtool_ops;
#endif

    sx_netdev_devlink_port_register(netdev);

    if (should_rtnl_lock) {
        sx_netdev_rtnl_lock();
    }
    err = register_netdevice(netdev);

    idev = __in6_dev_get(netdev);
    if (idev) {
        idev->cnf.disable_ipv6 = 1;
    }

    if (admin_state && net_priv->mac) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) || defined(CONFIG_SX_RHEL_8_6) || defined(CONFIG_SX_CENTOS_8_4)
        dev_change_flags(netdev, netdev->flags | IFF_UP, NULL);
#else
        dev_change_flags(netdev, netdev->flags | IFF_UP);
#endif
    } else {
        netif_tx_disable(netdev);
        netif_carrier_off(netdev);
    }
    if (should_rtnl_lock) {
        sx_netdev_rtnl_unlock();
    }

    if (err) {
        sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "Net Device registration "
                    "has failed. err=%d\n", err);
        return err;
    }

    sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: exit\n", __func__);
    return 0;
}

#if defined(CONFIG_NET_PORT) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)))
static void sx_netdev_fib_alias_hw_flags_set(struct fib_rt_info *fri, bool offload, bool trap, bool offload_failed)
{
    fri->offload = offload;
    fri->trap = trap;
    fri->offload_failed = offload_failed;
}

static inline bool sx_netdev_is_offloaded_device(struct net_device *dev)
{
    if ((dev->netdev_ops == &sx_netdev_ops) ||
        dev->rtnl_link_ops) {
        return true;
    }

    return false;
}

static void sx_netdev_fib_new_entry4(struct fib_entry_notifier_info *fen_info)
{
    struct fib_info *fi = fen_info->fi;
    unsigned int     i;
    bool             trap = false;
    struct net      *fb_net;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) || defined(CONFIG_SX_RHEL_8_6)
    fb_net = get_curr_net_namespace();
#else
    fb_net = fen_info->info.net;
#endif

    if ((fi->fib_type == RTN_LOCAL) || (fi->fib_type == RTN_BROADCAST)) {
        trap = true;
    }

    for (i = 0; i < fib_info_num_path(fi); i++) {
        struct fib_nh_common *nhc = fib_info_nhc(fi, i);
        struct net_device    *dev = nhc->nhc_dev;

        if (dev && sx_netdev_is_offloaded_device(dev)) {
            struct fib_rt_info fri;

            fri.fi = fi;
            fri.tb_id = fen_info->tb_id;
            fri.dst = cpu_to_be32(fen_info->dst);
            fri.dst_len = fen_info->dst_len;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 19, 0))
            fri.dscp = fen_info->dscp;
#else
            fri.tos = fen_info->tos;
#endif
            fri.type = fen_info->type;
            if (trap) {
                sx_netdev_fib_alias_hw_flags_set(&fri, false, true, false);
            } else {
                sx_netdev_fib_alias_hw_flags_set(&fri, true, false, false);
            }
            fib_alias_hw_flags_set(fb_net, NULL, &fri);
        }
    }
}

static void sx_netdev_fib_new_entry6(struct fib6_entry_notifier_info *f6en_info)
{
    struct fib6_info *f6i = f6en_info->rt;
    bool              trap = false;
    struct net       *fb_net;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) || defined(CONFIG_SX_RHEL_8_6)
    fb_net = get_curr_net_namespace();
#else
    fb_net = f6en_info->info.net;
#endif

    if ((f6i->fib6_type == RTN_LOCAL) || (f6i->fib6_type == RTN_BROADCAST)) {
        trap = true;
    }

    if (f6i->nh) {
        struct nexthop *nh = f6i->nh;
        unsigned int    i;

        for (i = 0; i < nexthop_num_path(nh); i++) {
            struct fib_nh_common *nhc = nexthop_fib_nhc(nh, i);
            struct net_device    *dev = nhc->nhc_dev;

            if (dev && sx_netdev_is_offloaded_device(dev)) {
                if (trap) {
                    fib6_info_hw_flags_set(fb_net, NULL, f6i, false,
                                           true, false);
                } else {
                    fib6_info_hw_flags_set(fb_net, NULL, f6i, true,
                                           false, false);
                }
            }
        }
    } else {
        /* For ipv6 multipath routes, a notification is sent for each
         * fib6 sibling.
         */
        struct fib6_nh    *fib6_nh = f6i->fib6_nh;
        struct net_device *dev = fib6_nh->fib_nh_dev;

        if (dev && sx_netdev_is_offloaded_device(dev)) {
            if (trap) {
                fib6_info_hw_flags_set(fb_net, NULL, f6i, false, true,
                                       false);
            } else {
                fib6_info_hw_flags_set(fb_net, NULL, f6i, true, false,
                                       false);
            }
        }
    }
}

static int sx_netdev_fib_event_nb(struct notifier_block *nb, unsigned long event, void *ptr)
{
    struct fib_notifier_info *info = ptr;

    if (!route_offload_flags) {
        return NOTIFY_OK;
    }

    if ((event == FIB_EVENT_ENTRY_ADD) || (event == FIB_EVENT_ENTRY_REPLACE)) {
        if (info->family == AF_INET) {
            struct fib_entry_notifier_info *fen_info = ptr;

            sx_netdev_fib_new_entry4(fen_info);
        } else if (info->family == AF_INET6) {
            struct fib6_entry_notifier_info *f6en_info = ptr;

            sx_netdev_fib_new_entry6(f6en_info);
        }
    }

    return NOTIFY_OK;
}

static struct notifier_block sx_netdev_fib_nb = {
    .notifier_call = sx_netdev_fib_event_nb,
};
#endif /* if defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) */

/* init the swid netdev*/
static void  * __sx_netdev_event_swid_up(struct sx_dev *dev, int swid, int synd, u64 mac)
{
    struct sx_net_priv        *net_priv;
    struct net_device         *netdev = NULL;
    int                        err;
    char                       name[10];
    int                        i;
    u8                         uc = 0;
    struct net                *netns = get_curr_net_namespace();
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return NULL;
    }

    sxd_log_info(dev, "Initializing dev [%u]\n", dev->device_id);

    priv->netns = netns;

    if ((swid < 0) || (swid >= NUMBER_OF_SWIDS)) { /* only swids 0-7 are allowed */
        return NULL;
    }

#if defined(CONFIG_NET_PORT) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)))
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) || defined(CONFIG_SX_RHEL_8_6)
    err = register_fib_notifier(netns, &sx_netdev_fib_nb, NULL, NULL);
#else
    err = register_fib_notifier(&sx_netdev_fib_nb, NULL);
#endif
    if (err < 0) {
        sxd_log_err(dev, "Failed to register fib notifier\n");
        goto out;
    }
#endif /* if defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) */


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) || defined(CONFIG_SX_RHEL_8_6)
    if (mirror_device_en) {
        if (!priv->span2cpu_dev) {
            /* Should be created per sx_dev, not per SWID. It's here because for multi-ASIC here is the earlier stage to get the netns */
            err = sx_span2cpu_create_mirror_netdev(&sx_netdev_ops, &sx_ethtool_ops, dev);
            if (err) {
                sxd_log_err(dev, "Error creating SPAN-to-CPU mirror interface\n");
                goto fail_on_sx_span2cpu_create_mirror_netdev;
            }
        }
    }
#endif

#if !defined(SX_DROP_MONITOR_DISABLED) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, \
                                                                                 0)) || defined(CONFIG_SX_RHEL_8_6))
    err = __devlink_dummy_init(priv);
    if (err) {
        sxd_log_err(dev, "Error creating devlink_dummy\n");
        goto fail_on_devlink_dummy_init;
    }
#endif

    /* create SWID netdev*/
    sprintf(name, "swid%d_eth", swid);

    netdev = alloc_netdev(sizeof(*net_priv), name, NET_NAME_UNKNOWN, ether_setup);
    if (!netdev) {
        sxd_log_err(dev, "Net Device struct %s alloc failed, "
                    "aborting.\n", name);
        return NULL;
    }

    /* Add the network name_space since sx_netdev_event_swid_up happens inside the container for multi-asic systems*/
    dev_net_set(netdev, netns);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
    netdev->max_mtu = 0; /* do not limit the max MTU */
#endif
    netdev->hard_header_len = ETH_HLEN + ISX_HDR_SIZE;
    memset(netdev_priv(netdev), 0, sizeof(*net_priv));
    net_priv = netdev_priv(netdev);
    net_priv->swid = swid;
    net_priv->dev = dev;
    net_priv->mac = mac;
    net_priv->is_oper_state_up = 1;
    net_priv->skip_tunnel = 0;
    net_priv->netdev = netdev;

    for (uc = USER_CHANNEL_L3_NETDEV; uc < NUM_OF_NET_DEV_TYPE; uc++) {
        for (i = 0; i < MAX_NUM_TRAPS_TO_REGISTER; i++) {
            net_priv->trap_ids[uc][i].synd = SX_INVALID_TRAP_ID;
        }
        net_priv->num_of_traps[uc] = 0;
    }

    err = sx_netdev_register_device(netdev, 1, 1);
    if (err) {
        goto out;
    }

    dev_netdev_priv_put(priv);
    return netdev;

out:
    if (netdev) {
        free_netdev(netdev);
    }

#if !defined(SX_DROP_MONITOR_DISABLED) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, \
                                                                                 0)) || defined(CONFIG_SX_RHEL_8_6))
    __devlink_dummy_exit(priv);
#endif

#if !defined(SX_DROP_MONITOR_DISABLED) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, \
                                                                                 0)) || defined(CONFIG_SX_RHEL_8_6))
fail_on_devlink_dummy_init:
    if (mirror_device_en) {
        __sx_span2cpu_free_mirror_netdev(dev);
    }
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) || defined(CONFIG_SX_RHEL_8_6)

fail_on_sx_span2cpu_create_mirror_netdev:
#endif /*(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) || defined(CONFIG_SX_RHEL_8_6) */

#if defined(CONFIG_NET_PORT) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)))
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    unregister_fib_notifier(netns, &sx_netdev_fib_nb);
#else
    unregister_fib_notifier(&sx_netdev_fib_nb);
#endif
#endif /* if defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) */

    dev_netdev_priv_put(priv);
    return NULL;
}
/* In Multi ASIC system, during init from dvs_start the function is called for each asic, in the container.
 * same for sx_netdev_remove_one_netdev()
 */
static int sx_netdev_event_swid_up(struct sx_dev *dev, struct sx_netdev_rsc *resources, int swid, int synd, u64 mac)
{
    struct net_device *netdev = NULL;
    int                err = 0;


    mutex_lock(&resources->rsc_lock);

    if (resources->in_attach_detach == 1) {
        mutex_unlock(&resources->rsc_lock);
        sxd_log_err(dev, "Can't init swid %d netdev during attach/detach \n", swid);
        return -EFAULT;
    }

    if (resources->allocated[swid]) {
        mutex_unlock(&resources->rsc_lock);
        sxd_log_debug(dev, "Net Device for swid %d was already allocated\n", swid);
        netdev = resources->sx_netdevs[swid];
        netif_carrier_on(netdev);
        netif_start_queue(netdev);
        return 0;
    }

    resources->allocated[swid] = 1;
    mutex_unlock(&resources->rsc_lock);

    netdev = __sx_netdev_event_swid_up(dev, swid, synd, mac);

    mutex_lock(&resources->rsc_lock);

    if (netdev) {
        resources->sx_netdevs[swid] = netdev;
    } else {
        resources->allocated[swid] = 0;
        err = -EFAULT;
    }

    mutex_unlock(&resources->rsc_lock);

    return err;
}

static void sx_netdev_remove_one_netdev(struct sx_dev *dev, void *rsc, int swid)
{
    struct net_device         *netdev;
    struct sx_netdev_rsc      *resources = rsc;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    mutex_lock(&resources->rsc_lock);

    sxd_log_debug(dev, "sx_netdev_remove_one_netdev dev [%u] , swid %u\n", dev->device_id, swid);

    if (resources->in_attach_detach == 1) {
        mutex_unlock(&resources->rsc_lock);
        sxd_log_err(dev, "Can't remove swid %u netdev during attach/detach \n", swid);
        goto out;
    }

    netdev = resources->sx_netdevs[swid];
    if (!resources->allocated[swid]) {
        mutex_unlock(&resources->rsc_lock);
        goto out;
    }

    resources->sx_netdevs[swid] = NULL;
    resources->allocated[swid] = 0;
    mutex_unlock(&resources->rsc_lock);


#if !defined(SX_DROP_MONITOR_DISABLED) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, \
                                                                                 0)) || defined(CONFIG_SX_RHEL_8_6))
    __devlink_dummy_exit(priv);
#else
    (void)priv;  /* dummy use so compiler won't yell */
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) || defined(CONFIG_SX_RHEL_8_6)
    if (mirror_device_en) {
        __sx_span2cpu_free_mirror_netdev(dev);
    }
#endif

#if defined(CONFIG_NET_PORT) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)))
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    unregister_fib_notifier(priv->netns, &sx_netdev_fib_nb);
#else
    unregister_fib_notifier(&sx_netdev_fib_nb);
#endif
#endif /* if defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) */

    sx_netdev_unregister_device(netdev);
out:
    dev_netdev_priv_put(priv);
}

static void __init_sx_core_if(struct sx_core_interface *sx_core_if)
{
    atomic_set(&sx_core_if->refcount, 1);
    init_completion(&sx_core_if->free);
    sx_core_if->bound = true;
}

/* this is a periodic work that runs once per second.
 * its role is to reset the PUDE monitoring state on the ports.
 */
static void __local_port_toggle_reset_work(struct work_struct *work)
{
    struct delayed_work       *dwork = to_delayed_work(work);
    struct sx_dev_netdev_priv *priv;
    u16                        lp;

    priv = container_of(dwork, struct sx_dev_netdev_priv, local_port_toggle_monitor_w);

    spin_lock_bh(&priv->local_port_toggle_monitor_lock);

    for (lp = 1; lp <= MAX_PHYPORT_NUM; lp++) {
        /* if port is marked as 'rapid flap' and in the last period the PUDE count for the port
         * is less than SX_MAX_PORT_TOGGLE_PER_SEC, we'll uncheck the 'rapid flap' and put a
         * log message with the latest operational state of the port.
         * Further PUDE events on the port will be logged as usual. */
        if (priv->local_port_toggle_monitor[lp - 1].rapid_flap &&
            (priv->local_port_toggle_monitor[lp - 1].toggle_count < SX_MAX_PORT_TOGGLE_PER_SEC)) {
            sxd_log_notice(priv->dev,
                           "Logical port %05X stopped flapping rapidly. Port status is %s\n",
                           SX_LOCAL_PORT_TO_LOG_PORT(lp),
                           priv->local_port_toggle_monitor[lp - 1].is_up ? "UP" : "DOWN");
            priv->local_port_toggle_monitor[lp - 1].rapid_flap = false;
        }

        /* reset the counter of PUDE events on the port for the next period */
        priv->local_port_toggle_monitor[lp - 1].toggle_count = 0;
    }

    spin_unlock_bh(&priv->local_port_toggle_monitor_lock);

    /* schedule this work to one second from now */
    queue_delayed_work(priv->netdev_wq, dwork, HZ);
}

/* In Multi ASIC system, during init from dvs_start the function is called for each asic, in the host.
 * During pci_restart, the function is called in the container context.*/
static void * sx_netdev_init_one(struct sx_dev *dev, struct sx_flow_info *fi)
{
    struct sx_dev_netdev_priv* priv = NULL;
    int                        ret;
    char                       wq_str[32];
    char                       object_name[ASIC_NAME_LEN_MAX + 1];
    int                        asic_index = dev->device_id - 1;

    sxd_log_info(dev, "sx_netdev_init_one dev_id [%u]\n", dev->device_id);

    priv = vzalloc(sizeof(struct sx_dev_netdev_priv));
    if (!priv) {
        sxd_log_err(dev, "Device struct alloc failed, aborting.\n");
        return NULL;
    }

    priv->dev = dev;

    /* Initialize local params with global values for Multi-ASIC systems */
    if (sx_core_has_pci_multi_dev()) {
        priv->local_params.sx_netdev_tx_debug = sx_netdev_tx_debug;
        priv->local_params.sx_netdev_rx_debug = sx_netdev_rx_debug;
        priv->local_params.inited = true;
    }

    sprintf(wq_str, "sx_link_dev_id_%u", dev->device_id);
    priv->netdev_wq = create_singlethread_workqueue(wq_str);
    if (!priv->netdev_wq) {
        sxd_log_err(dev, "Failed to create wq sx_link for dev_id [%u]. \n", dev->device_id);
        goto fail_on_wq;
    }

    INIT_LIST_HEAD(&priv->user_netdev_listener_set.caller_ctx_list);
    INIT_LIST_HEAD(&priv->user_netdev_listener_set.netdev_list);
    mutex_init(&priv->user_netdev_listener_set.netdev_list_lock);
    mutex_init(&priv->user_netdev_listener_set.user_netdev_set_lock);

    INIT_LIST_HEAD(&priv->priv_sx_netdev_list);

    write_lock_bh(&__priv_db_lock);
    list_add_tail(&priv->priv_sx_netdev_list, &__priv_db);
    write_unlock_bh(&__priv_db_lock);

    if (!fi->in_pci_restart) {
        __init_sx_core_if(&priv->sx_core_if);

        /* should happen only in the modprobe(sx_netdev) context. for multi asic, it should run only from the host context.
         * for PCI_restart, it shouldn't br called as the function running in the container context.
         * for PCI_restart, the sx_core init and the rest of init happens in __bind_unbind_sx_core()
         */
        ret = sx_netdev_register_global_event_handler(dev);
        if (ret) {
            sxd_log_err(dev, "Failed to register global event handler . \n");
            goto fail_on_register_global_event_handler;
        }

        sx_core_register_net_dm_hw_report(__sx_net_dm_hw_report, dev);
        sx_psample_init(dev);

        mutex_init(&priv->netdev_resources.rsc_lock);
    }

    /* bind sx_core to netdev */
    __bind_unbind_sx_core(true, priv, fi->in_pci_restart);

    sx_dbg_dump_read_handler_register("netdev_dump",
                                      __netdev_dbg_dump,
                                      NULL,
                                      priv->dev,
                                      NULL);

    /* sysfs creation*/

    if (sx_core_has_pci_multi_dev()) {
        memset(object_name, 0, sizeof(object_name));
        sprintf(object_name, "%s%d", ASIC_SYSFS_PREFIX, asic_index);

        priv->kobj[SX_KOBJECT_ASIC] = kobject_create_and_add(object_name, &(THIS_MODULE->mkobj.kobj));
        if (priv->kobj[SX_KOBJECT_ASIC] == NULL) {
            sxd_log_err(dev, "Failed to create asic sysfs\n");
            ret = -ENOMEM;
            goto fail_on_create_asic_sysfs;
        }

        ret = sysfs_create_group(priv->kobj[SX_KOBJECT_ASIC], &local_params_group);
        if (ret) {
            goto fail_on_create_local_params_group;
        }
    } else {
        priv->kobj[SX_KOBJECT_ASIC] = &(THIS_MODULE->mkobj.kobj);
    }

    ret = sysfs_create_file(priv->kobj[SX_KOBJECT_ASIC], &(bind_sx_core_attr.attr));
    if (ret) {
        goto fail_on_create_bind_sx_core_sysfs_file;
    }

    ret = sysfs_create_file(priv->kobj[SX_KOBJECT_ASIC], &(skip_netdev_attr.attr));
    if (ret) {
        goto fail_on_create_skip_netdev_sysfs_file;
    }

    spin_lock_init(&priv->local_port_toggle_monitor_lock);
    INIT_DELAYED_WORK(&priv->local_port_toggle_monitor_w, __local_port_toggle_reset_work);
    queue_delayed_work(priv->netdev_wq, &priv->local_port_toggle_monitor_w, 0 /* now */);

    return &priv->netdev_resources;

fail_on_create_skip_netdev_sysfs_file:
    sysfs_remove_file(priv->kobj[SX_KOBJECT_ASIC], &(bind_sx_core_attr.attr));

fail_on_create_bind_sx_core_sysfs_file:
    if (sx_core_has_pci_multi_dev()) {
        sysfs_remove_group(priv->kobj[SX_KOBJECT_ASIC], &local_params_group);
    }

fail_on_create_local_params_group:
    if (sx_core_has_pci_multi_dev()) {
        kobject_put(priv->kobj[SX_KOBJECT_ASIC]);
        priv->kobj[SX_KOBJECT_ASIC] = NULL;
    }

fail_on_create_asic_sysfs:
    sx_dbg_dump_read_handler_unregister("netdev_dump", dev);

    if (!fi->in_pci_restart) {
        sx_psample_cleanup(dev);
        sx_core_register_net_dm_hw_report(NULL, dev);
        sx_netdev_unregister_global_event_handler(dev);
    }

fail_on_register_global_event_handler:
    write_lock_bh(&__priv_db_lock);
    list_del(&priv->priv_sx_netdev_list);
    write_unlock_bh(&__priv_db_lock);

    if (priv->netdev_wq) {
        destroy_workqueue(priv->netdev_wq);
    }

fail_on_wq:
    vfree(priv);

    return NULL;
}

static void sx_netdev_remove_one(struct sx_dev *dev, void *rsc, struct sx_flow_info *fi)
{
    int                        i;
    struct sx_netdev_rsc      *resources = (struct sx_netdev_rsc*)rsc;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);
    long                       wait_ret;
    u32                        max_wait = 10; /* seconds */

    sxd_log_info(dev, "sx_netdev_remove_one dev_id [%u]\n", dev->device_id);

    if (!priv) {
        return;
    }
    __sx_user_ctx_drain(priv); /*drain all in progress netdevs register/deregister */
    __sx_user_netdev_list_dev_put_for_each(priv);/*drain all netdev attached to synd */

    for (i = 0; i < NUMBER_OF_SWIDS; i++) {
        sx_netdev_remove_one_netdev(dev, resources, i);
    }

    /* unbind sx_core from netdev */
    __bind_unbind_sx_core(false, priv, fi->in_pci_restart);

    write_lock_bh(&__priv_db_lock);
    list_del(&priv->priv_sx_netdev_list);
    write_unlock_bh(&__priv_db_lock);

    dev_netdev_priv_put(priv); /* this is for the current priv reference */
    dev_netdev_priv_put(priv); /* this is for the reference taken in sx_netdev_init_one() */

    do {
        wait_ret = wait_for_completion_interruptible_timeout(&priv->sx_core_if.free, HZ); /* one second */
        if (wait_ret == 0) { /* timed out */
            sxd_log_warning(dev, "waiting for all netdev-priv references ... (refcount=%d)\n",
                            atomic_read(&priv->sx_core_if.refcount));
        }
    } while (--max_wait > 0 && (wait_ret == 0 || wait_ret == -ERESTARTSYS));

    if (max_wait == 0) {
        sxd_log_err(dev, "quit waiting for all netdev-priv references ... (refcount=%d)\n",
                    atomic_read(&priv->sx_core_if.refcount));
    }

    /* from here, we are the only holders of priv.
     * this is the only place we can use 'priv' after releasing all its references */

    sysfs_remove_file(priv->kobj[SX_KOBJECT_ASIC], &(skip_netdev_attr.attr));
    sysfs_remove_file(priv->kobj[SX_KOBJECT_ASIC], &(bind_sx_core_attr.attr));

    if (sx_core_has_pci_multi_dev()) {
        sysfs_remove_group(priv->kobj[SX_KOBJECT_ASIC], &local_params_group);
        /* Remove from parent synchronously so the name is freed otherwise a subsequent
         * probe (e.g. triggered in pcidrv_restart) can create the same name and hit -EEXIST.
         */
        kobject_del(priv->kobj[SX_KOBJECT_ASIC]);
        kobject_put(priv->kobj[SX_KOBJECT_ASIC]);
        priv->kobj[SX_KOBJECT_ASIC] = NULL;
    }
    sx_dbg_dump_read_handler_unregister("netdev_dump", dev);

    if (priv->dev) {
        priv->dev = NULL; /*dont remove the dev-net namespace mapping */
    }

    if (priv->netdev_wq) {
        cancel_delayed_work_sync(&priv->local_port_toggle_monitor_w);
        flush_workqueue(priv->netdev_wq);
        destroy_workqueue(priv->netdev_wq);
    }

    /* before freeing resource, we must make sure that no RCU reader is using it */
    synchronize_rcu();

    vfree(priv);
}


static int __sx_netdev_dev_synd_add(struct net_device               *netdev,
                                    enum sx_netdev_user_channel_type uc_type,
                                    void                            *rsc,
                                    u16                              synd,
                                    struct ku_port_vlan_params      *port_vlan,
                                    u8                               is_register)
{
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    int                        err = 0, i = 0;
    union ku_filter_critireas  crit;
    cq_handler                 netdev_callback = 0;
    int                        synd_added = 0;
    char                       synd_str[SX_LISTENER_HANDLER_STR_MAX_LEN] = { 0 };
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return -ENODEV;
    }

    if (net_priv->num_of_traps[uc_type] == MAX_NUM_TRAPS_TO_REGISTER) {
        sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: swid %s Too many traps\n",
                    __func__, netdev->name);
        err = -ERANGE;
        goto out;
    }

    /* validate that the trap at new index is INVALID */
    BUG_ON(net_priv->trap_ids[uc_type][net_priv->num_of_traps[uc_type]].synd != SX_INVALID_TRAP_ID);

    for (i = 0; i < net_priv->num_of_traps[uc_type]; i++) {
        if (net_priv->trap_ids[uc_type][i].synd == synd) {
            sxd_log_info(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "The synd 0x%x is already added.\n", synd);
            synd_added = 1;
        }
    }

    if (!synd_added) {
        /* add trap to traps_db */
        net_priv->trap_ids[uc_type][net_priv->num_of_traps[uc_type]].synd = synd;
        net_priv->trap_ids[uc_type][net_priv->num_of_traps[uc_type]].info_type =
            (enum net_port_vlan_type)port_vlan->port_vlan_type;
        net_priv->trap_ids[uc_type][net_priv->num_of_traps[uc_type]].sysport = port_vlan->sysport;
        net_priv->trap_ids[uc_type][net_priv->num_of_traps[uc_type]].lag_id = port_vlan->lag_id;
        net_priv->trap_ids[uc_type][net_priv->num_of_traps[uc_type]].vlan = port_vlan->vlan;
        net_priv->num_of_traps[uc_type]++;
    }

    /* if netdev is active (opened) than config the synd, else the synd will be added after dev open */
    if (netdev->flags & IFF_UP) {
        err = __sx_netdev_uc_type_get_data(uc_type, &crit, &netdev_callback, synd_str, sizeof(synd_str));
        if (err) {
            sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: Failed get crit and handler for "
                        "user channel (%d).\n", __func__, uc_type);
            goto out;
        }

        /* register the listener according to the swid */
        err = sx_core_add_synd(net_priv->swid,
                               synd,
                               L2_TYPE_ETH,
                               NETDEV_DRV_NAME,
                               0,
                               &crit,
                               synd_str,
                               netdev_callback,
                               netdev,
                               CHECK_DUP_ENABLED_E,
                               net_priv->dev,
                               port_vlan,
                               is_register);

        if (err) {
            sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "error %s, Failed registering on "
                        "0x%x syndrome.\n", netdev->name, synd);
            goto out;
        }
    }

out:
    dev_netdev_priv_put(priv);
    return err;
}

static int __sx_netdev_dev_synd_remove(struct net_device               *netdev,
                                       enum sx_netdev_user_channel_type uc_type,
                                       void                            *rsc,
                                       u16                              synd,
                                       struct ku_port_vlan_params      *port_vlan,
                                       u8                               is_register)
{
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    int                        err = 0;
    u8                         trap_was_removed = 0;
    int                        i;
    union ku_filter_critireas  crit;
    cq_handler                 netdev_callback = 0;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return -ENODEV;
    }

    /* remove synd from sx_core driver */
    if (netdev->flags & IFF_UP) {
        err = __sx_netdev_uc_type_get_data(uc_type, &crit, &netdev_callback, NULL, 0);
        if (err) {
            sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "%s: Failed get crit and handler for "
                        "user channel (%d).\n", __func__, uc_type);
            goto out;
        }

        err = sx_core_remove_synd(net_priv->swid, synd, L2_TYPE_ETH, 0,
                                  &crit, netdev, net_priv->dev, netdev_callback, port_vlan, is_register,
                                  &trap_was_removed);

        if (err) {
            sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "error %s, Failed de registering on "
                        "0x%x syndrome.\n", netdev->name, synd);
            goto out;
        }
    }

    if (trap_was_removed == 0) {
        goto out;
    }

    for (i = 0; i < net_priv->num_of_traps[uc_type]; i++) {
        if (net_priv->trap_ids[uc_type][i].synd == synd) {
            sxd_log_info(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "The synd 0x%x was found and will be removed.\n",
                         synd);
            memcpy(&(net_priv->trap_ids[uc_type][i]),
                   &(net_priv->trap_ids[uc_type][net_priv->num_of_traps[uc_type] - 1]),
                   sizeof(struct net_port_vlan_info));
            net_priv->trap_ids[uc_type][net_priv->num_of_traps[uc_type] - 1].synd = SX_INVALID_TRAP_ID;
            net_priv->num_of_traps[uc_type]--;
            break;
        }
    }

out:
    dev_netdev_priv_put(priv);
    return err;
}

static void sx_netdev_set_synd_l3(struct sx_dev       *dev,
                                  void                *rsc,
                                  u16                  synd,
                                  enum sx_dev_event    event,
                                  union sx_event_data *event_data)
{
    struct sx_netdev_rsc      *resources = rsc;
    int                        i;
    int                        err;
    union ku_filter_critireas  crit;
    struct ku_port_vlan_params port_vlan;
    struct sx_dev_netdev_priv *priv;

    mutex_lock(&resources->rsc_lock);
    if (resources->in_attach_detach == 1) {
        mutex_unlock(&resources->rsc_lock);
        sxd_log_err(dev, "Can't set l3 synd %d during attach/detach \n", synd);
        return;
    }

    priv = dev_netdev_priv_from_dev(dev);
    if (!priv) {
        return;
    }

    memset(&port_vlan, 0, sizeof(port_vlan));
    switch (event_data->eth_l3_synd.type) {
    case L3_SYND_TYPE_GLOBAL:
        port_vlan.port_vlan_type = KU_PORT_VLAN_PARAMS_TYPE_GLOBAL;
        break;

    case L3_SYND_TYPE_PORT:
        port_vlan.port_vlan_type = KU_PORT_VLAN_PARAMS_TYPE_GLOBAL;
        port_vlan.sysport = event_data->eth_l3_synd.port;
        break;

    case L3_SYND_TYPE_LAG:
        port_vlan.port_vlan_type = KU_PORT_VLAN_PARAMS_TYPE_GLOBAL;
        port_vlan.lag_id = event_data->eth_l3_synd.port;
        break;

    case L3_SYND_TYPE_VLAN:
        port_vlan.port_vlan_type = KU_PORT_VLAN_PARAMS_TYPE_GLOBAL;
        port_vlan.vlan = event_data->eth_l3_synd.vlan;
        break;
    }

    /* go over all per swid devices */
    for (i = 0; i < NUMBER_OF_SWIDS; i++) {
        if (resources->sx_netdevs[i] != NULL) {
            if (event == SX_DEV_EVENT_ADD_SYND_NETDEV) {
                __sx_netdev_dev_synd_add(resources->sx_netdevs[i],
                                         USER_CHANNEL_L3_NETDEV,
                                         rsc,
                                         synd,
                                         &port_vlan,
                                         event_data->eth_l3_synd.is_register);
            } else {
                __sx_netdev_dev_synd_remove(resources->sx_netdevs[i],
                                            USER_CHANNEL_L3_NETDEV,
                                            rsc,
                                            synd,
                                            &port_vlan,
                                            event_data->eth_l3_synd.is_register);
            }
        }
    }

    /* register the listener according to the swid */
    memset(&crit, 0, sizeof(crit)); /* sets all ETH values to default */
    crit.eth.from_rp = IS_RP_FROM_RP_E;
    if (event == SX_DEV_EVENT_ADD_SYND_NETDEV) {
        err = sx_core_add_synd(0, synd, L2_TYPE_ETH, NETDEV_DRV_NAME, 0,
                               &crit, "NETDEV (Global)", sx_netdev_handle_global_pkt, NULL,
                               CHECK_DUP_DISABLED_E, dev, &port_vlan, event_data->eth_l3_synd.is_register);
        if (err) {
            sxd_log_err(dev, "Failed registering RP rx_handle on "
                        "syndrome %d.\n", synd);
        }
    } else {
        err = sx_core_remove_synd(0,
                                  synd,
                                  L2_TYPE_ETH,
                                  0,
                                  &crit,
                                  NULL,
                                  dev,
                                  sx_netdev_handle_global_pkt,
                                  &port_vlan,
                                  event_data->eth_l3_synd.is_register, NULL);
        if (err) {
            sxd_log_err(dev, "Failed unregistering RP rx_handle on "
                        "syndrome %d.\n", synd);
        }
    }

    /* register the listener according to the swid */
    memset(&crit, 0, sizeof(crit)); /* sets all ETH values to default */
    crit.eth.from_bridge = IS_BRIDGE_FROM_BRIDGE_E;
    if (event == SX_DEV_EVENT_ADD_SYND_NETDEV) {
        err = sx_core_add_synd(0, synd, L2_TYPE_ETH, NETDEV_DRV_NAME, 0,
                               &crit, "NETDEV (Global)", sx_netdev_handle_global_pkt, NULL,
                               CHECK_DUP_DISABLED_E, dev, &port_vlan, event_data->eth_l3_synd.is_register);
        if (err) {
            sxd_log_err(dev, "Failed registering bridge rx_handle on "
                        "syndrome %d.\n", synd);
        }
    } else {
        err = sx_core_remove_synd(0,
                                  synd,
                                  L2_TYPE_ETH,
                                  0,
                                  &crit,
                                  NULL,
                                  dev,
                                  sx_netdev_handle_global_pkt,
                                  &port_vlan,
                                  event_data->eth_l3_synd.is_register, NULL);
        if (err) {
            sxd_log_err(dev, "Failed unregistering bridge rx_handle on "
                        "syndrome %d.\n", synd);
        }
    }

    mutex_unlock(&resources->rsc_lock);
    dev_netdev_priv_put(priv);
}

static void sx_netdev_set_synd_l2(struct sx_dev       *dev,
                                  void                *rsc,
                                  u16                  synd,
                                  enum sx_dev_event    event,
                                  union sx_event_data *event_data)
{
    struct sx_netdev_rsc      *resources = rsc;
    int                        i;
    struct ku_port_vlan_params port_vlan;

    mutex_lock(&resources->rsc_lock);
    if (resources->in_attach_detach == 1) {
        mutex_unlock(&resources->rsc_lock);
        sxd_log_err(dev, "Can't set l2 synd %d during attach/detach \n", synd);
        return;
    }

    memset(&port_vlan, 0, sizeof(port_vlan));
    switch (event_data->eth_l3_synd.type) {
    case L3_SYND_TYPE_GLOBAL:
        port_vlan.port_vlan_type = KU_PORT_VLAN_PARAMS_TYPE_GLOBAL;
        break;

    case L3_SYND_TYPE_PORT:
        port_vlan.port_vlan_type = KU_PORT_VLAN_PARAMS_TYPE_PORT;
        port_vlan.sysport = event_data->eth_l3_synd.port;
        break;

    case L3_SYND_TYPE_LAG:
        port_vlan.port_vlan_type = KU_PORT_VLAN_PARAMS_TYPE_LAG;
        port_vlan.lag_id = event_data->eth_l3_synd.port;
        break;

    case L3_SYND_TYPE_VLAN:
        port_vlan.port_vlan_type = KU_PORT_VLAN_PARAMS_TYPE_VLAN;
        port_vlan.vlan = event_data->eth_l3_synd.vlan;
        break;
    }
    /* go over all per swid devices */
    for (i = 0; i < NUMBER_OF_SWIDS; i++) {
        if (resources->sx_netdevs[i] != NULL) {
            if (event == SX_DEV_EVENT_ADD_SYND_NETDEV) {
                __sx_netdev_dev_synd_add(resources->sx_netdevs[i], USER_CHANNEL_LOG_PORT_NETDEV, rsc, synd,
                                         &port_vlan,
                                         event_data->eth_l3_synd.is_register);
            } else {
                __sx_netdev_dev_synd_remove(resources->sx_netdevs[i],
                                            USER_CHANNEL_LOG_PORT_NETDEV,
                                            rsc,
                                            synd,
                                            &port_vlan,
                                            event_data->eth_l3_synd.is_register);
            }
        }
    }
    mutex_unlock(&resources->rsc_lock);
}

static void sx_netdev_set_synd_phy(struct sx_dev       *dev,
                                   void                *rsc,
                                   u16                  synd,
                                   enum sx_dev_event    event,
                                   union sx_event_data *event_data)
{
    struct sx_netdev_rsc      *resources = rsc;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);
    int                        i;
    struct ku_port_vlan_params port_vlan;

    if (!priv) {
        return;
    }

    mutex_lock(&resources->rsc_lock);
    if (resources->in_attach_detach == 1) {
        mutex_unlock(&priv->netdev_resources.rsc_lock);
        sxd_log_err(dev, "Can't set phy synd %d during attach/detach \n", synd);
        goto out;
    }

    memset(&port_vlan, 0, sizeof(port_vlan));
    switch (event_data->eth_l3_synd.type) {
    case L3_SYND_TYPE_GLOBAL:
        port_vlan.port_vlan_type = KU_PORT_VLAN_PARAMS_TYPE_GLOBAL;
        break;

    case L3_SYND_TYPE_PORT:
        port_vlan.port_vlan_type = KU_PORT_VLAN_PARAMS_TYPE_PORT;
        port_vlan.sysport = event_data->eth_l3_synd.port;
        break;

    case L3_SYND_TYPE_LAG:
        port_vlan.port_vlan_type = KU_PORT_VLAN_PARAMS_TYPE_LAG;
        port_vlan.lag_id = event_data->eth_l3_synd.port;
        break;

    case L3_SYND_TYPE_VLAN:
        port_vlan.port_vlan_type = KU_PORT_VLAN_PARAMS_TYPE_VLAN;
        port_vlan.vlan = event_data->eth_l3_synd.vlan;
        break;
    }

    /* go over all per swid devices */
    for (i = 0; i < NUMBER_OF_SWIDS; i++) {
        if (resources->sx_netdevs[i] != NULL) {
            if (event == SX_DEV_EVENT_ADD_SYND_NETDEV) {
                __sx_netdev_dev_synd_add(resources->sx_netdevs[i], USER_CHANNEL_PHY_PORT_NETDEV, rsc, synd,
                                         &port_vlan, event_data->eth_l3_synd.is_register);
            } else {
                __sx_netdev_dev_synd_remove(resources->sx_netdevs[i],
                                            USER_CHANNEL_PHY_PORT_NETDEV,
                                            rsc,
                                            synd,
                                            &port_vlan, event_data->eth_l3_synd.is_register);
            }
        }
    }
    mutex_unlock(&resources->rsc_lock);

out:
    dev_netdev_priv_put(priv);
}

static void __sx_netdev_user_netdev_add_synd(struct sx_dev *dev, struct net_device *netdev, u16 synd, u8 is_register)
{
    int                       err = 0;
    union ku_filter_critireas crit;
    cq_handler                netdev_callback = 0;
    char                      synd_str[SX_LISTENER_HANDLER_STR_MAX_LEN] = { 0 };


    /* if netdev is active (opened) than config the synd, else the synd will be added after dev open */
    if (netdev->flags & IFF_UP) {
        err = __sx_netdev_uc_type_get_data(USER_CHANNEL_USER_NETDEV,
                                           &crit,
                                           &netdev_callback,
                                           synd_str,
                                           sizeof(synd_str));
        if (err) {
            sxd_log_err(dev, "%s: Failed get crit and handler for "
                        "user channel USER_CHANNEL_USER_NETDEV.\n", __func__);
            goto out;
        }
    }

    /* register the listener according to the swid */
    err = sx_core_add_synd(0,  /*swid */
                           synd,
                           L2_TYPE_ETH,
                           NETDEV_DRV_NAME,
                           0,
                           &crit,
                           synd_str,
                           netdev_callback,
                           netdev,
                           CHECK_DUP_ENABLED_E,
                           dev,
                           NULL, /*port_vlan */
                           is_register);

    if (err) {
        sxd_log_err(dev, "error %s, Failed registering on "
                    "0x%x syndrome.\n", netdev->name, synd);
        goto out;
    }
out:
    return;
}


static int __sx_netdev_user_netdev_remove_synd(struct sx_dev *dev, struct net_device *netdev, u16 synd, u8 is_register)
{
    int                       err = 0;
    u8                        trap_was_removed = 0;
    union ku_filter_critireas crit;
    cq_handler                netdev_callback = 0;
    char                      synd_str[SX_LISTENER_HANDLER_STR_MAX_LEN] = { 0 };

    /* remove synd from sx_core driver */
    if (netdev->flags & IFF_UP) {
        err = __sx_netdev_uc_type_get_data(USER_CHANNEL_USER_NETDEV,
                                           &crit,
                                           &netdev_callback,
                                           synd_str,
                                           sizeof(synd_str));
        if (err) {
            sxd_log_err(dev, "%s: Failed get crit and handler for "
                        "user channel USER_CHANNEL_USER_NETDEV.\n", __func__);
            goto out;
        }

        err = sx_core_remove_synd(0, synd, L2_TYPE_ETH, 0,
                                  &crit, netdev, dev, netdev_callback, NULL /*port_vlan*/, is_register,
                                  &trap_was_removed);

        if (err) {
            sxd_log_err(dev, "error %s, Failed de registering on "
                        "0x%x syndrome, trap_was_removed %u.\n", netdev->name, synd, trap_was_removed);
            goto out;
        }
    }

out:
    return err;
}

static void __sx_netdev_set_synd_user_netdev(struct sx_dev       *dev,
                                             void                *rsc,
                                             u16                  synd,
                                             enum sx_dev_event    event,
                                             union sx_event_data *event_data)
{
    struct sx_netdev_rsc      *resources = rsc;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);
    struct net_device         *user_netdev = NULL;

    if (!priv) {
        sxd_log_err(dev, "Device not found for synd %d\n", synd);
        return;
    }

    mutex_lock(&resources->rsc_lock);
    if (resources->in_attach_detach == 1) {
        sxd_log_err(dev, "Can't set user netdev synd %d during attach/detach \n", synd);
        goto out;
    }

    if (event == SX_DEV_EVENT_ADD_SYND_NETDEV) {
        /*We get here after the netdev look up so we  should have the netdev waiting for us in the <netdev,ctx> list */
        /*we don't want to release the reference, as long as the listener is valid we should hold the reference */
        user_netdev = __sx_user_ctx_find_and_remove_netdev_for_current(priv);
        if (!user_netdev) {
            sxd_log_err(dev,
                        "Can't set user netdev synd %d, netdev not found , pid [%d]\n",
                        synd,
                        task_pid_nr(current));
            goto out;
        }

        /* Register the synd */
        __sx_netdev_user_netdev_add_synd(dev, user_netdev, synd, event_data->eth_user_netdev_synd.is_register);

        /* Add the netdev to the list of hold netdevs*/
        __sx_user_netdev_list_add(priv, user_netdev);
    } else {
        /* put the reference acquire during lookup of DEREGISTER */
        user_netdev = __sx_user_ctx_find_remove_and_put_netdev_for_current(priv);
        if (!user_netdev) {
            sxd_log_err(dev,
                        "Can't remove user netdev synd %d, netdev not found , pid [%d]\n",
                        synd,
                        task_pid_nr(current));
            goto out;
        }
        /* Deregister the synd */
        __sx_netdev_user_netdev_remove_synd(dev,
                                            user_netdev,
                                            synd,
                                            event_data->eth_user_netdev_synd.is_register);

        dev_put(user_netdev);  /* remove reference acquired during lookup at REGISTER */

        /* Remove the netdev to the list of hold netdevs*/
        __sx_user_netdev_list_remove(priv, user_netdev);

        sxd_log_debug(dev,
                      "__sx_netdev_set_synd_user_netdev: removed synd user netdev and reference released, synd %d, pid [%d] \n",
                      synd,
                      task_pid_nr(current));
    }

out:
    mutex_unlock(&resources->rsc_lock);
    dev_netdev_priv_put(priv);
    return;
}

static void sx_netdev_get_trap_info(struct sx_dev *dev, void *rsc, union sx_event_data *event_data)
{
    struct sx_netdev_rsc *resources = rsc;
    int                   i, j;
    struct sx_net_priv   *net_priv;
    uint8_t               uc_type = 0;

    mutex_lock(&resources->rsc_lock);

    for (i = 0; i < NUMBER_OF_SWIDS; i++) {
        if (resources->sx_netdevs[i] != NULL) {
            net_priv = netdev_priv(resources->sx_netdevs[i]);
            for (uc_type = USER_CHANNEL_L3_NETDEV; uc_type < NUM_OF_NET_DEV_TYPE; uc_type++) {
                event_data->netdev_trap_info.num_of_traps[uc_type] = net_priv->num_of_traps[uc_type];
                for (j = 0; j < net_priv->num_of_traps[uc_type]; j++) {
                    event_data->netdev_trap_info.trap_ids[uc_type][j] =
                        net_priv->trap_ids[uc_type][j].synd;
                    event_data->netdev_trap_info.port_vlan_params[uc_type][j].port_vlan_type =
                        (ku_port_vlan_params_type_t)net_priv->trap_ids[uc_type][j].info_type;
                    event_data->netdev_trap_info.port_vlan_params[uc_type][j].sysport =
                        net_priv->trap_ids[uc_type][j].sysport;
                    event_data->netdev_trap_info.port_vlan_params[uc_type][j].lag_id =
                        net_priv->trap_ids[uc_type][j].lag_id;
                    event_data->netdev_trap_info.port_vlan_params[uc_type][j].vlan =
                        net_priv->trap_ids[uc_type][j].vlan;
                }
            }
            /*Since the traps are registered for each swid netdev, we only need to get trap info from one swid netdev.*/
            break;
        }
    }

    mutex_unlock(&resources->rsc_lock);
}

static void sx_netdev_module_sysfs_create(void *rsc)
{
    struct net_device    *netdev;
    struct sx_netdev_rsc *resources = rsc;
    int                   port = 0;
    int                   i = 0;
    int                   err;
    struct sx_net_priv   *net_priv;

    mutex_lock(&resources->rsc_lock);
    if (resources->create_module_sysfs) {
        goto out;
    }
    resources->create_module_sysfs = 1;
    for (port = 0; port < MAX_SYSPORT_NUM; port++) {
        for (i = 0; i < MAX_PORT_NETDEV_NUM; i++) {
            netdev = resources->port_netdev[PORT_TYPE_SINGLE][port][i];
            if (netdev == NULL) {
                continue;
            }

            net_priv = netdev_priv(netdev);
            if (net_priv->kobj[SX_NET_KOBJECT_MODULE] != NULL) {
                continue;
            }

            err = sx_netdev_create_module_sysfs_nodes(netdev);
            if (err) {
                sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(net_priv), "Failed to create module sysfs nodes for netdev\n");
                goto err_out;
            }
        }
    }

    goto out;

err_out:
    for (i--; i >= 0; i--) {
        netdev = resources->port_netdev[PORT_TYPE_SINGLE][port][i];
        if (netdev != NULL) {
            sx_netdev_delete_module_sysfs_nodes(netdev);
        }
    }
    for (port--; port >= 0; port--) {
        for (i = 0; i < MAX_PORT_NETDEV_NUM; i++) {
            netdev = resources->port_netdev[PORT_TYPE_SINGLE][port][i];
            if (netdev != NULL) {
                sx_netdev_delete_module_sysfs_nodes(netdev);
            }
        }
    }

out:
    mutex_unlock(&resources->rsc_lock);
}

static int sx_netdev_event(struct sx_dev       *dev,
                           void                *resources,
                           enum sx_dev_event    event,
                           union sx_event_data *event_data)
{
    int err = 0;

    switch (event) {
    case SX_DEV_EVENT_ETH_SWID_UP:
        sxd_log_debug(dev, "sx_netdev_event :  sx_netdev_event: Got ETH swid up event. "
                      "dev = %p, dev_id = %u, swid = %d, "
                      "synd = 0x%x, mac = 0x%llx\n",
                      dev, dev->device_id, event_data->eth_swid_up.swid,
                      event_data->eth_swid_up.synd, event_data->eth_swid_up.mac);
        err = sx_netdev_event_swid_up(dev, resources, event_data->eth_swid_up.swid,
                                      event_data->eth_swid_up.synd, event_data->eth_swid_up.mac);
        break;

    case SX_DEV_EVENT_ETH_SWID_DOWN:
        sxd_log_debug(dev, "sx_netdev_event: Got ETH swid down event. "
                      "dev = %p, dev_id = %u, swid = %d\n",
                      dev, dev->device_id, event_data->eth_swid_down.swid);
        sx_netdev_remove_one_netdev(dev, resources, event_data->eth_swid_down.swid);
        break;

    case SX_DEV_EVENT_ADD_SYND_NETDEV:
        sxd_log_debug(dev, "sx_netdev_event: Got ADD_SYND_L3 event. "
                      "dev = %p, dev_id = %u, trap_id = 0x%x\n",
                      dev, dev->device_id, event_data->eth_l3_synd.hw_synd);
        sx_netdev_set_synd_l3(dev, resources, event_data->eth_l3_synd.hw_synd,
                              SX_DEV_EVENT_ADD_SYND_NETDEV, event_data);
        break;

    case SX_DEV_EVENT_REMOVE_SYND_NETDEV:
        sxd_log_debug(dev, "sx_netdev_event: Got REMOVE_SYND_L3 event. "
                      "dev = %p, dev_id = %u, trap_id = 0x%x\n",
                      dev, dev->device_id, event_data->eth_l3_synd.hw_synd);
        sx_netdev_set_synd_l3(dev, resources, event_data->eth_l3_synd.hw_synd,
                              SX_DEV_EVENT_REMOVE_SYND_NETDEV, event_data);
        break;

    case SX_DEV_EVENT_ADD_SYND_L2_NETDEV:
        sxd_log_debug(dev, "sx_netdev_event: Got ADD_SYND_L2 event. "
                      "dev = %p, dev_id = %u, trap_id = 0x%x\n",
                      dev, dev->device_id, event_data->eth_l3_synd.hw_synd);
        sx_netdev_set_synd_l2(dev, resources, event_data->eth_l3_synd.hw_synd,
                              SX_DEV_EVENT_ADD_SYND_NETDEV, event_data);
        break;

    case SX_DEV_EVENT_REMOVE_SYND_L2_NETDEV:
        sxd_log_debug(dev, "sx_netdev_event: Got REMOVE_SYND_L2 event. "
                      "dev = %p, dev_id = %u, trap_id = 0x%x\n",
                      dev, dev->device_id, event_data->eth_l3_synd.hw_synd);
        sx_netdev_set_synd_l2(dev, resources, event_data->eth_l3_synd.hw_synd,
                              SX_DEV_EVENT_REMOVE_SYND_NETDEV, event_data);
        break;

    case SX_DEV_EVENT_ADD_SYND_PHY_NETDEV:
        sxd_log_debug(dev, "sx_netdev_event: Got ADD_SYND_PHY event. "
                      "dev = %p, dev_id = %u, trap_id = 0x%x\n",
                      dev, dev->device_id, event_data->eth_l3_synd.hw_synd);
        sx_netdev_set_synd_phy(dev, resources, event_data->eth_l3_synd.hw_synd,
                               SX_DEV_EVENT_ADD_SYND_NETDEV, event_data);
        break;

    case SX_DEV_EVENT_REMOVE_SYND_PHY_NETDEV:
        sxd_log_debug(dev, "sx_netdev_event: Got REMOVE_SYND_PHY event. "
                      "dev = %p, dev_id = %u, trap_id = 0x%x\n",
                      dev, dev->device_id, event_data->eth_l3_synd.hw_synd);
        sx_netdev_set_synd_phy(dev, resources, event_data->eth_l3_synd.hw_synd,
                               SX_DEV_EVENT_REMOVE_SYND_NETDEV, event_data);
        break;

    case SX_DEV_EVENT_LAG_OPER_STATE_UPDATE:
        sxd_log_debug(dev, "sx_netdev_event: Got LAG_OPER_STATE_UPDATE event."
                      " dev = %p, dev_id = %u, lag_id = %u, oper_state = %u\n",
                      dev, dev->device_id, event_data->lag_oper_state_set.lag_id,
                      event_data->lag_oper_state_set.oper_state);
        __sx_netdev_set_lag_oper_state(dev, event_data->lag_oper_state_set.lag_id,
                                       event_data->lag_oper_state_set.oper_state);
        break;

    case SX_DEV_EVENT_OFFLOAD_MARK_SET:
        sxd_log_debug(dev, "sx_netdev_event: Got SX_DEV_EVENT_OFFLOAD_MARK_SET event."
                      " dev = %p, dev_id = %u, event_data->offload_mark_set.offload_mark_set_en = %u\n",
                      dev, dev->device_id, event_data->offload_mark_set.offload_mark_set_en);
        offload_fwd_mark_en = event_data->offload_mark_set.offload_mark_set_en;
        break;

    case SX_DEV_EVENT_GET_NETDEV_TRAP_INFO:
        sxd_log_debug(dev, "sx_netdev_event: Got SX_DEV_EVENT_GET_NETDEV_TRAP_INFO event.\n");
        sx_netdev_get_trap_info(dev, resources, event_data);
        break;

    case SX_DEV_EVENT_ADD_SYND_PSAMPLE:
        sxd_log_info(dev, "sx_netdev_event: Got ADD_SYND_PSAMPLE event. "
                     "dev = %p, dev_id = %u, trap_id = 0x%x\n",
                     dev, dev->device_id, event_data->psample_synd.hw_synd);
        err = sx_psample_set_synd(dev, event_data->psample_synd.hw_synd,
                                  SX_DEV_EVENT_ADD_SYND_PSAMPLE, event_data);
        break;

    case SX_DEV_EVENT_REMOVE_SYND_PSAMPLE:
        sxd_log_info(dev, "sx_netdev_event: Got REMOVE_SYND_PSAMPLE event. "
                     "dev = %p, dev_id = %u, trap_id = 0x%x\n",
                     dev, dev->device_id, event_data->psample_synd.hw_synd);
        err = sx_psample_set_synd(dev, event_data->psample_synd.hw_synd,
                                  SX_DEV_EVENT_REMOVE_SYND_PSAMPLE, event_data);
        break;

    case SX_DEV_EVENT_UPDATE_SAMPLE_RATE:
        sxd_log_info(dev, "sx_netdev_event: got psample-sample-rate update event (type=%u).",
                     event_data->psample_sample_rate.rate_type);
        err = sx_psample_rate_set(dev, &event_data->psample_sample_rate);
        break;

    case SX_DEV_EVENT_MODULE_SYSFS_CREATE:
        sxd_log_info(dev, "sx_netdev_event: Got MODULE_SYSFS_CREATE event.\n");
        sx_netdev_module_sysfs_create(resources);
        break;

    case SX_DEV_EVENT_NETDEV_LOOKUP_ACQUIRE:
        sxd_log_debug(dev, "sx_netdev_event: Got NETDEV_LOOKUP_ACQUIRE event"
                      "dev = %p, dev_id = %u, ifindex=%d\n",
                      dev,
                      dev->device_id,
                      event_data->netdev_lookup.ifindex);
        err = __sx_netdev_lookup_netdev_by_ifindex_in_netns(dev, event_data->netdev_lookup.ifindex);
        break;

    case SX_DEV_EVENT_ADD_SYND_USER_NETDEV:
        sxd_log_debug(dev, "sx_netdev_event: Got ADD_SYND_USER_NETDEV event. "
                      "dev = %p, dev_id = %u, trap_id = 0x%x\n",
                      dev, dev->device_id, event_data->eth_user_netdev_synd.hw_synd);
        __sx_netdev_set_synd_user_netdev(dev, resources, event_data->eth_user_netdev_synd.hw_synd,
                                         SX_DEV_EVENT_ADD_SYND_NETDEV, event_data);
        break;

    case SX_DEV_EVENT_REMOVE_SYND_USER_NETDEV:
        sxd_log_debug(dev, "sx_netdev_event: Got REMOVE_SYND_UDR_NETDEV event. "
                      "dev = %p, dev_id = %u, trap_id = 0x%x\n",
                      dev, dev->device_id, event_data->eth_user_netdev_synd.hw_synd);
        __sx_netdev_set_synd_user_netdev(dev, resources, event_data->eth_user_netdev_synd.hw_synd,
                                         SX_DEV_EVENT_REMOVE_SYND_NETDEV, event_data);
        break;


    case SX_DEV_EVENT_CATASTROPHIC_ERROR:
    default:
        break;
    }
    return err;
}

static struct sx_interface sx_netdev_interface = {
    .add = sx_netdev_init_one,
    .remove = sx_netdev_remove_one,
    .event = sx_netdev_event
};

#ifndef SX_DROP_MONITOR_DISABLED
/* MUST BE CALLED UNDER RCU_LOCK */
static struct net_device* __sx_netdev_get_netdev(int port_type, u16 port, struct sx_dev *dev)
{
    int                        i;
    struct net_device         *netdev = NULL;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(dev);

    if (!priv) {
        return NULL;
    }

    for (i = 0; i < MAX_PORT_NETDEV_NUM; i++) {
        netdev = rcu_dereference(priv->netdev_resources.port_netdev[port_type][port][i]);
        if (netdev) {
            break;
        }
    }

    dev_netdev_priv_put(priv);
    return netdev;
}

/* MUST BE CALLED UNDER RCU_LOCK */
static struct net_device* sx_netdev_get_netdev(struct completion_info *comp_info)
{
    int                        ret;
    u16                        local_port;
    struct net_device         *netdev = NULL;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(comp_info->dev);

    if (!priv) {
        return netdev;
    }

    if (comp_info->cqe_params.is_lag) {
        ret = sx_core_get_local(comp_info->dev,
                                comp_info->cqe_params.sysport_lag_id,
                                comp_info->cqe_params.lag_subport,
                                &local_port);

        if (ret) {
            goto out;
        }

        /* When the packet comes from a member port of a LAG, try to get
         * the member port netdev first, if no member port netdev has been
         * found, then try to get the LAG netdev.
         */
        netdev = __sx_netdev_get_netdev(PORT_TYPE_SINGLE, local_port, comp_info->dev);
        if (netdev) {
            goto out;
        }
    }

    netdev = __sx_netdev_get_netdev(comp_info->cqe_params.is_lag ? PORT_TYPE_LAG : PORT_TYPE_SINGLE,
                                    comp_info->cqe_params.sysport_lag_id, comp_info->dev);

out:
    dev_netdev_priv_put(priv);
    return netdev;
}
#endif /* SX_DROP_MONITOR_DISABLED */

static void __skip_tunnel_default(u8 skip_tunnel)
{
    struct net_device         *netdev = NULL;
    struct sx_net_priv        *net_priv = NULL;
    int                        i;
    int                        port_type = 0;
    int                        port = 0, br_id = 0;
    int                        swid = 0;
    int                        max_ports[] = {
        [PORT_TYPE_SINGLE] = MAX_SYSPORT_NUM,
        [PORT_TYPE_LAG] = MAX_LAG_NUM
    };
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_curr_net_namespace();

    if (!priv) {
        return;
    }

    sxd_log_info(SXD_DEV_FROM_NETDEV_PRIV(priv), "Set default skip_tunnel (%d)\n", skip_tunnel);
    priv->skip_tunnel = skip_tunnel;

    mutex_lock(&priv->netdev_resources.rsc_lock);

    for (swid = 0; swid < NUMBER_OF_SWIDS; swid++) {
        if (!priv->netdev_resources.allocated[swid]) {
            continue;
        }

        netdev = priv->netdev_resources.sx_netdevs[swid];
        if (netdev != NULL) {
            sxd_log_info(SXD_DEV_FROM_NETDEV_PRIV(priv),
                         "Set skip_tunnel (%d) for device '%s'\n",
                         skip_tunnel,
                         netdev->name);
            net_priv = netdev_priv(netdev);
            net_priv->skip_tunnel = skip_tunnel;
        }
    }

    for (port_type = 0; port_type < PORT_TYPE_NUM; port_type++) {
        for (port = 0; port < max_ports[port_type]; port++) {
            if (priv->netdev_resources.port_allocated[port_type][port] == 0) {
                continue;
            }
            for (i = 0; i < MAX_PORT_NETDEV_NUM; i++) {
                netdev = priv->netdev_resources.port_netdev[port_type][port][i];
                if (netdev != NULL) {
                    sxd_log_info(SXD_DEV_FROM_NETDEV_PRIV(priv),
                                 "Set skip_tunnel (%d) for device '%s'\n",
                                 skip_tunnel,
                                 netdev->name);
                    net_priv = netdev_priv(netdev);
                    net_priv->skip_tunnel = skip_tunnel;
                }
            }
        }
    }

    for (br_id = 0; br_id < MAX_BRIDGE_NUM; br_id++) {
        netdev = priv->bridge_netdev_db[br_id];
        if (netdev != NULL) {
            sxd_log_info(SXD_DEV_FROM_NETDEV_PRIV(priv),
                         "Set skip_tunnel (%d) for device '%s'\n",
                         skip_tunnel,
                         netdev->name);
            net_priv = netdev_priv(netdev);
            net_priv->skip_tunnel = skip_tunnel;
        }
    }

    mutex_unlock(&priv->netdev_resources.rsc_lock);
    dev_netdev_priv_put(priv);
}

static void __skip_tunnel_dev(char* dev_name, u8 skip_tunnel)
{
    struct net_device  *netdev = NULL;
    struct sx_net_priv *net_priv = NULL;
    struct net         *net = get_curr_net_namespace();

    if (dev_name == NULL) {
        sxd_log_err(SXD_DEV_NONE, "Skip tunnel set failed: no <dev_name>\n");
        return;
    }

    netdev = dev_get_by_name(net, dev_name);
    if (netdev == NULL) {
        sxd_log_err(SXD_DEV_NONE, "Skip tunnel set failed: Device name '%s' wasn't found\n", dev_name);
        return;
    }

    sxd_log_info(SXD_DEV_NONE, "Set skip_tunnel (%d) for net device '%s'\n", skip_tunnel, dev_name);
    net_priv = netdev_priv(netdev);
    net_priv->skip_tunnel = skip_tunnel;
    dev_put(netdev);
}

static char * __sx_netdev_proc_str_get_u32(char *buffer, u32 *val32)
{
    const char delimiters[] = " .,;:!-";
    char      *running;
    char      *token;

    running = (char*)buffer;
    token = strsep(&running, delimiters);
    if (token == NULL) {
        *val32 = 0;
        return NULL;
    }

    if (strstr(token, "0x")) {
        *val32 = simple_strtol(token, NULL, 16);
    } else {
        *val32 = simple_strtol(token, NULL, 10);
    }

    return running;
}

static char * __sx_netdev_proc_str_get_str(char *buffer, char **str)
{
    const char delimiters[] = " ,;:!-";
    char      *running;
    char      *token;

    running = buffer;
    token = strsep(&running, delimiters);
    if (token == NULL) {
        *str = 0;
        return NULL;
    }

    *str = token;
    return running;
}

static ssize_t skip_tunnel_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device         *netdev = NULL;
    struct sx_net_priv        *net_priv = NULL;
    int                        i;
    int                        port_type = 0;
    int                        port = 0, br_id = 0;
    int                        swid = 0;
    int                        max_ports[] = {
        [PORT_TYPE_SINGLE] = MAX_SYSPORT_NUM,
        [PORT_TYPE_LAG] = MAX_LAG_NUM
    };
    int                        offset = 0;
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_curr_net_namespace();

    if (!priv) {
        return -ENODEV;
    }

    offset = sprintf(buf, "%-15s | %s\n", "dev_name", "skip_tunnel");
    offset += sprintf(buf + offset, "------------------------------\n");
    offset += sprintf(buf + offset, "%-15s | %d\n", "default", priv->skip_tunnel);

    mutex_lock(&priv->netdev_resources.rsc_lock);

    for (swid = 0; swid < NUMBER_OF_SWIDS; swid++) {
        if (!priv->netdev_resources.allocated[swid]) {
            continue;
        }

        netdev = priv->netdev_resources.sx_netdevs[swid];
        if (netdev != NULL) {
            net_priv = netdev_priv(netdev);
            offset += sprintf(buf + offset, "%-15s | %d\n", netdev->name, net_priv->skip_tunnel);
        }
    }

    for (port_type = 0; port_type < PORT_TYPE_NUM; port_type++) {
        for (port = 0; port < max_ports[port_type]; port++) {
            if (priv->netdev_resources.port_allocated[port_type][port] == 0) {
                continue;
            }
            for (i = 0; i < MAX_PORT_NETDEV_NUM; i++) {
                netdev = priv->netdev_resources.port_netdev[port_type][port][i];
                if (netdev != NULL) {
                    net_priv = netdev_priv(netdev);
                    offset += sprintf(buf + offset, "%-15s | %d\n", netdev->name, net_priv->skip_tunnel);
                }
            }
        }
    }

    for (br_id = 0; br_id < MAX_BRIDGE_NUM; br_id++) {
        netdev = priv->bridge_netdev_db[br_id];
        if (netdev != NULL) {
            net_priv = netdev_priv(netdev);
            offset += sprintf(buf + offset, "%-15s | %d\n", netdev->name, net_priv->skip_tunnel);
        }
    }

    mutex_unlock(&priv->netdev_resources.rsc_lock);

    dev_netdev_priv_put(priv);
    return offset;
}

static ssize_t skip_tunnel_cb(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len)
{
    int   cmd = 0;
    char *p = NULL;
    char *running = NULL;
    char *cmd_str = NULL;
    u32   skip_tunnel = 0;
    char *dev_name = NULL;

    running = (char*)buf;

    running = __sx_netdev_proc_str_get_str(running, &cmd_str);
    p = strstr(cmd_str, "default");
    if (p != NULL) {
        running = __sx_netdev_proc_str_get_u32(running, &skip_tunnel);
        __skip_tunnel_default(skip_tunnel);
        cmd++;
    }

    p = strstr(cmd_str, "set");
    if (p != NULL) {
        running = __sx_netdev_proc_str_get_str(running, &dev_name);
        running = __sx_netdev_proc_str_get_u32(running, &skip_tunnel);
        __skip_tunnel_dev(dev_name, skip_tunnel);
        cmd++;
    }

    if (cmd == 0) {
        sxd_log_info(SXD_DEV_NONE, "Available Commands for skip_tunnel:\n");
        sxd_log_info(SXD_DEV_NONE, "  default [disable (0) / enable (1)] - Set the default device skip tunnel\n");
        sxd_log_info(SXD_DEV_NONE,
                     "  set [dev_name] [disable (0) / enable (1)] - Set skip tunnel for <dev_name>\n");
    }

    return len;
}

int sx_netdev_get_net_device_by_kobj(struct kobject          *kobj,
                                     struct net_device      **dev,
                                     enum sx_net_kobject_type kobj_type)
{
    int                        ret = 0;
    u8                         i = 0;
    u32                        port = 0;
    int                        port_type = 0;
    struct net_device         *netdev = NULL;
    struct sx_net_priv        *net_priv = NULL;
    int                        num_netdevs = single_netdev_per_port_enable ? 1 : MAX_PORT_NETDEV_NUM;
    int                        max_ports[] = {
        [PORT_TYPE_SINGLE] = MAX_SYSPORT_NUM,
        [PORT_TYPE_LAG] = MAX_LAG_NUM
    };
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_curr_net_namespace();

    if (!priv) {
        return -ENODEV;
    }


    if (!kobj) {
        sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(priv), "sysfs node is NULL\n");
        ret = -EINVAL;
        goto out;
    }

    mutex_lock(&priv->netdev_resources.rsc_lock);

    if (kobj_type > SX_NET_KOBJECT_MAX) {
        sxd_log_err(SXD_DEV_FROM_NETDEV_PRIV(priv), "kobject type is wrong\n");
        ret = -EINVAL;
        goto out;
    }

    for (port_type = 0; port_type < PORT_TYPE_NUM; port_type++) {
        for (port = 0; port < max_ports[port_type]; port++) {
            for (i = 0; i < num_netdevs; i++) {
                netdev = priv->netdev_resources.port_netdev[port_type][port][i];
                if (netdev != NULL) {
                    net_priv = netdev_priv(netdev);
                    if (kobj == net_priv->kobj[kobj_type]) {
                        *dev = netdev;
                        goto out;
                    }
                }
            }
        }
    }

    sxd_log_debug(SXD_DEV_FROM_NETDEV_PRIV(priv), "The net device is not existed for the kobj\n");
    ret = -ENOENT;

out:
    mutex_unlock(&priv->netdev_resources.rsc_lock);
    dev_netdev_priv_put(priv);
    return ret;
}

static ssize_t show_module_id(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                        ret = 0;
    struct net_device         *netdev = NULL;
    struct sx_net_priv        *net_priv = NULL;
    uint8_t                    slot_id = 0;
    uint8_t                    module_id = 0;
    int                        len = 0;
    struct sx_dev_netdev_priv *priv;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of module id sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE, &netdev);
    if (ret) {
        return ret;
    }

    priv = dev_netdev_priv_from_netdev(netdev);
    if (!priv) {
        return -ENODEV;
    }

    net_priv = netdev_priv(netdev);
    ret = sx_core_get_module_by_port(net_priv->dev,
                                     net_priv->port,
                                     &slot_id,
                                     &module_id);
    if (ret) {
        goto out;
    }

    len = sprintf(buf, "%u\n", module_id);

out:
    sx_netdev_sysfs_handler_end(netdev);
    dev_netdev_priv_put(priv);

    return (ret) ? ret : len;
}

static ssize_t reset_module(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len)
{
    int                ret = 0;
    int                reset = 0;
    struct net_device *netdev = NULL;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of reset sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__, buf, len);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = kstrtoint(buf, 10, &reset);
    if (ret) {
        sxd_log_info(SXD_DEV_NONE, "sysfs entry reset got invalid value\n");
        goto out;
    }

    if (!reset) {
        ret = -EINVAL;
        goto out;
    }

    ret = sx_netdev_module_reset(netdev);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "Failed in sx_netdev_module_reset\n");
        goto out;
    }

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t store_module_power_mode_policy(struct kobject        *kobj,
                                              struct kobj_attribute *attr,
                                              const char            *buf,
                                              size_t                 len)
{
    int                ret = 0;
    int                power_mode_policy = 0;
    struct net_device *netdev = NULL;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of power mode policy sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__, buf, len);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = kstrtoint(buf, 10, &power_mode_policy);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "sysfs entry power_mode_policy got invalid value\n");
        ret = -EINVAL;
        goto out;
    }

    if ((power_mode_policy != SX_MODULE_POWER_MODE_POLICY_AUTO) &&
        (power_mode_policy != SX_MODULE_POWER_MODE_POLICY_LOW) &&
        (power_mode_policy != SX_MODULE_POWER_MODE_POLICY_HIGH)) {
        sxd_log_err(SXD_DEV_NONE, "sysfs entry power_mode_policy got invalid value\n");
        ret = -EINVAL;
        goto out;
    }

    ret = sx_netdev_module_power_mode_policy_set(netdev, (enum sx_module_power_mode_policy)power_mode_policy);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "Failed in sx_netdev_module_power_mode_policy_set\n");
        goto out;
    }

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_power_mode_policy(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                                ret = 0;
    struct sx_module_power_mode_params params;
    struct net_device                 *netdev = NULL;
    int                                len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of power mode policy sysfs node is NULL\n");
        ret = -EINVAL;
        return ret;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    memset(&params, 0, sizeof(params));
    ret = sx_netdev_module_power_mode_get(netdev, &params);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "Failed in sx_netdev_module_power_mode_get\n");
        goto out;
    }

    len = sprintf(buf, "%u\n", params.policy);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_power_mode(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                                ret = 0;
    struct sx_module_power_mode_params params;
    struct net_device                 *netdev = NULL;
    int                                len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of power mode sysfs node is NULL\n");
        ret = -EINVAL;
        return ret;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    memset(&params, 0, sizeof(params));
    ret = sx_netdev_module_power_mode_get(netdev, &params);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "Failed in sx_netdev_module_power_mode_get\n");
        goto out;
    }

    len = sprintf(buf, "%u\n", params.mode);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_link_ext_state(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                           ret = 0;
    struct net_device            *netdev = NULL;
    struct sx_link_ext_state_info link_ext_state_info;
    int                           len = 0;

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_PORT, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    memset(&link_ext_state_info, 0, sizeof(link_ext_state_info));
    ret = __get_link_ext_state(netdev, &link_ext_state_info);
    if (ret) {
        sxd_log_debug(SXD_DEV_NONE, "Cannot get state info in __get_link_ext_state\n");
        goto out;
    }

    len = sprintf(buf, "%u %u\n", link_ext_state_info.link_ext_state, link_ext_state_info.__link_ext_substate);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? 0 : len;
}

static int __sx_netdev_get_module_temperature(struct net_device *netdev, struct sx_temperature_params *params)
{
    int                        ret = 0;
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return -ENODEV;
    }

    memset(params, 0, sizeof(struct sx_temperature_params));
    ret = sx_core_get_port_module_temperature(net_priv->dev,
                                              net_priv->port,
                                              params);

    dev_netdev_priv_put(priv);
    return ret;
}

static int __sx_netdev_get_module_temperature_tec(struct net_device *netdev, struct sx_tec_params *params)
{
    int                        ret = 0;
    struct sx_net_priv        *net_priv = netdev_priv(netdev);
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_netdev(netdev);

    if (!priv) {
        return -ENODEV;
    }

    memset(params, 0, sizeof(struct sx_tec_params));
    ret = sx_core_get_port_module_temperature_tec(net_priv->dev,
                                                  net_priv->port,
                                                  params);

    dev_netdev_priv_put(priv);
    return ret;
}

static ssize_t show_module_temp_input(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device           *netdev = NULL;
    int                          ret = 0;
    struct sx_temperature_params params;
    int                          len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of temp input sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_netdev_get_module_temperature(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module temperature information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%llu\n", params.input);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_temp_label(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device           *netdev = NULL;
    int                          ret = 0;
    struct sx_temperature_params params;
    int                          len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of temp label sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_netdev_get_module_temperature(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module temperature information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%s\n", params.label);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_temp_threshold_hi(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device           *netdev = NULL;
    int                          ret = 0;
    struct sx_temperature_params params;
    int                          len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of temp threshold hi sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_netdev_get_module_temperature(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module temperature high information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%d\n", params.threshold_hi);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_temp_threshold_lo(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device           *netdev = NULL;
    int                          ret = 0;
    struct sx_temperature_params params;
    int                          len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of temp threshold lo sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_netdev_get_module_temperature(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module temperature low information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%d\n", params.threshold_lo);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_tec_cooling_level(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device   *netdev = NULL;
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of reset sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP_TEC, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_netdev_get_module_temperature_tec(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module tec cooling_level information\n");
        }
        goto out;
    }


    len = sprintf(buf, "%u\n", params.cooling_level);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_tec_max_power(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device   *netdev = NULL;
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of reset sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP_TEC, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_netdev_get_module_temperature_tec(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module tec max_tec_power information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%u\n", params.max_tec_power);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_tec_power(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device   *netdev = NULL;
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of reset sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP_TEC, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }
    ret = __sx_netdev_get_module_temperature_tec(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module tec tec_power information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%u\n", params.tec_power);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_tec_power_warning_high(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device   *netdev = NULL;
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of reset sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP_TEC, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }
    ret = __sx_netdev_get_module_temperature_tec(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module tec tec_power_warning_high information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%u\n", params.tec_power_warning_high);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_tec_power_warning_low(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device   *netdev = NULL;
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of reset sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP_TEC, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }
    ret = __sx_netdev_get_module_temperature_tec(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module tec tec_power_warning_low information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%u\n", params.tec_power_warning_low);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_tec_power_alarm_high(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device   *netdev = NULL;
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of reset sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP_TEC, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }
    ret = __sx_netdev_get_module_temperature_tec(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module tec tec_power_alarm_high information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%u\n", params.tec_power_alarm_high);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_tec_power_alarm_low(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device   *netdev = NULL;
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of reset sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP_TEC, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }
    ret = __sx_netdev_get_module_temperature_tec(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module tec tec_power_alarm_low information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%u\n", params.tec_power_alarm_low);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_tec_max_set_point_temperature(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device   *netdev = NULL;
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of reset sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP_TEC, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }
    ret = __sx_netdev_get_module_temperature_tec(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module tec max_set_point_temperature information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%u\n", params.max_set_point_temperature);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_tec_set_point_temperature(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device   *netdev = NULL;
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of reset sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP_TEC, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }
    ret = __sx_netdev_get_module_temperature_tec(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module tec set_point_temperature information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%u\n", params.set_point_temperature);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_tec_set_point_temperature_warning_high(struct kobject        *kobj,
                                                                  struct kobj_attribute *attr,
                                                                  char                  *buf)
{
    struct net_device   *netdev = NULL;
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of reset sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP_TEC, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }
    ret = __sx_netdev_get_module_temperature_tec(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module tec set_point_temperature_warning_high information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%u\n", params.set_point_temperature_warning_high);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_tec_set_point_temperature_warning_low(struct kobject        *kobj,
                                                                 struct kobj_attribute *attr,
                                                                 char                  *buf)
{
    struct net_device   *netdev = NULL;
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of reset sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP_TEC, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }
    ret = __sx_netdev_get_module_temperature_tec(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module tec set_point_temperature_warning_low information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%u\n", params.set_point_temperature_warning_low);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_tec_set_point_temperature_alarm_high(struct kobject        *kobj,
                                                                struct kobj_attribute *attr,
                                                                char                  *buf)
{
    struct net_device   *netdev = NULL;
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of reset sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP_TEC, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }
    ret = __sx_netdev_get_module_temperature_tec(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module tec set_point_temperature_alarm_high information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%u\n", params.set_point_temperature_alarm_high);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_tec_set_point_temperature_alarm_low(struct kobject        *kobj,
                                                               struct kobj_attribute *attr,
                                                               char                  *buf)
{
    struct net_device   *netdev = NULL;
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of reset sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP_TEC, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }
    ret = __sx_netdev_get_module_temperature_tec(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module tec set_point_temperature_alarm_low information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%u\n", params.set_point_temperature_alarm_low);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_tec_max_cooling_level(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device   *netdev = NULL;
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of reset sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP_TEC, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_netdev_get_module_temperature_tec(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module tec max_cooling_level information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%u\n", params.max_cooling_level);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_tec_min_cooling_level(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device   *netdev = NULL;
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of reset sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP_TEC, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_netdev_get_module_temperature_tec(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module tec min_cooling_level information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%u\n", params.min_cooling_level);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_temp_threshold_critical_hi(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device           *netdev = NULL;
    int                          ret = 0;
    struct sx_temperature_params params;
    int                          len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of temp threshold critical hi sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_netdev_get_module_temperature(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module temperature threshold critical high information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%d\n", params.threshold_critical_hi);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static ssize_t show_module_temp_threshold_critical_lo(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct net_device           *netdev = NULL;
    int                          ret = 0;
    struct sx_temperature_params params;
    int                          len = 0;

    if (!kobj->parent) {
        sxd_log_err(SXD_DEV_NONE, "Parent of temp threshold critical lo sysfs node is NULL\n");
        return -EINVAL;
    }

    ret = sx_netdev_sysfs_handler_start(kobj, SX_NET_KOBJECT_MODULE_TEMP, &netdev);
    if (ret) {
        return ret;
    }

    ret = send_netdev_sniffer_event(netdev, kobj, &attr->attr, __FUNCTION__,  NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE,
                    "send_netdev_sniffer_event from sysfs netdev entry %s failed [%d]\n",
                    __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_netdev_get_module_temperature(netdev, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get module temperature threshold critical low information\n");
        }
        goto out;
    }

    len = sprintf(buf, "%d\n", params.threshold_critical_lo);

out:
    sx_netdev_sysfs_handler_end(netdev);
    return (ret) ? ret : len;
}

static int __sx_net_dm_hw_report(struct completion_info *comp_info,
                                 const char             *trap_name,
                                 const char             *trap_grp_name,
                                 const void             *fa_cookie,
                                 u8                      copy_skb)
{
#ifndef SX_DROP_MONITOR_DISABLED
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)) && !defined(CONFIG_SX_RHEL_8_6)
    struct net_dm_hw_metadata hw_metadata = { .trap_name = trap_name, .trap_group_name = trap_grp_name };
#else
    struct devlink_trap_metadata hw_metadata = {.trap_name = trap_name, .trap_group_name = trap_grp_name };
#endif
    struct sk_buff *new_skb = NULL;
    int             err = 0;

#ifdef SX_EXTEND_WJH_BUFFER_LINUX_CHANNEL
    u16                egress_port = INVALID_LOCAL_PORT;
    u16                egress_lag_subport = INVALID_LOCAL_PORT;
    struct net_device *netdev = NULL;
    int                i;
    u8                 port_type = PORT_TYPE_SINGLE;
    int                max_ports[] = {
        [PORT_TYPE_SINGLE] = MAX_PHYPORT_NUM,
        [PORT_TYPE_LAG] = MAX_LAG_NUM - 1
    };
#endif
    struct sx_dev_netdev_priv *priv = dev_netdev_priv_from_dev(comp_info->dev);
    if (!priv) {
        return -ENODEV;
    }

#ifndef SX_FLOW_OFFLOAD_DISABLED
    hw_metadata.fa_cookie = fa_cookie;
#endif

    rcu_read_lock();

    hw_metadata.input_dev = sx_netdev_get_netdev(comp_info);

    if (!hw_metadata.input_dev) {
        goto out;
    }

#ifdef SX_EXTEND_WJH_BUFFER_LINUX_CHANNEL
    hw_metadata.output_port_dev = NULL;
    hw_metadata.output_lag_dev = NULL;
    if ((comp_info->cqe_params.dest_sysport_lag_id != INVALID_LOCAL_PORT) &&
        (comp_info->cqe_params.dest_sysport_lag_id != MULTI_PORT_TYPE_OF_LOCAL_PORT)) {
        if (comp_info->cqe_params.dest_is_lag) {
            port_type = PORT_TYPE_LAG;
        }
        egress_port = comp_info->cqe_params.dest_sysport_lag_id;
        if (egress_port <= max_ports[port_type]) {
            for (i = 0; i < MAX_PORT_NETDEV_NUM; i++) {
                netdev = rcu_dereference(priv->netdev_resources.port_netdev[port_type][egress_port][i]);
                if (netdev) {
                    break;
                }
            }
        }
        if (comp_info->cqe_params.dest_is_lag) {
            hw_metadata.output_lag_dev = netdev;
        } else {
            hw_metadata.output_port_dev = netdev;
        }

        /* for lag port case */
        if (comp_info->cqe_params.dest_is_lag) {
            err = sx_core_get_local(comp_info->dev,
                                    comp_info->cqe_params.dest_sysport_lag_id,
                                    comp_info->cqe_params.dest_lag_subport,
                                    &egress_lag_subport);
            if (err) {
                egress_lag_subport = INVALID_LOCAL_PORT;
            }

            netdev = NULL;
            if (egress_lag_subport != INVALID_LOCAL_PORT) {
                if (egress_lag_subport <= max_ports[PORT_TYPE_SINGLE]) {
                    for (i = 0; i < MAX_PORT_NETDEV_NUM; i++) {
                        netdev =
                            rcu_dereference(
                                priv->netdev_resources.port_netdev[PORT_TYPE_SINGLE][egress_lag_subport][i]);
                        if (netdev) {
                            break;
                        }
                    }
                }
                hw_metadata.output_port_dev = netdev;
            }
        }
    }

    hw_metadata.out_tc_occ_valid = 0;
    hw_metadata.out_tc_occ = INVALID_LINUX_CHANNEL_MIRROR_CONG;
    if (comp_info->cqe_params.mirror_cong != INVALID_LINUX_CHANNEL_MIRROR_CONG) {
        hw_metadata.out_tc_occ_valid = 1;
        hw_metadata.out_tc_occ = comp_info->cqe_params.mirror_cong;
    }

    hw_metadata.latency_valid = 0;
    hw_metadata.latency = INVALID_LINUX_CHANNEL_MIRROR_LATENCY;
    if (comp_info->cqe_params.mirror_lantency != INVALID_LINUX_CHANNEL_MIRROR_LATENCY) {
        hw_metadata.latency_valid = 1;
        hw_metadata.latency = comp_info->cqe_params.mirror_lantency;
    }

    hw_metadata.out_tc_valid = 0;
    hw_metadata.out_tc = INVALID_LINUX_CHANNEL_MIRROR_TCLASS;
    if (comp_info->cqe_params.mirror_tclass != INVALID_LINUX_CHANNEL_MIRROR_TCLASS) {
        hw_metadata.out_tc_valid = 1;
        hw_metadata.out_tc = comp_info->cqe_params.mirror_tclass;
    }
#endif /* ifdef SX_EXTEND_WJH_BUFFER_LINUX_CHANNEL */

    new_skb = copy_skb ? skb_copy(comp_info->skb, GFP_ATOMIC) : skb_clone(comp_info->skb, GFP_ATOMIC);
    if (!new_skb) {
        sxd_log_warning(SXD_DEV_FROM_COMP_INFO(comp_info),
                        "Failed to allocate memory for %s skb",
                        copy_skb ? "copied" : "cloned");
        err = -ENOMEM;
        goto out;
    }

    trace_sx_netdev_dm_hw_report(new_skb, &hw_metadata);
    new_skb->protocol = eth_type_trans(new_skb, hw_metadata.input_dev);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)) && !defined(CONFIG_SX_RHEL_8_6) && !defined(CONFIG_SX_CENTOS_8_4)
    net_dm_hw_report(new_skb, &hw_metadata);
#elif !defined(CONFIG_SX_CENTOS_8_4)
    if (trace_devlink_trap_report_enabled()) {
        trace_devlink_trap_report(priv->devlink_dummy_instance, new_skb, &hw_metadata);
    }
#endif
out:
    rcu_read_unlock();

    if (new_skb) {
        kfree_skb(new_skb);
    }

    dev_netdev_priv_put(priv);
    return err;
#else  /* ifndef SX_DROP_MONITOR_DISABLED */
    return 0;
#endif /* ifndef SX_DROP_MONITOR_DISABLED */
}


static bool is_running_in_container(void)
{
    if (current->nsproxy->mnt_ns != init_task.nsproxy->mnt_ns) {
        return true; /* Non-root mount namespace — likely a container */
    } else {
        return false;
    }
}

static void __bind_unbind_sx_core(bool is_bind, struct sx_dev_netdev_priv *priv, bool in_pci_restart)
{
    struct sx_dev *dev = priv->dev;

    if (is_bind) {
        sxd_log_info(dev, "sx_netdev BIND start\n");

        if (priv->sx_core_if.bound) {
            sxd_log_info(dev, "sx_core already attach\n");
            return;
        }

        mutex_init(&priv->netdev_resources.rsc_lock);
        __init_sx_core_if(&priv->sx_core_if);
        sx_netdev_attach_global_event_handler(dev);
        attach_netdevs(dev);
        sx_core_register_net_dm_hw_report(__sx_net_dm_hw_report, dev);
        sx_psample_init(dev);

        sxd_log_info(dev, "sx_netdev BIND end\n");
    } else { /* unbind */
        sxd_log_info(dev, "sx_netdev UNBIND start\n");
        if (!priv->sx_core_if.bound) {
            sxd_log_info(dev, "sx_core already detached\n");
            return;
        }

        sx_core_register_net_dm_hw_report(NULL, dev);
        if ((is_running_in_container() == false) || (in_pci_restart == true)) {
            detach_netdevs(dev);
        } else {
            /*we got here from de-init flow in case of multi-asic or single asic as multi asic.
             *  we should remove the netdevs here */
            sx_netdev_remove_netdevs(dev);
        }

        sx_netdev_unregister_global_event_handler(dev);
        sx_psample_cleanup(dev);

        priv->sx_core_if.bound = false;
        sxd_log_info(dev, "sx_netdev UNBIND end\n");
    }
}

static ssize_t store_bind_sx_core(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len)
{
    int ret;
    int is_bind;

    ret = kstrtoint(buf, 10, &is_bind);
    if (ret) {
        sxd_log_info(SXD_DEV_NONE, "sysfs entry bind_sx_core got invalid value\n");
        return ret;
    }

    /* This sysfs entry is obsolete, we do nothing here.*/

    return len;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static void __sx_netdev_pernet_pre_exit(struct net *net)
{
    struct sx_dev_netdev_priv *priv = __dev_netdev_priv_from_namespace(net);
    struct sx_dev             *dev;

    /* WE SHOULD NOT GET HERE ON SINGLE-ASIC SYSTEMS! */

    if (!priv) {
        /* probably a namespace that is not related to us has been destroyed */
        return;
    }

    dev = priv->dev;
    sxd_log_notice(dev,
                   "sx_netdev: got notification for net-namespace deletion of device %u\n",
                   priv->dev->device_id);

    dev_netdev_priv_put(priv);

    sx_core_delete_pci_dev(dev);
}

static struct pernet_operations __sx_netdev_pernet_ops = {
    .pre_exit = __sx_netdev_pernet_pre_exit
};
#endif /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) */

void sx_netdev_rtnl_lock(void)
{
    rtnl_lock();
    /* rtnl_lock is a global lock.
     *  Better to move to rtnl_net_lock when the kernel will support it, (6.16 and above)
     */
}

void sx_netdev_rtnl_unlock(void)
{
    rtnl_unlock();
    /* rtnl_unlock is a global lock.
     *  Better to move to rtnl_net_unlock when the kernel will support it, (6.16 and above)
     */
}
/* In Single and Multi ASIC system, during init from dvs_start the function is called once, in the host.
 * same for sx_netdev_cleanup()*/
static int __init sx_netdev_init(void)
{
    int ret;

    sxd_log_info(SXD_DEV_NONE, "%s", sx_netdev_version);


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    if (sx_core_multi_asic_mode()) {
        ret = register_pernet_subsys(&__sx_netdev_pernet_ops);
        if (ret) {
            goto fail_on_pernet_subsys;
        }
    }
#endif /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) */

    ret = sx_register_interface(&sx_netdev_interface);
    if (ret) {
        goto fail_on_sx_register_interface;
    }

    ret = sx_netdev_rtnl_link_register();
    if (ret) {
        goto fail_on_netdev_rtnl_link_register;
    }

    ret = sx_bridge_rtnl_link_register();
    if (ret) {
        goto fail_on_bridge_rtnl_link_register;
    }

#if !defined(SX_DROP_MONITOR_DISABLED) && \
    ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) || defined(CONFIG_SX_RHEL_8_6))
    ret = bus_register(&devlink_dummy_bus);
    if (ret) {
        goto err_bus_unregister;
    }

    ret = driver_register(&devlink_dummy_driver);
    if (ret) {
        goto err_driver_unregister;
    }

    ret = device_register(&devlink_dummy_dev);
    if (ret) {
        goto err_device_unregister;
    }
#endif

    return 0;
#if !defined(SX_DROP_MONITOR_DISABLED) && \
    ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) || defined(CONFIG_SX_RHEL_8_6))
err_device_unregister:
    driver_unregister(&devlink_dummy_driver);

err_driver_unregister:
    bus_unregister(&devlink_dummy_bus);

err_bus_unregister:
    sx_bridge_rtnl_link_unregister();
#endif
fail_on_bridge_rtnl_link_register:
    sx_netdev_rtnl_link_unregister();

fail_on_netdev_rtnl_link_register:
    sx_unregister_interface(&sx_netdev_interface);

fail_on_sx_register_interface:
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    if (sx_core_multi_asic_mode()) {
        unregister_pernet_subsys(&__sx_netdev_pernet_ops);
    }

fail_on_pernet_subsys:
#endif /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) */
    return ret;
}

static void __exit sx_netdev_cleanup(void)
{
    sxd_log_info(SXD_DEV_NONE, "sx_netdev_cleanup \n");

    sx_unregister_interface(&sx_netdev_interface);

    sx_bridge_rtnl_link_unregister();
    sx_netdev_rtnl_link_unregister();

#if !defined(SX_DROP_MONITOR_DISABLED) && \
    ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) || defined(CONFIG_SX_RHEL_8_6))
    device_unregister(&devlink_dummy_dev);
    driver_unregister(&devlink_dummy_driver);
    bus_unregister(&devlink_dummy_bus);
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    if (sx_core_multi_asic_mode()) {
        unregister_pernet_subsys(&__sx_netdev_pernet_ops);
    }
#endif /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) */
}

module_init(sx_netdev_init);
module_exit(sx_netdev_cleanup);
