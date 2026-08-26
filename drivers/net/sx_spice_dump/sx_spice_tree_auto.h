/*
 * SPDX-FileCopyrightText: Copyright (c) 2018-2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

#include <linux/netdevice.h>
#include "sx_spice_wrapper.h"

/************************************************
 *  Defines
 ***********************************************/

/************************************************
 *  Macros
 ***********************************************/

/************************************************
 *  Type definitions
 ***********************************************/
typedef struct index_type_data {
    char    field_name[50];
    uint8_t field_value;
} index_type_data_t;


typedef enum {
    PLL_GROUP,
    LOCAL_PORT,
    LANE,
    FOM_MEASURMENT,
    MODULE_INDEX,
    I,
    SENSOR_INDEX,
    ROUTER_ENTITY,
    HIST_TYPE,
    DB,
    PLANE_IND,
    PNAT,
    IG,
    ASIC_INDEX,
    ENUM_INIT,
    LP_GL,
    SUB_MODULE,
    DB_INDEX,
    MODULE_IND_TYPE,
    TEST_MODE,
    ALL_LANES,
    PRIO_TC,
    MODULE_INDEX_CAP,
    SENSOR_MAP_TYPE,
    GRP_PROFILE,
    SWID,
    PORT_TYPE,
    PAGE_SELECT,
    PROTO_MASK,
    GRP,
    UNIT,
    IB_SEL,
    SLOT_INDEX,
    MAX_INDEX_TYPE
} index_type_e;


typedef enum {
    SPICE_LOADING_MODE_DEFAULT = 0, /* Default mode */
    SPICE_LOADING_MODE_REDUCED = 1, /* Reduced mode, not implemented yet*/
    SPICE_LOADING_MODE_PERF_CNTR_ONLY = 2, /* Performance counters only */
} spice_loading_mode_e;

/************************************************
 *  Global variables
 ***********************************************/

/************************************************
 *  Function declarations
 ***********************************************/

int sx_spice_tree_init(void);
void sx_spice_tree_deinit(void);
uint16_t sx_spice_tree_index_value_get(index_type_e index);
int sx_spice_tree_ber_app_dir_create(void);
int sx_spice_tree_ber_eth_create(dev_private_data_t *dev_data);
int sx_spice_tree_ber_ib_create(dev_private_data_t *dev_data);
int sx_spice_tree_asic_counters_app_dir_create(void);
int sx_spice_tree_asic_counters_eth_create(dev_private_data_t *dev_data);

