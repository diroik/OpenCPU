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
 *   ql_dbg.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   unilog debug trace API defines.
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

#ifndef __QL_DBG__
#define __QL_DBG__

#include "ql_type.h"
#include "ql_uart.h"

#define PORT_DBG_LOG  UART_PORT0


/*****************************************************************
* Function:     Ql_Debug_Trace
* 
* Description:
*		This function is used for print log is in EPAT,You can search
*		for "QUECTEL_CUSTOM" in EPAT to find your trace.
*
* Parameters:
* 		data:
* 		[in] String,Other types can be converted to string externally.
*                 
* Return:        
*****************************************************************/
void Ql_Debug_Trace(uint8_t* data);


/*****************************************************************
* Function:     Ql_Data_DirectSend 
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
*				Ql_Data_DirectSend is only used in callback_deepsleep_cb,
*					Please refer to the example_rtc.c for details
* Return:
*               QL_RET_OK indicates success; otherwise failure.      
*               QL_RET_ERR_PARAM indicates uart_receive_call_back error. 
*               QL_RET_ERR_INVALID_PORT indicates port not support. 
*****************************************************************/
s32 Ql_Data_DirectSend(Enum_SerialPort port,u8 *buff,u32 buff_len);


#endif

