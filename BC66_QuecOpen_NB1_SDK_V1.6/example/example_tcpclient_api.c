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
 *   example_tcpclient_api.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   This example demonstrates how to establish a TCP connection, when the module 
 *   is used for the client. Input the specified command through any serial port 
 *   and the result will be output through the debug port.We have adopted a timeout 
 *   mechanism,if in the process of connecting socket or getting the TCP socket ACK 
 *   number overtime 90s, the socket will be close and the network will be deactivated.
 *   In most of TCPIP functions,  return -22(SOC_WOULDBLOCK) doesn't indicate failed.
 *   It means app should wait, till the callback function is called.
 *   The app can get the information of success or failure in callback function.
 *   Get more info about return value. Please read the "OPEN_CPU_DGD" document.
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "C_PREDEF=-D __EXAMPLE_TCPCLIENT_API__" in gcc_makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
 * 
 *   Operation:
 *            
 *            Command:Set_Srv_Param=<srv ip>,<srv port>
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
#ifdef __EXAMPLE_TCPCLIENT_API__  
#include "custom_feature_def.h"
#include "ql_stdlib.h"
#include "ql_common.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_error.h"
#include "ql_gpio.h"
#include "ql_rtc.h"
#include "ql_power.h"
#include "ql_system.h"
#include "ql_wtd.h"
#include "ql_timer.h"
#include "ql_adc.h"
#include "ql_uart.h"
#include "ql_socket.h"
#include "ql_time.h"
#include "ril.h"
#include "ril_util.h"
#include "ril_sim.h"
#include "ril_network.h"
#include "ril_system.h"
/*****************************************************************
******************************************************************/
#include "typedef.h"
#include "flash.h"
#include "infrastructure.h"

/*****************************************************************
* UART Param
******************************************************************/
#define SERIAL_RX_BUFFER_LEN  1400
static u8 m_RxBuf_Uart[SERIAL_RX_BUFFER_LEN];

/*****************************************************************
* timer param
******************************************************************/
#define TCP_TIMER_ID              	TIMER_ID_USER_START
#define TCP_TIMEOUT_90S_TIMER_ID 	(TIMER_ID_USER_START + 1)   //timeout
#define GPIO_TIMER_ID 		 		(TIMER_ID_USER_START + 2)
#define LED_TIMER_ID 		 		(TIMER_ID_USER_START + 3)
#define LOGIC_WTD1_TMR_ID  			(TIMER_ID_USER_START + 4)

#define WTD_TMR_TIMEOUT 		1500

#define TCP_TIMER_PERIOD     	800//500
#define TIMEOUT_90S_PERIOD   	90000
#define GPIO_INPUT_TIMER_PERIOD 100

static s32 timeout_90S_monitor = FALSE;

/*****************************************************************
* Server Param
******************************************************************/
#define SEND_BUFFER_LEN     1400
#define RECV_BUFFER_LEN     1400

static u8 m_send_buf[SEND_BUFFER_LEN];
static u8 m_recv_buf[RECV_BUFFER_LEN];
static u32 m_nSentLen  = 0;      // Bytes of number sent data through current socket

static u8  m_ipaddress[IP_ADDR_LEN];  //only save the number of server ip, remove the comma
static s32 m_socketid = -1;

static s32 m_remain_len = 0;     // record the remaining number of bytes in send buffer.
static char *m_pCurrentPos = NULL;
/*****************************************************************
* rtc param
******************************************************************/
static u32 Rtc_id = 0x101;
static u32 Rtc_Interval = 60*1000  *1; //1min
volatile bool RTC_Flag = FALSE;
/*****************************************************************
* ADC Param
******************************************************************/
static u32 ADC_CustomParam = 1;
/*****************************************************************
* Local time param
******************************************************************/
ST_Time time;
ST_Time* pTime = NULL;
u32 totalSeconds = 0;
/*****************************************************************
* Other Param
******************************************************************/
static u8 m_tcp_state = TCP_STATE_NW_GET_SIMSTATE;
/*****************************************************************
* user param
******************************************************************/
static sProgrammData programmData =
{
	.firstInit 		= FALSE,
    .needReboot 	= FALSE,
    .initFlash		= FALSE,

    .rebootCnt 		= 0,
    .reconnectCnt 	= 0,
    .pingCnt		= 0,
    .buttonCnt 		= 0,
    .in1Cnt 		= 0,
    .in2Cnt 		= 0,

    .buttonState 	= FALSE,
    .HbuttonState	= FALSE,
    .in1State		= FALSE,
    .Hin1State		= FALSE,
    .in2State		= FALSE,
    .Hin2State		= FALSE,
    .tempValue 		= 0.0,
    .totalSeconds 	= 0,

    .autCnt			= 0
};

static sProgrammSettings programmSettings;

static s32 				led_cnt 	= 5;
static Enum_PinName  	led_pin 	= PINNAME_GPIO2;//30
static Enum_PinName  	button_pin 	= PINNAME_GPIO3;//31
static Enum_PinName  	in1_pin 	= PINNAME_GPIO4;//32
static Enum_PinName  	in2_pin 	= PINNAME_GPIO5;//33

/*****************************************************************
* GPRS and socket callback function
******************************************************************/
void callback_socket_connect(s32 socketId, s32 errCode, void* customParam );
void callback_socket_close(s32 socketId, s32 errCode, void* customParam );
void callback_socket_accept(s32 listenSocketId, s32 errCode, void* customParam );
void callback_socket_read(s32 socketId, s32 errCode, void* customParam );
void Callback_GetIpByName(u8 contexId,s32 errCode,  u32 ipAddrCnt, u8* ipAddr);

ST_SOC_Callback      callback_soc_func=
{
    callback_socket_connect,
    callback_socket_close,
    callback_socket_accept,
    callback_socket_read,
};


static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara);
static void TCP_Callback_Timer(u32 timerId, void* param);
static void Callback_OnADCSampling(Enum_ADCPin adcPin, u32 adcValue, void *customParam);
static void gpio_callback_onTimer(u32 timerId, void* param);
static void wdt_callback_onTimer(u32 timerId, void* param);
static void Rtc_handler(u32 rtcId, void* param);
/*****************************************************************
* other subroutines
******************************************************************/
//static s32 ReadSerialPort(Enum_SerialPort port, /*[out]*/u8* pBuffer, /*[in]*/u32 bufLen);
static void proc_handle(Enum_SerialPort port, u8 *pData,s32 len);
static void checkErr_AckNumber(s32 err_code);

static void InitFlash(void);
static void InitWDT(void);
static void InitUART(void);
static void InitTCP(void);
static void InitGPIO(void);
static void InitADC(void);
static bool GetLocalTime(void);
static void InitRTC(void);

void proc_main_task(s32 taskId)
{
    ST_MSG msg;

    // Register & open UART port
    Ql_UART_Register(UART_PORT0, CallBack_UART_Hdlr, NULL);//main uart
    Ql_UART_Register(UART_PORT2, CallBack_UART_Hdlr, NULL);//debug uart
    Ql_UART_Open(UART_PORT0, 115200, FC_NONE);
    Ql_UART_Open(UART_PORT2, 115200, FC_NONE);

    //APP_DEBUG("<--QuecOpen: TCP Client.-->\r\n");
    APP_DEBUG("<-- QuecOpen: Starting Application for BC66. -->\r\n");

    InitTCP();

    while(TRUE)
    {
    	//APP_DEBUG("<-- while(TRUE) -->\r\n");
        Ql_OS_GetMessage(&msg);
        //APP_DEBUG("<-- Ql_OS_GetMessage msg=%d-->\r\n", msg.message);
        switch(msg.message)
        {
        case MSG_ID_RIL_READY:
            Ql_RIL_Initialize();
            APP_DEBUG("<-- RIL is ready -->\r\n");

            RIL_QNbiotEvent_Enable(PSM_EVENT);
            programmData.firstInit = TRUE;
            break;

     	case MSG_ID_URC_INDICATION:
		{
			switch (msg.param1)
            {

				case URC_EGPRS_NW_STATE_IND:
					//APP_DEBUG("<--EGPRS Network Status:<%d>-->\r\n", msg.param2);
					if(msg.param2 == 1)
					{
						//Ql_Sleep(1000);
						//GetLocalTime();();
					}
				break;

    			case URC_PSM_EVENT:
					if(ENTER_PSM == msg.param2){
	          			APP_DEBUG("<--The modem enter PSM mode-->\r\n");
			 		}
					else if(EXIT_PSM == msg.param2){
	          			APP_DEBUG("<--The modem exit from PSM mode-->\r\n");
			 		}
                	break;

         		default:
         			APP_DEBUG("<--MSG_ID_URC_INDICATION,  msg.param1=<%u>, msg.param2=<%u>-->\r\n", msg.param1, msg.param2);
           		break;
			}
      	}
		break;

        default:
            break;
        }
    }
}

void proc_subtask1(s32 TaskId)
{
    //ST_MSG subtask1_msg;
    s32 tmpCNT 		= 0;


    //Ql_Sleep(100);
    APP_DEBUG("<-- subtask: entering -->\r\n");

    do
    {
    	Ql_Sleep(50);//in ms
    }
    while(programmData.firstInit == FALSE);

    APP_DEBUG("<-- subtask: starting -->\r\n");

    InitWDT();//mast be first
    InitFlash();
    InitUART();
    InitGPIO();
    InitADC();
    s32 ret = Ql_SleepDisable();

    while (TRUE)
    {
    	//Ql_OS_GetMessage(&subtask1_msg);//нельзя убирать!!! т.к. все колбеки, проинициализированные в этом процессе не будут вызываться!!! Ql_Sleep нельзя, но здесь почему-то работает...
    	Ql_Sleep(100);


    }
}

static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara)
{
    switch (msg)
    {
    case EVENT_UART_READY_TO_READ:
        {
           s32 totalBytes = ReadSerialPort(port, m_RxBuf_Uart, sizeof(m_RxBuf_Uart));
           if (totalBytes > 0)
           {
               proc_handle(port, m_RxBuf_Uart, totalBytes /*sizeof(m_RxBuf_Uart)*/);
           }
           break;
        }
    case EVENT_UART_READY_TO_WRITE:
        break;
    default:
        break;
    }
}

/*
static s32 ReadSerialPort(Enum_SerialPort port, u8* pBuffer, u32 bufLen)
{
    s32 rdLen = 0;
    s32 rdTotalLen = 0;
    if (NULL == pBuffer || 0 == bufLen)
    {
        return -1;
    }
    Ql_memset(pBuffer, 0x0, bufLen);
    while (1)
    {
        rdLen = Ql_UART_Read(port, pBuffer + rdTotalLen, bufLen - rdTotalLen);
        if (rdLen <= 0)  // All data is read out, or Serial Port Error!
        {
            break;
        }
        rdTotalLen += rdLen;
        Ql_GPIO_SetLevel(led_pin, Ql_GPIO_GetLevel(led_pin) == PINLEVEL_HIGH ? PINLEVEL_LOW : PINLEVEL_HIGH);//blink led
        // Continue to read...
    }
    if (rdLen < 0) // Serial Port Error!
    {
        APP_DEBUG("<--Fail to read from port[%d]-->\r\n", port);
        return -99;
    }
    return rdTotalLen;
}*/

static void proc_handle(Enum_SerialPort port, u8 *pData,s32 len)
{
	APP_DEBUG("Read data from port=%d, len=%d\r\n", port, len);
	pData[len] = 0;
	char tmp_buff[300] = {0};
	if(port == UART_PORT1)
	{
		//send it to server
		m_pCurrentPos = m_send_buf;
		Ql_memcpy(m_pCurrentPos + m_remain_len, pData, len);//!!!
		m_remain_len += len;
		/////////////////////////////////////
		if(len < sizeof(tmp_buff) - 50)
		{
			Ql_sprintf(tmp_buff ,"HEX STRING from port=[");
			char bf[3] = {0,0,0};
			for(int i = 0; i < len; i++){
				ByteToHex(bf, pData[i]); bf[2] = 0;
				Ql_strcat(tmp_buff, bf);
			}
			//Ql_strcat(tmp_buff, "]\r\n");
			APP_DEBUG("%s]\r\n", tmp_buff);
		}
	    /////////////////////////////////////
	}
	else
	{
		char *answer = Parse_Command(pData, tmp_buff, &programmSettings, &programmData);
		if( answer != NULL)
		{
			//u32 alen = Ql_strlen(answer);
			APP_DEBUG("%s", answer);
		}
		else
		{
			char *s = Ql_strstr(pData, "AT+");
			//APP_DEBUG("<--Ql_strstr s=[%d] pData=[%d]-->\r\n", (long)s, (long)pData);
			if(s != NULL && s == pData)
			{
				s32 aret = RIL_NW_SendATCmd(pData, tmp_buff);
				//APP_DEBUG("<--ATret=[%d]-->\r\n", aret);
				if(aret == RIL_AT_SUCCESS){
					APP_DEBUG("OK\r\n");
				}
				else{
					APP_DEBUG("ERROR\r\n");
				}
				APP_DEBUG("%s", tmp_buff);
				return;
			}
			//if not command, send it to server
			m_pCurrentPos = m_send_buf;
			//Ql_strcpy(m_pCurrentPos + m_remain_len, pData);
			Ql_memcpy(m_pCurrentPos + m_remain_len, pData, len);//!!!
			m_remain_len += len;
		}
	}
}

static void checkErr_AckNumber(s32 err_code)
{
    if(SOC_ERROR_CONN == err_code)
    {
        APP_DEBUG("<-- Not connected -->\r\n");
    }
    else if(SOC_ERROR_ARG == err_code)
    {
        APP_DEBUG("<-- Illegal argument for ACK number -->\r\n");
    }
    else if(SOC_ERROR_ABRT == err_code)
    {
        APP_DEBUG("<-- Connection aborted -->\r\n");
    }
    else if(SOC_ERROR_VAL == err_code)
    {
        APP_DEBUG("<-- Illegal value for ACK number -->\r\n");
    }
    else if(SOC_ERROR == err_code)
    {
        APP_DEBUG("<-- Unspecified error for ACK number -->\r\n");
    }
    else
    {
        // get the socket option successfully
    }
}

static void TCP_Callback_Timer(u32 timerId, void* param)
{
	s32 ret;
	//APP_DEBUG("<-- TCP_Callback_Timer, timerId=%d -->\r\n", timerId);
	if(programmData.firstInit == FALSE || programmData.initFlash == FALSE) return;

    if (TCP_TIMEOUT_90S_TIMER_ID == timerId)
    {
        APP_DEBUG("<-- 90s time out!!! -->\r\n");
        APP_DEBUG("<-- Close socket. -->\r\n");
        
        Ql_SOC_Close(m_socketid);
        m_socketid = -1;

        m_tcp_state = TCP_STATE_NW_GET_SIMSTATE;

        timeout_90S_monitor = FALSE;
    }
    else if (TCP_TIMER_ID == timerId)
    {
        //APP_DEBUG("<--...........m_tcp_state=%d..................-->\r\n",m_tcp_state);
        switch (m_tcp_state)
        {
            case TCP_STATE_NW_GET_SIMSTATE:
            {
                s32 simStat = 0;
                RIL_SIM_GetSimState(&simStat);
                if (simStat == SIM_STAT_READY)
                {
                    m_tcp_state = TCP_STATE_NW_QUERY_STATE;
                    APP_DEBUG("<-- SIM card status is normal! -->\r\n");
                }else
                {
                    APP_DEBUG("<-- SIM card status is unnormal! -->\r\n");
                }
                break;
            }        
            case TCP_STATE_NW_QUERY_STATE:
            {
                s32 cgreg = 0;
                ret = RIL_NW_GetEGPRSState(&cgreg);
                APP_DEBUG("<--Network State:cgreg=%d-->\r\n",cgreg);
                if((cgreg == NW_STAT_REGISTERED)||(cgreg == NW_STAT_REGISTERED_ROAMING))
                {
                    m_tcp_state = TCP_STATE_GET_LOCALIP;
                }
                break;
            }
            case TCP_STATE_GET_LOCALIP:
            {
                ST_Addr_Info_t addr_info;

                Ql_memset(addr_info.addr, 0, IP_ADDR_LEN);
				addr_info.addr_len = IP_ADDR_LEN;
				
                ret = Ql_GetLocalIPAddress(0, &addr_info);
                if (ret == SOC_SUCCESS)
                {
                	APP_DEBUG("<--Get Local Ip successfully,Local Ip=%s-->\r\n",addr_info.addr);
                    Ql_sprintf(programmSettings.ipSettings.srcAddress,"%s", addr_info.addr);

                    m_tcp_state = TCP_STATE_GET_DNSADDRESS;
                }else
                {
                    APP_DEBUG("<--Get Local Ip failure,ret=%d.-->\r\n",ret);
                }
                break;
            }
			case TCP_STATE_GET_DNSADDRESS:
            {         
				ST_Dns_Info_t dns_info;
				
				dns_info.ip_type= IP_TYPE_IPV4;
				Ql_memset(dns_info.primaryAddr,0,IP_ADDR_LEN);
				Ql_memset(dns_info.bkAddr,0,IP_ADDR_LEN);
				dns_info.addr_len = IP_ADDR_LEN;
				
                ret =Ql_GetDNSAddress(0,&dns_info);
                if (ret == SOC_SUCCESS)
                {
                    APP_DEBUG("<--Get DNS address successfully, primaryAddr=%s, bkAddr=%s-->\r\n",dns_info.primaryAddr,dns_info.bkAddr);
                    m_tcp_state = TCP_STATE_CHACK_SRVADDR;
                }else
                {
                    APP_DEBUG("<--Get DNS address failure,ret=%d.-->\r\n",ret);
                    m_tcp_state = TCP_STATE_CHACK_SRVADDR;
                }
                break;
            }
			case TCP_STATE_CHACK_SRVADDR:
            {
                ret = Ql_SocketCheckIp(programmSettings.ipSettings.dstAddress);
                if(ret == TRUE) // ip address, xxx.xxx.xxx.xxx
                {
					Ql_memset(m_ipaddress, 0, IP_ADDR_LEN);
                    Ql_memcpy(m_ipaddress, programmSettings.ipSettings.dstAddress, IP_ADDR_LEN);
                    APP_DEBUG("<--Convert Ip Address successfully,m_ipaddress=%s-->\r\n",m_ipaddress);
                    m_tcp_state = TCP_STATE_SOC_REGISTER;
                    
                }
                else  //domain name
                {
                    ret = Ql_IpHelper_GetIPByHostName(0, programmSettings.ipSettings.dstAddress, Callback_GetIpByName);
                    if(ret == SOC_SUCCESS)
                    {
                        APP_DEBUG("<--Get ip by hostname successfully.-->\r\n");
                    }
                    else if(ret == SOC_NONBLOCK)
                    {
						m_tcp_state = TCP_STATE_TOTAL_NUM;
                        APP_DEBUG("<--Waiting for the result of Getting ip by hostname,ret=%d.-->\r\n",ret);
                        //waiting CallBack_getipbyname
                    }
                    else
                    {
						m_tcp_state = TCP_STATE_TOTAL_NUM;
                        APP_DEBUG("<--Get ip by hostname failure:ret=%d-->\r\n",ret); 
                    }
                }
                break;
            }
            case TCP_STATE_SOC_REGISTER:
            {
                ret = Ql_SOC_Register(callback_soc_func, NULL);
                if (SOC_SUCCESS == ret)
                {
                    APP_DEBUG("<--Register socket callback function successfully.-->\r\n");
                    m_tcp_state = TCP_STATE_SOC_CREATE;
                }else if (SOC_ERROR_ALREADY == ret)
                {
                    APP_DEBUG("<--Socket callback function has already been registered,ret=%d.-->\r\n",ret);
                    m_tcp_state = TCP_STATE_SOC_CREATE;
                }else
                {
                    APP_DEBUG("<--Register Socket callback function failure,ret=%d.-->\r\n",ret);
                }
                break;
            }
            case TCP_STATE_SOC_CREATE:
            {
                m_socketid = Ql_SOC_Create(0, SOC_TYPE_TCP);
                if (m_socketid >= 0)
                {
                    APP_DEBUG("<--Create socket id successfully,socketid=%d.-->\r\n",m_socketid);
                    m_tcp_state = TCP_STATE_SOC_CONNECT;

                    u32 ackedNumCurr = 0;
                    ret = Ql_SOC_GetAckNumber(m_socketid, &ackedNumCurr);
                    if (ret < 0){
                        checkErr_AckNumber(ret);
                    }
                    else{
                    	m_nSentLen = ackedNumCurr;
                    }
                    APP_DEBUG("<--Create socket: first init ACK NUMBER ackedNumCurr=%d.-->\r\n", ackedNumCurr);
                }
                else
                {
                    APP_DEBUG("<--Create socket id failure,error=%d.-->\r\n",m_socketid);
                }
                break;
            }
            case TCP_STATE_SOC_CONNECT:
            {
                m_tcp_state = TCP_STATE_SOC_CONNECTING;
                ret = Ql_SOC_Connect(m_socketid,m_ipaddress, programmSettings.ipSettings.dstPort);
                if(ret == SOC_SUCCESS)
                {
                    APP_DEBUG("<--The socket is already connected.-->\r\n");
                    m_tcp_state = TCP_STATE_SOC_SEND;
                    
                }
                else if(ret == SOC_NONBLOCK)
                {
                      if (!timeout_90S_monitor)//start timeout monitor
                      {
                        Ql_Timer_Start(TCP_TIMEOUT_90S_TIMER_ID, TIMEOUT_90S_PERIOD, FALSE);
                        timeout_90S_monitor = TRUE;
                      }
                      APP_DEBUG("<--Waiting for the result of socket connection,ret=%d.-->\r\n",ret);
                      //waiting CallBack_getipbyname
                }
                else //error
                {
                    APP_DEBUG("<--Socket Connect failure,ret=%d.-->\r\n",ret);
                    APP_DEBUG("<-- Close socket.-->\r\n");
                    Ql_SOC_Close(m_socketid);
                    m_socketid = -1;
                    
                   m_tcp_state = TCP_STATE_SOC_CLOSE;
                }
                break;
            }
            case TCP_STATE_SOC_SEND:
            {
            	if(m_remain_len <= 0)
                {
					//APP_DEBUG("<-- No data need to send, waiting to send data -->\r\n");
                    break;
				}

                m_tcp_state = TCP_STATE_SOC_SENDING;
                do
                {
                    ret = Ql_SOC_Send(m_socketid, m_pCurrentPos, m_remain_len);
                    APP_DEBUG("<--Send data, socketid=%d,number of bytes sent=%d-->\r\n",m_socketid,ret);
                    if(ret == m_remain_len)//send compelete
                    {
                        m_remain_len = 0;
                        m_pCurrentPos = NULL;
                        m_nSentLen += ret;
                        m_tcp_state = TCP_STATE_SOC_ACK;
                        break;
                    }
                    else if((ret <= 0) && (ret == SOC_NONBLOCK)) 
                    {
						//APP_DEBUG("<--Wait for data send to finish-->\r\n");
                        break;
                    }
                    else if(ret <= 0)
                    {
                        APP_DEBUG("<--Send data failure,ret=%d.-->\r\n",ret); 
                        m_tcp_state = TCP_STATE_SOC_CLOSE;
                        break;
                    }
                    else if(ret < m_remain_len)//continue send, do not send all data
                    {
                        m_remain_len -= ret;
                        m_pCurrentPos += ret; 
                        m_nSentLen += ret;
                    }
                }
                while(1);
                break;
            }
            case TCP_STATE_SOC_ACK:
            {
                u32 ackedNumCurr = 0;
                ret = Ql_SOC_GetAckNumber(m_socketid, &ackedNumCurr);
                if (ret < 0)
                {
                	checkErr_AckNumber(ret);
                }
                if (m_nSentLen == ackedNumCurr)
                {
                    if (timeout_90S_monitor) //stop timeout monitor
                    {
                        Ql_Timer_Stop(TCP_TIMEOUT_90S_TIMER_ID);
                        timeout_90S_monitor = FALSE;
                    }
                    APP_DEBUG("<-- ACK Number:%d/%d. Server has received all data. -->\r\n\r\n", m_nSentLen, ackedNumCurr);
                    //Ql_memset(m_send_buf,0,SEND_BUFFER_LEN);//!!!
                    m_tcp_state = TCP_STATE_SOC_SEND;
                }
                else
                {
                    if (!timeout_90S_monitor)//start timeout monitor
                    {
                        Ql_Timer_Start(TCP_TIMEOUT_90S_TIMER_ID, TIMEOUT_90S_PERIOD, FALSE);
                        timeout_90S_monitor = TRUE;
                    }
                    APP_DEBUG("<-- ACK Number:%d/%d from socket[%d] -->\r\n", ackedNumCurr, m_nSentLen, m_socketid);
                }
                break;
            }
            case TCP_STATE_SOC_CLOSE:
            {
            	if(m_socketid >= 0)
            	{
            		ret = Ql_SOC_Close(m_socketid);//error , Ql_SOC_Close
            		APP_DEBUG("<--socket closed, ret(%d)-->\r\n", ret);
            		if(ret < 0){
            			programmData.needReboot = TRUE;
            		}
            	}
		        m_socketid = -1;
                m_remain_len = 0;
                m_pCurrentPos = NULL;

                m_tcp_state = TCP_STATE_NW_GET_SIMSTATE;//!!!!
                break;
            }
            default:
                break;
        }    
    }
}

void Callback_GetIpByName(u8 contexId, s32 errCode, u32 ipAddrCnt, u8* ipAddr)
{
    
    if (errCode == SOC_SUCCESS)
    {
        Ql_memset(m_ipaddress, 0, IP_ADDR_LEN);
        Ql_memcpy(m_ipaddress, ipAddr, IP_ADDR_LEN);
		APP_DEBUG("<-- %s:contexid=%d,error=%d, num_entry=%d, m_ipaddress(%s) -->\r\n", __func__, contexId,errCode,ipAddrCnt,m_ipaddress);
        m_tcp_state = TCP_STATE_SOC_REGISTER;
    }
}

void callback_socket_connect(s32 socketId, s32 errCode, void* customParam )
{
    if (errCode == SOC_SUCCESS)
    {
        if (timeout_90S_monitor) //stop timeout monitor
        {
           Ql_Timer_Stop(TCP_TIMEOUT_90S_TIMER_ID);
           timeout_90S_monitor = FALSE;
        }
        APP_DEBUG("<--Callback: socket connect successfully.-->\r\n");
        m_tcp_state = TCP_STATE_SOC_SEND;
    }
    else
    {
        APP_DEBUG("<--Callback: socket connect failure,(socketId=%d),errCode=%d-->\r\n",socketId,errCode);
        Ql_SOC_Close(socketId);
        m_tcp_state = TCP_STATE_SOC_CREATE;
    }
}

void callback_socket_close(s32 socketId, s32 errCode, void* customParam )
{
    m_nSentLen  = 0;
    
    if (errCode == SOC_SUCCESS)
    {
        m_tcp_state = TCP_STATE_SOC_CREATE;
        APP_DEBUG("<--CallBack: close socket successfully.-->\r\n"); 
    }else
    {
        APP_DEBUG("<--CallBack: close socket failure,(socketId=%d,error_cause=%d)-->\r\n",socketId,errCode); 
    }
}

void callback_socket_accept(s32 listenSocketId, s32 errCode, void* customParam )
{
}

void callback_socket_read(s32 socketId, s32 errCode, void* customParam )
{
    s32 ret;
    if(errCode)
    {
        APP_DEBUG("<--CallBack: socket read failure,(sock=%d,error=%d)-->\r\n",socketId,errCode);
        APP_DEBUG("<-- Close socket.-->\r\n");
        Ql_SOC_Close(socketId);
        m_socketid = -1;
        m_tcp_state = TCP_STATE_TOTAL_NUM; 
        return;
    }

    Ql_memset(m_recv_buf, 0, RECV_BUFFER_LEN);
    do
    {
        ret = Ql_SOC_Recv(socketId, m_recv_buf, RECV_BUFFER_LEN);
        if(ret < 0)
        {
            APP_DEBUG("<-- Receive data failure,ret=%d.-->\r\n",ret);
            APP_DEBUG("<-- Close socket.-->\r\n");
            Ql_SOC_Close(socketId); //you can close this socket  
            m_socketid = -1;
            m_tcp_state = TCP_STATE_SOC_CREATE;
            break;
        }
        else if(ret < RECV_BUFFER_LEN)
        {
            APP_DEBUG("<-- Receive data from sock(%d), len(%d) and write to UART_PORT1 -->\r\n", socketId, ret);
            Ql_UART_Write(UART_PORT1, m_recv_buf, ret);

            if(ret > 1)
            	programmData.reconnectCnt = 0;
            else if(ret == 1)
            	programmData.reconnectCnt = programmSettings.secondsToReconnect/10;
            break;
        }
        else if(ret == RECV_BUFFER_LEN)
        {
            APP_DEBUG("<-- Receive full buffer data from sock(%d), len(%d) and write to UART_PORT1 -->\r\n", socketId, ret);
            Ql_UART_Write(UART_PORT1, m_recv_buf, ret);
            if(ret > 1)
            	programmData.reconnectCnt = 0;
            else if(ret == 1)
            	programmData.reconnectCnt = programmSettings.secondsToReconnect/10;
        }
    }
    while(1);
}


static void Callback_OnADCSampling(Enum_ADCPin adcPin, u32 adcValue, void *customParam)
{
	u32 index = *((u32*)customParam);

	if(index % 60 == 0)
	{
		u32 capacity, voltage;
		s32 ret = RIL_GetPowerSupply(&capacity, &voltage);
		if(ret == QL_RET_OK)
		{
			programmData.tempValue = GetTempValue(adcValue);
			APP_DEBUG( "<-- PowerSupply: power voltage(mV)=%d, sampling voltage(mV)=%d, temp value=%f -->\r\n", voltage, adcValue, programmData.tempValue);
		}

		APP_DEBUG("<-- Callback_OnADCSampling: sampling voltage(mV)=%d times=%d ret=%d -->\r\n", adcValue, index, ret);
	}

    *((s32*)customParam) += 1;
}

static void gpio_callback_onTimer(u32 timerId, void* param)
{
	s32 ret;

	if(programmData.initFlash == FALSE) return;
	if (GPIO_TIMER_ID == timerId)
	{
		//APP_DEBUG("<-- Get the button_pin GPIO level value: %d -->\r\n", Ql_GPIO_GetLevel(button_pin));
		s32 btp = GetInputValue(&button_pin, 	&programmData.buttonCnt,(u32)(programmSettings.buttonTimeout * 	1000/GPIO_INPUT_TIMER_PERIOD), FALSE );
		s32 i1p = GetInputValue(&in1_pin, 		&programmData.in1Cnt, 	(u32)(programmSettings.in1Timeout * 	1000/GPIO_INPUT_TIMER_PERIOD), FALSE );
		s32 i2p = GetInputValue(&in2_pin, 		&programmData.in2Cnt, 	(u32)(programmSettings.in2Timeout * 	1000/GPIO_INPUT_TIMER_PERIOD), FALSE );

		if(btp >= 0) programmData.buttonState = (bool)btp;
		if(programmData.HbuttonState != programmData.buttonState){//is changed
			programmData.HbuttonState = programmData.buttonState;
			APP_DEBUG("<-- Get the button_pin GPIO level value changed: %d -->\r\n", programmData.buttonState);
			if(programmData.buttonState == FALSE){
				APP_DEBUG("<-- try restore default by user press button!!!-->\r\n");
				bool ret =  restore_default_flash(&programmSettings);
				APP_DEBUG("<-- restore_default_flash ret=%d -->\r\n", ret);
				reboot(&programmData);
			}
		}
		if(i1p >= 0) programmData.in1State = (bool)i1p;
		if(programmData.Hin1State != programmData.in1State){//is changed
			programmData.Hin1State = programmData.in1State;
			APP_DEBUG("<-- Get the in1_pin GPIO level value changed: %d -->\r\n", programmData.in1State);
		}
		if(i2p >= 0) programmData.in2State = (bool)i2p;
		if(programmData.Hin2State != programmData.in2State){//is changed
			programmData.Hin2State = programmData.in2State;
			APP_DEBUG("<-- Get the in2_pin GPIO level value changed: %d -->\r\n", programmData.in2State);
		}
	}
	else if (LED_TIMER_ID == timerId)
	{
		//programmData.totalMS += 50;
		//led_cnt
	    if(led_cnt-- < 0)
	    {
	      led_cnt = 20;
	      if(programmData.reconnectCnt++ > programmSettings.secondsToReconnect)
	      {
	        programmData.reconnectCnt = 0;
	        if(m_socketid >= 0)//
	        {
	        	APP_DEBUG("<-- Socket reconnect timeout, need_tcp_connect socketId=%d, secondsToReconnect=<%d> -->\r\n", m_socketid, programmSettings.secondsToReconnect);
				ret = Ql_SOC_Close(m_socketid);
				m_socketid = -1;
				APP_DEBUG("<-- Closing socket. ret=<%d> -->\r\n", ret);
				m_tcp_state =  TCP_STATE_SOC_CREATE;//STATE_GPRS_DEACTIVATE
	        }
	      }
	      if(programmData.rebootCnt++ > programmSettings.secondsToReboot)
	      {
	        programmData.rebootCnt  = 0;
	        programmData.needReboot = TRUE;
	        APP_DEBUG("<-- Need reboot timeout secondsToReboot=<%d> -->\r\n", programmSettings.secondsToReboot);
	      }
	      if(programmData.pingCnt++ > programmSettings.secondsToPing)
	      {
	        programmData.pingCnt  = 0;
	        if(m_remain_len == 0 && m_pCurrentPos == NULL)
	        {
	        	m_pCurrentPos = m_send_buf;
	        	m_pCurrentPos[0] = 0;//send 1 byte
	        	m_remain_len++;

	        	//APP_DEBUG("<--Socket ping timeout, need_tcp_ping 1 byte socketId=%d, m_remain_len=%d -->\r\n", m_socketid, m_remain_len);
	        }
	        APP_DEBUG("<-- Need ping timeout secondsToPing=<%d> -->\r\n", programmSettings.secondsToPing);
	      }
	      //Ql_GPIO_SetLevel(led_pin, Ql_GPIO_GetLevel(led_pin) == PINLEVEL_HIGH ? PINLEVEL_LOW : PINLEVEL_HIGH);
	      Ql_GPIO_SetLevel(led_pin, PINLEVEL_HIGH);

	      //authorization
	      if(programmData.autCnt > 0){
	    	  if(programmData.autCnt == 1){
	    		  APP_DEBUG("<-- authorization timer stop -->\r\n");
	    	  }
	    	  else if(programmData.autCnt == AUT_TIMEOUT){
	    		  APP_DEBUG("<-- authorization timer start -->\r\n");
	    	  }
	    	  programmData.autCnt--;
	      }

	    }
	    else{
	    	Ql_GPIO_SetLevel(led_pin, PINLEVEL_LOW);
	    }
	}
}

static void wdt_callback_onTimer(u32 timerId, void* param)
{
    s32* wtdid = (s32*)param;
    //APP_DEBUG("<-- time to feed logic watchdog wtdid=%d-->\r\n", *wtdid);
    if(programmData.needReboot == FALSE)
    {
    	Ql_WTD_Feed(*wtdid);
    }
    else
    {
    	u32 cnt = WTD_TMR_TIMEOUT*2/100 + 1;
    	APP_DEBUG("<-- time to not feed logic watchdog (wtdId=%d) needReboot=(%s) cnt=(%d)-->\r\n", *wtdid, programmData.needReboot == TRUE ? "TRUE" : "FALSE", cnt);
    	do
    	{
    		Ql_GPIO_SetLevel(led_pin, Ql_GPIO_GetLevel(led_pin) == PINLEVEL_HIGH ? PINLEVEL_LOW : PINLEVEL_HIGH);
    		Ql_Sleep(50);
    	}
    	while(cnt--);
    	APP_DEBUG("<-- wdt_callback_onTimer wait real wdt reboot successfull, try Ql_Reset-->\r\n");
    	Ql_Sleep(100);
    	Ql_Reset(0);
    }
}

static void Rtc_handler(u32 rtcId, void* param)
{
    if(Rtc_id == rtcId) {
    	*((bool*)param) = TRUE;
    }
}
/*****************************************************************
* other functions
******************************************************************/
static void InitFlash(void)
{
	APP_DEBUG("<-- OpenCPU: init_flash! Size=%d-->\r\n", sizeof(sProgrammSettings));

    bool ret = FALSE;
    for(int i=0; i < 10; i++)
    {
        ret = init_flash(&programmSettings);
        if(ret == TRUE)
        {
        	APP_DEBUG("<-- init_flash OK crc=<%d>\r\napn=<%s> user=<%s> pass=<%s>\r\nserver=<%s> port=<%d> baudrate=<%d>\r\n secondsToReboot=<%d> secondsToReconnect=<%d> secondsToPing=<%d>-->\r\n",
        			programmSettings.crc,
        			programmSettings.gsmSettings.gprsApn,
        			programmSettings.gsmSettings.gprsUser,
        			programmSettings.gsmSettings.gprsPass,
        			programmSettings.ipSettings.dstAddress,
        			programmSettings.ipSettings.dstPort,
        			programmSettings.serPortSettings.baudrate,
        			programmSettings.secondsToReboot,
        			programmSettings.secondsToReconnect,
        			programmSettings.secondsToPing
        			);

        	programmData.initFlash = TRUE;
        	return;
        }
        else
        {
        	//APP_DEBUG("<-- init_flash Err, try next cnt=%i\r\n", i);
        	Ql_Sleep(1000);
        }
        APP_DEBUG("<-- init_flash ret=%d-->\r\n", ret);
    }

    APP_DEBUG("<-- init_flash ERROR, try restore default!!! -->\r\n");
    ret = restore_default_flash(&programmSettings);

    APP_DEBUG("<-- restore_default_flash ret=%d -->\r\n", ret);
    reboot(&programmData);
}


static void InitWDT(void)
{
    s32 wtdid;
    s32 ret;

    APP_DEBUG("<-- InitWDT -->\r\n");
    // Initialize external watchdog:
    ret = Ql_WTD_Init(0, PINNAME_RI, 300);//дергаем ногой почаще тк время сброса у tps-ки 1600мс
    if (0 == ret)
        APP_DEBUG("\r\n<--OpenCPU: watchdog init OK!-->\r\n");

    // Create a logic watchdog, the interval is 1.5 s
    wtdid = Ql_WTD_Start(WTD_TMR_TIMEOUT);

    APP_DEBUG("<-- InitWDT wtdid=%d-->\r\n", wtdid);
    // Register & start a timer to feed the logic watchdog.
    ret = Ql_Timer_Register(LOGIC_WTD1_TMR_ID, wdt_callback_onTimer, &wtdid);
    if(ret < 0){
        APP_DEBUG("<--main task: register fail ret=%d-->\r\n", ret);
        return;
    }
    // The real feeding interval is 300 ms
    ret = Ql_Timer_Start(LOGIC_WTD1_TMR_ID, 300, TRUE);
    if(ret < 0){
        APP_DEBUG("<--main task: start timer fail ret=%d-->\r\n",ret);
        return;
    }
    APP_DEBUG("<-- InitWDT end -->\r\n");
    Ql_Sleep(300);
}


static void InitUART(void)
{
    // Register & open UART port
	APP_DEBUG("<-- InitUART -->\r\n");

	s32 ret;
    ST_UARTDCB dcb;

    dcb.baudrate = programmSettings.serPortSettings.baudrate;
    dcb.dataBits = programmSettings.serPortSettings.dataBits;
    dcb.stopBits = programmSettings.serPortSettings.stopBits;
    dcb.parity   = programmSettings.serPortSettings.parity;
    dcb.flowCtrl = programmSettings.serPortSettings.flowCtrl;

    ret = Ql_UART_Register(UART_PORT1, CallBack_UART_Hdlr, NULL);
    if (ret < QL_RET_OK)
        Ql_Debug_Trace("<--Ql_UART_Register(mySerialPort=%d)=%d-->\r\n", UART_PORT1, ret);

    ret = Ql_UART_OpenEx(UART_PORT1, &dcb);
    if (ret < QL_RET_OK)
        Ql_Debug_Trace("<--Ql_UART_OpenEx(mySerialPort=%d)=%d-->\r\n", UART_PORT1, ret);

    APP_DEBUG("<-- InitUART end -->\r\n");
}


static void InitTCP(void)
{
    //register & start timer
	APP_DEBUG("<-- InitTCP -->\r\n");

    //register & start timer
    Ql_Timer_Register(TCP_TIMER_ID, TCP_Callback_Timer, NULL);
    Ql_Timer_Start(TCP_TIMER_ID, TCP_TIMER_PERIOD, TRUE);

    Ql_Timer_Register(TCP_TIMEOUT_90S_TIMER_ID, TCP_Callback_Timer, NULL);
    timeout_90S_monitor = FALSE;

    APP_DEBUG("<-- End InitTCP -->\r\n");
}


static void InitGPIO(void)
{
	APP_DEBUG("<-- InitGPIO -->\r\n");

    Ql_GPIO_Init(led_pin, 		PINDIRECTION_OUT, 	PINLEVEL_HIGH, 	PINPULLSEL_PULLUP);
    Ql_GPIO_Init(button_pin, 	PINDIRECTION_IN, 	PINLEVEL_HIGH, 	PINPULLSEL_DISABLE);
    Ql_GPIO_Init(in1_pin, 		PINDIRECTION_IN, 	PINLEVEL_HIGH, 	PINPULLSEL_DISABLE);
    Ql_GPIO_Init(in2_pin, 		PINDIRECTION_IN, 	PINLEVEL_HIGH, 	PINPULLSEL_DISABLE);

    Ql_Timer_Register(GPIO_TIMER_ID, gpio_callback_onTimer, NULL);
    Ql_Timer_Start(GPIO_TIMER_ID, GPIO_INPUT_TIMER_PERIOD, TRUE);

    Ql_Timer_Register(LED_TIMER_ID, gpio_callback_onTimer, NULL);
    Ql_Timer_Start(LED_TIMER_ID, 50, TRUE);

    APP_DEBUG("<-- End InitGPIO -->\r\n");
}


static void InitADC(void)
{
    Enum_PinName adcPin = PIN_ADC0;
    // Register callback foR ADC
    APP_DEBUG("<-- Register callback for ADC -->\r\n")
    Ql_ADC_Register(adcPin, Callback_OnADCSampling, (void *)&ADC_CustomParam);

    // Initialize ADC (sampling count, sampling interval)
    APP_DEBUG("<-- Initialize ADC (sampling count=5, sampling interval=200ms) -->\r\n")
    Ql_ADC_Init(adcPin, 5, 200);

    // Start ADC sampling
    APP_DEBUG("<-- Start ADC sampling -->\r\n")
    Ql_ADC_Sampling(adcPin, TRUE);

    // Stop  sampling ADC
    //Ql_ADC_Sampling(adcPin, FALSE);
}


static bool GetLocalTime(void)
{
	bool ret = FALSE;
    if((Ql_GetLocalTime(&time)))
    {
        //This function get total seconds elapsed   since 1970.01.01 00:00:00.
        totalSeconds = Ql_Mktime(&time);
        APP_DEBUG("<--Local time successfuly determined: %i.%i.%i %i:%i:%i timezone=%i-->\r\n", time.day, time.month, time.year, time.hour, time.minute, time.second,time.timezone);

        if(totalSeconds > 0)
        	programmData.totalSeconds = totalSeconds;
        //pTime = Ql_MKTime2CalendarTime(totalSeconds, &time);

        APP_DEBUG("<--totalSeconds=%lu-->\r\n", programmData.totalSeconds);
        ret = TRUE;
    }
    else
    {
        APP_DEBUG("\r\n<--failed !! Local time not determined -->\r\n");
    }
    return ret;
}

static void InitRTC(void)
{// инициализация и переинициализация должны происходить в ОДНОМ и том же потоке!!!
	s32 ret;
    //rtc register
    ret = Ql_Rtc_RegisterFast(Rtc_id, Rtc_handler, (void*)&RTC_Flag);
    if(ret < 0){
		if(-4 != ret){
			APP_DEBUG("\r\n<--InitRTC rtc register failed!:RTC id(%u),ret(%d)-->\r\n", Rtc_id, ret);
		}
		else{
			APP_DEBUG("\r\n<--the rtc is already register-->\r\n");
		}
    }

/*    if(programmData.initFlash == TRUE)
    {
    	s32 timeout = programmSettings.rtcInterval;
		Rtc_Interval = (u32)timeout*1000;
		APP_DEBUG("\r\n<--InitRTC timeout=%lu sec-->\r\n", timeout);
    }
    */
	//start a rtc ,repeat=true;
	ret = Ql_Rtc_Start(Rtc_id, Rtc_Interval, TRUE);
	if(ret < 0){
		if(-4 != ret){
			APP_DEBUG("\r\n<--InitRTC rtc start failed!!:ret=%d-->\r\n",ret);
		}
		else{
			APP_DEBUG("\r\n<--the rtc is already start-->\r\n");
		}
	}
}


#endif // __EXAMPLE_TCPCLIENT_API__

