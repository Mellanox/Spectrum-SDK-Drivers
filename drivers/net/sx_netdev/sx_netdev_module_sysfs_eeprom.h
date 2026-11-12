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

#ifndef SX_NETDEV_MODULE_SYSFS_EEPROM_H
#define SX_NETDEV_MODULE_SYSFS_EEPROM_H

/*
 *  create an eeprom sysfs kobject tree under <parent> kobject, with the tree like:
 *   ../<parent>/eeprom/pages/0/i2c-0x50/data
 *   ../<parent>/eeprom/pages/0/i2c-0x51/data
 *   ../<parent>/eeprom/pages/<N = 1..255>/data
 */
int sx_netdev_create_module_sysfs_eeprom_tree(struct kobject                  *parent,
                                              struct module_sysfs_eeprom_tree *root);

/*
 *  remove an eeprom sysfs kobject tree, with the tree like:
 *   ./eeprom/pages/0/i2c-0x50/data
 *   ./eeprom/pages/0/i2c-0x51/data
 *   ./eeprom/pages/<N = 1..255>/data
 */
void sx_netdev_delete_module_sysfs_eeprom_tree(struct module_sysfs_eeprom_tree *root);

#endif /* SX_NETDEV_MODULE_SYSFS_EEPROM_H */
