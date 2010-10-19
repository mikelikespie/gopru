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
// file:   PRU_timer2Interrupt.p 
//
// brief:  PRU example to show interrupt processing from the TIMER2 interrupt
//
//
//  (C) Copyright 2010, Texas Instruments, Inc
//
//  author     M. Watkins
//
//  version    0.1     Created


.origin 0
.entrypoint TIMER2INTERRUPT
#include "PRU_timer2Interrupt.hp"

TIMER2INTERRUPT:
    // Initialize pointer to INTC registers
    MOV32     regOffset, 0x00000000
    // Clear SYS_EVT
    MOV32     r31, 0x00000000
    
    // Global enable of all host interrupts
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

    // Map Timer 0 interrupt to channel 0
    LDI       regOffset.w0, INTC_CHNMAP_REGS_OFFSET
    ADD       regOffset, regOffset, SYS_EVT
    LDI       regVal.b0, CHN_NUM
    SBCO      regVal.b0, CONST_PRUSSINTC, regOffset.w0, 1

    // Set the polarity registers
    MOV32     regPointer, (INTC_REGS_BASE + INTC_SYS_INT_REGS_OFFSET)
    MOV32     regVal, 0xFFFFFFFF
    SBBO      regVal, regPointer, 0x00, 4
    SBBO      regVal, regPointer, 0x04, 4

    // Set the type registers
    MOV32     regVal, 0x1C000000
    SBBO      regVal, regPointer, 0x80, 4
    LDI       regVal.w2, #0x0000
    SBBO      regVal, regPointer, 0x84, 4

    // Make sure the Timer 0 system interrupt is cleared
    MOV32     regVal, (0x00000000 | SYS_EVT)
    SBCO      regVal, CONST_PRUSSINTC, SICR_OFFSET, 4

    // Enable Timer 0 system interrupt
    SBCO      regVal, CONST_PRUSSINTC, EISR_OFFSET, 4
    	
CORE_LOOP:
    // Poll for receipt of interrupt on host 0
    QBBS      EVENT_HANDLER, eventStatus, #30
    JMP       CORE_LOOP

EVENT_HANDLER:
    // Initialize pointer to priority index register location in the INTC
    LDI       regOffset.w0, INTC_HOSTINTPRIO_REGS_OFFSET

    // Retrieve interrupt number from the INTC registers
    LBCO      jumpOffset, CONST_PRUSSINTC, regOffset.w0, 4
    AND       jumpOffset.b0, jumpOffset.b0, PT_MASK
	
    // Jump to the timer interrupt ISR
    QBEQ      TIMER64P2_EVENTOUT12, jumpOffset.b0, SYS_EVT
    JMP       DONE

TIMER64P2_EVENTOUT12:
    // Clear the status of the timer interrupt
    MOV32     regVal, (0x00000000 | SYS_EVT)
    SBCO      regVal, CONST_PRUSSINTC, SICR_OFFSET, 4
    MOV32     regVal, 0x00000000
    SBCO      regVal, CONST_PRUDRAM, 0x00, 4
	
DONE:
    // Send notification to Host for program completion
    MOV R31.b0, #PRU0_ARM_INTERRUPT
    HALT
