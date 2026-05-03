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

#ifndef __SX_BFD_RX_SESSION_H_
#define __SX_BFD_RX_SESSION_H_

#include <net/sock.h>
#include <linux/atomic.h>
#include "sx_bfd_workqueue.h"
#include <linux/sx_bfd/sx_bfd_ctrl_cmds.h>

struct sx_bfd_rx_session_entry {
    struct hlist_node         node;
    struct sx_bfd_rx_session *sess;
    uint32_t                  session_id;
};

struct ip_addr {
    uint8_t family;
    union {
        struct in_addr  ipv4;
        struct in6_addr ipv6;
    };
};
struct sx_bfd_rx_session {
    uint32_t                vrf_id;
    uint32_t                session_id;
    uint8_t                 deleted;
    struct completion       free_wait;
    char                   *packet;
    uint32_t                packet_len;
    uint32_t                interval;
    uint32_t                ref_count;
    sx_bfd_delayed_work_t * dwork;
    uint64_t                session_opaque_data;
    struct ip_addr          ip_addr;
    atomic64_t              rx_counter;
    atomic64_t              dropped_packets;
    uint64_t                last_time; /* last time a packet was received */
    atomic_t                remote_heard;
    unsigned long           bfd_pid;
};
unsigned long ip_addr_hash(struct ip_addr * ip_addr);

int sx_bfd_rx_session_init(struct sx_bfd_dev_db *bfd_dev_db);

int sx_bfd_rx_session_deinit(struct sx_bfd_dev_db *bfd_dev_db);

struct sx_bfd_rx_session * sx_bfd_rx_sess_get_by_ip_vrf(struct sx_bfd_dev_db *bfd_dev_db,
                                                        struct ip_addr      * ip_addr,
                                                        uint32_t              vrf_id,
                                                        struct socket        *t_sock);
struct sx_bfd_rx_session * sx_bfd_rx_sess_get_by_id(struct sx_bfd_dev_db *bfd_dev_db, uint32_t session_id);

void sx_bfd_rx_sess_put(struct sx_bfd_dev_db *bfd_dev_db, struct sx_bfd_rx_session * session);

int sx_bfd_rx_sess_add(struct sx_bfd_dev_db *bfd_dev_db, char * data);

int sx_bfd_rx_sess_del(struct sx_bfd_dev_db * bfd_dev_db, char * data);

void rx_delayed_work_dispatch(struct sx_bfd_dev_db *bfd_dev_db, struct sx_bfd_rx_session * session);

int sx_bfd_get_rx_sess_stats(struct sx_bfd_dev_db * bfd_dev_db, char* data, uint8_t clear_stats);

int sx_bfd_rx_sess_update(struct sx_bfd_dev_db *bfd_dev_db, char* data);

#endif /* __SX_BFD_RX_SESSION_H_ */
