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
*   example_adc.c
*
* Project:
* --------
*   QuecOpen
*
* Description:
* ------------
*   This example demonstrates how to program ADC interface in QuecOpen.
*
*   All debug information will be output through DEBUG port.
*
* Usage:
* ------
*   Compile & Run:
*
*     Set "C_PREDEF=-D __EXAMPLE_ADC__" in makefile file. And compile the 
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
#ifdef __EXAMPLE_ADC__

#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"
#include "ril.h"
#include "ql_power.h"
#include "ql_adc.h"
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


#define     ADC_COUNT       10

osTimerId_t adc_timer_id;
static u32 adc_time_ticks = 1000;
Enum_ADCPin adc_pin = PIN_ADC0;
static s32 m_param = 0;

static void adc_timer_callback(void *argument);


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

	Ql_SleepDisable();
	Ql_RIL_Initialize();

    
 	Ql_UART_Open(UART_PORT0,115200,MainUartRecvCallback);
	
	APP_DEBUG("<-- QuecOpen: ADC Example -->\r\n");
	
	//open adc0
    ret = Ql_ADC_Open(adc_pin);
    if(ret < QL_RET_OK)
    {
        APP_DEBUG("<-- open adc0 failed, ret = %d -->\r\n",ret);        
    }
	else
	{
    	APP_DEBUG("<-- open adc0 successfully -->\r\n");
	}

    adc_timer_id = osTimerNew((osTimerFunc_t)adc_timer_callback, osTimerPeriodic, &m_param, NULL);
    osTimerStart(adc_timer_id, adc_time_ticks);


    while (1)
    {
    	osDelay(1000);
	}
}

static void adc_timer_callback(void *argument)
{
	u32 adcvalue = 0;
    s32 tempValue = 0;
    
    *((s32*)argument) += 1;

    // timer repeat 
    if(*((s32*)argument) >= ADC_COUNT)
    {
        osTimerStop(adc_timer_id);
	    APP_DEBUG("<-- ADC closed (%d) -->\r\n",Ql_ADC_Close(adc_pin));
    } 
	else
	{
		Ql_ADC_Read(adc_pin,&adcvalue);
        Ql_Get_Temperature(&tempValue);
		APP_DEBUG("<-- read adc value = %d mV,tempValue = %d) -->\r\n",adcvalue,tempValue);
	}
}


#endif // __EXAMPLE_ADC__

