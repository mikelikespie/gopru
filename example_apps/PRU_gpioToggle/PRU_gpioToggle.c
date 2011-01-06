/*
 * PRU_gpioToggle.c
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
 
/******************************************************************************
* PRU_gpioToggle.c
*
* The PRU controls the GPIO output by writing to R30. The PRU configures the 
* pin mux registers for PRU_R30[30, 31]  outputs as an initialization step. 
* The example toggles bits 30-31 of R30 by inverting each bit. Lastly, the 
* pin mux registers are reset to their original values.
*
******************************************************************************/


/******************************************************************************
* Include Files                                                               *
******************************************************************************/

#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

// Driver header file
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

/******************************************************************************
* Explicit External Declarations                                              *
******************************************************************************/

/******************************************************************************
* Local Macro Declarations                                                    *
******************************************************************************/

#define PRU_NUM  		 0

#define FALSE 			 0
#define TRUE			 1


/******************************************************************************
* Local Function Declarations                                                 *
******************************************************************************/

static int LOCAL_exampleInit ( unsigned short prunum );
static unsigned short LOCAL_examplePassed ( );


/******************************************************************************
* Global Variable Definitions                                                 *
******************************************************************************/

static int mem_fd;
static void *mem_pru0reg, *mem_pruReg30;
static unsigned int reg30_initState, reg30_finalState;


/******************************************************************************
* Global Function Definitions                                                 *
******************************************************************************/

int main (void)
{

    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
	
    printf("\nINFO: Starting %s example.\r\n", "PRU_gpioToggle");
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
    prussdrv_exec_program (PRU_NUM, "./PRU_gpioToggle.bin");

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

    /* Reset pinmux registers */
    printf("\tINFO: Executing pinmux reset.\r\n");
    prussdrv_exec_program (PRU_NUM, "./pinmux_reset.bin");

    /* Wait until PRU0 has finished execution */
    printf("\tINFO: Waiting for HALT command.\r\n");
    prussdrv_pru_wait_event (PRU_EVTOUT_0);
    printf("\tINFO: PRU completed transfer.\r\n");
    prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);
    
    /* Disable PRU and close memory mapping*/
    prussdrv_pru_disable (PRU_NUM);
    prussdrv_exit ();
    munmap(mem_pru0reg, 0x00000FFF);
    close(mem_fd);
	
    return(0);
}

/******************************************************************************
* Local Function Definitions                                                  *
******************************************************************************/

static int LOCAL_exampleInit ( unsigned short prunum )
{
    /* Configure pinmux registers for PRU GPIOs */
    printf("INFO: Executing pinmux configuration.\r\n");
    prussdrv_exec_program (prunum, "./pinmux.bin");

    /* Wait until PRU0 has finished execution */
    printf("\tINFO: Waiting for HALT command.\r\n");
    prussdrv_pru_wait_event (PRU_EVTOUT_0);
    printf("\tINFO: PRU completed transfer.\r\n");
    prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);

    /* Read initial state of reg30 */

    /* open the device */
    mem_fd = open("/dev/mem", O_RDWR);
    if (mem_fd < 0) {
        printf("Failed to open /dev/mem (%s)\n", strerror(errno));
        return -1;
    }	

    /* map the memory */
    mem_pru0reg = mmap(0, 0x00000FFF, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, 0x01C37000);
    if (mem_pru0reg == NULL) {
        printf("Failed to map the device (%s)\n", strerror(errno));
        close(mem_fd);
        return -1;
    }

    mem_pruReg30 = mem_pru0reg + 0x00000478;

    reg30_initState = *(unsigned long*) mem_pruReg30;

    return(0);
}

static unsigned short LOCAL_examplePassed ( )
{
    /* Read final state of reg30 */
    reg30_finalState = *(unsigned long*) mem_pruReg30;

    /* Check bits 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 30 and 31 in reg30 are inverted */
    if ((reg30_initState ^ reg30_finalState) != 0xC0000000)
    {    
        return FALSE;
    }    

    return TRUE;
}

