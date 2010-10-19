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
// file:   PRU_memAccessL3andDDR.p
//
// brief:  PRU Example to access L3 and DDR Memory. 
//
//
//  (C) Copyright 2010, Texas Instruments, Inc
//
//  author     M. Watkins
//
//  version    0.1     Created


.origin 0
.entrypoint MEMACCESSL3ANDDDR

#include "PRU_memAccessL3andDDR.hp"

MEMACCESSL3ANDDDR:
    // Configure the programmable pointer register for PRU0 by setting c30_pointer[15:0] and 
    // c31_pointer[15:0] field to 0x0020 and 0x0010, respectively.  This will make C30 point
    // to 0x80002000 (L3) and C31 point to 0xC0001000 (DDR memory).
    MOV32     r0, 0x00100020
    MOV       r1, CTPPR_1
    ST32      r0, r1
	
    //Load values from external DDR Memory into Registers R0/R1/R2
    LBCO      r0, CONST_DDR, 0, 12

    //Store values from read from the DDR memory into L3 memory
    SBCO      r0, CONST_L3RAM, 0, 12

    // Send notification to Host for program completion
    MOV R31.b0, #PRU0_ARM_INTERRUPT

    // Halt the processor
    HALT


