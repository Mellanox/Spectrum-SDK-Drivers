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
#include <linux/slab.h>
#include <linux/pci.h>
#include <linux/seq_file.h>
#include <linux/rwsem.h>
#include <linux/rwlock_types.h>
#include <linux/rwlock.h>
#include "sx.h"
#include "dev_db.h"
#include "dev_init.h"

static int   __enumerated_pci_devices_count = 0;
static char *predefined_dev_pci_bus[SX_MAX_PCI_DEVICES] = { 0 };
static int   predefined_num_of_pci_devices = 0;
module_param_array(predefined_dev_pci_bus, charp, &predefined_num_of_pci_devices,  0444);

struct sx_dev_db_entry {
    struct list_head dev_list;  /* list of valid devices */
    struct pci_dev  *pdev;  /* always valid, even when device is in PCI-restart */
    struct sx_dev   *dev;
    const char      *bus_name;
};

/* array size is MAX_NUM_OF_REMOTE_SWITCHES because device ID might be allocated from
 * __allocate_dev_id_legacy() and it may hold DEFAULT_DEVICE_ID in non-PCI systems */
static struct sx_dev_db_entry __dev_db[MAX_NUM_OF_REMOTE_SWITCHES];
static LIST_HEAD(__valid_devices_list);
static u32 __num_of_valid_devices = 0;

/* Device database functions are called from several places: interrupt, user and kthread contexts.
 * Database updates are ONLY from user/kthread contexts. Readers, on the other hand, are from several contexts.
 *
 * Therefore we need two locks: RW spin-lock (fast-path) and RW semaphore (slow-path).
 * When updating the database, both locks are acquired.
 * When reading from the database, only the relevant lock will be called:
 * If it is a reader that can run in interrupt context, the function will use the fast-path lock.
 * If the reader is only from user/kthread context, the function will use the slow-path lock.
 */
static rwlock_t            __dev_db_fastpath_lock;
static struct rw_semaphore __dev_db_slowpath_lock;

#define DUMMY_BUS_NAME "<none>"

static void __write_lock(void)
{
    /* read the comment at the top of the file */

    /*
     * 1. database updater call this function to lock the database.
     * 2. updater MUST NOT be in interrupt context.
     * 3. lock order MUST be: acquire slow-path and then fast-path.
     **/
    down_write(&__dev_db_slowpath_lock);
    write_lock_bh(&__dev_db_fastpath_lock);
}

static void __write_unlock(void)
{
    /* read the comment at the top of the file */

    /*
     * 1. database updater call this function to unlock the database.
     * 2. updater MUST NOT be in interrupt context.
     * 3. unlock order MUST be: release fast-path and then slow-path.
     **/
    write_unlock_bh(&__dev_db_fastpath_lock);
    up_write(&__dev_db_slowpath_lock);
}

static void __read_lock_fastpath(void)
{
    /* read the comment at the top of the file */

    /*
     * 1. database reader call this function before reading from the database.
     * 2. reader can run from any context: interrupt, user, kthread.
     **/
    read_lock_bh(&__dev_db_fastpath_lock);
}

static void __read_unlock_fastpath(void)
{
    /* read the comment at the top of the file */

    /*
     * 1. database reader call this function after reading from the database.
     * 2. reader can run from any context: interrupt, user, kthread.
     **/
    read_unlock_bh(&__dev_db_fastpath_lock);
}

static void __read_lock_slowpath(void)
{
    /* read the comment at the top of the file */

    /*
     * 1. database reader call this function before reading from the database.
     * 2. reader MUST NOT run from interrupt context.
     **/
    down_read(&__dev_db_slowpath_lock);
}

static void __read_unlock_slowpath(void)
{
    /* read the comment at the top of the file */

    /*
     * 1. database reader call this function after reading from the database.
     * 2. reader MUST NOT run from interrupt context.
     **/
    up_read(&__dev_db_slowpath_lock);
}

static int __sx_dev_db_map_dev_id_to_bus(sxd_dev_id_t dev_id, const char *bus_name, struct pci_dev *pdev)
{
    int err = 0;

    /* this function MUST NOT be called from interrupt context */

    __write_lock();

    if (__dev_db[dev_id].bus_name) {
        sxd_log_err("Device_ID-to-Bus: Device %u has already bound to a bus\n", dev_id);
        err = -EEXIST;
        goto out;
    }

    __dev_db[dev_id].bus_name = kstrdup(bus_name, GFP_ATOMIC);
    if (!__dev_db[dev_id].bus_name) {
        sxd_log_err("Device_ID-to-Bus: failed to allocate bus name for device %d\n", dev_id);
        err = -ENOMEM;
        goto out;
    }

    __dev_db[dev_id].pdev = pdev;
    sxd_log_info("Device_ID-to-Bus: %u ==> %s\n", dev_id, bus_name);

out:
    __write_unlock();
    return err;
}

static int __sx_dev_db_assign_predefined_devices(const struct sx_pci_device_info *enumerated_pci_devices)
{
    char dev_bus[128];
    int  dev_id;
    int  i, rc, err = 0;

    if (predefined_num_of_pci_devices == 0) {
        if (sx_core_num_pci_devices() > 1) {
            sxd_log_err("more than one PCI device on bus but there is no mapping between device IDs and bus names\n");
            return -EFAULT;
        }

        /* for a single device, we map the bus of our single device with dev_id=1 */
        err = __sx_dev_db_map_dev_id_to_bus((sxd_dev_id_t)SX_SINGLE_ASIC_DEFAULT_DEV_ID,
                                            enumerated_pci_devices[0].bus_name, enumerated_pci_devices[0].pdev);
        if (err) {
            sxd_log_err("Predefined device: cannot bind single device ID to bus\n");
            goto out;
        }

        goto out;
    }

    sxd_log_info("Number of predefined PCI devices (Bus-to-Device_ID) is %d\n", predefined_num_of_pci_devices);

    for (i = 0; i < predefined_num_of_pci_devices; i++) {
        if (strlen(predefined_dev_pci_bus[i]) >= sizeof(dev_bus)) {
            sxd_log_err("Predefined device: name is too long on iteration %d\n", i);
            err = -EINVAL;
            goto out;
        }

        rc = sscanf(predefined_dev_pci_bus[i], "%d@%s", &dev_id, dev_bus);
        if (rc != 2) {
            sxd_log_err("Predefined device: name is invalid on iteration %d\n", i);
            err = -EINVAL;
            goto out;
        }

        if (!DEV_ID_IS_VALID(dev_id)) {
            sxd_log_err("Device_ID-to-Bus: invalid device ID (dev_id=%d)\n", dev_id);
            err = -EINVAL;
            goto out;
        }

        err = __sx_dev_db_map_dev_id_to_bus((sxd_dev_id_t)dev_id, dev_bus, enumerated_pci_devices[i].pdev);
        if (err) {
            sxd_log_err("Predefined device: cannot bind device ID to bus on iteration %d\n", i);
            goto out;
        }
    }

out:
    return err;
}

int dev_db_init(void)
{
    struct sx_pci_device_info enumerated_pci_devices[SX_MAX_PCI_DEVICES];
    int                       i, err = 0;

    rwlock_init(&__dev_db_fastpath_lock);
    init_rwsem(&__dev_db_slowpath_lock);

    __enumerated_pci_devices_count = sx_pci_scan(enumerated_pci_devices, ARRAY_SIZE(enumerated_pci_devices));
    if (__enumerated_pci_devices_count < 0) {
        return __enumerated_pci_devices_count;
    }

    for (i = 0; i < __enumerated_pci_devices_count; i++) {
        sxd_log_notice("Found device on PCI [%s, %d/0x%x]\n",
                       enumerated_pci_devices[i].bus_name,
                       enumerated_pci_devices[i].device_id,
                       enumerated_pci_devices[i].device_id);
    }

    err = __sx_dev_db_assign_predefined_devices(enumerated_pci_devices);
    if (err) {
        return err;
    }

    return __enumerated_pci_devices_count;
}

void dev_db_deinit(void)
{
    int i;

    /* this function MUST NOT be called from interrupt context */

    __write_lock();

    for (i = 0; i < MAX_NUM_OF_REMOTE_SWITCHES; i++) {
        if (__dev_db[i].bus_name) {
            kfree(__dev_db[i].bus_name);
            __dev_db[i].bus_name = NULL;
        }
    }

    __write_unlock();
}

static int __allocate_dev_id_legacy(sxd_dev_id_t *dev_id)
{
    int i;

    for (i = DEFAULT_DEVICE_ID; i >= 0; i--) {
        if (DEV_ID_IS_VALID(i) && !__dev_db[i].dev) {
            *dev_id = (sxd_dev_id_t)i;
            return 0;
        }
    }

    sxd_log_err("Add device to database: failed to allocate device ID (legacy mode)\n");
    return -ENOENT;
}

static int __allocate_dev_id(const char *bus_name, sxd_dev_id_t *dev_id)
{
    int i;

    for (i = 0; i < MAX_NUM_OF_REMOTE_SWITCHES; i++) {
        if (DEV_ID_IS_VALID(i) && __dev_db[i].bus_name && strstr(bus_name, __dev_db[i].bus_name)) {
            if (__dev_db[i].dev) {
                sxd_log_err("Add device to database: ID %d already bound to another device\n", i);
                return -EEXIST;
            }

            *dev_id = (sxd_dev_id_t)i;
            return 0;
        }
    }

    sxd_log_err("Add device to database: device's bus '%s' is not associated with ID\n", bus_name);
    return -ENODEV;
}

int sx_dev_db_pdev_to_dev_id(struct pci_dev *pdev, sxd_dev_id_t *dev_id)
{
    int i, err = -ENODEV;

    __read_lock_slowpath();
    for (i = 0; i < MAX_NUM_OF_REMOTE_SWITCHES; i++) {
        if (DEV_ID_IS_VALID(i) && (__dev_db[i].pdev == pdev)) {
            *dev_id = (sxd_dev_id_t)i;
            err = 0;
            break;
        }
    }
    __read_unlock_slowpath();

    return err;
}

int sx_dev_db_dev_id_to_pdev(sxd_dev_id_t dev_id, struct pci_dev **pdev)
{
    int err = 0;

    __read_lock_slowpath();
    if (DEV_ID_IS_VALID(dev_id)) {
        if (__dev_db[dev_id].pdev) {
            *pdev = __dev_db[dev_id].pdev;
        } else {
            err = -ENODEV;
        }
    } else {
        err = -EINVAL;
    }
    __read_unlock_slowpath();

    return err;
}

int sx_core_num_pci_devices(void)
{
    return __enumerated_pci_devices_count;
}
EXPORT_SYMBOL(sx_core_num_pci_devices);

int sx_dev_db_add_device(struct sx_dev *dev)
{
    const char  *bus_name = DUMMY_BUS_NAME;
    sxd_dev_id_t dev_id;
    int          err = 0;

    /* this function MUST NOT be called from interrupt context */

    if (!dev) {
        sxd_log_err("Add device to database: invalid device\n");
        return -EINVAL;
    }

    /* not always a PCI device is attached to the SX device:
     * On OOB system no PCI device is found on the bus so we create a non-PCI device to work with.
     *
     * In case a PCI is attached to the SX device, take the bus name from it.
     */
    if (dev->pdev) {
        bus_name = dev_name(&dev->pdev->dev);
    }

    __write_lock();

    if (sx_core_has_pci_dev()) {
        if (!dev->pdev) {
            sxd_log_err("Add device to database: there are predefined bus names but device is not on PCI\n");
            err = -EINVAL;
            goto out;
        }

        err = __allocate_dev_id(bus_name, &dev_id);
    } else { /* legacy mode, single device that is also referred as the default device */
        err = __allocate_dev_id_legacy(&dev_id);
    }

    if (err) {
        sxd_log_err("failed to allocate an ID for device\n");
        goto out;
    }

    INIT_LIST_HEAD(&__dev_db[dev_id].dev_list);
    list_add_tail(&__dev_db[dev_id].dev_list, &__valid_devices_list);
    __num_of_valid_devices++;
    __dev_db[dev_id].dev = dev;
    dev->device_id = dev_id;

    sxd_log_info("Add device to database: [id=%u, bus=%s]\n", dev_id, bus_name);

out:
    __write_unlock();
    return err;
}

int sx_dev_db_remove_device(struct sx_dev *dev)
{
    const char *bus_name = DUMMY_BUS_NAME;
    int         err = 0;

    /* this function MUST NOT be called from interrupt context */

    if (!dev) {
        sxd_log_err("Remove device from database: invalid device\n");
        return -EINVAL;
    }

    /* coverity[result_independent_of_operands] */
    if (!DEV_ID_IS_VALID(dev->device_id)) {
        sxd_log_err("Remove device from database: invalid device ID (id=%u)\n", dev->device_id);
        return -EINVAL;
    }

    if (dev->pdev) {
        bus_name = dev_name(&dev->pdev->dev);
    }

    __write_lock();

    if (!__dev_db[dev->device_id].dev) {
        __write_unlock();
        sxd_log_err("Remove device from database: device not found\n");
        err = -ENODEV;
        goto out;
    }

    if (__dev_db[dev->device_id].dev == __dev_db[DEFAULT_DEVICE_ID].dev) {
        sxd_log_notice("Removing the default device\n");
        __dev_db[DEFAULT_DEVICE_ID].dev = NULL;
    }

    list_del(&__dev_db[dev->device_id].dev_list);
    __num_of_valid_devices--;

    /* at this point, the device is out of the database which means that any caller that
     * will try lookup for the device will get a NULL pointer. however, we cannot return
     * from this function until there are no references to the device because the caller
     * of this function wants to delete the device. this is the time to release the
     * fastpath lock and then wait until the last reference is released.
     * */
    write_unlock_bh(&__dev_db_fastpath_lock);

    /* only now, where we're sure there are no references to the device, we can assign it to NULL */
    __dev_db[dev->device_id].dev = NULL;

    sxd_log_info("Remove device from database: [id=%u, bus=%s]\n", dev->device_id, bus_name);
    up_write(&__dev_db_slowpath_lock); /* release the slowpath write lock */

out:
    return err;
}

/* this function is used only for legacy (when sx_core_has_pci_multi_dev() == false):
 * we take the only device we have (that has the DEFAULT_DEVICE_ID) and assign it
 * an additional device ID.
 */
int sx_dev_db_reassign_dev_id_to_default_device(sxd_dev_id_t dev_id)
{
    int err = 0;

    /* we should not get here when using PCI */
    if (sx_core_has_pci_dev()) {
        WARN(1, "reassign device IDs is not allowed on PCI systems");
        return 0;
    }

    /* this function MUST NOT be called from interrupt context */

    if (DEFAULT_DEVICE_ID_CHECK(dev_id)) {
        /* two cases when we just return and do nothing:
         * 1. sx_core_has_pci_multi_dev() is true ==> There is no default device anymore.
         * 2. OOB backbone (SwitchX) PCI probe reached here with device id 254, we don't want it to be the default.
         */

        return 0;
    }

    /* coverity[result_independent_of_operands] */
    if (!DEV_ID_IS_VALID(dev_id)) {
        sxd_log_err("device ID %u is no valid\n", dev_id);
        return -EINVAL;
    }

    __write_lock();

    if (!__dev_db[DEFAULT_DEVICE_ID].dev) {
        sxd_log_err("There is no default device yet\n");
        err = -ENODEV;
        goto out;
    }

    if (__dev_db[dev_id].dev) {
        if (__dev_db[dev_id].dev != __dev_db[DEFAULT_DEVICE_ID].dev) {
            sxd_log_err("device ID %u is already assigned\n", dev_id);
            err = -EEXIST;
        }
        /* else: already assigned to this device, just ignore and go out gracefully */

        goto out;
    }

    __dev_db[dev_id].dev = __dev_db[DEFAULT_DEVICE_ID].dev;
    __dev_db[dev_id].dev->device_id = dev_id;
    list_del(&__dev_db[DEFAULT_DEVICE_ID].dev_list);
    list_add_tail(&__dev_db[dev_id].dev_list, &__valid_devices_list);

    sxd_log_info("Default device has a new device ID: %u\n", dev_id);

out:
    __write_unlock();
    return err;
}

struct sx_dev * sx_dev_db_get_dev_by_id(sxd_dev_id_t dev_id)
{
    struct sx_dev *dev = NULL;

    /* this function can run from any context: interrupt, user, kthread */

    /* coverity[result_independent_of_operands] */
    if (!DEV_ID_IS_VALID(dev_id)) {
        sxd_log_err("cannot find device %d - invalid device ID\n", (int)dev_id);
        return NULL;
    }

    if (likely(sx_core_has_pci_dev())) {
        /* no default device when driver is loaded with predefined PCI buses (multi ASICs PCI) */
        WARN_ON_ONCE(DEFAULT_DEVICE_ID_CHECK(dev_id));
    } else {
        dev_id = DEFAULT_DEVICE_ID;
    }

    __read_lock_fastpath();
    dev = __dev_db[dev_id].dev;
    __read_unlock_fastpath();

    return dev;
}

struct sx_dev * sx_dev_db_get_default_device(void)
{
    if (sx_core_has_pci_dev()) {
        sxd_log_err("there is no default device in PCI system\n");
        dump_stack();
        return NULL;
    }

    return sx_dev_db_get_dev_by_id(DEFAULT_DEVICE_ID);
}

int sx_dev_db_dump(struct seq_file *m, void *v, void *context)
{
    const char    *bus_name;
    struct sx_dev *dev;
    int            i;

    /* this function MUST NOT be called from interrupt context */

    seq_printf(m, "Device Database\n");
    seq_printf(m, "-------------------------------------------------------------------------------\n\n");
    seq_printf(m, "Predefined device IDs: %s\n\n", (sx_core_has_pci_multi_dev() ? "yes" : "no"));

    if (sx_core_has_pci_multi_dev()) {
        seq_printf(m, "%-3s   %-16s   %-16s   %-6s   %s\n", "ID", "Bus-Name", "Chip-Type", "GA", "MMAM-Supported");
    } else {
        seq_printf(m, "%-3s   %-16s   %s\n", "ID", "Bus-Name", "Chip-Type");
    }
    seq_printf(m, "...............................................................................\n");

    __read_lock_slowpath();

    for (i = 0; i < MAX_NUM_OF_REMOTE_SWITCHES; i++) {
        if (!DEV_ID_IS_VALID(i) || !__dev_db[i].dev) {
            continue;
        }

        dev = __dev_db[i].dev;
        if (dev->pdev) {
            bus_name = dev_name(&dev->pdev->dev);
        } else {
            bus_name = DUMMY_BUS_NAME;
        }

        if (sx_core_has_pci_multi_dev()) {
            seq_printf(m, "%-3d   %-16s   %-16s   %-6s   %s\n", i, bus_name,
                       sx_get_chip_type_str(sx_priv(dev)->dev_info.dev_info_ro.chip_type),
                       sx_get_ga_asic_str(sx_priv(dev)->dev_info.dev_info_ro.mgir.hw_info.ga),
                       MULTI_ASIC_MODULE_SYSFS_IS_SUPPORT(sx_priv(dev)) ? "yes" : "no");
        } else {
            seq_printf(m, "%-3d   %-16s   %s\n", i, bus_name,
                       sx_get_chip_type_str(sx_priv(dev)->dev_info.dev_info_ro.chip_type));
        }
    }
    __read_unlock_slowpath();

    seq_printf(m, "\n\n");
    return 0;
}
