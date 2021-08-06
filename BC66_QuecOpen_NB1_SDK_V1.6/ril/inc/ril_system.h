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
 *   ril_system.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   The file is for QuecOpen RIL sytem definitions and APIs.
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
#ifndef __RIL_SYSTEM_H__
#define __RIL_SYSTEM_H__

#define PSM_EVENT   (1)

typedef enum {
    ENTER_PSM = 0,
    EXIT_PSM  = 1,
    END_PSM
}Enum_PSM_State;

typedef enum {
    RAI_NO_INFORMATION = 0,
    RAI_ONLY_UPLOAD_DATA,
    RAI_UPLOAD_WITH_ACK
}Enum_RAI_Config;

/*****************************************************************
* Function:     RIL_GetFirmwareVer
* 
* Description:
*               This function get the firmware version.
*
* Parameters:       
*               version:
*                   [Out] buffer to store the firmware version. 
*
* Return:        
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
*****************************************************************/
s32 RIL_GetFirmwareVer(char* version);

/*****************************************************************
* Function:     RIL_GetIMEI
* 
* Description:
*               This function retrieves the IMEI number of module.
*
* Parameters:       
*               imei:
*                   [Out] buffer to store the imei number. The length 
*                         of buffer should be at least 15-byte.
* Return:        
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
*****************************************************************/
s32 RIL_GetIMEI(char* imei);

/*****************************************************************
* Function:     RIL_QNbiotRai 
* 
* Description:
*                This function is used to set the NB-IoT release assistance indication
*
* Parameters:       
*               event:
*                   [Int]Indicate the report event,now only supports PSM state(1).
* Return:        
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
*****************************************************************/
s32 RIL_QNbiotRai(Enum_RAI_Config rai);

/*****************************************************************
* Function:     RIL_QNbiotEvent_Enable 
* 
* Description:
*                Enable NB-IoT Related Event Report
*
* Parameters:       
*               event:
*                   [Int]Indicate the report event,now only supports PSM state(1).
* Return:        
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
*****************************************************************/
s32 RIL_QNbiotEvent_Enable(u32 event);


/*****************************************************************
* Function:     RIL_QNbiotEvent_Enable 
* 
* Description:
*                Disable NB-IoT Related Event Report
*
* Parameters:       
*               event:
*                   [Int]Indicate the report event,now only supports PSM state(1).
* Return:        
*                RIL_AT_SUCCESS,send AT successfully.
*                RIL_AT_FAILED, send AT failed.
*                RIL_AT_TIMEOUT,send AT timeout.
*                RIL_AT_BUSY,   sending AT.
*                RIL_AT_INVALID_PARAM, invalid input parameter.
*                RIL_AT_UNINITIALIZED, RIL is not ready, need to wait for MSG_ID_RIL_READY
*                                      and then call Ql_RIL_Initialize to initialize RIL.
*****************************************************************/
s32 RIL_QNbiotEvent_Disable(u32 event);


/*****************************************************************
* Function:     QSDK_Get_Str 
* 
* Description:
*               This function get strings based on the location of comma.
* Parameters:
*               None.
* Return:        
*               
*
*eg:src_string="GPRMC,235945.799,V,,,,,0.00,0.00,050180,,,N"
*index =1 ,dest_string="235945.799"; return TRUE
*index =,return FALSE
*****************************************************************/
bool QSDK_Get_Str(char *src_string,  char *dest_string, unsigned char index);
#endif  //__RIL_SYSTEM_H__

