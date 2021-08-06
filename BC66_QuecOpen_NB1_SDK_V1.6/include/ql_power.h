/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Quectel Co., Ltd. 2020
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   ql_power.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   Power APIs defines.
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
 

#ifndef __POWER_H__
#define __POWER_H__
#include "ql_type.h"

/** @brief This enum defines the power on result. */
typedef enum {
	QL_FIRST_BOOT_UP	   = 0,     /* first boot up*/
    QL_DEEP_SLEEP          = 1,     /* wakeup from deep sleep */
    QL_SYS_RESET           = 2,     /* sys_reset */
    QL_FORCED_SHUT_DOWN    = 3,     /* forced shut down */
    QL_WDT_HW_RESET        = 4,     /* wdt hw reset */
    QL_WDT_SW_RESET        = 5,     /* wdt sw reset */
	QL_FORCED_RESET 	   = 6, 	/* forced reset */
	QL_DEEPER_SLEEP 	   = 7, 	/* wakeup from deeper sleep */
    QL_POWER_ON_RESULT_MAX = 7
} Enum_Ql_Power_On_Result_t;

/** @brief This enum defines the wake up result. */
typedef enum {
	Ql_RESULT_NOT_WAKEUP   = 0,	 	/* power on. Not wakeup*/
    Ql_RTC_TC_WAKEUP       = 1,     /* rtc tc wakeup*/
	Ql_PSM_EINT_WAKEUP     = 2,	 	/* psm_eint wakeup*/
    Ql_RTC_AlARM_WAKEUP    = 3,	 	/* rtc timer wakeup*/
    Ql_POWER_KEY_WAKEUP    = 4,	 	/* power_key wakeup*/
    Ql_WAKE_UP_RESULT_MAX  = 4
} Enum_Ql_Wake_Up_Result_t;


/*****************************************************************
* Function:     Ql_SleepEnable 
* 
* Description:
*               Enable the module into deep sleep mode.
*
* Return:        
*               QL_RET_OK indicates this function successes.
*		   Ql_RET_NOT_SUPPORT this function not support.	
*****************************************************************/
s32 Ql_SleepEnable(void);

/*****************************************************************
* Function:     Ql_SleepDisable 
* 
* Description:
*               Disable the module into deep sleep mode.
*
* Return:        
*               QL_RET_OK indicates this function successes.
*		   Ql_RET_NOT_SUPPORT this function not support.	
*****************************************************************/
 s32 Ql_SleepDisable(void);

/*****************************************************************
* Function:     Ql_PowerDown
*
* Description:
*               Power down or restart the module.
*
* Parameters:
*               mode:
*				[in] 0  Normal power-off.
*					 1  Forced power-off.
*					 2  Reset the module.
*
* Return:
*               None.
*****************************************************************/
void Ql_PowerDown(u8 mode);

/*****************************************************************
* Function:     Ql_GetPowerVol 
* 
* Description:
*               This function querys the voltage value of power supply.
*
* Parameters:
*               voltage:
*                   [out] buffer to store the voltage value of power supply. 
*
* Return:        
*               QL_RET_OK indicates this function successes.
*               Others indicates failed.	
*****************************************************************/
s32 Ql_GetPowerVol(u32* voltage);

/*****************************************************************
* Function:     Ql_GetPowerOnReason
*
* Description:
*               This function gets a reason for power on.
*
* Parameters:
*               None
*
* Return:
*               The reason of the Power on, one value of Enum_Ql_Power_On_Result_t.
*****************************************************************/
s32 Ql_GetPowerOnReason(void);

/*****************************************************************
* Function:     Ql_GetWakeUpReason
*
* Description:
*               This function gets a reason for wake up the module.
*
* Parameters:
*               None
*
* Return:
*               The reason of the wake up, one value of Enum_Ql_Wake_Up_Result_t.
*****************************************************************/
Enum_Ql_Wake_Up_Result_t Ql_GetWakeUpReason(void);

/*****************************************************************
* Function:     Ql_DeepSleep_Register
*
* Description:
* 			This function registers a callback handler of UCR: ENTER DEEPSLEEP,
*			The callback will be called when the module enter deepsleep mode.
*
* Parameters:
*			callback_ds_event:
*				[out] Notify the application when the module enter deepsleep.
*		    param:
*				[in] User data.
*
* Return:
* 			QL_RET_OK indicates register ok;
*			QL_RET_ERR_PARAM indicates the param error.
*****************************************************************/
typedef void (*Callback_DeepSleep_Func)(void *param);
s32 Ql_DeepSleep_Register(Callback_DeepSleep_Func callback_ds_event,void * param);

#endif
