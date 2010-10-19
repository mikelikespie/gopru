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
// file:   PRU_gpioToggle.p 
//
// brief:  PRU Example to demonstrate toggling direct connected GPIOs (through R30).
//
//
//  (C) Copyright 2010, Texas Instruments, Inc
//
//  author     M. Watkins
//
//  version    0.1     Created

.origin 0
.entrypoint GPIO_TOGGLE

#include "PRU_gpioToggle.hp"

GPIO_TOGGLE:
    // Load starting bit to toggle
    LDI       global.count,	GPIO_BIT

LOOP:
    QBLT      DONE, global.count, GPIO_COUNT
    QBBC	  SET_GPIO, r30, global.count
    QBBS	  CLR_GPIO, r30, global.count

SET_GPIO:
    gpioSet   global.count
    ADD       global.count, global.count, 0x1
    JMP		  LOOP
	
CLR_GPIO:
    gpioClr   global.count
    ADD       global.count, global.count, 0x1
    JMP		  LOOP

DONE:
	// send notification to Host for program completion
    MOV R31.b0, #PRU0_ARM_INTERRUPT
    HALT
