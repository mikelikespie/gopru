/*
 * PRU_timer2Interrupt
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
* PRU_timer2Interrupt
*
* This example shows the PRU detecting a system event interrupt from a system 
* timer. The Timer2 interrupt within the PRU INTC requires the control signal, 
* PRUSSEVTSEL, to enabled. The ARM sets this control signal mux by writing to 
* the system configuration module's KICK0R, KICK1R, and CFGCHIP3 registers. 
* The ARM also configures and enables Timer2. The PRU then maps the event for 
* Timer2 to channel 0 and channel 0 to host 0 and then polls for the Timer2 
* interrupt. Upon detecting the interrupt, the PRU stores a value in the PRU 
* DRAM which the ARM checks to verify the PRU successfully received and 
* processed the interrupt. 
*
******************************************************************************/


/*****************************************************************************
* Include Files                                                              *
*****************************************************************************/

// Standard header files
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

// Driver header file
#include "prussdrv.h"
#include <pruss_intc_mapping.h>

/*****************************************************************************
* Explicit External Declarations                                             *
*****************************************************************************/


/*****************************************************************************
* Local Macro Declarations                                                   *
*****************************************************************************/

#define PRU_NUM             0
#define TIME_CONST			0x5000

//Timer addresses
#define T64P2_ADDR			(0x01F0C000u)
#define OFFSET_TIM12		(0x00000010)
#define OFFSET_PRD12		(0x00000018)
#define OFFSET_TCR			(0x00000020)
#define OFFSET_TGCR			(0x00000024)
#define OFFSET_INTCTLSTAT	(0x00000044)

//SysCFG addresses
#define SYSCFG0_BASEADDR	(0x01C14000u)
#define KICK0R_OFFSET		(0x00000038)
#define KICK1R_OFFSET		(0x0000003C)
#define CFG3_OFFSET			(0x00000188)

#define FALSE 0
#define TRUE 1

/*****************************************************************************
* Local Typedef Declarations                                                 *
*****************************************************************************/


/*****************************************************************************
* Local Function Declarations                                                *
*****************************************************************************/

static int LOCAL_exampleInit(unsigned short pruNum);
static unsigned short LOCAL_examplePassed(unsigned short pruNum);


/*****************************************************************************
* Local Variable Definitions                                                 *
*****************************************************************************/


/*****************************************************************************
* Global Variable Definitions                                                *
*****************************************************************************/

static void *pruDataMem;
static unsigned int  *pruDataMem_int;
static int mem_fd;
static void *mem_timer2, *mem_cfg;
static unsigned int *reg_TGCR, *reg_TIM12, *reg_TCR, *reg_PRD12, *reg_INTCTLSTAT;
static unsigned int *reg_KICK0R, *reg_KICK1R, *reg_CFG3;


/*****************************************************************************
* Global Function Definitions                                                *
*****************************************************************************/

int main(void)
{
    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    printf("\nINFO: Starting %s example.\r\n", "PRU_timer2Interrupt");
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
    LOCAL_exampleInit(PRU_NUM);

    /* Execute example on PRU */
    printf("\tINFO: Executing example.\r\n");
    prussdrv_exec_program (PRU_NUM, "./PRU_timer2Interrupt.bin");

    /* Give PRU some time */
    unsigned int  i;
    for (i = 0; i < 0x100; i++);
    
    /* Enable Timer2 in one-shot mode */
    *(unsigned long*) reg_TCR = 0x00000040;	 
    
    /* Wait until PRU has finished execution */
    printf("\tWaiting for HALT command.\r\n");
    prussdrv_pru_wait_event (PRU_EVTOUT_0);
    printf("\tPRU completed transfer.\r\n");
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
    
    /* Disable PRU, reset CFG3[3], and close memory mapping*/
    prussdrv_pru_disable(PRU_NUM); 

    // Unlock CFG3 register privileges	
    *(unsigned long*) reg_KICK0R = 0x83E70B13;
    *(unsigned long*) reg_KICK1R = 0x95A4F1E0;

    // Reset CFG3[3]
    *(unsigned long*) reg_CFG3 &= 0xFFFFFFF7;
    
    // Re-lock CFG3 register privileges
    *(unsigned long*) reg_KICK0R = 0x00000000;
    *(unsigned long*) reg_KICK1R = 0x00000000;

    prussdrv_exit ();
    munmap(mem_timer2, 0x0000007C);
    munmap(mem_cfg, 0x00000018C);
    close(mem_fd);

    return(0);
    
}


/*****************************************************************************
* Local Function Definitions                                                 *
*****************************************************************************/

static int LOCAL_exampleInit(unsigned short pruNum)
{
    /* Memory mapping for Timer2, SYSCFG0, and PRU DRAM */
    mem_fd = open("/dev/mem", O_RDWR);
    if (mem_fd < 0) {
        printf("Failed to open /dev/mem (%s)\n", strerror(errno));
        return -1;
    }	

    mem_timer2 = mmap(0, 0x0000007C, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, T64P2_ADDR);	 
    if (mem_timer2 == NULL) {		
        printf("Failed to map the device (%s)\n", strerror(errno));		 
        close(mem_fd);		
        return -1;	
    }	  

    mem_cfg = mmap(0, 0x00000018C, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, SYSCFG0_BASEADDR);    
    if (mem_cfg == NULL) { 	  
        printf("Failed to map the device (%s)\n", strerror(errno));	   
        close(mem_fd); 	  
        return -1;   
    }   
   
    if (pruNum == 0) {
        prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, &pruDataMem);
    }
    else if (pruNum == 1) {
        prussdrv_map_prumem (PRUSS0_PRU1_DATARAM, &pruDataMem);
    }  
    pruDataMem_int = (unsigned int*) pruDataMem;
    
    /* Configure Timer2 */
    reg_TGCR = mem_timer2 + OFFSET_TGCR;
    reg_TIM12 = mem_timer2 + OFFSET_TIM12;
    reg_TCR = mem_timer2 + OFFSET_TCR;
    reg_PRD12 = mem_timer2 + OFFSET_PRD12; 
    reg_INTCTLSTAT = mem_timer2 + OFFSET_INTCTLSTAT;
    
    *(unsigned long*) reg_TGCR = *(unsigned long*) reg_TGCR & 0xFFFFFFFC;
    *(unsigned long*) reg_TIM12 = 0;          // reset counter reg
    *(unsigned long*) reg_TCR = 0x00000000;	  // setup TCR reg
    *(unsigned long*) reg_TGCR = 0x00000015;  // setup global control reg
    *(unsigned long*) reg_PRD12 = TIME_CONST; // load time const into prd reg
    *(unsigned long*) reg_INTCTLSTAT = 0x000A000A;	 // clear pending interrupts
    *(unsigned long*) reg_INTCTLSTAT |= 0x00000001; // enable timer0 1:2 interrupt

    /* Write to CFG3[3] to set PRUSSEVTSEL = 1 for PRU Timer2 Interrupt */
    reg_KICK0R = mem_cfg + KICK0R_OFFSET;
    reg_KICK1R = mem_cfg + KICK1R_OFFSET;
    reg_CFG3 = mem_cfg + CFG3_OFFSET;
    
    // Unlock CFG3 register privileges	
    *(unsigned long*) reg_KICK0R = 0x83E70B13;
    *(unsigned long*) reg_KICK1R = 0x95A4F1E0;

    // Set CFG3[3]
    *(unsigned long*) reg_CFG3 |= 0x00000008;
    
    // Re-lock CFG3 register privileges
    *(unsigned long*) reg_KICK0R = 0x00000000;
    *(unsigned long*) reg_KICK1R = 0x00000000;

    /* Set PRU interrupt flag in PRU DRAM */
    pruDataMem_int[0] = 0xFFFFFFFF;

    return(0);
}

static unsigned short LOCAL_examplePassed(unsigned short pruNum)
{
    /* Wait for PRU to acknowledge completion. */
    while ( pruDataMem_int[0] == 0xFFFFFFFF );
    return TRUE;

}
