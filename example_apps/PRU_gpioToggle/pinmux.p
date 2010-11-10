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
// file:   pinmux.p
//
// brief:  This file is responsible of PIN MUX settings via R30/R31
//         for both the PRUs
//
//
//  (C) Copyright 2010, Texas Instruments, Inc
//
//  author     Amit Chatterjee
//
//  version    0.2     
//

#ifndef __pinmux_p
#define __pinmux_p 1

#include "pinmux.hp"

// Refer to this mapping in the file - \prussdrv\include\pruss_intc_mapping.h
#define PRU0_PRU1_INTERRUPT     32
#define PRU1_PRU0_INTERRUPT     33
#define PRU0_ARM_INTERRUPT      34
#define PRU1_ARM_INTERRUPT      35
#define ARM_PRU0_INTERRUPT      36
#define ARM_PRU1_INTERRUPT      37

.origin 0
.entrypoint main

main:

    //Configure PINMUX for PRU1

    //PRU1_R30[7]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX18, 4 
    AND R1.b3,R1.b3,#0xF0
    OR R1.b3,R1.b3,#0x04
    SBBO    R1, R0, PINMUX18, 4

    //PRU1_R30[10]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX14, 4 
    AND R1.b0,R1.b0,#0xF0
    OR R1.b0,R1.b0,#0x04
    SBBO    R1, R0, PINMUX14, 4 

    //PRU1_R30[13]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX13, 4 
    AND R1.b0,R1.b0,#0x0F
    OR R1.b0,R1.b0,#0x40
    SBBO    R1, R0, PINMUX13, 4

    //PRU1_R30[15]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX12, 4 
    AND R1.b0,R1.b0,#0xF0
    OR R1.b0,R1.b0,#0x04
    SBBO    R1, R0, PINMUX12, 4 

    //PRU1_R30[16]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX11, 4 
    AND R1.b3,R1.b3,#0x0F
    OR R1.b3,R1.b3,#0x40
    SBBO    R1, R0, PINMUX11, 4 

    //PRU1_R30[17]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX11, 4 
    AND R1.b3,R1.b3,#0xF0
    OR R1.b3,R1.b3,#0x04
    SBBO    R1, R0, PINMUX11, 4
    
    //PRU1_R30[18]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX11, 4 
    AND R1.b2,R1.b2,#0x0F
    OR R1.b2,R1.b2,#0x40
    SBBO    R1, R0, PINMUX11, 4

    //PRU1_R30[19]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX11, 4 
    AND R1.b2,R1.b2,#0xF0
    OR R1.b2,R1.b2,#0x04
    SBBO    R1, R0, PINMUX11, 4

    //PRU1_R30[20]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX11, 4 
    AND R1.b1,R1.b1,#0x0F
    OR R1.b1,R1.b1,#0x40
    SBBO    R1, R0, PINMUX11, 4

    //PRU1_R30[21]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX11, 4 
    AND R1.b1,R1.b1,#0xF0
    OR R1.b1,R1.b1,#0x04
    SBBO    R1, R0, PINMUX11, 4

    //PRU1_R30[22]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX11, 4 
    AND R1.b0,R1.b0,#0x0F
    OR R1.b0,R1.b0,#0x40
    SBBO    R1, R0, PINMUX11, 4

    //PRU1_R30[23]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX11, 4 
    AND R1.b0,R1.b0,#0xF0
    OR R1.b0,R1.b0,#0x04
    SBBO    R1, R0, PINMUX11, 4

    //PRU1_R30[24]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX10, 4 
    AND R1.b3,R1.b3,#0x0F
    OR R1.b3,R1.b3,#0x40
    SBBO    R1, R0, PINMUX10, 4

    //PRU1_R30[25]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX10, 4 
    AND R1.b3,R1.b3,#0xF0
    OR R1.b3,R1.b3,#0x04
    SBBO    R1, R0, PINMUX10, 4

    //PRU1_R30[26]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX10, 4 
    AND R1.b2,R1.b2,#0x0F
    OR R1.b2,R1.b2,#0x40
    SBBO    R1, R0, PINMUX10, 4

    //PRU1_R30[27]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX10, 4 
    AND R1.b2,R1.b2,#0xF0
    OR R1.b2,R1.b2,#0x04
    SBBO    R1, R0, PINMUX10, 4

    //PRU1_R30[28]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX10, 4 
    AND R1.b1,R1.b1,#0x0F
    OR R1.b1,R1.b1,#0x40
    SBBO    R1, R0, PINMUX10, 4

    //PRU1_R30[29]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX10, 4 
    AND R1.b1,R1.b1,#0xF0
    OR R1.b1,R1.b1,#0x04
    SBBO    R1, R0, PINMUX10, 4

    //PRU1_R30[30]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX10, 4 
    AND R1.b0,R1.b0,#0x0F
    OR R1.b0,R1.b0,#0x40
    SBBO    R1, R0, PINMUX10, 4

    //PRU1_R30[31]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX10, 4 
    AND R1.b0,R1.b0,#0xF0
    OR R1.b0,R1.b0,#0x04
    SBBO    R1, R0, PINMUX10, 4


    //PRU1_R31[16]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX14, 4 
    AND R1.b0,R1.b0,#0x0F
    OR R1.b0,R1.b0,#0x00
    SBBO    R1, R0, PINMUX14, 4

    //PRU1_R31[24]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX18, 4 
    AND R1.b2,R1.b2,#0x0F
    OR R1.b2,R1.b2,#0x00
    SBBO    R1, R0, PINMUX18, 4

    //PRU1_R31[25]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX18, 4 
    AND R1.b2,R1.b2,#0xF0
    OR R1.b2,R1.b2,#0x00
    SBBO    R1, R0, PINMUX18, 4

    //PRU1_R31[28]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX19, 4 
    AND R1.b3,R1.b3,#0xF0
    OR R1.b3,R1.b3,#0x00
    SBBO    R1, R0, PINMUX19, 4

    //PRU1_R31[0]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX16, 4 
    AND R1.b0,R1.b0,#0x0F
    OR R1.b0,R1.b0,#0x00
    SBBO    R1, R0, PINMUX16, 4

    //PRU1_R31[1]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX19, 4 
    AND R1.b2,R1.b2,#0x0F
    OR R1.b2,R1.b2,#0x00
    SBBO    R1, R0, PINMUX19, 4

    //PRU1_R31[2]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX19, 4 
    AND R1.b2,R1.b2,#0xF0
    OR R1.b2,R1.b2,#0x00
    SBBO    R1, R0, PINMUX19, 4

    //PRU1_R31[3]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX19, 4 
    AND R1.b1,R1.b1,#0x0F
    OR R1.b1,R1.b1,#0x00
    SBBO    R1, R0, PINMUX19, 4

    //PRU1_R31[4]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX19, 4 
    AND R1.b1,R1.b1,#0xF0
    OR R1.b1,R1.b1,#0x00
    SBBO    R1, R0, PINMUX19, 4

    //PRU1_R31[5]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX19, 4 
    AND R1.b0,R1.b0,#0x0F
    OR R1.b0,R1.b0,#0x00
    SBBO    R1, R0, PINMUX19, 4

    //PRU1_R31[6]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX19, 4 
    AND R1.b0,R1.b0,#0xF0
    OR R1.b0,R1.b0,#0x00
    SBBO    R1, R0, PINMUX19, 4

    //PRU1_R31[7]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX18, 4 
    AND R1.b3,R1.b3,#0x0F
    OR R1.b3,R1.b3,#0x00
    SBBO    R1, R0, PINMUX18, 4
    
    //PRU1_R31[8]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX17, 4 
    AND R1.b0,R1.b0,#0x0F
    OR R1.b0,R1.b0,#0x00
    SBBO    R1, R0, PINMUX17, 4

    //PRU1_R31[9]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX17, 4 
    AND R1.b0,R1.b0,#0xF0
    OR R1.b0,R1.b0,#0x00
    SBBO    R1, R0, PINMUX17, 4

    //PRU1_R31[10]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX16, 4 
    AND R1.b3,R1.b3,#0x0F
    OR R1.b3,R1.b3,#0x00
    SBBO    R1, R0, PINMUX16, 4

    //PRU1_R31[11]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX16, 4 
    AND R1.b3,R1.b3,#0xF0
    OR R1.b3,R1.b3,#0x00
    SBBO    R1, R0, PINMUX16, 4

    //PRU1_R31[12]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX16, 4 
    AND R1.b2,R1.b2,#0x0F
    OR R1.b2,R1.b2,#0x00
    SBBO    R1, R0, PINMUX16, 4

    //PRU1_R31[13]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX16, 4 
    AND R1.b2,R1.b2,#0xF0
    OR R1.b2,R1.b2,#0x00
    SBBO    R1, R0, PINMUX16, 4

    //PRU1_R31[14]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX16, 4 
    AND R1.b1,R1.b1,#0x0F
    OR R1.b1,R1.b1,#0x00
    SBBO    R1, R0, PINMUX16, 4

    //PRU1_R31[15]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX16, 4 
    AND R1.b1,R1.b1,#0xF0
    OR R1.b1,R1.b1,#0x00
    SBBO    R1, R0, PINMUX16, 4


    //GP7[0]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX18, 4 
    AND R1.b0,R1.b0,#0x0F
    OR R1.b0,R1.b0,#0x80
    SBBO    R1, R0, PINMUX18, 4

    //GP7[1]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX18, 4 
    AND R1.b0,R1.b0,#0xF0
    OR R1.b0,R1.b0,#0x08
    SBBO    R1, R0, PINMUX18, 4

    //GP7[2]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX17, 4 
    AND R1.b3,R1.b3,#0x0F
    OR R1.b3,R1.b3,#0x80
    SBBO    R1, R0, PINMUX17, 4

    //GP7[3]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX17, 4 
    AND R1.b3,R1.b3,#0xF0
    OR R1.b3,R1.b3,#0x08
    SBBO    R1, R0, PINMUX17, 4

    //GP7[4]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX17, 4 
    AND R1.b2,R1.b2,#0x0F
    OR R1.b2,R1.b2,#0x80
    SBBO    R1, R0, PINMUX17, 4

    //GP7[5]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX17, 4 
    AND R1.b2,R1.b2,#0xF0
    OR R1.b2,R1.b2,#0x08
    SBBO    R1, R0, PINMUX17, 4

    //GP7[6]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX17, 4 
    AND R1.b1,R1.b1,#0x0F
    OR R1.b1,R1.b1,#0x80
    SBBO    R1, R0, PINMUX17, 4

    //GP7[7]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX17, 4 
    AND R1.b1,R1.b1,#0xF0
    OR R1.b1,R1.b1,#0x08
    SBBO    R1, R0, PINMUX17, 4

    //GP7 Direction Register
    M_MOV32   R0, #0x1E26088      
    LBBO    R1, R0, 0, 4    
    AND R1.b2,R1.b2,#0x00
    OR R1.b2,R1.b2,#0xFF
    SBBO    R1, R0, 0, 4


    
    //Configure PINMUX for PRU0

    //GPIO0[15]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX0, 4  
    AND R1.b0,R1.b0,#0xF0
    OR R1.b0,R1.b0,#0x08
    SBBO    R1, R0, PINMUX0, 4

    // GPIO0 direction register
    M_MOV32 R0, 0x01E26010
    LBBO R1, R0, #0, 4
    CLR R1.t15
    SBBO R1, R0, #0, 4  

    //PRU0_R30[0]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX7, 4  
    AND R1.b3,R1.b3,#0x0F
    OR R1.b3,R1.b3,#0x40
    SBBO    R1, R0, PINMUX7, 4

    //PRU0_R30[1]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX6, 4  
    AND R1.b3,R1.b3,#0xF0
    OR R1.b3,R1.b3,#0x04
    SBBO    R1, R0, PINMUX6, 4

    //PRU0_R30[2]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX6, 4  
    AND R1.b1,R1.b1,#0x0F
    OR R1.b1,R1.b1,#0x40
    SBBO    R1, R0, PINMUX6, 4


    //PRU0_R30[4]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX6, 4  
    AND R1.b0,R1.b0,#0x0F
    OR R1.b0,R1.b0,#0x40
    SBBO    R1, R0, PINMUX6, 4

    //PRU0_R30[24]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX18, 4 
    AND R1.b1,R1.b1,#0x0F
    OR R1.b1,R1.b1,#0x10
    SBBO    R1, R0, PINMUX18, 4

    //PRU0_R30[25]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX18, 4 
    AND R1.b1,R1.b1,#0xF0
    OR R1.b1,R1.b1,#0x01
    SBBO    R1, R0, PINMUX18, 4

    //PRU0_R30[26]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX13, 4 
    AND R1.b3,R1.b3,#0x0F
    OR R1.b3,R1.b3,#0x10
    SBBO    R1, R0, PINMUX13, 4

    //PRU0_R30[27]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX13, 4 
    AND R1.b3,R1.b3,#0xF0
    OR R1.b3,R1.b3,#0x01
    SBBO    R1, R0, PINMUX13, 4

    //PRU0_R30[28]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX13, 4 
    AND R1.b2,R1.b2,#0x0F
    OR R1.b2,R1.b2,#0x10
    SBBO    R1, R0, PINMUX13, 4

    //PRU0_R30[29]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX13, 4 
    AND R1.b2,R1.b2,#0xF0
    OR R1.b2,R1.b2,#0x01
    SBBO    R1, R0, PINMUX13, 4

    //PRU0_R30[30]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX13, 4 
    AND R1.b1,R1.b1,#0x0F
    OR R1.b1,R1.b1,#0x10
    SBBO    R1, R0, PINMUX13, 4

    //PRU0_R30[31]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX13, 4 
    AND R1.b1,R1.b1,#0xF0
    OR R1.b1,R1.b1,#0x01
    SBBO    R1, R0, PINMUX13, 4


    //PRU0_R31[14]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX15, 4 
    AND R1.b1,R1.b1,#0x0F
    OR R1.b1,R1.b1,#0x00
    SBBO    R1, R0, PINMUX15, 4

    //PRU0_R31[15]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX15, 4 
    AND R1.b1,R1.b1,#0xF0
    OR R1.b1,R1.b1,#0x00
    SBBO    R1, R0, PINMUX15, 4

    //PRU0_R31[16]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX0, 4  
    AND R1.b3,R1.b3,#0xF0
    OR R1.b3,R1.b3,#0x00
    SBBO    R1, R0, PINMUX0, 4

    //PRU0_R31[17]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX0, 4  
    AND R1.b2,R1.b2,#0x0F
    OR R1.b2,R1.b2,#0x00
    SBBO    R1, R0, PINMUX0, 4

    //PRU0_R31[18]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX0, 4  
    AND R1.b2,R1.b2,#0xF0
    OR R1.b2,R1.b2,#0x00
    SBBO    R1, R0, PINMUX0, 4

    //PRU0_R31[19]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX0, 4  
    AND R1.b1,R1.b1,#0x0F
    OR R1.b1,R1.b1,#0x00
    SBBO    R1, R0, PINMUX0, 4

    //PRU0_R31[20]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX0, 4  
    AND R1.b1,R1.b1,#0xF0
    OR R1.b1,R1.b1,#0x00
    SBBO    R1, R0, PINMUX0, 4

    //PRU0_R31[21]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX0, 4  
    AND R1.b0,R1.b0,#0x0F
    OR R1.b0,R1.b0,#0x00
    SBBO    R1, R0, PINMUX0, 4


    //PRU0_R31[10]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX15, 4 
    AND R1.b3,R1.b3,#0x0F
    OR R1.b3,R1.b3,#0x00
    SBBO    R1, R0, PINMUX15, 4

    //PRU0_R31[11]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX15, 4 
    AND R1.b3,R1.b3,#0xF0
    OR R1.b3,R1.b3,#0x00
    SBBO    R1, R0, PINMUX15, 4

    //PRU0_R31[12]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX15, 4 
    AND R1.b2,R1.b2,#0x0F
    OR R1.b2,R1.b2,#0x00
    SBBO    R1, R0, PINMUX15, 4

    //PRU0_R31[13]
    M_MOV32   R0, SYS_BASE            
    LBBO    R1, R0, PINMUX15, 4 
    AND R1.b2,R1.b2,#0xF0
    OR R1.b2,R1.b2,#0x00
    SBBO    R1, R0, PINMUX15, 4

	// Send notification to Host for program completion
    MOV R31.b0, #PRU0_ARM_INTERRUPT
    
    // Halt the processor
    HALT
    
#endif