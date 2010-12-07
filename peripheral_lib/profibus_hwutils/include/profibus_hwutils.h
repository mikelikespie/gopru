/*
 * profibus_hwutils.h
 * 
 *  User space library defining APIs for various Profibus related h/w 
 *  (UART, ECAP timer, TXGPIO, System Timer) initializations and utility
 *  functions on AM18xx platform
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


#ifndef _PROFIBUS_HWUTILS_H
#define _PROFIBUS_HWUTILS_H

#include <sys/types.h>

#if defined (__cplusplus)
extern "C"
{
#endif

  typedef void *(*profibus_hwutils_function_handler) (void *);

  int profibus_hwutils_init (void);
  int profibus_hwutils_exit (void);

  int profibus_hwutils_systemtimer_init (unsigned int period, profibus_hwutils_function_handler timerfn);	//period in millisecs

  int profibus_hwutils_uart_init (unsigned int baudrate);

  int profibus_hwutils_timer_init (void);

  int profibus_hwutils_txgpio_init (void);
  int profibus_hwutils_txgpio_enable (unsigned int enable);

  int profibus_hwutils_msleep (unsigned long milisec);

#if defined (__cplusplus)
}
#endif
#endif
