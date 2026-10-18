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

#ifndef __SX_CORE_EMAD_H__
#define __SX_CORE_EMAD_H__

#include <linux/mlx_sx/driver.h>
#include "fw_internal.h"

int sx_emad_init(void);
void sx_emad_deinit(void);

int sx_emad_dev_init(struct sx_dev *dev);
void sx_emad_dev_deinit(struct sx_dev *dev);

bool sx_emad_dump_hanlders_per_device_init(struct sx_dev *dev, void *context);
bool sx_emad_dump_hanlders_per_device_deinit(struct sx_dev *dev, void *context);

int sx_emad_build(sxd_dev_id_t      dev_id,
                  const u8         *reg_buff,
                  u32               reg_buff_len,
                  struct sk_buff  **skb,
                  struct isx_meta  *meta,
                  u32               tid_high,
                  u32               tid_low,
                  gfp_t             gfp,
                  u16               reg_id,
                  sxd_emad_method_e method);

int sx_emad_access_reg_common(struct sx_dev                  *dev,
                              emad_prio_e                     priority,
                              sxd_emad_method_e               method,
                              u16                             reg_id,
                              void                           *ku_reg,
                              const struct sx_emad_in_params *in_params,
                              struct sx_emad_out_params      *out_params,
                              access_reg_encode_cb_t          encode_cb,
                              access_reg_decode_cb_t          decode_cb);

int fw_status_to_err(u32 fw_status);

/**
 * sx_emad_timeout_get - Get the current EMAD timeout value for a specific device
 * @dev: Pointer to sx_dev structure
 *
 * Returns: EMAD timeout in microseconds (e.g., 20000000 for 20 seconds)
 */
int sx_emad_timeout_get(struct sx_dev *dev);

/**
 * sx_emad_timeout_set - Set the EMAD timeout value for a specific device
 * @dev: Pointer to sx_dev structure
 * @timeout_usec: Timeout in microseconds (e.g., 20000000 for 20 seconds)
 *
 * Returns: 0 on success
 */
int sx_emad_timeout_set(struct sx_dev *dev, u32 timeout_usec);

/**
 * sx_emad_timeout_dbg_cmd_handler - Debug CLI handler for EMAD timeout commands
 */
int sx_emad_timeout_dbg_cmd_handler(int argc, const char *argv[], struct sx_dev *dev, void *context);

#endif /* __SX_CORE_EMAD_H__ */
