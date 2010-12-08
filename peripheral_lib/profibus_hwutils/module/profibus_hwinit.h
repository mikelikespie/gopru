/*
 * @file    profibus.h 
 * @brief   Describes the interface to the profibus module.
 * @version alpha3
 *
 * The profibus library which this header file interfaces had a kernel module
 * associated with it (profibus.ko), which needs to be loaded if calls to this
 * library are not to fail.
 *
 * The module is inserted like, for instance:
 * 
 * @verbatim /sbin/insmod profibus.ko  @endverbatim
 *
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _PROFIBUS_HWINIT_H
#define _PROFIBUS_HWINIT_H

#include <linux/ioctl.h>

#if defined (__cplusplus)
extern "C"
{
#endif

#define DRIVER_MAJOR 0		// define major number if 0 driver search for a free number

#define BAUD_9K 0		/* 9.6 KBaud */
#define BAUD_19K 1		/* 19.2 KBaud */
#define BAUD_93K 2		/* 93.75 KBaud */
#define BAUD_187K 3		/* 187.5 KBaud */
#define BAUD_500K 4		/* 500 KBaud */
#define BAUD_1_5M 5		/*  1,5 MBaud */
#define BAUD_3M 6		/*  3 MBaud */
#define BAUD_6M 7		/*  6 MBaud */
#define BAUD_12M 8		/* 12 MBaud  */

/* UART I/O offsets */
#define UART_RBR           0x00
#define UART_THR           0x00
#define UART_IER           0x04
#define UART_IIR           0x08
#define UART_FCR           0x08
#define UART_LCR           0x0C
#define UART_MSR           0x10
#define UART_LSR           0x14
#define UART_DLL           0x20
#define UART_DLH           0x24
#define UART_PID1          0x28
#define UART_PID2          0x2C
#define UART_PWREMU_MGMT   0x30
#define UART_MDR	    0x34
#define UART_IRQ_EN_RX		0x01
#define UART_IRQ_EN_TX		0x02
#define UART_IRQ_DISABLE	0x00
#define UART_IIR_THR_EMPTY	0x1
#define UART_IIR_RBR_FULL	0x2

/* ECAP I/O offsets */
#define ECCTL1	0x28		// Capture Control Register 1
#define ECCTL2	0x2A		// Capture Control Register 2
#define ECEINT	0x2C		// Capture Interrupt Enable Register
#define ECFLG	0x2E		// Capture Interrupt Flag Register
#define ECCLR	0x30		// Capture Interrupt Clear Register
#define ECFRC	0x32		// Capture Interrupt Force Register

#define PINMUX0		    0x20
#define PINMUX1		    0x24
#define PINMUX2		    0x28
#define PINMUX3		    0x2C
#define PINMUX4		    0x30
#define PINMUX5		    0x34
#define PINMUX6		    0x38
#define PINMUX7		    0x3C
#define PINMUX8		    0x40
#define PINMUX11	    0x4C
#define PINMUX16	    0x60


#define UART1_BASE           0x01D0C000
#define UART2_BASE           0x01D0D000

#define ECAP0_BASE	0x01F06000

#define SYS_BASE            0x01C14100

//define magic number for ioctl function
#define PROFIBUS_IOC_MAGIC  0xE1


#define PROFIBUS_IOCUART_INIT_BAUDRATE         _IOW(PROFIBUS_IOC_MAGIC, 1, unsigned int)
#define PROFIBUS_IOCECAP_INIT_TIMER            _IO(PROFIBUS_IOC_MAGIC, 2)
#define PROFIBUS_IOCECAP_GET_CLOCK             _IOR(PROFIBUS_IOC_MAGIC, 3, unsigned int)
#define PROFIBUS_IOCTXGPIO_INIT                _IO(PROFIBUS_IOC_MAGIC, 4)
#define PROFIBUS_IOCTXGPIO_ENABLE              _IOW(PROFIBUS_IOC_MAGIC, 5, unsigned int)




#if defined (__cplusplus)
}
#endif
#endif				// _PROFIBUS_H
