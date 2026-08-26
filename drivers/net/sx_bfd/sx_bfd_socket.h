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

#ifndef __SX_BFD_SOCKET_H_
#define __SX_BFD_SOCKET_H_

#include <net/sock.h>
#include "sx_bfd_main.h"

struct metadata {
    union {
        struct sockaddr_in  peer_in;
        struct sockaddr_in6 peer_in6;
    } peer_addr;
    union {
        struct sockaddr_in  local_in;
        struct sockaddr_in6 local_in6;
    } local_addr;
    int inbound_id;
    int ttl;
};
enum sx_bfd_sock_type {
    sx_bfd_SOCK_SINGLEHOP = 0,
    sx_bfd_SOCK_MULTIHOP,
    sx_bfd_SOCK_NUM
};


bool sx_bfd_socket_send(struct socket * sock, char* buf, size_t buf_len, struct sockaddr *peer);
int sx_bfd_socket_recv(struct socket *sock,
                       char* buf, size_t buf_len, struct metadata* metadata);

int sx_bfd_tx_socket_create(struct sockaddr * local_addr,
                            uint8_t           ttl,
                            uint8_t           dscp,
                            struct socket  ** sock,
                            uint16_t          sa_family,
                            uint8_t           use_vrf_device,
                            char            * linux_vrf_name);
int sx_bfd_rx_socket_init(struct sx_bfd_dev_db *bfd_dev_db,
                          uint16_t              port,
                          struct socket      ** sock,
                          uint32_t              vrf_id,
                          unsigned long         bfd_user_space_pid,
                          uint8_t               use_vrf_device,
                          char                 *linux_vrf_name);

void sx_bfd_tx_socket_destroy(struct socket * sock);
void sx_bfd_rx_socket_destroy(struct sx_bfd_dev_db *bfd_dev_db, struct socket * sock);

int sx_bfd_socket_init(struct sx_bfd_dev_db *bfd_dev_db);
void sx_bfd_socket_deinit(struct sx_bfd_dev_db *bfd_dev_db);

void sk_user_destruct(struct sock *sk);

void sx_bfd_rx_vrf_get(struct sx_bfd_dev_db *bfd_dev_db, uint32_t vrf_id);
void sx_bfd_rx_vrf_put(struct sx_bfd_dev_db *bfd_dev_db, uint32_t vrf_id, int delete);
void * sx_bfd_rx_vrf_init(struct sx_bfd_dev_db *bfd_dev_db,
                          uint32_t              vrf_id,
                          uint8_t               use_vrf_device,
                          char                 *linux_vrf_name,
                          unsigned long         bfd_pid);
void sx_bfd_rx_vrf_entry_free_if_needed(struct sx_bfd_dev_db *bfd_dev_db, void *rx_vrf_entry);
void * sx_bfd_rx_vrf_entry_lkp_by_vrf_id(struct sx_bfd_dev_db *bfd_dev_db, uint32_t vrf_id);
void sx_bfd_rx_vrf_hash_add(struct sx_bfd_dev_db *bfd_dev_db, void *rx_vrf_entry);
int sx_bfd_rx_vrf_is_sock_same(struct sx_bfd_dev_db *bfd_dev_db,  uint32_t vrf_id, struct socket *sock);


#endif /* __SX_BFD_SOCKET_H_ */
