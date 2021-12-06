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
 *   example_float_math.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   This example demonstrates how to use standard c lib math in QuecOpen.
 *
 *   All debug information will be output through DEBUG port.
 *
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "GLOBAL_EXPORT_FLAG += __EXAMPLE_FLOAT_MATH__" in makefile file. And compile the 
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
#ifdef __EXAMPLE_FLOAT_MATH__
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "cmsis_os2.h"
#include "ril.h"
#include "ql_power.h"
#include "ql_ps.h"
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
static char m_strTemp[100];
#define PI                      3.1415926  
#define EARTH_RADIUS            6378.137


static void MainUartRecvCallback(u32 event, void* dataPtr, u32 dataLen)
{
	if((USART_EVENT_RX_TIMEOUT == event) || (USART_EVENT_RECEIVE_COMPLETE == event))
	{
	    Ql_UART_Write(UART_PORT0,(u8 *)dataPtr,dataLen);
	}
}

static s32 pscallback(ENUMurcID eventId, void *param, u32 paramLen)
{
	CmiSimImsiStr *pCmiSimImsiInd = NULL;
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
			pCmiSimImsiInd = (CmiSimImsiStr *)param;
			APP_DEBUG("SIM_READY:[%s]\r\n",pCmiSimImsiInd->contents);
			msg.message = MSG_ID_URC_INDICATION;	
			msg.param1 = URC_SIM_CARD_STATE_IND;
			osMessageQueuePut(maintask_queue, &msg, 0,0);
			break;
		case URC_ID_SIM_REMOVED:
			APP_DEBUG("SIM_REMOVED\r\n");//param NULL
			break;

		default:
			break;
	}
	return 0;
}



// Calculate radian
static float radian(float d)  
{  
     return d * PI / 180.0;
}  

static float get_distance(float lat1, float lng1, float lat2, float lng2)  
{  
     float radLat1 = radian(lat1);  
     float radLat2 = radian(lat2);  
     float a = radLat1 - radLat2;  
     float b = radian(lng1) - radian(lng2);  
	 
     float dst = 2 * asin((sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2) )));  
     dst = dst * EARTH_RADIUS;  
     dst= round(dst * 10000) / 10000;  
     return dst;  
} 

static void test_dis(void)
{  
     float lat1 = 38.173603;  
     float lng1 = 114.511623;
     float lat2 = 31.821171;  
     float lng2 = 117.205259;
     float dst ;

     // test: float as arguments
     dst = get_distance(lat1, lng1, lat2, lng2);  
     APP_DEBUG("dst = %0.3fkm\r\n", dst);  //dst = 748.492km  
} 

static void test_float(void)
{
    double a,b,s,pos;
    double radLat1 = 31.11;
    double radLat2 = 121.29;

    double lat = atof("58.12348976");
    double deg = atof("0.5678");
    double sec = 10.0;
    double sum = deg + sec;

    sec = round(lat);
    lat += sec;
    APP_DEBUG("sum=%f\r\n", lat+deg);
    APP_DEBUG("atof1=%.3f atof2=%f var=%f sum=%f\r\n", lat, deg, sec, sum);
    APP_DEBUG("atof1=%.3f atof2=%f var=%f\r\n", lat, deg, sec);    //Works OK, prints: "atof1=58.123 atof2=0.567800 var=58.000000"


    a = sin(45.0);
    b = cos(30.0);
    s = sqrt(81);
    pos = 2 * asin(sqrt(pow(sin(a/2),2) + cos(radLat1)*cos(radLat2)*pow(sin(b/2),2)));
    APP_DEBUG("Float test, a=%.2f,b=%.2f,radLat1=%.3f,radLat2=%.3f, s=%.5f\r\n", a,b,radLat1,radLat2,s);
    APP_DEBUG("Float test, pos=%g\r\n", pos);
    sprintf(m_strTemp, "Float test, a=%.2f,b=%.2f,radLat1=%.3f,radLat2=%.3f, s=%.5f\r\n", a,b,radLat1,radLat2,s);
    APP_DEBUG(m_strTemp);

}


void proc_main_task(void)
{
	s32 ret = -1;
	ST_MSG msg;
	u8 nw_state = 0;
	Ql_SleepDisable();

	Ql_RIL_Initialize();
 	Ql_UART_Open(UART_PORT0,115200,MainUartRecvCallback);
	
	APP_DEBUG("<-- QuecOpen: FLOAT MATH Example -->\r\n");
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
		            switch (msg.param1)
		            {
		            	case URC_EGPRS_NW_STATE_IND:
							{
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
			case MSG_ID_APP_START:
				{
					test_float();
	    			test_dis();
				}
				break;
        	default:
            	break;
        }
	}
}


#endif

