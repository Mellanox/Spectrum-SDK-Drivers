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

#ifndef SX_NETDEV_H
#define SX_NETDEV_H

#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/etherdevice.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/netlink.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) || defined(CONFIG_SX_RHEL_8_6)
#include <net/devlink.h>
#endif
#include <linux/mlx_sx/device.h>
#include <linux/mlx_sx/driver.h>
#include <linux/mlx_sx/kernel_user.h>
#include <linux/net_tstamp.h>
#include <linux/if_vlan.h>
#include "sx_netdev_module_sysfs_eeprom.h"

#define DRV_VERSION     "1.0"
#define DRV_RELDATE     "Jan, 2012"
#define NETDEV_DRV_NAME "sx_netdev"

#define ISX_HDR_SIZE         16
#define MAX_JUMBO_FRAME_SIZE 10000
#define DEFAULT_FRAME_SIZE   9216
#define SX_INVALID_TRAP_ID   0xFFFF
#define SX_PORT_TO_ISX_LAG_ID(port) (port << 4)
#define SX_PACKET_DEFAULT_TC    5
#define TX_HEADER_RP_RIF_TO_FID (15 * 1024)
#define SX_VLAN_PRIO_MAX        7
#define MAX_PORT_NETDEV_NUM     2


static const char *net_port_vlan_type_str[] = {
    "GLOBAL",
    "PORT_VALID",
    "LAG_VALID",
    "VLAN_VALID",
};
static const int   net_port_vlan_type_str_len =
    sizeof(net_port_vlan_type_str) / sizeof(char *);
enum net_port_vlan_type {
    PORT_VLAN_TYPE_GLOBAL = 0,
    PORT_VLAN_TYPE_PORT   = 1,
    PORT_VLAN_TYPE_LAG    = 2,
    PORT_VLAN_TYPE_VLAN   = 3
};
#define NET_PORT_VLAN_TYPE_TO_STR(type)    \
    ((type < net_port_vlan_type_str_len) ? \
     net_port_vlan_type_str[type] : "Unknown type")

struct net_port_vlan_info {
    enum net_port_vlan_type info_type;
    u16                     synd;
    u16                     sysport;
    u16                     lag_id;
    u16                     vlan;
};

enum sx_net_kobject_type {
    SX_NET_KOBJECT_PORT            = 0,
    SX_NET_KOBJECT_MODULE          = 1,
    SX_NET_KOBJECT_MODULE_TEMP     = 2,
    SX_NET_KOBJECT_MODULE_TEMP_TEC = 3,
    SX_NET_KOBJECT_MAX             = SX_NET_KOBJECT_MODULE_TEMP_TEC,
};


struct sx_net_priv {
    u8                               swid;
    struct net_port_vlan_info        trap_ids[NUM_OF_NET_DEV_TYPE][MAX_NUM_TRAPS_TO_REGISTER];
    u16                              num_of_traps[NUM_OF_NET_DEV_TYPE];
    struct sx_dev                   *dev;
    struct net_device_stats          stats;
    u64                              mac;
    unsigned long                    active_vlans[BITS_TO_LONGS(VLAN_N_VID)];
    u16                              port;
    u16                              mid;
    int                              is_lag;
    int                              is_port_netdev;
    u32                              log_port;
    u8                               is_oper_state_up;
    enum sx_module_present_state     module_present;
    u8                               send_to_port_as_data_supported;
    int                              is_bridge;
    u16                              bridge_id; /* bridge id == fid */
    struct net_device              * netdev;
    struct delayed_work              pude_dwork; /* pude delayed work */
    struct workqueue_struct        * pude_wq;
    struct hwtstamp_config           hwtstamp_config;
    u8                               skip_tunnel;
    struct kobject                  *kobj[SX_NET_KOBJECT_MAX + 1];
    struct module_sysfs_eeprom_tree *eeprom_kobjs;
#if defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
    struct u64_stats_sync    syncp;
    struct rtnl_link_stats64 stats64;
#endif
#if !defined(SX_DROP_MONITOR_DISABLED) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, \
                                                                                 0)) || defined(CONFIG_SX_RHEL_8_6))
    struct devlink_port devlink_port;
#endif
    u32 pipeline;
};

enum {
    PORT_TYPE_SINGLE = 0,
    PORT_TYPE_LAG    = 1,
    PORT_TYPE_NUM
};

struct sx_netdev_rsc {
    struct net_device *sx_netdevs[NUMBER_OF_SWIDS];
    struct mutex       rsc_lock; /* the resource's lock (for updates), for reading use RCU */
    u8                 allocated[NUMBER_OF_SWIDS];

    /* port_netdev is used by handlers that run in interrupt context.
     * any update to it must be:
     *    1. acquire the mutex (rsc_lock)
     *    2. assign a pointer with rcu_assign_pointer()
     * any read from it must be:
     *    1. under rcu_read_lock() context
     *    2. get the pointer with rcu_dereference()
     *        - OR -
     *    1. acquire the mutex (rsc_lock) and read the pointers with no restrictions.
     */
    struct net_device *port_netdev[PORT_TYPE_NUM][MAX_SYSPORT_NUM][MAX_PORT_NETDEV_NUM];
    u8                 port_allocated[PORT_TYPE_NUM][MAX_SYSPORT_NUM];
    u8                 in_attach_detach;
    u8                 create_module_sysfs;
};

#define SX_VLAN_PRIO_MASK  0xe000         /* Priority Code Point */
#define SX_VLAN_PRIO_SHIFT 13

u64 sx_netdev_mac_to_u64(u8 *addr);
void sx_netdev_u64_to_mac(u8* addr, u64 mac);
int sx_netdev_register_device(struct net_device *netdev, int should_rtnl_lock,
                              int admin_state);
int sx_netdev_create_port_sysfs_nodes(struct net_device *netdev);
int sx_netdev_remove_port_sysfs_nodes(struct net_device *netdev);
int sx_netdev_create_module_sysfs_nodes(struct net_device *netdev);
void sx_netdev_delete_module_sysfs_nodes(struct net_device *netdev);
int sx_netdev_create_module_sysfs_nodes_for_tec(struct kobject *parent);
void sx_netdev_delete_module_sysfs_nodes_for_tec(struct kobject *parent);

enum {
    IFLA_SX_BRIDGE_UNSPEC,
    IFLA_SX_BRIDGE_ID,
    __IFLA_SX_BRIDGE_MAX
};
#define IFLA_SX_BRIDGE_MAX (__IFLA_SX_BRIDGE_MAX - 1)

struct sx_core_interface {
    atomic_t          refcount;
    struct completion free;
    bool              bound;
};

typedef struct psample_rate_info {
    uint8_t  valid;
    uint32_t rate;
} psample_rate_info_t;

struct psample_params {
/*psample */
    struct list_head         psample_listener_db;
    uint32_t                 psample_listener_db_size;
    struct mutex             psample_listener_db_lock;
    struct workqueue_struct *psample_wq;

    /* Used for psample port types rates, KU_PSAMPLE_RATE_TYPE_PORT_...
     * (all other rates types values will be always invalid) */
    psample_rate_info_t psample_port_rate_db[MAX_PHYPORT_NUM + 1][KU_PSAMPLE_RATE_TYPE_MAX_E + 1];

    /* Used for psample mirror binds types rates, KU_PSAMPLE_RATE_TYPE_MIRROR_BIND_...
     * (all other rates types values will be always invalid) */
    psample_rate_info_t psample_mirror_bind_rates_db[KU_PSAMPLE_RATE_TYPE_MAX_E + 1];

    /* Used for psample span session types rates, KU_PSAMPLE_RATE_TYPE_SPAN_SESSION_...
     * (all other rates types values will be always invalid)*/
    psample_rate_info_t psample_span_session_rates_db[KU_PSAMPLE_RATE_TYPE_MAX_E + 1];
};

enum sx_netdev_kobject_type {
    SX_KOBJECT_ASIC = 0,
    SX_KOBJECT_MAX  = SX_KOBJECT_ASIC
};


/* Node for tracking per-caller context associated with a net_device */
struct sx_caller_ctx_node {
    struct list_head    list;
    struct net_device  *netdev;
    struct task_struct *caller;
};

/* Node for tracking netdevs in the user listener set */
struct sx_user_netdev_node {
    struct list_head   list;
    struct net_device *netdev;
};

struct user_netdev_listener_set_params {
    /* caller context tracking */
    struct list_head caller_ctx_list; /* list for in-progress Register/DeRegister commands */
    /* list of netdevs under user listener set management */
    struct list_head netdev_list;
    struct mutex     netdev_list_lock;
    struct mutex     user_netdev_set_lock;
};

struct sx_dev_netdev_priv {
    struct sx_dev           *dev;
    struct sx_netdev_rsc     netdev_resources;
    struct net_device       *bridge_netdev_db[MAX_BRIDGE_NUM];
    struct workqueue_struct *netdev_wq;
    struct sx_core_interface sx_core_if;
    u8                       skip_tunnel;
    struct net              *netns; /* on single asic ==> &init_net, on multi-asic ==> container's private network namespace */

#if !defined(SX_DROP_MONITOR_DISABLED) && ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, \
                                                                                 0)) || defined(CONFIG_SX_RHEL_8_6))
    struct devlink *devlink_dummy_instance;
#endif


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) || defined(CONFIG_SX_RHEL_8_6)
    struct net_device *span2cpu_dev;
#endif

    struct psample_params psample_params;

    struct kobject *kobj[SX_KOBJECT_MAX + 1];

    /* Per-ASIC local parameters (default values are the global module parameters) */
    struct {
        int  sx_netdev_tx_debug;
        int  sx_netdev_rx_debug;
        bool inited;
    } local_params;

    struct user_netdev_listener_set_params user_netdev_listener_set;

    /*priv LL for multi-asic support */
    struct list_head priv_sx_netdev_list;
};

/* all these functions/macros take reference to sx_dev_netdev_priv instance */
struct sx_dev_netdev_priv * dev_netdev_priv_from_dev(struct sx_dev *dev);
struct sx_dev_netdev_priv * dev_netdev_priv_from_curr_net_namespace(void);
#define dev_netdev_priv_from_net_priv(net_priv) dev_netdev_priv_from_dev((net_priv)->dev);
#define dev_netdev_priv_from_netdev(netdev)                          \
    dev_netdev_priv_from_net_priv(((struct sx_net_priv*)netdev_priv( \
                                       netdev)))

/* this function releases reference from sx_dev_netdev_priv */
void dev_netdev_priv_put(struct sx_dev_netdev_priv *priv);

struct net* get_curr_net_namespace(void);

void sx_netdev_setup(struct net_device *dev);

int sx_netdev_synce_sysfs_init(struct kobject *kobj);
void sx_netdev_synce_sysfs_deinit(struct kobject *kobj);

int sx_netdev_create_module_sysfs_extension_for_s3ip(struct kobject *parent);
void sx_netdev_delete_module_sysfs_extension_for_s3ip(struct kobject *parent);

int sx_netdev_rtnl_link_register(void);
void sx_netdev_rtnl_link_unregister(void);
int sx_bridge_rtnl_link_register(void);
void sx_bridge_rtnl_link_unregister(void);
int sx_netdev_get_net_device_by_kobj(struct kobject          *kobj,
                                     struct net_device      **dev,
                                     enum sx_net_kobject_type kobj_type);
int sx_netdev_devlink_port_register(struct net_device *netdev);
void sx_netdev_devlink_port_unregister(struct net_device *netdev);

int sx_netdev_sysfs_handler_start(struct kobject          *kobj,
                                  enum sx_net_kobject_type kobj_type,
                                  struct net_device      **netdev);
void sx_netdev_sysfs_handler_end(struct net_device *netdev);
int send_netdev_sniffer_event(struct net_device *netdev,
                              struct kobject    *kobj,
                              struct attribute  *attr,
                              const char        *cb_name,
                              const char        *buf,
                              size_t             size);
void sx_netdev_rtnl_lock(void);
void sx_netdev_rtnl_unlock(void);

int sx_netdev_carrier_set_on_pude_disable_get(void);
int sx_netdev_oper_state_get(struct sx_dev *dev, u32 log_port, u8 *is_oper_state_up_p);
void netdev_linkstate_set(struct net_device *netdev);

#endif /* SX_NETDEV_H */
