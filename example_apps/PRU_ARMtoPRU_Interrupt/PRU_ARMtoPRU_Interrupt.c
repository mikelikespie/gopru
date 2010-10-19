/*
 * PRU_ARMtoPRU_Interrupt.c
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2010-11
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */
 
/*****************************************************************************
* PRU_ARMtoPRU_Interrupt.c
*
* The host ARM interrupts the PRU by generating a system event that the PRU 
* is polling. The PRU polls for interrupts by writing into its SRSR2 
* registers. Once the PRU completes the event handling, it informs the ARM 
* that the interrupt was serviced by loading a score value from external DDR 
* memory which was originally stored by the ARM, decrementing the score value 
* by 1, and writing the new score value in shared L3 memory. The ARM polls for 
* this flag in L3 memory to verify the example executed successfully. 
*
*****************************************************************************/
	 
/*****************************************************************************
* Include Files                                                              *
*****************************************************************************/

// Standard header files
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
	 
// Driver header file
#include <prussdrv.h>
#include <pruss_intc_mapping.h>	 
	 
/*****************************************************************************
* Explicit External Declarations                                             *
*****************************************************************************/
	 
	 
/*****************************************************************************
* Local Macro Declarations                                                   *
*****************************************************************************/
	 
#define  PRU_NUM            0
#define  SCORE              0x0000000A

#define  DDR_BASEADDR 		0xC0000000
 
/*****************************************************************************
* Local Function Declarations                                                *
*****************************************************************************/

static int LOCAL_exampleInit ( );
static unsigned short LOCAL_examplePassed ( );
	 
	 
/*****************************************************************************
* Global Variable Definitions                                                *
*****************************************************************************/
	 
static int mem_fd;
static void *ddrMem;	 

/*****************************************************************************
* Global Function Definitions                                                *
*****************************************************************************/
	 
int main(void)
{
    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
	
    printf("\nINFO: Starting %s example.\r\n", "PRU_ARMtoPRU_Interrupt");
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
    prussdrv_exec_program (PRU_NUM, "./PRU_ARMtoPRU_Interrupt.bin");

    printf ("\tGenerate interrupt \n");
    prussdrv_pru_send_event (ARM_PRU0_INTERRUPT);

    /* Wait until PRU0 has finished execution */
    printf("\tINFO: Waiting for HALT command.\r\n");
    prussdrv_pru_wait_event (PRU_EVTOUT_0);
    printf("\tINFO: PRU completed transfer.\r\n");
    prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);

    /* Check if example passed */
    if ( LOCAL_examplePassed() )
    {
         printf("Example completed successfully.\n");
    }
    else
    {
         printf("Example failed.\n");
    }
	 
    /* Disable PRU and close memory mapping*/
    prussdrv_pru_disable(PRU_NUM); 
    prussdrv_exit ();
    munmap(ddrMem, 0x0FFFFFFF);
    close(mem_fd);

    return(0);
}
	 
	 
/*****************************************************************************
* Local Function Definitions                                                 *
*****************************************************************************/

static int LOCAL_exampleInit ( )
{
    /* Initialize memory pointer to start of External DDR memory */
    /* open the device */	
    mem_fd = open("/dev/mem", O_RDWR);	
    if (mem_fd < 0) {		
        printf("Failed to open /dev/mem (%s)\n", strerror(errno));		
        return -1;	
    }		

    /* map the external DDR memory */       
    ddrMem = mmap(0, 0x0FFFFFFF, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, DDR_BASEADDR);    
    if (ddrMem == NULL) {        
        printf("Failed to map the device (%s)\n", strerror(errno));        
        close(mem_fd);        
        return -1;    
    }			   
	  
    /* Store Addend and offsets in external memory locations */
    *(unsigned long*) ddrMem = SCORE;

    return(0);	     
}
	 
static unsigned short LOCAL_examplePassed ( )
{
    /* Read example done flag to check if example completed successfully */
    return (*(unsigned long*) ddrMem == (SCORE-1));
}


