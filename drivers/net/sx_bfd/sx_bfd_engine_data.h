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

#ifndef __SX_BFD_ENGINE_DATA_H_
#define __SX_BFD_ENGINE_DATA_H_
#include <net/sock.h>
#include "sx_bfd_socket.h"

void recv_packet(struct sx_bfd_dev_db *bfd_dev_db,
                 struct socket        *t_sock,
                 enum sx_bfd_sock_type sock_type,
                 uint32_t              vrf_id,
                 unsigned long         bfd_user_space_pid);


#endif /* __SX_BFD_ENGINE_DATA_H_ */
