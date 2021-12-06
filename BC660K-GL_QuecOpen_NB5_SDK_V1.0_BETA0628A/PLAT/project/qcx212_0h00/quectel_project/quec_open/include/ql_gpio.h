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
 *   ql_gpio.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   GPIO API defines.
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


#ifndef __QL_GPIO_H__
#define __QL_GPIO_H__

#include "ql_type.h"

/****************************************************************************
 * Enumeration for GPIO Pins available.
 ***************************************************************************/
typedef enum
{
    /**************************************************
    *  <<<BEGIN: The fowllowing PINs for BC260Y
    *            (TOTAL: 16)
    ***************************************************/
    PINNAME_SPI_MISO = 0,
    PINNAME_SPI_MOSI,
    PINNAME_SPI_SCLK,
    PINNAME_SPI_CS,
    PINNAME_GPIO1,			//not support eint
    PINNAME_I2C_SCL,			
    PINNAME_I2C_SDA,
    PINNAME_MAIN_CTS,
    PINNAME_MAIN_RTS,
    PINNAME_GPIO2,
    PINNAME_RXD_AUX,		//not support eint
    PINNAME_TXD_AUX,		//not support eint
    PINNAME_GPIO3,
    PINNAME_GPIO4,
    PINNAME_GPIO5,
    PINNAME_GPIO6,		
    
	PINNAME_END
}Enum_PinName;


typedef enum
{
    PINDIRECTION_IN= 0,
    PINDIRECTION_OUT,
    END_OF_PINDIRECTION
}Enum_PinDirection;


typedef enum
{
    PINLEVEL_LOW = 0,
    PINLEVEL_HIGH,
}Enum_PinLevel;


typedef enum{
    PINPULLSEL_PULLUP    = 0,    	/**< select internal pull up */
    //PINPULLSEL_PULLDOWN  = 1,   	/**< select internal pull down */    platform not support
    PINPULLSEL_DISABLE   = 2   		/**< Pull up/down is controlled by muxed alt function */
}Enum_PinPullSel;



typedef enum{
	EINT_LEVEL_DISABLE = 0, 
	EINT_LEVEL_LOW = 1, 					/**< platform not support. */
    EINT_LEVEL_HIGH  = 2, 					/**< platform not support. */
    EINT_EDGE_FALLING  = 3,                 /**< Edge and falling trigger. */
    EINT_EDGE_RISING   = 4,                 /**< Edge and rising trigger. */
    
    END_OF_HAL_EINT
}Enum_EintType;


/*****************************************************************
* Function:     Ql_GPIO_Init
*
* Description:
*               This function enables the GPIO function of the specified pin,
*               and initialize the configurations, including direction,
*               level and pull selection.
*
* Parameters:
*               pinName:
*                   Pin name, one value of Enum_PinName.
*               dir:
*                   The initial direction of GPIO, one value of Enum_PinDirection.
*               level:
*                   The initial level of GPIO, one value of Enum_PinLevel.
*               pullSel:
*                   Pull selection, one value of Enum_PinPullSel.
* Return:
*               QL_RET_OK, this function succeeds.
*               QL_RET_ERR_NOSUPPORTPIN, the input GPIO is invalid.
*               QL_RET_ERR_PINALREADYSUBCRIBE, the GPIO is in use in
*               other place. For example this GPIO has been using as EINT.
*****************************************************************/
s32 Ql_GPIO_Init(Enum_PinName pinName,Enum_PinDirection dir,Enum_PinLevel  level ,Enum_PinPullSel pullsel);


/*****************************************************************
* Function:     Ql_GPIO_SetLevel
*
* Description:
*               This function sets the level of the specified GPIO.
*
* Parameters:
*               pinName:
*                   Pin name, one value of Enum_PinName.
*               level:
*                   The initial level of GPIO, one value of Enum_PinLevel.
* Return:
*               QL_RET_OK, this function succeeds.
*               QL_RET_ERR_PARAM, the input GPIO is invalid.
*               QL_RET_ERR_ALREADYUNSUBCRIBE, can't operate,Maybe the GPIO not Init.
*****************************************************************/
s32 Ql_GPIO_SetLevel(Enum_PinName pinName, Enum_PinLevel level);

/*****************************************************************
* Function:     Ql_GPIO_SetDirection 
* 
* Description:
*               This function sets the direction of the specified GPIO.
*
* Parameters:
*               pinName:
*                   [in] Pin name, one value of Enum_PinName.
*               dir:
*                   [in] Set direction of GPIO, one value of Enum_PinDirection.
* Return:        
*               QL_RET_OK indicates this function successes.
*               QL_RET_ERR_PARAM, the input GPIO is invalid. 
*               QL_RET_ERR_ALREADYUNSUBCRIBE, can't operate,Maybe the GPIO not Init.
*****************************************************************/
s32 Ql_GPIO_SetDirection(Enum_PinName pinName, Enum_PinDirection dir);

/*****************************************************************
* Function:     Ql_GPIO_GetDirection 
* 
* Description:
*               This function gets the direction of the specified GPIO.
*
* Parameters:
*               pinName:
*                   [in]Pin name, one value of Enum_PinName.
* Return:        
*               The direction of the specified GPIO, which in Enum_PinDirection.
*               QL_RET_ERR_PARAM, the input GPIO is invalid. 
*               QL_RET_ERR_ALREADYUNSUBCRIBE, can't operate,Maybe the GPIO not Init.
*****************************************************************/
s32 Ql_GPIO_GetDirection(Enum_PinName pinName);


/*****************************************************************
* Function:     Ql_GPIO_GetLevel
*
* Description:
*               This function gets the level of the specified GPIO.
*
* Parameters:
*               pinName:
*                   Pin name, one value of Enum_PinName.
* Return:
*               The level value of the specified GPIO, which in Enum_PinLevel.
*               QL_RET_ERR_PARAM, the input GPIO is invalid. 
*               QL_RET_ERR_ALREADYUNSUBCRIBE, can't operate,Maybe the GPIO not Init.
*****************************************************************/
s32 Ql_GPIO_GetLevel(Enum_PinName pinName);


/*****************************************************************
* Function:     Ql_GPIO_Uninit 
* 
* Description:
*               This function releases the specified GPIO that was 
*               initialized by calling Ql_GPIO_Init() previously.
*               After releasing, the GPIO can be used for other purpose.
* Parameters:
*               pinName:
*                   [in]Pin name, one value of Enum_PinName.
* Return:        
*               QL_RET_OK, this function succeeds.
*               QL_RET_ERR_PARAM, the input GPIO is invalid. 
*               QL_RET_ERR_ALREADYUNSUBCRIBE, can't operate,Maybe the GPIO not Init.
*****************************************************************/
s32 Ql_GPIO_Uninit(Enum_PinName pinName);


/*****************************************************************
* Function:     Ql_EINT_Init 
* 
* Description:
*               Initialize an external interrupt function. 
*
* Parameters:
*               eintPinName:
*                   [in]EINT pin name, one value of Enum_PinName that has 
*                   the interrupt function.
*
*               eintType:
*                   [in]Interrupt type, supports edge-triggered interrupt.
*
*		    	swDebounce:
*	                [in]Software debounce. Unit in ms.(0-1000ms)
* Return:        
*               QL_RET_OK, this function succeeds.
*               QL_RET_ERR_PARAM, the input IO is invalid. 
*               QL_RET_ERR_NOSUPPORTSET, swDebounce is More than 1000ms.
*               QL_RET_ERR_PINALREADYSUBCRIBE, can't operate,Maybe the GPIO already Init.
*				QL_RET_ERR_NOSUPPORTEINT,eint pin more than the maximum(maximum is 3 now).
*****************************************************************/
typedef void (*Callback_EINT_Handle)(Enum_PinName pinName);
s32 Ql_EINT_Init(Enum_PinName pinName ,Enum_EintType eintType ,u32 swDebounce,Callback_EINT_Handle eint_isr );


/*****************************************************************
* Function:     Ql_EINT_Uninit 
* 
* Description:
*               This function releases the specified EINT pin that was 
*               initialized by calling Ql_EINT_Init() previously.
*               After releasing, the pin can be used for other purpose.
* Parameters:
*               eintPinName:
*                   EINT pin name, one value of Enum_PinName that has 
*                   the interrupt function.
* Return:        
*               QL_RET_OK, this function succeeds.
*               QL_RET_ERR_PARAM, the input GPIO is invalid. 
*               QL_RET_ERR_ALREADYUNSUBCRIBE, can't operate,Maybe the GPIO not Init.
*****************************************************************/
s32 Ql_EINT_Uninit(Enum_PinName pinName);


#endif  // __QL_GPIO_H__

