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
 

#ifndef __POWER_MGMT_H__
#define __POWER_MGMT_H__

#include "ql_type.h"

/* This enum defines the power on reason. */
typedef enum {
	/* First power on or hardware reset */
	QL_POWERON               = 0,  
	/*  software reset */
	QL_SOFT_RESET		     = 1,
	/* Wake up by RTC timer from deep sleep. TAU/ALARM(oc,onet)/USER RTC */
	QL_RTCWAKEUP			 = 2,
	/* Wake up by UART0-RXD from deep sleep */
	QL_RXD_WAKEUP	         = 3,	
	/* Wake up by PSM_EINT0 from deep sleep */
	QL_PSM_EINT0_WAKEUP	     = 4,
	/* Wake up by PSM_EINT1 from deep sleep */
	QL_PSM_EINT1_WAKEUP	     = 5,
	
    /* others reset */
    QL_UNKNOWN_PWRON,
} Enum_WakeUp_Reason;

/*****************************************************************
* Function:     Ql_GetWakeUpReason
*
* Description:
*               This function gets a reason for power on.
*
* Parameters:
*               None
*
* Return:
*               Returns as an enumerated type: Enum_PowerOnReason
*				
* Note:         QL_RTCWAKEUP - QL_PSM_EINT1_WAKEUP represents wake up
*               from deep sleep (PSM).
*****************************************************************/
Enum_WakeUp_Reason Ql_GetWakeUpReason(void);

/*****************************************************************
* Function:     Ql_SleepEnable 
* 
* Description:
*               Enable the module into sleep mode.
*               When the module enters deep sleep, the serial port 
*               will not work and the app no longer runs. Only RTC
*               timer and PSM_EINT pin(QL_RTCWAKEUP/QL_PSM_EINT_WAKEUP/
*               QL_PSM_EINT1_WAKEUP) can wake the module from deep sleep.
*
*****************************************************************/
void Ql_SleepEnable(void);

/*****************************************************************
* Function:     Ql_SleepDisable 
* 
* Description:
*               Disable the module into sleep mode.
*               If customer don't need deep sleep mode need to call 
*               this api to disable when power on.
*               If customer need deep sleep mode, we recommened that 
*               disable deep sleep first, and enable deep sleep after 
*               your app has completed all business, and configure the
*               RTC timer to automatically wake up.
*
*****************************************************************/
 void Ql_SleepDisable(void);

 /*****************************************************************
 * Function:	 ql_get_power_vol 
 * 
 * Description:
 *				 This function querys the voltage value of power supply.
 *
 * Parameters:
 *				 voltage:
 *					 [out] buffer to store the voltage value of power supply. 
 *
 * Return:		  
 *				 QL_RET_OK indicates get voltage successes.
 *				 QL_RET_ERR_ERROR   indicates failed to get voltage.
 *****************************************************************/
s32 Ql_GetPowerVol(u32* voltage);
 
 /*****************************************************************
* Function:     Ql_DeepSleep_Register
*
* Description:
* 			This function registers a callback handler of UCR: +QATSLEEP,
*			The callback will be called when the module enter deepsleep mode.
*
* Parameters:
*			callback_deepsleep:
*				[out] Notify the application when the module enter deepsleep.
*
* Return:
* 			QL_RET_OK indicates register ok;
*			QL_RET_ERR_PARAM indicates the param error.
*****************************************************************/
typedef void (*Callback_DeepSleep_Func)(u8* buffer,u32 length);
s32 Ql_DeepSleep_Register(Callback_DeepSleep_Func callback_deepsleep);


#endif

