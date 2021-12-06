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
 *   example_time.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   This example demonstrates how to use time function with APIs in OpenCPU.
 *   It display how to set and get local time.You can modify struct ST_time to set diffrent local time.
 *   All debug information will be output through DEBUG port.
 *
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "GLOBAL_EXPORT_FLAG += __EXAMPLE_TIME__" in makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
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
#ifdef __EXAMPLE_TIME__
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "cmsis_os2.h"
#include "ril.h"
#include "ril_ntp.h"
#include "ril_network.h"
#include "ql_power.h"
#include "ql_ps.h"
#include "ql_time.h"
#include "ql_dbg.h"


#define DEBUG_ENABLE 1
#if DEBUG_ENABLE > 0
#define DEBUG_PORT  PORT_DBG_LOG
#define DBG_BUF_LEN   512
static char DBG_BUFFER[DBG_BUF_LEN];
#define APP_DEBUG(FORMAT,...) {\
    memset(DBG_BUFFER, 0, DBG_BUF_LEN);\
    snprintf(DBG_BUFFER,DBG_BUF_LEN,FORMAT,##__VA_ARGS__); \
    if (UART_PORT2 == (DEBUG_PORT)) \
    {\
        Ql_Debug_Trace((u8* )DBG_BUFFER);\
    } else {\
		Ql_UART_Write((Enum_SerialPort)(DEBUG_PORT), (u8*)(DBG_BUFFER), strlen((const char *)(DBG_BUFFER)));\
    }\
}
#else
#define APP_DEBUG(FORMAT,...) 
#endif

static u8 ceregCheckTimes = 1;


extern osMessageQueueId_t maintask_queue;
Ql_CellListInfo_t Curr_cellInfo = {0};
ST_Time date_time = {0};
u32 is_first_start = 0;


static void MainUartRecvCallback(u32 event, void* dataPtr, u32 dataLen)
{
	if((USART_EVENT_RX_TIMEOUT == event) || (USART_EVENT_RECEIVE_COMPLETE == event))
	{
	    Ql_UART_Write(UART_PORT0,(u8 *)dataPtr,dataLen);
	}
}

static s32 pscallback(ENUMurcID eventId, void *param, u32 paramLen)
{
	CmiMmNITZInd *pMmNitzInd = NULL;
	CmiSimImsiStr *pCmiSimImsiInd = NULL;
	ST_MSG msg;
	
	switch (eventId)
	{
		case URC_ID_PS_BEARER_ACTED:
			msg.message = MSG_ID_URC_INDICATION;	
			msg.param1 = URC_EGPRS_NW_STATE_IND;
			osMessageQueuePut(maintask_queue, &msg, 0,0);
			APP_DEBUG("BEARER_ACTED\r\n");//param NULL
			
			break;
		case URC_ID_PS_BEARER_DEACTED:
			msg.message = MSG_ID_URC_INDICATION;	
			msg.param1 = URC_EGPRS_NW_STATE_IND;
			osMessageQueuePut(maintask_queue, &msg, 0,0);
			APP_DEBUG("BEARER_DEACTED\r\n");//param NULL
			
			break;
		case URC_ID_SIM_READY:
			pCmiSimImsiInd = (CmiSimImsiStr *)param;
			APP_DEBUG("SIM_READY:[%s]\r\n",pCmiSimImsiInd->contents);
			msg.message = MSG_ID_URC_INDICATION;	
			msg.param1 = URC_SIM_CARD_STATE_IND;
			osMessageQueuePut(maintask_queue, &msg, 0,0);
			
			break;
		case URC_ID_SIM_REMOVED:
			APP_DEBUG("SIM_REMOVED\r\n");//param NULL
			
			break;
		case URC_ID_MM_NITZ_REPORT:
			pMmNitzInd = (CmiMmNITZInd *)param;
			if(pMmNitzInd->fullNwNameLen)
			{
				APP_DEBUG("NITZ_REPORT fullNwName:[%s]\r\n",pMmNitzInd->fullNwName);
			}
			if(pMmNitzInd->shortNwNameLen)
			{
				APP_DEBUG("NITZ_REPORT shortNwName:[%s]\r\n",pMmNitzInd->shortNwName);

			}
			if(pMmNitzInd->utcInfoPst)
			{
				APP_DEBUG("NITZ_REPORT date:%d.%d.%d\r\n",pMmNitzInd->utcInfo.year,pMmNitzInd->utcInfo.mon,pMmNitzInd->utcInfo.day);
				APP_DEBUG("NITZ_REPORT utctime:%d.%d.%d\r\n",pMmNitzInd->utcInfo.hour,pMmNitzInd->utcInfo.mins,pMmNitzInd->utcInfo.sec);
				APP_DEBUG("NITZ_REPORT timeZone:%d,%d\r\n",pMmNitzInd->utcInfo.tz,pMmNitzInd->localTimeZone);
			}				
	    	msg.message = MSG_ID_APP_START;	
			osMessageQueuePut(maintask_queue, &msg, 0,0);
			
			break;			
		default:
			//APP_DEBUG("undef eventID:[%x]\r\n",eventId);
			break;
	}
	return 0;
}

static void NTP_Callback(char *strURC)
{
	APP_DEBUG("<-- [NTP_Callback]:%s\r\n", strURC);	
}

void proc_main_task(void)
{
	s32 ret = -1;
	u32 curDateTimerSec = 0;
	ST_MSG msg;
	u8 nw_state = 0;
	
	Ql_SleepDisable();
	
	Ql_RIL_Initialize();
 	Ql_UART_Open(UART_PORT0,115200,MainUartRecvCallback);
	
	APP_DEBUG("<-- QuecOpen: Time example -->\r\n");
	ret = Ql_PSCallback_Register(GROUP_ALL_MASK,pscallback);
	osDelay(1000);
	maintask_queue = osMessageQueueNew(MAINTASK_QUEUE_LEN, sizeof(msg), NULL);

    while (1)
    {
		if(osOK == osMessageQueueGet(maintask_queue,(void*)&msg, NULL, osWaitForever))
		switch(msg.message)
        {		
        	case MSG_ID_URC_INDICATION:
			{
	            //APP_DEBUG("<-- Received URC: type: %d, -->\r\n", msg.param1);
	            switch (msg.param1)
	            {
	            	case URC_SIM_CARD_STATE_IND:
	            	{
	            	}	
	                	break;					
	            	case URC_EGPRS_NW_STATE_IND:
					{
						ret = Ql_GetCeregState(&nw_state);
		                if((1== nw_state) || (5 == nw_state))
		                {
							ceregCheckTimes = 1;
							
		                    APP_DEBUG("<-- Module has registered to network, status:%d -->\r\n",nw_state);
							
		                }
						else
		                {
							ceregCheckTimes++;
							if(ceregCheckTimes > 120)
							{
								ceregCheckTimes = 1;
		                    	APP_DEBUG("<-- Module has deregister to network, status:%d,ret:%d -->\r\n",nw_state,ret);

								// register network fail
							}
							else
							{
								msg.message = MSG_ID_URC_INDICATION;	
								msg.param1 = URC_EGPRS_NW_STATE_IND;
								osMessageQueuePut(maintask_queue, &msg, 0,0);
								osDelay(500);
							}
		                }
					}
	                	break; 					
	            	default:
	                	APP_DEBUG("<-- Other URC: type = %d -->\r\n", msg.param1);
	                	break;
	            }
			}
            	break;
			case MSG_ID_APP_START:
			{
				if(0 == is_first_start)
				{
					u8* server = "ntp.aliyun.com\0";//"pool.ntp.org\0";//"time.google.com";//"time.windows.com\0";//
					// 1. get date time 
					if(QL_RET_OK == Ql_TIME_Get(&date_time))
					{
						if(date_time.time_zone >= 0)
						{
							APP_DEBUG("<-- current date time:%d-%02d-%02d %02d:%02d:%02d +%d -->\r\n", 
											date_time.year, date_time.month, date_time.day,
											date_time.hour, date_time.minute, date_time.second,
											date_time.time_zone);
						}
						else
						{
							APP_DEBUG("<-- date time:%d-%02d-%02d %02d:%02d:%02d %d -->\r\n", 
											date_time.year, date_time.month, date_time.day,
											date_time.hour, date_time.minute, date_time.second,
											date_time.time_zone);
						}
					}
					// 2. get date time of seconds
					if(QL_RET_OK == Ql_Mktime(&curDateTimerSec))
					{
						APP_DEBUG("<-- current date time of second: %d -->\r\n", curDateTimerSec);
					}
					
					// 3. get date time by NTP
					ret = RIL_NTP_START(server, 0, NTP_Callback);
					APP_DEBUG("<-- RIL_NTP_START:%d \r\n",ret);
					is_first_start++;
				}
				
				// 4. get Random number
				Ql_Mktime(&curDateTimerSec);
				srand(curDateTimerSec);			
				ret = rand()%100+1;
				APP_DEBUG("<-- Random number:%d \r\n",ret);
				
				osDelay(1000);
            	msg.message = MSG_ID_APP_START;	
				osMessageQueuePut(maintask_queue, &msg, 0,0);
			}
				break;
        	default:
            	break;
        }       
	}
}
#endif

