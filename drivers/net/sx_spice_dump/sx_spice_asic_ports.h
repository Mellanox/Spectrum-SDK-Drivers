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

#ifndef __SPICE_ASIC_PORTS_H__
#define __SPICE_ASIC_PORTS_H__

#include <linux/debugfs.h>
#include <linux/types.h>
#include "sx_spice_wrapper.h"

/************************************************
 *  Type definitions
 ***********************************************/
/************************************************
 * Type declarations
 ***********************************************/

/************************************************
 *  Global variables
 ***********************************************/

/************************************************
* Global Functions                             *
************************************************/
/* Free all port_data_t objects used as i_private by asic_counters/ports debugfs files. */
void sx_spice_asic_ports_data_list_free(void);

/* Return true when local_port is within the valid range for this device (1..max_local_port).
 * This is a range check only; use sx_spice_local_port_emad_allowed() to gate EMAD by PMLP validity. */
bool sx_spice_is_local_port_in_range(dev_private_data_t *dev_data, uint16_t local_port);

/* Initialize the PMLP-valid bitmap used to gate local-port EMAD reads. */
int sx_spice_local_port_validity_bitmap_init(dev_private_data_t *dev_data);

/* Update one cached PMLP-valid bit from a Port DB mapping-change notification. */
void sx_spice_local_port_validity_set(dev_private_data_t *dev_data, uint16_t local_port, bool is_valid);

/* Create the asic_counters/ports directory and all local_port.<N> child directories. */
int sx_spice_access_reg_asic_ports_file_create(struct dentry *parent, dev_private_data_t *dev_data);

#endif /* __SPICE_ASIC_PORTS_H__ */
