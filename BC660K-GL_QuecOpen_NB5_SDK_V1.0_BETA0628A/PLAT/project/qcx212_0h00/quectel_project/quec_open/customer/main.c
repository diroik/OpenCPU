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
 *   app.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   This app demonstrates how to send AT command with RIL API, and transparently
 *   transfer the response through MAIN UART. And how to use UART port.
 *   Developer can program the application based on this example.
 * 
 ****************************************************************************/

#ifdef __CUSTOMER_CODE__
#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"
#include "ril.h"
#include "ql_uart.h"
#include "ql_ps.h"

#include "ql_dbg.h"
#include "ql_power.h"

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
							
void UartSendATCmd(u8* dataPtr, u32 dataLen);
static s32 ATResponse_Handler(const char *pStr, u32 strLen, void *pArg);

static void MainRecvCallback(u32 event, void* dataPtr, u32 dataLen)
{
	if((USART_EVENT_RX_TIMEOUT == event) || (USART_EVENT_RECEIVE_COMPLETE == event))
	{
	    UartSendATCmd(dataPtr, dataLen);
	}
}
static void AuxRecvCallback(u32 event, void* dataPtr, u32 dataLen)
{
	if((USART_EVENT_RX_TIMEOUT == event) || (USART_EVENT_RECEIVE_COMPLETE == event))
	{
	    UartSendATCmd(dataPtr, dataLen);
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
	s32 ret=0;
	ST_MSG msg;
	u8 nw_state = 0;
	char *ceregbuf = "AT+CEREG?\r\n";
	char *cfunDisable = "AT+CFUN=0\r\n";
	
	ret = Ql_UART_Open(UART_PORT0,115200,MainRecvCallback);
	
 	ret = Ql_UART_Open(UART_PORT1,115200,AuxRecvCallback);

	Ql_RIL_Initialize();

	APP_DEBUG("<------------- QuecOpen: Main App Example ----------->\r\n");

	ret = Ql_PSCallback_Register(GROUP_PS_MASK,pscallback);
    APP_DEBUG("<------------- QuecOpen: Ql_PSCallback_Register:[%d] ----------->\r\n",ret);

	maintask_queue = osMessageQueueNew(MAINTASK_QUEUE_LEN, sizeof(msg), NULL);

    Ql_SleepEnable();

    while (1)
	{	
		osDelay(2000);
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
						Ql_RIL_SendATCmd(ceregbuf,strlen(ceregbuf),NULL,NULL,0);
						ret = Ql_GetCeregState(&nw_state);
		                if((1== nw_state) || (5 == nw_state))
		                {
							ceregCheckTimes = 1;
						
		                	msg.message = MSG_ID_APP_START;	
							osMessageQueuePut(maintask_queue, &msg, 0,0);
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
								Ql_RIL_SendATCmd(cfunDisable,strlen(cfunDisable),NULL,NULL,0);
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
	                	APP_DEBUG("<-- Other URC: type = %d\r\n", msg.param1);
	                	break;
	            }
			}
	        	break;

			default:
				break;				
    	}
	}
}

void UartSendATCmd(u8* dataPtr, u32 dataLen)
{
    s32 ret;

    if(0 == dataLen)
    {
        return;
    }
    
    // Echo
    Ql_UART_Write(UART_PORT0, dataPtr, dataLen);

    ret = Ql_RIL_SendATCmd((char* )dataPtr, dataLen, (Callback_ATResponse)ATResponse_Handler, NULL, 0);
    APP_DEBUG("<-- UartSendATCmd,ret = %d\r\n", ret);
}

static s32 ATResponse_Handler(const char *pStr, u32 strLen, void *pArg)
{
    APP_DEBUG("[ATResponse_Handler] receData= [%s].-->\r\n",pStr);

    return 0;
}

#endif

