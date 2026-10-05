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

#include <linux/hashtable.h>
#include <linux/seq_file.h>
#include <linux/mlx_sx/kernel_user.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include "sx.h"
#include "mmap.h"
#include "dev_db.h"

#define SX_DUMMY_VMA_PTR ((void*)0xdeadbeef)

struct sx_mmap_metadata {
    struct ku_client_pid_get   client_id;
    char                       agent[32]; /* process name */
    void                      *realdata; /* the real allocation for mmap() */
    void                      *data; /* the page-aligned pointer within the real allocation */
    unsigned long              user_ptr; /* user-space pointer of mmap() */
    unsigned long              size; /* size of allocation */
    u64                        usr_size; /* size of allocation from user point of view*/
    u32                        refcnt_usr; /* reference count from userspace */
    u32                        refcnt_driver; /* reference count from driver */
    struct hlist_node          h_entry;
    ku_mmap_mem_alloc_scheme_e mmap_alloc_scheme;  /* memory map allocation scheme (vmalloc or kmalloc) */
    struct sx_rsc             *file; /* the file that created the shared memory */
    mmap_pfn_dtor              pfn_dtor;        /* Destructor for mapped memory */
    u64                        seq_id;
    unsigned long              allocation_ts; /* jiffies */
};

struct sx_mmap_synced_work {
    struct work_struct       work;
    struct completion        comp;
    struct sx_mmap_metadata *metadata;
    sxd_dev_id_t             dev_id;
};

#define SX_MMAP_MT_REFCNT(mt) ((mt)->refcnt_usr + (mt)->refcnt_driver)

/* must be called under lock */
static struct sx_mmap_metadata * __sx_mmap_lookup(struct sx_dev *dev, pid_t gpid, unsigned long user_ptr)
{
    struct sx_priv          *priv = sx_priv(dev);
    struct sx_mmap_metadata *iter;

    hash_for_each_possible(priv->mmap_db.info->hash_tb, iter, h_entry, user_ptr) {
        if ((iter->client_id.gpid == gpid) && (iter->user_ptr == user_ptr)) {
            return iter;
        }
    }

    return NULL;
}

/* must be called within a lock! */
static bool __sx_mmap_ref_dec_metadata(struct sx_mmap_metadata *metadata, bool is_user)
{
    if (is_user) {
        metadata->refcnt_usr--;
    } else {
        metadata->refcnt_driver--;
    }

    if (SX_MMAP_MT_REFCNT(metadata) == 0) {
        hash_del(&metadata->h_entry);
        return true;
    }

    return false;
}

/* must be called OUT OF lock! */
static void __sx_mmap_free_metadata(struct sx_dev *dev, struct sx_mmap_metadata *metadata)
{
    if (metadata->pfn_dtor) {
        metadata->pfn_dtor(dev, metadata->user_ptr, metadata->client_id.gpid, metadata->usr_size);
    }

    if (metadata->mmap_alloc_scheme == KU_MMAP_MEM_ALLOC_SCHEME_VMALLOC) {
        vfree(metadata->realdata);
    } else {
        kfree(metadata->realdata);
    }

    kfree(metadata);
}

static void __sx_vm_open(struct vm_area_struct *vma)
{
    /* this was called because of fork() so we're with a new vma object on the child process.
     * we don't need to track it. however, we need to mark it as 'dummy' so __sx_vm_close()
     * will ignore it too.
     */
    vma->vm_private_data = SX_DUMMY_VMA_PTR;
}

static void __sx_handle_vm_close(struct work_struct *work)
{
    struct sx_mmap_synced_work *mmap_work = container_of(work, struct sx_mmap_synced_work, work);
    struct sx_mmap_metadata    *metadata = mmap_work->metadata;
    bool                        del_entry;
    struct sx_dev              *dev;
    struct sx_priv             *priv;

    down_read(&sx_glb.pci_restart_lock);

    dev = sx_dev_db_get_dev_by_id(mmap_work->dev_id);
    if (!dev) {
        sxd_log_warning(SXD_DEV_NONE, "VMA-CLOSE handler: device %d not found\n", mmap_work->dev_id);
        goto out;
    }

    priv = sx_priv(dev);
    atomic_dec(&metadata->file->mmap_refcnt);

    spin_lock_bh(&priv->mmap_db.lock);
    del_entry = __sx_mmap_ref_dec_metadata(metadata, true);
    spin_unlock_bh(&priv->mmap_db.lock);

    if (del_entry) {
        __sx_mmap_free_metadata(dev, metadata);
    }

out:
    up_read(&sx_glb.pci_restart_lock);
    complete(&mmap_work->comp);
}

static void __sx_vm_close(struct vm_area_struct *vma)
{
    struct sx_mmap_metadata   *metadata = (struct sx_mmap_metadata*)vma->vm_private_data;
    struct sx_mmap_synced_work mmap_work;

    WARN_ON_ONCE(!metadata);
    if (!metadata || (metadata == SX_DUMMY_VMA_PTR)) {
        return;
    }

    /* in debug kernel we have a warning of 'possible circular lock dependency'
     * (two locks that are acquired in reversed order):
     *
     * ioctl():
     *     lock(&pci_restart_lock); // read lock, to synchronize ioctl() handlers with device PCI restart/shutdown
     *     lock(&mm->mmap_lock)     // kernel MM lock (acquired during copy_from_user()/copy_to_user())
     *
     * sx_core_mmap():
     * __sx_vm_open():
     * __sx_vm_close():
     *     lock(&mm->mmap_lock)    // kernel MM lock, these functions are called when this lock is already acquired
     *     lock(&pci_restart_lock) // read lock, to synchronize mmap/munmap with device PCI restart/shutdown
     *
     * we'll use deferred work to overcome this limitation.
     *
     */

    INIT_WORK(&mmap_work.work, __sx_handle_vm_close);
    init_completion(&mmap_work.comp);
    mmap_work.metadata = metadata;
    mmap_work.dev_id = sx_core_get_device_id_from_fd(metadata->file->owner);

    queue_work(sx_glb.global_wq, &mmap_work.work);
    wait_for_completion(&mmap_work.comp);
}

const struct vm_operations_struct __sx_vm_ops = {
    .open = __sx_vm_open,
    .close = __sx_vm_close
};

int sx_core_mmap(struct file *filp, struct vm_area_struct *vma)
{
    unsigned long            pfn, size;
    struct sx_mmap_metadata *metadata = NULL;
    void                    *alloc = NULL;
    int                      ret = 0;
    struct sx_dev           *dev = NULL;
    struct sx_priv          *priv = NULL;
    struct sx_rsc           *rsc = filp->private_data;

    sx_core_get_dev_and_id_from_fd(filp, &dev, NULL);
    if (!dev) {
        ret = -ENODEV;
        goto out;
    }

    priv = sx_priv(dev);

    if (!PAGE_ALIGNED(vma->vm_start) || !PAGE_ALIGNED(vma->vm_end)) {
        sxd_log_err(dev, "mapped area is not aligned with PAGE_SIZE\n");
        ret = -EINVAL;
        goto out;
    }

    size = vma->vm_end - vma->vm_start; /* size is multiple of PAGE_SIZE */
    if (size == 0) {
        sxd_log_err(dev, "size of mapped area is 0\n");
        ret = -EINVAL;
        goto out;
    }

    metadata = kzalloc(sizeof(*metadata), GFP_KERNEL);
    if (!metadata) {
        ret = -ENOMEM;
        goto out;
    }

    /* on debug-kernel, the allocation that is mapped to userspace MUST be aligned to PAGE_SIZE.
     * thus, we're allocating enough space that we can return a page-aligned pointer */
    if (rsc->mmap_alloc_scheme == KU_MMAP_MEM_ALLOC_SCHEME_VMALLOC) {
        metadata->realdata = vmalloc_user(size + PAGE_SIZE);
        if (!metadata->realdata) {
            sxd_log_err(dev, "vmalloc memory allocation failed.\n");
            ret = -ENOMEM;
            goto out;
        }
        /* if allocation succeeded, get the page-aligned pointer within the allocated area */
        alloc = (void*)PAGE_ALIGN((unsigned long)metadata->realdata);
        ret = remap_vmalloc_range(vma, alloc, 0);
        if (ret) {
            sxd_log_err(dev,
                        "vmalloc remap failed error=%d, vma=0x%p, alloc=0x%p, size=%ld, start=%ld, end=%ld\n",
                        ret,
                        vma,
                        alloc,
                        size,
                        vma->vm_start,
                        vma->vm_end);
            goto out;
        }
    } else {
        if (size + PAGE_SIZE > KMALLOC_MAX_SIZE) {
            sxd_log_err(dev,
                        "Size of mapped area %ld exceed max size %ld with kmalloc memory allocation scheme.\n",
                        size + PAGE_SIZE,
                        KMALLOC_MAX_SIZE);
            ret = -EINVAL;
            goto out;
        }

        metadata->realdata = kmalloc(size + PAGE_SIZE, GFP_KERNEL);
        if (!metadata->realdata) {
            sxd_log_err(dev, "kmalloc memory allocation failed.\n");
            ret = -ENOMEM;
            goto out;
        }
        /* if allocation succeeded, get the page-aligned pointer within the allocated area */
        alloc = (void*)PAGE_ALIGN((unsigned long)metadata->realdata);
        pfn = virt_to_phys(alloc) >> PAGE_SHIFT;
        ret = remap_pfn_range(vma,
                              vma->vm_start,
                              pfn,
                              size,
                              vma->vm_page_prot);
        if (ret) {
            sxd_log_err(dev,
                        "kmalloc remap failed error=%d, vma=0x%p, alloc=0x%p, size=%ld, start=%ld, end=%ld\n",
                        ret,
                        vma,
                        alloc,
                        size,
                        vma->vm_start,
                        vma->vm_end);
            goto out;
        }
    }

    sx_current_pid_get(&metadata->client_id);
    strncpy(metadata->agent, current->comm, sizeof(metadata->agent) - 1);
    metadata->refcnt_usr = 1;
    metadata->data = alloc;
    metadata->user_ptr = vma->vm_start;
    metadata->size = size;
    metadata->mmap_alloc_scheme = rsc->mmap_alloc_scheme;
    metadata->file = rsc;
    metadata->allocation_ts = jiffies;
    atomic_inc(&rsc->mmap_refcnt);

    spin_lock_bh(&priv->mmap_db.lock);
    metadata->seq_id = ++priv->mmap_db.info->seq;
    hash_add(priv->mmap_db.info->hash_tb, &metadata->h_entry, metadata->user_ptr);
    spin_unlock_bh(&priv->mmap_db.lock);

    vma->vm_ops = &__sx_vm_ops;
    vma->vm_private_data = metadata;

out:
    if (ret) {
        if (metadata) {
            if (metadata->realdata) {
                if (rsc->mmap_alloc_scheme == KU_MMAP_MEM_ALLOC_SCHEME_VMALLOC) {
                    vfree(metadata->realdata);
                } else {
                    kfree(metadata->realdata);
                }
            }
            kfree(metadata);
        }
    }

    return ret;
}

void* sx_mmap_user_to_kernel(struct sx_dev *dev, pid_t gpid, unsigned long user_ptr, bool get_ref)
{
    struct sx_priv          *priv = sx_priv(dev);
    struct sx_mmap_metadata *iter;
    void                    *ptr = NULL;

    spin_lock_bh(&priv->mmap_db.lock);
    iter = __sx_mmap_lookup(dev, gpid, user_ptr);
    if (iter) {
        if (get_ref) {
            iter->refcnt_driver++;
        }

        ptr = iter->data;
    }
    spin_unlock_bh(&priv->mmap_db.lock);
    return ptr;
}

void sx_mmap_ref_dec(struct sx_dev *dev, pid_t gpid, unsigned long user_ptr)
{
    struct sx_priv          *priv = sx_priv(dev);
    struct sx_mmap_metadata *metadata;
    bool                     del_entry = false;

    spin_lock_bh(&priv->mmap_db.lock);
    metadata = __sx_mmap_lookup(dev, gpid, user_ptr);
    if (metadata) {
        del_entry = __sx_mmap_ref_dec_metadata(metadata, false);
    }
    spin_unlock_bh(&priv->mmap_db.lock);

    if (del_entry) {
        __sx_mmap_free_metadata(dev, metadata);
    }
}

void sx_mmap_set_pfn_dtor(struct sx_dev *dev,
                          unsigned long  user_ptr,
                          pid_t          gpid,
                          uint64_t       usr_size,
                          mmap_pfn_dtor  pfn_dtor)
{
    struct sx_priv          *priv = sx_priv(dev);
    struct sx_mmap_metadata *iter;

    spin_lock_bh(&priv->mmap_db.lock);
    iter = __sx_mmap_lookup(dev, gpid, user_ptr);
    if (iter) {
        iter->pfn_dtor = pfn_dtor;
        iter->usr_size = usr_size;
    }
    spin_unlock_bh(&priv->mmap_db.lock);
}

int sx_mmap_dump(struct seq_file *m, void *v, void *context)
{
    struct sx_dev           *dev;
    struct sx_priv          *priv;
    struct sx_mmap_metadata *iter;
    char                     owner[128] = { 0 };
    int                      bucket;

    dev = sx_dbg_dump_get_device(m);
    if (!dev) {
        return 0;
    }

    priv = sx_priv(dev);

    seq_printf(m, "----------------------------------------------------------------------\n");
    seq_printf(m, "%-60s   %-8s   %-8s   %-16s   %-8s   %-8s   %-8s   %-8s   %s\n",
               "Agent",
               "Seq",
               "FD",
               "user_ptr",
               "kern_ptr",
               "size",
               "ref_user",
               "ref_kern",
               "time (Sec ago)");
    seq_printf(m, "----------------------------------------------------------------------\n");

    spin_lock_bh(&priv->mmap_db.lock);

    hash_for_each(priv->mmap_db.info->hash_tb, bucket, iter, h_entry) {
        snprintf(owner, sizeof(owner) - 1, "%s [" SX_PRINT_FMT_CLIENT_PID "]",
                 iter->agent, SX_PRINT_CLIENT_PID(&iter->client_id));

        seq_printf(m, "%-60s   %-8llu   %-8p   %-16lx   %-8p   %-8lu   %-8u   %-8u   %u\n",
                   owner,
                   iter->seq_id,
                   iter->file->owner,
                   iter->user_ptr,
                   iter->data,
                   iter->size,
                   iter->refcnt_usr, iter->refcnt_driver,
                   jiffies_to_msecs(jiffies - iter->allocation_ts) / 1000);
    }

    spin_unlock_bh(&priv->mmap_db.lock);
    return 0;
}
