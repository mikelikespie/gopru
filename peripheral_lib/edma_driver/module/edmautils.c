/*
 * edma.c
 * 
 * EDMA kernel driver
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
 
/*
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2010-11
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/device.h>

#include <asm/page.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <asm/io.h>
#include<asm/system.h>
#include <asm/irq.h>

#include <linux/completion.h>

#include <mach/edma.h>

#include "edmautils.h"


#ifdef __DEBUG
#define __D(fmt, args...) printk(KERN_DEBUG "edma Debug: " fmt, ## args)
#else
#define __D(fmt, args...)
#endif

#define __E(fmt, args...) printk(KERN_ERR "edma Error: " fmt, ## args)

typedef struct _dmacompletion {
    int channel;
    struct completion completion_evt;
} dmacompletion_t;

static int major = 0, channels_allocated = 0;
static dmacompletion_t edma_completion[MAX_EDMA_CHANNELS];

#if (USE_UDEV==1)
static struct class *edma_class;
#endif                          //USE_UDEV


/* Forward declaration of system calls */
static int ioctl(struct inode *inode, struct file *filp, unsigned int cmd,
                 unsigned long args);
//static int mmap (struct file *filp, struct vm_area_struct *vma);
static int open(struct inode *inode, struct file *filp);
static int release(struct inode *inode, struct file *filp);
static struct file_operations edma_fxns = {
    .owner = THIS_MODULE,
  unlocked_ioctl:ioctl,
//mmap:mmap,
  open:open,
  release:release
};


#if 0
static int mmap(struct file *filp, struct vm_area_struct *vma)
{
    __D("mmap: vma->vm_start     = %#lx\n", vma->vm_start);
    __D("mmap: vma->vm_pgoff     = %#lx\n", vma->vm_pgoff);
    __D("mmap: vma->vm_end       = %#lx\n", vma->vm_end);
    __D("mmap: size              = %#lx\n", vma->vm_end - vma->vm_start);
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    if (remap_pfn_range(vma, vma->vm_start,
                        vma->vm_pgoff,
                        vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
        __E("mmap: failed remap_pfn_range\n");
        return -EAGAIN;
    }
    return 0;
}
#endif

static void edma_callback(unsigned channel, u16 ch_status, void *data)
{
    int i;
    edma_stop(channel);
    for (i = 0; i < MAX_EDMA_CHANNELS; i++) {
        if (edma_completion[i].channel == channel)
            break;
    }
    if (i < MAX_EDMA_CHANNELS)
        complete(&edma_completion[i].completion_evt);

}


int EDMAAlloc(void)
{
    int channel, i;

    if (channels_allocated >= MAX_EDMA_CHANNELS)
        return -EFAULT;

    channel = edma_alloc_channel(EDMA_CHANNEL_ANY, &edma_callback, NULL,
                                 EVENTQ_1);
    edma_set_src_index(channel, 0, 0);
    edma_set_dest_index(channel, 0, 0);

    for (i = 0; i < MAX_EDMA_CHANNELS; i++) {
        if (edma_completion[i].channel == -1)
            break;
    }
    edma_completion[i].channel = channel;
    channels_allocated++;
    return channel;

}

int EDMAFree(unsigned int channel)
{
    int i;
    edma_free_channel(channel);
    for (i = 0; i < MAX_EDMA_CHANNELS; i++) {
        if (edma_completion[i].channel == channel)
            break;
    }
    if (i == MAX_EDMA_CHANNELS)
        return -EFAULT;

    edma_completion[i].channel = -1;
    channels_allocated--;
    return 0;
}

int EDMAMemCpy(memcpyparams_t memcpyparams)
{
    struct edmacc_param p_ram;
    int i;
    edma_set_src(memcpyparams.channel, memcpyparams.srcaddr, INCR, W8BIT);
    edma_set_dest(memcpyparams.channel, memcpyparams.dstaddr, INCR, W8BIT);
    edma_set_transfer_params(memcpyparams.channel, memcpyparams.len, 1,
                             1, 0, ASYNC);
    edma_read_slot(memcpyparams.channel, &p_ram);
    p_ram.opt |= TCINTEN | EDMA_TCC(EDMA_CHAN_SLOT(memcpyparams.channel));
    edma_write_slot(memcpyparams.channel, &p_ram);

    edma_start(memcpyparams.channel);

    for (i = 0; i < MAX_EDMA_CHANNELS; i++) {
        if (edma_completion[i].channel == memcpyparams.channel)
            break;
    }
    if (i == MAX_EDMA_CHANNELS)
        return -EFAULT;
    wait_for_completion_interruptible(&edma_completion[i].completion_evt);

    return 0;
}

int EDMAMemSet(memsetparams_t memsetparams)
{
    struct edmacc_param p_ram;
    int i;
    edma_set_src(memsetparams.channel, memsetparams.patternaddr, FIFO,
                 W8BIT);
    edma_set_dest(memsetparams.channel, memsetparams.dstaddr, INCR, W8BIT);
    edma_set_transfer_params(memsetparams.channel, memsetparams.len, 1,
                             1, 0, ASYNC);
    edma_read_slot(memsetparams.channel, &p_ram);
    p_ram.opt |= TCINTEN | EDMA_TCC(EDMA_CHAN_SLOT(memsetparams.channel));
    edma_write_slot(memsetparams.channel, &p_ram);

    edma_start(memsetparams.channel);

    for (i = 0; i < MAX_EDMA_CHANNELS; i++) {
        if (edma_completion[i].channel == memsetparams.channel)
            break;
    }
    if (i == MAX_EDMA_CHANNELS)
        return -EFAULT;
    wait_for_completion_interruptible(&edma_completion[i].completion_evt);
    return 0;

}

static int
ioctl(struct inode *inode, struct file *filp, unsigned int cmd,
      unsigned long args)
{
    unsigned int __user *argp = (unsigned int __user *) args;
    int channel;

    switch (cmd) {

    case EDMADRV_IOCMEMCPY_EDMA:
        {
            memcpyparams_t memcpy_params;
            if (copy_from_user
                (&memcpy_params, argp, sizeof(memcpy_params))) {
                __E("MEMCPY_EDMA failed\n");
                return -EFAULT;
            }
            return EDMAMemCpy(memcpy_params);
        }
        break;

    case EDMADRV_IOCMEMSET_EDMA:
        {
            memsetparams_t memset_params;
            if (copy_from_user
                (&memset_params, argp, sizeof(memset_params))) {
                __E("MEMSET_EDMA failed\n");
                return -EFAULT;
            }

            return EDMAMemSet(memset_params);
        }
        break;

    case EDMADRV_IOCALLOC_EDMA:
        {
            channel = EDMAAlloc();
            if (put_user(channel, argp)) {
                __E("ALLOC_ARMEDMA - put_user failed\n");
                return -EFAULT;
            }
            return 0;
        }
        break;

    case EDMADRV_IOCFREE_EDMA:
        {
            if (get_user(channel, argp)) {
                __E("FREE_ARMEDMA - get_user failed\n");
                return -EFAULT;
            }
            EDMAFree(channel);
            return 0;
        }
        break;

    default:
        __E("Unknown ioctl received = %d.\n", cmd);
        return -EINVAL;
    }
    return 0;
}


static int open(struct inode *inode, struct file *filp)
{
    __D("open: called.\n");
    return 0;
}


static int release(struct inode *inode, struct file *filp)
{
    __D("close: called.");
    return 0;
}


int __init edma_init(void)
{
    int i;
    __D("** EDMA kernel module built: " __DATE__ " " __TIME__ "\n");
    major = register_chrdev(DRIVER_MAJOR, "edma", &edma_fxns);
    if (major < 0) {
        __E("Failed to allocate major number.\n");
        return -ENODEV;
    }
#if (USE_UDEV==1)
    edma_class = class_create(THIS_MODULE, "edma");
    if (IS_ERR(edma_class)) {
        __E("Error creating edma device class.\n");
        return -EIO;
    }
    device_create(edma_class, NULL, MKDEV(major, 0), NULL, "edma");
#endif                          //USE_UDEV
    for (i = 0; i < MAX_EDMA_CHANNELS; i++) {
        init_completion(&edma_completion[i].completion_evt);
        edma_completion[i].channel = -1;
    }

    return 0;
}


void __exit edma_exit(void)
{
#if (USE_UDEV==1)
    device_destroy(edma_class, MKDEV(major, 0));
    class_destroy(edma_class);
#endif                          // USE_UDEV
    unregister_chrdev(major, "edma");
}

module_init(edma_init);
module_exit(edma_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Texas Instruments");
MODULE_DESCRIPTION("EDMA usercopy driver");
