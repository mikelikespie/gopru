/*
 * PRU_miscOperations.c
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

/****************************************************************************
*
* This example illustrates 
*  
* (a)extraction of 4bit values of a 32 bit number using masks 
* and bit shift operations. In the example a 32 bit value is placed in source 
* location in L3 memory. The PRU reads the source data and extracts 8 four 
* bit datas of a 32 bit number and stores the extracted 4 bits(byte rounded)
* in PRU1 Data memory.
*
* (b) The array of extracted bits is used as the initial array to demonstrate 
* implementation of bubble sort algorithm and the result of the algorithm is 
* stored in the DDR memory.
*
* (c) The sorted array is then thresholded by applying a cut-off value and 
* converted to an array of zeros and ones. The subsequent thresholded result 
* is stored in PRU0 Data memory.
*
* (d) The above implementations are also performed in C and the results from
* the PRU code and the C code are compared to determine the success of the
* example.
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
#include <unistd.h>
#include <string.h>

// Driver header file
#include <prussdrv.h>
#include <pruss_intc_mapping.h>	 


/*****************************************************************************
* Explicit External Declarations                                             *
*****************************************************************************/

// Memory locations 
//  SOURCE: 0x80000000(L3)
//  DESTINATION for Mask and bit shift operations: 0x01C30000-0x01c30008
//       (PRU0 Data Mem)
//  DESTINATION for Bubble Sort: 0xc0000000-0xc00008(DDR),  
//  DESTINATION for Thresholding ouptut:0x01C32000-0x01c32008(PRU1 Data Mem)

/*****************************************************************************
* Local Macro Declarations                                                   *
*****************************************************************************/

#define PRU_NUM       (0)
#define ADDEND1       (0x0FDB5BF2u)
#define MASK          (0x0000000Fu)
#define SHIFT         (0x00000004u)
#define THRESHOLD     (0x08u)

//decimal offset for a 32 bit pointer pointing to DDR memory
#define DDR_BASEADDR  (0xC0000000)
#define OFFSET_DDR    (0)
//Memory locations 0x80002000-0x800200C are used for the example 
//decimal offset for a 32 bit pointer pointing to L3 memory
#define OFFSET_L3     (0)

#define FALSE		  (0)
#define TRUE		  (1)

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
* Global Variable Definitions                                                *
*****************************************************************************/

static int mem_fd;
static void *pru0DataMem, *pru1DataMem, *l3mem;
static char *pruDataMem_char; 
static unsigned int *l3mem_int;


/*****************************************************************************
* Global Function Definitions                                                *
*****************************************************************************/
int main (void)
{
    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    printf("\nINFO: Starting %s example.\r\n", "PRU_miscOperations");
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
    prussdrv_exec_program (PRU_NUM, "./PRU_miscOperations.bin");
       
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
    prussdrv_pru_disable(PRU_NUM); 
    prussdrv_exit ();
    close(mem_fd);

    return(0);

}


/*****************************************************************************
* Local Function Definitions                                                 *
*****************************************************************************/

static int LOCAL_exampleInit ( )
{
     /* Allocate L3 memory. */
    prussdrv_map_l3mem (&l3mem);
    l3mem_int = (unsigned int*) l3mem;

    l3mem_int[OFFSET_L3] = ADDEND1;	

    return(0);
}

static unsigned short LOCAL_examplePassed ( unsigned short pruNum )
{
    
    char tmp,result[8];
    unsigned int temp,temp_mask=MASK;
    char result_expected[8];
    unsigned short pass_flag = TRUE;
    signed int i,j,shift_right=0;

    // (a) Mask and bitshift operations
    // Initialize memory pointer to start of PRU1_DATA memory
    prussdrv_map_prumem (PRUSS0_PRU1_DATARAM, &pru1DataMem);
    pruDataMem_char = (char*) pru1DataMem;
  
    for (i=0;i<8;i++)
    {
        //Read result computed from PRU code
        result[i] = pruDataMem_char[i]; 
            
        // Compute expected result in C
        temp= (ADDEND1 & temp_mask);
        result_expected[i]=temp>>shift_right;
        shift_right+=4;
        temp_mask=temp_mask<<SHIFT;
    
        //Compare the results from the C and PRU code
        pass_flag = pass_flag && (result[i] == result_expected[i]);
    }
  
    //Report result from comparision
    if (pass_flag) 
    {
        printf("Masking and bit_shift operations were executed succesfully\n");
    }
    else 
    {
        printf("Masking and bit_shift operations failed\n");
    }

    // (b) Implementation of Bubble sort Algortihm 
    // Initialize memory pointer to start of DDR memory
  
    /* open the device */
    mem_fd = open("/dev/mem", O_RDWR);
    if (mem_fd < 0) {
        printf("Failed to open /dev/mem (%s)\n", strerror(errno));
        return -1;
    }   
  
    /* map the memory */
    pruDataMem_char = mmap(0, 0x0FFFFFFF, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, DDR_BASEADDR);
    if (pruDataMem_char == NULL) {
        printf("Failed to map the device (%s)\n", strerror(errno));
        close(mem_fd);
        return -1;
    }

    //Read result computed from PRU code   
    for (i=0;i<8;i++)
    { 
        result[i] = pruDataMem_char[i];
    }

    //Compute expected result of bubble sort algorithm in C
    for (i=0;i<8;i++) 
    {
        for (j=0;j<7;j++) 
        {
            if(result_expected[j]>result_expected[j+1]) 
            {
                tmp = result_expected[j+1];
                result_expected[j+1] = result_expected[j];
                result_expected[j] = tmp;
            }
        }
    }
  
    //Compute expected result in C
    for(i=0;i<8;i++)
    {
        pass_flag = pass_flag && (result[i] == result_expected[i]);
    }
  
    //Report result from comparision
    if(pass_flag==1) 
    {
        printf("Bubble sort was executed succesfully\n");
    }
    else 
    {
        printf("Bubble sort function failed\n");
    }

    // Close memory mapping
       munmap(pruDataMem_char, 0x0FFFFFFF);
    
    // (c) Thresholding 
    // Initialize memory pointer to start of PRU0_DATA memory
    prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, &pru0DataMem);
    pruDataMem_char = (char*) pru0DataMem;
    
    //Read result computed from PRU code
    for (i=0;i<8;i++)
    { 
        result[i] = pruDataMem_char[i];
    }

    //Compute expected result of Thresholding in C 
    for(j=0;j<8;j++) 
    {
        if(result_expected[j]> THRESHOLD)
        {
            result_expected[j]=1;
        }	 
        else
        {
            result_expected[j]=0;
        } 
        //Compare the results from the C and PRU code
        pass_flag = pass_flag && (result[j] == result_expected[j]);
    }
  
    //Report result from comparision
    if(pass_flag)
    {
        printf("Thresholding functionality was executed succesfully\n");
    }
    else
    {  
        printf("Thresholding functionality failed\n");
    }

    return pass_flag;
}

