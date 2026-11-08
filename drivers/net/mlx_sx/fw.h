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

#ifndef SX_FW_H
#define SX_FW_H

#include <linux/mlx_sx/device.h>
#include "icm.h"

#define NUM_OF_HCRS 2
#define HCR1        0
#define HCR2        1

struct profile_driver_params {
    u8  cqe_version;
    u8  cqe_time_stamp_type;
    u32 supported_feature_set;
};

int sx_SET_PROFILE(struct sx_dev *dev, struct ku_profile *profile, struct profile_driver_params *params);
int sx_GET_PROFILE(struct sx_dev *dev, struct ku_profile *profile, struct profile_driver_params *params_p);
int sx_SET_KVH_CACHE_PARAMS(struct sx_dev             *dev,
                            struct profile_kvh_params *kvh_params);
int sx_GET_KVH_CACHE_PARAMS(struct sx_dev             *dev,
                            struct profile_kvh_params *kvh_params_p);
int sx_QUERY_FW_2(struct sx_dev *dev, int sx_dev_id, u8 hcr_number);
int sx_MAP_FA(struct sx_dev *dev, sxd_map_memory_type_e type, sx_map_fa_handle_t handle);
int sx_UNMAP_FA(struct sx_dev *dev);

#endif /* SX_FW_H */
