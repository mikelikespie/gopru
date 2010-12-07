/*
 * PRU_semaphore.c
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
* PRU_semaphore.c
*
* This example demonstrates the PRU and ARM sharing memory access via a 
* semaphore. The L3 shared memory is accessed by the PRU and ARM in succession 
* using flags and memory address pointers also stored in L3 memory. The PRU 
* accesses the L3 memory location which stores the number of memory reads 
* and the address of the source and destination locations. When the flag is 
* set to 1, the PRU then stores the values from L3 memory (source) into 
* external DDR memory (destination). When finished accessing the memory, the 
* PRU resets the flag to 0 and stores the new source address and the number 
* of reads for the ARM to follow. This process continues with the read size 
* (amount of L3 memory read from the source location) increasing by 0x4 each 
* iteration until the terminating condition (read size = 0x70) is satisfied. 
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
#define PRU_NUM       (0)

//Set read length and the flag for the memory exchange
#define FLAGINIT      (0x00000001u)
#define READSIZE      (0x00000004u)
#define READSIZEINCR  (0x00000004u)

// set the offsets for memory reads and writes
#define SRCOFFSET     (0x00000000u)
#define DSTOFFSET     (0x00000004u)
#define PTRSRC_OFFSET 16384
    
#define MAX_XFER_SIZE (70)
    
#define EXTERNALRAM_PHYS_BASE      0xC0000000 //DDR
#define EXTERNALRAM_SIZE		   0x04000000
#define SHAREDRAM_PHYS_BASE		   0x80000000 //L3
#define SHAREDRAM_SIZE		       0x00020000

#define FALSE 		   0
#define TRUE		   1

/*****************************************************************************
* Local Typedef Declarations                                                 *
*****************************************************************************/


/*****************************************************************************
* Local Function Declarations                                                *
*****************************************************************************/
static int LOCAL_exampleInit ( );
static unsigned short LOCAL_examplePassed ( unsigned short pruNum );

/*****************************************************************************
* Local Variable Definitions                                                 *
*****************************************************************************/ 


/*****************************************************************************
* Intertupt Service Routines                                                 *
*****************************************************************************/

void *pruevtout0_thread(void *arg)
{
    do
    {
    prussdrv_pru_wait_event (PRU_EVTOUT_0);
        prussdrv_pru_clear_event (PRU1_ARM_INTERRUPT);
        printf ("PRU1_ARM_INTERRUPT\n");
    } while (1);
}


/*****************************************************************************
* Global Variable Definitions                                                *
*****************************************************************************/

static int mem_fd;
static void *l3mem;
static unsigned int *l3mem_int, *regaddr_dst;
static char *ptr_src, *ptr_dst;


/*****************************************************************************
* Global Function Definitions                                                *
*****************************************************************************/

int main (void)
{
    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
    unsigned short semFlag, doneFlag;
    unsigned int read_size;
    
    printf("\nStarting PRU %s example.\r\n", "PRU_semaphore");
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
    LOCAL_exampleInit();

    doneFlag	= (unsigned short) l3mem_int[4];
    semFlag 	= (unsigned short) l3mem_int[0];

    while(!doneFlag)
    {
        // Get xfer size
        read_size = l3mem_int[1];
        printf("\tINFO: Current Transfer size = %d bytes.\r\n",read_size);
    
        // Check for terminating condition:
            //      number of memory transfer=0x46[d(70)]
        if (read_size >= MAX_XFER_SIZE)
        {
            printf("Done Flag = True. \n");
            l3mem_int[4] = (unsigned int) TRUE;
        }
        else
        {
              read_size += READSIZEINCR;
        }      

        // semFlag TRUE implies PRU has the permission to access memory
        if (semFlag)
        {
            printf("PRU has permission \n");

            printf("\tINFO: Loading and executing transfer in PRU.\r\n");
            // Enable, load, and execute the code on the specified pru 
            prussdrv_exec_program (PRU_NUM, "./PRU_semaphore.bin");

            /* Wait until PRU0 has finished execution */
            printf("\tINFO: Waiting for HALT command.\r\n");
            prussdrv_pru_wait_event (PRU_EVTOUT_0);
            printf("\tINFO: PRU completed transfer.\r\n");
            prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);

            // Disable the PRUSS when done
            prussdrv_pru_disable(PRU_NUM); 
         }
        // semFlag=0 implies ARM has the permission to access memory 
        else
        {  
            printf("ARM has permission \n");
            
            // Adjust destination pointers  
            unsigned int src_offset = l3mem_int[2];
            char *dst = ptr_dst + l3mem_int[3];

            unsigned int i=0;
            while (i < read_size)
            { 
                regaddr_dst = (unsigned int*) (dst + 4*i);
                *(unsigned long*) regaddr_dst = l3mem_int[PTRSRC_OFFSET+src_offset/4+i];
                i++;
            }

            // Set flag to indicate PRU has access
            l3mem_int[0]=TRUE;
            l3mem_int[1]=read_size;
            l3mem_int[2]+=read_size;
            l3mem_int[3]+=read_size; 

            printf("\tINFO: ARM completed transfer.\r\n");
        } 
    
        // Get current doneFlag 
        doneFlag = (unsigned short) l3mem_int[4];
        // Get current semFlag
        semFlag = (unsigned short) l3mem_int[0];
  } 
    
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
    munmap(ptr_dst, EXTERNALRAM_SIZE);
    close(mem_fd);

    return(0);
}

/*****************************************************************************
* Local Function Definitions                                                 *
*****************************************************************************/

static int LOCAL_exampleInit (  )
{
    /* Map shared memory */
    prussdrv_map_l3mem (&l3mem);	
    l3mem_int = (unsigned int*) l3mem;
    ptr_src = (char*) l3mem;

    /* open the device */
    mem_fd = open("/dev/mem", O_RDWR);
    if (mem_fd < 0) {
        printf("Failed to open /dev/mem (%s)\n", strerror(errno));
        return -1;
    }	

    /* Map memory storing destination pointer */
    ptr_dst = mmap(0, EXTERNALRAM_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, EXTERNALRAM_PHYS_BASE );
    if (ptr_dst == NULL) {
        printf("Failed to map the device (%s)\n", strerror(errno));
        close(mem_fd);
        return -1;
    }

    // Init the semFlag
    l3mem_int[0] = (unsigned int) FLAGINIT;
    
    // Set memory offsets in the Shared Ram memory locations 1,2,3 
    l3mem_int[1] = READSIZE;
    l3mem_int[2] = SRCOFFSET;
    l3mem_int[3] = DSTOFFSET;
    
    // Reset example done flag
    l3mem_int[4] = (unsigned int) FALSE; 

    return(0);
}


static unsigned short LOCAL_examplePassed ( unsigned short pruNum )
{
    unsigned short exampleDone = (unsigned short) l3mem_int[4];
    
    // Check if example completed 
    return (exampleDone);

}
