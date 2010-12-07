/*
 * profibus_hwinit.c 
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed .as is. WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/*
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2009
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/device.h>

#include <asm/page.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <asm/io.h>
#include<asm/system.h>
#include <asm/irq.h>

#include <linux/completion.h>


#include <linux/clk.h>

#include "profibus_hwinit.h"

#include <mach/mux.h>
#include <linux/gpio.h>

#define DA850_UART1_RTS_PIN GPIO_TO_PIN(0, 11)
#define DA850_UART2_RTS_PIN GPIO_TO_PIN(0, 9)

#ifdef __DEBUG
#define __D(fmt, args...) printk(KERN_DEBUG "profibus_hwinit Debug: " fmt, ## args)
#else
#define __D(fmt, args...)
#endif

#define __E(fmt, args...) printk(KERN_ERR "profibus_hwinit Error: " fmt, ## args)

static unsigned int profibus_uartnum = 1;
module_param_named (uartnum, profibus_uartnum, uint, S_IRUGO);
MODULE_PARM_DESC (uartnum, "UART for Profibus (1 or 2)");

static int major = 0;
static struct clk *ecap_clk;
static unsigned int profibus_uart_base = 0;
#if (USE_UDEV==1)
static struct class *profibus_class;
#endif //USE_UDEV


/* Forward declaration of system calls */
static int ioctl (struct inode *inode, struct file *filp, unsigned int cmd,
		  unsigned long args);
//static int mmap (struct file *filp, struct vm_area_struct *vma);
static int open (struct inode *inode, struct file *filp);
static int release (struct inode *inode, struct file *filp);
static struct file_operations profibus_fxns = {
  .owner = THIS_MODULE,
ioctl:ioctl,
//mmap:mmap,
open:open,
release:release
};


static inline unsigned int
syspinmux_read_reg (int offset)
{
  return (unsigned int) __raw_readl (IO_ADDRESS (SYS_BASE) + offset);
}

static inline void
syspinmux_write_reg (int offset, int value)
{
  __raw_writel (value, IO_ADDRESS (SYS_BASE) + offset);
}

static inline unsigned int
serial_read_reg (int offset)
{
  return (unsigned int) __raw_readl (IO_ADDRESS (profibus_uart_base) +
				     offset);
}

static inline void
serial_write_reg (int offset, int value)
{
  __raw_writel (value, IO_ADDRESS (profibus_uart_base) + offset);
}

static inline unsigned int
ecap_readw_reg (int offset)
{
  return (unsigned int) __raw_readw (IO_ADDRESS (ECAP0_BASE) + offset);
}

static inline void
ecap_writew_reg (int offset, int value)
{
  __raw_writew (value, IO_ADDRESS (ECAP0_BASE) + offset);

}

static inline unsigned int
ecap_read_reg (int offset)
{
  return (unsigned int) __raw_readl (IO_ADDRESS (ECAP0_BASE) + offset);
}

static inline void
ecap_write_reg (int offset, int value)
{
  __raw_writel (value, IO_ADDRESS (ECAP0_BASE) + offset);

}


static void
TXGPIOModuleInit (void)
{
  if (profibus_uartnum == 2)
    {
      gpio_direction_output (DA850_UART2_RTS_PIN, 0);
    }
  else
    {
      gpio_direction_output (DA850_UART1_RTS_PIN, 0);
    }
}

static void
TXGPIOEnable (unsigned int enable)
{
  if (profibus_uartnum == 2)
    {
      gpio_set_value (DA850_UART2_RTS_PIN, enable);
    }
  else
    {
      gpio_direction_output (DA850_UART1_RTS_PIN, enable);
    }
}



static void
UARTPinMuxInit (void)
{
  unsigned int regval;

  /* Set PRU0_R30 to zero */
  __raw_writew (0, IO_ADDRESS (0x01C37400) + 0x78);

  if (profibus_uartnum == 2)
    {
      // Configure Pinmux (Enable UART2 TXD & RXD)
      regval = syspinmux_read_reg (PINMUX4);
      regval &= ~(0xFF << 16);
      regval |= (0x22 << 16);
      syspinmux_write_reg (PINMUX4, regval);
      // Freon: Configure pinmux (set PRU0_R30[16] -- UART2_RTS output)
      regval = syspinmux_read_reg (PINMUX0);
      regval &= ~(0xF00 << 16);
      regval |= (0x200 << 16);
      syspinmux_write_reg (PINMUX0, regval);
    }
  else
    {
      // Configure Pinmux (Enable UART1 TXD & RXD)
      regval = syspinmux_read_reg (PINMUX4);
      regval &= ~(0xFF << 24);
      regval |= (0x22 << 24);
      syspinmux_write_reg (PINMUX4, regval);
      // Freon: Configure pinmux (set PRU0_R30[18] -- UART1_RTS output)
      regval = syspinmux_read_reg (PINMUX0);
      regval &= ~(0xF << 16);
      regval |= (0x2 << 16);
      syspinmux_write_reg (PINMUX0, regval);
    }
}

static void
UARTSetBaudRate (unsigned int DLL, unsigned int DLH)
{
  // Reset UART TX & RX components
  serial_write_reg (UART_PWREMU_MGMT, 0);
  //For 13x reference clock
  serial_write_reg (UART_MDR, 1);
  serial_write_reg (UART_DLL, DLL);
  serial_write_reg (UART_DLH, DLH);
  // Non-FIFO mode
  serial_write_reg (UART_FCR, 0);
  // Clear UART TX & RX FIFOs
  serial_write_reg (UART_FCR, 6);
  // 8-bit words, 1 STOP bit generated, Even Parity
  serial_write_reg (UART_LCR, 0x1B);
  // Enable UART TX & RX components
  serial_write_reg (UART_PWREMU_MGMT, 0x6001);
}

/* Profibus peripheral initialization */
static void
UARTModuleInit (unsigned int baudrate)
{
  if (profibus_uartnum == 2)
    {
      profibus_uart_base = UART2_BASE;
    }
  else
    {
      profibus_uart_base = UART1_BASE;
    }

  UARTPinMuxInit ();
  switch (baudrate)
    {
    case BAUD_12M:
      {
	UARTSetBaudRate (1, 0);
      }
      break;
    case BAUD_6M:
      {
	UARTSetBaudRate (2, 0);
      }
      break;
    case BAUD_3M:
      {
	UARTSetBaudRate (4, 0);
      }
      break;
    case BAUD_1_5M:
      {
	UARTSetBaudRate (8, 0);
      }
      break;

    case BAUD_500K:
      {
	UARTSetBaudRate (0x18, 0);
      }
      break;
    case BAUD_187K:
      {
	UARTSetBaudRate (0x40, 0);
      }
      break;
    case BAUD_93K:
      {
	UARTSetBaudRate (0x80, 0);
      }
      break;
    case BAUD_19K:
      {
	UARTSetBaudRate (0x71, 0x2);
      }
      break;
    case BAUD_9K:
      {
	UARTSetBaudRate (0xE2, 0x4);
      }
      break;

    default:
      {
	__E ("UARTModuleInit: baudrate :%d not supported\n", baudrate);
      }
      break;
    }
}


static void
ECAPModuleInit (void)
{
  // Disable all interrupts on ECAP interface
  ecap_writew_reg (ECEINT, 0);
  // Clear spurious eCAP interrupt flags
  ecap_writew_reg (ECCLR, 0xFFFF);

  // Bypass prescaler and disable REGISTER load on capture event
  ecap_writew_reg (ECCTL1, 0x8100);
  // Select Capture Mode, Continuous Mode and free run TS Counter
  ecap_writew_reg (ECCTL2, 0x86);
  // Enable Counter overflow as Interrupt sources
  ecap_writew_reg (ECEINT, 0x20);

}



static int
ioctl (struct inode *inode, struct file *filp, unsigned int cmd,
       unsigned long args)
{
  unsigned int __user *argp = (unsigned int __user *) args;

  switch (cmd)
    {

    case PROFIBUS_IOCUART_INIT_BAUDRATE:

      {
	unsigned int baudrate;
	if (get_user (baudrate, argp))
	  {
	    __E ("UART_INIT_BAUDRATE - get_user failed\n");
	    return -EFAULT;
	  }
	__D ("UART_INIT_BAUDRATE received: %d\n", baudrate);
	UARTModuleInit (baudrate);
	return 0;
      }
      break;

    case PROFIBUS_IOCECAP_INIT_TIMER:
      {
	__D ("ECAP_INIT_TIMER received.\n");
	ECAPModuleInit ();
	return 0;
      }
      break;

    case PROFIBUS_IOCECAP_GET_CLOCK:
      {
	unsigned int ecapclkfreq = clk_get_rate (ecap_clk);
	__D ("ECAP_GET_CLOCK received.\n");
	if (put_user (ecapclkfreq, argp))
	  {
	    __E ("ECAP_GET_CLOCK - put_user failed\n");
	    return -EFAULT;
	  }
	return 0;
      }
      break;

    case PROFIBUS_IOCTXGPIO_INIT:
      {
	__D ("TXGPIO_INIT received.\n");
	TXGPIOModuleInit ();
	return 0;
      }
      break;

    case PROFIBUS_IOCTXGPIO_ENABLE:
      {
	unsigned int enable;
	if (get_user (enable, argp))
	  {
	    __E ("TXGPIO_ENABLE - get_user failed\n");
	    return -EFAULT;
	  }
	__D ("TXGPIO_ENABLE received.\n");
	TXGPIOEnable (enable);
      }
      break;

    default:
      __E ("Unknown ioctl received = %d.\n", cmd);
      return -EINVAL;
    }
  return 0;
}


static int
open (struct inode *inode, struct file *filp)
{
  __D ("open: called.\n");
  return 0;
}


static int
release (struct inode *inode, struct file *filp)
{
  __D ("close: called.");
  return 0;
}


int __init
profibus_init (void)
{
  __D ("** profibus kernel module built: " __DATE__ " " __TIME__ "\n");
  major = register_chrdev (DRIVER_MAJOR, "profibus_hwinit", &profibus_fxns);
  if (major < 0)
    {
      __E ("Failed to allocate major number.\n");
      return -ENODEV;
    }
#if (USE_UDEV==1)
  profibus_class = class_create (THIS_MODULE, "profibus_hwinit");
  if (IS_ERR (profibus_class))
    {
      __E ("Error creating profibus device class.\n");
      return -EIO;
    }
  device_create (profibus_class, NULL, MKDEV (major, 0), NULL,
		 "profibus_hwinit");

#endif // USE_UDEV
  ecap_clk = clk_get (NULL, "ecap0");
  if (IS_ERR (ecap_clk))
    __E ("Failed to get ECAP clock\n");
  else
    {
      clk_enable (ecap_clk);
      __D ("ECAP clk= %lu\n", clk_get_rate (ecap_clk));
    }
#ifdef CONFIG_MACH_MITYOMAPL138
  if (profibus_uartnum == 2)
    {
      davinci_cfg_reg (DA850_GPIO0_9);
      if (gpio_request (DA850_UART2_RTS_PIN, "UART2 RTS\n") < 0)
	__E ("UART2 RTS gpio ownership failed\n");
    }
  else
#endif
    {
      davinci_cfg_reg (DA850_GPIO0_11);
      if (gpio_request (DA850_UART1_RTS_PIN, "UART1 RTS\n") < 0)
	__E ("UART1 RTS gpio ownership failed\n");
    }
  return 0;
}


void __exit
profibus_exit (void)
{

  clk_disable (ecap_clk);
  if (profibus_uartnum == 2)
    {
      gpio_free (DA850_UART2_RTS_PIN);

    }
  else
    {
      gpio_free (DA850_UART1_RTS_PIN);
    }
#if (USE_UDEV==1)
  device_destroy (profibus_class, MKDEV (major, 0));
  class_destroy (profibus_class);
#endif // USE_UDEV
  unregister_chrdev (major, "profibus_hwinit");
}

module_init (profibus_init);
module_exit (profibus_exit);


MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Texas Instruments");
MODULE_DESCRIPTION ("Profibus h/w initialization");
