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
// file:   PRU_edmaConfig.p
//
// brief:  PRU Example to configure an eDMA transfer. 
//
//
//  (C) Copyright 2010, Texas Instruments, Inc
//
//  author     M. Watkins
//
//  version    0.1     Created


#include "PRU_edmaConfig.hp"


//-----------------------------------------------------
// main()
//-----------------------------------------------------
.origin     0
.entrypoint main

main:

	// Load channel parameters from memory
	LBCO	  SRC_ADDR, CONST_PRUDRAM, 0, 4
	LBCO	  DST_ADDR, CONST_PRUDRAM, 4, 4
	LBCO      PRU_EDMA_CHANNEL, CONST_PRUDRAM, 8, 4
	
init_edma:

    // Load base addresses into registers
    MOV32 EDMA_BASE, EDMA0_CC_BASE
    LSL  EDMA_PARAM_BASE, PRU_EDMA_CHANNEL, 5 // channel*32
    LDI  R4, PARAM_OFFSET
    ADD  EDMA_PARAM_BASE, EDMA_PARAM_BASE, R4
    ADD  EDMA_PARAM_BASE, EDMA_PARAM_BASE, EDMA_BASE

    // Setup EDMA region access for Shadow Region 2
    LDI  R2, DRAE2
    LDI  R1, 1
    LSL  PRU_EDMA_CHANNEL_MASK, R1, PRU_EDMA_CHANNEL
    SBBO PRU_EDMA_CHANNEL_MASK, EDMA_BASE,  R2, 4 // Region 2
   
    // Clear channel event from EDMA event registers
    LDI   R5.w0, GLOBAL_ESR
    SBBO PRU_EDMA_CHANNEL_MASK, EDMA_BASE,  R5.w0, 4 
    LDI   R5.w0, GLOBAL_SECR
    SBBO PRU_EDMA_CHANNEL_MASK, EDMA_BASE,  R5.w0, 4 
    LDI   R5.w0, GLOBAL_ICR
    SBBO PRU_EDMA_CHANNEL_MASK, EDMA_BASE,  R5.w0, 4 
    // Enable channel interrupt
    LDI   R5.w0, GLOBAL_IESR
    SBBO PRU_EDMA_CHANNEL_MASK, EDMA_BASE,  R5.w0, 4 
    // Clear event missed register
    LDI   R5.w0, EMCR
    SBBO PRU_EDMA_CHANNEL_MASK, EDMA_BASE,  R5.w0, 4 
	
    // Setup channel to submit to EDMA Q0/TC0
    LDI   R5.w0, DMAQNUM0                // DMAQNUM0
    LBBO  R6, EDMA_BASE, R5.w0, 4
    AND	  R6.b3,	R6.b3,	0x00    // EVT6 and EVT7
    SBBO  R6, EDMA_BASE, R5.w0, 4
			
    // Setup and store PaRAM data for transfer
    //LINK = 0xFFFF, BCNTRLD = 0
    LDI   R6.w0,  0xFFFF
    SBBO  R6.w0, EDMA_PARAM_BASE, LINK_BCNTRLD, 2
    LSL   R6, PRU_EDMA_CHANNEL, 12
    OR    R6.w2, R6.w2, 0x90		//TCINTEN and TCC = PruDmaChannel
    SBBO  R6, EDMA_PARAM_BASE, OPT, 4
    LDI   R6.w0, 1
    SBBO  R6.w0, EDMA_PARAM_BASE, CCNT, 2
    LDI   LENGTH.w0, 0x0100
    SBBO  LENGTH.w0, EDMA_PARAM_BASE, A_B_CNT, 2  	
    SBBO  R6.w0, EDMA_PARAM_BASE, A_B_CNT+2, 2	 		
    SBBO  R6.w0, EDMA_PARAM_BASE, SRC_DST_BIDX, 2
    SBBO  R6.w0, EDMA_PARAM_BASE, SRC_DST_BIDX+2, 2
    SBBO  SRC_ADDR, EDMA_PARAM_BASE,  SRC, 4
    SBBO  DST_ADDR, EDMA_PARAM_BASE, DST, 4
    
    LDI   R6.w0, IEVAL
    SBBO PRU_EDMA_CHANNEL_MASK, EDMA_BASE, R6.w0, 4
    LDI   R6.w0, ESR  
    // Triggering the transfer and waiting for transfer completion
    SBBO  PRU_EDMA_CHANNEL_MASK , EDMA_BASE, R6.w0, 4
    
    //Read back interrupt for completion status
    LDI   R5.w0, IPR
WAIT_FOR_EDMA_COMPLETION:
    LBBO  R6, EDMA_BASE, R5.w0, 4
    QBBC WAIT_FOR_EDMA_COMPLETION, R6, PRU_EDMA_CHANNEL

    // Send notification to Host for program completion
    MOV R31.b0, #PRU0_ARM_INTERRUPT
    HALT
