/*
 * __prussdrv.h
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2010-11
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/unistd.h>

#define PRUSS_IRAM_SIZE                 4096
#define PRUSS_DATARAM_SIZE	            512

#define DATARAM0_PHYS_BASE            0x01C30000
#define DATARAM1_PHYS_BASE            0x01C32000
#define INTC_PHYS_BASE                0x01C34000
#define PRU0CONTROL_PHYS_BASE         0x01C37000
#define PRU0DEBUG_PHYS_BASE           0x01C37400
#define PRU1CONTROL_PHYS_BASE         0x01C37800
#define PRU1DEBUG_PHYS_BASE           0x01C37C00
#define PRU0IRAM_PHYS_BASE            0x01C38000
#define PRU1IRAM_PHYS_BASE            0x01C3C000
//NOTE: Above defines are SOC specific

#define PRU_INTC_REVID_REG  0x000
#define PRU_INTC_CR_REG  0x004
#define PRU_INTC_HCR_REG  0x00C
#define PRU_INTC_GER_REG    0x010
#define PRU_INTC_GNLR_REG    0x01C
#define PRU_INTC_SISR_REG    0x020
#define PRU_INTC_SICR_REG    0x024
#define PRU_INTC_EISR_REG    0x028
#define PRU_INTC_EICR_REG    0x02C
#define PRU_INTC_HIEISR_REG  0x034
#define PRU_INTC_HIDISR_REG  0x038
#define PRU_INTC_GPIR_REG  0x080

#define PRU_INTC_SRSR1_REG  0x200
#define PRU_INTC_SRSR2_REG  0x204

#define PRU_INTC_SECR1_REG  0x280
#define PRU_INTC_SECR2_REG  0x284

#define PRU_INTC_ESR1_REG  0x300
#define PRU_INTC_ESR2_REG  0x304

#define PRU_INTC_ECR1_REG  0x380
#define PRU_INTC_ECR2_REG  0x384

#define PRU_INTC_CMR1_REG    0x400
#define PRU_INTC_CMR2_REG    0x404
#define PRU_INTC_CMR3_REG    0x408
#define PRU_INTC_CMR4_REG    0x40C
#define PRU_INTC_CMR5_REG    0x410
#define PRU_INTC_CMR6_REG    0x414
#define PRU_INTC_CMR7_REG    0x418
#define PRU_INTC_CMR8_REG    0x41C
#define PRU_INTC_CMR9_REG    0x420
#define PRU_INTC_CMR10_REG   0x424
#define PRU_INTC_CMR11_REG   0x428
#define PRU_INTC_CMR12_REG    0x42C
#define PRU_INTC_CMR13_REG    0x430
#define PRU_INTC_CMR14_REG    0x434
#define PRU_INTC_CMR15_REG    0x438
#define PRU_INTC_CMR16_REG    0x43C

#define PRU_INTC_HMR1_REG    0x800
#define PRU_INTC_HMR2_REG    0x804
#define PRU_INTC_HMR3_REG    0x808

#define PRU_INTC_SIPR1_REG    0xD00
#define PRU_INTC_SIPR2_REG    0xD04

#define PRU_INTC_SITR1_REG    0xD80
#define PRU_INTC_SITR2_REG    0xD84

#define PRU_INTC_HIER_REG    	0x1500


#define PRUSS_UIO_DRV_PRUSS_BASE "/sys/class/uio/uio0/maps/map0/addr"
#define PRUSS_UIO_DRV_PRUSS_SIZE "/sys/class/uio/uio0/maps/map0/size"

#define PRUSS_UIO_DRV_L3RAM_BASE "/sys/class/uio/uio0/maps/map1/addr"
#define PRUSS_UIO_DRV_L3RAM_SIZE "/sys/class/uio/uio0/maps/map1/size"

#define PRUSS_UIO_DRV_EXTRAM_BASE "/sys/class/uio/uio0/maps/map2/addr"
#define PRUSS_UIO_DRV_EXTRAM_SIZE "/sys/class/uio/uio0/maps/map2/size"


typedef struct __prussdrv {
    int fd[NUM_PRU_HOSTIRQS];
    void *pru0dataram_base;
    void *pru1dataram_base;
    void *intc_base;
    void *pru0control_base;
    void *pru0debug_base;
    void *pru1control_base;
    void *pru1debug_base;
    void *pru0iram_base;
    void *pru1iram_base;
    void *l3ram_base;
    void *extram_base;
    pthread_t irq_thread[NUM_PRU_HOSTIRQS];
    int mmap_fd;
} tprussdrv;

static unsigned int pruss_phys_base, pruss_map_size;
static unsigned int l3ram_phys_base, l3ram_map_size;
static unsigned int extram_phys_base, extram_map_size;


void __prussintc_set_cmr(unsigned int *pruintc_io, unsigned short sysevt,
                         unsigned short channel)
{
    pruintc_io[(PRU_INTC_CMR1_REG + (sysevt & ~(0x3))) >> 2] |=
        ((channel & 0xF) << ((sysevt & 0x3) << 3));

}


void __prussintc_set_hmr(unsigned int *pruintc_io, unsigned short channel,
                         unsigned short host)
{
    pruintc_io[(PRU_INTC_HMR1_REG + (channel & ~(0x3))) >> 2] =
        pruintc_io[(PRU_INTC_HMR1_REG +
                    (channel & ~(0x3))) >> 2] | (((host) & 0xF) <<
                                                 (((channel) & 0x3) << 3));

}
