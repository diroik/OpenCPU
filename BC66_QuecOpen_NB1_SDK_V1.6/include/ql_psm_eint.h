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
 *   ql_psm_eint.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *  psm_eint related APIs
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
 

#ifndef __QL_PSM_EINT_H__
#define __QL_PSM_EINT_H__
#include "ql_type.h"

/**************************************************************
 * User RTC  ID Definition
 **************************************************************/
typedef void (*Callback_Psm_Eint_Func)(void *user_data);

/*****************************************************************
* Function:     Ql_Psm_Eint_Register 
* 
* Description:
*               Register the callback of PSM_EINT
*
* Parameters:
*               Callback_Psm_Eint_Func:
*                    [out] Notify the application when the PSM_EINT event occuerd.
* Return:
*               QL_RET_OK indicates register ok;
*               QL_RET_ERR_PARAM indicates the param error.
* Notes:
*		        When the module wakes up from the Deep Sleep, calls psm_eint callback first, then initializes the QuecOpen, 
*				so the API interface of Ql cannot be called in here.
*****************************************************************/
s32 Ql_Psm_Eint_Register(Callback_Psm_Eint_Func  callback_psm_eint,void* param);


#endif  // End-of __QL_PSM_EINT_H__

