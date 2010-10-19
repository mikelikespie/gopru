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
// file:   PRU_multiply.p 
//
// brief:  Example use of the common multiply macro. 
//
//
//  (C) Copyright 2010, Texas Instruments, Inc
//
//  author     M. Watkins
//
//  version    0.1     Created


// The MULTIPLY service routine facilitates simple 1D:1D transfer.
// Parameter Table: 
//
// 31        24        16        8        0  
//  ______________________________________
// |                   |       SRC1       |
// |___________________|__________________|
// |                   |       SRC2       |
// |___________________|__________________|
// |               RESULT                 |
// |______________________________________| 
//

.origin 0
.entrypoint MULTIPLY

#include "PRU_multiply.hp"

MULTIPLY:
    // Load the MEM1D parameters in registers R7:R9
    LBCO      mpyParams, CONST_PRUDRAM, #0x00, SIZE(MultiplyParams)
    MPY       mpyParams.result, mpyParams.src1, mpyParams.src2, 16
    
    // Store the current state back to PARAM
    SBCO      mpyParams.result, CONST_PRUDRAM, OFFSET(mpyParams.result), SIZE(mpyParams.result)

MULTIPLY_EXIT:  
    // Send notification to Host for program completion
    MOV R31.b0, #PRU0_ARM_INTERRUPT
    HALT
   
    
