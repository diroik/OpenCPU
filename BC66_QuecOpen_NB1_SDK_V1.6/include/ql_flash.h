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
 *   ql_flash.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   Flash API defines.
 *
 *  Note:
 *-------------
 *   1.
 *   QuecOpen has designed 2 blocks of system storage space to 
 *   backup critical user data. Developer may specify the first
 *   parameter index [1-2] to specify different storage block. 
 *   each blocks can store 2048 bytes 
 *   2.
 *   User should not call this API function frequently, which is not
 *   good for life cycle of flash.
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


#ifndef __QL_FLASH_H__
#define __QL_FLASH_H__
#include "ql_type.h"

/**************************************************************
 * Error Code Definition
 **************************************************************/
typedef enum {
    FLASH_SUCCESS=0,         // Success
    FLASH_ERROR_PARAM=-1,    //An unknown error occurred,may be parameter is error.
    FLASH_ERROR_CHECKSUM=-2, //The NVDM found a checksum error when reading the data item
    FLASH_ERROR_NOSPACE=-3,  //No space is available in the flash
    FLASH_ERROR_NOFOUND=-4,  //The data item wasn't found by the NVDM. Maybe the user never wrote it..
    FLASH_ERROR_INVALID=-5,  //The user parameter is invalid.
} Enum_Flash_ErrCode;



/*****************************************************************
* Function:     Ql_Flash_Write 
* 
* Description:
*              This function can be used to write user data to flash.
*
* Parameters:
*              index:
*                   [in] The index of the secure data block. The range is: 1~2, every block can 
*                   write 2048 bytes at most.
*               addr:
*                   [in] The address of start to write, the range is:0x0000~0x07FF(2KB)
*               pBuffer: 
*                   [in] The data to be written.
*               len:
*                   [in] The length of the user data. The maximum of this value is 2048.
* Return:       
*              Return value, 0 indicates success, other failures,please refer to Enum_Flash_ErrCode.
*****************************************************************/
s32 Ql_Flash_Write(u8 index ,u16 addr, u8* pBuffer, u32 len);

/*****************************************************************
* Function:     Ql_Flash_Read 
* 
* Description:
*              This functin reads flash data which is written previously by Ql_Flash_Write.
*
* Parameters:
*               index:
*                   [in] The index of the secure data block. The range is: 1~2.
*               addr:
*                    [in] The address of start to read.the range is:0x0000~0x07FF(2KB)
*               pBuffer: 
*                   [out] Save read data.
*               len:
*                   [in] The length of the user data. The maximum of this value is 2048.
* Return:       
*              Return value, 0 indicates success, other failures,please refer to Enum_Flash_ErrCode.
*****************************************************************/
s32 Ql_Flash_Read(u8 index,u16 addr, u8* pBuffer, u32 len);

#endif  // __QL_FLASH_H__

