/*
 * PRU_memCopy.c
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/ 
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

/* ============================================================================
 * Copyright (c) Texas Instruments Inc 2010-11
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

/******************************************************************************
* PRU_memCopy.c
* This example executes optimized memory to memory transfers on the PRU. 
* The following three cases are tests:
*       Case 1: Source and destination addresses are aligned. 
*       Case 2: Source and destination addresses are not aligned though the 
*                       offsets of source and destination addresses from next 
*                       aligned addresses are same. 
*       Case 3: Source and destination addresses are not aligned, and the 
*                       offsets of source and destination addresses from next
*                       aligned addresses are not same. 
*
* Note for 32 bit processor, aligned addresses mean that the addresses are in 
* multiple of 4. For example, 0x0000, 0x0004, 0x0008, etc are the aligned 
* addresses where as 0001, 0002, etc are not aligned addresses. They have some 
* offset from the aligned address.
*
* Below are example addresses for each case:
*
*       Case 1: Source address – 0x0004 and destination address – 0x0010 
*                        (both are aligned) 
*       Case 2: Source address – 0x0001 and destination address – 0x0011 
*                        (not aligned but the offset from aligned address is
*                         same i.e. 1) 
*       Case 3: Source address – 0x0001 and destination address – 0x0012 
*                        (not aligned and the offset from aligned address is
*                         also not same i.e. 1 for source address and 2 for 
*                         destination address) 
*
*****************************************************************************/

/*****************************************************************************
* Include Files                                                              *
*****************************************************************************/

// Standard header files
#include <stdlib.h>
#include <stdio.h>

// Driver header file
#include "prussdrv.h"
#include <pruss_intc_mapping.h>	 

/*****************************************************************************
* Explicit External Declarations                                             *
*****************************************************************************/

/*****************************************************************************
* Local Macro Declarations                                                   *
*****************************************************************************/

#define PRU_NUM 	0

#define SIZE 		8
#define	SOURCE_ADDR 0x00000010 
#define	DEST_ADDR   0x00000040


/*****************************************************************************
* Local Typedef Declarations                                                 *
*****************************************************************************/


/*****************************************************************************
* Local Function Declarations                                                *
*****************************************************************************/

static int LOCAL_exampleInit ( unsigned short pruNum, unsigned short exampleNum );
static unsigned short LOCAL_examplePassed ( unsigned short pruNum );

/*****************************************************************************
* Local Variable Definitions                                                 *
*****************************************************************************/


/*****************************************************************************
* Global Variable Definitions                                                *
*****************************************************************************/

static void *pruDataMem;
static unsigned int *pruDataMem_int;
static char *pruDataMem_char;
static unsigned int destPtr, srcPtr, src_offset, dest_offset;  


/*****************************************************************************
* Global Function Definitions                                                *
*****************************************************************************/

int main (void)
{
    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    printf("\nINFO: Starting %s example.\r\n",  "PRU_memCopy");
    /* Initialize the PRU */
    prussdrv_init ();		
    
    /* Open PRU Interrupt */
    ret = prussdrv_open(PRU_EVTOUT_0);
    if (ret)
    {
        printf("prussdrv_open open failed\n");
        return (ret);
    }
    
    /* Get the interrupt initialized */
    prussdrv_pruintc_init(&pruss_intc_initdata);
  
    unsigned int i;
    for (i=0; i<3; i++) 
    {
        /* Initialize example */
        LOCAL_exampleInit(PRU_NUM, i);
        
        /* Execute example on PRU */
        printf("\tINFO: Executing example.\r\n");
        prussdrv_exec_program (PRU_NUM, "./PRU_memCopy.bin");

        /* Wait until PRU0 has finished execution */
        printf("\tINFO: Waiting for HALT command.\r\n");
        prussdrv_pru_wait_event (PRU_EVTOUT_0);
        printf("\tINFO: PRU completed transfer.\r\n");
        prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);

        /* Check if example passed */
        if ( LOCAL_examplePassed(PRU_NUM) )
        {
            printf("Example executed succesfully.\r\n");
        }
        else
        {
            printf("Example failed.\r\n");
        }
    }

    /* Disable PRU and close memory mapping*/
    prussdrv_pru_disable (PRU_NUM);
    prussdrv_exit ();

    return(0);
}

/*****************************************************************************
* Local Function Definitions                                                 *
*****************************************************************************/

static int LOCAL_exampleInit ( unsigned short pruNum, unsigned short exampleNum )
{
    switch(exampleNum)
    {
        // Source and destination addresses are aligned
        case 0:
            printf("INFO: Initializing aligned example.\r\n");
            src_offset = 0x00000000;
            dest_offset = 0x00000000;
            break;
        // Source and destination addresses not aligned, but offset 
                // from aligned address are same
        case 1:
            printf("INFO: Initializing same offset example.\r\n");
            src_offset = 0x00000003;
            dest_offset = 0x00000003;
            break;
        // Source and destination addresses not aligned, and offset 
                // from aligned address are different
        case 2:
            printf("INFO: Initializing different offset example.\r\n");
            src_offset = 0x00000001;
            dest_offset = 0x00000003;
            break;
        default:
            return -1;
    }
        
    srcPtr = SOURCE_ADDR + src_offset;
    destPtr = DEST_ADDR + dest_offset;

    //Initialize pointer to PRU data memory
    if (pruNum == 0)
    {
        prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, &pruDataMem);
    }
    else if (pruNum == 1)
    {
        prussdrv_map_prumem (PRUSS0_PRU1_DATARAM, &pruDataMem);
    }  
    pruDataMem_int = (unsigned int*) pruDataMem;
    pruDataMem_char = (char*) pruDataMem;
    
    pruDataMem_int[0] = SIZE;
    pruDataMem_int[1] = srcPtr;
    pruDataMem_int[2] = destPtr;

    // Store values into source address
    unsigned int i;  
    for (i = 0; i < SIZE; i++)
    {
        pruDataMem_char[srcPtr+i] = rand() & 0xFF;
    }

    return(0);
}

static unsigned short LOCAL_examplePassed ( unsigned short pruNum )
{
    unsigned int i;
    
    for (i=0; i < SIZE; i++)
     {
       if (pruDataMem_char[srcPtr+i] != pruDataMem_char[destPtr+i])
       {
         return 0;
       }
    }
       return 1;
}
