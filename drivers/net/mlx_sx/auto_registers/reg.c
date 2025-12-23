/*
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */


/*
 * THIS FILE IS AUTO GENERATED.
 * DO NOT MAKE ANY CHANGES!
 * They will be erased with next update.
 *
 */

#include <linux/string.h>
#include <linux/mlx_sx/auto_registers/mlxsw_item.h>
#include <linux/mlx_sx/auto_registers/reg.h>

/************************************************
 *  Global variables
 ***********************************************/


/************************************************
 *  Local variables
 ***********************************************/

/************************************************
 *  Local function declarations
 ***********************************************/


/************************************************
 *  Function implementations
 ***********************************************/
/* hgcr
 * ----
 *
 */
/* truncation_size
 * Access: RW
 */
MLXSW_ITEM16(reg, hgcr, truncation_size, 0x2, 0, 14);

/* regtlve
 * Access: RW
 */
MLXSW_ITEM8(reg, hgcr, regtlve, 0x7, 0, 1);

/* rcv_fcs_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, hgcr, rcv_fcs_mode, 0xb, 0, 2);

/* rcv_fcs_value
 * Access: RW
 */
MLXSW_ITEM32(reg, hgcr, rcv_fcs_value, 0xc, 0, 32);

/* truncation_size_prof1
 * Access: RW
 */
MLXSW_ITEM16(reg, hgcr, truncation_size_prof1, 0x12, 0, 14);

/* truncation_size_prof2
 * Access: RW
 */
MLXSW_ITEM16(reg, hgcr, truncation_size_prof2, 0x16, 0, 14);

/* tac_ver
 * Access: RW
 */
MLXSW_ITEM8(reg, hgcr, tac_ver, 0x18, 0, 4);

/* tac_mirror_action
 * Access: RW
 */
MLXSW_ITEM8(reg, hgcr, tac_mirror_action, 0x19, 0, 2);

/* tac_crawler_timer
 * Access: RW
 */
MLXSW_ITEM8(reg, hgcr, tac_crawler_timer, 0x1a, 0, 5);

/* tac_crawler_action
 * Access: RW
 */
MLXSW_ITEM8(reg, hgcr, tac_crawler_action, 0x1b, 0, 2);

/* hcap
 * ----
 *
 */
/* max_cpu_ingress_tclass
 * Access: RO
 */
MLXSW_ITEM8(reg, hcap, max_cpu_ingress_tclass, 0xb, 0, 8);

/* max_num_trap_groups
 * Access: RO
 */
MLXSW_ITEM8(reg, hcap, max_num_trap_groups, 0xf, 0, 8);

/* max_num_dr_paths
 * Access: RO
 */
MLXSW_ITEM8(reg, hcap, max_num_dr_paths, 0x13, 0, 4);

/* hpkt
 * ----
 *
 */
/* source
 * Access: INDEX
 */
MLXSW_ITEM8(reg, hpkt, source, 0x0, 4, 3);

/* ack
 * Access: RW
 */
MLXSW_ITEM8(reg, hpkt, ack, 0x0, 0, 1);

/* action
 * Access: RW
 */
MLXSW_ITEM8(reg, hpkt, action, 0x1, 4, 4);

/* trap_group
 * Access: RW
 */
MLXSW_ITEM32(reg, hpkt, trap_group, 0x0, 12, 6);

/* trap_id
 * Access: INDEX
 */
MLXSW_ITEM16(reg, hpkt, trap_id, 0x2, 0, 10);

/* ctrl
 * Access: RW
 */
MLXSW_ITEM8(reg, hpkt, ctrl, 0x5, 0, 2);

/* tr_prof
 * Access: RW
 */
MLXSW_ITEM8(reg, hpkt, tr_prof, 0x6, 0, 2);

/* tr_en
 * Access: RW
 */
MLXSW_ITEM8(reg, hpkt, tr_en, 0x7, 0, 1);

/* htgt
 * ----
 *
 */
/* swid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, htgt, swid, 0x0, 0, 8);

/* type
 * Access: RW
 */
MLXSW_ITEM8(reg, htgt, type, 0x2, 0, 4);

/* trap_group
 * Access: INDEX
 */
MLXSW_ITEM8(reg, htgt, trap_group, 0x3, 0, 8);

/* pide
 * Access: RW
 */
MLXSW_ITEM8(reg, htgt, pide, 0x6, 7, 1);

/* pid
 * Access: RW
 */
MLXSW_ITEM16(reg, htgt, pid, 0x6, 0, 14);

/* mirror_action
 * Access: RW
 */
MLXSW_ITEM8(reg, htgt, mirror_action, 0xa, 0, 2);

/* mirror_agent
 * Access: RW
 */
MLXSW_ITEM8(reg, htgt, mirror_agent, 0xb, 0, 3);

/* priority
 * Access: RW
 */
MLXSW_ITEM8(reg, htgt, priority, 0xf, 0, 4);

/* cpu_tclass
 * Access: RW
 */
MLXSW_ITEM8(reg, htgt, cpu_tclass, 0x11, 0, 6);

/* rdq
 * Access: RW
 */
MLXSW_ITEM8(reg, htgt, rdq, 0x13, 0, 6);

/* cpu_split
 * Access: RW
 */
MLXSW_ITEM8(reg, htgt, cpu_split, 0x17, 0, 4);

/* mirror_probability_rate
 * Access: RW
 */
MLXSW_ITEM32(reg, htgt, mirror_probability_rate, 0x40, 0, 32);

/* htgc
 * ----
 *
 */
/* clr
 * Access: OP
 */
MLXSW_ITEM8(reg, htgc, clr, 0x0, 7, 1);

/* by_mask
 * Access: OP
 */
MLXSW_ITEM8(reg, htgc, by_mask, 0x0, 5, 1);

/* tg_mask
 * Access: INDEX
 */
MLXSW_ITEM64(reg, htgc, tg_mask, 0x18, 0, 64);

/* packets_counter_high
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, htgc, packets_counter_high, MLXSW_HTGC_TG_COUNTER_SET_BASE_LEN, 0, 32, MLXSW_HTGC_TG_COUNTER_SET_REC_LEN, 0, 4, 0);

/* packets_counter_low
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, htgc, packets_counter_low, MLXSW_HTGC_TG_COUNTER_SET_BASE_LEN, 0, 32, MLXSW_HTGC_TG_COUNTER_SET_REC_LEN, 4, 4, 0);

/* bytes_counter_high
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, htgc, bytes_counter_high, MLXSW_HTGC_TG_COUNTER_SET_BASE_LEN, 0, 32, MLXSW_HTGC_TG_COUNTER_SET_REC_LEN, 8, 4, 0);

/* bytes_counter_low
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, htgc, bytes_counter_low, MLXSW_HTGC_TG_COUNTER_SET_BASE_LEN, 0, 32, MLXSW_HTGC_TG_COUNTER_SET_REC_LEN, 12, 4, 0);

/* sll_discards_counter_high
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, htgc, sll_discards_counter_high, MLXSW_HTGC_TG_COUNTER_SET_BASE_LEN, 0, 32, MLXSW_HTGC_TG_COUNTER_SET_REC_LEN, 16, 4, 0);

/* sll_discards_counter_low
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, htgc, sll_discards_counter_low, MLXSW_HTGC_TG_COUNTER_SET_BASE_LEN, 0, 32, MLXSW_HTGC_TG_COUNTER_SET_REC_LEN, 20, 4, 0);

/* hll_discards_counter_high
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, htgc, hll_discards_counter_high, MLXSW_HTGC_TG_COUNTER_SET_BASE_LEN, 0, 32, MLXSW_HTGC_TG_COUNTER_SET_REC_LEN, 24, 4, 0);

/* hll_discards_counter_low
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, htgc, hll_discards_counter_low, MLXSW_HTGC_TG_COUNTER_SET_BASE_LEN, 0, 32, MLXSW_HTGC_TG_COUNTER_SET_REC_LEN, 28, 4, 0);

/* hca_discards_counter_high
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, htgc, hca_discards_counter_high, MLXSW_HTGC_TG_COUNTER_SET_BASE_LEN, 0, 32, MLXSW_HTGC_TG_COUNTER_SET_REC_LEN, 32, 4, 0);

/* hca_discards_counter_low
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, htgc, hca_discards_counter_low, MLXSW_HTGC_TG_COUNTER_SET_BASE_LEN, 0, 32, MLXSW_HTGC_TG_COUNTER_SET_REC_LEN, 36, 4, 0);

/* hmon
 * ----
 *
 */
/* clr
 * Access: OP
 */
MLXSW_ITEM8(reg, hmon, clr, 0x0, 7, 1);

/* cprdq
 * Access: INDEX
 */
MLXSW_ITEM8(reg, hmon, cprdq, 0x0, 6, 1);

/* by_mask
 * Access: OP
 */
MLXSW_ITEM8(reg, hmon, by_mask, 0x0, 5, 1);

/* read_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, hmon, read_type, 0x2, 0, 2);

/* crdq
 * Access: INDEX
 */
MLXSW_ITEM8(reg, hmon, crdq, 0x3, 0, 6);

/* wqe_of_wrap
 * Access: RO
 */
MLXSW_ITEM8(reg, hmon, wqe_of_wrap, 0x4, 7, 1);

/* wqe_overflow_high
 * Access: RO
 */
MLXSW_ITEM32(reg, hmon, wqe_overflow_high, 0x10, 0, 32);

/* wqe_overflow_low
 * Access: RO
 */
MLXSW_ITEM32(reg, hmon, wqe_overflow_low, 0x14, 0, 32);

/* rdq_mask
 * Access: INDEX
 */
MLXSW_ITEM64(reg, hmon, rdq_mask, 0x18, 0, 64);

/* wqe_overflow_rdq
 * Access: RW
 */
MLXSW_ITEM64_INDEXED(reg, hmon, wqe_overflow_rdq, 0x20, 0, 64, 8, 0, 8, 0);

/* hrwc
 * ----
 *
 */
/* clr
 * Access: OP
 */
MLXSW_ITEM8(reg, hrwc, clr, 0x0, 7, 1);

/* by_mask
 * Access: OP
 */
MLXSW_ITEM8(reg, hrwc, by_mask, 0x0, 5, 1);

/* rdq_mask
 * Access: INDEX
 */
MLXSW_ITEM64(reg, hrwc, rdq_mask, 0x18, 0, 64);

/* counter_high
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, hrwc, counter_high, MLXSW_HRWC_RDQ_WQE_COUNTER_BASE_LEN, 0, 32, MLXSW_HRWC_RDQ_WQE_COUNTER_REC_LEN, 0, 4, 0);

/* counter_low
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, hrwc, counter_low, MLXSW_HRWC_RDQ_WQE_COUNTER_BASE_LEN, 0, 32, MLXSW_HRWC_RDQ_WQE_COUNTER_REC_LEN, 4, 4, 0);

/* hctr
 * ----
 *
 */
/* custom_trap_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, hctr, custom_trap_index, 0x3, 0, 4);

/* tcp_dport
 * Access: RW
 */
MLXSW_ITEM8(reg, hctr, tcp_dport, 0x7, 3, 1);

/* tcp_sport
 * Access: RW
 */
MLXSW_ITEM8(reg, hctr, tcp_sport, 0x7, 2, 1);

/* udp_dport
 * Access: RW
 */
MLXSW_ITEM8(reg, hctr, udp_dport, 0x7, 1, 1);

/* range_min
 * Access: RW
 */
MLXSW_ITEM16(reg, hctr, range_min, 0xa, 0, 16);

/* range_max
 * Access: RW
 */
MLXSW_ITEM16(reg, hctr, range_max, 0xe, 0, 16);

/* hcot
 * ----
 *
 */
/* trap_id
 * Access: INDEX
 */
MLXSW_ITEM16(reg, hcot, trap_id, 0x2, 0, 10);

/* key_type
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, key_type, 0x7, 0, 8);

/* key_hcot_key_ethernet_ethertype
 * Access: RW
 */
MLXSW_ITEM16(reg, hcot, key_hcot_key_ethernet_ethertype, 0x12, 0, 16);

/* key_hcot_key_ethernet_tunnel_port
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, key_hcot_key_ethernet_tunnel_port, 0x17, 0, 4);

/* key_hcot_key_next_proto_is_ipv4
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, key_hcot_key_next_proto_is_ipv4, 0x11, 0, 1);

/* key_hcot_key_next_proto_ip_proto
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, key_hcot_key_next_proto_ip_proto, 0x13, 0, 8);

/* key_hcot_key_next_proto_tunnel_port
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, key_hcot_key_next_proto_tunnel_port, 0x17, 0, 4);

/* key_hcot_key_layer4_port_is_udp
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, key_hcot_key_layer4_port_is_udp, 0x11, 1, 1);

/* key_hcot_key_layer4_port_is_ipv4
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, key_hcot_key_layer4_port_is_ipv4, 0x11, 0, 1);

/* key_hcot_key_layer4_port_l4_port
 * Access: RW
 */
MLXSW_ITEM16(reg, hcot, key_hcot_key_layer4_port_l4_port, 0x12, 0, 16);

/* key_hcot_key_layer4_port_tunnel_port
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, key_hcot_key_layer4_port_tunnel_port, 0x17, 0, 4);

/* key_hcot_key_icmp_igmp_type_is_igmp
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, key_hcot_key_icmp_igmp_type_is_igmp, 0x11, 1, 1);

/* key_hcot_key_icmp_igmp_type_is_ipv4
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, key_hcot_key_icmp_igmp_type_is_ipv4, 0x11, 0, 1);

/* key_hcot_key_icmp_igmp_type_max
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, key_hcot_key_icmp_igmp_type_max, 0x12, 0, 8);

/* key_hcot_key_icmp_igmp_type_min
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, key_hcot_key_icmp_igmp_type_min, 0x13, 0, 8);

/* key_hcot_key_icmp_igmp_type_tunnel_port
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, key_hcot_key_icmp_igmp_type_tunnel_port, 0x17, 0, 4);

/* mask_hcot_key_ethernet_ethertype
 * Access: RW
 */
MLXSW_ITEM16(reg, hcot, mask_hcot_key_ethernet_ethertype, 0x22, 0, 16);

/* mask_hcot_key_ethernet_tunnel_port
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, mask_hcot_key_ethernet_tunnel_port, 0x27, 0, 4);

/* mask_hcot_key_next_proto_is_ipv4
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, mask_hcot_key_next_proto_is_ipv4, 0x21, 0, 1);

/* mask_hcot_key_next_proto_ip_proto
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, mask_hcot_key_next_proto_ip_proto, 0x23, 0, 8);

/* mask_hcot_key_next_proto_tunnel_port
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, mask_hcot_key_next_proto_tunnel_port, 0x27, 0, 4);

/* mask_hcot_key_layer4_port_is_udp
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, mask_hcot_key_layer4_port_is_udp, 0x21, 1, 1);

/* mask_hcot_key_layer4_port_is_ipv4
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, mask_hcot_key_layer4_port_is_ipv4, 0x21, 0, 1);

/* mask_hcot_key_layer4_port_l4_port
 * Access: RW
 */
MLXSW_ITEM16(reg, hcot, mask_hcot_key_layer4_port_l4_port, 0x22, 0, 16);

/* mask_hcot_key_layer4_port_tunnel_port
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, mask_hcot_key_layer4_port_tunnel_port, 0x27, 0, 4);

/* mask_hcot_key_icmp_igmp_type_is_igmp
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, mask_hcot_key_icmp_igmp_type_is_igmp, 0x21, 1, 1);

/* mask_hcot_key_icmp_igmp_type_is_ipv4
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, mask_hcot_key_icmp_igmp_type_is_ipv4, 0x21, 0, 1);

/* mask_hcot_key_icmp_igmp_type_max
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, mask_hcot_key_icmp_igmp_type_max, 0x22, 0, 8);

/* mask_hcot_key_icmp_igmp_type_min
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, mask_hcot_key_icmp_igmp_type_min, 0x23, 0, 8);

/* mask_hcot_key_icmp_igmp_type_tunnel_port
 * Access: RW
 */
MLXSW_ITEM8(reg, hcot, mask_hcot_key_icmp_igmp_type_tunnel_port, 0x27, 0, 4);

/* hrdqt
 * -----
 *
 */
/* rdq
 * Access: INDEX
 */
MLXSW_ITEM8(reg, hrdqt, rdq, 0x3, 0, 6);

/* tac_en
 * Access: RW
 */
MLXSW_ITEM8(reg, hrdqt, tac_en, 0x7, 0, 1);

/* mirror_agent
 * Access: RW
 */
MLXSW_ITEM8(reg, hrdqt, mirror_agent, 0xa, 0, 4);

/* htacg
 * -----
 *
 */
/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, htacg, status, 0x3, 0, 2);

/* tac_flush
 * Access: RW
 */
MLXSW_ITEM8(reg, htacg, tac_flush, 0x7, 0, 2);

/* fields_mirror_reason
 * Access: RW
 */
MLXSW_ITEM8(reg, htacg, fields_mirror_reason, 0x13, 0, 8);

/* fields_rdq
 * Access: RW
 */
MLXSW_ITEM8(reg, htacg, fields_rdq, 0x14, 0, 6);

/* fields_trap_id
 * Access: RW
 */
MLXSW_ITEM16(reg, htacg, fields_trap_id, 0x16, 0, 10);

/* fields_traffic_class
 * Access: RW
 */
MLXSW_ITEM8(reg, htacg, fields_traffic_class, 0x19, 0, 5);

/* fields_tx
 * Access: RW
 */
MLXSW_ITEM8(reg, htacg, fields_tx, 0x1b, 0, 8);

/* fields_max_mirror_cong
 * Access: RW
 */
MLXSW_ITEM16(reg, htacg, fields_max_mirror_cong, 0x1e, 0, 16);

/* fields_last_ts
 * Access: RW
 */
MLXSW_ITEM32(reg, htacg, fields_last_ts, 0x20, 0, 32);

/* fields_packet_count
 * Access: RW
 */
MLXSW_ITEM32(reg, htacg, fields_packet_count, 0x24, 0, 32);

/* fields_byte_count
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, htacg, fields_byte_count, 0x2b, 5);

/* mask_mirror_reason
 * Access: RW
 */
MLXSW_ITEM8(reg, htacg, mask_mirror_reason, 0x53, 0, 8);

/* mask_rdq
 * Access: RW
 */
MLXSW_ITEM8(reg, htacg, mask_rdq, 0x54, 0, 6);

/* mask_trap_id
 * Access: RW
 */
MLXSW_ITEM16(reg, htacg, mask_trap_id, 0x56, 0, 10);

/* mask_traffic_class
 * Access: RW
 */
MLXSW_ITEM8(reg, htacg, mask_traffic_class, 0x59, 0, 5);

/* mask_tx
 * Access: RW
 */
MLXSW_ITEM8(reg, htacg, mask_tx, 0x5b, 0, 8);

/* mask_max_mirror_cong
 * Access: RW
 */
MLXSW_ITEM16(reg, htacg, mask_max_mirror_cong, 0x5e, 0, 16);

/* mask_last_ts
 * Access: RW
 */
MLXSW_ITEM32(reg, htacg, mask_last_ts, 0x60, 0, 32);

/* mask_packet_count
 * Access: RW
 */
MLXSW_ITEM32(reg, htacg, mask_packet_count, 0x64, 0, 32);

/* mask_byte_count
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, htacg, mask_byte_count, 0x6b, 5);

/* htec
 * ----
 *
 */
/* tac_event_header
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, htec, tac_event_header, 0x0, 0, 32, 4, 0, 4, 0);

/* reg_tlv_type
 * Access: RW
 */
MLXSW_ITEM8(reg, htec, reg_tlv_type, 0x20, 3, 5);

/* res_reg_tlv
 * Access: RW
 */
MLXSW_ITEM16(reg, htec, res_reg_tlv, 0x22, 0, 16);

/* new_hdr_type
 * Access: RW
 */
MLXSW_ITEM8(reg, htec, new_hdr_type, 0x24, 3, 5);

/* res_4_new
 * Access: RW
 */
MLXSW_ITEM16(reg, htec, res_4_new, 0x26, 4, 12);

/* agg_hdr_type
 * Access: RW
 */
MLXSW_ITEM8(reg, htec, agg_hdr_type, 0x28, 3, 5);

/* res_4_agg
 * Access: RW
 */
MLXSW_ITEM16(reg, htec, res_4_agg, 0x2a, 4, 12);

/* end_tlv_type
 * Access: RW
 */
MLXSW_ITEM8(reg, htec, end_tlv_type, 0x2c, 3, 5);

/* end_tlv_len
 * Access: RW
 */
MLXSW_ITEM16(reg, htec, end_tlv_len, 0x2c, 0, 11);

/* res_end_tlv
 * Access: RW
 */
MLXSW_ITEM16(reg, htec, res_end_tlv, 0x2e, 0, 16);

/* htcr
 * ----
 *
 */
/* clr
 * Access: OP
 */
MLXSW_ITEM8(reg, htcr, clr, 0x0, 7, 1);

/* tac_received_pkts
 * Access: RO
 */
MLXSW_ITEM64(reg, htcr, tac_received_pkts, 0x8, 0, 64);

/* ingress_drop_rdq
 * Access: RO
 */
MLXSW_ITEM64_INDEXED(reg, htcr, ingress_drop_rdq, 0x10, 0, 64, 8, 0, 8, 0);

/* hcpnc
 * -----
 *
 */
/* clr
 * Access: OP
 */
MLXSW_ITEM8(reg, hcpnc, clr, 0x0, 7, 1);

/* cnt_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, hcpnc, cnt_type, 0x3, 0, 5);

/* pkt_ok_cnt
 * Access: RO
 */
MLXSW_ITEM64(reg, hcpnc, pkt_ok_cnt, 0x8, 0, 64);

/* byte_ok_cnt
 * Access: RO
 */
MLXSW_ITEM64(reg, hcpnc, byte_ok_cnt, 0x10, 0, 64);

/* pkt_discard_cnt
 * Access: RO
 */
MLXSW_ITEM64(reg, hcpnc, pkt_discard_cnt, 0x18, 0, 64);

/* hett
 * ----
 *
 */
/* opcode
 * Access: WO
 */
MLXSW_ITEM8(reg, hett, opcode, 0x2, 0, 1);

/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, hett, status, 0x3, 0, 2);

/* trap_group_bitmap
 * Access: INDEX
 */
MLXSW_ITEM64(reg, hett, trap_group_bitmap, 0x8, 0, 64);

/* trap_group
 * Access: RO
 */
MLXSW_ITEM8(reg, hett, trap_group, 0x13, 0, 8);

/* hccr
 * ----
 *
 */
/* use_nve_opt_chk
 * Access: RW
 */
MLXSW_ITEM8(reg, hccr, use_nve_opt_chk, 0x3, 0, 1);

/* hthm
 * ----
 *
 */
/* num_trap_ids
 * Access: RO
 */
MLXSW_ITEM8(reg, hthm, num_trap_ids, 0x1, 0, 8);

/* trap_id
 * Access: INDEX
 */
MLXSW_ITEM16(reg, hthm, trap_id, 0x2, 0, 10);

/* last
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, hthm, last, MLXSW_HTHM_HW_TRAP_IDS_BASE_LEN, 7, 1, MLXSW_HTHM_HW_TRAP_IDS_REC_LEN, 0, 1, 0);

/* hw_trap_id
 * Access: RO
 */
MLXSW_ITEM16_INDEXED(reg, hthm, hw_trap_id, MLXSW_HTHM_HW_TRAP_IDS_BASE_LEN + 2, 0, 10, MLXSW_HTHM_HW_TRAP_IDS_REC_LEN, 0, 2, 0);

/* mtpps
 * -----
 *
 */
/* cap_number_of_pps_pins
 * Access: RO
 */
MLXSW_ITEM8(reg, mtpps, cap_number_of_pps_pins, 0x1, 0, 4);

/* cap_max_num_of_pps_in_pins
 * Access: RO
 */
MLXSW_ITEM8(reg, mtpps, cap_max_num_of_pps_in_pins, 0x2, 0, 4);

/* cap_max_num_of_pps_out_pins
 * Access: RO
 */
MLXSW_ITEM8(reg, mtpps, cap_max_num_of_pps_out_pins, 0x3, 0, 4);

/* cap_pin_3_mode
 * Access: RO
 */
MLXSW_ITEM8(reg, mtpps, cap_pin_3_mode, 0x8, 0, 4);

/* cap_pin_2_mode
 * Access: RO
 */
MLXSW_ITEM8(reg, mtpps, cap_pin_2_mode, 0x9, 0, 4);

/* cap_pin_1_mode
 * Access: RO
 */
MLXSW_ITEM8(reg, mtpps, cap_pin_1_mode, 0xa, 0, 4);

/* cap_pin_0_mode
 * Access: RO
 */
MLXSW_ITEM8(reg, mtpps, cap_pin_0_mode, 0xb, 0, 4);

/* cap_pin_7_mode
 * Access: RO
 */
MLXSW_ITEM8(reg, mtpps, cap_pin_7_mode, 0xc, 0, 4);

/* cap_pin_6_mode
 * Access: RO
 */
MLXSW_ITEM8(reg, mtpps, cap_pin_6_mode, 0xd, 0, 4);

/* cap_pin_5_mode
 * Access: RO
 */
MLXSW_ITEM8(reg, mtpps, cap_pin_5_mode, 0xe, 0, 4);

/* cap_pin_4_mode
 * Access: RO
 */
MLXSW_ITEM8(reg, mtpps, cap_pin_4_mode, 0xf, 0, 4);

/* field_select
 * Access: WO
 */
MLXSW_ITEM32(reg, mtpps, field_select, 0x10, 0, 32);

/* enable
 * Access: RW
 */
MLXSW_ITEM8(reg, mtpps, enable, 0x20, 7, 1);

/* utc_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, mtpps, utc_mode, 0x20, 6, 1);

/* pattern
 * Access: RW
 */
MLXSW_ITEM8(reg, mtpps, pattern, 0x21, 0, 4);

/* pin_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, mtpps, pin_mode, 0x22, 0, 4);

/* pin
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mtpps, pin, 0x23, 0, 8);

/* time_stamp
 * Access: RW
 */
MLXSW_ITEM64(reg, mtpps, time_stamp, 0x28, 0, 64);

/* out_pulse_duration
 * Access: RW
 */
MLXSW_ITEM16(reg, mtpps, out_pulse_duration, 0x30, 0, 16);

/* mtppse
 * ------
 *
 */
/* pin
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mtppse, pin, 0x3, 0, 8);

/* event_arm
 * Access: RW
 */
MLXSW_ITEM8(reg, mtppse, event_arm, 0x4, 7, 1);

/* event_generation_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, mtppse, event_generation_mode, 0x7, 0, 4);

/* mtutc
 * -----
 *
 */
/* freq_adj_units
 * Access: RW
 */
MLXSW_ITEM8(reg, mtutc, freq_adj_units, 0x0, 0, 3);

/* log_max_freq_adjustment
 * Access: RO
 */
MLXSW_ITEM8(reg, mtutc, log_max_freq_adjustment, 0x1, 0, 5);

/* log_max_phase_adjustment
 * Access: RO
 */
MLXSW_ITEM16(reg, mtutc, log_max_phase_adjustment, 0x2, 5, 6);

/* operation
 * Access: OP
 */
MLXSW_ITEM8(reg, mtutc, operation, 0x3, 0, 4);

/* freq_adjustment
 * Access: RW
 */
MLXSW_ITEM32(reg, mtutc, freq_adjustment, 0x4, 0, 32);

/* utc_sec
 * Access: WO
 */
MLXSW_ITEM32(reg, mtutc, utc_sec, 0x10, 0, 32);

/* utc_nsec
 * Access: WO
 */
MLXSW_ITEM32(reg, mtutc, utc_nsec, 0x14, 0, 30);

/* time_adjustment
 * Access: WO
 */
MLXSW_ITEM32(reg, mtutc, time_adjustment, 0x18, 0, 32);

/* mtppst
 * ------
 *
 */
/* pin
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mtppst, pin, 0x3, 0, 8);

/* timestamp_msb
 * Access: RO
 */
MLXSW_ITEM32(reg, mtppst, timestamp_msb, 0x8, 0, 32);

/* timestamp_lsb
 * Access: RO
 */
MLXSW_ITEM32(reg, mtppst, timestamp_lsb, 0xc, 0, 32);

/* msecq
 * -----
 *
 */
/* network_option
 * Access: RO
 */
MLXSW_ITEM8(reg, msecq, network_option, 0x6, 4, 2);

/* local_ssm_code
 * Access: RO
 */
MLXSW_ITEM8(reg, msecq, local_ssm_code, 0x6, 0, 4);

/* local_enhanced_ssm_code
 * Access: RO
 */
MLXSW_ITEM8(reg, msecq, local_enhanced_ssm_code, 0x7, 0, 8);

/* local_clock_identity
 * Access: RO
 */
MLXSW_ITEM64(reg, msecq, local_clock_identity, 0x8, 0, 64);

/* msees
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, msees, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, msees, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, msees, lp_msb, 0x2, 4, 2);

/* field_select
 * Access: WO
 */
MLXSW_ITEM32(reg, msees, field_select, 0x4, 0, 32);

/* admin_status
 * Access: RW
 */
MLXSW_ITEM8(reg, msees, admin_status, 0x8, 4, 4);

/* oper_status
 * Access: RO
 */
MLXSW_ITEM8(reg, msees, oper_status, 0x8, 0, 4);

/* ho_acq
 * Access: RO
 */
MLXSW_ITEM8(reg, msees, ho_acq, 0x9, 7, 1);

/* admin_freq_measure
 * Access: RW
 */
MLXSW_ITEM8(reg, msees, admin_freq_measure, 0xa, 2, 1);

/* oper_freq_measure
 * Access: RO
 */
MLXSW_ITEM8(reg, msees, oper_freq_measure, 0xa, 1, 1);

/* failure_reason
 * Access: RO
 */
MLXSW_ITEM16(reg, msees, failure_reason, 0xa, 0, 9);

/* frequency_diff
 * Access: RO
 */
MLXSW_ITEM32(reg, msees, frequency_diff, 0xc, 0, 32);

/* mspi
 * ----
 *
 */
/* synce_support
 * Access: RO
 */
MLXSW_ITEM8(reg, mspi, synce_support, 0x0, 7, 1);

/* clk_dev
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mspi, clk_dev, 0x1, 0, 2);

/* vendor_id
 * Access: RO
 */
MLXSW_ITEM8(reg, mspi, vendor_id, 0x3, 0, 4);

/* config_version_id
 * Access: RO
 */
MLXSW_ITEM16(reg, mspi, config_version_id, 0x6, 0, 16);

/* bcc
 * Access: RO
 */
MLXSW_ITEM8(reg, mspi, bcc, 0xb, 0, 8);

/* obtv
 * Access: RO
 */
MLXSW_ITEM8(reg, mspi, obtv, 0x10, 7, 1);

/* obs_temperature
 * Access: RO
 */
MLXSW_ITEM16(reg, mspi, obs_temperature, 0x12, 0, 16);

/* bitv
 * Access: RO
 */
MLXSW_ITEM8(reg, mspi, bitv, 0x14, 7, 1);

/* bis_temperature
 * Access: RO
 */
MLXSW_ITEM16(reg, mspi, bis_temperature, 0x16, 0, 16);

/* mgpcb
 * -----
 *
 */
/* counter_index_base
 * Access: OP
 */
MLXSW_ITEM32(reg, mgpcb, counter_index_base, 0x0, 0, 24);

/* num_rec
 * Access: OP
 */
MLXSW_ITEM8(reg, mgpcb, num_rec, 0x7, 0, 8);

/* opcode
 * Access: OP
 */
MLXSW_ITEM8(reg, mgpcb, opcode, 0xb, 0, 4);

/* byte_counter
 * Access: RO
 */
MLXSW_ITEM64_INDEXED(reg, mgpcb, byte_counter, MLXSW_MGPCB_RECORD_BASE_LEN, 0, 64, MLXSW_MGPCB_RECORD_REC_LEN, 0, 8, 0);

/* packet_counter
 * Access: RO
 */
MLXSW_ITEM64_INDEXED(reg, mgpcb, packet_counter, MLXSW_MGPCB_RECORD_BASE_LEN, 0, 64, MLXSW_MGPCB_RECORD_REC_LEN, 8, 8, 0);

/* mcion
 * -----
 *
 */
/* module
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mcion, module, 0x1, 0, 8);

/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mcion, slot_index, 0x2, 4, 4);

/* module_status_bits
 * Access: RO
 */
MLXSW_ITEM16(reg, mcion, module_status_bits, 0x6, 0, 16);

/* module_status_bits_valid
 * Access: RO
 */
MLXSW_ITEM16(reg, mcion, module_status_bits_valid, 0xc, 0, 16);

/* module_inputs
 * Access: RW
 */
MLXSW_ITEM8(reg, mcion, module_inputs, 0xf, 0, 4);

/* module_inputs_mask
 * Access: RW
 */
MLXSW_ITEM8(reg, mcion, module_inputs_mask, 0x13, 0, 4);

/* mcfs
 * ----
 *
 */
/* module
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mcfs, module, 0x1, 0, 8);

/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mcfs, slot_index, 0x2, 4, 4);

/* freq
 * Access: RW
 */
MLXSW_ITEM8(reg, mcfs, freq, 0x7, 0, 2);

/* melseq
 * ------
 *
 */
/* cc
 * Access: OP
 */
MLXSW_ITEM8(reg, melseq, cc, 0x0, 7, 1);

/* err_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, melseq, err_type, 0x1, 0, 8);

/* module
 * Access: INDEX
 */
MLXSW_ITEM8(reg, melseq, module, 0x3, 0, 8);

/* laser_warn_cnt
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, melseq, laser_warn_cnt, 0x10, 8);

/* laser_alarm_cnt
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, melseq, laser_alarm_cnt, 0x20, 8);

/* mmta
 * ----
 *
 */
/* supported_measurements
 * Access: RO
 */
MLXSW_ITEM8(reg, mmta, supported_measurements, 0x0, 0, 4);

/* module
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mmta, module, 0x3, 0, 8);

/* module_name_hi
 * Access: RO
 */
MLXSW_ITEM32(reg, mmta, module_name_hi, 0x4, 0, 32);

/* module_name_lo
 * Access: RO
 */
MLXSW_ITEM32(reg, mmta, module_name_lo, 0x8, 0, 32);

/* module_temperature_mte
 * Access: RW
 */
MLXSW_ITEM8(reg, mmta, module_temperature_mte, 0xc, 7, 1);

/* module_temperature_mtr
 * Access: OP
 */
MLXSW_ITEM8(reg, mmta, module_temperature_mtr, 0xc, 6, 1);

/* module_temperature_twe
 * Access: OP
 */
MLXSW_ITEM8(reg, mmta, module_temperature_twe, 0xc, 5, 1);

/* module_temperature_twee
 * Access: RW
 */
MLXSW_ITEM8(reg, mmta, module_temperature_twee, 0xc, 3, 2);

/* module_temperature_temp_unit
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mmta, module_temperature_temp_unit, 0xc, 2, 1);

/* module_temperature_max_temperature
 * Access: RO
 */
MLXSW_ITEM16(reg, mmta, module_temperature_max_temperature, 0x10, 0, 16);

/* module_temperature_temperature
 * Access: RO
 */
MLXSW_ITEM16(reg, mmta, module_temperature_temperature, 0x12, 0, 16);

/* module_temperature_temperature_warning_high
 * Access: RW
 */
MLXSW_ITEM16(reg, mmta, module_temperature_temperature_warning_high, 0x14, 0, 16);

/* module_temperature_temperature_warning_low
 * Access: RW
 */
MLXSW_ITEM16(reg, mmta, module_temperature_temperature_warning_low, 0x16, 0, 16);

/* module_temperature_temperature_alarm_high
 * Access: RW
 */
MLXSW_ITEM16(reg, mmta, module_temperature_temperature_alarm_high, 0x18, 0, 16);

/* module_temperature_temperature_alarm_low
 * Access: RW
 */
MLXSW_ITEM16(reg, mmta, module_temperature_temperature_alarm_low, 0x1a, 0, 16);

/* mtpe
 * Access: RW
 */
MLXSW_ITEM8(reg, mmta, mtpe, 0x24, 7, 1);

/* mtpr
 * Access: OP
 */
MLXSW_ITEM8(reg, mmta, mtpr, 0x24, 6, 1);

/* temp_unit
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mmta, temp_unit, 0x24, 5, 1);

/* cooling_level
 * Access: RO
 */
MLXSW_ITEM16(reg, mmta, cooling_level, 0x26, 0, 16);

/* max_tec_power
 * Access: RO
 */
MLXSW_ITEM16(reg, mmta, max_tec_power, 0x28, 0, 16);

/* tec_power
 * Access: RO
 */
MLXSW_ITEM16(reg, mmta, tec_power, 0x2a, 0, 16);

/* tec_power_warning_high
 * Access: RW
 */
MLXSW_ITEM16(reg, mmta, tec_power_warning_high, 0x2c, 0, 16);

/* tec_power_warning_low
 * Access: RW
 */
MLXSW_ITEM16(reg, mmta, tec_power_warning_low, 0x2e, 0, 16);

/* tec_power_alarm_high
 * Access: RW
 */
MLXSW_ITEM16(reg, mmta, tec_power_alarm_high, 0x30, 0, 16);

/* tec_power_alarm_low
 * Access: RW
 */
MLXSW_ITEM16(reg, mmta, tec_power_alarm_low, 0x32, 0, 16);

/* max_set_point_temperature
 * Access: RO
 */
MLXSW_ITEM16(reg, mmta, max_set_point_temperature, 0x34, 0, 16);

/* set_point_temperature
 * Access: RO
 */
MLXSW_ITEM16(reg, mmta, set_point_temperature, 0x36, 0, 16);

/* set_point_temperature_warning_high
 * Access: RW
 */
MLXSW_ITEM16(reg, mmta, set_point_temperature_warning_high, 0x38, 0, 16);

/* set_point_temperature_warning_low
 * Access: RW
 */
MLXSW_ITEM16(reg, mmta, set_point_temperature_warning_low, 0x3a, 0, 16);

/* set_point_temperature_alarm_high
 * Access: RW
 */
MLXSW_ITEM16(reg, mmta, set_point_temperature_alarm_high, 0x3c, 0, 16);

/* set_point_temperature_alarm_low
 * Access: RW
 */
MLXSW_ITEM16(reg, mmta, set_point_temperature_alarm_low, 0x3e, 0, 16);

/* max_cooling_level
 * Access: RO
 */
MLXSW_ITEM16(reg, mmta, max_cooling_level, 0x40, 0, 16);

/* min_cooling_level
 * Access: RO
 */
MLXSW_ITEM16(reg, mmta, min_cooling_level, 0x42, 0, 16);

/* module_second_temperature_mte
 * Access: RW
 */
MLXSW_ITEM8(reg, mmta, module_second_temperature_mte, 0x44, 7, 1);

/* module_second_temperature_mtr
 * Access: OP
 */
MLXSW_ITEM8(reg, mmta, module_second_temperature_mtr, 0x44, 6, 1);

/* module_second_temperature_twe
 * Access: OP
 */
MLXSW_ITEM8(reg, mmta, module_second_temperature_twe, 0x44, 5, 1);

/* module_second_temperature_twee
 * Access: RW
 */
MLXSW_ITEM8(reg, mmta, module_second_temperature_twee, 0x44, 3, 2);

/* module_second_temperature_temp_unit
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mmta, module_second_temperature_temp_unit, 0x44, 2, 1);

/* module_second_temperature_max_temperature
 * Access: RO
 */
MLXSW_ITEM16(reg, mmta, module_second_temperature_max_temperature, 0x48, 0, 16);

/* module_second_temperature_temperature
 * Access: RO
 */
MLXSW_ITEM16(reg, mmta, module_second_temperature_temperature, 0x4a, 0, 16);

/* module_second_temperature_temperature_warning_high
 * Access: RW
 */
MLXSW_ITEM16(reg, mmta, module_second_temperature_temperature_warning_high, 0x4c, 0, 16);

/* module_second_temperature_temperature_warning_low
 * Access: RW
 */
MLXSW_ITEM16(reg, mmta, module_second_temperature_temperature_warning_low, 0x4e, 0, 16);

/* module_second_temperature_temperature_alarm_high
 * Access: RW
 */
MLXSW_ITEM16(reg, mmta, module_second_temperature_temperature_alarm_high, 0x50, 0, 16);

/* module_second_temperature_temperature_alarm_low
 * Access: RW
 */
MLXSW_ITEM16(reg, mmta, module_second_temperature_temperature_alarm_low, 0x52, 0, 16);

/* mddt
 * ----
 *
 */
/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mddt, slot_index, 0x2, 0, 4);

/* device_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mddt, device_index, 0x3, 0, 8);

/* read_size
 * Access: OP
 */
MLXSW_ITEM8(reg, mddt, read_size, 0x4, 0, 8);

/* write_size
 * Access: OP
 */
MLXSW_ITEM8(reg, mddt, write_size, 0x5, 0, 8);

/* type
 * Access: OP
 */
MLXSW_ITEM8(reg, mddt, type, 0x7, 0, 2);

/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, mddt, status, 0xc, 0, 8);

/* method
 * Access: OP
 */
MLXSW_ITEM8(reg, mddt, method, 0xd, 6, 2);

/* register_id
 * Access: INDEX
 */
MLXSW_ITEM16(reg, mddt, register_id, 0xe, 0, 16);

/* register_data
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mddt, register_data, 0x10, 0, 32, 4, 0, 4, 0);

/* command_payload_data
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mddt, command_payload_data, 0xc, 0, 32, 4, 0, 4, 0);

/* crspace_access_payload_address
 * Access: WO
 */
MLXSW_ITEM32(reg, mddt, crspace_access_payload_address, 0xc, 0, 32);

/* crspace_access_payload_data
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mddt, crspace_access_payload_data, 0x10, 0, 32, 4, 0, 4, 0);

/* mddq
 * ----
 *
 */
/* sie
 * Access: RW
 */
MLXSW_ITEM8(reg, mddq, sie, 0x0, 7, 1);

/* query_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mddq, query_type, 0x1, 0, 8);

/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mddq, slot_index, 0x3, 0, 4);

/* response_message_sequence
 * Access: RO
 */
MLXSW_ITEM8(reg, mddq, response_message_sequence, 0x5, 0, 8);

/* request_message_sequence
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mddq, request_message_sequence, 0x7, 0, 8);

/* data_valid
 * Access: RO
 */
MLXSW_ITEM8(reg, mddq, data_valid, 0x8, 7, 1);

/* query_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mddq, query_index, 0xb, 0, 8);

/* provisioned
 * Access: RO
 */
MLXSW_ITEM8(reg, mddq, provisioned, 0x10, 7, 1);

/* sr_valid
 * Access: RO
 */
MLXSW_ITEM8(reg, mddq, sr_valid, 0x10, 6, 1);

/* lc_ready
 * Access: RO
 */
MLXSW_ITEM8(reg, mddq, lc_ready, 0x10, 4, 2);

/* active
 * Access: RO
 */
MLXSW_ITEM8(reg, mddq, active, 0x10, 3, 1);

/* hw_revision
 * Access: RO
 */
MLXSW_ITEM16(reg, mddq, hw_revision, 0x14, 0, 16);

/* ini_file_version
 * Access: RO
 */
MLXSW_ITEM16(reg, mddq, ini_file_version, 0x16, 0, 16);

/* card_type
 * Access: RO
 */
MLXSW_ITEM8(reg, mddq, card_type, 0x1b, 0, 8);

/* uses_flash
 * Access: RO
 */
MLXSW_ITEM8(reg, mddq, uses_flash, 0x10, 7, 1);

/* flash_owner
 * Access: RO
 */
MLXSW_ITEM8(reg, mddq, flash_owner, 0x10, 6, 1);

/* thermal_sd
 * Access: RO
 */
MLXSW_ITEM8(reg, mddq, thermal_sd, 0x10, 5, 1);

/* lc_pwr_on
 * Access: RO
 */
MLXSW_ITEM8(reg, mddq, lc_pwr_on, 0x10, 4, 1);

/* flash_id
 * Access: RO
 */
MLXSW_ITEM8(reg, mddq, flash_id, 0x11, 0, 8);

/* device_index
 * Access: RO
 */
MLXSW_ITEM8(reg, mddq, device_index, 0x13, 0, 8);

/* fw_major
 * Access: RO
 */
MLXSW_ITEM16(reg, mddq, fw_major, 0x14, 0, 16);

/* device_type
 * Access: RO
 */
MLXSW_ITEM16(reg, mddq, device_type, 0x16, 0, 16);

/* fw_minor
 * Access: RO
 */
MLXSW_ITEM16(reg, mddq, fw_minor, 0x18, 0, 16);

/* fw_sub_minor
 * Access: RO
 */
MLXSW_ITEM16(reg, mddq, fw_sub_minor, 0x1a, 0, 16);

/* max_cmd_read_size_supp
 * Access: RO
 */
MLXSW_ITEM8(reg, mddq, max_cmd_read_size_supp, 0x1e, 0, 8);

/* max_cmd_write_size_supp
 * Access: RO
 */
MLXSW_ITEM8(reg, mddq, max_cmd_write_size_supp, 0x1f, 0, 8);

/* device_type_name
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, mddq, device_type_name, 0x20, 8);

/* slot_ascii_name
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, mddq, slot_ascii_name, 0x10, 20);

/* mddc
 * ----
 *
 */
/* level
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mddc, level, 0x0, 0, 4);

/* index
 * Access: INDEX
 */
MLXSW_ITEM32(reg, mddc, index, 0x0, 9, 8);

/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mddc, slot_index, 0x3, 0, 4);

/* rst
 * Access: OP
 */
MLXSW_ITEM8(reg, mddc, rst, 0x4, 5, 3);

/* device_enable
 * Access: RW
 */
MLXSW_ITEM8(reg, mddc, device_enable, 0x4, 4, 1);

/* mgpir
 * -----
 *
 */
/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mgpir, slot_index, 0x0, 4, 4);

/* device_type
 * Access: RO
 */
MLXSW_ITEM8(reg, mgpir, device_type, 0x0, 0, 4);

/* devices_per_flash
 * Access: RO
 */
MLXSW_ITEM8(reg, mgpir, devices_per_flash, 0x1, 0, 8);

/* num_of_modules_per_system
 * Access: RO
 */
MLXSW_ITEM8(reg, mgpir, num_of_modules_per_system, 0x2, 0, 8);

/* num_of_devices
 * Access: RO
 */
MLXSW_ITEM8(reg, mgpir, num_of_devices, 0x3, 0, 8);

/* first_pluggable_interconnect_offset
 * Access: RO
 */
MLXSW_ITEM8(reg, mgpir, first_pluggable_interconnect_offset, 0x4, 0, 8);

/* max_modules_per_slot
 * Access: RO
 */
MLXSW_ITEM8(reg, mgpir, max_modules_per_slot, 0x5, 0, 8);

/* num_of_slots
 * Access: RO
 */
MLXSW_ITEM8(reg, mgpir, num_of_slots, 0x6, 0, 8);

/* num_of_modules
 * Access: RO
 */
MLXSW_ITEM8(reg, mgpir, num_of_modules, 0x7, 0, 8);

/* max_sub_modules_index
 * Access: RO
 */
MLXSW_ITEM8(reg, mgpir, max_sub_modules_index, 0x8, 0, 4);

/* num_lanes_per_sub_module
 * Access: RO
 */
MLXSW_ITEM8(reg, mgpir, num_lanes_per_sub_module, 0x9, 0, 4);

/* num_of_resource_modules
 * Access: RO
 */
MLXSW_ITEM8(reg, mgpir, num_of_resource_modules, 0xb, 0, 8);

/* mdfcr
 * -----
 *
 */
/* device_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mdfcr, device_type, 0x3, 0, 8);

/* all
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mdfcr, all, 0x4, 7, 1);

/* device_index
 * Access: INDEX
 */
MLXSW_ITEM16(reg, mdfcr, device_index, 0x6, 0, 12);

/* fw_status
 * Access: RO
 */
MLXSW_ITEM8(reg, mdfcr, fw_status, 0xd, 4, 4);

/* first_fw_status_device
 * Access: RO
 */
MLXSW_ITEM16(reg, mdfcr, first_fw_status_device, 0xe, 0, 12);

/* expected_fw_version
 * Access: RO
 */
MLXSW_ITEM32(reg, mdfcr, expected_fw_version, 0x10, 0, 32);

/* fw_version
 * Access: RO
 */
MLXSW_ITEM32(reg, mdfcr, fw_version, 0x14, 0, 32);

/* build_id
 * Access: RO
 */
MLXSW_ITEM16(reg, mdfcr, build_id, 0x1a, 0, 16);

/* major
 * Access: RO
 */
MLXSW_ITEM8(reg, mdfcr, major, 0x1f, 0, 8);

/* minor
 * Access: RO
 */
MLXSW_ITEM8(reg, mdfcr, minor, 0x23, 0, 8);

/* error_id
 * Access: RO
 */
MLXSW_ITEM8(reg, mdfcr, error_id, 0x26, 3, 5);

/* mtecr
 * -----
 *
 */
/* last_sensor
 * Access: RO
 */
MLXSW_ITEM16(reg, mtecr, last_sensor, 0x0, 0, 12);

/* sensor_count
 * Access: RO
 */
MLXSW_ITEM16(reg, mtecr, sensor_count, 0x2, 0, 12);

/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mtecr, slot_index, 0x4, 4, 4);

/* internal_sensor_count
 * Access: RO
 */
MLXSW_ITEM8(reg, mtecr, internal_sensor_count, 0x7, 0, 7);

/* sensor_map
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mtecr, sensor_map, 0x8, 0, 32, 4, 0, 4, 0);

/* mtcap
 * -----
 *
 */
/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mtcap, slot_index, 0x1, 0, 4);

/* sensor_count
 * Access: RO
 */
MLXSW_ITEM8(reg, mtcap, sensor_count, 0x3, 0, 7);

/* internal_sensor_count
 * Access: RO
 */
MLXSW_ITEM8(reg, mtcap, internal_sensor_count, 0x7, 0, 8);

/* sensor_map
 * Access: RO
 */
MLXSW_ITEM64(reg, mtcap, sensor_map, 0x8, 0, 64);

/* mtwe
 * ----
 *
 */
/* sensor_warning
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mtwe, sensor_warning, 0x0, 0, 32, 4, 0, 4, 0);

/* mtewe
 * -----
 *
 */
/* last_sensor
 * Access: RO
 */
MLXSW_ITEM16(reg, mtewe, last_sensor, 0x0, 0, 12);

/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mtewe, slot_index, 0x2, 4, 4);

/* sensor_count
 * Access: RO
 */
MLXSW_ITEM16(reg, mtewe, sensor_count, 0x2, 0, 12);

/* sensor_warning
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mtewe, sensor_warning, 0x4, 0, 32, 4, 0, 4, 0);

/* mtsde
 * -----
 *
 */
/* last_sensor
 * Access: RO
 */
MLXSW_ITEM16(reg, mtsde, last_sensor, 0x0, 0, 10);

/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mtsde, slot_index, 0x2, 4, 4);

/* sensor_count
 * Access: RO
 */
MLXSW_ITEM16(reg, mtsde, sensor_count, 0x2, 0, 10);

/* sensor_shut_down_map
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mtsde, sensor_shut_down_map, 0x4, 0, 32, 4, 0, 4, 0);

/* mvcap
 * -----
 *
 */
/* sensor_map
 * Access: RO
 */
MLXSW_ITEM64(reg, mvcap, sensor_map, 0x0, 0, 64);

/* sensor_map_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mvcap, sensor_map_type, 0x8, 4, 2);

/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mvcap, slot_index, 0xb, 0, 4);

/* mtmp
 * ----
 *
 */
/* i
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mtmp, i, 0x0, 7, 1);

/* ig
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mtmp, ig, 0x0, 5, 2);

/* asic_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mtmp, asic_index, 0x0, 1, 4);

/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mtmp, slot_index, 0x1, 0, 4);

/* sensor_index
 * Access: INDEX
 */
MLXSW_ITEM16(reg, mtmp, sensor_index, 0x2, 0, 12);

/* max_operational_temperature
 * Access: RO
 */
MLXSW_ITEM16(reg, mtmp, max_operational_temperature, 0x4, 0, 16);

/* temperature
 * Access: RO
 */
MLXSW_ITEM16(reg, mtmp, temperature, 0x6, 0, 16);

/* mte
 * Access: RW
 */
MLXSW_ITEM8(reg, mtmp, mte, 0x8, 7, 1);

/* mtr
 * Access: OP
 */
MLXSW_ITEM8(reg, mtmp, mtr, 0x8, 6, 1);

/* weme
 * Access: OP
 */
MLXSW_ITEM8(reg, mtmp, weme, 0x8, 5, 1);

/* sdme
 * Access: OP
 */
MLXSW_ITEM8(reg, mtmp, sdme, 0x8, 4, 1);

/* max_temperature
 * Access: RO
 */
MLXSW_ITEM16(reg, mtmp, max_temperature, 0xa, 0, 16);

/* tee
 * Access: RW
 */
MLXSW_ITEM8(reg, mtmp, tee, 0xc, 6, 2);

/* sdee
 * Access: RW
 */
MLXSW_ITEM8(reg, mtmp, sdee, 0xc, 4, 2);

/* temperature_threshold_hi
 * Access: RW
 */
MLXSW_ITEM16(reg, mtmp, temperature_threshold_hi, 0xe, 0, 16);

/* temperature_threshold_lo
 * Access: RW
 */
MLXSW_ITEM16(reg, mtmp, temperature_threshold_lo, 0x12, 0, 16);

/* temperature_critical_threshold_hi
 * Access: RW
 */
MLXSW_ITEM16(reg, mtmp, temperature_critical_threshold_hi, 0x14, 0, 16);

/* temperature_critical_threshold_low
 * Access: RW
 */
MLXSW_ITEM16(reg, mtmp, temperature_critical_threshold_low, 0x16, 0, 16);

/* sensor_name_hi
 * Access: RO
 */
MLXSW_ITEM32(reg, mtmp, sensor_name_hi, 0x18, 0, 32);

/* sensor_name_lo
 * Access: RO
 */
MLXSW_ITEM32(reg, mtmp, sensor_name_lo, 0x1c, 0, 32);

/* mvcr
 * ----
 *
 */
/* i
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mvcr, i, 0x0, 7, 1);

/* volt_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, mvcr, volt_cap, 0x0, 6, 1);

/* curr_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, mvcr, curr_cap, 0x0, 5, 1);

/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mvcr, slot_index, 0x1, 0, 4);

/* sensor_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mvcr, sensor_index, 0x3, 0, 8);

/* voltage_sensor_value
 * Access: RO
 */
MLXSW_ITEM16(reg, mvcr, voltage_sensor_value, 0x6, 0, 16);

/* current_sensor_value_msb
 * Access: RO
 */
MLXSW_ITEM16(reg, mvcr, current_sensor_value_msb, 0x8, 0, 16);

/* current_sensor_value
 * Access: RO
 */
MLXSW_ITEM16(reg, mvcr, current_sensor_value, 0xa, 0, 16);

/* sensor_name
 * Access: RO
 */
MLXSW_ITEM64(reg, mvcr, sensor_name, 0x10, 0, 64);

/* mpscr
 * -----
 *
 */
/* warning_active_time
 * Access: RW
 */
MLXSW_ITEM8(reg, mpscr, warning_active_time, 0x5, 0, 8);

/* warning_inactive_time
 * Access: RW
 */
MLXSW_ITEM8(reg, mpscr, warning_inactive_time, 0x7, 0, 8);

/* critical_active_time
 * Access: RW
 */
MLXSW_ITEM8(reg, mpscr, critical_active_time, 0x9, 0, 8);

/* critical_inactive_time
 * Access: RW
 */
MLXSW_ITEM8(reg, mpscr, critical_inactive_time, 0xb, 0, 8);

/* l0_all_queues_are_import
 * Access: RW
 */
MLXSW_ITEM8(reg, mpscr, l0_all_queues_are_import, 0xf, 0, 1);

/* cc
 * Access: OP
 */
MLXSW_ITEM8(reg, mpscr, cc, 0x13, 0, 1);

/* queue_depth_th
 * Access: RW
 */
MLXSW_ITEM16(reg, mpscr, queue_depth_th, 0x16, 0, 16);

/* mppgcr
 * ------
 *
 */
/* persistent_mode
 * Access: RO
 */
MLXSW_ITEM8(reg, mppgcr, persistent_mode, 0x0, 6, 1);

/* phe
 * Access: RW
 */
MLXSW_ITEM8(reg, mppgcr, phe, 0x3, 2, 2);

/* e
 * Access: RW
 */
MLXSW_ITEM8(reg, mppgcr, e, 0x3, 0, 2);

/* next_profile_id
 * Access: RW
 */
MLXSW_ITEM8(reg, mppgcr, next_profile_id, 0x5, 0, 8);

/* curr_profile_id
 * Access: RO
 */
MLXSW_ITEM8(reg, mppgcr, curr_profile_id, 0x7, 0, 8);

/* idle_power_a_factor
 * Access: RW
 */
MLXSW_ITEM16(reg, mppgcr, idle_power_a_factor, 0xa, 0, 16);

/* idle_power_b_factor
 * Access: RW
 */
MLXSW_ITEM16(reg, mppgcr, idle_power_b_factor, 0xe, 0, 16);

/* port_power_factor
 * Access: RW
 */
MLXSW_ITEM16(reg, mppgcr, port_power_factor, 0x12, 0, 16);

/* active_power_factor
 * Access: RW
 */
MLXSW_ITEM16(reg, mppgcr, active_power_factor, 0x16, 0, 16);

/* port_peak_c_dyn
 * Access: RW
 */
MLXSW_ITEM16(reg, mppgcr, port_peak_c_dyn, 0x1a, 0, 16);

/* shaper_adj_num_of_iter
 * Access: RW
 */
MLXSW_ITEM8(reg, mppgcr, shaper_adj_num_of_iter, 0x1d, 0, 8);

/* sampling_time
 * Access: RW
 */
MLXSW_ITEM8(reg, mppgcr, sampling_time, 0x1f, 0, 8);

/* stc_efficiency
 * Access: RW
 */
MLXSW_ITEM8(reg, mppgcr, stc_efficiency, 0x21, 0, 8);

/* min_shaper
 * Access: RW
 */
MLXSW_ITEM8(reg, mppgcr, min_shaper, 0x23, 0, 8);

/* mppcr
 * -----
 *
 */
/* v
 * Access: RW
 */
MLXSW_ITEM8(reg, mppcr, v, 0x0, 7, 1);

/* profile_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mppcr, profile_id, 0x3, 0, 8);

/* param_b
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, mppcr, param_b, 0x4, 4);

/* profile_name
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, mppcr, profile_name, 0x8, 20);

/* profile_config_v
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, mppcr, profile_config_v, MLXSW_MPPCR_PROFILE_CONFIG_BASE_LEN, 6, 1, MLXSW_MPPCR_PROFILE_CONFIG_REC_LEN, 0, 1, 0);

/* profile_config_power_allocation
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, mppcr, profile_config_power_allocation, MLXSW_MPPCR_PROFILE_CONFIG_BASE_LEN, 0, 11, MLXSW_MPPCR_PROFILE_CONFIG_REC_LEN, 0, 2, 0);

/* profile_config_max_integral
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, mppcr, profile_config_max_integral, MLXSW_MPPCR_PROFILE_CONFIG_BASE_LEN + 2, 0, 16, MLXSW_MPPCR_PROFILE_CONFIG_REC_LEN, 0, 2, 0);

/* profile_config_avg_pwr_num_of_sampling
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, mppcr, profile_config_avg_pwr_num_of_sampling, MLXSW_MPPCR_PROFILE_CONFIG_BASE_LEN, 0, 8, MLXSW_MPPCR_PROFILE_CONFIG_REC_LEN, 4, 1, 0);

/* profile_config_kp_factor
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, mppcr, profile_config_kp_factor, MLXSW_MPPCR_PROFILE_CONFIG_BASE_LEN + 1, 0, 8, MLXSW_MPPCR_PROFILE_CONFIG_REC_LEN, 4, 1, 0);

/* profile_config_ki_factor
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, mppcr, profile_config_ki_factor, MLXSW_MPPCR_PROFILE_CONFIG_BASE_LEN + 2, 0, 8, MLXSW_MPPCR_PROFILE_CONFIG_REC_LEN, 4, 1, 0);

/* profile_config_kd_factor
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, mppcr, profile_config_kd_factor, MLXSW_MPPCR_PROFILE_CONFIG_BASE_LEN + 3, 0, 8, MLXSW_MPPCR_PROFILE_CONFIG_REC_LEN, 4, 1, 0);

/* profile_config_pid_update_num_of_sampling
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, mppcr, profile_config_pid_update_num_of_sampling, MLXSW_MPPCR_PROFILE_CONFIG_BASE_LEN + 3, 0, 8, MLXSW_MPPCR_PROFILE_CONFIG_REC_LEN, 12, 1, 0);

/* mppgh
 * -----
 *
 */
/* bin_unit_time_measure
 * Access: RO
 */
MLXSW_ITEM16(reg, mppgh, bin_unit_time_measure, 0x6, 0, 16);

/* hist_reset_counter
 * Access: RO
 */
MLXSW_ITEM8(reg, mppgh, hist_reset_counter, 0xb, 0, 8);

/* no_shaper_bin
 * Access: RO
 */
MLXSW_ITEM32(reg, mppgh, no_shaper_bin, 0xc, 0, 32);

/* shaper_bin
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mppgh, shaper_bin, 0x10, 0, 32, 4, 0, 4, 0);

/* mpghr
 * -----
 *
 */
/* short_bin_unit_time_measure
 * Access: RO
 */
MLXSW_ITEM16(reg, mpghr, short_bin_unit_time_measure, 0x0, 0, 16);

/* bin_size
 * Access: RO
 */
MLXSW_ITEM8(reg, mpghr, bin_size, 0x2, 0, 8);

/* hist_reset_counter
 * Access: RO
 */
MLXSW_ITEM8(reg, mpghr, hist_reset_counter, 0x3, 0, 8);

/* long_bin_unit_time_measure
 * Access: RO
 */
MLXSW_ITEM16(reg, mpghr, long_bin_unit_time_measure, 0x4, 0, 16);

/* hist_min_value
 * Access: RW
 */
MLXSW_ITEM16(reg, mpghr, hist_min_value, 0x6, 0, 16);

/* short_pwr_avg_bin
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mpghr, short_pwr_avg_bin, 0x10, 0, 32, 4, 0, 4, 0);

/* long_pwr_avg_bin
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mpghr, long_pwr_avg_bin, 0x50, 0, 32, 4, 0, 4, 0);

/* mppgs
 * -----
 *
 */
/* op
 * Access: OP
 */
MLXSW_ITEM8(reg, mppgs, op, 0x3, 0, 2);

/* critical_power_cnt
 * Access: RO
 */
MLXSW_ITEM16(reg, mppgs, critical_power_cnt, 0x6, 0, 16);

/* short_win_power_avg
 * Access: RO
 */
MLXSW_ITEM16(reg, mppgs, short_win_power_avg, 0xa, 0, 16);

/* long_win_power_avg
 * Access: RO
 */
MLXSW_ITEM16(reg, mppgs, long_win_power_avg, 0xe, 0, 16);

/* mrsrft
 * ------
 *
 */
/* state_perst
 * Access: RW
 */
MLXSW_ITEM8(reg, mrsrft, state_perst, 0x2, 4, 4);

/* state_sld
 * Access: RW
 */
MLXSW_ITEM8(reg, mrsrft, state_sld, 0x2, 0, 4);

/* state_sbr
 * Access: RW
 */
MLXSW_ITEM8(reg, mrsrft, state_sbr, 0x3, 4, 4);

/* state_ld
 * Access: RW
 */
MLXSW_ITEM8(reg, mrsrft, state_ld, 0x3, 0, 4);

/* mcqi
 * ----
 *
 */
/* read_pending_component
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mcqi, read_pending_component, 0x0, 7, 1);

/* device_index
 * Access: INDEX
 */
MLXSW_ITEM16(reg, mcqi, device_index, 0x0, 0, 12);

/* component_index
 * Access: INDEX
 */
MLXSW_ITEM16(reg, mcqi, component_index, 0x2, 0, 16);

/* device_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mcqi, device_type, 0x7, 0, 8);

/* info_type
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, info_type, 0xb, 0, 5);

/* info_size
 * Access: RO
 */
MLXSW_ITEM32(reg, mcqi, info_size, 0xc, 0, 32);

/* offset
 * Access: RW
 */
MLXSW_ITEM32(reg, mcqi, offset, 0x10, 0, 32);

/* data_size
 * Access: RW
 */
MLXSW_ITEM16(reg, mcqi, data_size, 0x16, 0, 16);

/* supported_info_bitmask
 * Access: RO
 */
MLXSW_ITEM32(reg, mcqi, supported_info_bitmask, 0x18, 0, 32);

/* component_size
 * Access: RO
 */
MLXSW_ITEM32(reg, mcqi, component_size, 0x1c, 0, 32);

/* max_component_size
 * Access: RO
 */
MLXSW_ITEM32(reg, mcqi, max_component_size, 0x20, 0, 32);

/* log_mcda_word_size
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, log_mcda_word_size, 0x24, 4, 4);

/* mcda_max_write_size
 * Access: RO
 */
MLXSW_ITEM16(reg, mcqi, mcda_max_write_size, 0x26, 0, 16);

/* rd_en
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, rd_en, 0x28, 7, 1);

/* signed_updates_only
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, signed_updates_only, 0x28, 6, 1);

/* match_chip_id
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, match_chip_id, 0x28, 5, 1);

/* match_psid
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, match_psid, 0x28, 4, 1);

/* check_user_timestamp
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, check_user_timestamp, 0x28, 3, 1);

/* match_base_guid_mac
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, match_base_guid_mac, 0x28, 2, 1);

/* mcqi_version_build_time_valid
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, mcqi_version_build_time_valid, 0x18, 5, 1);

/* mcqi_version_user_defined_time_valid
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, mcqi_version_user_defined_time_valid, 0x18, 4, 1);

/* mcqi_version_version_string_length
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, mcqi_version_version_string_length, 0x1b, 0, 8);

/* mcqi_version_version
 * Access: RW
 */
MLXSW_ITEM32(reg, mcqi, mcqi_version_version, 0x1c, 0, 32);

/* mcqi_version_build_time_seconds
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, mcqi_version_build_time_seconds, 0x20, 0, 8);

/* mcqi_version_build_time_minutes
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, mcqi_version_build_time_minutes, 0x21, 0, 8);

/* mcqi_version_build_time_hours
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, mcqi_version_build_time_hours, 0x22, 0, 8);

/* mcqi_version_build_time_year
 * Access: RW
 */
MLXSW_ITEM16(reg, mcqi, mcqi_version_build_time_year, 0x24, 0, 16);

/* mcqi_version_build_time_month
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, mcqi_version_build_time_month, 0x26, 0, 8);

/* mcqi_version_build_time_day
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, mcqi_version_build_time_day, 0x27, 0, 8);

/* mcqi_version_user_defined_time_seconds
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, mcqi_version_user_defined_time_seconds, 0x28, 0, 8);

/* mcqi_version_user_defined_time_minutes
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, mcqi_version_user_defined_time_minutes, 0x29, 0, 8);

/* mcqi_version_user_defined_time_hours
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, mcqi_version_user_defined_time_hours, 0x2a, 0, 8);

/* mcqi_version_user_defined_time_year
 * Access: RW
 */
MLXSW_ITEM16(reg, mcqi, mcqi_version_user_defined_time_year, 0x2c, 0, 16);

/* mcqi_version_user_defined_time_month
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, mcqi_version_user_defined_time_month, 0x2e, 0, 8);

/* mcqi_version_user_defined_time_day
 * Access: RW
 */
MLXSW_ITEM8(reg, mcqi, mcqi_version_user_defined_time_day, 0x2f, 0, 8);

/* mcqi_version_build_tool_version
 * Access: RW
 */
MLXSW_ITEM32(reg, mcqi, mcqi_version_build_tool_version, 0x30, 0, 32);

/* mcqi_version_version_string
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, mcqi, mcqi_version_version_string, 0x38, 92);

/* self_activation
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, self_activation, 0x1b, 6, 1);

/* pending_server_ac_power_cycle
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, pending_server_ac_power_cycle, 0x1b, 5, 1);

/* pending_server_dc_power_cycle
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, pending_server_dc_power_cycle, 0x1b, 4, 1);

/* pending_server_reboot
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, pending_server_reboot, 0x1b, 3, 1);

/* pending_fw_reset
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, pending_fw_reset, 0x1b, 2, 1);

/* auto_activate
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, auto_activate, 0x1b, 1, 1);

/* all_hosts_sync
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, all_hosts_sync, 0x1b, 0, 1);

/* fw_image_info_bitmap
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, fw_image_info_bitmap, 0x19, 0, 8);

/* fw_image_status_bitmap
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, fw_image_status_bitmap, 0x1b, 0, 8);

/* image_a_major
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, image_a_major, 0x1e, 0, 8);

/* image_a_minor
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, image_a_minor, 0x1f, 0, 8);

/* image_a_subminor
 * Access: RO
 */
MLXSW_ITEM16(reg, mcqi, image_a_subminor, 0x22, 0, 16);

/* image_b_major
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, image_b_major, 0x26, 0, 8);

/* image_b_minor
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, image_b_minor, 0x27, 0, 8);

/* image_b_subminor
 * Access: RO
 */
MLXSW_ITEM16(reg, mcqi, image_b_subminor, 0x2a, 0, 16);

/* factory_image_major
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, factory_image_major, 0x2e, 0, 8);

/* factory_image_minor
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, factory_image_minor, 0x2f, 0, 8);

/* factory_image_subminor
 * Access: RO
 */
MLXSW_ITEM16(reg, mcqi, factory_image_subminor, 0x32, 0, 16);

/* activation_type
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, activation_type, 0x36, 2, 4);

/* management_interface_protocol
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, management_interface_protocol, 0x37, 0, 6);

/* vendor_sn
 * Access: RO
 */
MLXSW_ITEM16(reg, mcqi, vendor_sn, 0x3a, 0, 16);

/* vendor_id
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, vendor_id, 0x19, 0, 8);

/* image_version_major
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, image_version_major, 0x1b, 4, 4);

/* image_version_minor
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, image_version_minor, 0x1b, 0, 4);

/* mcqi_dpa_apps_info_version
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, mcqi_dpa_apps_info_version, 0x18, 0, 8);

/* mcqi_dpa_apps_info_total_number_of_entries
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, mcqi_dpa_apps_info_total_number_of_entries, 0x19, 0, 8);

/* mcqi_dpa_apps_info_dpa_app_metadata_type
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, mcqi_dpa_apps_info_dpa_app_metadata_type, 0x20, 0, 8);

/* mcqi_dpa_apps_info_dpa_app_metadata_subtype
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, mcqi_dpa_apps_info_dpa_app_metadata_subtype, 0x23, 0, 8);

/* mcqi_dpa_apps_info_dpa_app_metadata_version
 * Access: RO
 */
MLXSW_ITEM8(reg, mcqi, mcqi_dpa_apps_info_dpa_app_metadata_version, 0x27, 0, 8);

/* mcqi_dpa_apps_info_dpa_app_metadata_length
 * Access: RO
 */
MLXSW_ITEM16(reg, mcqi, mcqi_dpa_apps_info_dpa_app_metadata_length, 0x2a, 0, 16);

/* mcqi_dpa_apps_info_dpa_app_metadata_dpa_app_version
 * Access: RO
 */
MLXSW_ITEM32(reg, mcqi, mcqi_dpa_apps_info_dpa_app_metadata_dpa_app_version, 0x30, 0, 32);

/* mcqi_dpa_apps_info_dpa_app_metadata_dpa_os_api_version
 * Access: RO
 */
MLXSW_ITEM32(reg, mcqi, mcqi_dpa_apps_info_dpa_app_metadata_dpa_os_api_version, 0x34, 0, 32);

/* mcqi_dpa_apps_info_dpa_app_metadata_dpa_fw_api_version
 * Access: RO
 */
MLXSW_ITEM32(reg, mcqi, mcqi_dpa_apps_info_dpa_app_metadata_dpa_fw_api_version, 0x3c, 0, 32);

/* mcqi_dpa_apps_info_dpa_app_metadata_dpa_app_uuid
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mcqi, mcqi_dpa_apps_info_dpa_app_metadata_dpa_app_uuid, 0x48, 0, 32, 4, 0, 4, 0);

/* mcqi_dpa_apps_info_dpa_app_metadata_dpa_app_description_string
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mcqi, mcqi_dpa_apps_info_dpa_app_metadata_dpa_app_description_string, 0x60, 0, 32, 4, 0, 4, 0);

/* mcc
 * ---
 *
 */
/* time_elapsed_since_last_cmd
 * Access: RO
 */
MLXSW_ITEM16(reg, mcc, time_elapsed_since_last_cmd, 0x0, 0, 12);

/* activation_delay_sec
 * Access: RW
 */
MLXSW_ITEM8(reg, mcc, activation_delay_sec, 0x2, 0, 8);

/* instruction
 * Access: RW
 */
MLXSW_ITEM8(reg, mcc, instruction, 0x3, 0, 8);

/* component_index
 * Access: INDEX
 */
MLXSW_ITEM16(reg, mcc, component_index, 0x6, 0, 16);

/* auto_update
 * Access: WO
 */
MLXSW_ITEM8(reg, mcc, auto_update, 0x8, 7, 1);

/* update_handle
 * Access: RW
 */
MLXSW_ITEM32(reg, mcc, update_handle, 0x8, 0, 24);

/* handle_owner_type
 * Access: RO
 */
MLXSW_ITEM8(reg, mcc, handle_owner_type, 0xc, 4, 4);

/* handle_owner_host_id
 * Access: RO
 */
MLXSW_ITEM8(reg, mcc, handle_owner_host_id, 0xc, 0, 4);

/* control_progress
 * Access: RO
 */
MLXSW_ITEM8(reg, mcc, control_progress, 0xd, 0, 7);

/* error_code
 * Access: RO
 */
MLXSW_ITEM8(reg, mcc, error_code, 0xe, 0, 8);

/* control_state
 * Access: RO
 */
MLXSW_ITEM8(reg, mcc, control_state, 0xf, 0, 4);

/* component_size
 * Access: WO
 */
MLXSW_ITEM32(reg, mcc, component_size, 0x10, 0, 32);

/* device_index
 * Access: INDEX
 */
MLXSW_ITEM16(reg, mcc, device_index, 0x14, 0, 12);

/* warning_code
 * Access: RO
 */
MLXSW_ITEM8(reg, mcc, warning_code, 0x16, 0, 8);

/* device_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mcc, device_type, 0x17, 0, 8);

/* rejected_device_index
 * Access: RO
 */
MLXSW_ITEM16(reg, mcc, rejected_device_index, 0x18, 0, 12);

/* device_index_size
 * Access: RW
 */
MLXSW_ITEM16(reg, mcc, device_index_size, 0x1a, 0, 12);

/* component_specific_err_code
 * Access: RO
 */
MLXSW_ITEM32(reg, mcc, component_specific_err_code, 0x1c, 0, 32);

/* mcda
 * ----
 *
 */
/* update_handle
 * Access: RW
 */
MLXSW_ITEM32(reg, mcda, update_handle, 0x0, 0, 24);

/* offset
 * Access: RW
 */
MLXSW_ITEM32(reg, mcda, offset, 0x4, 0, 32);

/* size
 * Access: RW
 */
MLXSW_ITEM16(reg, mcda, size, 0xa, 0, 16);

/* data
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mcda, data, 0x10, 0, 32, 4, 0, 4, 0);

/* mgir
 * ----
 *
 */
/* device_hw_revision
 * Access: RO
 */
MLXSW_ITEM16(reg, mgir, device_hw_revision, 0x0, 0, 16);

/* device_id
 * Access: RO
 */
MLXSW_ITEM16(reg, mgir, device_id, 0x2, 0, 16);

/* ib_mad_gen
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, ib_mad_gen, 0x4, 4, 3);

/* num_ports
 * Access: RO
 */
MLXSW_ITEM16(reg, mgir, num_ports, 0x4, 0, 12);

/* technology
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, technology, 0x6, 3, 5);

/* pvs
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, pvs, 0x7, 0, 5);

/* hw_dev_id
 * Access: RO
 */
MLXSW_ITEM16(reg, mgir, hw_dev_id, 0xa, 0, 16);

/* development
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, development, 0xc, 6, 1);

/* ga_valid
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, ga_valid, 0xc, 4, 1);

/* cpo_indication
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, cpo_indication, 0xc, 3, 1);

/* module_master_fw_default
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, module_master_fw_default, 0xd, 0, 2);

/* chip_type
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, chip_type, 0x10, 0, 4);

/* ga
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, ga, 0x11, 0, 6);

/* manufacturing_base_mac
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, mgir, manufacturing_base_mac, 0x12, 6);

/* device_ticks_per_msec
 * Access: RO
 */
MLXSW_ITEM32(reg, mgir, device_ticks_per_msec, 0x18, 0, 32);

/* uptime
 * Access: RO
 */
MLXSW_ITEM32(reg, mgir, uptime, 0x1c, 0, 32);

/* fw_info_dev_sc
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_dev_sc, 0x20, 6, 1);

/* fw_info_latency_tlv
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_latency_tlv, 0x20, 5, 1);

/* fw_info_string_tlv
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_string_tlv, 0x20, 4, 1);

/* fw_info_dev
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_dev, 0x20, 3, 1);

/* fw_info_debug
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_debug, 0x20, 2, 1);

/* fw_info_signed_fw
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_signed_fw, 0x20, 1, 1);

/* fw_info_secured
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_secured, 0x20, 0, 1);

/* fw_info_major
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_major, 0x21, 0, 8);

/* fw_info_minor
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_minor, 0x22, 0, 8);

/* fw_info_sub_minor
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_sub_minor, 0x23, 0, 8);

/* fw_info_build_id
 * Access: RO
 */
MLXSW_ITEM32(reg, mgir, fw_info_build_id, 0x24, 0, 32);

/* fw_info_month
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_month, 0x28, 0, 8);

/* fw_info_day
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_day, 0x29, 0, 8);

/* fw_info_year
 * Access: RO
 */
MLXSW_ITEM16(reg, mgir, fw_info_year, 0x2a, 0, 16);

/* fw_info_hour
 * Access: RO
 */
MLXSW_ITEM16(reg, mgir, fw_info_hour, 0x2e, 0, 16);

/* fw_info_psid
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, mgir, fw_info_psid, 0x30, 16);

/* fw_info_ini_file_version
 * Access: RO
 */
MLXSW_ITEM32(reg, mgir, fw_info_ini_file_version, 0x40, 0, 32);

/* fw_info_extended_major
 * Access: RO
 */
MLXSW_ITEM32(reg, mgir, fw_info_extended_major, 0x44, 0, 32);

/* fw_info_extended_minor
 * Access: RO
 */
MLXSW_ITEM32(reg, mgir, fw_info_extended_minor, 0x48, 0, 32);

/* fw_info_extended_sub_minor
 * Access: RO
 */
MLXSW_ITEM32(reg, mgir, fw_info_extended_sub_minor, 0x4c, 0, 32);

/* fw_info_disabled_tiles_bitmap
 * Access: RO
 */
MLXSW_ITEM16(reg, mgir, fw_info_disabled_tiles_bitmap, 0x50, 0, 16);

/* fw_info_isfu_major
 * Access: RO
 */
MLXSW_ITEM16(reg, mgir, fw_info_isfu_major, 0x52, 0, 16);

/* fw_info_dev_counter
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_dev_counter, 0x55, 0, 5);

/* fw_info_pds
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_pds, 0x56, 6, 1);

/* fw_info_issu_able
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_issu_able, 0x56, 5, 1);

/* fw_info_dev_state
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_dev_state, 0x56, 3, 1);

/* fw_info_life_cycle_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_life_cycle_msb, 0x57, 4, 3);

/* fw_info_encryption
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_encryption, 0x57, 3, 1);

/* fw_info_sec_boot
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_sec_boot, 0x57, 2, 1);

/* fw_info_life_cycle
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, fw_info_life_cycle, 0x57, 0, 2);

/* sw_info_major
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_major, 0x61, 0, 8);

/* sw_info_minor
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_minor, 0x62, 0, 8);

/* sw_info_sub_minor
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_sub_minor, 0x63, 0, 8);

/* sw_info_rom0_arch
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_rom0_arch, 0x64, 4, 4);

/* sw_info_rom0_type
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_rom0_type, 0x64, 0, 4);

/* sw_info_rom1_arch
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_rom1_arch, 0x65, 4, 4);

/* sw_info_rom1_type
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_rom1_type, 0x65, 0, 4);

/* sw_info_rom2_arch
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_rom2_arch, 0x66, 4, 4);

/* sw_info_rom2_type
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_rom2_type, 0x66, 0, 4);

/* sw_info_rom3_arch
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_rom3_arch, 0x67, 4, 4);

/* sw_info_rom3_type
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_rom3_type, 0x67, 0, 4);

/* sw_info_rom0_version_major
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_rom0_version_major, 0x68, 0, 8);

/* sw_info_rom0_version_minor
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_rom0_version_minor, 0x69, 0, 8);

/* sw_info_rom0_version_build
 * Access: RO
 */
MLXSW_ITEM16(reg, mgir, sw_info_rom0_version_build, 0x6a, 0, 16);

/* sw_info_rom1_version_major
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_rom1_version_major, 0x6c, 0, 8);

/* sw_info_rom1_version_minor
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_rom1_version_minor, 0x6d, 0, 8);

/* sw_info_rom1_version_build
 * Access: RO
 */
MLXSW_ITEM16(reg, mgir, sw_info_rom1_version_build, 0x6e, 0, 16);

/* sw_info_rom2_version_major
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_rom2_version_major, 0x70, 0, 8);

/* sw_info_rom2_version_minor
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_rom2_version_minor, 0x71, 0, 8);

/* sw_info_rom2_version_build
 * Access: RO
 */
MLXSW_ITEM16(reg, mgir, sw_info_rom2_version_build, 0x72, 0, 16);

/* sw_info_rom3_version_major
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_rom3_version_major, 0x74, 0, 8);

/* sw_info_rom3_version_minor
 * Access: RO
 */
MLXSW_ITEM8(reg, mgir, sw_info_rom3_version_minor, 0x75, 0, 8);

/* sw_info_rom3_version_build
 * Access: RO
 */
MLXSW_ITEM16(reg, mgir, sw_info_rom3_version_build, 0x76, 0, 16);

/* dev_branch_tag
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, mgir, dev_branch_tag, 0x80, 28);

/* msgi
 * ----
 *
 */
/* serial_number
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, msgi, serial_number, 0x0, 0, 32, 4, 0, 4, 0);

/* part_number
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, msgi, part_number, 0x20, 0, 32, 4, 0, 4, 0);

/* revision
 * Access: RO
 */
MLXSW_ITEM32(reg, msgi, revision, 0x38, 0, 32);

/* manufacturing_date
 * Access: RO
 */
MLXSW_ITEM32(reg, msgi, manufacturing_date, 0x3c, 0, 32);

/* product_name
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, msgi, product_name, 0x40, 0, 32, 4, 0, 4, 0);

/* mdir
 * ----
 *
 */
/* device_id
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mdir, device_id, 0x0, 0, 32, 4, 0, 4, 0);

/* mini
 * ----
 *
 */
/* index
 * Access: INDEX
 */
MLXSW_ITEM16(reg, mini, index, 0x0, 0, 16);

/* type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mini, type, 0x3, 0, 8);

/* valid
 * Access: RO
 */
MLXSW_ITEM8(reg, mini, valid, 0x7, 0, 1);

/* module_gw_num
 * Access: RO
 */
MLXSW_ITEM8(reg, mini, module_gw_num, 0x13, 0, 8);

/* spl
 * Access: RO
 */
MLXSW_ITEM8(reg, mini, spl, 0x13, 0, 8);

/* fused_out_ports_bitmap
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mini, fused_out_ports_bitmap, 0x10, 0, 32, 4, 0, 4, 0);

/* mcam
 * ----
 *
 */
/* feature_group
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mcam, feature_group, 0x1, 0, 8);

/* access_reg_group
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mcam, access_reg_group, 0x3, 0, 8);

/* mng_access_reg_cap_mask
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mcam, mng_access_reg_cap_mask, 0x8, 0, 32, 4, 0, 4, 0);

/* mng_feature_cap_mask
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mcam, mng_feature_cap_mask, 0x28, 0, 32, 4, 0, 4, 0);

/* meccc
 * -----
 *
 */
/* device_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, meccc, device_index, 0x1, 0, 8);

/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, meccc, slot_index, 0x3, 0, 4);

/* clr
 * Access: OP
 */
MLXSW_ITEM8(reg, meccc, clr, 0x7, 0, 1);

/* ecc_crt_cnt
 * Access: RO
 */
MLXSW_ITEM32(reg, meccc, ecc_crt_cnt, 0xc, 0, 32);

/* ecc_ucrt_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, meccc, ecc_ucrt_cnt, 0x13, 0, 8);

/* mmam
 * ----
 *
 */
/* module
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mmam, module, 0x1, 0, 8);

/* ga
 * Access: RO
 */
MLXSW_ITEM8(reg, mmam, ga, 0x7, 0, 4);

/* local_module
 * Access: RO
 */
MLXSW_ITEM8(reg, mmam, local_module, 0xb, 0, 8);

/* module_type
 * Access: RO
 */
MLXSW_ITEM8(reg, mmam, module_type, 0xf, 0, 8);

/* mcgi
 * ----
 *
 */
/* tray_index
 * Access: RO
 */
MLXSW_ITEM8(reg, mcgi, tray_index, 0x1, 0, 8);

/* chassis_slot_index
 * Access: RO
 */
MLXSW_ITEM8(reg, mcgi, chassis_slot_index, 0x3, 0, 8);

/* chassis_id
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mcgi, chassis_id, 0x10, 0, 32, 4, 0, 4, 0);

/* chassis_topology_id
 * Access: RO
 */
MLXSW_ITEM8(reg, mcgi, chassis_topology_id, 0x33, 0, 8);

/* mbct
 * ----
 *
 */
/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mbct, slot_index, 0x3, 0, 4);

/* ini_status
 * Access: RO
 */
MLXSW_ITEM8(reg, mbct, ini_status, 0x4, 5, 3);

/* data_size
 * Access: WO
 */
MLXSW_ITEM16(reg, mbct, data_size, 0x6, 0, 11);

/* op
 * Access: WO
 */
MLXSW_ITEM8(reg, mbct, op, 0x8, 4, 4);

/* last
 * Access: WO
 */
MLXSW_ITEM8(reg, mbct, last, 0x8, 2, 1);

/* oee
 * Access: WO
 */
MLXSW_ITEM8(reg, mbct, oee, 0x8, 1, 1);

/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, mbct, status, 0xc, 0, 5);

/* fsm_state
 * Access: RO
 */
MLXSW_ITEM8(reg, mbct, fsm_state, 0xd, 0, 4);

/* data
 * Access: WO
 */
MLXSW_ITEM32_INDEXED(reg, mbct, data, 0x20, 0, 32, 4, 0, 4, 0);

/* mpdc
 * ----
 *
 */
/* clr_err_cnt
 * Access: OP
 */
MLXSW_ITEM8(reg, mpdc, clr_err_cnt, 0x0, 7, 1);

/* allocation_error_counter
 * Access: RO
 */
MLXSW_ITEM32(reg, mpdc, allocation_error_counter, 0x8, 0, 32);

/* authentication_error_counter
 * Access: RO
 */
MLXSW_ITEM32(reg, mpdc, authentication_error_counter, 0xc, 0, 32);

/* mtcq
 * ----
 *
 */
/* token_opcode
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mtcq, token_opcode, 0x0, 0, 8);

/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, mtcq, status, 0x1, 0, 8);

/* device_index
 * Access: INDEX
 */
MLXSW_ITEM16(reg, mtcq, device_index, 0x2, 0, 12);

/* keypair_uuid
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mtcq, keypair_uuid, 0x4, 0, 32, 4, 0, 4, 0);

/* base_mac
 * Access: RO
 */
MLXSW_ITEM64(reg, mtcq, base_mac, 0x14, 0, 64);

/* psid
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mtcq, psid, 0x1c, 0, 32, 4, 0, 4, 0);

/* fw_version
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, mtcq, fw_version, 0x2f, 5);

/* source_address
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mtcq, source_address, 0x34, 0, 32, 4, 0, 4, 0);

/* challenge_version
 * Access: RO
 */
MLXSW_ITEM8(reg, mtcq, challenge_version, 0x44, 0, 8);

/* session_id
 * Access: RO
 */
MLXSW_ITEM16(reg, mtcq, session_id, 0x46, 0, 16);

/* challenge
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mtcq, challenge, 0x48, 0, 32, 4, 0, 4, 0);

/* mfcdr
 * -----
 *
 */
/* query_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mfcdr, query_type, 0x0, 7, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mfcdr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mfcdr, lp_msb, 0x2, 4, 2);

/* module
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mfcdr, module, 0x3, 0, 8);

/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, mfcdr, status, 0x7, 0, 2);

/* msgcr
 * -----
 *
 */
/* fake_cable_detect_en
 * Access: RW
 */
MLXSW_ITEM8(reg, msgcr, fake_cable_detect_en, 0x3, 0, 1);

/* support_error_injection
 * Access: RO
 */
MLXSW_ITEM8(reg, msgcr, support_error_injection, 0x6, 0, 1);

/* support_inter_asic_block
 * Access: RO
 */
MLXSW_ITEM8(reg, msgcr, support_inter_asic_block, 0x7, 1, 1);

/* support_perst_gpio
 * Access: RO
 */
MLXSW_ITEM8(reg, msgcr, support_perst_gpio, 0x7, 0, 1);

/* mspmer
 * ------
 *
 */
/* device_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mspmer, device_index, 0x3, 0, 4);

/* prev_en
 * Access: RO
 */
MLXSW_ITEM8(reg, mspmer, prev_en, 0x4, 0, 1);

/* clr
 * Access: OP
 */
MLXSW_ITEM8(reg, mspmer, clr, 0x5, 0, 1);

/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, mspmer, status, 0x7, 0, 4);

/* supported_physical_monitor
 * Access: RO
 */
MLXSW_ITEM32(reg, mspmer, supported_physical_monitor, 0xc, 0, 32);

/* general_err
 * Access: RO
 */
MLXSW_ITEM8(reg, mspmer, general_err, 0x16, 4, 1);

/* scpm_ctr
 * Access: RO
 */
MLXSW_ITEM8(reg, mspmer, scpm_ctr, 0x16, 0, 4);

/* vmon_ctr
 * Access: RO
 */
MLXSW_ITEM8(reg, mspmer, vmon_ctr, 0x17, 4, 4);

/* fmon_ctr
 * Access: RO
 */
MLXSW_ITEM8(reg, mspmer, fmon_ctr, 0x17, 0, 4);

/* mtrc_cap
 * --------
 *
 */
/* trace_owner
 * Access: RW
 */
MLXSW_ITEM8(reg, mtrc_cap, trace_owner, 0x0, 7, 1);

/* trace_to_memory
 * Access: RO
 */
MLXSW_ITEM8(reg, mtrc_cap, trace_to_memory, 0x0, 6, 1);

/* trc_ver
 * Access: RO
 */
MLXSW_ITEM8(reg, mtrc_cap, trc_ver, 0x0, 0, 2);

/* num_string_db
 * Access: RO
 */
MLXSW_ITEM8(reg, mtrc_cap, num_string_db, 0x3, 0, 4);

/* first_string_trace
 * Access: RO
 */
MLXSW_ITEM8(reg, mtrc_cap, first_string_trace, 0x4, 0, 8);

/* num_string_trace
 * Access: RO
 */
MLXSW_ITEM8(reg, mtrc_cap, num_string_trace, 0x5, 0, 8);

/* tracer_capabilities
 * Access: RO
 */
MLXSW_ITEM8(reg, mtrc_cap, tracer_capabilities, 0x8, 6, 2);

/* log_max_trace_buffer_size
 * Access: RO
 */
MLXSW_ITEM8(reg, mtrc_cap, log_max_trace_buffer_size, 0xb, 0, 8);

/* string_db_base_address
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mtrc_cap, string_db_base_address, MLXSW_MTRC_CAP_STRING_DB_PARAM_BASE_LEN, 0, 32, MLXSW_MTRC_CAP_STRING_DB_PARAM_REC_LEN, 0, 4, 0);

/* string_db_size
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mtrc_cap, string_db_size, MLXSW_MTRC_CAP_STRING_DB_PARAM_BASE_LEN, 0, 24, MLXSW_MTRC_CAP_STRING_DB_PARAM_REC_LEN, 4, 3, 0);

/* mtrc_stdb_v2
 * ------------
 *
 */
/* string_db_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mtrc_stdb_v2, string_db_index, 0x0, 4, 4);

/* read_size
 * Access: INDEX
 */
MLXSW_ITEM32(reg, mtrc_stdb_v2, read_size, 0x0, 0, 24);

/* start_offset
 * Access: INDEX
 */
MLXSW_ITEM32(reg, mtrc_stdb_v2, start_offset, 0x4, 0, 32);

/* string_db_data
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mtrc_stdb_v2, string_db_data, 0x8, 0, 32, 4, 0, 4, 0);

/* mteim
 * -----
 *
 */
/* cap_num_of_tile
 * Access: RO
 */
MLXSW_ITEM8(reg, mteim, cap_num_of_tile, 0x0, 0, 8);

/* cap_core_dpa
 * Access: RO
 */
MLXSW_ITEM8(reg, mteim, cap_core_dpa, 0x1, 0, 8);

/* cap_core_main
 * Access: RO
 */
MLXSW_ITEM8(reg, mteim, cap_core_main, 0x2, 0, 8);

/* cap_core_tile
 * Access: RO
 */
MLXSW_ITEM8(reg, mteim, cap_core_tile, 0x3, 0, 8);

/* is_dwsn_msb_supported
 * Access: RO
 */
MLXSW_ITEM8(reg, mteim, is_dwsn_msb_supported, 0x4, 7, 1);

/* is_phy_uc_supported
 * Access: RO
 */
MLXSW_ITEM8(reg, mteim, is_phy_uc_supported, 0x4, 6, 1);

/* type_core_dpa
 * Access: RO
 */
MLXSW_ITEM8(reg, mteim, type_core_dpa, 0x6, 0, 4);

/* type_core_main
 * Access: RO
 */
MLXSW_ITEM8(reg, mteim, type_core_main, 0x7, 4, 4);

/* type_core_tile
 * Access: RO
 */
MLXSW_ITEM8(reg, mteim, type_core_tile, 0x7, 0, 4);

/* first_main_core_event_id
 * Access: RO
 */
MLXSW_ITEM8(reg, mteim, first_main_core_event_id, 0x8, 0, 8);

/* first_dpa_core_event_id
 * Access: RO
 */
MLXSW_ITEM8(reg, mteim, first_dpa_core_event_id, 0x9, 0, 8);

/* first_tile_core_event_id
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, mteim, first_tile_core_event_id, 0xc, 8);

/* mpcir
 * -----
 *
 */
/* all
 * Access: OP
 */
MLXSW_ITEM8(reg, mpcir, all, 0x0, 6, 2);

/* fw_critical_events
 * Access: OP
 */
MLXSW_ITEM8(reg, mpcir, fw_critical_events, 0x5, 4, 2);

/* mads
 * Access: OP
 */
MLXSW_ITEM8(reg, mpcir, mads, 0x5, 2, 2);

/* gearbox
 * Access: OP
 */
MLXSW_ITEM8(reg, mpcir, gearbox, 0x5, 0, 2);

/* leds
 * Access: OP
 */
MLXSW_ITEM8(reg, mpcir, leds, 0x6, 6, 2);

/* voltage_current
 * Access: OP
 */
MLXSW_ITEM8(reg, mpcir, voltage_current, 0x6, 4, 2);

/* power
 * Access: OP
 */
MLXSW_ITEM8(reg, mpcir, power, 0x6, 2, 2);

/* fans
 * Access: OP
 */
MLXSW_ITEM8(reg, mpcir, fans, 0x6, 0, 2);

/* thermal
 * Access: OP
 */
MLXSW_ITEM8(reg, mpcir, thermal, 0x7, 6, 2);

/* module_mng
 * Access: OP
 */
MLXSW_ITEM8(reg, mpcir, module_mng, 0x7, 4, 2);

/* cpld
 * Access: OP
 */
MLXSW_ITEM8(reg, mpcir, cpld, 0x7, 2, 2);

/* ports
 * Access: OP
 */
MLXSW_ITEM8(reg, mpcir, ports, 0x7, 0, 2);

/* fw_critical_events_stat
 * Access: RO
 */
MLXSW_ITEM8(reg, mpcir, fw_critical_events_stat, 0xd, 4, 2);

/* mads_stat
 * Access: RO
 */
MLXSW_ITEM8(reg, mpcir, mads_stat, 0xd, 2, 2);

/* gearbox_stat
 * Access: RO
 */
MLXSW_ITEM8(reg, mpcir, gearbox_stat, 0xd, 0, 2);

/* leds_stat
 * Access: RO
 */
MLXSW_ITEM8(reg, mpcir, leds_stat, 0xe, 6, 2);

/* voltage_current_stat
 * Access: RO
 */
MLXSW_ITEM8(reg, mpcir, voltage_current_stat, 0xe, 4, 2);

/* power_stat
 * Access: RO
 */
MLXSW_ITEM8(reg, mpcir, power_stat, 0xe, 2, 2);

/* fans_stat
 * Access: RO
 */
MLXSW_ITEM8(reg, mpcir, fans_stat, 0xe, 0, 2);

/* thermal_stat
 * Access: RO
 */
MLXSW_ITEM8(reg, mpcir, thermal_stat, 0xf, 6, 2);

/* module_mng_stat
 * Access: RO
 */
MLXSW_ITEM8(reg, mpcir, module_mng_stat, 0xf, 4, 2);

/* cpld_stat
 * Access: RO
 */
MLXSW_ITEM8(reg, mpcir, cpld_stat, 0xf, 2, 2);

/* ports_stat
 * Access: RO
 */
MLXSW_ITEM8(reg, mpcir, ports_stat, 0xf, 0, 2);

/* page_sel
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mpcir, page_sel, 0x13, 0, 4);

/* fsm_ps_0
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, mpcir, fsm_ps_0, MLXSW_MPCIR_UC_FSM_INFO_BASE_LEN, 5, 3, MLXSW_MPCIR_UC_FSM_INFO_REC_LEN, 0, 1, 0);

/* phy_uc_curr_idx_0
 * Access: RO
 */
MLXSW_ITEM16_INDEXED(reg, mpcir, phy_uc_curr_idx_0, MLXSW_MPCIR_UC_FSM_INFO_BASE_LEN, 0, 13, MLXSW_MPCIR_UC_FSM_INFO_REC_LEN, 0, 2, 0);

/* fsm_ps_1
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, mpcir, fsm_ps_1, MLXSW_MPCIR_UC_FSM_INFO_BASE_LEN + 2, 5, 3, MLXSW_MPCIR_UC_FSM_INFO_REC_LEN, 0, 1, 0);

/* phy_uc_curr_idx_1
 * Access: RO
 */
MLXSW_ITEM16_INDEXED(reg, mpcir, phy_uc_curr_idx_1, MLXSW_MPCIR_UC_FSM_INFO_BASE_LEN + 2, 0, 13, MLXSW_MPCIR_UC_FSM_INFO_REC_LEN, 0, 2, 0);

/* fsm_ps_2
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, mpcir, fsm_ps_2, MLXSW_MPCIR_UC_FSM_INFO_BASE_LEN, 5, 3, MLXSW_MPCIR_UC_FSM_INFO_REC_LEN, 4, 1, 0);

/* phy_uc_curr_idx_2
 * Access: RO
 */
MLXSW_ITEM16_INDEXED(reg, mpcir, phy_uc_curr_idx_2, MLXSW_MPCIR_UC_FSM_INFO_BASE_LEN, 0, 13, MLXSW_MPCIR_UC_FSM_INFO_REC_LEN, 4, 2, 0);

/* fsm_ps_3
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, mpcir, fsm_ps_3, MLXSW_MPCIR_UC_FSM_INFO_BASE_LEN + 2, 5, 3, MLXSW_MPCIR_UC_FSM_INFO_REC_LEN, 4, 1, 0);

/* phy_uc_curr_idx_3
 * Access: RO
 */
MLXSW_ITEM16_INDEXED(reg, mpcir, phy_uc_curr_idx_3, MLXSW_MPCIR_UC_FSM_INFO_BASE_LEN + 2, 0, 13, MLXSW_MPCIR_UC_FSM_INFO_REC_LEN, 4, 2, 0);

/* phy_serdes_if_state_full_tasks_mask_master
 * Access: RO
 */
MLXSW_ITEM32(reg, mpcir, phy_serdes_if_state_full_tasks_mask_master, 0x40, 0, 32);

/* phy_serdes_if_state_full_tasks_mask_slave
 * Access: RO
 */
MLXSW_ITEM32(reg, mpcir, phy_serdes_if_state_full_tasks_mask_slave, 0x44, 0, 32);

/* lane_curr_task_mask_stuck_0
 * Access: RO
 */
MLXSW_ITEM16(reg, mpcir, lane_curr_task_mask_stuck_0, 0x48, 0, 16);

/* lane_curr_task_mask_stuck_1
 * Access: RO
 */
MLXSW_ITEM16(reg, mpcir, lane_curr_task_mask_stuck_1, 0x4a, 0, 16);

/* lane_curr_task_mask_stuck_2
 * Access: RO
 */
MLXSW_ITEM16(reg, mpcir, lane_curr_task_mask_stuck_2, 0x4c, 0, 16);

/* lane_curr_task_mask_stuck_3
 * Access: RO
 */
MLXSW_ITEM16(reg, mpcir, lane_curr_task_mask_stuck_3, 0x4e, 0, 16);

/* jobs_mask
 * Access: RO
 */
MLXSW_ITEM16(reg, mpcir, jobs_mask, 0x52, 0, 16);

/* phy_fsm_ps_mstr
 * Access: RO
 */
MLXSW_ITEM_BIT_ARRAY(reg, mpcir, phy_fsm_ps_mstr, 0x54, 4, 4);

/* phy_fsm_ps_slv
 * Access: RO
 */
MLXSW_ITEM_BIT_ARRAY(reg, mpcir, phy_fsm_ps_slv, 0x58, 4, 4);

/* phy_serdes_if_state_generic_fsm_debug_tracer_master
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mpcir, phy_serdes_if_state_generic_fsm_debug_tracer_master, 0x5c, 0, 32, 4, 0, 4, 0);

/* phy_serdes_if_state_generic_fsm_debug_tracer_slave
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mpcir, phy_serdes_if_state_generic_fsm_debug_tracer_slave, 0x64, 0, 32, 4, 0, 4, 0);

/* phy_manager_generic_fsm_debug_tracer_master
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mpcir, phy_manager_generic_fsm_debug_tracer_master, 0x6c, 0, 32, 4, 0, 4, 0);

/* parallel_detect_state_generic_fsm_debug_tracer_master
 * Access: RO
 */
MLXSW_ITEM32(reg, mpcir, parallel_detect_state_generic_fsm_debug_tracer_master, 0x80, 0, 32);

/* phy_hst_generic_fsm_debug_tracer_master
 * Access: RO
 */
MLXSW_ITEM32(reg, mpcir, phy_hst_generic_fsm_debug_tracer_master, 0x88, 0, 32);

/* eth_an_state_generic_fsm_debug_tracer_master
 * Access: RO
 */
MLXSW_ITEM32(reg, mpcir, eth_an_state_generic_fsm_debug_tracer_master, 0x90, 0, 32);

/* tile_port
 * Access: RO
 */
MLXSW_ITEM32(reg, mpcir, tile_port, 0x94, 14, 7);

/* tile
 * Access: RO
 */
MLXSW_ITEM8(reg, mpcir, tile, 0x96, 3, 3);

/* port
 * Access: RO
 */
MLXSW_ITEM16(reg, mpcir, port, 0x96, 0, 11);

/* mfrc
 * ----
 *
 */
/* trigger
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mfrc, trigger, 0x0, 0, 8);

/* e
 * Access: RW
 */
MLXSW_ITEM8(reg, mfrc, e, 0x3, 0, 1);

/* capability_bits
 * Access: RO
 */
MLXSW_ITEM32(reg, mfrc, capability_bits, 0x4, 0, 32);

/* en_error_notification
 * Access: RW
 */
MLXSW_ITEM8(reg, mfrc, en_error_notification, 0x8, 7, 1);

/* en_warning_notification
 * Access: RW
 */
MLXSW_ITEM8(reg, mfrc, en_warning_notification, 0x8, 6, 1);

/* en_normal_notification
 * Access: RW
 */
MLXSW_ITEM8(reg, mfrc, en_normal_notification, 0x8, 5, 1);

/* mmcr
 * ----
 *
 */
/* next_module_control
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mmcr, next_module_control, 0x0, 0, 32, 4, 0, 4, 0);

/* module_control_mask
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mmcr, module_control_mask, 0x20, 0, 32, 4, 0, 4, 0);

/* curr_module_control
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mmcr, curr_module_control, 0x40, 0, 32, 4, 0, 4, 0);

/* mfri
 * ----
 *
 */
/* port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mfri, port, 0x3, 0, 8);

/* trigger
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mfri, trigger, 0x4, 0, 8);

/* thr_type
 * Access: RO
 */
MLXSW_ITEM8(reg, mfri, thr_type, 0x5, 0, 2);

/* field_select
 * Access: RO
 */
MLXSW_ITEM16(reg, mfri, field_select, 0x6, 0, 16);

/* fields_overflow
 * Access: RO
 */
MLXSW_ITEM16(reg, mfri, fields_overflow, 0xa, 0, 16);

/* num_errors
 * Access: RO
 */
MLXSW_ITEM32(reg, mfri, num_errors, 0xc, 0, 32);

/* num_warnings
 * Access: RO
 */
MLXSW_ITEM32(reg, mfri, num_warnings, 0x10, 0, 32);

/* num_normals
 * Access: RO
 */
MLXSW_ITEM32(reg, mfri, num_normals, 0x14, 0, 32);

/* min_value
 * Access: RO
 */
MLXSW_ITEM32(reg, mfri, min_value, 0x18, 0, 32);

/* max_value
 * Access: RO
 */
MLXSW_ITEM32(reg, mfri, max_value, 0x1c, 0, 32);

/* consecutive_normal
 * Access: RO
 */
MLXSW_ITEM32(reg, mfri, consecutive_normal, 0x20, 0, 32);

/* last_value
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mfri, last_value, 0x24, 0, 32, 4, 0, 4, 0);

/* mfrp
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mfrp, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mfrp, lp_msb, 0x2, 4, 2);

/* trigger
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mfrp, trigger, 0x3, 0, 8);

/* error_thr_action
 * Access: RO
 */
MLXSW_ITEM8(reg, mfrp, error_thr_action, 0x7, 4, 2);

/* en_thr
 * Access: RO
 */
MLXSW_ITEM8(reg, mfrp, en_thr, 0x7, 0, 2);

/* error_thr
 * Access: RO
 */
MLXSW_ITEM32(reg, mfrp, error_thr, 0x8, 0, 32);

/* warning_thr
 * Access: RO
 */
MLXSW_ITEM32(reg, mfrp, warning_thr, 0xc, 0, 32);

/* normal_thr
 * Access: RO
 */
MLXSW_ITEM32(reg, mfrp, normal_thr, 0x10, 0, 32);

/* time_window
 * Access: RO
 */
MLXSW_ITEM32(reg, mfrp, time_window, 0x14, 0, 32);

/* sampling_rate
 * Access: RO
 */
MLXSW_ITEM32(reg, mfrp, sampling_rate, 0x18, 0, 32);

/* missuv
 * ------
 *
 */
/* valid
 * Access: RO
 */
MLXSW_ITEM8(reg, missuv, valid, 0x0, 0, 1);

/* target_major
 * Access: RO
 */
MLXSW_ITEM32(reg, missuv, target_major, 0x10, 0, 32);

/* target_minor
 * Access: RO
 */
MLXSW_ITEM32(reg, missuv, target_minor, 0x14, 0, 32);

/* target_sub_minor
 * Access: RO
 */
MLXSW_ITEM32(reg, missuv, target_sub_minor, 0x18, 0, 32);

/* target_issu_major
 * Access: RO
 */
MLXSW_ITEM16(reg, missuv, target_issu_major, 0x1c, 0, 16);

/* target_issu_minor
 * Access: RO
 */
MLXSW_ITEM16(reg, missuv, target_issu_minor, 0x1e, 0, 16);

/* mogcr
 * -----
 *
 */
/* tele_managed
 * Access: RW
 */
MLXSW_ITEM8(reg, mogcr, tele_managed, 0x2, 0, 1);

/* ptp_iftc
 * Access: RW
 */
MLXSW_ITEM8(reg, mogcr, ptp_iftc, 0x3, 1, 1);

/* ptp_eftc
 * Access: RW
 */
MLXSW_ITEM8(reg, mogcr, ptp_eftc, 0x3, 0, 1);

/* sid
 * Access: RW
 */
MLXSW_ITEM8(reg, mogcr, sid, 0xb, 0, 1);

/* mirroring_pid_base
 * Access: RW
 */
MLXSW_ITEM16(reg, mogcr, mirroring_pid_base, 0xe, 0, 14);

/* mirror_latency_units
 * Access: RW
 */
MLXSW_ITEM16(reg, mogcr, mirror_latency_units, 0x16, 0, 16);

/* latency_histogram_units
 * Access: RW
 */
MLXSW_ITEM16(reg, mogcr, latency_histogram_units, 0x1a, 0, 16);

/* local_toc_fmt
 * Access: RW
 */
MLXSW_ITEM8(reg, mogcr, local_toc_fmt, 0x1c, 1, 1);

/* toc_fmt
 * Access: RW
 */
MLXSW_ITEM8(reg, mogcr, toc_fmt, 0x1c, 0, 1);

/* molp
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, molp, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, molp, lp_msb, 0x2, 4, 2);

/* label_port
 * Access: RW
 */
MLXSW_ITEM16(reg, molp, label_port, 0x6, 0, 16);

/* mprs
 * ----
 *
 */
/* en_l4_raw_prs
 * Access: RW
 */
MLXSW_ITEM8(reg, mprs, en_l4_raw_prs, 0x0, 7, 1);

/* prs_depth_dis
 * Access: OP
 */
MLXSW_ITEM8(reg, mprs, prs_depth_dis, 0x0, 6, 1);

/* ctipl_dis
 * Access: OP
 */
MLXSW_ITEM8(reg, mprs, ctipl_dis, 0x0, 5, 1);

/* parsing_depth
 * Access: RW
 */
MLXSW_ITEM16(reg, mprs, parsing_depth, 0x2, 0, 16);

/* parsing_en
 * Access: RW
 */
MLXSW_ITEM16(reg, mprs, parsing_en, 0x6, 0, 16);

/* ctipl
 * Access: RW
 */
MLXSW_ITEM8(reg, mprs, ctipl, 0xa, 0, 1);

/* ctipl_l2_length
 * Access: RW
 */
MLXSW_ITEM8(reg, mprs, ctipl_l2_length, 0xb, 0, 7);

/* vxlan_udp_dport
 * Access: RW
 */
MLXSW_ITEM16(reg, mprs, vxlan_udp_dport, 0x12, 0, 16);

/* mmgcr
 * -----
 *
 */
/* device_uid
 * Access: RW
 */
MLXSW_ITEM32(reg, mmgcr, device_uid, 0x0, 0, 24);

/* elig_mask
 * Access: WO
 */
MLXSW_ITEM8(reg, mmgcr, elig_mask, 0xc, 7, 1);

/* elig_dscp_en
 * Access: RW
 */
MLXSW_ITEM8(reg, mmgcr, elig_dscp_en, 0xf, 4, 1);

/* elig_bth_en
 * Access: RW
 */
MLXSW_ITEM8(reg, mmgcr, elig_bth_en, 0xf, 3, 1);

/* elig_cbset_11_en
 * Access: RW
 */
MLXSW_ITEM8(reg, mmgcr, elig_cbset_11_en, 0xf, 2, 1);

/* elig_cbset_10_en
 * Access: RW
 */
MLXSW_ITEM8(reg, mmgcr, elig_cbset_10_en, 0xf, 1, 1);

/* elig_ip_len_en
 * Access: RW
 */
MLXSW_ITEM8(reg, mmgcr, elig_ip_len_en, 0xf, 0, 1);

/* elig_ip_min_size
 * Access: RW
 */
MLXSW_ITEM16(reg, mmgcr, elig_ip_min_size, 0x16, 0, 16);

/* elig_cbset_10
 * Access: RW
 */
MLXSW_ITEM16(reg, mmgcr, elig_cbset_10, 0x18, 0, 16);

/* elig_cbset_10_mask
 * Access: RW
 */
MLXSW_ITEM16(reg, mmgcr, elig_cbset_10_mask, 0x1a, 0, 16);

/* elig_cbset_11
 * Access: RW
 */
MLXSW_ITEM16(reg, mmgcr, elig_cbset_11, 0x1c, 0, 16);

/* elig_cbset_11_mask
 * Access: RW
 */
MLXSW_ITEM16(reg, mmgcr, elig_cbset_11_mask, 0x1e, 0, 16);

/* elig_bth_opcode_bitmap
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mmgcr, elig_bth_opcode_bitmap, 0x20, 0, 32, 4, 0, 4, 0);

/* elig_dscp_bitmap
 * Access: RW
 */
MLXSW_ITEM64(reg, mmgcr, elig_dscp_bitmap, 0x40, 0, 64);

/* mpat
 * ----
 *
 */
/* pa_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mpat, pa_id, 0x0, 4, 4);

/* session_id
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, session_id, 0x0, 0, 4);

/* mngr_type
 * Access: OP
 */
MLXSW_ITEM8(reg, mpat, mngr_type, 0x1, 0, 4);

/* system_port
 * Access: RW
 */
MLXSW_ITEM16(reg, mpat, system_port, 0x2, 0, 16);

/* e
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, e, 0x4, 7, 1);

/* c
 * Access: OP
 */
MLXSW_ITEM8(reg, mpat, c, 0x4, 4, 1);

/* qos
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, qos, 0x4, 2, 1);

/* be
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, be, 0x4, 1, 1);

/* tr
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, tr, 0x4, 0, 1);

/* nmp
 * Access: OP
 */
MLXSW_ITEM8(reg, mpat, nmp, 0x5, 1, 2);

/* imp
 * Access: OP
 */
MLXSW_ITEM8(reg, mpat, imp, 0x5, 0, 1);

/* stclass
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, stclass, 0x6, 5, 3);

/* trim
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, trim, 0x7, 4, 1);

/* span_type
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, span_type, 0x7, 0, 4);

/* itc
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, itc, 0x8, 7, 1);

/* tclass
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, tclass, 0x8, 0, 4);

/* switch_prio
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, switch_prio, 0x9, 0, 4);

/* truncation_size
 * Access: RW
 */
MLXSW_ITEM16(reg, mpat, truncation_size, 0xa, 0, 13);

/* pide
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, pide, 0xe, 7, 1);

/* pid
 * Access: RW
 */
MLXSW_ITEM16(reg, mpat, pid, 0xe, 0, 14);

/* local_span_tclass
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, local_span_tclass, 0x1f, 0, 4);

/* mpat_encap_rmtethvlan_pcp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethvlan_pcp, 0x1a, 5, 3);

/* mpat_encap_rmtethvlan_dei
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethvlan_dei, 0x1a, 4, 1);

/* mpat_encap_rmtethvlan_vid
 * Access: RW
 */
MLXSW_ITEM16(reg, mpat, mpat_encap_rmtethvlan_vid, 0x1a, 0, 12);

/* mpat_encap_rmtethvlan_vlan_et_id
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethvlan_vlan_et_id, 0x1d, 0, 2);

/* mpat_encap_rmtethvlan_tclass
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethvlan_tclass, 0x1f, 0, 4);

/* mpat_encap_rmtethl2_swid
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl2_swid, 0x10, 0, 8);

/* mpat_encap_rmtethl2_version
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl2_version, 0x11, 2, 4);

/* mpat_encap_rmtethl2_dpa
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl2_dpa, 0x11, 0, 2);

/* mpat_encap_rmtethl2_mac
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, mpat, mpat_encap_rmtethl2_mac, 0x12, 6);

/* mpat_encap_rmtethl2_tp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl2_tp, 0x19, 0, 1);

/* mpat_encap_rmtethl2_pcp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl2_pcp, 0x1a, 5, 3);

/* mpat_encap_rmtethl2_dei
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl2_dei, 0x1a, 4, 1);

/* mpat_encap_rmtethl2_vid
 * Access: RW
 */
MLXSW_ITEM16(reg, mpat, mpat_encap_rmtethl2_vid, 0x1a, 0, 12);

/* mpat_encap_rmtethl2_fid_msb
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl2_fid_msb, 0x1c, 0, 4);

/* mpat_encap_rmtethl2_vlan_et_id
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl2_vlan_et_id, 0x1d, 0, 2);

/* mpat_encap_rmtethl2_tclass
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl2_tclass, 0x1f, 0, 4);

/* mpat_encap_rmtethl3gre_swid
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl3gre_swid, 0x10, 0, 8);

/* mpat_encap_rmtethl3gre_version
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl3gre_version, 0x11, 2, 4);

/* mpat_encap_rmtethl3gre_dpa
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl3gre_dpa, 0x11, 0, 2);

/* mpat_encap_rmtethl3gre_mac
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, mpat, mpat_encap_rmtethl3gre_mac, 0x12, 6);

/* mpat_encap_rmtethl3gre_protocol
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl3gre_protocol, 0x18, 0, 4);

/* mpat_encap_rmtethl3gre_dscp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl3gre_dscp, 0x19, 2, 6);

/* mpat_encap_rmtethl3gre_tp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl3gre_tp, 0x19, 0, 1);

/* mpat_encap_rmtethl3gre_pcp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl3gre_pcp, 0x1a, 5, 3);

/* mpat_encap_rmtethl3gre_dei
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl3gre_dei, 0x1a, 4, 1);

/* mpat_encap_rmtethl3gre_vid
 * Access: RW
 */
MLXSW_ITEM16(reg, mpat, mpat_encap_rmtethl3gre_vid, 0x1a, 0, 12);

/* mpat_encap_rmtethl3gre_fid_msb
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl3gre_fid_msb, 0x1c, 0, 4);

/* mpat_encap_rmtethl3gre_vlan_et_id
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl3gre_vlan_et_id, 0x1d, 0, 2);

/* mpat_encap_rmtethl3gre_ecn
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl3gre_ecn, 0x1e, 4, 2);

/* mpat_encap_rmtethl3gre_ttl
 * Access: RW
 */
MLXSW_ITEM16(reg, mpat, mpat_encap_rmtethl3gre_ttl, 0x1e, 4, 8);

/* mpat_encap_rmtethl3gre_tclass
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_rmtethl3gre_tclass, 0x1f, 0, 4);

/* mpat_encap_rmtethl3gre_smac
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, mpat, mpat_encap_rmtethl3gre_smac, 0x22, 6);

/* mpat_encap_rmtethl3gre_dip
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mpat, mpat_encap_rmtethl3gre_dip, 0x40, 0, 32, 4, 0, 4, 0);

/* mpat_encap_rmtethl3gre_sip
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mpat, mpat_encap_rmtethl3gre_sip, 0x50, 0, 32, 4, 0, 4, 0);

/* mpat_encap_localib_vl
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_localib_vl, 0x1f, 0, 4);

/* remote_span_ib_local_ud_we
 * Access: WO
 */
MLXSW_ITEM8(reg, mpat, remote_span_ib_local_ud_we, 0x10, 0, 8);

/* remote_span_ib_local_ud_sl
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, remote_span_ib_local_ud_sl, 0x11, 4, 4);

/* remote_span_ib_local_ud_dpa
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, remote_span_ib_local_ud_dpa, 0x11, 0, 2);

/* remote_span_ib_local_ud_dlid
 * Access: RW
 */
MLXSW_ITEM16(reg, mpat, remote_span_ib_local_ud_dlid, 0x12, 0, 16);

/* remote_span_ib_local_ud_swid
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, remote_span_ib_local_ud_swid, 0x14, 0, 8);

/* remote_span_ib_local_ud_version
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, remote_span_ib_local_ud_version, 0x15, 4, 4);

/* remote_span_ib_local_ud_slid
 * Access: RW
 */
MLXSW_ITEM16(reg, mpat, remote_span_ib_local_ud_slid, 0x16, 0, 16);

/* remote_span_ib_local_ud_vl
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, remote_span_ib_local_ud_vl, 0x1f, 0, 4);

/* remote_span_ib_local_ud_se
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, remote_span_ib_local_ud_se, 0x49, 7, 1);

/* remote_span_ib_local_ud_m
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, remote_span_ib_local_ud_m, 0x49, 6, 1);

/* remote_span_ib_local_ud_pkey
 * Access: RW
 */
MLXSW_ITEM16(reg, mpat, remote_span_ib_local_ud_pkey, 0x4a, 0, 16);

/* remote_span_ib_local_ud_dest_qp
 * Access: RW
 */
MLXSW_ITEM32(reg, mpat, remote_span_ib_local_ud_dest_qp, 0x4c, 0, 24);

/* remote_span_ib_local_ud_ar
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, remote_span_ib_local_ud_ar, 0x50, 6, 1);

/* remote_span_ib_local_ud_psn_init_val
 * Access: RW
 */
MLXSW_ITEM32(reg, mpat, remote_span_ib_local_ud_psn_init_val, 0x50, 0, 24);

/* remote_span_ib_local_ud_qkey
 * Access: RW
 */
MLXSW_ITEM32(reg, mpat, remote_span_ib_local_ud_qkey, 0x54, 0, 32);

/* remote_span_ib_local_ud_src_qp
 * Access: RW
 */
MLXSW_ITEM32(reg, mpat, remote_span_ib_local_ud_src_qp, 0x58, 0, 24);

/* mpat_encap_remoteib_we
 * Access: WO
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_remoteib_we, 0x10, 0, 8);

/* mpat_encap_remoteib_sl
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_remoteib_sl, 0x11, 4, 4);

/* mpat_encap_remoteib_dpa
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_remoteib_dpa, 0x11, 0, 2);

/* mpat_encap_remoteib_dlid
 * Access: RW
 */
MLXSW_ITEM16(reg, mpat, mpat_encap_remoteib_dlid, 0x12, 0, 16);

/* mpat_encap_remoteib_swid
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_remoteib_swid, 0x14, 0, 8);

/* mpat_encap_remoteib_version
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_remoteib_version, 0x15, 4, 4);

/* mpat_encap_remoteib_slid
 * Access: RW
 */
MLXSW_ITEM16(reg, mpat, mpat_encap_remoteib_slid, 0x16, 0, 16);

/* mpat_encap_remoteib_vl
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_remoteib_vl, 0x1f, 0, 4);

/* mpat_encap_remoteib_tclass
 * Access: RW
 */
MLXSW_ITEM16(reg, mpat, mpat_encap_remoteib_tclass, 0x20, 4, 8);

/* mpat_encap_remoteib_flow_label
 * Access: RW
 */
MLXSW_ITEM32(reg, mpat, mpat_encap_remoteib_flow_label, 0x20, 0, 20);

/* mpat_encap_remoteib_hop_limit
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_remoteib_hop_limit, 0x27, 0, 8);

/* mpat_encap_remoteib_sgid
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mpat, mpat_encap_remoteib_sgid, 0x28, 0, 32, 4, 0, 4, 0);

/* mpat_encap_remoteib_dgid
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mpat, mpat_encap_remoteib_dgid, 0x38, 0, 32, 4, 0, 4, 0);

/* mpat_encap_remoteib_se
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_remoteib_se, 0x49, 7, 1);

/* mpat_encap_remoteib_m
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_remoteib_m, 0x49, 6, 1);

/* mpat_encap_remoteib_pkey
 * Access: RW
 */
MLXSW_ITEM16(reg, mpat, mpat_encap_remoteib_pkey, 0x4a, 0, 16);

/* mpat_encap_remoteib_dest_qp
 * Access: RW
 */
MLXSW_ITEM32(reg, mpat, mpat_encap_remoteib_dest_qp, 0x4c, 0, 24);

/* mpat_encap_remoteib_ar
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, mpat_encap_remoteib_ar, 0x50, 6, 1);

/* mpat_encap_remoteib_psn_init_val
 * Access: RW
 */
MLXSW_ITEM32(reg, mpat, mpat_encap_remoteib_psn_init_val, 0x50, 0, 24);

/* mpat_encap_remoteib_qkey
 * Access: RW
 */
MLXSW_ITEM32(reg, mpat, mpat_encap_remoteib_qkey, 0x54, 0, 32);

/* mpat_encap_remoteib_src_qp
 * Access: RW
 */
MLXSW_ITEM32(reg, mpat, mpat_encap_remoteib_src_qp, 0x58, 0, 24);

/* buffer_drop_high
 * Access: RO
 */
MLXSW_ITEM32(reg, mpat, buffer_drop_high, 0x60, 0, 32);

/* buffer_drop_low
 * Access: RO
 */
MLXSW_ITEM32(reg, mpat, buffer_drop_low, 0x64, 0, 32);

/* be_drop_high
 * Access: RO
 */
MLXSW_ITEM32(reg, mpat, be_drop_high, 0x68, 0, 32);

/* be_drop_low
 * Access: RO
 */
MLXSW_ITEM32(reg, mpat, be_drop_low, 0x6c, 0, 32);

/* pc
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, pc, 0x7b, 0, 2);

/* multi_port
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, mpat, multi_port, 0x7c, 0, 16, 2, 0, 2, 0);

/* trim_qos
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, trim_qos, 0x8d, 4, 1);

/* ip_len_fix
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, ip_len_fix, 0x8d, 0, 1);

/* trim_bts_profile
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, trim_bts_profile, 0x8e, 0, 8);

/* trim_fwd_profile
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, trim_fwd_profile, 0x8f, 0, 8);

/* ignore_agent_trim_profile
 * Access: OP
 */
MLXSW_ITEM8(reg, mpat, ignore_agent_trim_profile, 0x90, 7, 1);

/* overwrite_dscp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, overwrite_dscp, 0x91, 7, 1);

/* dscp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, dscp, 0x91, 0, 6);

/* dscp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, dscp_rw, 0x92, 0, 2);

/* pcp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, pcp_rw, 0x93, 0, 2);

/* modifier_offset
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, modifier_offset, 0x98, 0, 2);

/* modifier_base
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, modifier_base, 0x99, 0, 4);

/* emt_bind
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, emt_bind, 0x9a, 0, 4);

/* emt_index
 * Access: RW
 */
MLXSW_ITEM8(reg, mpat, emt_index, 0x9b, 0, 8);

/* mpagr
 * -----
 *
 */
/* swid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mpagr, swid, 0x0, 0, 8);

/* mngr_type
 * Access: OP
 */
MLXSW_ITEM8(reg, mpagr, mngr_type, 0x2, 0, 4);

/* trigger
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mpagr, trigger, 0x3, 0, 4);

/* enable
 * Access: RW
 */
MLXSW_ITEM8(reg, mpagr, enable, 0x4, 7, 1);

/* pa_id
 * Access: RW
 */
MLXSW_ITEM8(reg, mpagr, pa_id, 0x7, 0, 4);

/* probability_rate
 * Access: RW
 */
MLXSW_ITEM32(reg, mpagr, probability_rate, 0x8, 0, 32);

/* momte
 * -----
 *
 */
/* swid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, momte, swid, 0x0, 0, 8);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, momte, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, momte, lp_msb, 0x2, 4, 2);

/* mngr_type
 * Access: OP
 */
MLXSW_ITEM8(reg, momte, mngr_type, 0x2, 0, 4);

/* type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, momte, type, 0x7, 0, 8);

/* tclass_en_high
 * Access: RW
 */
MLXSW_ITEM32(reg, momte, tclass_en_high, 0x8, 0, 32);

/* tclass_en_low
 * Access: RW
 */
MLXSW_ITEM32(reg, momte, tclass_en_low, 0xc, 0, 32);

/* mocmi
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mocmi, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mocmi, lp_msb, 0x2, 4, 2);

/* cond_nelp
 * Access: RW
 */
MLXSW_ITEM16(reg, mocmi, cond_nelp, 0xa, 0, 16);

/* cond_elp
 * Access: RW
 */
MLXSW_ITEM16(reg, mocmi, cond_elp, 0xe, 0, 16);

/* elig
 * Access: RW
 */
MLXSW_ITEM8(reg, mocmi, elig, 0x13, 3, 1);

/* moni
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, moni, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, moni, lp_msb, 0x2, 4, 2);

/* en
 * Access: RW
 */
MLXSW_ITEM8(reg, moni, en, 0x7, 0, 1);

/* motpr
 * -----
 *
 */
/* profile
 * Access: INDEX
 */
MLXSW_ITEM8(reg, motpr, profile, 0x3, 0, 8);

/* type
 * Access: RW
 */
MLXSW_ITEM8(reg, motpr, type, 0x7, 0, 2);

/* overwrite_dscp
 * Access: RW
 */
MLXSW_ITEM8(reg, motpr, overwrite_dscp, 0x11, 7, 1);

/* dscp
 * Access: RW
 */
MLXSW_ITEM8(reg, motpr, dscp, 0x11, 0, 6);

/* dscp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, motpr, dscp_rw, 0x12, 0, 2);

/* pcp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, motpr, pcp_rw, 0x13, 0, 2);

/* bts_keep_udp
 * Access: RW
 */
MLXSW_ITEM8(reg, motpr, bts_keep_udp, 0x17, 2, 1);

/* bts_keep_mac
 * Access: RW
 */
MLXSW_ITEM8(reg, motpr, bts_keep_mac, 0x17, 0, 1);

/* modifier_offset
 * Access: RW
 */
MLXSW_ITEM8(reg, motpr, modifier_offset, 0x18, 0, 2);

/* modifier_base
 * Access: RW
 */
MLXSW_ITEM8(reg, motpr, modifier_base, 0x19, 0, 4);

/* emt_bind
 * Access: RW
 */
MLXSW_ITEM8(reg, motpr, emt_bind, 0x1a, 0, 4);

/* emt_index
 * Access: RW
 */
MLXSW_ITEM8(reg, motpr, emt_index, 0x1b, 0, 8);

/* motspt
 * ------
 *
 */
/* switch_prio
 * Access: INDEX
 */
MLXSW_ITEM8(reg, motspt, switch_prio, 0x3, 0, 4);

/* trim_tclass
 * Access: RW
 */
MLXSW_ITEM8(reg, motspt, trim_tclass, 0x5, 0, 5);

/* trim_switch_prio
 * Access: RW
 */
MLXSW_ITEM8(reg, motspt, trim_switch_prio, 0x7, 0, 4);

/* mocs
 * ----
 *
 */
/* type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mocs, type, 0x3, 0, 8);

/* clear
 * Access: RW
 */
MLXSW_ITEM8(reg, mocs, clear, 0x5, 0, 2);

/* opcode
 * Access: WO
 */
MLXSW_ITEM8(reg, mocs, opcode, 0x6, 0, 4);

/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, mocs, status, 0x7, 0, 2);

/* event_tid
 * Access: RW
 */
MLXSW_ITEM64(reg, mocs, event_tid, 0x8, 0, 64);

/* inf_iter
 * Access: RW
 */
MLXSW_ITEM8(reg, mocs, inf_iter, 0x10, 7, 1);

/* iter_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, mocs, iter_mode, 0x10, 4, 2);

/* min_iter_time
 * Access: RW
 */
MLXSW_ITEM8(reg, mocs, min_iter_time, 0x11, 0, 8);

/* num_of_iter
 * Access: RW
 */
MLXSW_ITEM16(reg, mocs, num_of_iter, 0x12, 0, 16);

/* metadata
 * Access: OP
 */
MLXSW_ITEM8(reg, mocs, metadata, 0x14, 0, 8);

/* iter_md_op
 * Access: OP
 */
MLXSW_ITEM8(reg, mocs, iter_md_op, 0x15, 7, 1);

/* mocs_ppcnt_port_mask
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mocs, mocs_ppcnt_port_mask, 0x20, 0, 32, 4, 0, 4, 0);

/* mocs_ppcnt_grp_mask
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mocs, mocs_ppcnt_grp_mask, 0x60, 0, 32, 4, 0, 4, 0);

/* mocs_ppcnt_tc_mask
 * Access: RW
 */
MLXSW_ITEM32(reg, mocs, mocs_ppcnt_tc_mask, 0x70, 0, 32);

/* mocs_ppcnt_prio_mask
 * Access: RW
 */
MLXSW_ITEM16(reg, mocs, mocs_ppcnt_prio_mask, 0x74, 0, 16);

/* mocs_ppcnt_rx_buffer_mask
 * Access: RW
 */
MLXSW_ITEM16(reg, mocs, mocs_ppcnt_rx_buffer_mask, 0x76, 0, 16);

/* mocs_ppcnt_grp_prf
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mocs, mocs_ppcnt_grp_prf, 0x80, 0, 32, 4, 0, 4, 0);

/* mocs_ppcnt_v2_grp_mask
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mocs, mocs_ppcnt_v2_grp_mask, 0x40, 0, 32, 4, 0, 4, 0);

/* mocs_ppcnt_v2_tc_mask
 * Access: RW
 */
MLXSW_ITEM32(reg, mocs, mocs_ppcnt_v2_tc_mask, 0x50, 0, 32);

/* mocs_ppcnt_v2_prio_mask
 * Access: RW
 */
MLXSW_ITEM32(reg, mocs, mocs_ppcnt_v2_prio_mask, 0x54, 0, 17);

/* mocs_ppcnt_v2_rx_buffer_mask
 * Access: RW
 */
MLXSW_ITEM16(reg, mocs, mocs_ppcnt_v2_rx_buffer_mask, 0x5a, 0, 16);

/* mocs_ppcnt_v2_grp_prf
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mocs, mocs_ppcnt_v2_grp_prf, 0x60, 0, 32, 4, 0, 4, 0);

/* mocs_ppcnt_v2_port_bitmap_size
 * Access: OP
 */
MLXSW_ITEM8(reg, mocs, mocs_ppcnt_v2_port_bitmap_size, 0xe3, 0, 8);

/* mocs_ppcnt_v2_port_mask
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mocs, mocs_ppcnt_v2_port_mask, 0xe4, 0, 32, 4, 0, 4, 0);

/* mocs_mgpcb_flush
 * Access: RW
 */
MLXSW_ITEM8(reg, mocs, mocs_mgpcb_flush, 0x20, 7, 1);

/* mocs_mgpcb_num_rec
 * Access: RW
 */
MLXSW_ITEM32(reg, mocs, mocs_mgpcb_num_rec, 0x20, 0, 24);

/* mocs_mgpcb_counter_index_base
 * Access: RW
 */
MLXSW_ITEM32(reg, mocs, mocs_mgpcb_counter_index_base, 0x24, 0, 24);

/* mocs_pbsr_port_mask
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mocs, mocs_pbsr_port_mask, 0x20, 0, 32, 4, 0, 4, 0);

/* mocs_pbsr_v2_bitmap_size
 * Access: OP
 */
MLXSW_ITEM8(reg, mocs, mocs_pbsr_v2_bitmap_size, 0x23, 0, 8);

/* mocs_pbsr_v2_port_mask
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mocs, mocs_pbsr_v2_port_mask, 0x30, 0, 32, 4, 0, 4, 0);

/* curr
 * Access: RW
 */
MLXSW_ITEM8(reg, mocs, curr, 0x20, 7, 1);

/* snap
 * Access: RW
 */
MLXSW_ITEM8(reg, mocs, snap, 0x20, 6, 1);

/* cells
 * Access: RW
 */
MLXSW_ITEM8(reg, mocs, cells, 0x20, 5, 1);

/* desc
 * Access: RW
 */
MLXSW_ITEM8(reg, mocs, desc, 0x20, 4, 1);

/* modi
 * Access: RW
 */
MLXSW_ITEM8(reg, mocs, modi, 0x20, 3, 1);

/* mocs_ceer_port_mask
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mocs, mocs_ceer_port_mask, 0x20, 0, 32, 4, 0, 4, 0);

/* mocs_mafbi_flush
 * Access: RW
 */
MLXSW_ITEM8(reg, mocs, mocs_mafbi_flush, 0x20, 7, 1);

/* mocs_mafbi_type
 * Access: RW
 */
MLXSW_ITEM8(reg, mocs, mocs_mafbi_type, 0x20, 4, 2);

/* mocs_mafbi_num_rec
 * Access: RW
 */
MLXSW_ITEM32(reg, mocs, mocs_mafbi_num_rec, 0x20, 0, 24);

/* mocs_mafbi_counter_index_base
 * Access: RW
 */
MLXSW_ITEM32(reg, mocs, mocs_mafbi_counter_index_base, 0x24, 0, 24);

/* mocs_fsed_op
 * Access: OP
 */
MLXSW_ITEM8(reg, mocs, mocs_fsed_op, 0x22, 0, 2);

/* mocs_fsed_filter_fields
 * Access: OP
 */
MLXSW_ITEM8(reg, mocs, mocs_fsed_filter_fields, 0x23, 0, 4);

/* mocs_fsed_group_id
 * Access: INDEX
 */
MLXSW_ITEM16(reg, mocs, mocs_fsed_group_id, 0x24, 0, 16);

/* mocs_fsed_entry_sem_s
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mocs, mocs_fsed_entry_sem_s, 0x26, 1, 1);

/* mocs_fsed_entry_activity
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mocs, mocs_fsed_entry_activity, 0x26, 0, 1);

/* mocs_fsed_entry_partition_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mocs, mocs_fsed_entry_partition_id, 0x27, 0, 3);

/* mocs_fsed_num_rec
 * Access: RW
 */
MLXSW_ITEM32(reg, mocs, mocs_fsed_num_rec, 0x28, 0, 20);

/* mocs_fsed_nt
 * Access: OP
 */
MLXSW_ITEM8(reg, mocs, mocs_fsed_nt, 0x2f, 0, 1);

/* flow_estimator_record_num_rec
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, mocs, flow_estimator_record_num_rec, MLXSW_MOCS_MOCS_MOFRB_FLOW_ESTIMATOR_RECORD_BASE_LEN + 2, 0, 11, MLXSW_MOCS_FLOW_ESTIMATOR_RECORD_REC_LEN, 0, 2, 0);

/* flow_estimator_record_counter_index_base
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, mocs, flow_estimator_record_counter_index_base, MLXSW_MOCS_MOCS_MOFRB_FLOW_ESTIMATOR_RECORD_BASE_LEN, 0, 24, MLXSW_MOCS_FLOW_ESTIMATOR_RECORD_REC_LEN, 4, 3, 0);

/* user_cookie
 * Access: RW
 */
MLXSW_ITEM64(reg, mocs, user_cookie, 0x28, 0, 64);

/* num_job
 * Access: OP
 */
MLXSW_ITEM16(reg, mocs, num_job, 0x22, 0, 12);

/* num_rec_log
 * Access: OP
 */
MLXSW_ITEM8_INDEXED(reg, mocs, num_rec_log, MLXSW_MOCS_MOCS_RARFTBR_JOB_BASE_LEN, 0, 5, MLXSW_MOCS_JOB_REC_LEN, 0, 1, 0);

/* arft_index
 * Access: INDEX
 */
MLXSW_ITEM32_INDEXED(reg, mocs, arft_index, MLXSW_MOCS_MOCS_RARFTBR_JOB_BASE_LEN, 0, 20, MLXSW_MOCS_JOB_REC_LEN, 0, 3, 0);

/* mafcr
 * -----
 *
 */
/* clear
 * Access: OP
 */
MLXSW_ITEM8(reg, mafcr, clear, 0x0, 7, 1);

/* counter_accu_overflow
 * Access: RO
 */
MLXSW_ITEM32(reg, mafcr, counter_accu_overflow, 0x4, 0, 32);

/* packets_inc_units
 * Access: RO
 */
MLXSW_ITEM16(reg, mafcr, packets_inc_units, 0x8, 0, 16);

/* bytes_inc_units
 * Access: RO
 */
MLXSW_ITEM16(reg, mafcr, bytes_inc_units, 0xa, 0, 16);

/* mafti
 * -----
 *
 */
/* bank_id
 * Access: RO
 */
MLXSW_ITEM8(reg, mafti, bank_id, 0x1, 0, 3);

/* num_rec
 * Access: RO
 */
MLXSW_ITEM8(reg, mafti, num_rec, 0x3, 0, 8);

/* counter_index
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mafti, counter_index, MLXSW_MAFTI_RECORD_BASE_LEN, 0, 24, MLXSW_MAFTI_RECORD_REC_LEN, 0, 3, 0);

/* bytes_inc
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mafti, bytes_inc, MLXSW_MAFTI_RECORD_BASE_LEN, 0, 32, MLXSW_MAFTI_RECORD_REC_LEN, 4, 4, 0);

/* packets_inc
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mafti, packets_inc, MLXSW_MAFTI_RECORD_BASE_LEN, 0, 32, MLXSW_MAFTI_RECORD_REC_LEN, 8, 4, 0);

/* mafbi
 * -----
 *
 */
/* type
 * Access: RO
 */
MLXSW_ITEM8(reg, mafbi, type, 0x0, 0, 2);

/* num_rec
 * Access: RO
 */
MLXSW_ITEM8(reg, mafbi, num_rec, 0x3, 0, 8);

/* counter_index
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mafbi, counter_index, MLXSW_MAFBI_MAFBI_ENTRY_TYPE0_RECORD_BASE_LEN, 0, 24, MLXSW_MAFBI_RECORD_REC_LEN, 0, 3, 0);

/* bytes_inc
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mafbi, bytes_inc, MLXSW_MAFBI_MAFBI_ENTRY_TYPE0_RECORD_BASE_LEN, 0, 32, MLXSW_MAFBI_RECORD_REC_LEN, 4, 4, 0);

/* packets_inc
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mafbi, packets_inc, MLXSW_MAFBI_MAFBI_ENTRY_TYPE0_RECORD_BASE_LEN, 0, 32, MLXSW_MAFBI_RECORD_REC_LEN, 8, 4, 0);

/* mafri
 * -----
 *
 */
/* fsf
 * Access: OP
 */
MLXSW_ITEM8(reg, mafri, fsf, 0x0, 0, 1);

/* event
 * Access: OP
 */
MLXSW_ITEM8(reg, mafri, event, 0x1, 0, 1);

/* num_rec
 * Access: OP
 */
MLXSW_ITEM8(reg, mafri, num_rec, 0x3, 0, 8);

/* counter_index_base
 * Access: OP
 */
MLXSW_ITEM32(reg, mafri, counter_index_base, 0x4, 0, 24);

/* user_val
 * Access: RW
 */
MLXSW_ITEM32(reg, mafri, user_val, 0x8, 0, 32);

/* bytes_inc
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mafri, bytes_inc, MLXSW_MAFRI_RECORD_BASE_LEN, 0, 32, MLXSW_MAFRI_RECORD_REC_LEN, 0, 4, 0);

/* packets_inc
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mafri, packets_inc, MLXSW_MAFRI_RECORD_BASE_LEN, 0, 32, MLXSW_MAFRI_RECORD_REC_LEN, 4, 4, 0);

/* mopce
 * -----
 *
 */
/* num_rec
 * Access: RW
 */
MLXSW_ITEM8(reg, mopce, num_rec, 0x3, 0, 7);

/* hw_unit_instance
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, mopce, hw_unit_instance, MLXSW_MOPCE_PERFORMANCE_COUNTER_INFO_BASE_LEN + 1, 0, 8, MLXSW_MOPCE_PERFORMANCE_COUNTER_INFO_REC_LEN, 0, 1, 0);

/* hw_unit_id
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, mopce, hw_unit_id, MLXSW_MOPCE_PERFORMANCE_COUNTER_INFO_BASE_LEN + 3, 0, 8, MLXSW_MOPCE_PERFORMANCE_COUNTER_INFO_REC_LEN, 0, 1, 0);

/* counter_index
 * Access: RO
 */
MLXSW_ITEM16_INDEXED(reg, mopce, counter_index, MLXSW_MOPCE_PERFORMANCE_COUNTER_INFO_BASE_LEN, 0, 16, MLXSW_MOPCE_PERFORMANCE_COUNTER_INFO_REC_LEN, 4, 2, 0);

/* hw_unit_sub_instance
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, mopce, hw_unit_sub_instance, MLXSW_MOPCE_PERFORMANCE_COUNTER_INFO_BASE_LEN + 3, 0, 8, MLXSW_MOPCE_PERFORMANCE_COUNTER_INFO_REC_LEN, 4, 1, 0);

/* counter_value
 * Access: RO
 */
MLXSW_ITEM64_INDEXED(reg, mopce, counter_value, MLXSW_MOPCE_PERFORMANCE_COUNTER_INFO_BASE_LEN, 0, 64, MLXSW_MOPCE_PERFORMANCE_COUNTER_INFO_REC_LEN, 8, 8, 0);

/* moca
 * ----
 *
 */
/* counter_type
 * Access: RW
 */
MLXSW_ITEM8(reg, moca, counter_type, 0x0, 0, 8);

/* counter_index
 * Access: INDEX
 */
MLXSW_ITEM32(reg, moca, counter_index, 0x0, 0, 24);

/* size
 * Access: OP
 */
MLXSW_ITEM32(reg, moca, size, 0x8, 0, 24);

/* mofrb
 * -----
 *
 */
/* clear
 * Access: OP
 */
MLXSW_ITEM8(reg, mofrb, clear, 0x0, 7, 1);

/* num_rec
 * Access: RW
 */
MLXSW_ITEM16(reg, mofrb, num_rec, 0x2, 0, 11);

/* counter_index_base
 * Access: INDEX
 */
MLXSW_ITEM32(reg, mofrb, counter_index_base, 0x4, 0, 24);

/* flow_estimator_bin
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, mofrb, flow_estimator_bin, 0x10, 512);

/* mofph
 * -----
 *
 */
/* profile
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mofph, profile, 0x2, 0, 3);

/* type
 * Access: RW
 */
MLXSW_ITEM8(reg, mofph, type, 0x3, 0, 4);

/* seed_en
 * Access: RW
 */
MLXSW_ITEM8(reg, mofph, seed_en, 0x4, 6, 2);

/* seed_by_sw
 * Access: RW
 */
MLXSW_ITEM8(reg, mofph, seed_by_sw, 0x7, 0, 8);

/* outer_header_enables
 * Access: RW
 */
MLXSW_ITEM16(reg, mofph, outer_header_enables, 0xa, 0, 16);

/* outer_header_hash
 * Access: RW
 */
MLXSW_ITEM64(reg, mofph, outer_header_hash, 0x10, 0, 64);

/* inner_header_enables
 * Access: RW
 */
MLXSW_ITEM16(reg, mofph, inner_header_enables, 0x1a, 0, 16);

/* inner_header_hash
 * Access: RW
 */
MLXSW_ITEM32(reg, mofph, inner_header_hash, 0x1c, 0, 32);

/* mofpc
 * -----
 *
 */
/* profile
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mofpc, profile, 0x3, 0, 3);

/* log_num_of_bins
 * Access: RW
 */
MLXSW_ITEM8(reg, mofpc, log_num_of_bins, 0x7, 0, 4);

/* mocbr
 * -----
 *
 */
/* e
 * Access: RW
 */
MLXSW_ITEM8(reg, mocbr, e, 0x3, 0, 2);

/* l
 * Access: RW
 */
MLXSW_ITEM8(reg, mocbr, l, 0x7, 0, 1);

/* base_counter_index
 * Access: RW
 */
MLXSW_ITEM32(reg, mocbr, base_counter_index, 0x8, 0, 24);

/* mocbs
 * -----
 *
 */
/* op
 * Access: OP
 */
MLXSW_ITEM8(reg, mocbs, op, 0x3, 0, 2);

/* cnt_err
 * Access: RO
 */
MLXSW_ITEM8(reg, mocbs, cnt_err, 0x7, 0, 1);

/* mopct
 * -----
 *
 */
/* cnt_grp_id
 * Access: RW
 */
MLXSW_ITEM8(reg, mopct, cnt_grp_id, 0x1, 0, 8);

/* entry_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mopct, entry_index, 0x3, 0, 8);

/* num_active_counters
 * Access: RW
 */
MLXSW_ITEM8(reg, mopct, num_active_counters, 0x4, 0, 5);

/* op
 * Access: OP
 */
MLXSW_ITEM8(reg, mopct, op, 0x5, 0, 2);

/* hw_unit_id
 * Access: RW
 */
MLXSW_ITEM16(reg, mopct, hw_unit_id, 0x6, 0, 11);

/* force
 * Access: RW
 */
MLXSW_ITEM8(reg, mopct, force, 0xb, 0, 1);

/* enabled_counters
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, mopct, enabled_counters, 0x10, 0, 16, 2, 0, 2, 0);

/* mopcs
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mopcs, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mopcs, lp_msb, 0x2, 4, 2);

/* dcp_index
 * Access: RO
 */
MLXSW_ITEM16(reg, mopcs, dcp_index, 0x4, 0, 16);

/* kvc_index
 * Access: RO
 */
MLXSW_ITEM16(reg, mopcs, kvc_index, 0x6, 0, 16);

/* plu_index
 * Access: RO
 */
MLXSW_ITEM16(reg, mopcs, plu_index, 0x8, 0, 16);

/* dcm_index
 * Access: RO
 */
MLXSW_ITEM16(reg, mopcs, dcm_index, 0xa, 0, 16);

/* ptb_index
 * Access: RO
 */
MLXSW_ITEM16(reg, mopcs, ptb_index, 0xc, 0, 16);

/* llu_index
 * Access: RO
 */
MLXSW_ITEM16(reg, mopcs, llu_index, 0xe, 0, 16);

/* tile_index
 * Access: RO
 */
MLXSW_ITEM16(reg, mopcs, tile_index, 0x10, 0, 16);

/* tile_plt_index
 * Access: RO
 */
MLXSW_ITEM16(reg, mopcs, tile_plt_index, 0x12, 0, 16);

/* dcp_sub_index
 * Access: RO
 */
MLXSW_ITEM16(reg, mopcs, dcp_sub_index, 0x14, 0, 16);

/* dcm_sub_index
 * Access: RO
 */
MLXSW_ITEM16(reg, mopcs, dcm_sub_index, 0x16, 0, 16);

/* ptb_sub_index
 * Access: RO
 */
MLXSW_ITEM16(reg, mopcs, ptb_sub_index, 0x18, 0, 16);

/* llu_sub_index
 * Access: RO
 */
MLXSW_ITEM16(reg, mopcs, llu_sub_index, 0x1a, 0, 16);

/* mopgcr
 * ------
 *
 */
/* perf_cnt_interval
 * Access: RW
 */
MLXSW_ITEM32(reg, mopgcr, perf_cnt_interval, 0x0, 0, 32);

/* moftc
 * -----
 *
 */
/* clear_mbo
 * Access: RW
 */
MLXSW_ITEM8(reg, moftc, clear_mbo, 0x0, 7, 1);

/* prio_bitmask
 * Access: RW
 */
MLXSW_ITEM8(reg, moftc, prio_bitmask, 0x7, 0, 8);

/* tc_bitmask
 * Access: RW
 */
MLXSW_ITEM32(reg, moftc, tc_bitmask, 0x8, 0, 17);

/* pg_bitmask
 * Access: RW
 */
MLXSW_ITEM16(reg, moftc, pg_bitmask, 0xe, 0, 9);

/* metadata_bitmask
 * Access: RW
 */
MLXSW_ITEM32(reg, moftc, metadata_bitmask, 0x10, 0, 32);

/* global_counters_bitmask
 * Access: RW
 */
MLXSW_ITEM32(reg, moftc, global_counters_bitmask, 0x18, 0, 32);

/* port_counters_bitmask
 * Access: RW
 */
MLXSW_ITEM64(reg, moftc, port_counters_bitmask, 0x20, 0, 64);

/* flow_counter_base_0
 * Access: RW
 */
MLXSW_ITEM32(reg, moftc, flow_counter_base_0, 0x30, 0, 24);

/* flow_counter_num_0
 * Access: RW
 */
MLXSW_ITEM16(reg, moftc, flow_counter_num_0, 0x36, 0, 16);

/* flow_counter_base_1
 * Access: RW
 */
MLXSW_ITEM32(reg, moftc, flow_counter_base_1, 0x38, 0, 24);

/* flow_counter_num_1
 * Access: RW
 */
MLXSW_ITEM16(reg, moftc, flow_counter_num_1, 0x3e, 0, 16);

/* bitmap_size
 * Access: OP
 */
MLXSW_ITEM8(reg, moftc, bitmap_size, 0x7f, 0, 4);

/* moftc_bitmap512_port_bitmask
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, moftc, moftc_bitmap512_port_bitmask, 0x80, 0, 32, 4, 0, 4, 0);

/* moftc_bitmap512_port_bitmask_actual
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, moftc, moftc_bitmap512_port_bitmask_actual, 0xc0, 0, 32, 4, 0, 4, 0);

/* moftc_bitmap544_port_bitmask
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, moftc, moftc_bitmap544_port_bitmask, 0x80, 0, 32, 4, 0, 4, 0);

/* moftc_bitmap544_port_bitmask_actual
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, moftc, moftc_bitmap544_port_bitmask_actual, 0xc4, 0, 32, 4, 0, 4, 0);

/* moftd
 * -----
 *
 */
/* num_rec
 * Access: RO
 */
MLXSW_ITEM16(reg, moftd, num_rec, 0x2, 0, 12);

/* itr_index
 * Access: RO
 */
MLXSW_ITEM16(reg, moftd, itr_index, 0x6, 0, 16);

/* num_entries_in_itr
 * Access: RO
 */
MLXSW_ITEM32(reg, moftd, num_entries_in_itr, 0x8, 0, 20);

/* entry_index
 * Access: RO
 */
MLXSW_ITEM32(reg, moftd, entry_index, 0xc, 0, 20);

/* first_port_in_chunk
 * Access: RO
 */
MLXSW_ITEM16(reg, moftd, first_port_in_chunk, 0x10, 0, 10);

/* num_entries_per_port
 * Access: RO
 */
MLXSW_ITEM16(reg, moftd, num_entries_per_port, 0x12, 0, 12);

/* user_cookie
 * Access: RO
 */
MLXSW_ITEM64(reg, moftd, user_cookie, 0x18, 0, 64);

/* entry
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, moftd, entry, 0x20, 0, 32, 4, 0, 4, 0);

/* moplhbr
 * -------
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, moplhbr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, moplhbr, lp_msb, 0x2, 4, 2);

/* en
 * Access: RW
 */
MLXSW_ITEM8(reg, moplhbr, en, 0x4, 7, 1);

/* reset
 * Access: WO
 */
MLXSW_ITEM8(reg, moplhbr, reset, 0x4, 6, 1);

/* mode
 * Access: RW
 */
MLXSW_ITEM8(reg, moplhbr, mode, 0x8, 4, 3);

/* num_bins
 * Access: RW
 */
MLXSW_ITEM8(reg, moplhbr, num_bins, 0x9, 0, 4);

/* hist_min_value
 * Access: RW
 */
MLXSW_ITEM32(reg, moplhbr, hist_min_value, 0x10, 0, 32);

/* hist_max_value
 * Access: RW
 */
MLXSW_ITEM32(reg, moplhbr, hist_max_value, 0x14, 0, 32);

/* moplhrr
 * -------
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, moplhrr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, moplhrr, lp_msb, 0x2, 4, 2);

/* valid
 * Access: RO
 */
MLXSW_ITEM8(reg, moplhrr, valid, 0x4, 7, 1);

/* wraparound
 * Access: RO
 */
MLXSW_ITEM8(reg, moplhrr, wraparound, 0x4, 6, 1);

/* total_count
 * Access: RO
 */
MLXSW_ITEM64(reg, moplhrr, total_count, 0xc, 0, 64);

/* total_events
 * Access: RO
 */
MLXSW_ITEM64(reg, moplhrr, total_events, 0x14, 0, 64);

/* min_count
 * Access: RO
 */
MLXSW_ITEM32(reg, moplhrr, min_count, 0x1c, 0, 32);

/* max_count
 * Access: RO
 */
MLXSW_ITEM32(reg, moplhrr, max_count, 0x20, 0, 32);

/* bin
 * Access: RO
 */
MLXSW_ITEM64_INDEXED(reg, moplhrr, bin, 0x30, 0, 64, 8, 0, 8, 0);

/* motcnt
 * ------
 *
 */
/* clr
 * Access: OP
 */
MLXSW_ITEM8(reg, motcnt, clr, 0x0, 7, 1);

/* clr_all
 * Access: OP
 */
MLXSW_ITEM8(reg, motcnt, clr_all, 0x0, 6, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, motcnt, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, motcnt, lp_msb, 0x2, 4, 2);

/* num_rec
 * Access: OP
 */
MLXSW_ITEM16(reg, motcnt, num_rec, 0x2, 0, 10);

/* packet_counter
 * Access: RW
 */
MLXSW_ITEM64_INDEXED(reg, motcnt, packet_counter, 0x10, 0, 64, 8, 0, 8, 0);

/* mtpspu
 * ------
 *
 */
/* message_type
 * Access: RW
 */
MLXSW_ITEM16(reg, mtpspu, message_type, 0x6, 0, 16);

/* mtpcpc
 * ------
 *
 */
/* pport
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mtpcpc, pport, 0x0, 7, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mtpcpc, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mtpcpc, lp_msb, 0x2, 4, 2);

/* egr_phbw
 * Access: RW
 */
MLXSW_ITEM8(reg, mtpcpc, egr_phbw, 0x3, 0, 1);

/* ptp_trap_en
 * Access: RW
 */
MLXSW_ITEM8(reg, mtpcpc, ptp_trap_en, 0x7, 0, 1);

/* ing_correction_message_type
 * Access: RW
 */
MLXSW_ITEM16(reg, mtpcpc, ing_correction_message_type, 0x12, 0, 16);

/* egr_correction_message_type
 * Access: RW
 */
MLXSW_ITEM16(reg, mtpcpc, egr_correction_message_type, 0x16, 0, 16);

/* ptp_pkey
 * Access: RW
 */
MLXSW_ITEM16(reg, mtpcpc, ptp_pkey, 0x1a, 0, 15);

/* num_domains
 * Access: RW
 */
MLXSW_ITEM8(reg, mtpcpc, num_domains, 0x23, 0, 8);

/* domain
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, mtpcpc, domain, 0x24, 4);

/* mfgd
 * ----
 *
 */
/* long_cmd_timeout_value
 * Access: RW
 */
MLXSW_ITEM8(reg, mfgd, long_cmd_timeout_value, 0x0, 0, 8);

/* immediate_interface_release_on_timeout
 * Access: RW
 */
MLXSW_ITEM8(reg, mfgd, immediate_interface_release_on_timeout, 0x1, 4, 2);

/* en_debug_assert
 * Access: RW
 */
MLXSW_ITEM8(reg, mfgd, en_debug_assert, 0x1, 0, 1);

/* fw_fatal_event_test
 * Access: OP
 */
MLXSW_ITEM8(reg, mfgd, fw_fatal_event_test, 0x2, 3, 4);

/* fw_fatal_event_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, mfgd, fw_fatal_event_mode, 0x2, 1, 2);

/* packet_state_test_time_value
 * Access: RW
 */
MLXSW_ITEM8(reg, mfgd, packet_state_test_time_value, 0x4, 0, 8);

/* serbi_test_value
 * Access: RW
 */
MLXSW_ITEM8(reg, mfgd, serbi_test_value, 0x5, 7, 1);

/* serbi_test_ov
 * Access: RW
 */
MLXSW_ITEM8(reg, mfgd, serbi_test_ov, 0x5, 6, 1);

/* packet_state_test_action
 * Access: RW
 */
MLXSW_ITEM8(reg, mfgd, packet_state_test_action, 0x7, 0, 2);

/* mifgd
 * -----
 *
 */
/* rif_cache_add_flow_disable
 * Access: RW
 */
MLXSW_ITEM8(reg, mifgd, rif_cache_add_flow_disable, 0x1, 6, 1);

/* rif_cache_update_flow_disable
 * Access: RW
 */
MLXSW_ITEM8(reg, mifgd, rif_cache_update_flow_disable, 0x1, 5, 1);

/* rif_cache_phase_update
 * Access: OP
 */
MLXSW_ITEM8(reg, mifgd, rif_cache_phase_update, 0x1, 4, 1);

/* rif_cache_phase
 * Access: RW
 */
MLXSW_ITEM8(reg, mifgd, rif_cache_phase, 0x1, 1, 3);

/* trigger_emad_pipeline_jam
 * Access: WO
 */
MLXSW_ITEM8(reg, mifgd, trigger_emad_pipeline_jam, 0x2, 1, 1);

/* trigger_stack_ovf
 * Access: WO
 */
MLXSW_ITEM8(reg, mifgd, trigger_stack_ovf, 0x2, 0, 1);

/* caches_flush
 * Access: OP
 */
MLXSW_ITEM8(reg, mifgd, caches_flush, 0x3, 5, 2);

/* code_cache_cnt_disable
 * Access: RW
 */
MLXSW_ITEM8(reg, mifgd, code_cache_cnt_disable, 0x3, 4, 1);

/* tcr_debug_enable
 * Access: WO
 */
MLXSW_ITEM8(reg, mifgd, tcr_debug_enable, 0x3, 3, 1);

/* caches_enable
 * Access: RW
 */
MLXSW_ITEM8(reg, mifgd, caches_enable, 0x3, 0, 3);

/* port_init_single_cpu
 * Access: WO
 */
MLXSW_ITEM8(reg, mifgd, port_init_single_cpu, 0x4, 7, 1);

/* restore_acl_group_elements
 * Access: WO
 */
MLXSW_ITEM8(reg, mifgd, restore_acl_group_elements, 0x5, 6, 1);

/* enable_invalid_fatal_cause_event
 * Access: RW
 */
MLXSW_ITEM8(reg, mifgd, enable_invalid_fatal_cause_event, 0x5, 5, 1);

/* atcam_bf_enable
 * Access: RW
 */
MLXSW_ITEM8(reg, mifgd, atcam_bf_enable, 0x5, 4, 1);

/* blocks_egress_enable
 * Access: RW
 */
MLXSW_ITEM16(reg, mifgd, blocks_egress_enable, 0x6, 0, 16);

/* fw_init_total_time
 * Access: RO
 */
MLXSW_ITEM32(reg, mifgd, fw_init_total_time, 0x8, 0, 32);

/* fw_init_open_ports_time
 * Access: RO
 */
MLXSW_ITEM32(reg, mifgd, fw_init_open_ports_time, 0xc, 0, 32);

/* boot_time
 * Access: RO
 */
MLXSW_ITEM32(reg, mifgd, boot_time, 0x10, 0, 32);

/* init_tiles_time
 * Access: RO
 */
MLXSW_ITEM32(reg, mifgd, init_tiles_time, 0x14, 0, 32);

/* init_phy_power_time
 * Access: RO
 */
MLXSW_ITEM32(reg, mifgd, init_phy_power_time, 0x18, 0, 32);

/* init_usr_algo_time
 * Access: RO
 */
MLXSW_ITEM32(reg, mifgd, init_usr_algo_time, 0x1c, 0, 32);

/* init_gearbox_time
 * Access: RO
 */
MLXSW_ITEM32(reg, mifgd, init_gearbox_time, 0x20, 0, 32);

/* boot_mode
 * Access: RO
 */
MLXSW_ITEM8(reg, mifgd, boot_mode, 0x24, 0, 4);

/* ind_in_na_mode
 * Access: RO
 */
MLXSW_ITEM8(reg, mifgd, ind_in_na_mode, 0x27, 0, 1);

/* modcr
 * -----
 *
 */
/* clear
 * Access: OP
 */
MLXSW_ITEM8(reg, modcr, clear, 0x0, 7, 1);

/* fatal_asserts
 * Access: RO
 */
MLXSW_ITEM32(reg, modcr, fatal_asserts, 0x8, 0, 32);

/* normal_asserts
 * Access: RO
 */
MLXSW_ITEM32(reg, modcr, normal_asserts, 0xc, 0, 32);

/* debug_asserts
 * Access: RO
 */
MLXSW_ITEM32(reg, modcr, debug_asserts, 0x10, 0, 32);

/* mofs
 * ----
 *
 */
/* type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mofs, type, 0x3, 0, 2);

/* register_id
 * Access: INDEX
 */
MLXSW_ITEM16(reg, mofs, register_id, 0x6, 0, 16);

/* section_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mofs, section_id, 0xa, 0, 8);

/* en
 * Access: RW
 */
MLXSW_ITEM8(reg, mofs, en, 0x11, 0, 1);

/* action
 * Access: WO
 */
MLXSW_ITEM8(reg, mofs, action, 0x13, 0, 2);

/* reg_status
 * Access: RO
 */
MLXSW_ITEM8(reg, mofs, reg_status, 0x17, 0, 2);

/* mfde
 * ----
 *
 */
/* irisc_id
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, irisc_id, 0x0, 0, 8);

/* severity
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, severity, 0x1, 0, 8);

/* event_id
 * Access: RO
 */
MLXSW_ITEM16(reg, mfde, event_id, 0x2, 0, 16);

/* notify_fw_dump_completion
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, notify_fw_dump_completion, 0x4, 7, 1);

/* method
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, method, 0x4, 5, 1);

/* packet_state
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, packet_state, 0x4, 3, 2);

/* mgmt_class
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, mgmt_class, 0x5, 0, 8);

/* reg_attr_id
 * Access: RO
 */
MLXSW_ITEM16(reg, mfde, reg_attr_id, 0x6, 0, 16);

/* packet_state_sticky
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, packet_state_sticky, 0x8, 3, 2);

/* tile_v
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, tile_v, 0x9, 7, 1);

/* tile_index
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, tile_index, 0x9, 0, 6);

/* packet_cmd_type
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, packet_cmd_type, 0xa, 0, 2);

/* packet_interface
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, packet_interface, 0xb, 0, 8);

/* crspace_timeout_log_address
 * Access: RO
 */
MLXSW_ITEM32(reg, mfde, crspace_timeout_log_address, 0x10, 0, 32);

/* crspace_timeout_is_main_farm
 * Access: RW
 */
MLXSW_ITEM8(reg, mfde, crspace_timeout_is_main_farm, 0x14, 7, 1);

/* crspace_timeout_is_iron
 * Access: RW
 */
MLXSW_ITEM8(reg, mfde, crspace_timeout_is_iron, 0x14, 6, 1);

/* crspace_timeout_is_yu
 * Access: RW
 */
MLXSW_ITEM8(reg, mfde, crspace_timeout_is_yu, 0x14, 5, 1);

/* crspace_timeout_oe
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, crspace_timeout_oe, 0x14, 0, 1);

/* crspace_timeout_tile_v
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, crspace_timeout_tile_v, 0x15, 7, 1);

/* crspace_timeout_tile_index
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, crspace_timeout_tile_index, 0x15, 0, 6);

/* crspace_timeout_trans_type
 * Access: RW
 */
MLXSW_ITEM8(reg, mfde, crspace_timeout_trans_type, 0x16, 5, 3);

/* crspace_timeout_error_type
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, crspace_timeout_error_type, 0x16, 0, 4);

/* crspace_timeout_log_id
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, crspace_timeout_log_id, 0x17, 0, 5);

/* crspace_timeout_log_ip
 * Access: RO
 */
MLXSW_ITEM64(reg, mfde, crspace_timeout_log_ip, 0x18, 0, 64);

/* oe
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, oe, 0x10, 0, 1);

/* pipes_mask
 * Access: RO
 */
MLXSW_ITEM16(reg, mfde, pipes_mask, 0x12, 0, 16);

/* fw_assert_assert_var0
 * Access: RO
 */
MLXSW_ITEM32(reg, mfde, fw_assert_assert_var0, 0x10, 0, 32);

/* fw_assert_assert_var1
 * Access: RO
 */
MLXSW_ITEM32(reg, mfde, fw_assert_assert_var1, 0x14, 0, 32);

/* fw_assert_assert_var2
 * Access: RO
 */
MLXSW_ITEM32(reg, mfde, fw_assert_assert_var2, 0x18, 0, 32);

/* fw_assert_assert_var3
 * Access: RO
 */
MLXSW_ITEM32(reg, mfde, fw_assert_assert_var3, 0x1c, 0, 32);

/* fw_assert_assert_var4
 * Access: RO
 */
MLXSW_ITEM32(reg, mfde, fw_assert_assert_var4, 0x20, 0, 32);

/* fw_assert_assert_exinstptr
 * Access: RO
 */
MLXSW_ITEM32(reg, mfde, fw_assert_assert_exinstptr, 0x24, 0, 32);

/* fw_assert_assert_callra
 * Access: RO
 */
MLXSW_ITEM32(reg, mfde, fw_assert_assert_callra, 0x28, 0, 32);

/* fw_assert_test
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, fw_assert_test, 0x2c, 7, 1);

/* fw_assert_oe
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, fw_assert_oe, 0x2c, 0, 1);

/* fw_assert_tile_v
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, fw_assert_tile_v, 0x2d, 7, 1);

/* fw_assert_tile_index
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, fw_assert_tile_index, 0x2d, 0, 6);

/* fw_assert_ext_synd
 * Access: RO
 */
MLXSW_ITEM16(reg, mfde, fw_assert_ext_synd, 0x2e, 0, 16);

/* fw_assert_assert_string
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, mfde, fw_assert_assert_string, 0x30, 0, 32, 4, 0, 4, 0);

/* fatal_cause_test
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, fatal_cause_test, 0x10, 7, 1);

/* fatal_cause_fw_cause
 * Access: RW
 */
MLXSW_ITEM8(reg, mfde, fatal_cause_fw_cause, 0x10, 6, 1);

/* fatal_cause_cause_id
 * Access: RO
 */
MLXSW_ITEM32(reg, mfde, fatal_cause_cause_id, 0x10, 0, 20);

/* fatal_cause_tile_v
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, fatal_cause_tile_v, 0x15, 7, 1);

/* fatal_cause_tile_index
 * Access: RO
 */
MLXSW_ITEM8(reg, mfde, fatal_cause_tile_index, 0x15, 0, 6);

/* mepc
 * Access: RO
 */
MLXSW_ITEM64(reg, mfde, mepc, 0x10, 0, 64);

/* mcause
 * Access: RO
 */
MLXSW_ITEM64(reg, mfde, mcause, 0x18, 0, 64);

/* mtval
 * Access: RO
 */
MLXSW_ITEM64(reg, mfde, mtval, 0x20, 0, 64);

/* mdif
 * ----
 *
 */
/* global
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mdif, global, 0x0, 7, 1);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mdif, lp_msb, 0x2, 4, 2);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mdif, local_port, 0x3, 0, 8);

/* field_select
 * Access: OP
 */
MLXSW_ITEM8(reg, mdif, field_select, 0x7, 0, 3);

/* crc_err_mode
 * Access: WO
 */
MLXSW_ITEM8(reg, mdif, crc_err_mode, 0xb, 1, 1);

/* crc_lanes_mode
 * Access: OP
 */
MLXSW_ITEM8(reg, mdif, crc_lanes_mode, 0xb, 0, 1);

/* inject_sram_ecc
 * Access: OP
 */
MLXSW_ITEM8(reg, mdif, inject_sram_ecc, 0xf, 0, 1);

/* mofrd
 * -----
 *
 */
/* trigger
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mofrd, trigger, 0x3, 0, 8);

/* err_ptr
 * Access: INDEX
 */
MLXSW_ITEM_BIT_ARRAY(reg, mofrd, err_ptr, 0x4, 64, 2);

/* num_errors
 * Access: WO
 */
MLXSW_ITEM32_INDEXED(reg, mofrd, num_errors, 0x44, 0, 32, 4, 0, 4, 0);

/* mpggc
 * -----
 *
 */
/* bp_threshold
 * Access: RW
 */
MLXSW_ITEM16(reg, mpggc, bp_threshold, 0x2, 0, 12);

/* mpgdq
 * -----
 *
 */
/* num_of_entries
 * Access: WO
 */
MLXSW_ITEM16(reg, mpgdq, num_of_entries, 0x0, 0, 16);

/* fw_app_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mpgdq, fw_app_id, 0x4, 0, 8);

/* num_fw_pages
 * Access: RO
 */
MLXSW_ITEM16(reg, mpgdq, num_fw_pages, 0x6, 0, 16);

/* mpgdi
 * -----
 *
 */
/* vld
 * Access: RW
 */
MLXSW_ITEM8(reg, mpgdi, vld, 0x0, 7, 1);

/* pgd_index
 * Access: INDEX
 */
MLXSW_ITEM16(reg, mpgdi, pgd_index, 0x2, 0, 16);

/* fw_app_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mpgdi, fw_app_id, 0x7, 0, 8);

/* ip_ver
 * Access: RW
 */
MLXSW_ITEM8(reg, mpgdi, ip_ver, 0x10, 1, 1);

/* vlan_exist
 * Access: RW
 */
MLXSW_ITEM8(reg, mpgdi, vlan_exist, 0x10, 0, 1);

/* tx_local_port
 * Access: RW
 */
MLXSW_ITEM16(reg, mpgdi, tx_local_port, 0x12, 0, 16);

/* mpb
 * Access: RW
 */
MLXSW_ITEM8(reg, mpgdi, mpb, 0x14, 0, 2);

/* tc_bitmask
 * Access: RW
 */
MLXSW_ITEM16(reg, mpgdi, tc_bitmask, 0x16, 0, 16);

/* pcp_die_for_tc
 * Access: RW
 */
MLXSW_ITEM_BIT_ARRAY(reg, mpgdi, pcp_die_for_tc, 0x18, 8, 4);

/* dscp_for_tc
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, mpgdi, dscp_for_tc, 0x24, 16);

/* packet_length
 * Access: RW
 */
MLXSW_ITEM8(reg, mpgdi, packet_length, 0x37, 0, 8);

/* packet_data
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, mpgdi, packet_data, 0x40, 80);

/* mpgsb
 * -----
 *
 */
/* bind
 * Access: RW
 */
MLXSW_ITEM8(reg, mpgsb, bind, 0x0, 7, 1);

/* sender_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mpgsb, sender_id, 0x3, 0, 4);

/* fw_app_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mpgsb, fw_app_id, 0x4, 0, 8);

/* start_entry_index
 * Access: RW
 */
MLXSW_ITEM16(reg, mpgsb, start_entry_index, 0x6, 0, 16);

/* num_of_entries
 * Access: RW
 */
MLXSW_ITEM16(reg, mpgsb, num_of_entries, 0xa, 0, 16);

/* interval
 * Access: RW
 */
MLXSW_ITEM16(reg, mpgsb, interval, 0xe, 0, 16);

/* mpgee
 * -----
 *
 */
/* bp_above_thr
 * Access: RO
 */
MLXSW_ITEM8(reg, mpgee, bp_above_thr, 0x3, 0, 1);

/* mpgec
 * -----
 *
 */
/* clr
 * Access: WO
 */
MLXSW_ITEM8(reg, mpgec, clr, 0x0, 7, 1);

/* bp_above_thr_cnt
 * Access: RO
 */
MLXSW_ITEM32(reg, mpgec, bp_above_thr_cnt, 0x4, 0, 32);

/* bp_num_cnt
 * Access: RO
 */
MLXSW_ITEM32(reg, mpgec, bp_num_cnt, 0x8, 0, 32);

/* unexpected_pkt_cnt
 * Access: RO
 */
MLXSW_ITEM32(reg, mpgec, unexpected_pkt_cnt, 0xc, 0, 32);

/* mpre
 * ----
 *
 */
/* en
 * Access: RW
 */
MLXSW_ITEM8(reg, mpre, en, 0x0, 7, 1);

/* pool
 * Access: RW
 */
MLXSW_ITEM8(reg, mpre, pool, 0x3, 0, 4);

/* fw_app_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mpre, fw_app_id, 0x4, 0, 8);

/* udp_dport
 * Access: RW
 */
MLXSW_ITEM16(reg, mpre, udp_dport, 0x12, 0, 16);

/* mslme
 * -----
 *
 */
/* num_rec
 * Access: RO
 */
MLXSW_ITEM8(reg, mslme, num_rec, 0x3, 0, 8);

/* error
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, mslme, error, MLXSW_MSLME_LATENCY_RECORD_BASE_LEN, 4, 4, MLXSW_MSLME_LATENCY_RECORD_REC_LEN, 0, 1, 0);

/* ip_ver
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, mslme, ip_ver, MLXSW_MSLME_LATENCY_RECORD_BASE_LEN, 0, 1, MLXSW_MSLME_LATENCY_RECORD_REC_LEN, 0, 1, 0);

/* original_tclass
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, mslme, original_tclass, MLXSW_MSLME_LATENCY_RECORD_BASE_LEN + 1, 0, 5, MLXSW_MSLME_LATENCY_RECORD_REC_LEN, 0, 1, 0);

/* tx_local_port
 * Access: RO
 */
MLXSW_ITEM16_INDEXED(reg, mslme, tx_local_port, MLXSW_MSLME_LATENCY_RECORD_BASE_LEN + 2, 0, 16, MLXSW_MSLME_LATENCY_RECORD_REC_LEN, 0, 2, 0);

/* latency
 * Access: RO
 */
MLXSW_ITEM16_INDEXED(reg, mslme, latency, MLXSW_MSLME_LATENCY_RECORD_BASE_LEN, 0, 16, MLXSW_MSLME_LATENCY_RECORD_REC_LEN, 4, 2, 0);

/* rx_local_port
 * Access: RO
 */
MLXSW_ITEM16_INDEXED(reg, mslme, rx_local_port, MLXSW_MSLME_LATENCY_RECORD_BASE_LEN + 2, 0, 16, MLXSW_MSLME_LATENCY_RECORD_REC_LEN, 4, 2, 0);

/* mslmc
 * -----
 *
 */
/* clr
 * Access: WO
 */
MLXSW_ITEM8(reg, mslmc, clr, 0x0, 7, 1);

/* packet_res_drop_cnt
 * Access: RO
 */
MLXSW_ITEM32(reg, mslmc, packet_res_drop_cnt, 0x4, 0, 32);

/* mpgss
 * -----
 *
 */
/* error_type
 * Access: RO
 */
MLXSW_ITEM8(reg, mpgss, error_type, 0x1, 0, 3);

/* sender_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, mpgss, sender_id, 0x3, 0, 4);

/* opcode
 * Access: WO
 */
MLXSW_ITEM8(reg, mpgss, opcode, 0x6, 0, 4);

/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, mpgss, status, 0x7, 0, 2);

/* igcr
 * ----
 *
 */
/* ddd_lpm_high_ipv6
 * Access: RW
 */
MLXSW_ITEM8(reg, igcr, ddd_lpm_high_ipv6, 0x1, 0, 8);

/* ddd_lpm_high_ipv4
 * Access: RW
 */
MLXSW_ITEM8(reg, igcr, ddd_lpm_high_ipv4, 0x3, 0, 8);

/* ddd_pe_actions_priority
 * Access: RW
 */
MLXSW_ITEM32(reg, igcr, ddd_pe_actions_priority, 0x8, 0, 24);

/* ddd_lag_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, igcr, ddd_lag_mode, 0xe, 0, 4);

/* pisbo
 * Access: RW
 */
MLXSW_ITEM8(reg, igcr, pisbo, 0xf, 0, 1);

/* iddd
 * ----
 *
 */
/* entry_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, iddd, entry_type, 0x3, 0, 8);

/* duplication
 * Access: RW
 */
MLXSW_ITEM8(reg, iddd, duplication, 0x7, 0, 5);

/* iedr
 * ----
 *
 */
/* bg
 * Access: OP
 */
MLXSW_ITEM8(reg, iedr, bg, 0x0, 0, 1);

/* num_rec
 * Access: OP
 */
MLXSW_ITEM8(reg, iedr, num_rec, 0x3, 0, 8);

/* type
 * Access: OP
 */
MLXSW_ITEM8_INDEXED(reg, iedr, type, MLXSW_IEDR_RECORD_BASE_LEN, 0, 8, MLXSW_IEDR_RECORD_REC_LEN, 0, 1, 0);

/* size
 * Access: OP
 */
MLXSW_ITEM16_INDEXED(reg, iedr, size, MLXSW_IEDR_RECORD_BASE_LEN + 2, 0, 13, MLXSW_IEDR_RECORD_REC_LEN, 0, 2, 0);

/* index_start
 * Access: OP
 */
MLXSW_ITEM32_INDEXED(reg, iedr, index_start, MLXSW_IEDR_RECORD_BASE_LEN, 0, 24, MLXSW_IEDR_RECORD_REC_LEN, 4, 3, 0);

/* ieds
 * ----
 *
 */
/* edpr
 * Access: OP
 */
MLXSW_ITEM8(reg, ieds, edpr, 0x3, 0, 1);

/* ed_status
 * Access: RO
 */
MLXSW_ITEM8(reg, ieds, ed_status, 0x7, 0, 4);

/* ifbo
 * ----
 *
 */
/* type
 * Access: RW
 */
MLXSW_ITEM8(reg, ifbo, type, 0x3, 0, 4);

/* outer_header_enables
 * Access: RW
 */
MLXSW_ITEM16(reg, ifbo, outer_header_enables, 0x12, 0, 16);

/* outer_header_fields_enable
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ifbo, outer_header_fields_enable, 0x14, 0, 32, 4, 0, 4, 0);

/* iicr
 * ----
 *
 */
/* kvh_mark_clear
 * Access: OP
 */
MLXSW_ITEM8(reg, iicr, kvh_mark_clear, 0xb, 0, 4);

/* clear_mc_rtr_tcam_value
 * Access: OP
 */
MLXSW_ITEM8(reg, iicr, clear_mc_rtr_tcam_value, 0xf, 4, 2);

/* clear_mc_rtr_tcam_mask
 * Access: OP
 */
MLXSW_ITEM8(reg, iicr, clear_mc_rtr_tcam_mask, 0xf, 0, 2);

/* clear_pe_regions_value
 * Access: OP
 */
MLXSW_ITEM16(reg, iicr, clear_pe_regions_value, 0x12, 0, 16);

/* clear_pe_regions_mask
 * Access: OP
 */
MLXSW_ITEM16(reg, iicr, clear_pe_regions_mask, 0x16, 0, 16);

/* clear_rifs_value
 * Access: OP
 */
MLXSW_ITEM16(reg, iicr, clear_rifs_value, 0x1a, 0, 16);

/* clear_rifs_mask
 * Access: OP
 */
MLXSW_ITEM16(reg, iicr, clear_rifs_mask, 0x1e, 0, 16);

/* traps_2_default
 * Access: OP
 */
MLXSW_ITEM8(reg, iicr, traps_2_default, 0x23, 0, 1);

/* clear_acl_group_ids_value
 * Access: OP
 */
MLXSW_ITEM16(reg, iicr, clear_acl_group_ids_value, 0x26, 0, 16);

/* clear_acl_group_ids_mask
 * Access: OP
 */
MLXSW_ITEM16(reg, iicr, clear_acl_group_ids_mask, 0x2a, 0, 16);

/* clear_acl_group_elements_value
 * Access: OP
 */
MLXSW_ITEM16(reg, iicr, clear_acl_group_elements_value, 0x2e, 0, 16);

/* clear_acl_group_elements_mask
 * Access: OP
 */
MLXSW_ITEM16(reg, iicr, clear_acl_group_elements_mask, 0x32, 0, 16);

/* ipac
 * ----
 *
 */
/* profile_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ipac, profile_index, 0x2, 0, 4);

/* opcode
 * Access: WO
 */
MLXSW_ITEM8(reg, ipac, opcode, 0x6, 0, 2);

/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, ipac, status, 0x7, 0, 2);

/* port_mask
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ipac, port_mask, 0x20, 0, 32, 4, 0, 4, 0);

/* log_op_time
 * Access: RO
 */
MLXSW_ITEM16(reg, ipac, log_op_time, 0xa2, 0, 16);

/* log_op_registers
 * Access: RO
 */
MLXSW_ITEM32(reg, ipac, log_op_registers, 0xa4, 0, 32);

/* error_lp_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, ipac, error_lp_msb, 0xa8, 0, 2);

/* error_local_port
 * Access: RO
 */
MLXSW_ITEM8(reg, ipac, error_local_port, 0xa9, 0, 8);

/* error_register_id
 * Access: RO
 */
MLXSW_ITEM16(reg, ipac, error_register_id, 0xaa, 0, 16);

/* error_status
 * Access: RO
 */
MLXSW_ITEM8(reg, ipac, error_status, 0xad, 0, 7);

/* error_entry_index
 * Access: RO
 */
MLXSW_ITEM16(reg, ipac, error_entry_index, 0xae, 0, 16);

/* ihsr
 * ----
 *
 */
/* hash_profile
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ihsr, hash_profile, 0x1, 0, 3);

/* gsh
 * Access: RW
 */
MLXSW_ITEM8(reg, ihsr, gsh, 0x2, 0, 3);

/* type
 * Access: RW
 */
MLXSW_ITEM8(reg, ihsr, type, 0x3, 0, 4);

/* general_fields
 * Access: RW
 */
MLXSW_ITEM32(reg, ihsr, general_fields, 0xc, 0, 32);

/* outer_header_enables
 * Access: RW
 */
MLXSW_ITEM16(reg, ihsr, outer_header_enables, 0x12, 0, 16);

/* outer_header_fields_enable
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ihsr, outer_header_fields_enable, 0x14, 0, 32, 4, 0, 4, 0);

/* inner_header_enables
 * Access: RW
 */
MLXSW_ITEM16(reg, ihsr, inner_header_enables, 0x2e, 0, 16);

/* inner_header_fields_enable
 * Access: RW
 */
MLXSW_ITEM64(reg, ihsr, inner_header_fields_enable, 0x30, 0, 64);

/* cbsets
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ihsr, cbsets, 0x40, 0, 32, 4, 0, 4, 0);

/* ihscr
 * -----
 *
 */
/* profile_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ihscr, profile_index, 0x0, 5, 3);

/* l2_type
 * Access: RW
 */
MLXSW_ITEM8(reg, ihscr, l2_type, 0x2, 0, 2);

/* inner_l2_type
 * Access: RW
 */
MLXSW_ITEM8(reg, ihscr, inner_l2_type, 0x3, 0, 2);

/* l3_type
 * Access: RW
 */
MLXSW_ITEM8(reg, ihscr, l3_type, 0x7, 0, 8);

/* inner_l3_type
 * Access: RW
 */
MLXSW_ITEM8(reg, ihscr, inner_l3_type, 0xb, 0, 8);

/* ip_frag
 * Access: RW
 */
MLXSW_ITEM8(reg, ihscr, ip_frag, 0xe, 0, 2);

/* inner_ip_frag
 * Access: RW
 */
MLXSW_ITEM8(reg, ihscr, inner_ip_frag, 0xf, 0, 2);

/* l4_type
 * Access: RW
 */
MLXSW_ITEM16(reg, ihscr, l4_type, 0x12, 0, 11);

/* inner_l4_type
 * Access: RW
 */
MLXSW_ITEM16(reg, ihscr, inner_l4_type, 0x16, 0, 11);

/* tunnel_type
 * Access: RW
 */
MLXSW_ITEM16(reg, ihscr, tunnel_type, 0x1a, 0, 9);

/* mask_inner
 * Access: RW
 */
MLXSW_ITEM8(reg, ihscr, mask_inner, 0x1f, 1, 1);

/* mask_outer
 * Access: RW
 */
MLXSW_ITEM8(reg, ihscr, mask_outer, 0x1f, 0, 1);

/* fpp_index
 * Access: RW
 */
MLXSW_ITEM16(reg, ihscr, fpp_index, 0x22, 0, 9);

/* inner_fpp_index
 * Access: RW
 */
MLXSW_ITEM16(reg, ihscr, inner_fpp_index, 0x26, 0, 9);

/* ports_bitmap
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ihscr, ports_bitmap, 0x28, 0, 32, 4, 0, 4, 0);

/* ipsr
 * ----
 *
 */
/* profile_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ipsr, profile_index, 0x2, 0, 4);

/* cap_size
 * Access: RO
 */
MLXSW_ITEM16(reg, ipsr, cap_size, 0xa, 0, 16);

/* current_size
 * Access: RO
 */
MLXSW_ITEM16(reg, ipsr, current_size, 0xe, 0, 16);

/* current_entries
 * Access: RO
 */
MLXSW_ITEM16(reg, ipsr, current_entries, 0x12, 0, 16);

/* icsr
 * ----
 *
 */
/* base_address
 * Access: INDEX
 */
MLXSW_ITEM32(reg, icsr, base_address, 0x4, 0, 32);

/* num_reads
 * Access: OP
 */
MLXSW_ITEM16(reg, icsr, num_reads, 0xa, 0, 9);

/* data
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, icsr, data, 0x10, 0, 32, 4, 0, 4, 0);

/* ipfr
 * ----
 *
 */
/* pgt_base_ptr
 * Access: OP
 */
MLXSW_ITEM16(reg, ipfr, pgt_base_ptr, 0x2, 0, 16);

/* size
 * Access: OP
 */
MLXSW_ITEM16(reg, ipfr, size, 0x6, 0, 16);

/* itgcr
 * -----
 *
 */
/* l4_validity
 * Access: RW
 */
MLXSW_ITEM8(reg, itgcr, l4_validity, 0x3, 0, 1);

/* def_masking_size
 * Access: RW
 */
MLXSW_ITEM16(reg, itgcr, def_masking_size, 0x6, 0, 10);

/* itpr
 * ----
 *
 */
/* trunc_prof
 * Access: INDEX
 */
MLXSW_ITEM8(reg, itpr, trunc_prof, 0x3, 0, 4);

/* tr_size
 * Access: RW
 */
MLXSW_ITEM16(reg, itpr, tr_size, 0x6, 0, 14);

/* ip_len_fix
 * Access: RW
 */
MLXSW_ITEM8(reg, itpr, ip_len_fix, 0xa, 1, 1);

/* dd
 * Access: RW
 */
MLXSW_ITEM8(reg, itpr, dd, 0xa, 0, 1);

/* mode
 * Access: RW
 */
MLXSW_ITEM8(reg, itpr, mode, 0xb, 0, 2);

/* masking_label_en
 * Access: RW
 */
MLXSW_ITEM16(reg, itpr, masking_label_en, 0xe, 0, 9);

/* addition_masking_offset
 * Access: RW
 */
MLXSW_ITEM16(reg, itpr, addition_masking_offset, 0x10, 0, 9);

/* addition_masking_offset_en
 * Access: RW
 */
MLXSW_ITEM16(reg, itpr, addition_masking_offset_en, 0x12, 0, 9);

/* irpcs
 * -----
 *
 */
/* set_mask
 * Access: OP
 */
MLXSW_ITEM8(reg, irpcs, set_mask, 0x0, 4, 4);

/* dip_row_seed
 * Access: RW
 */
MLXSW_ITEM16(reg, irpcs, dip_row_seed, 0x6, 0, 9);

/* dip_entry_seed
 * Access: 
 */
MLXSW_ITEM8_INDEXED(reg, irpcs, dip_entry_seed, 0xb, 0, 5, 0x4, 0x0, 1, 0);

/* ielr
 * ----
 *
 */
/* entry_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ielr, entry_type, 0x3, 0, 8);

/* locality_en
 * Access: RW
 */
MLXSW_ITEM8(reg, ielr, locality_en, 0x7, 0, 1);

/* sgcr
 * ----
 *
 */
/* llb
 * Access: RW
 */
MLXSW_ITEM8(reg, sgcr, llb, 0x7, 0, 1);

/* activity_dis
 * Access: RW
 */
MLXSW_ITEM8(reg, sgcr, activity_dis, 0xb, 0, 8);

/* lag_lookup_pgt_base
 * Access: RW
 */
MLXSW_ITEM16(reg, sgcr, lag_lookup_pgt_base, 0xe, 0, 16);

/* spad
 * ----
 *
 */
/* base_mac
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, spad, base_mac, 0x2, 6);

/* sfdb
 * ----
 *
 */
/* policy
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sfdb, policy, 0x9, 0, 2);

/* update_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sfdb, update_type, 0xb, 0, 4);

/* gfid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sfdb, gfid, 0xc, 7, 1);

/* entry_fid
 * Access: INDEX
 */
MLXSW_ITEM16(reg, sfdb, entry_fid, 0xe, 0, 16);

/* parameter
 * Access: INDEX
 */
MLXSW_ITEM32(reg, sfdb, parameter, 0x20, 0, 32);

/* parameter1
 * Access: INDEX
 */
MLXSW_ITEM32(reg, sfdb, parameter1, 0x24, 0, 32);

/* new_parameter
 * Access: WO
 */
MLXSW_ITEM32(reg, sfdb, new_parameter, 0x40, 0, 32);

/* new_parameter1
 * Access: WO
 */
MLXSW_ITEM32(reg, sfdb, new_parameter1, 0x44, 0, 32);

/* sfdb_v2
 * -------
 *
 */
/* key
 * Access: INDEX
 */
MLXSW_ITEM32_INDEXED(reg, sfdb_v2, key, 0x0, 0, 32, 4, 0, 4, 0);

/* key_mask
 * Access: INDEX
 */
MLXSW_ITEM32_INDEXED(reg, sfdb_v2, key_mask, 0x20, 0, 32, 4, 0, 4, 0);

/* action
 * Access: WO
 */
MLXSW_ITEM32_INDEXED(reg, sfdb_v2, action, 0x40, 0, 32, 4, 0, 4, 0);

/* action_mask
 * Access: WO
 */
MLXSW_ITEM32_INDEXED(reg, sfdb_v2, action_mask, 0x60, 0, 32, 4, 0, 4, 0);

/* count_updates
 * Access: RO
 */
MLXSW_ITEM32(reg, sfdb_v2, count_updates, 0x80, 0, 32);

/* smid_v2
 * -------
 *
 */
/* swid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, smid_v2, swid, 0x0, 0, 8);

/* mid
 * Access: INDEX
 */
MLXSW_ITEM16(reg, smid_v2, mid, 0x2, 0, 16);

/* op
 * Access: OP
 */
MLXSW_ITEM8(reg, smid_v2, op, 0x7, 0, 4);

/* smpe_valid
 * Access: RW
 */
MLXSW_ITEM8(reg, smid_v2, smpe_valid, 0x9, 4, 1);

/* smpe
 * Access: RW
 */
MLXSW_ITEM16(reg, smid_v2, smpe, 0xa, 0, 16);

/* port
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, smid_v2, port, 0x20, 0, 32, 4, 0, 4, 0);

/* mask
 * Access: WO
 */
MLXSW_ITEM32_INDEXED(reg, smid_v2, mask, 0xa0, 0, 32, 4, 0, 4, 0);

/* smpu
 * ----
 *
 */
/* swid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, smpu, swid, 0x0, 0, 8);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, smpu, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, smpu, lp_msb, 0x2, 4, 2);

/* op
 * Access: WO
 */
MLXSW_ITEM8(reg, smpu, op, 0x4, 6, 2);

/* size
 * Access: INDEX
 */
MLXSW_ITEM16(reg, smpu, size, 0x4, 0, 9);

/* mid
 * Access: 
 */
MLXSW_ITEM16_INDEXED(reg, smpu, mid, 0x12, 0, 16, 0x4, 0x0, 2, 0);

/* spms_v2
 * -------
 *
 */
/* allv
 * Access: OP
 */
MLXSW_ITEM8(reg, spms_v2, allv, 0x0, 7, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, spms_v2, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, spms_v2, lp_msb, 0x2, 4, 2);

/* num_rec
 * Access: INDEX
 */
MLXSW_ITEM8(reg, spms_v2, num_rec, 0x3, 0, 8);

/* state
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, spms_v2, state, MLXSW_SPMS_V2_VID_RECORD_BASE_LEN + 2, 4, 2, MLXSW_SPMS_V2_VID_RECORD_REC_LEN, 0, 1, 0);

/* vid
 * Access: INDEX
 */
MLXSW_ITEM16_INDEXED(reg, spms_v2, vid, MLXSW_SPMS_V2_VID_RECORD_BASE_LEN + 2, 0, 12, MLXSW_SPMS_V2_VID_RECORD_REC_LEN, 0, 2, 0);

/* spevet
 * ------
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, spevet, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, spevet, lp_msb, 0x2, 4, 2);

/* et_vlan
 * Access: RW
 */
MLXSW_ITEM8(reg, spevet, et_vlan, 0x5, 0, 2);

/* spaft
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, spaft, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, spaft, lp_msb, 0x2, 4, 2);

/* allow_untagged
 * Access: RW
 */
MLXSW_ITEM8(reg, spaft, allow_untagged, 0x4, 7, 1);

/* allow_priotagged
 * Access: RW
 */
MLXSW_ITEM8(reg, spaft, allow_priotagged, 0x4, 6, 1);

/* allow_tagged
 * Access: RW
 */
MLXSW_ITEM8(reg, spaft, allow_tagged, 0x4, 5, 1);

/* sfgc
 * ----
 *
 */
/* type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sfgc, type, 0x3, 0, 4);

/* bridge_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sfgc, bridge_type, 0x4, 0, 3);

/* table_type
 * Access: RW
 */
MLXSW_ITEM8(reg, sfgc, table_type, 0x5, 0, 3);

/* flood_table
 * Access: RW
 */
MLXSW_ITEM8(reg, sfgc, flood_table, 0x7, 0, 6);

/* counter_set_type
 * Access: RW
 */
MLXSW_ITEM8(reg, sfgc, counter_set_type, 0xc, 0, 8);

/* counter_index
 * Access: RW
 */
MLXSW_ITEM32(reg, sfgc, counter_index, 0xc, 0, 24);

/* mid_base
 * Access: RW
 */
MLXSW_ITEM16(reg, sfgc, mid_base, 0x12, 0, 16);

/* sftr_v2
 * -------
 *
 */
/* swid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sftr_v2, swid, 0x0, 0, 8);

/* flood_table
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sftr_v2, flood_table, 0x1, 0, 6);

/* index
 * Access: INDEX
 */
MLXSW_ITEM16(reg, sftr_v2, index, 0x2, 0, 16);

/* table_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sftr_v2, table_type, 0x5, 0, 3);

/* range
 * Access: INDEX
 */
MLXSW_ITEM16(reg, sftr_v2, range, 0x6, 0, 16);

/* port
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, sftr_v2, port, 0x20, 0, 32, 4, 0, 4, 0);

/* mask
 * Access: WO
 */
MLXSW_ITEM32_INDEXED(reg, sftr_v2, mask, 0xa0, 0, 32, 4, 0, 4, 0);

/* svfa
 * ----
 *
 */
/* swid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, svfa, swid, 0x0, 0, 8);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, svfa, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, svfa, lp_msb, 0x2, 4, 2);

/* mapping_table
 * Access: INDEX
 */
MLXSW_ITEM8(reg, svfa, mapping_table, 0x2, 0, 3);

/* tport
 * Access: INDEX
 */
MLXSW_ITEM8(reg, svfa, tport, 0x3, 7, 1);

/* v
 * Access: RW
 */
MLXSW_ITEM8(reg, svfa, v, 0x3, 0, 1);

/* fid
 * Access: RW
 */
MLXSW_ITEM16(reg, svfa, fid, 0x4, 0, 16);

/* vid
 * Access: INDEX
 */
MLXSW_ITEM16(reg, svfa, vid, 0x6, 0, 12);

/* counter_set_type
 * Access: RW
 */
MLXSW_ITEM8(reg, svfa, counter_set_type, 0x8, 0, 8);

/* counter_index
 * Access: RW
 */
MLXSW_ITEM32(reg, svfa, counter_index, 0x8, 0, 24);

/* trap_action
 * Access: RW
 */
MLXSW_ITEM8(reg, svfa, trap_action, 0xc, 4, 4);

/* vni
 * Access: INDEX
 */
MLXSW_ITEM32(reg, svfa, vni, 0x10, 0, 24);

/* irif_v
 * Access: RW
 */
MLXSW_ITEM8(reg, svfa, irif_v, 0x14, 0, 1);

/* irif
 * Access: RW
 */
MLXSW_ITEM16(reg, svfa, irif, 0x16, 0, 16);

/* sfdf
 * ----
 *
 */
/* swid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sfdf, swid, 0x0, 0, 8);

/* flush_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sfdf, flush_type, 0x4, 4, 4);

/* imdu
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sfdf, imdu, 0x4, 2, 1);

/* iut
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sfdf, iut, 0x4, 1, 1);

/* st
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sfdf, st, 0x4, 0, 1);

/* pb_v
 * Access: RW
 */
MLXSW_ITEM8(reg, sfdf, pb_v, 0x6, 0, 1);

/* pb
 * Access: RW
 */
MLXSW_ITEM8(reg, sfdf, pb, 0x7, 0, 3);

/* flush_per_fid_fid
 * Access: INDEX
 */
MLXSW_ITEM16(reg, sfdf, flush_per_fid_fid, 0xe, 0, 16);

/* flush_per_nve_and_fid_fid
 * Access: INDEX
 */
MLXSW_ITEM16(reg, sfdf, flush_per_nve_and_fid_fid, 0xe, 0, 16);

/* system_port
 * Access: INDEX
 */
MLXSW_ITEM16(reg, sfdf, system_port, 0xe, 0, 16);

/* flush_per_port_and_fid_system_port
 * Access: INDEX
 */
MLXSW_ITEM16(reg, sfdf, flush_per_port_and_fid_system_port, 0xa, 0, 16);

/* flush_per_port_and_fid_fid
 * Access: INDEX
 */
MLXSW_ITEM16(reg, sfdf, flush_per_port_and_fid_fid, 0xe, 0, 16);

/* lag_id
 * Access: INDEX
 */
MLXSW_ITEM16(reg, sfdf, lag_id, 0xe, 0, 10);

/* flush_per_lag_and_fid_lag_id
 * Access: INDEX
 */
MLXSW_ITEM16(reg, sfdf, flush_per_lag_and_fid_lag_id, 0xa, 0, 10);

/* flush_per_lag_and_fid_fid
 * Access: INDEX
 */
MLXSW_ITEM16(reg, sfdf, flush_per_lag_and_fid_fid, 0xe, 0, 16);

/* tport_mask
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sfdf, tport_mask, 0x9, 0, 4);

/* tport
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sfdf, tport, 0xb, 0, 4);

/* flush_per_tunnel_port_fid_tport_mask
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sfdf, flush_per_tunnel_port_fid_tport_mask, 0x9, 0, 4);

/* flush_per_tunnel_port_fid_tport
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sfdf, flush_per_tunnel_port_fid_tport, 0xb, 0, 4);

/* flush_per_tunnel_port_fid_fid
 * Access: INDEX
 */
MLXSW_ITEM16(reg, sfdf, flush_per_tunnel_port_fid_fid, 0xe, 0, 16);

/* flush_mc_dmac_uc_and_fid_fid
 * Access: INDEX
 */
MLXSW_ITEM16(reg, sfdf, flush_mc_dmac_uc_and_fid_fid, 0xe, 0, 16);

/* sfmr
 * ----
 *
 */
/* update
 * Access: OP
 */
MLXSW_ITEM8(reg, sfmr, update, 0x0, 7, 1);

/* op
 * Access: RW
 */
MLXSW_ITEM8(reg, sfmr, op, 0x0, 0, 4);

/* fid
 * Access: INDEX
 */
MLXSW_ITEM16(reg, sfmr, fid, 0x2, 0, 16);

/* update_mask
 * Access: OP
 */
MLXSW_ITEM32(reg, sfmr, update_mask, 0x4, 8, 16);

/* disable_tport_encap
 * Access: RW
 */
MLXSW_ITEM8(reg, sfmr, disable_tport_encap, 0x7, 0, 4);

/* flood_rsp
 * Access: RW
 */
MLXSW_ITEM8(reg, sfmr, flood_rsp, 0x8, 7, 1);

/* flood_bridge_type
 * Access: RW
 */
MLXSW_ITEM8(reg, sfmr, flood_bridge_type, 0x8, 4, 1);

/* fid_offset
 * Access: RW
 */
MLXSW_ITEM16(reg, sfmr, fid_offset, 0xa, 0, 16);

/* vtfp
 * Access: RW
 */
MLXSW_ITEM8(reg, sfmr, vtfp, 0xc, 7, 1);

/* nve_tunnel_flood_ptr
 * Access: RW
 */
MLXSW_ITEM32(reg, sfmr, nve_tunnel_flood_ptr, 0xc, 0, 24);

/* vv
 * Access: RW
 */
MLXSW_ITEM8(reg, sfmr, vv, 0x10, 7, 1);

/* vni
 * Access: RW
 */
MLXSW_ITEM32(reg, sfmr, vni, 0x10, 0, 24);

/* irif_v
 * Access: RW
 */
MLXSW_ITEM8(reg, sfmr, irif_v, 0x14, 0, 1);

/* irif
 * Access: RW
 */
MLXSW_ITEM16(reg, sfmr, irif, 0x16, 0, 16);

/* decap_counter_set_type
 * Access: RW
 */
MLXSW_ITEM8(reg, sfmr, decap_counter_set_type, 0x18, 0, 8);

/* decap_counter_index
 * Access: RW
 */
MLXSW_ITEM32(reg, sfmr, decap_counter_index, 0x18, 0, 24);

/* cff_counter_set_type
 * Access: RW
 */
MLXSW_ITEM8(reg, sfmr, cff_counter_set_type, 0x1c, 0, 8);

/* cff_counter_base_index
 * Access: RW
 */
MLXSW_ITEM32(reg, sfmr, cff_counter_base_index, 0x1c, 0, 24);

/* cff_mid_base
 * Access: RW
 */
MLXSW_ITEM16(reg, sfmr, cff_mid_base, 0x22, 0, 16);

/* tunnel_port_lbf_bitmap
 * Access: RW
 */
MLXSW_ITEM8(reg, sfmr, tunnel_port_lbf_bitmap, 0x25, 0, 4);

/* nve_flood_prf_id
 * Access: RW
 */
MLXSW_ITEM8(reg, sfmr, nve_flood_prf_id, 0x26, 0, 2);

/* cff_prf_id
 * Access: RW
 */
MLXSW_ITEM8(reg, sfmr, cff_prf_id, 0x27, 0, 2);

/* smpe_valid
 * Access: RW
 */
MLXSW_ITEM8(reg, sfmr, smpe_valid, 0x29, 4, 1);

/* smpe
 * Access: RW
 */
MLXSW_ITEM16(reg, sfmr, smpe, 0x2a, 0, 16);

/* spfsr
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, spfsr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, spfsr, lp_msb, 0x2, 4, 2);

/* security
 * Access: RW
 */
MLXSW_ITEM8(reg, spfsr, security, 0x4, 7, 1);

/* sffp
 * ----
 *
 */
/* profile_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sffp, profile_id, 0x1, 0, 2);

/* type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sffp, type, 0x3, 0, 4);

/* flood_offset
 * Access: RW
 */
MLXSW_ITEM8(reg, sffp, flood_offset, 0x7, 0, 3);

/* counter_en
 * Access: RW
 */
MLXSW_ITEM8(reg, sffp, counter_en, 0xa, 0, 1);

/* counter_offset
 * Access: RW
 */
MLXSW_ITEM8(reg, sffp, counter_offset, 0xb, 0, 3);

/* smpe
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, smpe, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, smpe, lp_msb, 0x2, 4, 2);

/* smpe_index
 * Access: INDEX
 */
MLXSW_ITEM16(reg, smpe, smpe_index, 0x6, 0, 16);

/* evid
 * Access: RW
 */
MLXSW_ITEM16(reg, smpe, evid, 0xa, 0, 12);

/* smpeb
 * -----
 *
 */
/* smpe_index
 * Access: INDEX
 */
MLXSW_ITEM16(reg, smpeb, smpe_index, 0x6, 0, 16);

/* elport_page
 * Access: INDEX
 */
MLXSW_ITEM8(reg, smpeb, elport_page, 0xb, 0, 4);

/* update
 * Access: OP
 */
MLXSW_ITEM8_INDEXED(reg, smpeb, update, 0x20, 7, 1, 0x4, 0x0, 1, 0);

/* type
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, smpeb, type, 0x20, 4, 3, 0x4, 0x0, 1, 0);

/* value
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, smpeb, value, 0x20, 0, 20, 0x4, 0x0, 3, 0);

/* slmt
 * ----
 *
 */
/* new_lag_lookup_pgt_base
 * Access: WO
 */
MLXSW_ITEM16(reg, slmt, new_lag_lookup_pgt_base, 0x2, 0, 16);

/* change_ddd_lag_mode
 * Access: OP
 */
MLXSW_ITEM8(reg, slmt, change_ddd_lag_mode, 0x4, 7, 1);

/* new_ddd_lag_mode
 * Access: WO
 */
MLXSW_ITEM8(reg, slmt, new_ddd_lag_mode, 0x4, 6, 1);

/* lag_cnt
 * Access: OP
 */
MLXSW_ITEM16(reg, slmt, lag_cnt, 0x6, 0, 10);

/* sfdt_v2
 * -------
 *
 */
/* fgl_ptr
 * Access: INDEX
 */
MLXSW_ITEM16(reg, sfdt_v2, fgl_ptr, 0x2, 0, 16);

/* size
 * Access: OP
 */
MLXSW_ITEM8(reg, sfdt_v2, size, 0x7, 0, 8);

/* port_member
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, sfdt_v2, port_member, 0x10, 0, 16, 0x10, 0x0, 2, 0);

/* sfdat
 * -----
 *
 */
/* swid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sfdat, swid, 0x0, 0, 8);

/* age_time
 * Access: RW
 */
MLXSW_ITEM32(reg, sfdat, age_time, 0x4, 0, 20);

/* spzr
 * ----
 *
 */
/* swid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, spzr, swid, 0x0, 0, 8);

/* router_entity
 * Access: INDEX
 */
MLXSW_ITEM8(reg, spzr, router_entity, 0x1, 1, 1);

/* cm2
 * Access: WO
 */
MLXSW_ITEM8(reg, spzr, cm2, 0x1, 0, 1);

/* ndm
 * Access: WO
 */
MLXSW_ITEM8(reg, spzr, ndm, 0x2, 7, 1);

/* enh_sw_p0_mask
 * Access: WO
 */
MLXSW_ITEM8(reg, spzr, enh_sw_p0_mask, 0x2, 6, 1);

/* cm
 * Access: WO
 */
MLXSW_ITEM8(reg, spzr, cm, 0x2, 5, 1);

/* vk
 * Access: WO
 */
MLXSW_ITEM8(reg, spzr, vk, 0x2, 4, 1);

/* mp
 * Access: WO
 */
MLXSW_ITEM8(reg, spzr, mp, 0x2, 3, 1);

/* sig
 * Access: WO
 */
MLXSW_ITEM8(reg, spzr, sig, 0x2, 2, 1);

/* ng
 * Access: WO
 */
MLXSW_ITEM8(reg, spzr, ng, 0x2, 1, 1);

/* g0
 * Access: WO
 */
MLXSW_ITEM8(reg, spzr, g0, 0x2, 0, 1);

/* enh_sw_p0
 * Access: RW
 */
MLXSW_ITEM8(reg, spzr, enh_sw_p0, 0x3, 1, 1);

/* capability_mask
 * Access: RW
 */
MLXSW_ITEM32(reg, spzr, capability_mask, 0x4, 0, 32);

/* system_image_guid_h
 * Access: RW
 */
MLXSW_ITEM32(reg, spzr, system_image_guid_h, 0x8, 0, 32);

/* system_image_guid_l
 * Access: RW
 */
MLXSW_ITEM32(reg, spzr, system_image_guid_l, 0xc, 0, 32);

/* guid0_h
 * Access: RW
 */
MLXSW_ITEM32(reg, spzr, guid0_h, 0x10, 0, 32);

/* guid0_l
 * Access: RW
 */
MLXSW_ITEM32(reg, spzr, guid0_l, 0x14, 0, 32);

/* node_guid_h
 * Access: RW
 */
MLXSW_ITEM32(reg, spzr, node_guid_h, 0x18, 0, 32);

/* node_guid_l
 * Access: RW
 */
MLXSW_ITEM32(reg, spzr, node_guid_l, 0x1c, 0, 32);

/* v_key_h
 * Access: RW
 */
MLXSW_ITEM32(reg, spzr, v_key_h, 0x20, 0, 32);

/* v_key_l
 * Access: RW
 */
MLXSW_ITEM32(reg, spzr, v_key_l, 0x24, 0, 32);

/* capability_mask2
 * Access: RW
 */
MLXSW_ITEM32(reg, spzr, capability_mask2, 0x28, 0, 32);

/* max_pkey
 * Access: RO
 */
MLXSW_ITEM16(reg, spzr, max_pkey, 0x2c, 0, 16);

/* node_description
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, spzr, node_description, 0x30, 64);

/* ibfmr
 * -----
 *
 */
/* atm_v
 * Access: RO
 */
MLXSW_ITEM8(reg, ibfmr, atm_v, 0x3, 0, 1);

/* attribute_id
 * Access: RO
 */
MLXSW_ITEM16(reg, ibfmr, attribute_id, 0xa, 0, 16);

/* attribute_modifier
 * Access: RO
 */
MLXSW_ITEM32(reg, ibfmr, attribute_modifier, 0xc, 0, 32);

/* ibfmrc
 * ------
 *
 */
/* attr_id_en
 * Access: RW
 */
MLXSW_ITEM32(reg, ibfmrc, attr_id_en, 0x0, 0, 32);

/* ibissu
 * ------
 *
 */
/* sm_approve
 * Access: RO
 */
MLXSW_ITEM8(reg, ibissu, sm_approve, 0x2, 0, 2);

/* req_to_sm
 * Access: RW
 */
MLXSW_ITEM8(reg, ibissu, req_to_sm, 0x3, 0, 1);

/* pcam
 * ----
 *
 */
/* feature_group
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pcam, feature_group, 0x1, 0, 8);

/* pl_op
 * Access: OP
 */
MLXSW_ITEM8(reg, pcam, pl_op, 0x2, 0, 1);

/* access_reg_group
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pcam, access_reg_group, 0x3, 0, 8);

/* port_access_reg_cap_mask
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, pcam, port_access_reg_cap_mask, 0x8, 0, 32, 4, 0, 4, 0);

/* feature_cap_mask
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, pcam, feature_cap_mask, 0x28, 0, 32, 4, 0, 4, 0);

/* ptys
 * ----
 *
 */
/* reserved_high
 * Access: RO
 */
MLXSW_ITEM8(reg, ptys, reserved_high, 0x0, 7, 1);

/* an_disable_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, ptys, an_disable_admin, 0x0, 6, 1);

/* an_disable_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ptys, an_disable_cap, 0x0, 5, 1);

/* ee_tx_ready
 * Access: WO
 */
MLXSW_ITEM8(reg, ptys, ee_tx_ready, 0x0, 4, 1);

/* tx_ready_e
 * Access: RW
 */
MLXSW_ITEM8(reg, ptys, tx_ready_e, 0x0, 2, 2);

/* dis_optic_neg_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, ptys, dis_optic_neg_admin, 0x0, 1, 1);

/* force_tx_aba_param
 * Access: RO
 */
MLXSW_ITEM8(reg, ptys, force_tx_aba_param, 0x0, 0, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ptys, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ptys, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ptys, lp_msb, 0x2, 4, 2);

/* port_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ptys, port_type, 0x2, 0, 4);

/* plane_ind
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ptys, plane_ind, 0x3, 4, 4);

/* transmit_allowed
 * Access: RW
 */
MLXSW_ITEM8(reg, ptys, transmit_allowed, 0x3, 3, 1);

/* proto_mask
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ptys, proto_mask, 0x3, 0, 3);

/* an_status
 * Access: RO
 */
MLXSW_ITEM8(reg, ptys, an_status, 0x4, 4, 4);

/* max_port_rate
 * Access: RO
 */
MLXSW_ITEM16(reg, ptys, max_port_rate, 0x4, 0, 12);

/* data_rate_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, ptys, data_rate_oper, 0x6, 0, 16);

/* ext_eth_proto_capability
 * Access: RO
 */
MLXSW_ITEM32(reg, ptys, ext_eth_proto_capability, 0x8, 0, 32);

/* eth_proto_capability
 * Access: RO
 */
MLXSW_ITEM32(reg, ptys, eth_proto_capability, 0xc, 0, 32);

/* ib_link_width_capability
 * Access: RO
 */
MLXSW_ITEM16(reg, ptys, ib_link_width_capability, 0x10, 0, 16);

/* ib_proto_capability
 * Access: RO
 */
MLXSW_ITEM16(reg, ptys, ib_proto_capability, 0x12, 0, 16);

/* ext_eth_proto_admin
 * Access: RW
 */
MLXSW_ITEM32(reg, ptys, ext_eth_proto_admin, 0x14, 0, 32);

/* eth_proto_admin
 * Access: RW
 */
MLXSW_ITEM32(reg, ptys, eth_proto_admin, 0x18, 0, 32);

/* ib_link_width_admin
 * Access: RW
 */
MLXSW_ITEM16(reg, ptys, ib_link_width_admin, 0x1c, 0, 16);

/* ib_proto_admin
 * Access: RW
 */
MLXSW_ITEM16(reg, ptys, ib_proto_admin, 0x1e, 0, 16);

/* ext_eth_proto_oper
 * Access: RO
 */
MLXSW_ITEM32(reg, ptys, ext_eth_proto_oper, 0x20, 0, 32);

/* eth_proto_oper
 * Access: RO
 */
MLXSW_ITEM32(reg, ptys, eth_proto_oper, 0x24, 0, 32);

/* ib_link_width_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, ptys, ib_link_width_oper, 0x28, 0, 16);

/* ib_proto_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, ptys, ib_proto_oper, 0x2a, 0, 16);

/* xdr_2x_slow_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ptys, xdr_2x_slow_cap, 0x2c, 7, 1);

/* force_lt_frames_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ptys, force_lt_frames_cap, 0x2c, 6, 1);

/* force_lt_frames_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, ptys, force_lt_frames_admin, 0x2c, 4, 2);

/* ext_eth_proto_lp_valid
 * Access: RO
 */
MLXSW_ITEM8(reg, ptys, ext_eth_proto_lp_valid, 0x2c, 3, 1);

/* eth_proto_lp_valid
 * Access: RO
 */
MLXSW_ITEM8(reg, ptys, eth_proto_lp_valid, 0x2c, 2, 1);

/* xdr_2x_slow_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, ptys, xdr_2x_slow_admin, 0x2c, 1, 1);

/* xdr_2x_slow_active
 * Access: RO
 */
MLXSW_ITEM8(reg, ptys, xdr_2x_slow_active, 0x2c, 0, 1);

/* lane_rate_oper
 * Access: RO
 */
MLXSW_ITEM32(reg, ptys, lane_rate_oper, 0x2c, 4, 20);

/* connector_type
 * Access: RO
 */
MLXSW_ITEM8(reg, ptys, connector_type, 0x2f, 0, 4);

/* link_partner_protocol_extended_ib
 * Access: RO
 */
MLXSW_ITEM32(reg, ptys, link_partner_protocol_extended_ib, 0x30, 0, 32);

/* link_partner_protocol_extended_eth
 * Access: RO
 */
MLXSW_ITEM32(reg, ptys, link_partner_protocol_extended_eth, 0x30, 0, 32);

/* eth_proto_lp_advertise
 * Access: RO
 */
MLXSW_ITEM32(reg, ptys, eth_proto_lp_advertise, 0x34, 0, 32);

/* ext_ib_proto_cap_ptys_ext_proto_ib_cap_oper_ext_proto_ib
 * Access: RO
 */
MLXSW_ITEM32(reg, ptys, ext_ib_proto_cap_ptys_ext_proto_ib_cap_oper_ext_proto_ib, 0x38, 0, 32);

/* ext_ib_proto_cap_ext_proto_nvlink
 * Access: RO
 */
MLXSW_ITEM32(reg, ptys, ext_ib_proto_cap_ext_proto_nvlink, 0x38, 0, 32);

/* ptys_ext_proto_ib_admin_ext_proto_ib
 * Access: RW
 */
MLXSW_ITEM32(reg, ptys, ptys_ext_proto_ib_admin_ext_proto_ib, 0x3c, 0, 32);

/* ext_proto_nvlink
 * Access: RW
 */
MLXSW_ITEM32(reg, ptys, ext_proto_nvlink, 0x3c, 0, 32);

/* ext_ib_proto_oper_ptys_ext_proto_ib_cap_oper_ext_proto_ib
 * Access: RO
 */
MLXSW_ITEM32(reg, ptys, ext_ib_proto_oper_ptys_ext_proto_ib_cap_oper_ext_proto_ib, 0x40, 0, 32);

/* ext_ib_proto_oper_ext_proto_nvlink
 * Access: RO
 */
MLXSW_ITEM32(reg, ptys, ext_ib_proto_oper_ext_proto_nvlink, 0x40, 0, 32);

/* paos
 * ----
 *
 */
/* swid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, paos, swid, 0x0, 0, 8);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, paos, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, paos, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, paos, lp_msb, 0x2, 4, 2);

/* admin_status
 * Access: RW
 */
MLXSW_ITEM8(reg, paos, admin_status, 0x2, 0, 4);

/* plane_ind
 * Access: INDEX
 */
MLXSW_ITEM8(reg, paos, plane_ind, 0x3, 4, 4);

/* oper_status
 * Access: RO
 */
MLXSW_ITEM8(reg, paos, oper_status, 0x3, 0, 4);

/* ase
 * Access: WO
 */
MLXSW_ITEM8(reg, paos, ase, 0x4, 7, 1);

/* ee
 * Access: WO
 */
MLXSW_ITEM8(reg, paos, ee, 0x4, 6, 1);

/* ee_ls
 * Access: WO
 */
MLXSW_ITEM8(reg, paos, ee_ls, 0x4, 5, 1);

/* ee_ps
 * Access: WO
 */
MLXSW_ITEM8(reg, paos, ee_ps, 0x4, 4, 1);

/* ee_nmxas
 * Access: WO
 */
MLXSW_ITEM8(reg, paos, ee_nmxas, 0x4, 3, 1);

/* ls_e
 * Access: RW
 */
MLXSW_ITEM8(reg, paos, ls_e, 0x5, 4, 4);

/* logical_state_status
 * Access: RO
 */
MLXSW_ITEM8(reg, paos, logical_state_status, 0x5, 0, 3);

/* ps_e
 * Access: RW
 */
MLXSW_ITEM8(reg, paos, ps_e, 0x6, 4, 4);

/* lock_en
 * Access: RW
 */
MLXSW_ITEM8(reg, paos, lock_en, 0x6, 2, 1);

/* sleep_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, paos, sleep_cap, 0x6, 1, 1);

/* fd
 * Access: RW
 */
MLXSW_ITEM8(reg, paos, fd, 0x6, 0, 1);

/* physical_state_status
 * Access: RO
 */
MLXSW_ITEM8(reg, paos, physical_state_status, 0x7, 4, 4);

/* e
 * Access: RW
 */
MLXSW_ITEM8(reg, paos, e, 0x7, 0, 2);

/* lock_mode
 * Access: RO
 */
MLXSW_ITEM8(reg, paos, lock_mode, 0x8, 0, 6);

/* last_oper_status
 * Access: RO
 */
MLXSW_ITEM8(reg, paos, last_oper_status, 0x9, 0, 3);

/* ps_e_ext
 * Access: RW
 */
MLXSW_ITEM8(reg, paos, ps_e_ext, 0xa, 4, 2);

/* nmxas_e
 * Access: RW
 */
MLXSW_ITEM8(reg, paos, nmxas_e, 0xb, 4, 4);

/* nmx_adminstate_status
 * Access: RO
 */
MLXSW_ITEM8(reg, paos, nmx_adminstate_status, 0xb, 0, 3);

/* pltc
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pltc, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pltc, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pltc, lp_msb, 0x2, 4, 2);

/* lane_mask
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pltc, lane_mask, 0x3, 0, 8);

/* gray_rx_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pltc, gray_rx_cap, 0x4, 7, 1);

/* simplex_lane_role_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pltc, simplex_lane_role_cap, 0x4, 6, 1);

/* gray_tx_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pltc, gray_tx_cap, 0x4, 5, 1);

/* ph_pol_rx_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pltc, ph_pol_rx_cap, 0x4, 3, 1);

/* ph_pol_tx_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pltc, ph_pol_tx_cap, 0x4, 2, 1);

/* precoding_cap_mask
 * Access: RO
 */
MLXSW_ITEM8(reg, pltc, precoding_cap_mask, 0x7, 0, 4);

/* gray_rx_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, pltc, gray_rx_admin, 0x8, 6, 2);

/* gray_tx_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, pltc, gray_tx_admin, 0x8, 4, 2);

/* ph_pol_rx_admin
 * Access: RO
 */
MLXSW_ITEM8(reg, pltc, ph_pol_rx_admin, 0x8, 3, 1);

/* ph_pol_tx_admin
 * Access: RO
 */
MLXSW_ITEM8(reg, pltc, ph_pol_tx_admin, 0x8, 2, 1);

/* simplex_lane_role
 * Access: RW
 */
MLXSW_ITEM8(reg, pltc, simplex_lane_role, 0x8, 0, 2);

/* local_rx_precoding_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, pltc, local_rx_precoding_admin, 0x9, 0, 2);

/* local_tx_precoding_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, pltc, local_tx_precoding_admin, 0xb, 0, 2);

/* gray_rx_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, pltc, gray_rx_oper, 0xc, 6, 2);

/* gray_tx_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, pltc, gray_tx_oper, 0xc, 4, 2);

/* local_rx_precoding_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, pltc, local_rx_precoding_oper, 0xd, 0, 2);

/* local_tx_precoding_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, pltc, local_tx_precoding_oper, 0xf, 0, 2);

/* ppcnt
 * -----
 *
 */
/* swid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppcnt, swid, 0x0, 0, 8);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppcnt, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppcnt, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppcnt, lp_msb, 0x2, 4, 2);

/* port_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppcnt, port_type, 0x2, 0, 4);

/* grp
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppcnt, grp, 0x3, 0, 6);

/* clr
 * Access: OP
 */
MLXSW_ITEM8(reg, ppcnt, clr, 0x4, 7, 1);

/* lp_gl
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppcnt, lp_gl, 0x4, 6, 1);

/* counters_cap
 * Access: OP
 */
MLXSW_ITEM8(reg, ppcnt, counters_cap, 0x4, 5, 1);

/* plane_ind
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppcnt, plane_ind, 0x6, 0, 4);

/* grp_profile
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppcnt, grp_profile, 0x7, 5, 3);

/* prio_tc
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppcnt, prio_tc, 0x7, 0, 5);

/* a_frames_transmitted_ok_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_frames_transmitted_ok_high, 0x8, 0, 32);

/* a_frames_transmitted_ok_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_frames_transmitted_ok_low, 0xc, 0, 32);

/* a_frames_received_ok_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_frames_received_ok_high, 0x10, 0, 32);

/* a_frames_received_ok_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_frames_received_ok_low, 0x14, 0, 32);

/* a_frame_check_sequence_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_frame_check_sequence_errors_high, 0x18, 0, 32);

/* a_frame_check_sequence_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_frame_check_sequence_errors_low, 0x1c, 0, 32);

/* a_alignment_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_alignment_errors_high, 0x20, 0, 32);

/* a_alignment_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_alignment_errors_low, 0x24, 0, 32);

/* a_octets_transmitted_ok_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_octets_transmitted_ok_high, 0x28, 0, 32);

/* a_octets_transmitted_ok_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_octets_transmitted_ok_low, 0x2c, 0, 32);

/* a_octets_received_ok_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_octets_received_ok_high, 0x30, 0, 32);

/* a_octets_received_ok_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_octets_received_ok_low, 0x34, 0, 32);

/* a_multicast_frames_xmitted_ok_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_multicast_frames_xmitted_ok_high, 0x38, 0, 32);

/* a_multicast_frames_xmitted_ok_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_multicast_frames_xmitted_ok_low, 0x3c, 0, 32);

/* a_broadcast_frames_xmitted_ok_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_broadcast_frames_xmitted_ok_high, 0x40, 0, 32);

/* a_broadcast_frames_xmitted_ok_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_broadcast_frames_xmitted_ok_low, 0x44, 0, 32);

/* a_multicast_frames_received_ok_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_multicast_frames_received_ok_high, 0x48, 0, 32);

/* a_multicast_frames_received_ok_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_multicast_frames_received_ok_low, 0x4c, 0, 32);

/* a_broadcast_frames_received_ok_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_broadcast_frames_received_ok_high, 0x50, 0, 32);

/* a_broadcast_frames_received_ok_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_broadcast_frames_received_ok_low, 0x54, 0, 32);

/* a_in_range_length_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_in_range_length_errors_high, 0x58, 0, 32);

/* a_in_range_length_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_in_range_length_errors_low, 0x5c, 0, 32);

/* a_out_of_range_length_field_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_out_of_range_length_field_high, 0x60, 0, 32);

/* a_out_of_range_length_field_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_out_of_range_length_field_low, 0x64, 0, 32);

/* a_frame_too_long_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_frame_too_long_errors_high, 0x68, 0, 32);

/* a_frame_too_long_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_frame_too_long_errors_low, 0x6c, 0, 32);

/* a_symbol_error_during_carrier_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_symbol_error_during_carrier_high, 0x70, 0, 32);

/* a_symbol_error_during_carrier_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_symbol_error_during_carrier_low, 0x74, 0, 32);

/* a_mac_control_frames_transmitted_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_mac_control_frames_transmitted_high, 0x78, 0, 32);

/* a_mac_control_frames_transmitted_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_mac_control_frames_transmitted_low, 0x7c, 0, 32);

/* a_mac_control_frames_received_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_mac_control_frames_received_high, 0x80, 0, 32);

/* a_mac_control_frames_received_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_mac_control_frames_received_low, 0x84, 0, 32);

/* a_unsupported_opcodes_received_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_unsupported_opcodes_received_high, 0x88, 0, 32);

/* a_unsupported_opcodes_received_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_unsupported_opcodes_received_low, 0x8c, 0, 32);

/* a_pause_mac_ctrl_frames_received_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_pause_mac_ctrl_frames_received_high, 0x90, 0, 32);

/* a_pause_mac_ctrl_frames_received_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_pause_mac_ctrl_frames_received_low, 0x94, 0, 32);

/* a_pause_mac_ctrl_frames_transmitted_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_pause_mac_ctrl_frames_transmitted_high, 0x98, 0, 32);

/* a_pause_mac_ctrl_frames_transmitted_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, a_pause_mac_ctrl_frames_transmitted_low, 0x9c, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_in_octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_in_octets_high, 0x8, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_in_octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_in_octets_low, 0xc, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_in_ucast_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_in_ucast_pkts_high, 0x10, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_in_ucast_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_in_ucast_pkts_low, 0x14, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_in_discards_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_in_discards_high, 0x18, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_in_discards_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_in_discards_low, 0x1c, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_in_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_in_errors_high, 0x20, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_in_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_in_errors_low, 0x24, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_in_unknown_protos_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_in_unknown_protos_high, 0x28, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_in_unknown_protos_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_in_unknown_protos_low, 0x2c, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_out_octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_out_octets_high, 0x30, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_out_octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_out_octets_low, 0x34, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_out_ucast_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_out_ucast_pkts_high, 0x38, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_out_ucast_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_out_ucast_pkts_low, 0x3c, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_out_discards_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_out_discards_high, 0x40, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_out_discards_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_out_discards_low, 0x44, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_out_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_out_errors_high, 0x48, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_out_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_out_errors_low, 0x4c, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_in_multicast_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_in_multicast_pkts_high, 0x50, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_in_multicast_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_in_multicast_pkts_low, 0x54, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_in_broadcast_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_in_broadcast_pkts_high, 0x58, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_in_broadcast_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_in_broadcast_pkts_low, 0x5c, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_out_multicast_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_out_multicast_pkts_high, 0x60, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_out_multicast_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_out_multicast_pkts_low, 0x64, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_out_broadcast_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_out_broadcast_pkts_high, 0x68, 0, 32);

/* eth_2863_cntrs_grp_data_layout_if_out_broadcast_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_2863_cntrs_grp_data_layout_if_out_broadcast_pkts_low, 0x6c, 0, 32);

/* ether_stats_drop_events_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_drop_events_high, 0x8, 0, 32);

/* ether_stats_drop_events_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_drop_events_low, 0xc, 0, 32);

/* ether_stats_octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_octets_high, 0x10, 0, 32);

/* ether_stats_octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_octets_low, 0x14, 0, 32);

/* ether_stats_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts_high, 0x18, 0, 32);

/* ether_stats_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts_low, 0x1c, 0, 32);

/* ether_stats_broadcast_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_broadcast_pkts_high, 0x20, 0, 32);

/* ether_stats_broadcast_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_broadcast_pkts_low, 0x24, 0, 32);

/* ether_stats_multicast_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_multicast_pkts_high, 0x28, 0, 32);

/* ether_stats_multicast_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_multicast_pkts_low, 0x2c, 0, 32);

/* ether_stats_crc_align_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_crc_align_errors_high, 0x30, 0, 32);

/* ether_stats_crc_align_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_crc_align_errors_low, 0x34, 0, 32);

/* ether_stats_undersize_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_undersize_pkts_high, 0x38, 0, 32);

/* ether_stats_undersize_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_undersize_pkts_low, 0x3c, 0, 32);

/* ether_stats_oversize_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_oversize_pkts_high, 0x40, 0, 32);

/* ether_stats_oversize_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_oversize_pkts_low, 0x44, 0, 32);

/* ether_stats_fragments_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_fragments_high, 0x48, 0, 32);

/* ether_stats_fragments_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_fragments_low, 0x4c, 0, 32);

/* ether_stats_jabbers_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_jabbers_high, 0x50, 0, 32);

/* ether_stats_jabbers_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_jabbers_low, 0x54, 0, 32);

/* ether_stats_collisions_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_collisions_high, 0x58, 0, 32);

/* ether_stats_collisions_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_collisions_low, 0x5c, 0, 32);

/* ether_stats_pkts64octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts64octets_high, 0x60, 0, 32);

/* ether_stats_pkts64octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts64octets_low, 0x64, 0, 32);

/* ether_stats_pkts65to127octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts65to127octets_high, 0x68, 0, 32);

/* ether_stats_pkts65to127octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts65to127octets_low, 0x6c, 0, 32);

/* ether_stats_pkts128to255octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts128to255octets_high, 0x70, 0, 32);

/* ether_stats_pkts128to255octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts128to255octets_low, 0x74, 0, 32);

/* ether_stats_pkts256to511octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts256to511octets_high, 0x78, 0, 32);

/* ether_stats_pkts256to511octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts256to511octets_low, 0x7c, 0, 32);

/* ether_stats_pkts512to1023octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts512to1023octets_high, 0x80, 0, 32);

/* ether_stats_pkts512to1023octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts512to1023octets_low, 0x84, 0, 32);

/* ether_stats_pkts1024to1518octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts1024to1518octets_high, 0x88, 0, 32);

/* ether_stats_pkts1024to1518octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts1024to1518octets_low, 0x8c, 0, 32);

/* ether_stats_pkts1519to2047octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts1519to2047octets_high, 0x90, 0, 32);

/* ether_stats_pkts1519to2047octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts1519to2047octets_low, 0x94, 0, 32);

/* ether_stats_pkts2048to4095octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts2048to4095octets_high, 0x98, 0, 32);

/* ether_stats_pkts2048to4095octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts2048to4095octets_low, 0x9c, 0, 32);

/* ether_stats_pkts4096to8191octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts4096to8191octets_high, 0xa0, 0, 32);

/* ether_stats_pkts4096to8191octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts4096to8191octets_low, 0xa4, 0, 32);

/* ether_stats_pkts8192to10239octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts8192to10239octets_high, 0xa8, 0, 32);

/* ether_stats_pkts8192to10239octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ether_stats_pkts8192to10239octets_low, 0xac, 0, 32);

/* dot3stats_alignment_errors_high
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_alignment_errors_high, 0x8, 0, 32);

/* dot3stats_alignment_errors_low
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_alignment_errors_low, 0xc, 0, 32);

/* dot3stats_fcs_errors_high
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_fcs_errors_high, 0x10, 0, 32);

/* dot3stats_fcs_errors_low
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_fcs_errors_low, 0x14, 0, 32);

/* dot3stats_single_collision_frames_high
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_single_collision_frames_high, 0x18, 0, 32);

/* dot3stats_single_collision_frames_low
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_single_collision_frames_low, 0x1c, 0, 32);

/* dot3stats_multiple_collision_frames_high
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_multiple_collision_frames_high, 0x20, 0, 32);

/* dot3stats_multiple_collision_frames_low
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_multiple_collision_frames_low, 0x24, 0, 32);

/* dot3stats_sqe_test_errors_high
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_sqe_test_errors_high, 0x28, 0, 32);

/* dot3stats_sqe_test_errors_low
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_sqe_test_errors_low, 0x2c, 0, 32);

/* dot3stats_deferred_transmissions_high
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_deferred_transmissions_high, 0x30, 0, 32);

/* dot3stats_deferred_transmissions_low
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_deferred_transmissions_low, 0x34, 0, 32);

/* dot3stats_late_collisions_high
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_late_collisions_high, 0x38, 0, 32);

/* dot3stats_late_collisions_low
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_late_collisions_low, 0x3c, 0, 32);

/* dot3stats_excessive_collisions_high
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_excessive_collisions_high, 0x40, 0, 32);

/* dot3stats_excessive_collisions_low
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_excessive_collisions_low, 0x44, 0, 32);

/* dot3stats_internal_mac_transmit_errors_high
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_internal_mac_transmit_errors_high, 0x48, 0, 32);

/* dot3stats_internal_mac_transmit_errors_low
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_internal_mac_transmit_errors_low, 0x4c, 0, 32);

/* dot3stats_carrier_sense_errors_high
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_carrier_sense_errors_high, 0x50, 0, 32);

/* dot3stats_carrier_sense_errors_low
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_carrier_sense_errors_low, 0x54, 0, 32);

/* dot3stats_frame_too_longs_high
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_frame_too_longs_high, 0x58, 0, 32);

/* dot3stats_frame_too_longs_low
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_frame_too_longs_low, 0x5c, 0, 32);

/* dot3stats_internal_mac_receive_errors_high
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_internal_mac_receive_errors_high, 0x60, 0, 32);

/* dot3stats_internal_mac_receive_errors_low
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_internal_mac_receive_errors_low, 0x64, 0, 32);

/* dot3stats_symbol_errors_high
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_symbol_errors_high, 0x68, 0, 32);

/* dot3stats_symbol_errors_low
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3stats_symbol_errors_low, 0x6c, 0, 32);

/* dot3control_in_unknown_opcodes_high
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3control_in_unknown_opcodes_high, 0x70, 0, 32);

/* dot3control_in_unknown_opcodes_low
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3control_in_unknown_opcodes_low, 0x74, 0, 32);

/* dot3in_pause_frames_high
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3in_pause_frames_high, 0x78, 0, 32);

/* dot3in_pause_frames_low
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3in_pause_frames_low, 0x7c, 0, 32);

/* dot3out_pause_frames_high
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3out_pause_frames_high, 0x80, 0, 32);

/* dot3out_pause_frames_low
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, dot3out_pause_frames_low, 0x84, 0, 32);

/* port_transmit_wait_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_transmit_wait_high, 0x8, 0, 32);

/* port_transmit_wait_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_transmit_wait_low, 0xc, 0, 32);

/* ecn_marked_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ecn_marked_high, 0x10, 0, 32);

/* ecn_marked_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ecn_marked_low, 0x14, 0, 32);

/* no_buffer_discard_mc_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, no_buffer_discard_mc_high, 0x18, 0, 32);

/* no_buffer_discard_mc_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, no_buffer_discard_mc_low, 0x1c, 0, 32);

/* rx_ebp_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_ebp_high, 0x20, 0, 32);

/* rx_ebp_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_ebp_low, 0x24, 0, 32);

/* tx_ebp_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_ebp_high, 0x28, 0, 32);

/* tx_ebp_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_ebp_low, 0x2c, 0, 32);

/* rx_buffer_almost_full_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_buffer_almost_full_high, 0x30, 0, 32);

/* rx_buffer_almost_full_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_buffer_almost_full_low, 0x34, 0, 32);

/* rx_buffer_full_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_buffer_full_high, 0x38, 0, 32);

/* rx_buffer_full_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_buffer_full_low, 0x3c, 0, 32);

/* rx_icrc_encapsulated_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_icrc_encapsulated_high, 0x40, 0, 32);

/* rx_icrc_encapsulated_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_icrc_encapsulated_low, 0x44, 0, 32);

/* tx_stats_pkts64octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts64octets_high, 0x58, 0, 32);

/* tx_stats_pkts64octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts64octets_low, 0x5c, 0, 32);

/* tx_stats_pkts65to127octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts65to127octets_high, 0x60, 0, 32);

/* tx_stats_pkts65to127octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts65to127octets_low, 0x64, 0, 32);

/* tx_stats_pkts128to255octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts128to255octets_high, 0x68, 0, 32);

/* tx_stats_pkts128to255octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts128to255octets_low, 0x6c, 0, 32);

/* tx_stats_pkts256to511octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts256to511octets_high, 0x70, 0, 32);

/* tx_stats_pkts256to511octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts256to511octets_low, 0x74, 0, 32);

/* tx_stats_pkts512to1023octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts512to1023octets_high, 0x78, 0, 32);

/* tx_stats_pkts512to1023octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts512to1023octets_low, 0x7c, 0, 32);

/* tx_stats_pkts1024to1518octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts1024to1518octets_high, 0x80, 0, 32);

/* tx_stats_pkts1024to1518octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts1024to1518octets_low, 0x84, 0, 32);

/* tx_stats_pkts1519to2047octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts1519to2047octets_high, 0x88, 0, 32);

/* tx_stats_pkts1519to2047octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts1519to2047octets_low, 0x8c, 0, 32);

/* tx_stats_pkts2048to4095octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts2048to4095octets_high, 0x90, 0, 32);

/* tx_stats_pkts2048to4095octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts2048to4095octets_low, 0x94, 0, 32);

/* tx_stats_pkts4096to8191octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts4096to8191octets_high, 0x98, 0, 32);

/* tx_stats_pkts4096to8191octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts4096to8191octets_low, 0x9c, 0, 32);

/* tx_stats_pkts8192to10239octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts8192to10239octets_high, 0xa0, 0, 32);

/* tx_stats_pkts8192to10239octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_stats_pkts8192to10239octets_low, 0xa4, 0, 32);

/* ece_marked_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ece_marked_high, 0xa8, 0, 32);

/* ece_marked_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ece_marked_low, 0xac, 0, 32);

/* tx_int_cksm_err_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_int_cksm_err_high, 0xb0, 0, 32);

/* tx_int_cksm_err_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_int_cksm_err_low, 0xb4, 0, 32);

/* ingress_general_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ingress_general_high, 0x8, 0, 32);

/* ingress_general_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ingress_general_low, 0xc, 0, 32);

/* ingress_policy_engine_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ingress_policy_engine_high, 0x10, 0, 32);

/* ingress_policy_engine_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ingress_policy_engine_low, 0x14, 0, 32);

/* ingress_vlan_membership_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ingress_vlan_membership_high, 0x18, 0, 32);

/* ingress_vlan_membership_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ingress_vlan_membership_low, 0x1c, 0, 32);

/* ingress_tag_frame_type_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ingress_tag_frame_type_high, 0x20, 0, 32);

/* ingress_tag_frame_type_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ingress_tag_frame_type_low, 0x24, 0, 32);

/* egress_vlan_membership_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, egress_vlan_membership_high, 0x28, 0, 32);

/* egress_vlan_membership_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, egress_vlan_membership_low, 0x2c, 0, 32);

/* loopback_filter_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, loopback_filter_high, 0x30, 0, 32);

/* loopback_filter_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, loopback_filter_low, 0x34, 0, 32);

/* egress_general_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, egress_general_high, 0x38, 0, 32);

/* egress_general_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, egress_general_low, 0x3c, 0, 32);

/* egress_link_down_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, egress_link_down_high, 0x40, 0, 32);

/* egress_link_down_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, egress_link_down_low, 0x44, 0, 32);

/* egress_hoq_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, egress_hoq_high, 0x48, 0, 32);

/* egress_hoq_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, egress_hoq_low, 0x4c, 0, 32);

/* port_isolation_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_isolation_high, 0x50, 0, 32);

/* port_isolation_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_isolation_low, 0x54, 0, 32);

/* egress_policy_engine_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, egress_policy_engine_high, 0x58, 0, 32);

/* egress_policy_engine_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, egress_policy_engine_low, 0x5c, 0, 32);

/* ingress_tx_link_down_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ingress_tx_link_down_high, 0x60, 0, 32);

/* ingress_tx_link_down_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ingress_tx_link_down_low, 0x64, 0, 32);

/* egress_stp_filter_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, egress_stp_filter_high, 0x68, 0, 32);

/* egress_stp_filter_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, egress_stp_filter_low, 0x6c, 0, 32);

/* egress_hoq_stall_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, egress_hoq_stall_high, 0x70, 0, 32);

/* egress_hoq_stall_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, egress_hoq_stall_low, 0x74, 0, 32);

/* egress_sll_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, egress_sll_high, 0x78, 0, 32);

/* egress_sll_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, egress_sll_low, 0x7c, 0, 32);

/* ingress_discard_all_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ingress_discard_all_high, 0x80, 0, 32);

/* ingress_discard_all_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ingress_discard_all_low, 0x84, 0, 32);

/* llr_rx_cells_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, llr_rx_cells_high, 0x8, 0, 32);

/* llr_rx_cells_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, llr_rx_cells_low, 0xc, 0, 32);

/* llr_rx_error_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, llr_rx_error_high, 0x10, 0, 32);

/* llr_rx_error_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, llr_rx_error_low, 0x14, 0, 32);

/* llr_rx_crc_error_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, llr_rx_crc_error_high, 0x18, 0, 32);

/* llr_rx_crc_error_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, llr_rx_crc_error_low, 0x1c, 0, 32);

/* llr_tx_cells_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, llr_tx_cells_high, 0x20, 0, 32);

/* llr_tx_cells_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, llr_tx_cells_low, 0x24, 0, 32);

/* llr_tx_ret_cells_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, llr_tx_ret_cells_high, 0x28, 0, 32);

/* llr_tx_ret_cells_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, llr_tx_ret_cells_low, 0x2c, 0, 32);

/* llr_tx_ret_events_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, llr_tx_ret_events_high, 0x30, 0, 32);

/* llr_tx_ret_events_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, llr_tx_ret_events_low, 0x34, 0, 32);

/* rx_octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_octets_high, 0x8, 0, 32);

/* rx_octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_octets_low, 0xc, 0, 32);

/* rx_uc_frames_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_uc_frames_high, 0x10, 0, 32);

/* rx_uc_frames_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_uc_frames_low, 0x14, 0, 32);

/* rx_mc_frames_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_mc_frames_high, 0x18, 0, 32);

/* rx_mc_frames_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_mc_frames_low, 0x1c, 0, 32);

/* rx_bc_frames_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_bc_frames_high, 0x20, 0, 32);

/* rx_bc_frames_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_bc_frames_low, 0x24, 0, 32);

/* rx_frames_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_frames_high, 0x28, 0, 32);

/* rx_frames_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_frames_low, 0x2c, 0, 32);

/* tx_octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_octets_high, 0x30, 0, 32);

/* tx_octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_octets_low, 0x34, 0, 32);

/* tx_uc_frames_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_uc_frames_high, 0x38, 0, 32);

/* tx_uc_frames_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_uc_frames_low, 0x3c, 0, 32);

/* tx_mc_frames_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_mc_frames_high, 0x40, 0, 32);

/* tx_mc_frames_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_mc_frames_low, 0x44, 0, 32);

/* tx_bc_frames_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_bc_frames_high, 0x48, 0, 32);

/* tx_bc_frames_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_bc_frames_low, 0x4c, 0, 32);

/* tx_frames_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_frames_high, 0x50, 0, 32);

/* tx_frames_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_frames_low, 0x54, 0, 32);

/* rx_pause_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_pause_high, 0x58, 0, 32);

/* rx_pause_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_pause_low, 0x5c, 0, 32);

/* rx_pause_duration_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_pause_duration_high, 0x60, 0, 32);

/* rx_pause_duration_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_pause_duration_low, 0x64, 0, 32);

/* tx_pause_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_pause_high, 0x68, 0, 32);

/* tx_pause_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_pause_low, 0x6c, 0, 32);

/* tx_pause_duration_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_pause_duration_high, 0x70, 0, 32);

/* tx_pause_duration_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, tx_pause_duration_low, 0x74, 0, 32);

/* rx_pause_transition_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_pause_transition_high, 0x78, 0, 32);

/* rx_pause_transition_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_pause_transition_low, 0x7c, 0, 32);

/* rx_discards_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_discards_high, 0x80, 0, 32);

/* rx_discards_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_discards_low, 0x84, 0, 32);

/* device_stall_minor_watermark_cnt_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, device_stall_minor_watermark_cnt_high, 0x88, 0, 32);

/* device_stall_minor_watermark_cnt_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, device_stall_minor_watermark_cnt_low, 0x8c, 0, 32);

/* device_stall_critical_watermark_cnt_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, device_stall_critical_watermark_cnt_high, 0x90, 0, 32);

/* device_stall_critical_watermark_cnt_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, device_stall_critical_watermark_cnt_low, 0x94, 0, 32);

/* transmit_queue_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, transmit_queue_high, 0x8, 0, 32);

/* transmit_queue_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, transmit_queue_low, 0xc, 0, 32);

/* no_buffer_discard_uc_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, no_buffer_discard_uc_high, 0x10, 0, 32);

/* no_buffer_discard_uc_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, no_buffer_discard_uc_low, 0x14, 0, 32);

/* phys_layer_cntrs_time_since_last_clear_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_time_since_last_clear_high, 0x8, 0, 32);

/* phys_layer_cntrs_time_since_last_clear_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_time_since_last_clear_low, 0xc, 0, 32);

/* phys_layer_cntrs_symbol_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_symbol_errors_high, 0x10, 0, 32);

/* phys_layer_cntrs_symbol_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_symbol_errors_low, 0x14, 0, 32);

/* phys_layer_cntrs_sync_headers_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_sync_headers_errors_high, 0x18, 0, 32);

/* phys_layer_cntrs_sync_headers_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_sync_headers_errors_low, 0x1c, 0, 32);

/* phys_layer_cntrs_edpl_bip_errors_lane0_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_edpl_bip_errors_lane0_high, 0x20, 0, 32);

/* phys_layer_cntrs_edpl_bip_errors_lane0_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_edpl_bip_errors_lane0_low, 0x24, 0, 32);

/* phys_layer_cntrs_edpl_bip_errors_lane1_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_edpl_bip_errors_lane1_high, 0x28, 0, 32);

/* phys_layer_cntrs_edpl_bip_errors_lane1_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_edpl_bip_errors_lane1_low, 0x2c, 0, 32);

/* phys_layer_cntrs_edpl_bip_errors_lane2_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_edpl_bip_errors_lane2_high, 0x30, 0, 32);

/* phys_layer_cntrs_edpl_bip_errors_lane2_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_edpl_bip_errors_lane2_low, 0x34, 0, 32);

/* phys_layer_cntrs_edpl_bip_errors_lane3_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_edpl_bip_errors_lane3_high, 0x38, 0, 32);

/* phys_layer_cntrs_edpl_bip_errors_lane3_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_edpl_bip_errors_lane3_low, 0x3c, 0, 32);

/* phys_layer_cntrs_fc_fec_corrected_blocks_lane0_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_fc_fec_corrected_blocks_lane0_high, 0x40, 0, 32);

/* phys_layer_cntrs_fc_fec_corrected_blocks_lane0_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_fc_fec_corrected_blocks_lane0_low, 0x44, 0, 32);

/* phys_layer_cntrs_fc_fec_corrected_blocks_lane1_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_fc_fec_corrected_blocks_lane1_high, 0x48, 0, 32);

/* phys_layer_cntrs_fc_fec_corrected_blocks_lane1_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_fc_fec_corrected_blocks_lane1_low, 0x4c, 0, 32);

/* phys_layer_cntrs_fc_fec_corrected_blocks_lane2_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_fc_fec_corrected_blocks_lane2_high, 0x50, 0, 32);

/* phys_layer_cntrs_fc_fec_corrected_blocks_lane2_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_fc_fec_corrected_blocks_lane2_low, 0x54, 0, 32);

/* phys_layer_cntrs_fc_fec_corrected_blocks_lane3_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_fc_fec_corrected_blocks_lane3_high, 0x58, 0, 32);

/* phys_layer_cntrs_fc_fec_corrected_blocks_lane3_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_fc_fec_corrected_blocks_lane3_low, 0x5c, 0, 32);

/* phys_layer_cntrs_fc_fec_uncorrectable_blocks_lane0_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_fc_fec_uncorrectable_blocks_lane0_high, 0x60, 0, 32);

/* phys_layer_cntrs_fc_fec_uncorrectable_blocks_lane0_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_fc_fec_uncorrectable_blocks_lane0_low, 0x64, 0, 32);

/* phys_layer_cntrs_fc_fec_uncorrectable_blocks_lane1_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_fc_fec_uncorrectable_blocks_lane1_high, 0x68, 0, 32);

/* phys_layer_cntrs_fc_fec_uncorrectable_blocks_lane1_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_fc_fec_uncorrectable_blocks_lane1_low, 0x6c, 0, 32);

/* phys_layer_cntrs_fc_fec_uncorrectable_blocks_lane2_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_fc_fec_uncorrectable_blocks_lane2_high, 0x70, 0, 32);

/* phys_layer_cntrs_fc_fec_uncorrectable_blocks_lane2_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_fc_fec_uncorrectable_blocks_lane2_low, 0x74, 0, 32);

/* phys_layer_cntrs_fc_fec_uncorrectable_blocks_lane3_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_fc_fec_uncorrectable_blocks_lane3_high, 0x78, 0, 32);

/* phys_layer_cntrs_fc_fec_uncorrectable_blocks_lane3_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_fc_fec_uncorrectable_blocks_lane3_low, 0x7c, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_blocks_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_blocks_high, 0x80, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_blocks_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_blocks_low, 0x84, 0, 32);

/* phys_layer_cntrs_rs_fec_uncorrectable_blocks_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_uncorrectable_blocks_high, 0x88, 0, 32);

/* phys_layer_cntrs_rs_fec_uncorrectable_blocks_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_uncorrectable_blocks_low, 0x8c, 0, 32);

/* phys_layer_cntrs_rs_fec_no_errors_blocks_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_no_errors_blocks_high, 0x90, 0, 32);

/* phys_layer_cntrs_rs_fec_no_errors_blocks_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_no_errors_blocks_low, 0x94, 0, 32);

/* phys_layer_cntrs_rs_fec_single_error_blocks_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_single_error_blocks_high, 0x98, 0, 32);

/* phys_layer_cntrs_rs_fec_single_error_blocks_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_single_error_blocks_low, 0x9c, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_symbols_total_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_symbols_total_high, 0xa0, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_symbols_total_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_symbols_total_low, 0xa4, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_symbols_lane0_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_symbols_lane0_high, 0xa8, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_symbols_lane0_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_symbols_lane0_low, 0xac, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_symbols_lane1_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_symbols_lane1_high, 0xb0, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_symbols_lane1_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_symbols_lane1_low, 0xb4, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_symbols_lane2_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_symbols_lane2_high, 0xb8, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_symbols_lane2_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_symbols_lane2_low, 0xbc, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_symbols_lane3_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_symbols_lane3_high, 0xc0, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_symbols_lane3_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_symbols_lane3_low, 0xc4, 0, 32);

/* phys_layer_cntrs_link_down_events
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_link_down_events, 0xc8, 0, 32);

/* phys_layer_cntrs_successful_recovery_events
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_successful_recovery_events, 0xcc, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_symbols_lane4_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_symbols_lane4_high, 0xd0, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_symbols_lane4_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_symbols_lane4_low, 0xd4, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_symbols_lane5_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_symbols_lane5_high, 0xd8, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_symbols_lane5_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_symbols_lane5_low, 0xdc, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_symbols_lane6_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_symbols_lane6_high, 0xe0, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_symbols_lane6_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_symbols_lane6_low, 0xe4, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_symbols_lane7_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_symbols_lane7_high, 0xe8, 0, 32);

/* phys_layer_cntrs_rs_fec_corrected_symbols_lane7_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_corrected_symbols_lane7_low, 0xec, 0, 32);

/* phys_layer_cntrs_rs_fec_total_blocks_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_total_blocks_high, 0xf0, 0, 32);

/* phys_layer_cntrs_rs_fec_total_blocks_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_rs_fec_total_blocks_low, 0xf4, 0, 32);

/* phys_layer_cntrs_unintentional_link_down_events
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_unintentional_link_down_events, 0xf8, 0, 32);

/* phys_layer_cntrs_intentional_link_down_events
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_cntrs_intentional_link_down_events, 0xfc, 0, 32);

/* wred_discard_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, wred_discard_high, 0x8, 0, 32);

/* wred_discard_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, wred_discard_low, 0xc, 0, 32);

/* ecn_marked_tc_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ecn_marked_tc_high, 0x10, 0, 32);

/* ecn_marked_tc_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ecn_marked_tc_low, 0x14, 0, 32);

/* rx_no_buffer_discard_uc_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_no_buffer_discard_uc_high, 0x8, 0, 32);

/* rx_no_buffer_discard_uc_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rx_no_buffer_discard_uc_low, 0xc, 0, 32);

/* phys_layer_stat_cntrs_time_since_last_clear_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_time_since_last_clear_high, 0x8, 0, 32);

/* phys_layer_stat_cntrs_time_since_last_clear_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_time_since_last_clear_low, 0xc, 0, 32);

/* phys_layer_stat_cntrs_phy_received_bits_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_received_bits_high, 0x10, 0, 32);

/* phys_layer_stat_cntrs_phy_received_bits_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_received_bits_low, 0x14, 0, 32);

/* phys_layer_stat_cntrs_phy_symbol_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_symbol_errors_high, 0x18, 0, 32);

/* phys_layer_stat_cntrs_phy_symbol_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_symbol_errors_low, 0x1c, 0, 32);

/* phys_layer_stat_cntrs_phy_corrected_bits_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_corrected_bits_high, 0x20, 0, 32);

/* phys_layer_stat_cntrs_phy_corrected_bits_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_corrected_bits_low, 0x24, 0, 32);

/* phys_layer_stat_cntrs_phy_raw_errors_lane0_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_raw_errors_lane0_high, 0x28, 0, 32);

/* phys_layer_stat_cntrs_phy_raw_errors_lane0_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_raw_errors_lane0_low, 0x2c, 0, 32);

/* phys_layer_stat_cntrs_phy_raw_errors_lane1_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_raw_errors_lane1_high, 0x30, 0, 32);

/* phys_layer_stat_cntrs_phy_raw_errors_lane1_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_raw_errors_lane1_low, 0x34, 0, 32);

/* phys_layer_stat_cntrs_phy_raw_errors_lane2_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_raw_errors_lane2_high, 0x38, 0, 32);

/* phys_layer_stat_cntrs_phy_raw_errors_lane2_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_raw_errors_lane2_low, 0x3c, 0, 32);

/* phys_layer_stat_cntrs_phy_raw_errors_lane3_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_raw_errors_lane3_high, 0x40, 0, 32);

/* phys_layer_stat_cntrs_phy_raw_errors_lane3_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_raw_errors_lane3_low, 0x44, 0, 32);

/* phys_layer_stat_cntrs_phy_raw_errors_lane4_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_raw_errors_lane4_high, 0x48, 0, 32);

/* phys_layer_stat_cntrs_phy_raw_errors_lane4_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_raw_errors_lane4_low, 0x4c, 0, 32);

/* phys_layer_stat_cntrs_phy_raw_errors_lane5_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_raw_errors_lane5_high, 0x50, 0, 32);

/* phys_layer_stat_cntrs_phy_raw_errors_lane5_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_raw_errors_lane5_low, 0x54, 0, 32);

/* phys_layer_stat_cntrs_phy_raw_errors_lane6_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_raw_errors_lane6_high, 0x58, 0, 32);

/* phys_layer_stat_cntrs_phy_raw_errors_lane6_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_raw_errors_lane6_low, 0x5c, 0, 32);

/* phys_layer_stat_cntrs_phy_raw_errors_lane7_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_raw_errors_lane7_high, 0x60, 0, 32);

/* phys_layer_stat_cntrs_phy_raw_errors_lane7_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_raw_errors_lane7_low, 0x64, 0, 32);

/* phys_layer_stat_cntrs_fc_zero_hist
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, phys_layer_stat_cntrs_fc_zero_hist, 0x68, 0, 10);

/* phys_layer_stat_cntrs_raw_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_magnitude, 0x6a, 0, 8);

/* phys_layer_stat_cntrs_raw_ber_coef_float
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_coef_float, 0x6b, 4, 4);

/* phys_layer_stat_cntrs_raw_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_coef, 0x6b, 0, 4);

/* phys_layer_stat_cntrs_estimated_effective_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_estimated_effective_ber_magnitude, 0x6c, 0, 8);

/* phys_layer_stat_cntrs_estimated_effective_ber_coef_float
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_estimated_effective_ber_coef_float, 0x6d, 4, 4);

/* phys_layer_stat_cntrs_estimated_effective_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_estimated_effective_ber_coef, 0x6d, 0, 4);

/* phys_layer_stat_cntrs_effective_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_effective_ber_magnitude, 0x6e, 0, 8);

/* phys_layer_stat_cntrs_effective_ber_coef_float
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_effective_ber_coef_float, 0x6f, 4, 4);

/* phys_layer_stat_cntrs_effective_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_effective_ber_coef, 0x6f, 0, 4);

/* phys_layer_stat_cntrs_ucr_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_ucr_ber_magnitude, 0x70, 0, 8);

/* phys_layer_stat_cntrs_ucr_ber_coef_float
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_ucr_ber_coef_float, 0x71, 4, 4);

/* phys_layer_stat_cntrs_ucr_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_ucr_ber_coef, 0x71, 0, 4);

/* phys_layer_stat_cntrs_symbol_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_symbol_ber_magnitude, 0x72, 0, 8);

/* phys_layer_stat_cntrs_symbol_ber_coef_float
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_symbol_ber_coef_float, 0x73, 4, 4);

/* phys_layer_stat_cntrs_symbol_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_symbol_ber_coef, 0x73, 0, 4);

/* phys_layer_stat_cntrs_raw_ber_magnitude_lane0
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_magnitude_lane0, 0x74, 0, 8);

/* phys_layer_stat_cntrs_raw_ber_coef_float_lane0
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_coef_float_lane0, 0x75, 4, 4);

/* phys_layer_stat_cntrs_raw_ber_coef_lane0
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_coef_lane0, 0x75, 0, 4);

/* phys_layer_stat_cntrs_raw_ber_magnitude_lane1
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_magnitude_lane1, 0x76, 0, 8);

/* phys_layer_stat_cntrs_raw_ber_coef_float_lane1
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_coef_float_lane1, 0x77, 4, 4);

/* phys_layer_stat_cntrs_raw_ber_coef_lane1
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_coef_lane1, 0x77, 0, 4);

/* phys_layer_stat_cntrs_raw_ber_magnitude_lane2
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_magnitude_lane2, 0x78, 0, 8);

/* phys_layer_stat_cntrs_raw_ber_coef_float_lane2
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_coef_float_lane2, 0x79, 4, 4);

/* phys_layer_stat_cntrs_raw_ber_coef_lane2
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_coef_lane2, 0x79, 0, 4);

/* phys_layer_stat_cntrs_raw_ber_magnitude_lane3
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_magnitude_lane3, 0x7a, 0, 8);

/* phys_layer_stat_cntrs_raw_ber_coef_float_lane3
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_coef_float_lane3, 0x7b, 4, 4);

/* phys_layer_stat_cntrs_raw_ber_coef_lane3
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_coef_lane3, 0x7b, 0, 4);

/* phys_layer_stat_cntrs_raw_ber_magnitude_lane4
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_magnitude_lane4, 0x7c, 0, 8);

/* phys_layer_stat_cntrs_raw_ber_coef_float_lane4
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_coef_float_lane4, 0x7d, 4, 4);

/* phys_layer_stat_cntrs_raw_ber_coef_lane4
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_coef_lane4, 0x7d, 0, 4);

/* phys_layer_stat_cntrs_raw_ber_magnitude_lane5
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_magnitude_lane5, 0x7e, 0, 8);

/* phys_layer_stat_cntrs_raw_ber_coef_float_lane5
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_coef_float_lane5, 0x7f, 4, 4);

/* phys_layer_stat_cntrs_raw_ber_coef_lane5
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_coef_lane5, 0x7f, 0, 4);

/* phys_layer_stat_cntrs_raw_ber_magnitude_lane6
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_magnitude_lane6, 0x80, 0, 8);

/* phys_layer_stat_cntrs_raw_ber_coef_float_lane6
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_coef_float_lane6, 0x81, 4, 4);

/* phys_layer_stat_cntrs_raw_ber_coef_lane6
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_coef_lane6, 0x81, 0, 4);

/* phys_layer_stat_cntrs_raw_ber_magnitude_lane7
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_magnitude_lane7, 0x82, 0, 8);

/* phys_layer_stat_cntrs_raw_ber_coef_float_lane7
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_coef_float_lane7, 0x83, 4, 4);

/* phys_layer_stat_cntrs_raw_ber_coef_lane7
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, phys_layer_stat_cntrs_raw_ber_coef_lane7, 0x83, 0, 4);

/* phys_layer_stat_cntrs_phy_effective_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_effective_errors_high, 0x84, 0, 32);

/* phys_layer_stat_cntrs_phy_effective_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_phy_effective_errors_low, 0x88, 0, 32);

/* phys_layer_stat_cntrs_serdes_received_bits_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_serdes_received_bits_high, 0x8c, 0, 32);

/* phys_layer_stat_cntrs_serdes_received_bits_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_serdes_received_bits_low, 0x90, 0, 32);

/* phys_layer_stat_cntrs_data_received_bits_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_data_received_bits_high, 0x94, 0, 32);

/* phys_layer_stat_cntrs_data_received_bits_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, phys_layer_stat_cntrs_data_received_bits_low, 0x98, 0, 32);

/* eth_oob_port_cntrs_if_in_ucast_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_if_in_ucast_pkts_high, 0x8, 0, 32);

/* eth_oob_port_cntrs_if_in_ucast_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_if_in_ucast_pkts_low, 0xc, 0, 32);

/* eth_oob_port_cntrs_if_in_octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_if_in_octets_high, 0x10, 0, 32);

/* eth_oob_port_cntrs_if_in_octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_if_in_octets_low, 0x14, 0, 32);

/* eth_oob_port_cntrs_if_in_discards_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_if_in_discards_high, 0x18, 0, 32);

/* eth_oob_port_cntrs_if_in_discards_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_if_in_discards_low, 0x1c, 0, 32);

/* eth_oob_port_cntrs_if_in_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_if_in_errors_high, 0x20, 0, 32);

/* eth_oob_port_cntrs_if_in_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_if_in_errors_low, 0x24, 0, 32);

/* eth_oob_port_cntrs_ether_stats_crc_align_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_ether_stats_crc_align_errors_high, 0x28, 0, 32);

/* eth_oob_port_cntrs_ether_stats_crc_align_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_ether_stats_crc_align_errors_low, 0x2c, 0, 32);

/* eth_oob_port_cntrs_ether_stats_undersize_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_ether_stats_undersize_pkts_high, 0x30, 0, 32);

/* eth_oob_port_cntrs_ether_stats_undersize_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_ether_stats_undersize_pkts_low, 0x34, 0, 32);

/* eth_oob_port_cntrs_ether_stats_oversize_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_ether_stats_oversize_pkts_high, 0x38, 0, 32);

/* eth_oob_port_cntrs_ether_stats_oversize_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_ether_stats_oversize_pkts_low, 0x3c, 0, 32);

/* eth_oob_port_cntrs_dot3stats_symbol_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_dot3stats_symbol_errors_high, 0x40, 0, 32);

/* eth_oob_port_cntrs_dot3stats_symbol_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_dot3stats_symbol_errors_low, 0x44, 0, 32);

/* eth_oob_port_cntrs_dot3in_pause_frames_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_dot3in_pause_frames_high, 0x48, 0, 32);

/* eth_oob_port_cntrs_dot3in_pause_frames_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_dot3in_pause_frames_low, 0x4c, 0, 32);

/* eth_oob_port_cntrs_dot3control_in_unknown_opcodes_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_dot3control_in_unknown_opcodes_high, 0x50, 0, 32);

/* eth_oob_port_cntrs_dot3control_in_unknown_opcodes_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_dot3control_in_unknown_opcodes_low, 0x54, 0, 32);

/* eth_oob_port_cntrs_if_out_ucast_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_if_out_ucast_pkts_high, 0x58, 0, 32);

/* eth_oob_port_cntrs_if_out_ucast_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_if_out_ucast_pkts_low, 0x5c, 0, 32);

/* eth_oob_port_cntrs_if_out_octets_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_if_out_octets_high, 0x60, 0, 32);

/* eth_oob_port_cntrs_if_out_octets_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_if_out_octets_low, 0x64, 0, 32);

/* eth_oob_port_cntrs_dot3out_pause_frames_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_dot3out_pause_frames_high, 0x68, 0, 32);

/* eth_oob_port_cntrs_dot3out_pause_frames_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_dot3out_pause_frames_low, 0x6c, 0, 32);

/* eth_oob_port_cntrs_if_out_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_if_out_errors_high, 0x70, 0, 32);

/* eth_oob_port_cntrs_if_out_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_if_out_errors_low, 0x74, 0, 32);

/* eth_oob_port_cntrs_phy_time_since_last_clear_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_phy_time_since_last_clear_high, 0x78, 0, 32);

/* eth_oob_port_cntrs_phy_time_since_last_clear_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_phy_time_since_last_clear_low, 0x7c, 0, 32);

/* eth_oob_port_cntrs_phy_received_bits_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_phy_received_bits_high, 0x80, 0, 32);

/* eth_oob_port_cntrs_phy_received_bits_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_phy_received_bits_low, 0x84, 0, 32);

/* eth_oob_port_cntrs_phy_symbol_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_phy_symbol_errors_high, 0x88, 0, 32);

/* eth_oob_port_cntrs_phy_symbol_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, eth_oob_port_cntrs_phy_symbol_errors_low, 0x8c, 0, 32);

/* symbol_error_counter
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, symbol_error_counter, 0x8, 0, 16);

/* link_error_recovery_counter
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, link_error_recovery_counter, 0xa, 0, 8);

/* link_downed_counter
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, link_downed_counter, 0xb, 0, 8);

/* port_rcv_errors
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, port_rcv_errors, 0xc, 0, 16);

/* port_rcv_remote_physical_errors
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, port_rcv_remote_physical_errors, 0xe, 0, 16);

/* port_rcv_switch_relay_errors
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, port_rcv_switch_relay_errors, 0x10, 0, 16);

/* port_xmit_discards
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, port_xmit_discards, 0x12, 0, 16);

/* port_xmit_constraint_errors
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, port_xmit_constraint_errors, 0x14, 0, 8);

/* port_rcv_constraint_errors
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, port_rcv_constraint_errors, 0x15, 0, 8);

/* local_link_integrity_errors
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, local_link_integrity_errors, 0x17, 4, 4);

/* excessive_buffer_overrun_errors
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, excessive_buffer_overrun_errors, 0x17, 0, 4);

/* qp1_dropped
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, qp1_dropped, 0x18, 0, 16);

/* vl_15_dropped
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, vl_15_dropped, 0x1a, 0, 16);

/* port_xmit_data
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_data, 0x1c, 0, 32);

/* port_rcv_data
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_data, 0x20, 0, 32);

/* port_xmit_pkts
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_pkts, 0x24, 0, 32);

/* port_rcv_pkts
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_pkts, 0x28, 0, 32);

/* port_xmit_wait
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_wait, 0x2c, 0, 32);

/* symbol_error_counter_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, symbol_error_counter_high, 0x8, 0, 32);

/* symbol_error_counter_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, symbol_error_counter_low, 0xc, 0, 32);

/* link_error_recovery_counter_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, link_error_recovery_counter_high, 0x10, 0, 32);

/* link_error_recovery_counter_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, link_error_recovery_counter_low, 0x14, 0, 32);

/* link_downed_counter_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, link_downed_counter_high, 0x18, 0, 32);

/* link_downed_counter_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, link_downed_counter_low, 0x1c, 0, 32);

/* port_rcv_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_errors_high, 0x20, 0, 32);

/* port_rcv_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_errors_low, 0x24, 0, 32);

/* port_rcv_remote_physical_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_remote_physical_errors_high, 0x28, 0, 32);

/* port_rcv_remote_physical_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_remote_physical_errors_low, 0x2c, 0, 32);

/* port_rcv_switch_relay_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_switch_relay_errors_high, 0x30, 0, 32);

/* port_rcv_switch_relay_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_switch_relay_errors_low, 0x34, 0, 32);

/* port_xmit_discards_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_discards_high, 0x38, 0, 32);

/* port_xmit_discards_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_discards_low, 0x3c, 0, 32);

/* port_xmit_constraint_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_constraint_errors_high, 0x40, 0, 32);

/* port_xmit_constraint_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_constraint_errors_low, 0x44, 0, 32);

/* port_rcv_constraint_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_constraint_errors_high, 0x48, 0, 32);

/* port_rcv_constraint_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_constraint_errors_low, 0x4c, 0, 32);

/* local_link_integrity_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, local_link_integrity_errors_high, 0x50, 0, 32);

/* local_link_integrity_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, local_link_integrity_errors_low, 0x54, 0, 32);

/* excessive_buffer_overrun_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, excessive_buffer_overrun_errors_high, 0x58, 0, 32);

/* excessive_buffer_overrun_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, excessive_buffer_overrun_errors_low, 0x5c, 0, 32);

/* vl_15_dropped_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, vl_15_dropped_high, 0x60, 0, 32);

/* vl_15_dropped_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, vl_15_dropped_low, 0x64, 0, 32);

/* port_xmit_data_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_data_high, 0x68, 0, 32);

/* port_xmit_data_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_data_low, 0x6c, 0, 32);

/* port_rcv_data_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_data_high, 0x70, 0, 32);

/* port_rcv_data_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_data_low, 0x74, 0, 32);

/* port_xmit_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_pkts_high, 0x78, 0, 32);

/* port_xmit_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_pkts_low, 0x7c, 0, 32);

/* port_rcv_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_pkts_high, 0x80, 0, 32);

/* port_rcv_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_pkts_low, 0x84, 0, 32);

/* port_xmit_wait_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_wait_high, 0x88, 0, 32);

/* port_xmit_wait_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_wait_low, 0x8c, 0, 32);

/* qp1_dropped_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, qp1_dropped_high, 0x90, 0, 32);

/* qp1_dropped_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, qp1_dropped_low, 0x94, 0, 32);

/* port_unicast_xmit_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_unicast_xmit_pkts_high, 0x98, 0, 32);

/* port_unicast_xmit_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_unicast_xmit_pkts_low, 0x9c, 0, 32);

/* port_multicast_xmit_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_multicast_xmit_pkts_high, 0xa0, 0, 32);

/* port_multicast_xmit_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_multicast_xmit_pkts_low, 0xa4, 0, 32);

/* port_unicast_rcv_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_unicast_rcv_pkts_high, 0xa8, 0, 32);

/* port_unicast_rcv_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_unicast_rcv_pkts_low, 0xac, 0, 32);

/* port_multicast_rcv_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_multicast_rcv_pkts_high, 0xb0, 0, 32);

/* port_multicast_rcv_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_multicast_rcv_pkts_low, 0xb4, 0, 32);

/* ppcnt_plr_counters_plr_rcv_codes_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_plr_rcv_codes_high, 0x8, 0, 32);

/* ppcnt_plr_counters_plr_rcv_codes_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_plr_rcv_codes_low, 0xc, 0, 32);

/* ppcnt_plr_counters_plr_rcv_code_err_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_plr_rcv_code_err_high, 0x10, 0, 32);

/* ppcnt_plr_counters_plr_rcv_code_err_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_plr_rcv_code_err_low, 0x14, 0, 32);

/* ppcnt_plr_counters_plr_rcv_uncorrectable_code_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_plr_rcv_uncorrectable_code_high, 0x18, 0, 32);

/* ppcnt_plr_counters_plr_rcv_uncorrectable_code_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_plr_rcv_uncorrectable_code_low, 0x1c, 0, 32);

/* ppcnt_plr_counters_plr_xmit_codes_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_plr_xmit_codes_high, 0x20, 0, 32);

/* ppcnt_plr_counters_plr_xmit_codes_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_plr_xmit_codes_low, 0x24, 0, 32);

/* ppcnt_plr_counters_plr_xmit_retry_codes_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_plr_xmit_retry_codes_high, 0x28, 0, 32);

/* ppcnt_plr_counters_plr_xmit_retry_codes_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_plr_xmit_retry_codes_low, 0x2c, 0, 32);

/* ppcnt_plr_counters_plr_xmit_retry_events_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_plr_xmit_retry_events_high, 0x30, 0, 32);

/* ppcnt_plr_counters_plr_xmit_retry_events_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_plr_xmit_retry_events_low, 0x34, 0, 32);

/* ppcnt_plr_counters_plr_sync_events_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_plr_sync_events_high, 0x38, 0, 32);

/* ppcnt_plr_counters_plr_sync_events_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_plr_sync_events_low, 0x3c, 0, 32);

/* ppcnt_plr_counters_plr_codes_loss_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_plr_codes_loss_high, 0x40, 0, 32);

/* ppcnt_plr_counters_plr_codes_loss_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_plr_codes_loss_low, 0x44, 0, 32);

/* ppcnt_plr_counters_plr_xmit_retry_events_within_t_sec_max_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_plr_xmit_retry_events_within_t_sec_max_high, 0x48, 0, 32);

/* ppcnt_plr_counters_plr_xmit_retry_events_within_t_sec_max_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_plr_xmit_retry_events_within_t_sec_max_low, 0x4c, 0, 32);

/* ppcnt_plr_counters_time_since_last_clear_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_time_since_last_clear_high, 0x50, 0, 32);

/* ppcnt_plr_counters_time_since_last_clear_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_plr_counters_time_since_last_clear_low, 0x54, 0, 32);

/* ppcnt_plr_counters_plr_outstanding_cells
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, ppcnt_plr_counters_plr_outstanding_cells, 0x58, 0, 8);

/* hist
 * Access: RO
 */
MLXSW_ITEM64_INDEXED(reg, ppcnt, hist, 0x8, 0, 64, 8, 0, 8, 0);

/* hist_diff
 * Access: RO
 */
MLXSW_ITEM64_INDEXED(reg, ppcnt, hist_diff, 0x8, 0, 64, 8, 0, 8, 0);

/* usr_xsr_physical_layer_time_since_last_clear_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_time_since_last_clear_high, 0x8, 0, 32);

/* usr_xsr_physical_layer_time_since_last_clear_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_time_since_last_clear_low, 0xc, 0, 32);

/* usr_xsr_physical_layer_fc_fec_corrected_blocks_lane0_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_corrected_blocks_lane0_high, 0x10, 0, 32);

/* usr_xsr_physical_layer_fc_fec_corrected_blocks_lane0_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_corrected_blocks_lane0_low, 0x14, 0, 32);

/* usr_xsr_physical_layer_fc_fec_corrected_blocks_lane1_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_corrected_blocks_lane1_high, 0x18, 0, 32);

/* usr_xsr_physical_layer_fc_fec_corrected_blocks_lane1_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_corrected_blocks_lane1_low, 0x1c, 0, 32);

/* usr_xsr_physical_layer_fc_fec_corrected_blocks_lane2_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_corrected_blocks_lane2_high, 0x20, 0, 32);

/* usr_xsr_physical_layer_fc_fec_corrected_blocks_lane2_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_corrected_blocks_lane2_low, 0x24, 0, 32);

/* usr_xsr_physical_layer_fc_fec_corrected_blocks_lane3_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_corrected_blocks_lane3_high, 0x28, 0, 32);

/* usr_xsr_physical_layer_fc_fec_corrected_blocks_lane3_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_corrected_blocks_lane3_low, 0x2c, 0, 32);

/* usr_xsr_physical_layer_fc_fec_corrected_blocks_lane4_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_corrected_blocks_lane4_high, 0x30, 0, 32);

/* usr_xsr_physical_layer_fc_fec_corrected_blocks_lane4_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_corrected_blocks_lane4_low, 0x34, 0, 32);

/* usr_xsr_physical_layer_fc_fec_corrected_blocks_lane5_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_corrected_blocks_lane5_high, 0x38, 0, 32);

/* usr_xsr_physical_layer_fc_fec_corrected_blocks_lane5_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_corrected_blocks_lane5_low, 0x3c, 0, 32);

/* usr_xsr_physical_layer_fc_fec_corrected_blocks_lane6_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_corrected_blocks_lane6_high, 0x40, 0, 32);

/* usr_xsr_physical_layer_fc_fec_corrected_blocks_lane6_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_corrected_blocks_lane6_low, 0x44, 0, 32);

/* usr_xsr_physical_layer_fc_fec_corrected_blocks_lane7_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_corrected_blocks_lane7_high, 0x48, 0, 32);

/* usr_xsr_physical_layer_fc_fec_corrected_blocks_lane7_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_corrected_blocks_lane7_low, 0x4c, 0, 32);

/* usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane0_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane0_high, 0x50, 0, 32);

/* usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane0_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane0_low, 0x54, 0, 32);

/* usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane1_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane1_high, 0x58, 0, 32);

/* usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane1_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane1_low, 0x5c, 0, 32);

/* usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane2_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane2_high, 0x60, 0, 32);

/* usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane2_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane2_low, 0x64, 0, 32);

/* usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane3_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane3_high, 0x68, 0, 32);

/* usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane3_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane3_low, 0x6c, 0, 32);

/* usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane4_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane4_high, 0x70, 0, 32);

/* usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane4_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane4_low, 0x74, 0, 32);

/* usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane5_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane5_high, 0x78, 0, 32);

/* usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane5_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane5_low, 0x7c, 0, 32);

/* usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane6_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane6_high, 0x80, 0, 32);

/* usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane6_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane6_low, 0x84, 0, 32);

/* usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane7_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane7_high, 0x88, 0, 32);

/* usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane7_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_fc_fec_uncorrectable_blocks_lane7_low, 0x8c, 0, 32);

/* usr_xsr_physical_layer_link_down_events
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, usr_xsr_physical_layer_link_down_events, 0x90, 0, 32);

/* ppcnt_infiniband_packets_counter_time_since_last_clear_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_time_since_last_clear_high, 0x8, 0, 32);

/* ppcnt_infiniband_packets_counter_time_since_last_clear_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_time_since_last_clear_low, 0xc, 0, 32);

/* ppcnt_infiniband_packets_counter_port_unicast_xmit_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_unicast_xmit_pkts_high, 0x10, 0, 32);

/* ppcnt_infiniband_packets_counter_port_unicast_xmit_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_unicast_xmit_pkts_low, 0x14, 0, 32);

/* ppcnt_infiniband_packets_counter_port_unicast_rcv_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_unicast_rcv_pkts_high, 0x18, 0, 32);

/* ppcnt_infiniband_packets_counter_port_unicast_rcv_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_unicast_rcv_pkts_low, 0x1c, 0, 32);

/* ppcnt_infiniband_packets_counter_sync_header_error_counter_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_sync_header_error_counter_high, 0x20, 0, 32);

/* ppcnt_infiniband_packets_counter_sync_header_error_counter_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_sync_header_error_counter_low, 0x24, 0, 32);

/* ppcnt_infiniband_packets_counter_port_local_physical_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_local_physical_errors_high, 0x28, 0, 32);

/* ppcnt_infiniband_packets_counter_port_local_physical_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_local_physical_errors_low, 0x2c, 0, 32);

/* ppcnt_infiniband_packets_counter_port_malformed_packet_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_malformed_packet_errors_high, 0x30, 0, 32);

/* ppcnt_infiniband_packets_counter_port_malformed_packet_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_malformed_packet_errors_low, 0x34, 0, 32);

/* ppcnt_infiniband_packets_counter_port_buffer_overrun_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_buffer_overrun_errors_high, 0x38, 0, 32);

/* ppcnt_infiniband_packets_counter_port_buffer_overrun_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_buffer_overrun_errors_low, 0x3c, 0, 32);

/* ppcnt_infiniband_packets_counter_port_dlid_mapping_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_dlid_mapping_errors_high, 0x40, 0, 32);

/* ppcnt_infiniband_packets_counter_port_dlid_mapping_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_dlid_mapping_errors_low, 0x44, 0, 32);

/* ppcnt_infiniband_packets_counter_port_vl_mapping_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_vl_mapping_errors_high, 0x48, 0, 32);

/* ppcnt_infiniband_packets_counter_port_vl_mapping_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_vl_mapping_errors_low, 0x4c, 0, 32);

/* ppcnt_infiniband_packets_counter_port_looping_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_looping_errors_high, 0x50, 0, 32);

/* ppcnt_infiniband_packets_counter_port_looping_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_looping_errors_low, 0x54, 0, 32);

/* ppcnt_infiniband_packets_counter_port_inactive_discards_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_inactive_discards_high, 0x58, 0, 32);

/* ppcnt_infiniband_packets_counter_port_inactive_discards_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_inactive_discards_low, 0x5c, 0, 32);

/* ppcnt_infiniband_packets_counter_port_neighbor_mtu_discards_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_neighbor_mtu_discards_high, 0x60, 0, 32);

/* ppcnt_infiniband_packets_counter_port_neighbor_mtu_discards_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_neighbor_mtu_discards_low, 0x64, 0, 32);

/* ppcnt_infiniband_packets_counter_port_llu_rx_icrc_error_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_llu_rx_icrc_error_high, 0x68, 0, 32);

/* ppcnt_infiniband_packets_counter_port_llu_rx_icrc_error_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_llu_rx_icrc_error_low, 0x6c, 0, 32);

/* ppcnt_infiniband_packets_counter_port_tx_parity_error_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_tx_parity_error_high, 0x70, 0, 32);

/* ppcnt_infiniband_packets_counter_port_tx_parity_error_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packets_counter_port_tx_parity_error_low, 0x74, 0, 32);

/* port_rcv_data_qword_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_data_qword_high, 0x8, 0, 32);

/* port_rcv_data_qword_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_data_qword_low, 0xc, 0, 32);

/* port_xmit_data_qword_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_data_qword_high, 0x10, 0, 32);

/* port_xmit_data_qword_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_data_qword_low, 0x14, 0, 32);

/* rq_general_error_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rq_general_error_high, 0x18, 0, 32);

/* rq_general_error_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, rq_general_error_low, 0x1c, 0, 32);

/* port_xmit_ebp_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_ebp_high, 0x20, 0, 32);

/* port_xmit_ebp_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_ebp_low, 0x24, 0, 32);

/* port_sw_hoq_lifetime_limit_discards_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_sw_hoq_lifetime_limit_discards_high, 0x28, 0, 32);

/* port_sw_hoq_lifetime_limit_discards_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_sw_hoq_lifetime_limit_discards_low, 0x2c, 0, 32);

/* dqs2llu_xmit_wait_arb_global_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, dqs2llu_xmit_wait_arb_global_high, 0x30, 0, 32);

/* dqs2llu_xmit_wait_arb_global_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, dqs2llu_xmit_wait_arb_global_low, 0x34, 0, 32);

/* grxb_fecn_mark_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, grxb_fecn_mark_high, 0x38, 0, 32);

/* grxb_fecn_mark_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, grxb_fecn_mark_low, 0x3c, 0, 32);

/* general_transmit_discard_external_contain_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, general_transmit_discard_external_contain_high, 0x40, 0, 32);

/* general_transmit_discard_external_contain_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, general_transmit_discard_external_contain_low, 0x44, 0, 32);

/* general_receive_discard_external_contain_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, general_receive_discard_external_contain_high, 0x48, 0, 32);

/* general_receive_discard_external_contain_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, general_receive_discard_external_contain_low, 0x4c, 0, 32);

/* port_receive_poison_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_receive_poison_high, 0x50, 0, 32);

/* port_receive_poison_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_receive_poison_low, 0x54, 0, 32);

/* l0_tx_idle_active_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, l0_tx_idle_active_high, 0x8, 0, 32);

/* l0_tx_idle_active_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, l0_tx_idle_active_low, 0xc, 0, 32);

/* l0_tx_idle_active_due_to_l1_req_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, l0_tx_idle_active_due_to_l1_req_high, 0x10, 0, 32);

/* l0_tx_idle_active_due_to_l1_req_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, l0_tx_idle_active_due_to_l1_req_low, 0x14, 0, 32);

/* l0_tx_idle_req_not_yet_active_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, l0_tx_idle_req_not_yet_active_high, 0x18, 0, 32);

/* l0_tx_idle_req_not_yet_active_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, l0_tx_idle_req_not_yet_active_low, 0x1c, 0, 32);

/* l0_tx_idle_entry_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, l0_tx_idle_entry_high, 0x20, 0, 32);

/* l0_tx_idle_entry_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, l0_tx_idle_entry_low, 0x24, 0, 32);

/* l0_tx_idle_entry_due_to_l1_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, l0_tx_idle_entry_due_to_l1_high, 0x28, 0, 32);

/* l0_tx_idle_entry_due_to_l1_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, l0_tx_idle_entry_due_to_l1_low, 0x2c, 0, 32);

/* l0_tx_idle_entry_due_to_l0_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, l0_tx_idle_entry_due_to_l0_high, 0x30, 0, 32);

/* l0_tx_idle_entry_due_to_l0_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, l0_tx_idle_entry_due_to_l0_low, 0x34, 0, 32);

/* l0_rx_idle_blocks_cnt_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, l0_rx_idle_blocks_cnt_high, 0x38, 0, 32);

/* l0_rx_idle_blocks_cnt_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, l0_rx_idle_blocks_cnt_low, 0x3c, 0, 32);

/* l0_rx_idle_entry_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, l0_rx_idle_entry_high, 0x40, 0, 32);

/* l0_rx_idle_entry_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, l0_rx_idle_entry_low, 0x44, 0, 32);

/* avg_tx_idle_active_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, avg_tx_idle_active_high, 0x48, 0, 32);

/* avg_tx_idle_active_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, avg_tx_idle_active_low, 0x4c, 0, 32);

/* avg_l0_tx_idle_active_due_to_l1_req_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, avg_l0_tx_idle_active_due_to_l1_req_high, 0x50, 0, 32);

/* avg_l0_tx_idle_active_due_to_l1_req_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, avg_l0_tx_idle_active_due_to_l1_req_low, 0x54, 0, 32);

/* avg_l0_tx_idle_req_not_yet_active_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, avg_l0_tx_idle_req_not_yet_active_high, 0x58, 0, 32);

/* avg_l0_tx_idle_req_not_yet_active_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, avg_l0_tx_idle_req_not_yet_active_low, 0x5c, 0, 32);

/* avg_l0_rx_idle_blocks_cnt_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, avg_l0_rx_idle_blocks_cnt_high, 0x60, 0, 32);

/* avg_l0_rx_idle_blocks_cnt_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, avg_l0_rx_idle_blocks_cnt_low, 0x64, 0, 32);

/* ppcnt_l1_general_counters_l1_entry_quiet_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_entry_quiet_high, 0x8, 0, 32);

/* ppcnt_l1_general_counters_l1_entry_quiet_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_entry_quiet_low, 0xc, 0, 32);

/* ppcnt_l1_general_counters_l1_force_entry_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_force_entry_high, 0x10, 0, 32);

/* ppcnt_l1_general_counters_l1_force_entry_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_force_entry_low, 0x14, 0, 32);

/* ppcnt_l1_general_counters_l1_exit_local_desired_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_exit_local_desired_high, 0x18, 0, 32);

/* ppcnt_l1_general_counters_l1_exit_local_desired_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_exit_local_desired_low, 0x1c, 0, 32);

/* ppcnt_l1_general_counters_l1_exit_local_recal_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_exit_local_recal_high, 0x20, 0, 32);

/* ppcnt_l1_general_counters_l1_exit_local_recal_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_exit_local_recal_low, 0x24, 0, 32);

/* ppcnt_l1_general_counters_l1_exit_local_recal_with_traffic_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_exit_local_recal_with_traffic_high, 0x28, 0, 32);

/* ppcnt_l1_general_counters_l1_exit_local_recal_with_traffic_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_exit_local_recal_with_traffic_low, 0x2c, 0, 32);

/* ppcnt_l1_general_counters_l1_exit_local_recal_without_traffic_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_exit_local_recal_without_traffic_high, 0x30, 0, 32);

/* ppcnt_l1_general_counters_l1_exit_local_recal_without_traffic_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_exit_local_recal_without_traffic_low, 0x34, 0, 32);

/* ppcnt_l1_general_counters_l1_exit_remote_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_exit_remote_high, 0x38, 0, 32);

/* ppcnt_l1_general_counters_l1_exit_remote_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_exit_remote_low, 0x3c, 0, 32);

/* ppcnt_l1_general_counters_l1_exit_remote_probably_recal_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_exit_remote_probably_recal_high, 0x40, 0, 32);

/* ppcnt_l1_general_counters_l1_exit_remote_probably_recal_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_exit_remote_probably_recal_low, 0x44, 0, 32);

/* ppcnt_l1_general_counters_local_full_bw_exit_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_local_full_bw_exit_high, 0x48, 0, 32);

/* ppcnt_l1_general_counters_local_full_bw_exit_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_local_full_bw_exit_low, 0x4c, 0, 32);

/* ppcnt_l1_general_counters_local_low_power_enter_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_local_low_power_enter_high, 0x50, 0, 32);

/* ppcnt_l1_general_counters_local_low_power_enter_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_local_low_power_enter_low, 0x54, 0, 32);

/* ppcnt_l1_general_counters_remote_full_bw_exit_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_remote_full_bw_exit_high, 0x58, 0, 32);

/* ppcnt_l1_general_counters_remote_full_bw_exit_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_remote_full_bw_exit_low, 0x5c, 0, 32);

/* ppcnt_l1_general_counters_remote_low_power_enter_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_remote_low_power_enter_high, 0x60, 0, 32);

/* ppcnt_l1_general_counters_remote_low_power_enter_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_remote_low_power_enter_low, 0x64, 0, 32);

/* ppcnt_l1_general_counters_local_low_power_exit_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_local_low_power_exit_high, 0x68, 0, 32);

/* ppcnt_l1_general_counters_local_low_power_exit_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_local_low_power_exit_low, 0x6c, 0, 32);

/* ppcnt_l1_general_counters_local_full_bw_enter_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_local_full_bw_enter_high, 0x70, 0, 32);

/* ppcnt_l1_general_counters_local_full_bw_enter_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_local_full_bw_enter_low, 0x74, 0, 32);

/* ppcnt_l1_general_counters_remote_low_power_exit_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_remote_low_power_exit_high, 0x78, 0, 32);

/* ppcnt_l1_general_counters_remote_low_power_exit_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_remote_low_power_exit_low, 0x7c, 0, 32);

/* ppcnt_l1_general_counters_remote_full_bw_enter_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_remote_full_bw_enter_high, 0x80, 0, 32);

/* ppcnt_l1_general_counters_remote_full_bw_enter_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_remote_full_bw_enter_low, 0x84, 0, 32);

/* ppcnt_l1_general_counters_high_speed_steady_state_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_high_speed_steady_state_high, 0x88, 0, 32);

/* ppcnt_l1_general_counters_high_speed_steady_state_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_high_speed_steady_state_low, 0x8c, 0, 32);

/* ppcnt_l1_general_counters_l1_steady_state_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_steady_state_high, 0x90, 0, 32);

/* ppcnt_l1_general_counters_l1_steady_state_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_steady_state_low, 0x94, 0, 32);

/* ppcnt_l1_general_counters_other_state_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_other_state_high, 0x98, 0, 32);

/* ppcnt_l1_general_counters_other_state_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_other_state_low, 0x9c, 0, 32);

/* ppcnt_l1_general_counters_time_since_last_clear_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_time_since_last_clear_high, 0xa0, 0, 32);

/* ppcnt_l1_general_counters_time_since_last_clear_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_time_since_last_clear_low, 0xa4, 0, 32);

/* ppcnt_l1_general_counters_l1_total_entries_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_total_entries_high, 0xa8, 0, 32);

/* ppcnt_l1_general_counters_l1_total_entries_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_total_entries_low, 0xac, 0, 32);

/* ppcnt_l1_general_counters_l1_total_local_exits_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_total_local_exits_high, 0xb0, 0, 32);

/* ppcnt_l1_general_counters_l1_total_local_exits_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_total_local_exits_low, 0xb4, 0, 32);

/* ppcnt_l1_general_counters_l1_total_exits_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_total_exits_high, 0xb8, 0, 32);

/* ppcnt_l1_general_counters_l1_total_exits_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_total_exits_low, 0xbc, 0, 32);

/* ppcnt_l1_general_counters_l1_rev2_granularity
 * Access: OP
 */
MLXSW_ITEM8(reg, ppcnt, ppcnt_l1_general_counters_l1_rev2_granularity, 0xc0, 6, 1);

/* ppcnt_l1_general_counters_relative_high_speed_steady_state_time
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, ppcnt_l1_general_counters_relative_high_speed_steady_state_time, 0xc2, 0, 10);

/* ppcnt_l1_general_counters_relative_l1_steady_state_time
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, ppcnt_l1_general_counters_relative_l1_steady_state_time, 0xc6, 0, 10);

/* ppcnt_l1_general_counters_relative_other_state_time
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, ppcnt_l1_general_counters_relative_other_state_time, 0xca, 0, 10);

/* ppcnt_l1_general_counters_l1_remote_desire_without_local_desire_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_remote_desire_without_local_desire_high, 0xcc, 0, 32);

/* ppcnt_l1_general_counters_l1_remote_desire_without_local_desire_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_l1_remote_desire_without_local_desire_low, 0xd0, 0, 32);

/* ppcnt_l1_general_counters_linkup_tx_idle_state_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_linkup_tx_idle_state_high, 0xd4, 0, 32);

/* ppcnt_l1_general_counters_linkup_tx_idle_state_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_linkup_tx_idle_state_low, 0xd8, 0, 32);

/* ppcnt_l1_general_counters_local_full_low_power_enter_high
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_local_full_low_power_enter_high, 0xdc, 0, 32);

/* ppcnt_l1_general_counters_local_full_low_power_enter_low
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_l1_general_counters_local_full_low_power_enter_low, 0xe0, 0, 32);

/* average_local_full_bw_exit_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, average_local_full_bw_exit_high, 0x8, 0, 32);

/* average_local_full_bw_exit_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, average_local_full_bw_exit_low, 0xc, 0, 32);

/* average_local_low_power_enter_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, average_local_low_power_enter_high, 0x10, 0, 32);

/* average_local_low_power_enter_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, average_local_low_power_enter_low, 0x14, 0, 32);

/* average_local_low_power_exit_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, average_local_low_power_exit_high, 0x18, 0, 32);

/* average_local_low_power_exit_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, average_local_low_power_exit_low, 0x1c, 0, 32);

/* average_remote_low_power_exit_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, average_remote_low_power_exit_high, 0x20, 0, 32);

/* average_remote_low_power_exit_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, average_remote_low_power_exit_low, 0x24, 0, 32);

/* average_low_power_exit_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, average_low_power_exit_high, 0x28, 0, 32);

/* average_low_power_exit_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, average_low_power_exit_low, 0x2c, 0, 32);

/* average_local_full_bw_enter_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, average_local_full_bw_enter_high, 0x30, 0, 32);

/* average_local_full_bw_enter_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, average_local_full_bw_enter_low, 0x34, 0, 32);

/* average_remote_full_bw_enter_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, average_remote_full_bw_enter_high, 0x38, 0, 32);

/* average_remote_full_bw_enter_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, average_remote_full_bw_enter_low, 0x3c, 0, 32);

/* average_full_bw_enter_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, average_full_bw_enter_high, 0x40, 0, 32);

/* average_full_bw_enter_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, average_full_bw_enter_low, 0x44, 0, 32);

/* average_local_full_low_power_enter_high
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, average_local_full_low_power_enter_high, 0x48, 0, 32);

/* average_local_full_low_power_enter_low
 * Access: RW
 */
MLXSW_ITEM32(reg, ppcnt, average_local_full_low_power_enter_low, 0x4c, 0, 32);

/* total_successful_recovery_events
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, total_successful_recovery_events, 0x8, 0, 32);

/* unintentional_link_down_events
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, unintentional_link_down_events, 0xc, 0, 32);

/* intentional_link_down_events
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, intentional_link_down_events, 0x10, 0, 32);

/* time_in_last_host_logical_recovery
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, time_in_last_host_logical_recovery, 0x14, 0, 32);

/* time_in_last_host_serdes_feq_recovery
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, time_in_last_host_serdes_feq_recovery, 0x18, 0, 32);

/* time_in_last_module_tx_disable_recovery
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, time_in_last_module_tx_disable_recovery, 0x1c, 0, 32);

/* time_in_last_module_datapath_full_toggle_recovery
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, time_in_last_module_datapath_full_toggle_recovery, 0x20, 0, 32);

/* total_time_in_host_logical_recovery
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, total_time_in_host_logical_recovery, 0x24, 0, 32);

/* total_time_in_host_serdes_feq_recovery
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, total_time_in_host_serdes_feq_recovery, 0x28, 0, 32);

/* total_time_in_module_tx_disable_recovery
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, total_time_in_module_tx_disable_recovery, 0x2c, 0, 32);

/* total_time_in_module_datapath_full_toggle_recovery
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, total_time_in_module_datapath_full_toggle_recovery, 0x30, 0, 32);

/* total_host_logical_recovery_count
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, total_host_logical_recovery_count, 0x34, 0, 32);

/* total_host_serdes_feq_recovery_count
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, total_host_serdes_feq_recovery_count, 0x38, 0, 32);

/* total_module_tx_disable_recovery_count
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, total_module_tx_disable_recovery_count, 0x3c, 0, 32);

/* total_module_datapath_full_toggle_recovery_count
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, total_module_datapath_full_toggle_recovery_count, 0x40, 0, 32);

/* total_host_logical_succesful_recovery_count
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, total_host_logical_succesful_recovery_count, 0x44, 0, 32);

/* total_host_serdes_feq_succesful_recovery_count
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, total_host_serdes_feq_succesful_recovery_count, 0x48, 0, 32);

/* total_module_tx_disable_succesful_recovery_count
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, total_module_tx_disable_succesful_recovery_count, 0x4c, 0, 32);

/* total_module_datapath_full_toggle_succesful_recovery_count
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, total_module_datapath_full_toggle_succesful_recovery_count, 0x50, 0, 32);

/* time_since_last_recovery
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, time_since_last_recovery, 0x54, 0, 32);

/* last_host_logical_recovery_attempts_count
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, last_host_logical_recovery_attempts_count, 0x58, 0, 32);

/* last_host_serdes_feq_attempts_count
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, last_host_serdes_feq_attempts_count, 0x5c, 0, 32);

/* time_between_last_2_recoveries
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, time_between_last_2_recoveries, 0x62, 0, 16);

/* last_host_serdes_partial_eq_attempts_count
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, last_host_serdes_partial_eq_attempts_count, 0x64, 0, 32);

/* last_successful_recovery_time
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, last_successful_recovery_time, 0x68, 0, 32);

/* total_successful_recovery_time
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, total_successful_recovery_time, 0x6c, 0, 32);

/* last_successful_recovery_step_attempts
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, last_successful_recovery_step_attempts, 0x70, 0, 32);

/* last_rs_fec_uncorrectable_during_recovery_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, last_rs_fec_uncorrectable_during_recovery_high, 0x74, 0, 32);

/* last_rs_fec_uncorrectable_during_recovery_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, last_rs_fec_uncorrectable_during_recovery_low, 0x78, 0, 32);

/* total_rs_fec_uncorrectable_during_recovery_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, total_rs_fec_uncorrectable_during_recovery_high, 0x7c, 0, 32);

/* total_rs_fec_uncorrectable_during_recovery_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, total_rs_fec_uncorrectable_during_recovery_low, 0x80, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_r8_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_r8_high, 0x8, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_r8_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_r8_low, 0xc, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_r16_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_r16_high, 0x10, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_r16_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_r16_low, 0x14, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_r_16_24_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_r_16_24_high, 0x18, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_r_16_24_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_r_16_24_low, 0x1c, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_r_25_32_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_r_25_32_high, 0x20, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_r_25_32_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_r_25_32_low, 0x24, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_r_33_64_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_r_33_64_high, 0x28, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_r_33_64_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_r_33_64_low, 0x2c, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_r_65_128_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_r_65_128_high, 0x30, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_r_65_128_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_r_65_128_low, 0x34, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_r_129_256_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_r_129_256_high, 0x38, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_r_129_256_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_r_129_256_low, 0x3c, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_r_257_512_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_r_257_512_high, 0x40, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_r_257_512_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_r_257_512_low, 0x44, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_r_513_mtu_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_r_513_mtu_high, 0x48, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_r_513_mtu_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_r_513_mtu_low, 0x4c, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_t8_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_t8_high, 0x50, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_t8_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_t8_low, 0x54, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_t16_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_t16_high, 0x58, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_t16_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_t16_low, 0x5c, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_t_16_24_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_t_16_24_high, 0x60, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_t_16_24_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_t_16_24_low, 0x64, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_t_25_32_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_t_25_32_high, 0x68, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_t_25_32_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_t_25_32_low, 0x6c, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_t_33_64_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_t_33_64_high, 0x70, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_t_33_64_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_t_33_64_low, 0x74, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_t_65_128_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_t_65_128_high, 0x78, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_t_65_128_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_t_65_128_low, 0x7c, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_t_129_256_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_t_129_256_high, 0x80, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_t_129_256_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_t_129_256_low, 0x84, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_t_257_512_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_t_257_512_high, 0x88, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_t_257_512_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_t_257_512_low, 0x8c, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_t_513_mtu_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_t_513_mtu_high, 0x90, 0, 32);

/* ppcnt_infiniband_packet_histograms_cnt_t_513_mtu_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_packet_histograms_cnt_t_513_mtu_low, 0x94, 0, 32);

/* port_rcv_ibg1_nvl_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_ibg1_nvl_pkts_high, 0x8, 0, 32);

/* port_rcv_ibg1_nvl_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_ibg1_nvl_pkts_low, 0xc, 0, 32);

/* port_rcv_ibg1_non_nvl_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_ibg1_non_nvl_pkts_high, 0x10, 0, 32);

/* port_rcv_ibg1_non_nvl_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_ibg1_non_nvl_pkts_low, 0x14, 0, 32);

/* port_rcv_ibg2_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_ibg2_pkts_high, 0x18, 0, 32);

/* port_rcv_ibg2_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_ibg2_pkts_low, 0x1c, 0, 32);

/* port_xmit_ibg1_nvl_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_ibg1_nvl_pkts_high, 0x20, 0, 32);

/* port_xmit_ibg1_nvl_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_ibg1_nvl_pkts_low, 0x24, 0, 32);

/* port_xmit_ibg1_non_nvl_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_ibg1_non_nvl_pkts_high, 0x28, 0, 32);

/* port_xmit_ibg1_non_nvl_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_ibg1_non_nvl_pkts_low, 0x2c, 0, 32);

/* port_xmit_ibg2_pkts_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_ibg2_pkts_high, 0x30, 0, 32);

/* port_xmit_ibg2_pkts_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_ibg2_pkts_low, 0x34, 0, 32);

/* dqs2llu_xmit_wait_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, dqs2llu_xmit_wait_high, 0x8, 0, 32);

/* dqs2llu_xmit_wait_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, dqs2llu_xmit_wait_low, 0xc, 0, 32);

/* dqs2llu_xmit_wait_arb_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, dqs2llu_xmit_wait_arb_high, 0x10, 0, 32);

/* dqs2llu_xmit_wait_arb_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, dqs2llu_xmit_wait_arb_low, 0x14, 0, 32);

/* port_rcv_pkts_vl_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_pkts_vl_high, 0x18, 0, 32);

/* port_rcv_pkts_vl_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_pkts_vl_low, 0x1c, 0, 32);

/* port_rcv_pkts_vl15_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_pkts_vl15_high, 0x20, 0, 32);

/* port_rcv_pkts_vl15_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_pkts_vl15_low, 0x24, 0, 32);

/* port_rcv_data_vl_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_data_vl_high, 0x28, 0, 32);

/* port_rcv_data_vl_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_data_vl_low, 0x2c, 0, 32);

/* port_rcv_data_vl15_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_data_vl15_high, 0x30, 0, 32);

/* port_rcv_data_vl15_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_rcv_data_vl15_low, 0x34, 0, 32);

/* port_xmit_pkts_vl_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_pkts_vl_high, 0x38, 0, 32);

/* port_xmit_pkts_vl_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_pkts_vl_low, 0x3c, 0, 32);

/* port_xmit_pkts_vl15_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_pkts_vl15_high, 0x40, 0, 32);

/* port_xmit_pkts_vl15_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_pkts_vl15_low, 0x44, 0, 32);

/* port_xmit_data_vl_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_data_vl_high, 0x48, 0, 32);

/* port_xmit_data_vl_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_data_vl_low, 0x4c, 0, 32);

/* port_xmit_data_vl15_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_data_vl15_high, 0x50, 0, 32);

/* port_xmit_data_vl15_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_xmit_data_vl15_low, 0x54, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_9_16_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_9_16_high, 0x8, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_9_16_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_9_16_low, 0xc, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_17_32_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_17_32_high, 0x10, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_17_32_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_17_32_low, 0x14, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_33_64_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_33_64_high, 0x18, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_33_64_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_33_64_low, 0x1c, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_65_127_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_65_127_high, 0x20, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_65_127_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_65_127_low, 0x24, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_128_255_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_128_255_high, 0x28, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_128_255_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_128_255_low, 0x2c, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_256_511_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_256_511_high, 0x30, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_256_511_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_256_511_low, 0x34, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_512_1023_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_512_1023_high, 0x38, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_512_1023_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_512_1023_low, 0x3c, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_1024_1518_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_1024_1518_high, 0x40, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_1024_1518_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_1024_1518_low, 0x44, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_1519_2047_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_1519_2047_high, 0x48, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_1519_2047_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_1519_2047_low, 0x4c, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_2048_4095_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_2048_4095_high, 0x50, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_2048_4095_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_2048_4095_low, 0x54, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_4096_8191_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_4096_8191_high, 0x58, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_4096_8191_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_4096_8191_low, 0x5c, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_mtu_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_mtu_high, 0x60, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_r_mtu_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_r_mtu_low, 0x64, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_9_16_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_9_16_high, 0x68, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_9_16_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_9_16_low, 0x6c, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_17_32_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_17_32_high, 0x70, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_17_32_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_17_32_low, 0x74, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_33_64_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_33_64_high, 0x78, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_33_64_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_33_64_low, 0x7c, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_65_127_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_65_127_high, 0x80, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_65_127_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_65_127_low, 0x84, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_128_255_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_128_255_high, 0x88, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_128_255_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_128_255_low, 0x8c, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_256_511_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_256_511_high, 0x90, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_256_511_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_256_511_low, 0x94, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_512_1023_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_512_1023_high, 0x98, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_512_1023_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_512_1023_low, 0x9c, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_1024_1518_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_1024_1518_high, 0xa0, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_1024_1518_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_1024_1518_low, 0xa4, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_1519_2047_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_1519_2047_high, 0xa8, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_1519_2047_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_1519_2047_low, 0xac, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_2048_4095_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_2048_4095_high, 0xb0, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_2048_4095_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_2048_4095_low, 0xb4, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_4096_8191_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_4096_8191_high, 0xb8, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_4096_8191_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_4096_8191_low, 0xbc, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_mtu_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_mtu_high, 0xc0, 0, 32);

/* ppcnt_infiniband_extended_packet_histograms_cnt_t_mtu_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, ppcnt_infiniband_extended_packet_histograms_cnt_t_mtu_low, 0xc4, 0, 32);

/* last_raw_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, last_raw_ber_coef, 0x8, 0, 4);

/* last_raw_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, last_raw_ber_magnitude, 0x9, 0, 8);

/* min_raw_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, min_raw_ber_coef, 0xc, 0, 4);

/* min_raw_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, min_raw_ber_magnitude, 0xd, 0, 8);

/* max_raw_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, max_raw_ber_coef, 0xe, 0, 4);

/* max_raw_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, max_raw_ber_magnitude, 0xf, 0, 8);

/* cons_raw_norm_ber
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, cons_raw_norm_ber, 0x12, 0, 16);

/* num_of_raw_ber_warnings
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, num_of_raw_ber_warnings, 0x14, 0, 16);

/* num_of_raw_ber_alarms
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, num_of_raw_ber_alarms, 0x16, 0, 16);

/* last_eff_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, last_eff_ber_coef, 0x18, 0, 4);

/* last_eff_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, last_eff_ber_magnitude, 0x19, 0, 8);

/* min_eff_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, min_eff_ber_coef, 0x1c, 0, 4);

/* min_eff_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, min_eff_ber_magnitude, 0x1d, 0, 8);

/* max_eff_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, max_eff_ber_coef, 0x1e, 0, 4);

/* max_eff_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, max_eff_ber_magnitude, 0x1f, 0, 8);

/* cons_eff_norm_ber
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, cons_eff_norm_ber, 0x22, 0, 16);

/* num_of_eff_ber_warnings
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, num_of_eff_ber_warnings, 0x24, 0, 16);

/* num_of_eff_ber_alarms
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, num_of_eff_ber_alarms, 0x26, 0, 16);

/* last_symbol_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, last_symbol_ber_coef, 0x28, 0, 4);

/* last_symbol_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, last_symbol_ber_magnitude, 0x29, 0, 8);

/* min_symbol_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, min_symbol_ber_coef, 0x2c, 0, 4);

/* min_symbol_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, min_symbol_ber_magnitude, 0x2d, 0, 8);

/* max_symbol_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, max_symbol_ber_coef, 0x2e, 0, 4);

/* max_symbol_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, max_symbol_ber_magnitude, 0x2f, 0, 8);

/* cons_symbol_norm_ber
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, cons_symbol_norm_ber, 0x32, 0, 16);

/* num_of_symbol_ber_warnings
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, num_of_symbol_ber_warnings, 0x34, 0, 16);

/* num_of_symbol_ber_alarms
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, num_of_symbol_ber_alarms, 0x36, 0, 16);

/* mean_est_eff_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, mean_est_eff_ber_coef, 0x38, 0, 4);

/* mean_est_eff_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, mean_est_eff_ber_magnitude, 0x39, 0, 8);

/* min_est_eff_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, min_est_eff_ber_coef, 0x3a, 0, 4);

/* min_est_eff_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, min_est_eff_ber_magnitude, 0x3b, 0, 8);

/* last_est_eff_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, last_est_eff_ber_coef, 0x3c, 0, 4);

/* last_est_eff_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, last_est_eff_ber_magnitude, 0x3d, 0, 8);

/* max_est_eff_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, max_est_eff_ber_coef, 0x3e, 0, 4);

/* max_est_eff_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, max_est_eff_ber_magnitude, 0x3f, 0, 8);

/* last_est_alarm_cross
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, last_est_alarm_cross, 0x40, 7, 1);

/* cons_est_norm_ber
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, cons_est_norm_ber, 0x42, 0, 16);

/* num_of_est_eff_ber_warnings
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, num_of_est_eff_ber_warnings, 0x44, 0, 16);

/* num_of_est_eff_ber_alarms
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, num_of_est_eff_ber_alarms, 0x46, 0, 16);

/* mean_ucr_eff_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, mean_ucr_eff_ber_coef, 0x48, 0, 4);

/* mean_ucr_eff_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, mean_ucr_eff_ber_magnitude, 0x49, 0, 8);

/* min_ucr_eff_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, min_ucr_eff_ber_coef, 0x4a, 0, 4);

/* min_ucr_eff_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, min_ucr_eff_ber_magnitude, 0x4b, 0, 8);

/* last_ucr_eff_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, last_ucr_eff_ber_coef, 0x4c, 0, 4);

/* last_ucr_eff_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, last_ucr_eff_ber_magnitude, 0x4d, 0, 8);

/* max_ucr_eff_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, max_ucr_eff_ber_coef, 0x4e, 0, 4);

/* max_ucr_eff_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, max_ucr_eff_ber_magnitude, 0x4f, 0, 8);

/* last_ucr_alarm_cross
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcnt, last_ucr_alarm_cross, 0x50, 7, 1);

/* cons_ucr_norm_ber
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, cons_ucr_norm_ber, 0x52, 0, 16);

/* num_of_ucr_eff_ber_warnings
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, num_of_ucr_eff_ber_warnings, 0x54, 0, 16);

/* num_of_ucr_eff_ber_alarms
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, num_of_ucr_eff_ber_alarms, 0x56, 0, 16);

/* diff_fc_zero_hist
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, diff_fc_zero_hist, 0x5a, 0, 10);

/* last_diff_time_stamp
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, last_diff_time_stamp, 0x5c, 0, 32);

/* mlid
 * Access: RO
 */
MLXSW_ITEM16(reg, ppcnt, mlid, 0xe, 0, 16);

/* incoming_good_mc_read_req_pkt_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, incoming_good_mc_read_req_pkt_high, 0x18, 0, 32);

/* incoming_good_mc_read_req_pkt_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, incoming_good_mc_read_req_pkt_low, 0x1c, 0, 32);

/* incoming_good_mc_read_req_data_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, incoming_good_mc_read_req_data_high, 0x20, 0, 32);

/* incoming_good_mc_read_req_data_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, incoming_good_mc_read_req_data_low, 0x24, 0, 32);

/* incoming_good_mc_write_req_pkt_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, incoming_good_mc_write_req_pkt_high, 0x28, 0, 32);

/* incoming_good_mc_write_req_pkt_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, incoming_good_mc_write_req_pkt_low, 0x2c, 0, 32);

/* incoming_good_mc_write_req_data_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, incoming_good_mc_write_req_data_high, 0x30, 0, 32);

/* incoming_good_mc_write_req_data_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, incoming_good_mc_write_req_data_low, 0x34, 0, 32);

/* incoming_good_mc_read_resp_pkt_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, incoming_good_mc_read_resp_pkt_high, 0x38, 0, 32);

/* incoming_good_mc_read_resp_pkt_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, incoming_good_mc_read_resp_pkt_low, 0x3c, 0, 32);

/* incoming_good_mc_read_resp_data_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, incoming_good_mc_read_resp_data_high, 0x40, 0, 32);

/* incoming_good_mc_read_resp_data_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, incoming_good_mc_read_resp_data_low, 0x44, 0, 32);

/* incoming_good_mc_write_resp_pkt_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, incoming_good_mc_write_resp_pkt_high, 0x48, 0, 32);

/* incoming_good_mc_write_resp_pkt_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, incoming_good_mc_write_resp_pkt_low, 0x4c, 0, 32);

/* incoming_good_mc_write_resp_data_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, incoming_good_mc_write_resp_data_high, 0x50, 0, 32);

/* incoming_good_mc_write_resp_data_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, incoming_good_mc_write_resp_data_low, 0x54, 0, 32);

/* generated_mc_read_resp_pkt_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, generated_mc_read_resp_pkt_high, 0x58, 0, 32);

/* generated_mc_read_resp_pkt_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, generated_mc_read_resp_pkt_low, 0x5c, 0, 32);

/* generated_mc_read_resp_data_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, generated_mc_read_resp_data_high, 0x60, 0, 32);

/* generated_mc_read_resp_data_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, generated_mc_read_resp_data_low, 0x64, 0, 32);

/* generated_mc_write_resp_pkt_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, generated_mc_write_resp_pkt_high, 0x68, 0, 32);

/* generated_mc_write_resp_pkt_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, generated_mc_write_resp_pkt_low, 0x6c, 0, 32);

/* generated_mc_write_resp_data_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, generated_mc_write_resp_data_high, 0x70, 0, 32);

/* generated_mc_write_resp_data_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, generated_mc_write_resp_data_low, 0x74, 0, 32);

/* dropped_pkt_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, dropped_pkt_high, 0x78, 0, 32);

/* dropped_pkt_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, dropped_pkt_low, 0x7c, 0, 32);

/* trapped_pkt_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, trapped_pkt_high, 0x80, 0, 32);

/* trapped_pkt_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, trapped_pkt_low, 0x84, 0, 32);

/* nvl_reduction_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, nvl_reduction_errors_high, 0x88, 0, 32);

/* nvl_reduction_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, nvl_reduction_errors_low, 0x8c, 0, 32);

/* nvl_reduction_penalty_box_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, nvl_reduction_penalty_box_errors_high, 0x90, 0, 32);

/* nvl_reduction_penalty_box_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, nvl_reduction_penalty_box_errors_low, 0x94, 0, 32);

/* port_nvr_local_alm_id_allocations_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_nvr_local_alm_id_allocations_high, 0x98, 0, 32);

/* port_nvr_local_alm_id_allocations_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_nvr_local_alm_id_allocations_low, 0x9c, 0, 32);

/* port_nvr_global_alm_id_allocations_high
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_nvr_global_alm_id_allocations_high, 0xa0, 0, 32);

/* port_nvr_global_alm_id_allocations_low
 * Access: RO
 */
MLXSW_ITEM32(reg, ppcnt, port_nvr_global_alm_id_allocations_low, 0xa4, 0, 32);

/* ptse
 * ----
 *
 */
/* swid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ptse, swid, 0x0, 0, 8);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ptse, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ptse, lp_msb, 0x2, 4, 2);

/* tx_ready
 * Access: RO
 */
MLXSW_ITEM8(reg, ptse, tx_ready, 0x3, 0, 2);

/* pter
 * ----
 *
 */
/* error_page
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pter, error_page, 0x0, 0, 4);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pter, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pter, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pter, lp_msb, 0x2, 4, 2);

/* plane_ind
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pter, plane_ind, 0x2, 0, 4);

/* port_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pter, port_type, 0x3, 4, 4);

/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, pter, status, 0x3, 0, 4);

/* pter_phy_page_reg_error_type_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, pter, pter_phy_page_reg_error_type_oper, 0x4, 0, 4);

/* pter_phy_page_reg_error_type_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pter, pter_phy_page_reg_error_type_cap, 0x6, 0, 8);

/* pter_phy_page_reg_error_type_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, pter, pter_phy_page_reg_error_type_admin, 0x7, 0, 8);

/* pter_phy_page_reg_error_injection_time
 * Access: RW
 */
MLXSW_ITEM32(reg, pter, pter_phy_page_reg_error_injection_time, 0x8, 12, 16);

/* pter_phy_page_reg_ber_mantissa
 * Access: RW
 */
MLXSW_ITEM8(reg, pter, pter_phy_page_reg_ber_mantissa, 0xa, 0, 4);

/* pter_phy_page_reg_ber_exp
 * Access: RW
 */
MLXSW_ITEM8(reg, pter, pter_phy_page_reg_ber_exp, 0xb, 0, 8);

/* pter_phy_page_reg_rs_symbol_errors
 * Access: RW
 */
MLXSW_ITEM16(reg, pter, pter_phy_page_reg_rs_symbol_errors, 0xe, 0, 16);

/* pter_phy_page_reg_rs_errors_interval
 * Access: RW
 */
MLXSW_ITEM16(reg, pter, pter_phy_page_reg_rs_errors_interval, 0x12, 0, 16);

/* pter_port_page_reg_error_type_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pter, pter_port_page_reg_error_type_cap, 0x6, 0, 8);

/* pter_port_page_reg_error_type_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, pter, pter_port_page_reg_error_type_admin, 0x7, 0, 8);

/* pter_port_page_reg_error_count
 * Access: RW
 */
MLXSW_ITEM32(reg, pter, pter_port_page_reg_error_count, 0x8, 12, 5);

/* prei
 * ----
 *
 */
/* mps
 * Access: INDEX
 */
MLXSW_ITEM8(reg, prei, mps, 0x0, 7, 1);

/* version
 * Access: RO
 */
MLXSW_ITEM8(reg, prei, version, 0x0, 0, 4);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, prei, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, prei, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, prei, lp_msb, 0x2, 4, 2);

/* plane_ind
 * Access: INDEX
 */
MLXSW_ITEM8(reg, prei, plane_ind, 0x2, 0, 4);

/* time_res
 * Access: OP
 */
MLXSW_ITEM8(reg, prei, time_res, 0x3, 4, 2);

/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, prei, status, 0x3, 0, 4);

/* error_type_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, prei, error_type_oper, 0x4, 0, 4);

/* error_type_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, prei, error_type_cap, 0x6, 0, 8);

/* error_type_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, prei, error_type_admin, 0x7, 0, 8);

/* error_injection_time
 * Access: RW
 */
MLXSW_ITEM32(reg, prei, error_injection_time, 0x8, 12, 16);

/* mixer_offset0
 * Access: RW
 */
MLXSW_ITEM16(reg, prei, mixer_offset0, 0x10, 0, 16);

/* mixer_offset1
 * Access: RW
 */
MLXSW_ITEM16(reg, prei, mixer_offset1, 0x12, 0, 16);

/* plar
 * ----
 *
 */
/* local_port_allocated
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, plar, local_port_allocated, 0x0, 0, 32, 4, 0, 4, 0);

/* pude
 * ----
 *
 */
/* swid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pude, swid, 0x0, 0, 8);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pude, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pude, lp_msb, 0x2, 4, 2);

/* admin_status
 * Access: RO
 */
MLXSW_ITEM8(reg, pude, admin_status, 0x2, 0, 4);

/* oper_status
 * Access: RO
 */
MLXSW_ITEM8(reg, pude, oper_status, 0x3, 0, 4);

/* local_reason_opcode
 * Access: RO
 */
MLXSW_ITEM8(reg, pude, local_reason_opcode, 0x4, 0, 8);

/* logical_state_status
 * Access: RO
 */
MLXSW_ITEM8(reg, pude, logical_state_status, 0x5, 0, 3);

/* physical_state_status
 * Access: RO
 */
MLXSW_ITEM8(reg, pude, physical_state_status, 0x7, 4, 4);

/* nmx_adminstate_status
 * Access: RO
 */
MLXSW_ITEM8(reg, pude, nmx_adminstate_status, 0x7, 0, 3);

/* pprm
 * ----
 *
 */
/* param_select
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pprm, param_select, 0x0, 4, 4);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pprm, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pprm, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pprm, lp_msb, 0x2, 4, 2);

/* plane_ind
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pprm, plane_ind, 0x2, 0, 4);

/* neg_type_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pprm, neg_type_cap, 0x3, 5, 1);

/* phy_recovery_algo_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pprm, phy_recovery_algo_cap, 0x3, 4, 1);

/* ovrd_no_neg_bhvr
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, ovrd_no_neg_bhvr, 0x3, 2, 2);

/* bypass_log_st
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, bypass_log_st, 0x3, 1, 1);

/* bypass_neg
 * Access: WO
 */
MLXSW_ITEM8(reg, pprm, bypass_neg, 0x3, 0, 1);

/* oper_logic_re_lock_res
 * Access: RO
 */
MLXSW_ITEM8(reg, pprm, oper_logic_re_lock_res, 0x4, 6, 2);

/* wd_logic_re_lock_res
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, wd_logic_re_lock_res, 0x4, 4, 2);

/* no_neg_bhvr
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, no_neg_bhvr, 0x4, 0, 4);

/* recovery_neg_type
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, recovery_neg_type, 0x5, 6, 2);

/* phy_recovery_algo
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, phy_recovery_algo, 0x5, 0, 3);

/* recovery_types_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pprm, recovery_types_cap, 0x7, 0, 8);

/* phy_recovery_steps
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, phy_recovery_steps, 0x9, 0, 4);

/* host_logic_re_lock
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, host_logic_re_lock, 0xa, 4, 4);

/* host_serdes_feq
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, host_serdes_feq, 0xa, 0, 4);

/* module_tx_disable
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, module_tx_disable, 0xb, 4, 4);

/* module_datapath_full_toggle
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, module_datapath_full_toggle, 0xb, 0, 4);

/* link_down_timeout
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, link_down_timeout, 0xe, 0, 16);

/* link_down_timeout_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, pprm, link_down_timeout_oper, 0x12, 0, 16);

/* oper_host_serdes_feq_res
 * Access: RO
 */
MLXSW_ITEM8(reg, pprm, oper_host_serdes_feq_res, 0x14, 6, 2);

/* wd_host_serdes_feq_res
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, wd_host_serdes_feq_res, 0x14, 4, 2);

/* draining_timeout
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, draining_timeout, 0x17, 0, 8);

/* phy_recovery_algo_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, pprm, phy_recovery_algo_oper, 0x19, 0, 3);

/* oper_recovery
 * Access: RO
 */
MLXSW_ITEM8(reg, pprm, oper_recovery, 0x1b, 0, 8);

/* wd_host_logic_re_lock
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, wd_host_logic_re_lock, 0x1c, 0, 8);

/* wd_host_serdes_feq
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, wd_host_serdes_feq, 0x1d, 0, 8);

/* wd_module_tx_disable
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, wd_module_tx_disable, 0x1e, 0, 8);

/* wd_module_full_toggle
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, wd_module_full_toggle, 0x1f, 0, 8);

/* oper_time_host_logic_re_lock
 * Access: RO
 */
MLXSW_ITEM8(reg, pprm, oper_time_host_logic_re_lock, 0x20, 0, 8);

/* oper_time_host_serdes_feq
 * Access: RO
 */
MLXSW_ITEM8(reg, pprm, oper_time_host_serdes_feq, 0x21, 0, 8);

/* oper_time_module_tx_disable
 * Access: RO
 */
MLXSW_ITEM8(reg, pprm, oper_time_module_tx_disable, 0x22, 0, 8);

/* oper_time_module_full_toggle
 * Access: RO
 */
MLXSW_ITEM8(reg, pprm, oper_time_module_full_toggle, 0x23, 0, 8);

/* eff_align_aq_time
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, eff_align_aq_time, 0x2c, 0, 8);

/* step2_preset_support_mask
 * Access: RO
 */
MLXSW_ITEM32(reg, pprm, step2_preset_support_mask, 0x2c, 12, 8);

/* step1_preset_support_mask
 * Access: RO
 */
MLXSW_ITEM8(reg, pprm, step1_preset_support_mask, 0x2f, 0, 8);

/* step2_time_gran
 * Access: RO
 */
MLXSW_ITEM8(reg, pprm, step2_time_gran, 0x30, 4, 2);

/* step2_preset_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, step2_preset_admin, 0x32, 0, 4);

/* step1_preset_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, step1_preset_admin, 0x33, 0, 4);

/* step2_peq_n_retry_pre1
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, step2_peq_n_retry_pre1, 0x34, 2, 5);

/* step1_peq_n_retry_pre1
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step1_peq_n_retry_pre1, 0x34, 4, 5);

/* step2_state_65_to_66_time_pre1
 * Access: RW
 */
MLXSW_ITEM32(reg, pprm, step2_state_65_to_66_time_pre1, 0x34, 10, 10);

/* step1_state_65_to_66_time_pre1
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step1_state_65_to_66_time_pre1, 0x36, 0, 10);

/* step2_state_66_to_67_time_pre1
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step2_state_66_to_67_time_pre1, 0x38, 0, 16);

/* step1_state_66_to_67_time_pre1
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step1_state_66_to_67_time_pre1, 0x3a, 0, 16);

/* step2_state_67_to_68_time_pre1
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step2_state_67_to_68_time_pre1, 0x3c, 0, 16);

/* step1_state_67_to_68_time_pre1
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step1_state_67_to_68_time_pre1, 0x3e, 0, 16);

/* step2_state_60_timeout
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step2_state_60_timeout, 0x40, 0, 16);

/* step1_state_60_timeout
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step1_state_60_timeout, 0x42, 0, 16);

/* step2_state_61_timeout
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step2_state_61_timeout, 0x44, 0, 16);

/* step1_state_61_timeout
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step1_state_61_timeout, 0x46, 0, 16);

/* step2_state_62_timeout
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step2_state_62_timeout, 0x48, 0, 16);

/* step1_state_62_timeout
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step1_state_62_timeout, 0x4a, 0, 16);

/* step2_state_60_to_linkup_timeout
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step2_state_60_to_linkup_timeout, 0x4c, 0, 16);

/* step2_state_60_to_linkup_timeout_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, pprm, step2_state_60_to_linkup_timeout_oper, 0x4e, 0, 16);

/* step2_peq_n_retry_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, pprm, step2_peq_n_retry_oper, 0x50, 0, 5);

/* step1_peq_n_retry_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, pprm, step1_peq_n_retry_oper, 0x51, 0, 5);

/* step2_preset_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, pprm, step2_preset_oper, 0x52, 0, 4);

/* step1_preset_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, pprm, step1_preset_oper, 0x53, 0, 4);

/* step2_state_65_to_66_time_oper
 * Access: RO
 */
MLXSW_ITEM32(reg, pprm, step2_state_65_to_66_time_oper, 0x54, 10, 10);

/* step1_state_65_to_66_time_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, pprm, step1_state_65_to_66_time_oper, 0x56, 0, 10);

/* step2_state_66_to_67_time_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, pprm, step2_state_66_to_67_time_oper, 0x58, 0, 16);

/* step1_state_66_to_67_time_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, pprm, step1_state_66_to_67_time_oper, 0x5a, 0, 16);

/* step2_state_67_to_68_time_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, pprm, step2_state_67_to_68_time_oper, 0x5c, 0, 16);

/* step1_state_67_to_68_time_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, pprm, step1_state_67_to_68_time_oper, 0x5e, 0, 16);

/* step2_state_60_timeout_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, pprm, step2_state_60_timeout_oper, 0x60, 0, 16);

/* step1_state_60_timeout_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, pprm, step1_state_60_timeout_oper, 0x62, 0, 16);

/* step2_state_61_timeout_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, pprm, step2_state_61_timeout_oper, 0x64, 0, 16);

/* step1_state_61_timeout_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, pprm, step1_state_61_timeout_oper, 0x66, 0, 16);

/* step2_state_62_timeout_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, pprm, step2_state_62_timeout_oper, 0x68, 0, 16);

/* step1_state_62_timeout_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, pprm, step1_state_62_timeout_oper, 0x6a, 0, 16);

/* custom1_abort_ugl_cmd_on_rec_entry
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, custom1_abort_ugl_cmd_on_rec_entry, 0x6c, 7, 1);

/* custom1_rec_send_pam4_en
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, custom1_rec_send_pam4_en, 0x6c, 6, 1);

/* custom1_ugl_tx_sync_fifo_reset_time
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, custom1_ugl_tx_sync_fifo_reset_time, 0x6c, 0, 4);

/* custom1_peq_start_time
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, custom1_peq_start_time, 0x6d, 0, 4);

/* custom1_rec_send_pam4_cmd
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, custom1_rec_send_pam4_cmd, 0x6e, 0, 6);

/* custom1_rec_send_pam2_cmd
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, custom1_rec_send_pam2_cmd, 0x6f, 0, 6);

/* custom2_abort_ugl_cmd_on_rec_entry
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, custom2_abort_ugl_cmd_on_rec_entry, 0x70, 7, 1);

/* custom2_rec_send_pam4_en
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, custom2_rec_send_pam4_en, 0x70, 6, 1);

/* custom2_ugl_tx_sync_fifo_reset_time
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, custom2_ugl_tx_sync_fifo_reset_time, 0x70, 0, 4);

/* custom2_peq_start_time
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, custom2_peq_start_time, 0x71, 0, 4);

/* custom2_rec_send_pam4_cmd
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, custom2_rec_send_pam4_cmd, 0x72, 0, 6);

/* custom2_rec_send_pam2_cmd
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, custom2_rec_send_pam2_cmd, 0x73, 0, 6);

/* step2_peq_n_retry_pre2
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, step2_peq_n_retry_pre2, 0x74, 2, 5);

/* step1_peq_n_retry_pre2
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step1_peq_n_retry_pre2, 0x74, 4, 5);

/* step2_state_65_to_66_time_pre2
 * Access: RW
 */
MLXSW_ITEM32(reg, pprm, step2_state_65_to_66_time_pre2, 0x74, 10, 10);

/* step1_state_65_to_66_time_pre2
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step1_state_65_to_66_time_pre2, 0x76, 0, 10);

/* step2_state_66_to_67_time_pre2
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step2_state_66_to_67_time_pre2, 0x78, 0, 16);

/* step1_state_66_to_67_time_pre2
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step1_state_66_to_67_time_pre2, 0x7a, 0, 16);

/* step2_state_67_to_68_time_pre2
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step2_state_67_to_68_time_pre2, 0x7c, 0, 16);

/* step1_state_67_to_68_time_pre2
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step1_state_67_to_68_time_pre2, 0x7e, 0, 16);

/* step2_peq_n_retry_pre3
 * Access: RW
 */
MLXSW_ITEM8(reg, pprm, step2_peq_n_retry_pre3, 0x80, 2, 5);

/* step1_peq_n_retry_pre3
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step1_peq_n_retry_pre3, 0x80, 4, 5);

/* step2_state_65_to_66_time_pre3
 * Access: RW
 */
MLXSW_ITEM32(reg, pprm, step2_state_65_to_66_time_pre3, 0x80, 10, 10);

/* step1_state_65_to_66_time_pre3
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step1_state_65_to_66_time_pre3, 0x82, 0, 10);

/* step2_state_66_to_67_time_pre3
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step2_state_66_to_67_time_pre3, 0x84, 0, 16);

/* step1_state_66_to_67_time_pre3
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step1_state_66_to_67_time_pre3, 0x86, 0, 16);

/* step2_state_67_to_68_time_pre3
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step2_state_67_to_68_time_pre3, 0x88, 0, 16);

/* step1_state_67_to_68_time_pre3
 * Access: RW
 */
MLXSW_ITEM16(reg, pprm, step1_state_67_to_68_time_pre3, 0x8a, 0, 16);

/* slrg
 * ----
 *
 */
/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, status, 0x0, 4, 4);

/* version
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, version, 0x0, 0, 4);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slrg, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slrg, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slrg, lp_msb, 0x2, 4, 2);

/* lane
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slrg, lane, 0x2, 0, 4);

/* port_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slrg, port_type, 0x3, 4, 4);

/* all_lanes
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slrg, all_lanes, 0x3, 1, 1);

/* test_mode
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slrg, test_mode, 0x3, 0, 1);

/* slrg_40nm_28nm_time_to_link_up
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_40nm_28nm_time_to_link_up, 0x4, 0, 16);

/* slrg_40nm_28nm_grade_lane_speed
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_40nm_28nm_grade_lane_speed, 0x7, 0, 4);

/* slrg_40nm_28nm_grade_version
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_40nm_28nm_grade_version, 0x8, 0, 8);

/* slrg_40nm_28nm_grade
 * Access: RO
 */
MLXSW_ITEM32(reg, slrg, slrg_40nm_28nm_grade, 0x8, 0, 24);

/* slrg_40nm_28nm_offset_units
 * Access: RW
 */
MLXSW_ITEM8(reg, slrg, slrg_40nm_28nm_offset_units, 0xc, 6, 2);

/* slrg_40nm_28nm_phase_units
 * Access: RW
 */
MLXSW_ITEM8(reg, slrg, slrg_40nm_28nm_phase_units, 0xc, 4, 2);

/* slrg_40nm_28nm_height_grade_type
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_40nm_28nm_height_grade_type, 0xc, 0, 4);

/* slrg_40nm_28nm_height_grade
 * Access: RO
 */
MLXSW_ITEM32(reg, slrg, slrg_40nm_28nm_height_grade, 0xc, 0, 24);

/* slrg_40nm_28nm_height_dz
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_40nm_28nm_height_dz, 0x10, 0, 16);

/* slrg_40nm_28nm_height_dv
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_40nm_28nm_height_dv, 0x12, 0, 16);

/* slrg_40nm_28nm_height_sigma
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_40nm_28nm_height_sigma, 0x16, 0, 16);

/* slrg_40nm_28nm_height_eo_pos
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_40nm_28nm_height_eo_pos, 0x18, 0, 16);

/* slrg_40nm_28nm_height_eo_neg
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_40nm_28nm_height_eo_neg, 0x1a, 0, 16);

/* slrg_40nm_28nm_phase_grade_type
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_40nm_28nm_phase_grade_type, 0x1c, 0, 4);

/* slrg_40nm_28nm_phase_grade
 * Access: RO
 */
MLXSW_ITEM32(reg, slrg, slrg_40nm_28nm_phase_grade, 0x1c, 0, 24);

/* slrg_40nm_28nm_phase_eo_pos
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_40nm_28nm_phase_eo_pos, 0x20, 0, 16);

/* slrg_40nm_28nm_phase_eo_neg
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_40nm_28nm_phase_eo_neg, 0x22, 0, 16);

/* slrg_40nm_28nm_ffe_set_tested
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_40nm_28nm_ffe_set_tested, 0x24, 0, 16);

/* slrg_40nm_28nm_test_errors_per_lane
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_40nm_28nm_test_errors_per_lane, 0x26, 0, 16);

/* slrg_16nm_grade_lane_speed
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_16nm_grade_lane_speed, 0x7, 0, 4);

/* slrg_16nm_grade_version
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_16nm_grade_version, 0x8, 0, 8);

/* slrg_16nm_grade
 * Access: RO
 */
MLXSW_ITEM32(reg, slrg, slrg_16nm_grade, 0x8, 0, 24);

/* slrg_16nm_height_eo_pos_up
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_16nm_height_eo_pos_up, 0xc, 0, 16);

/* slrg_16nm_height_eo_neg_up
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_16nm_height_eo_neg_up, 0xe, 0, 16);

/* slrg_16nm_phase_eo_pos_up
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_16nm_phase_eo_pos_up, 0x10, 0, 8);

/* slrg_16nm_phase_eo_neg_up
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_16nm_phase_eo_neg_up, 0x11, 0, 8);

/* slrg_16nm_height_eo_pos_mid
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_16nm_height_eo_pos_mid, 0x12, 0, 16);

/* slrg_16nm_height_eo_neg_mid
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_16nm_height_eo_neg_mid, 0x14, 0, 16);

/* slrg_16nm_phase_eo_pos_mid
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_16nm_phase_eo_pos_mid, 0x16, 0, 8);

/* slrg_16nm_phase_eo_neg_mid
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_16nm_phase_eo_neg_mid, 0x17, 0, 8);

/* slrg_16nm_height_eo_pos_low
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_16nm_height_eo_pos_low, 0x18, 0, 16);

/* slrg_16nm_height_eo_neg_low
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_16nm_height_eo_neg_low, 0x1a, 0, 16);

/* slrg_16nm_phase_eo_pos_low
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_16nm_phase_eo_pos_low, 0x1c, 0, 8);

/* slrg_16nm_phase_eo_neg_low
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_16nm_phase_eo_neg_low, 0x1d, 0, 8);

/* slrg_16nm_mid_eye_grade
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_16nm_mid_eye_grade, 0x20, 0, 16);

/* slrg_16nm_up_eye_grade
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_16nm_up_eye_grade, 0x22, 0, 16);

/* slrg_16nm_dn_eye_grade
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_16nm_dn_eye_grade, 0x26, 0, 16);

/* slrg_7nm_fom_measurement
 * Access: OP
 */
MLXSW_ITEM8(reg, slrg, slrg_7nm_fom_measurement, 0x7, 0, 4);

/* slrg_7nm_fom_meas_type_read
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_7nm_fom_meas_type_read, 0x8, 7, 1);

/* slrg_7nm_fom_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, slrg, slrg_7nm_fom_mode, 0x9, 0, 3);

/* slrg_7nm_initial_fom
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_7nm_initial_fom, 0xb, 0, 8);

/* slrg_7nm_last_fom
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_7nm_last_fom, 0xc, 0, 8);

/* slrg_7nm_upper_eye
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_7nm_upper_eye, 0xd, 0, 8);

/* slrg_7nm_mid_eye
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_7nm_mid_eye, 0xe, 0, 8);

/* slrg_7nm_lower_eye
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_7nm_lower_eye, 0xf, 0, 8);

/* slrg_7nm_fom_ext_conf
 * Access: WO
 */
MLXSW_ITEM8(reg, slrg, slrg_7nm_fom_ext_conf, 0x10, 7, 1);

/* slrg_7nm_fom_ext_conf_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_7nm_fom_ext_conf_cap, 0x10, 6, 1);

/* slrg_7nm_nblks_max
 * Access: RW
 */
MLXSW_ITEM8(reg, slrg, slrg_7nm_nblks_max, 0x12, 4, 4);

/* slrg_7nm_nerrs_min
 * Access: RW
 */
MLXSW_ITEM8(reg, slrg, slrg_7nm_nerrs_min, 0x13, 0, 8);

/* slrg_5nm_fom_measurement
 * Access: OP
 */
MLXSW_ITEM8(reg, slrg, slrg_5nm_fom_measurement, 0x7, 0, 4);

/* slrg_5nm_fom_meas_type_read
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_5nm_fom_meas_type_read, 0x8, 7, 1);

/* slrg_5nm_fom_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, slrg, slrg_5nm_fom_mode, 0x9, 0, 4);

/* slrg_5nm_initial_fom
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_5nm_initial_fom, 0xa, 0, 16);

/* slrg_5nm_last_fom
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_5nm_last_fom, 0xc, 0, 16);

/* slrg_5nm_upper_eye
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_5nm_upper_eye, 0xe, 0, 16);

/* slrg_5nm_mid_eye
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_5nm_mid_eye, 0x10, 0, 16);

/* slrg_5nm_lower_eye
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_5nm_lower_eye, 0x12, 0, 16);

/* slrg_5nm_fom_ext_conf
 * Access: WO
 */
MLXSW_ITEM8(reg, slrg, slrg_5nm_fom_ext_conf, 0x14, 7, 1);

/* slrg_5nm_fom_ext_conf_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_5nm_fom_ext_conf_cap, 0x14, 6, 1);

/* slrg_5nm_nblks_max
 * Access: RW
 */
MLXSW_ITEM8(reg, slrg, slrg_5nm_nblks_max, 0x16, 4, 4);

/* slrg_5nm_nerrs_min
 * Access: RW
 */
MLXSW_ITEM8(reg, slrg, slrg_5nm_nerrs_min, 0x17, 0, 8);

/* grade_threshold
 * Access: RW
 */
MLXSW_ITEM8(reg, slrg, grade_threshold, 0x6, 0, 8);

/* grade_dbi
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, grade_dbi, 0x8, 0, 8);

/* lanes_mask_cause
 * Access: RO
 */
MLXSW_ITEM32(reg, slrg, lanes_mask_cause, 0x8, 0, 17);

/* grade_lane
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, slrg, grade_lane, 0xc, 16);

/* fom_mode
 * Access: OP
 */
MLXSW_ITEM8(reg, slrg, fom_mode, 0x7, 0, 4);

/* fom_lane0
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, fom_lane0, 0x8, 0, 16);

/* fom_lane1
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, fom_lane1, 0xa, 0, 16);

/* fom_lane2
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, fom_lane2, 0xc, 0, 16);

/* fom_lane3
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, fom_lane3, 0xe, 0, 16);

/* fom_lane4
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, fom_lane4, 0x10, 0, 16);

/* fom_lane5
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, fom_lane5, 0x12, 0, 16);

/* fom_lane6
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, fom_lane6, 0x14, 0, 16);

/* fom_lane7
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, fom_lane7, 0x16, 0, 16);

/* slrg_3nm_fom_measurement
 * Access: OP
 */
MLXSW_ITEM8(reg, slrg, slrg_3nm_fom_measurement, 0x7, 0, 4);

/* slrg_3nm_fom_meas_type_read
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_3nm_fom_meas_type_read, 0x8, 7, 1);

/* slrg_3nm_fom_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, slrg, slrg_3nm_fom_mode, 0x9, 0, 4);

/* slrg_3nm_initial_fom
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_3nm_initial_fom, 0xa, 0, 16);

/* slrg_3nm_last_fom
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_3nm_last_fom, 0xc, 0, 16);

/* slrg_3nm_upper_eye
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_3nm_upper_eye, 0xe, 0, 16);

/* slrg_3nm_mid_eye
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_3nm_mid_eye, 0x10, 0, 16);

/* slrg_3nm_lower_eye
 * Access: RO
 */
MLXSW_ITEM16(reg, slrg, slrg_3nm_lower_eye, 0x12, 0, 16);

/* slrg_3nm_fom_ext_conf
 * Access: WO
 */
MLXSW_ITEM8(reg, slrg, slrg_3nm_fom_ext_conf, 0x14, 7, 1);

/* slrg_3nm_fom_ext_conf_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, slrg, slrg_3nm_fom_ext_conf_cap, 0x14, 6, 1);

/* slrg_3nm_nblks_max
 * Access: RW
 */
MLXSW_ITEM8(reg, slrg, slrg_3nm_nblks_max, 0x16, 4, 4);

/* slrg_3nm_nerrs_min
 * Access: RW
 */
MLXSW_ITEM8(reg, slrg, slrg_3nm_nerrs_min, 0x17, 0, 8);

/* ppslg
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppslg, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppslg, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppslg, lp_msb, 0x2, 4, 2);

/* page_select
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppslg, page_select, 0x7, 0, 5);

/* ppslg_l1_capabilities_and_status_l1_hw_local_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_capabilities_and_status_l1_hw_local_cap, 0x8, 7, 1);

/* ppslg_l1_capabilities_and_status_l1_hw_local_req
 * Access: RO
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_capabilities_and_status_l1_hw_local_req, 0x8, 6, 1);

/* ppslg_l1_capabilities_and_status_l1_hw_remote_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_capabilities_and_status_l1_hw_remote_cap, 0x8, 5, 1);

/* ppslg_l1_capabilities_and_status_l1_hw_remote_req
 * Access: RO
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_capabilities_and_status_l1_hw_remote_req, 0x8, 4, 1);

/* ppslg_l1_capabilities_and_status_l1_hw_neg_res
 * Access: RO
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_capabilities_and_status_l1_hw_neg_res, 0x8, 3, 1);

/* ppslg_l1_capabilities_and_status_l1_local_rev2_support
 * Access: RO
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_capabilities_and_status_l1_local_rev2_support, 0x8, 1, 1);

/* ppslg_l1_capabilities_and_status_l1_force_local_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_capabilities_and_status_l1_force_local_cap, 0x9, 7, 1);

/* ppslg_l1_capabilities_and_status_l1_force_local_req
 * Access: RO
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_capabilities_and_status_l1_force_local_req, 0x9, 6, 1);

/* ppslg_l1_capabilities_and_status_l1_force_remote_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_capabilities_and_status_l1_force_remote_cap, 0x9, 5, 1);

/* ppslg_l1_capabilities_and_status_l1_force_remote_req
 * Access: RO
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_capabilities_and_status_l1_force_remote_req, 0x9, 4, 1);

/* ppslg_l1_capabilities_and_status_l1_timeout_conf_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_capabilities_and_status_l1_timeout_conf_cap, 0x9, 3, 1);

/* ppslg_l1_capabilities_and_status_l1_local_peq_period_force_neg_support
 * Access: RO
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_capabilities_and_status_l1_local_peq_period_force_neg_support, 0x9, 2, 1);

/* ppslg_l1_capabilities_and_status_l1_remote_peq_duration_req
 * Access: RO
 */
MLXSW_ITEM16(reg, ppslg, ppslg_l1_capabilities_and_status_l1_remote_peq_duration_req, 0xe, 0, 16);

/* ppslg_l1_capabilities_and_status_l1_remote_peq_recal_period_req
 * Access: RO
 */
MLXSW_ITEM16(reg, ppslg, ppslg_l1_capabilities_and_status_l1_remote_peq_recal_period_req, 0x12, 0, 16);

/* ppslg_l1_capabilities_and_status_l1_remote_min_linkup_time_req
 * Access: RO
 */
MLXSW_ITEM16(reg, ppslg, ppslg_l1_capabilities_and_status_l1_remote_min_linkup_time_req, 0x16, 0, 16);

/* ppslg_l1_capabilities_and_status_l1_peq_duration_res
 * Access: RO
 */
MLXSW_ITEM16(reg, ppslg, ppslg_l1_capabilities_and_status_l1_peq_duration_res, 0x1a, 0, 16);

/* ppslg_l1_capabilities_and_status_l1_peq_recal_period_res
 * Access: RO
 */
MLXSW_ITEM16(reg, ppslg, ppslg_l1_capabilities_and_status_l1_peq_recal_period_res, 0x1e, 0, 16);

/* ppslg_l1_capabilities_and_status_l1_min_linkup_time_res
 * Access: RO
 */
MLXSW_ITEM16(reg, ppslg, ppslg_l1_capabilities_and_status_l1_min_linkup_time_res, 0x22, 0, 16);

/* ppslg_l1_capabilities_and_status_max_actual_active_recal_time
 * Access: RO
 */
MLXSW_ITEM16(reg, ppslg, ppslg_l1_capabilities_and_status_max_actual_active_recal_time, 0x26, 0, 11);

/* ppslg_l1_capabilities_and_status_max_active_recal_time
 * Access: RO
 */
MLXSW_ITEM16(reg, ppslg, ppslg_l1_capabilities_and_status_max_active_recal_time, 0x2a, 0, 11);

/* ppslg_l1_configuration_l1_local_peq_recal_period_force
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_configuration_l1_local_peq_recal_period_force, 0x8, 7, 1);

/* ppslg_l1_configuration_module_mode_in_l1
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_configuration_module_mode_in_l1, 0x8, 4, 2);

/* ppslg_l1_configuration_l1_local_rev2_support
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_configuration_l1_local_rev2_support, 0x8, 2, 1);

/* ppslg_l1_configuration_l1_timeout_conf
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_configuration_l1_timeout_conf, 0x8, 0, 2);

/* ppslg_l1_configuration_l1_local_peq_duration_req
 * Access: RW
 */
MLXSW_ITEM16(reg, ppslg, ppslg_l1_configuration_l1_local_peq_duration_req, 0xa, 0, 16);

/* ppslg_l1_configuration_l1_local_peq_recal_period_req
 * Access: RW
 */
MLXSW_ITEM16(reg, ppslg, ppslg_l1_configuration_l1_local_peq_recal_period_req, 0xe, 0, 16);

/* ppslg_l1_configuration_l1_local_min_linkup_time_req
 * Access: RW
 */
MLXSW_ITEM16(reg, ppslg, ppslg_l1_configuration_l1_local_min_linkup_time_req, 0x12, 0, 16);

/* ppslg_l1_debug_page_const_quiet_time
 * Access: RW
 */
MLXSW_ITEM16(reg, ppslg, ppslg_l1_debug_page_const_quiet_time, 0x8, 3, 8);

/* ppslg_l1_debug_page_l1_desired_pu
 * Access: RW
 */
MLXSW_ITEM32(reg, ppslg, ppslg_l1_debug_page_l1_desired_pu, 0x8, 15, 2);

/* ppslg_l1_debug_page_l1_force_local_pu
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_debug_page_l1_force_local_pu, 0xa, 4, 2);

/* ppslg_l1_debug_page_l1_force_rmt_pu
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_debug_page_l1_force_rmt_pu, 0xa, 1, 2);

/* ppslg_l1_debug_page_l1_traffic_allowed_pu
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_debug_page_l1_traffic_allowed_pu, 0xb, 6, 2);

/* ppslg_l1_debug_page_l1_pipe_empty_pu
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_debug_page_l1_pipe_empty_pu, 0xb, 3, 2);

/* ppslg_l1_debug_page_idle_status_pu
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_debug_page_idle_status_pu, 0xb, 0, 2);

/* ppslg_l1_debug_page_min_time_in_linkup
 * Access: RW
 */
MLXSW_ITEM32(reg, ppslg, ppslg_l1_debug_page_min_time_in_linkup, 0xc, 10, 16);

/* ppslg_l1_debug_page_min_sync_block_cnt_for_remote_l1_desire
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_debug_page_min_sync_block_cnt_for_remote_l1_desire, 0xf, 0, 4);

/* ppslg_l1_debug_page_min_time_in_linkup_tx_idle
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_debug_page_min_time_in_linkup_tx_idle, 0x10, 0, 4);

/* ppslg_l1_debug_page_linkup_tx_empty_to_l1_sleep_num_of_sync_blocks
 * Access: RW
 */
MLXSW_ITEM32(reg, ppslg, ppslg_l1_debug_page_linkup_tx_empty_to_l1_sleep_num_of_sync_blocks, 0x10, 13, 4);

/* ppslg_l1_debug_page_min_time_in_linkup_tx_empty
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_debug_page_min_time_in_linkup_tx_empty, 0x13, 0, 6);

/* ppslg_l1_debug_page_go_to_quiet_time
 * Access: RW
 */
MLXSW_ITEM16(reg, ppslg, ppslg_l1_debug_page_go_to_quiet_time, 0x14, 7, 6);

/* ppslg_l1_debug_page_retimer_align_time
 * Access: RW
 */
MLXSW_ITEM32(reg, ppslg, ppslg_l1_debug_page_retimer_align_time, 0x14, 12, 6);

/* ppslg_l1_debug_page_quiet_entry_time
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_debug_page_quiet_entry_time, 0x17, 0, 6);

/* ppslg_l1_debug_page_min_time_to_send_announce
 * Access: RW
 */
MLXSW_ITEM16(reg, ppslg, ppslg_l1_debug_page_min_time_to_send_announce, 0x18, 4, 8);

/* ppslg_l1_debug_page_min_time_in_tx_hs
 * Access: RW
 */
MLXSW_ITEM16(reg, ppslg, ppslg_l1_debug_page_min_time_in_tx_hs, 0x1a, 3, 10);

/* ppslg_l1_debug_page_reset_to_hw
 * Access: WO
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_debug_page_reset_to_hw, 0x1b, 0, 1);

/* ppslg_l1_debug_page_set_mask
 * Access: WO
 */
MLXSW_ITEM32(reg, ppslg, ppslg_l1_debug_page_set_mask, 0x1c, 0, 32);

/* ppslg_l1_debug_page_move_to_pam4_time
 * Access: RW
 */
MLXSW_ITEM16(reg, ppslg, ppslg_l1_debug_page_move_to_pam4_time, 0x20, 1, 8);

/* ppslg_l1_debug_page_wait_for_cdr_lock_time
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_debug_page_wait_for_cdr_lock_time, 0x22, 0, 6);

/* ppslg_l1_debug_page_retimer_align_rx_powerdown_timer
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_debug_page_retimer_align_rx_powerdown_timer, 0x23, 0, 6);

/* ppslg_l1_debug_page_l1_high_speed_pam2_prbs_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l1_debug_page_l1_high_speed_pam2_prbs_mode, 0x27, 0, 6);

/* l0_local_rx_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ppslg, l0_local_rx_cap, 0x8, 7, 1);

/* l0_local_tx_req
 * Access: RO
 */
MLXSW_ITEM8(reg, ppslg, l0_local_tx_req, 0x8, 6, 1);

/* l0_remote_rx_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ppslg, l0_remote_rx_cap, 0x8, 5, 1);

/* l0_remote_tx_req
 * Access: RO
 */
MLXSW_ITEM8(reg, ppslg, l0_remote_tx_req, 0x8, 4, 1);

/* l0_local_res_rx
 * Access: RO
 */
MLXSW_ITEM8(reg, ppslg, l0_local_res_rx, 0x8, 3, 1);

/* l0_local_res_tx
 * Access: RO
 */
MLXSW_ITEM8(reg, ppslg, l0_local_res_tx, 0x8, 2, 1);

/* ppslg_l0_debug_page_l0_desired_pu
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l0_debug_page_l0_desired_pu, 0x8, 5, 2);

/* ppslg_l0_debug_page_l0_off_countdown_value
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l0_debug_page_l0_off_countdown_value, 0x9, 5, 3);

/* ppslg_l0_debug_page_l0_on_countdown_value
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l0_debug_page_l0_on_countdown_value, 0x9, 0, 3);

/* ppslg_l0_debug_page_min_num_of_fec_blocks_in_linkup_tx_l0_idle
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l0_debug_page_min_num_of_fec_blocks_in_linkup_tx_l0_idle, 0xa, 1, 5);

/* ppslg_l0_debug_page_l0_min_time_in_linkup
 * Access: RW
 */
MLXSW_ITEM16(reg, ppslg, ppslg_l0_debug_page_l0_min_time_in_linkup, 0xc, 2, 10);

/* ppslg_l0_debug_page_l1_counter_max_min_sel
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l0_debug_page_l1_counter_max_min_sel, 0xe, 4, 4);

/* ppslg_l0_debug_page_max_num_of_fec_blocks_in_linkup_tx_l0_idle
 * Access: RW
 */
MLXSW_ITEM16(reg, ppslg, ppslg_l0_debug_page_max_num_of_fec_blocks_in_linkup_tx_l0_idle, 0xe, 0, 9);

/* ppslg_l0_debug_page_l1_counter_max_min_reset
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l0_debug_page_l1_counter_max_min_reset, 0x10, 7, 1);

/* ppslg_l0_debug_page_l1_counter_max_min_granularity
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslg, ppslg_l0_debug_page_l1_counter_max_min_granularity, 0x10, 6, 1);

/* ppslg_l0_debug_page_chosen_l1_counter_max_value
 * Access: RO
 */
MLXSW_ITEM32(reg, ppslg, ppslg_l0_debug_page_chosen_l1_counter_max_value, 0x10, 0, 20);

/* ppslg_l0_debug_page_chosen_l1_counter_min_value
 * Access: RO
 */
MLXSW_ITEM32(reg, ppslg, ppslg_l0_debug_page_chosen_l1_counter_min_value, 0x14, 0, 20);

/* ppslg_l0_debug_page_set_mask
 * Access: WO
 */
MLXSW_ITEM32(reg, ppslg, ppslg_l0_debug_page_set_mask, 0x18, 0, 32);

/* plds
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, plds, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, plds, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, plds, lp_msb, 0x2, 4, 2);

/* delayed_recovery_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, plds, delayed_recovery_cap, 0x3, 7, 1);

/* delayed_recovery_force
 * Access: RW
 */
MLXSW_ITEM8(reg, plds, delayed_recovery_force, 0x3, 6, 1);

/* plr_retry_force
 * Access: RW
 */
MLXSW_ITEM8(reg, plds, plr_retry_force, 0x3, 5, 1);

/* fec_uncor_force
 * Access: RW
 */
MLXSW_ITEM8(reg, plds, fec_uncor_force, 0x3, 4, 1);

/* plr_retry_force_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, plds, plr_retry_force_cap, 0x3, 3, 1);

/* fec_uncor_force_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, plds, fec_uncor_force_cap, 0x3, 2, 1);

/* plr_retry_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, plds, plr_retry_cap, 0x3, 1, 1);

/* fec_uncor_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, plds, fec_uncor_cap, 0x3, 0, 1);

/* config_down_cap
 * Access: RO
 */
MLXSW_ITEM16(reg, plds, config_down_cap, 0x4, 5, 11);

/* link_down_cap
 * Access: RO
 */
MLXSW_ITEM32(reg, plds, link_down_cap, 0x4, 10, 8);

/* link_dwn_default_state_cfg
 * Access: RW
 */
MLXSW_ITEM8(reg, plds, link_dwn_default_state_cfg, 0x7, 4, 4);

/* neg_res
 * Access: RO
 */
MLXSW_ITEM8(reg, plds, neg_res, 0x7, 0, 1);

/* link_dwn_local_error_cfg
 * Access: RW
 */
MLXSW_ITEM8(reg, plds, link_dwn_local_error_cfg, 0x8, 5, 3);

/* link_dwn_rmt_error_cfg
 * Access: RW
 */
MLXSW_ITEM8(reg, plds, link_dwn_rmt_error_cfg, 0x8, 1, 3);

/* link_dwn_rmt_disable_cfg
 * Access: RW
 */
MLXSW_ITEM8(reg, plds, link_dwn_rmt_disable_cfg, 0x9, 5, 3);

/* link_dwn_rmt_sleep_cfg
 * Access: RW
 */
MLXSW_ITEM8(reg, plds, link_dwn_rmt_sleep_cfg, 0x9, 1, 3);

/* link_dwn_rmt_reset_cfg
 * Access: RW
 */
MLXSW_ITEM8(reg, plds, link_dwn_rmt_reset_cfg, 0xa, 5, 3);

/* link_dwn_rmt_up_layer_cfg
 * Access: RW
 */
MLXSW_ITEM8(reg, plds, link_dwn_rmt_up_layer_cfg, 0xa, 1, 3);

/* link_dwn_rmt_port_lock_cfg
 * Access: RW
 */
MLXSW_ITEM8(reg, plds, link_dwn_rmt_port_lock_cfg, 0xb, 5, 3);

/* link_dwn_rmt_thermal_cfg
 * Access: RW
 */
MLXSW_ITEM8(reg, plds, link_dwn_rmt_thermal_cfg, 0xb, 1, 3);

/* linkup_retry_iterations
 * Access: RW
 */
MLXSW_ITEM8(reg, plds, linkup_retry_iterations, 0xc, 0, 8);

/* set_mask
 * Access: WO
 */
MLXSW_ITEM16(reg, plds, set_mask, 0xe, 0, 11);

/* rs_fec_uncor_th_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, plds, rs_fec_uncor_th_oper, 0x10, 1, 7);

/* plr_csn_retry_th
 * Access: RW
 */
MLXSW_ITEM16(reg, plds, plr_csn_retry_th, 0x10, 0, 9);

/* plr_csn_retry_th_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, plds, plr_csn_retry_th_oper, 0x12, 7, 9);

/* rs_fec_uncor_th
 * Access: RW
 */
MLXSW_ITEM8(reg, plds, rs_fec_uncor_th, 0x13, 0, 7);

/* delayed_recovery_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, plds, delayed_recovery_admin, 0x17, 2, 2);

/* delayed_recovery_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, plds, delayed_recovery_oper, 0x17, 0, 2);

/* pmaos
 * -----
 *
 */
/* rst
 * Access: OP
 */
MLXSW_ITEM8(reg, pmaos, rst, 0x0, 7, 1);

/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmaos, slot_index, 0x0, 0, 4);

/* module
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmaos, module, 0x1, 0, 8);

/* admin_status
 * Access: RW
 */
MLXSW_ITEM8(reg, pmaos, admin_status, 0x2, 0, 4);

/* oper_status
 * Access: RO
 */
MLXSW_ITEM8(reg, pmaos, oper_status, 0x3, 0, 4);

/* ase
 * Access: WO
 */
MLXSW_ITEM8(reg, pmaos, ase, 0x4, 7, 1);

/* ee
 * Access: WO
 */
MLXSW_ITEM8(reg, pmaos, ee, 0x4, 6, 1);

/* secondary
 * Access: RO
 */
MLXSW_ITEM8(reg, pmaos, secondary, 0x4, 5, 1);

/* rev_incompatible
 * Access: RO
 */
MLXSW_ITEM8(reg, pmaos, rev_incompatible, 0x4, 4, 1);

/* operational_notification
 * Access: RO
 */
MLXSW_ITEM8(reg, pmaos, operational_notification, 0x5, 0, 4);

/* error_type
 * Access: RO
 */
MLXSW_ITEM8(reg, pmaos, error_type, 0x6, 0, 5);

/* e
 * Access: RW
 */
MLXSW_ITEM8(reg, pmaos, e, 0x7, 0, 2);

/* pmtdb
 * -----
 *
 */
/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtdb, slot_index, 0x0, 0, 4);

/* module
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtdb, module, 0x1, 0, 8);

/* ports_width
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtdb, ports_width, 0x2, 4, 4);

/* num_ports
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtdb, num_ports, 0x2, 0, 4);

/* sub_module
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtdb, sub_module, 0x3, 4, 2);

/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, pmtdb, status, 0x3, 0, 4);

/* port1_width
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtdb, port1_width, 0x4, 4, 4);

/* port_num1
 * Access: RO
 */
MLXSW_ITEM16(reg, pmtdb, port_num1, 0x4, 0, 10);

/* port2_width
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtdb, port2_width, 0x6, 4, 4);

/* port_num2
 * Access: RO
 */
MLXSW_ITEM16(reg, pmtdb, port_num2, 0x6, 0, 10);

/* port3_width
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtdb, port3_width, 0x8, 4, 4);

/* port_num3
 * Access: RO
 */
MLXSW_ITEM16(reg, pmtdb, port_num3, 0x8, 0, 10);

/* port4_width
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtdb, port4_width, 0xa, 4, 4);

/* port_num4
 * Access: RO
 */
MLXSW_ITEM16(reg, pmtdb, port_num4, 0xa, 0, 10);

/* port5_width
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtdb, port5_width, 0xc, 4, 4);

/* port_num5
 * Access: RO
 */
MLXSW_ITEM16(reg, pmtdb, port_num5, 0xc, 0, 10);

/* port6_width
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtdb, port6_width, 0xe, 4, 4);

/* port_num6
 * Access: RO
 */
MLXSW_ITEM16(reg, pmtdb, port_num6, 0xe, 0, 10);

/* port7_width
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtdb, port7_width, 0x10, 4, 4);

/* port_num7
 * Access: RO
 */
MLXSW_ITEM16(reg, pmtdb, port_num7, 0x10, 0, 10);

/* port8_width
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtdb, port8_width, 0x12, 4, 4);

/* port_num8
 * Access: RO
 */
MLXSW_ITEM16(reg, pmtdb, port_num8, 0x12, 0, 10);

/* pmecr
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmecr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmecr, lp_msb, 0x2, 4, 2);

/* ee
 * Access: WO
 */
MLXSW_ITEM8(reg, pmecr, ee, 0x4, 6, 1);

/* eswi
 * Access: WO
 */
MLXSW_ITEM8(reg, pmecr, eswi, 0x4, 0, 1);

/* swi
 * Access: RW
 */
MLXSW_ITEM8(reg, pmecr, swi, 0x6, 0, 1);

/* e
 * Access: RW
 */
MLXSW_ITEM8(reg, pmecr, e, 0x7, 0, 2);

/* pmlpe
 * -----
 *
 */
/* rxtx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlpe, rxtx, 0x0, 7, 1);

/* autosplit
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlpe, autosplit, 0x0, 6, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmlpe, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmlpe, lp_msb, 0x2, 4, 2);

/* width
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlpe, width, 0x3, 0, 8);

/* rx_lane
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, pmlpe, rx_lane, MLXSW_PMLPE_LANE_MODULE_MAPPING_BASE_LEN, 0, 4, MLXSW_PMLPE_LANE_MODULE_MAPPING_REC_LEN, 0, 1, 0);

/* tx_lane
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, pmlpe, tx_lane, MLXSW_PMLPE_LANE_MODULE_MAPPING_BASE_LEN + 1, 0, 4, MLXSW_PMLPE_LANE_MODULE_MAPPING_REC_LEN, 0, 1, 0);

/* sub_module
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, pmlpe, sub_module, MLXSW_PMLPE_LANE_MODULE_MAPPING_BASE_LEN + 2, 4, 2, MLXSW_PMLPE_LANE_MODULE_MAPPING_REC_LEN, 0, 1, 0);

/* slot_index
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, pmlpe, slot_index, MLXSW_PMLPE_LANE_MODULE_MAPPING_BASE_LEN + 2, 0, 4, MLXSW_PMLPE_LANE_MODULE_MAPPING_REC_LEN, 0, 1, 0);

/* module
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, pmlpe, module, MLXSW_PMLPE_LANE_MODULE_MAPPING_BASE_LEN + 3, 0, 8, MLXSW_PMLPE_LANE_MODULE_MAPPING_REC_LEN, 0, 1, 0);

/* pmsc
 * ----
 *
 */
/* port_mapping_updated
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, pmsc, port_mapping_updated, 0x0, 0, 32, 4, 0, 4, 0);

/* pmpe
 * ----
 *
 */
/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmpe, slot_index, 0x0, 0, 4);

/* module
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmpe, module, 0x1, 0, 8);

/* module_status
 * Access: RO
 */
MLXSW_ITEM8(reg, pmpe, module_status, 0x3, 0, 4);

/* error_type
 * Access: RO
 */
MLXSW_ITEM8(reg, pmpe, error_type, 0x6, 0, 5);

/* pmmp
 * ----
 *
 */
/* override_target
 * Access: OP
 */
MLXSW_ITEM8(reg, pmmp, override_target, 0x0, 5, 1);

/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmmp, slot_index, 0x0, 0, 4);

/* module
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmmp, module, 0x1, 0, 8);

/* laser_mask
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmmp, laser_mask, 0x2, 0, 8);

/* els_config_abort
 * Access: WO
 */
MLXSW_ITEM8(reg, pmmp, els_config_abort, 0x3, 4, 1);

/* els_trigger_mode
 * Access: WO
 */
MLXSW_ITEM8(reg, pmmp, els_trigger_mode, 0x3, 2, 2);

/* apply_im
 * Access: WO
 */
MLXSW_ITEM8(reg, pmmp, apply_im, 0x3, 1, 1);

/* sticky
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, sticky, 0x3, 0, 1);

/* eeprom_override_mask
 * Access: WO
 */
MLXSW_ITEM16(reg, pmmp, eeprom_override_mask, 0x4, 0, 16);

/* eeprom_override
 * Access: RW
 */
MLXSW_ITEM16(reg, pmmp, eeprom_override, 0x6, 0, 16);

/* pmmp_qsfp_protocol_override_layout_cable_breakout
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, pmmp_qsfp_protocol_override_layout_cable_breakout, 0xa, 0, 8);

/* pmmp_qsfp_protocol_override_layout_ethernet_compliance_code
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, pmmp_qsfp_protocol_override_layout_ethernet_compliance_code, 0xb, 0, 8);

/* pmmp_qsfp_protocol_override_layout_ext_ethernet_compliance_code
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, pmmp_qsfp_protocol_override_layout_ext_ethernet_compliance_code, 0xe, 0, 8);

/* pmmp_qsfp_protocol_override_layout_giga_ethernet_compliance_code
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, pmmp_qsfp_protocol_override_layout_giga_ethernet_compliance_code, 0xf, 0, 8);

/* pmmp_cmis_protocol_override_layout_mod_pwrup_maxduration
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, pmmp_cmis_protocol_override_layout_mod_pwrup_maxduration, 0x8, 4, 4);

/* pmmp_cmis_protocol_override_layout_media_type_encoding
 * Access: RW
 */
MLXSW_ITEM16(reg, pmmp, pmmp_cmis_protocol_override_layout_media_type_encoding, 0x8, 5, 4);

/* pmmp_cmis_protocol_override_layout_cable_breakout
 * Access: RW
 */
MLXSW_ITEM32(reg, pmmp, pmmp_cmis_protocol_override_layout_cable_breakout, 0x8, 8, 13);

/* pmmp_cmis_protocol_override_layout_host_electrical_compliance
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, pmmp_cmis_protocol_override_layout_host_electrical_compliance, 0xb, 0, 8);

/* pmmp_cmis_protocol_override_layout_dp_init_maxduration
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, pmmp_cmis_protocol_override_layout_dp_init_maxduration, 0xc, 4, 4);

/* pmmp_cmis_protocol_override_layout_app0_disabled
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, pmmp_cmis_protocol_override_layout_app0_disabled, 0xc, 0, 1);

/* pmmp_cmis_protocol_override_layout_dp_tx_on_maxduration
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, pmmp_cmis_protocol_override_layout_dp_tx_on_maxduration, 0xd, 4, 4);

/* pmmp_cmis_protocol_override_layout_dp_tx_off_maxduration
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, pmmp_cmis_protocol_override_layout_dp_tx_off_maxduration, 0xd, 0, 4);

/* pmmp_cmis_protocol_override_layout_module_media_type
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, pmmp_cmis_protocol_override_layout_module_media_type, 0xe, 0, 8);

/* pmmp_cmis_protocol_override_layout_media_compliance
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, pmmp_cmis_protocol_override_layout_media_compliance, 0xf, 0, 8);

/* pmmp_cmis_protocol_override_layout_cmis_override_mask
 * Access: RW
 */
MLXSW_ITEM16(reg, pmmp, pmmp_cmis_protocol_override_layout_cmis_override_mask, 0x12, 0, 16);

/* bit_rate
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, bit_rate, 0x15, 0, 8);

/* cable_technology
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, cable_technology, 0x16, 0, 8);

/* tengig_ethernet_compliance_code
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, tengig_ethernet_compliance_code, 0x17, 0, 8);

/* ext_ethernet_compliance_code
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, ext_ethernet_compliance_code, 0x1a, 0, 8);

/* ethernet_compliance_code
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, ethernet_compliance_code, 0x1b, 0, 8);

/* set_laser_operation
 * Access: WO
 */
MLXSW_ITEM8(reg, pmmp, set_laser_operation, 0x20, 5, 3);

/* module_type
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, module_type, 0x22, 0, 4);

/* cable_length
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, cable_length, 0x23, 0, 8);

/* attenuation_25g
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, attenuation_25g, 0x24, 0, 8);

/* attenuation_12g
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, attenuation_12g, 0x25, 0, 8);

/* attenuation_7g
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, attenuation_7g, 0x26, 0, 8);

/* attenuation_5g
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, attenuation_5g, 0x27, 0, 8);

/* output_fiber_checked_flag
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, output_fiber_checked_flag, 0x28, 0, 8);

/* lane_enable
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, lane_enable, 0x29, 0, 8);

/* attenuation_53g
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, attenuation_53g, 0x2a, 0, 8);

/* module_identifier
 * Access: RW
 */
MLXSW_ITEM8(reg, pmmp, module_identifier, 0x2b, 0, 8);

/* power_setpoint_lane
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, pmmp, power_setpoint_lane, 0x30, 0, 16, 2, 0, 2, 0);

/* pmcr
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmcr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmcr, lp_msb, 0x2, 4, 2);

/* error_code_response
 * Access: RO
 */
MLXSW_ITEM8(reg, pmcr, error_code_response, 0x2, 0, 4);

/* plane_ind
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmcr, plane_ind, 0x3, 4, 4);

/* db
 * Access: OP
 */
MLXSW_ITEM8(reg, pmcr, db, 0x3, 0, 1);

/* dir_dp_ctrl
 * Access: OP
 */
MLXSW_ITEM8(reg, pmcr, dir_dp_ctrl, 0x4, 6, 2);

/* dir_tx_dis_ctrl
 * Access: OP
 */
MLXSW_ITEM8(reg, pmcr, dir_tx_dis_ctrl, 0x4, 4, 2);

/* dir_dp_ctrl_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmcr, dir_dp_ctrl_cap, 0x4, 3, 1);

/* dir_tx_dis_ctrl_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmcr, dir_tx_dis_ctrl_cap, 0x4, 2, 1);

/* tx_disable_override_value
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, tx_disable_override_value, 0x4, 0, 2);

/* tx_disable_override_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmcr, tx_disable_override_cap, 0x5, 2, 1);

/* tx_disable_override_cntl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, tx_disable_override_cntl, 0x5, 0, 2);

/* cdr_override_value
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, cdr_override_value, 0x6, 0, 4);

/* cdr_override_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmcr, cdr_override_cap, 0x7, 2, 2);

/* cdr_override_cntl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, cdr_override_cntl, 0x7, 0, 2);

/* rx_los_override_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, rx_los_override_admin, 0x8, 0, 2);

/* rx_los_override_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmcr, rx_los_override_cap, 0x9, 2, 1);

/* rx_los_override_cntl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, rx_los_override_cntl, 0x9, 0, 2);

/* rx_amp_override_value_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmcr, rx_amp_override_value_cap, 0xa, 4, 4);

/* rx_amp_override_value
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, rx_amp_override_value, 0xa, 0, 4);

/* rx_amp_override_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmcr, rx_amp_override_cap, 0xb, 2, 1);

/* rx_amp_override_cntl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, rx_amp_override_cntl, 0xb, 0, 2);

/* rx_post_emp_override_value_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmcr, rx_post_emp_override_value_cap, 0xc, 4, 4);

/* rx_post_emp_override_value
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, rx_post_emp_override_value, 0xc, 0, 4);

/* rx_post_emp_override_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmcr, rx_post_emp_override_cap, 0xd, 2, 1);

/* rx_post_emp_override_cntl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, rx_post_emp_override_cntl, 0xd, 0, 2);

/* rx_emp_override_value_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmcr, rx_emp_override_value_cap, 0xe, 4, 4);

/* rx_emp_override_value
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, rx_emp_override_value, 0xe, 0, 4);

/* rx_emp_override_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmcr, rx_emp_override_cap, 0xf, 2, 1);

/* rx_emp_override_cntl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, rx_emp_override_cntl, 0xf, 0, 2);

/* tx_adaptive_override_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmcr, tx_adaptive_override_cap, 0x10, 2, 1);

/* tx_adaptive_override_cntrl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, tx_adaptive_override_cntrl, 0x10, 0, 2);

/* tx_adaptive_override_value
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, tx_adaptive_override_value, 0x11, 1, 1);

/* tx_equ_override_value_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmcr, tx_equ_override_value_cap, 0x12, 4, 4);

/* tx_equ_override_value
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, tx_equ_override_value, 0x12, 0, 4);

/* tx_equ_override_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmcr, tx_equ_override_cap, 0x13, 2, 1);

/* tx_equ_override_cntl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, tx_equ_override_cntl, 0x13, 0, 2);

/* rs_override_value
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, rs_override_value, 0x16, 0, 4);

/* rs_override_cntl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, rs_override_cntl, 0x17, 0, 2);

/* agc_override_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmcr, agc_override_cap, 0x19, 2, 1);

/* agc_override_cntl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, agc_override_cntl, 0x19, 0, 2);

/* ap_sel_override_value
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, ap_sel_override_value, 0x1a, 0, 4);

/* ap_sel_override_cntrl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, ap_sel_override_cntrl, 0x1b, 0, 2);

/* agc_target_value
 * Access: RW
 */
MLXSW_ITEM16(reg, pmcr, agc_target_value, 0x1c, 0, 16);

/* media_type_override_value
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, media_type_override_value, 0x1e, 0, 2);

/* media_type_override_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmcr, media_type_override_cap, 0x1f, 2, 1);

/* media_type_override_cntl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, media_type_override_cntl, 0x1f, 0, 2);

/* prec_line_rx_override_val
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, prec_line_rx_override_val, 0x20, 1, 1);

/* prec_line_tx_override_val
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, prec_line_tx_override_val, 0x20, 0, 1);

/* prec_line_cap
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, prec_line_cap, 0x21, 7, 1);

/* prec_line_rx_override_cntl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, prec_line_rx_override_cntl, 0x21, 2, 2);

/* prec_line_tx_override_cntl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, prec_line_tx_override_cntl, 0x21, 0, 2);

/* prec_host_rx_override_val
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, prec_host_rx_override_val, 0x22, 1, 1);

/* prec_host_tx_override_val
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, prec_host_tx_override_val, 0x22, 0, 1);

/* prec_host_cap
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, prec_host_cap, 0x23, 7, 1);

/* prec_host_rx_override_cntl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, prec_host_rx_override_cntl, 0x23, 2, 2);

/* prec_host_tx_override_cntl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, prec_host_tx_override_cntl, 0x23, 0, 2);

/* rx_amp_agc_swing_override_val
 * Access: RW
 */
MLXSW_ITEM16(reg, pmcr, rx_amp_agc_swing_override_val, 0x24, 4, 12);

/* rx_amp_agc_swing_override_cap
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, rx_amp_agc_swing_override_cap, 0x27, 2, 1);

/* rx_amp_agc_swing_override_cntl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, rx_amp_agc_swing_override_cntl, 0x27, 0, 2);

/* tx_ctle_eq_override_val_preset
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, tx_ctle_eq_override_val_preset, 0x28, 4, 4);

/* ctle_uphy_peaking_en
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, ctle_uphy_peaking_en, 0x28, 3, 1);

/* ctle_atten_ctrl
 * Access: RW
 */
MLXSW_ITEM16(reg, pmcr, ctle_atten_ctrl, 0x28, 7, 4);

/* ctle_lfgain_ctr
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, ctle_lfgain_ctr, 0x29, 5, 2);

/* ctle_peak_ctrl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, ctle_peak_ctrl, 0x29, 4, 1);

/* ctle_gaincoarse_val
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, ctle_gaincoarse_val, 0x29, 2, 2);

/* ctle_gainfine_val
 * Access: RW
 */
MLXSW_ITEM32(reg, pmcr, ctle_gainfine_val, 0x28, 11, 7);

/* tx_ctle_eq_override_cap
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, tx_ctle_eq_override_cap, 0x2b, 2, 1);

/* tx_ctle_eq_override_cntl
 * Access: RW
 */
MLXSW_ITEM8(reg, pmcr, tx_ctle_eq_override_cntl, 0x2b, 0, 2);

/* pmtps
 * -----
 *
 */
/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtps, slot_index, 0x0, 0, 4);

/* module
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtps, module, 0x1, 0, 8);

/* module_type_admin
 * Access: RW
 */
MLXSW_ITEM16(reg, pmtps, module_type_admin, 0xa, 0, 16);

/* module_type_connected
 * Access: RO
 */
MLXSW_ITEM16(reg, pmtps, module_type_connected, 0xe, 0, 16);

/* eth_module_c2m
 * Access: RO
 */
MLXSW_ITEM32(reg, pmtps, eth_module_c2m, 0x10, 0, 32);

/* ib_width_module_c2m
 * Access: RO
 */
MLXSW_ITEM16(reg, pmtps, ib_width_module_c2m, 0x14, 0, 16);

/* ib_speed_module_c2m
 * Access: RO
 */
MLXSW_ITEM16(reg, pmtps, ib_speed_module_c2m, 0x16, 0, 16);

/* pmpt
 * ----
 *
 */
/* e
 * Access: RW
 */
MLXSW_ITEM8(reg, pmpt, e, 0x0, 7, 1);

/* ls
 * Access: RO
 */
MLXSW_ITEM8(reg, pmpt, ls, 0x0, 6, 1);

/* le
 * Access: OP
 */
MLXSW_ITEM8(reg, pmpt, le, 0x0, 5, 1);

/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmpt, slot_index, 0x0, 0, 4);

/* module
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmpt, module, 0x1, 0, 8);

/* swap_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmpt, swap_cap, 0x2, 6, 1);

/* invt_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmpt, invt_cap, 0x2, 5, 1);

/* host_media
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmpt, host_media, 0x2, 4, 1);

/* lane_mask
 * Access: INDEX
 */
MLXSW_ITEM16(reg, pmpt, lane_mask, 0x2, 4, 8);

/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, pmpt, status, 0x3, 0, 4);

/* prbs_modes_cap
 * Access: RO
 */
MLXSW_ITEM32(reg, pmpt, prbs_modes_cap, 0x4, 0, 32);

/* prbs_mode_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, pmpt, prbs_mode_admin, 0x8, 0, 8);

/* swap_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, pmpt, swap_admin, 0xa, 1, 1);

/* invt_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, pmpt, invt_admin, 0xa, 0, 1);

/* ch_ge
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmpt, ch_ge, 0xb, 4, 2);

/* modulation
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmpt, modulation, 0xb, 0, 4);

/* lane_rate_cap
 * Access: RO
 */
MLXSW_ITEM16(reg, pmpt, lane_rate_cap, 0xc, 0, 16);

/* lane_rate_admin
 * Access: RW
 */
MLXSW_ITEM16(reg, pmpt, lane_rate_admin, 0x10, 0, 16);

/* pmpd
 * ----
 *
 */
/* snr_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmpd, snr_cap, 0x0, 7, 1);

/* ber_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmpd, ber_cap, 0x0, 6, 1);

/* errors_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmpd, errors_cap, 0x0, 5, 1);

/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmpd, slot_index, 0x0, 0, 4);

/* module
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmpd, module, 0x1, 0, 8);

/* host_media
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmpd, host_media, 0x2, 4, 1);

/* lane
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmpd, lane, 0x2, 0, 4);

/* cl
 * Access: OP
 */
MLXSW_ITEM8(reg, pmpd, cl, 0x3, 4, 1);

/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, pmpd, status, 0x3, 0, 4);

/* prbs_bits_high
 * Access: RO
 */
MLXSW_ITEM32(reg, pmpd, prbs_bits_high, 0x4, 0, 32);

/* prbs_bits_low
 * Access: RO
 */
MLXSW_ITEM32(reg, pmpd, prbs_bits_low, 0x8, 0, 32);

/* prbs_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, pmpd, prbs_errors_high, 0xc, 0, 32);

/* prbs_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, pmpd, prbs_errors_low, 0x10, 0, 32);

/* measured_snr
 * Access: RO
 */
MLXSW_ITEM16(reg, pmpd, measured_snr, 0x14, 0, 16);

/* ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, pmpd, ber_magnitude, 0x1a, 0, 8);

/* ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, pmpd, ber_coef, 0x1b, 0, 4);

/* pmtm
 * ----
 *
 */
/* media_width_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pmtm, media_width_cap, 0x0, 7, 1);

/* slot_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtm, slot_index, 0x0, 0, 4);

/* module
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtm, module, 0x1, 0, 8);

/* module_media_width
 * Access: RO
 */
MLXSW_ITEM8(reg, pmtm, module_media_width, 0x5, 0, 5);

/* module_width
 * Access: RW
 */
MLXSW_ITEM8(reg, pmtm, module_width, 0x6, 0, 5);

/* module_type
 * Access: RW
 */
MLXSW_ITEM8(reg, pmtm, module_type, 0x7, 0, 5);

/* pmlse
 * -----
 *
 */
/* module_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmlse, module_index, 0x0, 0, 7);

/* laser_mask
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlse, laser_mask, 0x3, 0, 8);

/* laser0_operation
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlse, laser0_operation, 0x4, 5, 3);

/* laser1_operation
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlse, laser1_operation, 0x4, 2, 3);

/* laser2_operation
 * Access: RO
 */
MLXSW_ITEM16(reg, pmlse, laser2_operation, 0x4, 7, 3);

/* laser3_operation
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlse, laser3_operation, 0x5, 4, 3);

/* laser4_operation
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlse, laser4_operation, 0x5, 1, 3);

/* laser5_operation
 * Access: RO
 */
MLXSW_ITEM32(reg, pmlse, laser5_operation, 0x4, 14, 3);

/* laser6_operation
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlse, laser6_operation, 0x6, 3, 3);

/* laser7_operation
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlse, laser7_operation, 0x6, 0, 3);

/* operation_bandwidth
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlse, operation_bandwidth, 0x7, 0, 4);

/* get_operation_status_laser0
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlse, get_operation_status_laser0, 0x8, 4, 4);

/* get_operation_status_laser1
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlse, get_operation_status_laser1, 0x8, 0, 4);

/* get_operation_status_laser2
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlse, get_operation_status_laser2, 0x9, 4, 4);

/* get_operation_status_laser3
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlse, get_operation_status_laser3, 0x9, 0, 4);

/* get_operation_status_laser4
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlse, get_operation_status_laser4, 0xa, 4, 4);

/* get_operation_status_laser5
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlse, get_operation_status_laser5, 0xa, 0, 4);

/* get_operation_status_laser6
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlse, get_operation_status_laser6, 0xb, 4, 4);

/* get_operation_status_laser7
 * Access: RO
 */
MLXSW_ITEM8(reg, pmlse, get_operation_status_laser7, 0xb, 0, 4);

/* pplr
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pplr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pplr, lp_msb, 0x2, 4, 2);

/* linkup_flow
 * Access: OP
 */
MLXSW_ITEM8(reg, pplr, linkup_flow, 0x2, 2, 2);

/* apply_im
 * Access: OP
 */
MLXSW_ITEM8(reg, pplr, apply_im, 0x2, 1, 1);

/* op_mod
 * Access: OP
 */
MLXSW_ITEM8(reg, pplr, op_mod, 0x2, 0, 1);

/* port_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pplr, port_type, 0x3, 4, 4);

/* plane_ind
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pplr, plane_ind, 0x3, 0, 4);

/* lb_cap_mode_idx
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pplr, lb_cap_mode_idx, 0x4, 6, 1);

/* lb_cap
 * Access: RO
 */
MLXSW_ITEM16(reg, pplr, lb_cap, 0x4, 0, 12);

/* lb_en
 * Access: RW
 */
MLXSW_ITEM16(reg, pplr, lb_en, 0x6, 0, 12);

/* sltp
 * ----
 *
 */
/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sltp, lp_msb, 0x0, 6, 2);

/* lane_broadcast
 * Access: WO
 */
MLXSW_ITEM8(reg, sltp, lane_broadcast, 0x0, 5, 1);

/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, sltp, status, 0x0, 4, 1);

/* version
 * Access: RO
 */
MLXSW_ITEM8(reg, sltp, version, 0x0, 0, 4);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sltp, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sltp, pnat, 0x2, 6, 2);

/* tx_policy
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, tx_policy, 0x2, 5, 1);

/* conf_mod
 * Access: OP
 */
MLXSW_ITEM8(reg, sltp, conf_mod, 0x2, 4, 1);

/* lane
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sltp, lane, 0x2, 0, 4);

/* lane_speed
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, lane_speed, 0x3, 4, 4);

/* port_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sltp, port_type, 0x3, 1, 3);

/* c_db
 * Access: OP
 */
MLXSW_ITEM8(reg, sltp, c_db, 0x3, 0, 1);

/* sltp_28nm_40nm_polarity
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_28nm_40nm_polarity, 0x8, 0, 1);

/* sltp_28nm_40nm_ob_tap0
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_28nm_40nm_ob_tap0, 0x9, 0, 8);

/* sltp_28nm_40nm_ob_tap1
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_28nm_40nm_ob_tap1, 0xa, 0, 8);

/* sltp_28nm_40nm_ob_tap2
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_28nm_40nm_ob_tap2, 0xb, 0, 8);

/* sltp_28nm_40nm_ob_leva
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_28nm_40nm_ob_leva, 0xd, 4, 4);

/* sltp_28nm_40nm_ob_preemp_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_28nm_40nm_ob_preemp_mode, 0xd, 0, 4);

/* sltp_28nm_40nm_ob_reg
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_28nm_40nm_ob_reg, 0xe, 0, 8);

/* sltp_28nm_40nm_ob_bias
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_28nm_40nm_ob_bias, 0xf, 0, 8);

/* sltp_28nm_40nm_ob_norm
 * Access: RO
 */
MLXSW_ITEM8(reg, sltp, sltp_28nm_40nm_ob_norm, 0x10, 7, 1);

/* sltp_28nm_40nm_ob_bad_stat
 * Access: RO
 */
MLXSW_ITEM8(reg, sltp, sltp_28nm_40nm_ob_bad_stat, 0x10, 5, 2);

/* sltp_16nm_pre_2_tap
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_16nm_pre_2_tap, 0x8, 0, 8);

/* sltp_16nm_pre_tap
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_16nm_pre_tap, 0x9, 0, 8);

/* sltp_16nm_main_tap
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_16nm_main_tap, 0xa, 0, 8);

/* sltp_16nm_post_tap
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_16nm_post_tap, 0xb, 0, 8);

/* sltp_16nm_ob_m2lp
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_16nm_ob_m2lp, 0xc, 1, 7);

/* sltp_16nm_ob_amp
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_16nm_ob_amp, 0xe, 0, 7);

/* sltp_16nm_ob_alev_out
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_16nm_ob_alev_out, 0xf, 0, 5);

/* sltp_16nm_ob_bad_stat
 * Access: RO
 */
MLXSW_ITEM16(reg, sltp, sltp_16nm_ob_bad_stat, 0x10, 0, 16);

/* sltp_16nm_obplev
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_16nm_obplev, 0x14, 0, 8);

/* sltp_16nm_obnlev
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_16nm_obnlev, 0x15, 0, 8);

/* sltp_16nm_regn_bfm1p
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_16nm_regn_bfm1p, 0x16, 0, 8);

/* sltp_16nm_regp_bfm1n
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_16nm_regp_bfm1n, 0x17, 0, 8);

/* main_tap
 * Access: RW
 */
MLXSW_ITEM16(reg, sltp, main_tap, 0x4, 0, 16);

/* pre_tap
 * Access: RW
 */
MLXSW_ITEM16(reg, sltp, pre_tap, 0x6, 0, 16);

/* post_tap
 * Access: RW
 */
MLXSW_ITEM16(reg, sltp, post_tap, 0xa, 0, 16);

/* inner_eye
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, sltp, inner_eye, 0x10, 0, 16, 2, 0, 2, 0);

/* tx_swing
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, tx_swing, 0x13, 0, 4);

/* sltp_7nm_vs_peer_db
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_7nm_vs_peer_db, 0x4, 6, 2);

/* sltp_7nm_drv_amp
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_7nm_drv_amp, 0x4, 0, 6);

/* sltp_7nm_ob_bad_stat
 * Access: RO
 */
MLXSW_ITEM8(reg, sltp, sltp_7nm_ob_bad_stat, 0x5, 0, 8);

/* sltp_7nm_fir_pre3
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_7nm_fir_pre3, 0x6, 0, 8);

/* sltp_7nm_fir_pre2
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_7nm_fir_pre2, 0x7, 0, 8);

/* sltp_7nm_fir_pre1
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_7nm_fir_pre1, 0x8, 0, 8);

/* sltp_7nm_fir_main
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_7nm_fir_main, 0x9, 0, 8);

/* sltp_7nm_fir_post1
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_7nm_fir_post1, 0xa, 0, 8);

/* sltp_7nm_ph_pol_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_7nm_ph_pol_admin, 0xb, 0, 2);

/* sltp_lut_7nm_vs_peer_db
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_vs_peer_db, 0x4, 6, 2);

/* sltp_lut_7nm_drv_amp
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_drv_amp, 0x4, 0, 6);

/* sltp_lut_7nm_ob_bad_stat
 * Access: RO
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_ob_bad_stat, 0x5, 0, 8);

/* sltp_lut_7nm_fir_pre3
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_fir_pre3, 0x6, 0, 8);

/* sltp_lut_7nm_fir_pre2
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_fir_pre2, 0x7, 0, 8);

/* sltp_lut_7nm_lut3
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut3, 0xc, 0, 8);

/* sltp_lut_7nm_lut2
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut2, 0xd, 0, 8);

/* sltp_lut_7nm_lut1
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut1, 0xe, 0, 8);

/* sltp_lut_7nm_lut0
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut0, 0xf, 0, 8);

/* sltp_lut_7nm_lut7
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut7, 0x10, 0, 8);

/* sltp_lut_7nm_lut6
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut6, 0x11, 0, 8);

/* sltp_lut_7nm_lut5
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut5, 0x12, 0, 8);

/* sltp_lut_7nm_lut4
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut4, 0x13, 0, 8);

/* sltp_lut_7nm_lut11
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut11, 0x14, 0, 8);

/* sltp_lut_7nm_lut10
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut10, 0x15, 0, 8);

/* sltp_lut_7nm_lut9
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut9, 0x16, 0, 8);

/* sltp_lut_7nm_lut8
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut8, 0x17, 0, 8);

/* sltp_lut_7nm_lut15
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut15, 0x18, 0, 8);

/* sltp_lut_7nm_lut14
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut14, 0x19, 0, 8);

/* sltp_lut_7nm_lut13
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut13, 0x1a, 0, 8);

/* sltp_lut_7nm_lut12
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut12, 0x1b, 0, 8);

/* sltp_lut_7nm_lut19
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut19, 0x1c, 0, 8);

/* sltp_lut_7nm_lut18
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut18, 0x1d, 0, 8);

/* sltp_lut_7nm_lut17
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut17, 0x1e, 0, 8);

/* sltp_lut_7nm_lut16
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut16, 0x1f, 0, 8);

/* sltp_lut_7nm_lut23
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut23, 0x20, 0, 8);

/* sltp_lut_7nm_lut22
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut22, 0x21, 0, 8);

/* sltp_lut_7nm_lut21
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut21, 0x22, 0, 8);

/* sltp_lut_7nm_lut20
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut20, 0x23, 0, 8);

/* sltp_lut_7nm_lut27
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut27, 0x24, 0, 8);

/* sltp_lut_7nm_lut26
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut26, 0x25, 0, 8);

/* sltp_lut_7nm_lut25
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut25, 0x26, 0, 8);

/* sltp_lut_7nm_lut24
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut24, 0x27, 0, 8);

/* sltp_lut_7nm_lut31
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut31, 0x28, 0, 8);

/* sltp_lut_7nm_lut30
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut30, 0x29, 0, 8);

/* sltp_lut_7nm_lut29
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut29, 0x2a, 0, 8);

/* sltp_lut_7nm_lut28
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut28, 0x2b, 0, 8);

/* sltp_lut_7nm_lut35
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut35, 0x2c, 0, 8);

/* sltp_lut_7nm_lut34
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut34, 0x2d, 0, 8);

/* sltp_lut_7nm_lut33
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut33, 0x2e, 0, 8);

/* sltp_lut_7nm_lut32
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut32, 0x2f, 0, 8);

/* sltp_lut_7nm_lut39
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut39, 0x30, 0, 8);

/* sltp_lut_7nm_lut38
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut38, 0x31, 0, 8);

/* sltp_lut_7nm_lut37
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut37, 0x32, 0, 8);

/* sltp_lut_7nm_lut36
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut36, 0x33, 0, 8);

/* sltp_lut_7nm_lut43
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut43, 0x34, 0, 8);

/* sltp_lut_7nm_lut42
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut42, 0x35, 0, 8);

/* sltp_lut_7nm_lut41
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut41, 0x36, 0, 8);

/* sltp_lut_7nm_lut40
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut40, 0x37, 0, 8);

/* sltp_lut_7nm_lut47
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut47, 0x38, 0, 8);

/* sltp_lut_7nm_lut46
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut46, 0x39, 0, 8);

/* sltp_lut_7nm_lut45
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut45, 0x3a, 0, 8);

/* sltp_lut_7nm_lut44
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut44, 0x3b, 0, 8);

/* sltp_lut_7nm_lut51
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut51, 0x3c, 0, 8);

/* sltp_lut_7nm_lut50
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut50, 0x3d, 0, 8);

/* sltp_lut_7nm_lut49
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut49, 0x3e, 0, 8);

/* sltp_lut_7nm_lut48
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut48, 0x3f, 0, 8);

/* sltp_lut_7nm_lut55
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut55, 0x40, 0, 8);

/* sltp_lut_7nm_lut54
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut54, 0x41, 0, 8);

/* sltp_lut_7nm_lut53
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut53, 0x42, 0, 8);

/* sltp_lut_7nm_lut52
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut52, 0x43, 0, 8);

/* sltp_lut_7nm_lut59
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut59, 0x44, 0, 8);

/* sltp_lut_7nm_lut58
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut58, 0x45, 0, 8);

/* sltp_lut_7nm_lut57
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut57, 0x46, 0, 8);

/* sltp_lut_7nm_lut56
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut56, 0x47, 0, 8);

/* sltp_lut_7nm_lut63
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut63, 0x48, 0, 8);

/* sltp_lut_7nm_lut62
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut62, 0x49, 0, 8);

/* sltp_lut_7nm_lut61
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut61, 0x4a, 0, 8);

/* sltp_lut_7nm_lut60
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_lut_7nm_lut60, 0x4b, 0, 8);

/* sltp_5nm_ph_pol_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_5nm_ph_pol_admin, 0x4, 6, 2);

/* sltp_5nm_drv_amp
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_5nm_drv_amp, 0x4, 0, 6);

/* sltp_5nm_ob_bad_stat
 * Access: RO
 */
MLXSW_ITEM16(reg, sltp, sltp_5nm_ob_bad_stat, 0x6, 0, 16);

/* sltp_5nm_tap3
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_5nm_tap3, 0x8, 0, 8);

/* sltp_5nm_tap2
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_5nm_tap2, 0x9, 0, 8);

/* sltp_5nm_tap1
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_5nm_tap1, 0xa, 0, 8);

/* sltp_5nm_tap0
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_5nm_tap0, 0xb, 0, 8);

/* sltp_5nm_tap7
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_5nm_tap7, 0xc, 0, 8);

/* sltp_5nm_tap6
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_5nm_tap6, 0xd, 0, 8);

/* sltp_5nm_tap5
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_5nm_tap5, 0xe, 0, 8);

/* sltp_5nm_tap4
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_5nm_tap4, 0xf, 0, 8);

/* sltp_5nm_tap11
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_5nm_tap11, 0x10, 0, 8);

/* sltp_5nm_tap10
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_5nm_tap10, 0x11, 0, 8);

/* sltp_5nm_tap9
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_5nm_tap9, 0x12, 0, 8);

/* sltp_5nm_tap8
 * Access: RW
 */
MLXSW_ITEM8(reg, sltp, sltp_5nm_tap8, 0x13, 0, 8);

/* ptasv2
 * ------
 *
 */
/* version
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, version, 0x0, 0, 4);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ptasv2, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ptasv2, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ptasv2, lp_msb, 0x2, 4, 2);

/* ib_cfg_delaytimeout_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, ib_cfg_delaytimeout_cap, 0x2, 2, 1);

/* ib_cfg_delaytimeout
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, ib_cfg_delaytimeout, 0x2, 0, 2);

/* xdr_lt_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, xdr_lt_cap, 0x3, 4, 1);

/* ig_dme_mod_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, ig_dme_mod_cap, 0x3, 3, 1);

/* ig_dme_mod_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, ig_dme_mod_admin, 0x3, 2, 1);

/* force_an_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, force_an_cap, 0x3, 1, 1);

/* force_an_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, force_an_admin, 0x3, 0, 1);

/* kr_ext_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, kr_ext_cap, 0x4, 7, 1);

/* kr_ext_req
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, kr_ext_req, 0x4, 5, 2);

/* kr_ext_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, kr_ext_oper, 0x4, 0, 2);

/* lf_squelch_pol_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, lf_squelch_pol_admin, 0x6, 5, 2);

/* lf_squelch_pol_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, lf_squelch_pol_cap, 0x6, 4, 1);

/* ndr_lt_support_default
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, ndr_lt_support_default, 0x6, 2, 1);

/* lt_neg_support
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, lt_neg_support, 0x6, 0, 1);

/* hdr_lt_c2c_en
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, hdr_lt_c2c_en, 0x7, 6, 2);

/* ndr_lt_c2c_en
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, ndr_lt_c2c_en, 0x7, 4, 2);

/* xdr_lt_c2m_en
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, xdr_lt_c2m_en, 0x7, 2, 2);

/* xdr_lt_c2c_en
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, xdr_lt_c2c_en, 0x7, 0, 2);

/* tuning_type_support
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, tuning_type_support, 0x9, 0, 4);

/* tuning_type_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, tuning_type_admin, 0xb, 0, 4);

/* en_larger_el_buf
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, en_larger_el_buf, 0xb, 0, 1);

/* max_recovery_retries_en
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, max_recovery_retries_en, 0x8, 7, 1);

/* module_type_bkv_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, module_type_bkv_cap, 0x8, 6, 1);

/* lt_timers_override_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, lt_timers_override_cap, 0x8, 5, 1);

/* lt_seed_prbs_type
 * Access: RW
 */
MLXSW_ITEM16(reg, ptasv2, lt_seed_prbs_type, 0x8, 7, 2);

/* lt_seed_lane
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, lt_seed_lane, 0x9, 3, 4);

/* module_type_bkv_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, module_type_bkv_admin, 0x9, 1, 2);

/* module_type_bkv_oper
 * Access: RO
 */
MLXSW_ITEM32(reg, ptasv2, module_type_bkv_oper, 0x8, 15, 2);

/* bidi_extended_train_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, bidi_extended_train_admin, 0xa, 3, 3);

/* phy_count_linkup_delay
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, phy_count_linkup_delay, 0xa, 0, 2);

/* max_recovery_retries
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, max_recovery_retries, 0xb, 0, 4);

/* bidi_extended_train_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, bidi_extended_train_cap, 0xc, 1, 4);

/* lt_tuning_support
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, lt_tuning_support, 0xd, 0, 8);

/* bidi_extended_train_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, bidi_extended_train_oper, 0xe, 3, 2);

/* lt_tuning_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, lt_tuning_admin, 0xf, 0, 8);

/* lt_watchdog_timeout
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, lt_watchdog_timeout, 0x11, 0, 8);

/* lt_frame_lock_timeout
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, lt_frame_lock_timeout, 0x12, 0, 8);

/* preset_mask
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, preset_mask, 0x13, 0, 5);

/* lt_seed
 * Access: RW
 */
MLXSW_ITEM32(reg, ptasv2, lt_seed, 0x14, 0, 31);

/* iterations_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, iterations_cap, 0x18, 7, 1);

/* prbs_type_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, prbs_type_cap, 0x18, 6, 1);

/* lt_ext_timeout_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, lt_ext_timeout_cap, 0x18, 5, 1);

/* ber_target_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, ber_target_cap, 0x18, 4, 1);

/* ber_cnt_mlsd_dis_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, ber_cnt_mlsd_dis_cap, 0x18, 3, 1);

/* kr_extra_tune_fine_srch_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, kr_extra_tune_fine_srch_cap, 0x18, 2, 1);

/* lt_ext_neg_type
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, lt_ext_neg_type, 0x19, 0, 2);

/* iter_time_admin
 * Access: RW
 */
MLXSW_ITEM16(reg, ptasv2, iter_time_admin, 0x1c, 6, 10);

/* num_of_iter_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, num_of_iter_admin, 0x1d, 0, 4);

/* kr_extra_tune_fine_srch_en
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, kr_extra_tune_fine_srch_en, 0x1e, 3, 1);

/* ber_cnt_mlsd_dis
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, ber_cnt_mlsd_dis, 0x1e, 2, 1);

/* prbs_type_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, prbs_type_admin, 0x1e, 0, 2);

/* lt_ext_timeout_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, lt_ext_timeout_admin, 0x1f, 0, 8);

/* ber_target_magnitude_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, ber_target_magnitude_admin, 0x22, 0, 8);

/* ber_target_coef_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, ptasv2, ber_target_coef_admin, 0x23, 0, 4);

/* iter_time_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, ptasv2, iter_time_oper, 0x24, 6, 10);

/* num_of_iter_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, num_of_iter_oper, 0x25, 0, 4);

/* prbs_type_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, prbs_type_oper, 0x26, 0, 2);

/* lt_ext_timeout_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, lt_ext_timeout_oper, 0x27, 0, 8);

/* ber_target_magnitude_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, ber_target_magnitude_oper, 0x2a, 0, 8);

/* ber_target_coef_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, ber_target_coef_oper, 0x2b, 0, 4);

/* amplitude_est
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, amplitude_est, 0x2e, 0, 8);

/* restart_times
 * Access: RO
 */
MLXSW_ITEM8(reg, ptasv2, restart_times, 0x2f, 0, 6);

/* slsir
 * -----
 *
 */
/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, status, 0x0, 4, 4);

/* version
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, version, 0x0, 0, 4);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slsir, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slsir, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slsir, lp_msb, 0x2, 4, 2);

/* lane
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slsir, lane, 0x2, 0, 4);

/* port_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, slsir, port_type, 0x3, 4, 4);

/* slsir_7nm_nop_rsunf_error
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_nop_rsunf_error, 0x7, 7, 1);

/* slsir_7nm_nop_rsovf_error
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_nop_rsovf_error, 0x7, 6, 1);

/* slsir_7nm_nop_dsunf_error
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_nop_dsunf_error, 0x7, 5, 1);

/* slsir_7nm_nop_dsovf_error
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_nop_dsovf_error, 0x7, 4, 1);

/* slsir_7nm_peq_adc_overload
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_peq_adc_overload, 0x7, 3, 1);

/* slsir_7nm_feq_adc_overload
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_feq_adc_overload, 0x7, 2, 1);

/* slsir_7nm_cdr_error
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_cdr_error, 0x7, 1, 1);

/* slsir_7nm_imem_chksm_error
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_imem_chksm_error, 0x7, 0, 1);

/* slsir_7nm_rx_ugl_state
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_rx_ugl_state, 0x8, 4, 4);

/* slsir_7nm_rx_eom_ugl_state
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_rx_eom_ugl_state, 0x8, 0, 2);

/* slsir_7nm_rx_cal_ugl_state
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_rx_cal_ugl_state, 0x9, 5, 3);

/* slsir_7nm_rx_eq_ugl_state
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_rx_eq_ugl_state, 0x9, 0, 3);

/* slsir_7nm_tx_ugl_state
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_tx_ugl_state, 0xb, 0, 4);

/* slsir_7nm_recovery_retries_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_recovery_retries_cnt, 0xc, 4, 4);

/* slsir_7nm_imem_loading_retries
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_imem_loading_retries, 0xc, 0, 3);

/* slsir_7nm_sd_hits_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_sd_hits_cnt, 0xd, 0, 8);

/* slsir_7nm_sd_iter_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_sd_iter_cnt, 0xe, 0, 8);

/* slsir_7nm_rd_iter_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_rd_iter_cnt, 0xf, 0, 8);

/* slsir_7nm_ae_state
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_ae_state, 0x11, 0, 5);

/* slsir_7nm_rx_init_abort_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_rx_init_abort_cnt, 0x12, 0, 8);

/* slsir_7nm_rx_init_done_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_rx_init_done_cnt, 0x13, 0, 8);

/* slsir_7nm_cdr_abort_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_cdr_abort_cnt, 0x14, 0, 8);

/* slsir_7nm_cdr_done_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_cdr_done_cnt, 0x15, 0, 8);

/* slsir_7nm_cal_abort_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_cal_abort_cnt, 0x16, 0, 8);

/* slsir_7nm_cal_done_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_cal_done_cnt, 0x17, 0, 8);

/* slsir_7nm_eq_abort_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_eq_abort_cnt, 0x18, 0, 8);

/* slsir_7nm_eq_done_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_eq_done_cnt, 0x19, 0, 8);

/* slsir_7nm_eom_abort_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_eom_abort_cnt, 0x1a, 0, 8);

/* slsir_7nm_eom_done_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_eom_done_cnt, 0x1b, 0, 8);

/* slsir_7nm_uphy_rev_subminor
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_uphy_rev_subminor, 0x1c, 0, 4);

/* slsir_7nm_bkv_revision_cln
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_bkv_revision_cln, 0x1d, 0, 8);

/* slsir_7nm_bkv_revision_rx
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_bkv_revision_rx, 0x1e, 0, 8);

/* slsir_7nm_uphy_rev_major
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_uphy_rev_major, 0x1f, 4, 4);

/* slsir_7nm_uphy_rev_minor
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_uphy_rev_minor, 0x1f, 0, 4);

/* slsir_7nm_err_ind_it_3
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_err_ind_it_3, 0x20, 0, 4);

/* slsir_7nm_err_ind_it_2
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_err_ind_it_2, 0x21, 0, 4);

/* slsir_7nm_err_ind_it_1
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_err_ind_it_1, 0x22, 0, 4);

/* slsir_7nm_err_ind_it_0
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_7nm_err_ind_it_0, 0x23, 0, 4);

/* slsir_serdes_gen_7_clear_causes
 * Access: OP
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_clear_causes, 0x4, 7, 1);

/* slsir_serdes_gen_7_clear_counters
 * Access: OP
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_clear_counters, 0x4, 6, 1);

/* slsir_serdes_gen_7_sfec_align_ok
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_sfec_align_ok, 0x6, 2, 1);

/* slsir_serdes_gen_7_sfec_align_fail
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_sfec_align_fail, 0x6, 1, 1);

/* slsir_serdes_gen_7_dfd_trigger
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_dfd_trigger, 0x6, 0, 1);

/* slsir_serdes_gen_7_nop_rsunf_error
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_nop_rsunf_error, 0x7, 7, 1);

/* slsir_serdes_gen_7_nop_rsovf_error
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_nop_rsovf_error, 0x7, 6, 1);

/* slsir_serdes_gen_7_nop_dsunf_error
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_nop_dsunf_error, 0x7, 5, 1);

/* slsir_serdes_gen_7_nop_dsovf_error
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_nop_dsovf_error, 0x7, 4, 1);

/* slsir_serdes_gen_7_peq_adc_overload
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_peq_adc_overload, 0x7, 3, 1);

/* slsir_serdes_gen_7_feq_adc_overload
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_feq_adc_overload, 0x7, 2, 1);

/* slsir_serdes_gen_7_cdr_error
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_cdr_error, 0x7, 1, 1);

/* slsir_serdes_gen_7_imem_chksm_error
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_imem_chksm_error, 0x7, 0, 1);

/* slsir_serdes_gen_7_cdr_error_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_cdr_error_cnt, 0xc, 4, 4);

/* slsir_serdes_gen_7_cal_error_cnt
 * Access: RO
 */
MLXSW_ITEM16(reg, slsir, slsir_serdes_gen_7_cal_error_cnt, 0xc, 4, 8);

/* slsir_serdes_gen_7_bist_err_cnt
 * Access: RO
 */
MLXSW_ITEM16(reg, slsir, slsir_serdes_gen_7_bist_err_cnt, 0xe, 0, 16);

/* slsir_serdes_gen_7_cdr_seq_done_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_cdr_seq_done_cnt, 0x10, 0, 8);

/* slsir_serdes_gen_7_cdr_seq_abort_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_cdr_seq_abort_cnt, 0x11, 0, 8);

/* slsir_serdes_gen_7_rx_init_done_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_rx_init_done_cnt, 0x12, 0, 8);

/* slsir_serdes_gen_7_rx_init_abort_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_rx_init_abort_cnt, 0x13, 0, 8);

/* slsir_serdes_gen_7_eq_done_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_eq_done_cnt, 0x14, 0, 8);

/* slsir_serdes_gen_7_eq_abort_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_eq_abort_cnt, 0x15, 0, 8);

/* slsir_serdes_gen_7_cal_done_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_cal_done_cnt, 0x16, 0, 8);

/* slsir_serdes_gen_7_cal_abort_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_cal_abort_cnt, 0x17, 0, 8);

/* slsir_serdes_gen_7_margin_done_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_margin_done_cnt, 0x18, 0, 8);

/* slsir_serdes_gen_7_margin_yield_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_margin_yield_cnt, 0x19, 0, 8);

/* slsir_serdes_gen_7_eom_done_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_eom_done_cnt, 0x1a, 0, 8);

/* slsir_serdes_gen_7_eom_abort_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_eom_abort_cnt, 0x1b, 0, 8);

/* slsir_serdes_gen_7_ae_state
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_ae_state, 0x1c, 2, 5);

/* slsir_serdes_gen_7_bkv_rc_cln
 * Access: RO
 */
MLXSW_ITEM16(reg, slsir, slsir_serdes_gen_7_bkv_rc_cln, 0x1c, 6, 4);

/* slsir_serdes_gen_7_bkv_patch_cln
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_bkv_patch_cln, 0x1d, 2, 4);

/* slsir_serdes_gen_7_bkv_rc_dln
 * Access: RO
 */
MLXSW_ITEM32(reg, slsir, slsir_serdes_gen_7_bkv_rc_dln, 0x1c, 14, 4);

/* slsir_serdes_gen_7_bkv_patch_dln
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_bkv_patch_dln, 0x1e, 2, 4);

/* slsir_serdes_gen_7_pll_ui_shift_cnt
 * Access: RO
 */
MLXSW_ITEM16(reg, slsir, slsir_serdes_gen_7_pll_ui_shift_cnt, 0x1e, 0, 10);

/* slsir_serdes_gen_7_bin_cnt
 * Access: RO
 */
MLXSW_ITEM16(reg, slsir, slsir_serdes_gen_7_bin_cnt, 0x20, 0, 16);

/* slsir_serdes_gen_7_event_cnt
 * Access: RO
 */
MLXSW_ITEM16(reg, slsir, slsir_serdes_gen_7_event_cnt, 0x22, 0, 16);

/* slsir_serdes_gen_7_bkv_major_cln
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_bkv_major_cln, 0x24, 4, 4);

/* slsir_serdes_gen_7_bkv_minor_cln
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_bkv_minor_cln, 0x24, 0, 4);

/* slsir_serdes_gen_7_bkv_major_dln
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_bkv_major_dln, 0x25, 4, 4);

/* slsir_serdes_gen_7_bkv_minor_dln
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_bkv_minor_dln, 0x25, 0, 4);

/* slsir_serdes_gen_7_uphy_ver_major
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_uphy_ver_major, 0x27, 4, 4);

/* slsir_serdes_gen_7_uphy_ver_minor
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_7_uphy_ver_minor, 0x27, 0, 4);

/* slsir_serdes_gen_8_clear_causes
 * Access: OP
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_clear_causes, 0x4, 7, 1);

/* slsir_serdes_gen_8_clear_counters
 * Access: OP
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_clear_counters, 0x4, 6, 1);

/* slsir_serdes_gen_8_sfec_align_ok
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_sfec_align_ok, 0x6, 2, 1);

/* slsir_serdes_gen_8_sfec_align_fail
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_sfec_align_fail, 0x6, 1, 1);

/* slsir_serdes_gen_8_dfd_trigger
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_dfd_trigger, 0x6, 0, 1);

/* slsir_serdes_gen_8_nop_rsunf_error
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_nop_rsunf_error, 0x7, 7, 1);

/* slsir_serdes_gen_8_nop_rsovf_error
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_nop_rsovf_error, 0x7, 6, 1);

/* slsir_serdes_gen_8_nop_dsunf_error
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_nop_dsunf_error, 0x7, 5, 1);

/* slsir_serdes_gen_8_nop_dsovf_error
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_nop_dsovf_error, 0x7, 4, 1);

/* slsir_serdes_gen_8_peq_adc_overload
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_peq_adc_overload, 0x7, 3, 1);

/* slsir_serdes_gen_8_feq_adc_overload
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_feq_adc_overload, 0x7, 2, 1);

/* slsir_serdes_gen_8_cdr_error
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_cdr_error, 0x7, 1, 1);

/* slsir_serdes_gen_8_imem_chksm_error
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_imem_chksm_error, 0x7, 0, 1);

/* slsir_serdes_gen_8_cdr_error_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_cdr_error_cnt, 0xc, 4, 4);

/* slsir_serdes_gen_8_cal_error_cnt
 * Access: RO
 */
MLXSW_ITEM16(reg, slsir, slsir_serdes_gen_8_cal_error_cnt, 0xc, 4, 8);

/* slsir_serdes_gen_8_bist_err_cnt
 * Access: RO
 */
MLXSW_ITEM16(reg, slsir, slsir_serdes_gen_8_bist_err_cnt, 0xe, 0, 16);

/* slsir_serdes_gen_8_cdr_seq_done_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_cdr_seq_done_cnt, 0x10, 0, 8);

/* slsir_serdes_gen_8_cdr_seq_abort_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_cdr_seq_abort_cnt, 0x11, 0, 8);

/* slsir_serdes_gen_8_rx_init_done_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_rx_init_done_cnt, 0x12, 0, 8);

/* slsir_serdes_gen_8_rx_init_abort_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_rx_init_abort_cnt, 0x13, 0, 8);

/* slsir_serdes_gen_8_eq_done_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_eq_done_cnt, 0x14, 0, 8);

/* slsir_serdes_gen_8_eq_abort_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_eq_abort_cnt, 0x15, 0, 8);

/* slsir_serdes_gen_8_cal_done_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_cal_done_cnt, 0x16, 0, 8);

/* slsir_serdes_gen_8_cal_abort_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_cal_abort_cnt, 0x17, 0, 8);

/* slsir_serdes_gen_8_margin_done_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_margin_done_cnt, 0x18, 0, 8);

/* slsir_serdes_gen_8_margin_yield_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_margin_yield_cnt, 0x19, 0, 8);

/* slsir_serdes_gen_8_eom_done_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_eom_done_cnt, 0x1a, 0, 8);

/* slsir_serdes_gen_8_eom_abort_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_eom_abort_cnt, 0x1b, 0, 8);

/* slsir_serdes_gen_8_ae_state
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_ae_state, 0x1c, 2, 5);

/* slsir_serdes_gen_8_bkv_rc_cln
 * Access: RO
 */
MLXSW_ITEM16(reg, slsir, slsir_serdes_gen_8_bkv_rc_cln, 0x1c, 6, 4);

/* slsir_serdes_gen_8_bkv_patch_cln
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_bkv_patch_cln, 0x1d, 2, 4);

/* slsir_serdes_gen_8_bkv_rc_dln
 * Access: RO
 */
MLXSW_ITEM32(reg, slsir, slsir_serdes_gen_8_bkv_rc_dln, 0x1c, 14, 4);

/* slsir_serdes_gen_8_bkv_patch_dln
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_bkv_patch_dln, 0x1e, 2, 4);

/* slsir_serdes_gen_8_pll_ui_shift_cnt
 * Access: RO
 */
MLXSW_ITEM16(reg, slsir, slsir_serdes_gen_8_pll_ui_shift_cnt, 0x1e, 0, 10);

/* slsir_serdes_gen_8_bin_cnt
 * Access: RO
 */
MLXSW_ITEM16(reg, slsir, slsir_serdes_gen_8_bin_cnt, 0x20, 0, 16);

/* slsir_serdes_gen_8_event_cnt
 * Access: RO
 */
MLXSW_ITEM16(reg, slsir, slsir_serdes_gen_8_event_cnt, 0x22, 0, 16);

/* slsir_serdes_gen_8_bkv_major_cln
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_bkv_major_cln, 0x24, 4, 4);

/* slsir_serdes_gen_8_bkv_minor_cln
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_bkv_minor_cln, 0x24, 0, 4);

/* slsir_serdes_gen_8_bkv_major_dln
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_bkv_major_dln, 0x25, 4, 4);

/* slsir_serdes_gen_8_bkv_minor_dln
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_bkv_minor_dln, 0x25, 0, 4);

/* slsir_serdes_gen_8_uphy_ver_major
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_uphy_ver_major, 0x27, 4, 4);

/* slsir_serdes_gen_8_uphy_ver_minor
 * Access: RO
 */
MLXSW_ITEM8(reg, slsir, slsir_serdes_gen_8_uphy_ver_minor, 0x27, 0, 4);

/* pddr
 * ----
 *
 */
/* module_ind_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pddr, module_ind_type, 0x0, 0, 2);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pddr, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pddr, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pddr, lp_msb, 0x2, 4, 2);

/* plane_ind
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pddr, plane_ind, 0x2, 0, 4);

/* port_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pddr, port_type, 0x3, 4, 4);

/* clr
 * Access: OP
 */
MLXSW_ITEM8(reg, pddr, clr, 0x4, 7, 1);

/* module_info_ext
 * Access: OP
 */
MLXSW_ITEM8(reg, pddr, module_info_ext, 0x4, 5, 2);

/* dev
 * Access: OP
 */
MLXSW_ITEM8(reg, pddr, dev, 0x4, 4, 1);

/* page_select
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pddr, page_select, 0x7, 0, 8);

/* proto_active
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, proto_active, 0x9, 4, 4);

/* neg_mode_active
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, neg_mode_active, 0x9, 0, 4);

/* pd_fsm_state
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pd_fsm_state, 0xb, 0, 8);

/* phy_mngr_fsm_state
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, phy_mngr_fsm_state, 0xc, 0, 8);

/* eth_an_fsm_state
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, eth_an_fsm_state, 0xd, 0, 8);

/* ib_phy_fsm_state
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, ib_phy_fsm_state, 0xe, 0, 8);

/* phy_hst_fsm_state
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, phy_hst_fsm_state, 0xf, 0, 8);

/* phy_manager_link_width_enabled
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, phy_manager_link_width_enabled, 0x10, 0, 16);

/* phy_manager_link_proto_enabled
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, phy_manager_link_proto_enabled, 0x12, 0, 16);

/* phy_manager_link_eth_enabled
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, phy_manager_link_eth_enabled, 0x10, 0, 32);

/* phy_manager_link_nvlink_enabled
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, phy_manager_link_nvlink_enabled, 0x10, 0, 32);

/* core_to_phy_link_width_enabled
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, core_to_phy_link_width_enabled, 0x14, 0, 16);

/* core_to_phy_link_proto_enabled
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, core_to_phy_link_proto_enabled, 0x16, 0, 16);

/* core_to_phy_link_eth_enabled
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, core_to_phy_link_eth_enabled, 0x14, 0, 32);

/* core_to_phy_link_nvlink_enabled
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, core_to_phy_link_nvlink_enabled, 0x14, 0, 32);

/* cable_link_width_cap
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, cable_link_width_cap, 0x18, 0, 16);

/* cable_link_speed_cap
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, cable_link_speed_cap, 0x1a, 0, 16);

/* cable_ext_eth_proto_cap
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, cable_ext_eth_proto_cap, 0x18, 0, 32);

/* cable_nvlink_proto_cap
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, cable_nvlink_proto_cap, 0x18, 0, 32);

/* pddr_link_active_ib_link_width_active
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, pddr_link_active_ib_link_width_active, 0x1c, 0, 16);

/* pddr_link_active_ib_link_speed_active
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, pddr_link_active_ib_link_speed_active, 0x1e, 0, 16);

/* pddr_link_active_eth_link_eth_active
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, pddr_link_active_eth_link_eth_active, 0x1c, 0, 32);

/* link_nvlink_active
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, link_nvlink_active, 0x1c, 0, 32);

/* retran_mode_active
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, retran_mode_active, 0x20, 0, 8);

/* retran_mode_request
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, retran_mode_request, 0x21, 0, 8);

/* bidi_pri_or_sec
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, bidi_pri_or_sec, 0x22, 6, 2);

/* loopback_mode
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, loopback_mode, 0x22, 0, 12);

/* fec_mode_active
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, fec_mode_active, 0x24, 0, 16);

/* fec_mode_request
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, fec_mode_request, 0x26, 0, 16);

/* profile_fec_in_use
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, profile_fec_in_use, 0x28, 0, 16);

/* nlpn_fsm_state
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, nlpn_fsm_state, 0x2a, 0, 8);

/* eth_25g_50g_fec_support
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, eth_25g_50g_fec_support, 0x2b, 4, 4);

/* eth_100g_fec_support
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, eth_100g_fec_support, 0x2b, 0, 4);

/* pd_link_ib_enabled_link_width_active
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, pd_link_ib_enabled_link_width_active, 0x2c, 0, 16);

/* pd_link_ib_enabled_link_speed_active
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, pd_link_ib_enabled_link_speed_active, 0x2e, 0, 16);

/* pd_link_eth_enabled_link_eth_active
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, pd_link_eth_enabled_link_eth_active, 0x2c, 0, 32);

/* hst_link_ib_enabled_link_width_active
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, hst_link_ib_enabled_link_width_active, 0x30, 0, 16);

/* hst_link_ib_enabled_link_speed_active
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, hst_link_ib_enabled_link_speed_active, 0x32, 0, 16);

/* hst_link_eth_enabled_link_eth_active
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, hst_link_eth_enabled_link_eth_active, 0x30, 0, 32);

/* eth_an_link_enabled
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, eth_an_link_enabled, 0x34, 0, 32);

/* linkup_retry_iterations
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, linkup_retry_iterations, 0x38, 0, 8);

/* core_to_phy_state
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, core_to_phy_state, 0x3a, 0, 4);

/* psi_fsm_state
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, psi_fsm_state, 0x3b, 0, 4);

/* cable_proto_cap_ext
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, cable_proto_cap_ext, 0x3c, 0, 32);

/* test_mode_fsm_state
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, test_mode_fsm_state, 0x40, 0, 8);

/* group_opcode
 * Access: INDEX
 */
MLXSW_ITEM16(reg, pddr, group_opcode, 0xa, 0, 16);

/* monitor_opcode
 * Access: RW
 */
MLXSW_ITEM16(reg, pddr, monitor_opcode, 0xe, 0, 16);

/* advanced_opcode
 * Access: RW
 */
MLXSW_ITEM16(reg, pddr, advanced_opcode, 0xe, 0, 16);

/* user_feedback_data
 * Access: RW
 */
MLXSW_ITEM16(reg, pddr, user_feedback_data, 0x10, 0, 16);

/* user_feedback_index
 * Access: RW
 */
MLXSW_ITEM16(reg, pddr, user_feedback_index, 0x12, 0, 16);

/* status_message
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, pddr, status_message, 0x14, 236);

/* remote_device_type
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, remote_device_type, 0x8, 0, 8);

/* negotiation_mask
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, negotiation_mask, 0xa, 0, 8);

/* port_notifications
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, port_notifications, 0xb, 0, 8);

/* num_of_negotiation_attempts
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, num_of_negotiation_attempts, 0xc, 0, 16);

/* ib_revision
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, ib_revision, 0xe, 0, 8);

/* lp_ib_revision
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, lp_ib_revision, 0xf, 0, 8);

/* hw_link_phy_state
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, hw_link_phy_state, 0x10, 0, 8);

/* phy_manager_disable_mask
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, phy_manager_disable_mask, 0x10, 0, 24);

/* pcs_phy_state
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, pcs_phy_state, 0x14, 0, 32);

/* lp_proto_enabled
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, lp_proto_enabled, 0x18, 0, 32);

/* lp_fec_mode_support
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, lp_fec_mode_support, 0x1c, 0, 16);

/* lp_fec_mode_request
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, lp_fec_mode_request, 0x1e, 0, 16);

/* ib_last_link_down_reason
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, ib_last_link_down_reason, 0x20, 0, 32);

/* eth_last_link_down_lane
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, pddr, eth_last_link_down_lane, 0x24, 4);

/* speed_deg_db
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, speed_deg_db, 0x2c, 0, 32);

/* degrade_grade_lane0
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, degrade_grade_lane0, 0x30, 0, 24);

/* degrade_grade_lane1
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, degrade_grade_lane1, 0x34, 0, 24);

/* degrade_grade_lane2
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, degrade_grade_lane2, 0x38, 0, 24);

/* degrade_grade_lane3
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, degrade_grade_lane3, 0x3c, 0, 24);

/* num_of_presets_tested_lane4
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, num_of_presets_tested_lane4, 0x40, 0, 5);

/* num_of_presets_tested_lane5
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, num_of_presets_tested_lane5, 0x41, 0, 5);

/* num_of_presets_tested_lane6
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, num_of_presets_tested_lane6, 0x42, 0, 5);

/* num_of_presets_tested_lane7
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, num_of_presets_tested_lane7, 0x43, 0, 5);

/* kr_startup_debug_indications_4
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, kr_startup_debug_indications_4, 0x44, 0, 16);

/* kr_startup_debug_indications_5
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, kr_startup_debug_indications_5, 0x46, 0, 16);

/* kr_startup_debug_indications_6
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, kr_startup_debug_indications_6, 0x48, 0, 16);

/* kr_startup_debug_indications_7
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, kr_startup_debug_indications_7, 0x4a, 0, 16);

/* num_of_presets_tested_lane0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, num_of_presets_tested_lane0, 0x4c, 0, 5);

/* num_of_presets_tested_lane1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, num_of_presets_tested_lane1, 0x4d, 0, 5);

/* num_of_presets_tested_lane2
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, num_of_presets_tested_lane2, 0x4e, 0, 5);

/* num_of_presets_tested_lane3
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, num_of_presets_tested_lane3, 0x4f, 0, 5);

/* kr_startup_fsm_lane
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, pddr, kr_startup_fsm_lane, 0x50, 8);

/* eth_an_debug_indication
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, eth_an_debug_indication, 0x58, 0, 32);

/* cdr_not_locked_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cdr_not_locked_cnt, 0x5c, 5, 3);

/* fall_from_cfg_idle_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, fall_from_cfg_idle_cnt, 0x5c, 2, 3);

/* fall_from_cfg_idle_cdr_cnt
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, fall_from_cfg_idle_cdr_cnt, 0x5c, 7, 3);

/* eth_an_watchdog_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, eth_an_watchdog_cnt, 0x5d, 5, 2);

/* rounds_waited_for_peer_to_end_test
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, rounds_waited_for_peer_to_end_test, 0x5d, 0, 5);

/* ib_phy_fsm_state_trace
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, ib_phy_fsm_state_trace, 0x5e, 0, 16);

/* kr_startup_debug_indications_0
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, kr_startup_debug_indications_0, 0x60, 0, 16);

/* kr_startup_debug_indications_1
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, kr_startup_debug_indications_1, 0x62, 0, 16);

/* kr_startup_debug_indications_2
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, kr_startup_debug_indications_2, 0x64, 0, 16);

/* kr_startup_debug_indications_3
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, kr_startup_debug_indications_3, 0x66, 0, 16);

/* tx_tuning_stages_lane
 * Access: RO
 */
MLXSW_ITEM_BIT_ARRAY(reg, pddr, tx_tuning_stages_lane, 0x68, 4, 4);

/* plu_rx_polarity
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, plu_rx_polarity, 0x6c, 0, 8);

/* plu_tx_polarity
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, plu_tx_polarity, 0x6d, 0, 8);

/* plu_rx_pwrup
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, plu_rx_pwrup, 0x6e, 0, 8);

/* plu_tx_pwrup
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, plu_tx_pwrup, 0x6f, 0, 8);

/* signal_detected
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, signal_detected, 0x70, 0, 8);

/* plr_outstanding_cells
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, plr_outstanding_cells, 0x71, 0, 8);

/* plu_rx_speed
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, plu_rx_speed, 0x72, 4, 4);

/* plu_tx_speed
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, plu_tx_speed, 0x72, 0, 4);

/* sd_valid
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, sd_valid, 0x73, 6, 1);

/* ib_cfg_delay_timeout
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, ib_cfg_delay_timeout, 0x73, 4, 2);

/* irisc_status
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, irisc_status, 0x73, 0, 4);

/* stamping_reason
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, stamping_reason, 0x74, 0, 32);

/* kr_frame_lock_tuning_failure_events_count
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, kr_frame_lock_tuning_failure_events_count, 0x78, 0, 32);

/* kr_full_tuning_failure_count
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, kr_full_tuning_failure_count, 0x7c, 0, 32);

/* ib_debug_indication
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, ib_debug_indication, 0x80, 0, 16);

/* pm_debug_indication
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, pm_debug_indication, 0x82, 0, 16);

/* pd_cat_val
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pd_cat_val, 0x84, 7, 1);

/* pd_catastrophic_enum
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pd_catastrophic_enum, 0x84, 0, 7);

/* hst_cat_val
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, hst_cat_val, 0x85, 7, 1);

/* hst_catastrophic_enum
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, hst_catastrophic_enum, 0x85, 0, 7);

/* an_cat_val
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, an_cat_val, 0x86, 7, 1);

/* an_catastrophic_enum
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, an_catastrophic_enum, 0x86, 0, 7);

/* pm_cat_val
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pm_cat_val, 0x87, 7, 1);

/* pm_catastrophic_enum
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pm_catastrophic_enum, 0x87, 0, 7);

/* pd_debug_indication
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, pd_debug_indication, 0x88, 0, 32);

/* psi_collision1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, psi_collision1, 0x8c, 3, 5);

/* psi_collision2
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, psi_collision2, 0x8c, 6, 5);

/* hst_mismatch_reason
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, hst_mismatch_reason, 0x8d, 0, 2);

/* fp_signal_detect_count
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, fp_signal_detect_count, 0x8e, 0, 6);

/* pd_count
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pd_count, 0x8f, 0, 6);

/* nlpn_debug_ind_mask
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, nlpn_debug_ind_mask, 0x93, 0, 8);

/* phy2mod_speed_req
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, phy2mod_speed_req, 0x94, 0, 32);

/* eeprom_prsnt
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, eeprom_prsnt, 0x98, 7, 1);

/* no_dme_mod
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, no_dme_mod, 0x98, 6, 1);

/* one_pll_mod
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, one_pll_mod, 0x98, 5, 1);

/* tx_db_xdr_pointer_valid
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_db_xdr_pointer_valid, 0x98, 4, 1);

/* tx_db_ndr_pointer_valid
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_db_ndr_pointer_valid, 0x98, 3, 1);

/* tx_db_xdr_pointer
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_db_xdr_pointer, 0x99, 4, 4);

/* tx_db_ndr_pointer
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_db_ndr_pointer, 0x99, 0, 4);

/* phy2mod_ack_lanes
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, phy2mod_ack_lanes, 0x9a, 0, 8);

/* phy2mod_deactivate_lanes
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, phy2mod_deactivate_lanes, 0x9b, 0, 8);

/* port_xmit_pkts_inc_s1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, port_xmit_pkts_inc_s1, 0x9c, 4, 4);

/* port_xmit_pkts_inc_s0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, port_xmit_pkts_inc_s0, 0x9c, 0, 4);

/* cnt_rx_frame_received_ok_s1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cnt_rx_frame_received_ok_s1, 0x9d, 4, 4);

/* cnt_rx_frame_received_ok_s0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cnt_rx_frame_received_ok_s0, 0x9d, 0, 4);

/* macsec_en_plt1_s1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, macsec_en_plt1_s1, 0x9e, 7, 1);

/* macsec_en_plt1_s0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, macsec_en_plt1_s0, 0x9e, 6, 1);

/* macsec_en_plt0_s1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, macsec_en_plt0_s1, 0x9e, 5, 1);

/* macsec_en_plt0_s0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, macsec_en_plt0_s0, 0x9e, 4, 1);

/* reconciliation_mux_plt1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, reconciliation_mux_plt1, 0x9e, 2, 2);

/* reconciliation_mux_plt0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, reconciliation_mux_plt0, 0x9e, 0, 2);

/* tx_bypass_mux_plt1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_bypass_mux_plt1, 0x9f, 6, 2);

/* tx_bypass_mux_plt0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_bypass_mux_plt0, 0x9f, 4, 2);

/* rx_bypass_mux_plt1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, rx_bypass_mux_plt1, 0x9f, 2, 2);

/* rx_bypass_mux_plt0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, rx_bypass_mux_plt0, 0x9f, 0, 2);

/* plr_rtt_hdr_threshold
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, plr_rtt_hdr_threshold, 0xa0, 0, 16);

/* plr_rtt_ndr_4x_kp4_threshold
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, plr_rtt_ndr_4x_kp4_threshold, 0xa2, 0, 16);

/* plr_rtt_xdr_1x_kp4_threshold
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, plr_rtt_xdr_1x_kp4_threshold, 0xa4, 0, 16);

/* plr_rtt_ndr_2x_kp4_threshold
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, plr_rtt_ndr_2x_kp4_threshold, 0xa6, 0, 16);

/* plr_rtt_ndr_4x_ell_threshold
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, plr_rtt_ndr_4x_ell_threshold, 0xa8, 0, 16);

/* plr_rtt_ndr_2x_ell_threshold
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, plr_rtt_ndr_2x_ell_threshold, 0xaa, 0, 16);

/* plr_rtt_xdr_2x_kp4_threshold
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, plr_rtt_xdr_2x_kp4_threshold, 0xac, 0, 16);

/* plr_rtt_xdr_1x_ell_threshold
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, plr_rtt_xdr_1x_ell_threshold, 0xae, 0, 16);

/* recovery_retry_count
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, recovery_retry_count, 0xb4, 0, 16);

/* max_recovery_count
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, max_recovery_count, 0xb6, 0, 16);

/* cable_technology
 * Access: RW
 */
MLXSW_ITEM8(reg, pddr, cable_technology, 0x8, 0, 8);

/* cable_breakout
 * Access: RW
 */
MLXSW_ITEM8(reg, pddr, cable_breakout, 0x9, 0, 8);

/* ext_ethernet_compliance_code
 * Access: RW
 */
MLXSW_ITEM8(reg, pddr, ext_ethernet_compliance_code, 0xa, 0, 8);

/* ethernet_compliance_code
 * Access: RW
 */
MLXSW_ITEM8(reg, pddr, ethernet_compliance_code, 0xb, 0, 8);

/* cable_type
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cable_type, 0xc, 4, 4);

/* cable_vendor
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cable_vendor, 0xc, 0, 4);

/* cable_length
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cable_length, 0xd, 0, 8);

/* cable_identifier
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cable_identifier, 0xe, 0, 8);

/* cable_power_class
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cable_power_class, 0xf, 0, 8);

/* max_power
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, max_power, 0x10, 0, 8);

/* cable_rx_amp
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cable_rx_amp, 0x11, 0, 8);

/* cable_rx_emphasis
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cable_rx_emphasis, 0x12, 0, 8);

/* cable_tx_equalization
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cable_tx_equalization, 0x13, 0, 8);

/* cable_attenuation_25g
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cable_attenuation_25g, 0x14, 0, 8);

/* cable_attenuation_12g
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cable_attenuation_12g, 0x15, 0, 8);

/* cable_attenuation_7g
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cable_attenuation_7g, 0x16, 0, 8);

/* cable_attenuation_5g
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cable_attenuation_5g, 0x17, 0, 8);

/* cable_rx_post_emphasis
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cable_rx_post_emphasis, 0x18, 0, 8);

/* rx_cdr_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, rx_cdr_cap, 0x19, 4, 4);

/* tx_cdr_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_cdr_cap, 0x19, 0, 4);

/* rx_cdr_state
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, rx_cdr_state, 0x1a, 0, 8);

/* tx_cdr_state
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_cdr_state, 0x1b, 0, 8);

/* vendor_name
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, pddr, vendor_name, 0x1c, 16);

/* vendor_pn
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, pddr, vendor_pn, 0x2c, 16);

/* vendor_rev
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, vendor_rev, 0x3c, 0, 32);

/* fw_version
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, fw_version, 0x40, 0, 32);

/* vendor_sn
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, pddr, vendor_sn, 0x44, 16);

/* temperature
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, temperature, 0x54, 0, 16);

/* voltage
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, voltage, 0x56, 0, 16);

/* rx_power_lane0
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, rx_power_lane0, 0x58, 0, 16);

/* rx_power_lane1
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, rx_power_lane1, 0x5a, 0, 16);

/* rx_power_lane2
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, rx_power_lane2, 0x5c, 0, 16);

/* rx_power_lane3
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, rx_power_lane3, 0x5e, 0, 16);

/* rx_power_lane4
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, rx_power_lane4, 0x60, 0, 16);

/* rx_power_lane5
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, rx_power_lane5, 0x62, 0, 16);

/* rx_power_lane6
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, rx_power_lane6, 0x64, 0, 16);

/* rx_power_lane7
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, rx_power_lane7, 0x66, 0, 16);

/* tx_power_lane0
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_power_lane0, 0x68, 0, 16);

/* tx_power_lane1
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_power_lane1, 0x6a, 0, 16);

/* tx_power_lane2
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_power_lane2, 0x6c, 0, 16);

/* tx_power_lane3
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_power_lane3, 0x6e, 0, 16);

/* tx_power_lane4
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_power_lane4, 0x70, 0, 16);

/* tx_power_lane5
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_power_lane5, 0x72, 0, 16);

/* tx_power_lane6
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_power_lane6, 0x74, 0, 16);

/* tx_power_lane7
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_power_lane7, 0x76, 0, 16);

/* tx_bias_lane0
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_bias_lane0, 0x78, 0, 16);

/* tx_bias_lane1
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_bias_lane1, 0x7a, 0, 16);

/* tx_bias_lane2
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_bias_lane2, 0x7c, 0, 16);

/* tx_bias_lane3
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_bias_lane3, 0x7e, 0, 16);

/* tx_bias_lane4
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_bias_lane4, 0x80, 0, 16);

/* tx_bias_lane5
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_bias_lane5, 0x82, 0, 16);

/* tx_bias_lane6
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_bias_lane6, 0x84, 0, 16);

/* tx_bias_lane7
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_bias_lane7, 0x86, 0, 16);

/* temperature_high_th
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, temperature_high_th, 0x88, 0, 16);

/* temperature_low_th
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, temperature_low_th, 0x8a, 0, 16);

/* voltage_high_th
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, voltage_high_th, 0x8c, 0, 16);

/* voltage_low_th
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, voltage_low_th, 0x8e, 0, 16);

/* rx_power_high_th
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, rx_power_high_th, 0x90, 0, 16);

/* rx_power_low_th
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, rx_power_low_th, 0x92, 0, 16);

/* tx_power_high_th
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_power_high_th, 0x94, 0, 16);

/* tx_power_low_th
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_power_low_th, 0x96, 0, 16);

/* tx_bias_high_th
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_bias_high_th, 0x98, 0, 16);

/* tx_bias_low_th
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, tx_bias_low_th, 0x9a, 0, 16);

/* module_st
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, module_st, 0x9c, 5, 3);

/* rx_power_type
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, rx_power_type, 0x9c, 4, 1);

/* did_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, did_cap, 0x9c, 3, 1);

/* rx_output_valid_cap
 * Access: RW
 */
MLXSW_ITEM8(reg, pddr, rx_output_valid_cap, 0x9c, 2, 1);

/* smf_length
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, smf_length, 0x9c, 0, 10);

/* wavelength
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, wavelength, 0x9e, 0, 16);

/* active_set_host_compliance_code
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, active_set_host_compliance_code, 0xa0, 0, 8);

/* active_set_media_compliance_code
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, active_set_media_compliance_code, 0xa1, 0, 8);

/* tx_bias_scaling_factor
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_bias_scaling_factor, 0xa2, 2, 2);

/* ib_compliance_code
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, ib_compliance_code, 0xa2, 0, 10);

/* nbr250
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, nbr250, 0xa4, 0, 8);

/* nbr100
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, nbr100, 0xa5, 0, 8);

/* monitor_cap_mask
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, monitor_cap_mask, 0xa6, 0, 8);

/* ib_width
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, ib_width, 0xa7, 0, 6);

/* dp_st_lane
 * Access: RO
 */
MLXSW_ITEM_BIT_ARRAY(reg, pddr, dp_st_lane, 0xa8, 4, 4);

/* length_om2
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, length_om2, 0xac, 0, 8);

/* length_om3
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, length_om3, 0xad, 0, 8);

/* length_om4
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, length_om4, 0xae, 0, 8);

/* length_om5
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, length_om5, 0xaf, 0, 8);

/* length_om1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, length_om1, 0xb0, 0, 8);

/* wavelength_tolerance
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, wavelength_tolerance, 0xb0, 8, 16);

/* memory_map_rev
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, memory_map_rev, 0xb3, 0, 8);

/* memory_map_compliance
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, memory_map_compliance, 0xb4, 0, 32);

/* date_code
 * Access: RO
 */
MLXSW_ITEM64(reg, pddr, date_code, 0xb8, 0, 64);

/* connector_type
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, connector_type, 0xc0, 0, 8);

/* vendor_oui
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, vendor_oui, 0xc0, 0, 24);

/* tx_input_freq_sync
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_input_freq_sync, 0xc5, 0, 2);

/* cable_attenuation_53g
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cable_attenuation_53g, 0xc6, 0, 8);

/* rx_output_valid
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, rx_output_valid, 0xc7, 0, 8);

/* max_fiber_length
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, max_fiber_length, 0xc8, 0, 16);

/* cdr_vendor
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cdr_vendor, 0xcb, 4, 4);

/* error_code
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, error_code, 0xcb, 0, 4);

/* laser_enabled
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser_enabled, 0xcc, 7, 1);

/* laser2_enabled
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser2_enabled, 0xcc, 6, 1);

/* laser_status
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser_status, 0xcc, 4, 2);

/* laser2_status
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser2_status, 0xcc, 2, 2);

/* laser_restriction
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser_restriction, 0xcc, 1, 1);

/* laser2_restriction
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser2_restriction, 0xcc, 0, 1);

/* els_oper_state
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, els_oper_state, 0xcd, 4, 4);

/* els2_oper_state
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, els2_oper_state, 0xcd, 0, 4);

/* els_laser_fault_state
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, els_laser_fault_state, 0xce, 5, 3);

/* els_laser2_fault_state
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, els_laser2_fault_state, 0xce, 2, 3);

/* module_production_test_revision_lsb
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, module_production_test_revision_lsb, 0xd0, 0, 32);

/* module_production_test_revision_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, module_production_test_revision_msb, 0xd4, 0, 8);

/* pddr_port_events_page_layout_invalid_fsm_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_invalid_fsm_sv, 0x8, 7, 1);

/* pddr_port_events_page_layout_invalid_fsm_ps
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, pddr_port_events_page_layout_invalid_fsm_ps, 0xa, 0, 16);

/* pddr_port_events_page_layout_merger_tx_eb_underrun
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_merger_tx_eb_underrun, 0xc, 7, 1);

/* pddr_port_events_page_layout_bubble_in_pkt
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_bubble_in_pkt, 0xc, 6, 1);

/* pddr_port_events_page_layout_kr_false_ready_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_kr_false_ready_sv, 0xd, 1, 1);

/* pddr_port_events_page_layout_kr_false_ready
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_kr_false_ready, 0xd, 0, 1);

/* pddr_port_events_page_layout_mod_req_nack_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_mod_req_nack_sv, 0xe, 3, 1);

/* pddr_port_events_page_layout_mod_req_nack
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_mod_req_nack, 0xe, 2, 1);

/* pddr_port_events_page_layout_mod_req_busy_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_mod_req_busy_sv, 0xe, 1, 1);

/* pddr_port_events_page_layout_mod_req_busy
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_mod_req_busy, 0xe, 0, 1);

/* pddr_port_events_page_layout_mod_config_to_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_mod_config_to_sv, 0xf, 7, 1);

/* pddr_port_events_page_layout_mod_config_to
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_mod_config_to, 0xf, 6, 1);

/* pddr_port_events_page_layout_mod_adapt_faild_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_mod_adapt_faild_sv, 0xf, 5, 1);

/* pddr_port_events_page_layout_mod_adapt_faild
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_mod_adapt_faild, 0xf, 4, 1);

/* pddr_port_events_page_layout_pm_fifo_full_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_pm_fifo_full_sv, 0xf, 3, 1);

/* pddr_port_events_page_layout_pm_fifo_full
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_pm_fifo_full, 0xf, 2, 1);

/* pddr_port_events_page_layout_protocol_check_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_protocol_check_sv, 0xf, 1, 1);

/* pddr_port_events_page_layout_protocol_check
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_protocol_check, 0xf, 0, 1);

/* pddr_port_events_page_layout_input_event_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_input_event_sv, 0x10, 7, 1);

/* pddr_port_events_page_layout_input_event
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, pddr_port_events_page_layout_input_event, 0x12, 0, 16);

/* pddr_port_events_page_layout_lane_mapping_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_lane_mapping_sv, 0x17, 3, 1);

/* pddr_port_events_page_layout_lane_mapping
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_lane_mapping, 0x17, 2, 1);

/* pddr_port_events_page_layout_qsfp_zero_atten_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_qsfp_zero_atten_sv, 0x17, 1, 1);

/* pddr_port_events_page_layout_qsfp_zero_atten
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_qsfp_zero_atten, 0x17, 0, 1);

/* pddr_port_events_page_layout_mono_reach_low_limit_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_mono_reach_low_limit_sv, 0x1a, 1, 1);

/* pddr_port_events_page_layout_mono_reach_low_limit
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_mono_reach_low_limit, 0x1a, 0, 1);

/* pddr_port_events_page_layout_mono_reach_high_limit_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_mono_reach_high_limit_sv, 0x1b, 7, 1);

/* pddr_port_events_page_layout_mono_reach_high_limit
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_mono_reach_high_limit, 0x1b, 6, 1);

/* pddr_port_events_page_layout_mono_flow_height_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_mono_flow_height_sv, 0x1b, 5, 1);

/* pddr_port_events_page_layout_mono_flow_height
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_mono_flow_height, 0x1b, 4, 1);

/* pddr_port_events_page_layout_single_gains_bad_val_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_single_gains_bad_val_sv, 0x1b, 3, 1);

/* pddr_port_events_page_layout_single_gains_bad_val
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_single_gains_bad_val, 0x1b, 2, 1);

/* pddr_port_events_page_layout_init_all_gains_bad_val_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_init_all_gains_bad_val_sv, 0x1b, 1, 1);

/* pddr_port_events_page_layout_init_all_gains_bad_val
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_port_events_page_layout_init_all_gains_bad_val, 0x1b, 0, 1);

/* pddr_port_events_page_layout_clr_mask
 * Access: OP
 */
MLXSW_ITEM32_INDEXED(reg, pddr, pddr_port_events_page_layout_clr_mask, 0x28, 0, 32, 4, 0, 4, 0);

/* pddr_device_events_page_invalid_port_access_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_invalid_port_access_sv, 0x8, 7, 1);

/* pddr_device_events_page_invalid_port_access
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, pddr_device_events_page_invalid_port_access, 0xa, 0, 16);

/* pddr_device_events_page_pll_state_sv_1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_pll_state_sv_1, 0xc, 0, 1);

/* pddr_device_events_page_pll_state_mask_center1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_pll_state_mask_center1, 0xd, 0, 4);

/* pddr_device_events_page_pll_state_sv_0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_pll_state_sv_0, 0xe, 0, 1);

/* pddr_device_events_page_pll_state_mask_center0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_pll_state_mask_center0, 0xf, 0, 4);

/* pddr_device_events_page_main_irisc_timeout
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_main_irisc_timeout, 0x10, 7, 1);

/* pddr_device_events_page_tile_irisc_timeout
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_tile_irisc_timeout, 0x10, 6, 1);

/* pddr_device_events_page_plu_timeout0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_plu_timeout0, 0x10, 5, 1);

/* pddr_device_events_page_plu_timeout1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_plu_timeout1, 0x10, 4, 1);

/* pddr_device_events_page_plu_timeout2
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_plu_timeout2, 0x10, 3, 1);

/* pddr_device_events_page_plu_timeout3
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_plu_timeout3, 0x10, 2, 1);

/* pddr_device_events_page_plu_timeout4
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_plu_timeout4, 0x10, 1, 1);

/* pddr_device_events_page_cr_space_timeout_mcm_main
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_cr_space_timeout_mcm_main, 0x11, 2, 1);

/* pddr_device_events_page_cr_space_timeout_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_cr_space_timeout_sv, 0x11, 1, 1);

/* pddr_device_events_page_cr_space_timeout
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_cr_space_timeout, 0x11, 0, 1);

/* pddr_device_events_page_analog_phy_to_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_analog_phy_to_sv, 0x13, 7, 1);

/* pddr_device_events_page_analog_phy_to
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_analog_phy_to, 0x13, 6, 1);

/* pddr_device_events_page_logical_phy_to_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_logical_phy_to_sv, 0x13, 5, 1);

/* pddr_device_events_page_logical_phy_to
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_logical_phy_to, 0x13, 4, 1);

/* pddr_device_events_page_varactors_calib_fail_center1_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_varactors_calib_fail_center1_sv, 0x13, 3, 1);

/* pddr_device_events_page_varactors_calib_fail_center1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_varactors_calib_fail_center1, 0x13, 2, 1);

/* pddr_device_events_page_varactors_calib_fail_center0_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_varactors_calib_fail_center0_sv, 0x13, 1, 1);

/* pddr_device_events_page_varactors_calib_fail_center0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_varactors_calib_fail_center0, 0x13, 0, 1);

/* pddr_device_events_page_irisc_stuck_mask_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_irisc_stuck_mask_sv, 0x14, 7, 1);

/* pddr_device_events_page_irisc_stuck_mask
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, pddr_device_events_page_irisc_stuck_mask, 0x14, 0, 24);

/* pddr_device_events_page_pll_lock_sv_1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_pll_lock_sv_1, 0x1c, 0, 1);

/* pddr_device_events_page_pll_unlocl_mask_center1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_pll_unlocl_mask_center1, 0x1d, 0, 4);

/* pddr_device_events_page_pll_lock_sv_0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_pll_lock_sv_0, 0x1e, 0, 1);

/* pddr_device_events_page_pll_unlocl_mask_center0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_pll_unlocl_mask_center0, 0x1f, 0, 4);

/* pddr_device_events_page_uc_stack_overflow
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_uc_stack_overflow, 0x20, 1, 1);

/* pddr_device_events_page_uc_crspace_timeout
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_uc_crspace_timeout, 0x20, 0, 1);

/* pddr_device_events_page_pwr_governor_err_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_pwr_governor_err_sv, 0x21, 1, 1);

/* pddr_device_events_page_pwr_governor_err
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_pwr_governor_err, 0x21, 0, 1);

/* pddr_device_events_page_timer_error_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_timer_error_sv, 0x23, 5, 1);

/* pddr_device_events_page_timer_error
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_timer_error, 0x23, 4, 1);

/* pddr_device_events_page_iopl_err_center1_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_iopl_err_center1_sv, 0x23, 3, 1);

/* pddr_device_events_page_iopl_err_center1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_iopl_err_center1, 0x23, 2, 1);

/* pddr_device_events_page_iopl_err_center0_sv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_iopl_err_center0_sv, 0x23, 1, 1);

/* pddr_device_events_page_iopl_err_center0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_iopl_err_center0, 0x23, 0, 1);

/* pddr_device_events_page_clr_mask
 * Access: OP
 */
MLXSW_ITEM32_INDEXED(reg, pddr, pddr_device_events_page_clr_mask, 0x28, 0, 32, 4, 0, 4, 0);

/* pddr_device_events_page_main_irisc_address
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, pddr_device_events_page_main_irisc_address, 0x48, 0, 32);

/* pddr_device_events_page_main_irisc_ip
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, pddr_device_events_page_main_irisc_ip, 0x4c, 0, 32);

/* pddr_device_events_page_tile_irisc_address
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, pddr_device_events_page_tile_irisc_address, 0x50, 0, 32);

/* pddr_device_events_page_tile_irisc_ip
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, pddr_device_events_page_tile_irisc_ip, 0x54, 0, 32);

/* pddr_device_events_page_plu_timeout0_address
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, pddr_device_events_page_plu_timeout0_address, 0x58, 0, 16);

/* pddr_device_events_page_plu_timeout1_address
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, pddr_device_events_page_plu_timeout1_address, 0x5a, 0, 16);

/* pddr_device_events_page_plu_timeout2_address
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, pddr_device_events_page_plu_timeout2_address, 0x5c, 0, 16);

/* pddr_device_events_page_plu_timeout3_address
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, pddr_device_events_page_plu_timeout3_address, 0x5e, 0, 16);

/* pddr_device_events_page_plu_timeout4_address
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, pddr_device_events_page_plu_timeout4_address, 0x60, 0, 16);

/* pddr_device_events_page_plu_timeout0_cluster
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_plu_timeout0_cluster, 0x64, 0, 6);

/* pddr_device_events_page_plu_timeout1_cluster
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_plu_timeout1_cluster, 0x65, 0, 6);

/* pddr_device_events_page_plu_timeout2_cluster
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_plu_timeout2_cluster, 0x66, 0, 6);

/* pddr_device_events_page_plu_timeout3_cluster
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_plu_timeout3_cluster, 0x67, 0, 6);

/* pddr_device_events_page_plu_timeout4_cluster
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_plu_timeout4_cluster, 0x68, 0, 6);

/* pddr_device_events_page_irisc_tile_idx
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_irisc_tile_idx, 0x6b, 0, 4);

/* pddr_device_events_page_uc_crspace_timeout_lane
 * Access: RW
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_uc_crspace_timeout_lane, 0x6c, 0, 3);

/* pddr_device_events_page_uc_crspace_timeout_cl
 * Access: RW
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_uc_crspace_timeout_cl, 0x6d, 0, 6);

/* pddr_device_events_page_uc_stack_overflow_lane
 * Access: RW
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_uc_stack_overflow_lane, 0x6e, 0, 3);

/* pddr_device_events_page_uc_stack_overflow_cl
 * Access: RW
 */
MLXSW_ITEM8(reg, pddr, pddr_device_events_page_uc_stack_overflow_cl, 0x6f, 0, 6);

/* down_intent
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, down_intent, 0xb, 4, 4);

/* down_blame
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, down_blame, 0xb, 0, 4);

/* recovery_entry_reason
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, recovery_entry_reason, 0xd, 0, 8);

/* local_reason_opcode
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, local_reason_opcode, 0xf, 0, 8);

/* remote_reason_opcode
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, remote_reason_opcode, 0x13, 0, 8);

/* last_recovery_state
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, last_recovery_state, 0x15, 4, 4);

/* l1_failure_reason
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, l1_failure_reason, 0x15, 0, 4);

/* ts1_opcode
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, ts1_opcode, 0x16, 4, 4);

/* e2e_reason_opcode
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, e2e_reason_opcode, 0x17, 0, 8);

/* cons_raw_norm_ber
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cons_raw_norm_ber, 0x18, 4, 4);

/* last_raw_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, last_raw_ber_coef, 0x18, 0, 4);

/* last_raw_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, last_raw_ber_magnitude, 0x19, 0, 8);

/* num_of_symbol_ber_alarms
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, num_of_symbol_ber_alarms, 0x1a, 0, 16);

/* min_raw_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, min_raw_ber_coef, 0x1c, 0, 4);

/* min_raw_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, min_raw_ber_magnitude, 0x1d, 0, 8);

/* max_raw_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, max_raw_ber_coef, 0x1e, 0, 4);

/* max_raw_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, max_raw_ber_magnitude, 0x1f, 0, 8);

/* num_of_eff_ber_alarms
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, num_of_eff_ber_alarms, 0x20, 0, 16);

/* num_of_raw_ber_alarms
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, num_of_raw_ber_alarms, 0x22, 0, 16);

/* time_to_link_down_to_disable
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, time_to_link_down_to_disable, 0x24, 0, 32);

/* time_to_link_down_to_rx_loss
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, time_to_link_down_to_rx_loss, 0x28, 0, 32);

/* cons_eff_norm_ber
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cons_eff_norm_ber, 0x2c, 4, 4);

/* last_eff_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, last_eff_ber_coef, 0x2c, 0, 4);

/* last_eff_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, last_eff_ber_magnitude, 0x2d, 0, 8);

/* min_eff_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, min_eff_ber_coef, 0x2e, 0, 4);

/* min_eff_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, min_eff_ber_magnitude, 0x2f, 0, 8);

/* cons_symbol_norm_ber
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cons_symbol_norm_ber, 0x30, 4, 4);

/* max_symbol_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, max_symbol_ber_coef, 0x30, 0, 4);

/* max_symbol_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, max_symbol_ber_magnitude, 0x31, 0, 8);

/* max_eff_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, max_eff_ber_coef, 0x32, 0, 4);

/* max_eff_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, max_eff_ber_magnitude, 0x33, 0, 8);

/* last_symbol_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, last_symbol_ber_coef, 0x34, 0, 4);

/* last_symbol_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, last_symbol_ber_magnitude, 0x35, 0, 8);

/* min_symbol_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, min_symbol_ber_coef, 0x36, 0, 4);

/* min_symbol_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, min_symbol_ber_magnitude, 0x37, 0, 8);

/* hi_ser_counter
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, hi_ser_counter, 0x3b, 0, 8);

/* ib_port_events
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, ib_port_events, 0x8, 0, 32);

/* ethernet_port_events
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, ethernet_port_events, 0x10, 0, 32);

/* general_port_events
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, general_port_events, 0x18, 0, 32);

/* up_reason_pwr
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, up_reason_pwr, 0x9, 0, 4);

/* up_reason_drv
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, up_reason_drv, 0xa, 0, 4);

/* up_reason_mng
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, up_reason_mng, 0xb, 0, 4);

/* time_to_link_up
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, time_to_link_up, 0xc, 0, 32);

/* lt_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, lt_cnt, 0x12, 0, 4);

/* fast_link_up_status
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, fast_link_up_status, 0x13, 0, 4);

/* time_to_link_up_phy_up_to_active
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, time_to_link_up_phy_up_to_active, 0x14, 0, 32);

/* time_to_link_up_sd_to_phy_up
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, time_to_link_up_sd_to_phy_up, 0x18, 0, 32);

/* time_to_link_up_disable_to_sd
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, time_to_link_up_disable_to_sd, 0x1c, 0, 32);

/* time_to_link_up_disable_to_pd
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, time_to_link_up_disable_to_pd, 0x20, 0, 32);

/* time_of_module_conf_done_up
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, time_of_module_conf_done_up, 0x24, 0, 32);

/* time_of_module_conf_done_down
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, time_of_module_conf_done_down, 0x28, 0, 32);

/* time_logical_init_to_active
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, time_logical_init_to_active, 0x2c, 0, 32);

/* total_time_pcs_local_fault
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, total_time_pcs_local_fault, 0x30, 0, 32);

/* total_time_pcs_remote_fault
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, total_time_pcs_remote_fault, 0x34, 0, 32);

/* time_on_last_hi_ser
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, time_on_last_hi_ser, 0x38, 0, 32);

/* temp_flags
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, temp_flags, 0x8, 4, 4);

/* vcc_flags
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, vcc_flags, 0x8, 0, 4);

/* mod_fw_fault
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, mod_fw_fault, 0x9, 7, 1);

/* dp_fw_fault
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, dp_fw_fault, 0x9, 6, 1);

/* rx_los_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, rx_los_cap, 0xb, 0, 1);

/* tx_fault
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_fault, 0xc, 0, 8);

/* tx_los
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_los, 0xd, 0, 8);

/* tx_cdr_lol
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_cdr_lol, 0xe, 0, 8);

/* tx_ad_eq_fault
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_ad_eq_fault, 0xf, 0, 8);

/* tx_power_hi_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_power_hi_al, 0x10, 0, 8);

/* tx_power_lo_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_power_lo_al, 0x11, 0, 8);

/* tx_power_hi_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_power_hi_war, 0x12, 0, 8);

/* tx_power_lo_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_power_lo_war, 0x13, 0, 8);

/* tx_bias_hi_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_bias_hi_al, 0x14, 0, 8);

/* tx_bias_lo_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_bias_lo_al, 0x15, 0, 8);

/* tx_bias_hi_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_bias_hi_war, 0x16, 0, 8);

/* tx_bias_lo_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, tx_bias_lo_war, 0x17, 0, 8);

/* rx_los
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, rx_los, 0x18, 0, 8);

/* rx_cdr_lol
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, rx_cdr_lol, 0x19, 0, 8);

/* rx_power_hi_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, rx_power_hi_al, 0x1c, 0, 8);

/* rx_power_lo_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, rx_power_lo_al, 0x1d, 0, 8);

/* rx_power_hi_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, rx_power_hi_war, 0x1e, 0, 8);

/* rx_power_lo_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, rx_power_lo_war, 0x1f, 0, 8);

/* laser_fault_flag
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser_fault_flag, 0x20, 7, 1);

/* laser2_fault_flag
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser2_fault_flag, 0x20, 6, 1);

/* laser_warning_flag
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser_warning_flag, 0x20, 5, 1);

/* laser2_warning_flag
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser2_warning_flag, 0x20, 4, 1);

/* laser_source_flag_in_use_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser_source_flag_in_use_msb, 0x20, 0, 4);

/* rx_output_valid_change
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, rx_output_valid_change, 0x23, 0, 8);

/* flag_in_use
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, flag_in_use, 0x24, 0, 32);

/* laser_source_flag_in_use
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, laser_source_flag_in_use, 0x28, 8, 24);

/* laser_bias_hi_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser_bias_hi_al, 0x2b, 7, 1);

/* laser2_bias_hi_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser2_bias_hi_al, 0x2b, 6, 1);

/* laser_bias_lo_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser_bias_lo_al, 0x2b, 5, 1);

/* laser2_bias_lo_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser2_bias_lo_al, 0x2b, 4, 1);

/* laser_bias_hi_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser_bias_hi_war, 0x2b, 3, 1);

/* laser2_bias_hi_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser2_bias_hi_war, 0x2b, 2, 1);

/* laser_bias_lo_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser_bias_lo_war, 0x2b, 1, 1);

/* laser2_bias_lo_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser2_bias_lo_war, 0x2b, 0, 1);

/* laser_opt_pwr_hi_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser_opt_pwr_hi_al, 0x2c, 7, 1);

/* laser2_opt_pwr_hi_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser2_opt_pwr_hi_al, 0x2c, 6, 1);

/* laser_opt_pwr_lo_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser_opt_pwr_lo_al, 0x2c, 5, 1);

/* laser2_opt_pwr_lo_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser2_opt_pwr_lo_al, 0x2c, 4, 1);

/* laser_opt_pwr_hi_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser_opt_pwr_hi_war, 0x2c, 3, 1);

/* laser2_opt_pwr_hi_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser2_opt_pwr_hi_war, 0x2c, 2, 1);

/* laser_opt_pwr_lo_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser_opt_pwr_lo_war, 0x2c, 1, 1);

/* laser2_opt_pwr_lo_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser2_opt_pwr_lo_war, 0x2c, 0, 1);

/* laser_temp_hi_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser_temp_hi_al, 0x2d, 7, 1);

/* laser2_temp_hi_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser2_temp_hi_al, 0x2d, 6, 1);

/* laser_temp_lo_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser_temp_lo_al, 0x2d, 5, 1);

/* laser2_temp_lo_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser2_temp_lo_al, 0x2d, 4, 1);

/* laser_temp_hi_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser_temp_hi_war, 0x2d, 3, 1);

/* laser2_temp_hi_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser2_temp_hi_war, 0x2d, 2, 1);

/* laser_temp_lo_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser_temp_lo_war, 0x2d, 1, 1);

/* laser2_temp_lo_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, laser2_temp_lo_war, 0x2d, 0, 1);

/* optical_engine_flag_in_use
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, optical_engine_flag_in_use, 0x2e, 0, 16);

/* els_input_power_hi_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, els_input_power_hi_al, 0x30, 0, 8);

/* els_input_power_lo_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, els_input_power_lo_al, 0x31, 0, 8);

/* els_input_power_hi_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, els_input_power_hi_war, 0x32, 0, 8);

/* els_input_power_lo_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, els_input_power_lo_war, 0x33, 0, 8);

/* lane_temp_hi_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, lane_temp_hi_al, 0x34, 0, 8);

/* lane_temp_lo_al
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, lane_temp_lo_al, 0x35, 0, 8);

/* lane_temp_hi_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, lane_temp_hi_war, 0x36, 0, 8);

/* lane_temp_lo_war
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, lane_temp_lo_war, 0x37, 0, 8);

/* pport
 * Access: INDEX
 */
MLXSW_ITEM16(reg, pddr, pport, 0x8, 6, 10);

/* trigger_active
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, trigger_active, 0x9, 5, 1);

/* trigger_cond_state_event_val
 * Access: INDEX
 */
MLXSW_ITEM32(reg, pddr, trigger_cond_state_event_val, 0x8, 14, 7);

/* trigger_cond_state_or_event
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pddr, trigger_cond_state_or_event, 0xa, 5, 1);

/* trigger_cond_fsm
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pddr, trigger_cond_fsm, 0xa, 0, 5);

/* pre_trigger_buff_mode
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pddr, pre_trigger_buff_mode, 0xb, 5, 3);

/* tracer_mode
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pddr, tracer_mode, 0xb, 2, 3);

/* reset_tracer
 * Access: OP
 */
MLXSW_ITEM8(reg, pddr, reset_tracer, 0xb, 1, 1);

/* tracer_enable
 * Access: OP
 */
MLXSW_ITEM8(reg, pddr, tracer_enable, 0xb, 0, 1);

/* fsm_mask
 * Access: INDEX
 */
MLXSW_ITEM32(reg, pddr, fsm_mask, 0xc, 0, 32);

/* trigger_ptr
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, trigger_ptr, 0x10, 12, 12);

/* buffer_size
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, buffer_size, 0x12, 0, 12);

/* ptr_log_data
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, ptr_log_data, 0x14, 12, 12);

/* ptr_next_write
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, ptr_next_write, 0x16, 0, 12);

/* buffer_base_address
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, buffer_base_address, 0x18, 0, 32);

/* pport_second
 * Access: INDEX
 */
MLXSW_ITEM16(reg, pddr, pport_second, 0x1c, 4, 10);

/* second_port_valid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pddr, second_port_valid, 0x1d, 2, 1);

/* trigger_cond_port
 * Access: RW
 */
MLXSW_ITEM8(reg, pddr, trigger_cond_port, 0x1f, 5, 2);

/* revision
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, revision, 0x1f, 0, 4);

/* page_size
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, page_size, 0x20, 10, 8);

/* page_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pddr, page_index, 0x23, 0, 8);

/* port_selector
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, pddr, port_selector, MLXSW_PDDR_PDDR_PHY_DEBUG_PAGE_TRACER_DATA_ENTRY_BASE_LEN, 7, 1, MLXSW_PDDR_TRACER_DATA_ENTRY_REC_LEN, 0, 1, 0);

/* time_stamp_unit
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, pddr, time_stamp_unit, MLXSW_PDDR_PDDR_PHY_DEBUG_PAGE_TRACER_DATA_ENTRY_BASE_LEN, 4, 3, MLXSW_PDDR_TRACER_DATA_ENTRY_REC_LEN, 0, 1, 0);

/* time_stamp
 * Access: RO
 */
MLXSW_ITEM16_INDEXED(reg, pddr, time_stamp, MLXSW_PDDR_PDDR_PHY_DEBUG_PAGE_TRACER_DATA_ENTRY_BASE_LEN, 1, 11, MLXSW_PDDR_TRACER_DATA_ENTRY_REC_LEN, 0, 2, 0);

/* event_verifier_status
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, pddr, event_verifier_status, MLXSW_PDDR_PDDR_PHY_DEBUG_PAGE_TRACER_DATA_ENTRY_BASE_LEN, 14, 3, MLXSW_PDDR_TRACER_DATA_ENTRY_REC_LEN, 0, 3, 0);

/* state_or_event
 * Access: RO
 */
MLXSW_ITEM16_INDEXED(reg, pddr, state_or_event, MLXSW_PDDR_PDDR_PHY_DEBUG_PAGE_TRACER_DATA_ENTRY_BASE_LEN + 2, 7, 7, MLXSW_PDDR_TRACER_DATA_ENTRY_REC_LEN, 0, 2, 0);

/* event_state
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, pddr, event_state, MLXSW_PDDR_PDDR_PHY_DEBUG_PAGE_TRACER_DATA_ENTRY_BASE_LEN + 3, 5, 2, MLXSW_PDDR_TRACER_DATA_ENTRY_REC_LEN, 0, 1, 0);

/* phy_fsm
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, pddr, phy_fsm, MLXSW_PDDR_PDDR_PHY_DEBUG_PAGE_TRACER_DATA_ENTRY_BASE_LEN + 3, 0, 5, MLXSW_PDDR_TRACER_DATA_ENTRY_REC_LEN, 0, 1, 0);

/* fw_module_state
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, fw_module_state, 0xb, 0, 4);

/* fw_dp_state_lane7
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, fw_dp_state_lane7, 0xc, 4, 4);

/* fw_dp_state_lane6
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, fw_dp_state_lane6, 0xc, 0, 4);

/* fw_dp_state_lane5
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, fw_dp_state_lane5, 0xd, 4, 4);

/* fw_dp_state_lane4
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, fw_dp_state_lane4, 0xd, 0, 4);

/* fw_dp_state_lane3
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, fw_dp_state_lane3, 0xe, 4, 4);

/* fw_dp_state_lane2
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, fw_dp_state_lane2, 0xe, 0, 4);

/* fw_dp_state_lane1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, fw_dp_state_lane1, 0xf, 4, 4);

/* fw_dp_state_lane0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, fw_dp_state_lane0, 0xf, 0, 4);

/* dp_req_status_lane7
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, dp_req_status_lane7, 0x10, 4, 4);

/* dp_req_status_lane6
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, dp_req_status_lane6, 0x10, 0, 4);

/* dp_req_status_lane5
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, dp_req_status_lane5, 0x11, 4, 4);

/* dp_req_status_lane4
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, dp_req_status_lane4, 0x11, 0, 4);

/* dp_req_status_lane3
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, dp_req_status_lane3, 0x12, 4, 4);

/* dp_req_status_lane2
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, dp_req_status_lane2, 0x12, 0, 4);

/* dp_req_status_lane1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, dp_req_status_lane1, 0x13, 4, 4);

/* dp_req_status_lane0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, dp_req_status_lane0, 0x13, 0, 4);

/* dpsm_apply_state_lane7
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, dpsm_apply_state_lane7, 0x14, 4, 4);

/* dpsm_apply_state_lane6
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, dpsm_apply_state_lane6, 0x14, 0, 4);

/* dpsm_apply_state_lane5
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, dpsm_apply_state_lane5, 0x15, 4, 4);

/* dpsm_apply_state_lane4
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, dpsm_apply_state_lane4, 0x15, 0, 4);

/* dpsm_apply_state_lane3
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, dpsm_apply_state_lane3, 0x16, 4, 4);

/* dpsm_apply_state_lane2
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, dpsm_apply_state_lane2, 0x16, 0, 4);

/* dpsm_apply_state_lane1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, dpsm_apply_state_lane1, 0x17, 4, 4);

/* dpsm_apply_state_lane0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, dpsm_apply_state_lane0, 0x17, 0, 4);

/* timer_left_for_dp_active_transition
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, timer_left_for_dp_active_transition, 0x19, 0, 8);

/* timer_left_for_high_pwrt_transition
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, timer_left_for_high_pwrt_transition, 0x1b, 0, 8);

/* isr_access_enums
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pddr, isr_access_enums, 0x8, 6, 2);

/* isr_track_mod
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, isr_track_mod, 0x9, 1, 1);

/* isr_track_enum
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, isr_track_enum, 0x9, 0, 1);

/* max_isr_enum
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, max_isr_enum, 0xb, 0, 8);

/* max_time_isr
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, pddr, max_time_isr, 0x10, 0, 32, 4, 0, 4, 0);

/* info_supported_mask
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, info_supported_mask, 0xa, 0, 16);

/* bubble_agreement
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, bubble_agreement, 0xe, 4, 2);

/* partner_local_port
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, partner_local_port, 0xe, 0, 10);

/* partner_module_type
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, partner_module_type, 0x13, 0, 4);

/* partner_id_lsb
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, partner_id_lsb, 0x18, 0, 24);

/* tracer_sel
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pddr, tracer_sel, 0xb, 0, 2);

/* array512_auto
 * Access: 
 */
MLXSW_ITEM32_INDEXED(reg, pddr, array512_auto, 0xc, 0, 32, 0x40, 0x0, 4, 0);

/* cause_as_detected_not_on_slot
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cause_as_detected_not_on_slot, 0x9, 6, 2);

/* cause_high_ser
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cause_high_ser, 0x9, 4, 2);

/* cause_rs_rx_lane0_symbol_error
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cause_rs_rx_lane0_symbol_error, 0x9, 2, 2);

/* cause_tx_lane_fifo_underrun_l0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cause_tx_lane_fifo_underrun_l0, 0x9, 0, 2);

/* cause_deskew_fifo_overrun
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cause_deskew_fifo_overrun, 0xa, 0, 8);

/* cause_fec_rx_sync_m_reached_max0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, cause_fec_rx_sync_m_reached_max0, 0xb, 0, 8);

/* symbol_error_counter_lo
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, symbol_error_counter_lo, 0xc, 0, 32);

/* symbol_error_counter_hi
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, symbol_error_counter_hi, 0x10, 0, 32);

/* uncorrectable_block_counter_lo
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, uncorrectable_block_counter_lo, 0x14, 0, 32);

/* uncorrectable_block_counter_hi
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, uncorrectable_block_counter_hi, 0x18, 0, 32);

/* link_fail_due_align_loss
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, link_fail_due_align_loss, 0x1e, 0, 1);

/* block_lock_mask_at_fail_port1
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, block_lock_mask_at_fail_port1, 0x1f, 4, 4);

/* block_lock_mask_at_fail_port0
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, block_lock_mask_at_fail_port0, 0x1f, 0, 4);

/* link_down_counter
 * Access: RO
 */
MLXSW_ITEM32(reg, pddr, link_down_counter, 0x20, 0, 32);

/* effective_ber_coef
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, effective_ber_coef, 0x25, 0, 8);

/* effective_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, effective_ber_magnitude, 0x26, 4, 4);

/* raw_ber_coef
 * Access: RO
 */
MLXSW_ITEM16(reg, pddr, raw_ber_coef, 0x26, 4, 8);

/* raw_ber_magnitude
 * Access: RO
 */
MLXSW_ITEM8(reg, pddr, raw_ber_magnitude, 0x27, 0, 4);

/* pptt
 * ----
 *
 */
/* e
 * Access: RW
 */
MLXSW_ITEM8(reg, pptt, e, 0x0, 7, 1);

/* p
 * Access: RW
 */
MLXSW_ITEM8(reg, pptt, p, 0x0, 4, 1);

/* p_c
 * Access: RO
 */
MLXSW_ITEM8(reg, pptt, p_c, 0x0, 3, 1);

/* dm_ig
 * Access: OP
 */
MLXSW_ITEM8(reg, pptt, dm_ig, 0x0, 2, 1);

/* sw
 * Access: RW
 */
MLXSW_ITEM8(reg, pptt, sw, 0x0, 1, 1);

/* sw_c
 * Access: RO
 */
MLXSW_ITEM8(reg, pptt, sw_c, 0x0, 0, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pptt, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pptt, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pptt, lp_msb, 0x2, 4, 2);

/* lane
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pptt, lane, 0x2, 0, 4);

/* port_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pptt, port_type, 0x3, 4, 4);

/* ls
 * Access: RO
 */
MLXSW_ITEM8(reg, pptt, ls, 0x3, 1, 1);

/* le
 * Access: OP
 */
MLXSW_ITEM8(reg, pptt, le, 0x3, 0, 1);

/* prbs_modes_cap
 * Access: RO
 */
MLXSW_ITEM32(reg, pptt, prbs_modes_cap, 0x4, 0, 32);

/* prbs_mode_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, pptt, prbs_mode_admin, 0x8, 0, 8);

/* mode_b_idx
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pptt, mode_b_idx, 0x9, 6, 2);

/* expert_mode_idx
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pptt, expert_mode_idx, 0x9, 5, 1);

/* primary_secondary_idx
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pptt, primary_secondary_idx, 0x9, 4, 1);

/* pat_for_tun
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pptt, pat_for_tun, 0x9, 3, 1);

/* otf_ch_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pptt, otf_ch_cap, 0xa, 7, 1);

/* pat_ch_c
 * Access: RO
 */
MLXSW_ITEM8(reg, pptt, pat_ch_c, 0xa, 3, 1);

/* pat_ch
 * Access: OP
 */
MLXSW_ITEM8(reg, pptt, pat_ch, 0xa, 2, 1);

/* vs_peer_db
 * Access: OP
 */
MLXSW_ITEM8(reg, pptt, vs_peer_db, 0xb, 4, 2);

/* modulation
 * Access: RW
 */
MLXSW_ITEM8(reg, pptt, modulation, 0xb, 0, 4);

/* lane_rate_cap
 * Access: RO
 */
MLXSW_ITEM16(reg, pptt, lane_rate_cap, 0xc, 0, 16);

/* prbs_fec_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pptt, prbs_fec_cap, 0xf, 0, 1);

/* lane_rate_admin
 * Access: RW
 */
MLXSW_ITEM16(reg, pptt, lane_rate_admin, 0x10, 0, 16);

/* prbs_fec_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, pptt, prbs_fec_admin, 0x13, 0, 1);

/* pnlpnr
 * ------
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pnlpnr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pnlpnr, lp_msb, 0x2, 4, 2);

/* nlpn_v
 * Access: RO
 */
MLXSW_ITEM8(reg, pnlpnr, nlpn_v, 0x4, 7, 1);

/* link_up
 * Access: RO
 */
MLXSW_ITEM8(reg, pnlpnr, link_up, 0x4, 6, 1);

/* nlpn_rev
 * Access: RO
 */
MLXSW_ITEM8(reg, pnlpnr, nlpn_rev, 0x7, 0, 4);

/* page_req
 * Access: OP
 */
MLXSW_ITEM64(reg, pnlpnr, page_req, 0x10, 0, 64);

/* page_valid
 * Access: RO
 */
MLXSW_ITEM64(reg, pnlpnr, page_valid, 0x18, 0, 64);

/* pages
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, pnlpnr, pages, 0x20, 0, 32, 4, 0, 4, 0);

/* pprt
 * ----
 *
 */
/* e
 * Access: RW
 */
MLXSW_ITEM8(reg, pprt, e, 0x0, 7, 1);

/* s
 * Access: OP
 */
MLXSW_ITEM8(reg, pprt, s, 0x0, 6, 1);

/* tun_ovr
 * Access: OP
 */
MLXSW_ITEM8(reg, pprt, tun_ovr, 0x0, 5, 1);

/* p
 * Access: RW
 */
MLXSW_ITEM8(reg, pprt, p, 0x0, 4, 1);

/* p_c
 * Access: RO
 */
MLXSW_ITEM8(reg, pprt, p_c, 0x0, 3, 1);

/* dm_ig
 * Access: OP
 */
MLXSW_ITEM8(reg, pprt, dm_ig, 0x0, 2, 1);

/* sw
 * Access: RW
 */
MLXSW_ITEM8(reg, pprt, sw, 0x0, 1, 1);

/* sw_c
 * Access: RO
 */
MLXSW_ITEM8(reg, pprt, sw_c, 0x0, 0, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pprt, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pprt, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pprt, lp_msb, 0x2, 4, 2);

/* lane
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pprt, lane, 0x2, 0, 4);

/* port_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pprt, port_type, 0x3, 4, 4);

/* ls
 * Access: RO
 */
MLXSW_ITEM8(reg, pprt, ls, 0x3, 1, 1);

/* le
 * Access: OP
 */
MLXSW_ITEM8(reg, pprt, le, 0x3, 0, 1);

/* prbs_modes_cap
 * Access: RO
 */
MLXSW_ITEM32(reg, pprt, prbs_modes_cap, 0x4, 0, 32);

/* prbs_mode_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, pprt, prbs_mode_admin, 0x8, 0, 8);

/* mode_b_idx
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pprt, mode_b_idx, 0x9, 6, 2);

/* expert_mode_idx
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pprt, expert_mode_idx, 0x9, 5, 1);

/* primary_secondary_idx
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pprt, primary_secondary_idx, 0x9, 4, 1);

/* tuning_type
 * Access: RW
 */
MLXSW_ITEM8(reg, pprt, tuning_type, 0x9, 0, 4);

/* otf_ch_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pprt, otf_ch_cap, 0xa, 7, 1);

/* msb_lsb_cnt_c
 * Access: RO
 */
MLXSW_ITEM8(reg, pprt, msb_lsb_cnt_c, 0xa, 5, 1);

/* ph_p_c
 * Access: RO
 */
MLXSW_ITEM8(reg, pprt, ph_p_c, 0xa, 4, 1);

/* pat_ch_c
 * Access: RO
 */
MLXSW_ITEM8(reg, pprt, pat_ch_c, 0xa, 3, 1);

/* pat_ch
 * Access: OP
 */
MLXSW_ITEM8(reg, pprt, pat_ch, 0xa, 2, 1);

/* peq_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pprt, peq_cap, 0xa, 1, 1);

/* peq_dis
 * Access: RW
 */
MLXSW_ITEM8(reg, pprt, peq_dis, 0xa, 0, 1);

/* msb_lsb_cnt
 * Access: RW
 */
MLXSW_ITEM8(reg, pprt, msb_lsb_cnt, 0xb, 6, 2);

/* ph_p
 * Access: RW
 */
MLXSW_ITEM8(reg, pprt, ph_p, 0xb, 4, 1);

/* modulation
 * Access: RW
 */
MLXSW_ITEM8(reg, pprt, modulation, 0xb, 0, 4);

/* lane_rate_cap
 * Access: RO
 */
MLXSW_ITEM16(reg, pprt, lane_rate_cap, 0xc, 0, 16);

/* prbs_hist_fec_int
 * Access: RW
 */
MLXSW_ITEM8(reg, pprt, prbs_hist_fec_int, 0xf, 5, 2);

/* prbs_hist_en
 * Access: RW
 */
MLXSW_ITEM8(reg, pprt, prbs_hist_en, 0xf, 4, 1);

/* auto_pol_rev_fix_en
 * Access: RW
 */
MLXSW_ITEM8(reg, pprt, auto_pol_rev_fix_en, 0xf, 3, 1);

/* auto_pol_rev_fix_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pprt, auto_pol_rev_fix_cap, 0xf, 2, 1);

/* prbs_hist_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pprt, prbs_hist_cap, 0xf, 1, 1);

/* prbs_fec_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pprt, prbs_fec_cap, 0xf, 0, 1);

/* lane_rate_oper
 * Access: RW
 */
MLXSW_ITEM16(reg, pprt, lane_rate_oper, 0x10, 0, 16);

/* prbs_hist_fec_int_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pprt, prbs_hist_fec_int_cap, 0x12, 0, 4);

/* prbs_hist_lane_sel
 * Access: RW
 */
MLXSW_ITEM8(reg, pprt, prbs_hist_lane_sel, 0x13, 1, 4);

/* prbs_fec_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, pprt, prbs_fec_admin, 0x13, 0, 1);

/* prbs_rx_tuning_status
 * Access: RO
 */
MLXSW_ITEM8(reg, pprt, prbs_rx_tuning_status, 0x14, 4, 4);

/* prbs_lock_status
 * Access: RO
 */
MLXSW_ITEM8(reg, pprt, prbs_lock_status, 0x14, 0, 4);

/* prbs_lock_status_ext
 * Access: RO
 */
MLXSW_ITEM8(reg, pprt, prbs_lock_status_ext, 0x15, 4, 4);

/* supported_tuning_types
 * Access: RO
 */
MLXSW_ITEM16(reg, pprt, supported_tuning_types, 0x16, 0, 16);

/* tuning_params
 * Access: RO
 */
MLXSW_ITEM32(reg, pprt, tuning_params, 0x18, 0, 32);

/* lt_tuning_support
 * Access: RO
 */
MLXSW_ITEM8(reg, pprt, lt_tuning_support, 0x1d, 0, 8);

/* lt_tuning_admin
 * Access: RW
 */
MLXSW_ITEM8(reg, pprt, lt_tuning_admin, 0x1f, 0, 8);

/* sampling_time_res
 * Access: RW
 */
MLXSW_ITEM8(reg, pprt, sampling_time_res, 0x20, 7, 1);

/* sampling_time_multiplier
 * Access: RW
 */
MLXSW_ITEM8(reg, pprt, sampling_time_multiplier, 0x20, 4, 1);

/* preset_mask
 * Access: RW
 */
MLXSW_ITEM8(reg, pprt, preset_mask, 0x23, 0, 5);

/* pmdr
 * ----
 *
 */
/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, status, 0x0, 4, 4);

/* version
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, version, 0x0, 0, 4);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmdr, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmdr, pnat, 0x2, 6, 2);

/* gb_valid
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, gb_valid, 0x2, 5, 1);

/* mcm_tile_valid
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, mcm_tile_valid, 0x2, 4, 1);

/* scc_valid
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, scc_valid, 0x2, 3, 1);

/* width_valid
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, width_valid, 0x2, 2, 1);

/* plane_ind
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmdr, plane_ind, 0x3, 4, 4);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmdr, lp_msb, 0x3, 0, 2);

/* pport
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, pport, 0x4, 0, 8);

/* module
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, module, 0x5, 0, 8);

/* cluster
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, cluster, 0x6, 0, 7);

/* pport_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, pport_msb, 0x7, 6, 2);

/* clp_4x
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, clp_4x, 0x7, 4, 2);

/* clp_1x
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, clp_1x, 0x7, 0, 2);

/* gb_dp_num
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, gb_dp_num, 0x8, 3, 5);

/* split
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, split, 0x8, 0, 3);

/* swid
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, swid, 0x9, 0, 8);

/* module_lane_mask
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, module_lane_mask, 0xa, 0, 8);

/* ib_port
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, ib_port, 0xb, 0, 8);

/* label_port_query
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, label_port_query, 0xc, 0, 8);

/* local_port_query
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, local_port_query, 0xd, 0, 8);

/* usr_sd_4x_tx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, usr_sd_4x_tx, 0xe, 6, 2);

/* usr_sd_4x_rx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, usr_sd_4x_rx, 0xe, 4, 2);

/* port_width
 * Access: RO
 */
MLXSW_ITEM16(reg, pmdr, port_width, 0xe, 6, 4);

/* tile_pport_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, tile_pport_msb, 0xf, 4, 2);

/* lbp_query_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lbp_query_msb, 0xf, 2, 2);

/* lp_query_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lp_query_msb, 0xf, 0, 2);

/* slot_index
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, slot_index, 0x10, 4, 4);

/* tile_cluster
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, tile_cluster, 0x10, 0, 4);

/* mcm_tile_num
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, mcm_tile_num, 0x11, 0, 8);

/* pll_cnt_rx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, pll_cnt_rx, 0x12, 5, 3);

/* tile_pport
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, tile_pport, 0x12, 0, 5);

/* gearbox_die_num
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, gearbox_die_num, 0x13, 0, 7);

/* vl_num
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl_num, 0x14, 3, 5);

/* pll_cnt_tx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, pll_cnt_tx, 0x14, 0, 3);

/* lane7_physical_rx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane7_physical_rx, 0x15, 5, 3);

/* lane6_physical_rx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane6_physical_rx, 0x15, 2, 3);

/* lane5_physical_rx
 * Access: RO
 */
MLXSW_ITEM32(reg, pmdr, lane5_physical_rx, 0x14, 15, 3);

/* lane4_physical_rx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane4_physical_rx, 0x16, 4, 3);

/* lane3_physical_rx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane3_physical_rx, 0x16, 1, 3);

/* lane2_physical_rx
 * Access: RO
 */
MLXSW_ITEM16(reg, pmdr, lane2_physical_rx, 0x16, 6, 3);

/* lane1_physical_rx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane1_physical_rx, 0x17, 3, 3);

/* lane0_physical_rx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane0_physical_rx, 0x17, 0, 3);

/* pll_index
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, pll_index, 0x18, 0, 8);

/* lane7_physical_tx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane7_physical_tx, 0x19, 5, 3);

/* lane6_physical_tx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane6_physical_tx, 0x19, 2, 3);

/* lane5_physical_tx
 * Access: RO
 */
MLXSW_ITEM32(reg, pmdr, lane5_physical_tx, 0x18, 15, 3);

/* lane4_physical_tx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane4_physical_tx, 0x1a, 4, 3);

/* lane3_physical_tx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane3_physical_tx, 0x1a, 1, 3);

/* lane2_physical_tx
 * Access: RO
 */
MLXSW_ITEM16(reg, pmdr, lane2_physical_tx, 0x1a, 6, 3);

/* lane1_physical_tx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane1_physical_tx, 0x1b, 3, 3);

/* lane0_physical_tx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane0_physical_tx, 0x1b, 0, 3);

/* vl7_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl7_lane_map, 0x1c, 4, 4);

/* vl6_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl6_lane_map, 0x1c, 0, 4);

/* vl5_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl5_lane_map, 0x1d, 4, 4);

/* vl4_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl4_lane_map, 0x1d, 0, 4);

/* vl3_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl3_lane_map, 0x1e, 4, 4);

/* vl2_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl2_lane_map, 0x1e, 0, 4);

/* vl1_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl1_lane_map, 0x1f, 4, 4);

/* vl0_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl0_lane_map, 0x1f, 0, 4);

/* vl15_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl15_lane_map, 0x20, 4, 4);

/* vl14_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl14_lane_map, 0x20, 0, 4);

/* vl13_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl13_lane_map, 0x21, 4, 4);

/* vl12_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl12_lane_map, 0x21, 0, 4);

/* vl11_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl11_lane_map, 0x22, 4, 4);

/* vl10_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl10_lane_map, 0x22, 0, 4);

/* vl9_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl9_lane_map, 0x23, 4, 4);

/* vl8_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl8_lane_map, 0x23, 0, 4);

/* vl23_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl23_lane_map, 0x24, 4, 4);

/* vl22_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl22_lane_map, 0x24, 0, 4);

/* vl21_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl21_lane_map, 0x25, 4, 4);

/* vl20_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl20_lane_map, 0x25, 0, 4);

/* vl19_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl19_lane_map, 0x26, 4, 4);

/* vl18_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl18_lane_map, 0x26, 0, 4);

/* vl17_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl17_lane_map, 0x27, 4, 4);

/* vl16_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl16_lane_map, 0x27, 0, 4);

/* vl31_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl31_lane_map, 0x28, 4, 4);

/* vl30_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl30_lane_map, 0x28, 0, 4);

/* vl29_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl29_lane_map, 0x29, 4, 4);

/* vl28_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl28_lane_map, 0x29, 0, 4);

/* vl27_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl27_lane_map, 0x2a, 4, 4);

/* vl26_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl26_lane_map, 0x2a, 0, 4);

/* vl25_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl25_lane_map, 0x2b, 4, 4);

/* vl24_lane_map
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, vl24_lane_map, 0x2b, 0, 4);

/* physical_tx_cluster
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, physical_tx_cluster, 0x2c, 0, 8);

/* physical_rx_cluster
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, physical_rx_cluster, 0x2d, 0, 8);

/* scc_hs_lanes
 * Access: RO
 */
MLXSW_ITEM16(reg, pmdr, scc_hs_lanes, 0x2e, 0, 16);

/* lane7_ph_tx_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane7_ph_tx_msb, 0x30, 6, 2);

/* lane6_ph_tx_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane6_ph_tx_msb, 0x30, 4, 2);

/* lane5_ph_tx_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane5_ph_tx_msb, 0x30, 2, 2);

/* lane4_ph_tx_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane4_ph_tx_msb, 0x30, 0, 2);

/* lane3_ph_tx_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane3_ph_tx_msb, 0x31, 6, 2);

/* lane2_ph_tx_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane2_ph_tx_msb, 0x31, 4, 2);

/* lane1_ph_tx_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane1_ph_tx_msb, 0x31, 2, 2);

/* lane0_ph_tx_msb
 * Access: RW
 */
MLXSW_ITEM8(reg, pmdr, lane0_ph_tx_msb, 0x31, 0, 2);

/* lane7_ph_rx_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane7_ph_rx_msb, 0x32, 6, 2);

/* lane6_ph_rx_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane6_ph_rx_msb, 0x32, 4, 2);

/* lane5_ph_rx_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane5_ph_rx_msb, 0x32, 2, 2);

/* lane4_ph_rx_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane4_ph_rx_msb, 0x32, 0, 2);

/* lane3_ph_rx_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane3_ph_rx_msb, 0x33, 6, 2);

/* lane2_ph_rx_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane2_ph_rx_msb, 0x33, 4, 2);

/* lane1_ph_rx_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane1_ph_rx_msb, 0x33, 2, 2);

/* lane0_ph_rx_msb
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane0_ph_rx_msb, 0x33, 0, 2);

/* ph_cluster_tx_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, ph_cluster_tx_cnt, 0x34, 4, 3);

/* ph_cluster_rx_cnt
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, ph_cluster_rx_cnt, 0x34, 0, 3);

/* lane7_cluster_tx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane7_cluster_tx, 0x35, 5, 3);

/* lane6_cluster_tx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane6_cluster_tx, 0x35, 2, 3);

/* lane5_cluster_tx
 * Access: RO
 */
MLXSW_ITEM32(reg, pmdr, lane5_cluster_tx, 0x34, 15, 3);

/* lane4_cluster_tx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane4_cluster_tx, 0x36, 4, 3);

/* lane3_cluster_tx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane3_cluster_tx, 0x36, 1, 3);

/* lane2_cluster_tx
 * Access: RO
 */
MLXSW_ITEM16(reg, pmdr, lane2_cluster_tx, 0x36, 6, 3);

/* lane1_cluster_tx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane1_cluster_tx, 0x37, 3, 3);

/* lane0_cluster_tx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane0_cluster_tx, 0x37, 0, 3);

/* pll_index_rx
 * Access: RW
 */
MLXSW_ITEM8(reg, pmdr, pll_index_rx, 0x38, 0, 8);

/* lane7_cluster_rx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane7_cluster_rx, 0x39, 5, 3);

/* lane6_cluster_rx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane6_cluster_rx, 0x39, 2, 3);

/* lane5_cluster_rx
 * Access: RO
 */
MLXSW_ITEM32(reg, pmdr, lane5_cluster_rx, 0x38, 15, 3);

/* lane4_cluster_rx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane4_cluster_rx, 0x3a, 4, 3);

/* lane3_cluster_rx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane3_cluster_rx, 0x3a, 1, 3);

/* lane2_cluster_rx
 * Access: RO
 */
MLXSW_ITEM16(reg, pmdr, lane2_cluster_rx, 0x3a, 6, 3);

/* lane1_cluster_rx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane1_cluster_rx, 0x3b, 3, 3);

/* lane0_cluster_rx
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, lane0_cluster_rx, 0x3b, 0, 3);

/* module_lane_mask_msb
 * Access: RO
 */
MLXSW_ITEM32(reg, pmdr, module_lane_mask_msb, 0x3c, 8, 24);

/* oe_lane0_to_els_logical_laser
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, oe_lane0_to_els_logical_laser, 0x40, 4, 4);

/* oe_lane1_to_els_logical_laser
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, oe_lane1_to_els_logical_laser, 0x40, 0, 4);

/* oe_lane2_to_els_logical_laser
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, oe_lane2_to_els_logical_laser, 0x41, 4, 4);

/* oe_lane3_to_els_logical_laser
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, oe_lane3_to_els_logical_laser, 0x41, 0, 4);

/* oe_lane4_to_els_logical_laser
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, oe_lane4_to_els_logical_laser, 0x42, 4, 4);

/* oe_lane5_to_els_logical_laser
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, oe_lane5_to_els_logical_laser, 0x42, 0, 4);

/* oe_lane6_to_els_logical_laser
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, oe_lane6_to_els_logical_laser, 0x43, 4, 4);

/* oe_lane7_to_els_logical_laser
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, oe_lane7_to_els_logical_laser, 0x43, 0, 4);

/* cpo_indication
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, cpo_indication, 0x44, 7, 1);

/* oe_mcu_index
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, oe_mcu_index, 0x44, 0, 7);

/* els_index
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, els_index, 0x46, 2, 6);

/* els_module_index
 * Access: RO
 */
MLXSW_ITEM8(reg, pmdr, els_module_index, 0x47, 0, 8);

/* pphcr
 * -----
 *
 */
/* active_hist_type
 * Access: RO
 */
MLXSW_ITEM8(reg, pphcr, active_hist_type, 0x0, 4, 4);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pphcr, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pphcr, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pphcr, lp_msb, 0x2, 4, 2);

/* port_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pphcr, port_type, 0x2, 0, 4);

/* plane_ind
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pphcr, plane_ind, 0x3, 4, 4);

/* we
 * Access: RO
 */
MLXSW_ITEM8(reg, pphcr, we, 0x3, 0, 1);

/* num_of_bins
 * Access: RO
 */
MLXSW_ITEM8(reg, pphcr, num_of_bins, 0x5, 0, 8);

/* hist_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pphcr, hist_type, 0x7, 0, 4);

/* hist_max_measurement
 * Access: RO
 */
MLXSW_ITEM8(reg, pphcr, hist_max_measurement, 0x9, 0, 8);

/* hist_min_measurement
 * Access: RO
 */
MLXSW_ITEM8(reg, pphcr, hist_min_measurement, 0xb, 0, 8);

/* bin_range_write_mask
 * Access: WO
 */
MLXSW_ITEM16(reg, pphcr, bin_range_write_mask, 0xe, 0, 16);

/* high_val
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, pphcr, high_val, MLXSW_PPHCR_BIN_RANGE_BASE_LEN + 1, 0, 6, MLXSW_PPHCR_BIN_RANGE_REC_LEN, 0, 1, 0);

/* low_val
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, pphcr, low_val, MLXSW_PPHCR_BIN_RANGE_BASE_LEN + 3, 0, 6, MLXSW_PPHCR_BIN_RANGE_REC_LEN, 0, 1, 0);

/* pcnr
 * ----
 *
 */
/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, pcnr, status, 0x0, 4, 4);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pcnr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pcnr, lp_msb, 0x2, 4, 2);

/* flua_we_mask
 * Access: WO
 */
MLXSW_ITEM8(reg, pcnr, flua_we_mask, 0x3, 3, 4);

/* flua_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pcnr, flua_cap, 0x3, 2, 1);

/* force_fast_link_up
 * Access: WO
 */
MLXSW_ITEM8(reg, pcnr, force_fast_link_up, 0x3, 1, 1);

/* tuning_override
 * Access: WO
 */
MLXSW_ITEM8(reg, pcnr, tuning_override, 0x3, 0, 1);

/* flua_guar_sec
 * Access: RW
 */
MLXSW_ITEM8(reg, pcnr, flua_guar_sec, 0x4, 0, 5);

/* flua_polling_sec
 * Access: RW
 */
MLXSW_ITEM8(reg, pcnr, flua_polling_sec, 0x5, 0, 8);

/* flua_allowed_ber
 * Access: RW
 */
MLXSW_ITEM8(reg, pcnr, flua_allowed_ber, 0x6, 0, 8);

/* flua_oper
 * Access: RO
 */
MLXSW_ITEM8(reg, pcnr, flua_oper, 0x7, 6, 2);

/* flua_en
 * Access: RW
 */
MLXSW_ITEM8(reg, pcnr, flua_en, 0x7, 4, 2);

/* flu_disable
 * Access: RW
 */
MLXSW_ITEM8(reg, pcnr, flu_disable, 0x7, 3, 1);

/* flot_invalidation
 * Access: RW
 */
MLXSW_ITEM8(reg, pcnr, flot_invalidation, 0x7, 2, 1);

/* flot_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pcnr, flot_cap, 0x7, 1, 1);

/* keep_phy_setting
 * Access: WO
 */
MLXSW_ITEM8(reg, pcnr, keep_phy_setting, 0x7, 0, 1);

/* sllm
 * ----
 *
 */
/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, sllm, status, 0x0, 4, 4);

/* version
 * Access: RO
 */
MLXSW_ITEM8(reg, sllm, version, 0x0, 0, 4);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sllm, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sllm, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sllm, lp_msb, 0x2, 4, 2);

/* lane
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sllm, lane, 0x2, 0, 4);

/* port_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sllm, port_type, 0x3, 4, 4);

/* br_lanes_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, sllm, br_lanes_cap, 0x3, 2, 1);

/* br_lanes
 * Access: WO
 */
MLXSW_ITEM8(reg, sllm, br_lanes, 0x3, 1, 1);

/* c_db
 * Access: OP
 */
MLXSW_ITEM8(reg, sllm, c_db, 0x3, 0, 1);

/* lm_active
 * Access: RO
 */
MLXSW_ITEM8(reg, sllm, lm_active, 0x4, 7, 1);

/* lm_was_active
 * Access: RO
 */
MLXSW_ITEM8(reg, sllm, lm_was_active, 0x4, 6, 1);

/* pib_gw_lock
 * Access: RO
 */
MLXSW_ITEM8(reg, sllm, pib_gw_lock, 0x4, 5, 1);

/* lm_en
 * Access: RW
 */
MLXSW_ITEM8(reg, sllm, lm_en, 0x5, 0, 2);

/* lm_clk90_fl_err_max
 * Access: RW
 */
MLXSW_ITEM16(reg, sllm, lm_clk90_fl_err_max, 0x6, 0, 16);

/* lm_clk90_fl_err_acc
 * Access: RW
 */
MLXSW_ITEM32(reg, sllm, lm_clk90_fl_err_acc, 0x8, 0, 32);

/* ib3_max_lm_90_tries
 * Access: RW
 */
MLXSW_ITEM16(reg, sllm, ib3_max_lm_90_tries, 0xc, 6, 10);

/* lm_counter_up
 * Access: RW
 */
MLXSW_ITEM32(reg, sllm, lm_counter_up, 0xc, 0, 20);

/* ib1_max_lm_90_tries
 * Access: RW
 */
MLXSW_ITEM16(reg, sllm, ib1_max_lm_90_tries, 0x10, 6, 10);

/* lm_counter_mid
 * Access: RW
 */
MLXSW_ITEM32(reg, sllm, lm_counter_mid, 0x10, 0, 20);

/* lm_counter_dn
 * Access: RW
 */
MLXSW_ITEM32(reg, sllm, lm_counter_dn, 0x14, 0, 20);

/* enable_lm
 * Access: RW
 */
MLXSW_ITEM8(reg, sllm, enable_lm, 0x4, 7, 1);

/* enable_phase_maintenance
 * Access: RW
 */
MLXSW_ITEM8(reg, sllm, enable_phase_maintenance, 0x4, 6, 1);

/* enable_offset_maintenance
 * Access: RW
 */
MLXSW_ITEM8(reg, sllm, enable_offset_maintenance, 0x4, 5, 1);

/* lm_catastrophic_bit
 * Access: RO
 */
MLXSW_ITEM8(reg, sllm, lm_catastrophic_bit, 0x4, 0, 1);

/* lm_activation_counter
 * Access: RO
 */
MLXSW_ITEM8(reg, sllm, lm_activation_counter, 0x6, 0, 6);

/* lm_fixes_counter
 * Access: RO
 */
MLXSW_ITEM8(reg, sllm, lm_fixes_counter, 0x7, 0, 6);

/* lane_under_maintenance
 * Access: RO
 */
MLXSW_ITEM8(reg, sllm, lane_under_maintenance, 0x9, 0, 6);

/* optimal_phase
 * Access: RO
 */
MLXSW_ITEM8(reg, sllm, optimal_phase, 0xa, 0, 6);

/* link_tuning_error
 * Access: RO
 */
MLXSW_ITEM8(reg, sllm, link_tuning_error, 0xb, 0, 8);

/* lane_correctable_errors
 * Access: RO
 */
MLXSW_ITEM32(reg, sllm, lane_correctable_errors, 0xc, 0, 32);

/* sllm_7nm_peq_interval_period
 * Access: RW
 */
MLXSW_ITEM16(reg, sllm, sllm_7nm_peq_interval_period, 0x4, 0, 12);

/* sllm_7nm_peq_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, sllm, sllm_7nm_peq_cap, 0x7, 1, 1);

/* sllm_7nm_ctle_peq_en
 * Access: RW
 */
MLXSW_ITEM8(reg, sllm, sllm_7nm_ctle_peq_en, 0x7, 0, 1);

/* sllm_7nm_peq_tsense_en
 * Access: RW
 */
MLXSW_ITEM8(reg, sllm, sllm_7nm_peq_tsense_en, 0x8, 7, 1);

/* sllm_7nm_peq_f1_adapt_skip
 * Access: RW
 */
MLXSW_ITEM8(reg, sllm, sllm_7nm_peq_f1_adapt_skip, 0x8, 6, 1);

/* sllm_7nm_vref_peq_en
 * Access: RW
 */
MLXSW_ITEM8(reg, sllm, sllm_7nm_vref_peq_en, 0x8, 5, 1);

/* sllm_7nm_dffe_peq_scout_skip
 * Access: RW
 */
MLXSW_ITEM8(reg, sllm, sllm_7nm_dffe_peq_scout_skip, 0x8, 4, 1);

/* sllm_7nm_peq_train_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, sllm, sllm_7nm_peq_train_mode, 0x9, 4, 4);

/* sllm_7nm_peq_vref_iters
 * Access: RW
 */
MLXSW_ITEM8(reg, sllm, sllm_7nm_peq_vref_iters, 0xb, 4, 4);

/* sllm_7nm_peq_adc_vref_step
 * Access: RW
 */
MLXSW_ITEM8(reg, sllm, sllm_7nm_peq_adc_vref_step, 0xd, 4, 2);

/* sllm_7nm_dffe_peq_en
 * Access: RW
 */
MLXSW_ITEM8(reg, sllm, sllm_7nm_dffe_peq_en, 0xd, 0, 1);

/* sllm_7nm_peq_dffe_iters
 * Access: RW
 */
MLXSW_ITEM8(reg, sllm, sllm_7nm_peq_dffe_iters, 0xf, 4, 4);

/* sllm_7nm_peq_dffe_delay
 * Access: RW
 */
MLXSW_ITEM8(reg, sllm, sllm_7nm_peq_dffe_delay, 0xf, 0, 4);

/* sllm_7nm_ctle_peq_cnt
 * Access: RO
 */
MLXSW_ITEM16(reg, sllm, sllm_7nm_ctle_peq_cnt, 0x10, 0, 16);

/* sllm_7nm_ber_mon_mantissa
 * Access: RO
 */
MLXSW_ITEM8(reg, sllm, sllm_7nm_ber_mon_mantissa, 0x12, 0, 4);

/* sllm_7nm_ber_mon_exp
 * Access: RO
 */
MLXSW_ITEM8(reg, sllm, sllm_7nm_ber_mon_exp, 0x13, 0, 8);

/* sllm_5nm_peq_cap
 * Access: RW
 */
MLXSW_ITEM8(reg, sllm, sllm_5nm_peq_cap, 0x7, 0, 1);

/* sllm_5nm_peq_interval_period
 * Access: RW
 */
MLXSW_ITEM16(reg, sllm, sllm_5nm_peq_interval_period, 0xa, 0, 12);

/* sllm_5nm_peq_interval_period_oper
 * Access: RO
 */
MLXSW_ITEM16(reg, sllm, sllm_5nm_peq_interval_period_oper, 0xe, 0, 12);

/* ppcgp
 * -----
 *
 */
/* grp_profile
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppcgp, grp_profile, 0x3, 0, 3);

/* hi
 * Access: 
 */
MLXSW_ITEM32(reg, ppcgp, hi, 0x4, 0, 32);

/* lo
 * Access: 
 */
MLXSW_ITEM32(reg, ppcgp, lo, 0x8, 0, 32);

/* pcsr
 * ----
 *
 */
/* gs
 * Access: RO
 */
MLXSW_ITEM8(reg, pcsr, gs, 0x0, 7, 1);

/* offset
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pcsr, offset, 0x3, 0, 4);

/* port_status_mask
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, pcsr, port_status_mask, 0x4, 0, 32, 4, 0, 4, 0);

/* ppbme
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppbme, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppbme, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppbme, lp_msb, 0x2, 4, 2);

/* monitor_type
 * Access: RO
 */
MLXSW_ITEM8(reg, ppbme, monitor_type, 0x2, 0, 4);

/* port_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppbme, port_type, 0x3, 4, 4);

/* ievent
 * Access: OP
 */
MLXSW_ITEM8(reg, ppbme, ievent, 0x4, 5, 1);

/* monitor_type_ext
 * Access: RO
 */
MLXSW_ITEM8(reg, ppbme, monitor_type_ext, 0x6, 4, 4);

/* monitor_state
 * Access: RO
 */
MLXSW_ITEM8(reg, ppbme, monitor_state, 0x7, 0, 8);

/* peer_monitor_state
 * Access: RO
 */
MLXSW_ITEM8(reg, ppbme, peer_monitor_state, 0xb, 0, 8);

/* prtl
 * ----
 *
 */
/* rtt_support
 * Access: RO
 */
MLXSW_ITEM8(reg, prtl, rtt_support, 0x0, 7, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, prtl, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, prtl, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, prtl, lp_msb, 0x2, 4, 2);

/* plane_ind
 * Access: INDEX
 */
MLXSW_ITEM8(reg, prtl, plane_ind, 0x2, 0, 4);

/* latency_res
 * Access: RO
 */
MLXSW_ITEM8(reg, prtl, latency_res, 0x5, 0, 4);

/* latency_accuracy
 * Access: RO
 */
MLXSW_ITEM8(reg, prtl, latency_accuracy, 0x7, 0, 8);

/* local_phy_latency
 * Access: RO
 */
MLXSW_ITEM16(reg, prtl, local_phy_latency, 0xa, 0, 16);

/* local_mod_dp_latency
 * Access: RO
 */
MLXSW_ITEM16(reg, prtl, local_mod_dp_latency, 0xe, 0, 16);

/* round_trip_latency
 * Access: RO
 */
MLXSW_ITEM32(reg, prtl, round_trip_latency, 0x10, 0, 24);

/* pemi
 * ----
 *
 */
/* clr_statistic
 * Access: OP
 */
MLXSW_ITEM8(reg, pemi, clr_statistic, 0x0, 7, 1);

/* module_index_cap
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pemi, module_index_cap, 0x0, 2, 1);

/* module_ind_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pemi, module_ind_type, 0x0, 0, 2);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pemi, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pemi, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pemi, lp_msb, 0x2, 4, 2);

/* module_info_ext
 * Access: OP
 */
MLXSW_ITEM8(reg, pemi, module_info_ext, 0x2, 1, 2);

/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, status, 0x3, 0, 8);

/* group_cap_mask
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, group_cap_mask, 0x4, 0, 16);

/* page_select
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pemi, page_select, 0x7, 0, 5);

/* pemi_module_status_samples_temperature
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_temperature, 0x8, 0, 16);

/* pemi_module_status_samples_voltage
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_voltage, 0xa, 0, 16);

/* pemi_module_status_samples_rx_power_lane0
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_rx_power_lane0, 0xc, 0, 16);

/* pemi_module_status_samples_rx_power_lane1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_rx_power_lane1, 0xe, 0, 16);

/* pemi_module_status_samples_rx_power_lane2
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_rx_power_lane2, 0x10, 0, 16);

/* pemi_module_status_samples_rx_power_lane3
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_rx_power_lane3, 0x12, 0, 16);

/* pemi_module_status_samples_rx_power_lane4
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_rx_power_lane4, 0x14, 0, 16);

/* pemi_module_status_samples_rx_power_lane5
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_rx_power_lane5, 0x16, 0, 16);

/* pemi_module_status_samples_rx_power_lane6
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_rx_power_lane6, 0x18, 0, 16);

/* pemi_module_status_samples_rx_power_lane7
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_rx_power_lane7, 0x1a, 0, 16);

/* pemi_module_status_samples_tx_power_lane0
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_tx_power_lane0, 0x1c, 0, 16);

/* pemi_module_status_samples_tx_power_lane1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_tx_power_lane1, 0x1e, 0, 16);

/* pemi_module_status_samples_tx_power_lane2
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_tx_power_lane2, 0x20, 0, 16);

/* pemi_module_status_samples_tx_power_lane3
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_tx_power_lane3, 0x22, 0, 16);

/* pemi_module_status_samples_tx_power_lane4
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_tx_power_lane4, 0x24, 0, 16);

/* pemi_module_status_samples_tx_power_lane5
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_tx_power_lane5, 0x26, 0, 16);

/* pemi_module_status_samples_tx_power_lane6
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_tx_power_lane6, 0x28, 0, 16);

/* pemi_module_status_samples_tx_power_lane7
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_tx_power_lane7, 0x2a, 0, 16);

/* pemi_module_status_samples_tx_bias_lane0
 * Access: RO
 */
MLXSW_ITEM32(reg, pemi, pemi_module_status_samples_tx_bias_lane0, 0x2c, 0, 32);

/* pemi_module_status_samples_tx_bias_lane1
 * Access: RO
 */
MLXSW_ITEM32(reg, pemi, pemi_module_status_samples_tx_bias_lane1, 0x30, 0, 32);

/* pemi_module_status_samples_tx_bias_lane2
 * Access: RO
 */
MLXSW_ITEM32(reg, pemi, pemi_module_status_samples_tx_bias_lane2, 0x34, 0, 32);

/* pemi_module_status_samples_tx_bias_lane3
 * Access: RO
 */
MLXSW_ITEM32(reg, pemi, pemi_module_status_samples_tx_bias_lane3, 0x38, 0, 32);

/* pemi_module_status_samples_tx_bias_lane4
 * Access: RO
 */
MLXSW_ITEM32(reg, pemi, pemi_module_status_samples_tx_bias_lane4, 0x3c, 0, 32);

/* pemi_module_status_samples_tx_bias_lane5
 * Access: RO
 */
MLXSW_ITEM32(reg, pemi, pemi_module_status_samples_tx_bias_lane5, 0x40, 0, 32);

/* pemi_module_status_samples_tx_bias_lane6
 * Access: RO
 */
MLXSW_ITEM32(reg, pemi, pemi_module_status_samples_tx_bias_lane6, 0x44, 0, 32);

/* pemi_module_status_samples_tx_bias_lane7
 * Access: RO
 */
MLXSW_ITEM32(reg, pemi, pemi_module_status_samples_tx_bias_lane7, 0x48, 0, 32);

/* pemi_module_status_samples_module_st
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, pemi_module_status_samples_module_st, 0x4c, 5, 3);

/* pemi_module_status_samples_dp_st_lane0
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, pemi_module_status_samples_dp_st_lane0, 0x4c, 1, 4);

/* pemi_module_status_samples_dp_st_lane1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_dp_st_lane1, 0x4c, 5, 4);

/* pemi_module_status_samples_dp_st_lane2
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, pemi_module_status_samples_dp_st_lane2, 0x4d, 1, 4);

/* pemi_module_status_samples_dp_st_lane3
 * Access: RO
 */
MLXSW_ITEM32(reg, pemi, pemi_module_status_samples_dp_st_lane3, 0x4c, 13, 4);

/* pemi_module_status_samples_dp_st_lane4
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, pemi_module_status_samples_dp_st_lane4, 0x4e, 1, 4);

/* pemi_module_status_samples_dp_st_lane5
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_dp_st_lane5, 0x4e, 5, 4);

/* pemi_module_status_samples_dp_st_lane6
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, pemi_module_status_samples_dp_st_lane6, 0x4f, 1, 4);

/* pemi_module_status_samples_dp_st_lane7
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, pemi_module_status_samples_dp_st_lane7, 0x50, 4, 4);

/* pemi_module_status_samples_max_tec_power
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_max_tec_power, 0x52, 0, 16);

/* pemi_module_status_samples_laser_source_temperature
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_module_status_samples_laser_source_temperature, 0x54, 0, 16);

/* snr_media_lane0
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_media_lane0, 0x8, 0, 16);

/* snr_media_lane1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_media_lane1, 0xa, 0, 16);

/* snr_media_lane2
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_media_lane2, 0xc, 0, 16);

/* snr_media_lane3
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_media_lane3, 0xe, 0, 16);

/* snr_media_lane4
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_media_lane4, 0x10, 0, 16);

/* snr_media_lane5
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_media_lane5, 0x12, 0, 16);

/* snr_media_lane6
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_media_lane6, 0x14, 0, 16);

/* snr_media_lane7
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_media_lane7, 0x16, 0, 16);

/* snr_host_lane0
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_host_lane0, 0x18, 0, 16);

/* snr_host_lane1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_host_lane1, 0x1a, 0, 16);

/* snr_host_lane2
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_host_lane2, 0x1c, 0, 16);

/* snr_host_lane3
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_host_lane3, 0x1e, 0, 16);

/* snr_host_lane4
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_host_lane4, 0x20, 0, 16);

/* snr_host_lane5
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_host_lane5, 0x22, 0, 16);

/* snr_host_lane6
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_host_lane6, 0x24, 0, 16);

/* snr_host_lane7
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_host_lane7, 0x26, 0, 16);

/* laser_age_lane0
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_age_lane0, 0x8, 0, 16);

/* laser_age_lane1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_age_lane1, 0xa, 0, 16);

/* laser_age_lane2
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_age_lane2, 0xc, 0, 16);

/* laser_age_lane3
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_age_lane3, 0xe, 0, 16);

/* laser_age_lane4
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_age_lane4, 0x10, 0, 16);

/* laser_age_lane5
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_age_lane5, 0x12, 0, 16);

/* laser_age_lane6
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_age_lane6, 0x14, 0, 16);

/* laser_age_lane7
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_age_lane7, 0x16, 0, 16);

/* tec_current_laser0
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_current_laser0, 0x18, 0, 16);

/* tec_current_laser1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_current_laser1, 0x1a, 0, 16);

/* tec_current_laser2
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_current_laser2, 0x1c, 0, 16);

/* tec_current_laser3
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_current_laser3, 0x1e, 0, 16);

/* tec_current_laser4
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_current_laser4, 0x20, 0, 16);

/* tec_current_laser5
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_current_laser5, 0x22, 0, 16);

/* tec_current_laser6
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_current_laser6, 0x24, 0, 16);

/* tec_current_laser7
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_current_laser7, 0x26, 0, 16);

/* els_input_power_lane0
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, els_input_power_lane0, 0x28, 0, 16);

/* els_input_power_lane1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, els_input_power_lane1, 0x2a, 0, 16);

/* els_input_power_lane2
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, els_input_power_lane2, 0x2c, 0, 16);

/* els_input_power_lane3
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, els_input_power_lane3, 0x2e, 0, 16);

/* els_input_power_lane4
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, els_input_power_lane4, 0x30, 0, 16);

/* els_input_power_lane5
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, els_input_power_lane5, 0x32, 0, 16);

/* els_input_power_lane6
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, els_input_power_lane6, 0x34, 0, 16);

/* els_input_power_lane7
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, els_input_power_lane7, 0x36, 0, 16);

/* laser_frequency_error_lane0
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_frequency_error_lane0, 0x38, 0, 16);

/* laser_frequency_error_lane1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_frequency_error_lane1, 0x3a, 0, 16);

/* laser_frequency_error_lane2
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_frequency_error_lane2, 0x3c, 0, 16);

/* laser_frequency_error_lane3
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_frequency_error_lane3, 0x3e, 0, 16);

/* laser_frequency_error_lane4
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_frequency_error_lane4, 0x40, 0, 16);

/* laser_frequency_error_lane5
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_frequency_error_lane5, 0x42, 0, 16);

/* laser_frequency_error_lane6
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_frequency_error_lane6, 0x44, 0, 16);

/* laser_frequency_error_lane7
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_frequency_error_lane7, 0x46, 0, 16);

/* cooled_laser_temperature_lane0
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, cooled_laser_temperature_lane0, 0x48, 0, 16);

/* cooled_laser_temperature_lane1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, cooled_laser_temperature_lane1, 0x4a, 0, 16);

/* cooled_laser_temperature_lane2
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, cooled_laser_temperature_lane2, 0x4c, 0, 16);

/* cooled_laser_temperature_lane3
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, cooled_laser_temperature_lane3, 0x4e, 0, 16);

/* cooled_laser_temperature_lane4
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, cooled_laser_temperature_lane4, 0x50, 0, 16);

/* cooled_laser_temperature_lane5
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, cooled_laser_temperature_lane5, 0x52, 0, 16);

/* cooled_laser_temperature_lane6
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, cooled_laser_temperature_lane6, 0x54, 0, 16);

/* cooled_laser_temperature_lane7
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, cooled_laser_temperature_lane7, 0x56, 0, 16);

/* els_laser_temperature
 * Access: RW
 */
MLXSW_ITEM16(reg, pemi, els_laser_temperature, 0x58, 0, 16);

/* els_laser2_temperature
 * Access: RW
 */
MLXSW_ITEM16(reg, pemi, els_laser2_temperature, 0x5a, 0, 16);

/* pam4_level_transition_media_lane0
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_media_lane0, 0x8, 0, 16);

/* pam4_level_transition_media_lane1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_media_lane1, 0xa, 0, 16);

/* pam4_level_transition_media_lane2
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_media_lane2, 0xc, 0, 16);

/* pam4_level_transition_media_lane3
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_media_lane3, 0xe, 0, 16);

/* pam4_level_transition_media_lane4
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_media_lane4, 0x10, 0, 16);

/* pam4_level_transition_media_lane5
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_media_lane5, 0x12, 0, 16);

/* pam4_level_transition_media_lane6
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_media_lane6, 0x14, 0, 16);

/* pam4_level_transition_media_lane7
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_media_lane7, 0x16, 0, 16);

/* pam4_level_transition_host_lane0
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_host_lane0, 0x18, 0, 16);

/* pam4_level_transition_host_lane1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_host_lane1, 0x1a, 0, 16);

/* pam4_level_transition_host_lane2
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_host_lane2, 0x1c, 0, 16);

/* pam4_level_transition_host_lane3
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_host_lane3, 0x1e, 0, 16);

/* pam4_level_transition_host_lane4
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_host_lane4, 0x20, 0, 16);

/* pam4_level_transition_host_lane5
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_host_lane5, 0x22, 0, 16);

/* pam4_level_transition_host_lane6
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_host_lane6, 0x24, 0, 16);

/* pam4_level_transition_host_lane7
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_host_lane7, 0x26, 0, 16);

/* pre_fec_ber_min_media
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_min_media, 0x8, 0, 16);

/* pre_fec_ber_min_host
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_min_host, 0xa, 0, 16);

/* pre_fec_ber_max_media
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_max_media, 0xc, 0, 16);

/* pre_fec_ber_max_host
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_max_host, 0xe, 0, 16);

/* pre_fec_ber_avg_media
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_avg_media, 0x10, 0, 16);

/* pre_fec_ber_avg_host
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_avg_host, 0x12, 0, 16);

/* pre_fec_ber_val_media
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_val_media, 0x14, 0, 16);

/* pre_fec_ber_val_host
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_val_host, 0x16, 0, 16);

/* ferc_min_media
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, ferc_min_media, 0x8, 0, 16);

/* ferc_min_host
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, ferc_min_host, 0xa, 0, 16);

/* ferc_max_media
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, ferc_max_media, 0xc, 0, 16);

/* ferc_max_host
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, ferc_max_host, 0xe, 0, 16);

/* ferc_avg_media
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, ferc_avg_media, 0x10, 0, 16);

/* ferc_avg_host
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, ferc_avg_host, 0x12, 0, 16);

/* ferc_val_media
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, ferc_val_media, 0x14, 0, 16);

/* ferc_val_host
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, ferc_val_host, 0x16, 0, 16);

/* pemi_temperature_samples_uc_temp
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_temperature_samples_uc_temp, 0xa, 0, 16);

/* pemi_temperature_samples_lasers_temp0
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_temperature_samples_lasers_temp0, 0xc, 0, 16);

/* pemi_temperature_samples_lasers_temp1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_temperature_samples_lasers_temp1, 0xe, 0, 16);

/* pemi_temperature_samples_lasers_temp2
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_temperature_samples_lasers_temp2, 0x10, 0, 16);

/* pemi_temperature_samples_lasers_temp3
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_temperature_samples_lasers_temp3, 0x12, 0, 16);

/* pemi_temperature_samples_lasers_temp4
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_temperature_samples_lasers_temp4, 0x14, 0, 16);

/* pemi_temperature_samples_lasers_temp5
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_temperature_samples_lasers_temp5, 0x16, 0, 16);

/* pemi_temperature_samples_lasers_temp6
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_temperature_samples_lasers_temp6, 0x18, 0, 16);

/* pemi_temperature_samples_lasers_temp7
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_temperature_samples_lasers_temp7, 0x1a, 0, 16);

/* pemi_temperature_samples_cdr_temp0
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_temperature_samples_cdr_temp0, 0x1c, 0, 16);

/* pemi_temperature_samples_cdr_temp1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_temperature_samples_cdr_temp1, 0x1e, 0, 16);

/* pemi_temperature_samples_cdr_temp2
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_temperature_samples_cdr_temp2, 0x20, 0, 16);

/* pemi_temperature_samples_cdr_temp3
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_temperature_samples_cdr_temp3, 0x22, 0, 16);

/* pemi_temperature_samples_tia_temp0
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_temperature_samples_tia_temp0, 0x24, 0, 16);

/* pemi_temperature_samples_tia_temp1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_temperature_samples_tia_temp1, 0x26, 0, 16);

/* pemi_temperature_samples_pcb_sensor_temp0
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_temperature_samples_pcb_sensor_temp0, 0x28, 0, 16);

/* pemi_temperature_samples_pcb_sensor_temp1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_temperature_samples_pcb_sensor_temp1, 0x2a, 0, 16);

/* pemi_temperature_samples_max_tec_power
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pemi_temperature_samples_max_tec_power, 0x2c, 0, 16);

/* voltage_monitor1
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, voltage_monitor1, 0x8, 0, 8);

/* voltage_monitor2
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, voltage_monitor2, 0x9, 0, 8);

/* voltage_monitor3
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, voltage_monitor3, 0xa, 0, 8);

/* voltage_monitor4
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, voltage_monitor4, 0xb, 0, 8);

/* voltage_monitor5
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, voltage_monitor5, 0xc, 0, 8);

/* voltage_monitor6
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, voltage_monitor6, 0xd, 0, 8);

/* voltage_monitor7
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, voltage_monitor7, 0xe, 0, 8);

/* voltage_monitor8
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, voltage_monitor8, 0xf, 0, 8);

/* bias_current_monitor1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, bias_current_monitor1, 0x10, 0, 16);

/* bias_current_monitor2
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, bias_current_monitor2, 0x12, 0, 16);

/* bias_current_monitor3
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, bias_current_monitor3, 0x14, 0, 16);

/* bias_current_monitor4
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, bias_current_monitor4, 0x16, 0, 16);

/* bias_current_monitor5
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, bias_current_monitor5, 0x18, 0, 16);

/* bias_current_monitor6
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, bias_current_monitor6, 0x1a, 0, 16);

/* bias_current_monitor7
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, bias_current_monitor7, 0x1c, 0, 16);

/* bias_current_monitor8
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, bias_current_monitor8, 0x1e, 0, 16);

/* opt_power_monitor1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, opt_power_monitor1, 0x20, 0, 16);

/* opt_power_monitor2
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, opt_power_monitor2, 0x22, 0, 16);

/* opt_power_monitor3
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, opt_power_monitor3, 0x24, 0, 16);

/* opt_power_monitor4
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, opt_power_monitor4, 0x26, 0, 16);

/* opt_power_monitor5
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, opt_power_monitor5, 0x28, 0, 16);

/* opt_power_monitor6
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, opt_power_monitor6, 0x2a, 0, 16);

/* opt_power_monitor7
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, opt_power_monitor7, 0x2c, 0, 16);

/* opt_power_monitor8
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, opt_power_monitor8, 0x2e, 0, 16);

/* icc_monitor
 * Access: RW
 */
MLXSW_ITEM16(reg, pemi, icc_monitor, 0x30, 0, 16);

/* laser_mpd_lane1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_mpd_lane1, 0x8, 0, 16);

/* laser_mpd_lane2
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_mpd_lane2, 0xa, 0, 16);

/* laser_mpd_lane3
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_mpd_lane3, 0xc, 0, 16);

/* laser_mpd_lane4
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_mpd_lane4, 0xe, 0, 16);

/* laser_mpd_lane5
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_mpd_lane5, 0x10, 0, 16);

/* laser_mpd_lane6
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_mpd_lane6, 0x12, 0, 16);

/* laser_mpd_lane7
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_mpd_lane7, 0x14, 0, 16);

/* laser_mpd_lane8
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_mpd_lane8, 0x16, 0, 16);

/* tec_voltage_laser0
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_voltage_laser0, 0x18, 0, 16);

/* tec_voltage_laser1
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_voltage_laser1, 0x1a, 0, 16);

/* tec_voltage_laser2
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_voltage_laser2, 0x1c, 0, 16);

/* tec_voltage_laser3
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_voltage_laser3, 0x1e, 0, 16);

/* tec_voltage_laser4
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_voltage_laser4, 0x20, 0, 16);

/* tec_voltage_laser5
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_voltage_laser5, 0x22, 0, 16);

/* tec_voltage_laser6
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_voltage_laser6, 0x24, 0, 16);

/* tec_voltage_laser7
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_voltage_laser7, 0x26, 0, 16);

/* health_value_tec0
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, health_value_tec0, 0x28, 0, 8);

/* health_value_tec1
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, health_value_tec1, 0x29, 0, 8);

/* health_value_tec2
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, health_value_tec2, 0x2a, 0, 8);

/* health_value_tec3
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, health_value_tec3, 0x2b, 0, 8);

/* health_value_tec4
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, health_value_tec4, 0x2c, 0, 8);

/* health_value_tec5
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, health_value_tec5, 0x2d, 0, 8);

/* health_value_tec6
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, health_value_tec6, 0x2e, 0, 8);

/* health_value_tec7
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, health_value_tec7, 0x2f, 0, 8);

/* health_value_laser0
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, health_value_laser0, 0x30, 0, 8);

/* health_value_laser1
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, health_value_laser1, 0x31, 0, 8);

/* health_value_laser2
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, health_value_laser2, 0x32, 0, 8);

/* health_value_laser3
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, health_value_laser3, 0x33, 0, 8);

/* health_value_laser4
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, health_value_laser4, 0x34, 0, 8);

/* health_value_laser5
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, health_value_laser5, 0x35, 0, 8);

/* health_value_laser6
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, health_value_laser6, 0x36, 0, 8);

/* health_value_laser7
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, health_value_laser7, 0x37, 0, 8);

/* power_consumption
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, power_consumption, 0x38, 0, 16);

/* heater_voltage_lane0
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, heater_voltage_lane0, 0x8, 0, 16);

/* heater_voltage_lane1
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, heater_voltage_lane1, 0xa, 0, 16);

/* heater_voltage_lane2
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, heater_voltage_lane2, 0xc, 0, 16);

/* heater_voltage_lane3
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, heater_voltage_lane3, 0xe, 0, 16);

/* heater_voltage_lane4
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, heater_voltage_lane4, 0x10, 0, 16);

/* heater_voltage_lane5
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, heater_voltage_lane5, 0x12, 0, 16);

/* heater_voltage_lane6
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, heater_voltage_lane6, 0x14, 0, 16);

/* heater_voltage_lane7
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, heater_voltage_lane7, 0x16, 0, 16);

/* photocurrent_lane0
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, photocurrent_lane0, 0x18, 0, 16);

/* photocurrent_lane1
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, photocurrent_lane1, 0x1a, 0, 16);

/* photocurrent_lane2
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, photocurrent_lane2, 0x1c, 0, 16);

/* photocurrent_lane3
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, photocurrent_lane3, 0x1e, 0, 16);

/* photocurrent_lane4
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, photocurrent_lane4, 0x20, 0, 16);

/* photocurrent_lane5
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, photocurrent_lane5, 0x22, 0, 16);

/* photocurrent_lane6
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, photocurrent_lane6, 0x24, 0, 16);

/* photocurrent_lane7
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, photocurrent_lane7, 0x26, 0, 16);

/* tx_cm_cathode_voltage_lane0
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, tx_cm_cathode_voltage_lane0, 0x28, 0, 16);

/* tx_cm_cathode_voltage_lane1
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, tx_cm_cathode_voltage_lane1, 0x2a, 0, 16);

/* tx_cm_cathode_voltage_lane2
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, tx_cm_cathode_voltage_lane2, 0x2c, 0, 16);

/* tx_cm_cathode_voltage_lane3
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, tx_cm_cathode_voltage_lane3, 0x2e, 0, 16);

/* tx_cm_cathode_voltage_lane4
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, tx_cm_cathode_voltage_lane4, 0x30, 0, 16);

/* tx_cm_cathode_voltage_lane5
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, tx_cm_cathode_voltage_lane5, 0x32, 0, 16);

/* tx_cm_cathode_voltage_lane6
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, tx_cm_cathode_voltage_lane6, 0x34, 0, 16);

/* tx_cm_cathode_voltage_lane7
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, tx_cm_cathode_voltage_lane7, 0x36, 0, 16);

/* tx_cm_anode_voltage_lane0
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, tx_cm_anode_voltage_lane0, 0x38, 0, 16);

/* tx_cm_anode_voltage_lane1
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, tx_cm_anode_voltage_lane1, 0x3a, 0, 16);

/* tx_cm_anode_voltage_lane2
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, tx_cm_anode_voltage_lane2, 0x3c, 0, 16);

/* tx_cm_anode_voltage_lane3
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, tx_cm_anode_voltage_lane3, 0x3e, 0, 16);

/* tx_cm_anode_voltage_lane4
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, tx_cm_anode_voltage_lane4, 0x40, 0, 16);

/* tx_cm_anode_voltage_lane5
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, tx_cm_anode_voltage_lane5, 0x42, 0, 16);

/* tx_cm_anode_voltage_lane6
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, tx_cm_anode_voltage_lane6, 0x44, 0, 16);

/* tx_cm_anode_voltage_lane7
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, tx_cm_anode_voltage_lane7, 0x46, 0, 16);

/* rx_voa_xpol_voltage_lane0
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, rx_voa_xpol_voltage_lane0, 0x48, 0, 16);

/* rx_voa_xpol_voltage_lane1
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, rx_voa_xpol_voltage_lane1, 0x4a, 0, 16);

/* rx_voa_xpol_voltage_lane2
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, rx_voa_xpol_voltage_lane2, 0x4c, 0, 16);

/* rx_voa_xpol_voltage_lane3
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, rx_voa_xpol_voltage_lane3, 0x4e, 0, 16);

/* rx_voa_xpol_voltage_lane4
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, rx_voa_xpol_voltage_lane4, 0x50, 0, 16);

/* rx_voa_xpol_voltage_lane5
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, rx_voa_xpol_voltage_lane5, 0x52, 0, 16);

/* rx_voa_xpol_voltage_lane6
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, rx_voa_xpol_voltage_lane6, 0x54, 0, 16);

/* rx_voa_xpol_voltage_lane7
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, rx_voa_xpol_voltage_lane7, 0x56, 0, 16);

/* rx_voa_ypol_voltage_lane0
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, rx_voa_ypol_voltage_lane0, 0x58, 0, 16);

/* rx_voa_ypol_voltage_lane1
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, rx_voa_ypol_voltage_lane1, 0x5a, 0, 16);

/* rx_voa_ypol_voltage_lane2
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, rx_voa_ypol_voltage_lane2, 0x5c, 0, 16);

/* rx_voa_ypol_voltage_lane3
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, rx_voa_ypol_voltage_lane3, 0x5e, 0, 16);

/* rx_voa_ypol_voltage_lane4
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, rx_voa_ypol_voltage_lane4, 0x60, 0, 16);

/* rx_voa_ypol_voltage_lane5
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, rx_voa_ypol_voltage_lane5, 0x62, 0, 16);

/* rx_voa_ypol_voltage_lane6
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, rx_voa_ypol_voltage_lane6, 0x64, 0, 16);

/* rx_voa_ypol_voltage_lane7
 * Access: 
 */
MLXSW_ITEM16(reg, pemi, rx_voa_ypol_voltage_lane7, 0x66, 0, 16);

/* module_status_cap
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, module_status_cap, 0x8, 0, 16);

/* voltage_high_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, voltage_high_alarm, 0xc, 0, 16);

/* voltage_low_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, voltage_low_alarm, 0xe, 0, 16);

/* voltage_high_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, voltage_high_warning, 0x10, 0, 16);

/* voltage_low_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, voltage_low_warning, 0x12, 0, 16);

/* rx_power_high_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, rx_power_high_alarm, 0x14, 0, 16);

/* rx_power_low_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, rx_power_low_alarm, 0x16, 0, 16);

/* rx_power_high_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, rx_power_high_warning, 0x18, 0, 16);

/* rx_power_low_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, rx_power_low_warning, 0x1a, 0, 16);

/* tx_power_high_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tx_power_high_alarm, 0x1c, 0, 16);

/* tx_power_low_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tx_power_low_alarm, 0x1e, 0, 16);

/* tx_power_high_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tx_power_high_warning, 0x20, 0, 16);

/* tx_power_low_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tx_power_low_warning, 0x22, 0, 16);

/* tx_bias_high_alarm
 * Access: RO
 */
MLXSW_ITEM32(reg, pemi, tx_bias_high_alarm, 0x24, 0, 32);

/* tx_bias_low_alarm
 * Access: RO
 */
MLXSW_ITEM32(reg, pemi, tx_bias_low_alarm, 0x28, 0, 32);

/* tx_bias_high_warning
 * Access: RO
 */
MLXSW_ITEM32(reg, pemi, tx_bias_high_warning, 0x2c, 0, 32);

/* tx_bias_low_warning
 * Access: RO
 */
MLXSW_ITEM32(reg, pemi, tx_bias_low_warning, 0x30, 0, 32);

/* temperature_high_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, temperature_high_alarm, 0x34, 0, 16);

/* temperature_low_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, temperature_low_alarm, 0x36, 0, 16);

/* temperature_high_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, temperature_high_warning, 0x38, 0, 16);

/* temperature_low_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, temperature_low_warning, 0x3a, 0, 16);

/* max_tec_power_high_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, max_tec_power_high_alarm, 0x3c, 0, 16);

/* max_tec_power_low_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, max_tec_power_low_alarm, 0x3e, 0, 16);

/* max_tec_power_high_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, max_tec_power_high_warning, 0x40, 0, 16);

/* max_tec_power_low_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, max_tec_power_low_warning, 0x42, 0, 16);

/* snr_cap
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_cap, 0x8, 0, 16);

/* snr_media_low_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_media_low_alarm, 0xc, 0, 16);

/* snr_media_low_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_media_low_warning, 0xe, 0, 16);

/* snr_host_low_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_host_low_alarm, 0x10, 0, 16);

/* snr_host_low_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, snr_host_low_warning, 0x12, 0, 16);

/* laser_monitor_cap
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_monitor_cap, 0x8, 0, 16);

/* laser_age_high_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_age_high_alarm, 0xc, 0, 16);

/* laser_age_high_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_age_high_warning, 0xe, 0, 16);

/* tec_current_high_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_current_high_alarm, 0x10, 0, 16);

/* tec_current_low_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_current_low_alarm, 0x12, 0, 16);

/* tec_current_high_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_current_high_warning, 0x14, 0, 16);

/* tec_current_low_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, tec_current_low_warning, 0x16, 0, 16);

/* laser_frequency_error_high_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_frequency_error_high_alarm, 0x18, 0, 16);

/* laser_frequency_error_high_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_frequency_error_high_warning, 0x1a, 0, 16);

/* cooled_laser_temp_high_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, cooled_laser_temp_high_alarm, 0x1c, 0, 16);

/* cooled_laser_temp_low_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, cooled_laser_temp_low_alarm, 0x1e, 0, 16);

/* cooled_laser_temp_high_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, cooled_laser_temp_high_warning, 0x20, 0, 16);

/* cooled_laser_temp_low_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, cooled_laser_temp_low_warning, 0x22, 0, 16);

/* pam4_level_transition_cap
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_cap, 0x8, 0, 16);

/* pam4_level_transition_media_low_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_media_low_alarm, 0xc, 0, 16);

/* pam4_level_transition_media_low_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_media_low_warning, 0xe, 0, 16);

/* pam4_level_transition_host_low_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_host_low_alarm, 0x10, 0, 16);

/* pam4_level_transition_host_low_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pam4_level_transition_host_low_warning, 0x12, 0, 16);

/* pre_fec_ber_cap
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_cap, 0x8, 0, 16);

/* pre_fec_ber_min_high_media_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_min_high_media_alarm, 0xc, 0, 16);

/* pre_fec_ber_min_high_media_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_min_high_media_warning, 0xe, 0, 16);

/* pre_fec_ber_min_high_host_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_min_high_host_alarm, 0x10, 0, 16);

/* pre_fec_ber_min_high_host_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_min_high_host_warning, 0x12, 0, 16);

/* pre_fec_ber_max_high_media_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_max_high_media_alarm, 0x14, 0, 16);

/* pre_fec_ber_max_high_media_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_max_high_media_warning, 0x16, 0, 16);

/* pre_fec_ber_max_high_host_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_max_high_host_alarm, 0x18, 0, 16);

/* pre_fec_ber_max_high_host_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_max_high_host_warning, 0x1a, 0, 16);

/* pre_fec_ber_avg_high_media_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_avg_high_media_alarm, 0x1c, 0, 16);

/* pre_fec_ber_avg_high_media_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_avg_high_media_warning, 0x1e, 0, 16);

/* pre_fec_ber_avg_high_host_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_avg_high_host_alarm, 0x20, 0, 16);

/* pre_fec_ber_avg_high_host_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, pre_fec_ber_avg_high_host_warning, 0x22, 0, 16);

/* ferc_cap
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, ferc_cap, 0x8, 0, 16);

/* temp_cap
 * Access: RO
 */
MLXSW_ITEM32(reg, pemi, temp_cap, 0x8, 0, 32);

/* temp_high_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, temp_high_alarm, 0xc, 0, 16);

/* temp_low_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, temp_low_alarm, 0xe, 0, 16);

/* temp_high_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, temp_high_warning, 0x10, 0, 16);

/* temp_low_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, temp_low_warning, 0x12, 0, 16);

/* laser_source_essential_cap
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_source_essential_cap, 0x8, 0, 16);

/* laser_bias_high_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_bias_high_alarm, 0xc, 0, 16);

/* laser_bias_low_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_bias_low_alarm, 0xe, 0, 16);

/* laser_bias_high_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_bias_high_warning, 0x10, 0, 16);

/* laser_bias_low_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_bias_low_warning, 0x12, 0, 16);

/* laser_optical_power_high_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_optical_power_high_alarm, 0x14, 0, 16);

/* laser_optical_power_low_alarm
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_optical_power_low_alarm, 0x16, 0, 16);

/* laser_optical_power_high_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_optical_power_high_warning, 0x18, 0, 16);

/* laser_optical_power_low_warning
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_optical_power_low_warning, 0x1a, 0, 16);

/* laser_source_advance_cap
 * Access: RO
 */
MLXSW_ITEM16(reg, pemi, laser_source_advance_cap, 0x8, 0, 16);

/* optical_engine_telemetry_cap
 * Access: RO
 */
MLXSW_ITEM8(reg, pemi, optical_engine_telemetry_cap, 0x8, 0, 8);

/* pmtu
 * ----
 *
 */
/* protocol
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtu, protocol, 0x0, 4, 4);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtu, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtu, lp_msb, 0x2, 4, 2);

/* i_e
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pmtu, i_e, 0x3, 4, 2);

/* itre
 * Access: RW
 */
MLXSW_ITEM8(reg, pmtu, itre, 0x3, 0, 1);

/* max_mtu
 * Access: RO
 */
MLXSW_ITEM16(reg, pmtu, max_mtu, 0x4, 0, 16);

/* admin_mtu
 * Access: RW
 */
MLXSW_ITEM16(reg, pmtu, admin_mtu, 0x8, 0, 16);

/* oper_mtu
 * Access: RO
 */
MLXSW_ITEM16(reg, pmtu, oper_mtu, 0xc, 0, 16);

/* ppad
 * ----
 *
 */
/* single_base_mac
 * Access: OP
 */
MLXSW_ITEM8(reg, ppad, single_base_mac, 0x0, 4, 1);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppad, pnat, 0x0, 2, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppad, lp_msb, 0x0, 0, 2);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppad, local_port, 0x1, 0, 8);

/* mac
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, ppad, mac, 0x2, 6);

/* ppwcc
 * -----
 *
 */
/* polling_interval
 * Access: RW
 */
MLXSW_ITEM16(reg, ppwcc, polling_interval, 0x0, 0, 16);

/* profile_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppwcc, profile_index, 0x3, 0, 8);

/* detect_num
 * Access: RW
 */
MLXSW_ITEM8(reg, ppwcc, detect_num, 0x5, 0, 4);

/* cnt_bitmask
 * Access: RW
 */
MLXSW_ITEM8(reg, ppwcc, cnt_bitmask, 0x7, 0, 8);

/* pause_dura_thr
 * Access: RW
 */
MLXSW_ITEM32(reg, ppwcc, pause_dura_thr, 0x10, 0, 32);

/* tx_frame_thr
 * Access: RW
 */
MLXSW_ITEM8(reg, ppwcc, tx_frame_thr, 0x17, 0, 4);

/* transmit_queue_thr
 * Access: RW
 */
MLXSW_ITEM32(reg, ppwcc, transmit_queue_thr, 0x18, 0, 32);

/* tx_octets_thr
 * Access: RW
 */
MLXSW_ITEM8(reg, ppwcc, tx_octets_thr, 0x1f, 0, 4);

/* ppwpt
 * -----
 *
 */
/* profile_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppwpt, profile_index, 0x3, 0, 8);

/* cmd
 * Access: OP
 */
MLXSW_ITEM8(reg, ppwpt, cmd, 0x5, 0, 2);

/* num_records
 * Access: RW
 */
MLXSW_ITEM16(reg, ppwpt, num_records, 0x6, 0, 9);

/* local_port
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ppwpt, local_port, MLXSW_PPWPT_PFC_WD_RECORD_BASE_LEN, 0, 16, MLXSW_PPWPT_PFC_WD_RECORD_REC_LEN, 0, 2, 0);

/* tc_bitmask
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ppwpt, tc_bitmask, MLXSW_PPWPT_PFC_WD_RECORD_BASE_LEN + 2, 0, 16, MLXSW_PPWPT_PFC_WD_RECORD_REC_LEN, 0, 2, 0);

/* ppwe
 * ----
 *
 */
/* num_rec
 * Access: RO
 */
MLXSW_ITEM16(reg, ppwe, num_rec, 0x2, 0, 12);

/* local_port
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ppwe, local_port, MLXSW_PPWE_RECORD_BASE_LEN, 0, 16, MLXSW_PPWE_RECORD_REC_LEN, 0, 2, 0);

/* tc_bitmask
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ppwe, tc_bitmask, MLXSW_PPWE_RECORD_BASE_LEN + 2, 0, 16, MLXSW_PPWE_RECORD_REC_LEN, 0, 2, 0);

/* pifr_v2
 * -------
 *
 */
/* table_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pifr_v2, table_id, 0x0, 4, 4);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pifr_v2, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pifr_v2, lp_msb, 0x2, 4, 2);

/* port_filter
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, pifr_v2, port_filter, 0x20, 0, 32, 4, 0, 4, 0);

/* port_filter_update_en
 * Access: WO
 */
MLXSW_ITEM32_INDEXED(reg, pifr_v2, port_filter_update_en, 0xa0, 0, 32, 4, 0, 4, 0);

/* pbsr
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pbsr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pbsr, lp_msb, 0x2, 4, 2);

/* buffer_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pbsr, buffer_type, 0x3, 0, 2);

/* clear_wm_buff_mask
 * Access: OP
 */
MLXSW_ITEM16(reg, pbsr, clear_wm_buff_mask, 0x6, 0, 10);

/* clear_wm
 * Access: OP
 */
MLXSW_ITEM8(reg, pbsr, clear_wm, 0x8, 7, 1);

/* used_shared_headroom_buffer
 * Access: RO
 */
MLXSW_ITEM16(reg, pbsr, used_shared_headroom_buffer, 0xa, 0, 16);

/* stat_buffer_watermark
 * Access: RO
 */
MLXSW_ITEM16_INDEXED(reg, pbsr, stat_buffer_watermark, MLXSW_PBSR_STAT_BUFFER_BASE_LEN + 2, 0, 16, MLXSW_PBSR_STAT_BUFFER_REC_LEN, 0, 2, 0);

/* stat_buffer_used_buffer
 * Access: RO
 */
MLXSW_ITEM16_INDEXED(reg, pbsr, stat_buffer_used_buffer, MLXSW_PBSR_STAT_BUFFER_BASE_LEN + 2, 0, 16, MLXSW_PBSR_STAT_BUFFER_REC_LEN, 4, 2, 0);

/* stat_shared_headroom_pool_watermark
 * Access: RO
 */
MLXSW_ITEM16(reg, pbsr, stat_shared_headroom_pool_watermark, 0x5e, 0, 16);

/* stat_shared_headroom_pool_used_buffer
 * Access: RO
 */
MLXSW_ITEM16(reg, pbsr, stat_shared_headroom_pool_used_buffer, 0x62, 0, 16);

/* pllp
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pllp, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pllp, lp_msb, 0x2, 4, 2);

/* label_port
 * Access: RO
 */
MLXSW_ITEM16(reg, pllp, label_port, 0x2, 0, 10);

/* ipil_stat
 * Access: RO
 */
MLXSW_ITEM8(reg, pllp, ipil_stat, 0x4, 0, 4);

/* split_stat
 * Access: RO
 */
MLXSW_ITEM8(reg, pllp, split_stat, 0x5, 0, 4);

/* ipil_num
 * Access: RO
 */
MLXSW_ITEM8(reg, pllp, ipil_num, 0x6, 0, 4);

/* split_num
 * Access: RO
 */
MLXSW_ITEM8(reg, pllp, split_num, 0x7, 0, 4);

/* max_ports_width
 * Access: RO
 */
MLXSW_ITEM8(reg, pllp, max_ports_width, 0x9, 0, 4);

/* slot_num
 * Access: RO
 */
MLXSW_ITEM8(reg, pllp, slot_num, 0xb, 0, 4);

/* maf
 * Access: RO
 */
MLXSW_ITEM8(reg, pllp, maf, 0xd, 1, 1);

/* is_fnm
 * Access: RO
 */
MLXSW_ITEM8(reg, pllp, is_fnm, 0xd, 0, 1);

/* rmt_id
 * Access: RO
 */
MLXSW_ITEM8(reg, pllp, rmt_id, 0xe, 0, 8);

/* conn_type
 * Access: RO
 */
MLXSW_ITEM8(reg, pllp, conn_type, 0xf, 0, 3);

/* protocol
 * Access: RO
 */
MLXSW_ITEM8(reg, pllp, protocol, 0x13, 0, 3);

/* oe_identifier
 * Access: RO
 */
MLXSW_ITEM16(reg, pllp, oe_identifier, 0x14, 0, 16);

/* resource_label_port
 * Access: RO
 */
MLXSW_ITEM16(reg, pllp, resource_label_port, 0x16, 0, 16);

/* ppcr
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppcr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppcr, lp_msb, 0x2, 4, 2);

/* asymmetry_enable_supported
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcr, asymmetry_enable_supported, 0x4, 7, 1);

/* asymmetry_enable
 * Access: RO
 */
MLXSW_ITEM8(reg, ppcr, asymmetry_enable, 0x4, 6, 1);

/* split
 * Access: RW
 */
MLXSW_ITEM8(reg, ppcr, split, 0x8, 0, 3);

/* plane
 * Access: RW
 */
MLXSW_ITEM8(reg, ppcr, plane, 0x9, 0, 3);

/* aggregated_port
 * Access: RW
 */
MLXSW_ITEM8(reg, ppcr, aggregated_port, 0xb, 0, 8);

/* p_type
 * Access: RW
 */
MLXSW_ITEM8(reg, ppcr, p_type, 0xd, 0, 3);

/* num_of_planes
 * Access: RW
 */
MLXSW_ITEM8(reg, ppcr, num_of_planes, 0xf, 0, 8);

/* pldt
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pldt, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pldt, lp_msb, 0x2, 4, 2);

/* link_up_delay
 * Access: RW
 */
MLXSW_ITEM16(reg, pldt, link_up_delay, 0x6, 0, 16);

/* link_down_delay
 * Access: RW
 */
MLXSW_ITEM16(reg, pldt, link_down_delay, 0xa, 0, 16);

/* pldc
 * ----
 *
 */
/* clr
 * Access: OP
 */
MLXSW_ITEM8(reg, pldc, clr, 0x0, 7, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pldc, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pldc, lp_msb, 0x2, 4, 2);

/* link_up_delay_cnt
 * Access: RO
 */
MLXSW_ITEM16(reg, pldc, link_up_delay_cnt, 0x6, 0, 16);

/* link_down_delay_cnt
 * Access: RO
 */
MLXSW_ITEM16(reg, pldc, link_down_delay_cnt, 0xa, 0, 16);

/* timer_expired_cnt
 * Access: RO
 */
MLXSW_ITEM16(reg, pldc, timer_expired_cnt, 0xe, 0, 16);

/* timer_cancel_cnt
 * Access: RO
 */
MLXSW_ITEM16(reg, pldc, timer_cancel_cnt, 0x12, 0, 16);

/* plibdb
 * ------
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, plibdb, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, plibdb, lp_msb, 0x2, 4, 2);

/* ib_port_1x
 * Access: RO
 */
MLXSW_ITEM16(reg, plibdb, ib_port_1x, 0x6, 0, 10);

/* ib_port_2x
 * Access: RO
 */
MLXSW_ITEM16(reg, plibdb, ib_port_2x, 0xa, 0, 10);

/* ib_port_4x
 * Access: RO
 */
MLXSW_ITEM16(reg, plibdb, ib_port_4x, 0xe, 0, 10);

/* pbgcr
 * -----
 *
 */
/* per_ptb
 * Access: OP
 */
MLXSW_ITEM8(reg, pbgcr, per_ptb, 0x0, 6, 2);

/* ptb_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pbgcr, ptb_index, 0x1, 0, 8);

/* psbtt
 * Access: OP
 */
MLXSW_ITEM8(reg, pbgcr, psbtt, 0x3, 0, 4);

/* user_val
 * Access: RW
 */
MLXSW_ITEM16(reg, pbgcr, user_val, 0x6, 0, 9);

/* dib_user_value
 * Access: RW
 */
MLXSW_ITEM16(reg, pbgcr, dib_user_value, 0xa, 0, 9);

/* ppir
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppir, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppir, lp_msb, 0x2, 4, 2);

/* lpnf_no_hcd
 * Access: RO
 */
MLXSW_ITEM8(reg, ppir, lpnf_no_hcd, 0x2, 1, 1);

/* lpnf_phy
 * Access: RO
 */
MLXSW_ITEM8(reg, ppir, lpnf_phy, 0x2, 0, 1);

/* lcnf_pln
 * Access: RO
 */
MLXSW_ITEM8(reg, ppir, lcnf_pln, 0x3, 0, 1);

/* peer_p_type
 * Access: RO
 */
MLXSW_ITEM8(reg, ppir, peer_p_type, 0x5, 0, 3);

/* peer_num_of_planes
 * Access: RO
 */
MLXSW_ITEM8(reg, ppir, peer_num_of_planes, 0x7, 0, 8);

/* phbr
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, phbr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, phbr, lp_msb, 0x2, 4, 2);

/* hist_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, phbr, hist_id, 0x7, 0, 8);

/* mode
 * Access: RW
 */
MLXSW_ITEM8(reg, phbr, mode, 0x8, 4, 3);

/* hist_type
 * Access: RW
 */
MLXSW_ITEM16(reg, phbr, hist_type, 0xa, 0, 16);

/* first_bin_thr
 * Access: RW
 */
MLXSW_ITEM32(reg, phbr, first_bin_thr, 0xc, 0, 32);

/* bin_size
 * Access: RW
 */
MLXSW_ITEM32(reg, phbr, bin_size, 0x10, 0, 32);

/* hist_repeat_num
 * Access: RW
 */
MLXSW_ITEM32(reg, phbr, hist_repeat_num, 0x14, 0, 24);

/* sample_time
 * Access: RW
 */
MLXSW_ITEM8(reg, phbr, sample_time, 0x1b, 0, 6);

/* phrr
 * ----
 *
 */
/* clr
 * Access: RW
 */
MLXSW_ITEM8(reg, phrr, clr, 0x0, 7, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, phrr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, phrr, lp_msb, 0x2, 4, 2);

/* hist_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, phrr, hist_id, 0x7, 0, 8);

/* bin
 * Access: RO
 */
MLXSW_ITEM64_INDEXED(reg, phrr, bin, 0x10, 0, 64, 8, 0, 8, 0);

/* min_watermark
 * Access: RO
 */
MLXSW_ITEM32(reg, phrr, min_watermark, 0x60, 0, 32);

/* max_watermark
 * Access: RO
 */
MLXSW_ITEM32(reg, phrr, max_watermark, 0x64, 0, 32);

/* pbwc
 * ----
 *
 */
/* log_time_interval
 * Access: RW
 */
MLXSW_ITEM8(reg, pbwc, log_time_interval, 0x3, 0, 4);

/* alpha_factor
 * Access: RW
 */
MLXSW_ITEM8(reg, pbwc, alpha_factor, 0x7, 0, 3);

/* pbwr
 * ----
 *
 */
/* local_port_bitmap
 * Access: INDEX
 */
MLXSW_ITEM32_INDEXED(reg, pbwr, local_port_bitmap, 0x0, 0, 32, 4, 0, 4, 0);

/* dir
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pbwr, dir, 0x80, 7, 1);

/* num_rec
 * Access: OP
 */
MLXSW_ITEM16(reg, pbwr, num_rec, 0x82, 0, 11);

/* bw_record
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, pbwr, bw_record, 0x90, 0, 32, 4, 0, 4, 0);

/* pfcr
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pfcr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pfcr, lp_msb, 0x2, 4, 2);

/* rx_fcs_check_disable
 * Access: RW
 */
MLXSW_ITEM8(reg, pfcr, rx_fcs_check_disable, 0x8, 7, 1);

/* tx_fcs_recalc_disable
 * Access: RW
 */
MLXSW_ITEM8(reg, pfcr, tx_fcs_recalc_disable, 0x8, 6, 1);

/* rx_icrc_generation_disable
 * Access: RW
 */
MLXSW_ITEM8(reg, pfcr, rx_icrc_generation_disable, 0x9, 7, 1);

/* tx_icrc_check_disable
 * Access: RW
 */
MLXSW_ITEM8(reg, pfcr, tx_icrc_check_disable, 0x9, 6, 1);

/* rx_toc_insert
 * Access: RW
 */
MLXSW_ITEM8(reg, pfcr, rx_toc_insert, 0xa, 7, 1);

/* tx_toc_disable
 * Access: RW
 */
MLXSW_ITEM8(reg, pfcr, tx_toc_disable, 0xa, 6, 1);

/* tx_toc_mod
 * Access: RW
 */
MLXSW_ITEM8(reg, pfcr, tx_toc_mod, 0xa, 5, 1);

/* ppslc
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppslc, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppslc, lp_msb, 0x2, 4, 2);

/* l1_req_en
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslc, l1_req_en, 0x7, 0, 1);

/* l0i_tx_tmp_dis
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslc, l0i_tx_tmp_dis, 0x9, 5, 1);

/* l0i_all_queues_are_import
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslc, l0i_all_queues_are_import, 0x9, 4, 1);

/* l0i_tx_req_en
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslc, l0i_tx_req_en, 0x9, 3, 1);

/* l0i_tx_cap_adv
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslc, l0i_tx_cap_adv, 0x9, 2, 1);

/* l0i_rx_req_en
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslc, l0i_rx_req_en, 0x9, 1, 1);

/* l0i_rx_cap_adv
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslc, l0i_rx_cap_adv, 0x9, 0, 1);

/* l1_fw_cap_adv
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslc, l1_fw_cap_adv, 0xa, 0, 1);

/* l1_cap_tmp_dis
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslc, l1_cap_tmp_dis, 0xb, 5, 1);

/* l1_cap_adv
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslc, l1_cap_adv, 0xb, 4, 1);

/* l1_fw_req_en
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslc, l1_fw_req_en, 0xb, 0, 1);

/* pred_algo_en
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslc, pred_algo_en, 0xc, 7, 1);

/* ignore_pred_pm
 * Access: RW
 */
MLXSW_ITEM8(reg, ppslc, ignore_pred_pm, 0xc, 6, 1);

/* hp_queues_bitmap
 * Access: RW
 */
MLXSW_ITEM32(reg, ppslc, hp_queues_bitmap, 0x10, 0, 32);

/* l1_hw_active_time
 * Access: RW
 */
MLXSW_ITEM16(reg, ppslc, l1_hw_active_time, 0x1a, 0, 10);

/* l0i_hw_inactive_time
 * Access: RW
 */
MLXSW_ITEM16(reg, ppslc, l0i_hw_inactive_time, 0x1c, 0, 10);

/* l1_hw_inactive_time
 * Access: RW
 */
MLXSW_ITEM16(reg, ppslc, l1_hw_inactive_time, 0x1e, 0, 10);

/* qem
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, ppslc, qem, 0x20, 20);

/* sbgcr
 * -----
 *
 */
/* hsba_mask
 * Access: WO
 */
MLXSW_ITEM8(reg, sbgcr, hsba_mask, 0x2, 1, 1);

/* hsba
 * Access: RW
 */
MLXSW_ITEM8(reg, sbgcr, hsba, 0x2, 0, 1);

/* cong_fp
 * Access: RW
 */
MLXSW_ITEM8(reg, sbgcr, cong_fp, 0x3, 4, 1);

/* tele_entity
 * Access: RW
 */
MLXSW_ITEM8(reg, sbgcr, tele_entity, 0x3, 0, 2);

/* sbsrd
 * -----
 *
 */
/* clr
 * Access: OP
 */
MLXSW_ITEM8(reg, sbsrd, clr, 0x0, 7, 1);

/* snap
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbsrd, snap, 0x0, 6, 1);

/* desc
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbsrd, desc, 0x0, 5, 1);

/* modi
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbsrd, modi, 0x0, 4, 1);

/* type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbsrd, type, 0x3, 0, 8);

/* num_rec
 * Access: OP
 */
MLXSW_ITEM8(reg, sbsrd, num_rec, 0x7, 0, 8);

/* first_entry_index
 * Access: RO
 */
MLXSW_ITEM16(reg, sbsrd, first_entry_index, 0x8, 0, 16);

/* first_lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbsrd, first_lp_msb, 0xa, 0, 2);

/* first_local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbsrd, first_local_port, 0xb, 0, 8);

/* buff_occupancy
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, sbsrd, buff_occupancy, MLXSW_SBSRD_SHARED_BUFFER_STATUS_BASE_LEN, 0, 24, MLXSW_SBSRD_SHARED_BUFFER_STATUS_REC_LEN, 0, 3, 0);

/* max_buff_occupancy
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, sbsrd, max_buff_occupancy, MLXSW_SBSRD_SHARED_BUFFER_STATUS_BASE_LEN, 0, 24, MLXSW_SBSRD_SHARED_BUFFER_STATUS_REC_LEN, 4, 3, 0);

/* ext_buff_occupancy
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, sbsrd, ext_buff_occupancy, 0x410, 0, 24, 0x4, 0x0, 3, 0);

/* sbdcc
 * -----
 *
 */
/* clr
 * Access: OP
 */
MLXSW_ITEM8(reg, sbdcc, clr, 0x0, 7, 1);

/* by_mask
 * Access: OP
 */
MLXSW_ITEM8(reg, sbdcc, by_mask, 0x0, 5, 1);

/* cpu_tclass_mask
 * Access: INDEX
 */
MLXSW_ITEM64(reg, sbdcc, cpu_tclass_mask, 0x8, 0, 64);

/* count_high
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, sbdcc, count_high, MLXSW_SBDCC_NO_BUFFER_DISCARD_CPU_TCLASS_BASE_LEN, 0, 32, MLXSW_SBDCC_NO_BUFFER_DISCARD_CPU_TCLASS_REC_LEN, 0, 4, 0);

/* count_low
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, sbdcc, count_low, MLXSW_SBDCC_NO_BUFFER_DISCARD_CPU_TCLASS_BASE_LEN, 0, 32, MLXSW_SBDCC_NO_BUFFER_DISCARD_CPU_TCLASS_REC_LEN, 4, 4, 0);

/* sbhbr_v2
 * --------
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbhbr_v2, local_port, 0x1, 0, 8);

/* pg_buff
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbhbr_v2, pg_buff, 0x2, 0, 6);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbhbr_v2, lp_msb, 0x3, 4, 2);

/* dir
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbhbr_v2, dir, 0x3, 0, 2);

/* en
 * Access: RW
 */
MLXSW_ITEM8(reg, sbhbr_v2, en, 0x7, 0, 1);

/* mode
 * Access: RW
 */
MLXSW_ITEM8(reg, sbhbr_v2, mode, 0x8, 4, 3);

/* hist_type
 * Access: INDEX
 */
MLXSW_ITEM16(reg, sbhbr_v2, hist_type, 0xa, 0, 16);

/* hist_min_value
 * Access: RW
 */
MLXSW_ITEM32(reg, sbhbr_v2, hist_min_value, 0x10, 0, 32);

/* hist_max_value
 * Access: RW
 */
MLXSW_ITEM32(reg, sbhbr_v2, hist_max_value, 0x14, 0, 32);

/* sample_time
 * Access: RW
 */
MLXSW_ITEM8(reg, sbhbr_v2, sample_time, 0x1b, 0, 6);

/* sbhrr_v2
 * --------
 *
 */
/* clr
 * Access: OP
 */
MLXSW_ITEM8(reg, sbhrr_v2, clr, 0x0, 7, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbhrr_v2, local_port, 0x1, 0, 8);

/* pg_buff
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbhrr_v2, pg_buff, 0x2, 0, 6);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbhrr_v2, lp_msb, 0x3, 4, 2);

/* dir
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbhrr_v2, dir, 0x3, 0, 2);

/* hist_type
 * Access: INDEX
 */
MLXSW_ITEM16(reg, sbhrr_v2, hist_type, 0xa, 0, 16);

/* min_sampled_high
 * Access: RO
 */
MLXSW_ITEM32(reg, sbhrr_v2, min_sampled_high, 0x10, 0, 32);

/* min_sampled_low
 * Access: RO
 */
MLXSW_ITEM32(reg, sbhrr_v2, min_sampled_low, 0x14, 0, 32);

/* max_sampled_high
 * Access: RO
 */
MLXSW_ITEM32(reg, sbhrr_v2, max_sampled_high, 0x18, 0, 32);

/* max_sampled_low
 * Access: RO
 */
MLXSW_ITEM32(reg, sbhrr_v2, max_sampled_low, 0x1c, 0, 32);

/* bin
 * Access: RO
 */
MLXSW_ITEM64_INDEXED(reg, sbhrr_v2, bin, 0x40, 0, 64, 8, 0, 8, 0);

/* sbctr
 * -----
 *
 */
/* ievent
 * Access: OP
 */
MLXSW_ITEM8(reg, sbctr, ievent, 0x0, 7, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbctr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbctr, lp_msb, 0x2, 4, 2);

/* dir_ing
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbctr, dir_ing, 0x2, 0, 2);

/* fp
 * Access: RO
 */
MLXSW_ITEM8(reg, sbctr, fp, 0x3, 4, 1);

/* entity
 * Access: RO
 */
MLXSW_ITEM8(reg, sbctr, entity, 0x3, 0, 2);

/* tclass_vector_high
 * Access: RO
 */
MLXSW_ITEM32(reg, sbctr, tclass_vector_high, 0x8, 0, 32);

/* tclass_vector_low
 * Access: RO
 */
MLXSW_ITEM32(reg, sbctr, tclass_vector_low, 0xc, 0, 32);

/* sbhpc
 * -----
 *
 */
/* max_buff
 * Access: RW
 */
MLXSW_ITEM32(reg, sbhpc, max_buff, 0x10, 0, 24);

/* buff_occupancy
 * Access: RO
 */
MLXSW_ITEM32(reg, sbhpc, buff_occupancy, 0x20, 0, 24);

/* sbsnt
 * -----
 *
 */
/* take
 * Access: OP
 */
MLXSW_ITEM8(reg, sbsnt, take, 0x3, 0, 2);

/* sbsns
 * -----
 *
 */
/* status
 * Access: RO
 */
MLXSW_ITEM8(reg, sbsns, status, 0x3, 0, 1);

/* trigger_id
 * Access: RO
 */
MLXSW_ITEM8(reg, sbsns, trigger_id, 0x7, 0, 8);

/* trigger_parameters
 * Access: RO
 */
MLXSW_ITEM32(reg, sbsns, trigger_parameters, 0x8, 0, 32);

/* time_high
 * Access: RO
 */
MLXSW_ITEM8(reg, sbsns, time_high, 0x13, 0, 6);

/* time_low
 * Access: RO
 */
MLXSW_ITEM32(reg, sbsns, time_low, 0x14, 0, 32);

/* sbsnte
 * ------
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbsnte, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbsnte, lp_msb, 0x2, 4, 2);

/* type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, sbsnte, type, 0x7, 0, 8);

/* tclass_en_high
 * Access: RW
 */
MLXSW_ITEM32(reg, sbsnte, tclass_en_high, 0x8, 0, 32);

/* tclass_en_low
 * Access: RW
 */
MLXSW_ITEM32(reg, sbsnte, tclass_en_low, 0xc, 0, 32);

/* sbrmc
 * -----
 *
 */
/* clear_counters
 * Access: OP
 */
MLXSW_ITEM8(reg, sbrmc, clear_counters, 0x3, 1, 1);

/* en
 * Access: RW
 */
MLXSW_ITEM8(reg, sbrmc, en, 0x3, 0, 1);

/* sbrms
 * -----
 *
 */
/* res_event_mask
 * Access: RW
 */
MLXSW_ITEM8(reg, sbrms, res_event_mask, 0x2, 0, 2);

/* resonance
 * Access: RO
 */
MLXSW_ITEM8(reg, sbrms, resonance, 0x3, 0, 1);

/* rec_event_mask
 * Access: RW
 */
MLXSW_ITEM8(reg, sbrms, rec_event_mask, 0x6, 0, 2);

/* recovery
 * Access: RO
 */
MLXSW_ITEM8(reg, sbrms, recovery, 0x7, 0, 1);

/* resonance_counter
 * Access: RO
 */
MLXSW_ITEM32(reg, sbrms, resonance_counter, 0x8, 0, 32);

/* recovery_counter
 * Access: RO
 */
MLXSW_ITEM32(reg, sbrms, recovery_counter, 0xc, 0, 32);

/* sbrmr
 * -----
 *
 */
/* recover
 * Access: RW
 */
MLXSW_ITEM8(reg, sbrmr, recover, 0x3, 0, 1);

/* sbrmp
 * -----
 *
 */
/* backpressure_sample_time
 * Access: RW
 */
MLXSW_ITEM32(reg, sbrmp, backpressure_sample_time, 0x8, 0, 32);

/* backpressure_threshold
 * Access: RW
 */
MLXSW_ITEM32(reg, sbrmp, backpressure_threshold, 0xc, 0, 32);

/* coupling_rate_sample_time
 * Access: RW
 */
MLXSW_ITEM32(reg, sbrmp, coupling_rate_sample_time, 0x10, 0, 32);

/* coupling_rate_threshold
 * Access: RW
 */
MLXSW_ITEM32(reg, sbrmp, coupling_rate_threshold, 0x14, 0, 32);

/* reading_packet_denial_sample_time
 * Access: RW
 */
MLXSW_ITEM32(reg, sbrmp, reading_packet_denial_sample_time, 0x18, 0, 32);

/* reading_packet_denial_threshold
 * Access: RW
 */
MLXSW_ITEM32(reg, sbrmp, reading_packet_denial_threshold, 0x1c, 0, 32);

/* no_backpressure_min_samples
 * Access: RW
 */
MLXSW_ITEM32(reg, sbrmp, no_backpressure_min_samples, 0x20, 0, 32);

/* rib_high_min_samples
 * Access: RW
 */
MLXSW_ITEM32(reg, sbrmp, rib_high_min_samples, 0x24, 0, 32);

/* recovery_wait_time
 * Access: RW
 */
MLXSW_ITEM32(reg, sbrmp, recovery_wait_time, 0x28, 0, 32);

/* pgcr
 * ----
 *
 */
/* update_prune
 * Access: OP
 */
MLXSW_ITEM8(reg, pgcr, update_prune, 0x0, 7, 1);

/* pbs_table_size
 * Access: RW
 */
MLXSW_ITEM16(reg, pgcr, pbs_table_size, 0x2, 0, 16);

/* max_eacl
 * Access: RO
 */
MLXSW_ITEM16(reg, pgcr, max_eacl, 0x8, 0, 16);

/* max_iacl
 * Access: RO
 */
MLXSW_ITEM16(reg, pgcr, max_iacl, 0xa, 0, 16);

/* atcam_ignore_prune_vector
 * Access: RW
 */
MLXSW_ITEM16(reg, pgcr, atcam_ignore_prune_vector, 0x16, 0, 16);

/* activity_dis
 * Access: RW
 */
MLXSW_ITEM8(reg, pgcr, activity_dis, 0x1b, 0, 8);

/* default_action_pointer_base
 * Access: RW
 */
MLXSW_ITEM32(reg, pgcr, default_action_pointer_base, 0x1c, 0, 24);

/* ppbt
 * ----
 *
 */
/* e
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppbt, e, 0x0, 7, 1);

/* op
 * Access: RW
 */
MLXSW_ITEM8(reg, ppbt, op, 0x0, 4, 3);

/* tport
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppbt, tport, 0x0, 0, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppbt, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppbt, lp_msb, 0x2, 4, 2);

/* g
 * Access: RW
 */
MLXSW_ITEM8(reg, ppbt, g, 0x10, 7, 1);

/* acl_id_group_id
 * Access: RW
 */
MLXSW_ITEM16(reg, ppbt, acl_id_group_id, 0x12, 0, 16);

/* pevpb
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pevpb, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pevpb, lp_msb, 0x2, 4, 2);

/* ev
 * Access: RW
 */
MLXSW_ITEM8(reg, pevpb, ev, 0x7, 0, 1);

/* pvgt
 * ----
 *
 */
/* swid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pvgt, swid, 0x0, 0, 8);

/* op
 * Access: WO
 */
MLXSW_ITEM8(reg, pvgt, op, 0x1, 4, 3);

/* vid
 * Access: INDEX
 */
MLXSW_ITEM16(reg, pvgt, vid, 0x2, 0, 12);

/* vlan_group
 * Access: RW
 */
MLXSW_ITEM16(reg, pvgt, vlan_group, 0x6, 0, 12);

/* pvbt
 * ----
 *
 */
/* swid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pvbt, swid, 0x0, 0, 8);

/* e
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pvbt, e, 0x1, 7, 1);

/* op
 * Access: RW
 */
MLXSW_ITEM8(reg, pvbt, op, 0x1, 4, 3);

/* vlan_group
 * Access: INDEX
 */
MLXSW_ITEM16(reg, pvbt, vlan_group, 0x2, 0, 12);

/* g
 * Access: RW
 */
MLXSW_ITEM8(reg, pvbt, g, 0x10, 7, 1);

/* acl_id_group_id
 * Access: RW
 */
MLXSW_ITEM16(reg, pvbt, acl_id_group_id, 0x12, 0, 16);

/* pemrbt
 * ------
 *
 */
/* op
 * Access: RW
 */
MLXSW_ITEM8(reg, pemrbt, op, 0x1, 4, 3);

/* protocol
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pemrbt, protocol, 0x3, 0, 1);

/* group_id
 * Access: RW
 */
MLXSW_ITEM16(reg, pemrbt, group_id, 0x12, 0, 16);

/* perb
 * ----
 *
 */
/* pipe_stage
 * Access: INDEX
 */
MLXSW_ITEM8(reg, perb, pipe_stage, 0x1, 0, 4);

/* rule_profile_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, perb, rule_profile_index, 0x3, 0, 4);

/* op
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, op, 0x7, 0, 2);

/* group_id
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, group_id, 0xa, 0, 16);

/* perb_classifier_iport_base_classifier_l2_flood
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_iport_base_classifier_l2_flood, 0x20, 0, 2);

/* perb_classifier_iport_base_classifier_l2_mc
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_iport_base_classifier_l2_mc, 0x21, 4, 2);

/* perb_classifier_iport_base_classifier_l3_mc
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_iport_base_classifier_l3_mc, 0x21, 0, 2);

/* perb_classifier_iport_base_classifier_l2_type
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_iport_base_classifier_l2_type, 0x22, 0, 2);

/* perb_classifier_iport_base_classifier_inner_l2_type
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_iport_base_classifier_inner_l2_type, 0x23, 0, 2);

/* perb_classifier_iport_base_classifier_l3_type
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_iport_base_classifier_l3_type, 0x27, 0, 8);

/* perb_classifier_iport_base_classifier_inner_l3_type
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_iport_base_classifier_inner_l3_type, 0x2b, 0, 8);

/* perb_classifier_iport_base_classifier_ip_frag
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_iport_base_classifier_ip_frag, 0x2e, 0, 2);

/* perb_classifier_iport_base_classifier_inner_ip_frag
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_iport_base_classifier_inner_ip_frag, 0x2f, 0, 2);

/* perb_classifier_iport_base_classifier_l4_type
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_iport_base_classifier_l4_type, 0x32, 0, 11);

/* perb_classifier_iport_base_classifier_inner_l4_type
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_iport_base_classifier_inner_l4_type, 0x36, 0, 11);

/* perb_classifier_iport_base_classifier_tunnel_type
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_iport_base_classifier_tunnel_type, 0x3a, 0, 9);

/* perb_classifier_iport_base_classifier_decap_tq_command
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_iport_base_classifier_decap_tq_command, 0x3c, 6, 2);

/* perb_classifier_iport_base_classifier_tq_command
 * Access: RW
 */
MLXSW_ITEM32(reg, perb, perb_classifier_iport_base_classifier_tq_command, 0x3c, 0, 19);

/* perb_classifier_iport_base_classifier_fpp_index
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_iport_base_classifier_fpp_index, 0x42, 0, 9);

/* perb_classifier_iport_base_classifier_inner_fpp_index
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_iport_base_classifier_inner_fpp_index, 0x46, 0, 9);

/* perb_classifier_iport_ingress_local_port_list
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, perb, perb_classifier_iport_ingress_local_port_list, 0x60, 0, 32, 4, 0, 4, 0);

/* perb_classifier_iport_tunnel_port
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_iport_tunnel_port, 0xe3, 0, 4);

/* perb_classifier_irif_base_classifier_l2_flood
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_irif_base_classifier_l2_flood, 0x20, 0, 2);

/* perb_classifier_irif_base_classifier_l2_mc
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_irif_base_classifier_l2_mc, 0x21, 4, 2);

/* perb_classifier_irif_base_classifier_l3_mc
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_irif_base_classifier_l3_mc, 0x21, 0, 2);

/* perb_classifier_irif_base_classifier_l2_type
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_irif_base_classifier_l2_type, 0x22, 0, 2);

/* perb_classifier_irif_base_classifier_inner_l2_type
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_irif_base_classifier_inner_l2_type, 0x23, 0, 2);

/* perb_classifier_irif_base_classifier_l3_type
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_irif_base_classifier_l3_type, 0x27, 0, 8);

/* perb_classifier_irif_base_classifier_inner_l3_type
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_irif_base_classifier_inner_l3_type, 0x2b, 0, 8);

/* perb_classifier_irif_base_classifier_ip_frag
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_irif_base_classifier_ip_frag, 0x2e, 0, 2);

/* perb_classifier_irif_base_classifier_inner_ip_frag
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_irif_base_classifier_inner_ip_frag, 0x2f, 0, 2);

/* perb_classifier_irif_base_classifier_l4_type
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_irif_base_classifier_l4_type, 0x32, 0, 11);

/* perb_classifier_irif_base_classifier_inner_l4_type
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_irif_base_classifier_inner_l4_type, 0x36, 0, 11);

/* perb_classifier_irif_base_classifier_tunnel_type
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_irif_base_classifier_tunnel_type, 0x3a, 0, 9);

/* perb_classifier_irif_base_classifier_decap_tq_command
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_irif_base_classifier_decap_tq_command, 0x3c, 6, 2);

/* perb_classifier_irif_base_classifier_tq_command
 * Access: RW
 */
MLXSW_ITEM32(reg, perb, perb_classifier_irif_base_classifier_tq_command, 0x3c, 0, 19);

/* perb_classifier_irif_base_classifier_fpp_index
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_irif_base_classifier_fpp_index, 0x42, 0, 9);

/* perb_classifier_irif_base_classifier_inner_fpp_index
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_irif_base_classifier_inner_fpp_index, 0x46, 0, 9);

/* perb_classifier_irif_rb_irif_group
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_irif_rb_irif_group, 0x63, 0, 4);

/* perb_classifier_erif_base_classifier_l2_flood
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_erif_base_classifier_l2_flood, 0x20, 0, 2);

/* perb_classifier_erif_base_classifier_l2_mc
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_erif_base_classifier_l2_mc, 0x21, 4, 2);

/* perb_classifier_erif_base_classifier_l3_mc
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_erif_base_classifier_l3_mc, 0x21, 0, 2);

/* perb_classifier_erif_base_classifier_l2_type
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_erif_base_classifier_l2_type, 0x22, 0, 2);

/* perb_classifier_erif_base_classifier_inner_l2_type
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_erif_base_classifier_inner_l2_type, 0x23, 0, 2);

/* perb_classifier_erif_base_classifier_l3_type
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_erif_base_classifier_l3_type, 0x27, 0, 8);

/* perb_classifier_erif_base_classifier_inner_l3_type
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_erif_base_classifier_inner_l3_type, 0x2b, 0, 8);

/* perb_classifier_erif_base_classifier_ip_frag
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_erif_base_classifier_ip_frag, 0x2e, 0, 2);

/* perb_classifier_erif_base_classifier_inner_ip_frag
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_erif_base_classifier_inner_ip_frag, 0x2f, 0, 2);

/* perb_classifier_erif_base_classifier_l4_type
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_erif_base_classifier_l4_type, 0x32, 0, 11);

/* perb_classifier_erif_base_classifier_inner_l4_type
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_erif_base_classifier_inner_l4_type, 0x36, 0, 11);

/* perb_classifier_erif_base_classifier_tunnel_type
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_erif_base_classifier_tunnel_type, 0x3a, 0, 9);

/* perb_classifier_erif_base_classifier_decap_tq_command
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_erif_base_classifier_decap_tq_command, 0x3c, 6, 2);

/* perb_classifier_erif_base_classifier_tq_command
 * Access: RW
 */
MLXSW_ITEM32(reg, perb, perb_classifier_erif_base_classifier_tq_command, 0x3c, 0, 19);

/* perb_classifier_erif_base_classifier_fpp_index
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_erif_base_classifier_fpp_index, 0x42, 0, 9);

/* perb_classifier_erif_base_classifier_inner_fpp_index
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_erif_base_classifier_inner_fpp_index, 0x46, 0, 9);

/* perb_classifier_erif_rb_erif_group
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_erif_rb_erif_group, 0x63, 0, 4);

/* perb_classifier_eport_base_classifier_l2_flood
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_eport_base_classifier_l2_flood, 0x20, 0, 2);

/* perb_classifier_eport_base_classifier_l2_mc
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_eport_base_classifier_l2_mc, 0x21, 4, 2);

/* perb_classifier_eport_base_classifier_l3_mc
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_eport_base_classifier_l3_mc, 0x21, 0, 2);

/* perb_classifier_eport_base_classifier_l2_type
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_eport_base_classifier_l2_type, 0x22, 0, 2);

/* perb_classifier_eport_base_classifier_inner_l2_type
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_eport_base_classifier_inner_l2_type, 0x23, 0, 2);

/* perb_classifier_eport_base_classifier_l3_type
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_eport_base_classifier_l3_type, 0x27, 0, 8);

/* perb_classifier_eport_base_classifier_inner_l3_type
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_eport_base_classifier_inner_l3_type, 0x2b, 0, 8);

/* perb_classifier_eport_base_classifier_ip_frag
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_eport_base_classifier_ip_frag, 0x2e, 0, 2);

/* perb_classifier_eport_base_classifier_inner_ip_frag
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_eport_base_classifier_inner_ip_frag, 0x2f, 0, 2);

/* perb_classifier_eport_base_classifier_l4_type
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_eport_base_classifier_l4_type, 0x32, 0, 11);

/* perb_classifier_eport_base_classifier_inner_l4_type
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_eport_base_classifier_inner_l4_type, 0x36, 0, 11);

/* perb_classifier_eport_base_classifier_tunnel_type
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_eport_base_classifier_tunnel_type, 0x3a, 0, 9);

/* perb_classifier_eport_base_classifier_decap_tq_command
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_eport_base_classifier_decap_tq_command, 0x3c, 6, 2);

/* perb_classifier_eport_base_classifier_tq_command
 * Access: RW
 */
MLXSW_ITEM32(reg, perb, perb_classifier_eport_base_classifier_tq_command, 0x3c, 0, 19);

/* perb_classifier_eport_base_classifier_fpp_index
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_eport_base_classifier_fpp_index, 0x42, 0, 9);

/* perb_classifier_eport_base_classifier_inner_fpp_index
 * Access: RW
 */
MLXSW_ITEM16(reg, perb, perb_classifier_eport_base_classifier_inner_fpp_index, 0x46, 0, 9);

/* perb_classifier_eport_rb_eport_group
 * Access: RW
 */
MLXSW_ITEM32(reg, perb, perb_classifier_eport_rb_eport_group, 0x60, 0, 32);

/* perb_classifier_eport_tunnel_port
 * Access: RW
 */
MLXSW_ITEM8(reg, perb, perb_classifier_eport_tunnel_port, 0x67, 0, 4);

/* perbrg
 * ------
 *
 */
/* rif
 * Access: INDEX
 */
MLXSW_ITEM16(reg, perbrg, rif, 0x2, 0, 16);

/* dir
 * Access: INDEX
 */
MLXSW_ITEM8(reg, perbrg, dir, 0xb, 0, 1);

/* rb_rif_group
 * Access: RW
 */
MLXSW_ITEM8(reg, perbrg, rb_rif_group, 0xf, 0, 5);

/* perbeg
 * ------
 *
 */
/* egress_local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, perbeg, egress_local_port, 0x1, 0, 8);

/* egress_lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, perbeg, egress_lp_msb, 0x2, 4, 2);

/* rb_eport_group
 * Access: RW
 */
MLXSW_ITEM8(reg, perbeg, rb_eport_group, 0xf, 0, 5);

/* pagt_v2
 * -------
 *
 */
/* id_v
 * Access: RW
 */
MLXSW_ITEM8(reg, pagt_v2, id_v, 0x0, 7, 1);

/* n_v
 * Access: RW
 */
MLXSW_ITEM8(reg, pagt_v2, n_v, 0x0, 6, 1);

/* acl_group_element
 * Access: INDEX
 */
MLXSW_ITEM16(reg, pagt_v2, acl_group_element, 0x2, 0, 16);

/* next_acl_group_element
 * Access: RW
 */
MLXSW_ITEM16(reg, pagt_v2, next_acl_group_element, 0x6, 0, 16);

/* multi
 * Access: RW
 */
MLXSW_ITEM8(reg, pagt_v2, multi, 0x8, 7, 1);

/* commit
 * Access: RW
 */
MLXSW_ITEM8(reg, pagt_v2, commit, 0x8, 6, 1);

/* acl_id
 * Access: RW
 */
MLXSW_ITEM16(reg, pagt_v2, acl_id, 0xa, 0, 16);

/* pagtq
 * -----
 *
 */
/* busy_selector
 * Access: OP
 */
MLXSW_ITEM8(reg, pagtq, busy_selector, 0x0, 7, 1);

/* acl_group_element_busy_bitmap
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, pagtq, acl_group_element_busy_bitmap, 0x10, 0, 32, 4, 0, 4, 0);

/* perla
 * -----
 *
 */
/* hw_region
 * Access: INDEX
 */
MLXSW_ITEM16(reg, perla, hw_region, 0x2, 0, 16);

/* lookup_region_id
 * Access: RW
 */
MLXSW_ITEM16(reg, perla, lookup_region_id, 0x6, 0, 16);

/* perar
 * -----
 *
 */
/* region_id
 * Access: INDEX
 */
MLXSW_ITEM16(reg, perar, region_id, 0x2, 0, 16);

/* hw_region
 * Access: RW
 */
MLXSW_ITEM16(reg, perar, hw_region, 0x6, 0, 16);

/* ptcead
 * ------
 *
 */
/* op
 * Access: OP
 */
MLXSW_ITEM8(reg, ptcead, op, 0x0, 6, 2);

/* offset
 * Access: INDEX
 */
MLXSW_ITEM16(reg, ptcead, offset, 0x2, 0, 16);

/* num_rec
 * Access: OP
 */
MLXSW_ITEM16(reg, ptcead, num_rec, 0x6, 0, 13);

/* tcam_region_info
 * Access: INDEX
 */
MLXSW_ITEM_BUF(reg, ptcead, tcam_region_info, 0x10, 16);

/* activity_vector
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, ptcead, activity_vector, 0x20, 0, 32, 4, 0, 4, 0);

/* pefaad
 * ------
 *
 */
/* filter_fields
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pefaad, filter_fields, 0x3, 0, 8);

/* op
 * Access: OP
 */
MLXSW_ITEM8(reg, pefaad, op, 0x4, 0, 2);

/* num_rec
 * Access: OP
 */
MLXSW_ITEM16(reg, pefaad, num_rec, 0x6, 0, 9);

/* entry_a
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pefaad, entry_a, 0x9, 0, 1);

/* as_user_val
 * Access: INDEX
 */
MLXSW_ITEM16(reg, pefaad, as_user_val, 0x12, 0, 10);

/* index_dump
 * Access: 
 */
MLXSW_ITEM32_INDEXED(reg, pefaad, index_dump, 0x20, 0, 24, 0x4, 0x0, 3, 0);

/* pprr
 * ----
 *
 */
/* ipv4
 * Access: RW
 */
MLXSW_ITEM8(reg, pprr, ipv4, 0x0, 7, 1);

/* ipv6
 * Access: RW
 */
MLXSW_ITEM8(reg, pprr, ipv6, 0x0, 6, 1);

/* src
 * Access: RW
 */
MLXSW_ITEM8(reg, pprr, src, 0x0, 5, 1);

/* dst
 * Access: RW
 */
MLXSW_ITEM8(reg, pprr, dst, 0x0, 4, 1);

/* tcp
 * Access: RW
 */
MLXSW_ITEM8(reg, pprr, tcp, 0x0, 3, 1);

/* udp
 * Access: RW
 */
MLXSW_ITEM8(reg, pprr, udp, 0x0, 2, 1);

/* ignore_l3
 * Access: RW
 */
MLXSW_ITEM8(reg, pprr, ignore_l3, 0x0, 1, 1);

/* ignore_l4
 * Access: RW
 */
MLXSW_ITEM8(reg, pprr, ignore_l4, 0x0, 0, 1);

/* outer_inner
 * Access: RW
 */
MLXSW_ITEM8(reg, pprr, outer_inner, 0x1, 6, 2);

/* ip_length
 * Access: RW
 */
MLXSW_ITEM8(reg, pprr, ip_length, 0x1, 0, 4);

/* register_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, pprr, register_index, 0x3, 0, 8);

/* port_range_min
 * Access: RW
 */
MLXSW_ITEM16(reg, pprr, port_range_min, 0x4, 0, 16);

/* port_range_max
 * Access: RW
 */
MLXSW_ITEM16(reg, pprr, port_range_max, 0x6, 0, 16);

/* comp_type
 * Access: RW
 */
MLXSW_ITEM8(reg, pprr, comp_type, 0xb, 0, 2);

/* cbset
 * Access: RW
 */
MLXSW_ITEM8(reg, pprr, cbset, 0x13, 0, 8);

/* ppbs
 * ----
 *
 */
/* swid
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ppbs, swid, 0x4, 0, 8);

/* type
 * Access: RW
 */
MLXSW_ITEM8(reg, ppbs, type, 0x5, 4, 4);

/* pbs_ptr
 * Access: INDEX
 */
MLXSW_ITEM32(reg, ppbs, pbs_ptr, 0x8, 0, 24);

/* ppbs_unicast_v_fid
 * Access: RW
 */
MLXSW_ITEM8(reg, ppbs, ppbs_unicast_v_fid, 0xc, 7, 1);

/* ppbs_unicast_set_vid
 * Access: RW
 */
MLXSW_ITEM8(reg, ppbs, ppbs_unicast_set_vid, 0xc, 6, 1);

/* ppbs_unicast_vid
 * Access: RW
 */
MLXSW_ITEM16(reg, ppbs, ppbs_unicast_vid, 0xc, 0, 12);

/* ppbs_unicast_fid
 * Access: RW
 */
MLXSW_ITEM16(reg, ppbs, ppbs_unicast_fid, 0xe, 0, 16);

/* ppbs_unicast_action
 * Access: RW
 */
MLXSW_ITEM8(reg, ppbs, ppbs_unicast_action, 0x10, 4, 4);

/* ppbs_unicast_system_port
 * Access: RW
 */
MLXSW_ITEM16(reg, ppbs, ppbs_unicast_system_port, 0x12, 0, 16);

/* uvid
 * Access: RW
 */
MLXSW_ITEM8(reg, ppbs, uvid, 0xe, 7, 1);

/* vid
 * Access: RW
 */
MLXSW_ITEM16(reg, ppbs, vid, 0xe, 0, 12);

/* lag_id
 * Access: RW
 */
MLXSW_ITEM16(reg, ppbs, lag_id, 0x12, 0, 10);

/* ppbs_multicast_v_fid
 * Access: RW
 */
MLXSW_ITEM8(reg, ppbs, ppbs_multicast_v_fid, 0xc, 7, 1);

/* ppbs_multicast_pgi
 * Access: RW
 */
MLXSW_ITEM16(reg, ppbs, ppbs_multicast_pgi, 0xc, 0, 13);

/* ppbs_multicast_fid
 * Access: RW
 */
MLXSW_ITEM16(reg, ppbs, ppbs_multicast_fid, 0xe, 0, 16);

/* ppbs_multicast_mid
 * Access: RW
 */
MLXSW_ITEM16(reg, ppbs, ppbs_multicast_mid, 0x12, 0, 16);

/* udip
 * Access: RW
 */
MLXSW_ITEM32(reg, ppbs, udip, 0xc, 0, 32);

/* protocol
 * Access: RW
 */
MLXSW_ITEM8(reg, ppbs, protocol, 0x10, 3, 1);

/* gen_enc
 * Access: RW
 */
MLXSW_ITEM8(reg, ppbs, gen_enc, 0x10, 2, 1);

/* tunnel_port_lbf_bitmap
 * Access: RW
 */
MLXSW_ITEM16(reg, ppbs, tunnel_port_lbf_bitmap, 0x16, 0, 16);

/* ecmp_size
 * Access: RW
 */
MLXSW_ITEM16(reg, ppbs, ecmp_size, 0x1a, 0, 13);

/* multicast_tunnel_underlay_mc_ptr_msb
 * Access: RW
 */
MLXSW_ITEM16(reg, ppbs, multicast_tunnel_underlay_mc_ptr_msb, 0xc, 0, 16);

/* multicast_tunnel_fid
 * Access: RW
 */
MLXSW_ITEM16(reg, ppbs, multicast_tunnel_fid, 0xe, 0, 16);

/* multicast_tunnel_underlay_mc_ptr_lsb
 * Access: RW
 */
MLXSW_ITEM8(reg, ppbs, multicast_tunnel_underlay_mc_ptr_lsb, 0x10, 0, 8);

/* multicast_tunnel_v_fid
 * Access: RW
 */
MLXSW_ITEM8(reg, ppbs, multicast_tunnel_v_fid, 0x11, 7, 1);

/* multicast_tunnel_mid
 * Access: RW
 */
MLXSW_ITEM16(reg, ppbs, multicast_tunnel_mid, 0x12, 0, 16);

/* multicast_tunnel_lbf_tunnel_port_bitmap
 * Access: RW
 */
MLXSW_ITEM16(reg, ppbs, multicast_tunnel_lbf_tunnel_port_bitmap, 0x16, 0, 16);

/* multicast_tunnel_ecmp_size
 * Access: RW
 */
MLXSW_ITEM16(reg, ppbs, multicast_tunnel_ecmp_size, 0x1a, 0, 13);

/* peapbl
 * ------
 *
 */
/* region_id
 * Access: INDEX
 */
MLXSW_ITEM16(reg, peapbl, region_id, 0x2, 0, 16);

/* c_s
 * Access: WO
 */
MLXSW_ITEM8(reg, peapbl, c_s, 0x10, 7, 1);

/* pruning_ctcam_cs
 * Access: WO
 */
MLXSW_ITEM8(reg, peapbl, pruning_ctcam_cs, 0x10, 4, 1);

/* num_rec
 * Access: WO
 */
MLXSW_ITEM8(reg, peapbl, num_rec, 0x11, 0, 8);

/* erp_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, peapbl, erp_id, 0x13, 0, 4);

/* pruning_vector_cs
 * Access: WO
 */
MLXSW_ITEM16(reg, peapbl, pruning_vector_cs, 0x16, 0, 16);

/* peapbm
 * ------
 *
 */
/* region_id
 * Access: INDEX
 */
MLXSW_ITEM16(reg, peapbm, region_id, 0x2, 0, 16);

/* c_s
 * Access: WO
 */
MLXSW_ITEM8(reg, peapbm, c_s, 0x10, 7, 1);

/* pruning_ctcam_cs
 * Access: WO
 */
MLXSW_ITEM8(reg, peapbm, pruning_ctcam_cs, 0x10, 4, 1);

/* erp_id_mask
 * Access: INDEX
 */
MLXSW_ITEM8(reg, peapbm, erp_id_mask, 0x12, 0, 4);

/* erp_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, peapbm, erp_id, 0x13, 0, 4);

/* pruning_vector_cs
 * Access: WO
 */
MLXSW_ITEM16(reg, peapbm, pruning_vector_cs, 0x16, 0, 16);

/* priority_start
 * Access: INDEX
 */
MLXSW_ITEM32(reg, peapbm, priority_start, 0x18, 0, 24);

/* priority_end
 * Access: INDEX
 */
MLXSW_ITEM32(reg, peapbm, priority_end, 0x1c, 0, 24);

/* key
 * Access: INDEX
 */
MLXSW_ITEM32_INDEXED(reg, peapbm, key, 0x20, 0, 32, 4, 0, 4, 0);

/* key_mask
 * Access: INDEX
 */
MLXSW_ITEM32_INDEXED(reg, peapbm, key_mask, 0x80, 0, 32, 4, 0, 4, 0);

/* large_entry_key_id
 * Access: INDEX
 */
MLXSW_ITEM32(reg, peapbm, large_entry_key_id, 0xe0, 0, 24);

/* large_entry_key_id_mask
 * Access: INDEX
 */
MLXSW_ITEM32(reg, peapbm, large_entry_key_id_mask, 0xe4, 0, 24);

/* delta_start
 * Access: INDEX
 */
MLXSW_ITEM16(reg, peapbm, delta_start, 0xf2, 0, 10);

/* delta_mask
 * Access: INDEX
 */
MLXSW_ITEM8(reg, peapbm, delta_mask, 0xf5, 0, 8);

/* delta_value
 * Access: INDEX
 */
MLXSW_ITEM8(reg, peapbm, delta_value, 0xf7, 0, 8);

/* peaps
 * -----
 *
 */
/* busy
 * Access: RO
 */
MLXSW_ITEM8(reg, peaps, busy, 0x0, 7, 1);

/* ovf
 * Access: RO
 */
MLXSW_ITEM8(reg, peaps, ovf, 0x0, 6, 1);

/* ct_offset
 * Access: WO
 */
MLXSW_ITEM16(reg, peaps, ct_offset, 0xa, 0, 16);

/* ct_size
 * Access: WO
 */
MLXSW_ITEM16(reg, peaps, ct_size, 0xe, 0, 16);

/* tcam_region_info
 * Access: WO
 */
MLXSW_ITEM_BUF(reg, peaps, tcam_region_info, 0x10, 16);

/* priority_start
 * Access: WO
 */
MLXSW_ITEM32(reg, peaps, priority_start, 0x20, 0, 24);

/* priority_end
 * Access: WO
 */
MLXSW_ITEM32(reg, peaps, priority_end, 0x24, 0, 24);

/* priority_inc
 * Access: WO
 */
MLXSW_ITEM32(reg, peaps, priority_inc, 0x30, 0, 24);

/* pecnre
 * ------
 *
 */
/* region_id
 * Access: RW
 */
MLXSW_ITEM16(reg, pecnre, region_id, 0xa, 0, 16);

/* region_id_mask
 * Access: RW
 */
MLXSW_ITEM16(reg, pecnre, region_id_mask, 0xe, 0, 16);

/* pecnrr
 * ------
 *
 */
/* clear
 * Access: OP
 */
MLXSW_ITEM8(reg, pecnrr, clear, 0x0, 7, 1);

/* tcam_trigger_high
 * Access: RO
 */
MLXSW_ITEM32(reg, pecnrr, tcam_trigger_high, 0x10, 0, 32);

/* tcam_trigger_low
 * Access: RO
 */
MLXSW_ITEM32(reg, pecnrr, tcam_trigger_low, 0x14, 0, 32);

/* tcam_full_lookup_high
 * Access: RO
 */
MLXSW_ITEM32(reg, pecnrr, tcam_full_lookup_high, 0x18, 0, 32);

/* tcam_full_lookup_low
 * Access: RO
 */
MLXSW_ITEM32(reg, pecnrr, tcam_full_lookup_low, 0x1c, 0, 32);

/* pecnee
 * ------
 *
 */
/* region_id
 * Access: RW
 */
MLXSW_ITEM16(reg, pecnee, region_id, 0xa, 0, 16);

/* region_id_mask
 * Access: RW
 */
MLXSW_ITEM16(reg, pecnee, region_id_mask, 0xe, 0, 16);

/* erp_id_bitwise
 * Access: RW
 */
MLXSW_ITEM16(reg, pecnee, erp_id_bitwise, 0x12, 0, 16);

/* ctcam
 * Access: RW
 */
MLXSW_ITEM8(reg, pecnee, ctcam, 0x17, 0, 1);

/* pecner
 * ------
 *
 */
/* clear
 * Access: OP
 */
MLXSW_ITEM8(reg, pecner, clear, 0x0, 7, 1);

/* erp_initial_high
 * Access: RO
 */
MLXSW_ITEM32(reg, pecner, erp_initial_high, 0x10, 0, 32);

/* erp_initial_low
 * Access: RO
 */
MLXSW_ITEM32(reg, pecner, erp_initial_low, 0x14, 0, 32);

/* erp_post_bf_high
 * Access: RO
 */
MLXSW_ITEM32(reg, pecner, erp_post_bf_high, 0x18, 0, 32);

/* erp_post_bf_low
 * Access: RO
 */
MLXSW_ITEM32(reg, pecner, erp_post_bf_low, 0x1c, 0, 32);

/* erp_lookup_high
 * Access: RO
 */
MLXSW_ITEM32(reg, pecner, erp_lookup_high, 0x20, 0, 32);

/* erp_lookup_low
 * Access: RO
 */
MLXSW_ITEM32(reg, pecner, erp_lookup_low, 0x24, 0, 32);

/* erp_any_match_high
 * Access: RO
 */
MLXSW_ITEM32(reg, pecner, erp_any_match_high, 0x28, 0, 32);

/* erp_any_match_low
 * Access: RO
 */
MLXSW_ITEM32(reg, pecner, erp_any_match_low, 0x2c, 0, 32);

/* erp_final_match_high
 * Access: RO
 */
MLXSW_ITEM32(reg, pecner, erp_final_match_high, 0x30, 0, 32);

/* erp_final_match_low
 * Access: RO
 */
MLXSW_ITEM32(reg, pecner, erp_final_match_low, 0x34, 0, 32);

/* qgcr
 * ----
 *
 */
/* dwrr_unrestrict
 * Access: RW
 */
MLXSW_ITEM8(reg, qgcr, dwrr_unrestrict, 0x3, 0, 1);

/* tx_cnt_tclass
 * Access: RW
 */
MLXSW_ITEM8(reg, qgcr, tx_cnt_tclass, 0x7, 0, 1);

/* qcap
 * ----
 *
 */
/* max_policers_per_port
 * Access: RO
 */
MLXSW_ITEM8(reg, qcap, max_policers_per_port, 0xb, 0, 8);

/* max_policers_global
 * Access: RO
 */
MLXSW_ITEM8(reg, qcap, max_policers_global, 0xf, 0, 8);

/* max_policers_cpu
 * Access: RO
 */
MLXSW_ITEM8(reg, qcap, max_policers_cpu, 0x13, 0, 8);

/* qpcr
 * ----
 *
 */
/* g
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qpcr, g, 0x2, 6, 2);

/* pid
 * Access: INDEX
 */
MLXSW_ITEM16(reg, qpcr, pid, 0x2, 0, 14);

/* clear_counter
 * Access: OP
 */
MLXSW_ITEM8(reg, qpcr, clear_counter, 0x4, 7, 1);

/* add_counter
 * Access: OP
 */
MLXSW_ITEM8(reg, qpcr, add_counter, 0x4, 6, 1);

/* color_aware
 * Access: RW
 */
MLXSW_ITEM8(reg, qpcr, color_aware, 0x6, 7, 1);

/* bytes
 * Access: RW
 */
MLXSW_ITEM8(reg, qpcr, bytes, 0x6, 6, 1);

/* ir_units
 * Access: OP
 */
MLXSW_ITEM8(reg, qpcr, ir_units, 0x6, 4, 1);

/* type
 * Access: RW
 */
MLXSW_ITEM8(reg, qpcr, type, 0x6, 0, 2);

/* cbs
 * Access: RW
 */
MLXSW_ITEM8(reg, qpcr, cbs, 0x8, 0, 6);

/* ebs
 * Access: RW
 */
MLXSW_ITEM8(reg, qpcr, ebs, 0x9, 0, 6);

/* cir
 * Access: RW
 */
MLXSW_ITEM32(reg, qpcr, cir, 0xc, 0, 32);

/* eir
 * Access: RW
 */
MLXSW_ITEM32(reg, qpcr, eir, 0x10, 0, 32);

/* exceed_action
 * Access: RW
 */
MLXSW_ITEM8(reg, qpcr, exceed_action, 0x17, 0, 4);

/* violate_action
 * Access: RW
 */
MLXSW_ITEM8(reg, qpcr, violate_action, 0x1b, 0, 4);

/* violate_count_high
 * Access: RW
 */
MLXSW_ITEM32(reg, qpcr, violate_count_high, 0x20, 0, 32);

/* violate_count_low
 * Access: RW
 */
MLXSW_ITEM32(reg, qpcr, violate_count_low, 0x24, 0, 32);

/* qpbr
 * ----
 *
 */
/* op
 * Access: RW
 */
MLXSW_ITEM8(reg, qpbr, op, 0x0, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qpbr, lp_msb, 0x0, 0, 2);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qpbr, local_port, 0x1, 0, 8);

/* g
 * Access: RW
 */
MLXSW_ITEM8(reg, qpbr, g, 0x2, 7, 1);

/* pid
 * Access: RW
 */
MLXSW_ITEM16(reg, qpbr, pid, 0x2, 0, 14);

/* umc
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qpbr, umc, 0x7, 4, 1);

/* uuc
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qpbr, uuc, 0x7, 3, 1);

/* bc
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qpbr, bc, 0x7, 2, 1);

/* mc
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qpbr, mc, 0x7, 1, 1);

/* uc
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qpbr, uc, 0x7, 0, 1);

/* qeec
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qeec, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qeec, lp_msb, 0x2, 4, 2);

/* element_hierarchy
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qeec, element_hierarchy, 0x5, 0, 4);

/* element_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qeec, element_index, 0x7, 0, 8);

/* next_element_index
 * Access: RW
 */
MLXSW_ITEM8(reg, qeec, next_element_index, 0xb, 0, 8);

/* mise
 * Access: RW
 */
MLXSW_ITEM8(reg, qeec, mise, 0xc, 7, 1);

/* ptps
 * Access: RW
 */
MLXSW_ITEM8(reg, qeec, ptps, 0xc, 5, 1);

/* pb
 * Access: RW
 */
MLXSW_ITEM8(reg, qeec, pb, 0xc, 4, 1);

/* min_shaper_rate
 * Access: RW
 */
MLXSW_ITEM32(reg, qeec, min_shaper_rate, 0xc, 0, 28);

/* mase
 * Access: RW
 */
MLXSW_ITEM8(reg, qeec, mase, 0x10, 7, 1);

/* max_shaper_rate
 * Access: RW
 */
MLXSW_ITEM32(reg, qeec, max_shaper_rate, 0x10, 0, 31);

/* de
 * Access: RW
 */
MLXSW_ITEM8(reg, qeec, de, 0x18, 7, 1);

/* dwrr
 * Access: RW
 */
MLXSW_ITEM8(reg, qeec, dwrr, 0x1a, 7, 1);

/* dwrr_weight
 * Access: RW
 */
MLXSW_ITEM8(reg, qeec, dwrr_weight, 0x1b, 0, 8);

/* min_shaper_bs
 * Access: RW
 */
MLXSW_ITEM8(reg, qeec, min_shaper_bs, 0x1d, 0, 6);

/* max_shaper_bs
 * Access: RW
 */
MLXSW_ITEM8(reg, qeec, max_shaper_bs, 0x1f, 0, 6);

/* qecc
 * ----
 *
 */
/* cpu_split
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qecc, cpu_split, 0x3, 0, 4);

/* mode
 * Access: RW
 */
MLXSW_ITEM8(reg, qecc, mode, 0x7, 0, 4);

/* mase
 * Access: OP
 */
MLXSW_ITEM8(reg, qecc, mase, 0x8, 7, 1);

/* max_shaper_rate
 * Access: RW
 */
MLXSW_ITEM32(reg, qecc, max_shaper_rate, 0x8, 0, 31);

/* pb
 * Access: RW
 */
MLXSW_ITEM8(reg, qecc, pb, 0xc, 7, 1);

/* max_shaper_bs
 * Access: RW
 */
MLXSW_ITEM8(reg, qecc, max_shaper_bs, 0xf, 0, 6);

/* qtct
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qtct, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qtct, lp_msb, 0x2, 4, 2);

/* mc
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qtct, mc, 0x2, 0, 1);

/* switch_prio
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qtct, switch_prio, 0x3, 0, 4);

/* tclass
 * Access: RW
 */
MLXSW_ITEM8(reg, qtct, tclass, 0x7, 0, 4);

/* qtctm
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qtctm, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qtctm, lp_msb, 0x2, 4, 2);

/* mc
 * Access: RW
 */
MLXSW_ITEM8(reg, qtctm, mc, 0x7, 0, 2);

/* qspip
 * -----
 *
 */
/* switch_prio
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qspip, switch_prio, 0x3, 0, 4);

/* ieee_prio
 * Access: RW
 */
MLXSW_ITEM8(reg, qspip, ieee_prio, 0x7, 0, 3);

/* qspcp
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qspcp, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qspcp, lp_msb, 0x2, 4, 2);

/* switch_prio
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qspcp, switch_prio, 0x3, 0, 4);

/* rx_counting_prio
 * Access: RW
 */
MLXSW_ITEM8(reg, qspcp, rx_counting_prio, 0x7, 0, 3);

/* tx_counting_prio
 * Access: RW
 */
MLXSW_ITEM8(reg, qspcp, tx_counting_prio, 0xb, 0, 3);

/* qtttl
 * -----
 *
 */
/* tqos_profile
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qtttl, tqos_profile, 0x3, 0, 4);

/* decap_ttl
 * Access: RW
 */
MLXSW_ITEM8(reg, qtttl, decap_ttl, 0x5, 0, 2);

/* enc_ttlc
 * Access: RW
 */
MLXSW_ITEM8(reg, qtttl, enc_ttlc, 0x6, 0, 1);

/* enc_ttl_uc
 * Access: RW
 */
MLXSW_ITEM8(reg, qtttl, enc_ttl_uc, 0x7, 0, 8);

/* enc_ttl_mc
 * Access: RW
 */
MLXSW_ITEM8(reg, qtttl, enc_ttl_mc, 0xb, 0, 8);

/* qtqcr
 * -----
 *
 */
/* tqos_profile
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qtqcr, tqos_profile, 0x3, 0, 4);

/* enc_set_dscp
 * Access: RW
 */
MLXSW_ITEM8(reg, qtqcr, enc_set_dscp, 0x4, 4, 1);

/* enc_set_sp
 * Access: RW
 */
MLXSW_ITEM8(reg, qtqcr, enc_set_sp, 0x4, 0, 1);

/* enc_dscp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, qtqcr, enc_dscp_rw, 0x7, 2, 2);

/* enc_pcp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, qtqcr, enc_pcp_rw, 0x7, 0, 2);

/* dec_set_dscp
 * Access: RW
 */
MLXSW_ITEM8(reg, qtqcr, dec_set_dscp, 0x8, 4, 1);

/* dec_set_sp
 * Access: RW
 */
MLXSW_ITEM8(reg, qtqcr, dec_set_sp, 0x8, 0, 1);

/* dec_set_pcp
 * Access: RW
 */
MLXSW_ITEM8(reg, qtqcr, dec_set_pcp, 0x9, 4, 1);

/* dec_dscp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, qtqcr, dec_dscp_rw, 0xb, 2, 2);

/* dec_pcp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, qtqcr, dec_pcp_rw, 0xb, 0, 2);

/* qtqdr
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qtqdr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qtqdr, lp_msb, 0x2, 4, 2);

/* tqos_profile
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qtqdr, tqos_profile, 0x3, 0, 4);

/* color
 * Access: RW
 */
MLXSW_ITEM8(reg, qtqdr, color, 0x4, 0, 2);

/* switch_prio
 * Access: RW
 */
MLXSW_ITEM8(reg, qtqdr, switch_prio, 0x5, 0, 4);

/* exp
 * Access: RW
 */
MLXSW_ITEM8(reg, qtqdr, exp, 0x6, 0, 3);

/* dscp
 * Access: RW
 */
MLXSW_ITEM8(reg, qtqdr, dscp, 0x7, 0, 6);

/* qteem
 * -----
 *
 */
/* tqos_profile
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qteem, tqos_profile, 0x3, 0, 4);

/* overlay_ecn
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qteem, overlay_ecn, 0x4, 0, 2);

/* underlay_ecn
 * Access: RW
 */
MLXSW_ITEM8(reg, qteem, underlay_ecn, 0x5, 0, 2);

/* qtdem
 * -----
 *
 */
/* tqos_profile
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qtdem, tqos_profile, 0x3, 0, 4);

/* underlay_ecn
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qtdem, underlay_ecn, 0x4, 0, 2);

/* overlay_ecn
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qtdem, overlay_ecn, 0x5, 0, 2);

/* eip_ecn
 * Access: RW
 */
MLXSW_ITEM8(reg, qtdem, eip_ecn, 0x6, 0, 2);

/* trap_en
 * Access: RW
 */
MLXSW_ITEM8(reg, qtdem, trap_en, 0x8, 4, 4);

/* trap_id
 * Access: RW
 */
MLXSW_ITEM16(reg, qtdem, trap_id, 0xa, 0, 10);

/* qpdpc
 * -----
 *
 */
/* tport
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qpdpc, tport, 0x0, 4, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qpdpc, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qpdpc, lp_msb, 0x2, 4, 2);

/* dei
 * Access: RW
 */
MLXSW_ITEM8(reg, qpdpc, dei, 0x6, 0, 1);

/* pcp
 * Access: RW
 */
MLXSW_ITEM8(reg, qpdpc, pcp, 0x7, 0, 3);

/* qsll
 * ----
 *
 */
/* sll_time
 * Access: RW
 */
MLXSW_ITEM8(reg, qsll, sll_time, 0x7, 0, 5);

/* qhll
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qhll, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qhll, lp_msb, 0x2, 4, 2);

/* hll_time
 * Access: RW
 */
MLXSW_ITEM8(reg, qhll, hll_time, 0x7, 0, 5);

/* stall_en
 * Access: RW
 */
MLXSW_ITEM8(reg, qhll, stall_en, 0x8, 7, 1);

/* stall_cnt
 * Access: RW
 */
MLXSW_ITEM8(reg, qhll, stall_cnt, 0xb, 0, 3);

/* qptg
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qptg, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qptg, lp_msb, 0x2, 4, 2);

/* rx_group
 * Access: RW
 */
MLXSW_ITEM8(reg, qptg, rx_group, 0x5, 0, 8);

/* tx_group
 * Access: RW
 */
MLXSW_ITEM8(reg, qptg, tx_group, 0x7, 0, 8);

/* qtgtc
 * -----
 *
 */
/* rx_group
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qtgtc, rx_group, 0x5, 0, 8);

/* tx_group
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qtgtc, tx_group, 0x7, 0, 8);

/* switch_prio
 * Access: INDEX
 */
MLXSW_ITEM8(reg, qtgtc, switch_prio, 0xb, 0, 4);

/* disabled
 * Access: RW
 */
MLXSW_ITEM8(reg, qtgtc, disabled, 0xc, 7, 1);

/* tclass
 * Access: RW
 */
MLXSW_ITEM8(reg, qtgtc, tclass, 0xf, 0, 4);

/* cpqe
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, cpqe, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, cpqe, lp_msb, 0x2, 4, 2);

/* marking_percent
 * Access: RW
 */
MLXSW_ITEM8(reg, cpqe, marking_percent, 0x9, 0, 7);

/* en
 * Access: RW
 */
MLXSW_ITEM8(reg, cpqe, en, 0xc, 7, 1);

/* rate
 * Access: RW
 */
MLXSW_ITEM32(reg, cpqe, rate, 0xc, 0, 31);

/* bs
 * Access: RW
 */
MLXSW_ITEM8(reg, cpqe, bs, 0x13, 0, 6);

/* chltr
 * -----
 *
 */
/* hlt_table_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, chltr, hlt_table_index, 0x3, 0, 8);

/* high_latency_thr
 * Access: RW
 */
MLXSW_ITEM32(reg, chltr, high_latency_thr, 0x8, 0, 20);

/* chltm
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, chltm, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, chltm, lp_msb, 0x2, 4, 2);

/* traffic_class
 * Access: INDEX
 */
MLXSW_ITEM8(reg, chltm, traffic_class, 0x7, 0, 8);

/* hlt_table_pointer
 * Access: RW
 */
MLXSW_ITEM8(reg, chltm, hlt_table_pointer, 0x13, 0, 8);

/* chlmm
 * -----
 *
 */
/* switch_prio
 * Access: INDEX
 */
MLXSW_ITEM8(reg, chlmm, switch_prio, 0x2, 0, 4);

/* hlt_table_pointer
 * Access: RW
 */
MLXSW_ITEM8(reg, chlmm, hlt_table_pointer, 0x13, 0, 8);

/* cegcr
 * -----
 *
 */
/* f2e
 * Access: RW
 */
MLXSW_ITEM8(reg, cegcr, f2e, 0x3, 2, 1);

/* sticky
 * Access: RW
 */
MLXSW_ITEM8(reg, cegcr, sticky, 0x3, 1, 1);

/* max_floating_time
 * Access: RW
 */
MLXSW_ITEM16(reg, cegcr, max_floating_time, 0x6, 0, 10);

/* reference_rate
 * Access: RW
 */
MLXSW_ITEM32(reg, cegcr, reference_rate, 0x10, 0, 32);

/* elephants_thr
 * Access: RW
 */
MLXSW_ITEM32(reg, cegcr, elephants_thr, 0x14, 0, 20);

/* elephants_max_thr
 * Access: RW
 */
MLXSW_ITEM32(reg, cegcr, elephants_max_thr, 0x18, 0, 20);

/* mice_thr
 * Access: RW
 */
MLXSW_ITEM32(reg, cegcr, mice_thr, 0x1c, 0, 20);

/* cepc
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM16(reg, cepc, local_port, 0x0, 0, 9);

/* pp
 * Access: INDEX
 */
MLXSW_ITEM8(reg, cepc, pp, 0x2, 7, 1);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, cepc, lp_msb, 0x2, 4, 2);

/* en_ed
 * Access: RW
 */
MLXSW_ITEM8(reg, cepc, en_ed, 0x7, 0, 1);

/* cedr
 * ----
 *
 */
/* clear
 * Access: OP
 */
MLXSW_ITEM8(reg, cedr, clear, 0x0, 7, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, cedr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, cedr, lp_msb, 0x2, 4, 2);

/* detection_table
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, cedr, detection_table, 0x10, 0, 32, 4, 0, 4, 0);

/* ceer
 * ----
 *
 */
/* clear
 * Access: OP
 */
MLXSW_ITEM8(reg, ceer, clear, 0x0, 7, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ceer, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ceer, lp_msb, 0x2, 4, 2);

/* detection_entry
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ceer, detection_entry, 0x3, 0, 7);

/* det
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, det, 0x4, 7, 1);

/* l2_type
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, l2_type, 0xf, 0, 4);

/* extra_data_type
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, extra_data_type, 0x10, 4, 4);

/* tunnel_type
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, tunnel_type, 0x10, 0, 4);

/* l4_type
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, l4_type, 0x11, 0, 4);

/* l3_type
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, l3_type, 0x12, 0, 4);

/* vlan_type
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, vlan_type, 0x13, 0, 4);

/* hash_valid
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, hash_valid, 0x18, 7, 1);

/* lag_hash
 * Access: RO
 */
MLXSW_ITEM16(reg, ceer, lag_hash, 0x1a, 0, 12);

/* router_hash
 * Access: RO
 */
MLXSW_ITEM16(reg, ceer, router_hash, 0x1e, 0, 12);

/* dmac
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, ceer, dmac, 0x22, 6);

/* smac
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, ceer, smac, 0x2a, 6);

/* pcp
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, pcp, 0x32, 5, 3);

/* dei
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, dei, 0x32, 4, 1);

/* vid
 * Access: RO
 */
MLXSW_ITEM16(reg, ceer, vid, 0x32, 0, 12);

/* inner_pcp
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, inner_pcp, 0x36, 5, 3);

/* inner_dei
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, inner_dei, 0x36, 4, 1);

/* inner_vid
 * Access: RO
 */
MLXSW_ITEM16(reg, ceer, inner_vid, 0x36, 0, 12);

/* dscp
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, dscp, 0x39, 2, 6);

/* ecn
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, ecn, 0x39, 0, 2);

/* ttl
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, ttl, 0x3a, 0, 8);

/* dont_frag
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, dont_frag, 0x3b, 0, 1);

/* sport
 * Access: RO
 */
MLXSW_ITEM16(reg, ceer, sport, 0x3c, 0, 16);

/* dport
 * Access: RO
 */
MLXSW_ITEM16(reg, ceer, dport, 0x3e, 0, 16);

/* sip
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, ceer, sip, 0x40, 0, 32, 4, 0, 4, 0);

/* dip
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, ceer, dip, 0x50, 0, 32, 4, 0, 4, 0);

/* vni
 * Access: RO
 */
MLXSW_ITEM32(reg, ceer, vni, 0x60, 0, 24);

/* inner_dmac
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, ceer, inner_dmac, 0x6a, 6);

/* inner_smac
 * Access: RO
 */
MLXSW_ITEM_BUF(reg, ceer, inner_smac, 0x72, 6);

/* inner_l3_type
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, inner_l3_type, 0x7c, 0, 4);

/* inner_dscp
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, inner_dscp, 0x7d, 2, 6);

/* inner_ecn
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, inner_ecn, 0x7d, 0, 2);

/* inner_ttl
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, inner_ttl, 0x7e, 0, 8);

/* inner_dont_frag
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, inner_dont_frag, 0x7f, 0, 1);

/* inner_sip
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, ceer, inner_sip, 0x80, 0, 32, 4, 0, 4, 0);

/* inner_dip
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, ceer, inner_dip, 0x90, 0, 32, 4, 0, 4, 0);

/* mpls_bos
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, mpls_bos, 0x60, 7, 1);

/* mpls_exp
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, mpls_exp, 0x63, 0, 3);

/* mpls_lb0_v
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, mpls_lb0_v, 0x64, 7, 1);

/* mpls_label0_label_id
 * Access: RO
 */
MLXSW_ITEM32(reg, ceer, mpls_label0_label_id, 0x64, 8, 20);

/* mpls_label0_ttl
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, mpls_label0_ttl, 0x67, 0, 8);

/* mpls_lb1_v
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, mpls_lb1_v, 0x68, 7, 1);

/* mpls_label1_label_id
 * Access: RO
 */
MLXSW_ITEM32(reg, ceer, mpls_label1_label_id, 0x68, 8, 20);

/* mpls_label1_ttl
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, mpls_label1_ttl, 0x6b, 0, 8);

/* mpls_lb2_v
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, mpls_lb2_v, 0x6c, 7, 1);

/* mpls_label2_label_id
 * Access: RO
 */
MLXSW_ITEM32(reg, ceer, mpls_label2_label_id, 0x6c, 8, 20);

/* mpls_label2_ttl
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, mpls_label2_ttl, 0x6f, 0, 8);

/* mpls_lb3_v
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, mpls_lb3_v, 0x70, 7, 1);

/* mpls_label3_label_id
 * Access: RO
 */
MLXSW_ITEM32(reg, ceer, mpls_label3_label_id, 0x70, 8, 20);

/* mpls_label3_ttl
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, mpls_label3_ttl, 0x73, 0, 8);

/* mpls_lb4_v
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, mpls_lb4_v, 0x74, 7, 1);

/* mpls_label4_label_id
 * Access: RO
 */
MLXSW_ITEM32(reg, ceer, mpls_label4_label_id, 0x74, 8, 20);

/* mpls_label4_ttl
 * Access: RO
 */
MLXSW_ITEM8(reg, ceer, mpls_label4_ttl, 0x77, 0, 8);

/* cwgcr
 * -----
 *
 */
/* en
 * Access: RW
 */
MLXSW_ITEM8(reg, cwgcr, en, 0x0, 0, 4);

/* scd
 * Access: RW
 */
MLXSW_ITEM8(reg, cwgcr, scd, 0x1, 0, 1);

/* aqs_weight
 * Access: RW
 */
MLXSW_ITEM8(reg, cwgcr, aqs_weight, 0x2, 0, 4);

/* aqs_time
 * Access: RW
 */
MLXSW_ITEM8(reg, cwgcr, aqs_time, 0x3, 0, 8);

/* ece_inner_en
 * Access: RW
 */
MLXSW_ITEM8(reg, cwgcr, ece_inner_en, 0x7, 4, 1);

/* mece
 * Access: RW
 */
MLXSW_ITEM8(reg, cwgcr, mece, 0x7, 1, 1);

/* cece
 * Access: RW
 */
MLXSW_ITEM8(reg, cwgcr, cece, 0x7, 0, 1);

/* selector
 * Access: RW
 */
MLXSW_ITEM8(reg, cwgcr, selector, 0xb, 4, 4);

/* cwtpm
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, cwtpm, local_port, 0x1, 0, 8);

/* pnat
 * Access: INDEX
 */
MLXSW_ITEM8(reg, cwtpm, pnat, 0x2, 6, 2);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, cwtpm, lp_msb, 0x2, 4, 2);

/* traffic_class
 * Access: INDEX
 */
MLXSW_ITEM8(reg, cwtpm, traffic_class, 0x23, 0, 8);

/* ece
 * Access: RW
 */
MLXSW_ITEM8(reg, cwtpm, ece, 0x27, 2, 1);

/* ew
 * Access: RW
 */
MLXSW_ITEM8(reg, cwtpm, ew, 0x27, 1, 1);

/* ee
 * Access: RW
 */
MLXSW_ITEM8(reg, cwtpm, ee, 0x27, 0, 1);

/* tcp_g
 * Access: RW
 */
MLXSW_ITEM8(reg, cwtpm, tcp_g, 0x37, 0, 2);

/* tcp_y
 * Access: RW
 */
MLXSW_ITEM8(reg, cwtpm, tcp_y, 0x39, 0, 2);

/* tcp_r
 * Access: RW
 */
MLXSW_ITEM8(reg, cwtpm, tcp_r, 0x3b, 0, 2);

/* ntcp_g
 * Access: RW
 */
MLXSW_ITEM8(reg, cwtpm, ntcp_g, 0x3f, 0, 2);

/* ntcp_y
 * Access: RW
 */
MLXSW_ITEM8(reg, cwtpm, ntcp_y, 0x41, 0, 2);

/* ntcp_r
 * Access: RW
 */
MLXSW_ITEM8(reg, cwtpm, ntcp_r, 0x43, 0, 2);

/* rigr_v2
 * -------
 *
 */
/* list_type
 * Access: RW
 */
MLXSW_ITEM8(reg, rigr_v2, list_type, 0x1, 4, 4);

/* rigr_index
 * Access: INDEX
 */
MLXSW_ITEM32(reg, rigr_v2, rigr_index, 0x4, 0, 24);

/* vnext
 * Access: RW
 */
MLXSW_ITEM8(reg, rigr_v2, vnext, 0x8, 7, 1);

/* next_rigr_index
 * Access: RW
 */
MLXSW_ITEM32(reg, rigr_v2, next_rigr_index, 0x8, 0, 24);

/* rigr_erif_list_vrmid
 * Access: RW
 */
MLXSW_ITEM8(reg, rigr_v2, rigr_erif_list_vrmid, 0x20, 7, 1);

/* rigr_erif_list_rmid_index
 * Access: RW
 */
MLXSW_ITEM16(reg, rigr_v2, rigr_erif_list_rmid_index, 0x22, 0, 16);

/* rigr_erif_list_v
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, rigr_v2, rigr_erif_list_v, MLXSW_RIGR_V2_RIGR_ERIF_LIST_ERIF_ENTRY_BASE_LEN, 7, 1, MLXSW_RIGR_V2_ERIF_ENTRY_REC_LEN, 0, 1, 0);

/* rigr_erif_list_erif
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, rigr_v2, rigr_erif_list_erif, MLXSW_RIGR_V2_RIGR_ERIF_LIST_ERIF_ENTRY_BASE_LEN + 2, 0, 16, MLXSW_RIGR_V2_ERIF_ENTRY_REC_LEN, 0, 2, 0);

/* rigr_erif_list_adabe_reserved
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, rigr_v2, rigr_erif_list_adabe_reserved, 0xa4, 0, 32, 4, 0, 4, 0);

/* rigr_list_bit_vector_vrmid
 * Access: RW
 */
MLXSW_ITEM8(reg, rigr_v2, rigr_list_bit_vector_vrmid, 0x20, 7, 1);

/* rigr_list_bit_vector_rmid_index
 * Access: RW
 */
MLXSW_ITEM16(reg, rigr_v2, rigr_list_bit_vector_rmid_index, 0x22, 0, 16);

/* rigr_list_bit_vector_offset
 * Access: RW
 */
MLXSW_ITEM8(reg, rigr_v2, rigr_list_bit_vector_offset, 0x27, 0, 8);

/* rigr_list_bit_vector_erif_bit_vector
 * Access: RW
 */
MLXSW_ITEM64(reg, rigr_v2, rigr_list_bit_vector_erif_bit_vector, 0x28, 0, 64);

/* rigr_nve_decap_uerif
 * Access: RW
 */
MLXSW_ITEM16(reg, rigr_v2, rigr_nve_decap_uerif, 0x22, 0, 16);

/* rigr_nve_decap_tqos_profile_en
 * Access: OP
 */
MLXSW_ITEM8(reg, rigr_v2, rigr_nve_decap_tqos_profile_en, 0x24, 7, 1);

/* rigr_nve_decap_tqos_profile
 * Access: RW
 */
MLXSW_ITEM8(reg, rigr_v2, rigr_nve_decap_tqos_profile, 0x24, 0, 4);

/* rigr_nve_decap_decap_disable
 * Access: RW
 */
MLXSW_ITEM8(reg, rigr_v2, rigr_nve_decap_decap_disable, 0x27, 0, 4);

/* rigr_generic_checks_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, rigr_v2, rigr_generic_checks_mode, 0x22, 0, 2);

/* rigr_generic_allow_decap
 * Access: RW
 */
MLXSW_ITEM8(reg, rigr_v2, rigr_generic_allow_decap, 0x23, 0, 3);

/* rigr_generic_tqos_profile_l2
 * Access: RW
 */
MLXSW_ITEM8(reg, rigr_v2, rigr_generic_tqos_profile_l2, 0x24, 0, 4);

/* rigr_generic_tqos_profile_l3
 * Access: RW
 */
MLXSW_ITEM8(reg, rigr_v2, rigr_generic_tqos_profile_l3, 0x25, 0, 4);

/* rigr_generic_tunnel_port
 * Access: RW
 */
MLXSW_ITEM8(reg, rigr_v2, rigr_generic_tunnel_port, 0x27, 0, 4);

/* rigr_generic_checks_nve_decap_disable
 * Access: RW
 */
MLXSW_ITEM8(reg, rigr_v2, rigr_generic_checks_nve_decap_disable, 0x28, 0, 4);

/* rigr_generic_irif
 * Access: RW
 */
MLXSW_ITEM16(reg, rigr_v2, rigr_generic_irif, 0x2a, 0, 16);

/* rigr_generic_gre_key_check
 * Access: RW
 */
MLXSW_ITEM8(reg, rigr_v2, rigr_generic_gre_key_check, 0x2d, 0, 1);

/* rigr_generic_type_check
 * Access: RW
 */
MLXSW_ITEM8(reg, rigr_v2, rigr_generic_type_check, 0x2e, 0, 3);

/* rigr_generic_sip_check
 * Access: RW
 */
MLXSW_ITEM8(reg, rigr_v2, rigr_generic_sip_check, 0x2f, 0, 3);

/* rigr_generic_ipv4_usip
 * Access: RW
 */
MLXSW_ITEM32(reg, rigr_v2, rigr_generic_ipv4_usip, 0x30, 0, 32);

/* rigr_generic_ipv6_usip_ptr
 * Access: RW
 */
MLXSW_ITEM32(reg, rigr_v2, rigr_generic_ipv6_usip_ptr, 0x34, 0, 24);

/* rigr_generic_expected_gre_key
 * Access: RW
 */
MLXSW_ITEM32(reg, rigr_v2, rigr_generic_expected_gre_key, 0x38, 0, 32);

/* rigr_generic_uerif
 * Access: RW
 */
MLXSW_ITEM16(reg, rigr_v2, rigr_generic_uerif, 0x3e, 0, 16);

/* rmftad
 * ------
 *
 */
/* op
 * Access: OP
 */
MLXSW_ITEM8(reg, rmftad, op, 0x0, 6, 2);

/* type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rmftad, type, 0x1, 4, 2);

/* offset
 * Access: INDEX
 */
MLXSW_ITEM16(reg, rmftad, offset, 0x2, 0, 16);

/* num_rec
 * Access: OP
 */
MLXSW_ITEM16(reg, rmftad, num_rec, 0x6, 0, 13);

/* activity_vector
 * Access: RO
 */
MLXSW_ITEM32_INDEXED(reg, rmftad, activity_vector, 0x20, 0, 32, 4, 0, 4, 0);

/* ratrb
 * -----
 *
 */
/* v
 * Access: OP
 */
MLXSW_ITEM8(reg, ratrb, v, 0x0, 7, 1);

/* adjacency_index
 * Access: INDEX
 */
MLXSW_ITEM32(reg, ratrb, adjacency_index, 0x0, 0, 24);

/* num_rec
 * Access: OP
 */
MLXSW_ITEM8(reg, ratrb, num_rec, 0x7, 0, 8);

/* trap_action
 * Access: WO
 */
MLXSW_ITEM8(reg, ratrb, trap_action, 0x8, 4, 4);

/* trap_id
 * Access: WO
 */
MLXSW_ITEM16(reg, ratrb, trap_id, 0xa, 0, 9);

/* type
 * Access: WO
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, type, MLXSW_RATRB_RECORD_BASE_LEN, 4, 4, MLXSW_RATRB_RECORD_REC_LEN, 0, 1, 0);

/* size
 * Access: WO
 */
MLXSW_ITEM16_INDEXED(reg, ratrb, size, MLXSW_RATRB_RECORD_BASE_LEN, 0, 12, MLXSW_RATRB_RECORD_REC_LEN, 0, 2, 0);

/* egress_router_interface
 * Access: WO
 */
MLXSW_ITEM16_INDEXED(reg, ratrb, egress_router_interface, MLXSW_RATRB_RECORD_BASE_LEN + 2, 0, 16, MLXSW_RATRB_RECORD_REC_LEN, 0, 2, 0);

/* counter_set_type
 * Access: WO
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, counter_set_type, MLXSW_RATRB_RECORD_BASE_LEN, 0, 8, MLXSW_RATRB_RECORD_REC_LEN, 4, 1, 0);

/* counter_index
 * Access: WO
 */
MLXSW_ITEM32_INDEXED(reg, ratrb, counter_index, MLXSW_RATRB_RECORD_BASE_LEN, 0, 24, MLXSW_RATRB_RECORD_REC_LEN, 4, 3, 0);

/* destination_mac
 * Access: RW
 */
MLXSW_ITEM_BUF_INDEXED(reg, ratrb, destination_mac, MLXSW_RATRB_RECORD_BASE_LEN, 0x6, 0x20, 10, 1, 0);

/* mpls_adjacency_nhlfe_ptr
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ratrb, mpls_adjacency_nhlfe_ptr, MLXSW_RATRB_RECORD_BASE_LEN, 0, 24, MLXSW_RATRB_RECORD_REC_LEN, 8, 3, 0);

/* mpls_adjacency_tqos_profile
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, mpls_adjacency_tqos_profile, MLXSW_RATRB_RECORD_BASE_LEN, 0, 4, MLXSW_RATRB_RECORD_REC_LEN, 12, 1, 0);

/* mpls_adjacency_ecmp_size
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ratrb, mpls_adjacency_ecmp_size, MLXSW_RATRB_RECORD_BASE_LEN + 2, 0, 13, MLXSW_RATRB_RECORD_REC_LEN, 12, 2, 0);

/* mpls_adjacency_underlay_router_interface
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ratrb, mpls_adjacency_underlay_router_interface, MLXSW_RATRB_RECORD_BASE_LEN + 2, 0, 16, MLXSW_RATRB_RECORD_REC_LEN, 24, 2, 0);

/* ipinip_encap_adjacency_ipip_type
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, ipinip_encap_adjacency_ipip_type, MLXSW_RATRB_RECORD_BASE_LEN + 1, 0, 4, MLXSW_RATRB_RECORD_REC_LEN, 8, 1, 0);

/* ipinip_encap_adjacency_tqos_profile_en
 * Access: OP
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, ipinip_encap_adjacency_tqos_profile_en, MLXSW_RATRB_RECORD_BASE_LEN, 7, 1, MLXSW_RATRB_RECORD_REC_LEN, 12, 1, 0);

/* ipinip_encap_adjacency_tqos_profile
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, ipinip_encap_adjacency_tqos_profile, MLXSW_RATRB_RECORD_BASE_LEN, 0, 4, MLXSW_RATRB_RECORD_REC_LEN, 12, 1, 0);

/* ipinip_encap_adjacency_uirif_v
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, ipinip_encap_adjacency_uirif_v, MLXSW_RATRB_RECORD_BASE_LEN + 1, 0, 1, MLXSW_RATRB_RECORD_REC_LEN, 12, 1, 0);

/* ipinip_encap_adjacency_uirif
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ratrb, ipinip_encap_adjacency_uirif, MLXSW_RATRB_RECORD_BASE_LEN + 2, 0, 16, MLXSW_RATRB_RECORD_REC_LEN, 12, 2, 0);

/* ipinip_encap_adjacency_ipv4_udip
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ratrb, ipinip_encap_adjacency_ipv4_udip, MLXSW_RATRB_RECORD_BASE_LEN, 0, 32, MLXSW_RATRB_RECORD_REC_LEN, 16, 4, 0);

/* ipinip_encap_adjacency_ipv6_ptr
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ratrb, ipinip_encap_adjacency_ipv6_ptr, MLXSW_RATRB_RECORD_BASE_LEN, 0, 24, MLXSW_RATRB_RECORD_REC_LEN, 20, 3, 0);

/* l3_generic_encap_adjacency_u_ip_ver
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, l3_generic_encap_adjacency_u_ip_ver, MLXSW_RATRB_RECORD_BASE_LEN + 1, 0, 4, MLXSW_RATRB_RECORD_REC_LEN, 8, 1, 0);

/* l3_generic_encap_adjacency_emt_pointer
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, l3_generic_encap_adjacency_emt_pointer, MLXSW_RATRB_RECORD_BASE_LEN + 3, 0, 8, MLXSW_RATRB_RECORD_REC_LEN, 8, 1, 0);

/* l3_generic_encap_adjacency_tqos_profile
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, l3_generic_encap_adjacency_tqos_profile, MLXSW_RATRB_RECORD_BASE_LEN, 0, 4, MLXSW_RATRB_RECORD_REC_LEN, 12, 1, 0);

/* l3_generic_encap_adjacency_uirif
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ratrb, l3_generic_encap_adjacency_uirif, MLXSW_RATRB_RECORD_BASE_LEN + 2, 0, 16, MLXSW_RATRB_RECORD_REC_LEN, 12, 2, 0);

/* l3_generic_encap_adjacency_ipv4_udip
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ratrb, l3_generic_encap_adjacency_ipv4_udip, MLXSW_RATRB_RECORD_BASE_LEN, 0, 32, MLXSW_RATRB_RECORD_REC_LEN, 16, 4, 0);

/* l3_generic_encap_adjacency_ipv6_ptr
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ratrb, l3_generic_encap_adjacency_ipv6_ptr, MLXSW_RATRB_RECORD_BASE_LEN, 0, 24, MLXSW_RATRB_RECORD_REC_LEN, 20, 3, 0);

/* l3_generic_encap_adjacency_cbset_x_select
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, l3_generic_encap_adjacency_cbset_x_select, MLXSW_RATRB_RECORD_BASE_LEN, 0, 3, MLXSW_RATRB_RECORD_REC_LEN, 24, 1, 0);

/* l3_generic_encap_adjacency_cbset_y_select
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, l3_generic_encap_adjacency_cbset_y_select, MLXSW_RATRB_RECORD_BASE_LEN + 1, 0, 3, MLXSW_RATRB_RECORD_REC_LEN, 24, 1, 0);

/* l3_generic_encap_adjacency_cbset_x_set_en
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, l3_generic_encap_adjacency_cbset_x_set_en, MLXSW_RATRB_RECORD_BASE_LEN + 3, 1, 1, MLXSW_RATRB_RECORD_REC_LEN, 24, 1, 0);

/* l3_generic_encap_adjacency_cbset_y_set_en
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, l3_generic_encap_adjacency_cbset_y_set_en, MLXSW_RATRB_RECORD_BASE_LEN + 3, 0, 1, MLXSW_RATRB_RECORD_REC_LEN, 24, 1, 0);

/* l3_generic_encap_adjacency_cbset_x_val
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ratrb, l3_generic_encap_adjacency_cbset_x_val, MLXSW_RATRB_RECORD_BASE_LEN, 0, 16, MLXSW_RATRB_RECORD_REC_LEN, 28, 2, 0);

/* l3_generic_encap_adjacency_cbset_y_val
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ratrb, l3_generic_encap_adjacency_cbset_y_val, MLXSW_RATRB_RECORD_BASE_LEN + 2, 0, 16, MLXSW_RATRB_RECORD_REC_LEN, 28, 2, 0);

/* new_header_type
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, new_header_type, MLXSW_RATRB_RECORD_BASE_LEN, 0, 4, MLXSW_RATRB_RECORD_REC_LEN, 8, 1, 0);

/* irif
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ratrb, irif, MLXSW_RATRB_RECORD_BASE_LEN + 2, 0, 16, MLXSW_RATRB_RECORD_REC_LEN, 8, 2, 0);

/* qos_profile
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, qos_profile, MLXSW_RATRB_RECORD_BASE_LEN, 0, 4, MLXSW_RATRB_RECORD_REC_LEN, 12, 1, 0);

/* new_ipv4_udip
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ratrb, new_ipv4_udip, MLXSW_RATRB_RECORD_BASE_LEN, 0, 32, MLXSW_RATRB_RECORD_REC_LEN, 16, 4, 0);

/* new_ipv6_ptr
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ratrb, new_ipv6_ptr, MLXSW_RATRB_RECORD_BASE_LEN, 0, 24, MLXSW_RATRB_RECORD_REC_LEN, 20, 3, 0);

/* exp_l3
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, exp_l3, MLXSW_RATRB_RECORD_BASE_LEN + 3, 0, 2, MLXSW_RATRB_RECORD_REC_LEN, 24, 1, 0);

/* relookup_ecmp_adjacency_next_adjacency_ptr
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ratrb, relookup_ecmp_adjacency_next_adjacency_ptr, MLXSW_RATRB_RECORD_BASE_LEN, 0, 24, MLXSW_RATRB_RECORD_REC_LEN, 8, 3, 0);

/* relookup_ecmp_adjacency_next_ecmp_size
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ratrb, relookup_ecmp_adjacency_next_ecmp_size, MLXSW_RATRB_RECORD_BASE_LEN + 2, 0, 13, MLXSW_RATRB_RECORD_REC_LEN, 12, 2, 0);

/* relookup_ecmp_adjacency_rd
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, relookup_ecmp_adjacency_rd, MLXSW_RATRB_RECORD_BASE_LEN + 1, 0, 1, MLXSW_RATRB_RECORD_REC_LEN, 16, 1, 0);

/* relookup_ecmp_adjacency_rehash_seed
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ratrb, relookup_ecmp_adjacency_rehash_seed, MLXSW_RATRB_RECORD_BASE_LEN + 2, 0, 16, MLXSW_RATRB_RECORD_REC_LEN, 16, 2, 0);

/* relookup_lpm_adjacency_virtual_router
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ratrb, relookup_lpm_adjacency_virtual_router, MLXSW_RATRB_RECORD_BASE_LEN + 2, 0, 16, MLXSW_RATRB_RECORD_REC_LEN, 8, 2, 0);

/* relookup_lpm_adjacency_rd
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, relookup_lpm_adjacency_rd, MLXSW_RATRB_RECORD_BASE_LEN + 1, 0, 1, MLXSW_RATRB_RECORD_REC_LEN, 16, 1, 0);

/* relookup_lpm_adjacency_rehash_seed
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ratrb, relookup_lpm_adjacency_rehash_seed, MLXSW_RATRB_RECORD_BASE_LEN + 2, 0, 16, MLXSW_RATRB_RECORD_REC_LEN, 16, 2, 0);

/* arn_encap_adjacency_dip_type
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, arn_encap_adjacency_dip_type, MLXSW_RATRB_RECORD_BASE_LEN + 1, 0, 2, MLXSW_RATRB_RECORD_REC_LEN, 8, 1, 0);

/* arn_encap_adjacency_tqos_profile
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, arn_encap_adjacency_tqos_profile, MLXSW_RATRB_RECORD_BASE_LEN, 0, 4, MLXSW_RATRB_RECORD_REC_LEN, 12, 1, 0);

/* arn_encap_adjacency_uirif
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ratrb, arn_encap_adjacency_uirif, MLXSW_RATRB_RECORD_BASE_LEN + 2, 0, 16, MLXSW_RATRB_RECORD_REC_LEN, 12, 2, 0);

/* arn_encap_adjacency_ipv4_udip
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ratrb, arn_encap_adjacency_ipv4_udip, MLXSW_RATRB_RECORD_BASE_LEN, 0, 32, MLXSW_RATRB_RECORD_REC_LEN, 16, 4, 0);

/* arn_encap_adjacency_ipv6_ptr
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ratrb, arn_encap_adjacency_ipv6_ptr, MLXSW_RATRB_RECORD_BASE_LEN, 0, 24, MLXSW_RATRB_RECORD_REC_LEN, 20, 3, 0);

/* arn_encap_adjacency_vnext
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ratrb, arn_encap_adjacency_vnext, MLXSW_RATRB_RECORD_BASE_LEN, 7, 1, MLXSW_RATRB_RECORD_REC_LEN, 24, 1, 0);

/* arn_encap_adjacency_next_arn_ptr
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ratrb, arn_encap_adjacency_next_arn_ptr, MLXSW_RATRB_RECORD_BASE_LEN, 0, 24, MLXSW_RATRB_RECORD_REC_LEN, 24, 3, 0);

/* rtdp
 * ----
 *
 */
/* type
 * Access: RW
 */
MLXSW_ITEM8(reg, rtdp, type, 0x0, 4, 4);

/* tunnel_index
 * Access: INDEX
 */
MLXSW_ITEM32(reg, rtdp, tunnel_index, 0x0, 0, 24);

/* rtdp_nve_decap_tqos_profile_en
 * Access: OP
 */
MLXSW_ITEM8(reg, rtdp, rtdp_nve_decap_tqos_profile_en, 0x6, 7, 1);

/* rtdp_nve_decap_tqos_profile
 * Access: RW
 */
MLXSW_ITEM8(reg, rtdp, rtdp_nve_decap_tqos_profile, 0x6, 0, 4);

/* rtdp_nve_decap_decap_disable
 * Access: RW
 */
MLXSW_ITEM8(reg, rtdp, rtdp_nve_decap_decap_disable, 0x7, 0, 4);

/* rtdp_ipinip_irif
 * Access: RW
 */
MLXSW_ITEM16(reg, rtdp, rtdp_ipinip_irif, 0x4, 0, 16);

/* rtdp_ipinip_sip_check
 * Access: RW
 */
MLXSW_ITEM8(reg, rtdp, rtdp_ipinip_sip_check, 0x7, 0, 3);

/* rtdp_ipinip_type_check
 * Access: RW
 */
MLXSW_ITEM8(reg, rtdp, rtdp_ipinip_type_check, 0x8, 0, 3);

/* rtdp_ipinip_gre_key_check
 * Access: RW
 */
MLXSW_ITEM8(reg, rtdp, rtdp_ipinip_gre_key_check, 0x9, 7, 1);

/* rtdp_ipinip_tqos_profile_en
 * Access: OP
 */
MLXSW_ITEM8(reg, rtdp, rtdp_ipinip_tqos_profile_en, 0xb, 7, 1);

/* rtdp_ipinip_tqos_profile
 * Access: RW
 */
MLXSW_ITEM8(reg, rtdp, rtdp_ipinip_tqos_profile, 0xb, 0, 4);

/* rtdp_ipinip_ipv4_usip
 * Access: RW
 */
MLXSW_ITEM32(reg, rtdp, rtdp_ipinip_ipv4_usip, 0xc, 0, 32);

/* rtdp_ipinip_ipv6_usip_ptr
 * Access: RW
 */
MLXSW_ITEM32(reg, rtdp, rtdp_ipinip_ipv6_usip_ptr, 0x10, 0, 24);

/* rtdp_ipinip_expected_gre_key
 * Access: RW
 */
MLXSW_ITEM32(reg, rtdp, rtdp_ipinip_expected_gre_key, 0x14, 0, 32);

/* checks_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, rtdp, checks_mode, 0x6, 0, 2);

/* allow_decap
 * Access: RW
 */
MLXSW_ITEM8(reg, rtdp, allow_decap, 0x7, 0, 3);

/* tqos_profile_l2
 * Access: RW
 */
MLXSW_ITEM8(reg, rtdp, tqos_profile_l2, 0x8, 0, 4);

/* tqos_profile_l3
 * Access: RW
 */
MLXSW_ITEM8(reg, rtdp, tqos_profile_l3, 0x9, 0, 4);

/* tunnel_port
 * Access: RW
 */
MLXSW_ITEM8(reg, rtdp, tunnel_port, 0xb, 0, 4);

/* checks_nve_decap_disable
 * Access: RW
 */
MLXSW_ITEM8(reg, rtdp, checks_nve_decap_disable, 0xc, 0, 4);

/* irif
 * Access: RW
 */
MLXSW_ITEM16(reg, rtdp, irif, 0xe, 0, 16);

/* gre_key_check
 * Access: RW
 */
MLXSW_ITEM8(reg, rtdp, gre_key_check, 0x11, 0, 1);

/* type_check
 * Access: RW
 */
MLXSW_ITEM8(reg, rtdp, type_check, 0x12, 0, 3);

/* sip_check
 * Access: RW
 */
MLXSW_ITEM8(reg, rtdp, sip_check, 0x13, 0, 3);

/* ipv4_usip
 * Access: RW
 */
MLXSW_ITEM32(reg, rtdp, ipv4_usip, 0x14, 0, 32);

/* ipv6_usip_ptr
 * Access: RW
 */
MLXSW_ITEM32(reg, rtdp, ipv6_usip_ptr, 0x18, 0, 24);

/* expected_gre_key
 * Access: RW
 */
MLXSW_ITEM32(reg, rtdp, expected_gre_key, 0x1c, 0, 32);

/* egress_router_interface
 * Access: RW
 */
MLXSW_ITEM16(reg, rtdp, egress_router_interface, 0x42, 0, 16);

/* rips
 * ----
 *
 */
/* index
 * Access: INDEX
 */
MLXSW_ITEM32(reg, rips, index, 0x0, 0, 24);

/* ipv6
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, rips, ipv6, 0x4, 0, 32, 4, 0, 4, 0);

/* ralta
 * -----
 *
 */
/* op
 * Access: OP
 */
MLXSW_ITEM8(reg, ralta, op, 0x0, 4, 2);

/* protocol
 * Access: RW
 */
MLXSW_ITEM8(reg, ralta, protocol, 0x0, 0, 4);

/* tree_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ralta, tree_id, 0x3, 0, 8);

/* ralst
 * -----
 *
 */
/* root_bin
 * Access: RW
 */
MLXSW_ITEM8(reg, ralst, root_bin, 0x1, 0, 8);

/* tree_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ralst, tree_id, 0x3, 0, 8);

/* bin
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ralst, bin, 0x4, 0, 16, 2, 0, 2, 0);

/* raltb
 * -----
 *
 */
/* virtual_router
 * Access: INDEX
 */
MLXSW_ITEM16(reg, raltb, virtual_router, 0x0, 0, 16);

/* protocol
 * Access: INDEX
 */
MLXSW_ITEM8(reg, raltb, protocol, 0x2, 4, 4);

/* tree_id
 * Access: RW
 */
MLXSW_ITEM8(reg, raltb, tree_id, 0x3, 0, 8);

/* raleu
 * -----
 *
 */
/* protocol
 * Access: INDEX
 */
MLXSW_ITEM8(reg, raleu, protocol, 0x0, 0, 4);

/* type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, raleu, type, 0x1, 4, 2);

/* mvr
 * Access: OP
 */
MLXSW_ITEM8(reg, raleu, mvr, 0x1, 0, 1);

/* virtual_router
 * Access: INDEX
 */
MLXSW_ITEM16(reg, raleu, virtual_router, 0x2, 0, 16);

/* update_field
 * Access: OP
 */
MLXSW_ITEM8(reg, raleu, update_field, 0x7, 0, 4);

/* adjacency_index
 * Access: INDEX
 */
MLXSW_ITEM32(reg, raleu, adjacency_index, 0x10, 0, 24);

/* ecmp_size
 * Access: INDEX
 */
MLXSW_ITEM16(reg, raleu, ecmp_size, 0x16, 0, 13);

/* new_adjacency_index
 * Access: WO
 */
MLXSW_ITEM32(reg, raleu, new_adjacency_index, 0x20, 0, 24);

/* new_ecmp_size
 * Access: WO
 */
MLXSW_ITEM16(reg, raleu, new_ecmp_size, 0x26, 0, 13);

/* arn_ptr_v
 * Access: WO
 */
MLXSW_ITEM8(reg, raleu, arn_ptr_v, 0x28, 7, 1);

/* arn_ptr
 * Access: WO
 */
MLXSW_ITEM32(reg, raleu, arn_ptr, 0x28, 0, 24);

/* arlpgt_ptr
 * Access: WO
 */
MLXSW_ITEM16(reg, raleu, arlpgt_ptr, 0x2e, 0, 16);

/* raleu_v2
 * --------
 *
 */
/* protocols
 * Access: INDEX
 */
MLXSW_ITEM8(reg, raleu_v2, protocols, 0x0, 4, 4);

/* action_type
 * Access: INDEX
 */
MLXSW_ITEM8(reg, raleu_v2, action_type, 0x0, 0, 4);

/* mvr
 * Access: OP
 */
MLXSW_ITEM8(reg, raleu_v2, mvr, 0x1, 0, 1);

/* virtual_router
 * Access: INDEX
 */
MLXSW_ITEM16(reg, raleu_v2, virtual_router, 0x2, 0, 16);

/* match_mask
 * Access: OP
 */
MLXSW_ITEM8(reg, raleu_v2, match_mask, 0x7, 0, 8);

/* update_mask
 * Access: OP
 */
MLXSW_ITEM8(reg, raleu_v2, update_mask, 0xb, 0, 8);

/* ecmp_index
 * Access: INDEX
 */
MLXSW_ITEM32(reg, raleu_v2, ecmp_index, 0x10, 0, 24);

/* route_user_val
 * Access: INDEX
 */
MLXSW_ITEM8(reg, raleu_v2, route_user_val, 0x14, 0, 8);

/* ecmp_size
 * Access: INDEX
 */
MLXSW_ITEM16(reg, raleu_v2, ecmp_size, 0x16, 0, 13);

/* arn_ptr_v
 * Access: INDEX
 */
MLXSW_ITEM8(reg, raleu_v2, arn_ptr_v, 0x18, 7, 1);

/* arn_gen_ptr
 * Access: INDEX
 */
MLXSW_ITEM32(reg, raleu_v2, arn_gen_ptr, 0x18, 0, 24);

/* arlpgt_pointer
 * Access: INDEX
 */
MLXSW_ITEM16(reg, raleu_v2, arlpgt_pointer, 0x1e, 0, 16);

/* new_ecmp_index
 * Access: WO
 */
MLXSW_ITEM32(reg, raleu_v2, new_ecmp_index, 0x30, 0, 24);

/* new_route_user_val
 * Access: WO
 */
MLXSW_ITEM8(reg, raleu_v2, new_route_user_val, 0x34, 0, 8);

/* new_ecmp_size
 * Access: WO
 */
MLXSW_ITEM16(reg, raleu_v2, new_ecmp_size, 0x36, 0, 13);

/* new_arn_ptr_v
 * Access: WO
 */
MLXSW_ITEM8(reg, raleu_v2, new_arn_ptr_v, 0x38, 7, 1);

/* new_arn_gen_ptr
 * Access: WO
 */
MLXSW_ITEM32(reg, raleu_v2, new_arn_gen_ptr, 0x38, 0, 24);

/* new_arlpgt_pointer
 * Access: WO
 */
MLXSW_ITEM16(reg, raleu_v2, new_arlpgt_pointer, 0x3e, 0, 16);

/* ralup
 * -----
 *
 */
/* protocols
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ralup, protocols, 0x0, 4, 4);

/* mvr
 * Access: OP
 */
MLXSW_ITEM8(reg, ralup, mvr, 0x1, 0, 1);

/* virtual_router
 * Access: INDEX
 */
MLXSW_ITEM16(reg, ralup, virtual_router, 0x2, 0, 16);

/* match_mask
 * Access: OP
 */
MLXSW_ITEM8(reg, ralup, match_mask, 0x7, 0, 8);

/* update_mask
 * Access: OP
 */
MLXSW_ITEM8(reg, ralup, update_mask, 0xb, 0, 8);

/* flex_action_index
 * Access: INDEX
 */
MLXSW_ITEM32(reg, ralup, flex_action_index, 0x14, 0, 24);

/* new_flex_action_index
 * Access: WO
 */
MLXSW_ITEM32(reg, ralup, new_flex_action_index, 0x34, 0, 24);

/* ralcm
 * -----
 *
 */
/* virtual_router
 * Access: INDEX
 */
MLXSW_ITEM16(reg, ralcm, virtual_router, 0x0, 0, 16);

/* protocol
 * Access: INDEX
 */
MLXSW_ITEM8(reg, ralcm, protocol, 0x2, 4, 4);

/* rmid_v2
 * -------
 *
 */
/* rmid_index
 * Access: INDEX
 */
MLXSW_ITEM16(reg, rmid_v2, rmid_index, 0x2, 0, 16);

/* rmpe_index
 * Access: RW
 */
MLXSW_ITEM16(reg, rmid_v2, rmpe_index, 0x6, 0, 16);

/* egress_port
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, rmid_v2, egress_port, 0x20, 0, 32, 4, 0, 4, 0);

/* rmpe
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rmpe, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rmpe, lp_msb, 0x2, 4, 2);

/* rmpe_index
 * Access: INDEX
 */
MLXSW_ITEM16(reg, rmpe, rmpe_index, 0x6, 0, 16);

/* erif
 * Access: RW
 */
MLXSW_ITEM16(reg, rmpe, erif, 0xa, 0, 16);

/* reiv
 * ----
 *
 */
/* port_page
 * Access: INDEX
 */
MLXSW_ITEM8(reg, reiv, port_page, 0x3, 0, 4);

/* erif
 * Access: INDEX
 */
MLXSW_ITEM16(reg, reiv, erif, 0x6, 0, 16);

/* update
 * Access: OP
 */
MLXSW_ITEM8_INDEXED(reg, reiv, update, MLXSW_REIV_ELPORT_RECORD_BASE_LEN, 7, 1, MLXSW_REIV_ELPORT_RECORD_REC_LEN, 0, 1, 0);

/* evid
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, reiv, evid, MLXSW_REIV_ELPORT_RECORD_BASE_LEN + 2, 0, 12, MLXSW_REIV_ELPORT_RECORD_REC_LEN, 0, 2, 0);

/* rngcr
 * -----
 *
 */
/* v
 * Access: RW
 */
MLXSW_ITEM8(reg, rngcr, v, 0x0, 7, 1);

/* nat4to6_flc
 * Access: RW
 */
MLXSW_ITEM8(reg, rngcr, nat4to6_flc, 0x0, 1, 1);

/* nat4to6_flh
 * Access: RW
 */
MLXSW_ITEM8(reg, rngcr, nat4to6_flh, 0x0, 0, 1);

/* nat4to6_fl_prefix
 * Access: RW
 */
MLXSW_ITEM32(reg, rngcr, nat4to6_fl_prefix, 0x0, 8, 12);

/* nat4to6_fl_suffix
 * Access: RW
 */
MLXSW_ITEM8(reg, rngcr, nat4to6_fl_suffix, 0x3, 0, 8);

/* rrpcc
 * -----
 *
 */
/* use_vr
 * Access: RW
 */
MLXSW_ITEM8(reg, rrpcc, use_vr, 0x0, 7, 1);

/* ipv4_en
 * Access: RW
 */
MLXSW_ITEM8(reg, rrpcc, ipv4_en, 0x4, 7, 1);

/* ipv4_arn_en
 * Access: RW
 */
MLXSW_ITEM8(reg, rrpcc, ipv4_arn_en, 0x4, 6, 1);

/* ipv4_urpf_en
 * Access: RW
 */
MLXSW_ITEM8(reg, rrpcc, ipv4_urpf_en, 0x4, 5, 1);

/* ipv4_frag
 * Access: RW
 */
MLXSW_ITEM8(reg, rrpcc, ipv4_frag, 0x6, 6, 2);

/* ipv4_m
 * Access: RW
 */
MLXSW_ITEM8(reg, rrpcc, ipv4_m, 0x7, 0, 6);

/* ipv6_en
 * Access: RW
 */
MLXSW_ITEM8(reg, rrpcc, ipv6_en, 0x8, 7, 1);

/* ipv6_arn_en
 * Access: RW
 */
MLXSW_ITEM8(reg, rrpcc, ipv6_arn_en, 0x8, 6, 1);

/* ipv6_urpf_en
 * Access: RW
 */
MLXSW_ITEM8(reg, rrpcc, ipv6_urpf_en, 0x8, 5, 1);

/* ipv6_frag
 * Access: RW
 */
MLXSW_ITEM8(reg, rrpcc, ipv6_frag, 0xa, 6, 2);

/* ipv6_m
 * Access: RW
 */
MLXSW_ITEM8(reg, rrpcc, ipv6_m, 0xb, 0, 8);

/* rrpct
 * -----
 *
 */
/* mode
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rrpct, mode, 0x0, 7, 1);

/* table_index
 * Access: INDEX
 */
MLXSW_ITEM16(reg, rrpct, table_index, 0x2, 0, 16);

/* num_row
 * Access: OP
 */
MLXSW_ITEM16(reg, rrpct, num_row, 0x6, 0, 16);

/* length
 * Access: RW
 */
MLXSW_ITEM_BUF_INDEXED(reg, rrpct, length, 0x10, 0x20, 0x20, 0x0, 1, 0);

/* rimcb
 * -----
 *
 */
/* op
 * Access: OP
 */
MLXSW_ITEM8(reg, rimcb, op, 0x0, 0, 4);

/* rif
 * Access: INDEX
 */
MLXSW_ITEM16(reg, rimcb, rif, 0x2, 0, 16);

/* v
 * Access: RO
 */
MLXSW_ITEM8(reg, rimcb, v, 0xa, 7, 1);

/* counter_set_index
 * Access: RW
 */
MLXSW_ITEM8(reg, rimcb, counter_set_index, 0xb, 0, 8);

/* ralueb
 * ------
 *
 */
/* num_rec
 * Access: OP
 */
MLXSW_ITEM16(reg, ralueb, num_rec, 0x2, 0, 12);

/* success_num_rec
 * Access: RO
 */
MLXSW_ITEM16(reg, ralueb, success_num_rec, 0x6, 0, 12);

/* user_cookie
 * Access: OP
 */
MLXSW_ITEM64_INDEXED(reg, ralueb, user_cookie, MLXSW_RALUEB_RECORD_BASE_LEN, 0, 64, MLXSW_RALUEB_RECORD_REC_LEN, 0, 8, 0);

/* protocol
 * Access: INDEX
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, protocol, MLXSW_RALUEB_RECORD_BASE_LEN, 0, 4, MLXSW_RALUEB_RECORD_REC_LEN, 8, 1, 0);

/* op
 * Access: OP
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, op, MLXSW_RALUEB_RECORD_BASE_LEN + 1, 4, 3, MLXSW_RALUEB_RECORD_REC_LEN, 8, 1, 0);

/* a
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, a, MLXSW_RALUEB_RECORD_BASE_LEN + 1, 0, 1, MLXSW_RALUEB_RECORD_REC_LEN, 8, 1, 0);

/* virtual_router
 * Access: INDEX
 */
MLXSW_ITEM16_INDEXED(reg, ralueb, virtual_router, MLXSW_RALUEB_RECORD_BASE_LEN, 0, 16, MLXSW_RALUEB_RECORD_REC_LEN, 12, 2, 0);

/* route_user_val_u_mask
 * Access: WO
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, route_user_val_u_mask, MLXSW_RALUEB_RECORD_BASE_LEN + 2, 5, 1, MLXSW_RALUEB_RECORD_REC_LEN, 12, 1, 0);

/* arn_u_mask
 * Access: WO
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, arn_u_mask, MLXSW_RALUEB_RECORD_BASE_LEN + 2, 4, 1, MLXSW_RALUEB_RECORD_REC_LEN, 12, 1, 0);

/* op_u_mask
 * Access: WO
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, op_u_mask, MLXSW_RALUEB_RECORD_BASE_LEN + 2, 0, 4, MLXSW_RALUEB_RECORD_REC_LEN, 12, 1, 0);

/* jump_prefix
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, jump_prefix, MLXSW_RALUEB_RECORD_BASE_LEN + 2, 0, 8, MLXSW_RALUEB_RECORD_REC_LEN, 16, 1, 0);

/* prefix_len
 * Access: INDEX
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, prefix_len, MLXSW_RALUEB_RECORD_BASE_LEN + 3, 0, 8, MLXSW_RALUEB_RECORD_REC_LEN, 16, 1, 0);

/* dip
 * Access: INDEX
 */
MLXSW_ITEM32_INDEXED(reg, ralueb, dip, MLXSW_RALUEB_RECORD_BASE_LEN, 0, 32, MLXSW_RALUEB_RECORD_REC_LEN, 20, 4, 0);

/* entry_type
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, entry_type, MLXSW_RALUEB_RECORD_BASE_LEN, 6, 2, MLXSW_RALUEB_RECORD_REC_LEN, 36, 1, 0);

/* bmp_len
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, bmp_len, MLXSW_RALUEB_RECORD_BASE_LEN + 1, 0, 8, MLXSW_RALUEB_RECORD_REC_LEN, 36, 1, 0);

/* route_user_val
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, route_user_val, MLXSW_RALUEB_RECORD_BASE_LEN + 2, 0, 8, MLXSW_RALUEB_RECORD_REC_LEN, 36, 1, 0);

/* action_type
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, action_type, MLXSW_RALUEB_RECORD_BASE_LEN + 3, 0, 4, MLXSW_RALUEB_RECORD_REC_LEN, 36, 1, 0);

/* remote_action_trap_action
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, remote_action_trap_action, MLXSW_RALUEB_RECORD_BASE_LEN, 4, 4, MLXSW_RALUEB_RECORD_REC_LEN, 40, 1, 0);

/* remote_action_trap_id
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ralueb, remote_action_trap_id, MLXSW_RALUEB_RECORD_BASE_LEN + 2, 0, 9, MLXSW_RALUEB_RECORD_REC_LEN, 40, 2, 0);

/* remote_action_adjacency_index
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ralueb, remote_action_adjacency_index, MLXSW_RALUEB_RECORD_BASE_LEN, 0, 24, MLXSW_RALUEB_RECORD_REC_LEN, 44, 3, 0);

/* remote_action_ecmp_size
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ralueb, remote_action_ecmp_size, MLXSW_RALUEB_RECORD_BASE_LEN + 2, 0, 13, MLXSW_RALUEB_RECORD_REC_LEN, 48, 2, 0);

/* local_action_trap_action
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, local_action_trap_action, MLXSW_RALUEB_RECORD_BASE_LEN, 4, 4, MLXSW_RALUEB_RECORD_REC_LEN, 40, 1, 0);

/* local_action_trap_id
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ralueb, local_action_trap_id, MLXSW_RALUEB_RECORD_BASE_LEN + 2, 0, 9, MLXSW_RALUEB_RECORD_REC_LEN, 40, 2, 0);

/* local_action_local_erif
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ralueb, local_action_local_erif, MLXSW_RALUEB_RECORD_BASE_LEN + 2, 0, 16, MLXSW_RALUEB_RECORD_REC_LEN, 44, 2, 0);

/* ip2me_trap_action
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, ip2me_trap_action, MLXSW_RALUEB_RECORD_BASE_LEN, 4, 4, MLXSW_RALUEB_RECORD_REC_LEN, 40, 1, 0);

/* ip2me_v
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, ip2me_v, MLXSW_RALUEB_RECORD_BASE_LEN, 7, 1, MLXSW_RALUEB_RECORD_REC_LEN, 44, 1, 0);

/* ip2me_tunnel_ptr
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ralueb, ip2me_tunnel_ptr, MLXSW_RALUEB_RECORD_BASE_LEN, 0, 24, MLXSW_RALUEB_RECORD_REC_LEN, 44, 3, 0);

/* ar_action_trap_action
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, ar_action_trap_action, MLXSW_RALUEB_RECORD_BASE_LEN, 4, 4, MLXSW_RALUEB_RECORD_REC_LEN, 40, 1, 0);

/* ar_action_trap_id
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ralueb, ar_action_trap_id, MLXSW_RALUEB_RECORD_BASE_LEN + 2, 0, 9, MLXSW_RALUEB_RECORD_REC_LEN, 40, 2, 0);

/* ar_action_ar_lookup_prof_id
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, ar_action_ar_lookup_prof_id, MLXSW_RALUEB_RECORD_BASE_LEN + 3, 0, 4, MLXSW_RALUEB_RECORD_REC_LEN, 44, 1, 0);

/* ar_action_arft_pointer
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ralueb, ar_action_arft_pointer, MLXSW_RALUEB_RECORD_BASE_LEN, 0, 20, MLXSW_RALUEB_RECORD_REC_LEN, 48, 3, 0);

/* ar_action_ecmp_size
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ralueb, ar_action_ecmp_size, MLXSW_RALUEB_RECORD_BASE_LEN + 2, 0, 13, MLXSW_RALUEB_RECORD_REC_LEN, 52, 2, 0);

/* ar_action_arlpgt_pointer
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, ralueb, ar_action_arlpgt_pointer, MLXSW_RALUEB_RECORD_BASE_LEN + 2, 0, 16, MLXSW_RALUEB_RECORD_REC_LEN, 56, 2, 0);

/* counter_set_type
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, counter_set_type, MLXSW_RALUEB_RECORD_BASE_LEN, 0, 8, MLXSW_RALUEB_RECORD_REC_LEN, 60, 1, 0);

/* counter_index
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ralueb, counter_index, MLXSW_RALUEB_RECORD_BASE_LEN, 0, 24, MLXSW_RALUEB_RECORD_REC_LEN, 60, 3, 0);

/* arn_ptr_v
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, arn_ptr_v, MLXSW_RALUEB_RECORD_BASE_LEN, 7, 1, MLXSW_RALUEB_RECORD_REC_LEN, 64, 1, 0);

/* arn_gen_ptr
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ralueb, arn_gen_ptr, MLXSW_RALUEB_RECORD_BASE_LEN, 0, 24, MLXSW_RALUEB_RECORD_REC_LEN, 64, 3, 0);

/* set_rpc
 * Access: OP
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, set_rpc, MLXSW_RALUEB_RECORD_BASE_LEN, 7, 1, MLXSW_RALUEB_RECORD_REC_LEN, 68, 1, 0);

/* rpc_index
 * Access: INDEX
 */
MLXSW_ITEM16_INDEXED(reg, ralueb, rpc_index, MLXSW_RALUEB_RECORD_BASE_LEN + 2, 0, 16, MLXSW_RALUEB_RECORD_REC_LEN, 68, 2, 0);

/* rpc_entry_0_length
 * Access: WO
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, rpc_entry_0_length, MLXSW_RALUEB_RECORD_BASE_LEN + 1, 0, 8, MLXSW_RALUEB_RECORD_REC_LEN, 72, 1, 0);

/* rpc_entry_0
 * Access: INDEX
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, rpc_entry_0, MLXSW_RALUEB_RECORD_BASE_LEN + 3, 0, 8, MLXSW_RALUEB_RECORD_REC_LEN, 72, 1, 0);

/* rpc_entry_1_length
 * Access: WO
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, rpc_entry_1_length, MLXSW_RALUEB_RECORD_BASE_LEN + 1, 0, 8, MLXSW_RALUEB_RECORD_REC_LEN, 76, 1, 0);

/* rpc_entry_1
 * Access: INDEX
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, rpc_entry_1, MLXSW_RALUEB_RECORD_BASE_LEN + 3, 0, 8, MLXSW_RALUEB_RECORD_REC_LEN, 76, 1, 0);

/* set_flex_action
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, set_flex_action, MLXSW_RALUEB_RECORD_BASE_LEN, 7, 1, MLXSW_RALUEB_RECORD_REC_LEN, 80, 1, 0);

/* flex_action_removal
 * Access: OP
 */
MLXSW_ITEM8_INDEXED(reg, ralueb, flex_action_removal, MLXSW_RALUEB_RECORD_BASE_LEN, 6, 1, MLXSW_RALUEB_RECORD_REC_LEN, 80, 1, 0);

/* flex_action_index
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, ralueb, flex_action_index, MLXSW_RALUEB_RECORD_BASE_LEN, 0, 24, MLXSW_RALUEB_RECORD_REC_LEN, 80, 3, 0);

/* rxlte
 * -----
 *
 */
/* virtual_router
 * Access: INDEX
 */
MLXSW_ITEM16(reg, rxlte, virtual_router, 0x2, 0, 16);

/* protocol
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rxlte, protocol, 0x7, 0, 4);

/* lpm_xlt_en
 * Access: RW
 */
MLXSW_ITEM8(reg, rxlte, lpm_xlt_en, 0xb, 0, 4);

/* rxltm
 * -----
 *
 */
/* m_sel_update
 * Access: OP
 */
MLXSW_ITEM8(reg, rxltm, m_sel_update, 0x0, 7, 1);

/* go_m_sel
 * Access: RW
 */
MLXSW_ITEM8(reg, rxltm, go_m_sel, 0x3, 1, 1);

/* m_sel
 * Access: RW
 */
MLXSW_ITEM8(reg, rxltm, m_sel, 0x3, 0, 1);

/* m1_val_v6
 * Access: RW
 */
MLXSW_ITEM8(reg, rxltm, m1_val_v6, 0x10, 0, 8);

/* m0_val_v6
 * Access: RW
 */
MLXSW_ITEM8(reg, rxltm, m0_val_v6, 0x11, 0, 8);

/* m1_val_v4
 * Access: RW
 */
MLXSW_ITEM8(reg, rxltm, m1_val_v4, 0x12, 0, 6);

/* m0_val_v4
 * Access: RW
 */
MLXSW_ITEM8(reg, rxltm, m0_val_v4, 0x13, 0, 6);

/* rxltcc
 * ------
 *
 */
/* clear
 * Access: OP
 */
MLXSW_ITEM8(reg, rxltcc, clear, 0x0, 7, 1);

/* hit_ipv4_m_high
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, hit_ipv4_m_high, 0x10, 0, 32);

/* hit_ipv4_m_low
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, hit_ipv4_m_low, 0x14, 0, 32);

/* hit_ipv4_ml_high
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, hit_ipv4_ml_high, 0x18, 0, 32);

/* hit_ipv4_ml_low
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, hit_ipv4_ml_low, 0x1c, 0, 32);

/* miss_ipv4_m_high
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, miss_ipv4_m_high, 0x20, 0, 32);

/* miss_ipv4_m_low
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, miss_ipv4_m_low, 0x24, 0, 32);

/* miss_ipv4_ml_high
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, miss_ipv4_ml_high, 0x28, 0, 32);

/* miss_ipv4_ml_low
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, miss_ipv4_ml_low, 0x2c, 0, 32);

/* learned_ipv4_high
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, learned_ipv4_high, 0x30, 0, 32);

/* learned_ipv4_low
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, learned_ipv4_low, 0x34, 0, 32);

/* hit_ipv6_m_high
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, hit_ipv6_m_high, 0x40, 0, 32);

/* hit_ipv6_m_low
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, hit_ipv6_m_low, 0x44, 0, 32);

/* hit_ipv6_ml_high
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, hit_ipv6_ml_high, 0x48, 0, 32);

/* hit_ipv6_ml_low
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, hit_ipv6_ml_low, 0x4c, 0, 32);

/* miss_ipv6_m_high
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, miss_ipv6_m_high, 0x50, 0, 32);

/* miss_ipv6_m_low
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, miss_ipv6_m_low, 0x54, 0, 32);

/* miss_ipv6_ml_high
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, miss_ipv6_ml_high, 0x58, 0, 32);

/* miss_ipv6_ml_low
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, miss_ipv6_ml_low, 0x5c, 0, 32);

/* learned_ipv6_high
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, learned_ipv6_high, 0x60, 0, 32);

/* learned_ipv6_low
 * Access: RO
 */
MLXSW_ITEM32(reg, rxltcc, learned_ipv6_low, 0x64, 0, 32);

/* rlcme
 * -----
 *
 */
/* protocol
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rlcme, protocol, 0x0, 0, 4);

/* op
 * Access: OP
 */
MLXSW_ITEM8(reg, rlcme, op, 0x1, 4, 3);

/* m_idx
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rlcme, m_idx, 0x3, 0, 1);

/* virtual_router
 * Access: INDEX
 */
MLXSW_ITEM16(reg, rlcme, virtual_router, 0x4, 0, 16);

/* dip
 * Access: INDEX
 */
MLXSW_ITEM32_INDEXED(reg, rlcme, dip, 0xc, 0, 32, 4, 0, 4, 0);

/* action_type
 * Access: RW
 */
MLXSW_ITEM8(reg, rlcme, action_type, 0x1f, 0, 2);

/* action
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, rlcme, action, 0x20, 0, 32, 4, 0, 4, 0);

/* counter_set_type
 * Access: RW
 */
MLXSW_ITEM8(reg, rlcme, counter_set_type, 0x34, 0, 8);

/* counter_index
 * Access: RW
 */
MLXSW_ITEM32(reg, rlcme, counter_index, 0x34, 0, 24);

/* action_valid
 * Access: RW
 */
MLXSW_ITEM8(reg, rlcme, action_valid, 0x38, 1, 1);

/* l_vector
 * Access: RW
 */
MLXSW_ITEM8(reg, rlcme, l_vector, 0x3a, 0, 4);

/* l_value
 * Access: RW
 */
MLXSW_ITEM8(reg, rlcme, l_value, 0x3b, 0, 7);

/* rlcmle
 * ------
 *
 */
/* protocol
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rlcmle, protocol, 0x0, 0, 4);

/* op
 * Access: OP
 */
MLXSW_ITEM8(reg, rlcmle, op, 0x1, 4, 3);

/* m_idx
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rlcmle, m_idx, 0x3, 0, 1);

/* virtual_router
 * Access: INDEX
 */
MLXSW_ITEM16(reg, rlcmle, virtual_router, 0x4, 0, 16);

/* l_value
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rlcmle, l_value, 0x7, 0, 7);

/* dip
 * Access: INDEX
 */
MLXSW_ITEM32_INDEXED(reg, rlcmle, dip, 0xc, 0, 32, 4, 0, 4, 0);

/* action_type
 * Access: RW
 */
MLXSW_ITEM8(reg, rlcmle, action_type, 0x1f, 0, 2);

/* remote_action_trap_action
 * Access: RW
 */
MLXSW_ITEM8(reg, rlcmle, remote_action_trap_action, 0x20, 4, 4);

/* remote_action_trap_id
 * Access: RW
 */
MLXSW_ITEM16(reg, rlcmle, remote_action_trap_id, 0x22, 0, 9);

/* remote_action_adjacency_index
 * Access: RW
 */
MLXSW_ITEM32(reg, rlcmle, remote_action_adjacency_index, 0x24, 0, 24);

/* remote_action_ecmp_size
 * Access: RW
 */
MLXSW_ITEM16(reg, rlcmle, remote_action_ecmp_size, 0x2a, 0, 13);

/* local_action_trap_action
 * Access: RW
 */
MLXSW_ITEM8(reg, rlcmle, local_action_trap_action, 0x20, 4, 4);

/* local_action_trap_id
 * Access: RW
 */
MLXSW_ITEM16(reg, rlcmle, local_action_trap_id, 0x22, 0, 9);

/* local_action_local_erif
 * Access: RW
 */
MLXSW_ITEM16(reg, rlcmle, local_action_local_erif, 0x26, 0, 16);

/* ip2me_trap_action
 * Access: RW
 */
MLXSW_ITEM8(reg, rlcmle, ip2me_trap_action, 0x20, 4, 4);

/* ip2me_v
 * Access: RW
 */
MLXSW_ITEM8(reg, rlcmle, ip2me_v, 0x24, 7, 1);

/* ip2me_tunnel_ptr
 * Access: RW
 */
MLXSW_ITEM32(reg, rlcmle, ip2me_tunnel_ptr, 0x24, 0, 24);

/* counter_set_type
 * Access: RW
 */
MLXSW_ITEM8(reg, rlcmle, counter_set_type, 0x34, 0, 8);

/* counter_index
 * Access: RW
 */
MLXSW_ITEM32(reg, rlcmle, counter_index, 0x34, 0, 24);

/* rlcmld
 * ------
 *
 */
/* select
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rlcmld, select, 0x1, 0, 2);

/* filter_fields
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rlcmld, filter_fields, 0x3, 0, 8);

/* m_idx
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rlcmld, m_idx, 0x7, 0, 1);

/* protocol
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rlcmld, protocol, 0xb, 0, 4);

/* virtual_router
 * Access: INDEX
 */
MLXSW_ITEM16(reg, rlcmld, virtual_router, 0xe, 0, 16);

/* dip
 * Access: INDEX
 */
MLXSW_ITEM32_INDEXED(reg, rlcmld, dip, 0x10, 0, 32, 4, 0, 4, 0);

/* dip_mask
 * Access: INDEX
 */
MLXSW_ITEM32_INDEXED(reg, rlcmld, dip_mask, 0x20, 0, 32, 4, 0, 4, 0);

/* rargcr
 * ------
 *
 */
/* arft_index_base
 * Access: RW
 */
MLXSW_ITEM32(reg, rargcr, arft_index_base, 0x0, 0, 20);

/* arft_index_size
 * Access: RW
 */
MLXSW_ITEM16(reg, rargcr, arft_index_size, 0x6, 0, 16);

/* rarpc
 * -----
 *
 */
/* pc_default
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rarpc, pc_default, 0x2, 0, 1);

/* pc_entry
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rarpc, pc_entry, 0x3, 0, 8);

/* l4_inner
 * Access: RW
 */
MLXSW_ITEM8(reg, rarpc, l4_inner, 0x6, 1, 1);

/* inner_must
 * Access: RW
 */
MLXSW_ITEM8(reg, rarpc, inner_must, 0x6, 0, 1);

/* ar_packet_prof_id
 * Access: RW
 */
MLXSW_ITEM8(reg, rarpc, ar_packet_prof_id, 0x7, 0, 4);

/* switch_prio
 * Access: RW
 */
MLXSW_ITEM16(reg, rarpc, switch_prio, 0x12, 0, 16);

/* l3_type
 * Access: RW
 */
MLXSW_ITEM8(reg, rarpc, l3_type, 0x14, 0, 8);

/* inner_l3_type
 * Access: RW
 */
MLXSW_ITEM8(reg, rarpc, inner_l3_type, 0x15, 0, 8);

/* l4_type
 * Access: RW
 */
MLXSW_ITEM8(reg, rarpc, l4_type, 0x16, 0, 8);

/* inner_l4_type
 * Access: RW
 */
MLXSW_ITEM8(reg, rarpc, inner_l4_type, 0x17, 0, 8);

/* bth_ar
 * Access: RW
 */
MLXSW_ITEM8(reg, rarpc, bth_ar, 0x1a, 0, 2);

/* bth_opcode_msb
 * Access: RW
 */
MLXSW_ITEM8(reg, rarpc, bth_opcode_msb, 0x1b, 0, 8);

/* bth_opcode_lsb
 * Access: RW
 */
MLXSW_ITEM32(reg, rarpc, bth_opcode_lsb, 0x1c, 0, 32);

/* inner_bth_ar
 * Access: RW
 */
MLXSW_ITEM8(reg, rarpc, inner_bth_ar, 0x22, 0, 2);

/* inner_bth_opcode_msb
 * Access: RW
 */
MLXSW_ITEM8(reg, rarpc, inner_bth_opcode_msb, 0x23, 0, 8);

/* inner_bth_opcode_lsb
 * Access: RW
 */
MLXSW_ITEM32(reg, rarpc, inner_bth_opcode_lsb, 0x24, 0, 32);

/* ingress_port
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, rarpc, ingress_port, 0x30, 0, 32, 4, 0, 4, 0);

/* port_range_min
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, rarpc, port_range_min, MLXSW_RARPC_L4PORT_CMP_BASE_LEN, 0, 16, MLXSW_RARPC_L4PORT_CMP_REC_LEN, 0, 2, 0);

/* port_range_max
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, rarpc, port_range_max, MLXSW_RARPC_L4PORT_CMP_BASE_LEN + 2, 0, 16, MLXSW_RARPC_L4PORT_CMP_REC_LEN, 0, 2, 0);

/* port_src
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, rarpc, port_src, MLXSW_RARPC_L4PORT_CMP_BASE_LEN + 3, 4, 1, MLXSW_RARPC_L4PORT_CMP_REC_LEN, 4, 1, 0);

/* comp_field
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, rarpc, comp_field, MLXSW_RARPC_L4PORT_CMP_BASE_LEN + 3, 0, 2, MLXSW_RARPC_L4PORT_CMP_REC_LEN, 4, 1, 0);

/* rarsr
 * -----
 *
 */
/* ar_srf
 * Access: RW
 */
MLXSW_ITEM16(reg, rarsr, ar_srf, 0x2, 0, 16);

/* ar_srt
 * Access: RW
 */
MLXSW_ITEM16(reg, rarsr, ar_srt, 0x6, 0, 16);

/* rarpr
 * -----
 *
 */
/* ar_prof_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rarpr, ar_prof_id, 0x3, 0, 4);

/* prof_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, rarpr, prof_mode, 0x7, 0, 4);

/* port_select_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, rarpr, port_select_mode, 0x8, 4, 4);

/* elph
 * Access: RW
 */
MLXSW_ITEM8(reg, rarpr, elph, 0x8, 0, 1);

/* grade_thr_free
 * Access: RW
 */
MLXSW_ITEM8(reg, rarpr, grade_thr_free, 0xa, 0, 3);

/* grade_thr_busy
 * Access: RW
 */
MLXSW_ITEM8(reg, rarpr, grade_thr_busy, 0xb, 0, 3);

/* srf
 * Access: RW
 */
MLXSW_ITEM8(reg, rarpr, srf, 0xf, 1, 1);

/* srt
 * Access: RW
 */
MLXSW_ITEM8(reg, rarpr, srt, 0xf, 0, 1);

/* time_bound_time
 * Access: RW
 */
MLXSW_ITEM16(reg, rarpr, time_bound_time, 0x12, 0, 10);

/* rarcl
 * -----
 *
 */
/* th_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rarcl, th_index, 0x3, 0, 4);

/* eport_cong_level0
 * Access: RW
 */
MLXSW_ITEM32(reg, rarcl, eport_cong_level0, 0x10, 0, 24);

/* eport_cong_level1
 * Access: RW
 */
MLXSW_ITEM32(reg, rarcl, eport_cong_level1, 0x14, 0, 24);

/* eport_cong_level2
 * Access: RW
 */
MLXSW_ITEM32(reg, rarcl, eport_cong_level2, 0x18, 0, 24);

/* tc_cong_level0
 * Access: RW
 */
MLXSW_ITEM32(reg, rarcl, tc_cong_level0, 0x20, 0, 24);

/* tc_cong_level1
 * Access: RW
 */
MLXSW_ITEM32(reg, rarcl, tc_cong_level1, 0x24, 0, 24);

/* tc_cong_level2
 * Access: RW
 */
MLXSW_ITEM32(reg, rarcl, tc_cong_level2, 0x28, 0, 24);

/* rarppm
 * ------
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rarppm, local_port, 0x1, 0, 8);

/* pool
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rarppm, pool, 0x2, 0, 6);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rarppm, lp_msb, 0x3, 4, 2);

/* th_indx
 * Access: RW
 */
MLXSW_ITEM8(reg, rarppm, th_indx, 0x7, 0, 2);

/* rarlu
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rarlu, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rarlu, lp_msb, 0x2, 4, 2);

/* lut_en
 * Access: RW
 */
MLXSW_ITEM8(reg, rarlu, lut_en, 0x4, 7, 1);

/* link_utilization_thr
 * Access: RW
 */
MLXSW_ITEM32(reg, rarlu, link_utilization_thr, 0x4, 0, 31);

/* rarft
 * -----
 *
 */
/* arft_index
 * Access: INDEX
 */
MLXSW_ITEM32(reg, rarft, arft_index, 0x0, 0, 20);

/* arft_entry_in_row
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rarft, arft_entry_in_row, 0x7, 0, 4);

/* valid
 * Access: RW
 */
MLXSW_ITEM8(reg, rarft, valid, 0x9, 0, 1);

/* lp_msb
 * Access: RW
 */
MLXSW_ITEM8(reg, rarft, lp_msb, 0xa, 0, 2);

/* local_port
 * Access: RW
 */
MLXSW_ITEM8(reg, rarft, local_port, 0xb, 0, 8);

/* ar_prof_id
 * Access: RW
 */
MLXSW_ITEM8(reg, rarft, ar_prof_id, 0xf, 0, 4);

/* rarftb
 * ------
 *
 */
/* arft_index
 * Access: INDEX
 */
MLXSW_ITEM32(reg, rarftb, arft_index, 0x0, 0, 20);

/* num_of_arft_lines
 * Access: OP
 */
MLXSW_ITEM16(reg, rarftb, num_of_arft_lines, 0x6, 0, 16);

/* rarftbr
 * -------
 *
 */
/* num_rec
 * Access: OP
 */
MLXSW_ITEM16(reg, rarftbr, num_rec, 0x2, 0, 16);

/* arft_index
 * Access: INDEX
 */
MLXSW_ITEM32(reg, rarftbr, arft_index, 0x4, 0, 20);

/* ar_prof_id
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, rarftbr, ar_prof_id, MLXSW_RARFTBR_FLOW_TABLE_RECORDS_BASE_LEN + 1, 0, 4, MLXSW_RARFTBR_FLOW_TABLE_RECORDS_REC_LEN, 0, 1, 0);

/* lp_msb
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, rarftbr, lp_msb, MLXSW_RARFTBR_FLOW_TABLE_RECORDS_BASE_LEN + 2, 0, 2, MLXSW_RARFTBR_FLOW_TABLE_RECORDS_REC_LEN, 0, 1, 0);

/* local_port
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, rarftbr, local_port, MLXSW_RARFTBR_FLOW_TABLE_RECORDS_BASE_LEN + 3, 0, 8, MLXSW_RARFTBR_FLOW_TABLE_RECORDS_REC_LEN, 0, 1, 0);

/* arn_state
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, rarftbr, arn_state, MLXSW_RARFTBR_FLOW_TABLE_RECORDS_BASE_LEN + 1, 0, 2, MLXSW_RARFTBR_FLOW_TABLE_RECORDS_REC_LEN, 4, 1, 0);

/* rarlpgt
 * -------
 *
 */
/* arlpgt_index
 * Access: INDEX
 */
MLXSW_ITEM16(reg, rarlpgt, arlpgt_index, 0x2, 0, 16);

/* egress_port
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, rarlpgt, egress_port, 0x20, 0, 32, 4, 0, 4, 0);

/* rarcc
 * -----
 *
 */
/* clr
 * Access: OP
 */
MLXSW_ITEM8(reg, rarcc, clr, 0x0, 7, 1);

/* ar_changes_congestion_high
 * Access: RO
 */
MLXSW_ITEM32(reg, rarcc, ar_changes_congestion_high, 0x10, 0, 32);

/* ar_changes_congestion_low
 * Access: RO
 */
MLXSW_ITEM32(reg, rarcc, ar_changes_congestion_low, 0x14, 0, 32);

/* ar_changes_arn_high
 * Access: RO
 */
MLXSW_ITEM32(reg, rarcc, ar_changes_arn_high, 0x18, 0, 32);

/* ar_changes_arn_low
 * Access: RO
 */
MLXSW_ITEM32(reg, rarcc, ar_changes_arn_low, 0x1c, 0, 32);

/* rartm
 * -----
 *
 */
/* en
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, rartm, en, MLXSW_RARTM_MAPPING_BASE_LEN, 7, 1, MLXSW_RARTM_MAPPING_REC_LEN, 0, 1, 0);

/* ar_tclass
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, rartm, ar_tclass, MLXSW_RARTM_MAPPING_BASE_LEN + 3, 0, 5, MLXSW_RARTM_MAPPING_REC_LEN, 0, 1, 0);

/* rsnh
 * ----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rsnh, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rsnh, lp_msb, 0x2, 4, 2);

/* p2d_en
 * Access: RW
 */
MLXSW_ITEM8(reg, rsnh, p2d_en, 0x7, 0, 1);

/* dmac
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, rsnh, dmac, 0xa, 6);

/* rarngc
 * ------
 *
 */
/* udp_dst_port
 * Access: RW
 */
MLXSW_ITEM16(reg, rarngc, udp_dst_port, 0x0, 0, 16);

/* udp_src_port
 * Access: RW
 */
MLXSW_ITEM16(reg, rarngc, udp_src_port, 0x2, 0, 16);

/* ip_df
 * Access: RW
 */
MLXSW_ITEM8(reg, rarngc, ip_df, 0x4, 7, 1);

/* version
 * Access: RW
 */
MLXSW_ITEM8(reg, rarngc, version, 0x4, 0, 4);

/* switch_id
 * Access: RW
 */
MLXSW_ITEM16(reg, rarngc, switch_id, 0x6, 0, 16);

/* flh
 * Access: RW
 */
MLXSW_ITEM8(reg, rarngc, flh, 0x8, 1, 1);

/* flc
 * Access: RW
 */
MLXSW_ITEM8(reg, rarngc, flc, 0x8, 0, 1);

/* fl_prefix
 * Access: RW
 */
MLXSW_ITEM32(reg, rarngc, fl_prefix, 0x8, 8, 12);

/* fl_suffix
 * Access: RW
 */
MLXSW_ITEM8(reg, rarngc, fl_suffix, 0xb, 0, 8);

/* arn_vr_en
 * Access: RW
 */
MLXSW_ITEM8(reg, rarngc, arn_vr_en, 0xc, 7, 1);

/* prefix_match_type
 * Access: RW
 */
MLXSW_ITEM8(reg, rarngc, prefix_match_type, 0xc, 2, 2);

/* arn_gen_th_grade
 * Access: RW
 */
MLXSW_ITEM8(reg, rarngc, arn_gen_th_grade, 0xd, 0, 3);

/* arn_vr_base
 * Access: RW
 */
MLXSW_ITEM16(reg, rarngc, arn_vr_base, 0xe, 0, 13);

/* tr_en
 * Access: RW
 */
MLXSW_ITEM8(reg, rarngc, tr_en, 0x10, 7, 1);

/* truncation_size
 * Access: RW
 */
MLXSW_ITEM16(reg, rarngc, truncation_size, 0x12, 0, 13);

/* not_vld_set_type
 * Access: RW
 */
MLXSW_ITEM8(reg, rarngc, not_vld_set_type, 0x14, 0, 8);

/* arn_ptr_not_vld_cnt
 * Access: RW
 */
MLXSW_ITEM32(reg, rarngc, arn_ptr_not_vld_cnt, 0x14, 0, 24);

/* bad_sip_cnt_type
 * Access: RW
 */
MLXSW_ITEM8(reg, rarngc, bad_sip_cnt_type, 0x18, 0, 8);

/* arn_bad_sip_cnt
 * Access: RW
 */
MLXSW_ITEM32(reg, rarngc, arn_bad_sip_cnt, 0x18, 0, 24);

/* rarnpc
 * ------
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rarnpc, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rarnpc, lp_msb, 0x2, 4, 2);

/* egress_en
 * Access: RW
 */
MLXSW_ITEM8(reg, rarnpc, egress_en, 0x4, 7, 1);

/* ingress_en
 * Access: RW
 */
MLXSW_ITEM8(reg, rarnpc, ingress_en, 0x4, 6, 1);

/* shaper_en
 * Access: RW
 */
MLXSW_ITEM8(reg, rarnpc, shaper_en, 0x4, 5, 1);

/* clr_cnt
 * Access: OP
 */
MLXSW_ITEM8(reg, rarnpc, clr_cnt, 0x7, 0, 1);

/* arn_transmit_cnt
 * Access: RO
 */
MLXSW_ITEM64(reg, rarnpc, arn_transmit_cnt, 0x8, 0, 64);

/* rarns
 * -----
 *
 */
/* arn_gen_min_time
 * Access: RW
 */
MLXSW_ITEM32(reg, rarns, arn_gen_min_time, 0x4, 0, 32);

/* rarnpr
 * ------
 *
 */
/* elph
 * Access: RW
 */
MLXSW_ITEM8(reg, rarnpr, elph, 0x0, 7, 1);

/* arn_gen_en
 * Access: RW
 */
MLXSW_ITEM8(reg, rarnpr, arn_gen_en, 0x1, 0, 1);

/* profile_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rarnpr, profile_index, 0x3, 0, 4);

/* hold_time_after_change
 * Access: RW
 */
MLXSW_ITEM8(reg, rarnpr, hold_time_after_change, 0x5, 0, 8);

/* arn_aging_time
 * Access: RW
 */
MLXSW_ITEM8(reg, rarnpr, arn_aging_time, 0x7, 0, 8);

/* penalty_grade
 * Access: RW
 */
MLXSW_ITEM8(reg, rarnpr, penalty_grade, 0xb, 0, 3);

/* rarncp
 * ------
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rarncp, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rarncp, lp_msb, 0x2, 4, 2);

/* arn_dmac_31_0
 * Access: RW
 */
MLXSW_ITEM32(reg, rarncp, arn_dmac_31_0, 0x4, 0, 32);

/* arn_dmac_en
 * Access: RW
 */
MLXSW_ITEM8(reg, rarncp, arn_dmac_en, 0x8, 7, 1);

/* arn_dip_en
 * Access: RW
 */
MLXSW_ITEM8(reg, rarncp, arn_dip_en, 0x8, 6, 1);

/* arn_en
 * Access: RW
 */
MLXSW_ITEM8(reg, rarncp, arn_en, 0x8, 5, 1);

/* arn_dmac_47_32
 * Access: RW
 */
MLXSW_ITEM16(reg, rarncp, arn_dmac_47_32, 0xa, 0, 16);

/* rarncg
 * ------
 *
 */
/* arn_dip_ipv4
 * Access: RW
 */
MLXSW_ITEM32(reg, rarncg, arn_dip_ipv4, 0x0, 0, 32);

/* arn_dip_ipv6_31_0
 * Access: RW
 */
MLXSW_ITEM32(reg, rarncg, arn_dip_ipv6_31_0, 0x4, 0, 32);

/* arn_dip_ipv6_63_32
 * Access: RW
 */
MLXSW_ITEM32(reg, rarncg, arn_dip_ipv6_63_32, 0x8, 0, 32);

/* arn_dip_ipv6_95_64
 * Access: RW
 */
MLXSW_ITEM32(reg, rarncg, arn_dip_ipv6_95_64, 0xc, 0, 32);

/* arn_dip_ipv6_127_96
 * Access: RW
 */
MLXSW_ITEM32(reg, rarncg, arn_dip_ipv6_127_96, 0x10, 0, 32);

/* arn_udp_dport
 * Access: RW
 */
MLXSW_ITEM16(reg, rarncg, arn_udp_dport, 0x16, 0, 16);

/* rcv_ok_cnt_type
 * Access: RW
 */
MLXSW_ITEM8(reg, rarncg, rcv_ok_cnt_type, 0x18, 0, 8);

/* arn_rcv_ok_cnt
 * Access: RW
 */
MLXSW_ITEM32(reg, rarncg, arn_rcv_ok_cnt, 0x18, 0, 24);

/* rcv_bad_cnt_type
 * Access: RW
 */
MLXSW_ITEM8(reg, rarncg, rcv_bad_cnt_type, 0x1c, 0, 8);

/* arn_rcv_bad_cnt
 * Access: RW
 */
MLXSW_ITEM32(reg, rarncg, arn_rcv_bad_cnt, 0x1c, 0, 24);

/* rargcs
 * ------
 *
 */
/* e
 * Access: RW
 */
MLXSW_ITEM8(reg, rargcs, e, 0x0, 7, 1);

/* set
 * Access: RW
 */
MLXSW_ITEM8(reg, rargcs, set, 0x3, 0, 8);

/* rargtc
 * ------
 *
 */
/* g
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rargtc, g, 0x0, 7, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rargtc, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, rargtc, lp_msb, 0x2, 4, 2);

/* lossless_tc_map
 * Access: WO
 */
MLXSW_ITEM16(reg, rargtc, lossless_tc_map, 0xa, 0, 16);

/* rirt
 * ----
 *
 */
/* offset
 * Access: INDEX
 */
MLXSW_ITEM16(reg, rirt, offset, 0x2, 0, 12);

/* valid
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, rirt, valid, MLXSW_RIRT_ROUTER_ENTRY_RECORD_BASE_LEN, 7, 1, MLXSW_RIRT_ROUTER_ENTRY_RECORD_REC_LEN, 0, 1, 0);

/* swid
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, rirt, swid, MLXSW_RIRT_ROUTER_ENTRY_RECORD_BASE_LEN + 3, 0, 3, MLXSW_RIRT_ROUTER_ENTRY_RECORD_REC_LEN, 0, 1, 0);

/* subnet_prefix
 * Access: RO
 */
MLXSW_ITEM16_INDEXED(reg, rirt, subnet_prefix, MLXSW_RIRT_ROUTER_ENTRY_RECORD_BASE_LEN + 2, 0, 16, MLXSW_RIRT_ROUTER_ENTRY_RECORD_REC_LEN, 4, 2, 0);

/* mpnhlfe
 * -------
 *
 */
/* ca
 * Access: OP
 */
MLXSW_ITEM8(reg, mpnhlfe, ca, 0x0, 7, 1);

/* a
 * Access: RO
 */
MLXSW_ITEM8(reg, mpnhlfe, a, 0x1, 0, 1);

/* v
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, v, 0x4, 7, 1);

/* nhlfe_ptr
 * Access: INDEX
 */
MLXSW_ITEM32(reg, mpnhlfe, nhlfe_ptr, 0x4, 0, 24);

/* forward_action
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, forward_action, 0xb, 0, 4);

/* trap_action
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, trap_action, 0xc, 4, 4);

/* trap_id
 * Access: RW
 */
MLXSW_ITEM16(reg, mpnhlfe, trap_id, 0xe, 0, 9);

/* counter_set_type
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, counter_set_type, 0x10, 0, 8);

/* counter_index
 * Access: RW
 */
MLXSW_ITEM32(reg, mpnhlfe, counter_index, 0x10, 0, 24);

/* mpnhlfe_param_eth_dscp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_eth_dscp_rw, 0x30, 4, 2);

/* mpnhlfe_param_eth_uecn_exp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_eth_uecn_exp, 0x30, 2, 1);

/* mpnhlfe_param_eth_usp_dscp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_eth_usp_dscp, 0x30, 1, 1);

/* mpnhlfe_param_eth_usp_exp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_eth_usp_exp, 0x30, 0, 1);

/* mpnhlfe_param_eth_exp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_eth_exp_rw, 0x31, 4, 2);

/* mpnhlfe_param_eth_set_exp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_eth_set_exp, 0x31, 3, 1);

/* mpnhlfe_param_eth_exp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_eth_exp, 0x31, 0, 3);

/* mpnhlfe_param_eth_tqos_profile
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_eth_tqos_profile, 0x32, 4, 4);

/* mpnhlfe_param_eth_expected_ip_ver
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_eth_expected_ip_ver, 0x33, 4, 2);

/* mpnhlfe_param_eth_bos_check
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_eth_bos_check, 0x33, 0, 2);

/* mpnhlfe_param_eth_label_action
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_eth_label_action, 0x34, 0, 3);

/* mpnhlfe_param_eth_label_id
 * Access: RW
 */
MLXSW_ITEM32(reg, mpnhlfe, mpnhlfe_param_eth_label_id, 0x34, 0, 20);

/* mpnhlfe_param_eth_dmac
 * Access: RW
 */
MLXSW_ITEM_BUF(reg, mpnhlfe, mpnhlfe_param_eth_dmac, 0x3a, 6);

/* mpnhlfe_param_eth_egress_router_interface
 * Access: RW
 */
MLXSW_ITEM16(reg, mpnhlfe, mpnhlfe_param_eth_egress_router_interface, 0x42, 0, 16);

/* mpnhlfe_param_eth_protection_en
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_eth_protection_en, 0x44, 1, 1);

/* mpnhlfe_param_eth_protection_active
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_eth_protection_active, 0x44, 0, 1);

/* mpnhlfe_param_eth_protection_nhlfe_ptr
 * Access: RW
 */
MLXSW_ITEM32(reg, mpnhlfe, mpnhlfe_param_eth_protection_nhlfe_ptr, 0x48, 0, 24);

/* mpnhlfe_param_iprouter_dscp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_iprouter_dscp_rw, 0x30, 4, 2);

/* mpnhlfe_param_iprouter_uecn_exp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_iprouter_uecn_exp, 0x30, 2, 1);

/* mpnhlfe_param_iprouter_usp_dscp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_iprouter_usp_dscp, 0x30, 1, 1);

/* mpnhlfe_param_iprouter_usp_exp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_iprouter_usp_exp, 0x30, 0, 1);

/* mpnhlfe_param_iprouter_tqos_profile
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_iprouter_tqos_profile, 0x31, 0, 4);

/* mpnhlfe_param_iprouter_irifv
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_iprouter_irifv, 0x3c, 0, 1);

/* mpnhlfe_param_iprouter_irif
 * Access: RW
 */
MLXSW_ITEM16(reg, mpnhlfe, mpnhlfe_param_iprouter_irif, 0x3e, 0, 16);

/* mpnhlfe_param_iprouter_erifv
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_iprouter_erifv, 0x40, 0, 1);

/* mpnhlfe_param_iprouter_egress_router_interface
 * Access: RW
 */
MLXSW_ITEM16(reg, mpnhlfe, mpnhlfe_param_iprouter_egress_router_interface, 0x42, 0, 16);

/* mpnhlfe_param_iprouter_cbset_select
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_iprouter_cbset_select, 0x4b, 0, 3);

/* mpnhlfe_param_iprouter_cbset_val
 * Access: RW
 */
MLXSW_ITEM16(reg, mpnhlfe, mpnhlfe_param_iprouter_cbset_val, 0x4e, 0, 16);

/* mpnhlfe_param_iprouter_cbset_mask
 * Access: RW
 */
MLXSW_ITEM16(reg, mpnhlfe, mpnhlfe_param_iprouter_cbset_mask, 0x52, 0, 16);

/* mpnhlfe_param_lookups_dscp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_lookups_dscp_rw, 0x30, 4, 2);

/* mpnhlfe_param_lookups_uecn_exp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_lookups_uecn_exp, 0x30, 2, 1);

/* mpnhlfe_param_lookups_usp_dscp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_lookups_usp_dscp, 0x30, 1, 1);

/* mpnhlfe_param_lookups_usp_exp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_lookups_usp_exp, 0x30, 0, 1);

/* mpnhlfe_param_lookups_tqos_profile
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_lookups_tqos_profile, 0x31, 0, 4);

/* mpnhlfe_param_lookups_bos_check
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_lookups_bos_check, 0x33, 0, 2);

/* mpnhlfe_param_lookups_irifv
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_lookups_irifv, 0x3c, 0, 1);

/* mpnhlfe_param_lookups_irif
 * Access: RW
 */
MLXSW_ITEM16(reg, mpnhlfe, mpnhlfe_param_lookups_irif, 0x3e, 0, 16);

/* mpnhlfe_param_lookups_erifv
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_lookups_erifv, 0x40, 0, 1);

/* mpnhlfe_param_lookups_egress_router_interface
 * Access: RW
 */
MLXSW_ITEM16(reg, mpnhlfe, mpnhlfe_param_lookups_egress_router_interface, 0x42, 0, 16);

/* mpnhlfe_param_lookups_cbset_select
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, mpnhlfe_param_lookups_cbset_select, 0x4b, 0, 3);

/* mpnhlfe_param_lookups_cbset_val
 * Access: RW
 */
MLXSW_ITEM16(reg, mpnhlfe, mpnhlfe_param_lookups_cbset_val, 0x4e, 0, 16);

/* mpnhlfe_param_lookups_cbset_mask
 * Access: RW
 */
MLXSW_ITEM16(reg, mpnhlfe, mpnhlfe_param_lookups_cbset_mask, 0x52, 0, 16);

/* exp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, exp_rw, 0x31, 4, 2);

/* set_exp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, set_exp, 0x31, 3, 1);

/* exp
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, exp, 0x31, 0, 3);

/* label_action
 * Access: RW
 */
MLXSW_ITEM8(reg, mpnhlfe, label_action, 0x34, 0, 3);

/* label_id
 * Access: RW
 */
MLXSW_ITEM32(reg, mpnhlfe, label_id, 0x34, 0, 20);

/* next_nhlfe
 * Access: RW
 */
MLXSW_ITEM32(reg, mpnhlfe, next_nhlfe, 0x40, 0, 24);

/* ecmp_size
 * Access: RW
 */
MLXSW_ITEM16(reg, mpnhlfe, ecmp_size, 0x46, 0, 13);

/* tnqcr
 * -----
 *
 */
/* enc_set_dscp
 * Access: RW
 */
MLXSW_ITEM8(reg, tnqcr, enc_set_dscp, 0x4, 4, 1);

/* enc_set_sp
 * Access: RW
 */
MLXSW_ITEM8(reg, tnqcr, enc_set_sp, 0x4, 0, 1);

/* enc_dscp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, tnqcr, enc_dscp_rw, 0x7, 2, 2);

/* enc_pcp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, tnqcr, enc_pcp_rw, 0x7, 0, 2);

/* dec_set_dscp
 * Access: RW
 */
MLXSW_ITEM8(reg, tnqcr, dec_set_dscp, 0x8, 4, 1);

/* dec_set_sp
 * Access: RW
 */
MLXSW_ITEM8(reg, tnqcr, dec_set_sp, 0x8, 0, 1);

/* dec_set_pcp
 * Access: RW
 */
MLXSW_ITEM8(reg, tnqcr, dec_set_pcp, 0x9, 4, 1);

/* dec_dscp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, tnqcr, dec_dscp_rw, 0xb, 2, 2);

/* dec_pcp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, tnqcr, dec_pcp_rw, 0xb, 0, 2);

/* tnpc
 * ----
 *
 */
/* tunnel_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, tnpc, tunnel_port, 0x3, 0, 4);

/* learn_enable_v6
 * Access: RW
 */
MLXSW_ITEM8(reg, tnpc, learn_enable_v6, 0x7, 1, 1);

/* learn_enable_v4
 * Access: RW
 */
MLXSW_ITEM8(reg, tnpc, learn_enable_v4, 0x7, 0, 1);

/* lb_tx_uc_tunnel_port
 * Access: RW
 */
MLXSW_ITEM32(reg, tnpc, lb_tx_uc_tunnel_port, 0x14, 0, 32);

/* lb_tx_mc_tunnel_port
 * Access: RW
 */
MLXSW_ITEM32(reg, tnpc, lb_tx_mc_tunnel_port, 0x18, 0, 32);

/* tngee
 * -----
 *
 */
/* l2_enc_index
 * Access: INDEX
 */
MLXSW_ITEM32(reg, tngee, l2_enc_index, 0x0, 0, 24);

/* tunnel_type
 * Access: RW
 */
MLXSW_ITEM8(reg, tngee, tunnel_type, 0x7, 0, 4);

/* vnext
 * Access: RW
 */
MLXSW_ITEM8(reg, tngee, vnext, 0x8, 7, 1);

/* next_l2_enc_ptr
 * Access: RW
 */
MLXSW_ITEM32(reg, tngee, next_l2_enc_ptr, 0x8, 0, 24);

/* ecmp_size
 * Access: RW
 */
MLXSW_ITEM16(reg, tngee, ecmp_size, 0xe, 0, 13);

/* protocol
 * Access: RW
 */
MLXSW_ITEM8(reg, tngee, protocol, 0x10, 4, 1);

/* uirif
 * Access: RW
 */
MLXSW_ITEM16(reg, tngee, uirif, 0x12, 0, 16);

/* udip
 * Access: RW
 */
MLXSW_ITEM32(reg, tngee, udip, 0x14, 0, 32);

/* counter_set_type
 * Access: RW
 */
MLXSW_ITEM8(reg, tngee, counter_set_type, 0x3c, 0, 8);

/* counter_index
 * Access: RW
 */
MLXSW_ITEM32(reg, tngee, counter_index, 0x3c, 0, 24);

/* tngee_nve_vni_update
 * Access: RW
 */
MLXSW_ITEM8(reg, tngee, tngee_nve_vni_update, 0x40, 7, 1);

/* tngee_nve_tqos_prof_en
 * Access: OP
 */
MLXSW_ITEM8(reg, tngee, tngee_nve_tqos_prof_en, 0x40, 6, 1);

/* tngee_nve_vni
 * Access: RW
 */
MLXSW_ITEM32(reg, tngee, tngee_nve_vni, 0x40, 0, 24);

/* tngee_nve_tqos_profile
 * Access: RW
 */
MLXSW_ITEM8(reg, tngee, tngee_nve_tqos_profile, 0x47, 0, 4);

/* tngee_flex_reg_tqos_profile
 * Access: RW
 */
MLXSW_ITEM8(reg, tngee, tngee_flex_reg_tqos_profile, 0x40, 0, 4);

/* tngee_flex_reg_tunnel_port
 * Access: RW
 */
MLXSW_ITEM8(reg, tngee, tngee_flex_reg_tunnel_port, 0x43, 0, 4);

/* tngee_flex_reg_emt_pointer
 * Access: RW
 */
MLXSW_ITEM8(reg, tngee, tngee_flex_reg_emt_pointer, 0x47, 0, 8);

/* tngee_flex_reg_cbset_select
 * Access: RW
 */
MLXSW_ITEM8(reg, tngee, tngee_flex_reg_cbset_select, 0x4a, 0, 2);

/* tngee_flex_reg_cbset_x_set_en
 * Access: RW
 */
MLXSW_ITEM8(reg, tngee, tngee_flex_reg_cbset_x_set_en, 0x4b, 1, 1);

/* tngee_flex_reg_cbset_y_set_en
 * Access: RW
 */
MLXSW_ITEM8(reg, tngee, tngee_flex_reg_cbset_y_set_en, 0x4b, 0, 1);

/* tngee_flex_reg_cbset_x_val
 * Access: RW
 */
MLXSW_ITEM16(reg, tngee, tngee_flex_reg_cbset_x_val, 0x4c, 0, 16);

/* tngee_flex_reg_cbset_y_val
 * Access: RW
 */
MLXSW_ITEM16(reg, tngee, tngee_flex_reg_cbset_y_val, 0x4e, 0, 16);

/* tncr
 * ----
 *
 */
/* clear_counters
 * Access: OP
 */
MLXSW_ITEM8(reg, tncr, clear_counters, 0x0, 7, 1);

/* count_encap_high
 * Access: RO
 */
MLXSW_ITEM32(reg, tncr, count_encap_high, 0x10, 0, 32);

/* count_encap_low
 * Access: RO
 */
MLXSW_ITEM32(reg, tncr, count_encap_low, 0x14, 0, 32);

/* count_decap_high
 * Access: RO
 */
MLXSW_ITEM32(reg, tncr, count_decap_high, 0x18, 0, 32);

/* count_decap_low
 * Access: RO
 */
MLXSW_ITEM32(reg, tncr, count_decap_low, 0x1c, 0, 32);

/* count_decap_errors_high
 * Access: RO
 */
MLXSW_ITEM32(reg, tncr, count_decap_errors_high, 0x20, 0, 32);

/* count_decap_errors_low
 * Access: RO
 */
MLXSW_ITEM32(reg, tncr, count_decap_errors_low, 0x24, 0, 32);

/* count_decap_discards_high
 * Access: RO
 */
MLXSW_ITEM32(reg, tncr, count_decap_discards_high, 0x28, 0, 32);

/* count_decap_discards_low
 * Access: RO
 */
MLXSW_ITEM32(reg, tncr, count_decap_discards_low, 0x2c, 0, 32);

/* tncr_v2
 * -------
 *
 */
/* clear_counters
 * Access: OP
 */
MLXSW_ITEM8(reg, tncr_v2, clear_counters, 0x0, 7, 1);

/* tunnel_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, tncr_v2, tunnel_port, 0x3, 0, 4);

/* count_decap_discards_high
 * Access: RO
 */
MLXSW_ITEM32(reg, tncr_v2, count_decap_discards_high, 0x28, 0, 32);

/* count_decap_discards_low
 * Access: RO
 */
MLXSW_ITEM32(reg, tncr_v2, count_decap_discards_low, 0x2c, 0, 32);

/* count_encap_discards_high
 * Access: RO
 */
MLXSW_ITEM32(reg, tncr_v2, count_encap_discards_high, 0x30, 0, 32);

/* count_encap_discards_low
 * Access: RO
 */
MLXSW_ITEM32(reg, tncr_v2, count_encap_discards_low, 0x34, 0, 32);

/* tnumt
 * -----
 *
 */
/* record_type
 * Access: RW
 */
MLXSW_ITEM8(reg, tnumt, record_type, 0x0, 4, 4);

/* underlay_mc_ptr
 * Access: INDEX
 */
MLXSW_ITEM32(reg, tnumt, underlay_mc_ptr, 0x0, 0, 24);

/* vnext
 * Access: RW
 */
MLXSW_ITEM8(reg, tnumt, vnext, 0x4, 7, 1);

/* next_underlay_mc_ptr
 * Access: RW
 */
MLXSW_ITEM32(reg, tnumt, next_underlay_mc_ptr, 0x4, 0, 24);

/* tunnel_mc_ipv4_size
 * Access: RW
 */
MLXSW_ITEM8(reg, tnumt, tunnel_mc_ipv4_size, 0xb, 0, 3);

/* tunnel_mc_ipv4_udip
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, tnumt, tunnel_mc_ipv4_udip, 0xc, 0, 32, 4, 0, 4, 0);

/* tunnel_mc_ipv6_size
 * Access: RW
 */
MLXSW_ITEM8(reg, tnumt, tunnel_mc_ipv6_size, 0xb, 0, 3);

/* tunnel_mc_ipv6_udip_ptr
 * Access: 
 */
MLXSW_ITEM32_INDEXED(reg, tnumt, tunnel_mc_ipv6_udip_ptr, 0xc, 0, 24, 0x4, 0x0, 3, 0);

/* tngcr
 * -----
 *
 */
/* type
 * Access: RW
 */
MLXSW_ITEM8(reg, tngcr, type, 0x3, 0, 4);

/* nve_valid
 * Access: RW
 */
MLXSW_ITEM8(reg, tngcr, nve_valid, 0x4, 7, 1);

/* nve_decap_ttl
 * Access: RW
 */
MLXSW_ITEM8(reg, tngcr, nve_decap_ttl, 0x5, 0, 2);

/* nve_ttlc
 * Access: RW
 */
MLXSW_ITEM8(reg, tngcr, nve_ttlc, 0x6, 0, 1);

/* nve_ttl_uc
 * Access: RW
 */
MLXSW_ITEM8(reg, tngcr, nve_ttl_uc, 0x7, 0, 8);

/* nve_ttl_mc
 * Access: RW
 */
MLXSW_ITEM8(reg, tngcr, nve_ttl_mc, 0xb, 0, 8);

/* nve_flc
 * Access: RW
 */
MLXSW_ITEM8(reg, tngcr, nve_flc, 0xc, 1, 1);

/* nve_flh
 * Access: RW
 */
MLXSW_ITEM8(reg, tngcr, nve_flh, 0xc, 0, 1);

/* nve_fl_prefix
 * Access: RW
 */
MLXSW_ITEM32(reg, tngcr, nve_fl_prefix, 0xc, 8, 12);

/* nve_fl_suffix
 * Access: RW
 */
MLXSW_ITEM8(reg, tngcr, nve_fl_suffix, 0xf, 0, 8);

/* nve_enc_orig
 * Access: RW
 */
MLXSW_ITEM8(reg, tngcr, nve_enc_orig, 0x10, 7, 1);

/* nve_enc_orig_we
 * Access: WO
 */
MLXSW_ITEM8(reg, tngcr, nve_enc_orig_we, 0x10, 6, 1);

/* nve_udp_sport_type
 * Access: RW
 */
MLXSW_ITEM8(reg, tngcr, nve_udp_sport_type, 0x10, 0, 1);

/* et_vlan
 * Access: RW
 */
MLXSW_ITEM8(reg, tngcr, et_vlan, 0x11, 4, 2);

/* nve_udp_sport_prefix
 * Access: RW
 */
MLXSW_ITEM8(reg, tngcr, nve_udp_sport_prefix, 0x12, 0, 8);

/* nve_udp_sport_suffix
 * Access: RW
 */
MLXSW_ITEM8(reg, tngcr, nve_udp_sport_suffix, 0x13, 0, 8);

/* nve_group_size_mc
 * Access: RW
 */
MLXSW_ITEM8(reg, tngcr, nve_group_size_mc, 0x1b, 0, 8);

/* nve_group_size_flood
 * Access: RW
 */
MLXSW_ITEM8(reg, tngcr, nve_group_size_flood, 0x1f, 0, 8);

/* learn_enable
 * Access: RW
 */
MLXSW_ITEM8(reg, tngcr, learn_enable, 0x20, 7, 1);

/* dis_nve_opt_chk
 * Access: RW
 */
MLXSW_ITEM8(reg, tngcr, dis_nve_opt_chk, 0x20, 0, 1);

/* underlay_virtual_router
 * Access: RW
 */
MLXSW_ITEM16(reg, tngcr, underlay_virtual_router, 0x22, 0, 16);

/* underlay_rif
 * Access: RW
 */
MLXSW_ITEM16(reg, tngcr, underlay_rif, 0x26, 0, 16);

/* usipv4
 * Access: RW
 */
MLXSW_ITEM32(reg, tngcr, usipv4, 0x28, 0, 32);

/* usipv6
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, tngcr, usipv6, 0x30, 0, 32, 4, 0, 4, 0);

/* header0_bit_set
 * Access: RW
 */
MLXSW_ITEM32(reg, tngcr, header0_bit_set, 0x40, 0, 32);

/* nvgre_hdr_bits_en
 * Access: WO
 */
MLXSW_ITEM8(reg, tngcr, nvgre_hdr_bits_en, 0x47, 3, 1);

/* geneve_hdr_bits_en
 * Access: WO
 */
MLXSW_ITEM8(reg, tngcr, geneve_hdr_bits_en, 0x47, 2, 1);

/* gpe_hdr_bits_en
 * Access: WO
 */
MLXSW_ITEM8(reg, tngcr, gpe_hdr_bits_en, 0x47, 1, 1);

/* vxlan_hdr_bits_en
 * Access: WO
 */
MLXSW_ITEM8(reg, tngcr, vxlan_hdr_bits_en, 0x47, 0, 1);

/* vxlan_hdr_bits
 * Access: RW
 */
MLXSW_ITEM64(reg, tngcr, vxlan_hdr_bits, 0x48, 0, 64);

/* gpe_hdr_bits
 * Access: RW
 */
MLXSW_ITEM64(reg, tngcr, gpe_hdr_bits, 0x50, 0, 64);

/* geneve_hdr_bits
 * Access: RW
 */
MLXSW_ITEM64(reg, tngcr, geneve_hdr_bits, 0x58, 0, 64);

/* nvgre_hdr_bits
 * Access: RW
 */
MLXSW_ITEM32(reg, tngcr, nvgre_hdr_bits, 0x60, 0, 32);

/* tnqdr
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, tnqdr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, tnqdr, lp_msb, 0x2, 4, 2);

/* color
 * Access: RW
 */
MLXSW_ITEM8(reg, tnqdr, color, 0x4, 0, 2);

/* switch_prio
 * Access: RW
 */
MLXSW_ITEM8(reg, tnqdr, switch_prio, 0x5, 0, 4);

/* dscp
 * Access: RW
 */
MLXSW_ITEM8(reg, tnqdr, dscp, 0x7, 0, 6);

/* tneem
 * -----
 *
 */
/* overlay_ecn
 * Access: INDEX
 */
MLXSW_ITEM8(reg, tneem, overlay_ecn, 0x4, 0, 2);

/* underlay_ecn
 * Access: RW
 */
MLXSW_ITEM8(reg, tneem, underlay_ecn, 0x5, 0, 2);

/* tndem
 * -----
 *
 */
/* underlay_ecn
 * Access: INDEX
 */
MLXSW_ITEM8(reg, tndem, underlay_ecn, 0x4, 0, 2);

/* overlay_ecn
 * Access: INDEX
 */
MLXSW_ITEM8(reg, tndem, overlay_ecn, 0x5, 0, 2);

/* eip_ecn
 * Access: RW
 */
MLXSW_ITEM8(reg, tndem, eip_ecn, 0x6, 0, 2);

/* trap_en
 * Access: RW
 */
MLXSW_ITEM8(reg, tndem, trap_en, 0x8, 4, 4);

/* trap_id
 * Access: RW
 */
MLXSW_ITEM16(reg, tndem, trap_id, 0xa, 0, 9);

/* tnifr
 * -----
 *
 */
/* table_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, tnifr, table_id, 0x0, 4, 4);

/* tunnel_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, tnifr, tunnel_port, 0x3, 0, 4);

/* port_filter
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, tnifr, port_filter, 0x20, 0, 32, 4, 0, 4, 0);

/* port_filter_update_en
 * Access: OP
 */
MLXSW_ITEM32_INDEXED(reg, tnifr, port_filter_update_en, 0x40, 0, 32, 4, 0, 4, 0);

/* tnifr_v2
 * --------
 *
 */
/* table_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, tnifr_v2, table_id, 0x0, 4, 4);

/* tunnel_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, tnifr_v2, tunnel_port, 0x3, 0, 4);

/* port_filter
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, tnifr_v2, port_filter, 0x20, 0, 32, 4, 0, 4, 0);

/* port_filter_update_en
 * Access: OP
 */
MLXSW_ITEM32_INDEXED(reg, tnifr_v2, port_filter_update_en, 0xa0, 0, 32, 4, 0, 4, 0);

/* tigcr
 * -----
 *
 */
/* ipip_decap_ttl
 * Access: RW
 */
MLXSW_ITEM8(reg, tigcr, ipip_decap_ttl, 0x5, 0, 2);

/* ipip_ttlc
 * Access: RW
 */
MLXSW_ITEM8(reg, tigcr, ipip_ttlc, 0x6, 0, 1);

/* ipip_ttl_uc
 * Access: RW
 */
MLXSW_ITEM8(reg, tigcr, ipip_ttl_uc, 0x7, 0, 8);

/* ipip_flc
 * Access: RW
 */
MLXSW_ITEM8(reg, tigcr, ipip_flc, 0xc, 1, 1);

/* ipip_flh
 * Access: RW
 */
MLXSW_ITEM8(reg, tigcr, ipip_flh, 0xc, 0, 1);

/* ipip_fl_prefix
 * Access: RW
 */
MLXSW_ITEM32(reg, tigcr, ipip_fl_prefix, 0xc, 8, 12);

/* ipip_fl_suffix
 * Access: RW
 */
MLXSW_ITEM8(reg, tigcr, ipip_fl_suffix, 0xf, 0, 8);

/* ipip_gre_key_for_hash
 * Access: RW
 */
MLXSW_ITEM32(reg, tigcr, ipip_gre_key_for_hash, 0x14, 8, 24);

/* tiqcr
 * -----
 *
 */
/* enc_set_dscp
 * Access: RW
 */
MLXSW_ITEM8(reg, tiqcr, enc_set_dscp, 0x4, 4, 1);

/* enc_set_sp
 * Access: RW
 */
MLXSW_ITEM8(reg, tiqcr, enc_set_sp, 0x4, 0, 1);

/* enc_dscp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, tiqcr, enc_dscp_rw, 0x7, 2, 2);

/* enc_pcp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, tiqcr, enc_pcp_rw, 0x7, 0, 2);

/* dec_set_dscp
 * Access: RW
 */
MLXSW_ITEM8(reg, tiqcr, dec_set_dscp, 0x8, 4, 1);

/* dec_set_sp
 * Access: RW
 */
MLXSW_ITEM8(reg, tiqcr, dec_set_sp, 0x8, 0, 1);

/* dec_dscp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, tiqcr, dec_dscp_rw, 0xb, 2, 2);

/* dec_pcp_rw
 * Access: RW
 */
MLXSW_ITEM8(reg, tiqcr, dec_pcp_rw, 0xb, 0, 2);

/* tiqdr
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, tiqdr, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, tiqdr, lp_msb, 0x2, 4, 2);

/* color
 * Access: RW
 */
MLXSW_ITEM8(reg, tiqdr, color, 0x4, 0, 2);

/* switch_prio
 * Access: RW
 */
MLXSW_ITEM8(reg, tiqdr, switch_prio, 0x5, 0, 4);

/* dscp
 * Access: RW
 */
MLXSW_ITEM8(reg, tiqdr, dscp, 0x7, 0, 6);

/* tieem
 * -----
 *
 */
/* overlay_ecn
 * Access: INDEX
 */
MLXSW_ITEM8(reg, tieem, overlay_ecn, 0x4, 0, 2);

/* underlay_ecn
 * Access: RW
 */
MLXSW_ITEM8(reg, tieem, underlay_ecn, 0x5, 0, 2);

/* tidem
 * -----
 *
 */
/* underlay_ecn
 * Access: INDEX
 */
MLXSW_ITEM8(reg, tidem, underlay_ecn, 0x4, 0, 2);

/* overlay_ecn
 * Access: INDEX
 */
MLXSW_ITEM8(reg, tidem, overlay_ecn, 0x5, 0, 2);

/* eip_ecn
 * Access: RW
 */
MLXSW_ITEM8(reg, tidem, eip_ecn, 0x6, 0, 2);

/* trap_en
 * Access: RW
 */
MLXSW_ITEM8(reg, tidem, trap_en, 0x8, 4, 4);

/* trap_id
 * Access: RW
 */
MLXSW_ITEM16(reg, tidem, trap_id, 0xa, 0, 9);

/* fgcr
 * ----
 *
 */
/* flat_sec_offset
 * Access: RW
 */
MLXSW_ITEM8(reg, fgcr, flat_sec_offset, 0x3, 0, 8);

/* fpums
 * -----
 *
 */
/* global
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fpums, global, 0x0, 4, 2);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fpums, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fpums, lp_msb, 0x2, 4, 2);

/* bitmap_size
 * Access: OP
 */
MLXSW_ITEM8(reg, fpums, bitmap_size, 0x3, 0, 8);

/* mask
 * Access: OP
 */
MLXSW_ITEM8(reg, fpums, mask, 0xb, 0, 8);

/* port_user_mem
 * Access: RW
 */
MLXSW_ITEM8(reg, fpums, port_user_mem, 0xf, 0, 8);

/* port_mask
 * Access: INDEX
 */
MLXSW_ITEM32_INDEXED(reg, fpums, port_mask, 0x10, 0, 32, 4, 0, 4, 0);

/* fphhc
 * -----
 *
 */
/* hph
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fphhc, hph, 0x3, 0, 8);

/* my_ftlv_outer_en
 * Access: RW
 */
MLXSW_ITEM8(reg, fphhc, my_ftlv_outer_en, 0x8, 7, 1);

/* my_ftlv_inner_en
 * Access: RW
 */
MLXSW_ITEM8(reg, fphhc, my_ftlv_inner_en, 0x8, 6, 1);

/* my_ftlv_fpp
 * Access: RW
 */
MLXSW_ITEM8(reg, fphhc, my_ftlv_fpp, 0xb, 0, 8);

/* fppc
 * ----
 *
 */
/* as_ftlv
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, as_ftlv, 0x0, 4, 1);

/* fpp
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fppc, fpp, 0x3, 0, 8);

/* empty_fph
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, empty_fph, 0x7, 0, 1);

/* field_offset_dword_prot
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, field_offset_dword_prot, 0x12, 0, 2);

/* field_offset_bytes_prot
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, field_offset_bytes_prot, 0x13, 0, 2);

/* field_mask_prot_non_tlv
 * Access: RW
 */
MLXSW_ITEM16(reg, fppc, field_mask_prot_non_tlv, 0x16, 0, 16);

/* field_mask_prot_tlv
 * Access: RW
 */
MLXSW_ITEM32(reg, fppc, field_mask_prot_tlv, 0x18, 0, 24);

/* field_offset_dword_length
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, field_offset_dword_length, 0x22, 0, 2);

/* field_offset_bytes_length
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, field_offset_bytes_length, 0x23, 0, 2);

/* field_mask_length
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, field_mask_length, 0x27, 0, 8);

/* length_math_bitmap
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, length_math_bitmap, 0x28, 4, 1);

/* length_math_add_pos
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, length_math_add_pos, 0x28, 0, 1);

/* length_math_add
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, length_math_add, 0x29, 0, 7);

/* length_math_shift_left
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, length_math_shift_left, 0x2a, 0, 1);

/* length_math_shift
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, length_math_shift, 0x2b, 0, 3);

/* fexp_st_en
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, fexp_st_en, 0x30, 7, 1);

/* fexp_st_id
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, fexp_st_id, 0x30, 0, 3);

/* fexp_st_tlv_type
 * Access: RW
 */
MLXSW_ITEM32(reg, fppc, fexp_st_tlv_type, 0x30, 0, 24);

/* fexp_of_en
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, fexp_of_en, 0x40, 7, 1);

/* fexp_of_id
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, fexp_of_id, 0x40, 0, 3);

/* fexp_of_tlv_type
 * Access: RW
 */
MLXSW_ITEM32(reg, fppc, fexp_of_tlv_type, 0x40, 0, 24);

/* field_offset_dword_fexp_of
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, field_offset_dword_fexp_of, 0x46, 0, 2);

/* field_offset_bytes_fexp_of
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, field_offset_bytes_fexp_of, 0x47, 0, 2);

/* field_mask_fexp_of
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, field_mask_fexp_of, 0x4b, 0, 8);

/* fexp_of_math_bitmap
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, fexp_of_math_bitmap, 0x4c, 4, 1);

/* fexp_of_math_add_pos
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, fexp_of_math_add_pos, 0x4c, 0, 1);

/* fexp_of_math_add
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, fexp_of_math_add, 0x4d, 0, 7);

/* fexp_of_math_shift_left
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, fexp_of_math_shift_left, 0x4e, 0, 1);

/* fexp_of_math_shift
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, fexp_of_math_shift, 0x4f, 0, 3);

/* my_ftlv_en
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, my_ftlv_en, 0x50, 7, 1);

/* my_ftlv_start
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, my_ftlv_start, 0x51, 0, 6);

/* my_ftlv_fpp
 * Access: RW
 */
MLXSW_ITEM8(reg, fppc, my_ftlv_fpp, 0x53, 0, 8);

/* fpftt
 * -----
 *
 */
/* entry_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fpftt, entry_index, 0x3, 0, 8);

/* tran_en
 * Access: RW
 */
MLXSW_ITEM8(reg, fpftt, tran_en, 0x7, 0, 1);

/* cur_ph_inner
 * Access: RW
 */
MLXSW_ITEM8(reg, fpftt, cur_ph_inner, 0x9, 0, 1);

/* cur_ph
 * Access: RW
 */
MLXSW_ITEM8(reg, fpftt, cur_ph, 0xb, 0, 8);

/* next_protocol_value
 * Access: RW
 */
MLXSW_ITEM16(reg, fpftt, next_protocol_value, 0xe, 0, 16);

/* next_ph_inner
 * Access: RW
 */
MLXSW_ITEM8(reg, fpftt, next_ph_inner, 0x11, 0, 1);

/* next_ph
 * Access: RW
 */
MLXSW_ITEM8(reg, fpftt, next_ph, 0x13, 0, 8);

/* fphtt
 * -----
 *
 */
/* entry_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fphtt, entry_index, 0x3, 0, 8);

/* tran_en
 * Access: RW
 */
MLXSW_ITEM8(reg, fphtt, tran_en, 0x7, 0, 1);

/* fpts
 * ----
 *
 */
/* global
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fpts, global, 0x0, 7, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fpts, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fpts, lp_msb, 0x2, 4, 2);

/* tran_type
 * Access: RW
 */
MLXSW_ITEM8(reg, fpts, tran_type, 0x7, 0, 4);

/* next_ph
 * Access: RW
 */
MLXSW_ITEM8(reg, fpts, next_ph, 0xb, 0, 8);

/* fmqc
 * ----
 *
 */
/* s
 * Access: RO
 */
MLXSW_ITEM8(reg, fmqc, s, 0x1, 0, 1);

/* lp
 * Access: INDEX
 */
MLXSW_ITEM16(reg, fmqc, lp, 0x2, 0, 10);

/* modifier_cluster
 * Access: RO
 */
MLXSW_ITEM16(reg, fmqc, modifier_cluster, 0x6, 0, 11);

/* fmtpc
 * -----
 *
 */
/* prog_idx
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmtpc, prog_idx, 0x1, 0, 2);

/* pc
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmtpc, pc, 0x3, 0, 6);

/* op_code
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtpc, op_code, 0x7, 0, 8);

/* operand0
 * Access: RW
 */
MLXSW_ITEM16(reg, fmtpc, operand0, 0xa, 0, 16);

/* operand1
 * Access: RW
 */
MLXSW_ITEM16(reg, fmtpc, operand1, 0xe, 0, 16);

/* operand2
 * Access: RW
 */
MLXSW_ITEM16(reg, fmtpc, operand2, 0x12, 0, 16);

/* fmtpa
 * -----
 *
 */
/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmtpa, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmtpa, lp_msb, 0x2, 4, 2);

/* prog_idx
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmtpa, prog_idx, 0x3, 0, 2);

/* e
 * Access: OP
 */
MLXSW_ITEM8(reg, fmtpa, e, 0x4, 7, 1);

/* bound_emt
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtpa, bound_emt, 0x7, 0, 3);

/* reg0
 * Access: RW
 */
MLXSW_ITEM16(reg, fmtpa, reg0, 0xa, 0, 16);

/* fmtb
 * ----
 *
 */
/* prog_idx
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmtb, prog_idx, 0x3, 0, 2);

/* enable
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtb, enable, 0x7, 0, 1);

/* type
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtb, type, 0x13, 0, 2);

/* emt_binding_info_atomic
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtb, emt_binding_info_atomic, 0x15, 0, 1);

/* emt_binding_info_emt_bitmask
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtb, emt_binding_info_emt_bitmask, 0x17, 0, 8);

/* ram_binding_info_atomic
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtb, ram_binding_info_atomic, 0x15, 0, 1);

/* fmtc
 * ----
 *
 */
/* per_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmtc, per_port, 0x0, 4, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmtc, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmtc, lp_msb, 0x2, 4, 2);

/* emt_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmtc, emt_index, 0x3, 0, 8);

/* length_cb
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, length_cb, 0x6, 0, 8);

/* length_cb_mask
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, length_cb_mask, 0x7, 0, 8);

/* length_math_bitmap
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, length_math_bitmap, 0x8, 4, 1);

/* length_math_add_pos
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, length_math_add_pos, 0x8, 0, 1);

/* length_math_shift_left
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, length_math_shift_left, 0x9, 4, 1);

/* length_math_shift
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, length_math_shift, 0x9, 0, 3);

/* length
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, length, 0xb, 0, 8);

/* update_geneve_length
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, update_geneve_length, 0x12, 0, 1);

/* update_udp_tcp_length
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, update_udp_tcp_length, 0x13, 4, 1);

/* update_ipv4_ihl
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, update_ipv4_ihl, 0x13, 3, 1);

/* update_hbh_length
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, update_hbh_length, 0x13, 2, 1);

/* update_ipv6_length
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, update_ipv6_length, 0x13, 1, 1);

/* update_ipv4_length
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, update_ipv4_length, 0x13, 0, 1);

/* update_ipv4_cs
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, update_ipv4_cs, 0x17, 5, 1);

/* update_udp_tcp_cs
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, update_udp_tcp_cs, 0x17, 4, 1);

/* clear_udp_cs
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, clear_udp_cs, 0x17, 0, 1);

/* max_latency
 * Access: RW
 */
MLXSW_ITEM16(reg, fmtc, max_latency, 0x22, 0, 9);

/* latency_invalid_value
 * Access: RW
 */
MLXSW_ITEM32(reg, fmtc, latency_invalid_value, 0x24, 0, 32);

/* buffer_fill_level_invalid_value
 * Access: RW
 */
MLXSW_ITEM32(reg, fmtc, buffer_fill_level_invalid_value, 0x28, 0, 32);

/* enc_l2_uirif
 * Access: RW
 */
MLXSW_ITEM16(reg, fmtc, enc_l2_uirif, 0x32, 0, 16);

/* enc_ipv4_id
 * Access: RW
 */
MLXSW_ITEM16(reg, fmtc, enc_ipv4_id, 0x38, 0, 16);

/* enc_ipv4_flag
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, enc_ipv4_flag, 0x3a, 5, 3);

/* enc_ipv4_frag_offset
 * Access: RW
 */
MLXSW_ITEM16(reg, fmtc, enc_ipv4_frag_offset, 0x3a, 0, 13);

/* enc_next_header
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, enc_next_header, 0x3d, 0, 8);

/* enc_flc
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, enc_flc, 0x40, 1, 1);

/* enc_flh
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, enc_flh, 0x40, 0, 1);

/* enc_fl_prefix
 * Access: RW
 */
MLXSW_ITEM32(reg, fmtc, enc_fl_prefix, 0x40, 8, 12);

/* enc_fl_suffix
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, enc_fl_suffix, 0x43, 0, 8);

/* s0_sel_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, s0_sel_mode, 0x45, 0, 3);

/* s0_offset
 * Access: RW
 */
MLXSW_ITEM16(reg, fmtc, s0_offset, 0x46, 0, 9);

/* s1_sel_mode
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtc, s1_sel_mode, 0x49, 0, 3);

/* s1_offset
 * Access: RW
 */
MLXSW_ITEM16(reg, fmtc, s1_offset, 0x4a, 0, 9);

/* fmte
 * ----
 *
 */
/* per_mod_clu
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmte, per_mod_clu, 0x0, 4, 1);

/* modifier_cluster
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmte, modifier_cluster, 0x1, 0, 8);

/* emt_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmte, emt_index, 0x3, 0, 8);

/* entry_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmte, entry_index, 0x7, 0, 8);

/* cmd_id
 * Access: RW
 */
MLXSW_ITEM8(reg, fmte, cmd_id, 0xb, 0, 8);

/* iim
 * Access: RW
 */
MLXSW_ITEM8(reg, fmte, iim, 0xd, 0, 1);

/* imm
 * Access: RW
 */
MLXSW_ITEM16(reg, fmte, imm, 0xe, 0, 16);

/* edit_en
 * Access: RW
 */
MLXSW_ITEM8(reg, fmte, edit_en, 0x13, 0, 1);

/* fmtm
 * ----
 *
 */
/* mapper_index
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmtm, mapper_index, 0x3, 0, 8);

/* emt_index_in
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtm, emt_index_in, 0x5, 0, 8);

/* emt_index_out
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtm, emt_index_out, 0x7, 0, 8);

/* fmep
 * ----
 *
 */
/* global
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmep, global, 0x0, 7, 1);

/* local_port
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmep, local_port, 0x1, 0, 8);

/* lp_msb
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmep, lp_msb, 0x2, 4, 2);

/* ext_mod_parsing
 * Access: RW
 */
MLXSW_ITEM16(reg, fmep, ext_mod_parsing, 0x6, 0, 9);

/* fsgcr
 * -----
 *
 */
/* failure_on_invalid_lock
 * Access: RW
 */
MLXSW_ITEM8(reg, fsgcr, failure_on_invalid_lock, 0x3, 1, 1);

/* sem_lock_protection_en
 * Access: RW
 */
MLXSW_ITEM8(reg, fsgcr, sem_lock_protection_en, 0x3, 0, 1);

/* ticket_cb_index
 * Access: RW
 */
MLXSW_ITEM8(reg, fsgcr, ticket_cb_index, 0x5, 0, 4);

/* sem_poll_max_retries
 * Access: RW
 */
MLXSW_ITEM8(reg, fsgcr, sem_poll_max_retries, 0x7, 0, 8);

/* linear_group_id
 * Access: RW
 */
MLXSW_ITEM16(reg, fsgcr, linear_group_id, 0xa, 0, 16);

/* fsdb
 * ----
 *
 */
/* acl_group_id
 * Access: INDEX
 */
MLXSW_ITEM16(reg, fsdb, acl_group_id, 0x2, 0, 16);

/* fr
 * Access: RO
 */
MLXSW_ITEM8(reg, fsdb, fr, 0x5, 0, 3);

/* partition_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fsdb, partition_id, 0x6, 0, 3);

/* db_op
 * Access: OP
 */
MLXSW_ITEM8(reg, fsdb, db_op, 0x7, 4, 3);

/* sem_op
 * Access: OP
 */
MLXSW_ITEM8(reg, fsdb, sem_op, 0x7, 0, 2);

/* s
 * Access: RO
 */
MLXSW_ITEM8(reg, fsdb, s, 0x8, 7, 1);

/* hit
 * Access: RO
 */
MLXSW_ITEM8(reg, fsdb, hit, 0x8, 2, 1);

/* a
 * Access: RO
 */
MLXSW_ITEM8(reg, fsdb, a, 0x8, 1, 1);

/* sem_val
 * Access: RO
 */
MLXSW_ITEM8(reg, fsdb, sem_val, 0x8, 0, 1);

/* sem_lock_cnt
 * Access: RO
 */
MLXSW_ITEM16(reg, fsdb, sem_lock_cnt, 0xa, 0, 16);

/* data
 * Access: RW
 */
MLXSW_ITEM64(reg, fsdb, data, 0x10, 0, 64);

/* key_data
 * Access: INDEX
 */
MLXSW_ITEM_BUF(reg, fsdb, key_data, 0x18, 96);

/* fspt
 * ----
 *
 */
/* set_partitions_bitmap
 * Access: OP
 */
MLXSW_ITEM8(reg, fspt, set_partitions_bitmap, 0x3, 0, 8);

/* warning_th_partition_threshold
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, fspt, warning_th_partition_threshold, 0x4, 0, 24, 0x4, 0x0, 3, 0);

/* max_th_partition_threshold
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, fspt, max_th_partition_threshold, 0x24, 0, 24, 0x4, 0x0, 3, 0);

/* fsdc
 * ----
 *
 */
/* sif
 * Access: OP
 */
MLXSW_ITEM8(reg, fsdc, sif, 0x0, 7, 1);

/* filter_fields
 * Access: OP
 */
MLXSW_ITEM8(reg, fsdc, filter_fields, 0x3, 0, 4);

/* group_id
 * Access: INDEX
 */
MLXSW_ITEM16(reg, fsdc, group_id, 0x4, 0, 16);

/* entry_sem_s
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fsdc, entry_sem_s, 0x6, 1, 1);

/* entry_activity
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fsdc, entry_activity, 0x6, 0, 1);

/* entry_partition_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fsdc, entry_partition_id, 0x7, 0, 3);

/* num_entries
 * Access: RO
 */
MLXSW_ITEM32(reg, fsdc, num_entries, 0x8, 0, 20);

/* fsps
 * ----
 *
 */
/* tbm
 * Access: RO
 */
MLXSW_ITEM8(reg, fsps, tbm, 0x2, 1, 1);

/* tbw
 * Access: RO
 */
MLXSW_ITEM8(reg, fsps, tbw, 0x2, 0, 1);

/* partition_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fsps, partition_id, 0x3, 0, 3);

/* warning_threshold
 * Access: RO
 */
MLXSW_ITEM32(reg, fsps, warning_threshold, 0x4, 0, 24);

/* max_threshold
 * Access: RO
 */
MLXSW_ITEM32(reg, fsps, max_threshold, 0x8, 0, 24);

/* occupancy
 * Access: RO
 */
MLXSW_ITEM32(reg, fsps, occupancy, 0xc, 0, 32);

/* fstm
 * ----
 *
 */
/* record_num
 * Access: OP
 */
MLXSW_ITEM16(reg, fstm, record_num, 0x2, 0, 9);

/* ticket_machine_en
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, fstm, ticket_machine_en, MLXSW_FSTM_PORT_TICKET_INFO_BASE_LEN + 2, 7, 1, MLXSW_FSTM_PORT_TICKET_INFO_REC_LEN, 0, 1, 0);

/* lport
 * Access: WO
 */
MLXSW_ITEM16_INDEXED(reg, fstm, lport, MLXSW_FSTM_PORT_TICKET_INFO_BASE_LEN + 2, 0, 10, MLXSW_FSTM_PORT_TICKET_INFO_REC_LEN, 0, 2, 0);

/* fsed
 * ----
 *
 */
/* num_pages
 * Access: RO
 */
MLXSW_ITEM16(reg, fsed, num_pages, 0x0, 0, 9);

/* num_of_entries
 * Access: RO
 */
MLXSW_ITEM16(reg, fsed, num_of_entries, 0x4, 0, 10);

/* mode
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, fsed, mode, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN + 3, 0, 1, MLXSW_FSED_SF_DUMP_PAGE_REC_LEN, 0, 1, 0);

/* sf_page_dump_mode_0_key_54b
 * Access: RO
 */
MLXSW_ITEM_BUF_INDEXED(reg, fsed, sf_page_dump_mode_0_key_54b, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN, 0x38, 0x58, 8, 1, 0);

/* sf_page_dump_mode_0_valid
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, fsed, sf_page_dump_mode_0_valid, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN, 7, 1, MLXSW_FSED_SF_DUMP_PAGE_REC_LEN, 64, 1, 0);

/* sf_page_dump_mode_0_key_size
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, fsed, sf_page_dump_mode_0_key_size, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN, 4, 2, MLXSW_FSED_SF_DUMP_PAGE_REC_LEN, 64, 1, 0);

/* sf_page_dump_mode_0_partition
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, fsed, sf_page_dump_mode_0_partition, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN + 1, 4, 3, MLXSW_FSED_SF_DUMP_PAGE_REC_LEN, 64, 1, 0);

/* sf_page_dump_mode_0_sem_s
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, fsed, sf_page_dump_mode_0_sem_s, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN + 1, 3, 1, MLXSW_FSED_SF_DUMP_PAGE_REC_LEN, 64, 1, 0);

/* sf_page_dump_mode_0_a
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, fsed, sf_page_dump_mode_0_a, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN + 1, 0, 1, MLXSW_FSED_SF_DUMP_PAGE_REC_LEN, 64, 1, 0);

/* sf_page_dump_mode_0_data
 * Access: RO
 */
MLXSW_ITEM64_INDEXED(reg, fsed, sf_page_dump_mode_0_data, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN, 0, 64, MLXSW_FSED_SF_DUMP_PAGE_REC_LEN, 72, 8, 0);

/* sf_page_dump_mode_0_sem_count
 * Access: RO
 */
MLXSW_ITEM16_INDEXED(reg, fsed, sf_page_dump_mode_0_sem_count, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN, 0, 16, MLXSW_FSED_SF_DUMP_PAGE_REC_LEN, 80, 2, 0);

/* sf_page_dump_mode_0_region_id
 * Access: RO
 */
MLXSW_ITEM16_INDEXED(reg, fsed, sf_page_dump_mode_0_region_id, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN + 2, 0, 16, MLXSW_FSED_SF_DUMP_PAGE_REC_LEN, 80, 2, 0);

/* record_key_18b
 * Access: RO
 */
MLXSW_ITEM_BUF_INDEXED(reg, fsed, record_key_18b, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN, 0x14, 0x58, MLXSW_FSED_SF_PAGE_DUMP_MODE_1_RECORD_BASE_LEN, MLXSW_FSED_RECORD_REC_LEN, 0);

/* record_valid
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, fsed, record_valid, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN, 7, 1, MLXSW_FSED_SF_DUMP_PAGE_REC_LEN, MLXSW_FSED_SF_PAGE_DUMP_MODE_1_RECORD_BASE_LEN, MLXSW_FSED_RECORD_REC_LEN, 20);

/* record_key_size
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, fsed, record_key_size, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN, 4, 2, MLXSW_FSED_SF_DUMP_PAGE_REC_LEN, MLXSW_FSED_SF_PAGE_DUMP_MODE_1_RECORD_BASE_LEN, MLXSW_FSED_RECORD_REC_LEN, 20);

/* record_partition
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, fsed, record_partition, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN + 1, 4, 3, MLXSW_FSED_SF_DUMP_PAGE_REC_LEN, MLXSW_FSED_SF_PAGE_DUMP_MODE_1_RECORD_BASE_LEN, MLXSW_FSED_RECORD_REC_LEN, 20);

/* record_sem_s
 * Access: RO
 */
MLXSW_ITEM8_INDEXED(reg, fsed, record_sem_s, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN + 1, 3, 1, MLXSW_FSED_SF_DUMP_PAGE_REC_LEN, MLXSW_FSED_SF_PAGE_DUMP_MODE_1_RECORD_BASE_LEN, MLXSW_FSED_RECORD_REC_LEN, 20);

/* record_a
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, fsed, record_a, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN + 1, 0, 1, MLXSW_FSED_SF_DUMP_PAGE_REC_LEN, MLXSW_FSED_SF_PAGE_DUMP_MODE_1_RECORD_BASE_LEN, MLXSW_FSED_RECORD_REC_LEN, 20);

/* record_data
 * Access: RO
 */
MLXSW_ITEM64_INDEXED(reg, fsed, record_data, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN, 0, 64, MLXSW_FSED_SF_DUMP_PAGE_REC_LEN, MLXSW_FSED_SF_PAGE_DUMP_MODE_1_RECORD_BASE_LEN, MLXSW_FSED_RECORD_REC_LEN, 24);

/* record_sem_count
 * Access: RO
 */
MLXSW_ITEM16_INDEXED(reg, fsed, record_sem_count, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN, 0, 16, MLXSW_FSED_SF_DUMP_PAGE_REC_LEN, MLXSW_FSED_SF_PAGE_DUMP_MODE_1_RECORD_BASE_LEN, MLXSW_FSED_RECORD_REC_LEN, 32);

/* record_region_id
 * Access: RO
 */
MLXSW_ITEM16_INDEXED(reg, fsed, record_region_id, MLXSW_FSED_SF_DUMP_PAGE_BASE_LEN + 2, 0, 16, MLXSW_FSED_SF_DUMP_PAGE_REC_LEN, MLXSW_FSED_SF_PAGE_DUMP_MODE_1_RECORD_BASE_LEN, MLXSW_FSED_RECORD_REC_LEN, 32);

/* fshe
 * ----
 *
 */
/* op
 * Access: OP
 */
MLXSW_ITEM8(reg, fshe, op, 0x3, 0, 2);

/* hw_errors
 * Access: RO
 */
MLXSW_ITEM32(reg, fshe, hw_errors, 0x4, 0, 32);

/* fsfh
 * ----
 *
 */
/* failure_reason
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fsfh, failure_reason, 0x3, 0, 3);

/* action_info
 * Access: RW
 */
MLXSW_ITEM32_INDEXED(reg, fsfh, action_info, 0x4, 0, 32, 4, 0, 4, 0);

/* fmtsc
 * -----
 *
 */
/* enable
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtsc, enable, 0x0, 7, 1);

/* atomic
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtsc, atomic, 0x0, 6, 1);

/* mode
 * Access: WO
 */
MLXSW_ITEM8(reg, fmtsc, mode, 0x0, 0, 3);

/* num_fields_in_ram
 * Access: RW
 */
MLXSW_ITEM8(reg, fmtsc, num_fields_in_ram, 0x2, 0, 4);

/* emt_idx
 * Access: INDEX
 */
MLXSW_ITEM8(reg, fmtsc, emt_idx, 0x3, 0, 3);

/* cnt_type
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, fmtsc, cnt_type, MLXSW_FMTSC_FIELD_INFO_BASE_LEN, 4, 4, MLXSW_FMTSC_FIELD_INFO_REC_LEN, 0, 1, 0);

/* prio_sel
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, fmtsc, prio_sel, MLXSW_FMTSC_FIELD_INFO_BASE_LEN + 1, 0, 4, MLXSW_FMTSC_FIELD_INFO_REC_LEN, 0, 1, 0);

/* prio_val
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, fmtsc, prio_val, MLXSW_FMTSC_FIELD_INFO_BASE_LEN + 3, 0, 6, MLXSW_FMTSC_FIELD_INFO_REC_LEN, 0, 1, 0);

/* msb_or_lsb
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, fmtsc, msb_or_lsb, MLXSW_FMTSC_FIELD_INFO_BASE_LEN, 7, 1, MLXSW_FMTSC_FIELD_INFO_REC_LEN, 4, 1, 0);

/* cnt_idx
 * Access: RW
 */
MLXSW_ITEM8_INDEXED(reg, fmtsc, cnt_idx, MLXSW_FMTSC_FIELD_INFO_BASE_LEN + 3, 0, 8, MLXSW_FMTSC_FIELD_INFO_REC_LEN, 4, 1, 0);

/* port_bitmap_size
 * Access: OP
 */
MLXSW_ITEM8(reg, fmtsc, port_bitmap_size, 0x2f, 0, 8);

/* port_mask
 * Access: INDEX
 */
MLXSW_ITEM32_INDEXED(reg, fmtsc, port_mask, 0x30, 0, 32, 4, 0, 4, 0);

/* fpmr
 * ----
 *
 */
/* en
 * Access: RW
 */
MLXSW_ITEM8(reg, fpmr, en, 0x0, 7, 1);

/* fpp
 * Access: RW
 */
MLXSW_ITEM8(reg, fpmr, fpp, 0x3, 0, 8);

/* ft
 * Access: RW
 */
MLXSW_ITEM8(reg, fpmr, ft, 0x7, 0, 8);

/* fpgcr
 * -----
 *
 */
/* ftt_cur_ph_inner
 * Access: RW
 */
MLXSW_ITEM8(reg, fpgcr, ftt_cur_ph_inner, 0x3, 0, 1);

/* xgcr
 * ----
 *
 */
/* mtm
 * Access: RW
 */
MLXSW_ITEM8(reg, xgcr, mtm, 0x7, 0, 4);

/* xltq
 * ----
 *
 */
/* xm_device_id
 * Access: RO
 */
MLXSW_ITEM16(reg, xltq, xm_device_id, 0x6, 0, 16);

/* xlt_cap_mask0
 * Access: RO
 */
MLXSW_ITEM32(reg, xltq, xlt_cap_mask0, 0x10, 0, 32);

/* cap_xlt_entries
 * Access: RO
 */
MLXSW_ITEM32(reg, xltq, cap_xlt_entries, 0x20, 0, 32);

/* cap_xlt_mtable
 * Access: RO
 */
MLXSW_ITEM32(reg, xltq, cap_xlt_mtable, 0x24, 0, 32);

/* cap_xlt_shspm_max_trees
 * Access: RO
 */
MLXSW_ITEM32(reg, xltq, cap_xlt_shspm_max_trees, 0x28, 0, 32);

/* xmdr
 * ----
 *
 */
/* num_rec
 * Access: OP
 */
MLXSW_ITEM8(reg, xmdr, num_rec, 0x7, 0, 4);

/* reply_vect
 * Access: RO
 */
MLXSW_ITEM8(reg, xmdr, reply_vect, 0xb, 0, 4);

/* transaction
 * Access: WO
 */
MLXSW_ITEM32_INDEXED(reg, xmdr, transaction, 0x20, 0, 32, 4, 0, 4, 0);

/* xlkbu
 * -----
 *
 */
/* data_type
 * Access: OP
 */
MLXSW_ITEM8(reg, xlkbu, data_type, 0x7, 0, 4);

/* value
 * Access: OP
 */
MLXSW_ITEM32_INDEXED(reg, xlkbu, value, 0x40, 0, 32, 4, 0, 4, 0);

/* mask
 * Access: OP
 */
MLXSW_ITEM32_INDEXED(reg, xlkbu, mask, 0x80, 0, 32, 4, 0, 4, 0);

/* new_value
 * Access: OP
 */
MLXSW_ITEM32_INDEXED(reg, xlkbu, new_value, 0xc0, 0, 32, 4, 0, 4, 0);

/* new_value_mask
 * Access: OP
 */
MLXSW_ITEM32_INDEXED(reg, xlkbu, new_value_mask, 0x100, 0, 32, 4, 0, 4, 0);

/* xrmt
 * ----
 *
 */
/* index
 * Access: INDEX
 */
MLXSW_ITEM32(reg, xrmt, index, 0x4, 0, 20);

/* l0_val
 * Access: RW
 */
MLXSW_ITEM8(reg, xrmt, l0_val, 0x10, 0, 8);

/* l0_vec
 * Access: RW
 */
MLXSW_ITEM8(reg, xrmt, l0_vec, 0x11, 0, 8);

/* l1_val
 * Access: RW
 */
MLXSW_ITEM8(reg, xrmt, l1_val, 0x12, 0, 8);

/* l1_vec
 * Access: RW
 */
MLXSW_ITEM8(reg, xrmt, l1_vec, 0x13, 0, 8);

/* xralta
 * ------
 *
 */
/* op
 * Access: OP
 */
MLXSW_ITEM8(reg, xralta, op, 0x4, 4, 2);

/* protocol
 * Access: RW
 */
MLXSW_ITEM8(reg, xralta, protocol, 0x4, 0, 4);

/* tree_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, xralta, tree_id, 0x7, 0, 8);

/* xralst
 * ------
 *
 */
/* root_bin
 * Access: RW
 */
MLXSW_ITEM8(reg, xralst, root_bin, 0x5, 0, 8);

/* tree_id
 * Access: INDEX
 */
MLXSW_ITEM8(reg, xralst, tree_id, 0x7, 0, 8);

/* bin
 * Access: RW
 */
MLXSW_ITEM16_INDEXED(reg, xralst, bin, 0x8, 0, 16, 2, 0, 2, 0);

/* xraltb
 * ------
 *
 */
/* virtual_router
 * Access: INDEX
 */
MLXSW_ITEM16(reg, xraltb, virtual_router, 0x4, 0, 16);

/* protocol
 * Access: INDEX
 */
MLXSW_ITEM8(reg, xraltb, protocol, 0x6, 4, 4);

/* tree_id
 * Access: RW
 */
MLXSW_ITEM8(reg, xraltb, tree_id, 0x7, 0, 8);

