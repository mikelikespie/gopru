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
// file:   PRU_ARMtoPRU_Interrupt.p
//
// brief:  PRU Example showing PRU receiving ARM-generated interrupt.
//
//
//  (C) Copyright 2010, Texas Instruments, Inc
//
//  author     M. Watkins
//
//  version    0.1     Created


.origin 0
.entrypoint ARM_TO_PRU_INTERRUPT

#include "PRU_ARMtoPRU_Interrupt.hp"
      
ARM_TO_PRU_INTERRUPT:
POLL:
    // Poll for receipt of interrupt on host 0
    QBBS      EVENT, eventStatus, 30
    JMP       POLL

EVENT:
    //Clear system event in SECR2
     MOV32  regvalue,0x00000001
     MOV32  r3,SECR2_OFFSET
     SBCO	regvalue,	CONST_PRUSSINTC,	r3,        4 
        
    //Clear system event enable in ECR2
     MOV32  regvalue,0x00000001
     MOV32  r3,ECR2_OFFSET
     SBCO	regvalue,	CONST_PRUSSINTC,	r3,        4 

    // Functionality implemented in this code is simple decrement of the value of 
    // SCORE stored in the DDR memory. To access the DDR memory, the programmable
    // pointer register is configured by setting C31[15:0] field. Set R0 to zero 
    // and store that value into in CTPR_1 to configure c31_pointer[15:0].
    MOV32     r0, 0x00000000
    MOV       r1, CTPPR_1
    ST32      r0, r1

    // Load values from external DDR Memory into score register
    LBCO      global.score, CONST_DDR, 0, 4

    // Decrement value of score by 1
    SUB	      global.score, global.score, 1

    // Store values from read from the DDR memory into L3 memory
    SBCO      global.score, CONST_DDR, 0, 4

    // Send notification to Host for program completion
    MOV R31.b0, #PRU0_ARM_INTERRUPT
    HALT

.end
