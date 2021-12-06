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
 *   example_gpio.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   This example demonstrates how to program a GPIO pin in QuecOpen.
 *   This example choose PINNAME_GPIO1 pin as GPIO.
 *
 *   The "Enum_PinName" enumeration defines all the GPIO pins.
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "GLOBAL_EXPORT_FLAG += __EXAMPLE_GPIO__" in makefile file. And compile 
 *     the app using "make clean/new".
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
#ifdef __EXAMPLE_GPIO__
#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"
#include "ril.h"
#include "ql_gpio.h"
#include "ql_power.h"
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


Enum_PinName  gpioPin = PINNAME_GPIO1;

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
	ret = Ql_GPIO_Init(gpioPin, PINDIRECTION_OUT, gpioLvl, PINPULLSEL_PULLUP);
	APP_DEBUG("<---- Initialize GPIO pin RET = %d,gpioPin=%d---->\r\n",ret,gpioPin);
		
	// Set the GPIO level to low after 500ms.
	APP_DEBUG("<-- Set the GPIO level to low after 500ms -->\r\n");
	osDelay(500);
	Ql_GPIO_SetLevel(gpioPin, PINLEVEL_LOW);
	
	// Set the GPIO level to high after 500ms.
	APP_DEBUG("<-- Set the GPIO level to high after 500ms -->\r\n");
	osDelay(500);
	Ql_GPIO_SetLevel(gpioPin, PINLEVEL_HIGH);

}


void proc_main_task(void)
{
	s32 ret = -1;
	s32 gpioCount = 0;
	Ql_SleepDisable();
	
	Ql_RIL_Initialize();
 	Ql_UART_Open(UART_PORT0,115200,MainUartRecvCallback);
	
	APP_DEBUG("<-- QuecOpen: GPIO Example -->\r\n");
	// Initialize the GPIO pin (output high level, pull up)
	GPIO_init();
    osDelay(1000);	

    while (1)
    {	
		ret = Ql_GPIO_GetDirection(gpioPin);
		APP_DEBUG("Ql_GPIO_GetDirection,Dir=%d\r\n",ret);
        if (gpioCount & 0x1)
        {
        	gpioCount++;
            ret = Ql_GPIO_SetLevel(gpioPin, PINLEVEL_LOW);
			APP_DEBUG("GPIO_SetLevel LOW=%d ,Level=%d\r\n",ret,Ql_GPIO_GetLevel(gpioPin));
        }
        else
        {
        	gpioCount++;
            ret = Ql_GPIO_SetLevel(gpioPin, PINLEVEL_HIGH);
			APP_DEBUG("GPIO_SetLevel HIGH=%d ,Level=%d\r\n",ret,Ql_GPIO_GetLevel(gpioPin));
        }
		ret = Ql_GPIO_Uninit(gpioPin);
		APP_DEBUG("Ql_GPIO_Uninit,ret=%d\r\n",ret);

		osDelay(500);
		GPIO_init();
	}

}

#endif

