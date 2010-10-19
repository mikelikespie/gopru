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
// file:   PRU_access_const_tables.p 
//
// brief:  Use of constant tables to access memory and registers associated with peripherals
//
//
//  (C) Copyright 2010, Texas Instruments, Inc
//
//  author     M. Watkins
//
//  version    0.1     Created

//==============================================================================
// PRU Example to access constant tables 
// Note: 
// Fixed base addresses in constant table: Entry 0-23 
// Programmable base addresses in constant table: Entry 24-31 
// Programmable entries are configured using programmbale pointer registers and
// index registers of the PRU 
// 
// The following example demonstrates use of fixed and programmable entries in
// the constant table to write data into HPI registers from external DDR memory
//
// Intialization:
// A value of 0x00000001 is loaded into 1st memory location of the DDR and the 
// HPI clocks are enabled.
//==============================================================================

.origin 0
.entrypoint ACCESS_CONST_TABLE

#include "PRU_access_const_table.hp"

ACCESS_CONST_TABLE:
    // To access the DDR memory, the programmable pointer register is
    // configured by setting C31[15:0] field. Set R0 to zero and store
    // that value into in CTPR_1 to configure c31_pointer[15:0]

    MOV32     r0, 0x00000000
    MOV       r1, CTPPR_1
    ST32      r0, r1

    // Load values from external DDR Memory into registers r0
    LBCO      r0, CONST_DDR, 0, 4

    // Entry 15 of the Constant table contains the base address for HPI control registers.
    // Write into the PWREMU_MGMT register and read from HPIC using C15 as base address 

    //Setting the FREE bit of the PWREMU_MGMT to set the HPI in FREE mode  
    SBCO      r0, CONST_HPI, PWREMU_MGMT_OFFSET, 4

    // Read value in the HPIC register to find out HPI configurations 
    LBCO      r0, CONST_HPI, HPIC_REG_OFFSET, 4

    //Store HPI configuration in 2nd location of DDR
    SBCO      r0, CONST_DDR, 4, 4

    // Send notification to Host for program completion
    MOV R31.b0, #PRU0_ARM_INTERRUPT
    HALT

//==============================================================================
// End of file PRU_access_const_tables.p 
//
//  Copyright (c) 2009 Texas instruments,Incorporated
//  All rights reserved
//==============================================================================







