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
 *   ql_uart.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   UART API defines.
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
#ifndef _QL_UART_H_
#define _QL_UART_H_


#include "ql_type.h"
#include "ql_error.h "


typedef enum
{
	 UART_PORT0   = 0, 		//main port
     UART_PORT1   = 1, 		//aux port
	 UART_PORT2   = 2, 		//dbg port ,only for unilog 
     UART_NONE,
}Enum_SerialPort;




typedef enum {
	DB_8BIT = 0,
    DB_5BIT = 5,
    DB_6BIT	= 6,
    DB_7BIT	= 7  
} Enum_DataBits;

typedef enum {
    SB_ONE = 0,
    SB_TWO
} Enum_StopBits;

typedef enum {
    PB_NONE = 0,
    PB_EVEN,
    PB_ODD
} Enum_ParityBits;

typedef enum { 			// None Flow Control
    FC_NONE=0, 
    FC_RTS,    
    FC_CTS,
 	FC_RTS_CTS
} Enum_FlowCtrl;

typedef struct {
    u32                 baudrate; 
    Enum_DataBits       dataBits;
    Enum_StopBits       stopBits;
    Enum_ParityBits     parity;
    Enum_FlowCtrl       flowCtrl;
}ST_UARTDCB;



/****** USART Event *****/
#define USART_EVENT_RECEIVE_COMPLETE    (1UL << 1)  ///< Receive completed
#define USART_EVENT_RX_TIMEOUT          (1UL << 6)  ///< Receive character timeout (optional)

typedef void (*CallBack_UART_Notify)(u32 event, void* dataPtr, u32 dataLen);

/*****************************************************************
* Function:     Ql_UART_Open 
* 
* Description:
*               This function opens a specified UART port with the specified 
*               flow control mode. 
*               Which task call this function, which task will own the specified UART port.
*
* Parameters:
*               [in]port:       
*                       Port name
*               [in]baudrate:      
*                       The baud rate of the UART to be opened
*                       for the physical the baud rate support 4800,9600,
*                       19200,38400,57600,115200,230400,460800,921600.
*
*               [in]CallBack_UART_Notify:     
*                       The pointer of the UART call back function.
*
* Return:        
*               QL_RET_OK indicates success; otherwise failure.      
*               QL_RET_ERR_PARAM indicates uart_receive_call_back error. 
*               QL_RET_ERR_BAUDRATE indicates baudrate not support. 
*               QL_RET_ERR_UART_BUSY indicates uart init error. 
*               QL_RET_ERR_INVALID_PORT indicates port not support.    
*
*****************************************************************/
s32 Ql_UART_Open( Enum_SerialPort port,u32 baudrate,CallBack_UART_Notify uart_receive_call_back);


/*****************************************************************
* Function:     Ql_UART_Open 
* 
* Description:
*               This function opens a specified UART port with the specified 
*               flow control mode. 
*               Which task call this function, which task will own the specified UART port.
*
* Parameters:
*               [in]port:       
*                       Port name
*               [in]dcb:   
*                       The point to the UART dcb struct. 
*                       Include baud rate,data bits,stop bits,parity,and flow control
*                       Not supported hardware flow control currently
*
*               [in]CallBack_UART_Notify:     
*                       The pointer of the UART call back function.
*
* Return:        
*               QL_RET_OK indicates success; otherwise failure.      
*               QL_RET_ERR_PARAM indicates uart_receive_call_back error. 
*               QL_RET_ERR_BAUDRATE indicates baudrate not support. 
*               QL_RET_ERR_UART_BUSY indicates uart init error. 
*               QL_RET_ERR_INVALID_PORT indicates port not support.       
*
*****************************************************************/
s32 Ql_UART_OpenEx(Enum_SerialPort port, ST_UARTDCB *dcb,CallBack_UART_Notify uart_receive_call_back);


/*****************************************************************
* Function:     Ql_UART_Write 
* 
* Description:
*               This function is used to write data to the specified UART port directly
* Parameters:
*               port:
*               [In]Specified UART port
*               buff:
*               [In]Pointer to write data buffer
*               buff_len:
*               [In]The length of the write data buffer.
* Return:
*               QL_RET_OK indicates success; otherwise failure.      
*               QL_RET_ERR_PARAM indicates uart_receive_call_back error. 
*               QL_RET_ERR_INVALID_PORT indicates port not support. 
*****************************************************************/
s32 Ql_UART_Write(Enum_SerialPort port,u8 *buff,u32 buff_len);


/*****************************************************************
* Function:     Ql_UART_DirectSend 
* 
* Description:
*               This function is used to write data to the specified UART port
* Parameters:
*               port:
*               [In]Specified UART port
*               buff:
*               [In]Pointer to write data buffer
*               buff_len:
*               [In]The length of the write data buffer.
* Attention:
*				Ql_UART_DirectSend is only used in callback_deepsleep_cb,
*					Please refer to the example_rtc.c for details
* Return:
*               QL_RET_OK indicates success; otherwise failure.      
*               QL_RET_ERR_PARAM indicates uart_receive_call_back error. 
*               QL_RET_ERR_INVALID_PORT indicates port not support. 
*****************************************************************/
s32 Ql_UART_DirectSend(Enum_SerialPort port,u8 *buff,u32 buff_len);


/*****************************************************************
* Function:     Ql_UART_Close 
* 
* Description:
*               This function close a specified UART port.
* Parameters:
*               [in] port:
*                      Port name
* Return:       
*               QL_RET_OK indicates success; otherwise failure.      
*               QL_RET_ERR_INVALID_PORT indicates port not support.
*****************************************************************/
s32 Ql_UART_Close(Enum_SerialPort port);


#endif  //__QL_UART_H__

