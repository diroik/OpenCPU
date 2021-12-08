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
 *   example_dfota_http.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   This example demonstrates how to use dfota_http function with RIL APIs in QuecOpen.
 *   Input the specified command through any uart port and the result will be
 *   output through the debug port.
 *   App bin or core must be put in server first.It will be used to upgrade data through the air.
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "GLOBAL_EXPORT_FLAG += __EXAMPLE_DFOTA_HTTP__" in makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
 * 
 *   Operation:
 *  
 *     step 1: you must put your application file in your server.
 *     step 2: replace the "APP_ZIP_URL" with your own .
 *     step 3: input string : start dfota=XXXX, XXXX stands for URL.
 *
 *     The URL format for http is:   http://hostname:port/filePath/fileName                                
 *     NOTE:  if ":port" is be ignored, it means the port is http default port(80) 
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
#ifdef __EXAMPLE_DFOTA_HTTP__

#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"	
#include "ql_ps.h"
#include "ril.h"
#include "ril_dfota.h"
#include "ql_power.h"
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

#define APP_ZIP_URL   "http://220.180.239.212:8050/Yance/BC660K-GL/update.zip\0"

static u8 ceregCheckTimes = 1;

extern osMessageQueueId_t maintask_queue;

static void MainUartRecvCallback(u32 event, void* dataPtr, u32 dataLen)
{
	if((USART_EVENT_RX_TIMEOUT == event) || (USART_EVENT_RECEIVE_COMPLETE == event))
	{
	    Ql_UART_Write(UART_PORT0,(u8 *)dataPtr,dataLen);
	}
}

static s32 pscallback(ENUMurcID eventId, void *param, u32 paramLen)
{
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
		default:
			break;
		}
	return 0;
}

void proc_main_task(void)
{
	s32 ret = -1;
	ST_MSG msg;
	u8 nw_state = 0;

    Ql_SleepDisable();
    
	maintask_queue = osMessageQueueNew(8, sizeof(msg), NULL);
	
	Ql_RIL_Initialize();
 	Ql_UART_Open(UART_PORT0,115200,MainUartRecvCallback);
	ret = Ql_PSCallback_Register(GROUP_PS_MASK,pscallback);
	osDelay(1000);
	
	APP_DEBUG("<-- QuecOpen: http dfota Example -->\r\n");
	
    while (1)
    {
		if(osOK == osMessageQueueGet(maintask_queue,(void*)&msg, NULL, osWaitForever))
		switch(msg.message)
        {		
        	case MSG_ID_URC_INDICATION:
			{
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
						
		                	RIL_DFOTA_Upgrade(APP_ZIP_URL);
							
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
	            }
			}
                break;
        	default:
                break;
        }
        osDelay(10);	
	}

}

#endif /* __EXAMPLE_FOTA_HTTP__ */

