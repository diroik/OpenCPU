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
 *   ql_time.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   UART API defines.
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
#ifndef __QUECTEL_RTC_TIME_H__
#define __QUECTEL_RTC_TIME_H__


// header file content
#include "osasys.h"
#include "ql_type.h"
#include "ql_error.h"

typedef struct 
{
    u16 	year;
    u8 	    month;
    u8      day;
    u8		hour;
    u8		minute;
    u8		second;
    s32		time_zone;	// The range is(-47~48), one digit expresses 15 minutes, for example: 32 indicates "GMT+8"
} ST_Time;

/****************************************************************************
* Function: 		Ql_TIME_Get
*
* Description: 		This function is used to get UTC time and timezone,
* 						when the NITZ time is got, the time will be updated.
*
* Parameters:
*       			dateTime:
*       				[out] the defaut time is 2000.1.1 00:00:00
* Return: 
*					QL_RET_OK get system time successfully
*					QL_RET_ERR_PARAM invalid parameter
*					QL_RET_ERR_GET_DATETIME get current datetime error
****************************************************************************/
s32 Ql_TIME_Get(ST_Time *dateTime);


/****************************************************************************
* Function: 		Ql_TIME_Set
*
* Description: 		This function is used to set UTC time and 1/4 timezone,
* 						Set system time, the time must since 2000.1.1
* Parameters:
*       			dateTime:
*       				[In] the time must since 2000.1.1
*
* Return: 
*					QL_RET_OK set system time successfully
*					QL_RET_ERR_PARAM invalid parameter
*					QL_RET_ERR_SET_DATETIME set current datetime error
****************************************************************************/
s32 Ql_TIME_Set(ST_Time *dateTime);


/****************************************************************************
* Function: 		Ql_Mktime
*
* Description: 		This function is used to get total seconds of UTC time elapsed 
*               		since 1970.01.01 00:00:00.
*
* Parameters: 
*					seconds:  
*       				[out] secs since 1970
*
* Return: 
*					QL_RET_OK get system time of secs successfully
*					QL_RET_ERR_PARAM invalid parameter
*					QL_RET_ERR_GET_DATETIME_SEC get current datetime of seconds error
****************************************************************************/
s32 Ql_Mktime(u32 *seconds);


#endif

