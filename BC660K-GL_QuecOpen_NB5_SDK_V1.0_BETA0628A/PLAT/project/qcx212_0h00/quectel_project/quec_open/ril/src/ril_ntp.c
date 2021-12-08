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
 *   ril_ntp.c 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   The module implements NTP related APIs.
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
	
#include "ril_ntp.h"
#include "ril.h"
#include "ril_util.h"
#include "ql_error.h"

static CB_NTPCMD     callback_NTPCMD = NULL;

void OnURCHandler_NTPCMD(const char* strURC,  u32 Len)
{
	char urcHead[] = "\r\n+QNTP:\0";
        
 	if ( NULL != callback_NTPCMD )
 	{
		if( Ql_StrPrefixMatch(strURC, urcHead) )
		{
			callback_NTPCMD((char*)strURC);
		}
	}
}

s32 RIL_NTP_START(u8 *server_addr, u16 server_port, CB_NTPCMD NTP_Callback)
{
	s32 ret = RIL_AT_FAILED;
	char strAT[200]; 

    if (server_addr == NULL)
    {
        return RIL_AT_INVALID_PARAM;
    }

    callback_NTPCMD = NTP_Callback;
    
	memset( strAT, 0, sizeof(strAT) );
	if(0 != server_port)
	{
		sprintf( strAT, "AT+QNTP=0,\"%s\",%d\0", server_addr, server_port);
	}else{
		sprintf( strAT, "AT+QNTP=0,\"%s\"\0", server_addr);
	}
	ret = Ql_RIL_SendATCmd( strAT, strlen(strAT), NULL, NULL, 0 ) ;
    
    return ret;
}

