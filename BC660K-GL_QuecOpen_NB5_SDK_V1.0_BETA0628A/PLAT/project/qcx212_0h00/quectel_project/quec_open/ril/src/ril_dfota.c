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
 *   ril_dfota.c 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   The module implements dfota related APIs.
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
#include "ril_dfota.h"
#include "ril_system.h"
#include "ril.h"
#include "ql_type.h"
#include "cmsis_os2.h"
#include "ql_type.h"
#include "ql_dbg.h"

#define DEBUG_ENABLE 1
#if DEBUG_ENABLE > 0
#define DEBUG_PORT  PORT_DBG_LOG
#define DBG_BUF_LEN   512
static char DBG_BUFFER[DBG_BUF_LEN];
#define RIL_DFOTA_DEBUG(FORMAT,...) {\
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
#define RIL_DFOTA_DEBUG(FORMAT,...) 
#endif


void ATResp_DFOTA_Handler(char* line, u32 len, void* userData)
{
    RIL_DFOTA_DEBUG("[ATResp_DFOTA_Handler] [%s].-->\r\n",line);
}

s32 RIL_DFOTA_Upgrade(u8* url)
{

   s32 ret = RIL_AT_SUCCESS;
   char strAT[255] ;

    memset(strAT,0, sizeof(strAT));
    sprintf(strAT,"AT+QFOTADL=\"%s\"\0",url);
   
    ret = Ql_RIL_SendATCmd(strAT,strlen(strAT), ATResp_DFOTA_Handler,NULL,0);
    
    RIL_DFOTA_DEBUG("<-- Send AT:%s,ret=(%d) -->\r\n",strAT,ret);
    RIL_DFOTA_DEBUG("RIL_DFOTA_Upgrade,ret:[%d]",ret);
    
    if(RIL_AT_SUCCESS != ret)
    {
	  RIL_DFOTA_DEBUG("<-- Dfota failed  -->\r\n");
    }
    return ret;
}

void DFOTA_Analysis(u8* buffer,Dfota_Upgrade_State* upgrade_state, s32* dfota_errno)
{
	u8 strTmp[200];
	 
	memset(strTmp, 0x0,	sizeof(strTmp));
	QSDK_Get_Str((char *)buffer,(char *)strTmp,1);

    if(memcmp(strTmp,"\"HTTPSTART\"",strlen("\"HTTPSTART\"")) == 0)
	{
        *upgrade_state = DFOTA_START;		
	}
	else if (memcmp(strTmp,"\"DOWNLOAD START\"",strlen("\"DOWNLOAD START\"")) == 0)
	{
        *upgrade_state = DFOTA_START;		
	}	
	else if(memcmp(strTmp,"\"DOWNLOADING\"",strlen("\"DOWNLOADING\"")) == 0)
	{
		*upgrade_state = DFOTA_DOWNLOADING;
	}
	else if(memcmp(strTmp,"\"HTTPEND\"",strlen("\"HTTPEND\"")) == 0)
	{
		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str((char *)buffer,(char *)strTmp,2);
		*dfota_errno = atoi((char *)strTmp);
		if(*dfota_errno == 0)
		{
			*upgrade_state = DFOTA_DOWNLOAD_END;
		}
		else 
		{
			*upgrade_state = DFOTA_DOWNLOAD_FAILED;
		}
	}
	else if (memcmp(strTmp,"\"DOWNLOAD\"",strlen("\"DOWNLOAD\"")) == 0)
	{
        memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str((char *)buffer,(char *)strTmp,2);
		*dfota_errno = atoi((char *)strTmp);
		if(*dfota_errno == 0)
		{
			*upgrade_state = DFOTA_DOWNLOAD_END;
		}
		else 
		{
			*upgrade_state = DFOTA_FAILED;
		}		
	}
	else if(memcmp(strTmp,"\"START\"",strlen("\"START\"")) == 0)
	{
			*upgrade_state = DFOTA_UPGRADE_START;		
	}
	else if(memcmp(strTmp,"\"END\"",strlen("\"END\"")) == 0)
	{
		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str((char *)buffer,(char *)strTmp,2);
		*dfota_errno = atoi((char *)strTmp);

		if(*dfota_errno == 0)
		{
			*upgrade_state = DFOTA_FINISH;
		}
		else 
		{
			*upgrade_state = DFOTA_FAILED;
		}
	}
}

void Dfota_Upgrade_States(Dfota_Upgrade_State state, s32 errno)
{
    switch(state)
    {
        case DFOTA_START:
             RIL_DFOTA_DEBUG("<-- DFota start-->\r\n");
            break;
        case DFOTA_DOWNLOADING:
             RIL_DFOTA_DEBUG("<-- DFota downloading  file.-->\r\n");
            break;
        case DFOTA_DOWNLOAD_END:
             RIL_DFOTA_DEBUG("<-- DFota downloading  file finish.-->\r\n");
            break;
        case DFOTA_DOWNLOAD_FAILED:
             RIL_DFOTA_DEBUG("<-- DFota download file failed(%d).-->\r\n", errno);
            break;
        case DFOTA_UPGRADE_START:
             RIL_DFOTA_DEBUG("<-- DFota upgrade start.-->\r\n");
            break;    
        case DFOTA_FINISH:  
             RIL_DFOTA_DEBUG("<--DFota Finish.-->\r\n");
             break;
        case DFOTA_FAILED:  
             RIL_DFOTA_DEBUG("<--DFota failed(%d)!!-->\r\n",errno);
             break;
        default:
            break;
    }
}

