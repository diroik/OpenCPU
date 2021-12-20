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
 *   ril_sim.c 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   The module implements sim card related APIs.
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
#include "custom_feature_def.h"
#include "ril.h"
#include "ril_util.h"
#include "ril_sim.h"
#include "ql_stdlib.h"
#include "ql_trace.h"
#include "ql_error.h"
#include "ql_system.h"
#include "ql_trace.h"
#include "typedef.h"

#ifdef __OCPU_RIL_SUPPORT__

s32 RIL_SIM_GetSimStateByName(char* simStat, u32 len)
{
    Enum_SIMState ss = SIM_STAT_UNSPECIFIED;
    if (Ql_strncmp(simStat, "READY", len) == 0)
    {
        ss = SIM_STAT_READY;
    }
    else if (Ql_strncmp(simStat, "NOT INSERTED", len) == 0)
    {
        ss = SIM_STAT_NOT_INSERTED;
    }
    else if (Ql_strncmp(simStat, "SIM PIN", len) == 0)
    {
        ss = SIM_STAT_PIN_REQ;
    }
    else if (Ql_strncmp(simStat, "SIM PUK", len) == 0)
    {
        ss = SIM_STAT_PUK_REQ;
    }
    else if (Ql_strncmp(simStat, "PH-SIM PIN", len) == 0)
    {
        ss = SIM_STAT_PH_PIN_REQ;
    }
    else if (Ql_strncmp(simStat, "PH-SIM PUK", len) == 0)
    {
        ss = SIM_STAT_PH_PUK_REQ;
    }
    else if (Ql_strncmp(simStat, "SIM PIN2", len) == 0)
    {
        ss = SIM_STAT_PIN2_REQ;
    }
    else if (Ql_strncmp(simStat, "SIM PUK2", len) == 0)
    {
        ss = SIM_STAT_PUK2_REQ;
    }
    else if (Ql_strncmp(simStat, "SIM BUSY", len) == 0)
    {
        ss = SIM_STAT_BUSY;
    }
    else if (Ql_strncmp(simStat, "NOT READY", len) == 0)
    {
        ss = SIM_STAT_NOT_READY;
    }
    return ss;
}

static s32 RIL_SIM_GetSimStateByErrCode(s32 errCode)
{
    Enum_SIMState ss;
    switch (errCode)
    {
    case 10:
        ss = SIM_STAT_NOT_INSERTED;
        break;
    case 11:
        ss = SIM_STAT_PIN_REQ;
        break;
    case 12:
        ss = SIM_STAT_PUK_REQ;
        break;
    case 13:
    case 15:
    case 16:
        ss = SIM_STAT_UNSPECIFIED;
        break;
    case 14:
        ss = SIM_STAT_BUSY;
        break;
    case 17:
        ss = SIM_STAT_PIN2_REQ;
        break;
    case 18:
        ss = SIM_STAT_PUK2_REQ;
        break;
    default:
        ss = SIM_STAT_UNSPECIFIED;
        break;
    }
    return ss;
}

static s32 ATResponse_CPIN_Handler(char* line, u32 len, void* userdata)
{
    Enum_SIMState *result = (Enum_SIMState *)userdata;
    char *head = Ql_RIL_FindString(line, len, "+CPIN:"); //continue wait
    if(head)
    {
        char str[100] = {0};
        char *p = NULL;
        char *q = NULL; 
        p = head + Ql_strlen("+CPIN: ");
        q = Ql_strstr(p,"\r\n");
        if (p)
        {
            Ql_memcpy(str, p, q - p);
        }
        
        *result = RIL_SIM_GetSimStateByName(str,Ql_strlen(str));
        return  RIL_ATRSP_SUCCESS;
    }

   head = Ql_RIL_FindLine(line, len, "OK"); // find <CR><LF>OK<CR><LF>, <CR>OK<CR>£¬<LF>OK<LF>
   if(head)
   {  
       return  RIL_ATRSP_SUCCESS;
   }

    head = Ql_RIL_FindLine(line, len, "ERROR");// find <CR><LF>ERROR<CR><LF>, <CR>ERROR<CR>£¬<LF>ERROR<LF>
    if(head)
    {  
        *result = SIM_STAT_UNSPECIFIED;
        return  RIL_ATRSP_FAILED;
    } 

    head = Ql_RIL_FindString(line, len, "+CME ERROR:");//fail
    if(head)
    {
        s32 err = 0;
        Ql_sscanf(head,"%*[^ ]%d,%[^\r\n]",&err);
        *result = RIL_SIM_GetSimStateByErrCode(err);
        return  RIL_ATRSP_FAILED;
    }

    return RIL_ATRSP_CONTINUE; //continue wait
}

static s32 ATRsp_IMSI_Handler(char* line, u32 len, void* param)
{
    char* p1 = NULL;
    char* p2 = NULL;

    p1 =  Ql_RIL_FindString(line, len, "OK");
    if (p1)
    {  
        return  RIL_ATRSP_SUCCESS;
    }
	p1 = Ql_RIL_FindLine(line, len, "ERROR");
    if (p1)
    {  
        return RIL_ATRSP_FAILED;
    }
    p1 = Ql_RIL_FindLine(line, len, "+CME ERROR:");
    if (p1)
    {
        return  RIL_ATRSP_FAILED;
    }
    p1 = Ql_RIL_FindString(line, len, "\r\n");
    if (p1)
    {
        p2 = Ql_strstr(p1 + 2, "\r\n");
        if (p2)
        {
            Ql_memcpy((char*)param, p1  + 2, p2 - p1 -2);
        }
    }
    return RIL_ATRSP_CONTINUE;   // Wait for the next line of response
}


static s32 ATRsp_CCID_Handler(char* line, u32 len, void* param)
{
	char* p1 = NULL;
    char* p2 = NULL;

    APP_DEBUG("ATRsp_CCID_Handler line=<%s>\r\n", line);

    p1 = Ql_RIL_FindLine(line, len, "OK"); // find <CR><LF>OK<CR><LF>, <CR>OK<CR>£¬<LF>OK<LF>
    if (p1)
    {  
        return RIL_ATRSP_SUCCESS;
    }

    p1 = Ql_RIL_FindLine(line, len, "ERROR");// find <CR><LF>ERROR<CR><LF>, <CR>ERROR<CR>£¬<LF>ERROR<LF>
    if (p1)
    {  
        return RIL_ATRSP_FAILED;
    } 

    p1 = Ql_RIL_FindString(line, len, "+CME ERROR:");//fail
    if (p1)
    {
        return RIL_ATRSP_FAILED;
    }
    p1 = Ql_RIL_FindString(line, len, "+QCCID");
    if (p1)
    {
        p2 = Ql_strstr(p1, "\r\n");
        if (p2)
        {
            Ql_memcpy((char*)param, p1  + 8, p2 - p1 -8);
        }
    }
    return RIL_ATRSP_CONTINUE; //continue wait
}

s32  RIL_SIM_GetSimState(Enum_SIMState *stat)
{
    s32 retRes = -1;
    Enum_SIMState nStat = 0;
    char strAT[] = "AT+CPIN?\0";

    retRes = Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_CPIN_Handler, &nStat, 0);
    if(RIL_AT_SUCCESS == retRes)
    {
       *stat = nStat; 
    }
    return retRes;
}

s32 RIL_SIM_GetIMSI(char* imsi)
{
    char strAT[] = "AT+CIMI\0";
    if (NULL == imsi)
    {
        return RIL_AT_INVALID_PARAM;
    }
    return Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATRsp_IMSI_Handler,(void*)imsi, 0);
}

s32 RIL_SIM_GetCCID(char* ccid)
{
    char strAT[] = "AT+QCCID\0";
    if (NULL == ccid)
    {
        return RIL_AT_INVALID_PARAM;
    }
    return Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATRsp_CCID_Handler,(void*)ccid, 0);
}

#endif  //__OCPU_RIL_SUPPORT__

