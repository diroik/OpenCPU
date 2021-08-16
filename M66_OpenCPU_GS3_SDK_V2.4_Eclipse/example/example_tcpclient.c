/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Quectel Co., Ltd. 2013
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   example_tcpclient.c
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   This example demonstrates how to establish a TCP connection, when the module 
 *   is used for the client. Input the specified command through any serial port 
 *   and the result will be output through the debug port.We have adopted a timeout 
 *   mechanism,if in the process of connecting socket or getting the TCP socket ACK 
 *   number overtime 90s, the socket will be close and the network will be deactivated.
 *   In most of TCPIP functions,  return -2(QL_SOC_WOULDBLOCK) doesn't indicate failed.
 *   It means app should wait, till the callback function is called.
 *   The app can get the information of success or failure in callback function.
 *   Get more info about return value. Please read the "OPEN_CPU_DGD" document.
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "C_PREDEF=-D __EXAMPLE_TCPCLIENT__" in gcc_makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
 * 
 *   Operation:
 *            
 *     step1: set APN parameter.
 *            Command: Set_APN_Param=<APN>,<username>,<password>
 *     step2: set server parameter, which is you want to connect.
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
#ifdef __EXAMPLE_TCPCLIENT__  
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
* define process state
******************************************************************/
#include "typedef.h"
#include "flash.h"

/***********************************************************************
 * SMS CONSTANT DEFINITIONS
************************************************************************/
#define CON_SMS_BUF_MAX_CNT   (1)
#define CON_SMS_SEG_MAX_CHAR  (160)
#define CON_SMS_SEG_MAX_BYTE  (4 * CON_SMS_SEG_MAX_CHAR)
#define CON_SMS_MAX_SEG       (7)

/***********************************************************************
 * SMS STRUCT TYPE DEFINITIONS
************************************************************************/
typedef struct
{
    u8 aData[CON_SMS_SEG_MAX_BYTE];
    u16 uLen;
} ConSMSSegStruct;

typedef struct
{
    u16 uMsgRef;
    u8 uMsgTot;

    ConSMSSegStruct asSeg[CON_SMS_MAX_SEG];
    bool abSegValid[CON_SMS_MAX_SEG];
} ConSMSStruct;
/***********************************************************************
 * SMS FUNCTION DECLARATIONS
************************************************************************/
static bool ConSMSBuf_IsIntact(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,u8 uIdx,ST_RIL_SMS_Con *pCon);
static bool ConSMSBuf_AddSeg(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,u8 uIdx,ST_RIL_SMS_Con *pCon,u8 *pData,u16 uLen);
static s8 ConSMSBuf_GetIndex(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,ST_RIL_SMS_Con *pCon);
static bool ConSMSBuf_ResetCtx(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,u8 uIdx);
static bool SMS_Initialize(void);
/***********************************************************************
 * SMS GLOBAL DATA DEFINITIONS
************************************************************************/
ConSMSStruct g_asConSMSBuf[CON_SMS_BUF_MAX_CNT];

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

#define TCP_TIMER_PERIOD     500 //800
#define TIMEOUT_90S_PERIOD   90000
#define CSQ_TIMER_PERIOD     60000
#define GPIO_INPUT_TIMER_PERIOD 100

static s32 timeout_90S_monitor = FALSE;
/*****************************************************************
* APN Param
******************************************************************/
//static u8 m_apn[MAX_GPRS_APN_LEN] = "internet";
//static u8 m_userid[MAX_GPRS_USER_NAME_LEN] = "";
//static u8 m_passwd[MAX_GPRS_PASSWORD_LEN] = "";

static ST_GprsConfig  m_gprsCfg;
/*****************************************************************
* Server Param
******************************************************************/
#define SRVADDR_BUFFER_LEN  100
#define SEND_BUFFER_LEN     2048
#define RECV_BUFFER_LEN     2048

static u8 m_send_buf[SEND_BUFFER_LEN];
static u8 m_recv_buf[RECV_BUFFER_LEN];
static u64 m_nSentLen  = 0;      // Bytes of number sent data through current socket    

//static u8  m_SrvADDR[SRVADDR_BUFFER_LEN] = "megafon.techmonitor.ru";//"94.228.255.152";
//static u32 m_SrvPort = 9003;

static u8  m_ipaddress[5];  //only save the number of server ip, remove the comma
static s32 m_socketid = -1; 

static s32 m_remain_len = 0;     // record the remaining number of bytes in send buffer.
static char *m_pCurrentPos = NULL; 

/*****************************************************************
* ADC Param
******************************************************************/
static u32 ADC_CustomParam = 1;
/*****************************************************************
* Other Param
******************************************************************/
static u8 m_tcp_state = STATE_NW_GET_SIMSTATE;
//static s32 ret;
//static u64 totalSeconds;
//static ST_Time time;
//static ST_Time* pTime = NULL;

static sProgrammData programmData =
{
	.firstInit 		= FALSE,
    .needReboot 	= FALSE,

    .rebootCnt 		= 0,
    .reconnectCnt 	= 0,
    .buttonCnt 		= 0,
    .in1Cnt 		= 0,
    .in2Cnt 		= 0,

    .buttonState 	= FALSE,
    .HbuttonState	= FALSE,
    .in1State		= FALSE,
    .Hin1State		= FALSE,
    .in2State		= FALSE,
    .Hin2State		= FALSE,
};

static sProgrammSettings programmSettings;


static s32 				led_cnt = 5;
static Enum_PinName  	led_pin = PINNAME_PCM_CLK;//30
static Enum_PinName  	button_pin = PINNAME_PCM_SYNC;//31
static Enum_PinName  	in1_pin = PINNAME_PCM_IN;//32
static Enum_PinName  	in2_pin = PINNAME_PCM_OUT;//33



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
* PowerKey callback function
******************************************************************/
static void Callback_PowerKey_Hdlr(s32 param1, s32 param2);
/*****************************************************************
* uart callback function
******************************************************************/
static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara);

/*****************************************************************
* timer callback function
******************************************************************/
static void gsm_callback_onTimer(u32 timerId, void* param);
static void wdt_callback_onTimer(u32 timerId, void* param);
static void gpio_callback_onTimer(u32 timerId, void* param);
/*****************************************************************
* ADC callback function
******************************************************************/
static void Callback_OnADCSampling(Enum_ADCPin adcPin, u32 adcValue, void *customParam);
/*****************************************************************
* other subroutines
******************************************************************/
static void InitFlash(void);
static void InitWDT(void);
static void InitUART(void);
static void InitGPRS(void);
static void InitGPIO(void);
static void InitADC(void);

//extern s32 Analyse_Command(u8* src_str,s32 symbol_num,u8 symbol, u8* dest_buf);
extern int clear_all_nulls(char *_ptr, int _size);

static s32 ReadSerialPort(Enum_SerialPort port, /*[out]*/u8* pBuffer, /*[in]*/u32 bufLen);
static void proc_handle(Enum_SerialPort port, u8 *pData,s32 len);
static void checkErr_AckNumber(s32 err_code);
static void Restart_GSM(void);
static void Hdlr_RecvNewSMS(u32 nIndex, bool bAutoReply);

static void reboot(void);

static char *Parse_Command(char *src_str, char *tmp_buff, sProgrammSettings *sett_in_ram, sProgrammData *programmData);
static char *Gsm_GetSignal(char *tmp_buff);
static char *set_cmd(char *cmdstr, char *tmp_buff, sProgrammSettings* sett_in_ram);
static char *get_cmd(char *cmd, char *tmp_buff, sProgrammSettings* sett_in_ram);
static s32 GetInputValue(Enum_PinName *pin, s32 *cnt, u32 max_timeout);

/*****************************************************************
* other system tasks
******************************************************************/
void proc_main_task(s32 taskId)
{
	s32 iResult = 0;
    ST_MSG msg;

    //Ql_PwrKey_Register(Callback_PowerKey_Hdlr);

    Ql_UART_Register(UART_PORT1, CallBack_UART_Hdlr, NULL);//main uart
    Ql_UART_Register(UART_PORT2, CallBack_UART_Hdlr, NULL);//debug uart
    Ql_UART_Open(UART_PORT1, 115200, FC_NONE);
    Ql_UART_Open(UART_PORT2, 115200, FC_NONE);

    Ql_Debug_Trace("<-- proc_main_task: enter ->\r\n");

    APP_DEBUG("<--OpenCPU: Starting Application.-->\r\n");
    

    InitGPRS();


    while(TRUE)
    {
        Ql_OS_GetMessage(&msg);
        //APP_DEBUG("<-- Ql_OS_GetMessage msg=%d-->\r\n", msg.message);
        switch(msg.message)
        {
#ifdef __OCPU_RIL_SUPPORT__
        case MSG_ID_RIL_READY:
            APP_DEBUG("<-- RIL is ready -->\r\n");
            Ql_RIL_Initialize();

            programmData.firstInit = TRUE;
            break;
#endif
        //
            // Handle URC messages.
            // URC messages include "module init state", "CFUN state", "SIM card state(change)",
            // "GSM network state(change)", "GPRS network state(change)" and other user customized URC.
        case MSG_ID_URC_INDICATION:
            switch (msg.param1)
            {
                // URC for module initialization state
            case URC_SYS_INIT_STATE_IND:
                APP_DEBUG("<-- SysInit Status %d -->\r\n", msg.param2);
                if (SYS_STATE_SMSOK == msg.param2)
                {
                    // SMS option has been initialized, and application can program SMS
                    //APP_DEBUG("<-- Application can program SMS -->\r\n");

                    APP_DEBUG("\r\n<-- SMS module is ready -->\r\n");
                    APP_DEBUG("\r\n<-- Initialize SMS-related options -->\r\n");
                    iResult = SMS_Initialize();
                    if (!iResult) APP_DEBUG("Fail to initialize SMS\r\n");
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
                Hdlr_RecvNewSMS((msg.param2), TRUE);// FALSE);
                break;

            case URC_MODULE_VOLTAGE_IND:
                APP_DEBUG("\r\n<-- VBatt Voltage Ind: type=%d\r\n", msg.param2);
                break;

            default:
                APP_DEBUG("<-- Other URC: type=%d\r\n", msg.param1);
                break;
            }
            break;
        //
        default:
            break;
        }
    }
}

void proc_subtask1(s32 TaskId)
{
    ST_MSG subtask1_msg;
    //s32 wtdid;
    //s32 ret;

    //Ql_Debug_Trace("<-- subtask1: enter ->\r\n");

    do
    {
    	Ql_Sleep(100);//in ms
    }
    while(programmData.firstInit == FALSE);

    APP_DEBUG("<-- subtask1: starting -->\r\n");

    InitWDT();//mast be first

    InitFlash();
    InitUART();
    InitGPIO();
    InitADC();

    while (TRUE)
    {
        Ql_OS_GetMessage(&subtask1_msg);
        switch (subtask1_msg.message)
        {
            default:
                break;
        }
    }
}
/*****************************************************************/
//Callback definition
static void wdt_callback_onTimer(u32 timerId, void* param)
{
    s32* wtdid = (s32*)param;
    Ql_Debug_Trace("<-- time to feed logic watchdog wtdid=%d-->\r\n", *wtdid);
    if(programmData.needReboot == FALSE)
    {
    	Ql_WTD_Feed(*wtdid);
    }
    else
    {
    	u32 cnt = WTD_TMR_TIMEOUT*2/100 + 1;
    	Ql_Debug_Trace("<-- time to not feed logic watchdog (wtdId=%d) needReboot=(%s) cnt=(%d)-->\r\n", *wtdid, programmData.needReboot == TRUE ? "TRUE" : "FALSE", cnt);
    	//Ql_PowerDown(1);
    	//Ql_Debug_Trace("<-- wdt_callback_onTimer Ql_PowerDown successful-->\r\n");
    	do{
    		Ql_GPIO_SetLevel(led_pin, Ql_GPIO_GetLevel(led_pin) == PINLEVEL_HIGH ? PINLEVEL_LOW : PINLEVEL_HIGH);
    		Ql_Sleep(100);
    	}
    	while(cnt--);

    	Ql_Debug_Trace("<-- wdt_callback_onTimer wait real wdt reboot successfull, try Ql_Reset-->\r\n");
    	Ql_Reset(0);

    	while(1);
    }
}

void Callback_PowerKey_Hdlr(s32 param1, s32 param2)
{
	Ql_Debug_Trace("<-- Power Key: %s, %s -->\r\n", param1==POWER_OFF ? "Power Off":"Power On",param2==KEY_DOWN ? "Key Down":"Key Up");
	if (POWER_ON == param1){
		Ql_Debug_Trace("<-- App Lock Power Key! -->\r\n");
		//Ql_LockPower();
	}
	else if (POWER_OFF == param1)
	{
		Ql_Debug_Trace("<-- Ql_PowerDown! -->\r\n");
		//Post processing before power down
		//...
		//Power down
		//Ql_PowerDown(1);
	}
}

static void gpio_callback_onTimer(u32 timerId, void* param)
{
	if(programmData.firstInit == FALSE) return;
	if (GPIO_TIMER_ID == timerId)
	{
		//APP_DEBUG("<-- Get the button_pin GPIO level value: %d -->\r\n", Ql_GPIO_GetLevel(button_pin));
		s32 btp = GetInputValue(&button_pin, 	&programmData.buttonCnt,(u32)(programmSettings.buttonTimeout * 	1000/GPIO_INPUT_TIMER_PERIOD));
		s32 i1p = GetInputValue(&in1_pin, 		&programmData.in1Cnt, 	(u32)(programmSettings.in1Timeout * 	1000/GPIO_INPUT_TIMER_PERIOD));
		s32 i2p = GetInputValue(&in2_pin, 		&programmData.in2Cnt, 	(u32)(programmSettings.in2Timeout * 	1000/GPIO_INPUT_TIMER_PERIOD));

		if(btp >= 0) programmData.buttonState = (bool)btp;
		if(programmData.HbuttonState != programmData.buttonState){//is changed
			programmData.HbuttonState = programmData.buttonState;
			APP_DEBUG("<-- Get the button_pin GPIO level value changed: %d -->\r\n", programmData.buttonState);
			if(programmData.buttonState == FALSE){
				APP_DEBUG("<-- try restore default by user press button!!!-->\r\n");
				bool ret =  restore_default_flash(&programmSettings);
				APP_DEBUG("<-- restore_default_flash ret=%d -->\r\n", ret);
				reboot();
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
		//led_cnt
	    if(led_cnt-- < 0)
	    {
	      led_cnt = 10;
	      if(programmData.reconnectCnt++ > programmSettings.secondsToReconnect)
	      {
	        programmData.reconnectCnt = 0;
	        //gsmState.need_tcp_connect = TRUE;

	        APP_DEBUG("<--Socket reconnect timeout, need_tcp_connect socketId=%d -->\r\n", m_socketid);
	        Ql_SOC_Close(m_socketid);
	        m_socketid = -1;
	        m_tcp_state = m_tcp_state = STATE_SOC_CREATE;//STATE_GPRS_DEACTIVATE
	      }
	      if(programmData.rebootCnt++ > programmSettings.secondsToReboot)
	      {
	        programmData.rebootCnt  = 0;
	        programmData.needReboot = TRUE;
	      }
	      Ql_GPIO_SetLevel(led_pin, Ql_GPIO_GetLevel(led_pin) == PINLEVEL_HIGH ? PINLEVEL_LOW : PINLEVEL_HIGH);
	    }

	    /*
	    if(led_cnt < 3){
	    	Ql_GPIO_SetLevel(led_pin, PINLEVEL_HIGH);
	    }
	    else{
	    	 Ql_GPIO_SetLevel(led_pin, PINLEVEL_LOW);
	    }*/

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
static void gsm_callback_onTimer(u32 timerId, void* param)
{
	s32 ret;

	if(programmData.firstInit == FALSE) return;

    if (TIMEOUT_90S_TIMER_ID == timerId)
    {
        APP_DEBUG("<--90s time out!!!-->\r\n");
        APP_DEBUG("<-- Close socket.-->\r\n");
        
        Ql_SOC_Close(m_socketid);
        m_socketid = -1;

        m_tcp_state = STATE_GPRS_DEACTIVATE;

        timeout_90S_monitor = FALSE;
    }
    else if (CSQ_TIMER_ID == timerId)
    {
    	//APP_DEBUG("<--...........CSQ_TIMER_ID=%d..................-->\r\n", timerId);

        u32 rssi;
        u32 ber;
        RIL_NW_GetSignalQuality(&rssi, &ber);

        u64 totalMS;
        totalMS = Ql_GetMsSincePwrOn();
        APP_DEBUG("uptime: %lld ms, signal strength: %d, BER: %d\r\n", totalMS, rssi, ber);
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
                }else
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
                     APP_DEBUG("<--Waiting for the result of GPRS activated.,ret=%d.-->\r\n",ret);
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
                    
                }else  //domain name
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
                        APP_DEBUG("<--Get ip by hostname failure:ret=%d-->\r\n",ret);
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
                }
                else
                {
                    APP_DEBUG("<--Create socket id failure, error=%d.-->\r\n",m_socketid);
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
                    
                }else if(ret == SOC_WOULDBLOCK)
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
                    APP_DEBUG("<-- Close socket.-->\r\n");
                    Ql_SOC_Close(m_socketid);
                    m_socketid = -1;
                    
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
                    ret = Ql_SOC_Send(m_socketid, m_pCurrentPos, m_remain_len);
                    APP_DEBUG("<--Send data,socketid=%d,number of bytes sent=%d-->\r\n",m_socketid,ret);
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
                        //waiting CallBack_socket_write, then send data;     
                        break;
                    }
                    else if(ret <= 0)
                    {
                        APP_DEBUG("<--Send data failure,ret=%d.-->\r\n",ret);
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
                }while(1);
                break;
            }
            case STATE_SOC_ACK:
            {
                u64 ackedNumCurr;
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

                    Ql_memset(m_send_buf,0,SEND_BUFFER_LEN);
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
    
    APP_DEBUG("<-- %s:contexid=%d, requestId=%d,error=%d,num_entry=%d -->\r\n", __func__, contexId, requestId,errCode,ipAddrCnt);
    if (errCode == SOC_SUCCESS)
    {
        APP_DEBUG("<--CallBack: get ip by name successfully.-->\r\n");
        for(i=0;i<ipAddrCnt;i++)
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

        APP_DEBUG("<--Callback: socket connect successfully.-->\r\n");
        m_tcp_state = STATE_SOC_SEND;
    }else
    {
        APP_DEBUG("<--Callback: socket connect failure,(socketId=%d),errCode=%d-->\r\n",socketId,errCode);
        Ql_SOC_Close(socketId);
        m_tcp_state = STATE_SOC_CREATE;
    }
}

void callback_socket_close(s32 socketId, s32 errCode, void* customParam )
{
    m_nSentLen  = 0;
    
    if (errCode == SOC_SUCCESS)
    {
        APP_DEBUG("<--CallBack: close socket successfully.-->\r\n"); 
    }else if(errCode == SOC_BEARER_FAIL)
    {   
        m_tcp_state = STATE_GPRS_DEACTIVATE;
        APP_DEBUG("<--CallBack: close socket failure,(socketId=%d,error_cause=%d)-->\r\n",socketId,errCode); 
    }else
    {
        m_tcp_state = STATE_GPRS_GET_DNSADDRESS;
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
            APP_DEBUG("<-- Receive data failure,ret=%d.-->\r\n",ret);
            APP_DEBUG("<-- Close socket.-->\r\n");
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
        else if(ret < RECV_BUFFER_LEN)
        {
            APP_DEBUG("<--Receive data from sock(%d), len(%d):%s and write to UART_PORT3\r\n", socketId, ret, m_recv_buf);
            Ql_UART_Write(UART_PORT3, m_recv_buf, ret);

            //read finish, wait next CallBack_socket_read
            break;
        }
        else if(ret == RECV_BUFFER_LEN)
        {
            APP_DEBUG("<--Receive full buffer data from sock(%d), len(%d):%s and write to UART_PORT3\r\n", socketId, ret, m_recv_buf);
            Ql_UART_Write(UART_PORT3, m_recv_buf, ret);

            //read continue
        }
    }while(1);
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
        APP_DEBUG("<--CallBack: Send data,socketid=%d,number of bytes sent=%d-->\r\n",m_socketid,ret);

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
              APP_DEBUG("<--Send data failure, ret=%d.-->\r\n",ret);
              APP_DEBUG("<-- Close socket.-->\r\n");
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
        reboot();
    }
}

static void Callback_OnADCSampling(Enum_ADCPin adcPin, u32 adcValue, void *customParam)
{

	s32 index = *((s32*)customParam);
	if(index % 30 == 0)
	{
		//APP_DEBUG( "<-- Callback_OnADCSampling: sampling voltage(mV)=%d  times=%d -->\r\n", adcValue, *((s32*)customParam) );
		u32 capacity, voltage;
		s32 ret = RIL_GetPowerSupply(&capacity, &voltage);
		if(ret == QL_RET_OK)
		{
			APP_DEBUG( "<-- PowerSupply: capacity(percent)=%d  voltage(mV)=%d -->\r\n", capacity, voltage);
		}
	}

    *((s32*)customParam) += 1;
}

/*****************************************************************
* other functions
******************************************************************/
static void InitFlash(void)
{
	APP_DEBUG("<-- OpenCPU: init_flash! Size=%d-->\r\n", sizeof(sProgrammSettings));

    bool ret = FALSE;
    for(int i=0; i < 5; i++)
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
    reboot();
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
    //APP_DEBUG("<-- InitWDT end -->\r\n");
    Ql_Sleep(300);
}

static void InitUART(void)
{
    // Register & open UART port
	Ql_Debug_Trace("<-- InitUART -->\r\n");

	s32 ret;
    ST_UARTDCB dcb;

    dcb.baudrate = programmSettings.serPortSettings.baudrate;
    dcb.dataBits = programmSettings.serPortSettings.dataBits;
    dcb.stopBits = programmSettings.serPortSettings.stopBits;
    dcb.parity   = programmSettings.serPortSettings.parity;
    dcb.flowCtrl = programmSettings.serPortSettings.flowCtrl;

    ret = Ql_UART_Register(UART_PORT3, CallBack_UART_Hdlr, NULL);
    if (ret < QL_RET_OK)
        Ql_Debug_Trace("<--Ql_UART_Register(mySerialPort=%d)=%d-->\r\n", UART_PORT3, ret);

    ret = Ql_UART_OpenEx(UART_PORT3, &dcb);
    if (ret < QL_RET_OK)
        Ql_Debug_Trace("<--Ql_UART_OpenEx(mySerialPort=%d)=%d-->\r\n", UART_PORT3, ret);

    //Ql_Debug_Trace("<-- InitUART end -->\r\n");
}

static void InitGPRS(void)
{
    //register & start timer
	Ql_Debug_Trace("<-- InitGPRS -->\r\n");

    Ql_Timer_Register(TCP_TIMER_ID, gsm_callback_onTimer, NULL);
    Ql_Timer_Start(TCP_TIMER_ID, TCP_TIMER_PERIOD, TRUE);

    Ql_Timer_Register(TIMEOUT_90S_TIMER_ID, gsm_callback_onTimer, NULL);
    timeout_90S_monitor = FALSE;

    Ql_Timer_Register(CSQ_TIMER_ID, gsm_callback_onTimer, NULL);
    Ql_Timer_Start(CSQ_TIMER_ID, CSQ_TIMER_PERIOD, TRUE);

    //Ql_Debug_Trace("<-- InitGPRS end -->\r\n");
}

static void InitGPIO(void)
{
	Ql_Debug_Trace("<-- InitGPIO -->\r\n");

    Ql_GPIO_Init(led_pin, 		PINDIRECTION_OUT, 	PINLEVEL_HIGH, 	PINPULLSEL_PULLUP);
    Ql_GPIO_Init(button_pin, 	PINDIRECTION_IN, 	PINLEVEL_HIGH, 	PINPULLSEL_DISABLE);
    Ql_GPIO_Init(in1_pin, 		PINDIRECTION_IN, 	PINLEVEL_HIGH, 	PINPULLSEL_DISABLE);
    Ql_GPIO_Init(in2_pin, 		PINDIRECTION_IN, 	PINLEVEL_HIGH, 	PINPULLSEL_DISABLE);

    Ql_Timer_Register(GPIO_TIMER_ID, gpio_callback_onTimer, NULL);
    Ql_Timer_Start(GPIO_TIMER_ID, GPIO_INPUT_TIMER_PERIOD, TRUE);

    Ql_Timer_Register(LED_TIMER_ID, gpio_callback_onTimer, NULL);
    Ql_Timer_Start(LED_TIMER_ID, 100, TRUE);


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


static void Restart_GSM(void)
{
    APP_DEBUG("<--Deactivate GPRS.-->\r\n");
    s32 ret;
    ret = Ql_GPRS_DeactivateEx(0, TRUE);
    if (GPRS_PDP_SUCCESS == ret){
    	APP_DEBUG("<--GPRS is deactivated successfully.-->\r\n");
    	m_tcp_state = STATE_NW_GET_SIMSTATE;
    }else{
    	APP_DEBUG("<--Fail to activate GPRS, error code is in %d.-->\r\n", ret);
    	reboot();
    }
}

static s32 ReadSerialPort(Enum_SerialPort port, /*[out]*/u8* pBuffer, /*[in]*/u32 bufLen)
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
        // Continue to read...
    }
    if (rdLen < 0) // Serial Port Error!
    {
        APP_DEBUG("<--Fail to read from port[%d]-->\r\n", port);
        return -99;
    }
    return rdTotalLen;
}

static void proc_handle(Enum_SerialPort port, u8 *pData,s32 len)
{
	APP_DEBUG("Read data from port=%d, len=%d\r\n", port, len);
	pData[len] = 0;
	if(port == UART_PORT3)
	{
		//send it to server
		m_pCurrentPos = m_send_buf;
		Ql_strcpy(m_pCurrentPos + m_remain_len, pData);
		m_remain_len+=len;
	}
	else
	{
	    char tmp_buff[150] = {0};
		char *answer = Parse_Command(pData, tmp_buff, &programmSettings, &programmData);


		if( answer != NULL)
		{
			//u32 alen = Ql_strlen(answer);
			APP_DEBUG("%s", answer);
		}
		else
		{
			//if not command, send it to server
			m_pCurrentPos = m_send_buf;
			Ql_strcpy(m_pCurrentPos + m_remain_len, pData);
			m_remain_len+=len;
		}
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

/*****************************************************************************
 * FUNCTION
 *  Hdlr_RecvNewSMS
 *
 * DESCRIPTION
 *  The handler function of new received SMS.
 *
 * PARAMETERS
 *  <nIndex>     The SMS index in storage,it starts from 1
 *  <bAutoReply> TRUE: The module should reply a SMS to the sender;
 *               FALSE: The module only read this SMS.
 *
 * RETURNS
 *  VOID
 *
 * NOTE
 *  1. This is an internal function
 *****************************************************************************/
static void Hdlr_RecvNewSMS(u32 nIndex, bool bAutoReply)
{
    s32 iResult = 0;
    u32 uMsgRef = 0;
    ST_RIL_SMS_TextInfo *pTextInfo = NULL;
    ST_RIL_SMS_DeliverParam *pDeliverTextInfo = NULL;
    char aPhNum[RIL_SMS_PHONE_NUMBER_MAX_LEN] = {0,};
    //const char aReplyCon[] = {"Module has received SMS."};
    bool bResult = FALSE;


    pTextInfo = Ql_MEM_Alloc(sizeof(ST_RIL_SMS_TextInfo));
    if (NULL == pTextInfo)
    {
        APP_DEBUG("%s/%d:Ql_MEM_Alloc FAIL! size:%u\r\n", sizeof(ST_RIL_SMS_TextInfo), __func__, __LINE__);
        return;
    }
    Ql_memset(pTextInfo, 0x00, sizeof(ST_RIL_SMS_TextInfo));
    iResult = RIL_SMS_ReadSMS_Text(nIndex, LIB_SMS_CHARSET_GSM, pTextInfo);
    if (iResult != RIL_AT_SUCCESS)
    {
        Ql_MEM_Free(pTextInfo);
        APP_DEBUG("Fail to read text SMS[%d], cause:%d\r\n", nIndex, iResult);
        return;
    }

    if ((LIB_SMS_PDU_TYPE_DELIVER != (pTextInfo->type)) || (RIL_SMS_STATUS_TYPE_INVALID == (pTextInfo->status)))
    {
        Ql_MEM_Free(pTextInfo);
        APP_DEBUG("WARNING: NOT a new received SMS.\r\n");
        return;
    }

    pDeliverTextInfo = &((pTextInfo->param).deliverParam);

    if(TRUE == pDeliverTextInfo->conPres)  //Receive CON-SMS segment
    {
        s8 iBufIdx = 0;
        u8 uSeg = 0;
        u16 uConLen = 0;

        iBufIdx = ConSMSBuf_GetIndex(g_asConSMSBuf,CON_SMS_BUF_MAX_CNT,&(pDeliverTextInfo->con));
        if(-1 == iBufIdx)
        {
            APP_DEBUG("Enter Hdlr_RecvNewSMS,WARNING! ConSMSBuf_GetIndex FAIL! Show this CON-SMS-SEG directly!\r\n");

            APP_DEBUG(
                "status:%u,type:%u,alpha:%u,sca:%s,oa:%s,scts:%s,data length:%u,cp:1,cy:%d,cr:%d,ct:%d,cs:%d\r\n",
                    (pTextInfo->status),
                    (pTextInfo->type),
                    (pDeliverTextInfo->alpha),
                    (pTextInfo->sca),
                    (pDeliverTextInfo->oa),
                    (pDeliverTextInfo->scts),
                    (pDeliverTextInfo->length),
                    pDeliverTextInfo->con.msgType,
                    pDeliverTextInfo->con.msgRef,
                    pDeliverTextInfo->con.msgTot,
                    pDeliverTextInfo->con.msgSeg
            );
            APP_DEBUG("data = %s\r\n",(pDeliverTextInfo->data));

            Ql_MEM_Free(pTextInfo);

            return;
        }

        bResult = ConSMSBuf_AddSeg(
                    g_asConSMSBuf,
                    CON_SMS_BUF_MAX_CNT,
                    iBufIdx,
                    &(pDeliverTextInfo->con),
                    (pDeliverTextInfo->data),
                    (pDeliverTextInfo->length)
        );
        if(FALSE == bResult)
        {
            APP_DEBUG("Enter Hdlr_RecvNewSMS,WARNING! ConSMSBuf_AddSeg FAIL! Show this CON-SMS-SEG directly!\r\n");

            APP_DEBUG(
                "status:%u,type:%u,alpha:%u,sca:%s,oa:%s,scts:%s,data length:%u,cp:1,cy:%d,cr:%d,ct:%d,cs:%d\r\n",
                (pTextInfo->status),
                (pTextInfo->type),
                (pDeliverTextInfo->alpha),
                (pTextInfo->sca),
                (pDeliverTextInfo->oa),
                (pDeliverTextInfo->scts),
                (pDeliverTextInfo->length),
                pDeliverTextInfo->con.msgType,
                pDeliverTextInfo->con.msgRef,
                pDeliverTextInfo->con.msgTot,
                pDeliverTextInfo->con.msgSeg
            );
            APP_DEBUG("data = %s\r\n",(pDeliverTextInfo->data));

            Ql_MEM_Free(pTextInfo);

            return;
        }

        bResult = ConSMSBuf_IsIntact(
                    g_asConSMSBuf,
                    CON_SMS_BUF_MAX_CNT,
                    iBufIdx,
                    &(pDeliverTextInfo->con)
        );
        if(FALSE == bResult)
        {
            APP_DEBUG(
                "Enter Hdlr_RecvNewSMS,WARNING! ConSMSBuf_IsIntact FAIL! Waiting. cp:1,cy:%d,cr:%d,ct:%d,cs:%d\r\n",
                pDeliverTextInfo->con.msgType,
                pDeliverTextInfo->con.msgRef,
                pDeliverTextInfo->con.msgTot,
                pDeliverTextInfo->con.msgSeg
            );

            Ql_MEM_Free(pTextInfo);

            return;
        }

        //Show the CON-SMS
        APP_DEBUG(
            "status:%u,type:%u,alpha:%u,sca:%s,oa:%s,scts:%s",
            (pTextInfo->status),
            (pTextInfo->type),
            (pDeliverTextInfo->alpha),
            (pTextInfo->sca),
            (pDeliverTextInfo->oa),
            (pDeliverTextInfo->scts)
        );

        uConLen = 0;
        for(uSeg = 1; uSeg <= pDeliverTextInfo->con.msgTot; uSeg++)
        {
            uConLen += g_asConSMSBuf[iBufIdx].asSeg[uSeg-1].uLen;
        }

        APP_DEBUG(",data length:%u",uConLen);
        APP_DEBUG("\r\n"); //Print CR LF

        for(uSeg = 1; uSeg <= pDeliverTextInfo->con.msgTot; uSeg++)
        {
            APP_DEBUG("data = %s ,len = %d",
                g_asConSMSBuf[iBufIdx].asSeg[uSeg-1].aData,
                g_asConSMSBuf[iBufIdx].asSeg[uSeg-1].uLen
            );
        }

        APP_DEBUG("\r\n"); //Print CR LF

        //Reset CON-SMS context
        bResult = ConSMSBuf_ResetCtx(g_asConSMSBuf,CON_SMS_BUF_MAX_CNT,iBufIdx);
        if(FALSE == bResult)
        {
            APP_DEBUG("Enter Hdlr_RecvNewSMS,WARNING! ConSMSBuf_ResetCtx FAIL! iBufIdx:%d\r\n",iBufIdx);
        }

        Ql_MEM_Free(pTextInfo);

        return;
    }

    APP_DEBUG("<-- RIL_SMS_ReadSMS_Text OK. eCharSet:LIB_SMS_CHARSET_GSM,nIndex:%u -->\r\n",nIndex);
    APP_DEBUG("SMS status:%u,type:%u,alpha:%u,sca:%s,oa:%s,scts:%s,data length:%u\r\n",
        pTextInfo->status,
        pTextInfo->type,
        pDeliverTextInfo->alpha,
        pTextInfo->sca,
        pDeliverTextInfo->oa,
        pDeliverTextInfo->scts,
        pDeliverTextInfo->length);
    APP_DEBUG("SMS data=<%s>\r\n",(pDeliverTextInfo->data));

    Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
    //Ql_MEM_Free(pTextInfo);

    if (bAutoReply)
    {
        if (!Ql_strstr(aPhNum, "10086"))  // Not reply SMS from operator
        {
        	char tmp_buff[150] = {0};
        	char *data = pDeliverTextInfo->data;
        	char *answer = Parse_Command(data, tmp_buff, &programmSettings,  &programmData);

        	if( answer != NULL && Ql_strlen(answer) > 0 )
        	{
				APP_DEBUG("<-- Replying SMS... -->\r\n");
				iResult = RIL_SMS_SendSMS_Text(aPhNum, Ql_strlen(aPhNum), LIB_SMS_CHARSET_GSM, (u8*)answer, Ql_strlen(answer),&uMsgRef);
				if (iResult != RIL_AT_SUCCESS)
				{
					APP_DEBUG("RIL_SMS_SendSMS_Text FAIL! iResult:%u\r\n",iResult);
					return;
				}
				APP_DEBUG("<-- RIL_SMS_SendTextSMS OK. uMsgRef:%d -->\r\n", uMsgRef);
        	}
        }
    }
    Ql_MEM_Free(pTextInfo);

    return;
}
//SMS
/*****************************************************************************
 * FUNCTION
 *  ConSMSBuf_IsIntact
 *
 * DESCRIPTION
 *  This function is used to check the CON-SMS is intact or not
 *
 * PARAMETERS
 *  <pCSBuf>     The SMS index in storage,it starts from 1
 *  <uCSMaxCnt>  TRUE: The module should reply a SMS to the sender; FALSE: The module only read this SMS.
 *  <uIdx>       Index of <pCSBuf> which will be stored
 *  <pCon>       The pointer of 'ST_RIL_SMS_Con' data
 *
 * RETURNS
 *  FALSE:   FAIL!
 *  TRUE: SUCCESS.
 *
 * NOTE
 *  1. This is an internal function
 *****************************************************************************/
static bool ConSMSBuf_IsIntact(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,u8 uIdx,ST_RIL_SMS_Con *pCon)
{
    u8 uSeg = 1;

    if(    (NULL == pCSBuf)
        || (0 == uCSMaxCnt)
        || (uIdx >= uCSMaxCnt)
        || (NULL == pCon)
      )
    {
        APP_DEBUG("Enter ConSMSBuf_IsIntact,FAIL! Parameter is INVALID. pCSBuf:%x,uCSMaxCnt:%d,uIdx:%d,pCon:%x\r\n",pCSBuf,uCSMaxCnt,uIdx,pCon);
        return FALSE;
    }

    if((pCon->msgTot) > CON_SMS_MAX_SEG)
    {
        APP_DEBUG("Enter ConSMSBuf_GetIndex,FAIL! msgTot:%d is larger than limit:%d\r\n",pCon->msgTot,CON_SMS_MAX_SEG);
        return FALSE;
    }

	for (uSeg = 1; uSeg <= (pCon->msgTot); uSeg++)
	{
        if(FALSE == pCSBuf[uIdx].abSegValid[uSeg-1])
        {
            APP_DEBUG("Enter ConSMSBuf_IsIntact,FAIL! uSeg:%d has not received!\r\n",uSeg);
            return FALSE;
        }
	}

    return TRUE;
}
/*****************************************************************************
 * FUNCTION
 *  ConSMSBuf_AddSeg
 *
 * DESCRIPTION
 *  This function is used to add segment in <pCSBuf>
 *
 * PARAMETERS
 *  <pCSBuf>     The SMS index in storage,it starts from 1
 *  <uCSMaxCnt>  TRUE: The module should reply a SMS to the sender; FALSE: The module only read this SMS.
 *  <uIdx>       Index of <pCSBuf> which will be stored
 *  <pCon>       The pointer of 'ST_RIL_SMS_Con' data
 *  <pData>      The pointer of CON-SMS-SEG data
 *  <uLen>       The length of CON-SMS-SEG data
 *
 * RETURNS
 *  FALSE:   FAIL!
 *  TRUE: SUCCESS.
 *
 * NOTE
 *  1. This is an internal function
 *****************************************************************************/
static bool ConSMSBuf_AddSeg(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,u8 uIdx,ST_RIL_SMS_Con *pCon,u8 *pData,u16 uLen)
{
    u8 uSeg = 1;

    if(    (NULL == pCSBuf) || (0 == uCSMaxCnt)
        || (uIdx >= uCSMaxCnt)
        || (NULL == pCon)
        || (NULL == pData)
        || (uLen > (CON_SMS_SEG_MAX_CHAR * 4))
      )
    {
        APP_DEBUG("Enter ConSMSBuf_AddSeg,FAIL! Parameter is INVALID. pCSBuf:%x,uCSMaxCnt:%d,uIdx:%d,pCon:%x,pData:%x,uLen:%d\r\n",pCSBuf,uCSMaxCnt,uIdx,pCon,pData,uLen);
        return FALSE;
    }

    if((pCon->msgTot) > CON_SMS_MAX_SEG)
    {
        APP_DEBUG("Enter ConSMSBuf_GetIndex,FAIL! msgTot:%d is larger than limit:%d\r\n",pCon->msgTot,CON_SMS_MAX_SEG);
        return FALSE;
    }

    uSeg = pCon->msgSeg;
    pCSBuf[uIdx].abSegValid[uSeg-1] = TRUE;
    Ql_memcpy(pCSBuf[uIdx].asSeg[uSeg-1].aData,pData,uLen);
    pCSBuf[uIdx].asSeg[uSeg-1].uLen = uLen;

	return TRUE;
}
/*****************************************************************************
 * FUNCTION
 *  ConSMSBuf_GetIndex
 *
 * DESCRIPTION
 *  This function is used to get available index in <pCSBuf>
 *
 * PARAMETERS
 *  <pCSBuf>     The SMS index in storage,it starts from 1
 *  <uCSMaxCnt>  TRUE: The module should reply a SMS to the sender; FALSE: The module only read this SMS.
 *  <pCon>       The pointer of 'ST_RIL_SMS_Con' data
 *
 * RETURNS
 *  -1:   FAIL! Can not get available index
 *  OTHER VALUES: SUCCESS.
 *
 * NOTE
 *  1. This is an internal function
 *****************************************************************************/
static s8 ConSMSBuf_GetIndex(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,ST_RIL_SMS_Con *pCon)
{
	u8 uIdx = 0;

    if(    (NULL == pCSBuf) || (0 == uCSMaxCnt)
        || (NULL == pCon)
      )
    {
        APP_DEBUG("Enter ConSMSBuf_GetIndex,FAIL! Parameter is INVALID. pCSBuf:%x,uCSMaxCnt:%d,pCon:%x\r\n",pCSBuf,uCSMaxCnt,pCon);
        return -1;
    }

    if((pCon->msgTot) > CON_SMS_MAX_SEG)
    {
        APP_DEBUG("Enter ConSMSBuf_GetIndex,FAIL! msgTot:%d is larger than limit:%d\r\n",pCon->msgTot,CON_SMS_MAX_SEG);
        return -1;
    }

	for(uIdx = 0; uIdx < uCSMaxCnt; uIdx++)  //Match all exist records
	{
        if(    (pCon->msgRef == pCSBuf[uIdx].uMsgRef)
            && (pCon->msgTot == pCSBuf[uIdx].uMsgTot)
          )
        {
            return uIdx;
        }
	}

	for (uIdx = 0; uIdx < uCSMaxCnt; uIdx++)
	{
		if (0 == pCSBuf[uIdx].uMsgTot)  //Find the first unused record
		{
            pCSBuf[uIdx].uMsgTot = pCon->msgTot;
            pCSBuf[uIdx].uMsgRef = pCon->msgRef;

			return uIdx;
		}
	}

    APP_DEBUG("Enter ConSMSBuf_GetIndex,FAIL! No avail index in ConSMSBuf,uCSMaxCnt:%d\r\n",uCSMaxCnt);

	return -1;
}
/*****************************************************************************
 * FUNCTION
 *  ConSMSBuf_ResetCtx
 *
 * DESCRIPTION
 *  This function is used to reset ConSMSBuf context
 *
 * PARAMETERS
 *  <pCSBuf>     The SMS index in storage,it starts from 1
 *  <uCSMaxCnt>  TRUE: The module should reply a SMS to the sender; FALSE: The module only read this SMS.
 *  <uIdx>       Index of <pCSBuf> which will be stored
 *
 * RETURNS
 *  FALSE:   FAIL!
 *  TRUE: SUCCESS.
 *
 * NOTE
 *  1. This is an internal function
 *****************************************************************************/
static bool ConSMSBuf_ResetCtx(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,u8 uIdx)
{
    if(    (NULL == pCSBuf) || (0 == uCSMaxCnt)
        || (uIdx >= uCSMaxCnt)
      )
    {
        APP_DEBUG("Enter ConSMSBuf_ResetCtx,FAIL! Parameter is INVALID. pCSBuf:%x,uCSMaxCnt:%d,uIdx:%d\r\n",pCSBuf,uCSMaxCnt,uIdx);
        return FALSE;
    }

    //Default reset
    Ql_memset(&pCSBuf[uIdx],0x00,sizeof(ConSMSStruct));

    //TODO: Add special reset here

    return TRUE;
}
/*****************************************************************************
 * FUNCTION
 *  SMS_Initialize
 *
 * DESCRIPTION
 *  Initialize SMS environment.
 *
 * PARAMETERS
 *  VOID
 *
 * RETURNS
 *  TRUE:  This function works SUCCESS.
 *  FALSE: This function works FAIL!
 *****************************************************************************/
static bool SMS_Initialize(void)
{
    s32 iResult = 0;
    //u8  nCurrStorage = 0;
    //u32 nUsed = 0;
    //u32 nTotal = 0;

    // Set SMS storage:
    // By default, short message is stored into SIM card. You can change the storage to ME if needed, or
    // you can do it again to make sure the short message storage is SIM card.
    #if 0
    {
        iResult = RIL_SMS_SetStorage(RIL_SMS_STORAGE_TYPE_SM,&nUsed,&nTotal);
        if (RIL_ATRSP_SUCCESS != iResult)
        {
            APP_DEBUG("Fail to set SMS storage, cause:%d\r\n", iResult);
            return FALSE;
        }
        APP_DEBUG("<-- Set SMS storage to SM, nUsed:%u,nTotal:%u -->\r\n", nUsed, nTotal);

        iResult = RIL_SMS_GetStorage(&nCurrStorage, &nUsed ,&nTotal);
        if(RIL_ATRSP_SUCCESS != iResult)
        {
            APP_DEBUG("Fail to get SMS storage, cause:%d\r\n", iResult);
            return FALSE;
        }
        APP_DEBUG("<-- Check SMS storage: curMem=%d, used=%d, total=%d -->\r\n", nCurrStorage, nUsed, nTotal);
    }
    #endif

    // Enable new short message indication
    // By default, the auto-indication for new short message is enalbed. You can do it again to
    // make sure that the option is open.
    #if 0
    {
        iResult = Ql_RIL_SendATCmd("AT+CNMI=2,1",Ql_strlen("AT+CNMI=2,1"),NULL,NULL,0);
        if (RIL_AT_SUCCESS != iResult)
        {
            APP_DEBUG("Fail to send \"AT+CNMI=2,1\", cause:%d\r\n", iResult);
            return FALSE;
        }
        APP_DEBUG("<-- Enable new SMS indication -->\r\n");
    }
    #endif

    // Delete all existed short messages (if needed)
    iResult = RIL_SMS_DeleteSMS(0, RIL_SMS_DEL_ALL_MSG);
    if (iResult != RIL_AT_SUCCESS)
    {
        APP_DEBUG("Fail to delete all messages, iResult=%d,cause:%d\r\n", iResult, Ql_RIL_AT_GetErrCode());
        return FALSE;
    }
    APP_DEBUG("Delete all existed messages\r\n");

    return TRUE;
}
/*****************************************************************************
* Function:
*
* Description:
*
* Parameters:
*
* Return:
*
*****************************************************************************/
static void reboot(void)
{
    u64 totalMS;
    totalMS = Ql_GetMsSincePwrOn();
	APP_DEBUG("<--%lld: Rebooting-->\r\n", totalMS);

	programmData.needReboot = TRUE;
	//Ql_Timer_Stop(LOGIC_WTD1_TMR_ID);
}
/*****************************************************************************
* Function:
*
* Description:
*
* Parameters:
*
* Return:
*
*****************************************************************************/
static char *Gsm_GetSignal(char *tmp_buff)
{
    u32 rssi;
    u32 ber;
    RIL_NW_GetSignalQuality(&rssi, &ber);

    u64 totalMS;
    totalMS = Ql_GetMsSincePwrOn();

    //APP_DEBUG("uptime: %lld ms, signal strength: %d, BER: %d\r\n", totalMS, rssi, ber);
    //Ql_strcpy(tmp_buff, DBG_BUFFER);
    Ql_sprintf(tmp_buff ,"uptime: %lld ms, signal strength: %d, BER: %d", totalMS, rssi, ber);

    return tmp_buff;
}


/*****************************************************************************
* Function:
*
* Description:
*
* Parameters:
*
* Return:
*
*****************************************************************************/
static char *Parse_Command(char *src_str, char *tmp_buff, sProgrammSettings *sett_in_ram, sProgrammData *programmData)//
{
	char *ret = NULL;
	if(programmData->firstInit == TRUE)
	{
		if(Ql_strcmp(src_str, "cmd reboot") == 0)
		{
			reboot();
			Ql_strcpy(tmp_buff, "\r\nrebooting\r\n");
			ret = tmp_buff;
		}
		if(Ql_strcmp(src_str, "cmd reconnect") == 0)
		{
			programmData->reconnectCnt = sett_in_ram->secondsToReconnect;
			Ql_strcpy(tmp_buff, "\r\nreconnecting\r\n");
			ret = tmp_buff;
		}
		else if(Ql_strcmp(src_str, "cmd commit") == 0)
		{
			if(write_to_flash_settings(sett_in_ram) == TRUE)
				Ql_strcpy(tmp_buff, "\r\ncommit ok\r\n");
			else
				Ql_strcpy(tmp_buff, "\r\ncommit error\r\n");
			ret = tmp_buff;
		}
		else if(Ql_strcmp(src_str, "cmd update firmware by ftp") == 0)
		{
			//u8 m_URL_Buffer[512];
			s32 strLen = Ql_sprintf(tmp_buff, "ftp://%s/%s:%d@%s:%s",
					sett_in_ram->ftpSettings.srvAddress,
					sett_in_ram->ftpSettings.fileName,
					sett_in_ram->ftpSettings.srvPort,
					sett_in_ram->ftpSettings.usrName,
					sett_in_ram->ftpSettings.usrPassw);
			ret = tmp_buff;
		}
		else
		{
			char *cmdstart = "cmd set ";
			if(Ql_strstr(src_str, cmdstart) != 0)
			{//set
				s32 len = Ql_strlen(src_str) - 	Ql_strlen(cmdstart);
				//Ql_Debug_Trace("come cmd len=<%d>\r\n", len);
				if(len > 0)
					ret = set_cmd(&src_str[Ql_strlen(cmdstart)], tmp_buff, sett_in_ram);
			}
			else
			{
				cmdstart = "cmd get ";
				if(Ql_strstr(src_str, cmdstart) != 0)
				{//get
					s32 len = Ql_strlen(src_str) - 	Ql_strlen(cmdstart);
					if(len > 0)
						ret = get_cmd(&src_str[Ql_strlen(cmdstart)], tmp_buff, sett_in_ram);
				}
			}
		}
	}
	return ret;
}

static char *set_cmd(char *cmdstr, char *tmp_buff, sProgrammSettings* sett_in_ram)
{
  char *ret = NULL;
  //char tbuff[50] = {0};
  bool r = FALSE;
  char *ch = Ql_strchr(cmdstr, '=');
  if(ch > 0)
  {
	  char cmd[50] = {0};
      char val[50] = {0};

      int len = Ql_strlen(cmdstr);
      int clen = (int)ch++ - (int)cmdstr;
      int vlen = ((int)cmdstr + len) - (int)ch;

      Ql_Debug_Trace("set_cmd len=<%d> clen=<%d> vlen=<%d>\r\n", len, clen, vlen);

      if(clen > 0 && vlen > 0)
      {
    	  Ql_strncpy(cmd, cmdstr, clen);
    	  Ql_strncpy(val, ch, vlen);

    	  vlen = clear_all_nulls(val, vlen);
    	  if(vlen <= 0)
    		  return NULL;

    	  if(Ql_strcmp(cmd, "mode") == 0)
    	  {
    		  s32 mode = Ql_atoi(val);
    		  if(mode == 0)
    			  sett_in_ram->ipSettings.mode = 0;
    		  else if(mode == 1)
    			  sett_in_ram->ipSettings.mode = 1;
    		  r = TRUE;
    	  }
    	  else if(Ql_strcmp(cmd, "apn") == 0)
    	  {
    		  if(vlen <= MAX_GPRS_APN_LEN)
    		  {
    			  Ql_memset(sett_in_ram->gsmSettings.gprsApn, 0, MAX_GPRS_APN_LEN);
    			  Ql_strncpy(sett_in_ram->gsmSettings.gprsApn, val, vlen);
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "user") == 0)
    	  {
    		  if(vlen < MAX_GPRS_USER_NAME_LEN)
    		  {
    			  Ql_memset(sett_in_ram->gsmSettings.gprsUser, 0, MAX_GPRS_USER_NAME_LEN);
    			  Ql_strncpy(sett_in_ram->gsmSettings.gprsUser, val, vlen);
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "password") == 0)
    	  {
    		  if(vlen < MAX_GPRS_PASSWORD_LEN)
    		  {
    			  Ql_memset(sett_in_ram->gsmSettings.gprsPass, 0, MAX_GPRS_PASSWORD_LEN);
    			  Ql_strncpy(sett_in_ram->gsmSettings.gprsPass, val, vlen);
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "daddress") == 0)
    	  {
    		  if(vlen < MAX_ADDRESS_LEN)
    		  {
    			  Ql_memset(sett_in_ram->ipSettings.dstAddress, 0, MAX_ADDRESS_LEN);
    			  Ql_strncpy(sett_in_ram->ipSettings.dstAddress, val, vlen);
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "saddress") == 0)
    	  {
    		  if(vlen < MAX_ADDRESS_LEN)
    		  {
    			  Ql_memset(sett_in_ram->ipSettings.srcAddress, 0, MAX_ADDRESS_LEN);
    			  Ql_strncpy(sett_in_ram->ipSettings.srcAddress, val, vlen);
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "dport") == 0)
    	  {
    		  s32 port = Ql_atoi(val);
    		  if(port > 0){
    			  sett_in_ram->ipSettings.dstPort = port;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "sport") == 0)
    	  {
    		  s32 port = Ql_atoi(val);
    		  if(port > 0){
    			  sett_in_ram->ipSettings.srcPort = port;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "sertimeout") == 0)
    	  {
    		  s32 timeout = Ql_atoi(val);
    		  if(timeout > 0){
    			  sett_in_ram->serPortDataTimeout = timeout;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "gsmtimeout") == 0)
    	  {
    		  s32 timeout = Ql_atoi(val);
    		  if(timeout > 0){
    			  sett_in_ram->gsmPortDataTimeout = timeout;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "baudrate") == 0)
    	  {
    		  s32 speed = Ql_atoi(val);
    		  if(speed > 0){
        		sett_in_ram->serPortSettings.baudrate = speed;
        		r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "stopbits") == 0)
    	  {
    		  u32 value = Ql_atoi(val);
    		  if(value >= SB_ONE && value <= SB_ONE_DOT_FIVE){
    			  sett_in_ram->serPortSettings.stopBits = value;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "parity") == 0)
    	  {
    		  u32 value = Ql_atoi(val);
    		  if(value >= PB_NONE && value <= PB_MARK){
    			  sett_in_ram->serPortSettings.parity = value;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "databits") == 0)
    	  {
    		  u32 value = Ql_atoi(val);
    		  if(value >= DB_5BIT && value <= DB_8BIT){
    			  sett_in_ram->serPortSettings.dataBits = value;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "toreboot") == 0)
    	  {
    		  s32 timeout = Ql_atoi(val);
    		  if(timeout > 1800){ //30 min
    			  sett_in_ram->secondsToReboot = timeout;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "toreconnect") == 0)
    	  {
    		  s32 timeout = Ql_atoi(val);
    		  if(timeout > 180){ // 3 min
    			  sett_in_ram->secondsToReconnect = timeout;
    			  r = TRUE;
    		  }
    	  }

    	  if(r == TRUE){
    		  *(--ch) = 0;
    		  ret = get_cmd(cmdstr, tmp_buff, sett_in_ram);
    	  }
    	  else{
    	      Ql_strcpy(tmp_buff, "\r\n");
    	      Ql_strcat(tmp_buff, "cmd set ERROR!");
    	      Ql_strcat(tmp_buff, "\r\n");
    	      ret = tmp_buff;
    	  }
      }
  }
  return ret;
}

static char *get_cmd(char *cmd, char *tmp_buff, sProgrammSettings* sett_in_ram)
{
  char *ret = NULL;
  char tbuff[50] = {0};

  int len = Ql_strlen(cmd);
  Ql_Debug_Trace("get_cmd len=<%d>\r\n", len);
  if(len > 0)
  {
	tmp_buff[0] = 0;
    if(Ql_strcmp(cmd, "mode") == 0)
    {
	  Ql_sprintf(tbuff ,"%d", sett_in_ram->ipSettings.mode);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "apn") == 0)
    {
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, sett_in_ram->gsmSettings.gprsApn);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "user") == 0)
    {
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, sett_in_ram->gsmSettings.gprsUser);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "password") == 0)
    {
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, sett_in_ram->gsmSettings.gprsPass);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "daddress") == 0)
    {
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, sett_in_ram->ipSettings.dstAddress);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "saddress") == 0)
    {
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, sett_in_ram->ipSettings.srcAddress);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "dport") == 0)
    {
	  Ql_sprintf(tbuff ,"%d", sett_in_ram->ipSettings.dstPort);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "sport") == 0)
    {
      Ql_sprintf(tbuff ,"%d", sett_in_ram->ipSettings.srcPort);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "sertimeout") == 0)
    {
	  Ql_sprintf(tbuff ,"%d", sett_in_ram->serPortDataTimeout);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "gsmtimeout") == 0)
    {
	  Ql_sprintf(tbuff ,"%d", sett_in_ram->gsmPortDataTimeout);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "baudrate") == 0)
    {
	  Ql_sprintf(tbuff ,"%d", (int)sett_in_ram->serPortSettings.baudrate);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "stopbits") == 0)
    {
      u32 value = sett_in_ram->serPortSettings.stopBits;
	  Ql_sprintf(tbuff ,"%d", value);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "parity") == 0)
    {
      u32 value = sett_in_ram->serPortSettings.parity;
      Ql_sprintf(tbuff ,"%d", value);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "databits") == 0)
    {
      u32 value = sett_in_ram->serPortSettings.dataBits;
      Ql_sprintf(tbuff ,"%d", value);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "toreboot") == 0)
    {
	  Ql_sprintf(tbuff ,"%d", sett_in_ram->secondsToReboot);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "toreconnect") == 0)
    {
	  Ql_sprintf(tbuff ,"%d", sett_in_ram->secondsToReconnect);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "version") == 0)
    {
    	Ql_sprintf(tbuff ,"%s", FW_VERSION);
    	Ql_strcpy(tmp_buff, "\r\n");
      	Ql_strcat(tmp_buff, cmd);
      	Ql_strcat(tmp_buff, "=");
      	Ql_strcat(tmp_buff, tbuff);
      	Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "sampling count") == 0)
    {
    	Ql_sprintf(tbuff ,"%d", sett_in_ram->adcSettings.samplingCount);
    	Ql_strcpy(tmp_buff, "\r\n");
      	Ql_strcat(tmp_buff, cmd);
      	Ql_strcat(tmp_buff, "=");
      	Ql_strcat(tmp_buff, tbuff);
      	Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "sampling interval") == 0)
    {
    	Ql_sprintf(tbuff ,"%d", sett_in_ram->adcSettings.samplingInterval);
    	Ql_strcpy(tmp_buff, "\r\n");
      	Ql_strcat(tmp_buff, cmd);
      	Ql_strcat(tmp_buff, "=");
      	Ql_strcat(tmp_buff, tbuff);
      	Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "button timeout") == 0)
    {
    	Ql_sprintf(tbuff ,"%d", sett_in_ram->buttonTimeout);
    	Ql_strcpy(tmp_buff, "\r\n");
      	Ql_strcat(tmp_buff, cmd);
      	Ql_strcat(tmp_buff, "=");
      	Ql_strcat(tmp_buff, tbuff);
      	Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "input1 timeout") == 0)
    {
    	Ql_sprintf(tbuff ,"%d", sett_in_ram->in1Timeout);
    	Ql_strcpy(tmp_buff, "\r\n");
      	Ql_strcat(tmp_buff, cmd);
      	Ql_strcat(tmp_buff, "=");
      	Ql_strcat(tmp_buff, tbuff);
      	Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "input2 timeout") == 0)
    {
    	Ql_sprintf(tbuff ,"%d", sett_in_ram->in2Timeout);
    	Ql_strcpy(tmp_buff, "\r\n");
      	Ql_strcat(tmp_buff, cmd);
      	Ql_strcat(tmp_buff, "=");
      	Ql_strcat(tmp_buff, tbuff);
      	Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
	else if(Ql_strcmp(cmd, "signal") == 0)
	{
		ret = Gsm_GetSignal(tmp_buff);
	}

  }
  return ret;
}


static s32 GetInputValue(Enum_PinName *pin, s32 *cnt, u32 max_timeout)
{
	s32 ret = -1;
	s32 st = Ql_GPIO_GetLevel(*pin);
	if(st > 0){
		if( *cnt <  max_timeout)
			*(cnt) += 1;
		if(*cnt >= max_timeout)
			ret = TRUE;
	}
	else{
		if( *cnt >  0)
			*(cnt) -= 1;
		if(*cnt <= 0)
			ret = FALSE;
	}
	return ret;
}

#endif // __EXAMPLE_TCPCLIENT__

