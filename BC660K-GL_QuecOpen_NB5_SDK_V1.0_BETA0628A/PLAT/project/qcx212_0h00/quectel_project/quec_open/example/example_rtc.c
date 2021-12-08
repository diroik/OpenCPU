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
 *   example_rtc.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   This example demonstrates how to use rtc function with APIs in QuecOpen.
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "C_PREDEF=-D __EXAMPLE_RTC__" in gcc_makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
 * 
 *   Operation:
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
#ifdef __EXAMPLE_RTC__

#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"
#include "ril.h"
#include "ql_rtc.h"
#include "ql_power.h"
#include "ql_ps.h"
#include "ril.h"
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

/*****************************************************************
* rtc param
******************************************************************/
static u32 Rtc_id = 0x101;
static u32 Rtc_Interval = 5*60*1000; // 5min

static void MainUartRecvCallback(u32 event, void* dataPtr, u32 dataLen)
{
	if((USART_EVENT_RX_TIMEOUT == event) || (USART_EVENT_RECEIVE_COMPLETE == event))
	{
		APP_DEBUG("[MainUartReceiveCallback]recv data:%s.-->\r\n",(u8*)dataPtr);
	}
}

void callback_deepsleep_cb(u8* buffer,u32 length)
{
	/*****************************************************************
	* Ql_Data_DirectSend is only used here
	* Warning:do not use Ql_UART_Write or APP_DEBUG here!!!
	******************************************************************/
	Ql_Data_DirectSend(PORT_DBG_LOG,buffer,length);
}

/*****************************************************************
* Attention:
*	We suggest not doing a lot of things in Rtc_handler,since RTC timeout indication comes from
*	module initialization.Taking a long time in callback may influence the initialization.
*	We can set a flag ,and do things in open task!
* 	
*	Since the serial port has not been initialized when the Rtc_handler is triggered, 
*	Ql_UART_Write and APP_DEBUG should not be used in Rtc_handler
*****************************************************************/
void Rtc_handler(u32 rtcId)
{
    if(Rtc_id == rtcId)
    {
        APP_DEBUG("Rtc_handler: Rtc_id(0x%x),please add your service\r\n", Rtc_id);
		// add custom code below 
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
	s32 ret = 0;
	u8 nw_state = 0;	
	ST_MSG msg;
    static Enum_WakeUp_Reason pwr_reason;
	
 	Ql_UART_Open(UART_PORT0,115200,MainUartRecvCallback);
	maintask_queue = osMessageQueueNew(MAINTASK_QUEUE_LEN, sizeof(msg), NULL);
	Ql_RIL_Initialize();
	Ql_PSCallback_Register(GROUP_PS_MASK,PsCallback);
    Ql_DeepSleep_Register(callback_deepsleep_cb);

	APP_DEBUG("<-- QuecOpen: RTC Example -->\r\n");
	pwr_reason = Ql_GetWakeUpReason();
	if(QL_RTCWAKEUP == pwr_reason)//rtc
	{
		APP_DEBUG("<--The module wake up by rtc from deep sleep mode -->\r\n"); 
	}
	else if(QL_SOFT_RESET == pwr_reason)// soft reset
	{
		APP_DEBUG("<--The module soft reset -->\r\n");
    }
	else if(QL_POWERON == pwr_reason)// power
	{
		APP_DEBUG("<--The module first power on or hard reset -->\r\n");
    }
    else if(QL_PSM_EINT0_WAKEUP == pwr_reason)// eint0 pin
	{
		APP_DEBUG("<--The module wake up by psm_eint0 from deep sleep mode -->\r\n");  
    }
	else if(QL_PSM_EINT1_WAKEUP == pwr_reason)// eint1 pin
	{
		APP_DEBUG("<--The module wake up by psm_eint1 from deep sleep mode-->\r\n");
    }
    else if(QL_RXD_WAKEUP == pwr_reason)// rxd
	{
		APP_DEBUG("<--The module wake up by mainuart from deep sleep mode -->\r\n");  
    }


    while (1)
    {        
    	if(osOK == osMessageQueueGet(maintask_queue,(void*)&msg, NULL, osWaitForever))
		switch(msg.message)
        {		
        	case MSG_ID_RIL_READY://Wake up and start doing business
        	{
				APP_DEBUG("<-- RIL is ready -->\r\n");
        	}
            	break;			
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
							
                            //start a rtc ,repeat=true;
                            ret = Ql_RTC_Start(Rtc_id,Rtc_Interval,TRUE, Rtc_handler);
                            if(ret < 0)
                            {	 
                                if(-4 == ret)
                                {
                                    APP_DEBUG("<--the rtc is already start-->\r\n");	 
                                }
                                else
                                {
                                    APP_DEBUG("<--rtc start failed!!:ret=%d-->\r\n",ret);	
                                }
                            }
                            else
                            {
                                APP_DEBUG("<--rtc start successful:RTC id(%d),interval(%d),ret(%d)-->\r\n",Rtc_id,Rtc_Interval,ret);
                            }
                            
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
	                	APP_DEBUG("<-- Other URC: type = %d\r\n", msg.param1);
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

#endif // __EXAMPLE_RTC__

