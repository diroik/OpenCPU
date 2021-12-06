#ifndef __QL_URC_REGISTER_H__
#define __QL_URC_REGISTER_H__

typedef void(*Callback_Urc_Handle)(const char* buffer,u32 length);
/******************************************************************************
* Function:     Ql_Mqtt_Recv_Register
*
* Description:
*               Register callback for receiving data.When the module receives the urc of "+QMTRECV:", call particular callback.
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



#endif 
