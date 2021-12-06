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
 *   ril.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   The module defines the information, and APIs related to RIL.
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
	URC_LwM2M_OBSERVE,			// Indication for recv observe
	URC_LwM2M_DATA_STATUS,		// Indication for CON message sending status
	URC_MQTT_OPEN,              // Indication for open a network for MQTT client.
	URC_MQTT_CONN,              // Indication for requests a connection to MQTT server.
	URC_MQTT_SUB,               // Indication for subscribe one or more topics.
	URC_MQTT_PUB,               // Indication for publish messages to a servers.
	URC_MQTT_TUNS,              // Indication for unsubscribe  one or more topics.
	URC_MQTT_STATE,             // Indicate State Change in MQTT Link Layer.
	URC_MQTT_CLOSE,             // Indication for Close a Client for MQTT Client.
	URC_MQTT_DISC,              // Indication for Disconnect a Client from MQTT Server.

	URC_PSM_EVENT,				// Indication for psm event
    URC_SYS_END = 100,
    /*****************************************/
    /* System URC definition end             */
    /*****************************************/
    URC_END
}Enum_URCType;
	
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
    void  (* handler)(const char* strURC, u32 Len);
}ST_URC_HDLENTRY;

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
	RIL_AT_RSPNULL			 = -6,
}Enum_ATSndError;


typedef struct {
    unsigned int  message;
    unsigned int  param1;
    unsigned int  param2;
} ST_MSG;


#define     MSG_ID_USER_START       0x1000
#define     MSG_ID_RIL_READY        (MSG_ID_USER_START + 1)
#define     MSG_ID_URC_INDICATION   (MSG_ID_USER_START + 2)
#define     MSG_ID_APP_START         (MSG_ID_USER_START + 3)


#define MAINTASK_QUEUE_LEN	8

/******************************************************************************
* Function: 	Ql_RIL_Initialize
*  
* Description:
*				This function initializes RIl-related functions.
*				Set the initial AT commands, please refer to "m_InitCmds".
* Parameters:	 
*				None.
* Return:  
*				None.
******************************************************************************/
extern void Ql_RIL_Initialize(void);


/******************************************************************************
* Function: 	Ql_RIL_SendATCmd
*  
* Description:
*				 This function implements sending AT command with the result  
*				 being returned synchronously.The response of the AT command 
*				 will be reported to the callback function,you can get the results
*				 you want in it.
*
* Parameters:	 
*				 atCmd: 	
*					  [in]AT command string.
*				 atCmdLen:	
*					  [in]The length of AT command string.
*				 atRsp_callBack: 
*					  [in]Callback function for handle the response of the AT command.
*				 userData:	
*					  [out]Used to transfer the customer's parameter.
*				 timeOut:	
*					  [in]Timeout for the AT command, unit in ms. If set it to 0,
*						  RIL uses the default timeout time (3min).
*
* Return:  
*				 RIL_AT_SUCCESS,succeed in executing AT command, and the response is OK.
*				 RIL_AT_FAILED, fail to execute AT command, or the response is ERROR.
*				 RIL_AT_TIMEOUT,send AT timeout.
*				 RIL_AT_BUSY,	sending AT.
*				 RIL_AT_INVALID_PARAM, invalid input parameter.
*				 RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*									   and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
typedef void (*Callback_ATResponse)(char* line, u32 len, void* userData);
extern s32 Ql_RIL_SendATCmd(char*  atCmd, u32 atCmdLen, Callback_ATResponse atRsp_callBack, void* userData, u32 timeOut);


void OnURCHandler(const char* strURC, u32 Len);

#define NUM_ELEMS(x) (sizeof(x)/sizeof(x[0]))
#define FREE_MEM(x) {if (x) {void* ptr = (void*)(x); free(ptr); ptr = NULL;}}


#endif  //__RIL_H__

