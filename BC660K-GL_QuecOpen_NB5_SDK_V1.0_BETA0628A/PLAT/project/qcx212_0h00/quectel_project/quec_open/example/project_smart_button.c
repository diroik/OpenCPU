/*
 * project_smart_button.c
 *
 *  Created on: 18 февр. 2022 г.
 *      Author: DROIK
 */


#ifdef __PROJECT_SMART_BUTTON__

#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"
#include "ril.h"
#include "ql_power.h"
#include "ql_ps.h"
#include "ql_gpio.h"
#include "ql_uart.h"
#include "ql_dbg.h"
#include "ql_error.h"
#include "ql_time.h"

#include "typedef.h"
//#include "flash.h"
#include "infrastructure.h"
#include "convert.h"

/*****************************************************************
* system
******************************************************************/
extern osMessageQueueId_t maintask_queue;

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
* CallBack function declaration
******************************************************************/
static s32 pscallback(ENUMurcID eventId, void *param, u32 paramLen);
static void MainRecvCallback(u32 event, void* dataPtr, u32 dataLen);
/*****************************************************************
* User function declaration
******************************************************************/
//static void InitFlash(void);
static void InitGPIO(void);
//static bool GetLocalTime(void);
//static void InitRTC(void);

//static u32 getTimerTimeout(void);
//static void proc_handle(Enum_SerialPort port, char *pData,s32 len);

/*****************************************************************
* OS processes
******************************************************************/
void proc_main_task(void)
{
	s32 ret;
    ST_MSG msg;

	ret = Ql_UART_Open(UART_PORT0, 115200, MainRecvCallback);

	Ql_RIL_Initialize();
    APP_DEBUG("<--QuecOpen: Starting Application for BC660 smart_button-->\r\n");

	ret = Ql_PSCallback_Register(GROUP_PS_MASK, pscallback);
    APP_DEBUG("<------------- QuecOpen: Ql_PSCallback_Register:[%d] ----------->\r\n",ret);

	maintask_queue = osMessageQueueNew(MAINTASK_QUEUE_LEN, sizeof(msg), NULL);

    //Ql_SleepEnable();

	InitGPIO();

    while (1)
	{
		osDelay(2000);
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

	            	}
	                	break;
	            	default:
	                	APP_DEBUG("<-- Other URC: type = %d\r\n", msg.param1);
	                	break;
	            }
			}
	        	break;

			default:
				break;
    	}
	}
}


void sub_task1(void)
{
	//u32 msg;

	InitGPIO();

	while(1)
	{
		osDelay(5000);
		APP_DEBUG("<-- sub_task1 is alive -->\r\n");
	}
}

void sub_task2(void)
{
	//u32 msg;
	osDelay(1000);

	while(1)
	{
		osDelay(5000);
		APP_DEBUG("<-- sub_task2 is alive -->\r\n");
	}
}

/*****************************************************************
* Callback function implementation
******************************************************************/
static s32 pscallback(ENUMurcID eventId, void *param, u32 paramLen)
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

			break;
		case URC_ID_SIM_READY:
			msg.message = MSG_ID_URC_INDICATION;
			msg.param1 = URC_SIM_CARD_STATE_IND;
			osMessageQueuePut(maintask_queue, &msg, 0,0);

			break;
		default:
			break;
	}
	return 0;
}

static void MainRecvCallback(u32 event, void* dataPtr, u32 dataLen)
{
	if((USART_EVENT_RX_TIMEOUT == event) || (USART_EVENT_RECEIVE_COMPLETE == event))
	{
		APP_DEBUG("[MainUartReceiveCallback]recv data:%s.-->\r\n",(u8*)dataPtr);
	}
}

/*****************************************************************
* User function implementation
******************************************************************/
/*static void InitFlash(void)
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
}*/

static void InitGPIO(void)
{
    //Ql_GPIO_Init(led_pin1, 		PINDIRECTION_OUT, 	PINLEVEL_LOW, 	PINPULLSEL_DISABLE);
    //Ql_GPIO_Init(led_pin2, 		PINDIRECTION_OUT, 	PINLEVEL_LOW, 	PINPULLSEL_DISABLE);
    //Ql_GPIO_Init(button_pin, 	PINDIRECTION_IN, 	PINLEVEL_HIGH, 	PINPULLSEL_PULLUP);

/*
    programmData.timerTimeout  = getTimerTimeout();
    Ql_Timer_RegisterFast(GPIO_TIMER_ID, gpio_callback_onTimer, NULL);
    Ql_Timer_Start(GPIO_TIMER_ID, GPIO_INPUT_TIMER_PERIOD, TRUE);//Ql_Timer_RegisterFast//timerId


    s32 ret = Ql_SleepDisable();
    if(ret == QL_RET_OK){
    	programmData.sleepEnable = FALSE;
    }

    APP_DEBUG("<--Ql_Timer_Start TimerId=%d, Ql_SleepDisable, timerTimeout=%ld, period=%d-->\r\n", GPIO_TIMER_ID, programmData.timerTimeout, GPIO_INPUT_TIMER_PERIOD);

    */
}


#endif // __PROJECT_SMART_BUTTON__
