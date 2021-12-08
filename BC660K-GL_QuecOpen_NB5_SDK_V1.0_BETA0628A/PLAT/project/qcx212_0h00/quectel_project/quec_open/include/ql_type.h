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
 *   ql_type.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   type API defines.
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
#ifndef __QL_TYPE_H__
#define __QL_TYPE_H__


#include <stdbool.h>
#include <stdint.h>


#ifndef FALSE
#define FALSE    0
#endif

#ifndef TRUE
#define TRUE     1
#endif

#ifndef NULL
#define NULL    ((void *) 0)
#endif


/****************************************************************************
 * Type Definitions
 ***************************************************************************/
typedef unsigned char       ubool;
typedef          char       ascii;
typedef unsigned char       u8;
typedef signed   char       s8;
typedef unsigned short      u16;
typedef          short      s16;
typedef unsigned long        u32;
typedef          long        s32;
typedef unsigned long long  u64;
typedef          long long  s64;
typedef unsigned int        ticks;


#endif

