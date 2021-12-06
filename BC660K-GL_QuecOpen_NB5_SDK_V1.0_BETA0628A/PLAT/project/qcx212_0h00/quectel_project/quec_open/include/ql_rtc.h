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
 *   ql_rtc.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *  RTC related APIs
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
 

#ifndef __QL_RTC_H__
#define __QL_RTC_H__

#include "ql_type.h"


/**************************************************************
 * RTC callback
 **************************************************************/
typedef void(*callback_rtc_func)(u32 rtc_id);


/*****************************************************************
* Function:     Ql_RTC_Start 
* 
* Description:
*               Start up a rtc with the specified rtc id.
*
* Parameters:
*               rtcId:
*                       [in] rtc id,bigger than 0x00.
*               interval:
*                       [in] Set the interval of the rtc, unit: ms.
*					    when the RTC interval is less than or equal to 10s, the module may can not enter deep sleep mode.
*               autoRepeat:
*                       [in] TRUE indicates that the rtc is executed repeatedly.
*                            FALSE indicates that the rtc is executed only once.
*               callback_onTimer:
*                       [out] Notify the application when the time of the rtc arrives.
* Return:
*               QL_RET_OK indicates start ok.
*               QL_RET_ERR_PARAM indicates the param error.
*               QL_RET_ERR_RTC_RUNNING RTC is running.
* Notes:
*    
*****************************************************************/
s32 Ql_RTC_Start(u32 rtc_id, u32 interval, bool auto_repeat, callback_rtc_func callback_timer);

/*****************************************************************
* Function:     Ql_RTC_GetStatus 
* 
* Description:
*               Query RTC status by rtc id.
*
* Parameters:
*               rtcId:
*                       [in] rtc id
* Return:
*               QL_RET_OK indicates query ok.
*               QL_RET_ERR_PARAM indicates the param error.
*               QL_RET_ERR_RTC_RUNNING RTC is running.
*               QL_RET_ERR_RTC_NOTRUNNING RTC is not running.
* Notes:
*    
*****************************************************************/
s32 Ql_RTC_GetStatus(u32 rtc_id);

/*****************************************************************
* Function:     Ql_RTC_Stop 
* 
* Description:
*               Stop the rtc with the specified rtc id.
*
* Parameters:
*               rtcId:
*                   [in] the rtc id. The rtc has been started.
*                   by calling Ql_RTC_Start previously.
* Return:
*               QL_RET_OK indicates stop ok.
*               QL_RET_ERR_PARAM indicates the param error.
*               QL_RET_ERR_RTC_NOTRUNNING  rtc is not running.
* Notes:
*    
*****************************************************************/
s32 Ql_RTC_Stop(u32 rtc_id);

#endif  // __QL_RTC_H__

