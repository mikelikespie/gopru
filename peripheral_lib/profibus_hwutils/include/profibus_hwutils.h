/*
 * profibus_hwutils.h
 * 
 *  User space library defining APIs for various Profibus related h/w 
 *  (UART, ECAP timer, TXGPIO, System Timer) initializations and utility
 *  functions on AM18xx platform
 *
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
