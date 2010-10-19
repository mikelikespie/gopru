/*
 * PRU_multiply.c
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2010-11
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

/*****************************************************************************
* PRU_multiply.c
*
* This example uses a common multiply macro to multiply two 16-bit numbers on 
* the PRU. The resulting product is a 32-bit number. The ARM writes two 16-bit 
* numbers into PRU DRAM. The PRU uses a multiply macro to multiply the two 
* 16-bit numbers and store their product into DRAM. The ARM then verifies the 
* result stored in DRAM. 
*
******************************************************************************/


/*****************************************************************************
* Include Files                                                              *
*****************************************************************************/

#include <stdio.h>

// Driver header file
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

/*****************************************************************************
* Explicit External Declarations                                             *
*****************************************************************************/


/*****************************************************************************
* Local Macro Declarations                                                   *
*****************************************************************************/

#define PRU_NUM  	0
#define SRC1     	0x100
#define SRC2     	0x1AA

#define FALSE 		0
#define TRUE 		1

/*****************************************************************************
* Local Typedef Declarations                                                 *
*****************************************************************************/


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

static void *pruDataMem;
static unsigned int *pruDataMem_int;


/*****************************************************************************
* Global Function Definitions                                                *
*****************************************************************************/

int main (void)
{ 
    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
    
    printf("\nINFO: Starting %s example.\r\n", "PRU_multiply");
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
    prussdrv_exec_program (PRU_NUM, "./PRU_multiply.bin");

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

    /* Disable PRU and close memory mapping*/
    prussdrv_pru_disable (PRU_NUM);
    prussdrv_exit (); 

    return(0);
}


/*****************************************************************************
* Local Function Definitions                                                 *
*****************************************************************************/

static int LOCAL_exampleInit ( unsigned short pruNum )
{
       
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
    
    pruDataMem_int[0] = (unsigned int) SRC1; 
    pruDataMem_int[1] = (unsigned int) SRC2;
    pruDataMem_int[2] = 0xFFFFFFFF;	

    return(0);
}


static unsigned short LOCAL_examplePassed ( unsigned short pruNum )
{
  // Check result
  if (pruDataMem_int[2] != (SRC1*SRC2))
  {
      return FALSE;
  }
 
  return TRUE;
}

