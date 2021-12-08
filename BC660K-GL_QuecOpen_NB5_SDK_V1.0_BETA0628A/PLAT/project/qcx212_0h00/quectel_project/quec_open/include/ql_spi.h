/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Quectel Co., Ltd. 2021
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   ql_spi.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   SPI APIs definition.
 *
 * Author:
 * -------
 * -------
 *
 *============================================================================
 *             HISTORY
 *----------------------------------------------------------------------------
 * 
 ****************************************************************************/
#ifndef __QL_SPI_H__
#define __QL_SPI_H__

#include "ql_type.h"
#include "ql_gpio.h"

/*****************************************************************
* Function:     Ql_SPI_Init 
* 
* Description:
*		This function initializes the configurations for a SPI channel.
*		including the specified pins for SPI, SPI type, and SPI channel number.
*
* Parameters:
*		chnnlNo:
*		[in] SPI channel number, the range is 0~254.
*
*		pinClk:
*		[in] SPI CLK pin.
*
*		pinMiso:
*		[in] SPI MISO pin.
*
*		pinMosi:
*		[in] SPI MOSI pin.
*
*		pinCs
*		[in] SPI CS pin.
*
*		spiType:
*		[in] SPI type, 'FALSE' means simulate SPI , 'TRUE' means hardware SPI.
*       Note: the current version does not support simulated SPI.
*
* Return:        
*		QL_RET_OK, this function succeeds.
*		QL_RET_ERR_PARAM, parameter error.
*       QL_RET_ERR_NOSUPPORT, unsupported features.
*		QL_RET_ERR_INVALID_OP, the current operation is invalid.
*		QL_RET_ERR_SPI_INVALID_PIN, the input pin is invalid.
*		QL_RET_ERR_SPI_HW_INIT_FAILED, hardware initialization failed.
*****************************************************************/
s32 Ql_SPI_Init(u32 chnnlNo, Enum_PinName pinClk, Enum_PinName pinMiso, Enum_PinName pinMosi, Enum_PinName pinCs, bool spiType);

/*****************************************************************
* Function:     Ql_SPI_Config 
* 
* Description:
*		This function configures the SPI interface.
*
* Parameters:
*		chnnlNo:
*		[in] SPI channel number, the range is 0~254.
*
*		isHost:
*		[in] must be true, only support master mode.
*
*		cpol:
*		[in] Clock Polarity.
*
*		cpha:
*		[in] Clock Phase.
*
*		clkSpeed:
*		[in] the SPI speed, hardware SPI supports 1MHz~52MHz clock frequency, when use simulate SPI ignore it.
*
* Return:
*		QL_RET_OK, this function succeeds.
*		QL_RET_ERR_PARAM, parameter error.
*       QL_RET_ERR_NOSUPPORT, unsupported features.
*		QL_RET_ERR_INVALID_OP, The current operation is invalid.
*		QL_RET_ERR_SPI_NOT_CHAN, unsupported channel.
*****************************************************************/
s32 Ql_SPI_Config(u32 chnnlNo, bool isHost, bool cpol, bool cpha, u32 clkSpeed);

/*****************************************************************
* Function:     Ql_SPI_Write 
* 
* Description:
*		This function writes data to slave device through SPI interface.
*               
* Parameters:
*		chnnlNo:
*		[in] SPI channel number, the range is 0~254.
*
*		pData:
*		[in] Setting value to slave device.
*
*		len:
*		[in] Number of bytes to write.
*
* Return:        
*		QL_RET_OK, this function succeeds.
*		QL_RET_ERR_PARAM, parameter error.
*       QL_RET_ERR_NOSUPPORT, unsupported features.
*		QL_RET_ERR_INVALID_OP, the current operation is invalid.
*		QL_RET_ERR_SPI_NOT_CHAN, unsupported channel.
*****************************************************************/
s32 Ql_SPI_Write(u32 chnnlNo, u8* pData, u32 len);

/*****************************************************************
* Function:     Ql_SPI_Read 
* 
* Description:
*		This function reads data from the specified slave device through SPI interface.
*               
* Parameters:
*		chnnlNo:
*		[in] SPI channel number, the range is 0~254.
*
*		pBuffer:
*		[out] read buffer of reading data from slave device.
*
*		rdLen:
*		[in] Number of bytes to read.
*
* Return:        
*		QL_RET_OK, this function succeeds.
*		QL_RET_ERR_PARAM, parameter error.
*       QL_RET_ERR_NOSUPPORT, unsupported features.
*		QL_RET_ERR_INVALID_OP, the current operation is invalid.
*		QL_RET_ERR_SPI_NOT_CHAN, unsupported channel.
*****************************************************************/
s32 Ql_SPI_Read(u32 chnnlNo, u8* pBuffer, u32 rdLen);

/*****************************************************************
* Function:     Ql_SPI_WriteRead 
* 
* Description:
*		This function is used for SPI half-duplex communication.
*               
* Parameters:
*		chnnlNo:
*		[in] SPI channel number, the range is 0~254.
*
*		pData:
*		[in] Setting value to slave device.
*
*		wrtLen:
*		[in] Number of bytes to write.
*
*		pBuffer:
*		[out] read buffer of reading data from slave device.
*
*		rdLen:
*		[in] Number of bytes to read.
*
*		Notes:
*		if (wrtLen > rdLen) , the other read buffer data will be set 0xff;
*		if (rdLen > wrtLen) , the other write buffer data will be set 0xff;
*
* Return:        
*		QL_RET_OK, this function succeeds.
*		QL_RET_ERR_PARAM, parameter error.
*       QL_RET_ERR_NOSUPPORT, unsupported features.
*		QL_RET_ERR_INVALID_OP, the current operation is invalid.
*		QL_RET_ERR_SPI_NOT_CHAN, unsupported channel.
*****************************************************************/
s32 Ql_SPI_WriteRead(u32 chnnlNo, u8* pData, u32 wrtLen, u8* pBuffer, u32 rdLen);

/*****************************************************************
* Function:     Ql_SPI_Uninit 
* 
* Description:
*		This function releases the SPI pins.
*
* Parameters:
*		chnnlNo:
*		[in] SPI channel number, the range is 0~254.

* Return:        
*		QL_RET_OK, this function succeeds.
*		QL_RET_ERR_PARAM, parameter error.
*       QL_RET_ERR_NOSUPPORT, unsupported features.
*****************************************************************/
s32 Ql_SPI_Uninit(u32 chnnlNo);

#endif  //__QL_SPI_H__

