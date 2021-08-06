
#ifndef __QL_URC_REGISTER_H__
#define __QL_URC_REGISTER_H__

typedef void(*Callback_Urc_Handle)(u8* buffer,u32 length);


/******************************************************************************
* Function:     Ql_Mqtt_Recv_Register
*
* Description:
*               Register callback for receiving data.When the module receives downlink data, call particular callback.
* uses need to register the callback at the starting up and wake up from deep sleep.
*
* Parameters:
*              Callback_Urc_Handle:
*                       [out] Register a callback to process downlink data.

* Return:
*                QL_RET_OK,successfully.
*                QL_RET_ERR_PARAM, Parameter error, maybe callback is NULL.
******************************************************************************/
s32 Ql_Mqtt_Recv_Register(Callback_Urc_Handle callback_mqtt_recv);


/******************************************************************************
* Function:     Ql_Lwm2m_Write_Req_Register
*
* Description:
*               Register callback for receiving data.When the module receives downlink data, call particular callback.
* uses need to register the callback at the starting up and wake up from deep sleep.
*
* Parameters:
*              callback_mqtt_recv:
*                       [out] Register a callback to process downlink data.

* Return:
*                QL_RET_OK,successfully.
*                QL_RET_ERR_PARAM, Parameter error, maybe callback is NULL.
******************************************************************************/
s32 Ql_Lwm2m_Write_Req_Register(Callback_Urc_Handle callback_lwm2m_write_req);

/******************************************************************************
* Function:     Ql_Socket_Recv_Register
*
* Description:
*               Register callback for receiving data.When the module receives downlink data, call particular callback.
* 				uses need to register the callback at the starting up and wake up from deep sleep.
*
* Parameters:
*              callback_socket_recv:
*                       [out] Register a callback to process downlink data.

* Return:
*                QL_RET_OK,successfully.
*                QL_RET_ERR_PARAM, Parameter error, maybe callback is NULL.
******************************************************************************/
s32 Ql_Socket_Recv_Register(Callback_Urc_Handle callback_socket_recv);

/******************************************************************************
* Function:     Ql_SSL_Recv_Register
*
* Description:
*               Register callback for receiving data.When the module receives downlink data, call particular callback.
* uses need to register the callback at the starting up and wake up from deep sleep.
*
* Parameters:
*              callback_ssl_recv:
*                       [out] Register a callback to process downlink data.

* Return:
*                QL_RET_OK,successfully.
*                QL_RET_ERR_PARAM, Parameter error, maybe callback is NULL.
******************************************************************************/
s32 Ql_SSL_Recv_Register(Callback_Urc_Handle callback_ssl_recv);

#endif 
