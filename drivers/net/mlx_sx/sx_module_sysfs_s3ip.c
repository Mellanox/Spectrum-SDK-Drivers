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

#include <linux/module.h>
#include <linux/if_vlan.h>
#include <linux/poll.h>
#include <linux/mlx_sx/auto_registers/reg.h>
#include <linux/mlx_sx/cmd.h>
#include <linux/mlx_sx/skb_hook.h>
#include <linux/seq_file.h>
#include <linux/ktime.h>
#include <linux/mlx_sx/driver.h>
#include <linux/mlx_sx/kernel_user.h>
#include <linux/errno.h>

#include <linux/mlx_sx/auto_registers/cmd_auto.h>
#include <linux/mlx_sx/auto_registers/sx_core_reg_buffer_print_auto.h>
#include "sx_dbg_dump_proc.h"
#include "sx.h"
#include "alloc.h"
#include "dev_init.h"
#include "dev_db.h"

/************************************************
 *  Define
 ***********************************************/
#define PORT_IS_PRESENT(pmaos_oper_status)                             \
    ((pmaos_oper_status == SXD_PMAOS_OPER_STATUS_INITIALIZING_E) ||    \
     (pmaos_oper_status == SXD_PMAOS_OPER_STATUS_PLUGGED_ENABLED_E) || \
     (pmaos_oper_status == SXD_PMAOS_OPER_STATUS_PLUGGED_DISABLED_E))

#define PORT_IS_PRESENT_ERROR(pmaos_oper_status) \
    (pmaos_oper_status == SXD_PMAOS_OPER_STATUS_MODULE_PLUGGED_WITH_ERROR_E)


#define OP_NOT_SUPPORTED "Operation not supported"
#define GET_BIT_IN_MASK(mask, offset) (((mask) & (1 << (offset))) >> (offset))
#define MCION_STATUS_MASK_HW_PRESENT    (0x1)
#define MCION_STATUS_MASK_INTERRUPT     (0x80)
#define MCION_STATUS_MASK_POWER_GOOD    (0x200)
#define MCION_INPUT_BITS_MASK_LOW_POWER (0xD)
#define MCION_INPUT_BITS_MASK_HW_RESET  (0xE)
#define MCION_INPUT_BITS_MASK_POWER_ON  (0xB)

#define PMMP_ELS_OPERATION_CMD_NOP            0x0
#define PMMP_ELS_OPERATION_CMD_FIBER_CHECK    0x1
#define PMMP_ELS_OPERATION_CMD_FIBER_TUNING   0x2
#define PMMP_ELS_OPERATION_CMD_LASER_UP       0x3
#define PMMP_ELS_OPERATION_CMD_FORCE_LASER_UP 0x4

/************************************************
 *  Enum
 ***********************************************/
enum els_module_info_type {
    ELS_MODULE_INFO,
    ELS_MODULE_EXT_INFO,
    ELS_MODULE_LATCHED_FLAG,
};

/************************************************
 * Globals
 ***********************************************/


/************************************************
 *  Functions
 ***********************************************/
static ssize_t show_module_power_on(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t store_module_power_on(struct kobject        *kobj,
                                     struct kobj_attribute *attr,
                                     const char            *buf,
                                     size_t                 len);
static ssize_t show_module_tx_disable(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t store_module_tx_disable(struct kobject        *kobj,
                                       struct kobj_attribute *attr,
                                       const char            *buf,
                                       size_t                 len);
static ssize_t show_module_present(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_rx_los(struct kobject *kobj, struct kobj_attribute *attr, char *buf);

static ssize_t show_module_id(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t reset_module(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len);
static ssize_t store_module_power_mode_policy(struct kobject        *kobj,
                                              struct kobj_attribute *attr,
                                              const char            *buf,
                                              size_t                 len);
static ssize_t show_module_power_mode_policy(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_power_mode(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_temp_input(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_temp_label(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_temp_threshold_hi(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_temp_threshold_lo(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_temp_threshold_critical_hi(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_temp_threshold_critical_lo(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_status(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_error_type(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_pddr_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t show_module_ext_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t set_els_operation_cmd(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len);
static ssize_t show_module_latched_flag_pddr_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf);

/* Module kernel object attributes */
static struct kobj_attribute module_power_on_attr = __ATTR(power_on,
                                                           (S_IRUGO | S_IWUSR),
                                                           show_module_power_on,
                                                           store_module_power_on);
static struct kobj_attribute module_tx_disable_attr = __ATTR(tx_disable,
                                                             (S_IRUGO | S_IWUSR),
                                                             show_module_tx_disable,
                                                             store_module_tx_disable);
static struct kobj_attribute module_present_attr = __ATTR(present, S_IRUGO, show_module_present, NULL);
static struct kobj_attribute module_rx_los_attr = __ATTR(rx_los, S_IRUGO, show_module_rx_los, NULL);

static struct kobj_attribute module_id_attr = __ATTR(id, S_IRUGO, show_module_id, NULL);
static struct kobj_attribute module_reset_attr = __ATTR(reset, S_IWUSR, NULL, reset_module);
static struct kobj_attribute module_power_mode_policy_attr = __ATTR(power_mode_policy,
                                                                    (S_IRUGO | S_IWUSR),
                                                                    show_module_power_mode_policy,
                                                                    store_module_power_mode_policy);
static struct kobj_attribute module_info_attr = __ATTR(module_info, S_IRUGO, show_module_pddr_info, NULL);
static struct kobj_attribute module_ext_info_attr = __ATTR(module_ext_info, S_IRUGO, show_module_ext_info, NULL);
static struct kobj_attribute module_els_operation_cmd_attr = __ATTR(els_operation_cmd,
                                                                    S_IWUSR,
                                                                    NULL,
                                                                    set_els_operation_cmd);
static struct kobj_attribute module_latched_flag_attr = __ATTR(module_latched_flag_info,
                                                               S_IRUGO,
                                                               show_module_latched_flag_pddr_info,
                                                               NULL);
static struct kobj_attribute module_power_mode_attr = __ATTR(power_mode, S_IRUGO, show_module_power_mode, NULL);
static struct kobj_attribute module_temp_input_attr = __ATTR(input, S_IRUGO, show_module_temp_input, NULL);
static struct kobj_attribute module_temp_label_attr = __ATTR(label, S_IRUGO, show_module_temp_label, NULL);
static struct kobj_attribute module_temp_threshold_hi_attr = __ATTR(threshold_hi,
                                                                    S_IRUGO,
                                                                    show_module_temp_threshold_hi,
                                                                    NULL);
static struct kobj_attribute module_temp_threshold_lo_attr = __ATTR(threshold_lo,
                                                                    S_IRUGO,
                                                                    show_module_temp_threshold_lo,
                                                                    NULL);
static struct kobj_attribute module_temp_threshold_critical_hi_attr =
    __ATTR(threshold_critical_hi, S_IRUGO, show_module_temp_threshold_critical_hi, NULL);
static struct kobj_attribute module_temp_threshold_critical_lo_attr =
    __ATTR(threshold_critical_lo, S_IRUGO, show_module_temp_threshold_critical_lo, NULL);

static struct kobj_attribute module_status_attr = __ATTR(status, S_IRUGO, show_module_status, NULL);
static struct kobj_attribute module_error_type_attr = __ATTR(statuserror, S_IRUGO, show_module_error_type, NULL);

/* TEC Module*/
static ssize_t show_module_tec_cooling_level(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_cooling_level_attr = __ATTR(cooling_level,
                                                                    S_IRUGO,
                                                                    show_module_tec_cooling_level,
                                                                    NULL);

static ssize_t show_module_tec_max_power(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_max_power_attr = __ATTR(max_power,
                                                                S_IRUGO,
                                                                show_module_tec_max_power,
                                                                NULL);

static ssize_t show_module_tec_power(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_power_attr = __ATTR(power, S_IRUGO, show_module_tec_power, NULL);

static ssize_t show_module_tec_power_warning_high(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_power_warning_high_attr = __ATTR(power_threshold_warning_hi,
                                                                         S_IRUGO,
                                                                         show_module_tec_power_warning_high,
                                                                         NULL);

static ssize_t show_module_tec_power_warning_low(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_power_warning_low_attr = __ATTR(power_threshold_warning_lo,
                                                                        S_IRUGO,
                                                                        show_module_tec_power_warning_low,
                                                                        NULL);

static ssize_t show_module_tec_power_alarm_high(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_power_alarm_high_attr = __ATTR(power_threshold_alarm_hi,
                                                                       S_IRUGO,
                                                                       show_module_tec_power_alarm_high,
                                                                       NULL);

static ssize_t show_module_tec_power_alarm_low(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_power_alarm_low_attr = __ATTR(power_threshold_alarm_lo,
                                                                      S_IRUGO,
                                                                      show_module_tec_power_alarm_low,
                                                                      NULL);

static ssize_t show_module_tec_max_set_point_temperature(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_max_set_point_temperature_attr = __ATTR(max_laser_temperature,
                                                                                S_IRUGO,
                                                                                show_module_tec_max_set_point_temperature,
                                                                                NULL);

static ssize_t show_module_tec_set_point_temperature(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_set_point_temperature_attr = __ATTR(laser_temperature,
                                                                            S_IRUGO,
                                                                            show_module_tec_set_point_temperature,
                                                                            NULL);

static ssize_t show_module_tec_set_point_temperature_warning_high(struct kobject        *kobj,
                                                                  struct kobj_attribute *attr,
                                                                  char                  *buf);
static struct kobj_attribute module_tec_set_point_temperature_warning_high_attr = __ATTR(
    laser_temperature_threshold_warning_hi,
    S_IRUGO,
    show_module_tec_set_point_temperature_warning_high,
    NULL);

static ssize_t show_module_tec_set_point_temperature_warning_low(struct kobject        *kobj,
                                                                 struct kobj_attribute *attr,
                                                                 char                  *buf);
static struct kobj_attribute module_tec_set_point_temperature_warning_low_attr = __ATTR(
    laser_temperature_threshold_warning_lo,
    S_IRUGO,
    show_module_tec_set_point_temperature_warning_low,
    NULL);

static ssize_t show_module_tec_set_point_temperature_alarm_high(struct kobject        *kobj,
                                                                struct kobj_attribute *attr,
                                                                char                  *buf);
static struct kobj_attribute module_tec_set_point_temperature_alarm_high_attr = __ATTR(
    laser_temperature_threshold_alarm_hi,
    S_IRUGO,
    show_module_tec_set_point_temperature_alarm_high,
    NULL);

static ssize_t show_module_tec_set_point_temperature_alarm_low(struct kobject        *kobj,
                                                               struct kobj_attribute *attr,
                                                               char                  *buf);
static struct kobj_attribute module_tec_set_point_temperature_alarm_low_attr = __ATTR(
    laser_temperature_threshold_alarm_lo,
    S_IRUGO,
    show_module_tec_set_point_temperature_alarm_low,
    NULL);

static ssize_t show_module_tec_max_cooling_level(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_max_cooling_level_attr = __ATTR(max_cooling_level,
                                                                        S_IRUGO,
                                                                        show_module_tec_max_cooling_level,
                                                                        NULL);

static ssize_t show_module_tec_min_cooling_level(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_tec_min_cooling_level_attr = __ATTR(min_cooling_level,
                                                                        S_IRUGO,
                                                                        show_module_tec_min_cooling_level,
                                                                        NULL);

/* Independent module - begin */
static ssize_t show_module_control(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t store_module_control(struct kobject        *kobj,
                                    struct kobj_attribute *attr,
                                    const char            *buf,
                                    size_t                 len);
static struct kobj_attribute module_control_attr = __ATTR(control,
                                                          (S_IRUGO | S_IWUSR),
                                                          show_module_control,
                                                          store_module_control);

static ssize_t show_module_hw_reset(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t store_module_hw_reset(struct kobject        *kobj,
                                     struct kobj_attribute *attr,
                                     const char            *buf,
                                     size_t                 len);
static struct kobj_attribute module_hw_reset_attr = __ATTR(hw_reset,
                                                           (S_IRUGO | S_IWUSR),
                                                           show_module_hw_reset,
                                                           store_module_hw_reset);

static ssize_t show_module_low_power_mode(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t store_module_low_power_mode(struct kobject        *kobj,
                                           struct kobj_attribute *attr,
                                           const char            *buf,
                                           size_t                 len);
static struct kobj_attribute module_low_power_mode_attr = __ATTR(low_power_mode,
                                                                 (S_IRUGO | S_IWUSR),
                                                                 show_module_low_power_mode,
                                                                 store_module_low_power_mode);

static ssize_t show_module_hw_present(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_hw_present_attr = __ATTR(hw_present,
                                                             (S_IRUGO),
                                                             show_module_hw_present,
                                                             NULL);

static ssize_t show_module_interrupt(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_interrupt_attr = __ATTR(interrupt,
                                                            (S_IRUGO),
                                                            show_module_interrupt,
                                                            NULL);

static ssize_t show_module_power_good(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_power_good_attr = __ATTR(power_good,
                                                             (S_IRUGO),
                                                             show_module_power_good,
                                                             NULL);

static ssize_t store_module_reinsert(struct kobject        *kobj,
                                     struct kobj_attribute *attr,
                                     const char            *buf,
                                     size_t                 len);
static struct kobj_attribute module_reinsert_attr = __ATTR(reinsert,
                                                           S_IWUSR,
                                                           NULL,
                                                           store_module_reinsert);

static ssize_t show_module_power_limit(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_power_limit_attr = __ATTR(power_limit,
                                                              S_IRUGO,
                                                              show_module_power_limit,
                                                              NULL);


static ssize_t show_module_frequency(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t store_module_frequency(struct kobject        *kobj,
                                      struct kobj_attribute *attr,
                                      const char            *buf,
                                      size_t                 len);
static struct kobj_attribute module_frequency_attr = __ATTR(frequency,
                                                            S_IRUGO | S_IWUSR,
                                                            show_module_frequency,
                                                            store_module_frequency);

static ssize_t show_module_frequency_support(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute module_frequency_support_attr = __ATTR(frequency_support,
                                                                    S_IRUGO,
                                                                    show_module_frequency_support,
                                                                    NULL);

/* Independent module - end */

#define DISCARD_LSB_ZEROS(x)       \
    do {                           \
        if (x != 0) {              \
            while ((x & 1) == 0) { \
                x >>= 1;           \
            }                      \
        }                          \
    } while (0)


static int __sx_core_get_module_power_on(struct sx_dev *dev, uint8_t slot_id, uint8_t module_id, bool *power_on_p)
{
    int                        err = 0;
    struct ku_access_pmaos_reg pmaos_reg_data;
    struct ku_access_mcion_reg mcion_reg_data;
    bool                       is_independent = false;

    *power_on_p = false;

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        goto out;
    }

    if (is_independent) {
        memset(&mcion_reg_data, 0, sizeof(mcion_reg_data));
        mcion_reg_data.dev_id = dev->device_id;
        sx_cmd_set_op_tlv(&mcion_reg_data.op_tlv, MLXSW_MCION_ID, EMAD_METHOD_QUERY);

        mcion_reg_data.mcion_reg.slot_index = slot_id;
        mcion_reg_data.mcion_reg.module = module_id;

        err = sx_ACCESS_REG_MCION(dev, &mcion_reg_data);
        if (err) {
            sxd_log_err("Failed to access MCION, err=%d\n", err);
            goto out;
        }

        if (mcion_reg_data.op_tlv.status) {
            err = -EINVAL;
            sxd_log_err("Failed to access MCION, status=%d\n", mcion_reg_data.op_tlv.status);
            goto out;
        }

        *power_on_p = (mcion_reg_data.mcion_reg.module_inputs >> 2) & 0x1;
    } else {
        memset(&pmaos_reg_data, 0, sizeof(pmaos_reg_data));
        pmaos_reg_data.dev_id = dev->device_id;
        sx_cmd_set_op_tlv(&pmaos_reg_data.op_tlv, PMAOS_REG_ID, EMAD_METHOD_QUERY);

        pmaos_reg_data.pmaos_reg.slot_index = slot_id;
        pmaos_reg_data.pmaos_reg.module = module_id;

        err = sx_ACCESS_REG_PMAOS(dev, &pmaos_reg_data);
        if (err) {
            sxd_log_err("Failed to access PMAOS, err=%d\n", err);
            goto out;
        }

        if (pmaos_reg_data.op_tlv.status) {
            err = -EINVAL;
            sxd_log_err("Failed to access PMAOS, status=%d\n", pmaos_reg_data.op_tlv.status);
            goto out;
        }

        if (pmaos_reg_data.pmaos_reg.admin_status != SXD_PMAOS_ADMIN_STATUS_DISABLED_BY_CONFIGURATION_E) {
            *power_on_p = true;
        }
    }

out:
    return err;
}

static int __sx_core_get_module_status(struct sx_dev                  *dev,
                                       uint8_t                         slot_id,
                                       uint8_t                         module_id,
                                       struct sx_module_status_params *params)
{
    int                       err = 0;
    struct ku_access_pmpe_reg pmpe_reg_data;
    bool                      is_independent = false;

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        goto out;
    }
    if (is_independent) {
        params->status = SXD_PMPE_MODULE_STATUS_UNKNOWN_E;
        params->error_type = SXD_PMPE_ERROR_TYPE_MIN;
        goto out;
    }

    memset(&pmpe_reg_data, 0, sizeof(pmpe_reg_data));
    pmpe_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&pmpe_reg_data.op_tlv, MLXSW_PMPE_ID, EMAD_METHOD_QUERY);

    pmpe_reg_data.pmpe_reg.slot_index = slot_id;
    pmpe_reg_data.pmpe_reg.module = module_id;

    err = sx_ACCESS_REG_PMPE(dev, &pmpe_reg_data);
    if (err) {
        sxd_log_err("Failed to access PMPE, err=%d\n", err);
        goto out;
    }

    if (pmpe_reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to access PMPE, status=%d\n", pmpe_reg_data.op_tlv.status);
        goto out;
    }

    params->status = pmpe_reg_data.pmpe_reg.module_status;
    params->error_type = pmpe_reg_data.pmpe_reg.error_type;

out:
    return err;
}

int sx_core_get_port_module_power_on(struct sx_dev *dev, uint16_t local_port, bool          *power_on)
{
    int           err = 0;
    unsigned long flags;
    uint8_t       slot_id = 0;
    uint8_t       module_id = 0;

    spin_lock_irqsave(&sx_priv(dev)->db_lock, flags);
    err = sx_core_get_module_by_port(dev, local_port, &slot_id, &module_id);
    if (err) {
        spin_unlock_irqrestore(&sx_priv(dev)->db_lock, flags);
        goto out;
    }
    spin_unlock_irqrestore(&sx_priv(dev)->db_lock, flags);

    err = __sx_core_get_module_power_on(dev, slot_id, module_id, power_on);
out:
    return err;
}
EXPORT_SYMBOL(sx_core_get_port_module_power_on);

int sx_core_get_port_module_status(struct sx_dev *dev, uint16_t local_port, struct sx_module_status_params *params)
{
    int           err = 0;
    unsigned long flags;
    uint8_t       slot_id = 0;
    uint8_t       module_id = 0;

    spin_lock_irqsave(&sx_priv(dev)->db_lock, flags);
    err = sx_core_get_module_by_port(dev, local_port, &slot_id, &module_id);
    if (err) {
        spin_unlock_irqrestore(&sx_priv(dev)->db_lock, flags);
        goto out;
    }
    spin_unlock_irqrestore(&sx_priv(dev)->db_lock, flags);

    err = __sx_core_get_module_status(dev, slot_id, module_id, params);
out:
    return err;
}
EXPORT_SYMBOL(sx_core_get_port_module_status);

static int __sx_core_power_on_independent_module(struct sx_dev *dev, uint8_t slot_id, uint8_t module_id, bool power_on)
{
    int                        err = 0;
    struct ku_access_mcion_reg mcion_reg_data;

    memset(&mcion_reg_data, 0, sizeof(mcion_reg_data));
    mcion_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mcion_reg_data.op_tlv, MLXSW_MCION_ID, EMAD_METHOD_WRITE);

    mcion_reg_data.mcion_reg.slot_index = slot_id;
    mcion_reg_data.mcion_reg.module = module_id;
    mcion_reg_data.mcion_reg.module_inputs_mask = MCION_INPUT_BITS_MASK_POWER_ON;
    mcion_reg_data.mcion_reg.module_inputs = (power_on << 2);

    err = sx_ACCESS_REG_MCION(dev, &mcion_reg_data);
    if (err) {
        sxd_log_err("Failed to access MCION, err=%d\n", err);
        goto out;
    }

    if (mcion_reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to access MCION, status=%d\n", mcion_reg_data.op_tlv.status);
        goto out;
    }

out:
    return err;
}

static bool __module_any_port_up_get(struct sx_priv *priv, uint8_t slot_id, uint8_t module_id)
{
    int      i = 0;
    uint16_t local_port = 0;
    bool     any_port_up = false;

    for (i = 0; i < priv->modules_to_port_map_count[slot_id][module_id]; i++) {
        local_port = priv->module_to_ports_map[slot_id][module_id][i];
        if ((priv->local_port_state[local_port] == SXD_PAOS_ADMIN_STATUS_UP_E) ||
            (priv->local_port_state[local_port] == SXD_PAOS_ADMIN_STATUS_UP_ONCE_E)) {
            any_port_up = true;
            break;
        }
    }

    return any_port_up;
}

static int __sx_core_power_on_off_module(struct sx_dev *dev, uint8_t slot_id, uint8_t module_id, bool power_on)
{
    struct sx_priv *priv = sx_priv(dev);
    int             err = 0;
    unsigned long   flags;
    bool            any_port_up = false;
    bool            is_independent = false;

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        goto out;
    }

    spin_lock_irqsave(&priv->db_lock, flags);
    if (!power_on) {
        any_port_up = __module_any_port_up_get(priv, slot_id, module_id);
        if (any_port_up) {
            spin_unlock_irqrestore(&priv->db_lock, flags);
            err = -EACCES;
            goto out;
        }
    }
    spin_unlock_irqrestore(&priv->db_lock, flags);

    mutex_lock(&priv->module_access_mutex);
    /*No mutex should be added inside internal functions */
    if (is_independent) {
        err = __sx_core_power_on_independent_module(dev, slot_id, module_id, power_on);
    } else {
        err = sx_core_set_module(dev, slot_id, module_id, power_on, 0);
    }
    if (err) {
        mutex_unlock(&priv->module_access_mutex);
        goto out;
    }
    mutex_unlock(&priv->module_access_mutex);

out:
    return err;
}

int sx_core_power_on_off_port_module(struct sx_dev *dev, uint16_t local_port, bool power_on)
{
    struct sx_priv *priv = sx_priv(dev);
    int             err = 0;
    unsigned long   flags;
    uint8_t         slot_id = 0;
    uint8_t         module_id = 0;

    spin_lock_irqsave(&priv->db_lock, flags);
    err = sx_core_get_module_by_port(dev, local_port, &slot_id, &module_id);
    if (err) {
        spin_unlock_irqrestore(&priv->db_lock, flags);
        goto out;
    }
    spin_unlock_irqrestore(&priv->db_lock, flags);

    err = __sx_core_power_on_off_module(dev, slot_id, module_id, power_on);

out:
    return err;
}
EXPORT_SYMBOL(sx_core_power_on_off_port_module);


/*
 *   sx_core module sysfs nodes of non-eeprom are like:
 *       ./sx_core/$asic/$module_id/x_s3ip_x
 *       ./sx_core/$asic/$module_id/x_non-s3ip_x
 *       ./sx_core/$asic/$module_id/temperature/x
 *   For multi-asic system, return specific sx_dev and local module id.
 */
static int __module_sysfs_get_dev_slot_module(struct kobject *kobj,
                                              uint8_t         module_pos,
                                              struct sx_dev **dev,
                                              uint8_t        *slot,
                                              uint8_t        *module)
{
    int             ret = 0;
    int             module_id = 0;
    uint8_t         local_module_id = 0;
    int             mod_offset = 0;
    struct kobject *kobj_module = kobj;

    if (!kobj) {
        sxd_log_err("Invalid kobj %s\n", kobject_name(kobj));
        ret = -EINVAL;
        goto out;
    }

    if (!(*dev) && !slot && !module) {
        sxd_log_err("Wrong dev, slot, module for kobj %s\n", kobject_name(kobj));
        ret = -EINVAL;
        goto out;
    }

    mod_offset = module_pos;
    for ( ; mod_offset > 0; mod_offset--) {
        if (kobj_module) {
            kobj_module = kobj_module->parent;
        }
    }
    if (!kobj_module) {
        sxd_log_err("Invalid sysfs node entry because of no finding module kobject: kobj %s\n",
                    kobject_name(kobj));
        ret = -EINVAL;
        goto out;
    }

    ret = kstrtoint(kobj_module->name + strlen(MODULE_NODE_SYSFS_PREFIX), 10, &module_id);
    if (ret) {
        sxd_log_err("Invalid sysfs node entry because of wrong module kobject: kobj %s\n", kobject_name(kobj));
        ret = -EINVAL;
        goto out;
    }

    if (module_id > MAX_MODULE_NUM - 1) {
        sxd_log_err("Invalid module value %d for sysfs kobject %s\n", module_id, kobject_name(kobj));
        goto out;
    }

    ret = sx_core_asic_get_dev_global(module_id, dev);
    if (ret) {
        sxd_log_err("sysfs entry power_on got invalid value\n");
        goto out;
    }

    ret = sx_core_get_possible_local_module(module_id, &local_module_id);
    if (ret) {
        goto out;
    }

    if (module) {
        *module = local_module_id;
    }

    if (slot) {
        *slot = sx_priv(*dev)->module_to_slot_map[local_module_id];
    }

out:
    return ret;
}


static ssize_t show_module_power_on(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int            ret = 0;
    bool           power_on = false;
    int            len = 0;
    int            val = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;
    bool           hold = false;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("show_module_power_on: __module_sysfs_get_dev_slot_module failed (%d) for %s\n",
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    ret = __sx_core_get_module_power_on(dev, slot_id, module_id, &power_on);
    if (ret) {
        sxd_log_err("show_module_power_on: __sx_core_get_module_power_on failed (%d) for %s\n",
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    if (power_on) {
        val = 1;
    }

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Read power_on %d", module_id, power_on);
    sx_core_sysfs_dev_release(dev);

    len = sprintf(buf, "%d\n", val);

    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log, "Module ID %d | Read power_on failed status %d", module_id, ret);
        sx_core_sysfs_dev_release(dev);
    }

    return ret;
}

static ssize_t store_module_power_on(struct kobject        *kobj,
                                     struct kobj_attribute *attr,
                                     const char            *buf,
                                     size_t                 len)
{
    int            ret = 0;
    int            power_on = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;
    bool           hold = false;

    if (!kobj->parent) {
        sxd_log_err("Parent of power_on sysfs node is NULL\n");
        ret = -EINVAL;
        goto out_err;
    }

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, buf, len);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = kstrtoint(buf, 10, &power_on);
    if (ret) {
        sxd_log_err("sysfs entry power_on got invalid value\n");
        ret = -EINVAL;
        goto out_err;
    }

    if ((power_on != SX_MODULE_POWER_ON) &&
        (power_on != SX_MODULE_POWER_OFF)) {
        sxd_log_err("sysfs entry power_on got invalid value\n");
        ret = -EINVAL;
        goto out_err;
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("store_module_power_on: __module_sysfs_get_dev_slot_module failed (%d) for %s\n",
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    ret = __sx_core_power_on_off_module(dev, slot_id, module_id, (power_on == SX_MODULE_POWER_ON));
    if (ret) {
        sxd_log_err("store_module_power_on: __sx_core_power_on_off_module failed (%d) for %s\n",
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Write power_on %d", module_id, power_on);
    sx_core_sysfs_dev_release(dev);
    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log, "Module ID %d | Write power_on failed status %d", module_id, ret);
        sx_core_sysfs_dev_release(dev);
    }

    return ret;
}

int sx_core_get_port_module_independent(struct sx_dev *dev, uint16_t local_port, bool *is_independent_p)
{
    int           err = 0;
    uint8_t       slot_id = 0;
    uint8_t       module_id = 0;
    unsigned long flags;

    *is_independent_p = false;

    spin_lock_irqsave(&sx_priv(dev)->db_lock, flags);
    err = sx_core_get_module_by_port(dev, local_port, &slot_id, &module_id);
    if (err) {
        spin_unlock_irqrestore(&sx_priv(dev)->db_lock, flags);
        goto out;
    }
    spin_unlock_irqrestore(&sx_priv(dev)->db_lock, flags);

    err = sx_core_get_module_control(dev, slot_id, module_id, is_independent_p);
    if (err) {
        spin_unlock_irqrestore(&sx_priv(dev)->db_lock, flags);
        goto out;
    }

out:
    return err;
}
EXPORT_SYMBOL(sx_core_get_port_module_independent);

int sx_core_get_tec_support(struct sx_dev *dev, bool *tec_support_p)
{
    struct sx_priv *priv = sx_priv(dev);

    if (!dev || !tec_support_p) {
        return -EINVAL;
    }

    *tec_support_p = priv->tec_support;
    return 0;
}
EXPORT_SYMBOL(sx_core_get_tec_support);

static int __get_port_module_pmcr_data(struct sx_dev             *dev,
                                       uint16_t                   local_port,
                                       struct ku_access_pmcr_reg *pmcr_reg_data)
{
    int err = 0;

    memset(pmcr_reg_data, 0, sizeof(*pmcr_reg_data));
    pmcr_reg_data->dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&pmcr_reg_data->op_tlv, MLXSW_PMCR_ID, EMAD_METHOD_QUERY);

    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(pmcr_reg_data->pmcr_reg.local_port,
                                        pmcr_reg_data->pmcr_reg.lp_msb,
                                        local_port);

    err = sx_ACCESS_REG_PMCR(dev, pmcr_reg_data);
    if (err) {
        sxd_log_err("Failed to access PMCR, err=%d\n", err);
        err = -EIO;
        goto out;
    }

    if (pmcr_reg_data->op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to access PMCR, status=%d\n", pmcr_reg_data->op_tlv.status);
        goto out;
    }

    if (pmcr_reg_data->pmcr_reg.tx_disable_override_cap == 0) {
        sxd_log_notice("TX_DISABLE is not supported on the port(0x%x)'s module\n", local_port);
    }

out:
    return err;
}

int sx_core_get_port_module_tx_disable(struct sx_dev *dev, uint16_t local_port, bool *tx_disable)
{
    int                       err = 0;
    struct ku_access_pmcr_reg pmcr_reg_data;
    uint8_t                   slot_id = 0;
    uint8_t                   module_id = 0;
    unsigned long             flags;
    bool                      is_independent = false;

    spin_lock_irqsave(&sx_priv(dev)->db_lock, flags);
    err = sx_core_get_module_by_port(dev, local_port, &slot_id, &module_id);
    if (err) {
        spin_unlock_irqrestore(&sx_priv(dev)->db_lock, flags);
        goto out;
    }
    spin_unlock_irqrestore(&sx_priv(dev)->db_lock, flags);

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        goto out;
    }
    if (is_independent) {
        sxd_log_err("%s: Failed to read tx_disable: port module is independent (SW control).\n", __func__);
        err = -EFAULT;
        goto out;
    }

    err = __get_port_module_pmcr_data(dev, local_port, &pmcr_reg_data);
    if (err) {
        sxd_log_err("Failed to get the port(0x%x) module's pmcr data, err=%d\n", local_port, err);
        goto out;
    }

    *tx_disable = false;
    if ((pmcr_reg_data.pmcr_reg.tx_disable_override_cntl == 2) &&
        (pmcr_reg_data.pmcr_reg.tx_disable_override_value == 1)) {
        *tx_disable = true;
    }

out:
    return err;
}
EXPORT_SYMBOL(sx_core_get_port_module_tx_disable);

/* Notice: according to PRM PMCR, tx_disable will take effect after the port becomes admin_enable again. */
int sx_core_set_port_module_tx_disable(struct sx_dev *dev, uint16_t local_port, bool tx_disable)/*TBD: to "Not supported" */
{
    struct sx_priv           *priv = sx_priv(dev);
    int                       err = 0;
    struct ku_access_pmcr_reg pmcr_reg_data;
    bool                      release_lock = false;
    uint8_t                   slot_id = 0;
    uint8_t                   module_id = 0;
    unsigned long             flags;
    bool                      is_independent = false;

    spin_lock_irqsave(&priv->db_lock, flags);
    err = sx_core_get_module_by_port(dev, local_port, &slot_id, &module_id);
    if (err) {
        spin_unlock_irqrestore(&priv->db_lock, flags);
        goto out;
    }
    spin_unlock_irqrestore(&priv->db_lock, flags);

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        spin_unlock_irqrestore(&priv->db_lock, flags);
        goto out;
    }

    if (is_independent) {
        sxd_log_err("%s: Failed to write tx_disable: port module is independent (SW control).\n", __func__);
        err = -EFAULT;
        goto out;
    }

    err = __get_port_module_pmcr_data(dev, local_port, &pmcr_reg_data);
    if (err) {
        sxd_log_err("Failed to get the port(0x%x) module's pmcr data, err=%d\n", local_port, err);
        goto out;
    }

    pmcr_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&pmcr_reg_data.op_tlv, MLXSW_PMCR_ID, EMAD_METHOD_WRITE);

    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(pmcr_reg_data.pmcr_reg.local_port,
                                        pmcr_reg_data.pmcr_reg.lp_msb,
                                        local_port);
    pmcr_reg_data.pmcr_reg.tx_disable_override_cntl = 0;
    if (tx_disable) {
        pmcr_reg_data.pmcr_reg.tx_disable_override_cntl = 2;
        pmcr_reg_data.pmcr_reg.tx_disable_override_value = 1;
    }

    mutex_lock(&priv->module_access_mutex);
    release_lock = true;

    err = sx_ACCESS_REG_PMCR(dev, &pmcr_reg_data);
    if (err) {
        sxd_log_err("Failed to write PMCR, err=%d\n", err);
        goto out;
    }

    if (pmcr_reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to write PMCR, status=%d\n", pmcr_reg_data.op_tlv.status);
        goto out;
    }

out:
    if (release_lock) {
        mutex_unlock(&priv->module_access_mutex);
    }
    return err;
}
EXPORT_SYMBOL(sx_core_set_port_module_tx_disable);

/* sysfs entry "tx_disable" under sx_core will be further determined by arch team */
static ssize_t show_module_tx_disable(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    SX_CORE_UNUSED_PARAM(attr);
    SX_CORE_UNUSED_PARAM(buf);

    sprintf(buf, "%s", OP_NOT_SUPPORTED);
    sxd_log_notice("%s: %s for %s.\n", __func__, OP_NOT_SUPPORTED, kobject_name(kobj));

    return -EOPNOTSUPP;
}

static ssize_t store_module_tx_disable(struct kobject        *kobj,
                                       struct kobj_attribute *attr,
                                       const char            *buf,
                                       size_t                 len)
{
    SX_CORE_UNUSED_PARAM(attr);
    SX_CORE_UNUSED_PARAM(buf);
    SX_CORE_UNUSED_PARAM(len);

    sxd_log_notice("%s: %s for %s.\n", __func__, OP_NOT_SUPPORTED, kobject_name(kobj));
    return -EOPNOTSUPP;
}

static int __sx_core_get_module_present(struct sx_dev                     *dev,
                                        uint8_t                            slot_id,
                                        uint8_t                            module_id,
                                        enum sx_oper_module_present_state *present)
{
    int                        err = 0;
    struct ku_access_pmaos_reg pmaos_reg_data;
    bool                       is_independent = false;
    struct sx_priv            *priv = sx_priv(dev);

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        goto out;
    }
    if (is_independent) {
        *present = SX_OPER_MODULE_NOT_PRESENT;
        goto out;
    }

    memset(&pmaos_reg_data, 0, sizeof(pmaos_reg_data));
    pmaos_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&pmaos_reg_data.op_tlv, PMAOS_REG_ID, EMAD_METHOD_QUERY);

    pmaos_reg_data.pmaos_reg.slot_index = slot_id;
    pmaos_reg_data.pmaos_reg.module = module_id;

    err = sx_ACCESS_REG_PMAOS(dev, &pmaos_reg_data);
    if (err) {
        sxd_log_err("Failed to access PMAOS, err=%d\n", err);
        goto out;
    }

    if (pmaos_reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to access PMAOS, status=%d\n", pmaos_reg_data.op_tlv.status);
        goto out;
    }

    *present = SX_OPER_MODULE_NOT_PRESENT;
    if (PORT_IS_PRESENT(pmaos_reg_data.pmaos_reg.oper_status)) {
        *present = SX_OPER_MODULE_PRESENT;
    } else if (PORT_IS_PRESENT_ERROR(pmaos_reg_data.pmaos_reg.oper_status)) {
        *present = SX_OPER_MODULE_PRESENT_ERROR;
    }

    /*
     * To suppress false positive first event on present sysfs, if user query
     * present - sync internal DB with the initial queried info.
     */
    if (priv->module_data[slot_id][module_id].present == SX_MODULE_PRESENT_INVALID) {
        if (PORT_IS_PRESENT(pmaos_reg_data.pmaos_reg.oper_status)) {
            priv->module_data[slot_id][module_id].present = SX_MODULE_PRESENT_PLUGGED;
        } else if (PORT_IS_PRESENT_ERROR(pmaos_reg_data.pmaos_reg.oper_status)) {
            priv->module_data[slot_id][module_id].present = SX_MODULE_PRESENT_PLUGGED_ERROR;
        } else {
            priv->module_data[slot_id][module_id].present = SX_MODULE_PRESENT_UNPLUGGED;
        }
    }

out:
    return err;
}

int sx_core_get_port_module_present(struct sx_dev *dev, uint16_t local_port,
                                    enum sx_oper_module_present_state *present)
{
    int             err = 0;
    unsigned long   flags;
    struct sx_priv *priv = sx_priv(dev);
    uint8_t         slot_id = 0;
    uint8_t         module_id = 0;

    spin_lock_irqsave(&priv->db_lock, flags);
    err = sx_core_get_module_by_port(dev, local_port, &slot_id, &module_id);
    if (err) {
        spin_unlock_irqrestore(&priv->db_lock, flags);
        goto out;
    }
    spin_unlock_irqrestore(&priv->db_lock, flags);

    err = __sx_core_get_module_present(dev, slot_id, module_id, present);
out:
    return err;
}
EXPORT_SYMBOL(sx_core_get_port_module_present);

int sx_core_get_port_module_present_db(struct sx_dev *dev, uint16_t local_port,
                                       enum sx_module_present_state *present_p)
{
    int             err = 0;
    unsigned long   flags;
    struct sx_priv *priv = sx_priv(dev);
    uint8_t         slot_id = 0;
    uint8_t         module_id = 0;

    spin_lock_irqsave(&priv->db_lock, flags);
    err = sx_core_get_module_by_port(dev, local_port, &slot_id, &module_id);
    if (err) {
        spin_unlock_irqrestore(&priv->db_lock, flags);
        *present_p = SX_MODULE_PRESENT_INVALID;
        goto out;
    }
    *present_p = priv->module_data[slot_id][module_id].present;
    spin_unlock_irqrestore(&priv->db_lock, flags);
out:
    return err;
}
EXPORT_SYMBOL(sx_core_get_port_module_present_db);

static ssize_t show_module_present(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                               ret = 0;
    enum sx_oper_module_present_state present = SX_OPER_MODULE_NOT_PRESENT;
    int                               len = 0;
    int                               val = 0;
    struct sx_dev                    *dev = NULL;
    uint8_t                           slot_id = 0;
    uint8_t                           module_id = 0;
    bool                              hold = false;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("show_module_power_on: __module_sysfs_get_dev_slot_module failed (%d) for %s\n",
                    ret,
                    kobject_name(kobj));
        if (!dev) {
            return ret;
        }
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    ret = __sx_core_get_module_present(dev, slot_id, module_id, &present);
    if (ret) {
        sxd_log_err("Failed in sx_netdev_module_get_present\n");
        goto out_err;
    }

    if (present == SX_OPER_MODULE_PRESENT) {
        val = 1;
    } else if (present == SX_OPER_MODULE_PRESENT_ERROR) {
        val = 2;
    }

    len = sprintf(buf, "%d\n", val);

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Read present %d", module_id, present);
    sx_core_sysfs_dev_release(dev);

    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log, "Module ID %d | Read present failed status %d", module_id, ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}

int sx_core_get_port_module_rx_los(struct sx_dev *dev, uint16_t local_port, bool *rx_los)
{
    struct ku_access_pddr_reg reg_data;
    int                       err = 0;
    uint8_t                   status_opcode = 0;
    uint8_t                   slot_id = 0;
    uint8_t                   module_id = 0;
    unsigned long             flags;
    bool                      is_independent = false;

    spin_lock_irqsave(&sx_priv(dev)->db_lock, flags);
    err = sx_core_get_module_by_port(dev, local_port, &slot_id, &module_id);
    if (err) {
        spin_unlock_irqrestore(&sx_priv(dev)->db_lock, flags);
        goto out;
    }
    spin_unlock_irqrestore(&sx_priv(dev)->db_lock, flags);

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        goto out;
    }
    if (is_independent) {
        sxd_log_err("%s: Failed to read rx_los: port module is independent (SW control).\n", __func__);
        err = -EFAULT;
        goto out;
    }

    memset(&reg_data, 0, sizeof(reg_data));

    reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&reg_data.op_tlv, PDDR_REG_ID, EMAD_METHOD_QUERY);

    reg_data.pddr_reg.page_select = SXD_PDDR_PAGE_SELECT_MODULE_LATCHED_FLAG_INFO_PAGE_E;
    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data.pddr_reg.local_port,
                                        reg_data.pddr_reg.lp_msb,
                                        local_port);
    reg_data.pddr_reg.pnat = SXD_PDDR_PNAT_LOCAL_PORT_NUMBER_E;

    err = sx_ACCESS_REG_PDDR(dev, &reg_data);
    if (err) {
        sxd_log_err("Failed to call sx_ACCESS_REG_PDDR (err:%d)\n", err);
        goto out;
    }

    if (reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to access PDDR, status=%d\n", reg_data.op_tlv.status);
        goto out;
    }

    status_opcode =
        reg_data.pddr_reg.page_data.module_latched_flag_info.rx_los;
    sxd_log_debug("%s (latched_flag, status_opcode:0x%x)\n", __func__, status_opcode);

    *rx_los = false;
    if (status_opcode) {
        *rx_los = true;
    }

out:
    return err;
}
EXPORT_SYMBOL(sx_core_get_port_module_rx_los);

/* sysfs entry "rx_los" under sx_core will be further determined by arch team */
static ssize_t show_module_rx_los(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    sprintf(buf, "%s", OP_NOT_SUPPORTED);
    sxd_log_notice("%s: %s for %s.\n", __func__, OP_NOT_SUPPORTED, kobject_name(kobj));
    return -EOPNOTSUPP;
}

static ssize_t show_module_id(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    int module_id = 0;
    int len = 0;

    if (!kobj->parent) {
        sxd_log_err("Parent of id sysfs node is NULL\n");
        ret = -EINVAL;
        goto out;
    }

    ret = kstrtoint(kobj->name + strlen(MODULE_NODE_SYSFS_PREFIX), 10, &module_id);
    if (ret) {
        sxd_log_err("Invalid sysfs node entry because of wrong module kobject: kobj %s\n", kobject_name(kobj));
        ret = -EINVAL;
        goto out;
    }

    len = sprintf(buf, "%u\n", module_id);
    return len;

out:
    return ret;
}

static ssize_t reset_module(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len)
{
    int            ret = 0;
    int            reset = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;
    bool           hold = false;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, buf, len);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = kstrtoint(buf, 10, &reset);
    if (ret) {
        sxd_log_info("sysfs entry reset got invalid value\n");
        goto out_err;
    }

    if (!reset) {
        ret = -EINVAL;
        goto out_err;
    }

    if (!kobj->parent) {
        sxd_log_err("Parent of reset sysfs node is NULL\n");
        ret = -EINVAL;
        goto out_err;
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n", __func__, ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    ret = sx_core_reset_module(dev, slot_id, module_id);
    if (ret) {
        sxd_log_err("Failed in sx_core_reset_module\n");
        goto out_err;
    }

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Reset module", module_id);
    sx_core_sysfs_dev_release(dev);
    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log, "Module ID %d | Reset module failed status %d", module_id, ret);
        sx_core_sysfs_dev_release(dev);
    }

    return ret;
}

static ssize_t store_module_power_mode_policy(struct kobject        *kobj,
                                              struct kobj_attribute *attr,
                                              const char            *buf,
                                              size_t                 len)
{
    int            ret = 0;
    int            power_mode_policy = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;
    bool           hold = false;

    if (!kobj->parent) {
        sxd_log_err("Parent of power_mode_policy sysfs node is NULL\n");
        ret = -EINVAL;
        goto out_err;
    }

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, buf, len);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = kstrtoint(buf, 10, &power_mode_policy);
    if (ret) {
        sxd_log_err("sysfs entry power_mode_policy got invalid value\n");
        ret = -EINVAL;
        goto out_err;
    }

    if ((power_mode_policy != SX_MODULE_POWER_MODE_POLICY_AUTO) &&
        (power_mode_policy != SX_MODULE_POWER_MODE_POLICY_LOW) &&
        (power_mode_policy != SX_MODULE_POWER_MODE_POLICY_HIGH)) {
        sxd_log_err("sysfs entry power_mode_policy got invalid value\n");
        ret = -EINVAL;
        goto out_err;
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n", __func__, ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    ret = sx_core_set_module_power_mode_policy(dev,
                                               slot_id,
                                               module_id,
                                               (enum sx_module_power_mode_policy)power_mode_policy);
    if (ret) {
        sxd_log_err("%s: sx_core_set_module_power_mode_policy failed (%d) for %s\n", __func__, ret,
                    kobject_name(kobj));
        goto out_err;
    }

    sx_int_log_info(&sx_priv(dev)->module_log,
                    "Module ID %d | Write power mode policy %d",
                    module_id,
                    power_mode_policy);
    sx_core_sysfs_dev_release(dev);
    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log,
                         "Module ID %d | Write power mode policy failed status %d",
                         module_id,
                         ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}

static ssize_t show_module_power_mode_policy(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                                ret = 0;
    struct sx_module_power_mode_params params;
    struct sx_dev                     *dev = NULL;
    uint8_t                            slot_id = 0;
    uint8_t                            module_id = 0;
    int                                len = 0;
    bool                               hold = false;

    if (!kobj->parent) {
        sxd_log_err("Parent of power_mode_policy sysfs node is NULL\n");
        ret = -EINVAL;
        goto out_err;
    }

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n", __func__, ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    memset(&params, 0, sizeof(params));

    ret = sx_core_get_module_power_mode(dev, slot_id, module_id, &params);
    if (ret) {
        sxd_log_err("Failed to get power mode policy.\n");
        goto out_err;
    }

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Read power mode policy %d", module_id, params.policy);
    sx_core_sysfs_dev_release(dev);

    len = sprintf(buf, "%u\n", params.policy);
    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log,
                         "Module ID %d | Read power mode policy failed status %d",
                         module_id,
                         ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}

static ssize_t show_module_power_mode(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                                ret = 0;
    struct sx_module_power_mode_params params;
    struct sx_dev                     *dev = NULL;
    uint8_t                            slot_id = 0;
    uint8_t                            module_id = 0;
    int                                len = 0;
    bool                               hold = false;

    if (!kobj->parent) {
        sxd_log_err("Parent of reset sysfs node is NULL\n");
        ret = -EINVAL;
        goto out_err;
    }

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n", __func__, ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    memset(&params, 0, sizeof(params));

    ret = sx_core_get_module_power_mode(dev, slot_id, module_id, &params);
    if (ret) {
        sxd_log_err("Failed to get power mode.\n");
        goto out_err;
    }

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Read power mode %d", module_id, params.mode);
    sx_core_sysfs_dev_release(dev);
    len = sprintf(buf, "%u\n", params.mode);
    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log, "Module ID %d | Read power mode failed status %d", module_id, ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}

static ssize_t _get_pddr(struct sx_dev         *dev,
                         u16                    local_port,
                         char                  *buf,
                         sxd_pddr_page_select_t page,
                         uint8_t                module_ind_type,
                         ssize_t                consumed_buf_space)
{
    struct ku_access_pddr_reg reg_data;
    int                       err = 0;

    memset(&reg_data, 0, sizeof(reg_data));

    reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&reg_data.op_tlv, PDDR_REG_ID, EMAD_METHOD_QUERY);

    reg_data.pddr_reg.page_select = page;
    if (page == SXD_PDDR_PAGE_SELECT_TROUBLESHOOTING_INFO_PAGE_E) {
        reg_data.pddr_reg.page_data.pddr_troubleshooting_page.group_opcode =
            SXD_PDDR_GROUP_OPCODE_MONITOR_OPCODES_E;
    }
    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data.pddr_reg.local_port,
                                        reg_data.pddr_reg.lp_msb,
                                        local_port);

    reg_data.pddr_reg.pnat = SXD_PDDR_PNAT_LOCAL_PORT_NUMBER_E;
    reg_data.pddr_reg.module_info_ext = SXD_PDDR_MODULE_INFO_EXT_UW_E;
    if (sx_core_has_pci_multi_dev()) {
        reg_data.pddr_reg.module_ind_type = module_ind_type;
    }

    err = sx_ACCESS_REG_PDDR(dev, &reg_data);
    if (err) {
        sxd_log_err("Failed to call sx_ACCESS_REG_PDDR (err:%d)\n", err);
        goto out;
    }

    if (reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to access PDDR, status=%d\n", reg_data.op_tlv.status);
        goto out;
    }

    return sx_kernel_access_reg_pddr_data_to_buffer_print(buf + consumed_buf_space,
                                                          (PAGE_SIZE - 1 - consumed_buf_space),
                                                          &reg_data.pddr_reg);

out:
    return err;
}


static int __access_reg_pemi_data_to_buffer_print(char buffer[], size_t buffer_length, struct ku_pemi_reg * reg_data)
{
    int      buffer_size = 0, snp_res = 0;
    uint16_t local_port = 0;

    if (reg_data->module_index_cap == SXD_PEMI_MODULE_INDEX_CAP_MODULE_INDEX_E) {
        snp_res = snprintf(buffer, buffer_length - buffer_size, "module_id: 0x%x\n", reg_data->local_port);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
    } else {
        SX_PORT_BUILD_PHY_ID_FROM_LSB_MSB(local_port, reg_data->local_port, reg_data->lp_msb);
        snp_res = snprintf(buffer, buffer_length - buffer_size, "local_port: 0x%x\n", local_port);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
    }

    snp_res = snprintf(buffer, buffer_length - buffer_size, "status: 0x%x\n", reg_data->status);
    VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
    buffer += snp_res;
    buffer_size += snp_res;

    switch (reg_data->page_select) {
    case SXD_PEMI_PAGE_SELECT_MODULE_STATUS_SAMPLES_E:
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "temperature: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.temperature);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "voltage: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.voltage);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "rx_power_lane0: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.rx_power_lane0);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "rx_power_lane1: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.rx_power_lane1);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "rx_power_lane2: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.rx_power_lane2);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "rx_power_lane3: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.rx_power_lane3);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "rx_power_lane4: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.rx_power_lane4);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "rx_power_lane5: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.rx_power_lane5);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "rx_power_lane6: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.rx_power_lane6);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "rx_power_lane7: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.rx_power_lane7);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "tx_power_lane0: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.tx_power_lane0);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "tx_power_lane1: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.tx_power_lane1);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "tx_power_lane2: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.tx_power_lane2);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "tx_power_lane3: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.tx_power_lane3);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "tx_power_lane4: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.tx_power_lane4);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "tx_power_lane5: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.tx_power_lane5);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "tx_power_lane6: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.tx_power_lane6);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "tx_power_lane7: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.tx_power_lane7);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "tx_bias_lane0: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.tx_bias_lane0);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "tx_bias_lane1: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.tx_bias_lane1);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "tx_bias_lane2: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.tx_bias_lane2);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "tx_bias_lane3: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.tx_bias_lane3);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "tx_bias_lane4: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.tx_bias_lane4);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "tx_bias_lane5: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.tx_bias_lane5);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "tx_bias_lane6: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.tx_bias_lane6);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "tx_bias_lane7: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.tx_bias_lane7);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "module_st: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.module_st);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "dp_st_lane0: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.dp_st_lane0);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "dp_st_lane1: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.dp_st_lane1);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "dp_st_lane2: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.dp_st_lane2);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "dp_st_lane3: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.dp_st_lane3);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "dp_st_lane4: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.dp_st_lane4);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "dp_st_lane5: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.dp_st_lane5);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "dp_st_lane6: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.dp_st_lane6);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "dp_st_lane7: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.dp_st_lane7);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "max_tec_power: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.max_tec_power);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "laser_source_temperature: 0x%x\n",
                           reg_data->page_data.pemi_module_status_samples.laser_source_temperature);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        break;

    case SXD_PEMI_PAGE_SELECT_LASER_SOURCE_MODULE_ESSENTIAL_SAMPLES_E:
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "voltage_monitor1: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.voltage_monitor1);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "voltage_monitor2: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.voltage_monitor2);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "voltage_monitor3: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.voltage_monitor3);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "voltage_monitor4: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.voltage_monitor4);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "voltage_monitor5: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.voltage_monitor5);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "voltage_monitor6: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.voltage_monitor6);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "voltage_monitor7: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.voltage_monitor7);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "voltage_monitor8: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.voltage_monitor8);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "bias_current_monitor1: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.bias_current_monitor1);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "bias_current_monitor2: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.bias_current_monitor2);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "bias_current_monitor3: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.bias_current_monitor3);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "bias_current_monitor4: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.bias_current_monitor4);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "bias_current_monitor5: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.bias_current_monitor5);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "bias_current_monitor6: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.bias_current_monitor6);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "bias_current_monitor7: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.bias_current_monitor7);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "bias_current_monitor8: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.bias_current_monitor8);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "opt_power_monitor1: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.opt_power_monitor1);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "opt_power_monitor2: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.opt_power_monitor2);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "opt_power_monitor3: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.opt_power_monitor3);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "opt_power_monitor4: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.opt_power_monitor4);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "opt_power_monitor5: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.opt_power_monitor5);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "opt_power_monitor6: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.opt_power_monitor6);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "opt_power_monitor7: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.opt_power_monitor7);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "opt_power_monitor8: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.opt_power_monitor8);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "icc_monitor: 0x%x\n",
                           reg_data->page_data.pemi_laser_source_module_essential_samples.icc_monitor);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        break;

    default:
        /* Other pages not supported yet */
        break;
    }

    return buffer_size;
}


static ssize_t __get_els_module_pemi(struct sx_dev         *dev,
                                     u16                    module_id,
                                     char                  *buf,
                                     sxd_pemi_page_select_t page,
                                     ssize_t                already_in_buf)
{
    struct ku_access_pemi_reg reg_data;
    int                       err = 0;

    memset(&reg_data, 0, sizeof(reg_data));

    reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&reg_data.op_tlv, PEMI_REG_ID, EMAD_METHOD_QUERY);

    reg_data.pemi_reg.module_index_cap = SXD_PEMI_MODULE_INDEX_CAP_MODULE_INDEX_E;
    reg_data.pemi_reg.module_ind_type = SXD_PDDR_MODULE_IND_TYPE_ELS_E;
    reg_data.pemi_reg.local_port = module_id;
    reg_data.pemi_reg.page_select = page;

    err = sx_ACCESS_REG_PEMI(dev, &reg_data);
    if (err) {
        sxd_log_err("Failed to call sx_ACCESS_REG_PEMI (err:%d) status is %u \n", err, reg_data.pemi_reg.status);
        goto out;
    }

    if (reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to access PEMI, status=%d\n", reg_data.op_tlv.status);
        goto out;
    }

    return __access_reg_pemi_data_to_buffer_print(buf + already_in_buf,
                                                  (PAGE_SIZE - 1 - already_in_buf),
                                                  &reg_data.pemi_reg);

out:
    return err;
}

static ssize_t __get_module_info_els(enum els_module_info_type type, char *buf, struct sx_dev *dev, uint8_t module_id)
{
    /* This function is for ELS modules (Taipan)
     * For ELS modules:
     * - module_ext_info sysfs should return PDDR Module Info page.
     * - module_info sysfs should return PEMI Module Status Samples Page and Laser Source Module Essential Samples Pages.
     * - module_latched_flag sysfs should return PDDR Module Latched Flag Page. */

    uint8_t                local_port = 0;
    sxd_pddr_page_select_t page_select;
    int                    ret = 0;

    if (dev == NULL) {
        sxd_log_err("%s: dev is NULL\n", __func__);
        return -EINVAL;
        goto out;
    }

    if ((type == ELS_MODULE_LATCHED_FLAG) || (type == ELS_MODULE_INFO)) {
        sx_module_get_local_port_for_els(module_id, &local_port);
        if (local_port == 0) {
            sxd_log_err("%s: No local port for ELS module %u\n", __func__, module_id);
            ret = -EINVAL;
            goto out;
        }
        if (type == ELS_MODULE_LATCHED_FLAG) {
            page_select = SXD_PDDR_PAGE_SELECT_MODULE_LATCHED_FLAG_INFO_PAGE_E;
        } else {
            page_select = SXD_PDDR_PAGE_SELECT_MODULE_INFO_PAGE_E;
        }

        ret = sx_core_sysfs_dev_hold(dev);
        if (ret == 0) {
            ret =
                _get_pddr(dev,
                          local_port,
                          buf,
                          page_select,
                          SXD_PDDR_MODULE_IND_TYPE_ELS_E, 0);
            sx_core_sysfs_dev_release(dev);
        }
    } else if (type == ELS_MODULE_EXT_INFO) {
        ret = sx_core_sysfs_dev_hold(dev);
        if (ret == 0) {
            ret += __get_els_module_pemi(dev, module_id, buf, SXD_PEMI_PAGE_SELECT_MODULE_STATUS_SAMPLES_E, 0);
            ret += __get_els_module_pemi(dev,
                                         module_id,
                                         buf,
                                         SXD_PEMI_PAGE_SELECT_LASER_SOURCE_MODULE_ESSENTIAL_SAMPLES_E,
                                         ret);
            sx_core_sysfs_dev_release(dev);
        }
    } else {
        sxd_log_err("%s: Invalid type %d\n", __func__, type);
        ret = -EINVAL;
    }

out:
    return ret;
}

static ssize_t show_module_latched_flag_pddr_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int             ret = 0;
    struct sx_dev  *dev = NULL;
    uint8_t         slot_id = 0;
    uint8_t         module_id = 0;
    uint16_t        local_ports[MAX_LANE_NUM_PER_MODULE];
    uint16_t        sysports[MAX_LANE_NUM_PER_MODULE];
    uint8_t         ports_cnt = 0;
    unsigned long   flags;
    struct sx_priv *priv = NULL;
    bool            locked = false;
    uint8_t         module_ind_type = 0;
    int             mapped_port_idx = 0;

    if (!kobj->parent) {
        sxd_log_err("Parent of module info sysfs node is NULL\n");
        ret = -EINVAL;
        goto out;
    }

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n", __func__, ret,
                    kobject_name(kobj));
        goto out;
    }

    if (sx_core_has_pci_multi_dev()) {
        ret = sx_multi_asic_module_ind_type(module_id, &module_ind_type);
        if (ret) {
            sxd_log_err("%s: sx_multi_asic_module_ind_type failed (%d) for %s\n",
                        __func__,
                        ret,
                        kobject_name(kobj));
            goto out;
        }

        if (module_ind_type == SXD_PDDR_MODULE_IND_TYPE_ELS_E) {
            ret = __get_module_info_els(ELS_MODULE_LATCHED_FLAG, buf, dev, module_id);
            goto out;
        }
    }

    priv = sx_priv(dev);

    spin_lock_irqsave(&priv->db_lock, flags);
    locked = true;

    ret = sx_core_get_ports_by_module(dev, slot_id, module_id, local_ports, sysports, &ports_cnt);
    if (ret) {
        sxd_log_err("%s: sx_core_get_ports_by_module failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out;
    }

    if (ports_cnt == 0) {
        sprintf(buf, "%s module [%d] not mapped to a local port ", OP_NOT_SUPPORTED, module_id);
        sxd_log_notice("%s: %s for %s: not mapped to a local port \n", __func__, OP_NOT_SUPPORTED, kobject_name(kobj));
        ret = -EOPNOTSUPP;
        goto out;
    }

    for (mapped_port_idx = 0; mapped_port_idx <= ports_cnt; mapped_port_idx++) {
        if (mapped_port_idx == ports_cnt) { /* There are no mapped ports associated with the module */
            sprintf(buf, "%s local_port [0x%x] is not mapped to swid ", OP_NOT_SUPPORTED, local_ports[0]);
            sxd_log_notice("%s: %s for %s: not mapped to swid \n", __func__, OP_NOT_SUPPORTED, kobject_name(kobj));
            ret = -EOPNOTSUPP;
            goto out;
        }
        if (priv->local_to_swid_db[local_ports[mapped_port_idx]] == SWID_NUM_DONT_CARE) {
            /* Port is not mapped to a swid - continue to the next one */
            continue;
        } else {
            /* We'll use priv->local_to_swid_db[local_ports[mapped_port_idx]] since it's mapped to a swid */
            break;
        }
    }

    spin_unlock_irqrestore(&priv->db_lock, flags);
    locked = false;

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret == 0) {
        ret =
            _get_pddr(dev,
                      local_ports[mapped_port_idx],
                      buf,
                      SXD_PDDR_PAGE_SELECT_MODULE_LATCHED_FLAG_INFO_PAGE_E,
                      module_ind_type, 0);
        sx_core_sysfs_dev_release(dev);
    }

out:
    if (locked) {
        spin_unlock_irqrestore(&priv->db_lock, flags);
    }
    return ret;
}

static ssize_t show_module_pddr_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int             ret = 0;
    struct sx_dev  *dev = NULL;
    uint8_t         slot_id = 0;
    uint8_t         module_id = 0;
    uint16_t        local_ports[MAX_LANE_NUM_PER_MODULE];
    uint16_t        sysports[MAX_LANE_NUM_PER_MODULE];
    uint8_t         ports_cnt = 0;
    unsigned long   flags;
    struct sx_priv *priv = NULL;
    bool            locked = false;
    uint8_t         module_ind_type = 0;
    int             mapped_port_idx = 0;

    if (!kobj->parent) {
        sxd_log_err("Parent of module info sysfs node is NULL\n");
        ret = -EINVAL;
        goto out;
    }

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n", __func__, ret,
                    kobject_name(kobj));
        goto out;
    }

    if (sx_core_has_pci_multi_dev()) {
        ret = sx_multi_asic_module_ind_type(module_id, &module_ind_type);
        if (ret) {
            sxd_log_err("%s: sx_multi_asic_module_ind_type failed (%d) for %s\n",
                        __func__,
                        ret,
                        kobject_name(kobj));
            goto out;
        }

        if (module_ind_type == SXD_PDDR_MODULE_IND_TYPE_ELS_E) {
            ret = __get_module_info_els(ELS_MODULE_INFO, buf, dev, module_id);
            goto out;
        }
    }

    priv = sx_priv(dev);

    spin_lock_irqsave(&priv->db_lock, flags);
    locked = true;

    ret = sx_core_get_ports_by_module(dev, slot_id, module_id, local_ports, sysports, &ports_cnt);
    if (ret) {
        sxd_log_err("%s: sx_core_get_ports_by_module failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out;
    }

    if (ports_cnt == 0) {
        sprintf(buf, "%s module [%d] not mapped to a local port ", OP_NOT_SUPPORTED, module_id);
        sxd_log_notice("%s: %s for %s: not mapped to a local port \n", __func__, OP_NOT_SUPPORTED, kobject_name(kobj));
        ret = -EOPNOTSUPP;
        goto out;
    }

    for (mapped_port_idx = 0; mapped_port_idx <= ports_cnt; mapped_port_idx++) {
        if (mapped_port_idx == ports_cnt) { /* There are no mapped ports associated with the module */
            sprintf(buf, "%s local_port [0x%x] is not mapped to swid ", OP_NOT_SUPPORTED, local_ports[0]);
            sxd_log_notice("%s: %s for %s: not mapped to swid \n", __func__, OP_NOT_SUPPORTED, kobject_name(kobj));
            ret = -EOPNOTSUPP;
            goto out;
        }
        if (priv->local_to_swid_db[local_ports[mapped_port_idx]] == SWID_NUM_DONT_CARE) {
            /* Port is not mapped to a swid - continue to the next one */
            continue;
        } else {
            /* We'll use priv->local_to_swid_db[local_ports[mapped_port_idx]] since it's mapped to a swid */
            break;
        }
    }

    spin_unlock_irqrestore(&priv->db_lock, flags);
    locked = false;

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret == 0) {
        ret = _get_pddr(dev,
                        local_ports[mapped_port_idx],
                        buf,
                        SXD_PDDR_PAGE_SELECT_MODULE_INFO_PAGE_E,
                        module_ind_type, 0);
        if (ret > 0) {
            ret += _get_pddr(dev,
                             local_ports[mapped_port_idx],
                             buf,
                             SXD_PDDR_PAGE_SELECT_TROUBLESHOOTING_INFO_PAGE_E,
                             module_ind_type, ret);
        }
        sx_core_sysfs_dev_release(dev);
    }
out:
    if (locked) {
        spin_unlock_irqrestore(&priv->db_lock, flags);
    }
    return ret;
}

static ssize_t show_module_ext_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int            ret = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;

    if (!kobj->parent) {
        sxd_log_err("Parent of module ext info sysfs node is NULL\n");
        ret = -EINVAL;
        goto out;
    }

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n", __func__, ret,
                    kobject_name(kobj));
        goto out;
    }
    ret = __get_module_info_els(ELS_MODULE_EXT_INFO, buf, dev, module_id);

out:
    return ret;
}

static int __els_operation_cmd_set(struct sx_dev *dev, uint8_t slot_id, uint8_t module_id, uint8_t cmd)
{
    int                       err = 0;
    struct sx_priv           *priv = sx_priv(dev);
    struct ku_access_pmmp_reg pmmp_reg_data;
    bool                      release_lock = false;

    mutex_lock(&priv->module_access_mutex);
    release_lock = true;

    memset(&pmmp_reg_data, 0, sizeof(pmmp_reg_data));
    pmmp_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&pmmp_reg_data.op_tlv, PMMP_REG_ID, EMAD_METHOD_WRITE);

    pmmp_reg_data.pmmp_reg.els_trigger_mode = 1;
    pmmp_reg_data.pmmp_reg.module = module_id;
    pmmp_reg_data.pmmp_reg.slot_index = slot_id;
    pmmp_reg_data.pmmp_reg.set_laser_operation = cmd;

    err = sx_ACCESS_REG_PMMP(dev, &pmmp_reg_data);
    if (err) {
        sxd_log_err("Failed to write PMMP, err=%d\n", err);
        goto out;
    }

    if (pmmp_reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to write PMMP, status=%d\n", pmmp_reg_data.op_tlv.status);
        goto out;
    }

out:
    if (release_lock) {
        mutex_unlock(&priv->module_access_mutex);
    }
    return err;
}

static bool __is_valid_els_operation_cmd(uint8_t cmd)
{
    return cmd == PMMP_ELS_OPERATION_CMD_NOP ||
           cmd == PMMP_ELS_OPERATION_CMD_FIBER_CHECK ||
           cmd == PMMP_ELS_OPERATION_CMD_FIBER_TUNING ||
           cmd == PMMP_ELS_OPERATION_CMD_LASER_UP ||
           cmd == PMMP_ELS_OPERATION_CMD_FORCE_LASER_UP;
}

static ssize_t set_els_operation_cmd(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len)
{
    int            ret = 0;
    int            cmd = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;
    bool           hold = false;

    if (!kobj->parent) {
        sxd_log_err("Parent of set_els_operation_cmd sysfs node is NULL\n");
        ret = -EINVAL;
        goto out;
    }

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, buf, len);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = kstrtoint(buf, 10, &cmd);
    if (ret || !__is_valid_els_operation_cmd(cmd)) {
        ret = -EINVAL;
        sxd_log_err("sysfs entry set_els_operation_cmd got invalid value\n");
        goto out_err;
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n", __func__, ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    ret = __els_operation_cmd_set(dev, slot_id, module_id, cmd);
    if (ret) {
        sxd_log_err("Failed in __els_operation_cmd_set\n");
        goto out_err;
    }

out:
    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Set ELS operation command", module_id);
    sx_core_sysfs_dev_release(dev);
    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log,
                         "Module ID %d | Set ELS operation command failed status %d",
                         module_id,
                         ret);
        sx_core_sysfs_dev_release(dev);
    }

    return ret;
}

static int __sx_core_get_module_temperature(struct kobject *kobj, struct sx_temperature_params *params)
{
    int            ret = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;
    bool           hold = false;

    if (!kobj->parent) {
        sxd_log_err("Parent of temperature sysfs node is NULL\n");
        ret = -EINVAL;
        goto out_err;
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 1, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n", __func__, ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    memset(params, 0, sizeof(struct sx_temperature_params));

    ret = sx_core_get_module_temperature(dev, slot_id, module_id, params);
    if (ret) {
        goto out_err;
    }

    sx_int_log_info(&sx_priv(dev)->module_log,
                    "Module ID %d | Read module temperature input %d, label %d",
                    module_id,
                    params->input,
                    params->label);
    sx_core_sysfs_dev_release(dev);
    return ret;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log,
                         "Module ID %d | Read module temperature failed status %d",
                         module_id,
                         ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}

static int __sx_core_get_module_tec(struct kobject *kobj, struct sx_tec_params *params)
{
    int            ret = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;

    if (!kobj->parent) {
        sxd_log_err("Parent of tec sysfs node is NULL\n");
        ret = -EINVAL;
        goto out;
    }
    /* 2 is the offset of the tec sysfs node,  e.g. module$/temperature/tec */
    ret = __module_sysfs_get_dev_slot_module(kobj, 2, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n", __func__, ret,
                    kobject_name(kobj));
        goto out;
    }

    memset(params, 0, sizeof(struct sx_tec_params));

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out;
    }

    ret = sx_core_get_module_tec(dev, slot_id, module_id, params);
    if (ret) {
        sxd_log_err("Module ID %d | Read module tec failed status %d", module_id, ret);
        goto out_release;
    }
    sx_core_sysfs_dev_release(dev);

    sx_int_log_info(&sx_priv(dev)->module_log,
                    "Module ID %d | Cooling level %d, Max TEC power %d, TEC power %d",
                    module_id,
                    params->cooling_level,
                    params->max_tec_power,
                    params->tec_power);

    sx_int_log_info(&sx_priv(dev)->module_log,
                    "Module ID %d | TEC power warn hi/lo %d/%d, alarm hi/lo %d/%d",
                    module_id,
                    params->tec_power_warning_high,
                    params->tec_power_warning_low,
                    params->tec_power_alarm_high,
                    params->tec_power_alarm_low);

    sx_int_log_info(&sx_priv(
                        dev)->module_log,
                    "Module ID %d | Max set point temp %d, Set point temp %d, Set point warn hi/lo %d/%d, alarm hi/lo %d/%d",
                    module_id,
                    params->max_set_point_temperature,
                    params->set_point_temperature,
                    params->set_point_temperature_warning_high,
                    params->set_point_temperature_warning_low,
                    params->set_point_temperature_alarm_high,
                    params->set_point_temperature_alarm_low);

    sx_int_log_info(&sx_priv(dev)->module_log,
                    "Module ID %d | Max cooling level %d, Min cooling level %d",
                    module_id,
                    params->max_cooling_level,
                    params->min_cooling_level);

    return ret;

out_release:
    sx_core_sysfs_dev_release(dev);
out:
    sx_int_log_error(&sx_priv(dev)->module_log,
                     "Module ID %d | Read module tec failed status %d",
                     module_id,
                     ret);
    return ret;
}

static ssize_t show_module_temp_input(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                          ret = 0;
    struct sx_temperature_params params;
    int                          len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_module_temperature(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module temperature information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%llu\n", params.input);

    return len;
}

static ssize_t show_module_temp_label(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                          ret = 0;
    struct sx_temperature_params params;
    int                          len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_module_temperature(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module temperature information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%s\n", params.label);

    return len;
}

static ssize_t show_module_temp_threshold_hi(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                          ret = 0;
    struct sx_temperature_params params;
    int                          len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_module_temperature(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module temperature high information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%d\n", params.threshold_hi);

    return len;
}

static ssize_t show_module_temp_threshold_lo(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                          ret = 0;
    struct sx_temperature_params params;
    int                          len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_module_temperature(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module temperature low information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%d\n", params.threshold_lo);

    return len;
}

static ssize_t show_module_temp_threshold_critical_hi(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                          ret = 0;
    struct sx_temperature_params params;
    int                          len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_module_temperature(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module temperature threshold critical high information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%d\n", params.threshold_critical_hi);

    return len;
}

static ssize_t show_module_temp_threshold_critical_lo(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                          ret = 0;
    struct sx_temperature_params params;
    int                          len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_module_temperature(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module temperature threshold critical low information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%d\n", params.threshold_critical_lo);

    return len;
}

static ssize_t show_module_tec_cooling_level(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }
    ret = __sx_core_get_module_tec(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module tec cooling_level information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%u\n", params.cooling_level);

    return len;
}

static ssize_t show_module_tec_max_power(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }
    ret = __sx_core_get_module_tec(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module tec max_tec_power information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%u\n", params.max_tec_power);

    return len;
}

static ssize_t show_module_tec_power(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }
    ret = __sx_core_get_module_tec(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module tec tec_power information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%u\n", params.tec_power);

    return len;
}

static ssize_t show_module_tec_power_warning_high(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_module_tec(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module tec tec_power_warning_high information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%u\n", params.tec_power_warning_high);

    return len;
}

static ssize_t show_module_tec_power_warning_low(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_module_tec(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module tec tec_power_warning_low information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%u\n", params.tec_power_warning_low);

    return len;
}

static ssize_t show_module_tec_power_alarm_high(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_module_tec(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module tec tec_power_alarm_high information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%u\n", params.tec_power_alarm_high);

    return len;
}

static ssize_t show_module_tec_power_alarm_low(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_module_tec(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module tec tec_power_alarm_low information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%u\n", params.tec_power_alarm_low);

    return len;
}

static ssize_t show_module_tec_max_set_point_temperature(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_module_tec(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module tec max_set_point_temperature information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%u\n", params.max_set_point_temperature);

    return len;
}

static ssize_t show_module_tec_set_point_temperature(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_module_tec(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module tec set_point_temperature information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%u\n", params.set_point_temperature);

    return len;
}


static ssize_t show_module_tec_set_point_temperature_warning_high(struct kobject        *kobj,
                                                                  struct kobj_attribute *attr,
                                                                  char                  *buf)
{
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_module_tec(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module tec set_point_temperature_warning_high information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%u\n", params.set_point_temperature_warning_high);

    return len;
}

static ssize_t show_module_tec_set_point_temperature_warning_low(struct kobject        *kobj,
                                                                 struct kobj_attribute *attr,
                                                                 char                  *buf)
{
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_module_tec(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module tec set_point_temperature_warning_low information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%u\n", params.set_point_temperature_warning_low);

    return len;
}

static ssize_t show_module_tec_set_point_temperature_alarm_high(struct kobject        *kobj,
                                                                struct kobj_attribute *attr,
                                                                char                  *buf)
{
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_module_tec(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module tec set_point_temperature_alarm_high information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%u\n", params.set_point_temperature_alarm_high);

    return len;
}

static ssize_t show_module_tec_set_point_temperature_alarm_low(struct kobject        *kobj,
                                                               struct kobj_attribute *attr,
                                                               char                  *buf)
{
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_module_tec(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module tec set_point_temperature_alarm_low information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%u\n", params.set_point_temperature_alarm_low);

    return len;
}

static ssize_t show_module_tec_max_cooling_level(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }
    ret = __sx_core_get_module_tec(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module tec max_cooling_level information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%u\n", params.max_cooling_level);

    return len;
}

static ssize_t show_module_tec_min_cooling_level(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                  ret = 0;
    struct sx_tec_params params;
    int                  len = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __sx_core_get_module_tec(kobj, &params);
    if (ret) {
        if (ret != -ENODEV) {
            sxd_log_err("Failed to get module tec min_cooling_level information\n");
        }
        return ret;
    }

    len = sprintf(buf, "%u\n", params.min_cooling_level);

    return len;
}

static ssize_t show_module_status(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                            ret = 0;
    int                            len = 0;
    struct sx_dev                 *dev = NULL;
    uint8_t                        slot_id = 0;
    uint8_t                        module_id = 0;
    struct sx_module_status_params params;
    bool                           hold = false;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("show_module_status: __module_sysfs_get_dev_slot_module failed (%d) for %s\n",
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    ret = __sx_core_get_module_status(dev, slot_id, module_id, &params);
    if (ret) {
        sxd_log_err("show_module_status: __sx_core_get_module_status failed (%d) for %s\n",
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Read module status %d", module_id, params.status);
    sx_core_sysfs_dev_release(dev);

    len = sprintf(buf, "%d\n", params.status);
    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log,
                         "Module ID %d | Read module status failed status %d",
                         module_id,
                         ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}

static ssize_t show_module_error_type(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int                            ret = 0;
    int                            len = 0;
    struct sx_dev                 *dev = NULL;
    uint8_t                        slot_id = 0;
    uint8_t                        module_id = 0;
    struct sx_module_status_params params;
    bool                           hold = false;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("show_module_error_type: __module_sysfs_get_dev_slot_module failed (%d) for %s\n",
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    ret = __sx_core_get_module_status(dev, slot_id, module_id, &params);
    if (ret) {
        sxd_log_err("show_module_error_type: __sx_core_get_module_status failed (%d) for %s\n",
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    sx_int_log_info(&sx_priv(dev)->module_log,
                    "Module ID %d | Read module status error %d",
                    module_id,
                    params.error_type);
    sx_core_sysfs_dev_release(dev);

    len = sprintf(buf, "%d\n", params.error_type);
    return len;


out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log,
                         "Module ID %d | Read module status error failed status %d",
                         module_id,
                         ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}

int sx_core_create_module_sysfs_extension_for_indmod(struct kobject *parent)
{
    int err = 0;

    err = sysfs_create_file(parent, &(module_control_attr.attr));
    if (err) {
        goto out;
    }

    err = sysfs_create_file(parent, &(module_hw_reset_attr.attr));
    if (err) {
        goto phase1_err;
    }

    err = sysfs_create_file(parent, &(module_low_power_mode_attr.attr));
    if (err) {
        goto phase2_err;
    }

    err = sysfs_create_file(parent, &(module_hw_present_attr.attr));
    if (err) {
        goto phase3_err;
    }

    err = sysfs_create_file(parent, &(module_interrupt_attr.attr));
    if (err) {
        goto phase4_err;
    }

    err = sysfs_create_file(parent, &(module_power_good_attr.attr));
    if (err) {
        goto phase5_err;
    }

    err = sysfs_create_file(parent, &(module_reinsert_attr.attr));
    if (err) {
        goto phase6_err;
    }

    err = sysfs_create_file(parent, &(module_power_limit_attr.attr));
    if (err) {
        goto phase7_err;
    }

    err = sysfs_create_file(parent, &(module_frequency_attr.attr));
    if (err) {
        goto phase8_err;
    }

    err = sysfs_create_file(parent, &(module_frequency_support_attr.attr));
    if (err) {
        goto phase9_err;
    }

    goto out;

phase9_err:
    sysfs_remove_file(parent, &(module_frequency_attr.attr));
phase8_err:
    sysfs_remove_file(parent, &(module_power_limit_attr.attr));
phase7_err:
    sysfs_remove_file(parent, &(module_reinsert_attr.attr));
phase6_err:
    sysfs_remove_file(parent, &(module_power_good_attr.attr));
phase5_err:
    sysfs_remove_file(parent, &(module_interrupt_attr.attr));
phase4_err:
    sysfs_remove_file(parent, &(module_hw_present_attr.attr));
phase3_err:
    sysfs_remove_file(parent, &(module_low_power_mode_attr.attr));
phase2_err:
    sysfs_remove_file(parent, &(module_hw_reset_attr.attr));
phase1_err:
    sysfs_remove_file(parent, &(module_control_attr.attr));
out:
    return err;
}
EXPORT_SYMBOL(sx_core_create_module_sysfs_extension_for_indmod);

void sx_core_delete_module_sysfs_extension_for_indmod(struct kobject *parent)
{
    sysfs_remove_file(parent, &(module_control_attr.attr));
    sysfs_remove_file(parent, &(module_hw_reset_attr.attr));
    sysfs_remove_file(parent, &(module_low_power_mode_attr.attr));
    sysfs_remove_file(parent, &(module_hw_present_attr.attr));
    sysfs_remove_file(parent, &(module_interrupt_attr.attr));
    sysfs_remove_file(parent, &(module_power_good_attr.attr));
    sysfs_remove_file(parent, &(module_reinsert_attr.attr));
    sysfs_remove_file(parent, &(module_power_limit_attr.attr));
    sysfs_remove_file(parent, &(module_frequency_attr.attr));
    sysfs_remove_file(parent, &(module_frequency_support_attr.attr));
}
EXPORT_SYMBOL(sx_core_delete_module_sysfs_extension_for_indmod);

int sx_core_create_module_sysfs_extension_for_s3ip(struct kobject *parent)
{
    int err = 0;

    err = sysfs_create_file(parent, &(module_power_on_attr.attr));
    if (err) {
        goto out;
    }
    err = sysfs_create_file(parent, &(module_tx_disable_attr.attr));
    if (err) {
        goto phase1_err;
    }
    err = sysfs_create_file(parent, &(module_present_attr.attr));
    if (err) {
        goto phase2_err;
    }
    err = sysfs_create_file(parent, &(module_rx_los_attr.attr));
    if (err) {
        goto phase3_err;
    }
    err = sysfs_create_file(parent, &(module_status_attr.attr));
    if (err) {
        goto phase4_err;
    }
    err = sysfs_create_file(parent, &(module_error_type_attr.attr));
    if (err) {
        goto phase5_err;
    }

    return err;

phase5_err:
    sysfs_remove_file(parent, &(module_status_attr.attr));
phase4_err:
    sysfs_remove_file(parent, &(module_rx_los_attr.attr));
phase3_err:
    sysfs_remove_file(parent, &(module_present_attr.attr));
phase2_err:
    sysfs_remove_file(parent, &(module_tx_disable_attr.attr));
phase1_err:
    sysfs_remove_file(parent, &(module_power_on_attr.attr));
out:
    return err;
}
EXPORT_SYMBOL(sx_core_create_module_sysfs_extension_for_s3ip);

void sx_core_delete_module_sysfs_extension_for_s3ip(struct kobject *parent)
{
    sysfs_remove_file(parent, &(module_power_on_attr.attr));
    sysfs_remove_file(parent, &(module_rx_los_attr.attr));
    sysfs_remove_file(parent, &(module_present_attr.attr));
    sysfs_remove_file(parent, &(module_tx_disable_attr.attr));
    sysfs_remove_file(parent, &(module_status_attr.attr));
    sysfs_remove_file(parent, &(module_error_type_attr.attr));
}
EXPORT_SYMBOL(sx_core_delete_module_sysfs_extension_for_s3ip);

/* for misc module sysfs entries under sx_core: id, reset, power_mode_policy, power_mode */
int sx_core_create_module_sysfs_extension_for_misc(struct module_sysfs_node *root, uint8_t module_type)
{
    int             err = 0;
    struct kobject *module = root->module;
    struct kobject *temperature = NULL;

    /* struct kobject *tec = NULL; */

    err = sysfs_create_file(module, &(module_id_attr.attr));
    if (err) {
        goto out;
    }

    err = sysfs_create_file(module, &(module_reset_attr.attr));
    if (err) {
        goto phase1_err;
    }
    err = sysfs_create_file(module, &(module_power_mode_policy_attr.attr));
    if (err) {
        goto phase2_err;
    }
    err = sysfs_create_file(module, &(module_power_mode_attr.attr));
    if (err) {
        goto phase3_err;
    }
    err = sysfs_create_file(module, &(module_info_attr.attr));
    if (err) {
        goto phase4_err;
    }
    err = sysfs_create_file(module, &(module_latched_flag_attr.attr));
    if (err) {
        goto phase5_err;
    }
    temperature = kobject_create_and_add("temperature", module);
    if (temperature == NULL) {
        err = -ENOMEM;
        goto phase6_err;
    }
    err = sysfs_create_file(temperature, &(module_temp_input_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase7_err;
    }
    err = sysfs_create_file(temperature, &(module_temp_label_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase8_err;
    }
    err = sysfs_create_file(temperature, &(module_temp_threshold_hi_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase9_err;
    }
    err = sysfs_create_file(temperature, &(module_temp_threshold_lo_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase10_err;
    }
    err = sysfs_create_file(temperature, &(module_temp_threshold_critical_hi_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase11_err;
    }
    err = sysfs_create_file(temperature, &(module_temp_threshold_critical_lo_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase12_err;
    }
    if (module_type == SXD_PMTM_MODULE_TYPE_OSFP_ELS_E) {
        err = sysfs_create_file(module, &(module_ext_info_attr.attr));
        if (err) {
            err = -ENOMEM;
            goto phase13_err;
        }
        err = sysfs_create_file(module, &(module_els_operation_cmd_attr.attr));
        if (err) {
            err = -ENOMEM;
            goto phase14_err;
        }
    }
    root->temperature = temperature;

    return err;

phase14_err:
    if (module_type == SXD_PMTM_MODULE_TYPE_OSFP_ELS_E) {
        sysfs_remove_file(module, &(module_ext_info_attr.attr));
    }
phase13_err:
    sysfs_remove_file(temperature, &(module_temp_threshold_critical_lo_attr.attr));
phase12_err:
    sysfs_remove_file(temperature, &(module_temp_threshold_critical_hi_attr.attr));
phase11_err:
    sysfs_remove_file(temperature, &(module_temp_threshold_lo_attr.attr));
phase10_err:
    sysfs_remove_file(temperature, &(module_temp_threshold_hi_attr.attr));
phase9_err:
    sysfs_remove_file(temperature, &(module_temp_label_attr.attr));
phase8_err:
    sysfs_remove_file(temperature, &(module_temp_input_attr.attr));
phase7_err:
    kobject_put(temperature);
phase6_err:
    sysfs_remove_file(module, &(module_latched_flag_attr.attr));
phase5_err:
    sysfs_remove_file(module, &(module_info_attr.attr));
phase4_err:
    sysfs_remove_file(module, &(module_power_mode_attr.attr));
phase3_err:
    sysfs_remove_file(module, &(module_power_mode_policy_attr.attr));
phase2_err:
    sysfs_remove_file(module, &(module_reset_attr.attr));
phase1_err:
    sysfs_remove_file(module, &(module_id_attr.attr));
out:
    return err;
}
EXPORT_SYMBOL(sx_core_create_module_sysfs_extension_for_misc);

void sx_core_delete_module_sysfs_extension_for_misc(struct module_sysfs_node *root)
{
    sysfs_remove_file(root->temperature, &(module_temp_threshold_hi_attr.attr));
    sysfs_remove_file(root->temperature, &(module_temp_threshold_lo_attr.attr));
    sysfs_remove_file(root->temperature, &(module_temp_threshold_critical_hi_attr.attr));
    sysfs_remove_file(root->temperature, &(module_temp_threshold_critical_lo_attr.attr));
    sysfs_remove_file(root->temperature, &(module_temp_label_attr.attr));
    sysfs_remove_file(root->temperature, &(module_temp_input_attr.attr));
    kobject_put(root->temperature);
    sysfs_remove_file(root->module, &(module_latched_flag_attr.attr));
    sysfs_remove_file(root->module, &(module_info_attr.attr));
    sysfs_remove_file(root->module, &(module_power_mode_attr.attr));
    sysfs_remove_file(root->module, &(module_power_mode_policy_attr.attr));
    sysfs_remove_file(root->module, &(module_reset_attr.attr));
    sysfs_remove_file(root->module, &(module_id_attr.attr));
    sysfs_remove_file(root->module, &(module_ext_info_attr.attr));
    sysfs_remove_file(root->module, &(module_els_operation_cmd_attr.attr));
}
EXPORT_SYMBOL(sx_core_delete_module_sysfs_extension_for_misc);

int sx_core_create_module_sysfs_extension_for_tec(struct module_sysfs_node *root)
{
    int             err = 0;
    struct kobject *temperature = root->temperature;
    struct kobject *tec = NULL;

    if (temperature == NULL) {
        err = -ENOMEM;
        goto out;
    }
    tec = kobject_create_and_add("tec", temperature);
    if (tec == NULL) {
        err = -ENOMEM;
        goto phase1_err;
    }
    err = sysfs_create_file(tec, &(module_tec_cooling_level_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase2_err;
    }
    err = sysfs_create_file(tec, &(module_tec_max_power_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase3_err;
    }
    err = sysfs_create_file(tec, &(module_tec_power_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase4_err;
    }
    err = sysfs_create_file(tec, &(module_tec_power_warning_high_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase5_err;
    }
    err = sysfs_create_file(tec, &(module_tec_power_warning_low_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase6_err;
    }
    err = sysfs_create_file(tec, &(module_tec_power_alarm_high_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase7_err;
    }
    err = sysfs_create_file(tec, &(module_tec_power_alarm_low_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase8_err;
    }
    err = sysfs_create_file(tec, &(module_tec_max_set_point_temperature_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase9_err;
    }
    err = sysfs_create_file(tec, &(module_tec_set_point_temperature_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase10_err;
    }
    err = sysfs_create_file(tec, &(module_tec_set_point_temperature_warning_high_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase11_err;
    }
    err = sysfs_create_file(tec, &(module_tec_set_point_temperature_warning_low_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase12_err;
    }
    err = sysfs_create_file(tec, &(module_tec_set_point_temperature_alarm_high_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase13_err;
    }
    err = sysfs_create_file(tec, &(module_tec_set_point_temperature_alarm_low_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase14_err;
    }
    err = sysfs_create_file(tec, &(module_tec_max_cooling_level_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase15_err;
    }
    err = sysfs_create_file(tec, &(module_tec_min_cooling_level_attr.attr));
    if (err) {
        err = -ENOMEM;
        goto phase16_err;
    }
    root->tec = tec;

    return err;

phase16_err:
    sysfs_remove_file(tec, &(module_tec_min_cooling_level_attr.attr));
phase15_err:
    sysfs_remove_file(tec, &(module_tec_max_cooling_level_attr.attr));
phase14_err:
    sysfs_remove_file(tec, &(module_tec_set_point_temperature_alarm_low_attr.attr));
phase13_err:
    sysfs_remove_file(tec, &(module_tec_set_point_temperature_alarm_high_attr.attr));
phase12_err:
    sysfs_remove_file(tec, &(module_tec_set_point_temperature_warning_low_attr.attr));
phase11_err:
    sysfs_remove_file(tec, &(module_tec_set_point_temperature_warning_high_attr.attr));
phase10_err:
    sysfs_remove_file(tec, &(module_tec_set_point_temperature_attr.attr));
phase9_err:
    sysfs_remove_file(tec, &(module_tec_max_set_point_temperature_attr.attr));
phase8_err:
    sysfs_remove_file(tec, &(module_tec_power_alarm_low_attr.attr));
phase7_err:
    sysfs_remove_file(tec, &(module_tec_power_alarm_high_attr.attr));
phase6_err:
    sysfs_remove_file(tec, &(module_tec_power_warning_low_attr.attr));
phase5_err:
    sysfs_remove_file(tec, &(module_tec_power_warning_high_attr.attr));
phase4_err:
    sysfs_remove_file(tec, &(module_tec_power_attr.attr));
phase3_err:
    sysfs_remove_file(tec, &(module_tec_max_power_attr.attr));
phase2_err:
    sysfs_remove_file(tec, &(module_tec_cooling_level_attr.attr));
phase1_err:
    kobject_put(tec);
out:
    return err;
}
EXPORT_SYMBOL(sx_core_create_module_sysfs_extension_for_tec);

void sx_core_delete_module_sysfs_extension_for_tec(struct module_sysfs_node *root)
{
    if (root->tec) {
        sysfs_remove_file(root->tec, &(module_tec_min_cooling_level_attr.attr));
        sysfs_remove_file(root->tec, &(module_tec_max_cooling_level_attr.attr));
        sysfs_remove_file(root->tec, &(module_tec_set_point_temperature_alarm_low_attr.attr));
        sysfs_remove_file(root->tec, &(module_tec_set_point_temperature_alarm_high_attr.attr));
        sysfs_remove_file(root->tec, &(module_tec_set_point_temperature_warning_low_attr.attr));
        sysfs_remove_file(root->tec, &(module_tec_set_point_temperature_warning_high_attr.attr));
        sysfs_remove_file(root->tec, &(module_tec_set_point_temperature_attr.attr));
        sysfs_remove_file(root->tec, &(module_tec_max_set_point_temperature_attr.attr));
        sysfs_remove_file(root->tec, &(module_tec_cooling_level_attr.attr));
        sysfs_remove_file(root->tec, &(module_tec_power_alarm_low_attr.attr));
        sysfs_remove_file(root->tec, &(module_tec_power_alarm_high_attr.attr));
        sysfs_remove_file(root->tec, &(module_tec_power_warning_low_attr.attr));
        sysfs_remove_file(root->tec, &(module_tec_power_warning_high_attr.attr));
        sysfs_remove_file(root->tec, &(module_tec_power_attr.attr));
        sysfs_remove_file(root->tec, &(module_tec_max_power_attr.attr));
        sysfs_remove_file(root->tec, &(module_tec_cooling_level_attr.attr));
        kobject_put(root->tec);
        root->tec = NULL;
    }
}
EXPORT_SYMBOL(sx_core_delete_module_sysfs_extension_for_tec);

static void __sx_pmpe_dwork_handler(struct work_struct *dwork_p)
{
    pmpe_dwdata_t *dwdata_p = container_of(dwork_p, pmpe_dwdata_t, dwork.work);

    sysfs_notify(dwdata_p->module, NULL, "present");

    /* Set cached module type to invalid per each PMPE update, so that user calling sysfs eeprom would re-query + cache eeprom module type. */
    sx_core_set_cached_module_type(dwdata_p->dev, dwdata_p->local_module, EEPROM_MANAGEMENT_TYPE_INVALID);

    kfree(dwdata_p);
}

#define ASIC0_DEVICE_ID_IN_MULTI_ASIC (1)
static void __sx_handle_pmpe_event(struct completion_info *comp_info, void *context)
{
    struct ku_pmpe_reg pmpe_reg;
    u8                *pmpe_outbox = (u8*)comp_info->skb->data + sizeof(struct sx_emad) +
                                     sizeof(struct sxd_emad_tlv_reg);
    enum sx_module_present_state present = SX_MODULE_PRESENT_INVALID;
    struct sx_priv              *priv = sx_priv(comp_info->dev);
    pmpe_dwdata_t               *dwdata_p;
    int                          ret = 0;
    uint8_t                      global_module_id = 0;
    uint8_t                      local_module_id = 0;
    bool                         need_skip = false;
    struct module_sysfs_node    *default_module_sysfs_root = NULL;

    if (!sx_validate_emad_format(comp_info, MLXSW_PMPE_ID, 4, 0)) {
        return;
    }

    __PMPE_decode(pmpe_outbox, &pmpe_reg, NULL);

    local_module_id = pmpe_reg.module;
    global_module_id = local_module_id;
    default_module_sysfs_root = priv->module_sysfs_arr;
    if (sx_core_has_pci_multi_dev() && MULTI_ASIC_MODULE_SYSFS_IS_SUPPORT(priv)) {
        /* for multi-asic module sysfs, convert local module to global module. */
        ret = sx_multi_asic_module_sysfs_get_global_module(priv->dev_info.dev_info_ro.mgir.hw_info.ga,
                                                           local_module_id,
                                                           &global_module_id);
        if (ret) {
            sxd_log_err("%s: failed to check global and local module map: global module %u, local module %u, ga %u\n",
                        __func__, global_module_id, local_module_id, priv->dev_info.dev_info_ro.mgir.hw_info.ga);
            return;
        }
        sx_core_get_default_modules_sysfs_tree_root(&default_module_sysfs_root);
        if (!default_module_sysfs_root) {
            sxd_log_err("%s: failed to get asic0 module sysfs root: global module %u, local module %u\n",
                        __func__, global_module_id, local_module_id);
            return;
        }
    }

    /* skip if backplane module type */
    ret = sx_multi_asic_module_sysfs_need_skip(global_module_id, &need_skip);
    if (ret) {
        sxd_log_err("%s: failed to check module sysfs skipping mode: module %u.\n",
                    __func__, global_module_id);
        return;
    }

    if (need_skip) {
        /* It is skipped because no sysfs entry for it. While it should be weird because backplane module is assumed to be always ON. */
        sxd_log_debug("%s: module type is backplane and ignored due to no module sysfs (module %u)\n",
                      __func__, global_module_id);
        return;
    }

    if (PMPE_MODULE_PLUGGED(pmpe_reg.module_status)) {
        present = SX_MODULE_PRESENT_PLUGGED;
    } else if (PMPE_MODULE_PLUGGED_ERROR(pmpe_reg.module_status)) {
        present = SX_MODULE_PRESENT_PLUGGED_ERROR;
    } else {
        present = SX_MODULE_PRESENT_UNPLUGGED;
    }

    sx_int_log_info(&priv->module_log,
                    "Module ID %d | Receive PMPE event: module_status %d, present %d | DB: present %d",
                    local_module_id, pmpe_reg.module_status, present,
                    priv->module_data[pmpe_reg.slot_index][local_module_id].present);

    if (present != priv->module_data[pmpe_reg.slot_index][local_module_id].present) {
        priv->module_data[pmpe_reg.slot_index][local_module_id].present = present;
        dwdata_p = kzalloc(sizeof(*dwdata_p), GFP_ATOMIC);
        if (!dwdata_p) {
            sxd_log_err("PMPE handler failed to allocated memory\n");
            return;
        }
        dwdata_p->module = default_module_sysfs_root[global_module_id].module;
        dwdata_p->dev = comp_info->dev;
        dwdata_p->local_module = local_module_id;
        INIT_DELAYED_WORK(&dwdata_p->dwork, __sx_pmpe_dwork_handler);
        /* The delayed queueing used to change context */
        queue_delayed_work(priv->generic_wq, &dwdata_p->dwork, 0);
    }
}

static void __sx_mcion_dwork_handler(struct work_struct *dwork_p)
{
    mcion_dwdata_t *dwdata_p = container_of(dwork_p, mcion_dwdata_t, dwork.work);

    if (dwdata_p->hw_present) {
        sysfs_notify(dwdata_p->module, NULL, "hw_present");
    }

    if (dwdata_p->interrupt) {
        sysfs_notify(dwdata_p->module, NULL, "interrupt");
    }

    if (dwdata_p->power_good) {
        sysfs_notify(dwdata_p->module, NULL, "power_good");
    }

    kfree(dwdata_p);
}

static void __sx_handle_mcion_event(struct completion_info *comp_info, void *context)
{
    struct ku_mcion_reg mcion_reg;
    u8                 *mcion_outbox = (u8*)comp_info->skb->data + sizeof(struct sx_emad) +
                                       sizeof(struct sxd_emad_tlv_reg);
    uint8_t                         slot_id = 0;
    uint8_t                         module_id = 0;
    enum sx_module_interrupt_state  interrupt = SX_MODULE_INTERRUPT_INVALID;
    enum sx_module_hw_present_state hw_present = SX_MODULE_HW_PRESENT_INVALID;
    enum sx_module_power_good_state power_good = SX_MODULE_POWER_GOOD_INVALID;
    struct sx_priv                 *priv = sx_priv(comp_info->dev);
    mcion_dwdata_t                 *dwdata_p;
    bool                            send_notify = false;
    int                             ret = 0;
    bool                            need_skip = false;

    if (!sx_validate_emad_format(comp_info, MLXSW_MCION_ID, 6, 0)) {
        return;
    }

    __MCION_decode(mcion_outbox, &mcion_reg, NULL);

    sx_int_log_info(&priv->module_log,
                    "Module ID %d | Receive MCION event: module_status_bits_valid 0x%x, module_status_bits 0x%x | DB: hw_present %d, interrupt %d, power_good %d",
                    mcion_reg.module,
                    mcion_reg.module_status_bits_valid,
                    mcion_reg.module_status_bits,
                    priv->module_data[mcion_reg.slot_index][mcion_reg.module].independent_params.hw_present,
                    priv->module_data[mcion_reg.slot_index][mcion_reg.module].independent_params.interrupt,
                    priv->module_data[mcion_reg.slot_index][mcion_reg.module].independent_params.power_good);

    /* skip if backplane module type */
    ret = sx_multi_asic_module_sysfs_need_skip(module_id, &need_skip);
    if (ret) {
        sxd_log_err("%s: failed to check module map: global module %u, local module %u\n",
                    __func__, module_id, module_id);
        return;
    }

    if (need_skip) {
        sxd_log_debug("%s: module type is backplane and ignored due to no module sysfs (module %u)\n",
                      __func__, module_id);
        return;
    }

    dwdata_p = kzalloc(sizeof(*dwdata_p), GFP_ATOMIC);
    if (!dwdata_p) {
        sxd_log_err("MCION handler failed to allocated memory\n");
        return;
    }

    slot_id = mcion_reg.slot_index;
    module_id = mcion_reg.module;

    if (!(mcion_reg.module_status_bits_valid & MCION_STATUS_MASK_HW_PRESENT)) {
        if (mcion_reg.module_status_bits & MCION_STATUS_MASK_HW_PRESENT) {
            hw_present = SX_MODULE_HW_PRESENT_ON;
        } else {
            hw_present = SX_MODULE_HW_PRESENT_OFF;
        }

        if (hw_present != priv->module_data[slot_id][module_id].independent_params.hw_present) {
            send_notify = true;
            dwdata_p->hw_present = true;
            priv->module_data[slot_id][module_id].independent_params.hw_present = hw_present;
        }
    }

    if (!(mcion_reg.module_status_bits_valid & MCION_STATUS_MASK_INTERRUPT)) {
        if (mcion_reg.module_status_bits & MCION_STATUS_MASK_INTERRUPT) {
            interrupt = SX_MODULE_INTERRUPT_ON;
        } else {
            interrupt = SX_MODULE_INTERRUPT_OFF;
        }
        if (interrupt != priv->module_data[slot_id][module_id].independent_params.interrupt) {
            send_notify = true;
            dwdata_p->interrupt = true;
            priv->module_data[slot_id][module_id].independent_params.interrupt = interrupt;
        }
    }

    if (!(mcion_reg.module_status_bits_valid & MCION_STATUS_MASK_POWER_GOOD)) {
        if (mcion_reg.module_status_bits & MCION_STATUS_MASK_POWER_GOOD) {
            power_good = SX_MODULE_POWER_GOOD_ON;
        } else {
            power_good = SX_MODULE_POWER_GOOD_OFF;
        }
        if (power_good != priv->module_data[slot_id][module_id].independent_params.power_good) {
            send_notify = true;
            dwdata_p->power_good = true;
            priv->module_data[slot_id][module_id].independent_params.power_good = power_good;
        }
    }

    sx_int_log_info(&priv->module_log,
                    "Module ID %d | Receive MCION event: hw_present %d, interrupt %d, power_good %d | Notify: %d, sysfs: %s",
                    module_id,
                    hw_present,
                    interrupt,
                    power_good,
                    send_notify,
                    priv->module_sysfs_arr[module_id].module->name);

    if (send_notify) {
        dwdata_p->module = priv->module_sysfs_arr[module_id].module;
        INIT_DELAYED_WORK(&dwdata_p->dwork, __sx_mcion_dwork_handler);
        /* The delayed queueing used to change context */
        queue_delayed_work(priv->generic_wq, &dwdata_p->dwork, 0);
    } else {
        kfree(dwdata_p);
    }
}

int sx_module_sysfs_register_module_event_handler(struct sx_dev *dev)
{
    int                       err = 0;
    union ku_filter_critireas crit;

    memset(&crit, 0, sizeof(crit));

    err = sx_core_add_synd(0, SXD_TRAP_ID_PMPE, L2_TYPE_DONT_CARE, 0, "sx_core", 0,
                           &crit, "PMPE-event", __sx_handle_pmpe_event, NULL,
                           CHECK_DUP_ENABLED_E, dev, NULL, 1);
    if (err) {
        sxd_log_err("%s: Failed registering PMPE event rx_handler", __func__);
        return err;
    }

    err = sx_core_add_synd(0, SXD_TRAP_ID_MCION, L2_TYPE_DONT_CARE, 0, "sx_core", 0,
                           &crit, "MCION-event", __sx_handle_mcion_event, NULL,
                           CHECK_DUP_ENABLED_E, dev, NULL, 1);
    if (err) {
        sxd_log_err("%s: Failed registering MCION event rx_handler", __func__);
        return err;
    }
    return err;
}

int sx_module_sysfs_unregister_module_event_handler(struct sx_dev *dev)
{
    int                       err = 0;
    union ku_filter_critireas crit;

    memset(&crit, 0, sizeof(crit));

    err = sx_core_remove_synd(0, SXD_TRAP_ID_PMPE, L2_TYPE_DONT_CARE, 0,
                              &crit, NULL,
                              dev, NULL, NULL, 1, NULL);
    if (err) {
        sxd_log_err("%s: Failed unregistering PMPE event rx_handler", __func__);
        return err;
    }

    err = sx_core_remove_synd(0, SXD_TRAP_ID_MCION, L2_TYPE_DONT_CARE, 0,
                              &crit, NULL,
                              dev, NULL, NULL, 1, NULL);
    if (err) {
        sxd_log_err("%s: Failed unregistering MCION event rx_handler", __func__);
        return err;
    }


    return err;
}

int sx_core_get_module_control(struct sx_dev *dev, uint8_t slot_id, uint8_t module_id, bool *is_independent_p)
{
    int                       err = 0;
    struct sx_priv           *priv = sx_priv(dev);
    struct ku_access_mmcr_reg mmcr_reg_data;
    bool                      is_independent = false;

    if (priv->independent_module_params.module_support_type == SXD_MODULE_MASTER_MODE_SW_CONTROL_E) {
        memset(&mmcr_reg_data, 0, sizeof(mmcr_reg_data));
        mmcr_reg_data.dev_id = dev->device_id;
        sx_cmd_set_op_tlv(&mmcr_reg_data.op_tlv, MLXSW_MMCR_ID, EMAD_METHOD_QUERY);

        err = sx_ACCESS_REG_MMCR(dev, &mmcr_reg_data);
        if (err) {
            sxd_log_err("Failed to query MMCR, err=%d\n", err);
            goto out;
        }

        if (mmcr_reg_data.op_tlv.status) {
            err = -EINVAL;
            sxd_log_err("Failed to query MMCR, status=%d\n", mmcr_reg_data.op_tlv.status);
            goto out;
        }

        /*
         * Get MMCR bit X for module ID X.
         */
        if (SX_CORE_BITMAP_GET(mmcr_reg_data.mmcr_reg.next_module_control,
                               SXD_MMCR_NEXT_MODULE_CONTROL_NUM,
                               sizeof(mmcr_reg_data.mmcr_reg.next_module_control[0]) * 8,
                               module_id)) {
            is_independent = true;
        } else {
            is_independent = false;
        }
    } else if (priv->independent_module_params.module_support_type ==
               SXD_MODULE_MASTER_MODE_SW_CONTROL_NO_I2C_CONNECTIVITY_E) {
        is_independent = true;
    }

out:
    *is_independent_p = is_independent;
    return err;
}
EXPORT_SYMBOL(sx_core_get_module_control);

static int __module_event_enable_set(struct sx_dev *dev, uint8_t slot_id, uint8_t module_id, sxd_pmaos_e_t gen_mode)
{
    int                        err = 0;
    struct sx_priv            *priv = sx_priv(dev);
    struct ku_access_pmaos_reg pmaos_reg_data;
    bool                       release_lock = false;

    mutex_lock(&priv->module_access_mutex);
    release_lock = true;

    memset(&pmaos_reg_data, 0, sizeof(pmaos_reg_data));
    pmaos_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&pmaos_reg_data.op_tlv, PMAOS_REG_ID, EMAD_METHOD_WRITE);

    pmaos_reg_data.pmaos_reg.slot_index = slot_id;
    pmaos_reg_data.pmaos_reg.module = module_id;
    pmaos_reg_data.pmaos_reg.ase = 0; /* Don't change admin state */
    pmaos_reg_data.pmaos_reg.ee = 1;
    pmaos_reg_data.pmaos_reg.e = gen_mode;

    err = sx_ACCESS_REG_PMAOS(dev, &pmaos_reg_data);
    if (err) {
        sxd_log_err("Failed to write PMAOS, err=%d\n", err);
        goto out;
    }

    if (pmaos_reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to write PMAOS, status=%d\n", pmaos_reg_data.op_tlv.status);
        goto out;
    }

out:
    if (release_lock) {
        mutex_unlock(&priv->module_access_mutex);
    }
    return err;
}

static int __module_control_set(struct sx_dev *dev, uint8_t slot_id, uint8_t module_id, bool enable_sw_control)
{
    int                       err = 0;
    struct sx_priv           *priv = sx_priv(dev);
    struct ku_access_mmcr_reg mmcr_reg_data;
    bool                      release_lock = false;

    memset(&mmcr_reg_data, 0, sizeof(mmcr_reg_data));
    mmcr_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mmcr_reg_data.op_tlv, MLXSW_MMCR_ID, EMAD_METHOD_WRITE);

    /*
     * Set MMCR bit X for module ID X.
     * To shift control from SW to FW matching bit in module_sw_control should be 0.
     * To shift control from FW to SW matching bit in module_sw_control should be 1.
     */
    SX_CORE_BITMAP_SET(mmcr_reg_data.mmcr_reg.module_control_mask,
                       SXD_MMCR_MODULE_CONTROL_MASK_NUM,
                       sizeof(mmcr_reg_data.mmcr_reg.module_control_mask[0]) * 8,
                       module_id);
    if (enable_sw_control) {
        SX_CORE_BITMAP_SET(mmcr_reg_data.mmcr_reg.next_module_control,
                           SXD_MMCR_NEXT_MODULE_CONTROL_NUM,
                           sizeof(mmcr_reg_data.mmcr_reg.next_module_control[0]) * 8,
                           module_id);
    }

    mutex_lock(&priv->module_access_mutex);
    release_lock = true;

    err = sx_ACCESS_REG_MMCR(dev, &mmcr_reg_data);
    if (err) {
        sxd_log_err("Failed to write MMCR, err=%d\n", err);
        goto out;
    }

    if (mmcr_reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to write MMCR, status=%d\n", mmcr_reg_data.op_tlv.status);
        goto out;
    }

out:
    if (release_lock) {
        mutex_unlock(&priv->module_access_mutex);
    }
    return err;
}

static int __sx_core_set_module_control(struct sx_dev *dev, uint8_t slot_id, uint8_t module_id)
{
    int             err = 0;
    struct sx_priv *priv = sx_priv(dev);
    bool            is_independent = false;

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        sxd_log_err("%s: Failed to get current module control state.\n", __func__);
        goto out;
    }

    if (is_independent == false) {
        sxd_log_err("%s: Failed control change: module is dependent (FW control).\n", __func__);
        err = -EFAULT;
        goto out;
    }

    err = __module_control_set(dev, slot_id, module_id, false);
    if (err) {
        goto out;
    }

    /* Control change from SW to FW is valid for plugged modules.
     * When FW done with module init PMPE will be triggered.
     * To suppress this event from present sysfs, init DB with "plugged" state.
     */
    priv->module_data[slot_id][module_id].present = SX_MODULE_PRESENT_PLUGGED;

    /* Enable update events (all events) */
    err = __module_event_enable_set(dev, slot_id, module_id, SXD_PMAOS_E_GENERATE_EVENT_E);
    if (err) {
        goto out;
    }

    priv->module_data[slot_id][module_id].independent_params.hw_present = SX_MODULE_HW_PRESENT_INVALID;
    priv->module_data[slot_id][module_id].independent_params.interrupt = SX_MODULE_INTERRUPT_INVALID;
    priv->module_data[slot_id][module_id].independent_params.power_good = SX_MODULE_POWER_GOOD_INVALID;

out:
    return err;
}

static ssize_t show_module_control(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int            ret = 0;
    bool           is_independent = false;
    int            len = 0;
    int            val = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;
    bool           hold = false;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    ret = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (ret) {
        sxd_log_err("%s: sx_core_get_module_control failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    if (is_independent) {
        val = 1;
    }

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Read control %d", module_id, is_independent);
    sx_core_sysfs_dev_release(dev);

    len = sprintf(buf, "%d\n", val);
    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log, "Module ID %d | Read control failed status %d", module_id, ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}

static ssize_t store_module_control(struct kobject        *kobj,
                                    struct kobj_attribute *attr,
                                    const char            *buf,
                                    size_t                 len)
{
    int            ret = 0;
    int            control = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, buf, len);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = kstrtoint(buf, 10, &control);
    if (ret) {
        sxd_log_err("sysfs entry control got invalid value\n");
        ret = -EINVAL;
        goto out;
    }

    if (control != 0) {
        sxd_log_err("sysfs entry control got invalid value\n");
        ret = -EINVAL;
        goto out;
    }

    if (!kobj->parent) {
        sxd_log_err("Parent of control sysfs node is NULL\n");
        ret = -EINVAL;
        goto out;
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out;
    }

    ret = __sx_core_set_module_control(dev, slot_id, module_id);
    if (ret) {
        sxd_log_err("%s: __sx_core_set_module_control failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out;
    }

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Write control %d", module_id, control);

    return len;

out:
    sx_int_log_error(&sx_priv(dev)->module_log, "Module ID %d | Read control failed status %d", module_id, ret);
    return ret;
}

static int __sx_core_get_module_hw_reset(struct sx_dev *dev, uint8_t slot_id, uint8_t module_id, bool *is_hw_reset)
{
    int                        err = 0;
    struct sx_priv            *priv = sx_priv(dev);
    struct ku_access_mcion_reg mcion_reg_data;
    bool                       release_lock = false;
    bool                       is_independent = false;

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        goto out;
    }
    if (is_independent == false) {
        sxd_log_err("%s: Failed control change: module is dependent (FW control).\n", __func__);
        err = -EFAULT;
        goto out;
    }

    memset(&mcion_reg_data, 0, sizeof(mcion_reg_data));
    mcion_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mcion_reg_data.op_tlv, MLXSW_MCION_ID, EMAD_METHOD_QUERY);

    mcion_reg_data.mcion_reg.slot_index = slot_id;
    mcion_reg_data.mcion_reg.module = module_id;


    mutex_lock(&priv->module_access_mutex);
    release_lock = true;

    err = sx_ACCESS_REG_MCION(dev, &mcion_reg_data);
    if (err) {
        sxd_log_err("Failed to query MCION, err=%d\n", err);
        goto out;
    }

    if (mcion_reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to query MCION, status=%d\n", mcion_reg_data.op_tlv.status);
        goto out;
    }

    *is_hw_reset = mcion_reg_data.mcion_reg.module_inputs & 0x1;

out:
    if (release_lock) {
        mutex_unlock(&priv->module_access_mutex);
    }
    return err;
}

static ssize_t show_module_hw_reset(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int            ret = 0;
    bool           is_hw_reset = false;
    int            len = 0;
    int            val = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;
    bool           hold = false;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    ret = __sx_core_get_module_hw_reset(dev, slot_id, module_id, &is_hw_reset);
    if (ret) {
        sxd_log_err("%s: __sx_core_get_module_hw_reset failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    if (is_hw_reset) {
        val = 1;
    }

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Read hw_reset %d", module_id, is_hw_reset);
    sx_core_sysfs_dev_release(dev);

    len = sprintf(buf, "%d\n", val);
    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log, "Module ID %d | Read hw_reset failed status %d", module_id, ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}


static int __sx_core_set_module_hw_reset(struct sx_dev *dev, uint8_t slot_id, uint8_t module_id, int hw_reset)
{
    int                        err = 0;
    struct sx_priv            *priv = sx_priv(dev);
    struct ku_access_mcion_reg mcion_reg_data;
    bool                       release_lock = false;
    bool                       is_independent = false;

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        goto out;
    }
    if (is_independent == false) {
        sxd_log_err("%s: Failed hw reset change: module is dependent (FW control).\n", __func__);
        err = -EFAULT;
        goto out;
    }

    memset(&mcion_reg_data, 0, sizeof(mcion_reg_data));
    mcion_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mcion_reg_data.op_tlv, MLXSW_MCION_ID, EMAD_METHOD_WRITE);

    mcion_reg_data.mcion_reg.slot_index = slot_id;
    mcion_reg_data.mcion_reg.module = module_id;
    mcion_reg_data.mcion_reg.module_inputs_mask = MCION_INPUT_BITS_MASK_HW_RESET;
    mcion_reg_data.mcion_reg.module_inputs = hw_reset;


    mutex_lock(&priv->module_access_mutex);
    release_lock = true;

    err = sx_ACCESS_REG_MCION(dev, &mcion_reg_data);
    if (err) {
        sxd_log_err("Failed to write MCION, err=%d\n", err);
        goto out;
    }

    if (mcion_reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to write MCION, status=%d\n", mcion_reg_data.op_tlv.status);
        goto out;
    }


out:
    if (release_lock) {
        mutex_unlock(&priv->module_access_mutex);
    }
    return err;
}


static ssize_t store_module_hw_reset(struct kobject        *kobj,
                                     struct kobj_attribute *attr,
                                     const char            *buf,
                                     size_t                 len)
{
    int            ret = 0;
    int            hw_reset = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;
    bool           hold = false;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, buf, len);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = kstrtoint(buf, 10, &hw_reset);
    if (ret) {
        sxd_log_err("sysfs entry hw_reset got invalid value\n");
        ret = -EINVAL;
        goto out_err;
    }

    if (!kobj->parent) {
        sxd_log_err("Parent of hw_reset sysfs node is NULL\n");
        ret = -EINVAL;
        goto out_err;
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    if ((hw_reset != 0) && (hw_reset != 1)) {
        sxd_log_err("%s: Invalid hw_reset value (%d) failed  for %s\n",
                    __func__,
                    hw_reset,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = __sx_core_set_module_hw_reset(dev, slot_id, module_id, hw_reset);
    if (ret) {
        sxd_log_err("%s: __sx_core_set_module_hw_reset failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Write hw_reset %d", module_id, hw_reset);
    sx_core_sysfs_dev_release(dev);

    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log, "Module ID %d | Write hw_reset failed status %d", module_id, ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}


static int __sx_core_get_module_low_power_mode(struct sx_dev *dev,
                                               uint8_t        slot_id,
                                               uint8_t        module_id,
                                               bool          *is_low_power_mode)
{
    int                        err = 0;
    struct sx_priv            *priv = sx_priv(dev);
    struct ku_access_mcion_reg mcion_reg_data;
    bool                       release_lock = false;
    bool                       is_independent = false;

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        goto out;
    }
    if (is_independent == false) {
        sxd_log_err("%s: Failed control change: module is dependent (FW control).\n", __func__);
        err = -EFAULT;
        goto out;
    }

    memset(&mcion_reg_data, 0, sizeof(mcion_reg_data));
    mcion_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mcion_reg_data.op_tlv, MLXSW_MCION_ID, EMAD_METHOD_QUERY);

    mcion_reg_data.mcion_reg.slot_index = slot_id;
    mcion_reg_data.mcion_reg.module = module_id;


    mutex_lock(&priv->module_access_mutex);
    release_lock = true;

    err = sx_ACCESS_REG_MCION(dev, &mcion_reg_data);
    if (err) {
        sxd_log_err("Failed to query MCION, err=%d\n", err);
        goto out;
    }

    if (mcion_reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to query MCION, status=%d\n", mcion_reg_data.op_tlv.status);
        goto out;
    }

    *is_low_power_mode = mcion_reg_data.mcion_reg.module_inputs & 0x2;

out:
    if (release_lock) {
        mutex_unlock(&priv->module_access_mutex);
    }
    return err;
}

static ssize_t show_module_low_power_mode(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int            ret = 0;
    bool           is_low_power_mode = false;
    int            len = 0;
    int            val = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;
    bool           hold = false;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    ret = __sx_core_get_module_low_power_mode(dev, slot_id, module_id, &is_low_power_mode);
    if (ret) {
        sxd_log_err("%s: __sx_core_get_module_low_power_mode failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    if (is_low_power_mode) {
        val = 1;
    }

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Read low_power_mode %d", module_id, is_low_power_mode);
    sx_core_sysfs_dev_release(dev);

    len = sprintf(buf, "%d\n", val);
    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log,
                         "Module ID %d | Read low_power_mode failed status %d",
                         module_id,
                         ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}


static int __sx_core_set_module_low_power_mode(struct sx_dev *dev,
                                               uint8_t        slot_id,
                                               uint8_t        module_id,
                                               int            low_power_mode)
{
    int                        err = 0;
    struct sx_priv            *priv = sx_priv(dev);
    struct ku_access_mcion_reg mcion_reg_data;
    bool                       release_lock = false;
    bool                       is_independent = false;

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        goto out;
    }
    if (is_independent == false) {
        sxd_log_err("%s: Failed low power mode change: module is dependent (FW control).\n", __func__);
        err = -EFAULT;
        goto out;
    }

    memset(&mcion_reg_data, 0, sizeof(mcion_reg_data));
    mcion_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mcion_reg_data.op_tlv, MLXSW_MCION_ID, EMAD_METHOD_WRITE);

    mcion_reg_data.mcion_reg.slot_index = slot_id;
    mcion_reg_data.mcion_reg.module = module_id;
    mcion_reg_data.mcion_reg.module_inputs_mask = MCION_INPUT_BITS_MASK_LOW_POWER;
    mcion_reg_data.mcion_reg.module_inputs = low_power_mode << 1;


    mutex_lock(&priv->module_access_mutex);
    release_lock = true;

    err = sx_ACCESS_REG_MCION(dev, &mcion_reg_data);
    if (err) {
        sxd_log_err("Failed to write MCION, err=%d\n", err);
        goto out;
    }

    if (mcion_reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to write MCION, status=%d\n", mcion_reg_data.op_tlv.status);
        goto out;
    }


out:
    if (release_lock) {
        mutex_unlock(&priv->module_access_mutex);
    }
    return err;
}


static ssize_t store_module_low_power_mode(struct kobject        *kobj,
                                           struct kobj_attribute *attr,
                                           const char            *buf,
                                           size_t                 len)
{
    int            ret = 0;
    int            low_power_mode = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;
    bool           hold = false;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, buf, len);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = kstrtoint(buf, 10, &low_power_mode);
    if (ret) {
        sxd_log_err("sysfs entry low_power_mode got invalid value\n");
        ret = -EINVAL;
        goto out_err;
    }

    if (!kobj->parent) {
        sxd_log_err("Parent of low_power_mode sysfs node is NULL\n");
        ret = -EINVAL;
        goto out_err;
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    if ((low_power_mode != 0) && (low_power_mode != 1)) {
        sxd_log_err("%s: Invalid low_power_mode value (%d) failed  for %s\n",
                    __func__,
                    low_power_mode,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = __sx_core_set_module_low_power_mode(dev, slot_id, module_id, low_power_mode);
    if (ret) {
        sxd_log_err("%s: __sx_core_set_module_hw_reset failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Write low_power_mode %d", module_id, low_power_mode);
    sx_core_sysfs_dev_release(dev);

    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log, "Module ID %d | Write low_power_mode failed status %d", module_id,
                         ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}


static int __sx_core_get_module_hw_present(struct sx_dev *dev, uint8_t slot_id, uint8_t module_id, bool *is_hw_present)
{
    int                        err = 0;
    struct sx_priv            *priv = sx_priv(dev);
    struct ku_access_mcion_reg mcion_reg_data;
    bool                       release_lock = false;
    bool                       is_independent = false;

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        goto out;
    }
    if (is_independent == false) {
        sxd_log_err("%s: Failed control change: module is dependent (FW control).\n", __func__);
        err = -EFAULT;
        goto out;
    }

    memset(&mcion_reg_data, 0, sizeof(mcion_reg_data));
    mcion_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mcion_reg_data.op_tlv, MLXSW_MCION_ID, EMAD_METHOD_QUERY);

    mcion_reg_data.mcion_reg.slot_index = slot_id;
    mcion_reg_data.mcion_reg.module = module_id;


    mutex_lock(&priv->module_access_mutex);
    release_lock = true;

    err = sx_ACCESS_REG_MCION(dev, &mcion_reg_data);
    if (err) {
        sxd_log_err("Failed to query MCION, err=%d\n", err);
        goto out;
    }

    if (mcion_reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to query MCION, status=%d\n", mcion_reg_data.op_tlv.status);
        goto out;
    }
    if (mcion_reg_data.mcion_reg.module_status_bits_valid & MCION_STATUS_MASK_HW_PRESENT) {
        err = -EINVAL;
        sxd_log_err("MCION hw_present status is not valid, err=%d\n", err);
        goto out;
    }
    *is_hw_present = mcion_reg_data.mcion_reg.module_status_bits & MCION_STATUS_MASK_HW_PRESENT;

    /*
     * To suppress false positive first event on hw_present sysfs, first sysfs query
     * will sync internal DB with the initial queried info.
     */
    if (priv->module_data[slot_id][module_id].independent_params.hw_present == SX_MODULE_HW_PRESENT_INVALID) {
        if (*is_hw_present) {
            priv->module_data[slot_id][module_id].independent_params.hw_present = SX_MODULE_HW_PRESENT_ON;
        } else {
            priv->module_data[slot_id][module_id].independent_params.hw_present = SX_MODULE_HW_PRESENT_OFF;
        }
    }

out:
    if (release_lock) {
        mutex_unlock(&priv->module_access_mutex);
    }
    return err;
}

static ssize_t show_module_hw_present(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int            ret = 0;
    bool           is_hw_present = false;
    int            len = 0;
    int            val = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;
    bool           hold = false;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    ret = __sx_core_get_module_hw_present(dev, slot_id, module_id, &is_hw_present);
    if (ret) {
        sxd_log_err("%s: __sx_core_get_module_hw_present failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    if (is_hw_present) {
        val = 1;
    }

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Read hw_present %d", module_id, is_hw_present);
    sx_core_sysfs_dev_release(dev);

    len = sprintf(buf, "%d\n", val);
    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log, "Module ID %d | Read hw_present failed status %d", module_id, ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}


static int __sx_core_get_module_interrupt(struct sx_dev *dev, uint8_t slot_id, uint8_t module_id, bool *is_interrupt)
{
    int                        err = 0;
    struct sx_priv            *priv = sx_priv(dev);
    struct ku_access_mcion_reg mcion_reg_data;
    bool                       release_lock = false;
    bool                       is_independent = false;

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        goto out;
    }
    if (is_independent == false) {
        sxd_log_err("%s: Failed control change: module is dependent (FW control).\n", __func__);
        err = -EFAULT;
        goto out;
    }

    memset(&mcion_reg_data, 0, sizeof(mcion_reg_data));
    mcion_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mcion_reg_data.op_tlv, MLXSW_MCION_ID, EMAD_METHOD_QUERY);

    mcion_reg_data.mcion_reg.slot_index = slot_id;
    mcion_reg_data.mcion_reg.module = module_id;


    mutex_lock(&priv->module_access_mutex);
    release_lock = true;

    err = sx_ACCESS_REG_MCION(dev, &mcion_reg_data);
    if (err) {
        sxd_log_err("Failed to query MCION, err=%d\n", err);
        goto out;
    }

    if (mcion_reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to query MCION, status=%d\n", mcion_reg_data.op_tlv.status);
        goto out;
    }
    if (mcion_reg_data.mcion_reg.module_status_bits_valid & MCION_STATUS_MASK_INTERRUPT) {
        err = -EINVAL;
        sxd_log_err("MCION interrupt status is not valid, err=%d\n", err);
        goto out;
    }
    *is_interrupt = mcion_reg_data.mcion_reg.module_status_bits & MCION_STATUS_MASK_INTERRUPT;

    /*
     * To suppress false positive first event on interrupt sysfs, first sysfs query
     * will sync internal DB with the initial queried info.
     */
    if (priv->module_data[slot_id][module_id].independent_params.interrupt == SX_MODULE_INTERRUPT_INVALID) {
        if (*is_interrupt) {
            priv->module_data[slot_id][module_id].independent_params.interrupt = SX_MODULE_INTERRUPT_ON;
        } else {
            priv->module_data[slot_id][module_id].independent_params.interrupt = SX_MODULE_INTERRUPT_OFF;
        }
    }

out:
    if (release_lock) {
        mutex_unlock(&priv->module_access_mutex);
    }
    return err;
}

static ssize_t show_module_interrupt(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int            ret = 0;
    bool           is_interrupt = false;
    int            len = 0;
    int            val = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;
    bool           hold = false;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    ret = __sx_core_get_module_interrupt(dev, slot_id, module_id, &is_interrupt);
    if (ret) {
        sxd_log_err("%s: __sx_core_get_module_interrupt failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    if (is_interrupt) {
        val = 1;
    }

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Read interrupt %d", module_id, is_interrupt);
    sx_core_sysfs_dev_release(dev);

    len = sprintf(buf, "%d\n", val);
    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log, "Module ID %d | Read interrupt failed status %d", module_id, ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}


static int __sx_core_get_module_power_good(struct sx_dev *dev, uint8_t slot_id, uint8_t module_id, bool *is_power_good)
{
    int                        err = 0;
    struct sx_priv            *priv = sx_priv(dev);
    struct ku_access_mcion_reg mcion_reg_data;
    bool                       release_lock = false;
    bool                       is_independent = false;

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        goto out;
    }
    if (is_independent == false) {
        sxd_log_err("%s: Failed control change: module is dependent (FW control).\n", __func__);
        err = -EFAULT;
        goto out;
    }

    memset(&mcion_reg_data, 0, sizeof(mcion_reg_data));
    mcion_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mcion_reg_data.op_tlv, MLXSW_MCION_ID, EMAD_METHOD_QUERY);

    mcion_reg_data.mcion_reg.slot_index = slot_id;
    mcion_reg_data.mcion_reg.module = module_id;


    mutex_lock(&priv->module_access_mutex);
    release_lock = true;

    err = sx_ACCESS_REG_MCION(dev, &mcion_reg_data);
    if (err) {
        sxd_log_err("Failed to query MCION, err=%d\n", err);
        goto out;
    }

    if (mcion_reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to query MCION, status=%d\n", mcion_reg_data.op_tlv.status);
        goto out;
    }
    if (mcion_reg_data.mcion_reg.module_status_bits_valid & MCION_STATUS_MASK_POWER_GOOD) {
        err = -EINVAL;
        sxd_log_err("MCION power_good status is not valid, err=%d\n", err);
        goto out;
    }
    *is_power_good = mcion_reg_data.mcion_reg.module_status_bits & MCION_STATUS_MASK_POWER_GOOD;

    /*
     * To suppress false positive first event on power_good sysfs, first sysfs query
     * will sync internal DB with the initial queried info.
     */
    if (priv->module_data[slot_id][module_id].independent_params.power_good == SX_MODULE_POWER_GOOD_INVALID) {
        if (*is_power_good) {
            priv->module_data[slot_id][module_id].independent_params.power_good = SX_MODULE_POWER_GOOD_ON;
        } else {
            priv->module_data[slot_id][module_id].independent_params.power_good = SX_MODULE_POWER_GOOD_OFF;
        }
    }

out:
    if (release_lock) {
        mutex_unlock(&priv->module_access_mutex);
    }
    return err;
}

static ssize_t show_module_power_good(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int            ret = 0;
    bool           is_power_good = false;
    int            len = 0;
    int            val = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;
    bool           hold = false;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    ret = __sx_core_get_module_power_good(dev, slot_id, module_id, &is_power_good);
    if (ret) {
        sxd_log_err("%s: __sx_core_get_module_power_good failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    if (is_power_good) {
        val = 1;
    }

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Read power_good %d", module_id, is_power_good);
    sx_core_sysfs_dev_release(dev);

    len = sprintf(buf, "%d\n", val);
    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log, "Module ID %d | Read power_good failed status %d", module_id, ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}


static int __sx_core_get_module_power_limit(struct sx_dev *dev, uint8_t slot_id, uint8_t module_id, int *power_limit_p)
{
    int                       err = 0;
    struct sx_priv           *priv = sx_priv(dev);
    struct ku_access_mini_reg mini_reg_data;
    bool                      release_lock = false;
    bool                      is_independent = false;

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        goto out;
    }
    if (is_independent == false) {
        sxd_log_err("%s: Failed power_limit read: module is dependent (FW control).\n", __func__);
        err = -EFAULT;
        goto out;
    }

    memset(&mini_reg_data, 0, sizeof(mini_reg_data));
    mini_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mini_reg_data.op_tlv, MLXSW_MINI_ID, EMAD_METHOD_QUERY);

    mini_reg_data.mini_reg.type = SXD_MINI_TYPE_MODULE_POWER_LEVEL_E;
    mini_reg_data.mini_reg.index = module_id;

    mutex_lock(&priv->module_access_mutex);
    release_lock = true;

    err = sx_ACCESS_REG_MINI(dev, &mini_reg_data);
    if (err) {
        sxd_log_err("Failed to query MINI, err=%d\n", err);
        goto out;
    }

    if (mini_reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to query MINI, status=%d\n", mini_reg_data.op_tlv.status);
        goto out;
    }

    *power_limit_p = mini_reg_data.mini_reg.data.mini_module_power_level.spl;

out:
    if (release_lock) {
        mutex_unlock(&priv->module_access_mutex);
    }
    return err;
}

static ssize_t show_module_power_limit(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int            ret = 0;
    int            power_limit = 0;
    int            len = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;
    bool           hold = false;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    ret = __sx_core_get_module_power_limit(dev, slot_id, module_id, &power_limit);
    if (ret) {
        sxd_log_err("%s: __sx_core_get_module_power_limit failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Read power_limit %d", module_id, power_limit);
    sx_core_sysfs_dev_release(dev);

    len = sprintf(buf, "%d\n", power_limit);
    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log, "Module ID %d | Read power_limit failed status %d", module_id,
                         ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}

static int __sx_core_reinsert_module(struct sx_dev *dev, uint8_t slot_id, uint8_t module_id, int reinsert)
{
    int             err = 0;
    bool            is_independent = false;
    struct sx_priv *priv = sx_priv(dev);
    bool            any_port_up = false;
    unsigned long   flags;

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        goto out;
    }
    if (is_independent) {
        sxd_log_err("%s: Failed reinsert change: module is independent (SW control).\n", __func__);
        err = -EFAULT;
        goto out;
    }

    /* Invalid operation if any port is admin up */
    spin_lock_irqsave(&priv->db_lock, flags);
    any_port_up = __module_any_port_up_get(priv, slot_id, module_id);
    if (any_port_up) {
        sxd_log_err("%s: Invalid operation if any port is admin up.\n", __func__);
        spin_unlock_irqrestore(&priv->db_lock, flags);
        err = -EACCES;
        goto out;
    }
    spin_unlock_irqrestore(&priv->db_lock, flags);

    /*
     * Change module control via MMCR
     * Actual module ownership will change after pmaos.admin down & pmaos.admin up */
    err = __module_control_set(dev, slot_id, module_id, true);
    if (err) {
        goto out;
    }

    err = sx_core_set_module(dev, slot_id, module_id, 0, 0);
    if (err) {
        sxd_log_err("Failed to disable module %d.\n", module_id);
        goto out;
    }

    msleep(reinsert);

    err = sx_core_set_module(dev, slot_id, module_id, 1, 0);
    if (err) {
        sxd_log_err("Failed to enable module %d.\n", module_id);
        goto out;
    }

out:
    return err;
}

static ssize_t store_module_reinsert(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len)
{
    int            ret = 0;
    int            reinsert = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;
    bool           hold = false;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, buf, len);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = kstrtoint(buf, 10, &reinsert);
    if (ret) {
        sxd_log_info("sysfs entry reinsert got invalid value\n");
        goto out_err;
    }

    if ((reinsert < 0) || (reinsert > 10000)) {
        ret = -EINVAL;
        sxd_log_info("sysfs entry reinsert got invalid value (%d)\n", reinsert);
        goto out_err;
    }

    if (!kobj->parent) {
        sxd_log_err("Parent of reinsert sysfs node is NULL\n");
        ret = -EINVAL;
        goto out_err;
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n", __func__, ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    ret = __sx_core_reinsert_module(dev, slot_id, module_id, reinsert);
    if (ret) {
        sxd_log_err("Failed in __sx_core_reinsert_module\n");
        goto out_err;
    }

    sx_int_log_info(&sx_priv(dev)->module_log,
                    "Module ID %d | Reinsert successfully, sleep time %d",
                    module_id,
                    reinsert);
    sx_core_sysfs_dev_release(dev);
    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log, "Module ID %d | Reinsert module failed, status %d", module_id,
                         ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}

static int __sx_core_set_module_frequency(struct sx_dev *dev, uint8_t slot_id, uint8_t module_id, int frequency)
{
    int                        err = 0;
    struct sx_priv            *priv = sx_priv(dev);
    unsigned long              flags;
    struct  ku_access_mcfs_reg mcfs_reg_data;
    bool                       release_lock = false;
    bool                       is_independent = false;

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        goto out;
    }
    if (is_independent == false) {
        sxd_log_err("%s: Frequency write is supported when module is in FW control.", __func__);
        err = -EFAULT;
        goto out;
    }
    spin_lock_irqsave(&priv->db_lock, flags);
    if (priv->independent_module_params.frequency_support == false) {
        sxd_log_err("%s: Frequency write is not supported.", __func__);
        err = -EFAULT;
        spin_unlock_irqrestore(&priv->db_lock, flags);
        goto out;
    }
    spin_unlock_irqrestore(&priv->db_lock, flags);

    memset(&mcfs_reg_data, 0, sizeof(mcfs_reg_data));
    mcfs_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mcfs_reg_data.op_tlv, MLXSW_MCFS_ID, EMAD_METHOD_WRITE);

    mcfs_reg_data.mcfs_reg.slot_index = slot_id;
    mcfs_reg_data.mcfs_reg.module = module_id;
    mcfs_reg_data.mcfs_reg.freq = frequency;


    mutex_lock(&priv->module_access_mutex);
    release_lock = true;

    err = sx_ACCESS_REG_MCFS(dev, &mcfs_reg_data);
    if (err) {
        sxd_log_err("Failed to write MCFS, err=%d\n", err);
        goto out;
    }

    if (mcfs_reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to write MCFS, status=%d\n", mcfs_reg_data.op_tlv.status);
        goto out;
    }


out:
    if (release_lock) {
        mutex_unlock(&priv->module_access_mutex);
    }
    return err;
}

static ssize_t store_module_frequency(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len)
{
    int            ret = 0;
    int            frequency = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;
    bool           hold = false;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, buf, len);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = kstrtoint(buf, 10, &frequency);
    if (ret) {
        sxd_log_info("sysfs entry frequency got invalid value\n");
        goto out_err;
    }

    if ((frequency < SXD_MFCS_FREQ_MIN) || (frequency > SXD_MFCS_FREQ_MAX)) {
        ret = -EINVAL;
        sxd_log_info("sysfs entry frequency is out of range (%d)\n", frequency);
        goto out_err;
    }

    if (!kobj->parent) {
        sxd_log_err("Parent of frequency sysfs node is NULL\n");
        ret = -EINVAL;
        goto out_err;
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n", __func__, ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    ret = __sx_core_set_module_frequency(dev, slot_id, module_id, frequency);
    if (ret) {
        sxd_log_err("Failed in __sx_core_set_module_frequency\n");
        goto out_err;
    }

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Write module frequency %d", module_id, frequency);
    sx_core_sysfs_dev_release(dev);
    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log,
                         "Module ID %d | Write module frequency failed, status %d",
                         module_id,
                         ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}


static int __sx_core_get_module_frequency(struct sx_dev *dev, uint8_t slot_id, uint8_t module_id, int *frequency_p)
{
    int                       err = 0;
    struct sx_priv           *priv = sx_priv(dev);
    struct ku_access_mcfs_reg mcfs_reg_data;
    bool                      release_lock = false;
    unsigned long             flags;
    bool                      is_independent = false;

    err = sx_core_get_module_control(dev, slot_id, module_id, &is_independent);
    if (err) {
        goto out;
    }
    if (is_independent == false) {
        sxd_log_err("%s: Failed frequency read: module is dependent (FW control).\n", __func__);
        err = -EFAULT;
        goto out;
    }
    spin_lock_irqsave(&priv->db_lock, flags);
    if (priv->independent_module_params.frequency_support == false) {
        sxd_log_err("%s: Frequency read is not supported.", __func__);
        err = -EFAULT;
        spin_unlock_irqrestore(&priv->db_lock, flags);
        goto out;
    }
    spin_unlock_irqrestore(&priv->db_lock, flags);

    memset(&mcfs_reg_data, 0, sizeof(mcfs_reg_data));
    mcfs_reg_data.dev_id = dev->device_id;
    sx_cmd_set_op_tlv(&mcfs_reg_data.op_tlv, MLXSW_MCFS_ID, EMAD_METHOD_QUERY);

    mcfs_reg_data.mcfs_reg.module = module_id;
    mcfs_reg_data.mcfs_reg.slot_index = slot_id;

    mutex_lock(&priv->module_access_mutex);
    release_lock = true;

    err = sx_ACCESS_REG_MCFS(dev, &mcfs_reg_data);
    if (err) {
        sxd_log_err("Failed to query MCFS, err=%d\n", err);
        goto out;
    }

    if (mcfs_reg_data.op_tlv.status) {
        err = -EINVAL;
        sxd_log_err("Failed to query MCFS, status=%d\n", mcfs_reg_data.op_tlv.status);
        goto out;
    }

    *frequency_p = mcfs_reg_data.mcfs_reg.freq;

out:
    if (release_lock) {
        mutex_unlock(&priv->module_access_mutex);
    }
    return err;
}

static ssize_t show_module_frequency(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int            ret = 0;
    int            frequency = 0;
    int            len = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;
    bool           hold = false;

    ret = sx_core_send_sniffer_event(kobj, &attr->attr, __FUNCTION__, NULL, 0);
    if (ret) {
        sxd_log_err("sx_core_send_sniffer_event from sysfs entry %s failed [%d]\n", __FUNCTION__, ret);
        ret = 0;
        /*continue */
    }

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    ret = sx_core_sysfs_dev_hold(dev);
    if (ret) {
        goto out_err;
    }

    hold = true;

    ret = __sx_core_get_module_frequency(dev, slot_id, module_id, &frequency);
    if (ret) {
        sxd_log_err("%s: __sx_core_get_module_frequency failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out_err;
    }

    sx_int_log_info(&sx_priv(dev)->module_log, "Module ID %d | Read frequency %d", module_id, frequency);
    sx_core_sysfs_dev_release(dev);

    len = sprintf(buf, "%d\n", frequency);
    return len;

out_err:
    if (hold) {
        sx_int_log_error(&sx_priv(dev)->module_log, "Module ID %d | Read frequency failed status %d", module_id, ret);
        sx_core_sysfs_dev_release(dev);
    }
    return ret;
}

static int __sx_core_get_module_frequency_support(struct sx_dev *dev, int *frequency_support_p)
{
    int             err = 0;
    struct sx_priv *priv = sx_priv(dev);


    *frequency_support_p = priv->independent_module_params.frequency_support;

    return err;
}

static ssize_t show_module_frequency_support(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int            ret = 0;
    int            frequency_support = 0;
    int            len = 0;
    struct sx_dev *dev = NULL;
    uint8_t        slot_id = 0;
    uint8_t        module_id = 0;

    ret = __module_sysfs_get_dev_slot_module(kobj, 0, &dev, &slot_id, &module_id);
    if (ret) {
        sxd_log_err("%s: __module_sysfs_get_dev_slot_module failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out;
    }

    ret = __sx_core_get_module_frequency_support(dev, &frequency_support);
    if (ret) {
        sxd_log_err("%s: __sx_core_get_module_frequency_support failed (%d) for %s\n",
                    __func__,
                    ret,
                    kobject_name(kobj));
        goto out;
    }

    len = sprintf(buf, "%d\n", frequency_support);

    return len;

out:
    return ret;
}

int sx_sysfs_independent_module_db_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_priv *priv = NULL;
    struct sx_dev  *dev = sx_dbg_dump_get_device(m);
    char           *states[] = {"INVALID", "ON", "OFF"};
    char           *power_mode[] = {"N/A", "LOW", "HIGH"};
    char           *power_mode_policy[] = {"N/A", "HIGH", "AUTO", "LOW"};
    char           *present[] = {"INVALID", "PLUGGED", "UNPLUGGED", "PLUGGED_ERROR"};
    int             j, i = 0;

    if (!dev) {
        return 0;
    }

    priv = sx_priv(dev);
    sx_dbg_dump_print_header(m, "Independent module DB dump");

    sx_dbg_dump_print_table_header(m,  " %-8s | %-11s | %-11s | %-11s | %-17s | %-11s | %-13s |",
                                   "Module ID", "hw_present", "interrupt", "power_good", "power_mode_policy",
                                   "power_mode", "present");


    for (j = 0; j < MAX_MODULE_NUM; j++) {
        seq_printf(m, "  %-8d | %-11s | %-11s | %-11s | %-17s | %-11s | %-13s |\n",
                   /*module_id*/ j,
                   states[priv->module_data[i][j].independent_params.hw_present],
                   states[priv->module_data[i][j].independent_params.interrupt],
                   states[priv->module_data[i][j].independent_params.power_good],
                   power_mode_policy[priv->module_data[i][j].power_mode_policy],
                   power_mode[priv->module_data[i][j].power_mode],
                   present[priv->module_data[i][j].present]);
    }


    return 0;
}
