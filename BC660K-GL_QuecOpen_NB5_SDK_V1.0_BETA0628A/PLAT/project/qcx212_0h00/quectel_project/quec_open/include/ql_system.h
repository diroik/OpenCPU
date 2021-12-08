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
 *   ql_system.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *  System related APIs
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

#ifndef __QL_SYSTEM_H__
#define __QL_SYSTEM_H__

#include "ql_type.h"


/*****************************************************************
* Function:     Ql_Reset
*
* Description:
*               This function resets the system.
*
* Parameters:
*               resetType:
*                   [in]must be 0.
* Return:
*               None
*****************************************************************/
void Ql_Reset(u8 resetType);


/*****************************************************************
* Function:     Ql_GetSDKVer
*
* Description:
*               Get the version ID of the SDK.
*
* Parameters:
*               ptrVer:
*                   [out] Point to a unsigned char buffer, which is
*                           the the version ID of the SDK.
*               len:
*                   [in] A number will be compare with the length of version ID.
*
* Return:
*               The length of sdk version ID indicates success.
*               Negative indicates failure. please see Error Code Definition.
*****************************************************************/
s32  Ql_GetSDKVer(u8* ptrVer, u32 len);

#endif  // __QL_SYSTEM_H__
