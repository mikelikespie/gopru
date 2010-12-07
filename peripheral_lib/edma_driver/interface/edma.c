/*
 * edma.c
 *
 * User space driver for edma
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/ 
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

/*
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2010-11
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <pthread.h>

#include <stdlib.h>
#include <strings.h>


#include "../module/edmautils.h"
#include "edma.h"

#define EDMA_DEVICE "/dev/edma"

#ifdef __DEBUG
#define __D(fmt, args...) fprintf(stderr, "edma Debug: " fmt, ## args)
#else
#define __D(fmt, args...)
#endif

#ifdef __DEBUG
#define __E(fmt, args...) fprintf(stderr, "edma Error: " fmt, ## args)
#else
#define __E(fmt, args...)
#endif

static int refcount, edma_fd = -1;


int
edma_memcpy(int channel, unsigned int dst, unsigned int src,
            unsigned int length)
{
    memcpyparams_t memcpyparams;
    if (edma_fd == -1) {
        __E("You must initialize EDMA before making API calls\n");
        return -1;
    }

    memcpyparams.channel = channel;
    memcpyparams.srcaddr = src;
    memcpyparams.dstaddr = dst;
    memcpyparams.len = length;

    if (ioctl(edma_fd, EDMADRV_IOCMEMCPY_EDMA, &memcpyparams) == -1) {
        __E("%s could not do memcpy using EDMA\n", EDMA_DEVICE);
        return -1;
    }
    return 0;

}

int
edma_memset(int channel, unsigned int dst, unsigned int patternaddr,
            unsigned int length)
{
    memsetparams_t memsetparams;
    if (edma_fd == -1) {
        __E("You must initialize EDMA before making API calls\n");
        return -1;
    }

    memsetparams.channel = channel;
    memsetparams.patternaddr = patternaddr;
    memsetparams.dstaddr = dst;
    memsetparams.len = length;

    if (ioctl(edma_fd, EDMADRV_IOCMEMSET_EDMA, &memsetparams) == -1) {
        __E("%s could not do memcpy using  EDMA\n", EDMA_DEVICE);
        return -1;
    }
    return 0;

}


int edma_allocate_channel(int *channel)
{
    if (refcount == 0) {
        edma_fd = open(EDMA_DEVICE, O_RDWR | O_SYNC);   //O_SYNC is essential
        if (edma_fd == -1) {
            __E("init: Failed to open %s\n", EDMA_DEVICE);
            return -1;
        }
    }
    refcount++;

    if (ioctl(edma_fd, EDMADRV_IOCALLOC_EDMA, channel) == -1) {
        __E("%s could not alloc EDMA channel:%d\n", EDMA_DEVICE, *channel);
        return -1;
    }
    return 0;
}



int edma_free_channel(int channel)
{
    int result = 0;
    if (edma_fd == -1) {
        __E("exit: You must initialize EDMA before making API calls.\n");
        result = -1;
        goto end;
    }
    refcount--;

    if (ioctl(edma_fd, EDMADRV_IOCFREE_EDMA, &channel) == -1) {
        __E("%s could not free EDMA channel: %d\n", EDMA_DEVICE, channel);
        result = -1;
    }
    if (refcount == 0) {
        result = close(edma_fd);
        edma_fd = -1;
    }

  end:
    return result;
}
