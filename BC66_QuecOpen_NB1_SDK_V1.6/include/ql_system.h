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
#include "ql_common.h"
#include "ql_type.h"
#include "ql_memory.h"
#include "ql_freertos.h"

/**************************************************************
 * User Message ID Definition
 **************************************************************/
#define     MSG_ID_USER_START       0x1000
#define     MSG_ID_RIL_READY        (MSG_ID_USER_START + 1)
#define     MSG_ID_URC_INDICATION   (MSG_ID_USER_START + 2)
#define     MSG_ID_APP_TEST         (MSG_ID_USER_START + 3)

typedef enum {
    SYS_STATE_START = 0,
    SYS_STATE_ATOK  = 1,
    SYS_STATE_PHBOK = 2,
    SYS_STATE_SMSOK = 3
}Enum_SysInitState;

typedef enum {
    CFUN_STATE_0 = 0,
    CFUN_STATE_1 = 1,
    CFUN_STATE_4 = 4
}Enum_CfunState;


/*****************************************************************
* Function:     Ql_Delay_ms
*
* Description:
*               This is a delay interface,suspends the execution of the current task
*               until the time-out interval elapses.
* Parameters:
*               msec:
*                   The time interval for which execution is to be suspended, in milliseconds.
* Return:
*               None
* Note:
*               if  users set the msec to 1, the task will also delay 10 ms, because one tick of the OS is 10 ms.
*****************************************************************/
void Ql_Delay_ms(u32 msec);

/*****************************************************************
* Function:     Ql_Delay_us
*
* Description:
*               This is a delay interface,blocks the execution of the current task
*               until the time-out interval elapses.
* Parameters:
*               usec:
*                   The time interval for which execution is to be blocked, in microseconds.
* Return:
*               None
* Note:
*              If users needs to delay for several seconds, please use Ql_Delay_ms.
*****************************************************************/
void Ql_Delay_us(u32 usec);

/*****************************************************************
* Function:     Ql_Reset
*
* Description:
*               This function resets the system.
*
* Parameters:
*               resetType:
*                   must be 0.
* Return:
*               None
*****************************************************************/
void Ql_Reset(u8 resetType);

/*****************************************************************
* Function:     Ql_GetSDKVer
*
* Description:
*               This function gets the version of the SDK.
*
* Parameters:
*               ptrVer:
*                   [out] Point  to a unsigned char buffer, which is
*                           the the version of the SDK.
*               len:
*                   [in] A number will be compare with the length of version.
*
* Return:
*               The smaller between len and the length of version ID.
*****************************************************************/
s32  Ql_GetSDKVer(u8* ptrVer, u32 len);

/*****************************************************************
* Function:     Ql_SetLastErrorCode 
* 
* Description:
*               Set error code

* Parameters:
*               errCode: Error code
* Return:        
*              TRUE indicates success or FALSE indicates failure. 
*****************************************************************/
s32 Ql_SetLastErrorCode(s32 errCode);

/*****************************************************************
* Function:     Ql_GetLastErrorCode 
* 
* Description:
*               Retrieves the calling task's last-error code value.

* Parameters:
*               None
* Return:        
*               The return value is the calling task's last-error code.
*****************************************************************/
s32 Ql_GetLastErrorCode(void);

#if 1
/*****************************************************************
* Function:     Ql_SecureData_Store 
* 
* Description:
*              This function can be used to store some critical user data 
*              to prevent them from losing.
*
*              Note:
*              1.
*              QuecOpen has designed 2 blocks of system storage space to 
*              backup critical user data. Developer may specify the first
*              parameter index [1-2] to specify different storage block. 
*              each blocks can store 2048 bytes 
*              2.
*              User should not call this API function frequently, which is not
*              good for life cycle of flash.
*
* Parameters:
*              index:
*               [in]  the index of the secure data block. The range is: 1~2.
*              
*               pData: 
*                   [in] The data to be backed up. In 1~2 groups, every group can 
*                   save 2048 bytes at most. 
*
*               len:
*                   [in] The length of the user data. The maximum of this value is 2048B.
* Return:       
*               QL_RET_OK, this function succeeds.
*             -1:An unknown error occurred,may be parameter is error.
*             -2:The NVDM found a checksum error when reading the data item
*             -3:No space is available in the flash
*             -4: The data item wasn't found by the NVDM.
              -5:The user parameter is invalid.
*               ......
*****************************************************************/
s32 Ql_SecureData_Store(u8 index , u8* pData, u32 len);

/*****************************************************************
* Function:     Ql_SecureData_Read 
* 
* Description:
*              This functin reads secure data which is previously 
*              stored by Ql_SecureData_Store.
* Parameters:
*               index:
*                   [in] The index of the secure data block. The range is: 1~2.
*
*               len:
*                   [in] The length of the user data. The maximum of this value is 2048B.
* Return:       
*               QL_RET_OK, this function succeeds.
*             -1:An unknown error occurred,may be parameter is error.
*             -2:The NVDM found a checksum error when reading the data item
*             -3:No space is available in the flash
*             -4: The data item wasn't found by the NVDM.
              -5:The user parameter is invalid.

*****************************************************************/
s32 Ql_SecureData_Read(u8 index, u8* pBuffer, u32 len);

/*****************************************************************
* Function:     Ql_Sleep
*
* Description:
*               It's  the same as "Ql_Delay_ms".
* Parameters:
*               msec:
*                   The time interval for which execution is to
*                   be suspended, in milliseconds.
* Return:
*               None
* Note:
*               if  users set the msec to 1, the task will also delay 10 ms, because one tick of the OS is 10 ms.
*****************************************************************/
void Ql_Sleep(u32 msec);
#endif 

/********************************TASK DEFINITION*************************************************/
#ifdef TASK_ENTRYFUNC_DEF
#undef TASK_ENTRYFUNC_DEF
#endif
#ifdef TASK_DEFINITION
#undef TASK_DEFINITION
#endif
#define TASK_ID_DEF
#include "ql_common.h"
TASK_DEFINITION_BEGIN
#include "custom_task_cfg.h"
TASK_DEFINITION_END
#undef TASK_ID_DEF

#endif  // End-of __QL_SYSTEM_H__
