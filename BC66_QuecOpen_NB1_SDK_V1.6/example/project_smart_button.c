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
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_uart.h"
#include "ql_stdlib.h"
#include "ql_error.h"
#include "ql_power.h"
#include "ql_common.h"
#include "ql_system.h"
#include "ql_gpio.h"
#include "ql_rtc.h"
#include "ql_psm_eint.h"
#include "ql_timer.h"
#include "ql_time.h"
#include "ril.h"
#include "ril_system.h"
#include "ril_util.h"
#include "ril_sim.h"
#include "ril_network.h"

#include "typedef.h"
#include "flash.h"
#include "infrastructure.h"
#include "convert.h"
//#include "ql_nidd.h"

/*****************************************************************
* define process state
******************************************************************/
//s32 from_deep_mode_task_id = 0;

/*****************************************************************
* UART Param
******************************************************************/
#define SERIAL_RX_BUFFER_LEN  512
static char m_RxBuf_Uart[SERIAL_RX_BUFFER_LEN];

/*****************************************************************
* timer param
******************************************************************/
#define GPIO_TIMER_ID 		 		(TIMER_ID_USER_START + 1)
#define GPIO_INPUT_TIMER_PERIOD 	100
#define	BT_TIMER_TIMEOUT			60
/*****************************************************************
* psm_eint param
******************************************************************/
//static s32 eint_param = 0;
volatile bool PSM_EINT_Flag = FALSE;
/*****************************************************************
* rtc param
******************************************************************/
static u32 Rtc_id = 0x101;
static u32 Rtc_Interval = 60*1000  *5; //5min
//static s32 rtc_param = 0;
volatile bool RTC_Flag = FALSE;
/*****************************************************************
* power param
******************************************************************/
Enum_Ql_Power_On_Result_t m_pwrOnReason;
Enum_Ql_Wake_Up_Result_t m_wakeUpReason;
static s32 ds_param = 0;

/*****************************************************************
* Local time param
******************************************************************/
ST_Time time;
ST_Time* pTime = NULL;
u32 totalSeconds = 0;

/*****************************************************************
* Other Param
******************************************************************/
static Enum_PinName  	led_pin 	= PINNAME_GPIO2;//30
static Enum_PinName  	button_pin 	= PINNAME_GPIO3;//31
static sProgrammSettings programmSettings;

/*****************************************************************
* user param
******************************************************************/
static sProgrammData programmData =
{
	.firstInit 		= FALSE,
    .needReboot 	= FALSE,
    .initFlash		= FALSE,
    .timerStart		= FALSE,
    .sleepEnable	= TRUE,

    .totalSeconds 	= 0,
    .buttonCnt 		= 0,
    .buttonState 	= FALSE,
    .HbuttonState	= FALSE,

    .ledBlinkCnt	= 0,

    .needSendNidd 	= FALSE,
    .timerTimeout  = BT_TIMER_TIMEOUT * 1000
};
Enum_NIDDSTATE m_nidd_state = NIDD_STATE_WAIT;

NIDD_Urc_Param_t *nidd_urc_param_ptr = NULL;


#define NIDD_BUFFER_LEN     512
//#define RECV_BUFFER_LEN     200

static char m_nidd_buf[NIDD_BUFFER_LEN];
static u32  nidd_recv_len = 0;
//static u8 m_recv_buf[RECV_BUFFER_LEN];

static char tmp_buff[256] = {0};


/*****************************************************************
* CallBack func declaration
******************************************************************/
static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara);
static void gpio_callback_onTimer(u32 timerId, void* param);

static void Rtc_handler(u32 rtcId, void* param);
static void Deepsleep_handler(void* param);
static void PSM_EINT_handler(void* param);

/*****************************************************************
* User func declaration
******************************************************************/
static void InitFlash(void);
static void InitGPIO(void);
static bool GetLocalTime(void);
static void InitRTC(void);

//static void ReLoadRTC(void);
static u32 getTimerTimeout(void);
static void proc_handle(Enum_SerialPort port, char *pData,s32 len);

/*****************************************************************
* Main process
******************************************************************/
void proc_main_task(s32 taskId)
{
	s32 ret;
    ST_MSG msg;

    Ql_UART_Register(UART_PORT0, CallBack_UART_Hdlr, NULL);//main uart
    Ql_UART_Open(UART_PORT0, 115200, FC_NONE);
    APP_DEBUG("<--QuecOpen: Starting Application for BC66 smart_button, taskId=%d.-->\r\n", taskId);

	if(PSM_EINT_Flag == TRUE){
		programmData.timerStart = TRUE;
		//APP_DEBUG("<--deepsleep PSM_EINT_Flag=TRUE in start-->\r\n");
	}

	ret = Ql_DeepSleep_Register(Deepsleep_handler, &ds_param);
    if(ret <0){
		APP_DEBUG("\r\n<--deepsleep handler register failed-->\r\n");
    }

    //Eint register
    ret = Ql_Psm_Eint_Register(PSM_EINT_handler, (void*)&PSM_EINT_Flag);
    if(ret != QL_RET_OK){
	    APP_DEBUG("\r\n<--psm_eint register failed! ret(%d)-->\r\n",ret);
    }

    while(TRUE)
    {
        Ql_OS_GetMessage(&msg);
        //APP_DEBUG("<--proc_main_task Ql_OS_GetMessage msg=%d->\r\n", msg.message);
        switch(msg.message)
        {
        case MSG_ID_RIL_READY:
        {
            Ql_RIL_Initialize();
            APP_DEBUG("RIL is ready-->\r\n");

            //enable  psm event. if the module enter psm mode  which is reported by URC(+QNBIOTEVENT:  "ENTER PSM").
            RIL_QNbiotEvent_Enable(PSM_EVENT);
            programmData.firstInit 	= TRUE;
			m_pwrOnReason 			= Ql_GetPowerOnReason();
			if(QL_DEEP_SLEEP == m_pwrOnReason){//deep sleep wake up
				m_wakeUpReason = Ql_GetWakeUpReason();
				APP_DEBUG("\r\n<--The module wake up from deep sleep mode, m_wakeUpReason=%d, EINT_Flag=%d, RTC_Flag=%d-->\r\n", m_wakeUpReason, PSM_EINT_Flag, RTC_Flag);
				PSM_EINT_Flag 	= FALSE;
				RTC_Flag 		= FALSE;
			}
			else if (QL_SYS_RESET == m_pwrOnReason){// reset
           		APP_DEBUG("\r\n<--The module reset or first power on-->\r\n");
			}
        }
        break;

      	case MSG_ID_URC_INDICATION:
		{
			switch (msg.param1)
            {
        		case URC_EGPRS_NW_STATE_IND:
           			//APP_DEBUG("<--EGPRS Network Status:<%d>-->\r\n", msg.param2);
           			if(msg.param2 == 1)
           			{
           				Ql_Sleep(1000);
           				GetLocalTime();
           				//ReLoadRTC();
           			}
             	break;

    			case URC_PSM_EVENT:
					if(ENTER_PSM == msg.param2){
	          			APP_DEBUG("<--The modem enter PSM mode-->\r\n");
			 		}
                	break;

    			case URC_NIDD_EVENT:
    			{
    				nidd_urc_param_ptr = msg.param2;
					if(nidd_urc_param_ptr != NULL && RECVDATA_NIDD == nidd_urc_param_ptr->state)
					{
						//APP_DEBUG("<--Recv NIDD Data, len=<%d>, data=<%s>-->\r\n", nidd_urc_param_ptr->dataLen, nidd_urc_param_ptr->data);
	          			char *p1 = Ql_strchr(nidd_urc_param_ptr->data, (s32)'"');
	          			if(p1){
	          				p1++;
	          			}
	          			else{
	          				p1 = nidd_urc_param_ptr->data;
	          			}
	          			char *p2 = Ql_strchr(p1, (s32)'"');
	          			if(p2){
	          				*p2 = 0;
	          			}
	          			if(Ql_strlen(p1) == nidd_urc_param_ptr->dataLen*2){
	          				char *dst = Ql_strcpy(m_nidd_buf, p1);
	          				nidd_recv_len = Ql_strlen(dst);
	          			}
			 		}
                	break;
    			}

         		default:
         			APP_DEBUG("<--MSG_ID_URC_INDICATION,  msg.param1=<%u>, msg.param2=<%u>-->\r\n", msg.param1, msg.param2);
           		break;
			}
      	}
		break;

        default:
        	APP_DEBUG("<--Ql_OS_GetMessage, msg.message=<%u>, msg.param1=<%u>, msg.param2=<%u>-->\r\n", msg.message, msg.param1, msg.param2);
            break;
        }
    }
}


void proc_subtask1(s32 TaskId)
{
    //ST_MSG subtask1_msg;
    s32 ret 		= -1;
    s32 accountId 	= -1;
    s32 niddId 		= -1;
    s32 cgreg 		= 0;
    u32 rssi		= 0;
    u32 ber			= 0;
    s32 tmpCNT 		= 0;
    s32 niddErrorCNT = 0;
    s32 recvTimeout  = 0;
    u32 capacity, voltage;
    u32	secondsCNT 	= 0;
    u32 lastPid 	= 0;
    sDataJsonParams recv;

    do
    {
    	Ql_Sleep(50);//in ms
    }
    while(programmData.firstInit == FALSE);

    APP_DEBUG("<--subtask: starting, taskId=%d-->\r\n", TaskId);
    InitFlash();
    InitGPIO();
    InitRTC();

    while (TRUE)
    {
    	//Ql_GPIO_SetLevel(led_pin, programmData.buttonState);//goto proc_subtask2
    	Ql_Sleep(100);

    	if(programmData.initFlash == FALSE) continue;

    	if(programmData.needSendNidd == TRUE)
    	{
    		programmData.needSendNidd = FALSE;
    		if(m_nidd_state == NIDD_STATE_WAIT){
    			m_nidd_state = NIDD_CHECK_NET_REG;
    			programmData.timerTimeout  = getTimerTimeout();
    			cgreg = 0;
    		}
    	}

    	if(tmpCNT++ >= 9)
    	{
    		tmpCNT = 0;
    		secondsCNT++;
    		//APP_DEBUG("<--m_nidd_state=%d, programmData.timerTimeout=%ld-->\r\n", m_nidd_state, programmData.timerTimeout);
			switch (m_nidd_state)
			{
				case NIDD_CHECK_NET_REG:
					ret = RIL_NW_GetEGPRSState(&cgreg);
					APP_DEBUG("<--NIDD_CHECK_NET_REG Network State: cgreg=%d-->\r\n", cgreg);
					if((cgreg == NW_STAT_REGISTERED)||(cgreg == NW_STAT_REGISTERED_ROAMING)){
						ret = RIL_NW_GetSignalQuality(&rssi, &ber);
						APP_DEBUG("<--RIL_NW_GetSignalQuality, rssi=%u, ber=%u-->\r\n", rssi, ber);
						niddErrorCNT = 0;
						m_nidd_state = NIDD_STATE_ACT_PDN;
						//cgreg = 0;
					}
					else{
						if(niddErrorCNT++ > 50){//50s
							m_nidd_state = NIDD_STATE_FREE;
						}
					}
					break;

				case NIDD_STATE_ACT_PDN:
					ret = Ql_PDN_Activate(1, 4, programmSettings.gsmSettings.gprsApn, programmSettings.gsmSettings.gprsUser, programmSettings.gsmSettings.gprsPass);
					APP_DEBUG("<--Ql_PDN_Activate, ret=%d-->\r\n", ret);

					m_nidd_state = NIDD_STATE_CREATE_ACCOUNT;
					break;

				case NIDD_STATE_CREATE_ACCOUNT:
					ret = Ql_NIDD_CreateID(programmSettings.gsmSettings.gprsApn, programmSettings.gsmSettings.gprsUser, programmSettings.gsmSettings.gprsPass);
					APP_DEBUG("<--Ql_NIDD_CreateID, ret=%d-->\r\n", ret);
					if(ret >= 0){
						accountId = ret;
					}
					m_nidd_state = NIDD_STATE_CONNECT;
					break;

				case NIDD_STATE_CONNECT:
					if(accountId >= 0)
					{
						ret = Ql_NIDD_Connect(accountId);
						APP_DEBUG("<--Ql_NIDD_Connect, ret=%d-->\r\n", ret);

						if(ret >= 0){
							niddId = ret;

							Ql_NIDD_ActivateConnection(niddId);
							APP_DEBUG("<--Ql_NIDD_ActivateConnection, ret=%d-->\r\n", ret);
						}
					}

					m_nidd_state = NIDD_STATE_SEND;
					break;

				case NIDD_STATE_SEND:
					if(niddId >= 0)
					{
						Ql_memset(m_nidd_buf, 0, NIDD_BUFFER_LEN);
						Ql_memset(tmp_buff, 0, sizeof(tmp_buff));

						if(GetLocalTime() == TRUE){
							secondsCNT = programmData.totalSeconds;
						}
						lastPid = secondsCNT;
						u32 state = programmData.buttonState;

						s32 len = Ql_sprintf(tmp_buff, "{pid:%lu,state:%u,rssi:%u,ber:%u", lastPid, state, rssi, ber);
						//s32 len = Ql_sprintf(tmp_buff, "{state:%u,rssi:%u,ber:%u,pid:%lu", state, rssi, ber,lastPid);
						ret = RIL_GetPowerSupply(&capacity, &voltage);
						if(ret == QL_RET_OK){
							len += Ql_sprintf((char*)(tmp_buff+len), ",voltage:%u,capacity:%u", voltage, capacity);
						}

				        char strCCID[30];
				        Ql_memset(strCCID, 0x0, sizeof(strCCID));
				        ret = RIL_SIM_GetCCID(strCCID);
				        if(ret == QL_RET_OK){
				        	len += Ql_sprintf((char*)(tmp_buff+len), ",iccid:%s", strCCID);
				        }
				        Ql_strcat(tmp_buff, "}");
				        len++;
						convertToHex(m_nidd_buf, tmp_buff, len);
						ret = Ql_NIDD_SendData(niddId, m_nidd_buf);
						APP_DEBUG("<--Ql_NIDD_SendData ret=%d, data=<%s>-->\r\n", ret, tmp_buff);
						if(ret == RIL_AT_SUCCESS){
							recvTimeout = 60;
							programmData.timerTimeout += recvTimeout * 1000;
							Ql_memset(m_nidd_buf, 0, sizeof(m_nidd_buf));
							nidd_recv_len = 0;
						}
					}

					m_nidd_state = NIDD_STATE_RECV;
					break;

				case NIDD_STATE_RECV:
					if(nidd_recv_len > 0)
					{
						Ql_memset(tmp_buff, 0, sizeof(tmp_buff));

						APP_DEBUG("<--NIDD_STATE_RECV, nidd_recv_len=%u, m_nidd_buf=<%s>-->\r\n", nidd_recv_len, m_nidd_buf);
						u32 convlen = convertFromHex(tmp_buff, m_nidd_buf, nidd_recv_len);
						APP_DEBUG("<--NIDD_STATE_RECV, convert data len=%u, convert data=<%s>-->\r\n", convlen, tmp_buff);

						Ql_memset(&recv, 0x0,  sizeof(recv));
						bool fl = fromJSON(tmp_buff, &recv);
						if(fl){
							APP_DEBUG("<--NIDD_STATE_RECV: lastPid=<%lu>, recv.pid=<%lu>-->\r\n", lastPid, recv.pid);
							if(lastPid == recv.pid){
								programmData.ledBlinkCnt = 50;
							}
						}
						recvTimeout = 0;
					}
					if(--recvTimeout <= 0){
						//APP_DEBUG("<--NIDD_STATE_RECV, recvTimeout=%d-->\r\n", recvTimeout);
						m_nidd_state = NIDD_STATE_CLOSE;
					}
					break;

				case NIDD_STATE_CLOSE:
					if(niddId >= 0){
						ret = Ql_NIDD_CloseConnection(niddId);
						APP_DEBUG("<--Ql_NIDD_CloseConnection, ret=%d-->\r\n", ret);
					}
					m_nidd_state = NIDD_STATE_CLOSE_PDN;
					break;

				case NIDD_STATE_CLOSE_PDN:
					ret = Ql_PDN_Deactivate(1);
					APP_DEBUG("<--Ql_PDN_Deactivate, ret=%d-->\r\n", ret);

					m_nidd_state = NIDD_STATE_FREE;
					break;

				case NIDD_STATE_FREE:
					APP_DEBUG("<--NIDD_STATE_FREE-->\r\n");
					niddId 				= -1;
					accountId 			= -1;
					niddErrorCNT 		= 0;
					cgreg 				= 0;
				    rssi				= 0;
				    ber					= 0;

					m_nidd_state 		= NIDD_STATE_WAIT;
					break;

				default:
					break;
			};
    	}
    }
}


void proc_subtask2(s32 TaskId)
{
	APP_DEBUG("<--subtask: starting, taskId=%d-->\r\n", TaskId);
    while (TRUE)
    {
    	Ql_Sleep(100);
    	if(programmData.ledBlinkCnt--  > 0)
    	{
    		Ql_GPIO_SetLevel(led_pin, programmData.ledBlinkCnt%2 > 0 ? PINLEVEL_HIGH : PINLEVEL_LOW);//Enum_PinLevel
    	}
    	else
    	{
    		Ql_GPIO_SetLevel(led_pin, programmData.buttonState);
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
               proc_handle(port, m_RxBuf_Uart, totalBytes);
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
	s32 ret;
	//APP_DEBUG("<--buttonMaxCnt=%ld, buttonCnt=%d-->\r\n", programmData.buttonMaxCnt, programmData.buttonCnt);
	if(programmData.initFlash == FALSE) return;
	if (GPIO_TIMER_ID == timerId)
	{
		if(programmData.timerStart == TRUE){
			programmData.buttonMaxCnt 	= programmSettings.buttonTimeout - 800;
			if(programmData.buttonMaxCnt < 200) {
				programmData.buttonMaxCnt = 200;
			};
		}
		else{
			programmData.buttonMaxCnt = programmSettings.buttonTimeout;
		}

		u32 max_timeout = programmData.buttonMaxCnt/GPIO_INPUT_TIMER_PERIOD;
		s32 btp = GetInputValue(&button_pin, &programmData.buttonCnt, max_timeout, TRUE);
		if(btp >= 0){
			programmData.buttonState = (bool)btp;
			programmData.timerStart = FALSE;
		}

		if(programmData.HbuttonState != programmData.buttonState)
		{//is changed
			programmData.HbuttonState = programmData.buttonState;
			if(programmData.buttonState == 0){
				programmData.needSendNidd = TRUE;
				//APP_DEBUG("<--needSendNidd=%d", programmData.needSendNidd);
			}
			programmData.timerTimeout  = getTimerTimeout();

			//APP_DEBUG("<--Get the button_pin GPIO level value changed: %d-->\r\n", programmData.buttonState);
			APP_DEBUG("<--Get the button_pin GPIO level value changed: %d, max_timeout=%u, timerTimeout=%ld-->\r\n", programmData.buttonState, max_timeout, programmData.timerTimeout);
		}

		programmData.timerTimeout -= GPIO_INPUT_TIMER_PERIOD;
		if(--programmData.timerTimeout <= 0)
		{
			programmData.timerTimeout = 0;
			if(programmData.sleepEnable == FALSE){
				ret = Ql_SleepEnable();
				if(ret == QL_RET_OK){
					programmData.sleepEnable = TRUE;
				}
				APP_DEBUG("<--Ql_SleepEnable ret=%d-->\r\n", ret);
			}
		}
	}
}

// when the module wakes up from the deep sleep, calls psm_eint callback first,  then initializes the quecopen,
//so the API interface of Ql cannot be called in here.
void PSM_EINT_handler(void* param)
{
	*((bool*)param) = TRUE;
	//никакие функции здесь нельзя, зависает!!!
}

// rtc callback function
static void Rtc_handler(u32 rtcId, void* param)
{
    if(Rtc_id == rtcId) {
    	*((bool*)param) = TRUE;
    	programmData.timerTimeout  = getTimerTimeout();
    }
}

// deepsleep callback function
static void Deepsleep_handler(void* param)
{
	*((s32*)param) +=1;//not use
	APP_DEBUG("<--The module enter deepsleep mode-->\r\n");
}

/*****************************************************************
* User func implimentation
******************************************************************/
static void InitFlash(void)
{
	//APP_DEBUG("<--OpenCPU: init_flash! Size=%d-->\r\n", sizeof(sProgrammSettings));
    bool ret = FALSE;
    for(int i=0; i < 10; i++)
    {
        ret = init_flash(&programmSettings);
        if(ret == TRUE)
        {
        	programmData.initFlash = TRUE;
        	return;
        }
        else
        {
        	APP_DEBUG("<--init_flash Err, try next cnt=%i\r\n", i);
        	Ql_Sleep(1000);
        }
        //APP_DEBUG("<--init_flash ret=%d-->\r\n", ret);
    }
    //APP_DEBUG("<--init_flash ERROR, try restore default!!!-->\r\n");
    ret = restore_default_flash(&programmSettings);

    APP_DEBUG("<--restore_default_flash ret=%d-->\r\n", ret);
    reboot(&programmData);
}

static void InitGPIO(void)
{
    Ql_GPIO_Init(led_pin, 		PINDIRECTION_OUT, 	PINLEVEL_LOW, 	PINPULLSEL_DISABLE);
    Ql_GPIO_Init(button_pin, 	PINDIRECTION_IN, 	PINLEVEL_HIGH, 	PINPULLSEL_PULLUP);


    programmData.timerTimeout  = getTimerTimeout();
    Ql_Timer_RegisterFast(GPIO_TIMER_ID, gpio_callback_onTimer, NULL);
    Ql_Timer_Start(GPIO_TIMER_ID, GPIO_INPUT_TIMER_PERIOD, TRUE);//Ql_Timer_RegisterFast//timerId


    s32 ret = Ql_SleepDisable();
    if(ret == QL_RET_OK){
    	programmData.sleepEnable = FALSE;
    }

    APP_DEBUG("<--Ql_Timer_Start TimerId=%d, Ql_SleepDisable, timerTimeout=%ld, period=%d-->\r\n", GPIO_TIMER_ID, programmData.timerTimeout, GPIO_INPUT_TIMER_PERIOD);
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

    if(programmData.initFlash == TRUE)
    {
    	u64 timeout = programmSettings.rtcInterval;
		if(timeout >= 60 && timeout <= 86400){
			//programmSettings.rtcNeedCorrect
		}
		else {
			timeout = 86400;
		}

		Rtc_Interval = (u32)timeout*1000;
		APP_DEBUG("\r\n<--InitRTC timeout=%lu sec-->\r\n", timeout);

		/*
		if(programmSettings.rtcNeedCorrect == TRUE)
		{
			Enum_Ql_Wake_Up_Result_t res = Ql_GetWakeUpReason();

			APP_DEBUG("\r\n<--InitRTC Ql_GetWakeUpReason res=%d-->\r\n", res);
			if(res == Ql_RTC_AlARM_WAKEUP)
			{
				ret = Ql_Rtc_Stop(Rtc_id);
			 	APP_DEBUG("\r\n<--Need Rtc_Stop, ret=%d-->\r\n", ret);

			 	programmSettings.rtcNeedCorrect = FALSE;
		    	ret = write_to_flash_settings(&programmSettings);
		    	APP_DEBUG("\r\n<--Write rtcNeedCorrect, ret=%d-->\r\n", ret);
			}
		}
		*/
    }
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




/*
static void ReLoadRTC(void)
{// инициализация и переинициализация должны происходить в ОДНОМ и том же потоке!!!
	s32 ret;
	if(programmData.initFlash == TRUE)
	{
		u64 timeout = programmSettings.rtcInterval;

		if(timeout >= 60 && timeout <= 86400){}
		else{
			timeout = 86400;
		}

		u64 newSeconds = programmData.totalSeconds/timeout;
		newSeconds = (newSeconds+1) * timeout;
		if(newSeconds > programmData.totalSeconds){
			Rtc_Interval = newSeconds - programmData.totalSeconds;
			APP_DEBUG("\r\n<--ReLoadRTC Rtc_Interval=%d, newSeconds=%d, programmData.totalSeconds=%ld-->\r\n", Rtc_Interval, newSeconds, programmData.totalSeconds);

		    //rtc register
		    ret = Ql_Rtc_RegisterFast(Rtc_id, Rtc_handler, &RTC_Flag);
		    if(ret < 0){
				if(-4 != ret){
					APP_DEBUG("\r\n<--InitRTC rtc register failed!:RTC id(%d),ret(%d)-->\r\n", Rtc_id, ret);
				}
		    }

		    ret = Ql_Rtc_Stop(Rtc_id);
		    if(ret == QL_RET_OK){
			    ret = Ql_Rtc_Start(Rtc_id, Rtc_Interval, TRUE);
			    if(ret < 0){
			    	APP_DEBUG("\r\n<--rtc start failed!!:ret=%d-->\r\n",ret);
			    }
			    else{
			    	programmSettings.rtcNeedCorrect = TRUE;
			    	ret = write_to_flash_settings(&programmSettings);
			    	APP_DEBUG("\r\n<--Write rtcNeedCorrect, ret=%d-->\r\n", ret);
			    }
		    }
		    else
		    	APP_DEBUG("\r\n<--Rtc_Stop failed!!: ret=%d-->\r\n",ret);
		}
	}
}*/


static u32 getTimerTimeout(void)
{
	u32 ret = BT_TIMER_TIMEOUT * 1000;

	if(programmData.initFlash == TRUE){
		if(programmSettings.timerTimeout > 10000){
			ret = (u32)programmSettings.timerTimeout;
		}
	}
	//APP_DEBUG("<--getTimerTimeout=%u-->\r\n", ret);
	return ret;
}

static void proc_handle(Enum_SerialPort port, char *pData,s32 len)
{
	APP_DEBUG("<--Read data from port=%d, len=%d-->\r\n", port, len);
	pData[len] = 0;

	    //char tmp_buff[100] = {0};
		Ql_memset(tmp_buff, 0, sizeof(tmp_buff));
		char *answer = Parse_Command(pData, tmp_buff, &programmSettings, &programmData);
		if( answer != NULL)
		{
			//u32 alen = Ql_strlen(answer);
			APP_DEBUG("%s", answer);
			programmData.timerTimeout  = getTimerTimeout();
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
				programmData.timerTimeout  = getTimerTimeout();
				return;
			}
		}
}


#endif // __PROJECT_SMART_BUTTON__
