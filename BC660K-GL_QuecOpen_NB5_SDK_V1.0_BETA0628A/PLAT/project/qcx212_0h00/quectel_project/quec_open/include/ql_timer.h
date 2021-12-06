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
 *   ql_timer.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   timer API defines.
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

#ifndef __QL_TIMER_H__
#define __QL_TIMER_H__

// header file content
#include "osasys.h"
#include "ql_type.h"
#include "ql_error.h"

// OS Timer
/****************************************************************************
*
* Description:	If needed to program os timer, pls refer the "cmsis_os2.h"file.
*
****************************************************************************/

/**************************************************************
 * User TIMER ID Definition
 **************************************************************/
#define	TIMER_ID_USER_START    0x100

typedef void(*Callback_Timer_OnTimer)(u32 timerId, void* param);

/*****************************************************************
* Function:     Ql_TIMER_Register 
* 
* Description:
*               Register stack timer, the whole opencpu project supports 30 stack timers.
*
* Parameters:
*               timerId:
*                       [in] The timer id must be bigger than 0xFF.Of course, the ID that registered 
*                            by "Ql_TIMER_RegisterFast" also cannot be the same with it.
*                            
*               callback_onTimer:
*                       [In] Notify the application when the time of the timer arrives.
*               param:
*                       [in] Used to pass parameters of customers.
* Return:
*               QL_RET_OK indicates register ok;
*               QL_RET_ERR_PARAM indicates the param error.
*               QL_RET_ERR_INVALID_TIMER indicates that the timer ID is already being used  
*               						 or the timer is started or stopped.
*               QL_RET_ERR_TIMER_FULL indicates all timers are used up.
*****************************************************************/
s32 Ql_TIMER_Register(u32 timerId, Callback_Timer_OnTimer callback_onTimer, void* param);


/*****************************************************************
* Function:     Ql_TIMER_RegisterFast 
* 
* Description:
*               Register GP timer, the whole opencpu project supports 5 GP timers.
*
* Parameters:
*               timerId:
*                       [in] The timer id must be bigger than 0xFF.And make sure that the id is
*                            not the same with the ID that registered by "Ql_TIMER_Register".
*
*               callback_onTimer:
*                       [In] Notify the application when the time of the timer arrives.
*               param:
*                       [in] Used to pass parameters of customers.
* Return:
*               QL_RET_OK indicates register ok;
*               QL_RET_ERR_PARAM indicates the param error.
*               QL_RET_ERR_INVALID_TIMER indicates that the timer ID is already being used.
*               QL_RET_ERR_TIMER_FULL indicates all timers are used up.
*****************************************************************/
s32 Ql_TIMER_RegisterFast(u32 timerId, Callback_Timer_OnTimer callback_onTimer, void* param);


/*****************************************************************
* Function:     Ql_TIMER_Start 
* 
* Description:
*               Start up a timer with the specified timer id.
*
* Parameters:
*               timerId:
*                       [in] timer id, bigger than 0xFF,the timer id must be registed.
*               interval:
*                       [in] Set the interval of the timer, unit: ms.
*                            the interval must be greater than or equal to 1ms.
*               autoRepeat:
*                       [in] TRUE indicates that the timer is executed repeatedly.
*                            FALSE indicates that the timer is executed only once.
* Return:
*               QL_RET_OK indicates register ok;
*               QL_RET_ERR_PARAM indicates the param error.
*               QL_RET_ERR_INVALID_TIMER indicates that the timer ID is already being used  
*               or the timer is started or stopped
*               QL_RET_ERR_TIMER_FULL indicates all timers are used up.
*****************************************************************/
s32 Ql_TIMER_Start(u32 timerId, u32 interval, bool autoRepeat);


/*****************************************************************
* Function:     Ql_TIMER_Stop 
* 
* Description:
*               Stop the timer with the specified timer id.
*
* Parameters:
*               timerId:
*                   [in] the timer id. The timer has been started 
*                   by calling Ql_TIMER_Start previously.
* Return:
*               QL_RET_OK indicates register ok;
*               QL_RET_ERR_PARAM indicates the param error.
*               QL_RET_ERR_INVALID_TIMER indicates that the timer ID is already being used  
*               or the timer is started or stopped
*               QL_RET_ERR_TIMER_FULL indicates all timers are used up.  
*****************************************************************/
s32 Ql_TIMER_Stop(u32 timerId);


/*****************************************************************
* Function:     Ql_TIMER_Delete 
* 
* Description:
*			Delete the timer with the specified timer id. 
*			If you want re_register timer id or release current timer id, you can use this API.
*
* Parameters:
*               timerId:
*                   [in] the timer id. The timer has been started 
*                   by calling Ql_TIMER_Start previously.
* Return:
*               QL_RET_OK indicates register ok;
*               QL_RET_ERR_PARAM indicates the param error.
*               QL_RET_ERR_INVALID_TIMER indicates that the timer ID is already being used  
*               or the timer is started or stopped
*               QL_RET_ERR_TIMER_FULL indicates all timers are used up.
*****************************************************************/
s32 Ql_TIMER_Delete(u32 timerId);


#endif

