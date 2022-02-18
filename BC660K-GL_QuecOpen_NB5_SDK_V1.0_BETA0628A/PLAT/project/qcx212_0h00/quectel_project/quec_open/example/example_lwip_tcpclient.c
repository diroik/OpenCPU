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
 *   example_lwip_tcpclient.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *		
 *	This example demonstrates how to use LwIP API to connect TCP server, 
 *	but simple data sending and receiving demonstration.For specific use, please 
 *	refer to the relevant information of LwIP on the Internet. The specific LwIP 
 *	related header file is in the PLAT / middleware / thirdparty / lwip folder
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "GLOBAL_EXPORT_FLAG += __EXAMPLE_LWIP_TCPCLIENT__" in makefile file. And compile the 
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
#ifdef __EXAMPLE_LWIP_TCPCLIENT__

#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"
#include "lwip/netdb.h"
#include "ql_power.h"
#include "ql_ps.h"
#include "ril.h"
#include "ql_dbg.h"

#define DEBUG_ENABLE 0
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

/*****************************************************************
* define process state
******************************************************************/
typedef enum{
	TCP_STATE_SOC_CREATE,
	TCP_STATE_SOC_CONNECT,
	TCP_STATE_SOC_SEND,
	TCP_STATE_SOC_CLOSE,
	TCP_STATE_TOTAL_NUM
}Enum_TCPSTATE;

extern osMessageQueueId_t maintask_queue;
/*****************************************************************
* Server Param
******************************************************************/
#define  MSG_ID_APP_RECV_DATA   MSG_ID_APP_START+1
#define SRVADDR_BUFFER_LEN  100
#define SEND_BUFFER_LEN     256
#define RECV_BUFFER_LEN     256
#define   IP_ADDR_LEN   64 

static s32 m_socketid = -1; 
static u8 m_recv_buf[RECV_BUFFER_LEN];
//static s32 m_remain_len = 70;	 // record the remaining number of bytes in send buffer.
static char *m_pCurrentPos = "5175656374656C44616B6169\0"; 
static u8 ceregCheckTimes = 1;

struct addrinfo hints, *server_res;
static char serverip[] = "220.180.239.212";
static char serverport[] = "8426";
/*****************************************************************
* Server Param
******************************************************************/

static void MainUartRecvCallback(u32 event, void* dataPtr, u32 dataLen)
{
	if((USART_EVENT_RX_TIMEOUT == event) || (USART_EVENT_RECEIVE_COMPLETE == event))
	{
		APP_DEBUG("[MainUartReceiveCallback]recv data:%s.-->\r\n",(u8*)dataPtr);
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
			APP_DEBUG("BEARER_DEACTED\r\n");//param NULL
			
			break;
		default:
			break;
	}
	return 0;
}


static osStatus_t SendMsgToApp(Enum_TCPSTATE state)
{	
	ST_MSG msg;
	msg.message = MSG_ID_APP_START;	
	msg.param1 = state;
	return osMessageQueuePut(maintask_queue, &msg, 0,0);
}
static osStatus_t LoopToRecv(void)
{	
	ST_MSG msg;
	msg.message = MSG_ID_APP_RECV_DATA;	
	return osMessageQueuePut(maintask_queue, &msg, 0,0);
}


static void lwip_tcpclient_demo_entry(ST_MSG msg);
void proc_main_task(void)
{
	s32 ret = 0;
	u8 nw_state = 0;	
	ST_MSG msg;
	maintask_queue = osMessageQueueNew(MAINTASK_QUEUE_LEN, sizeof(msg), NULL);
	
 	Ql_UART_Open(UART_PORT0,115200,MainUartRecvCallback);
	Ql_PSCallback_Register(GROUP_PS_MASK,PsCallback);
	APP_DEBUG("<-- QuecOpen: lwip TCPclient Example -->\r\n");
	Ql_SleepDisable();
	
    memset( &hints, 0, sizeof( hints ) );
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    while (1)
    {
    	LoopToRecv();
    	if(osOK == osMessageQueueGet(maintask_queue,(void*)&msg, NULL, osWaitForever))
		switch(msg.message)
        {		
        	case MSG_ID_URC_INDICATION:
			{
	            switch (msg.param1)
	            {
	            	case URC_SIM_CARD_STATE_IND:
	                	break;					
	            	case URC_EGPRS_NW_STATE_IND:
					{
						ret = Ql_GetCeregState(&nw_state);
		                if((1== nw_state) || (5 == nw_state))
		                {
							ceregCheckTimes = 1;
							
							SendMsgToApp(TCP_STATE_SOC_CREATE);
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
				lwip_tcpclient_demo_entry(msg);
			}
				break;
			case MSG_ID_APP_RECV_DATA:
			{
				ret = recv(m_socketid, m_recv_buf, RECV_BUFFER_LEN,MSG_DONTWAIT);
				if(0 < ret)
				{
					APP_DEBUG("<-- recv data:(");
					Ql_UART_Write((Enum_SerialPort)(DEBUG_PORT),m_recv_buf,ret);
					APP_DEBUG("),len:%d\r\n",ret);					
				}else{
					Ql_Debug_Trace("No data came in\r\n");
				}
			}
				break;
        	default:
            	break;
        }
	osDelay(1000); 
    }
	
}




static void lwip_tcpclient_demo_entry(ST_MSG msg)
{	
	switch(msg.param1)
	{
		case TCP_STATE_SOC_CREATE:
		{	
			if (getaddrinfo( serverip, serverport , &hints, &server_res ) != 0 ) 
			{
			   APP_DEBUG("TCP connect unresolved dns");
			}
			m_socketid = socket(AF_INET, SOCK_STREAM, 0);
			if (m_socketid >= 0)
			{
				SendMsgToApp(TCP_STATE_SOC_CONNECT);
				APP_DEBUG("<--Create socket id successfully,socketid=%d.-->\r\n",m_socketid);
			}else
			{
				APP_DEBUG("<--Create socket id failure,error=%d.-->\r\n",m_socketid);
			}
			
			break;
		}
		case TCP_STATE_SOC_CONNECT:
		{
			if (connect(m_socketid, (struct sockaddr *) server_res->ai_addr, server_res->ai_addrlen) < 0 && errno != EINPROGRESS) 
			{
				APP_DEBUG("<--socket connect fail-->\r\n");
				close(m_socketid);
				break;
			}
			APP_DEBUG("<--socket connect success-->\r\n");
			SendMsgToApp(TCP_STATE_SOC_SEND);			
			break;
		}
		case TCP_STATE_SOC_SEND:
		{
			APP_DEBUG("start to send \r\n");	
			send( m_socketid, m_pCurrentPos, strlen(m_pCurrentPos)+1,0);
            APP_DEBUG("<--socket send success-->\r\n");	
			SendMsgToApp(TCP_STATE_SOC_SEND);		
			break;
		}
		case TCP_STATE_SOC_CLOSE:
		{
			if (m_socketid > 0)
            {
               close(m_socketid);
               m_socketid = -1;
            }
			SendMsgToApp(TCP_STATE_TOTAL_NUM);
			
			break;
		}
		default:
			break;
	}

}

#endif 

