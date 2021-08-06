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
 *   ql_socket.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   socket APIs defines.
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
 

#ifndef __QL_SOCKET_H__
#define __QL_SOCKET_H__

#include "ql_network.h"

/* Socket Type */ 
typedef enum
{
    SOC_TYPE_TCP = 0,  /* TCP */ 
    SOC_TYPE_UDP,       /* datagram socket, UDP */ 
} Socket_Type;

typedef enum
{
    SOC_SUCCESS           =  0,    /* success */
    SOC_ERROR_MEM         = -1,    /* Out of memory error */
    SOC_ERROR_BUFF        = -2,    /* Buffer error.*/
    SOC_ERROR_TIMEOUT     = -3,    /*Timeout */
    SOC_ERROR_RTE         = -4,    /* Routing problem.*/
    SOC_ERROR_INPROGRESS  = -5,    /*Operation in progress */
    SOC_ERROR_VAL         = -6,    /* Illegal value. */
    SOC_ERROR_WOULDBLOCK  = -7,    /* Operation would block. */
    SOC_ERROR_USE         = -8,    /*  Address in use */
    SOC_ERROR_ALREADY     = -9,    /* operation already in progress*/
    SOC_ERROR_ISCONN      = -10,   /* Conn already established.*/
    SOC_ERROR_CONN        = -11,   /*  Not connected */
    SOC_ERROR_ABRT        = -12,   /*  Connection aborted*/
    SOC_ERROR_RST         = -13,   /*  Connection rese */
    SOC_ERROR_CLSD        = -14,   /* Connection closed */
    SOC_ERROR_ARG         = -15,   /* TIllegal argument */
    SOC_ERROR_IF          = -16,   /* Low-level netif error */
    SOC_ERROR             = -20,    /* error */
	SOC_ERROR_PARAM       = -21,    /*param error */
	SOC_NONBLOCK          = -22     /*nonblock*/
} Enum_Soc_Error;

//callback
typedef struct {
    void(*callback_socket_connect)(s32 socketId, s32 errCode, void* customParam );
    void(*callback_socket_close)(s32 socketId, s32 errCode, void* customParam );
    void(*callback_socket_accept)(s32 listenSocketId, s32 errCode, void* customParam );
    void(*callback_socket_read)(s32 socketId, s32 errCode, void* customParam );
}ST_SOC_Callback;


/*****************************************************************
* Function:     Ql_SOC_Register
*
* Description:
*               This function registers callback functions for the specified socket.
*
* Parameters:
*               cb:
*                   [In] The pointer of the socket-related callback function.
*
*               customParam:
*                   [In] One customized parameter that can be passed into the callback functions.
*
* Return:
*               SOC_SUCCESS indicates register ok;
*               SOC_ERROR_PARAM indicates the param error.
*****************************************************************/
s32 Ql_SOC_Register(ST_SOC_Callback cb, void* customParam);


/*****************************************************************
* Function:     Ql_SOC_Create
*
* Description:
*               This function creates a socket. The maximum number of socket is 6.
*
* Parameters:
*               contextId:
*                   [in]PDP context id, only support 0 . 
*
*               socketType:
*                   A value of socket_type_enum.
*
* Return:
*               The socket ID, or other Error Codes. To get extended
*               information, please see Enum_Soc_Error.
* Note:
*               The user needs to call Ql_SOC_Register first, then  calls Ql_SOC_Create.
*****************************************************************/
s32     Ql_SOC_Create(u8 contextId, Socket_Type socketType);


/*****************************************************************
* Function:     Ql_SOC_Close
*
* Description:
*               This function closes a socket.
*
* Parameters:
*               socketId:
*                   A socket Id, the range is 0-5.
*
* Return:
*               This return value will be SOC_SUCCESS (0) if
*               the function succeeds; or a negative number (Error Code) will be returned.
*****************************************************************/
s32   Ql_SOC_Close(s32 socketId);


/*****************************************************************
* Function:     Ql_SOC_Connect
*
* Description:
*               This function connects to TCP server. The server
*               is specified by an IP address and a port number.
*
* Parameters:
*               socketId:
*                   A socket Id,the range is 0-5.
*
*               remoteIP:
*                   An address pointer to the string address(ipv4). for example: "114.114.114.114".
*
*               remotePort:
*                   Socket port.
* Return:
*               SOC_SUCCESS: the function succeeds.
*               SOC_NONBLOCK: this return value is for none-blocking mode. 
*                   It mean the operation is in progressing, needs to wait callback_socket_connect() for the result.
*               Other values: error code, please refer to Enum_Soc_Error.
*****************************************************************/
s32   Ql_SOC_Connect(s32 socketId, u8* remoteIP, u16 remotePort);


/*****************************************************************
* Function:     Ql_SOC_Send
*
* Description:
*               The function sends data to a connected TCP socket.
*
* Parameters:
*               socketId:
*                   Socket Id,the range is 0-5.
*
*               pData:
*                   Pointer to the data to send.
*
*               dataLen:
*                   Number of bytes to send.
* Return:
*               If no error occurs, Ql_SOC_Send returns the total
*               number of bytes sent, which can be less than the number
*               requested to be sent in the dataLen parameter.
*               Otherwise, a value of Enum_Soc_Error is returned.
*****************************************************************/
s32   Ql_SOC_Send(s32 socketId, u8* pData, s32 dataLen);


/*****************************************************************
* Function:     Ql_SOC_Recv
*
* Description:
*               The function receives data from a bound socket.
*
* Parameters:
*               socketId:
*                   A socket Id,the range is 0-5.
*
*               pBuffer:
*                   Point to a buffer that is the storage space
*                   for the received data.
*
*               bufferLen:
*                   Length of pData, in bytes.
* Return:
*               If no error occurs, Ql_SOC_Recv returns the total
*               number of bytes received. Otherwise, a value of Enum_Soc_Error is returned.
*****************************************************************/
s32   Ql_SOC_Recv(s32 socketId, u8* pBuffer, s32 bufferLen);


/*****************************************************************
* Function:     Ql_SOC_GetAckNumber
*
* Description:
*               The function gets the TCP socket ACK number.
*
* Parameters:
*               socket:
*                   [in] Socket Id,the range is 0-5.
*
*               ackNum:
*                   [out] Point to a u32 type  variable that is the storage
*                   space for the TCP ACK number
* Return:
*               If no error occurs, this return value will be SOC_SUCCESS (0). 
*               Otherwise, a value of "Enum_Soc_Error" is returned.
*****************************************************************/
s32   Ql_SOC_GetAckNumber(s32 socketId, u32* ackNum);


/*****************************************************************
* Function:     Ql_SOC_SendTo
*
* Description:
*               The function sends data to a specific destination
*               through UDP socket.
*
* Parameters:
*               socketId:
*                   [in] Socket Id to send to. The range is 0-5.
*
*               pData:
*                   [in] Buffer containing the data to be transmitted.
*
*               dataLen:
*                   [in] Length of the data in pData, in bytes.
*
*               remoteIP:
*                   [in] Point to the string address of the target socket.
*
*               remotePort:
*                   [in] The target port number.
* Return:
*               If no error occurs, this function returns the number
*               of bytes actually sent. Otherwise, a value of Enum_Soc_Error is returned.
*****************************************************************/
s32   Ql_SOC_SendTo(s32 socketId, u8* pData, s32 dataLen, u8* remoteIP, u16 remotePort);


/*****************************************************************
* Function:     Ql_SocketRecvFrom
*
* Description:
*               The function receives a datagram data through TCP socket.
*
* Parameters:
*               socketId:
*                   [in] Socket Id to receive from. The range is 0-5.
*
*               pBuffer:
*                   [out] Buffer for the incoming data.
*
*               recvLen:
*                   [out] Length of pData, in bytes.
*
*               remoteIP:
*                   [out] An IP address comes from the server, for example: "114.114.114.114".
*
*               remotePort:
*                   [out] A  port comes from the server.
*
* Return:
*               If no error occurs, this function returns the number
*               of bytes received. Otherwise, a value of Enum_Soc_Error is returned.
*****************************************************************/
s32   Ql_SOC_RecvFrom(s32 socketId, u8* pBuffer, s32 recvLen, u8* remoteIP, u16* remotePort);


/*****************************************************************
* Function:     Ql_IpHelper _GetIPByHostName
*
* Description:
*               The function retrieves host IP corresponding to a host name.
*
* Parameters:
*               contextId:
*                   [in]PDP context id, only support 0 .
*
*               hostName:
*                   [in] The host name.
*
*               requestId:
*	             [Out] Embedded in response message.
*
*               errCode:	
*                   [Out] Error code if fail
*
*               ipAddrCnt: 	
*                   [Out] Get address number.
*
*               ipAddr:
*                   [Out] The host IPv4 address.
*
*               callback_GetIpByName:
*                   [in] This callback is called by Core System to notify
*                   whether this function retrieves host IP successfully or not.
*
* Return:
*               If no error occurs, this function returns SOC_SUCCESS (0).
*               Otherwise, a value of Enum_Soc_Error is returned.
*****************************************************************/
typedef void (*Callback_IpHelper_GetIpByName)(u8 contexId, s32 errCode,  u32 ipAddrCnt, u32* ipAddr);
s32 Ql_IpHelper_GetIPByHostName(u8 contextId, 
                                u8 *hostName, 
                                Callback_IpHelper_GetIpByName  callback_GetIpByName);

/*****************************************************************
* Function:     Ql_SocketCheckIp
*
* Description:
*               This function checks  addressstring is domain name or IP address
*
* Parameters:
*               addressstring:
*                   [in]  address string.
*
* Return:
*              TRUE: IP, FALSE:domain name.
* Note:
*    if return FALSE, the user needs to call "Ql_IpHelper_GetIPByHostName" for domain name resolution.
*****************************************************************/
bool  Ql_SocketCheckIp(u8* addressstring);

#endif //__QL_TCPIP_H__
