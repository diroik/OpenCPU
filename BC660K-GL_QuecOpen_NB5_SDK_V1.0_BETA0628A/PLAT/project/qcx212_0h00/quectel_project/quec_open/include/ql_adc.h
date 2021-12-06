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
 *   ql_adc.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   The module defines the information, and APIs related to the ADC function.
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
#ifndef __QL_ADC_H__
#define __QL_ADC_H__

#include "ql_type.h"

typedef enum
{
    PIN_ADC0,
    PIN_ADC1,
    PIN_ADC_MAX
}Enum_ADCPin;


/*****************************************************************
* Function:     Ql_ADC_Open
* 
* Description:
*		This function is used for open an ADC channel.
*
* Parameters:
* 		adcPin:
* 		[in] adc pin name.
*                 
* Return:        
*		QL_RET_OK, this function succeeds.
*		QL_RET_ERR_PARAM, the input pin is invalid.
*       Ql_RET_ERR_ADC_NOT_OPEN, the ADC channel is not open.
*****************************************************************/
s32 Ql_ADC_Open(Enum_ADCPin adcPin);


/*****************************************************************
* Function:     Ql_ADC_Read 
* 
* Description:
*		This function is used for read the ADC value of the specified pin.
*       out range: 0 ~ 1.2V
*
* Parameters:
*		adcPin:
*		[in] adc pin name.
*
*		adcValue:
*		[out] the ADC value of the specified pin.
*                 
* Return:        
*		QL_RET_OK, this function succeeds.
*		QL_RET_ERR_PARAM, the input pin is invalid.
*       Ql_RET_ERR_ADC_CONVER_STOP, adc channel conversion stop.
*       Ql_RET_ERR_ADC_CONVER_TIMEOUT, adc channel conversion timeout.
*****************************************************************/
s32 Ql_ADC_Read(Enum_ADCPin adcPin,u32 *adcValue);


/*****************************************************************
* Function:     Ql_ADC_Close 
* 
* Description:
*		This function is used for close the specified ADC channel.
*
* Parameters:
*		adcPin:
*		[in] adc pin name.
*                 
* Return:
*		QL_RET_OK, this function succeeds.
*		QL_RET_ERR_PARAM, the input pin is invalid.
*       Ql_RET_ERR_ADC_NOT_OPEN, the ADC channel is not open.
*****************************************************************/
s32 Ql_ADC_Close(Enum_ADCPin adcPin);


/*****************************************************************
* Function:     Ql_Get_Temperature 
* 
* Description:
*		This function is used to read the internal temperature of the module.
*       out range: -40°„C ~ 85°„C
*
* Parameters:
*		tempValue:
*		[out] temperature value.
*                 
* Return:
*		QL_RET_OK, this function succeeds.
*		QL_RET_ERR_ERROR,get failed.
*		QL_RET_ERR_PARAM, the input pin is invalid.
*		Ql_RET_ERR_ADC_CONVER_TIMEOUT, the adc channel conversion timeout.
*		Ql_RET_ERR_ADC_ONGOING, the adc channel conversion is running.
*****************************************************************/
s32 Ql_Get_Temperature(s32* tempValue);


#endif  //__QL_ADC_H__

