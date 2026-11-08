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

#ifndef __SX_BFD_EVENT_H_
#define __SX_BFD_EVENT_H_

#include <net/sock.h>
#include "sx_bfd_socket.h"
#include "sx_bfd_rx_session.h"

void sx_bfd_event_send_packet(struct sx_bfd_dev_db     *bfd_dev_db,
                              struct sx_bfd_rx_session *session,
                              char                     *packet,
                              uint32_t                  size,
                              struct metadata          *metadata,
                              unsigned long             bfd_user_space_pid);
void sx_bfd_event_send_timeout(struct sx_bfd_dev_db *bfd_dev_db, struct sx_bfd_rx_session *session);

#endif /* __SX_BFD_EVENT_H_ */
