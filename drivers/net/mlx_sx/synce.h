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

#ifndef SX_CORE_SYNCE_H
#define SX_CORE_SYNCE_H

#include <linux/seq_file.h>

int sx_core_synce_init(struct sx_dev *dev);
void sx_core_synce_deinit(struct sx_dev *dev);
int sx_core_synce_init_spectrum(struct sx_dev *dev);
void sx_core_synce_deinit_spectrum(struct sx_dev *dev);
int sx_core_synce_dump(struct seq_file *m, void *v, void *context);


#endif /* SX_CORE_SYNCE_H */
