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
// file:   PRU_memAccessPRUDataRam.p 
//
// brief:  PRU access of internal Data Ram.
//
//
//  (C) Copyright 2010, Texas Instruments, Inc
//
//  author     M. Watkins
//
//  version    0.1     Created

.origin 0
.entrypoint MEMACCESSPRUDATARAM

#include "PRU_memAccessPRUDataRam.hp"

MEMACCESSPRUDATARAM:
    //Load 32 bit value in r1
    MOV32     r1, 0x0010f012

    //Load address of PRU data memory in r2
    MOV       r2, 0x0004

    // Move value from register to the PRU local data memory using registers
    ST32      r1,r2

    // Load 32 bit value into r3
    MOV32     r3, 0x0000567A

    LBCO      r4, CONST_PRUDRAM, 4, 4 //Load 4 bytes from memory location c3(PRU0/1 Local Data)+4 into r4 using constant table

    // Add r3 and r4 
    ADD       r3, r3, r4

    //Store result in into memory location c3(PRU0/1 Local Data)+8 using constant table
    SBCO      r3, CONST_PRUDRAM, 8, 4
	
    // Send notification to Host for program completion
    MOV R31.b0, #PRU0_ARM_INTERRUPT

    HALT



