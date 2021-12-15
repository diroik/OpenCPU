/*
 * ril_network_time.c
 *
 *  Created on: 15 äåê. 2021 ã.
 *      Author: Àäìèí
 */
#include "ril.h"
#include "ril_util.h"
#include "ril_network_time.h"
#include "ql_stdlib.h"
#include "ql_trace.h"
#include "ql_error.h"
#include "ql_system.h"
#include "typedef.h"

static s32 ATResponse_Handler(char* line, u32 len, void* userData)
{

    if (Ql_RIL_FindLine(line, len, "OK"))
    {
        return  RIL_ATRSP_SUCCESS;
    }
    else if (Ql_RIL_FindLine(line, len, "ERROR"))
    {
        return  RIL_ATRSP_FAILED;
    }
    else if (Ql_RIL_FindString(line, len, "+CME ERROR"))
    {
        return  RIL_ATRSP_FAILED;
    }
    else if (Ql_RIL_FindString(line, len, "+CMS ERROR:"))
    {
        return  RIL_ATRSP_FAILED;
    }

    char *pHead = Ql_RIL_FindString(line, len, "+");//continue wait
    if(pHead)
    {
    	 char *pEnd = Ql_RIL_FindString(line, len, ":");
    	 if(pEnd)
    	 {
    		 s64 alen = pEnd - pHead;
    		 if(alen > 0 && userData != NULL)
    		 {
    			 Ql_memcpy((char*)userData, line, len);
    		 }
    	 }
    }
    return RIL_ATRSP_CONTINUE; //continue wait
}

static s32 ATResponse_CTZU_Status_Handler(char* line, u32 len, void* userData)
{
	char *head = Ql_RIL_FindString(line, len, "+CTZU:"); //continue wait
	APP_DEBUG("[ATResponse_CTZU_Status_Handler] line=<%s>\r\n", (u8*)line);

    if(head)
    {
        u8 *st = (u8 *)userData;
        //APP_DEBUG("[ATResponse_CTZU_Status_Handler] head=<%s>\r\n", (u8*)head);
        Ql_sscanf(head,"%*[^ ]%d,%[^\r\n]", st);
        //APP_DEBUG("[ATResponse_CTZU_Status_Handler] state=%d\r\n", *st);
        return  RIL_ATRSP_CONTINUE;
    }

    head = Ql_RIL_FindLine(line, len, "OK"); // find <CR><LF>OK<CR><LF>, <CR>OK<CR>£¬<LF>OK<LF>
    if(head)
    {
    	return  RIL_ATRSP_SUCCESS;
    }
    head = Ql_RIL_FindLine(line, len, "ERROR");// find <CR><LF>ERROR<CR><LF>, <CR>ERROR<CR>£¬<LF>ERROR<LF>
    if(head)
    {
        return  RIL_ATRSP_FAILED;
    }
    head = Ql_RIL_FindString(line, len, "+CME ERROR:");//fail
    if(head)
    {
        return  RIL_ATRSP_FAILED;
    }
    head = Ql_RIL_FindString(line, len, "+CMEE ERROR:");//fail
    if(head)
    {
        return  RIL_ATRSP_FAILED;
    }
    return RIL_ATRSP_CONTINUE; //continue wait
}

s32 RIL_GetTimeSynch_Status(u8* status)
{
    s32 retRes = -1;
    s32 nStat = 0;
    char strAT[] = "AT+CTZU?\0";

    retRes = Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_CTZU_Status_Handler, &nStat, 0);
    if(RIL_AT_SUCCESS == retRes)
    {
       *status = nStat;
    }
    return retRes;
}

s32 RIL_SetTimeSynch_Status(u8 status)
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[100];
    Ql_memset(strAT, 0x00, sizeof(strAT));

	Ql_sprintf(strAT,"AT+CTZU=%d", status);
    ret = Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_Handler, NULL, 0);

    APP_DEBUG("<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;
}

