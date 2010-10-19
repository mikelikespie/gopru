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
// file:   PRU_semaphore.p
//
// brief:  Co-ordinated PRU and DSP access of L3 memory  
//
//
//  (C) Copyright 2010, Texas Instruments, Inc
//
//  author     M. Watkins
//
//  version    0.1     Created


.origin 0
.entrypoint SEMAPHORE

#include "PRU_semaphore.hp"

SEMAPHORE:

CONFIG_SHAREDMEM:  
    // Configure the programmable pointer register by setting c30_pointer[15:0] field to 0x0000
    // 30th entry in the constant table will point to base address 0x80000000 of the L3 (Shared RAM)
    MOV32     r0, 0x00000000
    MOV       r1, CTPPR_1
    ST32      r0, r1

// Read values of flags, readSize and source and destination offsets
// from the start of L3 Shared RAM
INITIAL:
    // Load the PSC CTL parameters
    LBCO      semParams, CONST_L3RAM, #0x00, SIZE(SemParams)	//CONST_L3RAM = 0x80000000
    MOV       global.tempSize, semParams.readSize
    QBNE      INITIAL, semParams.semFlag, 1

CONFIG_PTRSRC:
    // Configure the programmable pointer register by setting c30_pointer[15:0] field to 0x0100
    // 30th entry in the constant table will point to base address 0x80010000 of the L3 (Shared RAM)                           
    MOV32     r0, 0x00001000
    MOV       r1, CTPPR_1  
    ST32      r0, r1
	
//Read the memory one byte at a time 
COPYLOOP:  
    QBEQ      HANDSHAKE, semParams.readSize, 0
    LBCO      global.data, CONST_L3RAM, semParams.srcOffset, 1    // read a byte, CONST_L3RAM = 0x80010000
    ADD       semParams.srcOffset, semParams.srcOffset, 1
    SBCO      global.data, CONST_DDR, semParams.dstOffset, 1      // write a byte
    ADD       semParams.dstOffset, semParams.dstOffset, 1
    SUB       semParams.readSize, semParams.readSize, 1
    JMP       COPYLOOP
	
// Set the Read size , Source and destination location for the ARM
// memory read to follow 
HANDSHAKE:
    // Configure the programmable pointer register by setting c30_pointer[15:0] field to 0x0000
    // 30th entry in the constant table will point to base address 0x80000000 of the L3 (Shared RAM)
    MOV32     r0, 0x00000000
    MOV       r1, CTPPR_1
    ST32      r0, r1
	
    ADD       semParams.readSize, global.tempSize, READSIZEINCR 
    MOV       semParams.semFlag, 0
    SBCO      semParams, CONST_L3RAM, #0x00, SIZE(SemParams)	//CONST_L3RAM = 0x80000000

END: 
    // Send notification to Host for program completion
    MOV R31.b0, #PRU0_ARM_INTERRUPT
    HALT

  



















          
