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

#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/mlx_sx/auto_registers/reg.h>
#include <linux/mlx_sx/cmd.h>
#include <linux/mlx_sx/driver.h>
#include <linux/mlx_sx/auto_registers/cmd_auto.h>
#include "sx.h"
#include "dev_db.h"
#include <linux/mlx_sx/kernel_user.h>

#define MCAM_GROUP2_CONTOL_MASK_END_ID   (0x917F)
#define SX_INTERNAL_LOG_DEFAULT_CAPACITY 10000
static uint8_t __max_module_num = 0;
static uint8_t __max_submodule_num = 0;

/*********temp**********/
static ssize_t __show_asic_temp_highest(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t __show_asic_temp_input(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t __show_asic_temp_label(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute asic_temp_highest_attr = __ATTR(highest, S_IRUGO, __show_asic_temp_highest, NULL);
static struct kobj_attribute asic_temp_input_attr = __ATTR(input, S_IRUGO, __show_asic_temp_input, NULL);
static struct kobj_attribute asic_temp_label_attr = __ATTR(label, S_IRUGO, __show_asic_temp_label, NULL);


/********performance counters********/
#define PERF_CNTR_MAX_HW_UNIT_ID_SPEC1 34
#define PERF_CNTR_MAX_HW_UNIT_ID_SPEC2 57
#define PERF_CNTR_MAX_HW_UNIT_ID_SPEC3 62
#define PERF_CNTR_MAX_HW_UNIT_ID_SPEC4 133
#define PERF_CNTR_MAX_HW_UNIT_ID_SPEC5 152
#define PERF_CNTR_MAX_HW_UNIT_ID_SPEC6 213
#define MAX_COUNTER_FILE_ROW_LEN       14 /*max hw_unit_id is 0xD4, max counter is (for SPC6) 0x1223, + 3 char for ":,\n,\0"  + 1 to be on the safe side*/
#define SYSFS_MAX_BUFF_LEN             PAGE_SIZE /*buf size is sysfs is PAGE_SIZE */
#define PERF_CNTR_INTERVAL_MIN_VALUE   1 /*minimum interval is 100us (100usec units)*/
#define PERF_CNTR_INTERVAL_MAX_VALUE   10000 /*maximum interval is 1sec (100usec units)*/

static size_t perf_cntr_hw_unit_max_cntr_arr_spc1[PERF_CNTR_MAX_HW_UNIT_ID_SPEC1] = {
    7, 7, 7, 0, 5, 3, 3, 7, 7, 7, 7, 3, 3, 7, 3, 7, 7, 3, 3, 7, 7, 5, 7, 7, 7, 7, 7, 3, 3, 7, 7, 3,
    3, 5
};

static size_t perf_cntr_hw_unit_max_cntr_arr_spc2[PERF_CNTR_MAX_HW_UNIT_ID_SPEC2] = {
    3, 3, 3, 3, 15, 15, 7, 15, 15, 15, 3, 7, 7, 3, 3, 7, 3, 7, 7, 5, 7, 3, 7, 7, 7, 15, 15, 15, 15, 7,
    7, 7, 7, 7, 7, 15, 7, 3, 3, 3, 3, 15, 15, 7, 15, 7, 15, 7, 3, 7, 0, 0, 7, 7, 15, 7, 5
};

static size_t perf_cntr_hw_unit_max_cntr_arr_spc3[PERF_CNTR_MAX_HW_UNIT_ID_SPEC3] = {
    3, 3, 3, 3, 15, 15, 7, 15, 15, 15, 3, 7, 7, 3, 3, 7, 3, 7, 7, 5, 7, 3, 7, 7, 7, 15, 15, 15, 15, 7,
    7, 7, 7, 7, 7, 15, 7, 3, 3, 3, 3, 15, 15, 7, 15, 7, 15, 7, 3, 7, 0, 0, 7, 7, 15, 7, 5, 7, 15, 7,
    7, 15
};

static size_t perf_cntr_hw_unit_max_cntr_arr_spc4[PERF_CNTR_MAX_HW_UNIT_ID_SPEC4] = {
    3, 15, 0, 7, 7, 5, 0, 0, 0, 0, 15, 15, 15, 15, 15, 3, 15, 15, 15, 7, 15, 15, 15, 15, 0, 0, 0, 0,
    15, 15, 15, 7, 15, 3, 15, 7, 15, 15, 3, 15, 7, 15, 7, 15, 15, 15, 15, 7, 7, 0, 0, 0, 15, 19, 1, 7,
    3, 7, 7, 15, 3, 0, 15, 3, 7, 7, 7, 7, 15, 15, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 3, 1, 1, 1, 1, 3, 1, 1, 1,
    3, 15, 3, 15, 7, 3, 3, 3, 0, 0, 0, 0, 19, 1
};

static size_t perf_cntr_hw_unit_max_cntr_arr_spc5[PERF_CNTR_MAX_HW_UNIT_ID_SPEC5] = {
    3, 15, 0, 7, 7, 5, 0, 0, 0, 0, 15, 15, 15, 15, 15, 3, 15, 15, 15, 7, 15, 15, 15, 15, 0, 0, 0, 0,
    15, 0, 15, 7, 15, 3, 0, 7, 15, 15, 3, 0, 7, 15, 7, 15, 15, 15, 15, 7, 7, 0, 0, 0, 15, 19, 1, 7,
    3, 7, 7, 15, 0, 0, 15, 3, 7, 7, 7, 7, 15, 15, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    0, 0, 0, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 3, 1, 3, 1, 1, 1, 1, 0, 0, 1, 1,
    3, 15, 0, 0, 7, 3, 0, 0, 0, 0, 0, 0, 19, 1, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 7,
    7, 1
};

static size_t perf_cntr_hw_unit_max_cntr_arr_spc6[PERF_CNTR_MAX_HW_UNIT_ID_SPEC6] = {
    15, 15, 15, 15, 15, 3, 1, 15, 15, 15, 15, 15, 3, 7, 3, 15, 7, 15, 7, 15, 7, 7, 7, 15, 15, 3, 15,
    15, 15, 15, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 15, 15, 3, 15, 15,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 15, 15, 15, 15, 15, 15, 15, 15, 7, 15, 7, 15, 1,
    1, 3, 7, 15, 7, 15, 3, 7, 1, 15, 31, 3, 15, 15, 31, 3, 3, 3, 3, 15, 15, 15, 15, 7, 7, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 7, 7, 15, 15, 15, 15, 15, 7, 7, 15, 15, 15, 15, 7, 15, 1, 7, 7, 15, 1, 1,
    7, 15, 15, 15, 15, 31, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, 3,
    3, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 15, 15, 15, 3, 7, 15, 15, 15, 15, 15, 15, 15, 15, 7, 7, 7, 15,
    15, 31, 0, 3, 7, 7, 7, 7, 7, 3, 3, 1, 3, 3
};

static size_t perf_cntr_max_cntr_id_in_hw_unit_arr_spc1[PERF_CNTR_MAX_HW_UNIT_ID_SPEC1] =
{203, 86, 1204, 0, 85, 153, 136, 268, 315, 66, 21, 8, 10, 11, 5, 1029, 140, 153, 136, 268, 315,
 186, 135, 91, 91, 22, 260, 74, 11, 177, 32, 1589, 108, 85};

static size_t perf_cntr_max_cntr_id_in_hw_unit_arr_spc2[PERF_CNTR_MAX_HW_UNIT_ID_SPEC2] =
{153, 136, 128, 79, 202, 806, 1346, 134, 173, 491, 8, 69, 47, 14, 15, 12, 19, 159, 289, 16, 141,
 13, 49, 177, 210, 491, 1058, 775, 54, 15, 30, 312, 186, 43, 43, 1269, 2, 153, 136, 128, 79, 202,
 806, 1346, 134, 483, 222, 17, 72, 4439, 0, 0, 710, 399, 173, 1857, 16};

static size_t perf_cntr_max_cntr_id_in_hw_unit_arr_spc3[PERF_CNTR_MAX_HW_UNIT_ID_SPEC3] =
{153, 136, 128, 79, 156, 418, 1396, 130, 177, 519, 8, 69, 47, 14, 15, 12, 19, 238, 311, 16, 145,
 21, 49, 305, 338, 519, 1124, 843, 54, 15, 30, 410, 236, 51, 51, 1376, 9, 153, 136, 128, 79, 156,
 418, 1396, 130, 291, 222, 17, 136, 4926, 0, 0, 218, 437, 177, 971, 16, 93, 167, 1786, 1574, 89};

static size_t perf_cntr_max_cntr_id_in_hw_unit_arr_spc4[PERF_CNTR_MAX_HW_UNIT_ID_SPEC4] =
{6, 882, 0, 347, 317, 6, 0, 0, 0, 0, 588, 335, 201, 96, 69, 244, 207, 166, 55, 99, 539, 539, 1465,
 787, 0, 0, 0, 0, 1129, 2745, 655, 15, 1869, 6, 196, 1702, 502, 1869, 6, 196, 1702, 502, 302, 311,
 338, 72, 549, 5895, 3645, 0, 0, 0, 155, 20, 10, 237, 6, 411, 865, 239, 6, 0, 155, 123, 471, 115,
 241, 865, 239, 552, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 777, 6, 774, 3072, 3870, 48, 6, 768, 774, 777, 271, 616,
 6, 897, 625, 6, 6, 6, 0, 0, 0, 0, 20, 10};

static size_t perf_cntr_max_cntr_id_in_hw_unit_arr_spc5[PERF_CNTR_MAX_HW_UNIT_ID_SPEC5] =
{6, 873, 0, 351, 317, 6, 0, 0, 0, 0, 588, 335, 201, 96, 69, 244, 207, 166, 55, 99, 534, 534,
 1467, 787, 0, 0, 0, 0, 1174, 0, 655, 15, 2189, 6, 0, 1775, 644, 2189, 6, 0, 1775, 644, 634, 441,
 338, 174, 573, 5433, 3447, 0, 0, 0, 303, 20, 10, 333, 6, 795, 814, 316, 0, 0, 303, 123, 471,
 115, 241, 814, 316, 552, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6,
 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 6, 1551, 6, 1548, 3072, 3870, 48, 0, 0, 1548, 1551,
 529, 1076, 0, 0, 625, 6, 0, 0, 0, 0, 0, 0, 20, 10, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 6, 6, 6, 1391, 1403, 1551};

static size_t perf_cntr_max_cntr_id_in_hw_unit_arr_spc6[PERF_CNTR_MAX_HW_UNIT_ID_SPEC6] =
{1255, 942, 1360, 3900, 494, 6, 13, 1255, 942, 1360, 3900, 494, 6, 1089, 6, 264, 3899, 264, 44, 1551,
 3230, 3230, 3268, 3072, 1548, 6, 1548, 4644, 1548, 65, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
 6, 6, 6, 6, 6, 6, 6, 6, 48, 1551, 6, 1551, 63, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
 809, 565, 565, 359, 2602, 708, 232, 1461, 15, 655, 317, 674, 23, 23, 12, 129, 298, 51, 1326, 6,
 8, 73, 304, 710, 6, 304, 304, 710, 6, 6, 6, 6, 73, 1477, 277, 1784, 319, 319, 2255, 807, 1518,
 3447, 3447, 993, 993, 797, 797, 394, 376, 3538, 993, 993, 797, 797, 412, 376, 3538, 1209, 1190,
 726, 2246, 709, 14, 1488, 317, 329, 23, 23, 81, 381, 381, 1159, 1159, 866, 866, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 672,
 672, 547, 547, 135, 135, 524, 34, 39, 39, 166, 207, 55, 221, 107, 81, 128, 79, 138, 69, 28,
 40, 28, 41, 822, 3141, 381, 1159, 866, 0, 6, 348, 475, 95, 218, 22, 4, 5, 73, 4, 4};

/* List of valid counter_id ranges for HW units with non-contiguous counter_ids.
 * Only HW units with gaps are listed.
 * __perf_cntr_is_counter_id_in_valid_range scans these range lists (typically 0–1 ranges per HW unit) */

/* SPC2 */
static struct perf_cntr_id_range                 perf_cntr_id_range_spc2[] = {
    { .start = 0x00, .end = 0x20 },
    { .start = 0x108, .end = 0x1B8 },
};
static const struct perf_cntr_hw_unit_gaps_entry perf_cntr_hw_unit_gaps_spc2[] = {
    {
        .hw_unit_id = 0x18,
        .num_valid_ranges = 2,
        .valid_ranges = perf_cntr_id_range_spc2
    },
};

/* SPC3 */
static struct perf_cntr_id_range                 perf_cntr_id_range_spc3[] = {
    { .start = 0x00, .end = 0x20 },
    { .start = 0x108, .end = 0x238 },
};
static const struct perf_cntr_hw_unit_gaps_entry perf_cntr_hw_unit_gaps_spc3[] = {
    {
        .hw_unit_id = 0x18,
        .num_valid_ranges = 2,
        .valid_ranges = perf_cntr_id_range_spc3
    },
};

/* SPC4 */
static struct perf_cntr_id_range                 perf_cntr_id_range_spc4[] = {
    { .start = 0x00, .end = 0x2E },
    { .start = 0x8C, .end = 0x93 },
};
static const struct perf_cntr_hw_unit_gaps_entry perf_cntr_hw_unit_gaps_spc4[] = {
    {
        .hw_unit_id = 0x12,
        .num_valid_ranges = 2,
        .valid_ranges = perf_cntr_id_range_spc4
    },
};

/* SPC5 */
static struct perf_cntr_id_range                 perf_cntr_id_range_spc5[] = {
    { .start = 0x00, .end = 0x2E },
    { .start = 0x8C, .end = 0x93 },
};
static const struct perf_cntr_hw_unit_gaps_entry perf_cntr_hw_unit_gaps_spc5[] = {
    {
        .hw_unit_id = 0x12,
        .num_valid_ranges = 2,
        .valid_ranges = perf_cntr_id_range_spc5
    },
};

/* SPC6 */
static struct perf_cntr_id_range                 perf_cntr_id_range_spc6[] = {
    { .start = 0x00, .end = 0x2E },
    { .start = 0x8C, .end = 0x93 },
};
static const struct perf_cntr_hw_unit_gaps_entry perf_cntr_hw_unit_gaps_spc6[] = {
    {
        .hw_unit_id = 0xB8,
        .num_valid_ranges = 2,
        .valid_ranges = perf_cntr_id_range_spc6
    },
};

static ssize_t __show_asic_perf_cntr_counters(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t __store_asic_perf_cntr_counters(struct kobject        *kobj,
                                               struct kobj_attribute *attr,
                                               const char            *buf,
                                               size_t                 len);
static ssize_t __show_asic_perf_cntr_interval(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t __store_asic_perf_cntr_interval(struct kobject        *kobj,
                                               struct kobj_attribute *attr,
                                               const char            *buf,
                                               size_t                 len);
static ssize_t __show_asic_perf_cntr_force(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t __store_asic_perf_cntr_force(struct kobject        *kobj,
                                            struct kobj_attribute *attr,
                                            const char            *buf,
                                            size_t                 len);
static struct kobj_attribute asic_perf_cntr_counters_attr =
    __ATTR(counters, (S_IRUGO | S_IWUSR), __show_asic_perf_cntr_counters, __store_asic_perf_cntr_counters);
static struct kobj_attribute asic_perf_cntr_interval_attr =
    __ATTR(interval, (S_IRUGO | S_IWUSR), __show_asic_perf_cntr_interval, __store_asic_perf_cntr_interval);
static struct kobj_attribute asic_perf_cntr_force_attr =
    __ATTR(force, (S_IRUGO | S_IWUSR), __show_asic_perf_cntr_force, __store_asic_perf_cntr_force);


/* ---------------- I2C Utilization (per-ASIC) ---------------- */
static ssize_t __show_asic_i2c_utl(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t __store_asic_i2c_utl(struct kobject        *kobj,
                                    struct kobj_attribute *attr,
                                    const char            *buf,
                                    size_t                 len);
static struct kobj_attribute asic_i2c_utl_attr =
    __ATTR(i2c_utilization, (S_IRUGO | S_IWUSR), __show_asic_i2c_utl, __store_asic_i2c_utl);

/* ---------------- local_parameters (per-ASIC) ---------------- */
extern int tx_debug;
extern int rx_debug;
extern int chip_info_type;
extern int chip_info_revision;
extern int enable_keep_packet_crc;

static ssize_t __show_local_param_int(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t __store_local_param_int(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len);

static struct kobj_attribute lp_tx_debug_attr = __ATTR(tx_debug,
                                                       (S_IRUGO | S_IWUSR),
                                                       __show_local_param_int,
                                                       __store_local_param_int);
static struct kobj_attribute lp_rx_debug_attr = __ATTR(rx_debug,
                                                       (S_IRUGO | S_IWUSR),
                                                       __show_local_param_int,
                                                       __store_local_param_int);
static struct kobj_attribute lp_chip_info_type_attr = __ATTR(chip_info_type,
                                                             (S_IRUGO | S_IWUSR),
                                                             __show_local_param_int,
                                                             __store_local_param_int);
static struct kobj_attribute lp_chip_info_revision_attr = __ATTR(chip_info_revision,
                                                                 (S_IRUGO | S_IWUSR),
                                                                 __show_local_param_int,
                                                                 __store_local_param_int);
static struct kobj_attribute lp_enable_keep_packet_crc_attr = __ATTR(enable_keep_packet_crc,
                                                                     (S_IRUGO | S_IWUSR),
                                                                     __show_local_param_int,
                                                                     __store_local_param_int);

static struct attribute *local_params_attrs[] = {
    &lp_tx_debug_attr.attr,
    &lp_rx_debug_attr.attr,
    &lp_chip_info_type_attr.attr,
    &lp_chip_info_revision_attr.attr,
    &lp_enable_keep_packet_crc_attr.attr,
    NULL,
};

static const struct attribute_group local_params_group = {
    .name = "local_parameters",
    .attrs = local_params_attrs,
};


static bool module_sysfs_tree_created = false;
static bool els_module_found = false;

#define GA_MAX_NUM (GA_MAX_INDEX - GA_MIN_INDEX + 1)

/* MMAM have same module type enum as PMTM */
#define MODULE_IS_BACKPLANE_TYPE(module_type)                              \
    ((module_type == SXD_PMTM_MODULE_TYPE_BACKPLANE_WITH_SINGLE_LANE_E) || \
     (module_type == SXD_PMTM_MODULE_TYPE_BACKPLANE_WITH_TWO_LANES_E) ||   \
     (module_type == SXD_PMTM_MODULE_TYPE_BACKPLANE_WITH_4_LANES_E) ||     \
     (module_type == SXD_PMTM_MODULE_TYPE_BACKPLANE_WITH_8_LANES_E) ||     \
     (module_type == SXD_PMTM_MODULE_TYPE_NO_CAGE_E) ||                    \
     (module_type == SXD_PMTM_MODULE_TYPE_CHIP2CHIP4X_E) ||                \
     (module_type == SXD_PMTM_MODULE_TYPE_CHIP2CHIP2X_E) ||                \
     (module_type == SXD_PMTM_MODULE_TYPE_CHIP2CHIP1X_E) ||                \
     (module_type == SXD_PMTM_MODULE_TYPE_CHIP2CHIP8X_E) ||                \
     (module_type == SXD_PMTM_MODULE_TYPE_LOOPBACK_E))


#define MCIA_MAX_ACCESS_SIZE_128B 128
#define MCIA_MAX_ACCESS_SIZE_48B  48

#define MCIA_SUPPORT_128B_OFFSET 34
#define IS_MCIA_SUPPORT_128B(feature_cap_mask1) \
    ((feature_cap_mask1) &                      \
     (1 << (MCIA_SUPPORT_128B_OFFSET - 32)))

#define MCIA_SUPPORT_ASYNC_MODE_OFFSET 82
#define IS_MCIA_SUPPORT_ASYNC_MODE(feature_cap_mask1) \
    ((feature_cap_mask1) &                            \
     (1 << (MCIA_SUPPORT_ASYNC_MODE_OFFSET - 64)))


struct sx_global_module {
    bool    inited;
    uint8_t ga;             /* Asic GA id */
    uint8_t local_module;   /* Local module id */
    uint8_t module_type;    /* Module type */
    uint8_t local_port_for_els; /* Local port for ELS modules */
};

struct sx_local_module {
    bool    inited;
    uint8_t global_module;   /* Global module id */
};

struct sx_ga_map_dev_id_entry {
    bool         inited;
    sxd_dev_id_t dev_id;
};

struct sx_module_type {
    bool    inited;
    uint8_t module_type;    /* Module type */
};

static struct sx_global_module       __global_modules[MAX_MODULE_NUM] = {
    {0}
};
static struct sx_local_module        __local_modules[GA_MAX_NUM][MAX_MODULE_NUM] = {
    {
        {0}
    }
};
static struct sx_ga_map_dev_id_entry ga_devid_map[GA_MAX_NUM] = {
    {0}
};

static struct sx_module_type __modules[MAX_MODULE_NUM] = {
    {0}
};
struct module_sysfs_node    *__default_module_sysfs_root_arr = NULL;

/* Multi-asic module SysFS shared state (driver lifetime for topology/parent). */
static DEFINE_MUTEX(__module_sysfs_shared_lock);
static bool            __global_topology_inited = false;
static struct kobject *__shared_module_parent_kobj = NULL;
static bool            __module_indmod_sysfs_created[MAX_MODULE_NUM] = {
    false
};

char * __sx_sysfs_get_str(char *buffer, char **str, const char* delimiters);
char * __sx_sysfs_get_id_str(char *buffer, char **str);
char * __sx_sysfs_get_hw_unit_counter_pair_str(char *buffer, char **str);
static int __perf_cntr_add_pair_to_counters_db(struct sx_dev   *dev,
                                               int              hw_unit_id,
                                               int              counter_id,
                                               sxd_chip_types_t chip_type);
static int __perf_cntr_validate_hw_unit_id(sxd_chip_types_t chip_type, size_t hw_unit_id);
static int __perf_cntr_validate_counter_id(sxd_chip_types_t chip_type, size_t hw_unit_id, size_t counter_id);
static int __perf_cntr_validate_hw_unit_and_cntr_id(size_t           hw_unit_id,
                                                    size_t           counter_id,
                                                    sxd_chip_types_t chip_type);
static int __perf_cntr_add_new_mopct_node_to_list(struct sx_perf_cntr *perf_cntr_p,
                                                  int                  hw_unit_id,
                                                  int                  counter_id);
static struct mopct_node* __perf_cntr_allocate_mopct_node(int hw_unit_id,
                                                          int counter_id,
                                                          int group_id,
                                                          u8  force);

int find_full_path(const struct kobject *kobj, const struct attribute *attr, char *path_p);

/*********************************************************************/


int sx_core_init_module_eeprom_access_mode(struct sx_dev *dev)
{
    int                       err = 0;
    struct sx_priv           *priv = sx_priv(dev);
    struct ku_access_mcam_reg mcam_reg_data;
    uint8_t                   mcia_max_access_size = MCIA_MAX_ACCESS_SIZE_48B; /* default 48B in old system */
    bool                      mcia_async_access_mode = false;

    memset(&mcam_reg_data, 0, sizeof(mcam_reg_data));
    mcam_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mcam_reg_data.op_tlv, MLXSW_MCAM_ID, SXD_EMAD_METHOD_QUERY_E);
    mcam_reg_data.mcam_reg.access_reg_group = SXD_MCAM_ACCESS_REG_GROUP_FIRST_128_REG_ID_E;

    err = sx_ACCESS_REG_MCAM(dev, &mcam_reg_data);
    if (err) {
        sxd_log_err(dev, "Failed to query MCAM, err=%d\n", err);
        goto out;
    }

    err = sxd_emad_operation_tlv_status_get(&mcam_reg_data.op_tlv);
    if (err) {
        sxd_log_err(dev, "Failed to query MCAM, status=%d\n", err);
        err = -EINVAL;
        goto out;
    }

    if (IS_MCIA_SUPPORT_128B(mcam_reg_data.mcam_reg.mng_feature_cap_mask[1])) {
        mcia_max_access_size = MCIA_MAX_ACCESS_SIZE_128B;
    }

    if (IS_MCIA_SUPPORT_ASYNC_MODE(mcam_reg_data.mcam_reg.mng_feature_cap_mask[2])) {
        mcia_async_access_mode = true;
    }

    if (mcia_async_access_mode) {
        err = sx_core_parallel_mod_access_wq_init(dev);
        if (err) {
            sxd_log_err(dev,
                        "Failed to initialize parallel module access work queue, err=%d\n",
                        err);
            /* Keep SW state consistent: there is no workqueue and
             * parallel_mod_access_initialized is false, so async MCIA cannot be
             * used. Clear the local mode flag so the value stored into priv at
             * 'out:' reflects reality. The original err is still propagated. */
            mcia_async_access_mode = false;
            goto out;
        }
        if (!priv->module_eeprom_access_db.async_mod_access_db.parallel_mod_access_initialized) {
            sxd_log_warning(dev,
                            "Async MCIA init incomplete - disabling async MCIA mode and falling back to sync path.\n");
            mcia_async_access_mode = false;
        }
    }

    /* Initialize module eeprom access DB. */
    mutex_init(&priv->module_eeprom_access_db.access_mutex);
    mutex_init(&priv->module_eeprom_mcia_access_mutex);
    memset(priv->module_eeprom_access_db.eeprom_types, EEPROM_MANAGEMENT_TYPE_INVALID,
           sizeof(priv->module_eeprom_access_db.eeprom_types));

out:
    sxd_log_notice(dev, "mcia_max_access_size: %d, mcia_async_access_mode: %d\n",
                   mcia_max_access_size,
                   mcia_async_access_mode);
    priv->module_eeprom_max_access_size = mcia_max_access_size;
    priv->module_eeprom_async_access_mode = mcia_async_access_mode;
    return err;
}

static void __deinit_module_eeprom_access_mode(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);

    if (priv->module_eeprom_async_access_mode) {
        sx_core_parallel_mod_access_wq_deinit(dev);
    }

    priv->module_eeprom_max_access_size = MCIA_MAX_ACCESS_SIZE_48B;
    priv->module_eeprom_async_access_mode = false;
    memset(priv->module_eeprom_access_db.eeprom_types, 0, sizeof(priv->module_eeprom_access_db.eeprom_types));
}

static void __read_frequency_support_to_db(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);

    if (SX_CORE_BITMAP_LE_GET(priv->mcam_reg_data.group_0X9100.mcam_reg.mng_access_reg_cap_mask,
                              SXD_MCAM_MNG_ACCESS_REG_CAP_MASK_NUM,
                              sizeof(priv->mcam_reg_data.group_0X9100.mcam_reg.mng_access_reg_cap_mask[0]) * 8,
                              (MLXSW_MCFS_ID & 0xff))) {
        priv->independent_module_params.frequency_support = true;
    } else {
        priv->independent_module_params.frequency_support = false;
    }
}

static int __read_apply_im_supported(struct sx_dev *dev)
{
    int                       err = 0;
    struct sx_priv           *priv = sx_priv(dev);
    struct ku_access_pcam_reg pcam_reg_data;
    bool                      apply_im_supported = false;

    memset(&pcam_reg_data, 0, sizeof(pcam_reg_data));
    pcam_reg_data.dev_id = dev->device_id;
    pcam_reg_data.pcam_reg.feature_group = SXD_PCAM_FEATURE_GROUP_ENHANCED_FEATURES_E;
    sx_cmd_set_op_tlv(&pcam_reg_data.op_tlv, MLXSW_PCAM_ID, SXD_EMAD_METHOD_QUERY_E);

    err = sx_ACCESS_REG_PCAM(dev, &pcam_reg_data);
    if (err) {
        sxd_log_notice(dev, "Failed to query PCAM, err=%d\n", err);
        goto out;
    }

    err = sxd_emad_operation_tlv_status_get(&pcam_reg_data.op_tlv);
    if (err) {
        sxd_log_notice(dev, "Failed to query PCAM, status=%d\n", err);
        err = -EINVAL;
        goto out;
    }

    apply_im_supported =
        !!(pcam_reg_data.pcam_reg.feature_cap_mask.pcam_feature_capability_mask_group0.feature_cap_mask[1] &
           SXD_PCAM_FEATURE_CAP_MASK_PMMP_APPLY_IM_SUPPORTED_E);

out:
    priv->apply_im_supported = apply_im_supported;
    return err;
}

static int __init_max_module_num(struct sx_dev *dev)
{
    int                        err = 0;
    struct ku_access_mgpir_reg reg_data;
    u8                         fw_status = 0;

    reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&reg_data.op_tlv, MLXSW_MGPIR_ID, SXD_EMAD_METHOD_QUERY_E);
    reg_data.mgpir_reg.hw_info.slot_index = 0; /*slot is set 0 by default */

    err = sx_ACCESS_REG_MGPIR(dev, &reg_data);

    fw_status = sxd_emad_operation_tlv_status_get(&reg_data.op_tlv);
    if (err || fw_status) {
        sxd_log_err(dev,
                    "Fails to access register MGPIR, err: %d, status: %d.\n",
                    err,
                    fw_status);
        err = -EFAULT;
        goto out;
    }
    __max_module_num = reg_data.mgpir_reg.hw_info.num_of_modules;
    __max_submodule_num = reg_data.mgpir_reg.hw_info.max_sub_modules_index + 1;
    if (sx_core_has_pci_multi_dev()) {
        /* number of total modules in all ASICs together */
        __max_module_num = reg_data.mgpir_reg.hw_info.num_of_modules_per_system;
    }
    sxd_log_info(dev, "__max_module_num:%d.\n", __max_module_num);

out:
    return err;
}

static int __get_pmtm_module_type(struct sx_dev *dev, uint8_t module, uint8_t *module_type)
{
    int                       err = 0;
    struct ku_access_pmtm_reg pmtm_reg_data;

    memset(&pmtm_reg_data, 0, sizeof(pmtm_reg_data));
    pmtm_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&pmtm_reg_data.op_tlv, MLXSW_PMTM_ID, SXD_EMAD_METHOD_QUERY_E);
    pmtm_reg_data.pmtm_reg.slot_index = 0;
    pmtm_reg_data.pmtm_reg.module = module;

    err = sx_ACCESS_REG_PMTM(dev, &pmtm_reg_data);
    if (err) {
        sxd_log_err(dev, "Failed to query PMTM, err=%d\n", err);
        goto out;
    }

    err = sxd_emad_operation_tlv_status_get(&pmtm_reg_data.op_tlv);
    if (err) {
        sxd_log_err(dev, "Failed to query PMTM, status=%d\n", err);
        err = -EINVAL;
        goto out;
    }

    if (module_type) {
        *module_type = pmtm_reg_data.pmtm_reg.module_type;
    }

out:
    return err;
}

static int __init_module_type_map(struct sx_dev *dev, uint8_t module)
{
    int     ret = 0;
    uint8_t module_type = 0;

    ret = __get_pmtm_module_type(dev, module, &module_type);
    if (ret) {
        goto out;
    }

    __modules[module].module_type = module_type;
    __modules[module].inited = true;

out:
    return ret;
}

static void __deinit_module_type_map(struct sx_dev *dev, uint8_t module)
{
    __modules[module].module_type = 0;
    __modules[module].inited = false;
}

int sx_module_sysfs_dump(struct seq_file *m, void *v, void *context)
{
    int i = 0;

    /* this function MUST NOT be called from interrupt context */

    seq_printf(m, "Module Sysfs Mappings\n");
    seq_printf(m, "-------------------------------------------------------------------------------\n\n");
    seq_printf(m, "Predefined device IDs: %s\n\n", (sx_core_has_pci_multi_dev() ? "yes" : "no"));

    seq_printf(m, "\n%-14s   %s\n", "Module", "Module-Type");
    seq_printf(m, "...............................................................................\n");
    for (i = 0; i < MAX_MODULE_NUM; i++) {
        if (!__modules[i].inited) {
            continue;
        }
        seq_printf(m, "%-14d   %u\n", i, __modules[i].module_type);
    }

    seq_printf(m, "\n\n");
    return 0;
}

static void __get_skip_sysfs_mode_for_modules(bool *module_skip_sysfs_arr, uint8_t module_num)
{
    int i = 0;

    if (sx_core_has_pci_multi_dev()) {
        for (i = 0; i < module_num; i++) {
            module_skip_sysfs_arr[i] = __global_modules[(i)].inited && MODULE_IS_BACKPLANE_TYPE(
                __global_modules[(i)].module_type);
        }
    } else {
        for (i = 0; i < module_num; i++) {
            module_skip_sysfs_arr[i] = __modules[i].inited && MODULE_IS_BACKPLANE_TYPE(__modules[i].module_type);
        }
    }

    return;
}

static int __get_module_type(uint8_t module)
{
    if (sx_core_has_pci_multi_dev()) {
        if (__global_modules[module].inited) {
            return __global_modules[module].module_type;
        }
    } else {
        if (__modules[module].inited) {
            return __modules[module].module_type;
        }
    }
    return 0;
}

int sx_sysfs_asic_get_dev_from_kobj(struct kobject *kobj_p, struct sx_dev **dev_pp, const char *module_folder_p)
{
    const struct kobject *kobj_iter_p = NULL;
    struct sx_dev        *dev_p = NULL;
    sxd_dev_id_t          dev_id = SX_SINGLE_ASIC_DEFAULT_DEV_ID;
    uint8_t               asic_id = 0;
    int                   ret = 0;
    int                   scan_result = 0;
    char                  extra_char = '\0';
    const char           *kobj_name_p = NULL;

    /* Validate input parameters */
    if (!dev_pp || !kobj_p || !module_folder_p) {
        sxd_log_err(SXD_DEV_NONE, "%s is NULL\n", !dev_pp ? "dev_pp" : (!kobj_p ? "kobj_p" : "module_folder_p"));
        ret = -EINVAL;
        goto out;
    }

    /* Initialize iterator */
    kobj_iter_p = kobj_p;

    /* Iterate through the kobject's parents to find appropriate asic_id */
    while (kobj_iter_p) {
        /* Check if kobj_iter has a parent before accessing it */
        if (kobj_iter_p->parent) {
            /* Check if the parent matches the module folder */
            if (strcmp(kobject_name(kobj_iter_p->parent), module_folder_p) == 0) {
                kobj_name_p = kobject_name(kobj_iter_p);

                scan_result = sscanf(kobj_name_p, "asic%hhu%c", &asic_id, &extra_char);

                if (scan_result == 1) {
                    break;
                }
            }
        }
        kobj_iter_p = kobj_iter_p->parent;
    }

    /* If no matching asic_id is found, default to 0 */
    if (!kobj_iter_p || (scan_result != 1)) {
        asic_id = 0;
    }

    /* Calculate dev_id */
    dev_id = (sxd_dev_id_t)(asic_id + 1);

    /* Get the device pointer */
    dev_p = sx_dev_db_get_dev_by_id(dev_id);
    if (!dev_p) {
        sxd_log_err(SXD_DEV_NONE, "Failed to get sx_dev for dev_id %d\n", dev_id);
        ret = -ENODEV;
        goto out;
    }

    /* Set the output device pointer */
    *dev_pp = dev_p;

out:
    return ret;
}
EXPORT_SYMBOL(sx_sysfs_asic_get_dev_from_kobj);

int sx_core_asic_get_dev_global(uint8_t global_module, struct sx_dev **dev_pp)
{
    int          ret = 0;
    sxd_dev_id_t dev_id = SX_SINGLE_ASIC_DEFAULT_DEV_ID;
    bool         global_module_map_inited = false;

    /* Validate input parameters */
    if (!dev_pp) {
        sxd_log_err(SXD_DEV_NONE, "dev_pp is NULL\n");
        ret = -EINVAL;
        goto out;
    }

    /* Multi-asic module sysfs entries are only under asic0 directory. */
    ret = sx_multi_asic_module_sysfs_module_is_inited(global_module, &global_module_map_inited);
    if (ret) {
        goto out;
    }

    if (global_module_map_inited) {
        ret = sx_multi_asic_module_sysfs_get_devid_via_ga(__global_modules[global_module].ga, &dev_id);
        if (ret) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get device via ga %d of global_module %d, err: %d.\n",
                        __global_modules[global_module].ga, global_module, ret);
            goto out;
        }
    }

    *dev_pp = sx_dev_db_get_dev_by_id(dev_id);

    if (!(*dev_pp)) {
        sxd_log_err(SXD_DEV_NONE, "Failed to get device\n");
        ret = -ENODEV;
        goto out;
    }

out:
    return ret;
}

static int __get_global_module_info_from_mmam(struct sx_dev *dev,
                                              uint8_t        global_module_id,
                                              uint8_t       *ga,
                                              uint8_t       *local_module_id,
                                              uint8_t       *module_type)
{
    int                       err = 0;
    struct ku_access_mmam_reg mmam_reg_data;

    memset(&mmam_reg_data, 0, sizeof(mmam_reg_data));
    sx_cmd_set_op_tlv(&mmam_reg_data.op_tlv, MLXSW_MMAM_ID, SXD_EMAD_METHOD_QUERY_E);
    mmam_reg_data.dev_id = dev->device_id;
    mmam_reg_data.mmam_reg.module = global_module_id;

    err = sx_ACCESS_REG_MMAM(dev, &mmam_reg_data);
    if (err) {
        sxd_log_err(dev,
                    "Failed to access MMAM, dev_id: %d, module %u, err=%d.\n",
                    dev->device_id,
                    global_module_id,
                    err);
        goto out;
    }

    err = sxd_emad_operation_tlv_status_get(&mmam_reg_data.op_tlv);
    if (err) {
        sxd_log_err(dev, "MMAM return error status %d,dev_id: %d, module %u. \n",
                    err,
                    dev->device_id,
                    global_module_id);
        err = -EINVAL;
        goto out;
    }

    *ga = mmam_reg_data.mmam_reg.ga;
    *local_module_id = mmam_reg_data.mmam_reg.local_module;
    *module_type = mmam_reg_data.mmam_reg.module_type;
    if (mmam_reg_data.mmam_reg.module_type == SXD_PMTM_MODULE_TYPE_OSFP_ELS_E) {
        els_module_found = true;
    }

out:
    return err;
}

/* True for the Quantum (InfiniBand) chip family.  */
static bool __is_qtm_device(struct sx_dev *dev)
{
    switch (sx_priv(dev)->dev_info.dev_info_ro.chip_type) {
    case SXD_CHIP_TYPE_QUANTUM:
    case SXD_CHIP_TYPE_QUANTUM2:
    case SXD_CHIP_TYPE_QUANTUM3:
    case SXD_CHIP_TYPE_QUANTUM4:
    case SXD_CHIP_TYPE_QUANTUM5:
        return true;

    default:
        return false;
    }
}

int sx_multi_asic_module_sysfs_get_type_by_local_module(struct sx_dev *dev,
                                                        uint8_t        local_module,
                                                        uint8_t       *module_type_p)
{
    struct sx_priv            *priv_p = NULL;
    struct ku_access_mgpir_reg mgpir_reg_data;
    uint8_t                    ga = 0;
    uint8_t                    num_modules = 0;
    uint8_t                    global_module = 0;
    uint8_t                    mmam_ga = 0;
    uint8_t                    mmam_local_module = 0;
    uint8_t                    mmam_module_type = 0;
    bool                       ga_valid = false;
    bool                       had_mmam_err = false;
    int                        err = 0;
    int                        first_err = 0;
    int                        scan_err = 0;

    if (!module_type_p) {
        err = -EINVAL;
        goto out;
    }

    if (!dev) {
        err = -ENODEV;
        goto out;
    }

    /* Ethernet devices address modules by global module id, so MMAM lookup by local module
     * is irrelevant for them and the caller should fall back to PMTM. */
    if (!__is_qtm_device(dev)) {
        err = -ENOENT;
        goto out;
    }

    priv_p = sx_priv(dev);

    ga_valid = priv_p->dev_info.dev_info_ro.mgir.hw_info.ga_valid;
    ga = priv_p->dev_info.dev_info_ro.mgir.hw_info.ga;

    if (ga_valid && (ga < GA_MAX_NUM) && (local_module < MAX_MODULE_NUM) &&
        __local_modules[ga][local_module].inited) {
        err = sx_multi_asic_module_sysfs_get_global_module(dev, local_module, &global_module);
        if (!err && __global_modules[global_module].inited) {
            *module_type_p = __global_modules[global_module].module_type;
            goto out;
        }
    }

    memset(&mgpir_reg_data, 0, sizeof(mgpir_reg_data));
    mgpir_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mgpir_reg_data.op_tlv, MLXSW_MGPIR_ID, SXD_EMAD_METHOD_QUERY_E);
    mgpir_reg_data.mgpir_reg.hw_info.slot_index = 0;

    err = sx_ACCESS_REG_MGPIR(dev, &mgpir_reg_data);
    if (err || (mgpir_reg_data.op_tlv.dr_status & 0x7f)) {
        sxd_log_err(dev,
                    "Failed to read MGPIR for MMAM module lookup (err=%d, status=%d).\n",
                    err,
                    (mgpir_reg_data.op_tlv.dr_status & 0x7f));
        err = -EIO;
        goto out;
    }

    num_modules = mgpir_reg_data.mgpir_reg.hw_info.num_of_modules_per_system;
    if (!num_modules || (num_modules > MAX_MODULE_NUM)) {
        sxd_log_err(dev,
                    "Invalid num_of_modules_per_system (%u) for MMAM module lookup.\n",
                    num_modules);
        err = -EINVAL;
        goto out;
    }

    for (global_module = 0; global_module < num_modules; global_module++) {
        scan_err = __get_global_module_info_from_mmam(dev,
                                                      global_module,
                                                      &mmam_ga,
                                                      &mmam_local_module,
                                                      &mmam_module_type);
        if (scan_err) {
            if (!had_mmam_err) {
                first_err = scan_err;
                had_mmam_err = true;
            }
            continue;
        }

        if (ga_valid) {
            if ((mmam_ga == ga) && (mmam_local_module == local_module)) {
                *module_type_p = mmam_module_type;
                err = 0;
                goto out;
            }
        } else if (mmam_local_module == local_module) {
            *module_type_p = mmam_module_type;
            err = 0;
            goto out;
        }
    }

    if (had_mmam_err) {
        sxd_log_rl_notice(dev,
                          "MMAM module scan failed for local module %u (first_err=%d).\n",
                          local_module,
                          first_err);
        err = first_err;
    } else {
        err = -ENOENT;
    }

out:
    return err;
}

int sx_sysfs_get_mcam_reg_data(struct sx_dev *dev)
{
    struct sx_priv           *priv = sx_priv(dev);
    struct ku_access_mcam_reg mcam_reg_data;
    int                       err = 0;

    /* Query MCAM register once during device initialization */
    memset(&mcam_reg_data, 0, sizeof(mcam_reg_data));
    mcam_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mcam_reg_data.op_tlv, MLXSW_MCAM_ID, SXD_EMAD_METHOD_QUERY_E);
    mcam_reg_data.mcam_reg.access_reg_group = SXD_MCAM_ACCESS_REG_GROUP_REGISTER_IDS_0X9100_E;

    err = sx_ACCESS_REG_MCAM(dev, &mcam_reg_data);
    if (err) {
        sxd_log_notice(dev, "Failed to query MCAM, err=%d\n", err);
        goto out;
    }

    err = sxd_emad_operation_tlv_status_get(&mcam_reg_data.op_tlv);
    if (err) {
        sxd_log_notice(dev, "Failed to query MCAM, status=%d\n", err);
        err = -EINVAL;
        goto out;
    }

    /* Cache the result */
    priv->mcam_reg_data.group_0X9100 = mcam_reg_data;

out:
    return err;
}

void sx_sysfs_update_tec_support(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);

    priv->tec_support = SX_CORE_BITMAP_LE_GET(priv->mcam_reg_data.group_0X9100.mcam_reg.mng_access_reg_cap_mask,
                                              SXD_MCAM_MNG_ACCESS_REG_CAP_MASK_NUM,
                                              sizeof(priv->mcam_reg_data.group_0X9100.mcam_reg.mng_access_reg_cap_mask[
                                                         0]) * 8,
                                              (MLXSW_MMTA_ID & 0xff));
}

const char * sx_get_ga_asic_str(enum asic_ga_index ga)
{
    static const char * const ga_asic_str[] = {
        [GA_ASIC_0] = "ASIC0",
        [GA_ASIC_1] = "ASIC1",
        [GA_ASIC_2] = "ASIC2",
        [GA_ASIC_3] = "ASIC3",
        [GA_ASIC_4] = "ASIC4",
        [GA_ASIC_5] = "ASIC5",
        [GA_ASIC_6] = "ASIC6",
        [GA_ASIC_7] = "ASIC7",
    };

    if ((unsigned int)ga >= ARRAY_SIZE(ga_asic_str)) {
        return "N/A";
    }

    return ga_asic_str[ga];
}

int sx_multi_asic_module_sysfs_map_ga_to_devid(u8 ga, sxd_dev_id_t dev_id)
{
    int ret = 0;

    if (ga > GA_MAX_INDEX) {
        sxd_log_err(SXD_DEV_NONE, "Invalid ga %d for device_id %d.\n", ga, dev_id);
        ret = -EINVAL;
        goto out;
    }
    if (ga_devid_map[ga].inited) {
        sxd_log_err(SXD_DEV_NONE, "Trying to map ga %d to device %d, but it is already mapped to device %d.\n",
                    ga, dev_id, ga_devid_map[ga].dev_id);
        ret = -EINVAL;
        goto out;
    }
    ga_devid_map[ga].dev_id = dev_id;
    ga_devid_map[ga].inited = true;
out:
    return ret;
}

void sx_multi_asic_module_unmap_ga_devid(u8 ga, sxd_dev_id_t dev_id)
{
    if (ga > GA_MAX_INDEX) {
        return;
    }
    ga_devid_map[ga].dev_id = 0;
    ga_devid_map[ga].inited = false;
}

int sx_multi_asic_module_sysfs_get_devid_via_ga(u8 ga, sxd_dev_id_t *dev_id)
{
    int ret = 0;

    if (ga > GA_MAX_INDEX) {
        sxd_log_err(SXD_DEV_NONE, "Invalid ga %d.\n", ga);
        ret = -EINVAL;
        goto out;
    }
    if (!ga_devid_map[ga].inited) {
        sxd_log_err(SXD_DEV_NONE, "ga %d is not mapped to a dev_id.\n", ga);
        ret = -EINVAL;
        goto out;
    }

    if (dev_id) {
        *dev_id = ga_devid_map[ga].dev_id;
    }
out:
    return ret;
}

/*
 *   Convert the module id reported by FW to the global module id used by module sysfs:
 *   - Quantum devices report a local (per-ASIC) module id, which is translated via the MMAM map.
 *   - Ethernet devices already report the global module id, so the conversion is a no-op.
 */
int sx_multi_asic_module_sysfs_get_global_module(struct sx_dev *dev, uint8_t local_module, uint8_t *global_module)
{
    uint8_t ga = 0;
    int     ret = 0;

    if (!dev) {
        sxd_log_err(SXD_DEV_NONE, "Input device is NULL.\n");
        ret = -ENODEV;
        goto out;
    }

    if (local_module >= MAX_MODULE_NUM) {
        sxd_log_err(SXD_DEV_NONE, "Input local module (%u) is out of range (%d).\n", local_module, MAX_MODULE_NUM);
        ret = -EINVAL;
        goto out;
    }

    /* Ethernet devices already report the global module id, so conversion is identity. */
    if (!__is_qtm_device(dev)) {
        if (global_module) {
            *global_module = local_module;
        }
        goto out;
    }

    ga = sx_priv(dev)->dev_info.dev_info_ro.mgir.hw_info.ga;
    if (ga >= GA_MAX_NUM) {
        sxd_log_err(SXD_DEV_NONE, "Input ga (%u) is out of range (%d).\n", ga, GA_MAX_NUM);
        ret = -EINVAL;
        goto out;
    }

    if (!__local_modules[ga][local_module].inited) {
        sxd_log_err(SXD_DEV_NONE, "Input local module (%u) map is not inited.\n", local_module);
        ret = -EINVAL;
        goto out;
    }

    if (global_module) {
        *global_module = __local_modules[ga][local_module].global_module;
    }

out:
    return ret;
}

int sx_multi_asic_module_sysfs_module_is_inited(uint8_t global_module, bool *inited)
{
    int ret = 0;

    if (global_module >= MAX_MODULE_NUM) {
        sxd_log_err(SXD_DEV_NONE, "Input global module (%u) is out of range (%d).\n", global_module,
                    MAX_MODULE_NUM);
        ret = -EINVAL;
        goto out;
    }

    if (inited) {
        *inited = __global_modules[global_module].inited;
    }

out:
    return ret;
}

/*
 *   Used for module related PRM register access, necessary for multi-asic system while not necessary for single asic system (nor independent or standalone module):
 *   - For multi-asic system, local module is used for most module related PRM register access, which may be different from global module.
 *   - For single asic system, local module is assumed to be same as global module for PRM register access.
 *   - For Ethernet devices PRM consumes the global module only, so the conversion is a no-op.
 */
int sx_core_get_possible_local_module(struct sx_dev *dev, uint8_t global_module, uint8_t *local_module)
{
    int  ret = 0;
    bool global_module_map_inited = false;

    if (!dev) {
        sxd_log_err(SXD_DEV_NONE, "Input device is NULL.\n");
        ret = -ENODEV;
        goto out;
    }

    if (local_module) {
        *local_module = global_module;
        if (__is_qtm_device(dev) && sx_core_has_pci_multi_dev()) {
            ret = sx_multi_asic_module_sysfs_module_is_inited(global_module, &global_module_map_inited);
            if (ret) {
                goto out;
            }
            if (global_module_map_inited && local_module) {
                *local_module = __global_modules[global_module].local_module;
            }
        }
    }
out:
    return ret;
}

static void __init_mmam_support(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);

    /* mcam_reg.mng_access_reg_cap_mask is little-endian */
    if (SX_CORE_BITMAP_LE_GET(priv->mcam_reg_data.group_0X9100.mcam_reg.mng_access_reg_cap_mask,
                              SXD_MCAM_MNG_ACCESS_REG_CAP_MASK_NUM,
                              sizeof(priv->mcam_reg_data.group_0X9100.mcam_reg.mng_access_reg_cap_mask[0]) * 8,
                              (MLXSW_MMAM_ID & 0xff))) {
        priv->mmam_support = true;
    } else {
        priv->mmam_support = false;
    }
}

void sx_multi_asic_module_sysfs_env_init(struct sx_dev *dev)
{
    __init_mmam_support(dev);
}

/* Set up map of global module idx - ga - local module id */
static int __init_map_of_global_module_and_ga_local_module(struct sx_dev *dev, uint8_t global_module)
{
    int     ret = 0;
    uint8_t ga = 0;
    uint8_t local_module = 0;
    uint8_t module_type = 0;

    ret = __get_global_module_info_from_mmam(dev,
                                             global_module,
                                             &ga,
                                             &local_module,
                                             &module_type);
    if (ret) {
        goto out;
    }

    __global_modules[global_module].ga = ga;
    __global_modules[global_module].local_module = local_module;
    __global_modules[global_module].module_type = module_type;
    __global_modules[global_module].inited = true;

    __local_modules[ga][local_module].global_module = global_module;
    __local_modules[ga][local_module].inited = true;

out:
    return ret;
}

static int __deinit_map_of_global_module_and_ga_local_module(struct sx_dev *dev, uint8_t global_module)
{
    int     ret = 0;
    uint8_t ga = 0;
    uint8_t local_module = 0;
    uint8_t module_type = 0;

    ret = __get_global_module_info_from_mmam(dev,
                                             global_module,
                                             &ga,
                                             &local_module,
                                             &module_type);
    if (ret) {
        goto out;
    }

    __global_modules[global_module].ga = 0;
    __global_modules[global_module].local_module = 0;
    __global_modules[global_module].module_type = 0;
    __global_modules[global_module].local_port_for_els = 0;
    __global_modules[global_module].inited = false;

    __local_modules[ga][local_module].global_module = 0;
    __local_modules[ga][local_module].inited = false;

out:
    return ret;
}

/*
 *   ga -> pci device id
 *   global module id -> ga, local module id, type (including those backplane types)
 *   <pci device id, local module> -> global module
 */
int sx_multi_asic_module_sysfs_dump(struct seq_file *m, void *v, void *context)
{
    int            i = 0;
    int            j = 0;
    struct sx_dev *dev = NULL;

    /* this function MUST NOT be called from interrupt context */

    seq_printf(m, "Multi-asic Module Sysfs Mappings\n");
    seq_printf(m, "-------------------------------------------------------------------------------\n\n");
    seq_printf(m, "Predefined device IDs: %s\n\n", (sx_core_has_pci_multi_dev() ? "yes" : "no"));
    if (!sx_core_has_pci_multi_dev()) {
        goto out;
    }

    seq_printf(m, "\n%-6s   %-5s   %-9s\n", "GA", "Inited", "Device-Id");
    seq_printf(m, "...............................................................................\n");
    for (i = 0; i < GA_MAX_NUM; i++) {
        if (ga_devid_map[i].inited) {
            dev = sx_dev_db_get_dev_by_id(ga_devid_map[i].dev_id);
            if (dev) {
                seq_printf(m, "%-6s   %-5s   %-9u\n",
                           sx_get_ga_asic_str(i),
                           "yes",
                           ga_devid_map[i].dev_id);
            }
        } else {
            seq_printf(m, "%-6s   %-5s   %-9s\n",
                       sx_get_ga_asic_str(i), "no", "NA");
        }
    }

    seq_printf(m, "\n%-14s   %-6s   %-14s   %s\n", "Global-Module", "GA", "Local-Module", "Module-Type");
    seq_printf(m, "...............................................................................\n");
    for (i = 0; i < MAX_MODULE_NUM; i++) {
        if (!__global_modules[i].inited) {
            continue;
        }
        seq_printf(m, "%-14d   %-6s   %-14u   %u\n", i,
                   sx_get_ga_asic_str(__global_modules[i].ga),
                   __global_modules[i].local_module,
                   __global_modules[i].module_type);
    }

    seq_printf(m, "\n%-6s   %-14s   %-14s\n", "GA", "Local-Module", "Global-Module");
    seq_printf(m, "...............................................................................\n");
    for (i = 0; i < GA_MAX_NUM; i++) {
        for (j = 0; j < MAX_MODULE_NUM; j++) {
            if (!__local_modules[i][j].inited) {
                continue;
            }
            seq_printf(m, "%-6s   %-14u   %-14u\n",
                       sx_get_ga_asic_str(i), j,
                       __local_modules[i][j].global_module);
        }
    }

    if (els_module_found) {
        seq_printf(m, "\n%-14s   %-14s\n", "ELS module", "Local-Port");
        for (j = 0; j < MAX_MODULE_NUM; j++) {
            if (!__global_modules[j].inited || (__global_modules[j].module_type != SXD_PMTM_MODULE_TYPE_OSFP_ELS_E)) {
                continue;
            }
            seq_printf(m, "%-14u   %-14u\n",
                       j,
                       __global_modules[j].local_port_for_els);
        }
    }

out:
    seq_printf(m, "\n\n");
    return 0;
}

int sx_multi_asic_module_sysfs_need_skip(uint8_t module, bool *need_skip)
{
    int ret = 0;

    if (module >= MAX_MODULE_NUM) {
        sxd_log_err(SXD_DEV_NONE, "Input module (%u) is out of range (%d).\n", module, MAX_MODULE_NUM);
        ret = -EINVAL;
        goto out;
    }

    if (!need_skip) {
        goto out;
    }

    *need_skip = false;
    /* if module is not inited or module is initiated as backplane type - skip it*/
    if (sx_core_has_pci_multi_dev()) {
        *need_skip =
            (__global_modules[module].inited) ? MODULE_IS_BACKPLANE_TYPE(__global_modules[module].module_type) : true;
    } else {
        *need_skip = (__modules[module].inited) ? MODULE_IS_BACKPLANE_TYPE(__modules[module].module_type) : true;
    }

out:
    return ret;
}

int sx_multi_asic_module_ind_type(uint8_t module, uint8_t *ind_type)
{
    int ret = 0;

    if (module >= MAX_MODULE_NUM) {
        sxd_log_err(SXD_DEV_NONE, "Input module (%u) is out of range (%d).\n", module, MAX_MODULE_NUM);
        ret = -EINVAL;
        goto out;
    }

    if (!ind_type) {
        goto out;
    }

    *ind_type = 0;
    if (sx_core_has_pci_multi_dev() && __global_modules[module].inited) {
        switch (__global_modules[module].module_type) {
        case SXD_PMTM_MODULE_TYPE_OSFP_ELS_E:
            *ind_type = SXD_PDDR_MODULE_IND_TYPE_ELS_E;
            break;

        case SXD_PMTM_MODULE_TYPE_OE_16X_E:
            *ind_type = SXD_PDDR_MODULE_IND_TYPE_OE_E;
            break;

        default:
            *ind_type = SXD_PDDR_MODULE_IND_TYPE_CPO_OR_PLUGGABLE_MODULES_E;
            break;
        }
    }

out:
    return ret;
}

int sx_module_get_local_port_for_els(uint8_t module, uint8_t *local_port)
{
    int ret = 0;

    if (module >= MAX_MODULE_NUM) {
        sxd_log_err(SXD_DEV_NONE, "%s: Input module (%u) is out of range (%d).\n", __func__, module,
                    MAX_MODULE_NUM);
        ret = -EINVAL;
        goto out;
    }

    if (!local_port) {
        sxd_log_err(SXD_DEV_NONE, "%s: local_port is NULL\n", __func__);
        ret = -EINVAL;
        goto out;
    }

    if (__get_module_type(module) != SXD_PMTM_MODULE_TYPE_OSFP_ELS_E) {
        sxd_log_err(SXD_DEV_NONE, "%s: module (%u) is not an ELS module\n", __func__, module);
        ret = -EINVAL;
        goto out;
    }

    *local_port = 0;
    if (sx_core_has_pci_multi_dev() && __global_modules[module].inited) {
        *local_port = __global_modules[module].local_port_for_els;
    }

out:
    return ret;
}

int sx_module_get_module_type(uint8_t module_id, uint8_t *module_type)
{
    int ret = 0;

    if (module_id >= MAX_MODULE_NUM) {
        sxd_log_err(SXD_DEV_NONE, "%s: Input module (%u) is out of range (%d).\n", __func__, module_id,
                    MAX_MODULE_NUM);
        ret = -EINVAL;
        goto out;
    }

    if (!module_type) {
        sxd_log_err(SXD_DEV_NONE, "%s: module_type is NULL\n", __func__);
        ret = -EINVAL;
        goto out;
    }

    *module_type = (uint8_t)__get_module_type(module_id);

out:
    return ret;
}

uint8_t sx_module_get_max_submodule_num(void)
{
    return __max_submodule_num;
}

static int __get_module_for_port_from_pmdr(struct sx_dev *dev, uint8_t local_port, uint8_t       *module)
{
    int                       err = 0;
    struct ku_access_pmdr_reg pmdr_reg_data;

    memset(&pmdr_reg_data, 0, sizeof(pmdr_reg_data));
    sx_cmd_set_op_tlv(&pmdr_reg_data.op_tlv, MLXSW_PMDR_ID, SXD_EMAD_METHOD_QUERY_E);
    pmdr_reg_data.dev_id = dev->device_id;
    pmdr_reg_data.pmdr_reg.local_port = local_port;

    err = sx_ACCESS_REG_PMDR(dev, &pmdr_reg_data);
    if (err) {
        sxd_log_err(dev,
                    "Failed to access PMDR, dev_id: %d, local_port %u, err=%d.\n",
                    dev->device_id,
                    local_port,
                    err);
        goto out;
    }

    err = sxd_emad_operation_tlv_status_get(&pmdr_reg_data.op_tlv);
    if (err) {
        sxd_log_err(dev, "PMDR return error status %d,dev_id: %d, local_port %u. \n",
                    err,
                    dev->device_id,
                    local_port);
        err = -EINVAL;
        goto out;
    }
    *module = pmdr_reg_data.pmdr_reg.els_module_index;

out:
    return err;
}

/* Set up local_port_for_els in map of global module for ELS modules */
/* This function will be called once as part of the module sysfs tree creation, using device 1 */
static int __init_els_modules_port_map(struct sx_dev *dev)
{
    int     ret = 0;
    uint8_t local_port = 0;
    uint8_t module_id = 0;
    u16     max_ports = 0;

    if (!els_module_found) {
        goto out;
    }

    ret = sx_get_cap_max_switch_ports(dev, &max_ports);
    if (ret) {
        sxd_log_err(dev, "Failed to get max ports, err=%d\n", ret);
        goto out;
    }

    for (local_port = 1; local_port < max_ports; local_port++) {
        ret = __get_module_for_port_from_pmdr(dev,
                                              local_port,
                                              &module_id);
        if (ret) {
            goto out;
        }

        if (__global_modules[module_id].inited &&
            (__global_modules[module_id].module_type == SXD_PMTM_MODULE_TYPE_OSFP_ELS_E)) {
            __global_modules[module_id].local_port_for_els = local_port;
        }
    }

out:
    return ret;
}

static bool __is_multi_asic_module_sysfs(struct sx_priv *priv)
{
    return sx_core_has_pci_multi_dev() && MULTI_ASIC_MODULE_SYSFS_IS_SUPPORT(priv);
}

static bool __is_global_module_owned_by_dev(struct sx_dev *dev, uint16_t module_id)
{
    struct sx_priv *priv = sx_priv(dev);

    if (!__global_modules[module_id].inited) {
        return false;
    }

    return __global_modules[module_id].ga == priv->dev_info.dev_info_ro.mgir.hw_info.ga;
}

static struct kobject * __create_asic_sysfs_kobj(uint8_t asic_id)
{
    char object_name[ASIC_NAME_LEN_MAX + 1];

    memset(object_name, 0, sizeof(object_name));
    snprintf(object_name, sizeof(object_name), "%s%u", ASIC_SYSFS_PREFIX, (unsigned int)asic_id);
    return kobject_create_and_add(object_name, &(THIS_MODULE->mkobj.kobj));
}

static int __ensure_global_module_topology(struct sx_dev *dev)
{
    int      err = 0;
    int      err2 = 0;
    uint16_t h = 0;

    if (__global_topology_inited) {
        goto out;
    }

    err = __init_max_module_num(dev);
    if (err) {
        sxd_log_err(dev, "Failed to get maximum module number for topology init.\n");
        goto out;
    }

    /* Align with single-ASIC create: out-of-range is notice + success (no tree). */
    if (!__max_module_num || (__max_module_num > MAX_MODULE_NUM)) {
        sxd_log_notice(dev,
                       "module topology will not be initialized because __max_module_num (%d) is out of range.\n",
                       __max_module_num);
        goto out;
    }

    for (h = 0; h < __max_module_num; h++) {
        err = __init_map_of_global_module_and_ga_local_module(dev, h);
        if (err) {
            sxd_log_err(dev, "Failed to build global module (%u) map for multi-asic system.\n", h);
            goto deinit_map;
        }
    }

    if (els_module_found) {
        err = __init_els_modules_port_map(dev);
        if (err) {
            sxd_log_err(dev, "Failed to init ELS modules port map for multi-asic system.\n");
            goto deinit_map;
        }
    }

    __global_topology_inited = true;
    goto out;

deinit_map:
    for (; h > 0; h--) {
        err2 = __deinit_map_of_global_module_and_ga_local_module(dev, h - 1);
        if (err2) {
            sxd_log_err(dev, "Failed to deinit global module (%u) map for multi-asic system.\n", h - 1);
        }
    }

out:
    return err;
}

static void __deinit_global_module_topology(struct sx_dev *dev)
{
    int      err = 0;
    uint16_t h = 0;

    if (!__global_topology_inited) {
        return;
    }

    for (h = __max_module_num; h > 0; h--) {
        err = __deinit_map_of_global_module_and_ga_local_module(dev, h - 1);
        if (err) {
            sxd_log_err(dev, "Failed to deinit global module (%u) map for multi-asic system.\n",
                        h - 1);
        }
    }

    __global_topology_inited = false;
    els_module_found = false;
}

static int __init_module_logs(struct sx_dev *dev)
{
    int             err = 0;
    struct sx_priv *priv = sx_priv(dev);

    if (priv->module_log_init) {
        goto out;
    }

    err = sx_internal_log_init(&priv->module_log,
                               dev,
                               SX_INTERNAL_LOG_DEFAULT_CAPACITY,
                               SX_INTERNAL_LOG_SEVERITY_INFO_E,
                               "module_log");
    if (err) {
        sxd_log_err(dev, "Failed to init internal log for module.\n");
        goto out;
    }

    err = sx_internal_log_init(&priv->module_eeprom_log,
                               dev,
                               SX_INTERNAL_LOG_DEFAULT_CAPACITY,
                               SX_INTERNAL_LOG_SEVERITY_INFO_E,
                               "eeprom_module_log");
    if (err) {
        sxd_log_err(dev, "Failed to init internal log for module eeprom.\n");
        goto deinit_module_log;
    }

    err = sx_internal_log_init(&priv->service_effective_log,
                               dev,
                               SX_INTERNAL_LOG_DEFAULT_CAPACITY,
                               SX_INTERNAL_LOG_SEVERITY_INFO_E,
                               "service_effective_log");
    if (err) {
        sxd_log_err(dev, "Failed to init internal log for service effective.\n");
        goto deinit_module_eeprom_log;
    }

    err = sx_internal_log_init(&priv->mcia_error_log,
                               dev,
                               SX_INTERNAL_LOG_DEFAULT_CAPACITY,
                               SX_INTERNAL_LOG_SEVERITY_INFO_E,
                               "mcia_error_log");
    if (err) {
        sxd_log_err(dev, "Failed to init internal log for mcia error.\n");
        goto deinit_service_effective_log;
    }

    priv->module_log_init = true;
    goto out;

deinit_service_effective_log:
    sx_internal_log_deinit(&priv->service_effective_log, dev);
deinit_module_eeprom_log:
    sx_internal_log_deinit(&priv->module_eeprom_log, dev);
deinit_module_log:
    sx_internal_log_deinit(&priv->module_log, dev);

out:
    return err;
}

static void __deinit_module_logs(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);

    if (!priv->module_log_init) {
        return;
    }

    sx_internal_log_deinit(&priv->module_log, dev);
    sx_internal_log_deinit(&priv->module_eeprom_log, dev);
    sx_internal_log_deinit(&priv->service_effective_log, dev);
    sx_internal_log_deinit(&priv->mcia_error_log, dev);
    priv->module_log_init = false;
}

static int __ensure_shared_module_parent(struct sx_dev *dev, struct kobject **parent_pp)
{
    int err = 0;

    if (__shared_module_parent_kobj) {
        *parent_pp = __shared_module_parent_kobj;
        goto out;
    }

    /*
     * asic0 not probed yet (or not yet registered the shared parent): create the
     * same asic0 kobject sx_sysfs_asic_create would. When asic0 later probes, it
     * reuses __shared_module_parent_kobj.
     */
    __shared_module_parent_kobj = __create_asic_sysfs_kobj(0);
    if (!__shared_module_parent_kobj) {
        sxd_log_err(dev, "Failed to create shared asic0 parent for module sysfs\n");
        err = -ENOMEM;
        goto out;
    }
    *parent_pp = __shared_module_parent_kobj;
    sxd_log_info(dev, "Created shared asic0 parent for multi-asic module SysFS\n");

out:
    return err;
}

static void __delete_one_default_module_sysfs_node(struct module_sysfs_node *node, uint16_t module_id)
{
    uint8_t module_type = 0;

    module_type = __get_module_type(module_id);
    sx_core_delete_module_sysfs_default_eeprom_tree(node, module_type);
    sx_core_delete_module_sysfs_extension_for_s3ip(node->module);
    sx_core_delete_module_sysfs_extension_for_misc(node);
    sx_core_delete_module_sysfs_extension_for_quantum(node);
    sx_core_delete_module_sysfs_extension_for_tec(node);
    kobject_put(node->module);
    memset(node, 0, sizeof(*node));
}

/*
 * Delete only modules owned by this ASIC. Preserves static topology, the shared
 * node array, sibling modules, and the shared asic0 parent.
 */
static void __sx_core_delete_owned_modules_sysfs_tree(struct sx_dev *dev)
{
    uint16_t        i = 0;
    bool            module_skip_sysfs_arr[MAX_MODULE_NUM];
    struct sx_priv *priv = sx_priv(dev);
    uint16_t        deleted_count = 0;

    mutex_lock(&__module_sysfs_shared_lock);

    if (!__default_module_sysfs_root_arr || !__max_module_num) {
        priv->module_sysfs_arr = NULL;
        goto unlock;
    }

    memset(&module_skip_sysfs_arr, 0, sizeof(module_skip_sysfs_arr));
    __get_skip_sysfs_mode_for_modules(module_skip_sysfs_arr, __max_module_num);

    for (i = 0; i < __max_module_num; i++) {
        if (module_skip_sysfs_arr[i]) {
            continue;
        }
        if (!__is_global_module_owned_by_dev(dev, i)) {
            continue;
        }
        if (!__default_module_sysfs_root_arr[i].module) {
            continue;
        }

        if (priv->independent_module_params.module_support_type == SXD_MODULE_MASTER_MODE_SW_CONTROL_E) {
            sx_core_delete_module_sysfs_extension_for_indmod(__default_module_sysfs_root_arr[i].module);
            __module_indmod_sysfs_created[i] = false;
        }
        __delete_one_default_module_sysfs_node(&__default_module_sysfs_root_arr[i], i);
        deleted_count++;
    }

    priv->module_sysfs_arr = NULL;
    sxd_log_info(dev, "Deleted %u owned module sysfs node(s).\n", deleted_count);

unlock:
    mutex_unlock(&__module_sysfs_shared_lock);
}

/*
 * Final shared multi-ASIC module sysfs cleanup for sx_core unload / init unwind.
 * Safe to call when no devices remain (or after all PCI removes).
 */
void sx_core_cleanup_multi_asic_module_sysfs(void)
{
    uint16_t i = 0;

    mutex_lock(&__module_sysfs_shared_lock);

    if (__default_module_sysfs_root_arr) {
        for (i = 0; i < __max_module_num; i++) {
            if (!__default_module_sysfs_root_arr[i].module) {
                continue;
            }
            /* Best-effort: remove IM attrs if present, then the default node. */
            sx_core_delete_module_sysfs_extension_for_indmod(__default_module_sysfs_root_arr[i].module);
            __module_indmod_sysfs_created[i] = false;
            __delete_one_default_module_sysfs_node(&__default_module_sysfs_root_arr[i], i);
        }
        vfree(__default_module_sysfs_root_arr);
        __default_module_sysfs_root_arr = NULL;
    }

    memset(__global_modules, 0, sizeof(__global_modules));
    memset(__local_modules, 0, sizeof(__local_modules));
    memset(__modules, 0, sizeof(__modules));
    memset(__module_indmod_sysfs_created, 0, sizeof(__module_indmod_sysfs_created));
    __global_topology_inited = false;
    els_module_found = false;
    module_sysfs_tree_created = false;
    __max_module_num = 0;

    if (__shared_module_parent_kobj) {
        kobject_put(__shared_module_parent_kobj);
        __shared_module_parent_kobj = NULL;
    }

    mutex_unlock(&__module_sysfs_shared_lock);
}

static int __create_one_default_module_sysfs_node(struct sx_dev            *dev,
                                                  struct kobject           *parent,
                                                  struct module_sysfs_node *node,
                                                  uint16_t                  module_id)
{
    int             err = 0;
    char            mod_name[16];
    uint8_t         module_type = 0;
    struct sx_priv *priv = sx_priv(dev);
    bool            tec_support = false;

    memset(mod_name, 0, sizeof(mod_name));
    snprintf(mod_name, sizeof(mod_name), "%s%u", MODULE_NODE_SYSFS_PREFIX, (unsigned int)module_id);
    node->module = kobject_create_and_add(mod_name, parent);
    if (!node->module) {
        err = -ENOMEM;
        goto out;
    }

    module_type = __get_module_type(module_id);
    err = sx_core_create_module_sysfs_default_eeprom_tree(node->module, node, module_type);
    if (err) {
        sxd_log_err(dev, "Failed to create eeprom sysfs for module %u.\n", module_id);
        goto delete_node;
    }

    err = sx_core_create_module_sysfs_extension_for_s3ip(node->module);
    if (err) {
        sxd_log_err(dev, "Failed to create s3ip sysfs for module %u.\n", module_id);
        goto delete_node;
    }

    err = sx_core_create_module_sysfs_extension_for_misc(node, module_type);
    if (err) {
        sxd_log_err(dev, "Failed to create misc sysfs for module %u.\n", module_id);
        goto delete_node;
    }

    if (__is_qtm_device(dev)) {
        err = sx_core_create_module_sysfs_extension_for_quantum(dev, node);
        if (err) {
            sxd_log_err(dev, "Failed to create quantum sysfs for module %u.\n", module_id);
            goto delete_node;
        }
    }

    tec_support = priv->tec_support;
    if (tec_support) {
        err = sx_core_create_module_sysfs_extension_for_tec(node);
        if (err) {
            sxd_log_err(dev, "Failed to create tec sysfs for module %u.\n", module_id);
            goto delete_node;
        }
    }

    goto out;

delete_node:
    __delete_one_default_module_sysfs_node(node, module_id);

out:
    return err;
}

/*
 * Multi-asic module SysFS: create only modules owned by this ASIC under the shared asic0 parent.
 * Topology and the shared node array are system-lifetime; this call is idempotent for
 * already-published owned modules.
 */
int sx_core_create_owned_modules_sysfs_tree(struct sx_dev *dev, struct module_sysfs_node **root_arr)
{
    int             err = 0;
    uint16_t        i = 0;
    uint16_t        created_count = 0;
    bool            module_skip_sysfs_arr[MAX_MODULE_NUM];
    bool            created_modules[MAX_MODULE_NUM];
    struct kobject *parent = NULL;
    struct sx_priv *priv = sx_priv(dev);
    bool            module_log_init = false;

    if (!root_arr) {
        err = -EINVAL;
        goto out;
    }

    memset(created_modules, 0, sizeof(created_modules));
    mutex_lock(&__module_sysfs_shared_lock);

    err = __ensure_global_module_topology(dev);
    if (err) {
        goto unlock;
    }

    if (!__max_module_num || (__max_module_num > MAX_MODULE_NUM)) {
        goto unlock;
    }

    err = __ensure_shared_module_parent(dev, &parent);
    if (err) {
        goto unlock;
    }

    err = __read_apply_im_supported(dev);
    if (err) {
        sxd_log_err(dev, "Failed to read apply immediate is supported.\n");
        goto unlock;
    }

    if (!priv->module_log_init) {
        err = __init_module_logs(dev);
        if (err) {
            goto unlock;
        }
        module_log_init = true;
    }

    if (!__default_module_sysfs_root_arr) {
        /* vzalloc, not kzalloc: this array scales with __max_module_num (up to
         * MAX_MODULE_NUM=255) and sizeof(struct module_sysfs_node) is ~10 KB,
         * so it must avoid the contiguous-page requirement (see the vzalloc
         * comment in sx_core_create_default_modules_sysfs_tree). Freed with
         * vfree() in sx_core_cleanup_multi_asic_module_sysfs(). */
        __default_module_sysfs_root_arr = vzalloc(__max_module_num * sizeof(struct module_sysfs_node));
        if (!__default_module_sysfs_root_arr) {
            sxd_log_err(dev, "Failed to allocate shared module sysfs node array.\n");
            err = -ENOMEM;
            goto deinit_log;
        }
    }

    memset(&module_skip_sysfs_arr, 0, sizeof(module_skip_sysfs_arr));
    __get_skip_sysfs_mode_for_modules(module_skip_sysfs_arr, __max_module_num);

    for (i = 0; i < __max_module_num; i++) {
        if (module_skip_sysfs_arr[i]) {
            continue;
        }
        if (!__is_global_module_owned_by_dev(dev, i)) {
            continue;
        }
        if (__default_module_sysfs_root_arr[i].module) {
            continue;
        }

        err = __create_one_default_module_sysfs_node(dev,
                                                     parent,
                                                     &__default_module_sysfs_root_arr[i],
                                                     i);
        if (err) {
            sxd_log_err(dev, "Failed to create owned module %u sysfs node.\n", i);
            goto rollback;
        }
        created_modules[i] = true;
        created_count++;
    }

    *root_arr = __default_module_sysfs_root_arr;
    sxd_log_info(dev, "Created %u owned module sysfs node(s).\n", created_count);
    goto unlock;

rollback:
    for (i = 0; i < __max_module_num; i++) {
        if (!created_modules[i]) {
            continue;
        }
        __delete_one_default_module_sysfs_node(&__default_module_sysfs_root_arr[i], i);
    }

deinit_log:
    if (module_log_init) {
        __deinit_module_logs(dev);
    }

unlock:
    mutex_unlock(&__module_sysfs_shared_lock);

out:
    return err;
}

int sx_core_create_default_modules_sysfs_tree(struct sx_dev             *dev,
                                              struct kobject            *parent,
                                              struct module_sysfs_node **root_arr)
{
    int             err = 0;
    uint16_t        i = 0;
    uint16_t        h = 0;
    bool            module_skip_sysfs_arr[MAX_MODULE_NUM];
    bool            created_modules[MAX_MODULE_NUM];
    bool            module_log_init = false;
    bool            multi_asic_module_sysfs = false;
    bool            topology_inited_here = false;
    bool            topology_was_inited = false;
    struct sx_priv *priv = sx_priv(dev);

    if (module_sysfs_tree_created) {
        sxd_log_notice(dev, "module sysfs tree was already created!\n");
        return err;
    }

    memset(created_modules, 0, sizeof(created_modules));
    sxd_log_debug(dev, "create module sysfs for dev id %u.\n", dev->device_id);

    multi_asic_module_sysfs = __is_multi_asic_module_sysfs(priv);
    if (multi_asic_module_sysfs) {
        topology_was_inited = __global_topology_inited;
        err = __ensure_global_module_topology(dev);
        if (err) {
            goto out;
        }
        topology_inited_here = !topology_was_inited && __global_topology_inited;
    } else {
        err = __init_max_module_num(dev);
        if (err) {
            sxd_log_err(dev, "module sysfs nodes handler failed to get maximum module number.\n");
            goto out;
        }
    }

    if (!__max_module_num || (__max_module_num > MAX_MODULE_NUM)) {
        sxd_log_notice(dev,
                       "module eeprom sysfs will not be created because __max_module_num (%d) is out of range.\n",
                       __max_module_num);
        goto out;
    }

    err = __read_apply_im_supported(dev);
    if (err) {
        sxd_log_err(dev, "Failed to read apply immediate is supported.\n");
        goto deinit_map;
    }

    err = __init_module_logs(dev);
    if (err) {
        goto deinit_map;
    }
    module_log_init = true;

    /* root_arr is CPU-only sysfs/kobject bookkeeping (no DMA, no userspace
     * mapping). sizeof(struct module_sysfs_node) is ~10 KB and
     * __max_module_num is capped at MAX_MODULE_NUM=255, so this allocation
     * scales to ~2.67 MB - an order:10 contiguous-page request that easily
     * fails under memory fragmentation. The array is only ever indexed in C,
     * never DMA-mapped, so vmalloc-backed memory is safe. Use vzalloc to avoid
     * the contiguous-page requirement entirely.
     */
    *root_arr = vzalloc(__max_module_num * sizeof(struct module_sysfs_node));
    if (!(*root_arr)) {
        sxd_log_err(dev, "module sysfs nodes handler failed to allocated memory.\n");
        err = -ENOMEM;
        goto deinit_map;
    }

    if (!multi_asic_module_sysfs) {
        for (h = 0; h < __max_module_num; h++) {
            err = __init_module_type_map(dev, h);
            if (err) {
                sxd_log_err(dev, "Failed to build module (%u) map.\n", h);
                goto deinit_map;
            }
        }

        if (els_module_found) {
            err = __init_els_modules_port_map(dev);
            if (err) {
                sxd_log_err(dev,
                            "Failed to init ELS modules port map for multi-asic system.\n");
                goto deinit_map;
            }
        }
    }

    memset(&module_skip_sysfs_arr, 0, sizeof(module_skip_sysfs_arr));
    __get_skip_sysfs_mode_for_modules(module_skip_sysfs_arr, __max_module_num);

    for (i = 0; i < __max_module_num; i++) {
        if (module_skip_sysfs_arr[i]) {
            continue;
        }

        err = __create_one_default_module_sysfs_node(dev, parent, &(*root_arr)[i], i);
        if (err) {
            sxd_log_err(dev, "module sysfs nodes handler failed to create module %u sysfs node.\n", i);
            goto phase1_err;
        }
        created_modules[i] = true;
    }

    /* MBBU/MBAU registers (i2c statistics) supported on Ethernet switches, Spectrum3 and above.
     * Do not create the sysfs entry on unsupported chips */
    switch (priv->dev_info.dev_info_ro.chip_type) {
    case SXD_CHIP_TYPE_SPECTRUM:
    case SXD_CHIP_TYPE_SPECTRUM_A1:
    case SXD_CHIP_TYPE_SPECTRUM2:
    case SXD_CHIP_TYPE_QUANTUM:
    case SXD_CHIP_TYPE_QUANTUM2:
    case SXD_CHIP_TYPE_QUANTUM3:
    case SXD_CHIP_TYPE_QUANTUM4:
    case SXD_CHIP_TYPE_QUANTUM5:
        break;

    default: /* Supported on Ethernet switches, Spectrum3 and above */
        err = sysfs_create_file(priv->kobj[SX_KOBJECT_ASIC], &(asic_i2c_utl_attr.attr));
        if (err) {
            sxd_log_err(dev, "Failed to create asic I2C utilization attribute\n");
            goto phase1_err;
        }
        priv->is_i2c_utl_supported = true;
        break;
    }

    module_sysfs_tree_created = true;
    return err;

phase1_err:
    for (i = 0; i < __max_module_num; i++) {
        if (!created_modules[i]) {
            continue;
        }
        __delete_one_default_module_sysfs_node(&(*root_arr)[i], i);
    }
    vfree(*root_arr);
    *root_arr = NULL;

deinit_map:
    if (topology_inited_here) {
        __deinit_global_module_topology(dev);
    } else if (!multi_asic_module_sysfs) {
        for (; h > 0; h--) {
            __deinit_module_type_map(dev, h - 1);
        }
    }

out:
    if (module_log_init) {
        __deinit_module_logs(dev);
    }
    return err;
}

void sx_core_delete_default_modules_sysfs_tree(struct module_sysfs_node **root_arr)
{
    uint16_t i;
    bool     module_skip_sysfs_arr[MAX_MODULE_NUM];

    if (!__max_module_num) {
        return;
    }

    memset(&module_skip_sysfs_arr, 0, sizeof(module_skip_sysfs_arr));
    __get_skip_sysfs_mode_for_modules(module_skip_sysfs_arr, __max_module_num);

    for (i = 0; i < __max_module_num; i++) {
        if (module_skip_sysfs_arr[i]) {
            continue;
        }
        __delete_one_default_module_sysfs_node(&(*root_arr)[i], i);
    }

    if (__default_module_sysfs_root_arr == *root_arr) {
        __default_module_sysfs_root_arr = NULL;
    }
    vfree(*root_arr);
    *root_arr = NULL;

    /* global and local module maps only exist in default module sysfs */
    memset(__global_modules, 0, sizeof(__global_modules));
    memset(__local_modules, 0, sizeof(__local_modules));
    memset(__modules, 0, sizeof(__modules));

    __global_topology_inited = false;
    els_module_found = false;
    module_sysfs_tree_created = false;
}

void sx_core_set_default_modules_sysfs_tree_root(struct module_sysfs_node *root_arr)
{
    __default_module_sysfs_root_arr = root_arr;
}

void sx_core_get_default_modules_sysfs_tree_root(struct module_sysfs_node **root_arr)
{
    if (root_arr) {
        *root_arr = __default_module_sysfs_root_arr;
    }
}

static int __sx_core_create_independent_modules_sysfs_tree(struct sx_dev *dev, struct module_sysfs_node **root_arr)
{
    int      err = 0;
    uint16_t k;
    bool     module_skip_sysfs_arr[MAX_MODULE_NUM];
    bool     multi_asic_module_sysfs = false;

    multi_asic_module_sysfs = __is_multi_asic_module_sysfs(sx_priv(dev));

    memset(&module_skip_sysfs_arr, 0, sizeof(module_skip_sysfs_arr));
    __get_skip_sysfs_mode_for_modules(module_skip_sysfs_arr, __max_module_num);

    for (k = 0; k < __max_module_num; k++) {
        if (module_skip_sysfs_arr[k]) {
            continue;
        }
        if (multi_asic_module_sysfs && !__is_global_module_owned_by_dev(dev, k)) {
            continue;
        }
        if (!(*root_arr)[k].module) {
            continue;
        }
        /* Warm boot / repeated profile: skip modules that already have IM attrs. */
        if (__module_indmod_sysfs_created[k]) {
            continue;
        }
        err = sx_core_create_module_sysfs_extension_for_indmod((*root_arr)[k].module);
        if (err) {
            sxd_log_err(dev,
                        "module sysfs nodes handler failed to create independent module sysfs sub-tree.\n");
            err = -ENOMEM;
            goto out_rollback;
        }
        __module_indmod_sysfs_created[k] = true;
    }

    goto out;

out_rollback:
    for (; k > 0; k--) {
        if (module_skip_sysfs_arr[k - 1]) {
            continue;
        }
        if (multi_asic_module_sysfs && !__is_global_module_owned_by_dev(dev, k - 1)) {
            continue;
        }
        if (!(*root_arr)[k - 1].module) {
            continue;
        }
        if (!__module_indmod_sysfs_created[k - 1]) {
            continue;
        }
        sx_core_delete_module_sysfs_extension_for_indmod((*root_arr)[k - 1].module);
        __module_indmod_sysfs_created[k - 1] = false;
    }

out:
    return err;
}

static void __sx_core_delete_independent_modules_sysfs_tree(struct sx_dev *dev, struct module_sysfs_node **root_arr)
{
    uint16_t i;
    bool     module_skip_sysfs_arr[MAX_MODULE_NUM];
    bool     multi_asic_module_sysfs = false;

    multi_asic_module_sysfs = __is_multi_asic_module_sysfs(sx_priv(dev));

    memset(&module_skip_sysfs_arr, 0, sizeof(module_skip_sysfs_arr));
    __get_skip_sysfs_mode_for_modules(module_skip_sysfs_arr, __max_module_num);

    for (i = 0; i < __max_module_num; i++) {
        if (module_skip_sysfs_arr[i]) {
            continue;
        }
        if (multi_asic_module_sysfs && !__is_global_module_owned_by_dev(dev, i)) {
            continue;
        }
        if (!(*root_arr)[i].module) {
            continue;
        }
        sx_core_delete_module_sysfs_extension_for_indmod((*root_arr)[i].module);
        __module_indmod_sysfs_created[i] = false;
    }
}


int sx_sysfs_asic_independent_init(struct sx_dev *dev)
{
    int             err = 0;
    struct sx_priv *priv = sx_priv(dev);
    uint8_t         i = 0, j = 0;

    priv->independent_module_params.frequency_support = false;

    /*
     * Multi-asic module SysFS: create IM attributes only for modules owned by this ASIC.
     * If this ASIC has not published any module nodes yet, skip sysfs extension
     * but still initialize the per-ASIC module DB below.
     */
    if (priv->module_sysfs_arr) {
        err = __sx_core_create_independent_modules_sysfs_tree(dev,
                                                              &(priv->module_sysfs_arr));
        if (err) {
            goto out;
        }
    }

    for (i = 0; i < MAX_SLOT_NUM; i++) {
        for (j = 0; j < MAX_MODULE_NUM; j++) {
            priv->module_data[i][j].independent_params.hw_present = SX_MODULE_HW_PRESENT_INVALID;
            priv->module_data[i][j].independent_params.interrupt = SX_MODULE_INTERRUPT_INVALID;
            priv->module_data[i][j].independent_params.power_good = SX_MODULE_POWER_GOOD_INVALID;
        }
    }

    __read_frequency_support_to_db(dev);


out:
    return err;
}

int sx_sysfs_asic_create(struct sx_dev *dev)
{
    int             err = 0;
    struct sx_priv *priv = sx_priv(dev);
    uint8_t         asic_id = 0;
    bool            reused_shared_asic0 = false;

    if (priv->dev_specific_cb.multi_asic_sysfs_env_init != NULL) {
        priv->dev_specific_cb.multi_asic_sysfs_env_init(dev);
    }

    if (sx_core_has_pci_multi_dev() && MULTI_ASIC_MODULE_SYSFS_IS_SUPPORT(priv)) {
        err = sx_multi_asic_module_sysfs_map_ga_to_devid(priv->dev_info.dev_info_ro.mgir.hw_info.ga,
                                                         priv->dev.device_id);
        if (err < 0) {
            sxd_log_err(dev, "__sx_map_ga_to_sxdev() failed (err=%d)\n", err);
            goto clear_multi_asic_module_map;
        }
    }

    if (sx_core_has_pci_multi_dev()) {
        asic_id = dev->device_id - 1;
    } else {
        asic_id = 0;
    }

    /*
     * Multi-asic module SysFS: asic0 may already exist as the shared module parent
     * (created by another ASIC's profile set via __ensure_shared_module_parent()).
     * Reuse it so module nodes keep a stable public path under
     * /sys/module/sx_core/asic0/. Lookup, create, and publish happen in one
     * locked section to avoid racing that concurrent creator.
     */
    if ((asic_id == 0) && __is_multi_asic_module_sysfs(priv)) {
        mutex_lock(&__module_sysfs_shared_lock);
        if (__shared_module_parent_kobj) {
            priv->kobj[SX_KOBJECT_ASIC] = __shared_module_parent_kobj;
            kobject_get(priv->kobj[SX_KOBJECT_ASIC]);
            reused_shared_asic0 = true;
        } else {
            priv->kobj[SX_KOBJECT_ASIC] = __create_asic_sysfs_kobj(asic_id);
            if (!priv->kobj[SX_KOBJECT_ASIC]) {
                mutex_unlock(&__module_sysfs_shared_lock);
                sxd_log_err(dev, "Failed to create asic sysfs\n");
                err = -ENOMEM;
                goto clear_multi_asic_module_map;
            }
            __shared_module_parent_kobj = priv->kobj[SX_KOBJECT_ASIC];
            kobject_get(__shared_module_parent_kobj);
        }
        mutex_unlock(&__module_sysfs_shared_lock);
    } else if (!priv->kobj[SX_KOBJECT_ASIC]) {
        priv->kobj[SX_KOBJECT_ASIC] = __create_asic_sysfs_kobj(asic_id);
        if (priv->kobj[SX_KOBJECT_ASIC] == NULL) {
            sxd_log_err(dev, "Failed to create asic sysfs\n");
            err = -ENOMEM;
            goto clear_multi_asic_module_map;
        }
    }

    priv->kobj[SX_KOBJECT_ASIC_TEMP] = kobject_create_and_add("temperature", priv->kobj[SX_KOBJECT_ASIC]);
    if (priv->kobj[SX_KOBJECT_ASIC_TEMP] == NULL) {
        sxd_log_err(dev, "Failed to create asic temperature sysfs\n");
        err = -ENOMEM;
        goto temperature_kobj_failed;
    }

    err = sysfs_create_file(priv->kobj[SX_KOBJECT_ASIC_TEMP], &(asic_temp_highest_attr.attr));
    if (err) {
        sxd_log_err(dev, "Failed to create asic temperature sysfs highest attribute\n");
        goto highest_failed;
    }
    err = sysfs_create_file(priv->kobj[SX_KOBJECT_ASIC_TEMP], &(asic_temp_input_attr.attr));
    if (err) {
        sxd_log_err(dev, "Failed to create asic temperature sysfs input attribute\n");
        goto input_failed;
    }
    err = sysfs_create_file(priv->kobj[SX_KOBJECT_ASIC_TEMP], &(asic_temp_label_attr.attr));
    if (err) {
        sxd_log_err(dev, "Failed to create asic temperature sysfs label attribute\n");
        goto label_failed;
    }
    /* ASIC performance counters*/
    priv->kobj[SX_KOBJECT_ASIC_PERF_CNTR] = kobject_create_and_add("performance", priv->kobj[SX_KOBJECT_ASIC]);
    if (priv->kobj[SX_KOBJECT_ASIC_PERF_CNTR] == NULL) {
        sxd_log_err(dev, "Failed to create asic performance sysfs\n");
        err = -ENOMEM;
        goto perf_cntr_kobj_failed;
    }

    err = sysfs_create_file(priv->kobj[SX_KOBJECT_ASIC_PERF_CNTR], &(asic_perf_cntr_counters_attr.attr));
    if (err) {
        sxd_log_err(dev, "Failed to create asic performance counters sysfs counters attribute\n");
        goto counters_failed;
    }
    err = sysfs_create_file(priv->kobj[SX_KOBJECT_ASIC_PERF_CNTR], &(asic_perf_cntr_interval_attr.attr));
    if (err) {
        sxd_log_err(dev, "Failed to create asic performance counters sysfs interval attribute\n");
        goto interval_failed;
    }
    err = sysfs_create_file(priv->kobj[SX_KOBJECT_ASIC_PERF_CNTR], &(asic_perf_cntr_force_attr.attr));
    if (err) {
        sxd_log_err(dev, "Failed to create asic performance counters sysfs force attribute\n");
        goto force_failed;
    }

    if (sx_core_has_pci_multi_dev() || sx_core_multi_asic_fs_mode()) {
        err = sysfs_create_group(priv->kobj[SX_KOBJECT_ASIC], &local_params_group);
        if (err) {
            sxd_log_err(dev, "Failed to create asic local_parameters sysfs group\n");
            goto local_params_failed;
        }
    }

    return 0;

local_params_failed:
    sysfs_remove_file(priv->kobj[SX_KOBJECT_ASIC_PERF_CNTR], &(asic_perf_cntr_force_attr.attr));

force_failed:
    sysfs_remove_file(priv->kobj[SX_KOBJECT_ASIC_PERF_CNTR], &(asic_perf_cntr_interval_attr.attr));

interval_failed:
    sysfs_remove_file(priv->kobj[SX_KOBJECT_ASIC_PERF_CNTR], &(asic_perf_cntr_counters_attr.attr));

counters_failed:
    kobject_put(priv->kobj[SX_KOBJECT_ASIC_PERF_CNTR]);
    priv->kobj[SX_KOBJECT_ASIC_PERF_CNTR] = NULL;

perf_cntr_kobj_failed:
    sysfs_remove_file(priv->kobj[SX_KOBJECT_ASIC_TEMP], &(asic_temp_label_attr.attr));

label_failed:
    sysfs_remove_file(priv->kobj[SX_KOBJECT_ASIC_TEMP], &(asic_temp_input_attr.attr));

input_failed:
    sysfs_remove_file(priv->kobj[SX_KOBJECT_ASIC_TEMP], &(asic_temp_highest_attr.attr));

highest_failed:
    kobject_put(priv->kobj[SX_KOBJECT_ASIC_TEMP]);
    priv->kobj[SX_KOBJECT_ASIC_TEMP] = NULL;

temperature_kobj_failed:
    if (!reused_shared_asic0) {
        mutex_lock(&__module_sysfs_shared_lock);
        if (__shared_module_parent_kobj == priv->kobj[SX_KOBJECT_ASIC]) {
            /* This device registered the shared asic0 parent above; undo that
             * registration and its extra reference so a later ASIC probe does
             * not reuse a kobject whose creation ultimately failed. */
            __shared_module_parent_kobj = NULL;
            kobject_put(priv->kobj[SX_KOBJECT_ASIC]);
        }
        mutex_unlock(&__module_sysfs_shared_lock);
    }
    kobject_put(priv->kobj[SX_KOBJECT_ASIC]);
    priv->kobj[SX_KOBJECT_ASIC] = NULL;

clear_multi_asic_module_map:
    if (sx_core_has_pci_multi_dev() && MULTI_ASIC_MODULE_SYSFS_IS_SUPPORT(priv)) {
        sx_multi_asic_module_unmap_ga_devid(priv->dev_info.dev_info_ro.mgir.hw_info.ga, priv->dev.device_id);
    }

    return err;
}

void sx_sysfs_asic_remove(struct sx_dev *dev)
{
    struct sx_priv *priv = sx_priv(dev);

    if (sx_core_has_pci_multi_dev() && MULTI_ASIC_MODULE_SYSFS_IS_SUPPORT(priv)) {
        sx_multi_asic_module_unmap_ga_devid(priv->dev_info.dev_info_ro.mgir.hw_info.ga, priv->dev.device_id);
    }

    __deinit_module_eeprom_access_mode(dev);

    if (priv->module_sysfs_arr) {
        if (__is_multi_asic_module_sysfs(priv) &&
            (priv->module_sysfs_arr == __default_module_sysfs_root_arr)) {
            /*
             * Event handlers were unregistered earlier in PCI remove, and
             * generic_wq was destroyed (drained) before this call. Delete only
             * this ASIC's owned modules; keep topology, shared array, sibling
             * nodes, and the shared asic0 parent.
             */
            __sx_core_delete_owned_modules_sysfs_tree(dev);
        } else {
            if (priv->independent_module_params.module_support_type == SXD_MODULE_MASTER_MODE_SW_CONTROL_E) {
                __sx_core_delete_independent_modules_sysfs_tree(dev, &(priv->module_sysfs_arr));
            }
            sx_core_delete_default_modules_sysfs_tree(&(priv->module_sysfs_arr));

            if (priv->is_i2c_utl_supported) {
                sysfs_remove_file(priv->kobj[SX_KOBJECT_ASIC], &(asic_i2c_utl_attr.attr));
            }
        }
    }

    if (priv->module_log_init == true) {
        __deinit_module_logs(dev);
    }

    if (priv->kobj[SX_KOBJECT_ASIC_PERF_CNTR]) {
        kobject_put(priv->kobj[SX_KOBJECT_ASIC_PERF_CNTR]);
    }

    if (priv->kobj[SX_KOBJECT_ASIC_TEMP]) {
        kobject_put(priv->kobj[SX_KOBJECT_ASIC_TEMP]);
    }

    /*/ */

    if (priv->kobj[SX_KOBJECT_ASIC]) {
        if (sx_core_has_pci_multi_dev() || sx_core_multi_asic_fs_mode()) {
            sysfs_remove_group(priv->kobj[SX_KOBJECT_ASIC], &local_params_group);
        }
        kobject_put(priv->kobj[SX_KOBJECT_ASIC]);
        priv->kobj[SX_KOBJECT_ASIC] = NULL;
    }
}

static int __init_mtmp(struct sx_dev *dev, uint8_t slot_id, uint16_t sensor_index)
{
    int                       err = 0;
    struct ku_access_mtmp_reg mtmp_reg_data;

    memset(&mtmp_reg_data, 0, sizeof(mtmp_reg_data));
    sx_cmd_set_op_tlv(&mtmp_reg_data.op_tlv, MLXSW_MTMP_ID, SXD_EMAD_METHOD_QUERY_E);
    mtmp_reg_data.dev_id = dev->device_id;
    mtmp_reg_data.mtmp_reg.sensor_index = sensor_index;
    mtmp_reg_data.mtmp_reg.slot_index = slot_id;

    err = sx_ACCESS_REG_MTMP(dev, &mtmp_reg_data);
    if (err) {
        sxd_log_err(dev, "Failed to access MTMP, err=%d\n", err);
        goto out;
    }

    err = sxd_emad_operation_tlv_status_get(&mtmp_reg_data.op_tlv);
    if (err) {
        sxd_log_err(dev, "Failed to access MTMP, status=%d\n", err);
        err = -EINVAL;
        goto out;
    }

    if (mtmp_reg_data.mtmp_reg.mte == 1) {
        goto out;
    }

    sx_cmd_set_op_tlv(&mtmp_reg_data.op_tlv, MLXSW_MTMP_ID, SXD_EMAD_METHOD_WRITE_E);
    mtmp_reg_data.mtmp_reg.mte = 1;
    mtmp_reg_data.mtmp_reg.mtr = 1;

    err = sx_ACCESS_REG_MTMP(dev, &mtmp_reg_data);
    if (err) {
        sxd_log_err(dev, "Failed to access MTMP, err=%d\n", err);
        goto out;
    }

    err = sxd_emad_operation_tlv_status_get(&mtmp_reg_data.op_tlv);
    if (err) {
        sxd_log_err(dev, "Failed to access MTMP, status=%d\n", err);
        err = -EINVAL;
        goto out;
    }

out:
    return err;
}

int sx_sysfs_asic_init_tempeature(struct sx_dev *dev)
{
    int                        err = 0;
    uint32_t                   sensor_idx = 0;
    struct ku_access_mtecr_reg mtecr_reg_data;

    memset(&mtecr_reg_data, 0, sizeof(mtecr_reg_data));
    sx_cmd_set_op_tlv(&mtecr_reg_data.op_tlv, MLXSW_MTECR_ID, SXD_EMAD_METHOD_QUERY_E);
    mtecr_reg_data.dev_id = dev->device_id;
    mtecr_reg_data.mtecr_reg.slot_index = 0;

    err = sx_ACCESS_REG_MTECR(dev, &mtecr_reg_data);
    if (err) {
        sxd_log_err(dev, "Failed to access MTECR, err=%d\n", err);
        goto out;
    }

    err = sxd_emad_operation_tlv_status_get(&mtecr_reg_data.op_tlv);
    if (err) {
        sxd_log_err(dev, "Failed to access MTECR, status=%d\n", err);
        err = -EINVAL;
        goto out;
    }

    err = __init_mtmp(dev, 0, sensor_idx);
    if (err) {
        sxd_log_err(dev, "Failed to init MTMP, err=%d\n", err);
        goto out;
    }

out:
    return err;
}

static int __sx_core_get_asic_temperature(struct kobject *kobj, struct sx_temperature_params *params)
{
    int                       ret = 0;
    long                      asic_id = 0;
    struct sx_dev            *dev = NULL;
    struct kobject           *kobj_parent_p = kobj->parent;
    uint16_t                  temperature = 0;
    struct ku_access_mtbr_reg mtbr_reg_data;

    ret = sx_sysfs_asic_get_dev_from_kobj(kobj_parent_p, &dev, DRV_NAME);
    if (ret) {
        sxd_log_err(dev, "Failed to get device from kobject, err=%d\n", ret);
        goto out;
    }

    asic_id = dev->device_id - 1;

    memset(params, 0, sizeof(struct sx_temperature_params));

    memset(&mtbr_reg_data, 0, sizeof(mtbr_reg_data));
    mtbr_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mtbr_reg_data.op_tlv, MTBR_REG_ID, SXD_EMAD_METHOD_QUERY_E);

    mtbr_reg_data.mtbr_reg.base_sensor_index = 0;
    mtbr_reg_data.mtbr_reg.slot_index = 0;
    mtbr_reg_data.mtbr_reg.num_rec = 1;

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out;
    }

    ret = sx_ACCESS_REG_MTBR(dev, &mtbr_reg_data);
    if (ret) {
        sxd_log_err(dev, "Failed to access MTBR, err=%d\n", ret);
        goto out_release;
    }

    ret = sxd_emad_operation_tlv_status_get(&mtbr_reg_data.op_tlv);
    if (ret) {
        sxd_log_err(dev, "Failed to access MTBR, status=%d\n", ret);
        ret = -EINVAL;
        goto out_release;
    }

    temperature = mtbr_reg_data.mtbr_reg.temperature_record[0].temperature;
    if ((temperature == 0) || (temperature == SXD_MTBR_NO_CABLE) || (temperature == SXD_MTBR_NO_READ) ||
        (temperature == SXD_MTBR_INVALID_INDEX) || (temperature == SXD_MTBR_READ_FAILED) ||
        (temperature == SXD_MTBR_NO_SENSOR)) {
        ret = -ENODEV;
        sxd_log_debug(dev,
                      "Temperature sensing is not supported on asic, status=0x%x\n",
                      temperature);
        goto out_release;
    }

    params->highest = mtbr_reg_data.mtbr_reg.temperature_record[0].max_temperature;
    params->input = mtbr_reg_data.mtbr_reg.temperature_record[0].temperature;

    sprintf(params->label, "Asic%ld", asic_id);

out_release:
    sx_core_sysfs_dev_release(dev);
out:
    return ret;
}

static ssize_t __show_asic_temp_highest(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                          ret = 0;
    struct sx_temperature_params params;
    int                          len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }


    ret = __sx_core_get_asic_temperature(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get ASIC temperature highest information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%llu\n", params.highest);

    return len;
}

static ssize_t __show_asic_temp_input(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                          ret = 0;
    struct sx_temperature_params params;
    int                          len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_asic_temperature(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get ASIC temperature input information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%llu\n", params.input);

    return len;
}

static ssize_t __show_asic_temp_label(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                          ret = 0;
    struct sx_temperature_params params;
    int                          len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__,
                    ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_asic_temperature(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err(SXD_DEV_NONE, "Failed to get ASIC temperature label information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%s\n", params.label);

    return len;
}


/*performance counters*/
static int __sx_core_get_asic_perf_cntr_interval(struct kobject *kobj, int *interval)
{
    int             ret = 0;
    struct sx_dev  *dev = NULL;
    struct kobject *kobj_parent_p = kobj->parent;
    struct sx_priv *priv = NULL;

    ret = sx_sysfs_asic_get_dev_from_kobj(kobj_parent_p, &dev, DRV_NAME);
    if (ret) {
        sxd_log_err(dev, "sysfs entry performance counters got invalid value, ret=%d\n", ret);
        goto out;
    }

    priv = sx_priv(dev);

    *interval = priv->perf_cntr.interval;

out:
    return ret;
}


static ssize_t __show_local_param_int(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int             ret = 0;
    struct sx_dev  *dev = NULL;
    struct sx_priv *priv = NULL;
    int             val = 0;

    ret = sx_sysfs_asic_get_dev_from_kobj(kobj, &dev, DRV_NAME);
    if (ret) {
        sxd_log_err(dev, "sysfs error while getting dev from kobj ret=%d\n", ret);
        goto out;
    }
    priv = sx_priv(dev);

    if (attr == &lp_tx_debug_attr) {
        val = MODULE_PARAM_INSTANCE_GET(priv, tx_debug);
    } else if (attr == &lp_rx_debug_attr) {
        val = MODULE_PARAM_INSTANCE_GET(priv, rx_debug);
    } else if (attr == &lp_chip_info_type_attr) {
        val = MODULE_PARAM_INSTANCE_GET(priv, chip_info_type);
    } else if (attr == &lp_chip_info_revision_attr) {
        val = MODULE_PARAM_INSTANCE_GET(priv, chip_info_revision);
    } else if (attr == &lp_enable_keep_packet_crc_attr) {
        val = MODULE_PARAM_INSTANCE_GET(priv, enable_keep_packet_crc);
    } else {
        sxd_log_err(dev, "sx_core unknown local param %s\n", attr->attr.name);
        ret = -EINVAL;
        goto out;
    }

    ret = sprintf(buf, "%d\n", val);

out:
    return ret;
}

static ssize_t __store_local_param_int(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len)
{
    int             ret = 0;
    struct sx_dev  *dev = NULL;
    struct sx_priv *priv = NULL;
    int             val = 0;

    ret = sx_sysfs_asic_get_dev_from_kobj(kobj, &dev, DRV_NAME);
    if (ret) {
        sxd_log_err(dev, "sysfs error while getting dev from kobj ret=%d\n", ret);
        goto out;
    }

    priv = sx_priv(dev);
    if (!priv) {
        sxd_log_err(dev, "sysfs error while getting priv from dev ret=%d\n", ret);
        goto out;
    }

    ret = kstrtoint(buf, 10, &val);
    if (ret) {
        sxd_log_err(dev, "sysfs entry / interval got invalid value %d\n", ret);
        goto out;
    }


    if (attr == &lp_tx_debug_attr) {
        MODULE_PARAM_INSTANCE_SET(priv, tx_debug, val);
    } else if (attr == &lp_rx_debug_attr) {
        MODULE_PARAM_INSTANCE_SET(priv, rx_debug, val);
    } else if (attr == &lp_chip_info_type_attr) {
        MODULE_PARAM_INSTANCE_SET(priv, chip_info_type, val);
    } else if (attr == &lp_chip_info_revision_attr) {
        MODULE_PARAM_INSTANCE_SET(priv, chip_info_revision, val);
    } else if (attr == &lp_enable_keep_packet_crc_attr) {
        MODULE_PARAM_INSTANCE_SET(priv, enable_keep_packet_crc, val);
    } else {
        sxd_log_err(dev, "sx_core unknown local param %s\n", attr->attr.name);
        ret = -EINVAL;
        goto out;
    }

    ret = len;

out:
    return ret;
}


static int __sx_core_set_asic_perf_cntr_interval(struct kobject *kobj, int interval)
{
    int                         ret = 0;
    struct sx_dev              *dev = NULL;
    struct kobject             *kobj_parent_p = kobj->parent;
    struct sx_priv             *priv = NULL;
    struct ku_access_mopgcr_reg mopgcr_reg_data;

    ret = sx_sysfs_asic_get_dev_from_kobj(kobj_parent_p, &dev, DRV_NAME);
    if (ret) {
        sxd_log_err(dev, "sysfs entry performance counters got invalid value\n");
        goto out;
    }

    priv = sx_priv(dev);

    priv->perf_cntr.interval = interval;

    memset(&mopgcr_reg_data, 0, sizeof(mopgcr_reg_data));
    mopgcr_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mopgcr_reg_data.op_tlv, MLXSW_MOPGCR_ID, SXD_EMAD_METHOD_WRITE_E);

    mopgcr_reg_data.mopgcr_reg.perf_cnt_interval = interval;

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out;
    }

    ret = sx_ACCESS_REG_MOPGCR(dev, &mopgcr_reg_data);
    if (ret) {
        sxd_log_err(dev, "Failed to access MOPGCR, err=%d\n", ret);
        goto out_release;
    }

    ret = sxd_emad_operation_tlv_status_get(&mopgcr_reg_data.op_tlv);
    if (ret) {
        sxd_log_err(dev, "Failed to access ,MOPGCR, status=%d\n", ret);
        ret = -EINVAL;
        goto out_release;
    }

out_release:
    sx_core_sysfs_dev_release(dev);

out:
    return ret;
}

/* In case hw_unit_id has gaps, validate counter_id is within the valid ranges.
 * Returns true if counter_id is valid.
 * Sets *has_gaps to true when the hw_unit has gaps. */
static bool __perf_cntr_is_counter_id_in_valid_range(size_t                                     hw_unit_id,
                                                     size_t                                     counter_id,
                                                     const struct perf_cntr_hw_unit_gaps_entry *gaps_entries,
                                                     size_t                                     num_of_hw_units_with_gaps,
                                                     bool                                      *has_gaps)
{
    size_t hw_unit_idx, range_idx;

    *has_gaps = false;

    for (hw_unit_idx = 0; hw_unit_idx < num_of_hw_units_with_gaps; hw_unit_idx++) {
        if (unlikely(gaps_entries[hw_unit_idx].hw_unit_id == hw_unit_id)) {
            *has_gaps = true;

            for (range_idx = 0; range_idx < gaps_entries[hw_unit_idx].num_valid_ranges; range_idx++) {
                if ((counter_id >= gaps_entries[hw_unit_idx].valid_ranges[range_idx].start) &&
                    (counter_id <= gaps_entries[hw_unit_idx].valid_ranges[range_idx].end)) {
                    /* counter_id is within a valid range */
                    return true;
                }
            }

            /* HW unit has gaps, counter_id not in any valid range */
            return false;
        }
    }

    /* Likely case - has_gaps is false, validation check not performed */
    return true;
}

static int __perf_cntr_is_counter_id_valid(size_t                                     hw_unit_id,
                                           size_t                                     counter_id,
                                           const struct perf_cntr_hw_unit_gaps_entry *gaps_entries,
                                           size_t                                     num_of_hw_units_with_gaps,
                                           size_t                                     max_cntr_id)
{
    bool has_gaps = false;
    int  ret = 0;

    if ((num_of_hw_units_with_gaps > 0) &&
        !__perf_cntr_is_counter_id_in_valid_range(hw_unit_id, counter_id, gaps_entries, num_of_hw_units_with_gaps,
                                                  &has_gaps)) {
        sxd_log_err(SXD_DEV_NONE,
                    "counter_id %lu for hw_unit_id %lu is not in any valid range (hw unit has gaps)\n",
                    counter_id, hw_unit_id);
        ret = -EINVAL;
        goto out;
    }

    if (!has_gaps && (counter_id > max_cntr_id)) {
        sxd_log_err(SXD_DEV_NONE,
                    "counter_id %lu for hw_unit_id %lu exceeds maximum %lu\n",
                    counter_id, hw_unit_id, max_cntr_id);
        ret = -EINVAL;
        goto out;
    }

out:
    return ret;
}

static int __perf_cntr_validate_counter_id(sxd_chip_types_t chip_type, size_t hw_unit_id, size_t counter_id)
{
    int ret = 0;

    switch (chip_type) {
    case SXD_CHIP_TYPE_SPECTRUM:
    case SXD_CHIP_TYPE_SPECTRUM_A1:
        if (__perf_cntr_is_counter_id_valid(hw_unit_id, counter_id, NULL, 0,
                                            perf_cntr_max_cntr_id_in_hw_unit_arr_spc1[hw_unit_id])) {
            sxd_log_err(SXD_DEV_NONE, "counter_id %lu for hw_unit_id %lu is invalid for SPC1\n",
                        counter_id, hw_unit_id);
            ret = -EINVAL;
            goto out;
        }
        break;

    case SXD_CHIP_TYPE_SPECTRUM2:
        if (__perf_cntr_is_counter_id_valid(hw_unit_id, counter_id, perf_cntr_hw_unit_gaps_spc2,
                                            ARRAY_SIZE(perf_cntr_hw_unit_gaps_spc2),
                                            perf_cntr_max_cntr_id_in_hw_unit_arr_spc2[hw_unit_id])) {
            sxd_log_err(SXD_DEV_NONE, "counter_id %lu for hw_unit_id %lu is invalid for SPC2\n",
                        counter_id, hw_unit_id);
            ret = -EINVAL;
            goto out;
        }
        break;

    case SXD_CHIP_TYPE_SPECTRUM3:
        if (__perf_cntr_is_counter_id_valid(hw_unit_id, counter_id, perf_cntr_hw_unit_gaps_spc3,
                                            ARRAY_SIZE(perf_cntr_hw_unit_gaps_spc3),
                                            perf_cntr_max_cntr_id_in_hw_unit_arr_spc3[hw_unit_id])) {
            sxd_log_err(SXD_DEV_NONE, "counter_id %lu for hw_unit_id %lu is invalid for SPC3\n",
                        counter_id, hw_unit_id);
            ret = -EINVAL;
            goto out;
        }
        break;

    case SXD_CHIP_TYPE_SPECTRUM4:
        if (__perf_cntr_is_counter_id_valid(hw_unit_id, counter_id, perf_cntr_hw_unit_gaps_spc4,
                                            ARRAY_SIZE(perf_cntr_hw_unit_gaps_spc4),
                                            perf_cntr_max_cntr_id_in_hw_unit_arr_spc4[hw_unit_id])) {
            sxd_log_err(SXD_DEV_NONE, "counter_id %lu for hw_unit_id %lu is invalid for SPC4\n",
                        counter_id, hw_unit_id);
            ret = -EINVAL;
            goto out;
        }
        break;

    case SXD_CHIP_TYPE_SPECTRUM5:
        if (__perf_cntr_is_counter_id_valid(hw_unit_id, counter_id, perf_cntr_hw_unit_gaps_spc5,
                                            ARRAY_SIZE(perf_cntr_hw_unit_gaps_spc5),
                                            perf_cntr_max_cntr_id_in_hw_unit_arr_spc5[hw_unit_id])) {
            sxd_log_err(SXD_DEV_NONE, "counter_id %lu for hw_unit_id %lu is invalid for SPC5\n",
                        counter_id, hw_unit_id);
            ret = -EINVAL;
            goto out;
        }
        break;

    case SXD_CHIP_TYPE_SPECTRUM6:
    case SXD_CHIP_TYPE_SPECTRUM7:
        if (__perf_cntr_is_counter_id_valid(hw_unit_id, counter_id, perf_cntr_hw_unit_gaps_spc6,
                                            ARRAY_SIZE(perf_cntr_hw_unit_gaps_spc6),
                                            perf_cntr_max_cntr_id_in_hw_unit_arr_spc6[hw_unit_id])) {
            sxd_log_err(SXD_DEV_NONE, "counter_id %lu for hw_unit_id %lu is invalid for SPC6/SPC7\n",
                        counter_id, hw_unit_id);
            ret = -EINVAL;
            goto out;
        }
        break;

    default:
        sxd_log_err(SXD_DEV_NONE, "chip type %u is unknown \n", chip_type);
        ret = -EINVAL;
        goto out;
    }
out:
    return ret;
}


static int __perf_cntr_validate_hw_unit_id(sxd_chip_types_t chip_type, size_t hw_unit_id)
{
    int ret = 0;


    switch (chip_type) {
    case SXD_CHIP_TYPE_SPECTRUM:
    case SXD_CHIP_TYPE_SPECTRUM_A1:
        if (hw_unit_id >= PERF_CNTR_MAX_HW_UNIT_ID_SPEC1) {
            sxd_log_err(SXD_DEV_NONE, "hw_unit_id %lu value is invalid. The maximum for SPC1 is %u\n",
                        hw_unit_id, (PERF_CNTR_MAX_HW_UNIT_ID_SPEC1 - 1));
            ret = -EINVAL;
            goto out;
        }
        break;

    case SXD_CHIP_TYPE_SPECTRUM2:
        if (hw_unit_id >= PERF_CNTR_MAX_HW_UNIT_ID_SPEC2) {
            sxd_log_err(SXD_DEV_NONE, "hw_unit_id %lu value is invalid. The maximum for SPC2 is %u\n",
                        hw_unit_id, (PERF_CNTR_MAX_HW_UNIT_ID_SPEC2 - 1));
            ret = -EINVAL;
            goto out;
        }
        break;

    case SXD_CHIP_TYPE_SPECTRUM3:
        if (hw_unit_id >= PERF_CNTR_MAX_HW_UNIT_ID_SPEC3) {
            sxd_log_err(SXD_DEV_NONE, "hw_unit_id %lu value is invalid. The maximum for SPC3 is %u\n",
                        hw_unit_id, (PERF_CNTR_MAX_HW_UNIT_ID_SPEC3 - 1));
            ret = -EINVAL;
            goto out;
        }
        break;

    case SXD_CHIP_TYPE_SPECTRUM4:
        if (hw_unit_id >= PERF_CNTR_MAX_HW_UNIT_ID_SPEC4) {
            sxd_log_err(SXD_DEV_NONE, "hw_unit_id %lu value is invalid. The maximum for SPC4 is %u\n",
                        hw_unit_id, (PERF_CNTR_MAX_HW_UNIT_ID_SPEC4 - 1));
            ret = -EINVAL;
            goto out;
        }
        break;

    case SXD_CHIP_TYPE_SPECTRUM5:
        if (hw_unit_id >= PERF_CNTR_MAX_HW_UNIT_ID_SPEC5) {
            sxd_log_err(SXD_DEV_NONE, "hw_unit_id %lu value is invalid. The maximum for SPC5 is %u\n",
                        hw_unit_id, (PERF_CNTR_MAX_HW_UNIT_ID_SPEC5 - 1));
            ret = -EINVAL;
            goto out;
        }
        break;

    case SXD_CHIP_TYPE_SPECTRUM6:
    case SXD_CHIP_TYPE_SPECTRUM7:
        if (hw_unit_id >= PERF_CNTR_MAX_HW_UNIT_ID_SPEC6) {
            sxd_log_err(SXD_DEV_NONE, "hw_unit_id %lu value is invalid. The maximum for SPC6/SPC7 is %u\n",
                        hw_unit_id, (PERF_CNTR_MAX_HW_UNIT_ID_SPEC6 - 1));
            ret = -EINVAL;
            goto out;
        }
        break;

    default:
        sxd_log_err(SXD_DEV_NONE, "chip type %u is unknown \n", chip_type);
        ret = -EINVAL;
        goto out;
    }
out:
    return ret;
}


static int __perf_cntr_validate_hw_unit_and_cntr_id(size_t hw_unit_id, size_t counter_id, sxd_chip_types_t chip_type)
{
    int ret = 0;


    ret = __perf_cntr_validate_hw_unit_id(chip_type, hw_unit_id);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "Invalid hw_unit_id\n");
        goto out;
    }

    ret = __perf_cntr_validate_counter_id(chip_type, hw_unit_id, counter_id);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "Invalid counter_id\n");
        goto out;
    }
out:
    return ret;
}

static int __perf_cntr_get_hw_unit_max_counters(sxd_chip_types_t chip_type, int hw_unit_id)
{
    int ret = 0;

    switch (chip_type) {
    case SXD_CHIP_TYPE_SPECTRUM:
    case SXD_CHIP_TYPE_SPECTRUM_A1:
        return perf_cntr_hw_unit_max_cntr_arr_spc1[hw_unit_id];

    case SXD_CHIP_TYPE_SPECTRUM2:
        return perf_cntr_hw_unit_max_cntr_arr_spc2[hw_unit_id];

    case SXD_CHIP_TYPE_SPECTRUM3:
        return perf_cntr_hw_unit_max_cntr_arr_spc3[hw_unit_id];

    case SXD_CHIP_TYPE_SPECTRUM4:
        return perf_cntr_hw_unit_max_cntr_arr_spc4[hw_unit_id];

    case SXD_CHIP_TYPE_SPECTRUM5:
        return perf_cntr_hw_unit_max_cntr_arr_spc5[hw_unit_id];

    case SXD_CHIP_TYPE_SPECTRUM6:
    case SXD_CHIP_TYPE_SPECTRUM7:
        return perf_cntr_hw_unit_max_cntr_arr_spc6[hw_unit_id];

    default:
        sxd_log_err(SXD_DEV_NONE, "chip type %u is unknown \n", chip_type);
        ret = -EINVAL;
        goto out;
    }
out:
    return ret;
}


static struct mopct_node* __perf_cntr_allocate_mopct_node(int hw_unit_id, int counter_id, int group_id, u8 force)
{
    struct mopct_node *mopct_node = NULL;

    mopct_node = (struct mopct_node*)kmalloc(sizeof(struct  mopct_node), GFP_KERNEL);

    if (mopct_node) {
        INIT_LIST_HEAD(&mopct_node->hw_unit_list);
        INIT_LIST_HEAD(&mopct_node->group_id_list);

        mopct_node->mopct.cnt_grp_id = group_id;
        mopct_node->mopct.enabled_counters[0] = counter_id;
        mopct_node->mopct.force = force;
        mopct_node->mopct.hw_unit_id = hw_unit_id;
        mopct_node->mopct.num_active_counters = 1;
    }

    return mopct_node;
}


static int __perf_cntr_add_new_mopct_node_to_list(struct sx_perf_cntr *perf_cntr_p, int hw_unit_id, int counter_id)
{
    struct mopct_node *new_mopct_node = NULL;
    int                group_id = 0;
    int                ret = 0;

    if (perf_cntr_p == NULL) {
        ret = -EINVAL;
        goto out;
    }
    /*Need to add new node to the hw_unit_id and the group_id lists */
    group_id = perf_cntr_p->hw_units_arr[hw_unit_id].mopct_in_hw_unit;

    new_mopct_node = __perf_cntr_allocate_mopct_node(hw_unit_id,
                                                     counter_id,
                                                     group_id,
                                                     perf_cntr_p->force);
    if (new_mopct_node == NULL) {
        sxd_log_err(SXD_DEV_NONE, "Cannot add new performance counters entry.\n");
        ret = -ENOMEM;
        goto out;
    }

    list_add_tail(&new_mopct_node->hw_unit_list, &perf_cntr_p->hw_units_arr[hw_unit_id].mopct_list);
    list_add(&new_mopct_node->group_id_list, &perf_cntr_p->counter_group_arr[group_id].mopct_list);

    perf_cntr_p->counter_group_arr[group_id].mopct_in_group++;
    perf_cntr_p->hw_units_arr[hw_unit_id].mopct_in_hw_unit++;


out:
    return ret;
}


static int __perf_cntr_add_pair_to_counters_db(struct sx_dev   *dev,
                                               int              hw_unit_id,
                                               int              counter_id,
                                               sxd_chip_types_t chip_type)
{
    struct sx_priv    *priv = NULL;
    struct mopct_node *last_mopct_node = NULL;
    int                hw_unit_max_counters = 0;
    int                ret = 0;

    priv = sx_priv(dev);

    /* Retrieve the hw_unit_id mopct list
     * If list is empty, add new a mopct node.
     * If list is not empty, we should look at the last mopct node,
     * We check if counters_num <= max_counters_num for the hw_unit_id.
     * If the last node still has a room, add the counter_id to the node.
     * If not, we should generate a new mopct node and add it to the end of the list.
     */
    if (list_empty(&priv->perf_cntr.hw_units_arr[hw_unit_id].mopct_list) == 0) { /* list is not empty*/
        hw_unit_max_counters = __perf_cntr_get_hw_unit_max_counters(chip_type, hw_unit_id);

        last_mopct_node = list_last_entry(&priv->perf_cntr.hw_units_arr[hw_unit_id].mopct_list,
                                          struct mopct_node,
                                          hw_unit_list);
        if (last_mopct_node->mopct.num_active_counters < hw_unit_max_counters) {
            last_mopct_node->mopct.enabled_counters[last_mopct_node->mopct.num_active_counters] = counter_id;
            last_mopct_node->mopct.num_active_counters++;
        } else {
            ret = __perf_cntr_add_new_mopct_node_to_list(&priv->perf_cntr, hw_unit_id, counter_id);
            if (ret) {
                sxd_log_err(dev, "failed to add new sysfs entry performance counter\n");
                goto out;
            }
        }
    } else {
        ret = __perf_cntr_add_new_mopct_node_to_list(&priv->perf_cntr, hw_unit_id, counter_id);
        if (ret) {
            sxd_log_err(dev,
                        "failed to add new counter_id [0x%x], hw_unit_id [0x%x] for sysfs entry performance counter\n",
                        counter_id,
                        hw_unit_id);
            goto out;
        }
    }

out:
    return ret;
}

char * __sx_sysfs_get_str(char *buffer, char **str, const char* delimiters)
{
    char *running;
    char *token;

    running = buffer;
    token = strsep(&running, delimiters);
    if (token == NULL) {
        *str = 0;
        return NULL;
    }

    *str = token;
    return running;
}


char * __sx_sysfs_get_id_str(char *buffer, char **str)
{
    const char delimiters[] = ":";
    char      *running;

    running = __sx_sysfs_get_str(buffer, str, delimiters);
    return running;
}

char * __sx_sysfs_get_hw_unit_counter_pair_str(char *buffer, char **str)
{
    const char delimiters[] = "\r\n";
    char      *running;

    running = __sx_sysfs_get_str(buffer, str, delimiters);
    return running;
}


static ssize_t __show_asic_perf_cntr_interval(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    int interval = 0;
    int len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__,
                    ret);
        ret = 0;
        /*continue*/
    }

    ret = __sx_core_get_asic_perf_cntr_interval(kobj, &interval);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "Failed to get module performance counter information\n");
        return ret;
    }

    len = sprintf(buf, "%d\n", interval);

    return len;
}

static ssize_t __store_asic_perf_cntr_interval(struct kobject        *kobj,
                                               struct kobj_attribute *attr,
                                               const char            *buf,
                                               size_t                 len)
{
    int ret = 0;
    int interval = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, buf, len);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__,
                    ret);
        ret = 0;
        /*continue*/
    }

    ret = kstrtoint(buf, 10, &interval);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "sysfs entry /interval got invalid value %d\n", ret);
        ret = -EINVAL;
        goto out;
    }

    /* Interval valid range is 100usec to 1sec
     * interval units are 100usec*/
    if ((interval < PERF_CNTR_INTERVAL_MIN_VALUE) || (interval > PERF_CNTR_INTERVAL_MAX_VALUE)) {
        sxd_log_err(SXD_DEV_NONE,
                    "sysfs entry interval value is out of range [%d, %d]\n",
                    PERF_CNTR_INTERVAL_MIN_VALUE,
                    PERF_CNTR_INTERVAL_MAX_VALUE);
        ret = -EINVAL;
        goto out;
    }

    ret = __sx_core_set_asic_perf_cntr_interval(kobj, interval);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "Failed to get module performance counter information\n");
        goto out;
    }
    return len;

out:
    return ret;
}

static ssize_t __show_asic_perf_cntr_force(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int             ret = 0;
    int             len = 0;
    struct sx_dev  *dev = NULL;
    struct sx_priv *priv = NULL;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__,
                    ret);
        ret = 0;
        /*continue*/
    }

    ret = sx_sysfs_asic_get_dev_from_kobj(kobj->parent, &dev, DRV_NAME);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "Failed to get dev from kobject, ret=%d\n", ret);
        return ret;
    }

    priv = sx_priv(dev);
    len = sprintf(buf, "%u\n", priv->perf_cntr.force);

    return len;
}

static ssize_t __store_asic_perf_cntr_force(struct kobject        *kobj,
                                            struct kobj_attribute *attr,
                                            const char            *buf,
                                            size_t                 len)
{
    int             ret = 0;
    u8              force = 0;
    struct sx_dev  *dev = NULL;
    struct sx_priv *priv = NULL;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, buf, len);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__,
                    ret);
        ret = 0;
        /*continue*/
    }

    ret = kstrtou8(buf, 10, &force);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "sysfs entry force got invalid value %d\n", ret);
        ret = -EINVAL;
        goto out;
    }

    ret = sx_sysfs_asic_get_dev_from_kobj(kobj->parent, &dev, DRV_NAME);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "Failed to get dev from kobject, ret=%d\n", ret);
        goto out;
    }

    priv = sx_priv(dev);

    priv->perf_cntr.force = !!force;

    return len;

out:
    return ret;
}

static ssize_t __show_asic_perf_cntr_counters(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                ret = 0;
    struct sx_dev     *dev = NULL;
    struct kobject    *kobj_parent_p = kobj->parent;
    struct sx_priv    *priv = NULL;
    struct mopct_node *iter = NULL;
    int                hw_unit_index = 0;
    int                counter_index = 0;
    ssize_t            len = 0, buf_len = 0;
    ssize_t            remaining = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__,
                    ret);
        ret = 0;
        /*continue*/
    }
    ret = sx_sysfs_asic_get_dev_from_kobj(kobj_parent_p, &dev, DRV_NAME);
    if (ret) {
        sxd_log_err(dev, "sysfs entry performance counters got invalid value\n");
        return ret;
    }

    priv = sx_priv(dev);

    /* Bounded writes directly into the sysfs buffer using scnprintf, which
     * returns the number of bytes written (excluding the trailing NUL) and
     * always NUL-terminates when size > 0. Do not assume buf is pre-zeroed. */
    for (hw_unit_index = 0; hw_unit_index < PERF_CNTR_MAX_HW_UNIT_ID; hw_unit_index++) {
        if (!list_empty(&priv->perf_cntr.hw_units_arr[hw_unit_index].mopct_list)) {
            list_for_each_entry(iter, &priv->perf_cntr.hw_units_arr[hw_unit_index].mopct_list, hw_unit_list){
                for (counter_index = 0; counter_index < (iter->mopct.num_active_counters); counter_index++) {
                    remaining = SYSFS_MAX_BUFF_LEN - buf_len;
                    /* Need at least 2 bytes left (1 char + NUL); stop cleanly otherwise. */
                    if (remaining <= 1) {
                        goto out;
                    }
                    len = scnprintf(buf + buf_len, remaining, "0x%x:0x%x\n",
                                    iter->mopct.hw_unit_id, iter->mopct.enabled_counters[counter_index]);
                    if (len < 0) {
                        sxd_log_err(dev, "sysfs entry performance counters internal error. \n");
                        goto out;
                    }
                    buf_len += len;
                }
            }
        }
    }
out:
    return buf_len;
}

static ssize_t __show_asic_i2c_utl(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int             ret = 0;
    int             len = 0;
    struct sx_dev  *dev = NULL;
    struct kobject *kobj_parent_p = kobj->parent;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__,
                    ret);
        ret = 0;
        /*continue*/
    }

    ret = sx_sysfs_asic_get_dev_from_kobj(kobj_parent_p, &dev, DRV_NAME);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "sysfs entry performance counters got invalid value\n");
        return ret;
    }

    len = sx_module_parallel_mcia_access_i2c_statistics_read(dev, buf);
    if (len <= 0) {
        sxd_log_err(dev, "Failed to get i2c statistics\n");
    }

    return len;
}

static ssize_t __store_asic_i2c_utl(struct kobject        *kobj,
                                    struct kobj_attribute *attr,
                                    const char            *buf,
                                    size_t                 len)
{
    int             ret = 0;
    int             value;
    struct sx_dev  *dev = NULL;
    struct kobject *kobj_parent_p = kobj->parent;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, buf, len);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__,
                    ret);
        ret = 0;
        /*continue*/
    }

    ret = kstrtoint(buf, 10, &value);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "sysfs entry /i2c_utilization got invalid input parameter %d\n", ret);
        ret = -EINVAL;
        goto out_err;
    }

    if (value != 0) {
        sxd_log_err(SXD_DEV_NONE, "sysfs entry /i2c_utilization only valid input parameter is 0, got %d\n", value);
        ret = -EINVAL;
        goto out_err;
    }

    ret = sx_sysfs_asic_get_dev_from_kobj(kobj_parent_p, &dev, DRV_NAME);
    if (ret) {
        sxd_log_err(dev, "sysfs entry performance counters got invalid value\n");
        goto out_err;
    }

    ret = sx_module_parallel_mcia_access_i2c_statistics_clear(dev);
    if (ret) {
        goto out_err;
    }

    return len;

out_err:
    return ret;
}

int find_full_path(const struct kobject *kobj, const struct attribute *attr, char *path_p)
{
#define SYS_PREFIX     "/sys/"
#define SYS_PREFIX_LEN (sizeof(SYS_PREFIX) - 1)
    const struct kobject *kobj_iter = NULL;
    size_t                written = 0;

    if (!kobj || !attr || !path_p) {
        return -EINVAL;
    }

    /* Handle case: when `kobj` has no parent (mounted directly under THIS_MODULE kobj) */
    if (!kobj->parent) {
        written = snprintf(path_p, SXD_SYSFS_FILE_PATH_MAX, SYS_PREFIX "module/%s/%s", attr->name, kobject_name(kobj));
        if (written >= SXD_SYSFS_FILE_PATH_MAX) {
            return -ENOSPC;
        }
    } else {
        /* Normal case: construct the path in `buf` directly */
        written = snprintf(path_p, SXD_SYSFS_FILE_PATH_MAX, "%s/%s", kobject_name(kobj), attr->name);
        if (written >= SXD_SYSFS_FILE_PATH_MAX) {
            return -ENOSPC;
        }

        kobj_iter = kobj;
        while ((kobj_iter = kobj_iter->parent) != NULL) {
            /* Prepend the parent's name to the path */
            size_t prefix_len = strlen(kobject_name(kobj_iter)) + 1; /* +1 for '/' */
            if (prefix_len + written + 1 > SXD_SYSFS_FILE_PATH_MAX) {
                return -ENOSPC;
            }

            /* Move the current path forward within `buf` to make space for the parent prefix */
            memmove(path_p + prefix_len, path_p, written + 1); /* +1 for the null terminator */
            memcpy(path_p, kobject_name(kobj_iter), prefix_len - 1);
            path_p[prefix_len - 1] = '/';

            written += prefix_len;
        }

        /* Prepend "/sys" to the path */
        if (SYS_PREFIX_LEN + written + 1 > SXD_SYSFS_FILE_PATH_MAX) {
            return -ENOSPC;
        }

        /* Move existing path forward and insert "/sys" */
        memmove(path_p + SYS_PREFIX_LEN, path_p, written + 1); /* +1 for null terminator */
        memcpy(path_p, SYS_PREFIX, SYS_PREFIX_LEN);
    }

    return 0;
}


static ssize_t __store_asic_perf_cntr_counters(struct kobject        *kobj,
                                               struct kobj_attribute *attr,
                                               const char            *buf,
                                               size_t                 len)
{
    int              ret = 0;
    struct sx_dev   *dev = NULL;
    char            *running = NULL;
    char            *hw_unit_counter_id_pair = NULL;
    char            *token = NULL;
    size_t           hw_unit_id = 0;
    size_t           counter_id = 0;
    struct kobject  *kobj_parent_p = kobj->parent;
    struct sx_priv  *priv = NULL;
    sxd_chip_types_t chip_type = SXD_CHIP_TYPE_UNKNOWN;
    size_t           buff_parsed_len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, buf, len);
    if (ret) {
        sxd_log_err(SXD_DEV_NONE, "sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__,
                    ret);
        ret = 0;
        /*continue*/
    }

    ret = sx_sysfs_asic_get_dev_from_kobj(kobj_parent_p, &dev, DRV_NAME);
    if (ret) {
        sxd_log_err(dev, "sysfs entry performance counters got invalid value\n");
        goto out;
    }

    priv = sx_priv(dev);
    chip_type = priv->dev_info.dev_info_ro.chip_type;

    running = (char*)buf;

    /* Parse the counters file and store in DB. The DB is array of linked lists.
     * The counter file holds a list of <hw_unit_id : counter_id>.
     * Each element in the DB array is a head of a hw_unit list.
     * The index to the DB array is the hw_unit_id.
     * Each hw_unit list builds up from MOPCT registers that stores the counters of the hw_unit_id.
     * Every node in the lists is ku_mopct_reg.
     * Every time the user opens the counters file, the DB is initialize with the new counter file content.
     */
    sx_asic_perf_counter_deinit(dev);
    sx_asic_perf_counter_init(dev);

    while (*running) {
        if (buff_parsed_len >= (len - 1)) {
            sxd_log_notice(dev, "Reached end of counters file buffer. %ld \n", (len - 1));
            goto out;
        }
        running = __sx_sysfs_get_hw_unit_counter_pair_str(running, &hw_unit_counter_id_pair);
        buff_parsed_len += strlen(hw_unit_counter_id_pair); /*the hw_unit_id:counter_id length + \n . */
        buff_parsed_len++;
        sxd_log_debug(dev, "buff_parsed_len: %ld, hw_unit_counter_id_pair %s\n",
                      buff_parsed_len, hw_unit_counter_id_pair);

        hw_unit_counter_id_pair = __sx_sysfs_get_id_str(hw_unit_counter_id_pair, &token);
        ret = kstrtol(token, 0, &hw_unit_id);
        if (ret) {
            sxd_log_err(dev, "Failed to get hw unit id, err=%d, token: %s\n", ret, token);
            ret = -EINVAL;
            goto out;
        }
        hw_unit_counter_id_pair = __sx_sysfs_get_id_str(hw_unit_counter_id_pair, &token);
        ret = kstrtol(token, 0, &counter_id);
        if (ret) {
            sxd_log_err(dev, "Failed to get counter id, err=%d, token: %s\n", ret, token);
            ret = -EINVAL;
            goto out;
        }

        ret = __perf_cntr_validate_hw_unit_and_cntr_id(hw_unit_id, counter_id, chip_type);
        if (ret) {
            sxd_log_err(dev,
                        "counter_id [0x%lx] or hw_unit_id [0x%lx] value is invalid and will be ignored.\n",
                        counter_id,
                        hw_unit_id);
            continue;
        }
        ret = __perf_cntr_add_pair_to_counters_db(dev, hw_unit_id, counter_id, chip_type);
        if (ret != 0) {
            sxd_log_err(dev, "Failed to process counter_id [0x%lx], hw_unit_id [0x%lx].\n",
                        counter_id, hw_unit_id);
            goto out;
        }
    }

out:
    return len;
}


int sx_sysfs_asic_perf_cntr_counters_db_groups_num_get(struct sx_dev *dev, size_t *perf_counter_groups_max)
{
    int             ret = 0;
    struct sx_priv *priv = NULL;
    int             i = 0;

    priv = sx_priv(dev);

    for (i = 0; i < PERF_CNTR_MAX_GROUP_ID; i++) {
        if (priv->perf_cntr.counter_group_arr[i].mopct_in_group == 0) {
            /*search for the first empty group. */
            *perf_counter_groups_max = i;
            break;
        }
    }
    return ret;
}
EXPORT_SYMBOL(sx_sysfs_asic_perf_cntr_counters_db_groups_num_get);

int sx_sysfs_asic_perf_cntr_counters_db_get(struct sx_dev *dev, const struct group_id_counters_list **counters_bank)
{
    int             ret = 0;
    struct sx_priv *priv = NULL;

    priv = sx_priv(dev);

    *counters_bank = priv->perf_cntr.counter_group_arr;

    return ret;
}
EXPORT_SYMBOL(sx_sysfs_asic_perf_cntr_counters_db_get);

ssize_t sx_asic_perf_counter_init(struct sx_dev *dev)
{
    int             err = 0;
    struct sx_priv *priv = sx_priv(dev);
    int             i = 0;

    for (i = 0; i < PERF_CNTR_MAX_HW_UNIT_ID; i++) {
        INIT_LIST_HEAD(&priv->perf_cntr.hw_units_arr[i].mopct_list);
        priv->perf_cntr.hw_units_arr[i].mopct_in_hw_unit = 0;
    }

    for (i = 0; i < PERF_CNTR_MAX_GROUP_ID; i++) {
        INIT_LIST_HEAD(&priv->perf_cntr.counter_group_arr[i].mopct_list);
        priv->perf_cntr.counter_group_arr[i].mopct_in_group = 0;
    }

    return err;
}

void sx_asic_perf_counter_deinit(struct sx_dev *dev)
{
    struct sx_priv    *priv = sx_priv(dev);
    int                i = 0;
    struct mopct_node *iter, *tmp;

    for (i = 0; i < PERF_CNTR_MAX_HW_UNIT_ID; i++) {
        list_for_each_entry_safe(iter, tmp, &priv->perf_cntr.hw_units_arr[i].mopct_list, hw_unit_list) {
            list_del(&iter->hw_unit_list);
            /* We don't free the allocation here because hw_unit_list and group_id_list are pointing to the same allocation*/
        }
    }

    for (i = 0; i < PERF_CNTR_MAX_GROUP_ID; i++) {
        list_for_each_entry_safe(iter, tmp, &priv->perf_cntr.counter_group_arr[i].mopct_list, group_id_list) {
            list_del(&iter->group_id_list);
            kfree(iter);
        }
    }

    return;
}


int send_sysfs_common_sniffer_event(struct kobject                 *kobj,
                                    struct attribute               *attr,
                                    struct sx_dev                  *dev,
                                    const char                     *cb_name,
                                    const char                     *buf,
                                    size_t                          size,
                                    enum sysfs_sniffer_event_type_e sysfs_type)
{
    int                                      ret = 0;
    struct sx_timeval                        now;
    sxd_sysfs_access_sniffer_notification_t *sysfs_access_sniff_p = NULL;
    int                                      sniff_struct_len = sizeof(sxd_sysfs_access_sniffer_notification_t);
    struct sx_priv                          *priv = NULL;

    priv = sx_priv(dev);

    if (priv->sniffer_enable == false) {
        goto out;
    }

    if (((size > 0) && (buf == NULL)) || ((size == 0) && (buf != NULL))) {
        sxd_log_err(dev,
                    "sysfs entry %s failed to send sys sniffer event. Invalid buffer value\n",
                    cb_name);
        ret = -EINVAL;
        goto out;
    } else {
        sniff_struct_len += size;
    }

    sysfs_access_sniff_p = (sxd_sysfs_access_sniffer_notification_t*)kzalloc(sniff_struct_len + 1, GFP_KERNEL);
    if (!sysfs_access_sniff_p) {
        sxd_log_err(dev,
                    "module sysfs, sx_core_send_sniffer_event() failed to allocated memory.\n");
        ret = -ENOMEM;
        goto out;
    }

    ret = find_full_path(kobj, attr, sysfs_access_sniff_p->file_path);
    if (ret) {
        sxd_log_err(dev, "find_full_path faild for cb %s, ret [%d]\n", cb_name, ret);
        goto out;
    }

    sysfs_access_sniff_p->device_id = dev->device_id;
    strncpy(sysfs_access_sniff_p->func_name, cb_name, SXD_SYSFS_CB_NAME_MAX);
    sysfs_access_sniff_p->len = size;

    if (size > 0) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0))
        memcpy(sysfs_access_sniff_p->buf, buf, size);
#else
        memcpy(sysfs_access_sniff_p->buf, buf, flex_array_size(sysfs_access_sniff_p, buf, size));
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0)) */
    }
    sysfs_access_sniff_p->buf[size] = '\0';

    sx_dbg_get_time(&now);
    sysfs_access_sniff_p->timestamp.tv_sec = now.t.tv_sec;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    sysfs_access_sniff_p->timestamp.tv_nsec = now.t.tv_nsec;
#else
    sysfs_access_sniff_p->timestamp.tv_nsec = 1000 * now.t.tv_usec;
#endif

    sxd_log_debug(dev,
                  "Sending sysfs access notification [dev=%u, cb_name=%s, path=%s, total len = %d, write_buf_len = %d, buf = %s]\n",
                  sysfs_access_sniff_p->device_id,
                  sysfs_access_sniff_p->func_name,
                  sysfs_access_sniff_p->file_path,
                  sniff_struct_len,
                  sysfs_access_sniff_p->len,
                  sysfs_access_sniff_p->buf);

    ret = send_trap(sysfs_access_sniff_p,
                    sniff_struct_len,
                    SXD_TRAP_ID_SYSFS_SNIFFER,
                    0,
                    dev->device_id,
                    TARGET_PID_DONT_CARE,
                    GFP_KERNEL);
    if (ret) {
        sxd_log_err(dev, "send_trap failed for cb %s, ret [%d]\n", cb_name, ret);
        goto out;
    }

out:
    if (sysfs_access_sniff_p) {
        kfree(sysfs_access_sniff_p);
    }

    return ret;
}
EXPORT_SYMBOL(send_sysfs_common_sniffer_event);


int sx_core_send_sniffer_event(struct kobject   *kobj,
                               struct attribute *attr,
                               const char       *cb_name,
                               const char       *buf,
                               size_t            size)
{
    int            ret = 0;
    struct sx_dev *dev = NULL;

    if (!kobj) {
        sxd_log_err(SXD_DEV_NONE, "kobj is NULL\n");
        ret = -EINVAL;
        goto out;
    }

    ret = sx_sysfs_asic_get_dev_from_kobj(kobj, &dev, DRV_NAME);
    if (ret) {
        sxd_log_err(dev, "sysfs entry %s failed to find device. Invalid value\n", cb_name);
        ret = -EINVAL;
        goto out;
    }

    ret = send_sysfs_common_sniffer_event(kobj, attr, dev, cb_name, buf, size, SYSFS_SX_CORE);
    if (ret) {
        sxd_log_err(dev, "send_sniffer_event for %s failed.\n", cb_name);
        ret = -EINVAL;
        goto out;
    }

out:
    return ret;
}

struct kobject * sx_core_get_sysfs_parent(struct sx_dev *dev)
{
    struct sx_priv *priv = NULL;
    struct kobject *parent_kobj_p = NULL;

    /* For global operations(dev is NULL) or single - ASIC mode */
    if (!dev || (!sx_core_has_pci_multi_dev() && !sx_core_multi_asic_fs_mode())) {
        parent_kobj_p = &(THIS_MODULE->mkobj.kobj);
        goto out;
    }

    /* Multi-ASIC mode with specific device */
    priv = sx_priv(dev);

    if (!priv || !priv->kobj[SX_KOBJECT_ASIC]) {
        sxd_log_err(dev, "Invalid device or ASIC kobject not created\n");
        parent_kobj_p = &(THIS_MODULE->mkobj.kobj); /* Fallback to global */
        goto out;
    }

    parent_kobj_p = priv->kobj[SX_KOBJECT_ASIC];

out:
    return parent_kobj_p;
}
