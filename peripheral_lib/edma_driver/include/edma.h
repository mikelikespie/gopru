/*
 * edma.h
 * 
 * EDMA user space library - Allocates EDMA channel for memory to memory
 * copy using kernel DMA API. Allocates from the same TC (Transfer Controller)
 * so that multiple bus masters (ARM9 and PRU) can read/write to shared memory
 * using EDMA and EDMA h/w will take care of data integrity.
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2010-11
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */


#ifndef _EDMA_H
#define _EDMA_H

#include <sys/types.h>

#if defined (__cplusplus)
extern "C" {
#endif
    int edma_allocate_channel(int *channel);
    int edma_memcpy(int channel, unsigned int dst, unsigned int src,
                    unsigned int length);
    int edma_memset(int channel, unsigned int dst,
                    unsigned int patternaddr, unsigned int length);
    int edma_free_channel(int channel);

#if defined (__cplusplus)
}
#endif
#endif
