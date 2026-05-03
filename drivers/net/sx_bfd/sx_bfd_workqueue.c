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

#include <linux/slab.h>
#include <linux/workqueue.h>

#include "sx_bfd_workqueue.h"

struct sx_bfd_delayed_work {
    struct delayed_work   work;
    handler_func          func;
    uint32_t              data;
    struct sx_bfd_dev_db *bfd_dev_db;
};
static void handler(struct work_struct * work)
{
    /*
     *  <work> is actually of type <struct delayed_work> and it is the <work> field of <struct sx_bfd_delayed_work>.
     *  So we need to retrieve the parent struct of <work> and we will get <struct sx_bfd_delayed_work>.
     */
    struct sx_bfd_delayed_work * sx_bfd_work = container_of((struct delayed_work*)work,
                                                            struct sx_bfd_delayed_work,
                                                            work);

    /* sxd_log_err(SXD_DEV_NONE, "sx_bfd_work %p.\n", sx_bfd_work); */
    /* sxd_log_err(SXD_DEV_NONE, "sx_bfd_work->data %d.\n", sx_bfd_work->data); */


    /* Call function with data as argument */
    sx_bfd_work->func(sx_bfd_work->bfd_dev_db, sx_bfd_work->data);
}

int sx_bfd_create_delayed_work(struct sx_bfd_dev_db   * bfd_dev_db,
                               sx_bfd_delayed_work_t ** dwork,
                               handler_func             func,
                               uint32_t                 data)
{
    int                          err = 0;
    struct sx_bfd_delayed_work * t_dwork = NULL;

    t_dwork = (struct sx_bfd_delayed_work*)kmalloc(sizeof(struct sx_bfd_delayed_work), GFP_KERNEL);
    if (t_dwork == NULL) {
        sxd_log_err(SXD_DEV_FROM_BFD(bfd_dev_db), "Memory allocation for delayed work failed.\n");
        err = -ENOMEM;
        goto bail;
    }

    INIT_DELAYED_WORK(&t_dwork->work, handler);
    t_dwork->func = func;
    t_dwork->data = data;
    t_dwork->bfd_dev_db = bfd_dev_db;

    *dwork = t_dwork;

bail:
    return err;
}

void sx_bfd_destroy_delayed_work_sync(sx_bfd_delayed_work_t * dwork)
{
    cancel_delayed_work_sync(&dwork->work); /* we must make sure work is not running while
                                             *  it is being deleted (next step here) */
    kfree(dwork);
}
void sx_bfd_delayed_work_dispatch(struct workqueue_struct * workqueue,
                                  sx_bfd_delayed_work_t   * dwork,
                                  unsigned long             delay)
{
    queue_delayed_work(workqueue, &dwork->work, delay);
}

void sx_bfd_delayed_work_cancel(sx_bfd_delayed_work_t * dwork)
{
    cancel_delayed_work(&dwork->work);
}

int sx_bfd_workqueue_init(struct sx_bfd_dev_db *bfd_dev_db)
{
    int err = 0;

    if (!bfd_dev_db->initialized) {
        bfd_dev_db->workqueue = sx_health_check_create_monitored_workqueue(bfd_dev_db->dev, "bfd");
        if (bfd_dev_db->workqueue == NULL) {
            sxd_log_err(SXD_DEV_FROM_BFD(bfd_dev_db), "Kernel BFD work queue initialization failed.\n");
            err = -EIO;
            goto bail;
        }
    }

bail:
    return err;
}

void sx_bfd_workqueue_deinit(struct sx_bfd_dev_db *bfd_dev_db)
{
    if (bfd_dev_db->initialized) {
        flush_workqueue(bfd_dev_db->workqueue);
        sx_health_check_destroy_monitored_workqueue(bfd_dev_db->dev, bfd_dev_db->workqueue);
    }
}
