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

#ifndef __SX_BFD_ENGINE_CTRL_H_
#define __SX_BFD_ENGINE_CTRL_H_

#include "sx_bfd_main.h"
/*
 *  Initialize BFD control engine
 */
int sx_bfd_engine_ctrl_init(struct sx_bfd_dev_db *bfd_dev_db);

/*
 *  De-initialize BFD control engine
 */
void sx_bfd_engine_ctrl_deinit(struct sx_bfd_dev_db *bfd_dev_db);

/*
 *  get session statistics
 */
int sx_bfd_get_sess_stats(char * data, uint8_t session_type, uint8_t clear_stats);


#endif /* __SX_BFD_ENGINE_CTRL_H_ */
