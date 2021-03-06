/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Quectel Co., Ltd. 2020
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
 *   This example demonstrates how to use timer function with APIs in QuecOpen.
 *   It starts a repeat stack timer.The stack-timer runs twice.You can modify TIMEOUT_COUNT to make it run more times.
 *   And it starts a one-time GPTimer.The GPTimer runs once.
 *   All debug trace will be output through MAIN port.
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "C_PREDEF=-D __EXAMPLE_TIMER__" in gcc_makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
 * 
 *   Operation:
 *             
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
#include <string.h>
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_timer.h"
#include "ql_uart.h"
#include "ql_stdlib.h"
#include "ql_error.h"


#define DEBUG_ENABLE 1
#if DEBUG_ENABLE > 0
#define DEBUG_PORT  UART_PORT0
#define DBG_BUF_LEN   512
static char DBG_BUFFER[DBG_BUF_LEN];
#define APP_DEBUG(FORMAT,...) {\
    Ql_memset(DBG_BUFFER, 0, DBG_BUF_LEN);\
    Ql_sprintf(DBG_BUFFER,FORMAT,##__VA_ARGS__); \
    if (UART_PORT2 == (DEBUG_PORT)) \
    {\
        Ql_Debug_Trace(DBG_BUFFER);\
    } else {\
        Ql_UART_Write((Enum_SerialPort)(DEBUG_PORT), (u8*)(DBG_BUFFER), Ql_strlen((const char *)(DBG_BUFFER)));\
    }\
}
#else
#define APP_DEBUG(FORMAT,...) 
#endif


static Enum_SerialPort m_myUartPort  = UART_PORT0;

/*****************************************************************
* timer param
******************************************************************/
#define TIMEOUT_COUNT 2
static u32 Stack_timer = 0x102; // timerId =102; timerID is Specified by customer, but must ensure the timer id is unique in the quecopen task
static u32 ST_Interval = 2000;
static s32 m_param1 = 0;

static u32 GP_timer = 0x101;
static u32 GPT_Interval =1000;
static s32 m_param2 = 0;

//timer callback
static void Timer_handler(u32 timerId, void* param);

//uart callback
static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara)
{
     
}


void proc_main_task(s32 taskId)
{
    s32 ret;
    ST_MSG msg;

    // Register & open UART port
    ret = Ql_UART_Register(m_myUartPort, CallBack_UART_Hdlr, NULL);
    if (ret < QL_RET_OK)
    {
        Ql_Debug_Trace("Fail to register serial port[%d], ret=%d\r\n", m_myUartPort, ret);
    }
    ret = Ql_UART_Open(m_myUartPort, 115200, FC_NONE);
    if (ret < QL_RET_OK)
    {
        Ql_Debug_Trace("Fail to open serial port[%d], ret=%d\r\n", m_myUartPort, ret);
    }
    
    APP_DEBUG("\r\n<--QuecOpen: Timer TEST!-->\r\n");  

    //register  a timer
    ret = Ql_Timer_Register(Stack_timer, Timer_handler, &m_param1);
    if(ret <0)
    {
        APP_DEBUG("\r\n<--failed!!, Ql_Timer_Register: timer(%d) fail ,ret = %d -->\r\n",Stack_timer,ret);
    }
    APP_DEBUG("\r\n<--Register: timerId=%d, param = %d,ret = %d -->\r\n", Stack_timer ,m_param1,ret); 

    //register  a GP-Timer
    ret = Ql_Timer_RegisterFast(GP_timer, Timer_handler, &m_param2);
    if(ret <0)
    {
        APP_DEBUG("\r\n<--failed!!Ql_Timer_RegisterFast: GP_timer(%d)fail ,ret = %d -->\r\n",GP_timer,ret);
    }
    APP_DEBUG("\r\n<--RegisterFast: timerId=%d, param = %d,ret=%d -->\r\n", GP_timer ,m_param1,ret); 

    //start a timer,repeat=true;
    ret = Ql_Timer_Start(Stack_timer,ST_Interval,TRUE);
    if(ret < 0)
    {
        APP_DEBUG("\r\n<--failed!! stack timer,Ql_Timer_Start,ret=%d-->\r\n",ret);        
    }
    APP_DEBUG("\r\n<--stack timer,Ql_Timer_Start(Id=%d,Interval=%d),ret=%d-->\r\n",Stack_timer,ST_Interval,ret);

    //start a GPTimer ,repeat=false
    ret = Ql_Timer_Start(GP_timer,GPT_Interval,FALSE);
    if(ret < 0)
    {
        APP_DEBUG("\r\n<--failed!! GP-timer Ql_Timer_Start fail,ret=%d-->\r\n",ret);
    }                
    APP_DEBUG("\r\n<--GP-timer,Ql_Timer_Start(Id=%d,Interval=%d),ret=%d-->\r\n",GP_timer,GPT_Interval,ret);
      
    while (1)
    {
		
		//Task uses this interface to process interrupt messages, so it is necessary and cannot be deleted.
        Ql_OS_GetMessage(&msg);
        switch(msg.message)
        {
        case MSG_ID_USER_START:
            break;
        default:
            break;
        }
        
    }
}

// timer callback function
void Timer_handler(u32 timerId, void* param)
{
    *((s32*)param) +=1;
    if(Stack_timer == timerId)// stack_timer repeat 
    {
        APP_DEBUG("<-- stack Timer_handler, param:%d -->\r\n", *((s32*)param));

        if(*((s32*)param) >= TIMEOUT_COUNT)
        {
            s32 ret;
            ret = Ql_Timer_Stop(Stack_timer);
            if(ret < 0)
            {
                  APP_DEBUG("\r\n<--failed!! stack timer Ql_Timer_Stop ret=%d-->\r\n",ret);           
            }
            APP_DEBUG("\r\n<--stack timer,Ql_Timer_Stop(Id=%d,) ret=%d-->\r\n",Stack_timer,ret);            
        }        
    }
    if(GP_timer == timerId) //not repeat 
    {
        APP_DEBUG("<-- GP-Timer_handler, param:%d -->\r\n", *((s32*)param));
    }
    
}



#endif
