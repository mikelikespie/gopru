/*
 * PRU_edmaConfig.c
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
 
/*****************************************************************************
* PRU_edmaConfig.c
*
* This example code demonstrates EDMA configuration by the PRU. In the 
* initialization, the PRU configures the PSC (Power and Sleep Controller) to 
* enable the EDMA clocks. Note this instruction set (PRU_pscConfig.p) could  
* be combined with PRU_edmaConfig.p, as memory space is not an issue. The ARM 
* allocates a DMA channel and stores the source address, destination address, 
* and channel number into PRU DRAM. The PRU0 configures and initializes the 
* EDMA. The PRU1 polls for the EDMA interrupt notifying the completion of the 
* EDMA transfer and sets a flag in memory. The ARM verifies the values stored 
* at the destination address and the interrupt flag. 
*
*****************************************************************************/

/*****************************************************************************
* Include Files                                                              *
*****************************************************************************/
 
// Standard header files
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

// Driver header file
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <edma.h>


/*****************************************************************************
* Explicit External Declarations                                             *
*****************************************************************************/

/*****************************************************************************
* Local Macro Declarations                                                   *
*****************************************************************************/

#define PRU_NUM0         0
#define PRU_NUM1         1

#define PSC_0 			 0
#define PSC_1 			 1
#define PSC_CC0			 0
#define PSC_TC0 		 1
#define PSC_TC1			 2
#define PSC_MDCTL_NEXT_ENABLE   0x00000003

#define SRC_SIZE 		 0x100

#define FALSE 			 0
#define TRUE			 1


/*****************************************************************************
* Local Function Declarations                                                *
*****************************************************************************/

static int LOCAL_exampleInit ( );
static unsigned short LOCAL_examplePassed ( );


/*****************************************************************************
* Local Variable Definitions                                                 *
*****************************************************************************/


/*****************************************************************************
* Global Variable Definitions                                                *
*****************************************************************************/

static void *pru0DataMem, *pru1DataMem, *l3mem;
static unsigned int *pru0DataMem_int, *pru1DataMem_int;
static char *l3mem_char;
static int PruDmaChannel;


/*****************************************************************************
* Global Function Definitions                                                *
*****************************************************************************/

int main(void)
{
    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    printf("\t\nINFO: Starting %s example.\r\n", "PRU_edmaConfig");
    /* Initialize the PRU */
    prussdrv_init ();		
	
    /* Open PRU Interrupt */
    ret = prussdrv_open(PRU_EVTOUT_0);
    if (ret)
    {
        printf("prussdrv_open open failed\n");
        return (ret);
    }
	
    ret = prussdrv_open(PRU_EVTOUT_1);
    if (ret)
    {
        printf("prussdrv_open open failed\n");
        return (ret);
    }
	
    /* Get the interrupt initialized */
    prussdrv_pruintc_init(&pruss_intc_initdata);

    /* Initialize example */
    printf("\tINFO: Initializing example.\r\n");
    LOCAL_exampleInit(PRU_NUM0);

    /* Execute interrupt handler on PRU1 */
    printf("\tINFO: Executing interrupt handler.\r\n");
    prussdrv_exec_program (PRU_NUM1, "./PRU_edmaInterrupt.bin");  
	
    /* Execute example on PRU0 */
    printf("\t\tINFO: Executing example.\r\n");
    prussdrv_exec_program (PRU_NUM0, "./PRU_edmaConfig.bin"); 
 
    /* Wait until PRU0 has finished execution */
    printf("\t\tINFO: Waiting for HALT command.\r\n");
    prussdrv_pru_wait_event (PRU_EVTOUT_0);
    printf("\t\tINFO: PRU0 completed transfer.\r\n");
    prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);

    /* Wait until PRU1 has finished execution */
    printf("\tINFO: Waiting for HALT command.\r\n");
    prussdrv_pru_wait_event (PRU_EVTOUT_1);
    printf("\tINFO: PRU1 completed transfer.\r\n");
    prussdrv_pru_clear_event (PRU1_ARM_INTERRUPT);

    /* Check if example passed */
    if ( LOCAL_examplePassed() )
    {
        printf("Example executed succesfully.\r\n");
    }	
    else
    {
        printf("Example failed.\r\n");
    }

    /* Disable PRU, close memory mapping, and free DMA channel */
    prussdrv_pru_disable(PRU_NUM0); 
    prussdrv_pru_disable(PRU_NUM1);
    prussdrv_exit ();
    edma_free_channel(PruDmaChannel);

    return(0);
}


/*****************************************************************************
* Local Function Definitions                                                 *
*****************************************************************************/

static int LOCAL_exampleInit()
{
    /* Initialize pointer to PRU data memory	*/
    prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, &pru0DataMem);	
    pru0DataMem_int = (unsigned int*) pru0DataMem;

    pru0DataMem_int[0] = 3;	
    pru0DataMem_int[1] = ((PSC_0                   & 0xFF)<< 24 ) |
  			        ((0	                      & 0xFF)<< 16 ) |
			        ((PSC_CC0   	          & 0xFF)<<  8 ) |	
		            ((PSC_MDCTL_NEXT_ENABLE   & 0xFF)<<  0 );	
    pru0DataMem_int[2] = ((PSC_0                   & 0xFF)<< 24 ) |					
		            ((0 			          & 0xFF)<< 16 ) |			
		            ((PSC_TC0 	  	          & 0xFF)<<  8 ) |				
		            ((PSC_MDCTL_NEXT_ENABLE   & 0xFF)<<  0 );
    pru0DataMem_int[3] = ((PSC_0 		          & 0xFF)<< 24 ) |
			        ((0 					  & 0xFF)<< 16 ) |
					((PSC_TC1	  	  		  & 0xFF)<<  8 ) |	
					((PSC_MDCTL_NEXT_ENABLE   & 0xFF)<<  0 );

    printf("\tINFO: Executing PSC configuration code .\r\n"); 
    prussdrv_exec_program (PRU_NUM0, "./PRU_pscConfig.bin");

    /* Wait until PRU0 has finished execution */
    printf("\tINFO: Waiting for HALT command.\r\n");
    prussdrv_pru_wait_event (PRU_EVTOUT_0);
    printf("\tINFO: PRU completed transfer.\r\n");
    prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);

    prussdrv_map_l3mem (&l3mem);
    l3mem_char = (char*) l3mem;

    /* Put src and dst addr into PRU data mem for PRU code to read */
    pru0DataMem_int[0] = 0x80000000; //source addr:  0x80000000
    pru0DataMem_int[1] = 0x800000A0; //destination addr: 0x800000A0
    
    /* Allocate DMA channel and store into PRU data memory */
    edma_allocate_channel(&PruDmaChannel);
    pru0DataMem_int[2] = PruDmaChannel;
	
    /* Init src and dst buffers */
    unsigned int i;
    for (i = 0; i < SRC_SIZE; i++)
    {
        l3mem_char[i] = rand() & 0xFF;  // source
        l3mem_char[160+i] = 0x00;	 // destination
    }
	
    /* Set interrupt flag */
    prussdrv_map_prumem (PRUSS0_PRU1_DATARAM, &pru1DataMem);
    pru1DataMem_int = (unsigned int*) pru1DataMem;
    pru1DataMem_int[0] = 0xFFFFFFFF;

    return(0);
}  

static unsigned short LOCAL_examplePassed()
{
    /* Check data. */ 
    unsigned int i; 
    for (i = 0; i < SRC_SIZE; i++)
    {
        if( l3mem_char[i] != l3mem_char[160+i] )	// source != destination values
        {
            return FALSE;
        }
    }
	
    /* Check interrupt flag */
    while ( pru1DataMem_int[0] == 0xFFFFFFFF );
 
    return TRUE;
}
