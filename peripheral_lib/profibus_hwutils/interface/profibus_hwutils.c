/*
 * profibus_hwutils.c
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

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <pthread.h>

#include <stdlib.h>
#include <strings.h>

/* For nano sleep */
#include <time.h>



#include "../module/profibus_hwinit.h"
#include "profibus_hwutils.h"



#ifdef __DEBUG
#define __D(fmt, args...) fprintf(stderr, "Profibus HwUtils Debug: " fmt, ## args)
#else
#define __D(fmt, args...)
#endif

#ifdef __DEBUG
#define __E(fmt, args...) fprintf(stderr, "Profibus HwUtils Error: " fmt, ## args)
#else
#define __E(fmt, args...)
#endif




#define PROFIBUS_SYSTEMTIMER_THREAD_PRIORITY sched_get_priority_max(SCHED_FIFO)

static int profibus_hwutils_fd = -1;
static int refcount = 0;
static int profibus_thread_exit = 1;

static profibus_hwutils_function_handler profibus_hwutils_system_timerfn =
  NULL;
static pthread_t systemtimer_thread = (pthread_t) NULL;
static pthread_attr_t systemtimer_attr;

static unsigned int profibus_ecap_clk = -1;

static unsigned int profibus_hwutils_system_timerperiod = 10;	//10ms by default


static int
__nsleep (const struct timespec *req, struct timespec *rem)
{
  struct timespec temp_rem;
  if (nanosleep (req, rem) == -1)
    return (__nsleep (rem, &temp_rem));
  else
    return 1;
}

int
profibus_hwutils_msleep (unsigned long milisec)
{
  struct timespec req = { 0 }, rem =
  {
  0};
  time_t sec = (int) (milisec / 1000);
  milisec = milisec - (sec * 1000);
  req.tv_sec = sec;
  req.tv_nsec = milisec * 1000000L;
  __nsleep (&req, &rem);
  return 1;
}


static void *
systemtimer_fn (void *arg)
{
  while (profibus_thread_exit)
    {
      profibus_hwutils_msleep (profibus_hwutils_system_timerperiod);
      if (profibus_hwutils_system_timerfn)
	profibus_hwutils_system_timerfn (arg);
    }
  if (!profibus_thread_exit)
    __D ("Exiting system timer thread\n");
  return NULL;
}


static int
__profibus_hwutils_get_timer_clock (unsigned int *clockfreq)
{
  if (profibus_hwutils_fd == -1)
    {
      __E ("ECAP_GET_CLOCK: invalid fd\n");
      return -1;
    }
  if (ioctl (profibus_hwutils_fd, PROFIBUS_IOCECAP_GET_CLOCK, clockfreq)
      == -1)
    {
      __E ("ECAP_GET_CLOCK failed.\n");
      return -1;
    }
  return 0;
}

int
profibus_hwutils_init (void)
{
  if (refcount == 0)
    {
      profibus_hwutils_fd = open ("/dev/profibus_hwinit", O_RDWR | O_SYNC);	//O_SYNC is essential
      if (profibus_hwutils_fd == -1)
	{
	  __E ("init: Failed to open /dev/profibus_hwinit.\n");
	  return -1;
	}
    }
  refcount++;
  profibus_thread_exit = 1;

  return 0;
}


int
profibus_hwutils_exit (void)
{
  int result = 0;
  if (profibus_hwutils_fd == -1)
    {
      __E ("exit: You must initialize Profibus before making API calls.\n");
      result = -1;
      goto end;
    }
  refcount--;
  if (refcount == 0)
    {
      result = close (profibus_hwutils_fd);
      profibus_hwutils_fd = -1;
    }
end:
  //Clean up  the threads
  profibus_thread_exit = 0;
  profibus_hwutils_msleep (10);

  if (systemtimer_thread)
    pthread_join (systemtimer_thread, NULL);
  return result;
}




int
profibus_hwutils_systemtimer_init (unsigned int period,
				   profibus_hwutils_function_handler timerfn)
{
  struct sched_param sched_param;
  if (profibus_hwutils_fd == -1)
    {
      __E ("SET_SYSTEM_TIMER: invalid fd\n");
      return -1;
    }
  profibus_hwutils_system_timerfn = timerfn;
  profibus_hwutils_system_timerperiod = period;

  pthread_attr_init (&systemtimer_attr);
  pthread_attr_setinheritsched (&systemtimer_attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy (&systemtimer_attr, SCHED_FIFO);
  sched_param.sched_priority = PROFIBUS_SYSTEMTIMER_THREAD_PRIORITY;
  pthread_attr_setschedparam (&systemtimer_attr, &sched_param);

  pthread_create (&systemtimer_thread, &systemtimer_attr, systemtimer_fn,
		  NULL);

  pthread_attr_destroy (&systemtimer_attr);

  return 0;
}

int
profibus_hwutils_uart_init (unsigned int baudrate)
{
  if (profibus_hwutils_fd == -1)
    {
      __E ("UART_INIT_BAUDRATE: invalid fd\n");
      return -1;
    }
  if (ioctl
      (profibus_hwutils_fd, PROFIBUS_IOCUART_INIT_BAUDRATE, &baudrate) == -1)
    {
      __E ("UART_INIT_BAUDRATE failed.\n");
      return -1;
    }
  return 0;
}


int
profibus_hwutils_timer_init (void)
{
  if (profibus_hwutils_fd == -1)
    {
      __E ("ECAP_INIT_TIMER: invalid fd\n");

      return -1;
    }
  __profibus_hwutils_get_timer_clock (&profibus_ecap_clk);

  if (ioctl (profibus_hwutils_fd, PROFIBUS_IOCECAP_INIT_TIMER, NULL) == -1)
    {
      __E ("ECAP_INIT_TIMER failed.\n");
      return -1;
    }
  return 0;
}


int
profibus_hwutils_txgpio_init (void)
{
  if (profibus_hwutils_fd == -1)
    {
      __E ("TXGPIO_INIT : invalid fd\n");
      return -1;
    }
  if (ioctl (profibus_hwutils_fd, PROFIBUS_IOCTXGPIO_INIT, NULL) == -1)
    {
      __E ("TXGPIO_INIT  failed.\n");
      return -1;
    }
  return 0;
}


int
profibus_hwutils_txgpio_enable (unsigned int enable)
{
  if (profibus_hwutils_fd == -1)
    {
      __E ("TXGPIO_ENABLE: invalid fd\n");
      return -1;
    }
  if (ioctl (profibus_hwutils_fd, PROFIBUS_IOCTXGPIO_ENABLE, &enable) == -1)
    {
      __E ("TXGPIO_ENABLE failed.\n");
      return -1;
    }
  return 0;
}
