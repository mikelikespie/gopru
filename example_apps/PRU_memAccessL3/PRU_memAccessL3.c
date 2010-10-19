/*
 * PRU_memAccessL3.c
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2010-11
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

/*****************************************************************************
* PRU_memAccessL3.c
*
* This example demonstrates how values can be loaded into the L3 shared memory 
* by configuring the PRU Constant Table Programmable Pointer Register 30. The 
* PRU loads three values into an offset of L3 shared memory. The ARM verifies 
* these values in the destination address. 
*
*****************************************************************************/


/*****************************************************************************
* Include Files                                                              *
*****************************************************************************/

#include <stdio.h>

// Header files
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

/*****************************************************************************
* Explicit External Declarations                                             *
*****************************************************************************/


/*****************************************************************************
* Local Macro Declarations                                                   *
*****************************************************************************/
#define ADDEND1  (0x52737829u)
#define ADDEND2  (0x12341234u)
#define ADDEND3  (0xAAAA0210u)
    
#define PRU_NUM  (0)


/*****************************************************************************
* Local Function Declarations                                                *
*****************************************************************************/

static int LOCAL_exampleInit (  );
static unsigned short LOCAL_examplePassed ( unsigned short pruNum );


/*****************************************************************************
* Local Variable Definitions                                                 *
*****************************************************************************/ 


/*****************************************************************************
* Global Variable Definitions                                                *
*****************************************************************************/

static void *l3mem;
static unsigned int *l3mem_int;


/*****************************************************************************
* Global Function Definitions                                                *
*****************************************************************************/

int main (void)
{
    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
    
    printf("\nINFO: Starting %s example.\r\n", "PRU_memAccessL3");
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
    LOCAL_exampleInit( );

    /* Execute example on PRU */
    printf("\tINFO: Executing example.\r\n");
    prussdrv_exec_program (PRU_NUM, "./PRU_memAccessL3.bin");

    /* Wait until PRU0 has finished execution */
    printf("\tINFO: Waiting for HALT command.\r\n");
    prussdrv_pru_wait_event (PRU_EVTOUT_0);
    printf("\tINFO: PRU completed transfer.\r\n");
    prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);

    /* Check if example passed */
    if ( LOCAL_examplePassed(PRU_NUM) )
    {
        printf("INFO: Example executed succesfully.\r\n");
    }
    else
    {
        printf("INFO: Example failed.\r\n");
    }

    /* Disable PRU and close memory mapping*/
    prussdrv_pru_disable(PRU_NUM); 
    prussdrv_exit ();

    return(0);
}

/*****************************************************************************
* Local Function Definitions                                                 *
*****************************************************************************/

static int LOCAL_exampleInit (  )
{
    prussdrv_map_l3mem (&l3mem);
    l3mem_int = (unsigned int*) l3mem;

    // Flush the values in the shared data memory locations
    l3mem_int[1027] = 0x00000000;
    l3mem_int[1028] = 0x00000000;
    l3mem_int[1029] = 0x00000000;

    return(0);
}

static unsigned short LOCAL_examplePassed ( unsigned short pruNum )
{
    // Return acceptance/denial of success of the example
    return (( l3mem_int[1027] == ADDEND1) && ( l3mem_int[1028] == ADDEND2) && ( l3mem_int[1029] == ADDEND3)) ;
}
