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

#ifndef SX_PSAMPLE_H
#define SX_PSAMPLE_H

#if IS_ENABLED(CONFIG_PSAMPLE)

int sx_psample_set_synd(struct sx_dev       *dev,
                        u16                  synd,
                        enum sx_dev_event    event,
                        union sx_event_data *event_data);
int sx_psample_rate_set(struct ku_psample_sample_rate *psample_sample_rate_p);
void psample_rate_dump(struct seq_file *m);
void sx_psample_init(void);
void sx_psample_cleanup(void);

#if defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
int sx_psample_setup_tc(struct net_device *dev, enum tc_setup_type type, void *type_data);
#endif /* if defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) */

#else /* IS_ENABLED(CONFIG_PSAMPLE) */

int sx_psample_set_synd(struct sx_dev *dev, u16 synd, enum sx_dev_event event, union sx_event_data *event_data)
{
    return 0;
}

int sx_psample_rate_set(struct ku_psample_sample_rate *psample_sample_rate_p)
{
    return 0;
}

void psample_rate_dump(struct seq_file *m)
{
}

void sx_psample_init(void)
{
}

void sx_psample_cleanup(void)
{
}

#if defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
int sx_psample_setup_tc(struct net_device *dev, enum tc_setup_type type, void *type_data)
{
    return 0;
}
#endif /* defined(CONFIG_NET_PORT) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) */

#endif /* IS_ENABLED(CONFIG_PSAMPLE) */

#endif /* SX_PSAMPLE_H */
