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

#ifndef SX_CORE_REG_BUFFER_PRINT_WRAPPER_H
#define SX_CORE_REG_BUFFER_PRINT_WRAPPER_H

#include <linux/netdevice.h>
#include <linux/mlx_sx/kernel_user.h>
#include <linux/mlx_sx/auto_registers/reg.h>
#include <linux/mlx_sx/cmd.h>
#include <linux/mlx_sx/auto_registers/cmd_auto.h>
#include <linux/mlx_sx/auto_registers/mlxsw_item.h>

#define VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size)            \
    do {                                                                  \
        if ((snp_res == 0) || (snp_res >= buffer_length - buffer_size)) { \
            return buffer_size;                                           \
        }                                                                 \
    } while (0)

#define MODULE_INFO_DATE_CODE_YEAR_OF    (0x0)
#define MODULE_INFO_DATE_CODE_YEAR_MASK  (0xFFFF)
#define MODULE_INFO_DATE_CODE_MONTH_OF   (8)
#define MODULE_INFO_DATE_CODE_MONTH_MASK (0x00000000FFFF0000)
#define MODULE_INFO_DATE_CODE_DAY_OF     (16)
#define MODULE_INFO_DATE_CODE_DAY_MASK   (0x0000FFFF00000000)
#define MODULE_INFO_DATE_CODE_SEPERATOR  (0X00002D00002D0000)
#define MODULE_INFO_DATE_CODE_STRING_CONVERT(date_code)                                  \
    (date_code & MODULE_INFO_DATE_CODE_YEAR_MASK) |                                      \
    ((date_code & MODULE_INFO_DATE_CODE_MONTH_MASK) << MODULE_INFO_DATE_CODE_MONTH_OF) | \
    ((date_code & MODULE_INFO_DATE_CODE_DAY_MASK) << MODULE_INFO_DATE_CODE_DAY_OF) |     \
    MODULE_INFO_DATE_CODE_SEPERATOR

#define MODULE_INFO_TX_RX_POWER_GRAN        10U /* uW */
#define MODULE_INFO_TX_RX_BIAS_CURRNET_GRAN 2U  /* uA */
#define SXD_MFCS_FREQ_MIN                   (0)
#define SXD_MFCS_FREQ_MAX                   (2)
#define DIV(x, y) (x / y)
#define MOD(x, y) (x % y)


#endif /* SX_CORE__REG_BUFFER_PRINT_WRAPPER_H_ */
