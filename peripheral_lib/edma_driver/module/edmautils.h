/*
 * edma.h
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

#ifndef _EDMADRV_H
#define _EDMADRV_H

#include <linux/ioctl.h>

#if defined (__cplusplus)
extern "C" {
#endif

#define MAX_EDMA_CHANNELS    8

    typedef struct _memcpy {
        int channel;
        unsigned int srcaddr;
        unsigned int dstaddr;
        unsigned int len;
    } memcpyparams_t;

    typedef struct _memset {
        int channel;
        unsigned int dstaddr;
        unsigned int patternaddr;
        unsigned int len;
    } memsetparams_t;


#define DRIVER_MAJOR 0          // define major number if 0 driver search for a free number


//define magic number for ioctl function
#define EDMADRV_IOC_MAGIC  0xE2


#define EDMADRV_IOCALLOC_EDMA	    _IOW(EDMADRV_IOC_MAGIC, 1, int)
#define EDMADRV_IOCMEMCPY_EDMA	    _IOR(EDMADRV_IOC_MAGIC, 2, memcpyparams_t*)
#define EDMADRV_IOCMEMSET_EDMA	    _IOR(EDMADRV_IOC_MAGIC, 3, memsetparams_t*)
#define EDMADRV_IOCFREE_EDMA	    _IOR(EDMADRV_IOC_MAGIC, 4, int)


#if defined (__cplusplus)
}
#endif
#endif
