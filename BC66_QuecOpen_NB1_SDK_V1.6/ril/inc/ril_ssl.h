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
 *   ril_ssl.h
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   The module implements SSL related APIs.
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
#ifndef __RIL_SSL_H__
#define __RIL_SSL_H__

#include "ql_type.h"
#include "ril.h"

#define QSSL_SEND_DATA_MAX_LEN     (1460) //Maximum length of  total topic information.
#define QSSL_DATAMODE_MAX_LEN      (3*1024) // SSL datamode max length
#define QSSL_HOSTNAME_MAX_LEN      (150) //hostname length.

//Authentication mode.
typedef enum{
    Auth_Mode_None,				// No authentication
    Auth_Mode_Server,			// Manage server authentication
    Auth_Mode_Server_and_Client	//Manage server and client authentication if requested by the remote server
}Enum_SSL_Auth_Mode;

//Data format.
typedef enum{
    Data_Format_Text,	//text format
	Data_Format_Hex		//hex format
}Enum_SSL_Data_Format;

//The printable debug log level.
typedef enum{
    Debug_Level_No_Log,		//no debug log
	Debug_Level_Error_Log,	//error debug log
	Debug_Level_State_Log,	//state debug log
	Debug_Level_Info_Log,	//info debug log
	Debug_Level_Detail_Log	//detail debug log
}Enum_SSL_Debug_Log_Level;

//Transferring mode of SSL connection
typedef enum{
    Transfer_Mode_Non_Transparent,	//non-transparent mode
	Transfer_Mode_Transparent		//transparent mode 
}Enum_SSL_Transfer_Mode;

//Parameter item
typedef enum{
	Param_Item_None,		//query the setting of the context.
    Param_Item_Seclevel,	//authentication mode.
	Param_Item_Dataformat,	//dataformat.
	Param_Item_Timeout,		//Timeout value of connection or message delivery.
	Param_Item_Debug,		//The printable debug log level.
	Param_Item_Cacert,		//the content of trusted CA certificate in PEM format.
 	Param_Item_Clientcert,	//the content of client certificate in PEM format
	Param_Item_Clientkey	//the content of client private key in PEM format
}Enum_SSL_Param_Item;

//configure select flag
typedef enum{
    Cfg_Select_Query_Mode,	//query the specified parameter value
	Cfg_Select_Config_Mode	//set value of the specified parameter
}Enum_SSL_Cfg_Select;

typedef struct {
    void* pbuffer;
    u32 length;
}ST_SSL_Datamode_Userdata_t;

//SSL connection parameters
typedef struct{
	u8 contextid;		  //SSL context index. The range is 1-3.
	u8 connectid;		  //SSL connect index. The range is 0-5.
	u8* host_name;		  //IP address or URL of SSL server.
	u32 port;			  //Port number of remote server.
	Enum_SSL_Auth_Mode seclevel;		  //authentication mode.
	Enum_SSL_Data_Format send_data_format;  //the format of the sent data.
	Enum_SSL_Data_Format recv_data_format;  //the format of the received data.
	u32 timeout;		  //timeout value of connection or message delivery.The range is 10-300s, the default value is 90s.
	Enum_SSL_Debug_Log_Level debug_level; 	  //the printable debug log level. 
	Enum_SSL_Transfer_Mode connect_mode;	  //Transferring mode of SSL connection.
	Enum_SSL_Param_Item cfg_param;		//parameter item wanted to configure.
	u32 checksum;		  //number of certficate bytes entered.
}ST_SSL_Param_t;

/******************************************************************************
* Function:     RIL_QSSL_CFG
*
* Description:
*                The function is used to configure optional parameters for SSL functionalities.
*
* Parameters:
*               ssl_param_t:
*                   [in]
*                   SSL optional parameters, please refer to ST_SSL_Param_t.
*
*		   config_flag:
*			  [in]
*			  query the specified parameter value or set value of specified parameter, refer to Enum_SSL_Cfg_Select.
*               
* Return:
*                RIL_AT_SUCCESS, or other Error Codes.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QSSL_CFG(ST_SSL_Param_t* ssl_param_t, Enum_SSL_Cfg_Select config_flag,ST_SSL_Datamode_Userdata_t *pdata);

/******************************************************************************
* Function:     RIL_QSSL_OPEN
*
* Description:
*               This function is used to open an SSL socket to connect a remote Server.
*
* Parameters:
*               ssl_param_t:
*                   [in]
*                   SSL optional parameters, please refer to ST_SSL_Param_t.
*               
* Return:
*                RIL_AT_SUCCESS, or other Error Codes.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QSSL_OPEN(ST_SSL_Param_t* ssl_param_t);

/******************************************************************************
* Function:     RIL_QSSL_SEND
*
* Description:
*               The function is used for send data through SSL connection.
*
* Parameters:
*               contextID:
*                   [in]
*                   SSL context index. The range is 1-3.
*
*               connectID:
*                   [in]
*                   SSL connect index. The range is 0-5.
*
*               pdata:
*			  [in]
*			  The attributes of the data to be sent, include data and the length of data.
*			  The range is 1-1460 in Text format and 1-730 in Hex format.
*
* Return:
*                RIL_AT_SUCCESS, or other Error Codes.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QSSL_SEND(u8 contextID, u8 connectID, ST_SSL_Datamode_Userdata_t *pdata);

/******************************************************************************
* Function:     RIL_QSSL_CLOSE
*
* Description:
*               The function is used to close an SSL connection. 
*		    If all SSL connections based on the same SSL context are closed, the module will release the SSL context.
*
* Parameters:
*               contextID:
*                   [in]
*                   SSL context index. The range is 1-3.
*               connectID:
*                   [in]
*                   SSL connect index. The range is 0-5.
*
* Return:
*                RIL_AT_SUCCESS, or other Error Codes.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY, sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
******************************************************************************/
s32 RIL_QSSL_CLOSE(u8 contextID, u8 connectID);

#endif //__RIL_SSL_H__

