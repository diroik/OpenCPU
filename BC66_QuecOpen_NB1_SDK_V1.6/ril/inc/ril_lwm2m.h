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
 *   ril_lwm2m.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   The file declares some API functions, which are related to lwm2m
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
#ifndef __RIL_LwM2M_H__
#define __RIL_LwM2M_H__   
#include "ql_type.h"

#define MAX_IP_HOST_NAME       (150)  ////Maximum length of host name.
#define MAX_RES_ID_NUMBER      (14)   //Maximum nuber of resource id.
#define RIL_lWM2M_LENGTH_MAX   (1400) //Maximum length of mqtt AT command.

typedef enum
{
    LWM2M_BS_MODE_DISABLE = 0,   //disable bootstrap  
    LWM2M_BS_MODE_ENABLE         //enable bootstrap 
}Enum_Lwm2m_Bs_Mode;

typedef enum
{
    LWM2M_SECURITY_MODE_0 = 0,    //Secure mode: PRE_SHARED_KEY
    LWM2M_SECURITY_MODE_3 = 3     //No security 
}Enum_Lwm2m_Security_Mode;

typedef enum
{
    LWM2M_UPDATE_MODE_LIFETIME = 0,   //lifetime
    LWM2M_UPDATE_MODE_BINDING         //binding
}Enum_Lwm2m_Update_Mode;

typedef enum
{
    LWM2M_OPTION_AUTO_REG = 0,   //Configure LwM2M auto registration enable/disable
    LWM2M_OPTION_RETRANSMIT, //Configure LwM2M retransmit time and times.
    LWM2M_OPTION_PER_REGSTATUS_FEAT,//Configure LwM2M update registration after reboot enable/disable
    LWM2M_OPTION_LOCATION,  //Configure LwM2M location object information
    LWM2M_OPTION_PORTFOLIO, //Configure LwM2M portfolio object information
    LWM2M_OPTION_ACCESS_MODE,//Configure LwM2M cache mode
    LWM2M_OPTION_END
}Enum_Lwm2m_Option_Mode;

typedef enum
{
	LWM2M_RESULT_CODE_1= 1,     //2.05 Content, indicate the correct result.
	LWM2M_RESULT_CODE_2 = 2,    //2.04 Changed, indicates the correct result
    LWM2M_RESULT_CODE_11 = 11,  //4.00 Bad Request. 
    LWM2M_RESULT_CODE_12 ,      //4.01 Unauthorized.
    LWM2M_RESULT_CODE_13,       //4.04 Not Found.
    LWM2M_RESULT_CODE_14,       //4.05 Method Not Allowed.
    LWM2M_RESULT_CODE_15        //4.06 Not Acceptable.
}Enum_Lwm2m_Result_Code;

//value type
typedef enum
{
    LWM2M_VALUE_TYPE_STRING= 1,     
    LWM2M_VALUE_TYPE_OPAQUE,        
    LWM2M_VALUE_TYPE_INTEGER,      
    LWM2M_VALUE_TYPE_FLOAT,       
    LWM2M_VALUE_TYPE_BOOLEAN      
}Enum_LwM2M_Value_Type;


//config param
typedef struct{
 u32 ins_id;   //  Instance ID.
 u8  address[MAX_IP_HOST_NAME];  //IP address or domain name.
 u32 remote_port;  // The port of the remote server, range is 1-65535
 u8* endpoint_name;   //The Devices Endpoint name.The maximum size is 150 bytes..
 u32 lifetime;     //Lifetime, unit is second. Range is 20~4294967295.
 u8* psk_id; //PSK identifier.The maximum size is 150 bytes(hex string).
 u8* psk;    //PSK content.The maximum size is 64 bytes.
 Enum_Lwm2m_Bs_Mode bs_mode;   // bootstrap disable or enable.
 Enum_Lwm2m_Security_Mode security_mode;
}ST_Lwm2m_Config_Param_t;

//add object param
typedef struct{
  u32 obj_id; //object ID; 
  u32 ins_id; //  Instance ID.
  u8 res_num; // resource indentifier number. 
  u32 res_id[MAX_RES_ID_NUMBER];
}ST_Lwm2m_object_Param_t;

//lwm2m project param.
typedef struct{
 u32 msgid;      //The message identifier, which comes from the URC "+MIPLDISCOVER:"
 u32 objid;      //Object identifier.
 u32 insid;      //The instance identifier, which comes from the URC "+MIPLOBSERVE :"
 s32 resid;      //The resource identifier, which comes from the URC "+MIPLOBSERVE :".
 u32 res_num;    //resource indentifier number.
 u32 len;        //The value length.
 u8* value;      //string 
 u32 index;      //The index number of the data,if set 0,it means that this is the last message of the data.
 u8  ack;        // ack, 0  NON type data,1  CON type data
 u8  rai;        //Apply for RRC disconnection after sending data.
 Enum_Lwm2m_Result_Code result_code; //result codes.
 Enum_LwM2M_Value_Type value_type; //The value type.
}ST_LwM2M_Param_t;  

/******************************************************************************
* Function:     RIL_QLwM2M_Cfg
*  
* Description:
*               Configure the Registration Platform Parameters
*
* Parameters:    
*               lwm2m_cfg_t:
*                   [in] 
*                     lwm2m config param,please refer to lwm2m_cfg_t.
*               psk_flag:
*                   [in] 
*                    whether to set up PSK.
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Cfg(ST_Lwm2m_Config_Param_t *lwm2m_cfg_t,bool psk_flag);


/******************************************************************************
* Function:     RIL_QLwM2M_Reg
*  
* Description:
*               send a register request.
*
* Parameters:    
*               void
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Reg(void);


/******************************************************************************
* Function:     RIL_QLwM2M_Addobj
*  
* Description:
*                   Add a LwM2M Object
*
* Parameters:    
*                 lwm2m_object_param_t:
*                   [in] 
*                   lwm2m add object param,please refer to lwm2m_object_param_t.
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Addobj(ST_Lwm2m_object_Param_t* lwm2m_object_param_t); 


/******************************************************************************
* Function:     RIL_QLwM2M_Delobj
*  
* Description:
*                   Delete a LwM2M Object
*
* Parameters:    
*               obj_id:
*                   [in] 
*                   Object id. The max object id number is 65535.
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Delobj(u32 obj_id); 


/******************************************************************************
* Function:     RIL_QLwM2M_Cfg_Autoreg
*  
* Description:
*               Configure whether to support automatic registration.
*
* Parameters:    
*               auto_reg_enable:
*                   [in] TRUE, enable auto registration.
*                         FALSE, disable auto registration.
*
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Cfg_Autoreg(bool auto_reg_enable);

/******************************************************************************
* Function:     RIL_QLwM2M_Cfg_Retransmit
*  
* Description:
*               Configure confirmable data retransmit time and period.
*
* Parameters:    
*               ack_timeout:
*                   [in] date retransmit time.
*                
*		    	retrains_max_times:
*			  		[in] date retransmit times.
*
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Cfg_Retrans(u32 ack_timeout, u8 retrains_max_times);


/******************************************************************************
* Function:     RIL_QLwM2M_Update
*  
* Description:
*                  Send an Update Request
*
* Parameters:    
*                   Enum_Lwm2m_Update_Mode:
*                   [in] 
*                   update type: 0,lifetime  1,binding mode. 
*                   param:
*                   [in] 
*                   life time,unit is second. Range is 20~4294967295.
*                   bind mode: 0,UDP  1,UDP&queue mode. 
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Update(Enum_Lwm2m_Update_Mode update_mode,u32 param); 


/******************************************************************************
* Function:     RIL_QLwM2M_Write_Rsp
*  
* Description:
*                   Respond the write Request
*
* Parameters:    
*               msgid:
*                   [in] 
*                  The message identifier.
*               result:
*                    [in]            
*                 The results of execute operation and the result codes are as follows:
*                 Result code CoAP response code Description
*                 2 2.04 Changed, indicates the correct result.
*                 11 4.00 Bad Request
*                 12 4.01 Unauthorized
*                 13 4.04 Not Found
*                 14 4.05 Method Not Allowed
*
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Write_Rsp(u32 msgid, Enum_Lwm2m_Result_Code result);


/******************************************************************************
* Function:     RIL_QLwM2M_Observe_Rsp
*  
* Description:
*                   Respond the observe Request
*
* Parameters:    
*                  lwm2m_observe_param_t:
*                  [in]
*                  lwm2m observe request param,please refer to ST_LwM2M_Param_t.
*
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Observe_Rsp(ST_LwM2M_Param_t* lwm2m_observe_param_t);


/******************************************************************************
* Function:     RIL_QONENET_Read_Rsp
*  
* Description:
*               Respond the Read Request
*
* Parameters:    
*               lwm2m_observe_param_t:
*                  [in]lwm2m observe request param,please refer to ST_LwM2M_Param_t.
*
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Read_Rsp(ST_LwM2M_Param_t* lwm2m_observe_param_t);

/******************************************************************************
* Function:     RIL_QLwM2M_Execute_Rsp
*  
* Description:
*                   Respond the execute Request
*
* Parameters:    
*               msgid:
*                   [in] 
*                    The message identifier.
*               result:
*                    [in]            
*                 The results of execute operation and the result codes are as follows:
*                 Result code CoAP response code Description
*                 2 2.04 Changed, indicates the correct result.
*                 11 4.00 Bad Request
*                 12 4.01 Unauthorized
*                 13 4.04 Not Found
*                 14 4.05 Method Not Allowed
*
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Execute_Rsp(u32 msgid,Enum_Lwm2m_Result_Code  result);


/******************************************************************************
* Function:     RIL_QONENET_Notify
*  
* Description:
*                   Notify the Data to OneNET platform or Application Server
*
* Parameters:    
*                  ST_ONENET_Notify_Param_t:
*                   [in] 
*                   onenet notify param,please refer to ST_ONENET_Notify_Param_t.
**                 ack_flag:
*                   [in] 
*                   FALSE      No need to set  ack .
*                   TRUE       set ack mode,you can set ack value 0 or 1 .
**                 rai_flag:
*                   [in] 
*                   FALSE      No need to set rai.
*                   TRUE       set rai flag, you can set ack value 0 or 1 .

* 
* Return:  .
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_LwM2M_Notify(ST_LwM2M_Param_t* lwm2m_notify_param_t,bool ack_flag,bool rai_flag);


/******************************************************************************
* Function:     RIL_QLwM2M_Dereg
*  
* Description:
*               send a deregister request to the IoT platform
*
* Parameters:    
*               void
*
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Dereg(void);


/******************************************************************************
* Function:     RIL_QLwM2M_Status
*  
* Description:
*               Query current status of Lwm2m.
*
* Parameters:    
*               status:
*                   [in] 
*                    current status of lwm2m.
*
* Return:  
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QLwM2M_Status(u8* status);

#endif // __RIL_NETWORK_H__
