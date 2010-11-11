/*
 * pru/hal/uart/src/suart_api.c
 *
 * Copyright (C) 2010 Texas Instruments Incorporated
 * Author: Jitendra Kumar <jitendra@mistralsolutions.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as  published by the
 * Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any kind,
 * whether express or implied; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

/*
 *====================
 * Includes
 *====================
 */

#include "suart_api.h"
#include "suart_pru_regs.h"
#include "pru.h"
#include "omapl_suart_board.h"
#include "suart_utils.h"
#include "suart_err.h"

static unsigned char gUartStatuTable[8];
static arm_pru_iomap pru_arm_iomap;
static int suart_set_pru_id (unsigned int pru_no);
/*
 * suart Initialization routine 
 */
short pru_softuart_init(unsigned int txBaudValue,
			unsigned int rxBaudValue,
			unsigned int oversampling,
			unsigned char *pru_suart_emu_code,
			unsigned int fw_size, arm_pru_iomap * arm_iomap_pru)
{
	unsigned int omapl_addr;
	short status = PRU_SUART_SUCCESS;
	short idx;
	short retval;

	pru_arm_iomap.pru_io_addr = arm_iomap_pru->pru_io_addr;
	pru_arm_iomap.mcasp_io_addr = arm_iomap_pru->mcasp_io_addr;
	pru_arm_iomap.psc0_io_addr = arm_iomap_pru->psc0_io_addr;
	pru_arm_iomap.psc1_io_addr = arm_iomap_pru->psc1_io_addr;
	pru_arm_iomap.syscfg_io_addr = arm_iomap_pru->syscfg_io_addr;
	pru_arm_iomap.pFifoBufferPhysBase = arm_iomap_pru->pFifoBufferPhysBase;
	pru_arm_iomap.pFifoBufferVirtBase = arm_iomap_pru->pFifoBufferVirtBase;

	omapl_addr = (unsigned int)arm_iomap_pru->syscfg_io_addr;

	omapl_addr = (unsigned int)arm_iomap_pru->psc1_io_addr;
	suart_mcasp_psc_enable(omapl_addr);

	omapl_addr = (unsigned int)arm_iomap_pru->mcasp_io_addr;
	// Configure McASP0
	suart_mcasp_config(omapl_addr, txBaudValue, rxBaudValue, oversampling,
			   arm_iomap_pru);
    retval = arm_to_pru_intr_init();
    if (-1 == retval) {
        return status;
    }
	

	pru_enable(0, arm_iomap_pru);
	pru_enable(1, arm_iomap_pru);

	pru_load(PRU_NUM0, (unsigned int *)pru_suart_emu_code,
		 (fw_size / sizeof(unsigned int)), arm_iomap_pru);
	pru_load(PRU_NUM1, (unsigned int *)pru_suart_emu_code,
		 (fw_size / sizeof(unsigned int)), arm_iomap_pru);

	suart_set_pru_id(0);
	suart_set_pru_id(1);

	pru_run(PRU_NUM0, arm_iomap_pru);
	// pru_run(PRU_NUM1, arm_iomap_pru); 

	/* Initialize gUartStatuTable */
	for (idx = 0; idx < 8; idx++) {
		gUartStatuTable[idx] = ePRU_SUART_UART_FREE;
	}
	
	return status;
}

void pru_set_fifo_timeout(Uint32 timeout)
{
	/* PRU 0 */
	pru_ram_write_data(PRU_SUART_PRU0_IDLE_TIMEOUT_OFFSET, 
				(Uint8 *)&timeout, 2, &pru_arm_iomap);

    /* PRU 1 */
	pru_ram_write_data(PRU_SUART_PRU1_IDLE_TIMEOUT_OFFSET, 
				(Uint8 *)&timeout, 2, &pru_arm_iomap);
}

short pru_softuart_deinit(void)
{
	unsigned int offset;
    short s16retval = 0;
    unsigned int u32value = 0;

	offset =
        (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_STATCLRINT1 &
                               0xFFFF);
    u32value = 0xFFFFFFFF;
    s16retval =
        pru_ram_write_data_4byte(offset, (unsigned int *)&u32value, 1);
    if (-1 == s16retval) {
        return -1;
    }
	offset =
        (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_STATCLRINT0 &
                               0xFFFF);
    u32value = 0xFFFFFFFF;
    s16retval =
        pru_ram_write_data_4byte(offset, (unsigned int *)&u32value, 1);
    if (-1 == s16retval) {
        return -1;
    }
	pru_disable(&pru_arm_iomap);

	return PRU_SUART_SUCCESS;
}
/*
 * suart Instance open routine
 */
short pru_softuart_open(suart_handle hSuart)
{
	short status = PRU_SUART_SUCCESS;
	
	switch (hSuart->uartNum) {
		/* ************ PRU 0 ************** */
	case PRU_SUART_UART1:
		if (gUartStatuTable[PRU_SUART_UART1 - 1] ==
		    ePRU_SUART_UART_IN_USE) {
			status = SUART_UART_IN_USE;
			return status;
		} else {
			hSuart->uartStatus = ePRU_SUART_UART_IN_USE;
			hSuart->uartType = PRU_SUART1_CONFIG_DUPLEX;
			hSuart->uartTxChannel = PRU_SUART1_CONFIG_TX_SER;
			hSuart->uartRxChannel = PRU_SUART1_CONFIG_RX_SER;

			gUartStatuTable[PRU_SUART_UART1 - 1] =
			    ePRU_SUART_UART_IN_USE;
		}

		break;

	case PRU_SUART_UART2:
		if (gUartStatuTable[PRU_SUART_UART2 - 1] ==
		    ePRU_SUART_UART_IN_USE) {
			status = SUART_UART_IN_USE;
			return status;
		} else {

			hSuart->uartStatus = ePRU_SUART_UART_IN_USE;
            hSuart->uartType = PRU_SUART2_CONFIG_DUPLEX;
            hSuart->uartTxChannel = PRU_SUART2_CONFIG_TX_SER;
            hSuart->uartRxChannel = PRU_SUART2_CONFIG_RX_SER;

			gUartStatuTable[PRU_SUART_UART2 - 1] =
			    ePRU_SUART_UART_IN_USE;
		}

		break;

	case PRU_SUART_UART3:
		if (gUartStatuTable[PRU_SUART_UART3 - 1] ==
		    ePRU_SUART_UART_IN_USE) {
			status = SUART_UART_IN_USE;
			return status;
		} else {

			hSuart->uartStatus = ePRU_SUART_UART_IN_USE;
            hSuart->uartType = PRU_SUART3_CONFIG_DUPLEX;
            hSuart->uartTxChannel = PRU_SUART3_CONFIG_TX_SER;
            hSuart->uartRxChannel = PRU_SUART3_CONFIG_RX_SER;

			gUartStatuTable[PRU_SUART_UART3 - 1] =
			    ePRU_SUART_UART_IN_USE;
		}

		break;

	case PRU_SUART_UART4:
		if (gUartStatuTable[PRU_SUART_UART4 - 1] ==
		    ePRU_SUART_UART_IN_USE) {
			status = SUART_UART_IN_USE;
			return status;
		} else {

			hSuart->uartStatus = ePRU_SUART_UART_IN_USE;
            hSuart->uartType = PRU_SUART4_CONFIG_DUPLEX;
            hSuart->uartTxChannel = PRU_SUART4_CONFIG_TX_SER;
            hSuart->uartRxChannel = PRU_SUART4_CONFIG_RX_SER;

			gUartStatuTable[PRU_SUART_UART4 - 1] =
			    ePRU_SUART_UART_IN_USE;
		}
		break;

		/* ************ PRU 1 ************** */
	case PRU_SUART_UART5:
		if (gUartStatuTable[PRU_SUART_UART5 - 1] ==
		    ePRU_SUART_UART_IN_USE) {
			status = SUART_UART_IN_USE;
			return status;
		} else {
			hSuart->uartStatus = ePRU_SUART_UART_IN_USE;
            hSuart->uartType = PRU_SUART5_CONFIG_DUPLEX;
            hSuart->uartTxChannel = PRU_SUART5_CONFIG_TX_SER;
            hSuart->uartRxChannel = PRU_SUART5_CONFIG_RX_SER;

			gUartStatuTable[PRU_SUART_UART5 - 1] =
			    ePRU_SUART_UART_IN_USE;
		}
		break;

	case PRU_SUART_UART6:
		if (gUartStatuTable[PRU_SUART_UART6 - 1] ==
		    ePRU_SUART_UART_IN_USE) {
			status = SUART_UART_IN_USE;
			return status;
		} else {
			hSuart->uartStatus = ePRU_SUART_UART_IN_USE;
            hSuart->uartType = PRU_SUART6_CONFIG_DUPLEX;
            hSuart->uartTxChannel = PRU_SUART6_CONFIG_TX_SER;
            hSuart->uartRxChannel = PRU_SUART6_CONFIG_RX_SER;

			gUartStatuTable[PRU_SUART_UART6 - 1] =
			    ePRU_SUART_UART_IN_USE;
		}

		break;

	case PRU_SUART_UART7:
		if (gUartStatuTable[PRU_SUART_UART7 - 1] ==
		    ePRU_SUART_UART_IN_USE) {
			status = SUART_UART_IN_USE;
			return status;
		} else {
			hSuart->uartStatus = ePRU_SUART_UART_IN_USE;
            hSuart->uartType = PRU_SUART7_CONFIG_DUPLEX;
            hSuart->uartTxChannel = PRU_SUART7_CONFIG_TX_SER;
            hSuart->uartRxChannel = PRU_SUART7_CONFIG_RX_SER;

			gUartStatuTable[PRU_SUART_UART7 - 1] =
			    ePRU_SUART_UART_IN_USE;
		}
		break;

	case PRU_SUART_UART8:
		if (gUartStatuTable[PRU_SUART_UART8 - 1] ==
		    ePRU_SUART_UART_IN_USE) {
			status = SUART_UART_IN_USE;
			return status;
		} else {
			hSuart->uartStatus = ePRU_SUART_UART_IN_USE;
            hSuart->uartType = PRU_SUART8_CONFIG_DUPLEX;
            hSuart->uartTxChannel = PRU_SUART8_CONFIG_TX_SER;
            hSuart->uartRxChannel = PRU_SUART8_CONFIG_RX_SER;

			gUartStatuTable[PRU_SUART_UART8 - 1] =
			    ePRU_SUART_UART_IN_USE;
		}
		break;

	default:
		/* return invalid UART */
		status = SUART_INVALID_UART_NUM;
		break;
	}
	return (status);
}

/*
 * suart instance close routine 
 */
short pru_softuart_close(suart_handle hUart)
{
	short status = SUART_SUCCESS;

	if (hUart == NULL) {
		return (PRU_SUART_ERR_HANDLE_INVALID);
	} else {
		gUartStatuTable[hUart->uartNum - 1] = ePRU_SUART_UART_FREE;
		/* Reset the Instance to Invalid */
		hUart->uartNum = PRU_SUART_UARTx_INVALID;
		hUart->uartStatus = ePRU_SUART_UART_FREE;
	}
	return (status);
}

/*
 * suart routine for setting relative baud rate 
 */
short pru_softuart_setbaud
    (suart_handle hUart,
     unsigned short txClkDivisor, 
	 unsigned short rxClkDivisor) 
{
	unsigned int offset;
	unsigned int pruOffset;
	short status = SUART_SUCCESS;
	unsigned short chNum;
	unsigned short regval = 0;

	if (hUart == NULL) 
	{
		return (PRU_SUART_ERR_HANDLE_INVALID);
	}

	/* Set the clock divisor value into the McASP */
	if ((txClkDivisor > 385) || (txClkDivisor == 0)) 
	{
		return SUART_INVALID_CLKDIVISOR;
	}

	if ((rxClkDivisor > 385) || (rxClkDivisor == 0)) 
	{
		return SUART_INVALID_CLKDIVISOR;
	}

	/* channel starts from 0 and uart instance starts from 1 */
	chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if (hUart->uartNum <= 4) 
	{
		/* PRU0 */
		pruOffset = PRU_SUART_PRU0_CH0_OFFSET;
	} else {
		/* PRU1 */
		pruOffset = PRU_SUART_PRU1_CH0_OFFSET;
		/* First 8 channel corresponds to PRU0 */
		chNum -= 8;
	}

//	/* channel starts from 0 and uart instance starts from 1 */
//	chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if (txClkDivisor != 0) 
	{
		offset =
		    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
		    PRU_SUART_CH_CONFIG1_OFFSET;
		pru_ram_read_data(offset, (Uint8 *) &regval, 2,
				   &pru_arm_iomap);
		regval &= (~0x3FF);
		regval |= txClkDivisor;
		pru_ram_write_data(offset, (Uint8 *) &regval, 2,
				   &pru_arm_iomap);
	}

	chNum++;
	regval = 0;
	if (rxClkDivisor != 0) 
	{
		offset =
		    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
		    PRU_SUART_CH_CONFIG1_OFFSET;
		pru_ram_read_data(offset, (Uint8 *) &regval, 2,
				   &pru_arm_iomap);
		regval &= (~0x3FF);
		regval |= txClkDivisor;
		pru_ram_write_data(offset, (Uint8 *) &regval, 2,
				   &pru_arm_iomap);
	}
	return status;
}

/*
 * suart routine for setting number of bits per character for a specific uart 
 */
short pru_softuart_setdatabits
    (suart_handle hUart, unsigned short txDataBits, unsigned short rxDataBits) {
	unsigned int offset;
	unsigned int pruOffset;
	short status = SUART_SUCCESS;
	unsigned short chNum;
    unsigned int  reg_val;

	if (hUart == NULL) {
		return (PRU_SUART_ERR_HANDLE_INVALID);
	}

	/*
	 * NOTE:
	 * The supported data bits are 6, 7, 8, 9, 10, 11 and 12 bits per character
	 */

	if ((txDataBits < ePRU_SUART_DATA_BITS6) || (txDataBits > ePRU_SUART_DATA_BITS12)) {
		return PRU_SUART_ERR_PARAMETER_INVALID;
	}

	if ((rxDataBits < ePRU_SUART_DATA_BITS6) || (rxDataBits > ePRU_SUART_DATA_BITS12)) {
		return PRU_SUART_ERR_PARAMETER_INVALID;
	}

    /* channel starts from 0 and uart instance starts from 1 */
    chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if (hUart->uartNum <= 4) {
		/* PRU0 */
		pruOffset = PRU_SUART_PRU0_CH0_OFFSET;
	} else {
		/* PRU1 */
		pruOffset = PRU_SUART_PRU1_CH0_OFFSET;
		/* First 8 channel corresponds to PRU0 */
		chNum -= 8;
	}

//	/* channel starts from 0 and uart instance starts from 1 */
//	chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if (txDataBits != 0) {
		offset =
		    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
		    PRU_SUART_CH_CONFIG2_OFFSET;
		pru_ram_read_data(offset, (Uint8 *) &reg_val, 1,
				   &pru_arm_iomap);

        reg_val &= ~(0xF);
        reg_val |= txDataBits;
		pru_ram_write_data(offset, (Uint8 *) &reg_val, 1,
				   &pru_arm_iomap);
	}

	chNum++;
	if (rxDataBits != 0) {
		offset =
		    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
		    PRU_SUART_CH_CONFIG2_OFFSET;

        pru_ram_read_data(offset, (Uint8 *) &reg_val, 1,
                   &pru_arm_iomap);

        reg_val &= ~(0xF);
        reg_val |= rxDataBits;

		pru_ram_write_data(offset, (Uint8 *) & rxDataBits, 1,
				   &pru_arm_iomap);
	}

	return (status);
}

/*
 * suart routine to configure specific uart 
 */
short pru_softuart_setconfig(suart_handle hUart, suart_config * configUart)
{
	unsigned int offset;
	unsigned int pruOffset;
	short status = SUART_SUCCESS;
	unsigned short chNum;
	unsigned short regVal = 0;
	if (hUart == NULL) {
		return (PRU_SUART_ERR_HANDLE_INVALID);
	}

	/*
	 * NOTE:
	 * Dependent baud rate for the given UART, the value MUST BE LESS THAN OR 
	 * EQUAL TO 64, preScalarValue <= 64
	 */
	/* Validate the value of relative buad rate */
	if ((configUart->txClkDivisor > 384) || (configUart->rxClkDivisor > 384)) {
		return SUART_INVALID_CLKDIVISOR;
	}
	/* Validate the bits per character */
	if ((configUart->txBitsPerChar < 8) || (configUart->txBitsPerChar > 14)) {
		return PRU_SUART_ERR_PARAMETER_INVALID;
	}

	if ((configUart->rxBitsPerChar < 8) || (configUart->rxBitsPerChar > 14)) {
		return PRU_SUART_ERR_PARAMETER_INVALID;
	}

    /* channel starts from 0 and uart instance starts from 1 */
    chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if (hUart->uartNum <= 4) {
		/* PRU0 */
		pruOffset = PRU_SUART_PRU0_CH0_OFFSET;
	} else {
		/* PRU1 */
		pruOffset = PRU_SUART_PRU1_CH0_OFFSET;
		chNum -= 8;
	}

//	/* channel starts from 0 and uart instance starts from 1 */
//	chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	/* Configuring the Transmit part of the given UART */
	if (configUart->TXSerializer != PRU_SUART_SERIALIZER_NONE) {
		suart_mcasp_tx_serialzier_set (configUart->TXSerializer, &pru_arm_iomap);
		/* Configuring TX serializer  */
		offset =
		    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
		    PRU_SUART_CH_CTRL_OFFSET;
		pru_ram_read_data(offset, (Uint8 *) & regVal, 2,
				  &pru_arm_iomap);
		regVal =
		    regVal | (configUart->TXSerializer <<
			      PRU_SUART_CH_CTRL_SR_SHIFT);
		pru_ram_write_data(offset, (Uint8 *) & regVal, 2,
				   &pru_arm_iomap);
		/* Configuring the Transmit part of the given UART */
		/* Configuring TX prescalar value */
		offset =
		    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
		    PRU_SUART_CH_CONFIG1_OFFSET;
		pru_ram_read_data(offset, (Uint8 *) & regVal, 2,
				  &pru_arm_iomap);
		regVal =
		    regVal | (configUart->txClkDivisor <<
			      PRU_SUART_CH_CONFIG1_DIVISOR_SHIFT);
		pru_ram_write_data(offset, (Uint8 *) & regVal, 2,
				   &pru_arm_iomap);
		/* Configuring TX bits per character value */
		offset =
		    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
		    PRU_SUART_CH_CONFIG2_OFFSET;
		pru_ram_read_data(offset, (Uint8 *) & regVal, 2,
				  &pru_arm_iomap);
		regVal =
		    regVal | (configUart->txBitsPerChar <<
			      PRU_SUART_CH_CONFIG2_BITPERCHAR_SHIFT);
		pru_ram_write_data(offset, (Uint8 *) & regVal, 2,
				   &pru_arm_iomap);
	}

	chNum++;
	/* Configuring the Transmit part of the given UART */
	if (configUart->RXSerializer != PRU_SUART_SERIALIZER_NONE) {
		/* Configuring RX serializer  */
		offset =
		    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
		    PRU_SUART_CH_CTRL_OFFSET;
		pru_ram_read_data(offset, (Uint8 *) & regVal, 2,
				  &pru_arm_iomap);
		regVal =
		    pruOffset | (configUart->RXSerializer <<
				 PRU_SUART_CH_CTRL_SR_SHIFT);
		pru_ram_write_data(offset, (Uint8 *) & regVal, 2,
				   &pru_arm_iomap);

		/* Configuring RX prescalar value and Oversampling */
		offset =
		    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
		    PRU_SUART_CH_CONFIG1_OFFSET;
		pru_ram_read_data(offset, (Uint8 *) & regVal, 2,
				  &pru_arm_iomap);
		regVal =
		    regVal | (configUart->rxClkDivisor <<
			      PRU_SUART_CH_CONFIG1_DIVISOR_SHIFT) |
		    (configUart->Oversampling <<
		     PRU_SUART_CH_CONFIG1_OVS_SHIFT);
		pru_ram_write_data(offset, (Uint8 *) & regVal, 2,
				   &pru_arm_iomap);

		/* Configuring RX bits per character value */
		offset =
		    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
		    PRU_SUART_CH_CONFIG2_OFFSET;
		pru_ram_read_data(offset, (Uint8 *) & regVal, 2,
				  &pru_arm_iomap);
		regVal =
		    regVal | (configUart->rxBitsPerChar <<
			      PRU_SUART_CH_CONFIG1_DIVISOR_SHIFT);
		pru_ram_write_data(offset, (Uint8 *) & regVal, 2,
				   &pru_arm_iomap);
	}
	return (status);
}

/*
 * suart routine for getting the number of bytes transfered
 */
short pru_softuart_getTxDataLen(suart_handle hUart)
{
	unsigned int offset;
	unsigned int pruOffset;
	unsigned short chNum;
	unsigned short u16ReadValue = 0;

	if (hUart == NULL) {
		return (PRU_SUART_ERR_HANDLE_INVALID);
	}

    /* channel starts from 0 and uart instance starts from 1 */
    chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if (hUart->uartNum <= 4) {
		/* PRU0 */
		pruOffset = PRU_SUART_PRU0_CH0_OFFSET;
	} else {
		/* PRU1 */
		pruOffset = PRU_SUART_PRU1_CH0_OFFSET;
		/* First 8 channel corresponds to PRU0 */
		chNum -= 8;
	}

//	/* channel starts from 0 and uart instance starts from 1 */
//	chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	/* Transmit channel number is (UartNum * 2) - 2  */

	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_CONFIG2_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) & u16ReadValue, 2, &pru_arm_iomap);

	u16ReadValue = ((u16ReadValue & PRU_SUART_CH_CONFIG1_DIVISOR_MASK) >>
			PRU_SUART_CH_CONFIG2_DATALEN_SHIFT);
	return (u16ReadValue);
}

/*
 * suart routine for getting the number of bytes received
 */
short pru_softuart_getRxDataLen(suart_handle hUart)
{
	unsigned int offset;
	unsigned int pruOffset;
	unsigned short chNum;
	unsigned short u16ReadValue = 0;

	if (hUart == NULL) {
		return (PRU_SUART_ERR_HANDLE_INVALID);
	}

    /* channel starts from 0 and uart instance starts from 1 */
    chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if (hUart->uartNum <= 4) {
		/* PRU0 */
		pruOffset = PRU_SUART_PRU0_CH0_OFFSET;
	} else {
		/* PRU1 */
		pruOffset = PRU_SUART_PRU1_CH0_OFFSET;
		/* First 8 channel corresponds to PRU0 */
		chNum -= 8;
	}
    
//	chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 1;

    chNum++;
//	/* Transmit channel number is (UartNum * 2) - 1  */
	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_CONFIG2_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) & u16ReadValue, 2, &pru_arm_iomap);

	u16ReadValue = ((u16ReadValue & PRU_SUART_CH_CONFIG1_DIVISOR_MASK) >>
			PRU_SUART_CH_CONFIG2_DATALEN_SHIFT);

	return (u16ReadValue);
}

/*
 * suart routine to get the configuration information from a specific uart 
 */
short pru_softuart_getconfig(suart_handle hUart, suart_config * configUart)
{
	unsigned int offset;
	unsigned int pruOffset;
	unsigned short chNum;
	unsigned short regVal = 0;
	short status = SUART_SUCCESS;

	if (hUart == NULL) {
		return (PRU_SUART_ERR_HANDLE_INVALID);
	}

	/*
	 * NOTE:
	 * Dependent baud rate for the given UART, the value MUST BE LESS THAN OR 
	 * EQUAL TO 64, preScalarValue <= 64
	 */

    /* channel starts from 0 and uart instance starts from 1 */
    chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if (hUart->uartNum <= 4) {
		/* PRU0 */
		pruOffset = PRU_SUART_PRU0_CH0_OFFSET;
	} else {
		/* PRU1 */
		pruOffset = PRU_SUART_PRU1_CH0_OFFSET;
		/* First 8 channel corresponds to PRU0 */
		chNum -= 8;
	}

	/* channel starts from 0 and uart instance starts from 1 */
//	chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	/* Configuring the Transmit part of the given UART */
	/* Configuring TX serializer  */
	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_CTRL_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) & regVal, 2, &pru_arm_iomap);
	configUart->TXSerializer =
	    ((regVal & PRU_SUART_CH_CTRL_SR_MASK) >>
	     PRU_SUART_CH_CTRL_SR_SHIFT);

	/* Configuring TX prescalar value */
	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_CONFIG1_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) & regVal, 2, &pru_arm_iomap);
	configUart->txClkDivisor =
	    ((regVal & PRU_SUART_CH_CONFIG1_DIVISOR_MASK) >>
	     PRU_SUART_CH_CONFIG1_DIVISOR_SHIFT);

	/* Configuring TX bits per character value */
	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_CONFIG2_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) & regVal, 2, &pru_arm_iomap);
	configUart->txBitsPerChar =
	    ((regVal & PRU_SUART_CH_CONFIG1_DIVISOR_MASK) >>
	     PRU_SUART_CH_CONFIG1_DIVISOR_SHIFT);

	chNum++;
	/* Configuring the Transmit part of the given UART */
	/* Configuring RX serializer  */
	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_CTRL_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) & regVal, 2, &pru_arm_iomap);
	configUart->RXSerializer =
	    ((regVal & PRU_SUART_CH_CTRL_SR_MASK) >>
	     PRU_SUART_CH_CTRL_SR_SHIFT);

	/* Configuring RX prescalar value and Oversampling */
	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_CONFIG1_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) & regVal, 2, &pru_arm_iomap);
	configUart->rxClkDivisor =
	    ((regVal & PRU_SUART_CH_CONFIG1_DIVISOR_MASK) >>
	     PRU_SUART_CH_CONFIG1_DIVISOR_SHIFT);
	configUart->Oversampling =
	    ((regVal & PRU_SUART_CH_CONFIG1_OVS_MASK) >>
	     PRU_SUART_CH_CONFIG1_OVS_SHIFT);

	/* Configuring RX bits per character value */
	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_CONFIG2_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) & regVal, 2, &pru_arm_iomap);
	configUart->rxBitsPerChar =
	    ((regVal & PRU_SUART_CH_CONFIG1_DIVISOR_MASK) >>
	     PRU_SUART_CH_CONFIG1_DIVISOR_SHIFT);

	return (status);
}

/*
 * suart data transmit routine 
 */
short pru_softuart_write
    (suart_handle hUart, unsigned int *ptTxDataBuf, unsigned short dataLen) {
	unsigned int offset = 0;
	unsigned int pruOffset;
	short status = SUART_SUCCESS;
	unsigned short chNum;
	unsigned short regVal = 0;
	if (hUart == NULL) {
		return (PRU_SUART_ERR_HANDLE_INVALID);
	}

    /* channel starts from 0 and uart instance starts from 1 */
    chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if (hUart->uartNum <= 4) {
		/* PRU0 */
		pruOffset = PRU_SUART_PRU0_CH0_OFFSET;
	} else {
		/* PRU1 */
		pruOffset = PRU_SUART_PRU1_CH0_OFFSET;
		/* First 8 channel corresponds to PRU0 */
		chNum -= 8;
	}

//	/* channel starts from 0 and uart instance starts from 1 */
//	chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	/* Writing data length to SUART channel register */
	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_CONFIG2_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) & regVal, 2, &pru_arm_iomap);
	regVal &= ~PRU_SUART_CH_CONFIG2_DATALEN_MASK;
	regVal = regVal | (dataLen << PRU_SUART_CH_CONFIG2_DATALEN_SHIFT);
	pru_ram_write_data(offset, (Uint8 *) & regVal, 2, &pru_arm_iomap);

	/* Writing the data pointer to channel TX data pointer */
	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_TXRXDATA_OFFSET;
	pru_ram_write_data(offset, (Uint8 *) ptTxDataBuf, 4, &pru_arm_iomap);

	/* Service Request to PRU */
	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_CTRL_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) & regVal, 2, &pru_arm_iomap);

	regVal &= ~(PRU_SUART_CH_CTRL_MODE_MASK |PRU_SUART_CH_CTRL_SREQ_MASK);

	regVal |= (PRU_SUART_CH_CTRL_TX_MODE << PRU_SUART_CH_CTRL_MODE_SHIFT) | 
			 (PRU_SUART_CH_CTRL_SREQ    <<    PRU_SUART_CH_CTRL_SREQ_SHIFT);

	pru_ram_write_data(offset, (Uint8 *) & regVal, 2, &pru_arm_iomap);

	/* generate ARM->PRU event */
	suart_arm_to_pru_intr(hUart->uartNum);

	return (status);
}

/*
 * suart data receive routine 
 */
short pru_softuart_read
    (suart_handle hUart, unsigned int *ptDataBuf, unsigned short dataLen) {
	unsigned int offset = 0;
	unsigned int pruOffset;
	short status = SUART_SUCCESS;
	unsigned short chNum;
	unsigned short regVal = 0;

	if (hUart == NULL) {
		return (PRU_SUART_ERR_HANDLE_INVALID);
	}

    /* channel starts from 0 and uart instance starts from 1 */
    chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if (hUart->uartNum <= 4) {
		/* PRU0 */
		pruOffset = PRU_SUART_PRU0_CH0_OFFSET;
	} else {
		/* PRU1 */
		pruOffset = PRU_SUART_PRU1_CH0_OFFSET;
		/* First 8 channel corresponds to PRU0 */
		chNum -= 8;
	}

//	/* channel starts from 0 and uart instance starts from 1 */
//	chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	chNum++;

	/* Writing data length to SUART channel register */
	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_CONFIG2_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) & regVal, 2, &pru_arm_iomap);
	regVal &= ~PRU_SUART_CH_CONFIG2_DATALEN_MASK;
	regVal = regVal | (dataLen << PRU_SUART_CH_CONFIG2_DATALEN_SHIFT);
	pru_ram_write_data(offset, (Uint8 *) & regVal, 2, &pru_arm_iomap);

	/* Writing the data pointer to channel RX data pointer */
	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_TXRXDATA_OFFSET;
	pru_ram_write_data(offset, (Uint8 *) ptDataBuf, 4, &pru_arm_iomap);

	/* Service Request to PRU */
	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_CTRL_OFFSET;

	
	pru_ram_read_data(offset, (Uint8 *) & regVal, 2, &pru_arm_iomap);

	regVal &= ~(PRU_SUART_CH_CTRL_MODE_MASK |PRU_SUART_CH_CTRL_SREQ_MASK);

	regVal |=  ( PRU_SUART_CH_CTRL_RX_MODE << PRU_SUART_CH_CTRL_MODE_SHIFT) | 
				(PRU_SUART_CH_CTRL_SREQ << PRU_SUART_CH_CTRL_SREQ_SHIFT);
				
	pru_ram_write_data(offset, (Uint8 *) & regVal, 2, &pru_arm_iomap);
	
	/* enable the timeout interrupt */
	suart_intr_setmask (hUart->uartNum, PRU_RX_INTR, CHN_TXRX_IE_MASK_TIMEOUT);

	/* generate ARM->PRU event */
	suart_arm_to_pru_intr(hUart->uartNum);

	return (status);
}

/* 
 * suart routine to read the data from the RX FIFO
 */
short pru_softuart_read_data (
			suart_handle hUart, 
			Uint8 * pDataBuffer, 
			Int32 s32MaxLen, 
			Uint32 * pu32DataRead)
{
	short retVal = PRU_SUART_SUCCESS;
	Uint8 * pu8SrcAddr = NULL;
	Uint32  u32DataRead = 0;
	Uint32	u32DataLen = 0;
	Uint32	u32CharLen = 0;
	unsigned int offset = 0;
	unsigned int pruOffset;
	unsigned short chNum;
	unsigned short u16Status = 0;
	

	if (hUart == NULL) {
		return (PRU_SUART_ERR_HANDLE_INVALID);
	}

    /* channel starts from 0 and uart instance starts from 1 */
    chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if (hUart->uartNum <= 4) {
		/* PRU0 */
		pruOffset = PRU_SUART_PRU0_CH0_OFFSET;
	} else {
		/* PRU1 */
		pruOffset = PRU_SUART_PRU1_CH0_OFFSET;

		/* First 8 channel corresponds to PRU0 */
		chNum -= 8;
	}
	
//	/* channel starts from 0 and uart instance starts from 1 */
//	chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;
	chNum++;
	
	/* Get the data pointer from channel RX data pointer */
	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_TXRXDATA_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) &pu8SrcAddr, 4, &pru_arm_iomap);

	/* Reading data length from SUART channel register */
	offset = pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
			 PRU_SUART_CH_CONFIG2_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) & u32DataLen, 2, &pru_arm_iomap);
	
	/* read the character length */
	u32CharLen = u32DataLen & PRU_SUART_CH_CONFIG2_BITPERCHAR_MASK;
	u32CharLen -= 2; /* remove the START & STOP bit */
	
	u32DataLen &= PRU_SUART_CH_CONFIG2_DATALEN_MASK;
	u32DataLen = u32DataLen >> PRU_SUART_CH_CONFIG2_DATALEN_SHIFT;
	u32DataLen ++;	
	
	/* if the character length is greater than 8, then the size doubles */
	if (u32CharLen > 8)
	{
		u32DataLen *= 2;
	}
		
	/* Check if the time-out had occured. If, yes, then we need to find the 
	 * number of bytes read from PRU. Else, we need to read the requested bytes
	 */
	offset = pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
			 PRU_SUART_CH_TXRXSTATUS_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) & u16Status, 1, &pru_arm_iomap);
	
	if (CHN_TXRX_STATUS_TIMEOUT == (u16Status & CHN_TXRX_STATUS_TIMEOUT))
	{
		/* determine the number of bytes read into the FIFO */
		offset = pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
				 PRU_SUART_CH_BYTESDONECNTR_OFFSET;
				 
		pru_ram_read_data(offset, (Uint8 *) & u32DataRead, 1, &pru_arm_iomap);
		
		/* if the character length is greater than 8, then the size doubles */
		if (u32CharLen > 8)
		{
			u32DataRead *= 2;
		}
		
		/* the data corresponding is loaded in second half during the timeout */
		if (u32DataRead > u32DataLen)
		{
			u32DataRead -= u32DataLen;
			pu8SrcAddr += (u32DataLen + 1);
		}
		
		pru_softuart_clrRxFifo (hUart);
	}
	else
	{
		u32DataRead = u32DataLen;
		/* Determine the buffer index by reading the FIFO_OddEven flag*/
		if (u16Status & CHN_TXRX_STATUS_CMPLT)
		{
			/* if the bit is set, the data is in the first half of the FIFO else
			 * the data is in the second half
			 */
			pu8SrcAddr += u32DataLen;
		}
	}

	/* we should be copying only max len given by the application */
	if (u32DataRead > s32MaxLen)
	{
		u32DataRead = s32MaxLen;
	}
	
	/* evaluate the virtual address of the FIFO address based on the physical addr */
	pu8SrcAddr = (Uint8 *) ((Uint32) pu8SrcAddr - (Uint32) pru_arm_iomap.pFifoBufferPhysBase + 
							(Uint32) pru_arm_iomap.pFifoBufferVirtBase);
	
	/* Now we have both the data length and the source address. copy */
	for (offset = 0; offset < u32DataRead; offset++)
		{
		* pDataBuffer++ = * pu8SrcAddr++;
		}
	* pu32DataRead = u32DataRead;	
	
	retVal = PRU_SUART_SUCCESS;
	
	return (retVal);
}			

/*
 * suart routine to disable the receive functionality. This routine stops the PRU
 * from receiving on selected UART and also disables the McASP serializer corresponding
 * to this UART Rx line.
 */
short pru_softuart_stopReceive (suart_handle hUart)
{
	unsigned short status = SUART_SUCCESS;
	unsigned int offset;
	unsigned int pruOffset;
	unsigned short chNum;
	unsigned short u16Status;

	if (hUart == NULL) {
		return (PRU_SUART_ERR_HANDLE_INVALID);
	}

    /* channel starts from 0 and uart instance starts from 1 */
    chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if (hUart->uartNum <= 4) {
		/* PRU0 */
		pruOffset = PRU_SUART_PRU0_CH0_OFFSET;
	} else {
		/* PRU1 */
		pruOffset = PRU_SUART_PRU1_CH0_OFFSET;
		/* First 8 channel corresponds to PRU0 */
		chNum -= 8;
	}
	
	/* channel starts from 0 and uart instance starts from 1 */
//	chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;
	chNum++;
	
	/* read the existing value of status flag */
	offset = pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
			 PRU_SUART_CH_TXRXSTATUS_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) &u16Status, 2, &pru_arm_iomap);
	
	/* we need to clear the busy bit corresponding to this receive channel */
	u16Status &= ~(CHN_TXRX_STATUS_RDY);
	pru_ram_write_data(offset, (Uint8 *) & u16Status, 2, &pru_arm_iomap);
	
	/* get the serizlizer number being used for this Rx channel */
	offset = pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
			 PRU_SUART_CH_CTRL_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) &u16Status, 2, &pru_arm_iomap);
	u16Status &= PRU_SUART_CH_CTRL_SR_MASK;
	u16Status = u16Status >> PRU_SUART_CH_CTRL_SR_SHIFT;	
	
	/* we need to de-activate the serializer corresponding to this receive */
	status =  suart_asp_serializer_deactivate (u16Status, &pru_arm_iomap);
	
	return (status);

}

/*
 * suart routine to get the tx status for a specific uart 
 */
short pru_softuart_getTxStatus(suart_handle hUart)
{
	unsigned int offset;
	unsigned int pruOffset;
	unsigned short status = SUART_SUCCESS;
	unsigned short chNum;

	if (hUart == NULL) {
		return (PRU_SUART_ERR_HANDLE_INVALID);
	}

    /* channel starts from 0 and uart instance starts from 1 */
    chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if (hUart->uartNum <= 4) {
		/* PRU0 */
		pruOffset = PRU_SUART_PRU0_CH0_OFFSET;
	} else {
		/* PRU1 */
		pruOffset = PRU_SUART_PRU1_CH0_OFFSET;
		/* First 8 channel corresponds to PRU0 */
		chNum -= 8;
	}

//	/* channel starts from 0 and uart instance starts from 1 */
//	chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_TXRXSTATUS_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) & status, 1, &pru_arm_iomap);
	return (status);
}

short pru_softuart_clrTxStatus(suart_handle hUart)
{
	unsigned int offset;
	unsigned int pruOffset;
	unsigned short status = SUART_SUCCESS;
	unsigned short chNum;

	if (hUart == NULL) {
		return (PRU_SUART_ERR_HANDLE_INVALID);
	}

    /* channel starts from 0 and uart instance starts from 1 */
    chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if (hUart->uartNum <= 4) {
		/* PRU0 */
		pruOffset = PRU_SUART_PRU0_CH0_OFFSET;
	} else {
		/* PRU1 */
		pruOffset = PRU_SUART_PRU1_CH0_OFFSET;
		/* First 8 channel corresponds to PRU0 */
		chNum -= 8;
	}

//	/* channel starts from 0 and uart instance starts from 1 */
//	chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_TXRXSTATUS_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) & status, 2, &pru_arm_iomap);

	status &= ~(0x2);
	pru_ram_write_data(offset, (Uint8 *) & status, 2, &pru_arm_iomap);
	return (status);
}

/*
 * suart routine to get the rx status for a specific uart 
 */
short pru_softuart_getRxStatus(suart_handle hUart)
{
	unsigned int offset;
	unsigned int pruOffset;
	unsigned short status = SUART_SUCCESS;
	unsigned short chNum;

	if (hUart == NULL) {
		return (PRU_SUART_ERR_HANDLE_INVALID);
	}

    /* channel starts from 0 and uart instance starts from 1 */
    chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if (hUart->uartNum <= 4) {
		/* PRU0 */
		pruOffset = PRU_SUART_PRU0_CH0_OFFSET;
	} else {
		/* PRU1 */
		pruOffset = PRU_SUART_PRU1_CH0_OFFSET;
		/* First 8 channel corresponds to PRU0 */
		chNum -= 8;
	}

//	/* channel starts from 0 and uart instance starts from 1 */
//	chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	chNum++;
	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_TXRXSTATUS_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) &status, 2, &pru_arm_iomap);
	return (status);
}

short pru_softuart_clrRxFifo(suart_handle hUart)
{
	unsigned int offset;
	unsigned int pruOffset;
	unsigned short status = SUART_SUCCESS;
	unsigned short chNum;
	unsigned short regVal;

	if (hUart == NULL) {
		return (PRU_SUART_ERR_HANDLE_INVALID);
	}

    /* channel starts from 0 and uart instance starts from 1 */
    chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if (hUart->uartNum <= 4) {
		/* PRU0 */
		pruOffset = PRU_SUART_PRU0_CH0_OFFSET;
	} else {
		/* PRU1 */
		pruOffset = PRU_SUART_PRU1_CH0_OFFSET;
		/* First 8 channel corresponds to PRU0 */
		chNum -= 8;
	}

//	/* channel starts from 0 and uart instance starts from 1 */
//	chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;
	chNum++;

	/* Service Request to PRU */
	offset = pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
			 PRU_SUART_CH_CTRL_OFFSET;

	pru_ram_read_data(offset, (Uint8 *) & regVal, 2, &pru_arm_iomap);

	regVal &= ~(PRU_SUART_CH_CTRL_MODE_MASK |PRU_SUART_CH_CTRL_SREQ_MASK);

	regVal |=  ( PRU_SUART_CH_CTRL_RX_MODE << PRU_SUART_CH_CTRL_MODE_SHIFT) | 
				(PRU_SUART_CH_CTRL_SREQ << PRU_SUART_CH_CTRL_SREQ_SHIFT);

	pru_ram_write_data(offset, (Uint8 *) & regVal, 2, &pru_arm_iomap);
	suart_intr_setmask (hUart->uartNum, PRU_RX_INTR, CHN_TXRX_IE_MASK_TIMEOUT);
	/* generate ARM->PRU event */
	suart_arm_to_pru_intr(hUart->uartNum);

	return (status);
}


short pru_softuart_clrRxStatus(suart_handle hUart)
{
	unsigned int offset;
	unsigned int pruOffset;
	unsigned short status = SUART_SUCCESS;
	unsigned short chNum;

	if (hUart == NULL) {
		return (PRU_SUART_ERR_HANDLE_INVALID);
	}

    /* channel starts from 0 and uart instance starts from 1 */
    chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if (hUart->uartNum <= 4) {
		/* PRU0 */
		pruOffset = PRU_SUART_PRU0_CH0_OFFSET;
	} else {
		/* PRU1 */
		pruOffset = PRU_SUART_PRU1_CH0_OFFSET;

		/* First 8 channel corresponds to PRU0 */
		chNum -= 8;
	}

//	/* channel starts from 0 and uart instance starts from 1 */
//	chNum = (hUart->uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	chNum++;
	offset =
	    pruOffset + (chNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
	    PRU_SUART_CH_TXRXSTATUS_OFFSET;
	pru_ram_read_data(offset, (Uint8 *) & status, 2, &pru_arm_iomap);

	status &= ~(0x3C);
	pru_ram_write_data(offset, (Uint8 *) & status, 2, &pru_arm_iomap);
	return (status);
}

/*
 * suart_intr_status_read: Gets the Global Interrupt status register 
 * for the specified SUART.
 * uartNum < 1 to 6 >
 * txrxFlag < Indicates TX or RX interrupt for the uart >
 */
short pru_softuart_get_isrstatus(unsigned short uartNum,
								unsigned short *txrxFlag)
{
	unsigned int offset;
	unsigned int chNum;
	unsigned int regVal = 0;
	unsigned int u32RegVal = 0;
	unsigned int u32ISRValue = 0;
	unsigned int u32AckRegVal = 0;

	/* initialize the status & Flag to known value */
	*txrxFlag = 0;
	
	/* Read PRU Interrupt Status Register from PRU */
	offset =
        (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_STATCLRINT1 &
                               0xFFFF);

	pru_ram_read_data_4byte(offset, (unsigned int *)&u32ISRValue, 1);

	/* determine if the interrupt occured in the current UART context */
	u32RegVal = (PRU_SUART0_TX_EVT_BIT | PRU_SUART0_RX_EVT_BIT) << ((uartNum -1) * 2);
	
	/* channel starts from 0 and uart instance starts from 1 */
	chNum = uartNum * 2 - 2;
	
	if (u32ISRValue & u32RegVal)
	{
		/* Check if the interrupt occured for Tx */
		u32RegVal = PRU_SUART0_TX_EVT_BIT << ((uartNum - 1)* 2);
		if (u32ISRValue & u32RegVal)
		{
			/* interupt occured for TX */
			*txrxFlag |= PRU_TX_INTR;
			
			/* acknowledge the interrupt  */
			u32AckRegVal  = chNum + PRU_SUART0_TX_EVT;
			offset = (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_STATIDXCLR &
									   0xFFFF);
			pru_ram_write_data_4byte(offset, (unsigned int *)&u32AckRegVal, 1);			
		}
		
		/* Check if the interrupt occured for Rx */
		u32RegVal = PRU_SUART0_RX_EVT_BIT << ((uartNum - 1)* 2);
		if (u32ISRValue & u32RegVal)
		{
			chNum += 1;
			/* interupt occured for RX */
			*txrxFlag |= PRU_RX_INTR;
			
			/* acknowledge the interrupt  */
			u32AckRegVal  = chNum + PRU_SUART0_TX_EVT;
			offset = (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_STATIDXCLR &
									   0xFFFF);
			pru_ram_write_data_4byte(offset, (unsigned int *)&u32AckRegVal, 1);			
		}		
		
		regVal = SUART_SUCCESS;
	}
	return regVal;
}

int pru_intr_clr_isrstatus(unsigned short uartNum, unsigned int txrxmode)
{
	unsigned int offset;
	unsigned short txrxFlag = 0;
	unsigned short chnNum;

    /* channel starts from 0 and uart instance starts from 1 */
    chnNum = (uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if ((uartNum > 0) && (uartNum <= 4)) 
	{
		offset = PRU_SUART_PRU0_ISR_OFFSET + 1;
	} 
	else if ((uartNum > 4) && (uartNum <= 8))
	{
		/* PRU1 */
		offset = PRU_SUART_PRU1_ISR_OFFSET + 1;

		/* First 8 channel corresponds to PRU0 */
		chnNum -= 8;
	} else 
	{
		return SUART_INVALID_UART_NUM;
	}

//	/* determine the channel number from UART number and direction */
//	chnNum = uartNum * 2 - 2;

	if (2 == txrxmode)
		chnNum++;
		
	pru_ram_read_data(offset, (Uint8 *) & txrxFlag, 1, &pru_arm_iomap);
	txrxFlag &= ~(0x2);
	pru_ram_write_data(offset, (Uint8 *) & txrxFlag, 1, &pru_arm_iomap);

	return 0;
}

short suart_arm_to_pru_intr(unsigned short uartNum)
{
	unsigned int u32offset;
	unsigned int u32value;
	short s16retval;

	if ((uartNum > 0) && (uartNum <= 4)) {
		/* PRU0 SYS_EVT32 */
		u32value = 0x1;
	} else if ((uartNum > 4) && (uartNum <= 8)) {
		/* PRU1 SYS_EVT32 */
		u32value = 0x1;
	} else {
		return SUART_INVALID_UART_NUM;
	}
	
	u32value = 0x20;
	
	u32offset =
	    (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_STATIDXSET &
						       0xFFFF);
	s16retval =
	    pru_ram_write_data_4byte(u32offset, (unsigned int *)&u32value, 1);
	if (s16retval == -1) {
		return -1;
	}
	return 0;
}

short arm_to_pru_intr_init(void)
{
	unsigned int u32offset;
	unsigned int u32value;
	unsigned int intOffset;
	short s16retval = -1;

	/* Clear all the host interrupts */
    for (intOffset = 0; intOffset < PRU_INTC_HOSTINTLVL_MAX; intOffset++)
    {
	    u32offset =
    	    (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_HSTINTENIDXCLR&
                               0xFFFF);
    	u32value = intOffset;
    	s16retval =
        	pru_ram_write_data_4byte(u32offset, (unsigned int *)&u32value, 1);
    	if (s16retval == -1) {
        	return -1;
    	}
	}
	
	/* Enable the global interrupt */
	u32offset = (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_GLBLEN &
						       0xFFFF);
	u32value = 0x1;
	s16retval = pru_ram_write_data_4byte(u32offset, (unsigned int *)&u32value, 1);
	if (s16retval == -1) {
		return -1;
	}

	/* Enable the Host interrupts for all host channels */
    for (intOffset = 0; intOffset < PRU_INTC_HOSTINTLVL_MAX; intOffset++)
	{
		u32offset = (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_HSTINTENIDXSET &
						       0xFFFF);
		u32value = intOffset;
		s16retval = pru_ram_write_data_4byte(u32offset, (unsigned int *)&u32value, 1);
		if (s16retval == -1) {
			return -1;
		}
	}

	/* host to channel mapping : Setting the host interrupt for channels 0,1,2,3 */
	u32offset =
	    (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_HOSTMAP0 &
						       0xFFFF);
	u32value = 0x03020100;
	s16retval =
	    pru_ram_write_data_4byte(u32offset, (unsigned int *)&u32value, 1);
	if (-1 == s16retval) {
		return -1;
	}

	/* host to channel mapping : Setting the host interrupt for channels 4,5,6,7 */
	u32offset =
	    (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_HOSTMAP1 &
						       0xFFFF);
	u32value = 0x07060504;
	s16retval =
	    pru_ram_write_data_4byte(u32offset, (unsigned int *)&u32value, 1);
	if (-1 == s16retval) {
		return -1;
	}

	/* host to channel mapping : Setting the host interrupt for channels 8,9 */
	u32offset =
	    (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_HOSTMAP2 &
						       0xFFFF);
	u32value = 0x00000908;
	s16retval =
	    pru_ram_write_data_4byte(u32offset, (unsigned int *)&u32value, 1);
	if (-1 == s16retval) {
		return -1;
	}

	/* Sets the channel for the system interrupt 
	 * MAP channel 0 to SYS_EVT32
	 * MAP channel 1 to SYS_EVT33 
	 * MAP channel 2 to SYS_EVT34  SUART0-Tx
	 * MAP channel 2 to SYS_EVT35  SUART0-Rx
	 */
	u32offset =
	    (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_CHANMAP8 &
						       0xFFFF);
	u32value = 0x02020100;
	s16retval =
	    pru_ram_write_data_4byte(u32offset, (unsigned int *)&u32value, 1);
	if (-1 == s16retval) {
		return -1;
	}

	/* Sets the channel for the system interrupt 
	 * MAP channel 3 to SYS_EVT36	SUART1-Tx
	 * MAP channel 3 to SYS_EVT37 	SUART1-Rx
	 * MAP channel 4 to SYS_EVT38 	SUART2-Tx
	 * MAP channel 4 to SYS_EVT39 	SUART2-Rx
	 */
	u32offset =
	    (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_CHANMAP9 &
						       0xFFFF);
	u32value = 0x04040303;
	s16retval =
	    pru_ram_write_data_4byte(u32offset, (unsigned int *)&u32value, 1);
	if (-1 == s16retval) {
		return -1;
	}

	/* Sets the channel for the system interrupt 
	 * MAP channel 5 to SYS_EVT40	SUART3-Tx
	 * MAP channel 5 to SYS_EVT41 	SUART3-Rx
	 * MAP channel 6 to SYS_EVT42 	SUART4-Tx
	 * MAP channel 6 to SYS_EVT43 	SUART4-Rx
	 */
	u32offset =
	    (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_CHANMAP10 &
						       0xFFFF);
	u32value = 0x06060505;
	s16retval =
	    pru_ram_write_data_4byte(u32offset, (unsigned int *)&u32value, 1);
	if (-1 == s16retval) {
		return -1;
	}

	/* Sets the channel for the system interrupt 
	 * MAP channel 7 to SYS_EVT44	SUART5-Tx
	 * MAP channel 7 to SYS_EVT45 	SUART5-Rx
	 * MAP channel 8 to SYS_EVT46 	SUART6-Tx
	 * MAP channel 8 to SYS_EVT47 	SUART6-Rx
	 */
	u32offset =
	    (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_CHANMAP11 &
						       0xFFFF);
	u32value = 0x08080707;
	s16retval =
	    pru_ram_write_data_4byte(u32offset, (unsigned int *)&u32value, 1);
	if (-1 == s16retval) {
		return -1;
	}

	/* Sets the channel for the system interrupt 
	 * MAP channel 9 to SYS_EVT48	SUART7-Tx
	 * MAP channel 9 to SYS_EVT49 	SUART7-Rx
	 */
	u32offset =
	    (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_CHANMAP11 &
						       0xFFFF);
	u32value = 0x00000909;
	s16retval =
	    pru_ram_write_data_4byte(u32offset, (unsigned int *)&u32value, 1);
	if (-1 == s16retval) {
		return -1;
	}

	/* Clear required set of system events and enable them using indexed register */
	for  (intOffset = 0; intOffset < 18; intOffset++)
	{
    	u32offset = (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_STATIDXCLR & 0xFFFF);
    	u32value = 32 + intOffset;
    	s16retval = pru_ram_write_data_4byte(u32offset, (unsigned int *) &u32value, 1);
    	if (s16retval == -1) {
        	return -1;
    	}
		
	}
	
	/* enable only the HOST to PRU interrupts and let the PRU to Host events be
     * enabled by the separate API on demand basis.
	 */
	u32offset = (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_ENIDXSET & 0xFFFF);
	u32value = 32;
	s16retval = pru_ram_write_data_4byte(u32offset, (unsigned int*) &u32value, 1);
	if (s16retval == -1) {
		return -1;
	}
	u32offset = (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_ENIDXSET & 0xFFFF);
	u32value = 33;
	s16retval = pru_ram_write_data_4byte(u32offset, (unsigned int*) &u32value, 1);
	if (s16retval == -1) {
		return -1;
	}

	/* Enable the global interrupt */
	u32offset = (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_GLBLEN &
						       0xFFFF);
	u32value = 0x1;
	s16retval = pru_ram_write_data_4byte(u32offset, (unsigned int *)&u32value, 1);
	if (s16retval == -1) {
		return -1;
	}

	/* Enable the Host interrupts for all host channels */
    for (intOffset = 0; intOffset < PRU_INTC_HOSTINTLVL_MAX; intOffset++)
	{
		u32offset = (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_HSTINTENIDXSET &
						       0xFFFF);
		u32value = intOffset;
		s16retval = pru_ram_write_data_4byte(u32offset, (unsigned int *)&u32value, 1);
		if (s16retval == -1) {
			return -1;
		}
	}

	return 0;
}

int suart_pru_to_host_intr_enable (unsigned short uartNum,
		       unsigned int txrxmode, int s32Flag)
{
	int    retVal = 0;
	unsigned int u32offset;
	unsigned int chnNum;
	unsigned int u32value;
	short s16retval = 0;
	
	if (uartNum > 8) {
		return SUART_INVALID_UART_NUM;
	}

	chnNum = (uartNum * 2) - 2;
	if (2 == txrxmode) {	/* Rx mode */
		chnNum++;
	}
	
	if (TRUE == s32Flag)
	{
		u32offset = (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_ENIDXSET & 0xFFFF);
		u32value = 34 + chnNum;
		s16retval = pru_ram_write_data_4byte(u32offset, (unsigned int*) &u32value, 1);
		if (s16retval == -1) {
			return -1;
		}
	}
	else
	{
		u32offset = (unsigned int)pru_arm_iomap.pru_io_addr | (PRU_INTC_ENIDXCLR & 0xFFFF);
		u32value = 34 + chnNum;
		s16retval = pru_ram_write_data_4byte(u32offset, (unsigned int*) &u32value, 1);
		if (s16retval == -1) {
			return -1;
		}
	}
	return (retVal);
}

int suart_intr_setmask(unsigned short uartNum,
		       unsigned int txrxmode, unsigned int intrmask)
{
	unsigned int offset;
	unsigned int pruOffset;
	unsigned int txrxFlag;
	unsigned int regval = 0;
	unsigned int chnNum;

    /* channel starts from 0 and uart instance starts from 1 */
    chnNum = (uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if ((uartNum > 0) && (uartNum <= 4)) {

		pruOffset = PRU_SUART_PRU0_CH0_OFFSET;
		offset = PRU_SUART_PRU0_IMR_OFFSET;
	} else if ((uartNum > 4) && (uartNum <= 8)) {
		/* PRU1 */
		offset = PRU_SUART_PRU1_IMR_OFFSET;
		pruOffset = PRU_SUART_PRU1_CH0_OFFSET;

		/* First 8 channel corresponds to PRU0 */
		chnNum -= 8;
	} else {
		return SUART_INVALID_UART_NUM;
	}

//	chnNum = (uartNum * 2) - 2;
	if (1 == txrxmode) {	/* tx mode */
		regval = 1 << chnNum;
	} else if (2 == txrxmode) {	/* rx mode */
		chnNum++;
		regval = 1 << chnNum;
	}

	if (CHN_TXRX_IE_MASK_CMPLT == (intrmask & CHN_TXRX_IE_MASK_CMPLT)) 
	{
		pru_ram_read_data(offset, (Uint8 *) & txrxFlag, 2,
				  &pru_arm_iomap);
		txrxFlag &= ~(regval);
		txrxFlag |= regval;
		pru_ram_write_data(offset, (Uint8 *) & txrxFlag, 2,
				   &pru_arm_iomap);
	}

	if ((intrmask & SUART_GBL_INTR_ERR_MASK) == SUART_GBL_INTR_ERR_MASK) {
		regval = 0;
		pru_ram_read_data(offset, (Uint8 *) & regval, 2,
				  &pru_arm_iomap);
		regval &= ~(SUART_GBL_INTR_ERR_MASK);
		regval |= (SUART_GBL_INTR_ERR_MASK);
		pru_ram_write_data(offset, (Uint8 *) & regval, 2,
				   &pru_arm_iomap);

	}
	// Break Indicator Interrupt Masked
	if ((intrmask & CHN_TXRX_IE_MASK_FE) == CHN_TXRX_IE_MASK_FE) {
		regval = 0;
		offset =
		    pruOffset + (chnNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
		    PRU_SUART_CH_CONFIG1_OFFSET;
		pru_ram_read_data(offset, (Uint8 *) & regval, 2,
				  &pru_arm_iomap);
		regval &= ~(CHN_TXRX_IE_MASK_FE);
		regval |= CHN_TXRX_IE_MASK_FE;
		pru_ram_write_data(offset, (Uint8 *) & regval, 2,
				   &pru_arm_iomap);
	}
	//Framing Error Interrupt Masked
	if (CHN_TXRX_IE_MASK_BI == (intrmask & CHN_TXRX_IE_MASK_BI)) {
		regval = 0;
		offset =
		    pruOffset + (chnNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
		    PRU_SUART_CH_CONFIG1_OFFSET;

		pru_ram_read_data(offset, (Uint8 *) & regval, 2,
				  &pru_arm_iomap);
		regval &= ~(CHN_TXRX_IE_MASK_BI);
		regval |= CHN_TXRX_IE_MASK_BI;
		pru_ram_write_data(offset, (Uint8 *) & regval, 2,
				   &pru_arm_iomap);
	}
	//Timeout error Interrupt Masked
	if (CHN_TXRX_IE_MASK_TIMEOUT == (intrmask & CHN_TXRX_IE_MASK_TIMEOUT)) {
		regval = 0;
		offset =
		    pruOffset + (chnNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
		    PRU_SUART_CH_CONFIG1_OFFSET;

		pru_ram_read_data(offset, (Uint8 *) & regval, 2,
				  &pru_arm_iomap);
		regval &= ~(CHN_TXRX_IE_MASK_TIMEOUT);
		regval |= CHN_TXRX_IE_MASK_TIMEOUT;
		pru_ram_write_data(offset, (Uint8 *) & regval, 2,
				   &pru_arm_iomap);
	}
	return 0;
}

int suart_intr_clrmask(unsigned short uartNum,
		       unsigned int txrxmode, unsigned int intrmask)
{
	unsigned int offset;
	unsigned int pruOffset;
	unsigned short txrxFlag;
	unsigned short regval = 0;
	unsigned short chnNum;

    /* channel starts from 0 and uart instance starts from 1 */
    chnNum = (uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if ((uartNum > 0) && (uartNum <= 4)) {

		pruOffset = PRU_SUART_PRU0_CH0_OFFSET;
		offset = PRU_SUART_PRU0_IMR_OFFSET;
	} else if ((uartNum > 4) && (uartNum <= 8)) {
		/* PRU1 */
		offset = PRU_SUART_PRU1_IMR_OFFSET;
		pruOffset = PRU_SUART_PRU1_CH0_OFFSET;
		/* First 8 channel corresponds to PRU0 */
		chnNum -= 8;
	} else {
		return SUART_INVALID_UART_NUM;
	}

//	chnNum = uartNum * 2 - 2;
	if (1 == txrxmode) {	/* tx mode */
		regval = 1 << chnNum;
	} else if (2 == txrxmode) {	/* rx mode */
		chnNum++;
		regval = 1 << chnNum;
	}

	if (CHN_TXRX_IE_MASK_CMPLT == (intrmask & CHN_TXRX_IE_MASK_CMPLT)) {
		pru_ram_read_data(offset, (Uint8 *) & txrxFlag, 2,
				  &pru_arm_iomap);
		txrxFlag &= ~(regval);
		pru_ram_write_data(offset, (Uint8 *) & txrxFlag, 2,
				   &pru_arm_iomap);
	}

	if ((intrmask & SUART_GBL_INTR_ERR_MASK) == SUART_GBL_INTR_ERR_MASK) {
		regval = 0;
		pru_ram_read_data(offset, (Uint8 *) & regval, 2,
				  &pru_arm_iomap);
		regval &= ~(SUART_GBL_INTR_ERR_MASK);
		pru_ram_write_data(offset, (Uint8 *) & regval, 2,
				   &pru_arm_iomap);

	}
	// Break Indicator Interrupt Masked
	if ((intrmask & CHN_TXRX_IE_MASK_FE) == CHN_TXRX_IE_MASK_FE) {
		regval = 0;
		offset =
		    pruOffset + (chnNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
		    PRU_SUART_CH_CONFIG1_OFFSET;
		pru_ram_read_data(offset, (Uint8 *) & regval, 2,
				  &pru_arm_iomap);
		regval &= ~(CHN_TXRX_IE_MASK_FE);
		pru_ram_write_data(offset, (Uint8 *) & regval, 2,
				   &pru_arm_iomap);
	}
	//Framing Error Interrupt Masked
	if (CHN_TXRX_IE_MASK_BI == (intrmask & CHN_TXRX_IE_MASK_BI)) {
		regval = 0;
		offset =
		    pruOffset + (chnNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
		    PRU_SUART_CH_CONFIG1_OFFSET;

		pru_ram_read_data(offset, (Uint8 *) & regval, 2,
				  &pru_arm_iomap);
		regval &= ~(CHN_TXRX_IE_MASK_BI);
		pru_ram_write_data(offset, (Uint8 *) & regval, 2,
				   &pru_arm_iomap);
	}
	
	//Timeout error Interrupt Masked
	if (CHN_TXRX_IE_MASK_TIMEOUT == (intrmask & CHN_TXRX_IE_MASK_TIMEOUT)) {
		regval = 0;
		offset =
		    pruOffset + (chnNum * SUART_NUM_OF_BYTES_PER_CHANNEL) +
		    PRU_SUART_CH_CONFIG1_OFFSET;

		pru_ram_read_data(offset, (Uint8 *) & regval, 2,
				  &pru_arm_iomap);
		regval &= ~(CHN_TXRX_IE_MASK_TIMEOUT);
		pru_ram_write_data(offset, (Uint8 *) & regval, 2,
				   &pru_arm_iomap);
	}	
	return 0;
}

int suart_intr_getmask(unsigned short uartNum,
		       unsigned int txrxmode, unsigned int intrmask)
{
	unsigned short chnNum;
	unsigned int offset;
	unsigned short txrxFlag;
	unsigned short regval = 1;

    /* channel starts from 0 and uart instance starts from 1 */
    chnNum = (uartNum * SUART_NUM_OF_CHANNELS_PER_SUART) - 2;

	if ((uartNum > 0) && (uartNum <= 4)) {

		offset = PRU_SUART_PRU0_IMR_OFFSET;
	} else if ((uartNum > 4) && (uartNum <= 8)) {
		/* PRU1 */
		offset = PRU_SUART_PRU1_ISR_OFFSET;

		/* First 8 channel corresponds to PRU0 */
		chnNum -= 8;
	} else {
		return SUART_INVALID_UART_NUM;
	}

//	chnNum = uartNum * 2 - 2;

	if (1 == txrxmode) {	/* tx mode */
		regval = regval << chnNum;
	} else if (2 == txrxmode) {	/* rx mode */
		chnNum++;
		regval = regval << chnNum;
	}
	pru_ram_read_data(offset, (Uint8 *) & txrxFlag, 2, &pru_arm_iomap);
	txrxFlag &= regval;
	if (0 == intrmask) {
		if (txrxFlag == 0)
			return 1;
	}

	if (1 == intrmask) {
		if (txrxFlag == regval)
			return 1;
	}
	return 0;
}

static int suart_set_pru_id (unsigned int pru_no)
{
    unsigned int offset;
	unsigned short reg_val = 0;

	if (0 == pru_no)
	{	
		offset = PRU_SUART_PRU0_ID_ADDR;
	}
    else if (1 == pru_no)
    {
		offset = PRU_SUART_PRU1_ID_ADDR;	
    }
    else
    {
		return PRU_SUART_FAILURE;
	}

	pru_ram_read_data(offset, (Uint8 *) & reg_val, 1, &pru_arm_iomap);
    reg_val &=~SUART_PRU_ID_MASK;
    reg_val = pru_no;
	pru_ram_write_data(offset, (Uint8 *) & reg_val, 1, &pru_arm_iomap);

	return PRU_SUART_SUCCESS;
}
/* End of file */
