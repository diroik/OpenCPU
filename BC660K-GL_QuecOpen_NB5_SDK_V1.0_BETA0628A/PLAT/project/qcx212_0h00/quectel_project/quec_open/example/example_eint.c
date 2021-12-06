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
 *   example_eint.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   This example demonstrates how to program a EINT pin in QuecOpen.
 *   This example choose PINNAME_I2C_SCL\PINNAME_GPIO2\PINNAME_SPI_CS pin as EINT.
 *
 *   The "Enum_PinName" enumeration defines all the GPIO pins.
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "GLOBAL_EXPORT_FLAG += __EXAMPLE_EINT__" in makefile file. And compile the 
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
#ifdef __EXAMPLE_EINT__
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

u32 gpioInterruptCount1 = 0;
u32 gpioInterruptCount2 = 0;
u32 gpioInterruptCount3 = 0;
extern osMessageQueueId_t maintask_queue;

Enum_PinName  eintPin1 = PINNAME_I2C_SCL;
Enum_PinName  eintPin2 = PINNAME_GPIO2;
Enum_PinName  eintPin3 = PINNAME_SPI_CS;


static void EintCallback(Enum_PinName pinName)
{	
	ST_MSG msg;

	switch(pinName)
	{
		case PINNAME_I2C_SCL:
		        msg.message = (u32)PINNAME_I2C_SCL;
		        msg.param1 = gpioInterruptCount1++;
				osMessageQueuePut(maintask_queue, (ST_MSG*)&msg, 0,0);
			break;
		case PINNAME_GPIO2:			
		        msg.message = (u32)PINNAME_GPIO2;
		        msg.param1 = gpioInterruptCount2++;
				osMessageQueuePut(maintask_queue, (ST_MSG*)&msg, 0,0);
			break;
		case PINNAME_SPI_CS:
		        msg.message = (u32)PINNAME_SPI_CS;
		        msg.param1 = gpioInterruptCount3++;
				osMessageQueuePut(maintask_queue, (ST_MSG*)&msg, 0,0);
			break;
		default:
			break;
	}
}


static void MainUartRecvCallback(u32 event, void* dataPtr, u32 dataLen)
{
	if((USART_EVENT_RX_TIMEOUT == event) || (USART_EVENT_RECEIVE_COMPLETE == event))
	{
	    Ql_UART_Write(UART_PORT0,(u8 *)dataPtr,dataLen);
	}
}


void proc_main_task(void)
{
	s32 ret=-1;	
	ST_MSG msg;
	Ql_SleepDisable();
	Ql_RIL_Initialize();
 	Ql_UART_Open(UART_PORT0,115200,MainUartRecvCallback);
	maintask_queue = osMessageQueueNew(MAINTASK_QUEUE_LEN, sizeof(ST_MSG), NULL);
	APP_DEBUG("<-- QuecOpen: EINT Example -->\r\n");

	ret = Ql_EINT_Init(eintPin1 ,EINT_EDGE_RISING,50,EintCallback);
	APP_DEBUG("eintPin1 EINT_Init ret:%d,%d\r\n",ret,eintPin1);
	ret = Ql_EINT_Init(eintPin2 ,EINT_EDGE_RISING,1000,EintCallback);
	APP_DEBUG("eintPin2 EINT_Init ret:%d,%d\r\n",ret,eintPin2);
	ret = Ql_EINT_Init(eintPin3 ,EINT_EDGE_RISING,1000,EintCallback);
	APP_DEBUG("eintPin3 EINT_Init ret:%d,%d\r\n",ret,eintPin3);
    while (1)
    {
    	if(osOK == osMessageQueueGet(maintask_queue,(void *)&msg, NULL, osWaitForever))
    	{	
    		switch(msg.message)
    			{
				case PINNAME_I2C_SCL:
					APP_DEBUG("eintPin1 trigger gpioInterruptCount1:%d\r\n",gpioInterruptCount1);
					if(gpioInterruptCount1 > 5)
					{
						ret = Ql_EINT_Uninit(eintPin1);
						APP_DEBUG("eintPin1 EINT_UnInit ret:%d\r\n",ret);
					}
					break;
				case PINNAME_GPIO2:
					APP_DEBUG("eintPin2 trigger gpioInterruptCount2:%d\r\n",gpioInterruptCount2);
					if(gpioInterruptCount2 > 5)
					{
						ret = Ql_EINT_Uninit(eintPin2);
						APP_DEBUG("eintPin2 EINT_UnInit ret:%d\r\n",ret);
					}			
					break;
				case PINNAME_SPI_CS:
					APP_DEBUG("eintPin3 trigger gpioInterruptCount3:%d\r\n",gpioInterruptCount3);
					if(gpioInterruptCount3 > 5)
					{
						ret = Ql_EINT_Uninit(eintPin3);
						APP_DEBUG("eintPin3 EINT_UnInit ret:%d\r\n",ret);
					}			
					break;
				default:
					APP_DEBUG("default message:%d\r\n",msg.message);
					break;
    			}
		}
    	
	}

}

#endif

