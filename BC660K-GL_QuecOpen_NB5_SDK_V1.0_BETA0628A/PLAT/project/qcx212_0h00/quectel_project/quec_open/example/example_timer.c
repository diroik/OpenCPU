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
 *   example_timer.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   This example demonstrates how to use timer function with APIs in OpenCPU.
 *   It starts a repeat stack timer.The stack-timer runs twice.
 *   You can modify TIMEOUT_COUNT to make it run more times.
 *   And it starts a one-time GPTimer.The GPTimer runs once.
 *   All debug trace will be output through DEBUG port.
 *   Warning:	
 *			A total of 5 IDs is available for GP timer
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "GLOBAL_EXPORT_FLAG += __EXAMPLE_TIMER__" in makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
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
#ifdef __EXAMPLE_TIMER__
#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"
#include "ril.h"
#include "ql_power.h" 
#include "ql_gpio.h"
#include "ql_timer.h"
#include "ql_dbg.h"

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

// Specify a GPIO pin
#define TEST_GP_TIMER_PERIOD	(2000)
#define TEST_OS_TIMER_PERIOD	(1000)	
Enum_PinName  gpioPinLED = PINNAME_GPIO1;
Enum_PinName  gpioPinCTS = PINNAME_MAIN_CTS;
volatile static int update_counter = 0;
volatile static int cts_counter = 0;

#define TIMEOUT_COUNT 100
static u32 stackTimer = 0x102; // timerId =102; timerID is Specified by customer
static u32 stackTimerInterval = 2000;
static s32 stackTimerParam = 0;

static u32 gpTimer = 0x101;
static u32 gpTimerInterval = 1000;
static s32 gpTimerParam = 0;


static void MainUartRecvCallback(u32 event, void* dataPtr, u32 dataLen)
{
	if((USART_EVENT_RX_TIMEOUT == event) || (USART_EVENT_RECEIVE_COMPLETE == event))
	{
	    Ql_UART_Write(UART_PORT0,(u8 *)dataPtr,dataLen);
	}
}

static void GPIO_init(void)
{
	s32 ret = -1;
	// Define the initial level for GPIO pin
	Enum_PinLevel gpioLvl = PINLEVEL_HIGH;
	
	// Initialize the GPIO pin (output high level, pull up)
	ret = Ql_GPIO_Init(gpioPinLED, PINDIRECTION_OUT, gpioLvl, PINPULLSEL_PULLUP);
	APP_DEBUG("<---- Initialize GPIO pin RET = %d,gpioPin=%d---->\r\n",ret,gpioPinLED);

	ret = Ql_GPIO_Init(gpioPinCTS, PINDIRECTION_OUT, gpioLvl, PINPULLSEL_PULLUP);
	APP_DEBUG("<---- Initialize Test GPIO pin RET = %d,gpioPin=%d---->\r\n",ret,gpioPinLED);
	
	// Set the GPIO level to low after 500ms.
	APP_DEBUG("<-- Set the GPIO level to low after 500ms -->\r\n");
	osDelay(500);
	Ql_GPIO_SetLevel(gpioPinLED, PINLEVEL_LOW);
	Ql_GPIO_SetLevel(gpioPinCTS, PINLEVEL_LOW);
	
	// Set the GPIO level to high after 500ms.
	APP_DEBUG("<-- Set the GPIO level to high after 500ms -->\r\n");
	osDelay(500);
	Ql_GPIO_SetLevel(gpioPinLED, PINLEVEL_HIGH);
	Ql_GPIO_SetLevel(gpioPinCTS, PINLEVEL_HIGH);
}

// timer callback function
void Timer_handler(u32 timerId, void* param)
{
    *((s32*)param) += 1;
    if(stackTimer == timerId)// stack_timer
    {
        APP_DEBUG("<-- stack Timer_handler, param:%d -->\r\n", *((s32*)param));
		cts_counter++;
        if(*((s32*)param) >= TIMEOUT_COUNT)
        {
            s32 ret;
            ret = Ql_TIMER_Stop(stackTimer);
            if(ret < 0)
            {
                  APP_DEBUG("\r\n<--failed!! stack timer Ql_TIMER_Stop ret=%d-->\r\n",ret);           
            }
            APP_DEBUG("\r\n<--stack timer,Ql_TIMER_Stop(Id=%d,) ret=%d-->\r\n",stackTimer, ret);            
        }        
    }
    if(gpTimer == timerId) //gp_timer
    {
		// Warning: Time consuming and other interrupt operations are not allowed,include APP_DEBUG
		update_counter++;
		
		if (update_counter & 0x1)
		{
			Ql_GPIO_SetLevel(gpioPinLED, PINLEVEL_LOW);
		}
		else
		{	
			Ql_GPIO_SetLevel(gpioPinLED, PINLEVEL_HIGH);
		}
    }   
}

void proc_main_task(void)
{
	s32 ret = 0;
	
	Ql_SleepDisable();
	Ql_RIL_Initialize();
 	Ql_UART_Open(UART_PORT0, 115200, MainUartRecvCallback);
	GPIO_init();

	APP_DEBUG("<-- QuecOpen: Timer Example -->\r\n");

	 //register a stack Timer
    ret = Ql_TIMER_Register(stackTimer, Timer_handler, &stackTimerParam);
    if(ret <0)
    {
        APP_DEBUG("\r\n<--failed!!, Ql_TIMER_Register: timer(%d) fail ,ret = %d -->\r\n",stackTimer,ret);
    }
    APP_DEBUG("\r\n<--Register: timerId=%d, param = %d,ret = %d -->\r\n", stackTimer ,stackTimerParam,ret); 

    //register a gp Timer
    ret = Ql_TIMER_RegisterFast(gpTimer, Timer_handler, &gpTimerParam);
    if(ret <0)
    {
        APP_DEBUG("\r\n<--failed!!Ql_TIMER_RegisterFast: GP_timer(%d)fail ,ret = %d -->\r\n",gpTimer,ret);
    }
    APP_DEBUG("\r\n<--RegisterFast: timerId=%d, param = %d,ret=%d -->\r\n", gpTimer ,gpTimerParam,ret); 

    //start a stack Timer,repeat=true;
    ret = Ql_TIMER_Start(stackTimer, stackTimerInterval, TRUE);
    if(ret < 0)
    {
        APP_DEBUG("\r\n<--failed!! stack timer,Ql_TIMER_Start,ret=%d-->\r\n",ret);        
    }
    APP_DEBUG("\r\n<--stack timer,Ql_TIMER_Start(Id=%d,Interval=%d),ret=%d-->\r\n",stackTimer,stackTimerInterval,ret);

    //start a gp Timer ,repeat=false
	ret = Ql_TIMER_Start(gpTimer, gpTimerInterval, false);
    if(ret < 0)
    {
        APP_DEBUG("\r\n<--failed!! GP-timer Ql_TIMER_Start fail,ret=%d-->\r\n",ret);
    }                
    APP_DEBUG("\r\n<--GP-timer,Ql_TIMER_Start(Id=%d,Interval=%d),ret=%d-->\r\n",gpTimer,gpTimerInterval,ret);
	
	while(1)
    {
		osDelay(1000);
		
		APP_DEBUG("task:stackTimer(%d) cts_counter=%d and gpTimer(%d) update_counter=%d and  demo task\r\n", stackTimer, cts_counter, gpTimer, update_counter);
    }
}
#endif

