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
 *   example_pwm.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   This example demonstrates how to use PWM function with APIs in QuecOpen.
 *   Input the specified command through any uart port and the result will be 
 *   output through the uart port.
 *   Warning:	
 *			Only 1 PWM is supported, 3 GPIO are optional,only support PINNAME_SPI_SCLK,PINNAME_GPIO4,PINNAME_MAIN_RTS now
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "GLOBAL_EXPORT_FLAG += __EXAMPLE_PWM__" in makefile file. And compile the 
 *     app using "make clean/new".
 *     Download bootloader&app image bin to module to run.
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
#ifdef __EXAMPLE_PWM__
#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"
#include "ril.h"
#include "ql_power.h" 
#include "ql_gpio.h"
#include "ql_pwm.h"
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


Enum_PinName pwmpin = PINNAME_SPI_SCLK;

static void MainUartRecvCallback(u32 event, void* dataPtr, u32 dataLen)
{
	if((USART_EVENT_RX_TIMEOUT == event) || (USART_EVENT_RECEIVE_COMPLETE == event))
	{
	    Ql_UART_Write(UART_PORT0,(u8 *)dataPtr,dataLen);
	}
}


void proc_main_task(void)
{
	s32 ret = -1;
	u32	dutyCyclePercent =50;
	Ql_SleepDisable();
	
	Ql_RIL_Initialize();
 	Ql_UART_Open(UART_PORT0,115200,MainUartRecvCallback);

	APP_DEBUG("<-- QuecOpen: PWM Example -->\r\n");
	ret = Ql_PWM_Init(pwmpin, PWMSOURCE_26M, 1000000, dutyCyclePercent);
	APP_DEBUG("<-- Ql_PWM_Init ret:%d-->\r\n",ret);
	ret = Ql_PWM_Output(pwmpin,1);
	APP_DEBUG("<-- Ql_PWM_Output ret:%d-->\r\n",ret);

	while(1)
    {
		osDelay(1000);		
        if (dutyCyclePercent >= 100)
            dutyCyclePercent = 0;
        dutyCyclePercent += 5;
		
		ret = Ql_PWM_UpdateDutyCycle(pwmpin, dutyCyclePercent);
		APP_DEBUG("Ql_PWM_UpdateDutyCycle ret= %d,duty=%d\r\n", ret,dutyCyclePercent);
    }
}

#endif

