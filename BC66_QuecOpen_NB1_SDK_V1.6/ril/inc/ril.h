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
 *   ril.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   The file intends for QuecOpen RIL. Declare the APIs for RIL initialization,
 *   send AT command and register URC callback.
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
#ifndef __RIL_H__
#define __RIL_H__
#include "ql_type.h"
/******************************************************************************
* Module URC definition
******************************************************************************/
typedef enum {
    /****************************************/
    /* System URC definition begin          */
    /****************************************/
    URC_SYS_BEGIN = 0,
    URC_SYS_INIT_STATE_IND,     // Indication for module initialization state during boot stage, see also "Enum_SysInitState".
    URC_SIM_CARD_STATE_IND,     // Indication for SIM card state (state change), see also "Enum_SIMState".
    URC_EGPRS_NW_STATE_IND,     // Indication for EGPRS network state (state change), see also "Enum_NetworkState".
    URC_CFUN_STATE_IND,         // Indication for CFUN state, see also "Enum_CfunState".
    URC_NEW_SMS_IND,            // Indication for new short message.
	URC_ALARM_RING_IND,		    // Indication for clock alarm
	URC_SOCKET_OPEN,            // Indication for open a network for tcp/udp client.
	URC_SOCKET_CLOSE,           // Indication for close socket from server
	URC_PSM_EVENT,              //Indication for psm event
	URC_MQTT_OPEN,              // Indication for open a network for MQTT client.
	URC_MQTT_CONN,              // Indication for requests a connection to MQTT server.
	URC_MQTT_SUB,               // Indication for subscribe one or more topics.
	URC_MQTT_PUB,               // Indication for publish messages to a servers.
	URC_MQTT_TUNS,              // Indication for unsubscribe  one or more topics.
	URC_MQTT_STATE,             // Indicate State Change in MQTT Link Layer.
	URC_MQTT_CLOSE,             // Indication for Close a Client for MQTT Client.
	URC_MQTT_DISC,              // Indication for Disconnect a Client from MQTT Server.
	URC_LWM2M_REG,              // Indication for send a register request to IoT platform.
	URC_LWM2M_UPDATE,           // Indication for send an update request to the IoT platform. 	
	URC_LWM2M_DEREG,            // Indication for send a deregister request to the IoT platform.
	URC_LWM2M_STATUS,           // Indication for query current lwm2m status.
	URC_LWM2M_NOTIFY,           // Indication for notify result.
	URC_LWM2M_PING,             // Indication for notify periodic ping result.
	URC_LWM2M_OBSERVE_REQ,      // Indication for observe request from server. 	
	URC_LWM2M_OBSERVE_RSQ,		// Indication for observe response from the module.
	URC_LWM2M_READ_REQ,         // Indication for read request from server.
	URC_LWM2M_READ_RSQ,			// Indication for read response from the module.
	URC_LWM2M_WRITE_REQ, 		// Indication for write request from server.
	URC_LWM2M_WRITE_RSQ, 		// Indication for write response from the module.
	URC_LWM2M_EXECUTE_REQ,      // Indication for execute request from server.
	URC_LWM2M_EXECUTE_RSQ,		// Indication for execute response from the module.
	URC_LWM2M_CON_REPORT,		// Indication for con data report.
	URC_SSL_CONFIG,             // Indication for configure parameters of a ssl context.
	URC_SSL_OPEN,				// Indication for open a ssl socket to connect to a remote server.
	URC_SSL_CLOSE,				// Indication for close a ssl connection.
	URC_SSL_DISC,				// Indication for SSL connection disconnected

    URC_SYS_END = 100, 
    /*****************************************/
    /* System URC definition end             */
    /*****************************************/
    URC_END
}Enum_URCType;

/******************************************************************************
* Define URC struct
******************************************************************************/
typedef struct {
    u32   urcType;
    void* urcResult;
}ST_URC, *PST_URC;

/******************************************************************************
* Define URC Handle struct
******************************************************************************/
#define  RIL_MAX_URC_PREFIX_LEN    50
#define  RIL_MAX_AT_HEAD_LEN       100
typedef struct {
    char  keyword[RIL_MAX_URC_PREFIX_LEN];
    void  (* handler)(const char* strURC, void* reserved);
}ST_URC_HDLENTRY;

/******************************************************************************
* Define the return value of AT callback function.
* RIL_ATRSP_FAILED,  means that the AT command executed failed. 
* RIL_ATRSP_SUCCESS, means that the AT command executed successfully.
* RIL_ATRSP_CONTINUE,means that continue to wait for the response of AT command.
* if return RIL_ATRSP_CONTINUE,the "Ql_RIL_SendATCmd" function will be blocked, 
* until you return to RIL_ATRSP_FAILED or RIL_ATRSP_SUCCESS.
******************************************************************************/
typedef enum {
    RIL_ATRSP_FAILED   =  -1,
    RIL_ATRSP_SUCCESS  =   0,
    RIL_ATRSP_CONTINUE =   1,
}Enum_ATRspError;

/******************************************************************************
* Define the return value of Ql_RIL_SendATCmd.
* RIL_AT_SUCCESS,send AT successfully.
* RIL_AT_FAILED, send AT failed.
* RIL_AT_TIMEOUT,send AT timeout.
* RIL_AT_BUSY,   sending AT.
* RIL_AT_INVALID_PARAM, invalid input parameter.
* RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
* and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
typedef enum {
    RIL_AT_SUCCESS           =  0,
    RIL_AT_FAILED            = -1,
    RIL_AT_TIMEOUT           = -2,
    RIL_AT_BUSY              = -3,
    RIL_AT_INVALID_PARAM     = -4,
    RIL_AT_UNINITIALIZED     = -5,
}Enum_ATSndError;

/******************************************************************************
* Define socekt recv param
******************************************************************************/
#define RIL_AT_HEAD_LENGTH    (200)  //Maximum length of AT without buffer length

/******************************************************************************
* Define socekt recv param
******************************************************************************/
typedef struct{
	u8 contextID;	// context ID, range is 1-3
    u8 connectID;   //Socket service index. The range is 0-4.
	u8* service_type;//0: Start a TCP connection as a client,1: Start a UDP connection as a client 			   
	u8* address;  //IP address or domain name.
	u32 remote_port;  // The port of the remote server, range is 1-65535
	u32 local_port;   //The local port, range is 1-65535,if <local_port> is 0, then the local port will be assigned automatically,	else the local port is assigned as specified
	u8 access_mode;//0: Buffer access mode, 1: Direct push mode
	u8 socket_state; // socket service state. The range is 0-3.
	u32 error_no;    //error number.
}Socket_Urc_Param_t;

/******************************************************************************
* Define lwm2m param
******************************************************************************/
typedef struct{
	u32 msgid;	// message id
	u32 obj_id; //object id
	u32 ins_id; //instance id.
	s32 res_id;  //Resources id.
	u32 res_num; //resource number.
	bool observe_flag;	 //Indicates whether or not to observe.0  Observe	1  Cancel observe
	u8	result_code;   //result code
	u8	error_code;    //error code
	u8 update_status;  //update status.  
}Lwm2m_Urc_Param_t;

/******************************************************************************
* Define mqtt param
******************************************************************************/
#define MQTT_MAX_TOPIC    (9)  //Users can configure other values
typedef struct{
  u32 connectid; //MQTT socket identifier. The range is 0-5.
  u32 msgid;     //msgid
  s32 result;    //Result of the command execution.
  u8 mqtt_state; //mqtt status.
  u8 connect_code;//Connect return code
  u32 pub_value;    //If <result> is 1, it means the times of packet retransmission.  If <result> is 0 or 2, it will not be presented
  u32 sub_value[MQTT_MAX_TOPIC];//If <result> is 0, it is a vector of granted QoS levels. if  the value is 128, indicating that the subscription was rejected by the server. 
                                //If <result> is 1, it means the times of packet retransmission.
                                //If <result> is 2, it will not be presented.
}MQTT_Urc_Param_t;

typedef struct{
  u8 contextid;       	//SSL context index. The range is 1-3.
  u8 connectid;  		//SSL connect index. The range is 0-5.
  u8 seclevel;   		//authentication mode.
  u8 send_data_format; 	//the format of the sent data.
  u8 recv_data_format; 	//the format of the received data.
  u32 timeout;    		//timeout value of connection or message delivery.The range is 10-300s, the default value is 90s.
  u8 debug_level; 		//the printable debug log level. 
  u32 checksum;			//number of certficate bytes entered.
  u8 param_item[20];	//optional parameters for SSL functionalities.
  s8 err;				//the result of connection. 0 indicates  successful operation and other value indicates an error.
}SSL_Urc_Param_t;

/******************************************************************************
* Function:     Ql_RIL_Initialize
*  
* Description:
*               This function initializes RIl-related functions.
*               Set the initial AT commands, please refer to "m_InitCmds".
* Parameters:    
*               None.
* Return:  
*               None.
******************************************************************************/
extern void Ql_RIL_Initialize(void);


/******************************************************************************
* Function:     Ql_RIL_SendATCmd
*  
* Description:
*                This function implements sending AT command with the result  
*                being returned synchronously.The response of the AT command 
*                will be reported to the callback function,you can get the results
*                you want in it.
*
* Parameters:    
*                atCmd:     
*                     [in]AT command string.
*                atCmdLen:  
*                     [in]The length of AT command string.
*                atRsp_callBack: 
*                     [in]Callback function for handle the response of the AT command.
*                userData:  
*                     [out]Used to transfer the customer's parameter.
*                timeOut:   
*                     [in]Timeout for the AT command, unit in ms. If set it to 0,
*                         RIL uses the default timeout time (3min).
*
* Return:  
*                RIL_AT_SUCCESS,succeed in executing AT command, and the response is OK.
*                RIL_AT_FAILED, fail to execute AT command, or the response is ERROR.
*                               you may call Ql_RIL_AT_GetErrCode() to get the 
*                               specific error code.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
typedef s32 (*Callback_ATResponse)(char* line, u32 len, void* userData);
extern s32 Ql_RIL_SendATCmd(char*  atCmd, u32 atCmdLen, Callback_ATResponse atRsp_callBack, void* userData, u32 timeOut);

/*****************************************************************
* Function:     Ql_RIL_WriteDataToCore
*
* Description:
*			This function is used for send data to core, only used for datamode.
*
* Parameters:
*			pdata:
*			[in] Pointer to data to be send.
*
*			pdata_len:
*			[in] The length of the data to be send.
*
* Return Value:
*     		 0,   //send data succeed. 
*			-1,	//input parameter is not correct. 
*			-2,  //no free channel can be assigned. 
*			-3, 	//the MUX channel is not found. 
*			-4, 	//the MUX channel is not ready. 
*****************************************************************/
s32 Ql_RIL_WriteDataToCore(u8* pdata, u32 pdata_len);

/******************************************************************************
* Function:     Ql_RIL_AT_GetErrCode
*  
* Description:
*                This function retrieves the specific error code after executing AT failed.
*
* Parameters:    
*                None.
*
* Return:  
*                0, succeed in executing AT
*                -1, fails to execute AT
*                other value indicates the specific error code when executing AT failed.
*                you can usually find the meaning of error code in ATC document.
******************************************************************************/
extern s32 Ql_RIL_AT_GetErrCode(void);



#define NUM_ELEMS(x) (sizeof(x)/sizeof(x[0]))
#define FREE_MEM(x) {if (x) {void* ptr = (void*)(x); Ql_MEM_Free(ptr); ptr = NULL;}}

#endif  //__RIL_H__

