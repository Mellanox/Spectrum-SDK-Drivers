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

#ifndef __SX_DEV_CALLBACKS_H__
#define __SX_DEV_CALLBACKS_H__

int sx_core_dev_init_cb(struct sx_priv     *priv,
                        enum sxd_chip_types chip_type);

int sx_core_dev_init_switchx_cb(struct sx_dev      *dev,
                                enum sxd_chip_types chip_type,
                                bool                force);

int sx_core_dev_deinit_switchx_cb(struct sx_dev *dev);

#endif /* __SX_DEV_CALLBACKS_H__ */
