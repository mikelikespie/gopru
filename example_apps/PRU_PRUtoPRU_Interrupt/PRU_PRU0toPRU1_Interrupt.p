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
// file:   PRU_PRU0toPRU1_Interrupt.p 
//
// brief:  PRU example to show PRU to PRU interrupts.
//
//
//  (C) Copyright 2010, Texas Instruments, Inc
//
//  author     M. Watkins
//
//  version    0.1     Created


.origin 0
.entrypoint PRU0_TO_PRU1_INTERRUPT

#include "PRU_PRUtoPRU_Interrupt.hp"


// ***************************************
// *       Local Macro definitions       *
// ***************************************

#define SYS_EVT         PRU0_PRU1_INTERRUPT
#define SYS_EVT_PRU1    PRU1_PRU0_INTERRUPT    
#define HOST_NUM        0
#define CHN_NUM         0


PRU0_TO_PRU1_INTERRUPT:
    // Initialize pointer to INTC registers
    MOV32     regOffset, 0x00000000
    // Clear SYS_EVT32
    MOV32     r31, 0x00000000

    // Global enable the all host interrupts
    LDI       regVal.w0, 0x0001
    SBCO      regVal, CONST_PRUSSINTC, GER_OFFSET, 2

    // Enable host interrupt 0
    MOV32     regVal, (0x00000000 | HOST_NUM)
    SBCO      regVal, CONST_PRUSSINTC, HIESR_OFFSET, 4

    // Map channel 0 to host 0
    LDI       regOffset.w0, INTC_HOSTMAP_REGS_OFFSET
    ADD       regOffset.w0, regOffset.w0, HOST_NUM
    LDI       regVal.w0, CHN_NUM
    SBCO      regVal, CONST_PRUSSINTC, regOffset.w0, 1

    // Map SYS_EVT32 interrupt to channel 0
    LDI       regOffset.w0, INTC_CHNMAP_REGS_OFFSET
    ADD       regOffset, regOffset, SYS_EVT
    LDI       regVal.b0, CHN_NUM
    SBCO      regVal.b0, CONST_PRUSSINTC, regOffset.w0, 1

    // Make sure the SYS_EVT32 system interrupt is cleared
    MOV32     regVal, (0x00000000 | SYS_EVT)
    SBCO      regVal, CONST_PRUSSINTC, SICR_OFFSET, 4

    // Enable SYS_EVT32 system interrupt
    SBCO      regVal, CONST_PRUSSINTC, EISR_OFFSET, 4

    //Generate SYS_EVT32
    MOV32     r31, SYS_EVT

    // Poll for receipt of interrupt on host 1
POLL:
    QBBS      DONE, eventStatus, #31
    JMP       POLL

DONE:
    // Configure the programmable pointer register for PRU0 by setting c31_pointer[31:16] 
    // field to 0x0000.  This will make C31 point to 0xC0001000 (DDR memory).
    MOV32     r0, 0x00000000
    MOV       r1, CTPPR_1
    ST32      r0, r1
	
    MOV32     regVal, 0x0B
    SBCO      regVal, CONST_DDR, 0x04, 4

    // Clear the status of the interrupt
    LDI	regVal.w2,	0x0000
    LDI	regVal.w0,	SYS_EVT_PRU1
    SBCO	regVal,	CONST_PRUSSINTC,	0x24,        4 
	
    // Send notification to Host for program completion
    MOV R31.b0, #PRU0_ARM_INTERRUPT
    HALT
