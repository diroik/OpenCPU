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
 *   example_multitask.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   This example demonstrates how to use multitask function and os APIs in QuecOpen.
 *   os code is open source,BC260Y is a freertos-based product,It also supports the cmsis API,
 *	 If you don't know much about rtos, you can refer to the relevant information online
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "GLOBAL_EXPORT_FLAG += __EXAMPLE_MULTITASK__" in makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
 * 
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

#ifdef __EXAMPLE_MULTITASK__
#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"
#include "ril.h"
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

static void MainUartRecvCallback(u32 event, void* dataPtr, u32 dataLen)
{
	if((USART_EVENT_RX_TIMEOUT == event) || (USART_EVENT_RECEIVE_COMPLETE == event))
	{
	    Ql_UART_Write(UART_PORT0,(u8 *)dataPtr,dataLen);
	}
}

static s32 PsCallback(ENUMurcID eventId, void *param, u32 paramLen)
{
	ST_MSG msg;
	
	switch (eventId)
	{
		case URC_ID_PS_BEARER_ACTED:
			msg.message = MSG_ID_URC_INDICATION;	
			msg.param1 = URC_EGPRS_NW_STATE_IND;
			osMessageQueuePut(maintask_queue, &msg, 0,0);
			
			break;
		case URC_ID_PS_BEARER_DEACTED:
			msg.message = MSG_ID_URC_INDICATION;	
			msg.param1 = URC_EGPRS_NW_STATE_IND;
			osMessageQueuePut(maintask_queue, &msg, 0,0);
			
			break;			
		case URC_ID_SIM_READY:
			msg.message = MSG_ID_URC_INDICATION;	
			msg.param1 = URC_SIM_CARD_STATE_IND;
			osMessageQueuePut(maintask_queue, &msg, 0,0);
			
			break;
		default:
			break;
	}
	return 0;
}


void proc_main_task(void)
{
	ST_MSG msg;
	u8 nw_state = 0;
	Ql_SleepDisable();
	
	Ql_RIL_Initialize();
 	Ql_UART_Open(UART_PORT0,115200,MainUartRecvCallback);
	Ql_PSCallback_Register(GROUP_ALL_MASK,PsCallback);
	
	APP_DEBUG("<-- QuecOpen: MULTITASK Example -->\r\n");
	maintask_queue = osMessageQueueNew(MAINTASK_QUEUE_LEN, sizeof(msg), NULL);
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
						Ql_GetCeregState(&nw_state);
		                if((1== nw_state) || (5 == nw_state))
		                {
							ceregCheckTimes = 1;
						}
						else
		                {
							ceregCheckTimes++;
							if(ceregCheckTimes > 120)
							{
								ceregCheckTimes = 1;
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
	                	//APP_DEBUG("<-- Other URC: type = %d\r\n", msg.param1);
	                	break;
	            }
			}
            	break;
			case MSG_ID_APP_START:
			{
				
			}
				break;
        	default:
            	break;
        }
       
	}

}


static osSemaphoreId_t test_semaphore = NULL;
static osMessageQueueId_t test_queue = NULL;
void sub_task1(void)
{
	u32 msg;
	osStatus_t ret = osError;
	osStatus_t ret1 = osError;
	osDelay(500);
	APP_DEBUG("<-- sub_task1 entry -->\r\n");
	test_semaphore = osSemaphoreNew(1U, 0, NULL);
	test_queue = osMessageQueueNew(2, sizeof(u32), NULL);
	APP_DEBUG("<-- 1.sub_task1 wait Semaphore-->\r\n");
	osSemaphoreAcquire (test_semaphore, osWaitForever); // wait for sub task2 Release
	APP_DEBUG("<-- 4.sub_task1 send msgqueue-->\r\n");
	osMessageQueuePut(test_queue, &msg, 0,0);
	APP_DEBUG("<-- 5.sub_task1 wait Semaphore again-->\r\n");
	osSemaphoreAcquire (test_semaphore, osWaitForever); // wait for sub task2 Release
	ret = osSemaphoreDelete (test_semaphore);
	ret1 = osMessageQueueDelete (test_queue);
	APP_DEBUG("<-- 7.sub_task1 Delete Sem[%d] and Que[%d] -->\r\n",ret,ret1);
	while(1)
	{
		osDelay(5000);
		APP_DEBUG("<-- sub_task1 is alive -->\r\n");
	}
}

void sub_task2(void)
{
	u32 msg;
	osDelay(500);
	APP_DEBUG("<-- sub_task2 entry -->\r\n");
	APP_DEBUG("<-- 2.sub_task2 Release semaphore-->\r\n");
	osSemaphoreRelease (test_semaphore); 
	APP_DEBUG("<-- 3.sub_task2 wait msgqueue-->\r\n");
	osMessageQueueGet(test_queue,(void*)&msg, NULL, osWaitForever);  // wait for the message from sub task1
	APP_DEBUG("<-- 6.sub_task2 Release semaphore again-->\r\n");
	osSemaphoreRelease (test_semaphore); 
	
	while(1)
	{
		osDelay(5000);
		APP_DEBUG("<-- sub_task2 is alive -->\r\n");
	}
}


#endif

