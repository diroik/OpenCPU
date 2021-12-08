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
 *   ql_i2c.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   I2C interface APIs defines.
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
#ifndef __QL_I2C_H__
#define __QL_I2C_H__

#include "ql_type.h"
#include "ql_gpio.h"

// ============================================================================
// Enum_I2cSpeed
// ----------------------------------------------------------------------------
/// That type describes the supported frequencies for the I2C bus.
/// They are sustained as long as the driver is opened.
// ============================================================================
typedef enum
{
    HAL_I2C_BPS_100K = 1,
    HAL_I2C_BPS_400K,
    HAL_I2C_BPS_1M,
    HAL_I2C_BPS_3_4M
    
} Enum_I2cSpeed;
    
/*****************************************************************
* Function: 	Ql_I2C_Init 
* 
* Description:
*		This function initializes the configurations for an I2C channel,
*		include the specified pins for I2C, I2C type, and I2C channel number.
*
* Parameters:
*		chnnlNo:
*		[in] I2C channel number, the range is 0~254.
*
*		pinSCL:
*		[in] I2C SCL pin.
*
*		pinSDA:
*		[in] I2C SDA pin.
*
*		I2Ctype:
*		[in] I2C type, 'FALSE' means simulate I2C , 'TRUE' means hardware I2C.
*       Note: the current version does not support simulated IIC.
* Return:		 
*		QL_RET_OK, this function succeeds.
*		QL_RET_ERR_PARAM, the channel No is be used.
*		QL_RET_ERR_NOSUPPORT, not support.
*		QL_RET_ERR_I2C_NOT_CHAN, unsupported channel.
*		QL_RET_ERR_I2C_CHAN_USED, the channel is already in use.
*		QL_RET_ERR_I2C_COUNT_FULL, exceeding the maximum total.
*		QL_RET_ERR_I2C_INVALID_PIN, invalid pins.
*		QL_RET_ERR_I2C_CHAN_USED, The channel is already in use.
*		QL_RET_ERR_I2C_HW_FAILED, hardware error.
*		QL_RET_ERR_I2C_HW_INIT_FAILED, hardware I2C initialization failed.
*****************************************************************/
s32 Ql_I2C_Init(u32 chnnlNo, Enum_PinName pinSCL, Enum_PinName pinSDA, bool I2Ctype);


/*****************************************************************
* Function: 	Ql_I2C_Config 
* 
* Description:
*		This function configures the I2C interface.
*		  
* Parameters:
*		chnnlNo:
*		[in] I2C channel number, the number is specified by Ql_I2C_Init function.
*
*		isHost:
*		[in] must be ture, just support host mode.
*
*		I2CSpeed:
*		[in] just for hardware I2C controller, and the parameter can be ingored if use simulate I2C.
*
* Return:		 
*		QL_RET_OK, this function succeeds.
*		QL_RET_ERR_PARAM, the input pin is invalid.
*		QL_RET_ERR_I2C_CHAN_NOT_FOUND, the IIC channel ID does not exist.
*		QL_RET_ERR_I2C_NOSUPPORT_SPEND, unsupported configuration param.
*		QL_RET_ERR_I2C_HW_INIT_FAILED, hardware I2C initialization failed.
*****************************************************************/
s32  Ql_I2C_Config(u32 chnnlNo, bool isHost, Enum_I2cSpeed I2CSpeed);


/*****************************************************************
* Function: 	Ql_I2C_Write 
* 
* Description:
*		This function writes data to the specified slave device through I2C interface.
*				
* Parameters:
*		chnnlNo:
*		[in] I2C channel number, the number is specified by Ql_I2C_Init function.
*
*		slaveAddr:
*		[in] slave address.
*
*		pData:
*		[in] Setting value to slave.
*
*		len:
*		[in] Number of bytes to write. 
*
* Return:
*		If write succeed,  return the length of the write data.
*		QL_RET_ERR_PARAM, patameter error.
*		QL_RET_ERR_NOSUPPORT, not support.
*		QL_RET_ERR_I2C_CHAN_NOT_FOUND, the I2C channel ID does not exist.
*		QL_RET_ERR_I2C_INVALID_PIN, invalid pins.
*		QL_RET_ERR_I2C_HW_FAILED, hardware error.
*****************************************************************/
s32 Ql_I2C_Write(u32 chnnlNo, u8 slaveAddr, u8* pData, u32 len);


/*****************************************************************
* Function: 	Ql_I2C_Read 
* 
* Description:
*		This function reads data from the specified slave device through I2C interface.
*				
* Parameters:
*		chnnlNo:
*		[in] I2C channel number, the number is specified by Ql_I2C_Init function.
*
*		slaveAddr:
*		[in] slave address.
*
*		pBuffer:
*		[out] read buffer of reading the specified register from slave device.
*
*		len:
*		[in] Number of bytes to read.
*
* Return:		 
*		If read succeed, return the length of the read data.
*		QL_RET_ERR_PARAM, patameter error.
*		QL_RET_ERR_NOSUPPORT, not support.
*		QL_RET_ERR_I2C_CHAN_NOT_FOUND, the I2C channel ID does not exist.
*		QL_RET_ERR_I2C_INVALID_PIN, invalid pins.
*		QL_RET_ERR_I2C_HW_FAILED, hardware error.
*****************************************************************/
s32 Ql_I2C_Read(u32 chnnlNo, u8 slaveAddr, u8* pBuffer, u32 len);


/*****************************************************************
* Function: 	Ql_I2C_Write_Read 
* 
* Description:
*		This function read data fromm the specified register(or address) of slave device.
*				
* Parameters:
*		chnnlNo:
*		[in] I2C channel number, the number is specified by Ql_I2C_Init function.
*
*		slaveAddr:
*		[in] slave address.
*
*		pData:
*		[in] Setting value of the specified register of slave device.
*
*		wrtLen:
*		[in] Number of bytes to write.
*
*		pBuffer:
*		[out] read buffer of reading the specified register from slave.
*
*		rdLen:
*		[in] Number of bytes to read.
*
* Return:		 
*		If no error, return the length of the read data.
*		QL_RET_ERR_PARAM, patameter error.
*		QL_RET_ERR_NOSUPPORT, not support.
*		QL_RET_ERR_I2C_CHAN_NOT_FOUND, the I2C channel ID does not exist.
*		QL_RET_ERR_I2C_HW_FAILED, hardware error.
*****************************************************************/
s32 Ql_I2C_Write_Read(u32 chnnlNo, u8 slaveAddr, u8* pData, u32 wrtLen, u8* pBuffer, u32 rdLen);


/*****************************************************************
* Function: 	Ql_I2C_Uninit 
* 
* Description:
*		This function releases the pins.
*				
* Parameters:
*		chnnlNo:
*		[in] I2C channel number, the number is specified by Ql_I2C_Init function.
*
* Return:		 
*		QL_RET_OK, this function succeeds.
*		QL_RET_ERR_I2C_CHAN_NOT_FOUND, the I2C channel ID does not exist.
*		QL_RET_ERR_I2C_CHAN_USED, the channel is already in use
*		QL_RET_ERR_I2C_INVALID_PIN, invalid pins.
*****************************************************************/
s32 Ql_I2C_Uninit(u32 chnnlNo);

#endif  //__QL_I2C_H__
