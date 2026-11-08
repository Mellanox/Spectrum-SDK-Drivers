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

#include <linux/fs.h>
#include <linux/uaccess.h>

#include <linux/mlx_sx/device.h>
#include <linux/mlx_sx/driver.h>
#include <linux/mlx_sx/kernel_user.h>

#include "sx.h"
#include "dev_db.h"
#include "dev_init.h"
#include "alloc.h"
#include "fw.h"
#include "ioctl_internal.h"

void sx_core_get_dev_and_id_from_fd(struct file *file, struct sx_dev **dev_pp, sxd_dev_id_t *dev_id_p)
{
    sxd_dev_id_t   dev_id;
    struct sx_dev *dev;
    struct sx_rsc *rsc;

    rsc = file ? file->private_data : NULL;

    if (likely(sx_core_has_pci_dev())) {
        /* If the FD is already marked as closing (device removal / restart in progress),
         * return NULL without triggering WARN(). This keeps WARN() meaningful for real bugs
         * while avoiding noisy backtrace during controlled teardown.
         */
        if (rsc && READ_ONCE(rsc->tmp_device_restart_closing)) {
            dev_id = sx_core_get_device_id_from_fd(file);
            dev = NULL;
            goto out;
        }
        dev_id = sx_core_get_device_id_from_fd(file);
        dev = sx_dev_db_get_dev_by_id(dev_id);
    } else { /* OOB / MST / I2C */
        dev_id_p = NULL; /* ignore dev_id_p (caller already has it initialized) */
        dev_id = DEFAULT_DEVICE_ID;
        dev = sx_glb.fake_dev;
    }

    WARN((!dev), "device ID %u not found\n", dev_id);

out:
    if (dev_pp) {
        *dev_pp = dev;
    }

    if (dev_id_p) {
        *dev_id_p = dev_id;
    }
}

int sx_core_ioctl_enable_swid(struct sx_dev *dev, unsigned long data)
{
    struct sx_priv        *priv = NULL;
    int                    err = 0;
    struct ku_swid_details swid_data;

    err = copy_from_user(&swid_data, (void*)data, sizeof(swid_data));
    if (err) {
        goto out;
    }

    priv = sx_priv(dev);

    if (sx_core_fw_is_faulty(dev)) {
        sxd_log_notice(dev, "Enable SWID: Faulty FW - ignoring.\n");
        return 0;
    }

    sxd_log_notice(dev, "Enable SWID %u for device %u\n", swid_data.swid, swid_data.dev_id);
    spin_lock(&priv->profile.profile_lock);
    if (!sx_cr_mode() && ((!priv->profile.pci_profile_set) || (swid_data.swid >= NUMBER_OF_SWIDS)
                          || (priv->profile.pci_profile.swid_type[swid_data.swid] ==
                              SX_KU_L2_TYPE_DONT_CARE) ||
                          sx_bitmap_test(&sx_priv(dev)->swid_bitmap,
                                         swid_data.swid))) {
        spin_unlock(&priv->profile.profile_lock);
        return -EINVAL;
    }

    spin_unlock(&priv->profile.profile_lock);
    err = sx_enable_swid(dev, swid_data.dev_id, swid_data.swid, swid_data.iptrap_synd, swid_data.mac);
    if (err) {
        goto out;
    }

out:
    return err;
}


static bool __is_set_profile_needed(const struct ku_profile           * profile,
                                    const struct profile_driver_params* actual_params,
                                    const struct profile_driver_params* wanted_params)
{
    /* If profile set_mask has only cqe version bit set (bit 32), and the cqe version is the same as the wanted one, there is no need to set profile */
    return !(profile->set_mask_0_63 == 0x100000000 && profile->set_mask_64_127 == 0 &&
             actual_params->cqe_version == wanted_params->cqe_version);
}


int sx_core_ioctl_set_pci_profile(struct sx_dev *dev, unsigned long data, uint8_t set_profile)
{
    struct sx_priv              *priv = NULL;
    int                          err = 0;
    struct sx_pci_profile        pci_profile;
    struct ku_profile            profile, current_profile;
    struct profile_driver_params addition_params, current_params;

    err = copy_from_user((void*)&pci_profile,
                         (void*)data, sizeof(pci_profile));
    if (err) {
        err = -ENOMEM;
        goto out;
    }

    priv = sx_priv(dev);

    if (sx_core_fw_is_faulty(dev)) {
        sxd_log_notice(dev, "Set PCI Profile: Faulty FW - ignoring.\n");
        return 0;
    }

    memcpy(&priv->profile.pci_profile, &pci_profile, sizeof(priv->profile.pci_profile));

    sxd_log_notice(dev, "Setting PCI profile for device %u\n", pci_profile.dev_id);
    spin_lock(&priv->profile.profile_lock);
    if (priv->profile.pci_profile_set) {
        sxd_log_warning(dev, "Err: cannot set profile twice\n");
        spin_unlock(&priv->profile.profile_lock);
        err = -EINVAL;
        goto out;
    }

    priv->profile.pci_profile_set = true;
    priv->dev_info.dev_info_ro.flags |= SX_DEV_INFO_F_PCI_PROFILE_SET;
    spin_unlock(&priv->profile.profile_lock);

    memset(&profile, 0, sizeof(profile));
    memset(&addition_params, 0, sizeof(addition_params));
    memset(&current_profile, 0, sizeof(current_profile));
    memset(&current_params, 0, sizeof(current_params));
    profile.dev_id = dev->device_id;
    current_profile.dev_id = dev->device_id;

    err = __sx_core_dev_specific_cb_get_reference(dev);
    if (err) {
        sxd_log_err(dev, "__sx_core_dev_specific_cb_get_reference failed.\n");
        goto out;
    }

    if (sx_priv(dev)->dev_specific_cb.sx_set_device_profile_update_cb != NULL) {
        sx_priv(dev)->dev_specific_cb.sx_set_device_profile_update_cb(&profile, &addition_params);
    }

    __sx_core_dev_specific_cb_release_reference(dev);

    if (set_profile) {
        err = sx_GET_PROFILE(dev, &current_profile, &current_params);
        if (err) {
            sxd_log_err(dev, "Err: fail to get current profile\n");
            goto out;
        }
        if (__is_set_profile_needed(&profile, &current_params, &addition_params)) {
            sxd_log_debug(dev, "ioctl set pci profile: set cqe version (%u)\n",
                          addition_params.cqe_version);
            err = sx_SET_PROFILE(dev, &profile, &addition_params);
            if (err) {
                sxd_log_warning(dev,
                                "Err: fail to set profile cqe version (%u)\n",
                                addition_params.cqe_version);
                goto out;
            }
        }
    }

    err = sx_handle_set_profile(dev);
    if (err) {
        spin_lock(&priv->profile.profile_lock);
        priv->profile.pci_profile_set = false;
        priv->dev_info.dev_info_ro.flags &= ~SX_DEV_INFO_F_PCI_PROFILE_SET;
        spin_unlock(&priv->profile.profile_lock);
        goto out;
    }

    err = copy_to_user((void*)data, (void*)&priv->profile.pci_profile,
                       sizeof(priv->profile.pci_profile));
    if (err) {
        err = -ENOMEM;
        goto out;
    }

out:
    return err;
}


int sx_core_ioctl_get_pci_profile(struct sx_dev *dev, unsigned long data)
{
    struct sx_priv            *priv = sx_priv(dev);
    int                        err = 0;
    struct ku_get_pci_profile *pci_prof = NULL;

    spin_lock(&priv->profile.profile_lock);
    if (!priv->profile.pci_profile_set) {
        sxd_log_warning(dev, "Err: profile is not set\n");
        spin_unlock(&priv->profile.profile_lock);
        err = -EINVAL;
        goto out;
    }

    pci_prof = kmalloc(sizeof(*pci_prof), GFP_ATOMIC);
    if (pci_prof == NULL) {
        sxd_log_err(dev, "Err: alloc of pci_prof is failed\n");
        spin_unlock(&priv->profile.profile_lock);
        err = -ENOMEM;
        goto out;
    }

    pci_prof->pci_profile = priv->profile.pci_profile.pci_profile;
    pci_prof->profile = priv->profile.pci_profile;
    spin_unlock(&priv->profile.profile_lock);
    err = copy_to_user((struct ku_get_pci_profile *)data,
                       pci_prof, sizeof(*pci_prof));

    if (pci_prof != NULL) {
        kfree(pci_prof);
    }

    if (err) {
        err = -ENOMEM;
        goto out;
    }

out:
    return err;
}


int sx_core_add_synd_l2(u8                                swid,
                        u16                               hw_synd,
                        struct sx_dev                    *dev,
                        const struct ku_port_vlan_params *port_vlan_params,
                        u8                                is_register)
{
    union sx_event_data *event_data;

    event_data = kzalloc(sizeof(union sx_event_data), GFP_KERNEL);
    if (!event_data) {
        return -ENOMEM;
    }

    event_data->eth_l3_synd.swid = swid;
    event_data->eth_l3_synd.hw_synd = hw_synd;
    event_data->eth_l3_synd.is_register = is_register;
    switch (port_vlan_params->port_vlan_type) {
    case KU_PORT_VLAN_PARAMS_TYPE_GLOBAL:
        event_data->eth_l3_synd.type = L3_SYND_TYPE_GLOBAL;
        break;

    case KU_PORT_VLAN_PARAMS_TYPE_PORT:
        event_data->eth_l3_synd.type = L3_SYND_TYPE_PORT;
        event_data->eth_l3_synd.port = port_vlan_params->sysport;
        break;

    case KU_PORT_VLAN_PARAMS_TYPE_LAG:
        event_data->eth_l3_synd.type = L3_SYND_TYPE_LAG;
        event_data->eth_l3_synd.port = port_vlan_params->lag_id;
        break;

    case KU_PORT_VLAN_PARAMS_TYPE_VLAN:
        event_data->eth_l3_synd.type = L3_SYND_TYPE_VLAN;
        event_data->eth_l3_synd.vlan = port_vlan_params->vlan;
        break;

    default:
        break;
    }

    sx_core_dispatch_event(dev, SX_DEV_EVENT_ADD_SYND_L2_NETDEV, event_data);
    kfree(event_data);

    return 0;
}


int sx_core_add_synd_l3(u8                                swid,
                        u16                               hw_synd,
                        struct sx_dev                    *dev,
                        const struct ku_port_vlan_params *port_vlan_params,
                        u8                                is_register)
{
    union sx_event_data *event_data;

    event_data = kzalloc(sizeof(union sx_event_data), GFP_KERNEL);
    if (!event_data) {
        return -ENOMEM;
    }

    event_data->eth_l3_synd.swid = swid;
    event_data->eth_l3_synd.hw_synd = hw_synd;
    event_data->eth_l3_synd.is_register = is_register;
    switch (port_vlan_params->port_vlan_type) {
    case KU_PORT_VLAN_PARAMS_TYPE_GLOBAL:
        event_data->eth_l3_synd.type = L3_SYND_TYPE_GLOBAL;
        break;

    case KU_PORT_VLAN_PARAMS_TYPE_PORT:
        event_data->eth_l3_synd.type = L3_SYND_TYPE_PORT;
        event_data->eth_l3_synd.port = port_vlan_params->sysport;
        break;

    case KU_PORT_VLAN_PARAMS_TYPE_LAG:
        event_data->eth_l3_synd.type = L3_SYND_TYPE_LAG;
        event_data->eth_l3_synd.port = port_vlan_params->lag_id;
        break;

    case KU_PORT_VLAN_PARAMS_TYPE_VLAN:
        event_data->eth_l3_synd.type = L3_SYND_TYPE_VLAN;
        event_data->eth_l3_synd.vlan = port_vlan_params->vlan;
        break;

    default:
        break;
    }

    sx_core_dispatch_event(dev, SX_DEV_EVENT_ADD_SYND_NETDEV, event_data);
    kfree(event_data);

    return 0;
}


int sx_core_add_synd_phy(u8                                swid,
                         u16                               hw_synd,
                         struct sx_dev                    *dev,
                         const struct ku_port_vlan_params *port_vlan_params,
                         u8                                is_register)
{
    union sx_event_data *event_data;

    event_data = kzalloc(sizeof(union sx_event_data), GFP_KERNEL);
    if (!event_data) {
        return -ENOMEM;
    }

    event_data->eth_l3_synd.swid = swid;
    event_data->eth_l3_synd.hw_synd = hw_synd;
    event_data->eth_l3_synd.is_register = is_register;
    switch (port_vlan_params->port_vlan_type) {
    case KU_PORT_VLAN_PARAMS_TYPE_GLOBAL:
        event_data->eth_l3_synd.type = L3_SYND_TYPE_GLOBAL;
        break;

    case KU_PORT_VLAN_PARAMS_TYPE_PORT:
        event_data->eth_l3_synd.type = L3_SYND_TYPE_PORT;
        event_data->eth_l3_synd.port = port_vlan_params->sysport;
        break;

    case KU_PORT_VLAN_PARAMS_TYPE_LAG:
        event_data->eth_l3_synd.type = L3_SYND_TYPE_LAG;
        event_data->eth_l3_synd.port = port_vlan_params->lag_id;
        break;

    case KU_PORT_VLAN_PARAMS_TYPE_VLAN:
        event_data->eth_l3_synd.type = L3_SYND_TYPE_VLAN;
        event_data->eth_l3_synd.vlan = port_vlan_params->vlan;
        break;

    default:
        break;
    }
    sx_core_dispatch_event(dev, SX_DEV_EVENT_ADD_SYND_PHY_NETDEV, event_data);
    kfree(event_data);

    return 0;
}

int sx_core_add_synd_user_netdev(u16 hw_synd, struct sx_dev                    *dev, u8 is_register)
{
    union sx_event_data *event_data;

    event_data = kzalloc(sizeof(union sx_event_data), GFP_KERNEL);
    if (!event_data) {
        return -ENOMEM;
    }

    event_data->eth_user_netdev_synd.hw_synd = hw_synd;
    event_data->eth_user_netdev_synd.is_register = is_register;

    sx_core_dispatch_event(dev, SX_DEV_EVENT_ADD_SYND_USER_NETDEV, event_data);
    kfree(event_data);

    return 0;
}
