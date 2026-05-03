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

#define SX_DUMMY_VMA_PTR ((void*)0xdeadbeef)

struct sx_mmap_metadata {
    pid_t                      pid; /* the PID of the mmap() caller */
    void                      *realdata; /* the real allocation for mmap() */
    void                      *data; /* the page-aligned pointer within the real allocation */
    unsigned long              user_ptr; /* user-space pointer of mmap() */
    unsigned long              size; /* size of allocation */
    uint64_t                   usr_size; /* size of allocation from user point of view*/
    uint32_t                   refcnt; /* reference count for the mmap() allocation */
    struct hlist_node          h_entry;
    ku_mmap_mem_alloc_scheme_e mmap_alloc_scheme;  /* memory map allocation scheme (vmalloc or kmalloc) */
    struct sx_rsc             *file; /* the file that created the shared memory */
    struct sx_dev             *dev; /* the device that is associated with the shared memory */
    mmap_pfn_dtor              pfn_dtor;        /* Destructor for mapped memory */
    struct vm_area_struct     *vma;
};

/* must be called under lock */
static struct sx_mmap_metadata * __sx_mmap_lookup(struct sx_dev *dev, pid_t pid, unsigned long user_ptr)
{
    struct sx_priv          *priv = sx_priv(dev);
    struct sx_mmap_metadata *iter;

    hash_for_each_possible(priv->mmap_db.hash_tb, iter, h_entry, user_ptr) {
        if ((iter->pid == pid) && (iter->user_ptr == user_ptr)) {
            return iter;
        }
    }

    return NULL;
}

static void __sx_vm_open(struct vm_area_struct *vma)
{
    /* this was called because of fork() so we're with a new vma object on the child process.
     * we don't need to track it. however, we need to mark it as 'dummy' so __sx_vm_close()
     * will ignore it too.
     */
    vma->vm_private_data = SX_DUMMY_VMA_PTR;
}

static void __sx_vm_close(struct vm_area_struct *vma)
{
    struct sx_mmap_metadata *metadata = (struct sx_mmap_metadata*)vma->vm_private_data;

    if (!metadata || (metadata == SX_DUMMY_VMA_PTR)) {
        return;
    }

    atomic_dec(&metadata->file->mmap_refcnt);
    sx_mmap_ref_dec(metadata->dev, metadata->pid, metadata->user_ptr);
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

    metadata->pid = current->tgid;
    metadata->refcnt = 1;
    metadata->data = alloc;
    metadata->user_ptr = vma->vm_start;
    metadata->vma = vma;
    metadata->size = size;
    metadata->mmap_alloc_scheme = rsc->mmap_alloc_scheme;
    metadata->file = rsc;
    metadata->dev = dev;
    atomic_inc(&rsc->mmap_refcnt);

    spin_lock_bh(&priv->mmap_db.lock);
    hash_add(priv->mmap_db.hash_tb, &metadata->h_entry, metadata->user_ptr);
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

void* sx_mmap_user_to_kernel(struct sx_dev *dev, pid_t pid, unsigned long user_ptr, bool get_ref)
{
    struct sx_priv          *priv = sx_priv(dev);
    struct sx_mmap_metadata *iter;
    void                    *ptr = NULL;

    spin_lock_bh(&priv->mmap_db.lock);
    iter = __sx_mmap_lookup(dev, pid, user_ptr);
    if (iter) {
        if (get_ref) {
            iter->refcnt++;
        }

        ptr = iter->data;
    }
    spin_unlock_bh(&priv->mmap_db.lock);
    return ptr;
}

void sx_mmap_ref_inc(struct sx_dev *dev, pid_t pid, unsigned long user_ptr)
{
    struct sx_priv          *priv = sx_priv(dev);
    struct sx_mmap_metadata *iter;

    spin_lock_bh(&priv->mmap_db.lock);
    iter = __sx_mmap_lookup(dev, pid, user_ptr);
    if (iter) {
        iter->refcnt++;
    }
    spin_unlock_bh(&priv->mmap_db.lock);
}

void sx_mmap_ref_dec(struct sx_dev *dev, pid_t pid, unsigned long user_ptr)
{
    struct sx_priv          *priv = sx_priv(dev);
    struct sx_mmap_metadata *iter;
    bool                     del_entry = false;

    spin_lock_bh(&priv->mmap_db.lock);
    iter = __sx_mmap_lookup(dev, pid, user_ptr);
    if (iter && (--iter->refcnt == 0)) {
        hash_del(&iter->h_entry);
        iter->vma->vm_private_data = NULL;
        del_entry = true;
    }
    spin_unlock_bh(&priv->mmap_db.lock);

    if (del_entry) {
        if (iter->pfn_dtor) {
            iter->pfn_dtor(iter->dev, iter->user_ptr, iter->pid, iter->usr_size);
        }

        if (iter->mmap_alloc_scheme == KU_MMAP_MEM_ALLOC_SCHEME_VMALLOC) {
            vfree(iter->realdata);
        } else {
            kfree(iter->realdata);
        }

        kfree(iter);
    }
}

void sx_mmap_set_pfn_dtor(struct sx_dev *dev,
                          unsigned long  user_ptr,
                          pid_t          pid,
                          uint64_t       usr_size,
                          mmap_pfn_dtor  pfn_dtor)
{
    struct sx_priv          *priv = sx_priv(dev);
    struct sx_mmap_metadata *iter;

    spin_lock_bh(&priv->mmap_db.lock);
    iter = __sx_mmap_lookup(dev, pid, user_ptr);
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
    int                      bucket;

    dev = sx_dbg_dump_get_device(m);
    if (!dev) {
        return 0;
    }

    priv = sx_priv(dev);

    seq_printf(m, "----------------------------------------------------------------------\n");
    seq_printf(m, "%-8s   %-18s   %-8s   %-6s\n",
               "owner", "allocation", "size", "refcnt");
    seq_printf(m, "----------------------------------------------------------------------\n");

    spin_lock_bh(&priv->mmap_db.lock);

    hash_for_each(priv->mmap_db.hash_tb, bucket, iter, h_entry) {
        seq_printf(m, "%-8u   0x%-16p   %-8lu   %-6u\n",
                   iter->pid,
                   iter->data,
                   iter->size,
                   iter->refcnt);
    }

    spin_unlock_bh(&priv->mmap_db.lock);
    return 0;
}
