/*
 * linux/<file location within the kernel tree>
 *
 * Copyright (C) 2010 Texas Instruments Incorporated
 * Author: Ganeshan N
 *
 * Based on <Give reference of old kernel file from which this file is derived from>
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

#ifndef _OMAPL_SUART_BOARD_H_
#define _OMAPL_SUART_BOARD_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PRU_SUART1_CONFIG_DUPLEX (ePRU_SUART_HALF_TX | ePRU_SUART_HALF_RX)
#define PRU_SUART1_CONFIG_RX_SER (PRU_SUART_SERIALIZER_7)
#define PRU_SUART1_CONFIG_TX_SER (PRU_SUART_SERIALIZER_8)

#define PRU_SUART2_CONFIG_DUPLEX (ePRU_SUART_HALF_TX | ePRU_SUART_HALF_RX)
#define PRU_SUART2_CONFIG_RX_SER (PRU_SUART_SERIALIZER_9)
#define PRU_SUART2_CONFIG_TX_SER (PRU_SUART_SERIALIZER_10)

#define PRU_SUART3_CONFIG_DUPLEX (ePRU_SUART_HALF_TX | ePRU_SUART_HALF_RX)
#define PRU_SUART3_CONFIG_RX_SER (PRU_SUART_SERIALIZER_13)
#define PRU_SUART3_CONFIG_TX_SER (PRU_SUART_SERIALIZER_14)

#define PRU_SUART4_CONFIG_DUPLEX (ePRU_SUART_HALF_TX | ePRU_SUART_HALF_RX)
#define PRU_SUART4_CONFIG_RX_SER (PRU_SUART_SERIALIZER_NONE)
#define PRU_SUART4_CONFIG_TX_SER (PRU_SUART_SERIALIZER_NONE)

#define PRU_SUART5_CONFIG_DUPLEX (ePRU_SUART_HALF_TX | ePRU_SUART_HALF_RX)
#define PRU_SUART5_CONFIG_RX_SER (PRU_SUART_SERIALIZER_NONE)
#define PRU_SUART5_CONFIG_TX_SER (PRU_SUART_SERIALIZER_NONE)

#define PRU_SUART6_CONFIG_DUPLEX (ePRU_SUART_HALF_TX | ePRU_SUART_HALF_RX)
#define PRU_SUART6_CONFIG_RX_SER (PRU_SUART_SERIALIZER_NONE)
#define PRU_SUART6_CONFIG_TX_SER (PRU_SUART_SERIALIZER_NONE)

#define PRU_SUART7_CONFIG_DUPLEX (ePRU_SUART_HALF_TX | ePRU_SUART_HALF_RX)
#define PRU_SUART7_CONFIG_RX_SER (PRU_SUART_SERIALIZER_NONE)
#define PRU_SUART7_CONFIG_TX_SER (PRU_SUART_SERIALIZER_NONE)

#define PRU_SUART8_CONFIG_DUPLEX (ePRU_SUART_HALF_TX | ePRU_SUART_HALF_RX)
#define PRU_SUART8_CONFIG_RX_SER (PRU_SUART_SERIALIZER_NONE)
#define PRU_SUART8_CONFIG_TX_SER (PRU_SUART_SERIALIZER_NONE)

#ifdef __cplusplus
}				/* End of extern C */
#endif				/* #ifdef __cplusplus */
#endif				/* End of _<MOD>_FILENAME_API_H_ */
