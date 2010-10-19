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
// file:   PRU_mem1DTransfer.p 
//
// brief:  Example to demonstrate simple 1-D data movement using PRU.
//
//
//  (C) Copyright 2010, Texas Instruments, Inc
//
//  author     M. Watkins
//
//  version    0.1     Created


// The MEM1D service routine facilitates simple 1D:1D transfer.
// Parameter Table: 
//
// 31________24________16________8________0  
// |                 SRC                  |
// |______________________________________|
// |                 DST                  |
// |______________________________________|
// |                 CNT                  |
// |______________________________________| 
//

.origin 0
.entrypoint MEM1D

#include "PRU_mem1DTransfer.hp"

MEM1D:
    // Load the MEM1D parameters in registers R7:R9
    LBCO      txParams, CONST_PRUDRAM, #0x00, SIZE(Mem1DParams)

    // If count is zero, return
    QBEQ      MEM1D_EXIT, txParams.cnt, #0x00

MEM1D_COPY:
    // Load burst length (in bytes)
    LDI       mem1d_eblen, #0x20

    // Load the data from source in registers R14-R21
    LBBO      mem1d_data, txParams.src, #0x00, b0 

    // Store the data to destination
    SBBO      mem1d_data, txParams.dst, #0x00, b0 

    // Calculate the remaining count
    SUB       txParams.cnt, txParams.cnt, mem1d_eblen

    // Update the source and destination registers
    ADD       txParams.src, txParams.src, mem1d_eblen
    ADD       txParams.dst, txParams.dst, mem1d_eblen
    
    // Store the current state back to PARAM
    SBCO      txParams.src, C3, #0x00, 12

    // If count is zero, transfer is complete, return
    QBEQ      MEM1D_EXIT, txParams.cnt, #0x00

    // Perform the burst again
    QBA       MEM1D_COPY

MEM1D_EXIT:  
    // Send notification to Host for program completion
    MOV R31.b0, #PRU0_ARM_INTERRUPT
	
    HALT
   
    
