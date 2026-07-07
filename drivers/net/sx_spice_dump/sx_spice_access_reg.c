/*
 * SPDX-FileCopyrightText: Copyright (c) 2021-2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

#include "sx_spice_access_reg.h"
#include "sx_spice_tree_auto.h"
#include "sx_spice_access_reg_auto.h"

/************************************************
 *  Local variables
 ***********************************************/

/************************************************
 *  Type definitions
 ***********************************************/

static LIST_HEAD(__slrg_list_head);
static LIST_HEAD(__slrp_list_head);
static LIST_HEAD(__pmlp_list_head);

/************************************************
 *  Local function declarations
 ***********************************************/

/************************************************
 * Functions                                    *
 ***********************************************/


int sx_spice_access_reg_slrp_reg_data_to_buffer_print(char                 buffer[],
                                                      size_t               buffer_length,
                                                      struct ku_slrp_reg * reg_data)
{
    int      buffer_size = 0, snp_res = 0;
    uint16_t local_port = 0;

    snp_res = snprintf(buffer, buffer_length - buffer_size, "status: 0x%x\n", reg_data->status);
    VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
    buffer += snp_res;
    buffer_size += snp_res;
    snp_res = snprintf(buffer, buffer_length - buffer_size, "version: 0x%x\n", reg_data->version);
    VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
    buffer += snp_res;
    buffer_size += snp_res;
    SX_PORT_BUILD_PHY_ID_FROM_LSB_MSB(local_port, reg_data->local_port, reg_data->lp_msb);
    snp_res = snprintf(buffer, buffer_length - buffer_size, "local_port: 0x%x\n", local_port);
    VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
    buffer += snp_res;
    buffer_size += snp_res;
    snp_res = snprintf(buffer, buffer_length - buffer_size, "pnat: 0x%x\n", reg_data->pnat);
    VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
    buffer += snp_res;
    buffer_size += snp_res;
    snp_res = snprintf(buffer, buffer_length - buffer_size, "lane: 0x%x\n", reg_data->lane);
    VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
    buffer += snp_res;
    buffer_size += snp_res;
    snp_res = snprintf(buffer, buffer_length - buffer_size, "port_type: 0x%x\n", reg_data->port_type);
    VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
    buffer += snp_res;
    buffer_size += snp_res;

    switch (reg_data->port_type) {
    case 0:
    case 2:
        switch (reg_data->version) {
        case 0:
        case 1:
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ib_sel: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.ib_sel);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "dp_sel: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.dp_sel);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "dp90sel: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.dp90sel);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "mix90phase: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.mix90phase);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ffe_tap0: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.ffe_tap0);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ffe_tap1: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.ffe_tap1);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ffe_tap2: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.ffe_tap2);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ffe_tap3: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.ffe_tap3);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ffe_tap4: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.ffe_tap4);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ffe_tap5: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.ffe_tap5);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ffe_tap6: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.ffe_tap6);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ffe_tap7: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.ffe_tap7);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ffe_tap8: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.ffe_tap8);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "mixerbias_tap_amp: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.mixerbias_tap_amp);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ffe_tap_en: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.ffe_tap_en);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ffe_tap_offset0: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.ffe_tap_offset0);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ffe_tap_offset1: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.ffe_tap_offset1);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "slicer_offset0: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.slicer_offset0);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "mixer_offset0: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.mixer_offset0);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "mixer_offset1: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.mixer_offset1);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "mixerbgn_inp: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.mixerbgn_inp);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "mixerbgn_inn: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.mixerbgn_inn);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "mixerbgn_refp: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.mixerbgn_refp);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "mixerbgn_refn: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.mixerbgn_refn);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_slicer_lctrl_h: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.sel_slicer_lctrl_h);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_slicer_lctrl_l: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.sel_slicer_lctrl_l);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ref_mixer_vreg: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.ref_mixer_vreg);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "slicer_gctrl: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.slicer_gctrl);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "lctrl_input: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.lctrl_input);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "mixer_offset_cm1: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.mixer_offset_cm1);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "common_mode: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.common_mode);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "mixer_offset_cm0: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.mixer_offset_cm0);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "slicer_offset_cm: 0x%x\n",
                               reg_data->page_data.slrp_40nm_28nm.slicer_offset_cm);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            break;

        case 3:
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "mixer_offset_up: 0x%x\n",
                               reg_data->page_data.slrp_16nm.mixer_offset_up);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "mixer_offset_down: 0x%x\n",
                               reg_data->page_data.slrp_16nm.mixer_offset_down);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_enc: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_enc);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "dp_sel: 0x%x\n",
                               reg_data->page_data.slrp_16nm.dp_sel);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_ref_in_mixer_biasgen_up: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_ref_in_mixer_biasgen_up);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_ref_in_mixer_biasgen_mid: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_ref_in_mixer_biasgen_mid);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_ref_in_mixer_biasgen_down: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_ref_in_mixer_biasgen_down);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_ref_in_mixer_biasgen_eo: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_ref_in_mixer_biasgen_eo);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_ref_out_mixer_biasgen_up: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_ref_out_mixer_biasgen_up);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_ref_out_mixer_biasgen_mid: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_ref_out_mixer_biasgen_mid);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_ref_out_mixer_biasgen_down: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_ref_out_mixer_biasgen_down);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_ref_out_mixer_biasgen_eo: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_ref_out_mixer_biasgen_eo);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_amp_mixer_biasgen_up: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_amp_mixer_biasgen_up);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_amp_mixer_biasgen_mid: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_amp_mixer_biasgen_mid);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_amp_mixer_biasgen_down: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_amp_mixer_biasgen_down);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_amp_mixer_biasgen_eo: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_amp_mixer_biasgen_eo);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "mixer_offset_eye_scan: 0x%x\n",
                               reg_data->page_data.slrp_16nm.mixer_offset_eye_scan);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "gctrl_bin_bgn0_n: 0x%x\n",
                               reg_data->page_data.slrp_16nm.gctrl_bin_bgn0_n);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "gctrl_bin_bgn0_p: 0x%x\n",
                               reg_data->page_data.slrp_16nm.gctrl_bin_bgn0_p);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "gctrl_bin_bgn1_n: 0x%x\n",
                               reg_data->page_data.slrp_16nm.gctrl_bin_bgn1_n);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "gctrl_bin_bgn1_p: 0x%x\n",
                               reg_data->page_data.slrp_16nm.gctrl_bin_bgn1_p);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_digital_gctrl: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_digital_gctrl);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_bias2ctle: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_bias2ctle);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "selgc_ref_op0_alev_ctle: 0x%x\n",
                               reg_data->page_data.slrp_16nm.selgc_ref_op0_alev_ctle);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "selgc_ref_op1_alev_ctle: 0x%x\n",
                               reg_data->page_data.slrp_16nm.selgc_ref_op1_alev_ctle);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_bias_en_0: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_bias_en_0);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_bias_en_1: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_bias_en_1);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_bias_en_2: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_bias_en_2);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_bias_en_3: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_bias_en_3);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_bias_en_4: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_bias_en_4);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_bias_en_5: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_bias_en_5);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_bias_en_6: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_bias_en_6);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_gctrln_en_0: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_gctrln_en_0);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_gctrln_en_1: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_gctrln_en_1);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_gctrln_en_2: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_gctrln_en_2);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_gctrln_en_3: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_gctrln_en_3);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_gctrln_en_4: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_gctrln_en_4);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_gctrln_en_5: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_gctrln_en_5);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_gctrln_en_6: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_gctrln_en_6);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_gctrlp_en_0: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_gctrlp_en_0);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_gctrlp_en_1: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_gctrlp_en_1);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_gctrlp_en_2: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_gctrlp_en_2);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_gctrlp_en_3: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_gctrlp_en_3);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_gctrlp_en_4: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_gctrlp_en_4);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_gctrlp_en_5: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_gctrlp_en_5);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "sel_gctrlp_en_6: 0x%x\n",
                               reg_data->page_data.slrp_16nm.sel_gctrlp_en_6);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "crnt_bgn_offset_p1: 0x%x\n",
                               reg_data->page_data.slrp_16nm.crnt_bgn_offset_p1);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "crnt_bgn_offset_n1: 0x%x\n",
                               reg_data->page_data.slrp_16nm.crnt_bgn_offset_n1);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "crnt_bgn_offset_p0: 0x%x\n",
                               reg_data->page_data.slrp_16nm.crnt_bgn_offset_p0);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "crnt_bgn_offset_n0: 0x%x\n",
                               reg_data->page_data.slrp_16nm.crnt_bgn_offset_n0);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            break;

        case 4:
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "adc_recording_admin: 0x%x\n",
                               reg_data->page_data.slrp_7nm.adc_recording_admin);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "adc_recording_status: 0x%x\n",
                               reg_data->page_data.slrp_7nm.adc_recording_status);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "adc_rocording_lanes: 0x%x\n",
                               reg_data->page_data.slrp_7nm.adc_rocording_lanes);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "edge_vos_ccal_en: 0x%x\n",
                               reg_data->page_data.slrp_7nm.edge_vos_ccal_en);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "adc_gain_shift_auto: 0x%x\n",
                               reg_data->page_data.slrp_7nm.adc_gain_shift_auto);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ccal_mode: 0x%x\n",
                               reg_data->page_data.slrp_7nm.ccal_mode);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ctle_override_ctrl: 0x%x\n",
                               reg_data->page_data.slrp_7nm.ctle_override_ctrl);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "vga_override_ctrl: 0x%x\n",
                               reg_data->page_data.slrp_7nm.vga_override_ctrl);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "adc_vos_override_ctrl: 0x%x\n",
                               reg_data->page_data.slrp_7nm.adc_vos_override_ctrl);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "adc_gain_override_ctrl: 0x%x\n",
                               reg_data->page_data.slrp_7nm.adc_gain_override_ctrl);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "phos_override_ctrl: 0x%x\n",
                               reg_data->page_data.slrp_7nm.phos_override_ctrl);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "cal_error_cnt: 0x%x\n",
                               reg_data->page_data.slrp_7nm.cal_error_cnt);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ccal_state: 0x%x\n",
                               reg_data->page_data.slrp_7nm.ccal_state);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ccal_op: 0x%x\n",
                               reg_data->page_data.slrp_7nm.ccal_op);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res =
                snprintf(buffer,
                         buffer_length - buffer_size,
                         "phos: 0x%x\n",
                         reg_data->page_data.slrp_7nm.phos);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "vga_vos1: 0x%x\n",
                               reg_data->page_data.slrp_7nm.vga_vos1);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "vga_vos0: 0x%x\n",
                               reg_data->page_data.slrp_7nm.vga_vos0);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "ctle_vos0: 0x%x\n",
                               reg_data->page_data.slrp_7nm.ctle_vos0);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "adc_vos: 0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
                               reg_data->page_data.slrp_7nm.adc_vos[0],
                               reg_data->page_data.slrp_7nm.adc_vos[1],
                               reg_data->page_data.slrp_7nm.adc_vos[2],
                               reg_data->page_data.slrp_7nm.adc_vos[3],
                               reg_data->page_data.slrp_7nm.adc_vos[4],
                               reg_data->page_data.slrp_7nm.adc_vos[5],
                               reg_data->page_data.slrp_7nm.adc_vos[6],
                               reg_data->page_data.slrp_7nm.adc_vos[7],
                               reg_data->page_data.slrp_7nm.adc_vos[8],
                               reg_data->page_data.slrp_7nm.adc_vos[9],
                               reg_data->page_data.slrp_7nm.adc_vos[10],
                               reg_data->page_data.slrp_7nm.adc_vos[11],
                               reg_data->page_data.slrp_7nm.adc_vos[12],
                               reg_data->page_data.slrp_7nm.adc_vos[13],
                               reg_data->page_data.slrp_7nm.adc_vos[14],
                               reg_data->page_data.slrp_7nm.adc_vos[15],
                               reg_data->page_data.slrp_7nm.adc_vos[16],
                               reg_data->page_data.slrp_7nm.adc_vos[17],
                               reg_data->page_data.slrp_7nm.adc_vos[18],
                               reg_data->page_data.slrp_7nm.adc_vos[19],
                               reg_data->page_data.slrp_7nm.adc_vos[20],
                               reg_data->page_data.slrp_7nm.adc_vos[21],
                               reg_data->page_data.slrp_7nm.adc_vos[22],
                               reg_data->page_data.slrp_7nm.adc_vos[23],
                               reg_data->page_data.slrp_7nm.adc_vos[24],
                               reg_data->page_data.slrp_7nm.adc_vos[25],
                               reg_data->page_data.slrp_7nm.adc_vos[26],
                               reg_data->page_data.slrp_7nm.adc_vos[27],
                               reg_data->page_data.slrp_7nm.adc_vos[28],
                               reg_data->page_data.slrp_7nm.adc_vos[29],
                               reg_data->page_data.slrp_7nm.adc_vos[30],
                               reg_data->page_data.slrp_7nm.adc_vos[31],
                               reg_data->page_data.slrp_7nm.adc_vos[32],
                               reg_data->page_data.slrp_7nm.adc_vos[33],
                               reg_data->page_data.slrp_7nm.adc_vos[34],
                               reg_data->page_data.slrp_7nm.adc_vos[35],
                               reg_data->page_data.slrp_7nm.adc_vos[36],
                               reg_data->page_data.slrp_7nm.adc_vos[37],
                               reg_data->page_data.slrp_7nm.adc_vos[38],
                               reg_data->page_data.slrp_7nm.adc_vos[39],
                               reg_data->page_data.slrp_7nm.adc_vos[40],
                               reg_data->page_data.slrp_7nm.adc_vos[41],
                               reg_data->page_data.slrp_7nm.adc_vos[42],
                               reg_data->page_data.slrp_7nm.adc_vos[43],
                               reg_data->page_data.slrp_7nm.adc_vos[44],
                               reg_data->page_data.slrp_7nm.adc_vos[45],
                               reg_data->page_data.slrp_7nm.adc_vos[46],
                               reg_data->page_data.slrp_7nm.adc_vos[47],
                               reg_data->page_data.slrp_7nm.adc_vos[48],
                               reg_data->page_data.slrp_7nm.adc_vos[49],
                               reg_data->page_data.slrp_7nm.adc_vos[50],
                               reg_data->page_data.slrp_7nm.adc_vos[51],
                               reg_data->page_data.slrp_7nm.adc_vos[52],
                               reg_data->page_data.slrp_7nm.adc_vos[53],
                               reg_data->page_data.slrp_7nm.adc_vos[54],
                               reg_data->page_data.slrp_7nm.adc_vos[55],
                               reg_data->page_data.slrp_7nm.adc_vos[56],
                               reg_data->page_data.slrp_7nm.adc_vos[57],
                               reg_data->page_data.slrp_7nm.adc_vos[58],
                               reg_data->page_data.slrp_7nm.adc_vos[59],
                               reg_data->page_data.slrp_7nm.adc_vos[60],
                               reg_data->page_data.slrp_7nm.adc_vos[61],
                               reg_data->page_data.slrp_7nm.adc_vos[62],
                               reg_data->page_data.slrp_7nm.adc_vos[63]);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "adc_gos: 0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
                               reg_data->page_data.slrp_7nm.adc_gos[0],
                               reg_data->page_data.slrp_7nm.adc_gos[1],
                               reg_data->page_data.slrp_7nm.adc_gos[2],
                               reg_data->page_data.slrp_7nm.adc_gos[3],
                               reg_data->page_data.slrp_7nm.adc_gos[4],
                               reg_data->page_data.slrp_7nm.adc_gos[5],
                               reg_data->page_data.slrp_7nm.adc_gos[6],
                               reg_data->page_data.slrp_7nm.adc_gos[7],
                               reg_data->page_data.slrp_7nm.adc_gos[8],
                               reg_data->page_data.slrp_7nm.adc_gos[9],
                               reg_data->page_data.slrp_7nm.adc_gos[10],
                               reg_data->page_data.slrp_7nm.adc_gos[11],
                               reg_data->page_data.slrp_7nm.adc_gos[12],
                               reg_data->page_data.slrp_7nm.adc_gos[13],
                               reg_data->page_data.slrp_7nm.adc_gos[14],
                               reg_data->page_data.slrp_7nm.adc_gos[15],
                               reg_data->page_data.slrp_7nm.adc_gos[16],
                               reg_data->page_data.slrp_7nm.adc_gos[17],
                               reg_data->page_data.slrp_7nm.adc_gos[18],
                               reg_data->page_data.slrp_7nm.adc_gos[19],
                               reg_data->page_data.slrp_7nm.adc_gos[20],
                               reg_data->page_data.slrp_7nm.adc_gos[21],
                               reg_data->page_data.slrp_7nm.adc_gos[22],
                               reg_data->page_data.slrp_7nm.adc_gos[23],
                               reg_data->page_data.slrp_7nm.adc_gos[24],
                               reg_data->page_data.slrp_7nm.adc_gos[25],
                               reg_data->page_data.slrp_7nm.adc_gos[26],
                               reg_data->page_data.slrp_7nm.adc_gos[27],
                               reg_data->page_data.slrp_7nm.adc_gos[28],
                               reg_data->page_data.slrp_7nm.adc_gos[29],
                               reg_data->page_data.slrp_7nm.adc_gos[30],
                               reg_data->page_data.slrp_7nm.adc_gos[31],
                               reg_data->page_data.slrp_7nm.adc_gos[32],
                               reg_data->page_data.slrp_7nm.adc_gos[33],
                               reg_data->page_data.slrp_7nm.adc_gos[34],
                               reg_data->page_data.slrp_7nm.adc_gos[35],
                               reg_data->page_data.slrp_7nm.adc_gos[36],
                               reg_data->page_data.slrp_7nm.adc_gos[37],
                               reg_data->page_data.slrp_7nm.adc_gos[38],
                               reg_data->page_data.slrp_7nm.adc_gos[39],
                               reg_data->page_data.slrp_7nm.adc_gos[40],
                               reg_data->page_data.slrp_7nm.adc_gos[41],
                               reg_data->page_data.slrp_7nm.adc_gos[42],
                               reg_data->page_data.slrp_7nm.adc_gos[43],
                               reg_data->page_data.slrp_7nm.adc_gos[44],
                               reg_data->page_data.slrp_7nm.adc_gos[45],
                               reg_data->page_data.slrp_7nm.adc_gos[46],
                               reg_data->page_data.slrp_7nm.adc_gos[47],
                               reg_data->page_data.slrp_7nm.adc_gos[48],
                               reg_data->page_data.slrp_7nm.adc_gos[49],
                               reg_data->page_data.slrp_7nm.adc_gos[50],
                               reg_data->page_data.slrp_7nm.adc_gos[51],
                               reg_data->page_data.slrp_7nm.adc_gos[52],
                               reg_data->page_data.slrp_7nm.adc_gos[53],
                               reg_data->page_data.slrp_7nm.adc_gos[54],
                               reg_data->page_data.slrp_7nm.adc_gos[55],
                               reg_data->page_data.slrp_7nm.adc_gos[56],
                               reg_data->page_data.slrp_7nm.adc_gos[57],
                               reg_data->page_data.slrp_7nm.adc_gos[58],
                               reg_data->page_data.slrp_7nm.adc_gos[59],
                               reg_data->page_data.slrp_7nm.adc_gos[60],
                               reg_data->page_data.slrp_7nm.adc_gos[61],
                               reg_data->page_data.slrp_7nm.adc_gos[62],
                               reg_data->page_data.slrp_7nm.adc_gos[63]);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "phos3: 0x%x\n",
                               reg_data->page_data.slrp_7nm.phos3);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "phos2: 0x%x\n",
                               reg_data->page_data.slrp_7nm.phos2);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "phos1: 0x%x\n",
                               reg_data->page_data.slrp_7nm.phos1);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "phos0: 0x%x\n",
                               reg_data->page_data.slrp_7nm.phos0);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "phos7: 0x%x\n",
                               reg_data->page_data.slrp_7nm.phos7);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "phos6: 0x%x\n",
                               reg_data->page_data.slrp_7nm.phos6);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "phos5: 0x%x\n",
                               reg_data->page_data.slrp_7nm.phos5);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            snp_res = snprintf(buffer,
                               buffer_length - buffer_size,
                               "phos4: 0x%x\n",
                               reg_data->page_data.slrp_7nm.phos4);
            VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
            buffer += snp_res;
            buffer_size += snp_res;
            break;

        default:
            break;
        }
        break;

    case 1:
    case 3:
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "ffe_tap_index: 0x%x\n",
                           reg_data->page_data.slrp_gearbox.ffe_tap_index);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "ffe_sub_channel: 0x%x\n",
                           reg_data->page_data.slrp_gearbox.ffe_sub_channel);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "snr_en: 0x%x\n",
                           reg_data->page_data.slrp_gearbox.snr_en);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "dsp_mode: 0x%x\n",
                           reg_data->page_data.slrp_gearbox.dsp_mode);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "afe_trim: 0x%x\n",
                           reg_data->page_data.slrp_gearbox.afe_trim);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "ctle: 0x%x\n",
                           reg_data->page_data.slrp_gearbox.ctle);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "raw_snr_value: 0x%x\n",
                           reg_data->page_data.slrp_gearbox.raw_snr_value);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "db_snr_value: 0x%x\n",
                           reg_data->page_data.slrp_gearbox.db_snr_value);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "ffe_tap_value: 0x%x\n",
                           reg_data->page_data.slrp_gearbox.ffe_tap_value);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        break;

    case 4:
    case 5:
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "mixer00: 0x%x\n",
                           reg_data->page_data.slrp_usr.mixer00);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "mixer90: 0x%x\n",
                           reg_data->page_data.slrp_usr.mixer90);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "termination: 0x%x\n",
                           reg_data->page_data.slrp_usr.termination);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "neg_cap_up: 0x%x\n",
                           reg_data->page_data.slrp_usr.neg_cap_up);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "neg_cap_mid: 0x%x\n",
                           reg_data->page_data.slrp_usr.neg_cap_mid);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "neg_cap_dn: 0x%x\n",
                           reg_data->page_data.slrp_usr.neg_cap_dn);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "offset_coarse_polarity_up: 0x%x\n",
                           reg_data->page_data.slrp_usr.offset_coarse_polarity_up);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "offset_coarse_polarity_mid: 0x%x\n",
                           reg_data->page_data.slrp_usr.offset_coarse_polarity_mid);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "offset_coarse_polarity_dn: 0x%x\n",
                           reg_data->page_data.slrp_usr.offset_coarse_polarity_dn);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "offset_fine_polarity_up: 0x%x\n",
                           reg_data->page_data.slrp_usr.offset_fine_polarity_up);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "offset_fine_polarity_mid: 0x%x\n",
                           reg_data->page_data.slrp_usr.offset_fine_polarity_mid);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "offset_fine_polarity_dn: 0x%x\n",
                           reg_data->page_data.slrp_usr.offset_fine_polarity_dn);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "amp_vga: 0x%x\n",
                           reg_data->page_data.slrp_usr.amp_vga);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "amp_vga_last_stage: 0x%x\n",
                           reg_data->page_data.slrp_usr.amp_vga_last_stage);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "offset_coarse_up: 0x%x\n",
                           reg_data->page_data.slrp_usr.offset_coarse_up);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "offset_coarse_mid: 0x%x\n",
                           reg_data->page_data.slrp_usr.offset_coarse_mid);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "offset_coarse_dn: 0x%x\n",
                           reg_data->page_data.slrp_usr.offset_coarse_dn);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "offset_fine_up: 0x%x\n",
                           reg_data->page_data.slrp_usr.offset_fine_up);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "offset_fine_mid: 0x%x\n",
                           reg_data->page_data.slrp_usr.offset_fine_mid);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        snp_res = snprintf(buffer,
                           buffer_length - buffer_size,
                           "offset_fine_dn: 0x%x\n",
                           reg_data->page_data.slrp_usr.offset_fine_dn);
        VALIDATE_ROLLBACK(snp_res, buffer_length, buffer_size);
        buffer += snp_res;
        buffer_size += snp_res;
        break;

    default:
        break;
    }

    return buffer_size;
}

ssize_t sx_spice_access_reg_slrp_read(struct file *filp, char *buf, size_t lbuf, loff_t *ppos)
{
    char              *kbuffer = NULL;
    int                err = 0;
    ssize_t            size = 0;
    ssize_t            print_data_size = 0;
    struct ku_slrp_reg reg_data;
    const slrp_data_t *data = (slrp_data_t*)filp->f_inode->i_private;

    if (*ppos != 0) {
        return 0;
    }

    kbuffer = sx_spice_buffer_alloc(GFP_KERNEL);
    if (!kbuffer) {
        sxd_log_err(SXD_DEV_FROM_DEVDATA_CONT(data),
                    "Failed to allocate kernel buffer for SLRP SPICE file generation\n");
        return 0;
    }

    memset(kbuffer, 0, sizeof(char) * KERNEL_BUFFER_SIZE);
    memset(&reg_data, 0, sizeof(reg_data));

    SX_PORT_EXTRACT_LSB_MSB_FROM_PHY_ID(reg_data.local_port,
                                        reg_data.lp_msb,
                                        data->local_port);
    reg_data.pnat = data->pnat;
    reg_data.lane = data->lane;
    reg_data.port_type = data->port_type;


    err = SX_SPICE_EMAD_ACCESS_REG(SLRP, data->dev_data, &reg_data);
    if (err) {
        print_data_size = snprintf(kbuffer, sizeof(char) * 5, "N/A\n");
        size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);
        goto out;
    }

    print_data_size = sx_spice_access_reg_slrp_reg_data_to_buffer_print(kbuffer,
                                                                        sizeof(char) * KERNEL_BUFFER_SIZE,
                                                                        &reg_data);

    size = simple_read_from_buffer(buf, lbuf, ppos, kbuffer, print_data_size + 1);

out:
    sx_spice_buffer_free(kbuffer);
    return size;
}

static const struct file_operations slrp_fops = {
    .read = sx_spice_access_reg_slrp_read
};

int sx_spice_access_reg_slrp_file_create(struct dentry *parent, dev_private_data_t *dev_data)
{
    slrp_data_t *slrp = NULL;

    slrp = kzalloc(sizeof(slrp_data_t), GFP_KERNEL);
    if (!slrp) {
        sxd_log_err(SXD_CONT_DEV(dev_data), "Failed to allocate slrp for SPICE\n");
        goto out;
    }

    list_add_tail(&slrp->slrp_list, &__slrp_list_head);

    slrp->dev_data = dev_data;
    slrp->local_port = sx_spice_tree_index_value_get(LOCAL_PORT);
    slrp->pnat = sx_spice_tree_index_value_get(PNAT);
    slrp->lane = sx_spice_tree_index_value_get(LANE);
    slrp->port_type = sx_spice_tree_index_value_get(PORT_TYPE);


    debugfs_create_file("SLRP", 0644, parent, (void *)slrp, &slrp_fops);

    return 0;

out:
    return -ENOMEM;
}

void sx_spice_access_reg_slrp_list_free(void)
{
    slrp_data_t *iter_slrp, *tmp_slrp;

    list_for_each_entry_safe(iter_slrp, tmp_slrp, &__slrp_list_head, slrp_list) {
        list_del(&iter_slrp->slrp_list);
        kfree(iter_slrp);
    }
}

void sx_spice_access_reg_slrp_list_dev_data_free(dev_private_data_t *dev_data)
{
    slrp_data_t *iter_slrp, *tmp_slrp;

    list_for_each_entry_safe(iter_slrp, tmp_slrp, &__slrp_list_head, slrp_list) {
        if (dev_data == iter_slrp->dev_data) {
            list_del(&iter_slrp->slrp_list);
            kfree(iter_slrp);
        }
    }
}
