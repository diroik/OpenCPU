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
 *   ql_pwm.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *  PWM related APIs
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
 

#ifndef __QL_PWM_H__
#define __QL_PWM_H__

#include "ql_type.h"
#include "ql_error.h"
#include "ql_gpio.h"


typedef enum{
    PWMSOURCE_26M= 0,
    PWMSOURCE_32K,
    END_OF_PWMSOURCE
}Enum_PwmSource;

/*****************************************************************
* Function:     Ql_PWM_Init 
* 
* Description:
*               This function initializes the PWM pin. 
*
*               NOTES:
*                   The PWM waveform can't out immediately after Ql_PWM_Init Initialization
*                   you must invoke Ql_PWM_Output function to control PWM waveform on or off
* Parameters:
*               pinName:
*                   [in] Pin name, Enum_PinName, support only one now, PINNAME_SPI_SCLK,PINNAME_GPIO4,PINNAME_MAIN_RTS choose one of three.
*               pwmSrcClk:
*                   [in] PWM source clock , one value of Enum_PwmSource.
*               freq:
*                   [in] The frequency of the PWM output,Range:2-6500000.
*               dutyCyclePercent:
*                   [in] set the number of clock cycles to stay at High and low ratio,Range:0-100.
*
*			
* Return:        
*               QL_RET_OK, this function succeeds.
*               QL_RET_ERR_PARAM, the input pin is invalid.
*               QL_RET_ERR_NOSUPPORTPIN, the input pin is not support PWM.
*               QL_RET_ERR_INVALID_OP,freq or not dutyCyclePercent error.
*               QL_RET_ERR_PINALREADYSUBCRIBE,the PIN already init.
*****************************************************************/
s32 Ql_PWM_Init(Enum_PinName pwmPinName, Enum_PwmSource pwmSrcClk, u32 freq, u32 dutyCyclePercent);


/*****************************************************************
* Function:     Ql_PWM_Output 
* 
* Description:
*               This function switches on/off PWM waveform output.
*
* Parameters:
*               pinName:
*                   [in] Pin name, Enum_PinName, support only one now, PINNAME_SPI_SCLK,PINNAME_GPIO4,PINNAME_MAIN_RTS choose one of three.
*               pwmOnOff:
*                   PWM enable. control the PWM waveform output or not output.
*					1:output
*					0:not output
*
* Return:        
*               QL_RET_OK, this function succeeds.
*               QL_RET_ERR_PARAM, the input pin is invalid.
*               QL_RET_ERR_NOSUPPORTPIN, the input pin is not support PWM.
*               QL_RET_ERR_ALREADYUNSUBCRIBE, the PIN not in PWM mode or not init.
*****************************************************************/
s32 Ql_PWM_Output(Enum_PinName pwmPinName,bool pwmOnOff);


/*****************************************************************
* Function:     Ql_PWM_UpdateDutyCycle 
* 
* Description:
*               This function directly update PWM output dutyCyclePercent.
*
* Parameters:
*               pinName:
*                   [in] Pin name, Enum_PinName, support only one now, PINNAME_SPI_SCLK,PINNAME_GPIO4,PINNAME_MAIN_RTS choose one of three.
*               dutyCyclePercent:
*                   [in] set the number of clock cycles to stay at High and low ratio,Range:0-100.

*
* Return:        
*               QL_RET_OK, this function succeeds.
*               QL_RET_ERR_PARAM, the input pin is invalid.
*               QL_RET_ERR_NOSUPPORTPIN, the input pin is not support PWM.
*               QL_RET_ERR_INVALID_OP, dutyCyclePercent error.
*               QL_RET_ERR_ALREADYUNSUBCRIBE, the PIN not in PWM mode or not init.
*****************************************************************/
s32 Ql_PWM_UpdateDutyCycle(Enum_PinName pwmPinName, u32 dutyCyclePercent);	


/*****************************************************************
* Function:     Ql_PWM_Uninit 
* 
* Description:
*               This function release a PWM pin.
*
* Parameters:
*               pinName:
*                   [in] Pin name, Enum_PinName, support only one now, PINNAME_SPI_SCLK,PINNAME_GPIO4,PINNAME_MAIN_RTS choose one of three.
* Return:        
*               QL_RET_OK, this function succeeds.
*               QL_RET_ERR_PARAM, the input pin is invalid.
*               QL_RET_ERR_NOSUPPORTPIN, the input pin is not support PWM.
*               QL_RET_ERR_ALREADYUNSUBCRIBE, the PIN not in PWM mode or not init.
*****************************************************************/
s32 Ql_PWM_Uninit(Enum_PinName pwmPinName);


#endif

