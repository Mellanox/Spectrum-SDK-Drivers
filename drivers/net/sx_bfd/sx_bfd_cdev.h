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

#ifndef __SX_BFD_CDEV_H_
#define __SX_BFD_CDEV_H_

#include "sx_bfd_main.h"

/*
 *  Initialize BFD cdev
 */
int sx_bfd_cdev_init(struct sx_bfd_dev_db *bfd_dev_db);

void sx_bfd_init_char_dev(void);
void sx_bfd_deinit_char_dev(void);

#endif /* __SX_BFD_CDEV_H_ */
