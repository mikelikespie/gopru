//******************************************************************************
//+--------------------------------------------------------------------------+**
//|                            ****                                          |**
//|                            ****                                          |**
//|                            ******o***                                    |**
//|                      ********_///_****                                   |**
//|                      ***** /_//_/ ****                                   |**
//|                       ** ** (__/ ****                                    |**
//|                           *********                                      |**
//|                            ****                                          |**
//|                            ***                                           |**
//|                                                                          |**
//|         Copyright (c) 1998-2010 Texas Instruments Incorporated           |**
//|                        ALL RIGHTS RESERVED                               |**
//|                                                                          |**
//| Permission is hereby granted to licensees of Texas Instruments           |**
//| Incorporated (TI) products to use this computer program for the sole     |**
//| purpose of implementing a licensee product based on TI products.         |**
//| No other rights to reproduce, use, or disseminate this computer          |**
//| program, whether in part or in whole, are granted.                       |**
//|                                                                          |**
//| TI makes no representation or warranties with respect to the             |**
//| performance of this computer program, and specifically disclaims         |**
//| any responsibility for any damages, special or consequential,            |**
//| connected with the use of this program.                                  |**
//|                                                                          |**
//+--------------------------------------------------------------------------+**
//*****************************************************************************/
// file:   pinmux_reset.p
//
// brief:  This file is responsible of resetting the PIN MUX settings to the 
//         original settings that were changed by pinmux.p
//
//
//  (C) Copyright 2010, Texas Instruments, Inc
//
//  author     M. Watkins
//
//  version    0.1
//

#ifndef __pinmux_reset_p
#define __pinmux_reset_p 1

#include "pinmux.hp"

// Refer to this mapping in the file - \prussdrv\include\pruss_intc_mapping.h
#define PRU0_PRU1_INTERRUPT     32
#define PRU1_PRU0_INTERRUPT     33
#define PRU0_ARM_INTERRUPT      34
#define PRU1_ARM_INTERRUPT      35
#define ARM_PRU0_INTERRUPT      36
#define ARM_PRU1_INTERRUPT      37

.origin 0
.entrypoint main

main:

    //Reset PINMUX13
    M_MOV32   R0, SYS_BASE            
    LBCO    R1, C3, 0, 4
    SBBO    R1, R0, PINMUX13, 4

    // Send notification to Host for program completion
    MOV R31.b0, #PRU0_ARM_INTERRUPT
    
    // Halt the processor
    HALT
    
#endif
