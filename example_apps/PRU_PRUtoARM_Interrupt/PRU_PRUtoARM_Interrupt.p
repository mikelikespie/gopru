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
// file:   PRU_PRUtoARM_Interrupt.p 
//
// brief:  PRU example to Configure INTC registers to interrupt the ARM 
//		   by generating PRUSS_EVT0.
//
//  (C) Copyright 2010, Texas Instruments, Inc
//
//  author     M. Watkins
//
//  version    0.1     Created


.origin 0
.entrypoint PRUTOARM_INTERRUPT

#include "PRU_PRUtoARM_Interrupt.hp"

PRUTOARM_INTERRUPT:
    // Initialize pointer to INTC registers
    MOV32     regOffset, 0x00000000
    //Clear SYS_EVT34
    MOV32     r31, 0x00000000
    //Clear counter (r2)
    MOV32	  r2, 0x00000000

    // Global enable the all host interrupts
    LDI       regVal.w0, 0x0001
    SBCO      regVal, CONST_PRUSSINTC, GER_OFFSET, 2
    SBCO      regVal, CONST_DDR, 0x04, 2

    // Enable host interrupt 2
    MOV32     regVal, (0x00000000 | HOST_NUM)
    SBCO      regVal, CONST_PRUSSINTC, HIESR_OFFSET, 4

    // Map channel 2 to host 2
    LDI       regOffset.w0, INTC_HOSTMAP_REGS_OFFSET
    ADD       regOffset.w0, regOffset.w0, HOST_NUM
    LDI       regVal.w0, CHN_NUM
    SBCO      regVal, CONST_PRUSSINTC, regOffset.w0, 1

    // Map SYS_EVT34 interrupt to channel 2
    LDI       regOffset.w0, INTC_CHNMAP_REGS_OFFSET
    ADD       regOffset, regOffset, SYS_EVT
    LDI       regVal.b0, CHN_NUM
    SBCO      regVal.b0, CONST_PRUSSINTC, regOffset.w0, 1

    // Make sure the SYS_EVT34 system interrupt is cleared
    MOV32     regVal, SYS_EVT
    SBCO      regVal, CONST_PRUSSINTC, SICR_OFFSET, 4

    // Enable SYS_EVT34 system interrupt
    SBCO      regVal, CONST_PRUSSINTC, EISR_OFFSET,  4

EVT_GEN:	
    //Generate SYS_EVT34 by event out mapping
    MOV32     r31, SYS_EVT
    SBCO      regVal, CONST_DDR, 0x04, 4
    ADD	      r2, r2, 1


DONE:
    MOV32     regVal, 0x0B
    SBCO      regVal, CONST_DDR, 0x04, 4
    
    // Send notification to Host for program completion
    MOV R31.b0, #PRU1_ARM_INTERRUPT
    HALT
