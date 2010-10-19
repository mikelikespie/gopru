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
// file:   PRU_memAccessL3.p
//
// brief:  Example accessing L3 memory from the PRU.
//
//
//  (C) Copyright 2010, Texas Instruments, Inc
//
//  author     M. Watkins
//
//  version    0.1     Created

.origin 0
.entrypoint MEMACCESS_L3

#include "PRU_memAccessL3.hp"

MEMACCESS_L3:
    // Configure the programmable pointer register for PRU0 by setting c30_pointer[15:0] field to 0x1000
    // to access L3 memory @0x80001000       						
    MOV32     r0, 0x00000010
    MOV       r1, CTPPR_1
    ST32      r0, r1

    // Configure the programmable pointer register for PRU1 by setting c30_pointer[15:0] field to 0x1000
    // to access L3 memory @0x80001000       						
    MOV       r1, CTPPR_1+0x800
    ST32      r0, r1    

    // Load values to be stored in the L3 memory
    MOV32     r0, 0x52737829
    MOV32     r1, 0x12341234
    MOV32     r2, 0xAAAA0210

    // Store the three values into L3 memory locations
    SBCO      r0, CONST_L3RAM, 12, 12 						

    // Send notification to Host for program completion
    MOV R31.b0, #PRU0_ARM_INTERRUPT
	
    HALT
