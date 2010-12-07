/*
 * PRU_PRUtoARM_Interrupt.c
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

/*
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2010-11
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */


/*****************************************************************************
 * PRU_PRUtoARM_Interrupt.c
*
* The PRU generates an interrupt to the host ARM. The PRU configures the PRU 
* INTC registers and connects system event 34 to channel 2 which in turn is 
* hooked to the host port 2. Host port 2 generates the SYS_EVT0 for the ARM. 
* PRU follows the event out mapping procedure by writing to its own status 
* register 31 to generate an internal system event. After generating the 
* event, the PRU stores a value into external DDR memory. The ARM clears a 
* flag upon receiving the SYS_EVT0 interrupt and verifies the value in DDR 
* memory. 
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
#include <pthread.h>
    
// Driver header file
#include <prussdrv.h>
#include <pruss_intc_mapping.h>	 

/*****************************************************************************
* Explicit External Declarations                                             *
*****************************************************************************/


/*****************************************************************************
* Local Macro Declarations                                                   *
*****************************************************************************/

#define  PRU_NUM        0

#define DDR_BASEADDR 	0xC0000000
#define OFFSET_DDR		0x00001000 //1024

#define SECR1 			160

#define FALSE			0
#define TRUE			1

/*****************************************************************************
* Local Function Declarations                                                *
*****************************************************************************/

static int LOCAL_exampleInit();
static unsigned short LOCAL_examplePassed();


/*****************************************************************************
* Global Variable Definitions                                                *
*****************************************************************************/

static char PRU_IntReceived;
static int mem_fd;
static void  *ddrMem;


/*****************************************************************************
* Intertupt Service Routines                                                 *
*****************************************************************************/
    
void *pruevtout0_thread(void *arg)
{
    do
    {
    prussdrv_pru_wait_event (PRU_EVTOUT_0);
        prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);
        PRU_IntReceived = 1;
    } while (1);
}


/*****************************************************************************
* Global Function Definitions                                                *
*****************************************************************************/

int main(void)
{	

    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    printf("\nINFO: Starting %s example.\r\n", "PRU_PRUtoARM_Interrupt");
    /* Initialize the PRU */
    prussdrv_init ();		
    
    /* Open PRU Interrupt */
    ret = prussdrv_open(PRU_EVTOUT_0);
    if (ret)
    {
        printf("prussdrv_open open failed\n");
        return (ret);
    }
    
    /* Open PRU Interrupt */
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
    LOCAL_exampleInit( );

    /* Start IRQ thread for event handling */
    prussdrv_start_irqthread (PRU_EVTOUT_0, sched_get_priority_max(SCHED_FIFO) - 2, pruevtout0_thread);

    /* Execute example on PRU */
    printf("\tINFO: Executing example.\r\n");
    prussdrv_exec_program (PRU_NUM, "./PRU_PRUtoARM_Interrupt.bin");
      
    /* Wait until PRU0 has finished execution */
    printf("\tINFO: Waiting for HALT command.\r\n");
    prussdrv_pru_wait_event (PRU_EVTOUT_1);
    printf("\tINFO: PRU completed transfer.\r\n");
    prussdrv_pru_clear_event (PRU1_ARM_INTERRUPT);
    
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
    munmap(ddrMem, 0x0FFFFFFF);
    close(mem_fd);

    return(0);
}


/*****************************************************************************
* Local Function Definitions                                                 *
*****************************************************************************/
static int LOCAL_exampleInit()
{
    PRU_IntReceived = 0;

    return(0);
}  
        
        
static unsigned short LOCAL_examplePassed()
{ 
    void *DDR_regaddr; 
    // Wait for PRUSS_EVTOUT0 interrupt.
    while ( PRU_IntReceived != 1 );

    /* open the device */
    mem_fd = open("/dev/mem", O_RDWR);
    if (mem_fd < 0) {
        printf("Failed to open /dev/mem (%s)\n", strerror(errno));
        return -1;
    }	

    /* map the memory */
    ddrMem = mmap(0, 0x0FFFFFFF, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, 0xC0000000);
    if (ddrMem == NULL) {
        printf("Failed to map the device (%s)\n", strerror(errno));
        close(mem_fd);
        return -1;
    }

    DDR_regaddr = ddrMem + 0x00000004;
    if (*(unsigned long*) DDR_regaddr == 0x0B)
    {
          // PRU interrupt received and memory written to
          return TRUE;
    } 
    else
    {
          // PRU interrupt received and memory not written to
          return FALSE;
    }
}
