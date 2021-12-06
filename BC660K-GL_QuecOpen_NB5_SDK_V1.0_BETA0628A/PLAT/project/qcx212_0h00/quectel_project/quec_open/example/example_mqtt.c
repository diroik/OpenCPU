/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Quectel Co., Ltd. 2019
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   example_mqtt.c
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   This example demonstrates how to use MQTT function with APIs in OpenCPU.
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "GLOBAL_EXPORT_FLAG += __EXAMPLE_MQTT__" in Makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
 * 
 * note:
 *     
 *============================================================================
 *             HISTORY
 *----------------------------------------------------------------------------
 * 
 ****************************************************************************/
#ifdef __EXAMPLE_MQTT__
#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"

#include "ril.h"
#include "ril_mqtt.h"
#include "ql_dbg.h"
#include "ql_power.h "
#include "ql_ps.h"
#include "ql_urc_register.h"


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

/*****************************************************************
* define process state
******************************************************************/
typedef enum{
    STATE_NW_QUERY_STATE,
    STATE_MQTT_CFG,
    STATE_MQTT_OPEN,
    STATE_MQTT_CONN,
    STATE_MQTT_SUB,
    STATE_MQTT_PUB,
    STATE_MQTT_TUNS,
    STATE_MQTT_CLOSE,
    STATE_MQTT_DISC,
    STATE_MQTT_TOTAL_NUM
}Enum_ONENETSTATE;

/****************************************************************************
* Definition for APN
****************************************************************************/
#define APN      "CMNET\0"
#define USERID   ""
#define PASSWD   ""

/*****************************************************************
* MQTT  timer param
******************************************************************/
#define MQTT_TIMER_ID         0x200
#define MQTT_TIMER_PERIOD     500

/*****************************************************************
* Server Param
******************************************************************/
#define SRVADDR_BUFFER_LEN    100

//#define HOST_NAME             "yourproductkey.iot-as-mqtt.cn-shanghai.aliyuncs.com"
#define HOST_NAME             "220.180.239.212"//"other cloud platform"
#define HOST_PORT             8265//1883

/*****************************************************************
*  MQTT Param
******************************************************************/
MQTT_Urc_Param_t*	  mqtt_urc_param_ptr = NULL;
ST_MQTT_topic_info_t  mqtt_topic_info_t;
bool DISC_flag  = TRUE;
bool CLOSE_flag = TRUE;

/*****************************************************************
*  Sample Param
******************************************************************/
Enum_ConnectID connect_id = ConnectID_0;
u32 pub_message_id = 0;
u32 sub_message_id = 0;

u8 product_key[] =   "your-productkey\0";   //<ali cloud needs it.
u8 device_name[]=    "your-devicename\0";   //<ali cloud needs it.
u8 device_secret[] = "your-devicesecret\0"; //<ali cloud needs it.
u8 clientID[] =      "your-clientID\0";
u8 username[] =      "your-username\0";
u8 passwd[] =        "your-passwd\0";

static u8 test_data[128] =  "hello cloud,this is quectel test code!!!\0"; //<first packet data
static u8 test_topic[128] = "The topic that need to be subscribed and published\0"; //<topic
char uart_read_buff[1024] = {0x00};
static u8 m_mqtt_state = STATE_NW_QUERY_STATE;
static u8 ceregCheckTimes = 1;
extern osMessageQueueId_t maintask_queue;

/*****************************************************************
* Uart callback function
******************************************************************/
static void MainUartRecvCallback(u32 event, void* dataPtr, u32 dataLen)
{
	ST_MSG msg;
	char *qTest = "AT+QTEST?\r\n";
	
	if((USART_EVENT_RX_TIMEOUT == event) || (USART_EVENT_RECEIVE_COMPLETE == event))
	{
        if(dataLen > 0)
        {
            s32 ret;
            char *p = NULL;
            char* pData= NULL;
            
            memset(uart_read_buff, 0x00, sizeof(uart_read_buff));
            memcpy(uart_read_buff, dataPtr, dataLen);

            pData = uart_read_buff;
			
            p = strstr(pData,"DISC");
			if(p)
			{
		        ret = RIL_MQTT_QMTDISC(connect_id);
                if(RIL_AT_SUCCESS == ret)
                {
                    APP_DEBUG("//<Start disconnect MQTT socket\r\n");
                    if(TRUE == DISC_flag)
                    {
                        DISC_flag  = FALSE;
                    } 
                }
				else
                {
                    APP_DEBUG("//<Disconnect MQTT socket failure,ret = %d\r\n",ret); 
                }
				m_mqtt_state = STATE_MQTT_TOTAL_NUM;
				return;
			}
			
            p = strstr(pData,"CLOSE");
			if(p)
			{
				ret = RIL_MQTT_QMTCLOSE(connect_id);
                if (RIL_AT_SUCCESS == ret)
                {
                    APP_DEBUG("//<Start closed MQTT socket\r\n");
                    if(TRUE == CLOSE_flag)
                    {
                        CLOSE_flag = FALSE;
                    }
                }else
                {
                    APP_DEBUG("//<Closed MQTT socket failure,ret = %d\r\n",ret);
                    
			    }
				m_mqtt_state = STATE_MQTT_TOTAL_NUM;

				return;
			}
		
            p = strstr(pData,"RECONN");
			if(p)
			{
                if((FALSE == DISC_flag)||(FALSE == CLOSE_flag))
                {
					msg.message = MSG_ID_APP_START;	
					msg.param1 = STATE_MQTT_OPEN;
					osMessageQueuePut(maintask_queue, &msg, 0,0);
					
                    m_mqtt_state = STATE_MQTT_OPEN;
					
                    APP_DEBUG("\r\n");
                }

				APP_DEBUG("//<Start RECONN m_mqtt_state:%d\r\n",m_mqtt_state);
				if(STATE_NW_QUERY_STATE == m_mqtt_state)
				{
					msg.message = MSG_ID_URC_INDICATION;	
					msg.param1 = URC_EGPRS_NW_STATE_IND;
					osMessageQueuePut(maintask_queue, &msg, 0,0);
				}
				
				return;
			}
			
			p = strstr(pData,"QTEST");
			if(p)
			{
                Ql_RIL_SendATCmd(qTest,strlen(qTest),NULL,NULL,0);
				return;
			}

            //<The rest of the UART data is published as data.
            {
                if((TRUE == DISC_flag)&&(TRUE == CLOSE_flag))
                {
                    pub_message_id++;  // The range is 0-65535. It will be 0 only when<qos>=0.
    				ret = RIL_MQTT_QMTPUB(connect_id,pub_message_id,QOS1_AT_LEASET_ONCE,0,test_topic,dataLen, (u8*)pData);
                    if (RIL_AT_SUCCESS == ret)
                    {
                        APP_DEBUG("//<Start publish a message to server\r\n");
                    }else
                    {
                        APP_DEBUG("//<Publish a message to server failure,ret = %d,m_mqtt_state = %d\r\n",ret, m_mqtt_state);
                    }
                }
                else
                {
                    //<No connection to the cloud platform, just echo.
                    APP_DEBUG("\r\n//<No connection to the cloud platform, just echo.\r\n");
					Ql_UART_Write(UART_PORT0, (u8 *)dataPtr,dataLen);
                }
				return;
            }
        }
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

static void callback_mqtt_recv(const char* buffer,u32 length)
{					  
	APP_DEBUG("//<Receive messages(%s) from MQTT server\r\n", buffer);    					 
}


static void app_mqtt_demo_entry(ST_MSG msg);
void proc_main_task(void)
{
	s32 ret = -1;
	ST_MSG msg;
	u8 nw_state = 0; 
	char *ceregbuf = "AT+CEREG?\r\n";
	char *cfunDisable = "AT+CFUN=0\r\n";
	
	maintask_queue = osMessageQueueNew(MAINTASK_QUEUE_LEN, sizeof(msg), NULL);
	
	Ql_RIL_Initialize();
 	Ql_UART_Open(UART_PORT0, 115200, MainUartRecvCallback);
	ret = Ql_PSCallback_Register(GROUP_PS_MASK,pscallback);
	
	APP_DEBUG("<-- QuecOpen: MQTT Example -->\r\n");

	//register mqtt callback
    ret = Ql_Mqtt_Recv_Register(callback_mqtt_recv);
	APP_DEBUG("<--register recv callback successful(%d)-->\r\n",ret);

//	Ql_SleepDisable();
	
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
						Ql_RIL_SendATCmd(ceregbuf,strlen(ceregbuf),NULL,NULL,0);
						ret = Ql_GetCeregState(&nw_state);
		                if((1== nw_state) || (5 == nw_state))
		                {
							ceregCheckTimes = 1;
						
		                	msg.message = MSG_ID_APP_START;	
							msg.param1 = STATE_MQTT_CFG;
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
						break;	
					}
					case URC_MQTT_OPEN:
					{
						mqtt_urc_param_ptr = (MQTT_Urc_Param_t*)msg.param2;
    					if(0 == mqtt_urc_param_ptr->result)
    					{
         					APP_DEBUG("//<Open a MQTT client successfully\r\n");
                            m_mqtt_state = STATE_MQTT_CONN;
							
		                    msg.message = MSG_ID_APP_START;	
							msg.param1 = STATE_MQTT_CONN;		
							osMessageQueuePut(maintask_queue, &msg, 0,0);
						}
    					else
    					{
    						APP_DEBUG("//<Open a MQTT client failure,error = %d\r\n",mqtt_urc_param_ptr->result);
    					}
						break;
					}
					case URC_MQTT_CONN:
					{
						mqtt_urc_param_ptr = (MQTT_Urc_Param_t*)msg.param2;
    					if(0 == mqtt_urc_param_ptr->result)
    					{
            		        APP_DEBUG("//<Connect to MQTT server successfully\r\n");
    						m_mqtt_state = STATE_MQTT_SUB;
							
							msg.message = MSG_ID_APP_START;	
							msg.param1 = STATE_MQTT_SUB;		
							osMessageQueuePut(maintask_queue, &msg, 0,0);
    					}
    					else
    					{
    						APP_DEBUG("//<Connect to MQTT server failure,error = %d\r\n",mqtt_urc_param_ptr->result);
    					}
						break;
					}
					case URC_MQTT_SUB:
					{
						mqtt_urc_param_ptr = (MQTT_Urc_Param_t*)msg.param2;
    					if((0 == mqtt_urc_param_ptr->result)&&(128 != mqtt_urc_param_ptr->sub_value[0]))
    					{
            		        APP_DEBUG("//<Subscribe topics successfully\r\n");
    						m_mqtt_state = STATE_MQTT_PUB;

							msg.message = MSG_ID_APP_START;	
							msg.param1 = STATE_MQTT_PUB;		
							osMessageQueuePut(maintask_queue, &msg, 0,0);
    					}
    					else
    					{
    						APP_DEBUG("//<Subscribe topics failure,error = %d\r\n",mqtt_urc_param_ptr->result);
    					}
						break;
					}
					case URC_MQTT_PUB:
					{
						mqtt_urc_param_ptr = (MQTT_Urc_Param_t*)msg.param2;
    					if(0 == mqtt_urc_param_ptr->result)
    					{
            		        APP_DEBUG("//<Publish messages to MQTT server successfully\r\n");
    						m_mqtt_state = STATE_MQTT_TOTAL_NUM;
    					}
    					else
    					{
    						APP_DEBUG("//<Publish messages to MQTT server failure,error = %d\r\n",mqtt_urc_param_ptr->result);
    					}
						break;
					}
					case URC_MQTT_TUNS:
					{
						mqtt_urc_param_ptr = (MQTT_Urc_Param_t*)msg.param2;
    					if(0 == mqtt_urc_param_ptr->result)
    					{
            		        APP_DEBUG("//<UnSubscribe topics successfully\r\n");
    						m_mqtt_state = STATE_MQTT_TOTAL_NUM;
    					}
    					else
    					{
    						APP_DEBUG("//<UnSubscribe topics failure,error = %d\r\n",mqtt_urc_param_ptr->result);
    					}
						break;
					}
					case URC_MQTT_CLOSE:
					{
						mqtt_urc_param_ptr = (MQTT_Urc_Param_t*)msg.param2;
    					if(0 == mqtt_urc_param_ptr->result)
    					{
            		        APP_DEBUG("//<Closed MQTT socket successfully\r\n");
    					}
    					else
    					{
    						APP_DEBUG("//<Closed MQTT socket failure,error = %d\r\n",mqtt_urc_param_ptr->result);
    					}
						break;
					}
					case URC_MQTT_DISC:
					{
						mqtt_urc_param_ptr = (MQTT_Urc_Param_t*)msg.param2;
    					if(0 == mqtt_urc_param_ptr->result)
    					{
            		        APP_DEBUG("//<Disconnect MQTT successfully\r\n");
    					}
    					else
    					{
    						APP_DEBUG("//<Disconnect MQTT failure,error = %d\r\n",mqtt_urc_param_ptr->result);
    					}
						break;
					}
					case URC_MQTT_STATE:
					{
						mqtt_urc_param_ptr = (MQTT_Urc_Param_t*)msg.param2;
                    	APP_DEBUG("//<MQTT state report,connectID:%d,statecode:%d\r\n",mqtt_urc_param_ptr->connectid,mqtt_urc_param_ptr->mqtt_state);	
						break;
					}
	            	default:
	                	APP_DEBUG("<-- Other URC: type = %d\r\n", msg.param1);
	                	break;
	            }
			}
            	break;
			case MSG_ID_APP_START:
			{
				app_mqtt_demo_entry(msg);
			}
				break;
        	default:
           		break;
        }

		osDelay(10);	
	}

}

static void app_mqtt_demo_entry(ST_MSG msg)
{
	s32 ret = -1;

	osDelay(1000);
	switch (msg.param1)
	{
		case STATE_MQTT_CFG:
			{
				//ret = RIL_MQTT_QMTCFG_Ali(connect_id,product_key,device_name,device_secret);//<This configuration is required to connect to Ali Cloud.
                RIL_MQTT_QMTCFG_Showrecvlen(connect_id,ShowFlag_1);//<This sentence must be configured. The configuration will definitely succeed, so there is no need to care about.
                ret = RIL_MQTT_QMTCFG_Version_Select(connect_id,Version_3_1_1);
                if(RIL_AT_SUCCESS == ret)
                {
                    //APP_DEBUG("//<Ali Platform configure successfully\r\n");
                    APP_DEBUG("//<Select version 3.1.1 successfully\r\n");
					msg.message = MSG_ID_APP_START;	
					msg.param1 = STATE_MQTT_OPEN;		
					osMessageQueuePut(maintask_queue, &msg, 0,0);
                }
                else
                {
                    //APP_DEBUG("//<Ali Platform configure failure,ret = %d\r\n",ret);
                    APP_DEBUG("//<Select version 3.1.1 failure,ret = %d\r\n",ret);
                }
			    break;
			}	
		
		case STATE_MQTT_OPEN:
			{
				ret = RIL_MQTT_QMTOPEN(connect_id,HOST_NAME,HOST_PORT);
                if(RIL_AT_SUCCESS == ret)
                {
                    APP_DEBUG("//<Start opening a MQTT client\r\n");
                    if(FALSE == CLOSE_flag)
                        CLOSE_flag = TRUE;
                }
                else
                {
                    APP_DEBUG("//<Open a MQTT client failure,ret = %d-->\r\n",ret);
                }
				break;
			}
	
		case STATE_MQTT_CONN:
			{
				ret = RIL_MQTT_QMTCONN(connect_id,clientID,username,passwd);
	            if(RIL_AT_SUCCESS == ret)
                {
                    APP_DEBUG("//<Start connect to MQTT server\r\n");
                    if(FALSE == DISC_flag)
                        DISC_flag = TRUE;
                }
                else
                {
                    APP_DEBUG("//<connect to MQTT server failure,ret = %d\r\n",ret);
                }
				break;
			}
		case STATE_MQTT_SUB:
			{
				mqtt_topic_info_t.count = 1;
				mqtt_topic_info_t.topic[0] = (u8*)malloc(sizeof(u8)*256);
				
				memset(mqtt_topic_info_t.topic[0],0,256);
				memcpy(mqtt_topic_info_t.topic[0],test_topic,strlen((char *)test_topic));
                mqtt_topic_info_t.qos[0] = QOS1_AT_LEASET_ONCE;
				sub_message_id++;  //< 1-65535.
				
				ret = RIL_MQTT_QMTSUB(connect_id,sub_message_id,&mqtt_topic_info_t);
				
				free(mqtt_topic_info_t.topic[0]);
	            mqtt_topic_info_t.topic[0] = NULL;
                if(RIL_AT_SUCCESS == ret)
                {
                    APP_DEBUG("//<Start subscribe topic\r\n");
                }
                else
                {
                    APP_DEBUG("//<Subscribe topic failure,ret = %d\r\n",ret);
                }
				break;
			}
		case STATE_MQTT_PUB:
			{
				pub_message_id++;  //< The range is 0-65535. It will be 0 only when<qos>=0.
				ret = RIL_MQTT_QMTPUB(connect_id,pub_message_id,QOS1_AT_LEASET_ONCE,0,test_topic,strlen((char *)test_data),test_data);
                if(RIL_AT_SUCCESS == ret)
                {
                    APP_DEBUG("//<Start publish a message to MQTT server\r\n");
                    APP_DEBUG("//<Start publish a message\r\n");
                    msg.message = MSG_ID_APP_START;	
					msg.param1 = STATE_MQTT_TOTAL_NUM;		
					osMessageQueuePut(maintask_queue, &msg, 0,0);
                }
                else
                {
					msg.message = MSG_ID_APP_START;	
					msg.param1 = STATE_MQTT_DISC;		
					osMessageQueuePut(maintask_queue, &msg, 0,0);
                    APP_DEBUG("//<Publish a message to MQTT server failure,ret = %d\r\n",ret);
                }
				break;
			}
		case STATE_MQTT_TOTAL_NUM:
			{			
				APP_DEBUG("// STATE_MQTT_TOTAL_NUM\r\n");
				//else do nothing
				break;
			}
	
		default:
			break;
	}
}

#endif // __EXAMPLE_MQTT__

