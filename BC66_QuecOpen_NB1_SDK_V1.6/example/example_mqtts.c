/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Quectel Co., Ltd. 2020
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   example_ssl.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   This example demonstrates how to establish a SSL connection, when the module is used for the client.
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "C_PREDEF=-D __EXAMPLE_MQTTs" in gcc_makefile file. 
 *	 And compile the app using "make clean/new".
 *     Download image bin to module to run.
 * 
 * -------
 * -------
 *
 *============================================================================
 *             HISTORY
 *----------------------------------------------------------------------------
 * 
 ****************************************************************************/
#ifdef __EXAMPLE_MQTTs__

#include "custom_feature_def.h"
#include "ql_stdlib.h"
#include "ql_common.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_error.h"
#include "ql_uart.h"
#include "ql_timer.h"
#include "ril.h"
#include "ril_network.h"
#include "ril_ssl.h"
#include "ril_mqtt.h"
#include "ql_network.h"
#include "ql_socket.h"

#define DEBUG_ENABLE 1
#if DEBUG_ENABLE > 0
#define DEBUG_PORT  UART_PORT0
#define DBG_BUF_LEN   1500
static char DBG_BUFFER[DBG_BUF_LEN];
#define APP_DEBUG(FORMAT,...) {\
    Ql_memset(DBG_BUFFER, 0, DBG_BUF_LEN);\
    Ql_sprintf(DBG_BUFFER,FORMAT,##__VA_ARGS__); \
    if (UART_PORT2 == (DEBUG_PORT)) \
    {\
        Ql_Debug_Trace(DBG_BUFFER);\
    } else {\
        Ql_UART_Write((Enum_SerialPort)(DEBUG_PORT), (u8*)(DBG_BUFFER), Ql_strlen((const char *)(DBG_BUFFER)));\
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
	STATE_NW_GET_LOCALIP,
    STATE_MQTTs_CFG,
    STATE_MQTTs_OPEN,
    STATE_MQTTs_CONN,
    STATE_MQTTs_SUB,
    STATE_MQTTs_PUB,
    STATE_MQTTs_TUNS,
    STATE_MQTTs_DISC,
    STATE_MQTTs_TOTAL_NUM
}Enum_MQTTs_STATE;
static Enum_MQTTs_STATE m_mqtts_state = STATE_NW_QUERY_STATE;

/*****************************************************************
* UART Param
******************************************************************/
static Enum_SerialPort m_myUartPort  = UART_PORT0;

#define SERIAL_RX_BUFFER_LEN  2048
static u8 m_RxBuf_Uart[SERIAL_RX_BUFFER_LEN];

/*****************************************************************
* timer param
******************************************************************/
#define MQTTs_TIMER_ID          TIMER_ID_USER_START
#define MQTTs_TIMER_PERIOD      1000

/*****************************************************************
* ssl param
******************************************************************/

#define QMQTTs_SEND_DATA_MAX_LEN     (1024) //Maximum length of  total topic information.
#define QMQTTs_DATAMODE_MAX_LEN      (3*1024) // SSL datamode max length
#define QMQTTs_HOSTNAME_MAX_LEN      (150) //hostname length.

static u8 m_send_buf[QMQTTs_SEND_DATA_MAX_LEN] = "123400120d0a551a\0";
static u8 hostname[QMQTTs_HOSTNAME_MAX_LEN] = "a3pupxb4was62j-ats.iot.us-east-2.amazonaws.com\0";
static u32 svr_port = 8883;

/*****************************************************************
*  TEST Param
******************************************************************/
u8 tcp_connect_id = 0;
u32 pub_message_id = 0;
u32 sub_message_id = 0;
u8 clientID[] = "clientExample111\0";

static u8 test_data[128] = "hello quectel!!!\0"; //send data
static u8 test_topic[128] = "topic/example/tls\0"; //topic

/*****************************************************************
*  MQTT Param
******************************************************************/
MQTT_Urc_Param_t*	 mqtts_urc_param_ptr = NULL;
ST_MQTT_topic_info_t  mqtts_topic_info_t;

static u8 cacert[QSSL_DATAMODE_MAX_LEN] = "-----BEGIN CERTIFICATE-----\r\n\
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\r\n\
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\n\
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\r\n\
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n\
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\n\
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n\
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\n\
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\n\
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n\
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\n\
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\r\n\
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\r\n\
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\r\n\
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\r\n\
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\r\n\
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\r\n\
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\r\n\
rqXRfboQnoZsG4q5WTP468SQvvG5\r\n\
-----END CERTIFICATE-----\r\n\0";

static u8 clientcert[QSSL_DATAMODE_MAX_LEN] = "-----BEGIN CERTIFICATE-----\r\n\
MIIDWTCCAkGgAwIBAgIUaCdUOvasxESXqwStsV9hQuG6zX8wDQYJKoZIhvcNAQEL\r\n\
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\r\n\
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIwMDMyMzA4MzQy\r\n\
MloXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\r\n\
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAM5AqItp9Orch1K7Z97r\r\n\
kwqE7hfBtUHdceFrDq1nhFwisw7jEszoZznFGlJLU7fwhXzGcaoM6jxIXjYfrFBK\r\n\
Z/j+ftDm+DlD7Sn2MpWb1xRyimmfDs4V9lN0lh5nC3aOTgWa9o0tR0t/IObxvlWW\r\n\
IoXvDm5Qqf40u+LFX8llv77/Gl3cgxMShfrObLql2jwCvVMhjLrOOA/KHEk0j53M\r\n\
K8ktrMsX+f49YEy1zxS9R9GT+OAU9PcHN2oavB5o5DQSihNW9p19ckEoJ98+tOHA\r\n\
hDJ+JB0LbVgcykVAFxMIm+gdFhg08Undy9nuLRQwweHVpj27tipazHwK6fSUAcHH\r\n\
s90CAwEAAaNgMF4wHwYDVR0jBBgwFoAUzsAvr2FOt5Ls7iy5X1mtWCxqj3IwHQYD\r\n\
VR0OBBYEFHiwklTJv81RT6GohvPOc8Cl8HLIMAwGA1UdEwEB/wQCMAAwDgYDVR0P\r\n\
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCfiBhaJGEHp55hP1mthCQcvKk9\r\n\
ax/Svc83GbhzXvvJYJb8naPqbhaURGI3a0T/QDHFUYX5cGyi+RNp+tdvnCAZt1fF\r\n\
jrQeElagwJzYrKgNoRA1SICl1nA3SjErZl+yZGohduRNQnc/t9Yk3PAjdjuBTlVd\r\n\
C5CALc7H2Tcc1B+CGt3/c0EPrr6AJ26Mul3KRLdSbyi25OAl3x7mOwmmedEFa/aQ\r\n\
AP7bMSet/5R+UvTlgffl6eK3L9T9yFe+sm/v4leUjeAZ7dSbsouaJ6tH96NNkh2Z\r\n\
fpkFvsmBN3f+ntFFbLcivto9g1GB7pbQlP7NZA6aCDp4JADpo3f+tVXPyemv\r\n\
-----END CERTIFICATE-----\r\n\0";

static u8 clientkey[QSSL_DATAMODE_MAX_LEN] = "-----BEGIN RSA PRIVATE KEY-----\r\n\
MIIEpQIBAAKCAQEAzkCoi2n06tyHUrtn3uuTCoTuF8G1Qd1x4WsOrWeEXCKzDuMS\r\n\
zOhnOcUaUktTt/CFfMZxqgzqPEheNh+sUEpn+P5+0Ob4OUPtKfYylZvXFHKKaZ8O\r\n\
zhX2U3SWHmcLdo5OBZr2jS1HS38g5vG+VZYihe8OblCp/jS74sVfyWW/vv8aXdyD\r\n\
ExKF+s5suqXaPAK9UyGMus44D8ocSTSPncwryS2syxf5/j1gTLXPFL1H0ZP44BT0\r\n\
9wc3ahq8HmjkNBKKE1b2nX1yQSgn3z604cCEMn4kHQttWBzKRUAXEwib6B0WGDTx\r\n\
Sd3L2e4tFDDB4dWmPbu2KlrMfArp9JQBwcez3QIDAQABAoIBAAFkGj7mdgpndlou\r\n\
fWLZf+JgRyLN4aUSE0SL7ktpGVFtwntlOKk9IAUZuTW44FWrP5s502ZsM+A4NLp0\r\n\
uDrKZa0eOki6Zg4s0RqpWGn95RN0Ok7ADd+B8mlZrfGCjGc+SnRrOSJ2nbZ6owQr\r\n\
uTbmjlEM7bH8xUi7nlDHOAApuYNWPgOovjWbcxXRXH8Ly6YT7fSrAgAcddYXfCE/\r\n\
tS28B7YexiewJ1NvioWby3bI+gYbdboQ01sZZ0JjqYN/eMNoZRiW31J3qIIEEk3R\r\n\
SlA45z7I2sdOM52nw3QRCWWUf34+d5G3tJZYzl1aQKjTdGEtmRiuT+Ka7Zm59p/+\r\n\
f2bTMBkCgYEA6koiIdLHgfRsXzAgv/gUYDzAAxsiQ/2clK6RyoDMkSRHasaBw0zg\r\n\
bs83DCoA8oWAXcV4kuBvNPQPL7UgNkV+340MdGtbLdDPyYwCtd7h2/mf62mGXmcb\r\n\
7+AGRLow10a8k3wXk9X6K8Ava0TedIJpSthpJmMClcNk5oJaNW6jLE8CgYEA4V1s\r\n\
2FxKnVqve1mzCPaQhCA/jsR5A/ftsOFj+GeXq+7BB9TqL/r/I8rsL3fNPYCAlr7H\r\n\
k/EWEHtNob+XTB6OvaWV2p1LZ2DzZ6gHyyq1Q5/nvdI6Y5XZ/BYsO2bN4Sjt8IxR\r\n\
UK4oty0UeDUxK2ncj1xl7RNTdg12URTn51wRdhMCgYEA1Qh5hrXI8RKWI+t4K/Xg\r\n\
WS2EDo7ylk4ZvVwKds6ss+EAC0faoAHcJQXH8QiUefEIKruvCiaHF+g/ksqoMpD4\r\n\
bz8qrwUbEErJVWM457R1PHb7gawfIOGylmfma0G9vVG1kImKSsOLSZLjyy8A84HN\r\n\
SeADpILsy86yhaLLccBbIKMCgYEApQZ6i7yZTrx3P4YhmfCjRn0uzWaFjyhfv2Rs\r\n\
hhulbts4Mnnwuw27AwS3CtNZUS3l+3zcPDMQCyWfZVAchDrkH28/WoRQEPVnh4H9\r\n\
owI1Fb3kPpXQ99dldjjuTtkq7TPBeKHncYL2gZvbH2MkDDxKrBfm80FxnkKhXyJF\r\n\
5p7+kk0CgYEAjhv/pIMxAXmVKvKL1PrNXrO1FQv57ePktflWTBiJVLhz6dTYKuhe\r\n\
aBK+B72yyx9kcRUgKIFKJNO8HQfnTLR5VGTUlFkkDbtijT109DyE0BrrUdOlF8lD\r\n\
iFWqTV0h95FAp3RudH50DzfT4DhidY3V+hsRIIzbUgxsBQf996lz+P4=\r\n\
-----END RSA PRIVATE KEY-----\r\n\0";

/*****************************************************************
* ssl recv callback function
******************************************************************/
static void callback_ssl_recv(u8* buffer,u32 length);

/*****************************************************************
* uart callback function
******************************************************************/
static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara);

/*****************************************************************
* timer callback function
******************************************************************/
static void Callback_Timer(u32 timerId, void* param);

//SSL_Urc_Param_t* ssl_urc_param_ptr = NULL;
ST_SSL_Param_t ssl_param_t = {0,0,NULL,0,0,0,0,0,0,0,0,0};
ST_SSL_Datamode_Userdata_t userdata = {NULL,0};

static void callback_mqtts_recv(u8* buffer,u32 length)
{
	APP_DEBUG("<--ssl receive data: %s,len = %d-->\r\n",buffer,length);
	m_mqtts_state = STATE_MQTTs_DISC;
}

void proc_main_task(s32 taskId)
{
    ST_MSG msg;
	s32 ret;

    // Register & open UART port
    ret = Ql_UART_Register(m_myUartPort, CallBack_UART_Hdlr, NULL);
    if (ret < QL_RET_OK)
    {
        Ql_Debug_Trace("Fail to register serial port[%d], ret=%d\r\n", m_myUartPort, ret);
    }
	
    ret = Ql_UART_Open(m_myUartPort, 115200, FC_NONE);
    if (ret < QL_RET_OK)
    {
        Ql_Debug_Trace("Fail to open serial port[%d], ret=%d\r\n", m_myUartPort, ret);
    }
    APP_DEBUG("<--QuecOpen: MQTTs TEST.-->\r\n");

    //register process timer 
    Ql_Timer_Register(MQTTs_TIMER_ID, Callback_Timer, NULL);

	//register  recv callback
	Ql_Mqtt_Recv_Register(callback_mqtts_recv);
	APP_DEBUG("<--register recv callback successful(%d)-->\r\n",ret);

    while(TRUE)
    {
        Ql_OS_GetMessage(&msg);
        switch(msg.message)
        {
#ifdef __OCPU_RIL_SUPPORT__
        case MSG_ID_RIL_READY:
            APP_DEBUG("<-- RIL is ready -->\r\n");
            Ql_RIL_Initialize();
            break;
#endif
		case MSG_ID_URC_INDICATION:
		{     
			switch (msg.param1)
            {
    		    case URC_SIM_CARD_STATE_IND:
	    			APP_DEBUG("<-- SIM Card Status:%d -->\r\n", msg.param2);
     				if(SIM_STAT_READY == msg.param2)
     				{
                        Ql_Timer_Start(MQTTs_TIMER_ID, MQTTs_TIMER_PERIOD, TRUE);
     				}
					
    			break;
				
				case URC_MQTT_OPEN:
				{
					mqtts_urc_param_ptr = msg.param2;
					if(0 == mqtts_urc_param_ptr->result)
					{
     					APP_DEBUG("<-- open a MQTT client successfully-->\r\n");
                        m_mqtts_state = STATE_MQTTs_CONN;
					}
					else
					{
						APP_DEBUG("<-- open a MQTT client failure,error=%d.-->\r\n",mqtts_urc_param_ptr->result);
					}
				}
				break;
				
    		    case URC_MQTT_CONN:
				{
					mqtts_urc_param_ptr = msg.param2;
					if(0 == mqtts_urc_param_ptr->result)
					{
        		        APP_DEBUG("<-- connect to MQTT server successfully->\r\n");
						m_mqtts_state = STATE_MQTTs_SUB;
					}
					else
					{
						APP_DEBUG("<-- connect to MQTT failure,error=%d.-->\r\n",mqtts_urc_param_ptr->result);
					}
    		    }
    			break;
				
                case URC_MQTT_SUB:
				{
					mqtts_urc_param_ptr = msg.param2;
					if((0 == mqtts_urc_param_ptr->result)&&(128 != mqtts_urc_param_ptr->sub_value[0]))
					{
        		        APP_DEBUG("<-- subscribe topics successfully->\r\n");
						m_mqtts_state = STATE_MQTTs_PUB;
					}
					else
					{
						APP_DEBUG("<-- subscribe topics failure,error=%d.-->\r\n",mqtts_urc_param_ptr->result);
					}
    		    }
    			break;
				
				case URC_MQTT_PUB:
				{
					mqtts_urc_param_ptr = msg.param2;
					if(0 == mqtts_urc_param_ptr->result)
					{
        		        APP_DEBUG("<-- publish messages to ali server successfully->\r\n");
						m_mqtts_state = STATE_MQTTs_TOTAL_NUM;
					}
					else
					{
						APP_DEBUG("<-- publish messages to ali server failure,error=%d.-->\r\n",mqtts_urc_param_ptr->result);
					}
    		    }
    			break;
				
				case URC_MQTT_DISC:
				{
					mqtts_urc_param_ptr = msg.param2;
					if(0 == mqtts_urc_param_ptr->result)
					{
        		        APP_DEBUG("<-- disconnect to MQTT server successfully->\r\n");
						Ql_Timer_Stop(MQTTs_TIMER_ID);
					}
					else
					{
						APP_DEBUG("<-- disconnect to MQTT failure,error=%d.-->\r\n",mqtts_urc_param_ptr->result);
					}
    		    }
    			break;
				
				case URC_MQTT_STATE:
				{
					mqtts_urc_param_ptr = msg.param2;
					APP_DEBUG("<--MQTT status of connect id %d is %d-->\r\n",mqtts_urc_param_ptr->connectid,mqtts_urc_param_ptr->mqtt_state);
				}
				break;
				
				default:
					APP_DEBUG("<-- Other URC: type=%d\r\n", msg.param1);
				break;
			}
		}
		break;
		
		default:
			break;
        }
    }
}
 
static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara)
{

}

static void Callback_Timer(u32 timerId, void* param)
{
	s32 ret;
    if (MQTTs_TIMER_ID == timerId)
    {
    	switch (m_mqtts_state)
        {
        	case STATE_NW_QUERY_STATE:
			{
                s32 cgreg = 0;
				
                ret = RIL_NW_GetEGPRSState(&cgreg);
                APP_DEBUG("<-- Network State: cgreg = %d-->\r\n", cgreg);
				
                if((cgreg == NW_STAT_REGISTERED) || (cgreg == NW_STAT_REGISTERED_ROAMING))
                {
                    m_mqtts_state = STATE_NW_GET_LOCALIP;
                }
			}
			break;
			
			case STATE_NW_GET_LOCALIP:
            {
                ST_Addr_Info_t addr_info;

                Ql_memset(addr_info.addr, 0, IP_ADDR_LEN);
				addr_info.addr_len = IP_ADDR_LEN;
				
                ret = Ql_GetLocalIPAddress(0, &addr_info);
                if(ret == SOC_SUCCESS)
                {
                    APP_DEBUG("<-- Get Local IP Successfully, Local IP = %s -->\r\n",addr_info.addr);
                    m_mqtts_state = STATE_MQTTs_CFG;
                }
				else
                {
                    APP_DEBUG("<-- Get Local IP Failed, ret = %d -->\r\n", ret);
                }
                break;
            }

			case STATE_MQTTs_CFG:
			{
				ssl_param_t.contextid = 1;
		        ssl_param_t.connectid = 0;
		        ssl_param_t.cfg_param = Param_Item_Seclevel;
				ssl_param_t.seclevel = Auth_Mode_Server_and_Client;//Two-way Authentication
					
				ret = RIL_QSSL_CFG(&ssl_param_t, Cfg_Select_Config_Mode, NULL);
				if(ret == RIL_AT_SUCCESS)
				{
					APP_DEBUG("OK\r\n");
				}
				else
				{
					APP_DEBUG("ERROR\r\n");
					return;
				}

				if((ssl_param_t.seclevel == Auth_Mode_Server)||(ssl_param_t.seclevel == Auth_Mode_Server_and_Client))
				{
					ssl_param_t.contextid = 1;
			        ssl_param_t.connectid = 0;
			        ssl_param_t.cfg_param = Param_Item_Cacert;
					
					userdata.pbuffer = Ql_MEM_Alloc(sizeof(u8)*QSSL_DATAMODE_MAX_LEN);
					if(userdata.pbuffer != NULL)
					{
						Ql_memset(userdata.pbuffer, 0, QSSL_DATAMODE_MAX_LEN);
						Ql_memcpy(userdata.pbuffer, cacert, Ql_strlen(cacert));
					}
					
					userdata.length = Ql_strlen(cacert);
					ret = RIL_QSSL_CFG(&ssl_param_t, Cfg_Select_Config_Mode, &userdata);
					if(ret == RIL_AT_SUCCESS)
					{
						APP_DEBUG("OK\r\n");
						APP_DEBUG("<-- Configure cacert successfully!-->\r\n");
					}
					else
					{
						APP_DEBUG("ERROR\r\n");
						APP_DEBUG("<-- Configure cacert failed, ret = %d -->\r\n",ret);
						Ql_MEM_Free(userdata.pbuffer);
						userdata.pbuffer = NULL;
						return;
					}
				}

				if(ssl_param_t.seclevel == Auth_Mode_Server_and_Client)
				{
					ssl_param_t.contextid = 1;
					ssl_param_t.connectid = 0;
					ssl_param_t.cfg_param = Param_Item_Clientcert;
					
					userdata.pbuffer = Ql_MEM_Alloc(sizeof(u8)*QSSL_DATAMODE_MAX_LEN);
					if(userdata.pbuffer != NULL)
					{
						Ql_memset(userdata.pbuffer, 0, QSSL_DATAMODE_MAX_LEN);
						Ql_memcpy(userdata.pbuffer, clientcert, Ql_strlen(clientcert));
					}
					
					userdata.length = Ql_strlen(clientcert);
					ret = RIL_QSSL_CFG(&ssl_param_t, Cfg_Select_Config_Mode, &userdata);
					if(ret == RIL_AT_SUCCESS)
					{
						APP_DEBUG("OK\r\n");
						APP_DEBUG("<-- Configure clientcert successfully!-->\r\n");
					}
					else
					{
						APP_DEBUG("ERROR\r\n");
						APP_DEBUG("<-- Configure clientcert failed, ret = %d -->\r\n",ret);
						Ql_MEM_Free(userdata.pbuffer);
						userdata.pbuffer = NULL;
						return;
					}
					
					ssl_param_t.contextid = 1;
			        ssl_param_t.connectid = 0;
			        ssl_param_t.cfg_param = Param_Item_Clientkey;
						
					userdata.pbuffer = Ql_MEM_Alloc(sizeof(u8)*QSSL_DATAMODE_MAX_LEN);
					if(userdata.pbuffer != NULL)
					{
						Ql_memset(userdata.pbuffer, 0, QSSL_DATAMODE_MAX_LEN);
						Ql_memcpy(userdata.pbuffer, clientkey, Ql_strlen(clientkey));
					}
					
					userdata.length = Ql_strlen(clientkey);
					ret = RIL_QSSL_CFG(&ssl_param_t, Cfg_Select_Config_Mode, &userdata);
					if(ret == RIL_AT_SUCCESS)
					{
						APP_DEBUG("OK\r\n");
						APP_DEBUG("<-- Configure clientkey successfully!-->\r\n");
					}
					else
					{
						APP_DEBUG("ERROR\r\n");
						APP_DEBUG("<-- Configure clientkey failed, ret = %d -->\r\n",ret);
					}
					Ql_MEM_Free(userdata.pbuffer);
					userdata.pbuffer = NULL;
				}
				
				ret = RIL_MQTT_QMTCFG_SSL(tcp_connect_id, TRUE, ssl_param_t.contextid, ssl_param_t.connectid);
				if(ret == RIL_AT_SUCCESS)
				{
					APP_DEBUG("OK\r\n");
					APP_DEBUG("<-- Configure SSL secure connection successfully!-->\r\n");
					m_mqtts_state = STATE_MQTTs_OPEN;
				}
				else
				{
					APP_DEBUG("ERROR\r\n");
					APP_DEBUG("<-- Configure SSL secure connection failed, ret = %d -->\r\n",ret);
				}
			}
			break;

			case STATE_MQTTs_OPEN:
			{
				ret = RIL_MQTT_QMTOPEN(tcp_connect_id,hostname,svr_port);
				if (RIL_AT_SUCCESS == ret)
				{
					APP_DEBUG("<-- Start open an MQTT client-->\r\n");
					m_mqtts_state = STATE_MQTTs_TOTAL_NUM;
				}else
				{
					APP_DEBUG("<-- open a MQTT client failure,error=%d.-->\r\n",ret);
				}
				break;
			}
			
			case STATE_MQTTs_CONN:
			{
				ret = RIL_MQTT_QMTCONN(tcp_connect_id,clientID,NULL,NULL);
				if (RIL_AT_SUCCESS == ret)
				{
					APP_DEBUG("<--Start connection to MQTT server-->\r\n");
					m_mqtts_state = STATE_MQTTs_TOTAL_NUM;
				}else
				{
					APP_DEBUG("<--connection to MQTT server failure,error=%d.-->\r\n",ret);
				}
				break;
			}
			
			case STATE_MQTTs_SUB:
			{				
				mqtts_topic_info_t.count = 1;
				mqtts_topic_info_t.topic[0] = (u8*)Ql_MEM_Alloc(sizeof(u8)*256);
				
				Ql_memset(mqtts_topic_info_t.topic[0],0,256);
				Ql_memcpy(mqtts_topic_info_t.topic[0],test_topic,Ql_strlen(test_topic));
				mqtts_topic_info_t.qos[0] = QOS1_AT_LEASET_ONCE;
				sub_message_id++;  // 1-65535.
				
				ret = RIL_MQTT_QMTSUB(tcp_connect_id,sub_message_id,&mqtts_topic_info_t);
				
				Ql_MEM_Free(mqtts_topic_info_t.topic[0]);
				mqtts_topic_info_t.topic[0] = NULL;
				if (RIL_AT_SUCCESS == ret)
				{
					APP_DEBUG("<--Start subscribe topics-->\r\n");
					m_mqtts_state = STATE_MQTTs_TOTAL_NUM;
				}else
				{
					APP_DEBUG("<--subscribe topics failure,error=%d.-->\r\n",ret);
				}
				break;
			}
			
			case STATE_MQTTs_PUB:
			{
				pub_message_id++;  // The range is 0-65535. It will be 0 only when<qos>=0.
				ret = RIL_MQTT_QMTPUB(tcp_connect_id,pub_message_id,QOS1_AT_LEASET_ONCE,0,test_topic,test_data);
				if (RIL_AT_SUCCESS == ret)
				{
					APP_DEBUG("<--Start publish messages to ali server-->\r\n");
					m_mqtts_state = STATE_MQTTs_TOTAL_NUM;
				}else
				{
					APP_DEBUG("<--publish messages to ali server failure,error=%d.-->\r\n",ret);
				}
				break;
			}
			
			case STATE_MQTTs_DISC:
			{
				ret = RIL_MQTT_QMTDISC(tcp_connect_id);
				if (RIL_AT_SUCCESS == ret)
				{
					APP_DEBUG("<--Start disconnect MQTT socket-->\r\n");
					m_mqtts_state = STATE_MQTTs_TOTAL_NUM;
					
				}else
				{
					APP_DEBUG("<--disconnect MQTT socket failure,error=%d.-->\r\n",ret); 
				}
				break;
			}
			
			case STATE_MQTTs_TOTAL_NUM:
			{
			  //APP_DEBUG("<--mqtt process wait->\r\n");
			  m_mqtts_state = STATE_MQTTs_TOTAL_NUM;
			  break;
			}
			
			default:
				break;
		}  
    }
}


#endif // __EXAMPLE_TCPCLIENT__
