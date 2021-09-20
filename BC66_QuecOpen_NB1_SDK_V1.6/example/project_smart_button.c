/*****************************************************************************
*  Copyright Statement:
*  --------------------

*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   project_smartbutton.c
 *
 * Project:
 * --------
 *   myQuecOpen
 *
 * Description:
 * ------------
 *
 * Usage:
 * ------
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
#ifdef __PROJECT_SMART_BUTTON__
#include "custom_feature_def.h"
#include "ql_stdlib.h"
#include "ql_common.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_rtc.h"
#include "ql_error.h"
#include "ql_gpio.h"
#include "ql_power.h"
#include "ql_system.h"
#include "ql_wtd.h"
#include "ql_timer.h"
#include "ql_adc.h"
#include "ql_uart.h"
#include "ql_socket.h"
#include "ql_time.h"
#include "ril.h"
#include "ril_system.h"
#include "ril_util.h"
#include "ril_sim.h"
#include "ril_network.h"

#include "typedef.h"
#include "flash.h"
#include "infrastructure.h"

/*****************************************************************
* define process state
******************************************************************/
s32 from_deep_mode_task_id = 0;

/*****************************************************************
* UART Param
******************************************************************/
#define SERIAL_RX_BUFFER_LEN  1400
static char m_RxBuf_Uart[SERIAL_RX_BUFFER_LEN];

/*****************************************************************
* timer param
******************************************************************/
//#define TCP_TIMER_ID              	TIMER_ID_USER_START
//#define TCP_TIMEOUT_90S_TIMER_ID 	(TIMER_ID_USER_START + 1)   //timeout
#define GPIO_TIMER_ID 		 		(TIMER_ID_USER_START + 2)
#define LED_TIMER_ID 		 		(TIMER_ID_USER_START + 3)
//#define LOGIC_WTD1_TMR_ID  			(TIMER_ID_USER_START + 4)

//#define WTD_TMR_TIMEOUT 		1500
//#define TCP_TIMER_PERIOD     	500
//#define TIMEOUT_90S_PERIOD   	90000
#define GPIO_INPUT_TIMER_PERIOD 100

//static s32 timeout_90S_monitor = FALSE;
/*****************************************************************
* rtc param
******************************************************************/
static u32 Rtc_id = 0x101;
static u32 Rtc_Interval = 60*1000*1; //1min
static s32 m_param = 0;
Enum_Ql_Power_On_Result_t m_pwrOnReason;

static s32 ds_param = 0;
/*****************************************************************
* Other Param
******************************************************************/
static sProgrammSettings programmSettings;

static s32 				led_cnt 	= 5;
static Enum_PinName  	led_pin 	= PINNAME_GPIO2;//30
static Enum_PinName  	button_pin 	= PINNAME_GPIO3;//31
static Enum_PinName  	in1_pin 	= PINNAME_GPIO4;//32
static Enum_PinName  	in2_pin 	= PINNAME_GPIO5;//33
/*****************************************************************
* user param
******************************************************************/
static sProgrammData programmData =
{
	.firstInit 		= FALSE,
    .needReboot 	= FALSE,
    .initFlash		= FALSE,

    .totalMS 		= 0,
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


/*****************************************************************
* CallBack func declaration
******************************************************************/
static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara);
static void gpio_callback_onTimer(u32 timerId, void* param);
static void Rtc_handler(u32 rtcId, void* param);
static void Deepsleep_handler(void* param);
/*****************************************************************
* User func declaration
******************************************************************/
static void InitFlash(void);
//static void InitWDT(void);
//static void InitUART(void);
//static void InitTCP(void);
static void InitGPIO(void);
//static void InitADC(void);

static void proc_handle(Enum_SerialPort port, char *pData,s32 len);
/*****************************************************************
* Main process
******************************************************************/
void proc_main_task(s32 taskId)
{
	from_deep_mode_task_id = taskId;
	s32 ret;
    ST_MSG msg;

    // Register & open UART port
    Ql_UART_Register(UART_PORT0, CallBack_UART_Hdlr, NULL);//main uart
    Ql_UART_Register(UART_PORT1, CallBack_UART_Hdlr, NULL);//debug uart
    Ql_UART_Open(UART_PORT0, 115200, FC_NONE);
    Ql_UART_Open(UART_PORT1, 115200, FC_NONE);

    APP_DEBUG("<-- QuecOpen: Starting Application for BC66 smart_button, taskId=%d. -->\r\n", taskId);


    // register deepsleep handler
	ret = Ql_DeepSleep_Register(Deepsleep_handler,&ds_param);
    if(ret <0){
		APP_DEBUG("\r\n<--deepsleep handler register failed-->\r\n");
    }
	else {
        APP_DEBUG("\r\n<--deepsleep handler register successfully-->\r\n");
	}

    //rtc register
    ret = Ql_Rtc_RegisterFast(Rtc_id, Rtc_handler, &m_param);
    if(ret < 0){
		if(-4 == ret){
			APP_DEBUG("\r\n<--the rtc is already register-->\r\n");
		}
		else{
		  APP_DEBUG("\r\n<--rtc register failed!:RTC id(%d),ret(%d)-->\r\n",Rtc_id,ret);
		}
    }
	else{
        APP_DEBUG("\r\n<--rtc register successful:RTC id(%d),param(%d),ret(%d)-->\r\n",Rtc_id,m_param,ret);
	}


    //start a rtc ,repeat=true;
    ret = Ql_Rtc_Start(Rtc_id,Rtc_Interval,TRUE);
    if(ret < 0){
		if(-4 == ret){
			APP_DEBUG("\r\n<--the rtc is already start-->\r\n");
		}
		else{
           APP_DEBUG("\r\n<--rtc start failed!!:ret=%d-->\r\n",ret);
		}
    }
	else{
         APP_DEBUG("\r\n<--rtc start successful:RTC id(%d),interval(%d),ret(%d)-->\r\n",Rtc_id,Rtc_Interval,ret);
	}


    while(TRUE)
    {
        Ql_OS_GetMessage(&msg);
        APP_DEBUG("<-- proc_main_task Ql_OS_GetMessage msg=%d-->\r\n", msg.message);

        switch(msg.message)
        {
        case MSG_ID_RIL_READY:
        {
            Ql_RIL_Initialize();
            APP_DEBUG("<-- RIL is ready -->\r\n");
            programmData.firstInit = TRUE;

			RIL_QNbiotEvent_Enable(PSM_EVENT);//enable  psm event. if the module enter psm mode  which is reported by URC(+QNBIOTEVENT:  "ENTER PSM").

			m_pwrOnReason = Ql_GetPowerOnReason();
			if(QL_DEEP_SLEEP == m_pwrOnReason)//deep sleep wake up
			{
				APP_DEBUG("\r\n<--The module wake up from deep sleep mode -->\r\n");
			}
			else if (QL_SYS_RESET == m_pwrOnReason)// reset
			{
           		APP_DEBUG("\r\n<--The module reset or first power on-->\r\n");
			}
        }
        break;

		case MSG_ID_APP_TEST:
		{
			APP_DEBUG("\r\n<--recv the message of rtc.(%d)-->\r\n",msg.param1);
		}
        break;

      	case MSG_ID_URC_INDICATION:
		{
			switch (msg.param1)
            {
        		case URC_EGPRS_NW_STATE_IND:
           			APP_DEBUG("<-- EGPRS Network Status:%d -->\r\n", msg.param2);
             	break;

    			case URC_PSM_EVENT:
					if(ENTER_PSM== msg.param2)
			 		{
	          			APP_DEBUG("<-- The modem enter PSM mode-->\r\n");
			 		}
                	break;

         		default:
                     //APP_DEBUG("<-- Other URC: type=%d\r\n", msg.param1);
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
    ST_MSG subtask1_msg;

    //Ql_Sleep(100);
    APP_DEBUG("<-- subtask: entering -->\r\n");

    do
    {
    	Ql_Sleep(100);//in ms
    }
    while(programmData.firstInit == FALSE);

    APP_DEBUG("<-- subtask: starting, taskId=%d -->\r\n", TaskId);

    //InitWDT();//mast be first
    InitFlash();
    //InitUART();
    InitGPIO();
    //InitADC();

    while (TRUE)
    {
        Ql_OS_GetMessage(&subtask1_msg);
        APP_DEBUG("<-- proc_subtask1 Ql_OS_GetMessage subtask1_msg=%d-->\r\n", subtask1_msg.message);
        switch (subtask1_msg.message)
        {
            default:
                break;
        }
    }
}

/*****************************************************************
* Callback func implimentation
******************************************************************/
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
		programmData.totalMS += 100;

		//led_cnt
	    if(led_cnt-- < 0)
	    {
	      led_cnt = 10;

	      if(programmData.rebootCnt++ > programmSettings.secondsToReboot)
	      {
	        programmData.rebootCnt  = 0;
	        programmData.needReboot = TRUE;
	      }
	      Ql_GPIO_SetLevel(led_pin, Ql_GPIO_GetLevel(led_pin) == PINLEVEL_HIGH ? PINLEVEL_LOW : PINLEVEL_HIGH);
	    }
	}
}

// rtc callback function
static void Rtc_handler(u32 rtcId, void* param)
{
	*((s32*)param) +=1;

    if(Rtc_id == rtcId)
    {
		//Ql_SleepDisable();

		//if you want  send message in ISR,please use the following API.
		Ql_OS_SendMessageFromISR(from_deep_mode_task_id, MSG_ID_APP_TEST,*((s32*)param),0);

		APP_DEBUG("\r\n<--Rtc_handler(%d)->\r\n",*((s32*)param));
    }
}

// deepsleep callback function
static void Deepsleep_handler(void* param)
{
	*((s32*)param) +=1;

	APP_DEBUG("<--The module enter deepsleep mode, param is %d-->\r\n",*((s32*)param));
}
/*****************************************************************
* User func implimentation
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
        	APP_DEBUG("<-- init_flash OK crc=<%d> apn=<%s> user=<%s> pass=<%s> server=<%s> port=<%d> baudrate=<%d>-->\r\n",
        			programmSettings.crc,
        			programmSettings.gsmSettings.gprsApn,
        			programmSettings.gsmSettings.gprsUser,
        			programmSettings.gsmSettings.gprsPass,
        			programmSettings.ipSettings.dstAddress,
        			programmSettings.ipSettings.dstPort,
        			programmSettings.serPortSettings.baudrate);

        	programmData.initFlash = TRUE;
        	return;
        }
        else
        {
        	APP_DEBUG("<-- init_flash Err, try next cnt=%i\r\n", i);
        	Ql_Sleep(1000);
        }
        //APP_DEBUG("<-- init_flash ret=%d-->\r\n", ret);
    }

    APP_DEBUG("<-- init_flash ERROR, try restore default!!! -->\r\n");
    ret = restore_default_flash(&programmSettings);

    APP_DEBUG("<-- restore_default_flash ret=%d -->\r\n", ret);
    reboot(&programmData);
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
    Ql_Timer_Start(LED_TIMER_ID, 100, TRUE);

    APP_DEBUG("<-- End InitGPIO -->\r\n");
}

static void proc_handle(Enum_SerialPort port, char *pData,s32 len)
{
	APP_DEBUG("Read data from port=%d, len=%d\r\n", port, len);
	pData[len] = 0;

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
			//m_pCurrentPos = m_send_buf;
			//Ql_strcpy(m_pCurrentPos + m_remain_len, pData);
			//m_remain_len+=len;
		}
	}
}


#endif // __PROJECT_SMART_BUTTON__
