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
 *   ril_system.c 
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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ril_system.h"
#include "ql_error.h"
#include "ril.h "
#include "ql_type.h"
#include "cmsis_os2.h"
#include "ql_type.h"
#include "ql_dbg.h"

#define RIL_SYS_DBG_ENABLE 1
#if RIL_SYS_DBG_ENABLE > 0
#define DEBUG_PORT  UART_PORT2
#define DBG_BUF_LEN   512
static char DBG_BUFFER[DBG_BUF_LEN];
#define APP_DEBUG(FORMAT,...) {\
    memset(DBG_BUFFER, 0, DBG_BUF_LEN);\
    snprintf(DBG_BUFFER,DBG_BUF_LEN,FORMAT,##__VA_ARGS__); \
    if (UART_PORT2 == (DEBUG_PORT)) \
    {\
        Ql_Debug_Trace((u8* )DBG_BUFFER);\
    } else {\
		Ql_UART_Write((Enum_SerialPort)(DEBUG_PORT), (u8*)(DBG_BUFFER), strlen((const char *)(DBG_BUFFER)));\
    }\
}
#else
#define APP_DEBUG(FORMAT,...) 
#endif


void DefAtRsp_CallBack(char* line, u32 len, void* userData)
{
    APP_DEBUG("[DefAtRsp][%s].-->\r\n",line);
}

bool QSDK_Get_Str(char *src_string,  char *dest_string, unsigned char index)
{
    u32 SentenceCnt = 0;
    u32 ItemSum = 0;
    u32 ItemLen = 0, Idx = 0;
    u32 len = 0;
    unsigned int i = 0;
    
    if (src_string ==NULL)
    {
        return FALSE;
    }
    len = strlen(src_string);
	for ( i = 0; i < len; i++)
	{
		if (*(src_string + i) == ',')
		{
			ItemLen = i - ItemSum - SentenceCnt;
			ItemSum  += ItemLen;
            if (index == SentenceCnt)
            {
                if (ItemLen == 0)
                {
                    return FALSE;
                }
		        else
                {
                    memcpy(dest_string, src_string + Idx, ItemLen);
                    *(dest_string + ItemLen) = '\0';
                    return TRUE;
                }
            }
			SentenceCnt++; 	 
			Idx = i + 1;
		}		
	}
    if (index == SentenceCnt && (len - Idx) != 0)
    {
        memcpy(dest_string, src_string + Idx, len - Idx);
        *(dest_string + len) = '\0';
        return TRUE;
    }
    else 
    {
        return FALSE;
    }
}

