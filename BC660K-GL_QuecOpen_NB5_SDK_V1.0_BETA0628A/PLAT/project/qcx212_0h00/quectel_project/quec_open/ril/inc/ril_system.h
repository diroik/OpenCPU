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

#include "ql_type.h"


void DefAtRsp_CallBack(char* line, u32 len, void* userData);

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

