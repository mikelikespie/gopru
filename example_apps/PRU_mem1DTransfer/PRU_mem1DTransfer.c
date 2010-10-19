/*
 * PRU_mem1DTransfer.c
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2010-11
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 * 
 */

/*****************************************************************************
* PRU_mem1DTransfer.c
*
* The PRU executes a simple 1-D byte array system memory transfer. As an 
* initialization step, the ARM writes an array of random byte-sized values 
* into a source address in L3 memory. The ARM then stores the source address, 
* destination address, and number of bytes to transfer into PRU's DRAM. The 
* PRU then transfers the bytes from the source address to the destination 
* address. The ARM compares the values at the source and destination address 
* to verify the transfer was successful. 
*
*****************************************************************************/

/*****************************************************************************
* Include Files                                                              *
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

// Header file
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

/*****************************************************************************
* Explicit External Declarations                                             *
*****************************************************************************/

/*****************************************************************************
* Local Macro Declarations                                                   *
*****************************************************************************/
#define PRU_NUM  (0)

#define SOMEDATASRC_ADDR 0x80000000u
#define SOMEDATADEST_ADDR 0x80000100u
#define SIZE 32

#define FALSE 0
#define TRUE  1


/*****************************************************************************
* Local Function Declarations                                                *
*****************************************************************************/

static int LOCAL_exampleInit ( unsigned short pruNum );
static unsigned short LOCAL_examplePassed ( unsigned short pruNum );


/*****************************************************************************
* Local Variable Definitions                                                 *
*****************************************************************************/ 


/*****************************************************************************
* Global Variable Definitions                                                *
*****************************************************************************/

static void *pruDataMem, *l3mem;
static char *l3mem_char;
static unsigned int *pruDataMem_int;


/*****************************************************************************
* Global Function Definitions                                                *
*****************************************************************************/

int main (void)
{
    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
	
    printf("\nINFO: Starting %s example.\r\n", "PRU_mem1DTransfer");
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

    /* Initialize example */
    printf("\tINFO: Initializing example.\r\n");
    LOCAL_exampleInit(PRU_NUM);

    /* Execute example on PRU */
    printf("\tINFO: Executing example.\r\n");
    prussdrv_exec_program (PRU_NUM, "./PRU_mem1DTransfer.bin");

    /* Wait until PRU has finished execution */
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

    /* Disable PRU and close memory mapping*/
    prussdrv_pru_disable(PRU_NUM); 
    prussdrv_exit ();

    return(0);
}

/*****************************************************************************
* Local Function Definitions                                                 *
*****************************************************************************/

static int LOCAL_exampleInit ( unsigned short pruNum )
{
    if (pruNum == 0)
    {
        prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, &pruDataMem);
    }
    else if (pruNum == 1)
    {
        prussdrv_map_prumem (PRUSS0_PRU1_DATARAM, &pruDataMem);
    }
    pruDataMem_int = (unsigned int*) pruDataMem;

    pruDataMem_int[0] = SOMEDATASRC_ADDR; 
    pruDataMem_int[1] = SOMEDATADEST_ADDR;
    pruDataMem_int[2] = (unsigned int) SIZE;  
	
    prussdrv_map_l3mem (&l3mem);	
    l3mem_char = (char*) l3mem;

    // Store values into source
    unsigned int i;
    for (i=0; i < SIZE; i++)
    {
        l3mem_char[i] = rand() + 0xFF;
    }

    // Clear each byte at the destination
    for (i=0; i < SIZE; i++)
    {
        l3mem_char[i+256] = 0xFF;
    }

    return(0);
}

static unsigned short LOCAL_examplePassed ( unsigned short pruNum )
{
    // Check counter
    if (pruDataMem_int[2] != 0)
    {
        return FALSE;
    }

    unsigned int i;	
    for (i=0; i < SIZE; i++)
    {
        if(l3mem_char[i] != l3mem_char[i+256])	// if( src != dst values)
        {
            return FALSE;
        }
    }
  
    return TRUE;
}

