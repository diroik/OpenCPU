/*
 * gsm_terminal.c
 *
 *  Created on: 23 нояб. 2021 г.
 *      Author: Админ
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
#include "fota_main.h"
/*****************************************************************
******************************************************************/
#include "typedef.h"
#include "flash.h"
#include "infrastructure.h"

/*****************************************************************
* UART Param
******************************************************************/
#define SERIAL_RX_BUFFER_LEN  2048
static u8 m_RxBuf_Uart[SERIAL_RX_BUFFER_LEN];

/*****************************************************************
* timer param
******************************************************************/
#define LOGIC_WTD1_TMR_ID  		(TIMER_ID_USER_START + 1)
#define LOGIC_WTD2_TMR_ID  		(TIMER_ID_USER_START + 2)
#define TCP_TIMER_ID         	(TIMER_ID_USER_START + 3)
#define TIMEOUT_90S_TIMER_ID 	(TIMER_ID_USER_START + 4)   //timeout
#define CSQ_TIMER_ID 		 	(TIMER_ID_USER_START + 5)   //signal quality
#define GPIO_TIMER_ID 		 	(TIMER_ID_USER_START + 6)
#define LED_TIMER_ID 		 	(TIMER_ID_USER_START + 7)

#define WTD_TMR_TIMEOUT 1500

#define TCP_TIMER_PERIOD     800 //800/500
#define TIMEOUT_90S_PERIOD   90000
#define CSQ_TIMER_PERIOD     180000
#define GPIO_INPUT_TIMER_PERIOD 100

static s32 timeout_90S_monitor = FALSE;

/*****************************************************************
* APN Param
******************************************************************/
static ST_GprsConfig  m_gprsCfg;

/*****************************************************************
* Server Param
******************************************************************/
#define SEND_BUFFER_LEN     1400
#define RECV_BUFFER_LEN     1400

static u8 m_send_buf[SEND_BUFFER_LEN];
static u8 m_recv_buf[RECV_BUFFER_LEN];
static u64 m_nSentLen  = 0;      // Bytes of number sent data through current socket

static u8  m_ipaddress[5];  //only save the number of server ip, remove the comma
static s32 m_socketid = -1;

static s32 m_remain_len = 0;     // record the remaining number of bytes in send buffer.
static char *m_pCurrentPos = NULL;

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
static u8 m_tcp_state = STATE_NW_GET_SIMSTATE;

static sProgrammData programmData =
{
	.firstInit 		= FALSE,
    .needReboot 	= FALSE,

    .rebootCnt 		= 0,
    .reconnectCnt 	= 0,
    .pingCnt		= 0,
    .buttonCnt 		= 0,
    .in1Cnt 		= 0,
    .in2Cnt 		= 0,

    .totalSeconds 	= 0,
    .buttonState 	= FALSE,
    .HbuttonState	= FALSE,
    .in1State		= FALSE,
    .Hin1State		= FALSE,
    .in2State		= FALSE,
    .Hin2State		= FALSE,
    .tempValue 		= 0.0,
    .autCnt			= 0,
    .rssi			= 0,
    .ber 			= 0
};
static sProgrammSettings programmSettings;

static s32 				led_cnt = 5;
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
//static void gsm_callback_onTimer(u32 timerId, void* param);
//static void gpio_callback_onTimer(u32 timerId, void* param);

/*****************************************************************
* init function declaration
******************************************************************/
static void InitWDT(s32 *wtdid);
static void InitFlash(void);
static void InitUART(void);
static void InitGPRS(void);
static void InitGPIO(void);
static void InitADC(void);
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

    // Register & open UART ports
    Ql_UART_Register(UART_PORT1, CallBack_UART_Hdlr, NULL);//main uart
    Ql_UART_Register(UART_PORT2, CallBack_UART_Hdlr, NULL);//debug uart
    Ql_UART_Open(UART_PORT1, 115200, FC_NONE);
    Ql_UART_Open(UART_PORT2, 115200, FC_NONE);

    APP_DEBUG("<--OpenCPU: Starting Application.-->\r\n");

    InitWDT(&wtdid);
    //InitGPRS();

    while (TRUE)
    {
        Ql_OS_GetMessage(&msg);
        switch(msg.message)
        {
            // Application will receive this message when OpenCPU RIL starts up.
            // Then application needs to call Ql_RIL_Initialize to launch the initialization of RIL.
        case MSG_ID_RIL_READY:
            APP_DEBUG("<-- RIL is ready -->\r\n");
            Ql_RIL_Initialize();
            //
            // After RIL initialization, developer may call RIL-related APIs in the .h files in the directory of SDK\ril\inc
            // RIL-related APIs may simplify programming, and quicken the development.
            //
            programmData.firstInit = TRUE;
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
                APP_DEBUG("<-- Other URC: type=%d\r\n", msg.param1);
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
   // ST_MSG subtask1_msg;
    s32 ret;
    s32 tmpSEC 		= 30;

    do
    {
    	Ql_Sleep(100);//in ms
    }
    while(programmData.firstInit == FALSE);

    ret = Ql_SleepDisable();
    InitFlash();
    //InitGPIO();
    //InitUART();
    //InitADC();


    APP_DEBUG("<-- subtask1: enter, Ql_SleepDisable ret=%d ->\r\n", ret);
    while (TRUE)
    {
        Ql_Sleep(1000);
        programmData.totalSeconds++;
        //APP_DEBUG("<-- subtask1: totalSeconds=%d -->\r\n", programmData.totalSeconds);
        if(tmpSEC++ >= 59)
        {
        	tmpSEC = 0;
    		if(GetLocalTime() == TRUE)
    		{

    		}
        }
    };
}

/*****************************************************************/
//Callback implementation
/*****************************************************************/
static void wdt_callback_onTimer(u32 timerId, void* param)
{
    s32* wtdid = (s32*)param;

    if(programmData.needReboot == FALSE)
    {
    	Ql_WTD_Feed(*wtdid);
        APP_DEBUG("<-- time to feed logic watchdog wtdId=%d -->\r\n",*wtdid);
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

/*****************************************************************/
//Init implementation
/*****************************************************************/
static void InitWDT(s32 *wtdid)
{
    s32 ret;

    APP_DEBUG("<-- InitWDT -->\r\n");
    // Initialize external watchdog: specify the GPIO pin (PINNAME_RI) and the overflow time is 600ms.
    ret = Ql_WTD_Init(0, PINNAME_RI, 600);//дергаем ногой почаще тк время сброса у tps-ки 1600мс
    if (0 == ret)
        APP_DEBUG("\r\n<--OpenCPU: watchdog init OK!-->\r\n");

    // Create a logic watchdog, the interval is 1.5 s
    *wtdid = Ql_WTD_Start(WTD_TMR_TIMEOUT);
    APP_DEBUG("<-- InitWDT wtdid=%d-->\r\n", *wtdid);


    // Register & start a timer to feed the logic watchdog.
    ret = Ql_Timer_Register(LOGIC_WTD1_TMR_ID, wdt_callback_onTimer, wtdid);
    if(ret < 0){
        APP_DEBUG("<--main task: register fail ret=%d-->\r\n", ret);
        return;
    }
    // The real feeding interval is 1 s
    ret = Ql_Timer_Start(LOGIC_WTD1_TMR_ID, 1000, TRUE);
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
        	APP_DEBUG("<-- init_flash OK crc=<%d> apn=<%s> user=<%s> pass=<%s> server=<%s> port=<%d> baudrate=<%d>-->\r\n",
        			programmSettings.crc,
        			programmSettings.gsmSettings.gprsApn,
        			programmSettings.gsmSettings.gprsUser,
        			programmSettings.gsmSettings.gprsPass,
        			programmSettings.ipSettings.dstAddress,
        			programmSettings.ipSettings.dstPort,
        			programmSettings.serPortSettings.baudrate);
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


/*****************************************************************/
//Other implementation
/*****************************************************************/
static bool GetLocalTime(void)
{
	bool ret = FALSE;

	APP_DEBUG("<--Try get Local time -->\r\n");
    if( Ql_GetLocalTime(&time) )
    {
        //This function get total seconds elapsed   since 1970.01.01 00:00:00.
        totalSeconds = Ql_Mktime(&time);
        APP_DEBUG("<--Local time successfuly determined: %i.%i.%i %i:%i:%i timezone=%i-->\r\n", time.day, time.month, time.year, time.hour, time.minute, time.second,time.timezone);

        if(totalSeconds > 0)
        	programmData.totalSeconds = totalSeconds;
        APP_DEBUG("<--totalSeconds=<%llu>-->\r\n", programmData.totalSeconds);
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
	APP_DEBUG("<-- Read data from port=%d, len=%d -->\r\n", port, len);
	pData[len] = 0;
	char tmp_buff[300] = {0};
	if(port == UART_PORT3)
	{
		//send it to server
		m_pCurrentPos = m_send_buf;
		//Ql_strcpy(m_pCurrentPos + m_remain_len, pData);
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

#endif // __GSM_TEMINAL__
