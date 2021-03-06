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
 *   ql_timer.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *  Timer related APIs
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
#include "ql_psm_eint.h"

/**************************************************************
 * User RTC  ID Definition
 **************************************************************/
#define     RTC_ID_USER_START    0x200
typedef void(*Callback_Rtc_Func)(u32 rtcId, void* param);
/*****************************************************************
* Function:     Ql_Rtc_RegisterFast 
* 
* Description:
*               Register rtc, only support one RTC in the whole quecopen project.
*
* Parameters:
*               timerId:
*                       [in] The rtc id must be bigger than 0xFF.
*               callback_onTimer:
*                       [out] Notify the application when the time of the rtc arrives.
*               param:
*                       [in] Used to pass parameters of customers.
* Return:
*               QL_RET_OK indicates register ok;
*               QL_RET_ERR_PARAM indicates the param error.
*               QL_RET_ERR_INVALID_TIMER indicates that the rtc ID is already registered
*               or the timer is started or stopped not in the same task with it registered.
*               QL_RET_ERR_TIMER_FULL indicates rtc is used up.
*               QL_RET_ERR_INVALID_TASK_ID indicates the task invalid.
* Notes:
*
*    
*****************************************************************/
s32 Ql_Rtc_RegisterFast(u32 rtcId, Callback_Rtc_Func callback_onTimer, void* param);

/*****************************************************************
* Function:     Ql_Rtc_Start 
* 
* Description:
*               Start up a rtc with the specified rtc id.
*
* Parameters:
*               timerId:
*                       [in] rtc id, bigger than 0xFF,the rtc id must be registed.
*               interval:
*                       [in] Set the interval of the rtc, unit: ms.
*                           this value must be is the multiple of 100ms
*               autoRepeat:
*                       [in] TRUE indicates that the rtc is executed repeatedly.
*                            FALSE indicates that the rtc is executed only once.
* Return:
*               QL_RET_OK indicates register ok;
*               QL_RET_ERR_PARAM indicates the param error.
*               QL_RET_ERR_INVALID_TIMER indicates that the rtc ID is already registered
*               or the timer is started or stopped not in the same task with it registered.
*               QL_RET_ERR_INVALID_TASK_ID indicates the task invalid.
* Notes:
*    
*****************************************************************/
s32 Ql_Rtc_Start(u32 rtcId, u32 interval, bool autoRepeat);

/*****************************************************************
* Function:     Ql_Rtc_Stop 
* 
* Description:
*               Stop the rtc with the specified rtc id.
*
* Parameters:
*               timerId:
*                   [in] the rtc id. The rtc has been started 
*                   by calling Ql_rtc_Start previously.
* Return:
*               QL_RET_OK indicates register ok;
*               QL_RET_ERR_PARAM indicates the param error.
*               QL_RET_ERR_INVALID_TIMER indicates that the rtc ID is already registered
*               or the timer is started or stopped not in the same task with it registered.
*               QL_RET_ERR_INVALID_TASK_ID indicates the task invalid.
* Notes:
*    
*****************************************************************/
s32 Ql_Rtc_Stop(u32 rtcId);

#endif  // End-of __QL_TIMER_H__

