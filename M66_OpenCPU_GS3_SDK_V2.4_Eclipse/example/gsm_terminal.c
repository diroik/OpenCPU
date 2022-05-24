/*
 * gsm_terminal.c
 *
 *  Created on: 23 но€б. 2021 г.
 *      Author: јдмин
 */

#ifdef __GSM_TEMINAL__
#include "custom_feature_def.h"
#include "ql_stdlib.h"
#include "ql_common.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_error.h"
#include "ql_gpio.h"
#include "ql_power.h"
#include "ql_system.h"
#include "ql_wtd.h"
#include "ql_timer.h"
#include "ql_adc.h"
#include "ql_uart.h"
#include "ql_gprs.h"
#include "ql_socket.h"
#include "ql_time.h"
#include "ril.h"
#include "ril_util.h"
#include "ril_sms.h"
#include "ril_sim.h"
#include "ril_telephony.h"
#include "ril_network.h"
#include "ril_network_time.h"
#include "fota_main.h"
/*****************************************************************
******************************************************************/
#include "typedef.h"
#include "flash.h"
#include "infrastructure.h"

#define SAME_BUFFER_LEN 1400

/*****************************************************************
* UART Param
******************************************************************/
#define SERIAL_RX_BUFFER_LEN  SAME_BUFFER_LEN
static u8 m_RxBuf_Uart[SERIAL_RX_BUFFER_LEN];

/*****************************************************************
* timer param
******************************************************************/
#define LOGIC_WTD1_TMR_ID  		(TIMER_ID_USER_START + 1)
#define TCP_TIMER_ID         	(TIMER_ID_USER_START + 2)
#define TIMEOUT_90S_TIMER_ID 	(TIMER_ID_USER_START + 3)   //timeout
#define CSQ_TIMER_ID 		 	(TIMER_ID_USER_START + 4)   //signal quality
#define GPIO_TIMER_ID 		 	(TIMER_ID_USER_START + 5)
#define LED_TIMER_ID 		 	(TIMER_ID_USER_START + 6)
#define SYSTEM_TIME_TIMER_ID 	(TIMER_ID_USER_START + 7)

#define MSG_ID_NETWORK_REGISTRATION   (MSG_ID_USER_START+0x100)


#define WTD_TMR_TIMEOUT 1000

#define TCP_TIMER_PERIOD     800 //800/500
#define TIMEOUT_90S_PERIOD   90000
#define CSQ_TIMER_PERIOD     180000
#define GPIO_INPUT_TIMER_PERIOD 10

static s32 timeout_90S_monitor = FALSE;

/*****************************************************************
* APN Param
******************************************************************/
static ST_GprsConfig  m_gprsCfg;

/*****************************************************************
* Server Param
******************************************************************/
#define SEND_BUFFER_LEN     SAME_BUFFER_LEN
#define RECV_BUFFER_LEN     SAME_BUFFER_LEN

static u8 m_send_buf[SEND_BUFFER_LEN];
static u8 m_recv_buf[RECV_BUFFER_LEN];
static u64 m_nSentLen  = 0;      // Bytes of number sent data through current socket

static u8  m_ipaddress[5];  //only save the number of server ip, remove the comma
static s32 m_socketid = -1;

static s32 m_remain_len = 0;     // record the remaining number of bytes in send buffer.
static char *m_pCurrentPos = NULL;

#define TEMP_BUFFER_LEN 512


/*****************************************************************
* ADC Param
******************************************************************/
static u32 ADC_CustomParam = 0;

/*****************************************************************
* Local time param
******************************************************************/
ST_Time time;
ST_Time* pTime = NULL;
u32 upTime = 0;

/*****************************************************************
* Other Param
******************************************************************/
static u8 m_tcp_state = STATE_NW_GET_SIMSTATE;

static sProgrammData programmData =
{
	.mainTaskId	    = -1,
	.subTaskId1		= -1,

	.firstInit 		= FALSE,
	.timeInit		= FALSE,
	.socketTimersInit = FALSE,
    .needReboot 	= FALSE,

    .rebootCnt 		= 0,
    .reconnectCnt 	= 0,
    .tryconnectCnt 	= 0,
    .durationCnt    = 0,//!!!
    .pingCnt		= 0,
    .buttonCnt 		= 0,
    .in1Cnt 		= 0,
    .in2Cnt 		= 0,



    .HbuttonState	= FALSE,
    .Hin1State		= FALSE,
    .Hin2State		= FALSE,

    .autCnt			= 0,
    .ledCnt			= 0,

    .dataState.totalSeconds 	= 0,
    .dataState.button	= FALSE,
    .dataState.in1		= FALSE,
    .dataState.in2		= FALSE,
    .dataState.temp 	= 0.0,
    .dataState.rssi			= 0,
    .dataState.ber 			= 0,
    .dataState.imei			= "000000000000000",
    .dataState.iccid	    = "00000000000000000000",
    .dataState.version		= FW_VERSION
};
static sProgrammSettings programmSettings;

static s32 				led_cnt = 5;
static Enum_PinName  	wdt_pin 	= PINNAME_RI;
static Enum_PinName  	led_pin 	= PINNAME_PCM_CLK;//30
static Enum_PinName  	button_pin 	= PINNAME_PCM_SYNC;//31
static Enum_PinName  	in1_pin 	= PINNAME_PCM_OUT;//33
static Enum_PinName  	in2_pin 	= PINNAME_PCM_IN;//32

/*****************************************************************
* uart callback function
******************************************************************/
static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara);

/*****************************************************************
* timer callback function declaration
******************************************************************/
static void wdt_callback_onTimer(u32 timerId, void* param);
static void gsm_callback_onTimer(u32 timerId, void* param);
static void gpio_callback_onTimer(u32 timerId, void* param);
static void time_callback_onTimer(u32 timerId, void* param);

/*****************************************************************
* ADC callback function
******************************************************************/
static void Callback_OnADCSampling(Enum_ADCPin adcPin, u32 adcValue, void *customParam);

/*****************************************************************
* GPRS and socket callback function
******************************************************************/
static void callback_socket_connect(s32 socketId, s32 errCode, void* customParam );
static void callback_socket_close(s32 socketId, s32 errCode, void* customParam );
static void callback_socket_accept(s32 listenSocketId, s32 errCode, void* customParam );
static void callback_socket_read(s32 socketId, s32 errCode, void* customParam );
static void callback_socket_write(s32 socketId, s32 errCode, void* customParam );

static void Callback_GPRS_Actived(u8 contexId, s32 errCode, void* customParam);
static void CallBack_GPRS_Deactived(u8 contextId, s32 errCode, void* customParam );
static void Callback_GetIpByName(u8 contexId, u8 requestId, s32 errCode,  u32 ipAddrCnt, u32* ipAddr);

static void checkErr_AckNumber(s32 err_code);
static void Restart_GSM(void);
/*****************************************************************
* socket Param must be after socket callbaks functions !!!
******************************************************************/
static ST_PDPContxt_Callback     callback_gprs_func =
{
    Callback_GPRS_Actived,
    CallBack_GPRS_Deactived
};
static ST_SOC_Callback      callback_soc_func=
{
    callback_socket_connect,
    callback_socket_close,
    callback_socket_accept,
    callback_socket_read,
    callback_socket_write
};


/*****************************************************************
* init function declaration
******************************************************************/
static void InitWDT(s32 *wtdid);
static void InitFlash(void);
static void InitUART(void);
static void InitGPRS(void);
static void InitGPIO(void);
static void InitADC(void);
static void InitTIME(void);
static void InitSN(void);

/*****************************************************************
* Other function declaration
******************************************************************/
static void proc_handle(Enum_SerialPort port, char *pData, s32 len);
static bool GetLocalTime(void);

/**************************************************************
* main task
***************************************************************/
void proc_main_task(s32 TaskId)
{
    ST_MSG msg;
    s32 wtdid;
    s32 ret;

    programmData.mainTaskId = TaskId;

    // Register & open UART ports
    Ql_UART_Register(UART_PORT1, CallBack_UART_Hdlr, NULL);//main uart
    Ql_UART_Register(UART_PORT2, CallBack_UART_Hdlr, NULL);//debug uart
    Ql_UART_Open(UART_PORT1, 115200, FC_NONE);
    Ql_UART_Open(UART_PORT2, 115200, FC_NONE);

    APP_DEBUG("<-- OpenCPU: Starting Application. MainTaskId=%d -->\r\n",  programmData.mainTaskId);

    InitWDT(&wtdid);
    InitGPRS();

    while (TRUE)
    {
        Ql_OS_GetMessage(&msg);
        switch(msg.message)
        {
            // Application will receive this message when OpenCPU RIL starts up.
            // Then application needs to call Ql_RIL_Initialize to launch the initialization of RIL.
        case MSG_ID_RIL_READY:
            Ql_RIL_Initialize();
            //
            // After RIL initialization, developer may call RIL-related APIs in the .h files in the directory of SDK\ril\inc
            // RIL-related APIs may simplify programming, and quicken the development.
            //
            programmData.firstInit = TRUE;
            APP_DEBUG("<-- RIL is ready firstInit=%d-->\r\n", programmData.firstInit);
            break;

            // Handle URC messages.
            // URC messages include "module init state", "CFUN state", "SIM card state(change)",
            // "GSM network state(change)", "GPRS network state(change)" and other user customized URC.
        case MSG_ID_URC_INDICATION:
            switch (msg.param1)
            {
            // URC for module initialization state
			case URC_SYS_INIT_STATE_IND:
				if (SYS_STATE_SMSOK == msg.param2)
				{
					// SMS option has been initialized, and application can program SMS
					//APP_DEBUG("<-- Application can program SMS -->\r\n");
					APP_DEBUG("\r\n<-- SMS module is ready -->\r\n");
					APP_DEBUG("\r\n<-- Initialize SMS-related options -->\r\n");
					ret = SMS_Initialize();
					if (!ret)
						APP_DEBUG("Fail to initialize SMS\r\n");
				}
            break;

            // URC for SIM card state(change)
			case URC_SIM_CARD_STATE_IND:
				if (SIM_STAT_READY == msg.param2)
				{
					APP_DEBUG("<-- SIM card is ready -->\r\n");
				}
				else
				{
					APP_DEBUG("<-- SIM card is not available, cause:%d -->\r\n", msg.param2);
					/* cause: 0 = SIM card not inserted
					 *        2 = Need to input PIN code
					 *        3 = Need to input PUK code
					 *        9 = SIM card is not recognized
					 */
				}
            break;

            // URC for GSM network state(change).
            // Application receives this URC message when GSM network state changes, such as register to
            // GSM network during booting, GSM drops down.
			case URC_GSM_NW_STATE_IND:
				if (NW_STAT_REGISTERED == msg.param2 || NW_STAT_REGISTERED_ROAMING == msg.param2)
				{
					APP_DEBUG("<-- Module has registered to GSM network -->\r\n");
					ret = Ql_OS_SendMessage(programmData.subTaskId1, MSG_ID_NETWORK_REGISTRATION, msg.param1, msg.param2);
				}
				else
				{
					APP_DEBUG("<-- GSM network status:%d -->\r\n", msg.param2);
					/* status: 0 = Not registered, module not currently search a new operator
					 *         2 = Not registered, but module is currently searching a new operator
					 *         3 = Registration denied
					 */
				}
            break;

            // URC for GPRS network state(change).
            // Application receives this URC message when GPRS network state changes, such as register to
            // GPRS network during booting, GSM drops down.
			case URC_GPRS_NW_STATE_IND:
				if (NW_STAT_REGISTERED == msg.param2 || NW_STAT_REGISTERED_ROAMING == msg.param2)
				{
					APP_DEBUG("<-- Module has registered to GPRS network -->\r\n");

				}
				else
				{
					APP_DEBUG("<-- GPRS network status:%d -->\r\n", msg.param2);
					/* status: 0 = Not registered, module not currently search a new operator
					 *         2 = Not registered, but module is currently searching a new operator
					 *         3 = Registration denied
					 */
				}
            break;

            case URC_NEW_SMS_IND:
                APP_DEBUG("<-- New SMS Arrives: index=%d\r\n", msg.param2);
                Hdlr_RecvNewSMS((msg.param2), TRUE, &programmSettings,  &programmData);;// FALSE);
                break;

            case URC_MODULE_VOLTAGE_IND:
                APP_DEBUG("\r\n<-- VBatt Voltage Ind: type=%d\r\n", msg.param2);
                break;

            case URC_COMING_CALL_IND:
                {
                    ST_ComingCall* pComingCall = (ST_ComingCall*)msg.param2;
                    APP_DEBUG("<-- Coming call, number:%s, type:%d -->\r\n", pComingCall->phoneNumber, pComingCall->type);

                    // Enable module sleep mode
                    //ret = Ql_SleepEnable();
                    //APP_DEBUG("\r\n<-- Request sleep mode, ret=%d -->\r\n\r\n", ret);
                    break;
                }

            default:
            	APP_DEBUG("<--MSG_ID_URC_INDICATION,  msg.param1=<%u>, msg.param2=<%u>-->\r\n", msg.param1, msg.param2);
                //APP_DEBUG("<-- Other URC: type=%d\r\n", msg.param1);
                break;
            };
            break;

        default:
            break;
        };
    }
}

/**************************************************************
* the 1st sub task
***************************************************************/
void proc_subtask1(s32 TaskId)
{
    ST_MSG subtask1_msg;
    s32 ret;

    programmData.subTaskId1 = TaskId;

    do
    {
    	Ql_Sleep(100);//in ms
    }
    while(programmData.firstInit == FALSE);
    ret = Ql_SleepDisable();

    InitFlash();
    InitUART();
    InitGPIO();
    InitADC();
    //InitSN();

    APP_DEBUG("<-- subtask1: enter, subTaskId1=%d ->\r\n", programmData.subTaskId1);

    while (TRUE)
    {
        Ql_OS_GetMessage(&subtask1_msg);//нельз€ убирать!!! т.к. все колбеки, проинициализированные в этом процессе не будут вызыватьс€!!!
        switch(subtask1_msg.message)
        {
			case MSG_ID_NETWORK_REGISTRATION:
			{
				if(programmData.timeInit == FALSE)
				{
					programmData.timeInit = TRUE;
					InitSN();
					InitTIME();
				}
				break;
			}
            default:
            	APP_DEBUG("<-- URC from subtask1: message=%d, param1=%d, param2=%d\r\n",subtask1_msg.message,subtask1_msg.param1,subtask1_msg.param1);
                break;
        }
    }
}

/*****************************************************************/
//Callback implementation
/*****************************************************************/
static void wdt_callback_onTimer(u32 timerId, void* param)
{
    s32* wtdid = (s32*)param;

    if(programmData.needReboot == FALSE)
    {
    	//feed logic wdt
    	Ql_WTD_Feed(*wtdid);//APP_DEBUG("<-- time to feed logic watchdog wtdId=%d -->\r\n",*wtdid);

    	//feed HW wdt
    	Ql_GPIO_SetLevel(wdt_pin, Ql_GPIO_GetLevel(wdt_pin) == PINLEVEL_HIGH ? PINLEVEL_LOW : PINLEVEL_HIGH);
    }
    else
    {
    	u32 cnt = WTD_TMR_TIMEOUT*2/100 + 1;
    	APP_DEBUG("<-- time to not feed logic watchdog (wtdId=%d) needReboot=(%s) cnt=(%d)-->\r\n", *wtdid, programmData.needReboot == TRUE ? "TRUE" : "FALSE", cnt);
    	do{
    		Ql_GPIO_SetLevel(led_pin, Ql_GPIO_GetLevel(led_pin) == PINLEVEL_HIGH ? PINLEVEL_LOW : PINLEVEL_HIGH);
    		Ql_Sleep(100);
    	}
    	while(cnt--);

    	APP_DEBUG("<-- wdt_callback_onTimer wait real wdt reboot successfull, try Ql_Reset-->\r\n");
    	Ql_Sleep(100);
    	Ql_Reset(0);
    	while(1);
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
               proc_handle(port, m_RxBuf_Uart, totalBytes/*sizeof(m_RxBuf_Uart)*/);
           }
           break;
        }
    case EVENT_UART_READY_TO_WRITE:
        break;
    default:
        break;
    }
}

static void Callback_OnADCSampling(Enum_ADCPin adcPin, u32 adcValue, void *customParam)
{
	u32 index = *((u32*)customParam);
	//APP_DEBUG( "<-- Callback_OnADCSampling: index=%d -->\r\n", index);
	if(index % 60 == 0)
	{
		//APP_DEBUG( "<-- Callback_OnADCSampling: sampling voltage(mV)=%d  times=%d -->\r\n", adcValue, *((s32*)customParam) );
		u32 capacity, voltage;
		s32 ret = RIL_GetPowerSupply(&capacity, &voltage);
		programmData.dataState.temp = GetTempValue(adcValue);
		if(ret == QL_RET_OK){
			programmData.dataState.voltage = voltage;
			APP_DEBUG( "<-- PowerSupply: power voltage(mV)=%d, sampling voltage(mV)=%d, temp value=%.2f -->\r\n", voltage, adcValue, programmData.dataState.temp);
		}
	}
    *((u32*)customParam) += 1;
}

static void gpio_callback_onTimer(u32 timerId, void* param)
{
	s32 ret;
	if(programmData.firstInit == FALSE) return;
	if (GPIO_TIMER_ID == timerId)
	{
		//APP_DEBUG("<-- Get the button_pin GPIO level value: %d -->\r\n", Ql_GPIO_GetLevel(button_pin));
		s32 btp = GetInputValue(&button_pin, 	&programmData.buttonCnt,(u32)(programmSettings.buttonTimeout * 	1000/GPIO_INPUT_TIMER_PERIOD));
		s32 i1p = GetInputValue(&in1_pin, 		&programmData.in1Cnt, 	(u32)(programmSettings.in1Timeout * 	1000/GPIO_INPUT_TIMER_PERIOD));
		s32 i2p = GetInputValue(&in2_pin, 		&programmData.in2Cnt, 	(u32)(programmSettings.in2Timeout * 	1000/GPIO_INPUT_TIMER_PERIOD));

		if(btp >= 0) programmData.dataState.button = (bool)btp;
		if(programmData.HbuttonState != programmData.dataState.button){//is changed
			programmData.HbuttonState = programmData.dataState.button;
			APP_DEBUG("<-- Get the button_pin GPIO level value changed: %d -->\r\n", programmData.dataState.button);
			if(programmData.dataState.button == FALSE){
				APP_DEBUG("<-- try restore default by user press button!!!-->\r\n");
				bool ret =  restore_default_flash(&programmSettings);
				APP_DEBUG("<-- restore_default_flash ret=%d -->\r\n", ret);
				reboot(&programmData);
			}
		}
		if(i1p >= 0) programmData.dataState.in1 = (bool)i1p;
		if(programmData.Hin1State != programmData.dataState.in1){//is changed
			programmData.Hin1State = programmData.dataState.in1;
			APP_DEBUG("<-- Get the in1_pin GPIO level value changed: %d -->\r\n", programmData.dataState.in1);
		}
		if(i2p >= 0) programmData.dataState.in2 = (bool)i2p;
		if(programmData.Hin2State != programmData.dataState.in2){//is changed
			programmData.Hin2State = programmData.dataState.in2;
			APP_DEBUG("<-- Get the in2_pin GPIO level value changed: %d -->\r\n", programmData.dataState.in2);
		}

		if(programmData.ledCnt > 0){
			programmData.ledCnt--;
			Ql_GPIO_SetLevel(led_pin, Ql_GPIO_GetLevel(led_pin) == PINLEVEL_HIGH ? PINLEVEL_LOW : PINLEVEL_HIGH);
		}

	}
	else if (LED_TIMER_ID == timerId)
	{
	    if(led_cnt-- < 0)
	    {// 1 sec
	      led_cnt = 20;

	      if(programmData.rebootCnt++ > programmSettings.secondsToReboot)
	      {
	        programmData.rebootCnt  = 0;
	        programmData.needReboot = TRUE;
	      }

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

	      Ql_GPIO_SetLevel(led_pin, PINLEVEL_HIGH);//Ql_GPIO_SetLevel(led_pin, Ql_GPIO_GetLevel(led_pin) == PINLEVEL_HIGH ? PINLEVEL_LOW : PINLEVEL_HIGH);
	    }
	    else{
	    	Ql_GPIO_SetLevel(led_pin, PINLEVEL_LOW);
	    }
	}
}

static void time_callback_onTimer(u32 timerId, void* param)
{
	s32 ret;
	if (SYSTEM_TIME_TIMER_ID == timerId)
	{
		programmData.dataState.totalSeconds++;
		if(programmData.timeInit == TRUE)
		{
			if ( *((u32*)param) % 60 == 0)
			{
				if(GetLocalTime() == TRUE){

				}
				//u32 totalMinutes = programmData.dataState.totalSeconds/60;
				//APP_DEBUG("<--time_callback_onTimer: totalSeconds=%d.-->\r\n", programmData.dataState.totalSeconds);
				//u16 time_mode = programmSettings.ipSettings.mode % 100;
			}
		}
		*((u32*)param) += 1;//должно быть после % 60
		///////////////
		if(programmData.dataState.totalSeconds >= programmData.reconnectCnt)
		{
			u16 step = 300;
			if(programmSettings.secondsToReconnect % step == 0)
				programmData.reconnectCnt = ((programmData.dataState.totalSeconds/step)+0) * step + programmSettings.secondsToReconnect;
			else
				programmData.reconnectCnt = programmData.dataState.totalSeconds + programmSettings.secondsToReconnect;

	        if(m_socketid >= 0)
	        {
				APP_DEBUG("<--Socket reconnect timeout, need_tcp_connect socketId=%d, secondsToReconnect=<%d> -->\r\n", m_socketid, programmSettings.secondsToReconnect);
				m_tcp_state = STATE_SOC_CLOSE;
	        }
	        else{
	        	m_tcp_state = STATE_NW_GET_SIMSTATE;
	        }
	        APP_DEBUG("<-- reconnectCnt reload = <%lu> -->\r\n", programmData.reconnectCnt);
	        u32 dur = 60;
	        if(programmSettings.secondsOfDuration >= 30){
	        	programmData.durationCnt  = programmSettings.secondsOfDuration;
	        	APP_DEBUG("<-- Duration reload = <%lu> -->\r\n", programmData.durationCnt);
	        }
	        programmData.tryconnectCnt = 0;
		}
	    if(programmData.durationCnt > 0)
	    {
	    	programmData.durationCnt--;
	    	if(programmData.durationCnt == 0)
	    	{
	  	        if(m_socketid >= 0){
	  				m_tcp_state = STATE_SOC_CLOSE;
	  	        }
	  	        else{
	  	        	m_tcp_state = STATE_NW_GET_SIMSTATE;
	  	        }
	  	        APP_DEBUG("<-- Duration is off, close socket and go sim_state -->\r\n");
	    	}
	    }
	    if(programmData.pingCnt++ >= programmSettings.secondsToPing)
	    {
	    	programmData.pingCnt  = 0;
	    	if(m_remain_len == 0 && m_pCurrentPos == NULL && m_socketid >= 0)
	        {
	        	char tmp_buff[TEMP_BUFFER_LEN] = {0};
	        	Ql_memset(tmp_buff, 0x0, sizeof(tmp_buff));

	        	//if(programmSettings.ipSettings.mode)
	        	s32 len = toJSON(tmp_buff, &programmData.dataState);
	        	if(len > 0 && len <= sizeof(tmp_buff)){
	        		m_pCurrentPos = m_send_buf;
	        		Ql_memcpy(m_pCurrentPos, tmp_buff, len);
	        		m_remain_len += len;
	        		m_pCurrentPos[m_remain_len] = 0;
	        		APP_DEBUG("<-- Send ping [%s] -->\r\n", m_pCurrentPos);
	        	}
	        }
	    }
		////////////////
	}
}

//gsm functions//////////////////////////////////////////
void Callback_GPRS_Actived(u8 contexId, s32 errCode, void* customParam)
{
    if(errCode == SOC_SUCCESS)
    {
        APP_DEBUG("<--CallBack: active GPRS successfully.-->\r\n");
        m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
    }else
    {
        APP_DEBUG("<--CallBack: active GPRS successfully,errCode=%d-->\r\n",errCode);
        m_tcp_state = STATE_GPRS_ACTIVATE;
    }
}

void Callback_GetIpByName(u8 contexId, u8 requestId, s32 errCode,  u32 ipAddrCnt, u32* ipAddr)
{
    u8 i=0;
    u8* ipSegment = (u8*)ipAddr;

    APP_DEBUG("<-- %s:contexid=%d, requestId=%d,error=%d,num_entry=%d -->\r\n", __func__, contexId, requestId, errCode, ipAddrCnt);
    if (errCode == SOC_SUCCESS)
    {
        APP_DEBUG("<--CallBack: get ip by name successfully.-->\r\n");
        for(i=0; i<ipAddrCnt; i++)
        {
            ipSegment = (u8*)(ipAddr + i);
            APP_DEBUG("<--Entry=%d, ip=%d.%d.%d.%d-->\r\n",i,ipSegment[0],ipSegment[1],ipSegment[2],ipSegment[3]);
        }

        // Fetch the first ip address as the valid IP
        Ql_memcpy(m_ipaddress, ipAddr, 4);
        m_tcp_state = STATE_SOC_REGISTER;
    }
}

void callback_socket_connect(s32 socketId, s32 errCode, void* customParam )
{
    if (errCode == SOC_SUCCESS)
    {
        if (timeout_90S_monitor) //stop timeout monitor
        {
           Ql_Timer_Stop(TIMEOUT_90S_TIMER_ID);
           timeout_90S_monitor = FALSE;
        }
        programmData.pingCnt  = programmSettings.secondsToPing - 5;//reload ping cnt (was 0)
        if(programmData.tryconnectCnt < programmSettings.tryConnectCnt)//add tryconnectCnt
        	programmData.tryconnectCnt++;

        APP_DEBUG("<--Callback: socket connect successfully. Try connect=%d-->\r\n", programmData.tryconnectCnt);

        //////// forming send init packet
        programmData.lastPacket.pid = 0;
        programmData.lastPacket.timeStamp = programmData.dataState.totalSeconds;
		m_pCurrentPos = m_send_buf;
		m_remain_len = 0;
		if(programmSettings.ipSettings.mode == 101){
			m_pCurrentPos[m_remain_len++] = 0;
			m_pCurrentPos[m_remain_len++] = 0x0B;
			bShort length;
			length.Data_s = Ql_strlen(programmData.dataState.imei) + Ql_strlen(programmData.dataState.iccid) + 1;
			m_pCurrentPos[m_remain_len++] = length.Data_b[1];
			m_pCurrentPos[m_remain_len++] = length.Data_b[0];
		}
		u32 len = Ql_strlen(programmData.dataState.imei);
		Ql_memcpy((m_pCurrentPos + m_remain_len), programmData.dataState.imei, len);
		m_remain_len += len;
		*(m_pCurrentPos + m_remain_len) = 0xFF;
		m_remain_len += 1;
		len = Ql_strlen(programmData.dataState.iccid);
		Ql_memcpy((m_pCurrentPos + m_remain_len), programmData.dataState.iccid, len);
		m_remain_len += len;
		*(m_pCurrentPos + m_remain_len) = 0xFF;
		m_remain_len += 1;
		s32 rs =  Ql_sprintf((m_pCurrentPos + m_remain_len), "%s", HW_VERSION);
		if(rs > 0){
			m_remain_len += rs;
		}
		////////////
        m_tcp_state = STATE_SOC_SEND;
    }
    else
    {
        APP_DEBUG("<--Callback: socket connect failure,(socketId=%d),errCode=%d-->\r\n",socketId, errCode);
        Ql_SOC_Close(socketId);
        socketId = -1;
        m_tcp_state = STATE_SOC_CREATE;
    }
}

void callback_socket_close(s32 socketId, s32 errCode, void* customParam )
{
    m_nSentLen  = 0;

    if (errCode == SOC_SUCCESS)
    {
        APP_DEBUG("<-- CallBack: close socket successfully. -->\r\n");
    }
    else if(errCode == SOC_BEARER_FAIL)
    {
        m_tcp_state = STATE_GPRS_DEACTIVATE;
        APP_DEBUG("<-- CallBack: close socket failure, (socketId=%d, error_cause=%d) -->\r\n", socketId, errCode);
    }
    else
    {
        m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
        APP_DEBUG("<-- CallBack: close socket failure, (socketId=%d, error_cause=%d) -->\r\n", socketId, errCode);
    }
    m_socketid = -1;
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
        if(errCode == SOC_BEARER_FAIL)
        {
            m_tcp_state = STATE_GPRS_DEACTIVATE;
        }
        else
        {
            m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
        }
        return;
    }


    Ql_memset(m_recv_buf, 0, RECV_BUFFER_LEN);
    do
    {
        ret = Ql_SOC_Recv(socketId, m_recv_buf, RECV_BUFFER_LEN);
        if((ret < 0) && (ret != -2))
        {
            APP_DEBUG("<-- Receive data failure,ret=%d. -->\r\n",ret);
            APP_DEBUG("<-- Close socket. -->\r\n");
            Ql_SOC_Close(socketId); //you can close this socket
            m_socketid = -1;
            m_tcp_state = STATE_SOC_CREATE;
            break;
        }
        else if(ret == -2)
        {
            //wait next CallBack_socket_read
            break;
        }
        else if(ret <= RECV_BUFFER_LEN)
        {
        	programmData.ledCnt = ret;//blink

        	char tmp_buff[TEMP_BUFFER_LEN] = {0};
        	Ql_memset(tmp_buff, 0x0, sizeof(tmp_buff));

            APP_DEBUG("<-- Receive data from sock(%d), len(%d) -->\r\n", socketId, ret);
    		char *answer = Parse_Command(m_recv_buf, tmp_buff, &programmSettings, &programmData);
    		if( answer != NULL)
    		{
    			u32 alen = Ql_strlen(answer);
    			APP_DEBUG("cmd from socket, answer=<%s>", answer);

    			//send answer to server
    			m_pCurrentPos = m_send_buf;
    			Ql_memcpy(m_pCurrentPos + m_remain_len, answer, alen);//!!!
    			m_remain_len += alen;
    		}
    		else
    		{
    			APP_DEBUG("<-- and write to UART_PORT3 mode=%d, ret=%d -->\r\n", programmSettings.ipSettings.mode, ret);
    			if(programmSettings.ipSettings.mode == 101)
    			{//with pid
    				if(ret > 4 &&  AnalizePidPacket(m_recv_buf, ret, &programmData.lastPacket) == TRUE){
    					programmData.lastPacket.timeStamp = programmData.dataState.totalSeconds;
    					s32 nlen = (ret-4);
    					Ql_UART_Write(UART_PORT3, (u8*)(m_recv_buf+4), nlen);
    				}
    			}
    			else{
    				Ql_UART_Write(UART_PORT3, m_recv_buf, ret);
    			}
    		}

    		/*//сброс счетчика переконнекта, убрал тк вли€ет на логику переконнектов
            if(ret > 1)
            	programmData.reconnectCnt = 0;
            else if(ret == 1)
            	programmData.reconnectCnt = programmSettings.secondsToReconnect/10;
 	 	 	 */
            //read finish, wait next CallBack_socket_read
            break;
        }
    }
    while(1);
}

void callback_socket_write(s32 socketId, s32 errCode, void* customParam )
{
    s32 ret;

    if(errCode)
    {
        APP_DEBUG("<--CallBack: socket write failure,(sock=%d,error=%d)-->\r\n",socketId,errCode);
        APP_DEBUG("<-- Close socket.-->\r\n");
        ret = Ql_SOC_Close(socketId);
        m_socketid = -1;

        if(ret == SOC_BEARER_FAIL)
        {
            m_tcp_state = STATE_GPRS_DEACTIVATE;
        }
        else
        {
            m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
        }
        return;
    }


    m_tcp_state = STATE_SOC_SENDING;

    do
    {
        ret = Ql_SOC_Send(m_socketid, m_pCurrentPos, m_remain_len);
        APP_DEBUG("<--CallBack: Send data, socketid=%d, number of bytes sent=%d, m_remain_len=%d -->\r\n", m_socketid, ret, m_remain_len);

        if(ret == m_remain_len)//send compelete
        {
            m_remain_len = 0;
            m_pCurrentPos = NULL;
            m_nSentLen += ret;
            m_tcp_state = STATE_SOC_ACK;
            break;
         }
         else if((ret < 0) && (ret == SOC_WOULDBLOCK))
         {
            //you must wait CallBack_socket_write, then send data;
            break;
         }
         else if(ret < 0)
         {
              APP_DEBUG("<-- Send data failure, ret=%d. -->\r\n", ret);
              APP_DEBUG("<-- Close socket. -->\r\n");
              Ql_SOC_Close(socketId);//error , Ql_SOC_Close
              m_socketid = -1;

              m_remain_len = 0;
              m_pCurrentPos = NULL;
              if(ret == SOC_BEARER_FAIL)
              {
                  m_tcp_state = STATE_GPRS_DEACTIVATE;
              }
              else
              {
                  m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
              }
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
}

void CallBack_GPRS_Deactived(u8 contextId, s32 errCode, void* customParam )
{
    if (errCode == SOC_SUCCESS)
    {
        APP_DEBUG("<--CallBack:deactived GPRS successfully.-->\r\n");
        m_tcp_state = STATE_NW_GET_SIMSTATE;
    }else
    {
        APP_DEBUG("<--CallBack:deactived GPRS failure,(contexid=%d,error_cause=%d)-->\r\n", contextId,errCode);
        reboot(&programmData);
    }
}

static void Restart_GSM(void)
{
    APP_DEBUG("<--Deactivate GPRS.-->\r\n");
    s32 ret;
    ret = Ql_GPRS_DeactivateEx(0, TRUE);
    if (GPRS_PDP_SUCCESS == ret){
    	APP_DEBUG("<-- GPRS is deactivated successfully. -->\r\n");
    	m_tcp_state = STATE_NW_GET_SIMSTATE;
    }else{
    	APP_DEBUG("<--Fail to activate GPRS, error code is in %d.-->\r\n", ret);
    	reboot(&programmData);
    }
}

static void checkErr_AckNumber(s32 err_code)
{
    if(SOC_INVALID_SOCKET == err_code)
    {
        APP_DEBUG("<-- Invalid socket ID -->\r\n");
    }
    else if(SOC_INVAL == err_code)
    {
        APP_DEBUG("<-- Invalid parameters for ACK number -->\r\n");
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
/////////////////////////////////////////////////////////
static void gsm_callback_onTimer(u32 timerId, void* param)
{
	s32 ret;

	if(programmData.firstInit == FALSE) return;

    if (TIMEOUT_90S_TIMER_ID == timerId)
    {
        APP_DEBUG("<-- 90s time out!!! -->\r\n");

        ret = Ql_SOC_Close(m_socketid);
        m_socketid = -1;

        APP_DEBUG("<-- Closing socket. ret=<%d> -->\r\n", ret);

        m_tcp_state = STATE_GPRS_DEACTIVATE;

        timeout_90S_monitor = FALSE;
    }
    else if (CSQ_TIMER_ID == timerId)
    {
    	//APP_DEBUG("<--...........CSQ_TIMER_ID=%d..................-->\r\n", timerId);
        RIL_NW_GetSignalQuality(&programmData.dataState.rssi, &programmData.dataState.ber);
        u64 totalMS;
        totalMS = Ql_GetMsSincePwrOn();
        APP_DEBUG("<-- Uptime: %lld ms, signal strength: %d, BER: %d -->\r\n", totalMS, programmData.dataState.rssi, programmData.dataState.ber);
    }
    else if (TCP_TIMER_ID == timerId)
    {
        //APP_DEBUG("<--...........m_tcp_state=%d..................-->\r\n", m_tcp_state);
        switch (m_tcp_state)
        {
            case STATE_NW_GET_SIMSTATE:
            {
				s32 simStat = 0;
				RIL_SIM_GetSimState(&simStat);
				if (simStat == SIM_STAT_READY)
				{
					m_tcp_state = STATE_NW_QUERY_STATE;
					APP_DEBUG("<--SIM card status is normal!-->\r\n");
				}
				else
				{
					//    Ql_Timer_Stop(TCP_TIMER_ID);
					APP_DEBUG("<--SIM card status is unnormal!-->\r\n");
				}
                break;
            }
            case STATE_NW_QUERY_STATE:
            {
                s32 creg = 0;
                s32 cgreg = 0;
                //Ql_NW_GetNetworkState(&creg, &cgreg);
                ret = RIL_NW_GetGSMState(&creg);
                ret = RIL_NW_GetGPRSState(&cgreg);
                APP_DEBUG("<--Network State:creg=%d,cgreg=%d-->\r\n",creg,cgreg);
                if((cgreg == NW_STAT_REGISTERED)||(cgreg == NW_STAT_REGISTERED_ROAMING))
                {
                    m_tcp_state = STATE_GPRS_REGISTER;
                }
                break;
            }
            case STATE_GPRS_REGISTER:
            {
                ret = Ql_GPRS_Register(0, &callback_gprs_func, NULL);
                if (GPRS_PDP_SUCCESS == ret)
                {
                    APP_DEBUG("<--Register GPRS callback function successfully.-->\r\n");
                    m_tcp_state = STATE_GPRS_CONFIG;
                }else if (GPRS_PDP_ALREADY == ret)
                {
                    APP_DEBUG("<--GPRS callback function has already been registered,ret=%d.-->\r\n",ret);
                    m_tcp_state = STATE_GPRS_CONFIG;
                }else
                {
                    APP_DEBUG("<--Register GPRS callback function failure,ret=%d.-->\r\n",ret);
                }
                break;
            }
            case STATE_GPRS_CONFIG:
            {
                Ql_strcpy(m_gprsCfg.apnName, programmSettings.gsmSettings.gprsApn);
                Ql_strcpy(m_gprsCfg.apnUserId, programmSettings.gsmSettings.gprsUser);
                Ql_strcpy(m_gprsCfg.apnPasswd, programmSettings.gsmSettings.gprsPass);
                m_gprsCfg.authtype = 0;
                ret = Ql_GPRS_Config(0, &m_gprsCfg);
                if (GPRS_PDP_SUCCESS == ret)
                {
                    APP_DEBUG("<--configure GPRS param successfully.-->\r\n");
                }else
                {
                    APP_DEBUG("<--configure GPRS param failure,ret=%d.-->\r\n",ret);
                }

                m_tcp_state = STATE_GPRS_ACTIVATE;
                break;
            }
            case STATE_GPRS_ACTIVATE:
            {
                m_tcp_state = STATE_GPRS_ACTIVATING;
                ret = Ql_GPRS_Activate(0);
                if (ret == GPRS_PDP_SUCCESS)
                {
                    APP_DEBUG("<--Activate GPRS successfully.-->\r\n");
                    m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
                }else if (ret == GPRS_PDP_WOULDBLOCK)
                {
                     APP_DEBUG("<--Waiting for the result of GPRS activated.,ret=%d.-->\r\n", ret);
                    //waiting Callback_GPRS_Actived
                }else if (ret == GPRS_PDP_ALREADY)
                {
                    APP_DEBUG("<--GPRS has already been activated,ret=%d.-->\r\n",ret);
                    m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
                }else//error
                {
                    APP_DEBUG("<--Activate GPRS failure,ret=%d.-->\r\n",ret);
                    m_tcp_state = STATE_GPRS_ACTIVATE;
                }
                break;
            }
            case STATE_GPRS_GET_DNSADDRESS:
            {
                u8 primaryAddr[16] = {0};
                u8 bkAddr[16] = {0};
                ret =Ql_GPRS_GetDNSAddress(0, (u32*)primaryAddr,  (u32*)bkAddr);
                if (ret == GPRS_PDP_SUCCESS)
                {
                    APP_DEBUG("<--Get DNS address successfully,primaryAddr=%d.%d.%d.%d,bkAddr=%d.%d.%d.%d-->\r\n",primaryAddr[0],primaryAddr[1],primaryAddr[2],primaryAddr[3],bkAddr[0],bkAddr[1],bkAddr[2],bkAddr[3]);
                    m_tcp_state = STATE_GPRS_GET_LOCALIP;
                }else
                {
                     APP_DEBUG("<--Get DNS address failure,ret=%d.-->\r\n",ret);
                    m_tcp_state = STATE_GPRS_DEACTIVATE;
                }
                break;
            }
            case STATE_GPRS_GET_LOCALIP:
            {
                u8 ip_addr[5];
                Ql_memset(ip_addr, 0, 5);
                ret = Ql_GPRS_GetLocalIPAddress(0, (u32 *)ip_addr);
                if (ret == GPRS_PDP_SUCCESS)
                {
                	Ql_sprintf(programmSettings.ipSettings.srcAddress,"%d.%d.%d.%d", ip_addr[0],ip_addr[1],ip_addr[2],ip_addr[3]);
                	APP_DEBUG("<--Get Local Ip successfully, Local Ip=%s-->\r\n", programmSettings.ipSettings.srcAddress);

                    //APP_DEBUG("<--Get Local Ip successfully, Local Ip=%d.%d.%d.%d-->\r\n",ip_addr[0],ip_addr[1],ip_addr[2],ip_addr[3]);
                    m_tcp_state = STATE_CHACK_SRVADDR;
                }else
                {
                    APP_DEBUG("<--Get Local Ip failure,ret=%d.-->\r\n",ret);
                }
                break;
            }
            case STATE_CHACK_SRVADDR:
            {
                Ql_memset(m_ipaddress,0,5);
                ret = Ql_IpHelper_ConvertIpAddr(/*m_SrvADDR*/programmSettings.ipSettings.dstAddress, (u32 *)m_ipaddress);
                if(ret == SOC_SUCCESS) // ip address, xxx.xxx.xxx.xxx
                {
                    APP_DEBUG("<--Convert Ip Address successfully,m_ipaddress=%d,%d,%d,%d-->\r\n",m_ipaddress[0],m_ipaddress[1],m_ipaddress[2],m_ipaddress[3]);
                    m_tcp_state = STATE_SOC_REGISTER;

                }
                else  //domain name
                {
                    ret = Ql_IpHelper_GetIPByHostName(0, 0, /*m_SrvADDR*/programmSettings.ipSettings.dstAddress, Callback_GetIpByName);
                    if(ret == SOC_SUCCESS)
                    {
                        APP_DEBUG("<--Get ip by hostname successfully.-->\r\n");
                    }
                    else if(ret == SOC_WOULDBLOCK)
                    {
                        APP_DEBUG("<--Waiting for the result of Getting ip by hostname,ret=%d.-->\r\n", ret);
                        //waiting CallBack_getipbyname
                    }
                    else
                    {
                        APP_DEBUG("<--Get ip by hostname failure:ret=%d-->\r\n", ret);
                        if(ret == SOC_BEARER_FAIL)
                        {
                             m_tcp_state = STATE_GPRS_DEACTIVATE;
                        }
                        else
                        {
                             m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
                        }
                    }
                }
                break;
            }
            case STATE_SOC_REGISTER:
            {
            	if(programmData.durationCnt == 0) break;
            	if(programmData.tryconnectCnt >= programmSettings.tryConnectCnt){
            		programmData.durationCnt = 0;
            		APP_DEBUG("<--The max number of connection attempts has been exceeded! tryconnectCnt=%d-->\r\n", programmData.tryconnectCnt);
            		programmData.tryconnectCnt = 0;//na vsiakiy pojarniy
    	  	        if(m_socketid >= 0){
    	  				m_tcp_state = STATE_SOC_CLOSE;
    	  	        }
    	  	        else{
    	  	        	m_tcp_state = STATE_NW_GET_SIMSTATE;
    	  	        }
            		break;
            	}

                ret = Ql_SOC_Register(callback_soc_func, NULL);
                if (SOC_SUCCESS == ret)
                {
                    APP_DEBUG("<--Register socket callback function successfully.-->\r\n");
                    m_tcp_state = STATE_SOC_CREATE;
                }else if (SOC_ALREADY == ret)
                {
                    APP_DEBUG("<--Socket callback function has already been registered,ret=%d.-->\r\n",ret);
                    m_tcp_state = STATE_SOC_CREATE;
                }else
                {
                    APP_DEBUG("<--Register Socket callback function failure,ret=%d.-->\r\n",ret);
                }
                break;
            }
            case STATE_SOC_CREATE:
            {
                m_socketid = Ql_SOC_Create(0, SOC_TYPE_TCP);
                if (m_socketid >= 0)
                {
                    APP_DEBUG("<--Create socket id successfully, socketid=%d.-->\r\n", m_socketid);
                    m_tcp_state = STATE_SOC_CONNECT;

                    u64 ackedNumCurr = 0;
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
                    APP_DEBUG("<--Create socket id failure, error=%d.-->\r\n", m_socketid);
                }
                break;
            }
            case STATE_SOC_CONNECT:
            {
                m_tcp_state = STATE_SOC_CONNECTING;
                ret = Ql_SOC_Connect(m_socketid,(u32) m_ipaddress, /*m_SrvPort*/programmSettings.ipSettings.dstPort);
                if(ret == SOC_SUCCESS)
                {
                    APP_DEBUG("<--The socket is already connected.-->\r\n");
                    m_tcp_state = STATE_SOC_SEND;

                }
                else if(ret == SOC_WOULDBLOCK)
                {
                      if (!timeout_90S_monitor)//start timeout monitor
                      {
                        Ql_Timer_Start(TIMEOUT_90S_TIMER_ID, TIMEOUT_90S_PERIOD, FALSE);
                        timeout_90S_monitor = TRUE;
                      }
                      APP_DEBUG("<--Waiting for the result of socket connection,ret=%d.-->\r\n",ret);
                      //waiting CallBack_getipbyname

                }
                else //error
                {
                    APP_DEBUG("<--Socket Connect failure,ret=%d.-->\r\n",ret);
                    Ql_SOC_Close(m_socketid);
                    m_socketid = -1;

                    APP_DEBUG("<-- Closing socket.-->\r\n");
                    if(ret == SOC_BEARER_FAIL)
                    {
                        m_tcp_state = STATE_GPRS_DEACTIVATE;
                    }
                    else
                    {
                        m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
                    }
                }
                break;
            }
            case STATE_SOC_SEND:
            {
            	if(m_remain_len <= 0)
                {
					//APP_DEBUG("<-- No data need to send, waiting to send data -->\r\n");
                    break;
				}

                m_tcp_state = STATE_SOC_SENDING;
                do
                {
                	APP_DEBUG("<-- Sending data, socketid=%d, number of bytes sent=%d -->\r\n", m_socketid, m_remain_len);
                    ret = Ql_SOC_Send(m_socketid, m_pCurrentPos, m_remain_len);
                    if(ret == m_remain_len)//send compelete
                    {
                        m_remain_len = 0;
                        m_pCurrentPos = NULL;
                        m_nSentLen += ret;
                        m_tcp_state = STATE_SOC_ACK;
                        break;
                    }
                    else if((ret <= 0) && (ret == SOC_WOULDBLOCK))
                    {
                    	APP_DEBUG("<-- waiting CallBack_socket_write, then send data, ret=%d.-->\r\n", ret);
                        //waiting CallBack_socket_write, then send data;
                        break;
                    }
                    else if(ret <= 0)
                    {
                        APP_DEBUG("<--Send data failure, ret=%d.-->\r\n", ret);
                        APP_DEBUG("<-- Close socket.-->\r\n");
                        Ql_SOC_Close(m_socketid);//error , Ql_SOC_Close
                        m_socketid = -1;

                        m_remain_len = 0;
                        m_pCurrentPos = NULL;
                        if(ret == SOC_BEARER_FAIL)
                        {
                            m_tcp_state = STATE_GPRS_DEACTIVATE;
                        }
                        else
                        {
                            m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
                        }
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
            case STATE_SOC_ACK:
            {
                u64 ackedNumCurr = 0;
                ret = Ql_SOC_GetAckNumber(m_socketid, &ackedNumCurr);
                if (ret < 0)
                {
                    checkErr_AckNumber(ret);
                }
                if (m_nSentLen == ackedNumCurr)
                {
                    if (timeout_90S_monitor) //stop timeout monitor
                    {
                        Ql_Timer_Stop(TIMEOUT_90S_TIMER_ID);
                        timeout_90S_monitor = FALSE;
                    }
                    APP_DEBUG("<-- ACK Number:%llu/%llu. Server has received all data. -->\r\n\r\n", m_nSentLen, ackedNumCurr);
                    //Ql_memset(m_send_buf, 0, SEND_BUFFER_LEN);//!!!
                    m_tcp_state = STATE_SOC_SEND;
                }
                else
                {
                    if (!timeout_90S_monitor)//start timeout monitor
                    {
                        Ql_Timer_Start(TIMEOUT_90S_TIMER_ID, TIMEOUT_90S_PERIOD, FALSE);
                        timeout_90S_monitor = TRUE;
                    }

                    APP_DEBUG("<-- ACK Number:%llu/%llu from socket[%d] -->\r\n", ackedNumCurr, m_nSentLen, m_socketid);
                }
                break;
            }
            case STATE_SOC_CLOSE:
            {
            	if(m_socketid >= 0)
            	{
            		s16 rcnt = 3;
            		do
            		{
                		ret = Ql_SOC_Close(m_socketid);//error , Ql_SOC_Close
                		APP_DEBUG("<--socket <%d> closed, ret(%d)-->\r\n", m_socketid, ret);
                		if(ret >= 0)
                			break;
            		}
            		while(rcnt-- > 0);
            		if(rcnt <= 0){
            			programmData.needReboot = TRUE;
            		}

            				/*;
            		ret = Ql_SOC_Close(m_socketid);//error , Ql_SOC_Close
            		APP_DEBUG("<--socket <%d> closed, ret(%d)-->\r\n", m_socketid, ret);
            		if(ret < 0){
            			programmData.needReboot = TRUE;
            		}*/
            	}
		        m_socketid = -1;
                m_remain_len = 0;
                m_pCurrentPos = NULL;
                Ql_Timer_Stop(TIMEOUT_90S_TIMER_ID);
                m_tcp_state = STATE_NW_GET_SIMSTATE;//!!!!
                break;
            }

            case STATE_GPRS_DEACTIVATE:
            {
                //APP_DEBUG("<--Deactivate GPRS.-->\r\n");
                //Ql_GPRS_Deactivate(0);
            	Restart_GSM();
                break;
            }
            default:
                break;
        }
    }
}
/*****************************************************************/
//Init implementation
/*****************************************************************/
static void InitWDT(s32 *wtdid)
{
    s32 ret;

    APP_DEBUG("<-- InitWDT -->\r\n");

    //ѕеределал HW WDT на самосто€тельное дерганье ногой.
    // Initialize external watchdog: specify the GPIO pin (PINNAME_RI) and the overflow time is 600ms.
    //ret = Ql_WTD_Init(0, PINNAME_RI, 300);//дергаем ногой почаще тк врем€ сброса у tps-ки 1600мс/ возможный минимум 200мс
    //if (0 == ret)
    //    APP_DEBUG("\r\n<--OpenCPU: watchdog init OK!-->\r\n");

    //init pin
    Ql_GPIO_Init(wdt_pin, PINDIRECTION_OUT, 	PINLEVEL_HIGH, 	PINPULLSEL_PULLUP);
    // Create a logic watchdog, the interval is 1.5 s
    *wtdid = Ql_WTD_Start(WTD_TMR_TIMEOUT);
    APP_DEBUG("<-- InitWDT wtdid=%d-->\r\n", *wtdid);

    // Register & start a timer to feed the logic watchdog.
    ret = Ql_Timer_RegisterFast(LOGIC_WTD1_TMR_ID, wdt_callback_onTimer, wtdid);
    if(ret < 0){
        APP_DEBUG("<--main task: register fail ret=%d-->\r\n", ret);
        return;
    }
    // The real feeding interval is 0.3 s
    ret = Ql_Timer_Start(LOGIC_WTD1_TMR_ID, 300, TRUE);
    if(ret < 0){
        APP_DEBUG("<--main task: start timer fail ret=%d-->\r\n",ret);
        return;
    }
    APP_DEBUG("<-- InitWDT end -->\r\n");
}

static void InitFlash(void)
{
	APP_DEBUG("<-- OpenCPU: init_flash! Size=%d-->\r\n", sizeof(sProgrammSettings));

    bool ret = FALSE;
    for(int i=0; i < 10; i++)
    {
        ret = init_flash(&programmSettings);
        if(ret == TRUE)
        {
        	APP_DEBUG("<-- init_flash OK crc=<%d> apn=<%s> user=<%s> pass=<%s> server=<%s> port=<%d> baudrate=<%d> toreconnect=<%d> duration=<%d> toreboot=<%d> -->\r\n",
        			programmSettings.crc,
        			programmSettings.gsmSettings.gprsApn,
        			programmSettings.gsmSettings.gprsUser,
        			programmSettings.gsmSettings.gprsPass,
        			programmSettings.ipSettings.dstAddress,
        			programmSettings.ipSettings.dstPort,
        			programmSettings.serPortSettings.baudrate,
        			programmSettings.secondsToReconnect,
        			programmSettings.secondsOfDuration,
        			programmSettings.secondsToReboot
        	);
        	return;
        }
        else
        {
        	Ql_Sleep(1000);
        }
        APP_DEBUG("<-- init_flash ret=%d-->\r\n", ret);
    }

    APP_DEBUG("<-- init_flash ERROR, try restore default!!! -->\r\n");
    ret = restore_default_flash(&programmSettings);

    APP_DEBUG("<-- restore_default_flash ret=%d -->\r\n", ret);
    reboot(&programmData);
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

    ret = Ql_UART_Register(UART_PORT3, CallBack_UART_Hdlr, NULL);
    if (ret < QL_RET_OK)
    	APP_DEBUG("<-- Ql_UART_Register(mySerialPort=%d)=%d -->\r\n", UART_PORT3, ret);

    ret = Ql_UART_OpenEx(UART_PORT3, &dcb);
    if (ret < QL_RET_OK)
    	APP_DEBUG("<-- Ql_UART_OpenEx(mySerialPort=%d)=%d -->\r\n", UART_PORT3, ret);

    APP_DEBUG("<-- InitUART end -->\r\n");
}

static void InitGPRS(void)
{
    //register & start timer
	APP_DEBUG("<-- InitGPRS -->\r\n");

    //register & start timer
    Ql_Timer_Register(TCP_TIMER_ID, gsm_callback_onTimer, NULL);
    Ql_Timer_Start(TCP_TIMER_ID, TCP_TIMER_PERIOD, TRUE);

    Ql_Timer_Register(TIMEOUT_90S_TIMER_ID, gsm_callback_onTimer, NULL);
    timeout_90S_monitor = FALSE;

    Ql_Timer_Register(CSQ_TIMER_ID, gsm_callback_onTimer, NULL);
    Ql_Timer_Start(CSQ_TIMER_ID, CSQ_TIMER_PERIOD, TRUE);

    APP_DEBUG("<-- InitGPRS end -->\r\n");
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

    APP_DEBUG("<-- InitGPIO end -->\r\n");
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

static void InitTIME(void)
{
	APP_DEBUG("<-- InitTIME start -->\r\n");

	u8 status = 0;
	s32 ret = RIL_GetTimeSynch_Status(&status);
	if(ret == RIL_AT_SUCCESS)
	{
		APP_DEBUG("<-- InitTIME status=%d -->\r\n", status);
		if(status != 1){
			status = 1;
			ret = RIL_SetTimeSynch_Status(status);
			if(ret == RIL_AT_SUCCESS){
				//programmData.needReboot = TRUE;
			}
		}
	}

    Ql_Timer_Register(SYSTEM_TIME_TIMER_ID, time_callback_onTimer, (void*)&upTime);
    Ql_Timer_Start(SYSTEM_TIME_TIMER_ID, 1000, TRUE);

    ret = RIL_NW_GetSignalQuality(&programmData.dataState.rssi, &programmData.dataState.ber);
    APP_DEBUG("<-- Signal strength: %d, BER: %d -->\r\n", upTime, programmData.dataState.rssi, programmData.dataState.ber);

	//SYSTEM_TIME_TIMER_ID
	APP_DEBUG("<-- InitTIME end -->\r\n");
}

static void InitSN(void)
{
	APP_DEBUG("<-- InitSN -->\r\n");

    char strIMEI[30];
    Ql_memset(strIMEI, 0x0, sizeof(strIMEI));
    s32 ret = RIL_GetIMEI(strIMEI);
    if(ret == QL_RET_OK){
    	Ql_memset(programmData.dataState.imei, 0x0, sizeof(programmData.dataState.imei));
    	Ql_strcpy(programmData.dataState.imei, strIMEI);
    	APP_DEBUG("<-- IMEI:%s, ret=%d -->\r\n", programmData.dataState.imei, ret);
    }

    char strCCID[30];
    Ql_memset(strCCID, 0x0, sizeof(strCCID));
    ret = RIL_SIM_GetCCID(strCCID);
    if(ret == QL_RET_OK){
    	Ql_memset(programmData.dataState.iccid, 0x0, sizeof(programmData.dataState.iccid));
    	Ql_strcpy(programmData.dataState.iccid, strCCID);
    	APP_DEBUG("<-- ICCID:%s, ret=%d -->\r\n", programmData.dataState.iccid, ret);
    }

    APP_DEBUG("<-- InitSN end -->\r\n");
}

/*****************************************************************/
//Other implementation
/*****************************************************************/
static bool GetLocalTime(void)
{
	bool ret = FALSE;

	//APP_DEBUG("<--Try get Local time -->\r\n");
    if( Ql_GetLocalTime(&time) )
    {
        //This function get total seconds elapsed   since 1970.01.01 00:00:00.
        u32 ts = Ql_Mktime(&time);
        APP_DEBUG("<-- Time successfuly determined (UTC): %i.%i.%i %i:%i:%i timezone=%i-->\r\n", time.day, time.month, time.year, time.hour, time.minute, time.second,time.timezone);

        if(ts > 0){
        	programmData.dataState.timezone = time.timezone/4;
        	programmData.dataState.totalSeconds = ts + (programmData.dataState.timezone*60*60);
        }
        APP_DEBUG("<--totalSeconds=<%lu>-->\r\n", programmData.dataState.totalSeconds);
        ret = TRUE;
    }
    else
    {
        APP_DEBUG("\r\n<--failed !! Local time not determined -->\r\n");
    }
    return ret;
}

static void proc_handle(Enum_SerialPort port, char *pData, s32 len)
{
	APP_DEBUG("<-- Read data from serial port=%d, len=%d -->\r\n", port, len);
	pData[len] = 0;
	char tmp_buff[TEMP_BUFFER_LEN] = {0};
	Ql_memset(tmp_buff, 0x0, sizeof(tmp_buff));

	if(port == UART_PORT3)//485
	{
		programmData.ledCnt = len;//blink

		//send it to server
		m_pCurrentPos = m_send_buf;
		if(programmSettings.ipSettings.mode == 101)
		{
			//APP_DEBUG("<-- proc_handle: m_remain_len=%d -->\r\n", m_remain_len);
			m_remain_len += AddPidHeader(0x03, (u8*)(m_pCurrentPos + m_remain_len), len, &programmData.lastPacket);
			//APP_DEBUG("proc_handle: m_remain_len(after AddPidHeader)=%d", m_remain_len);
		}
		Ql_memcpy(m_pCurrentPos + m_remain_len, pData, len);//!!!
		m_remain_len += len;
		/////////////////////////////////////
		/*
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
		}*/
	    /////////////////////////////////////
		APP_DEBUG("<-- Need send by socket, remain_len=%d -->\r\n", m_remain_len);
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
			char *s1 = Ql_strstr(pData, "ATI");
			char *s2 = Ql_strstr(pData, "ATE");
			char *s3 = Ql_strstr(pData, "AT&W");
			//APP_DEBUG("<--Ql_strstr s=[%d] pData=[%d]-->\r\n", (long)s, (long)pData);
			if( (s != NULL && s == pData) || (s1 != NULL && s1 == pData) || (s2 != NULL && s2 == pData) || (s3 != NULL && s3 == pData)   )
			{
				s32 aret = RIL_NW_SendATCmd(pData, tmp_buff);
				if(aret == RIL_AT_SUCCESS){
					//APP_DEBUG("OK\r\n");
				}
				else{
					//APP_DEBUG("ERROR\r\n");
				}
				//APP_DEBUG("%s", tmp_buff);
				return;
			}

			//if not command, send it to server
			/*
			m_pCurrentPos = m_send_buf;
			Ql_memcpy(m_pCurrentPos + m_remain_len, pData, len);//!!!
			m_remain_len += len;
			*/
		}
	}
}
#endif // __GSM_TEMINAL__
