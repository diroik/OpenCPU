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
 *   ril_network.c 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   The module implements network related APIs.
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
#include "ril_network.h"
#include "ril.h"
#include "ril_util.h"
#include "ql_stdlib.h"
#include "ql_trace.h"
#include "ql_error.h"
#include "ql_system.h"
#include "ql_trace.h"
#include "typedef.h"
#include "convert.h"

#ifdef __OCPU_RIL_SUPPORT__


/******************************************************************************
* Function:     ATResponse_CEREG_Handler
*  
* Description:
*               This function is used to deal with the response of the AT+CEREG command.
*
* Parameters:    
*                line:  
*                    [in]The address of the string.
*                len:   
*                    [in]The length of the string.
*                userdata: 
*                    [out]Used to transfer the customer's parameter.
*                       
* Return:  
*               RIL_ATRSP_SUCCESS, indicates AT executed successfully..
*               RIL_ATRSP_FAILED, indicates AT executed failed. 
*               RIL_ATRSP_CONTINUE,indicates continue to wait for the response
*               of the AT command.
* Notes:
*               1.Can't send any new AT commands in this function.
*               2.RIL handle the AT response line by line, so this function may 
*                 be called multiple times.
******************************************************************************/
static s32 ATResponse_CEREG_Handler(char* line, u32 len, void* userdata)
{
    char *head = Ql_RIL_FindString(line, len, "+CEREG:"); //continue wait
    if(head)
    {
        s32 n = 0;
        s32 *state = (s32 *)userdata;
        Ql_sscanf(head,"%*[^ ]%d,%d,%[^\r\n]",&n, state);
        return  RIL_ATRSP_CONTINUE;
    }

   head = Ql_RIL_FindLine(line, len, "OK"); // find <CR><LF>OK<CR><LF>, <CR>OK<CR>??<LF>OK<LF>
   if(head)
   {  
       return  RIL_ATRSP_SUCCESS;
   }

    head = Ql_RIL_FindLine(line, len, "ERROR");// find <CR><LF>ERROR<CR><LF>, <CR>ERROR<CR>??<LF>ERROR<LF>
    if(head)
    {  
        return  RIL_ATRSP_FAILED;
    } 

    head = Ql_RIL_FindString(line, len, "+CME ERROR:");//fail
    if(head)
    {
        return  RIL_ATRSP_FAILED;
    }

    return RIL_ATRSP_CONTINUE; //continue wait
}

static s32 ATResponse_CSQ_Handler(char* line, u32 len, void* param)
{
    char* pHead = NULL;
    pHead = Ql_RIL_FindLine(line, len, "OK"); // find <CR><LF>OK<CR><LF>, <CR>OK<CR>??<LF>OK<LF>
    if (pHead)
    {  
        return RIL_ATRSP_SUCCESS;
    }

    pHead = Ql_RIL_FindLine(line, len, "ERROR");// find <CR><LF>ERROR<CR><LF>, <CR>ERROR<CR>??<LF>ERROR<LF>
    if (pHead)
    {  
        return RIL_ATRSP_FAILED;
    } 

    pHead = Ql_RIL_FindString(line, len, "+CME ERROR:");//fail
    if (pHead)
    {
        return RIL_ATRSP_FAILED;
    }
	pHead = Ql_RIL_FindString(line, len, "+CSQ:");//continue wait
    if (pHead)
    {
        char* p1 = NULL;
        char* p2 = NULL;
		
        p1 = Ql_strstr(pHead, ":");
        p2 = Ql_strstr(p1 + 1, "\r\n");
        if (p1 && p2)
        {
            Ql_memcpy((char*)param, p1 + 2, (p2 - p1) - 2);
        }
    }
    return RIL_ATRSP_CONTINUE; //continue wait
}

/******************************************************************************
* Function:     ATResponse_CSQ_Handler
*
* Description:
*               This function is used to deal with the response of the AT+CSQ command.
*
* Parameters:
*                line:
*                    [in]The address of the string.
*                len:
*                    [in]The length of the string.
*                userdata:
*                    [out]Used to transfer the customer's parameter.
*
* Return:
*               RIL_ATRSP_SUCCESS, indicates AT executed successfully..
*               RIL_ATRSP_FAILED, indicates AT executed failed.
*               RIL_ATRSP_CONTINUE,indicates continue to wait for the response
*               of the AT command.
* Notes:
*               1.Can't send any new AT commands in this function.
*               2.RIL handle the AT response line by line, so this function may
*                 be called multiple times.
******************************************************************************/
static s32 ATResponse_CSQ_Handler_new(char* line, u32 len, void* userdata)
{
    ST_CSQ_Reponse *CSQ_Reponse = (ST_CSQ_Reponse*)userdata;

    char *head = Ql_RIL_FindString(line, len, "+CSQ:"); //continue wait
    if(head)
    {
    	//APP_DEBUG("ATResponse_CSQ_Handler_new head=<%s>\r\n", line);
        Ql_sscanf(head,"%*[^ ]%d,%d,%[^\r\n]",&CSQ_Reponse->rssi,&CSQ_Reponse->ber);
        return  RIL_ATRSP_CONTINUE;
    }

    head = Ql_RIL_FindLine(line, len, "OK"); // find <CR><LF>OK<CR><LF>, <CR>OK<CR>??<LF>OK<LF>
    if(head)
    {
        return  RIL_ATRSP_SUCCESS;
    }

    head = Ql_RIL_FindLine(line, len, "ERROR");// find <CR><LF>ERROR<CR><LF>, <CR>ERROR<CR>??<LF>ERROR<LF>
    if(head)
    {
        return  RIL_ATRSP_FAILED;
    }

    head = Ql_RIL_FindString(line, len, "+CME ERROR:");//fail
    if(head)
    {
        return  RIL_ATRSP_FAILED;
    }

    return RIL_ATRSP_CONTINUE; //continue wait
}


static s32 ATResponse_CSCON_Handler(char* line, u32 len, void* userdata)
{
    char *head = Ql_RIL_FindString(line, len, "+CSCON:"); //continue wait
    if(head)
    {
        s32 n = 0;
        s32 *state = (s32 *)userdata;
        Ql_sscanf(head,"%*[^ ]%d,%d,%[^\r\n]",&n,state);
        return  RIL_ATRSP_CONTINUE;
    }

   head = Ql_RIL_FindLine(line, len, "OK"); // find <CR><LF>OK<CR><LF>, <CR>OK<CR>??<LF>OK<LF>
   if(head)
   {  
       return  RIL_ATRSP_SUCCESS;
   }

    head = Ql_RIL_FindLine(line, len, "ERROR");// find <CR><LF>ERROR<CR><LF>, <CR>ERROR<CR>??<LF>ERROR<LF>
    if(head)
    {  
        return  RIL_ATRSP_FAILED;
    } 

    head = Ql_RIL_FindString(line, len, "+CME ERROR:");//fail
    if(head)
    {
        return  RIL_ATRSP_FAILED;
    }

    return RIL_ATRSP_CONTINUE; //continue wait
}


static s32 ATResponse_QENG_Handler(char* line, u32 len, void* param)
{
    char* pHead = NULL;
    pHead = Ql_RIL_FindLine(line, len, "OK"); // find <CR><LF>OK<CR><LF>, <CR>OK<CR>??<LF>OK<LF>
    if (pHead)
    {  
        return RIL_ATRSP_SUCCESS;
    }

    pHead = Ql_RIL_FindLine(line, len, "ERROR");// find <CR><LF>ERROR<CR><LF>, <CR>ERROR<CR>??<LF>ERROR<LF>
    if (pHead)
    {  
        return RIL_ATRSP_FAILED;
    } 

    pHead = Ql_RIL_FindString(line, len, "+CME ERROR:");//fail
    if (pHead)
    {
        return RIL_ATRSP_FAILED;
    }
	pHead = Ql_RIL_FindString(line, len, "+QENG:");//continue wait
    if (pHead)
    {
        char* p1 = NULL;
        char* p2 = NULL;
		
        p1 = Ql_strstr(pHead, ":");
        p2 = Ql_strstr(p1 + 1, "\r\n");
        if (p1 && p2)
        {
            Ql_memcpy((char*)param, p1 + 2, (p2 - p1) - 2);
        }
    }
    return RIL_ATRSP_CONTINUE; //continue wait
}

static s32 ATResponse_DEFCON_Handler(char* line, u32 len, void* userdata)
{
    char *head = Ql_RIL_FindString(line, len, "+QCGDEFCONT:"); //continue wait
    if(head)
    {
        ST_PdnConfig *cfg = (ST_PdnConfig *)userdata;
    	//APP_DEBUG("ATResponse_DEFCON_Handler head=<%s>\r\n", line);
    	char all[100];
    	char mode[20];
    	Ql_memset(mode, 0, sizeof(mode));
    	Ql_memset(all, 0, sizeof(all));

    	int sr = Ql_sscanf(head,"%*[^ ]%s[^\r\n]", all);
    	//APP_DEBUG("ATResponse_DEFCON_Handler Ql_sscanf ret=<%d>, all=[%s]\r\n", sr, all);

    	int index = 0;
    	char* pch = strtok (all,",");
    	while (pch != NULL)
    	{
    		u32 len = Ql_strlen(pch);
    		//APP_DEBUG("index %d: %s, len=%d\n", index, pch, len);
    		if(len > 0){
            	if(index == 0){
            		memcpy_exept(mode,  pch, len, '\"');
            		RIL_NW_ConvertPDPtypeTo(mode, &cfg->mode);
            	}
            	if(index == 1){
            		memcpy_exept(&cfg->gprsApn[0], pch, len, '\"');
            	}
            	if(index == 2){
            		memcpy_exept(&cfg->gprsUser[0], pch, len, '\"');
            	}
            	if(index == 3){
            		memcpy_exept(&cfg->gprsPass[0], pch, len, '\"');
            		break;
            	}
    		}

    		pch = strtok (NULL, ",");
    		index++;
    	}

    	//APP_DEBUG("ATResponse_DEFCON_Handler mode=[%s]\r\n", mode);
        return  RIL_ATRSP_CONTINUE;
    }

   head = Ql_RIL_FindLine(line, len, "OK"); // find <CR><LF>OK<CR><LF>, <CR>OK<CR>??<LF>OK<LF>
   if(head)
   {
       return  RIL_ATRSP_SUCCESS;
   }

    head = Ql_RIL_FindLine(line, len, "ERROR");// find <CR><LF>ERROR<CR><LF>, <CR>ERROR<CR>??<LF>ERROR<LF>
    if(head)
    {
        return  RIL_ATRSP_FAILED;
    }

    head = Ql_RIL_FindString(line, len, "+CME ERROR:");//fail
    if(head)
    {
        return  RIL_ATRSP_FAILED;
    }

    return RIL_ATRSP_CONTINUE; //continue wait
}

static s32 ATResponse_Handler(char* line, u32 len, void* userData)
{
	if(line){
		APP_DEBUG("%s", line);
	}

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

s32 RIL_NW_SendATCmd(char* strAT, char *outValue)
{
	s32 retRes = -1;
    //if (NULL == outValue)
    //{
    //    return RIL_AT_INVALID_PARAM;
    //}
	//Ql_memset(outValue, 0x0, sizeof(tmp_buff));
    retRes = Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_Handler, outValue, 0);
    //if(RIL_AT_SUCCESS == retRes){}
    return retRes;
}

s32 RIL_NW_SetCFUN(Enum_Cfun_Level cfun)
{
	char strAT[20];
	
    if ((cfun != 0)&&(cfun != 1)&&(cfun != 4)&&(cfun != 7))
    {
        return RIL_AT_INVALID_PARAM;
    }
	Ql_sprintf(strAT,"AT+CFUN=%d\0",cfun);
    return Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_Handler, NULL, 0);
}

s32 RIL_NW_SetPSM(Enum_PSM_Mode mode)
{
	char strAT[20];

	if ((mode != 0)&&(mode != 1))
    {
        return RIL_AT_INVALID_PARAM;
    }
	Ql_sprintf(strAT,"AT+CPSMS=%d\0",mode);
    return Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_Handler, NULL, 0);
}

s32  RIL_NW_GetEGPRSState(s32 *stat)
{
    s32 retRes = -1;
    s32 nStat = 0;
    char strAT[] = "AT+CEREG?\0";

    retRes = Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_CEREG_Handler, &nStat, 0);
    if(RIL_AT_SUCCESS == retRes)
    {
       *stat = nStat; 
    }
    return retRes;
}

s32  RIL_NW_GetCSQ(s32 *value)
{
    char strAT[] = "AT+CSQ\0";
    if (NULL == value)
    {
        return RIL_AT_INVALID_PARAM;
    }
    return Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_CSQ_Handler, value, 0);
}

s32  RIL_NW_GetSignalQuality(u32* rssi, u32* ber)
{
    s32 retRes = 0;
    char strAT[] = "AT+CSQ\0";
    ST_CSQ_Reponse pCSQ_Reponse;
    Ql_memset(&pCSQ_Reponse,0, sizeof(pCSQ_Reponse));
    retRes = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_CSQ_Handler_new, (void*)&pCSQ_Reponse, 0);
    if(RIL_AT_SUCCESS == retRes)
    {
       *rssi = pCSQ_Reponse.rssi;
       *ber = pCSQ_Reponse.ber;
    }

    return retRes;
}

s32  RIL_NW_GetCSCON(u8 *status)
{
    s32 retRes = -1;
    s32 state = 0;
    char strAT[] = "AT+CSCON?\0";

    retRes = Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_CSCON_Handler, &state, 0);
    if(RIL_AT_SUCCESS == retRes)
    {
       *status = state; 
    }
    return retRes;
}

s32  RIL_NW_GetQENG(u8 mode, s32* rsp)
{
	if (NULL == rsp)
    {
        return RIL_AT_INVALID_PARAM;
    }
	if((mode != 0)&&(mode != 1)&&(mode != 2))
	{
        return RIL_AT_INVALID_PARAM;
	}
	if(mode == 0)
	{
		char strAT[] = "AT+QENG=0\0";
		return Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_QENG_Handler, rsp, 0);
	}
	else if(mode == 1)
	{
		char strAT[] = "AT+QENG=1\0";
		return Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_QENG_Handler, rsp, 0);
	}
	else
	{
		char strAT[] = "AT+QENG=2\0";
		return Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_QENG_Handler, rsp, 0);
	}
}




bool RIL_NW_ConvertPDPtypeFrom(u8 mode, char *ret)
{
	bool r = TRUE;
	Ql_sprintf(ret,"");

	if(mode == 1){
		Ql_sprintf(ret,"IP");
	}
	else if(mode == 2){
		Ql_sprintf(ret,"IPV6");
	}
	else if(mode == 3){
		Ql_sprintf(ret,"IPV4V6");
	}
	else if(mode == 4){
		Ql_sprintf(ret,"Non-IP");
	}
	else{
		r = FALSE;
	}
	return r;
}

bool RIL_NW_ConvertPDPtypeTo(char *mode, u8 *ret)
{
	bool r = TRUE;
	*ret = 0;

	//APP_DEBUG("RIL_NW_ConvertPDPtypeTo mode=<%s>\r\n", mode);

	if(Ql_strcmp("IP", mode) == 0){
		*ret = 1;
	}
	else if(Ql_strcmp("IPV6", mode) == 0){
		*ret = 2;
	}
	else if(Ql_strcmp("IPV4V6", mode) == 0){
		*ret = 3;
	}
	else if(Ql_strcmp("Non-IP", mode) == 0){
		*ret = 4;
	}
	else{
		r = FALSE;
	}

	//APP_DEBUG("RIL_NW_ConvertPDPtypeTo *ret=<%d>\r\n", *ret);
	return r;
}

s32  RIL_NW_GetDEFCONT(ST_PdnConfig *cfg)
{
	s32 retRes = RIL_AT_FAILED;
	char strAT[] = "AT+QCGDEFCONT?\0";
	//Ql_memset(cfg, 0, sizeof(cfg));
    retRes = Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_DEFCON_Handler, cfg, 0);
	return retRes;
}

s32  RIL_NW_SetDEFCONT(ST_PdnConfig cfg)
{
    s32 retRes = RIL_AT_SUCCESS;
    char strAT[150] ;
    char tmp_buff[50] = {0};

    Ql_memset(strAT,0x00, sizeof(strAT));
    Ql_memset(tmp_buff,0x00, sizeof(tmp_buff));

    if(RIL_NW_ConvertPDPtypeFrom(cfg.mode, tmp_buff) == FALSE){
    	return RIL_AT_FAILED;
    }

    Ql_sprintf(strAT,"AT+QCGDEFCONT=");
    Ql_strcat(strAT, "\"");
    Ql_strcat(strAT, tmp_buff);
    Ql_strcat(strAT, "\",\"");
    Ql_strcat(strAT, cfg.gprsApn);
    Ql_strcat(strAT, "\",\"");
    Ql_strcat(strAT, cfg.gprsUser);
    Ql_strcat(strAT, "\",\"");
    Ql_strcat(strAT, cfg.gprsPass);
    Ql_strcat(strAT, "\"");

    u32 cmdLen 			= Ql_strlen(strAT);
    APP_DEBUG("RIL_NW_SetDEFCONT strAT=<%s>, cmdLen=<%d>\r\n", strAT, cmdLen);

    retRes = Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_Handler, NULL, 0);
    return retRes;
}

///////////////////////////////////////////////////////////////////////
static s32 ATResponse_NIDD_Handler(char* line, u32 len, void* userdata)
{

    char *head = Ql_RIL_FindString(line, len, "+QNIDD:"); //continue wait
    if(head)
    {
    	NIDD_CR_Reponse *response = (NIDD_CR_Reponse*)userdata;

    	//APP_DEBUG("ATResponse_NIDD_Handler head=<%s>\r\n", head);
        Ql_sscanf(head,"%*[^ ]%d,%d,%[^\r\n]",&response->st, &response->id);
        return  RIL_ATRSP_CONTINUE;
    }

    head = Ql_RIL_FindLine(line, len, "OK"); // find <CR><LF>OK<CR><LF>, <CR>OK<CR>??<LF>OK<LF>
    if(head)
    {
        return  RIL_ATRSP_SUCCESS;
    }
    head = Ql_RIL_FindLine(line, len, "ERROR");// find <CR><LF>ERROR<CR><LF>, <CR>ERROR<CR>??<LF>ERROR<LF>
    if(head)
    {
        return  RIL_ATRSP_FAILED;
    }
    head = Ql_RIL_FindString(line, len, "+CME ERROR:");//fail
    if(head)
    {
        return  RIL_ATRSP_FAILED;
    }
    return RIL_ATRSP_CONTINUE; //continue wait
}

s32  Ql_PDN_Activate(u8 sid, u8 mode, char* apn, char* userName, char* pw)
{
    s32 retRes = 0;
    char strAT[150] ;
    //char tmp_buff[50] = {0};

    Ql_memset(strAT,0x00, sizeof(strAT));

    Ql_sprintf(strAT,"AT+QGACT=%d,%d,", sid, mode);
    Ql_strcat(strAT, "\"");
    Ql_strcat(strAT, apn);
    Ql_strcat(strAT, "\",\"");
    Ql_strcat(strAT, userName);
    Ql_strcat(strAT, "\",\"");
    Ql_strcat(strAT, pw);
    Ql_strcat(strAT, "\"");

    //u32 cmdLen 			= Ql_strlen(strAT);
    //APP_DEBUG("Ql_PDN_Activate strAT=<%s>, cmdLen=<%d>\r\n", strAT, cmdLen);
    //retRes = RIL_NW_SendATCmd(strAT, tmp_buff);

    //APP_DEBUG("Ql_PDN_Activate retRes=<%d>, tmp_buff=<%s>\r\n", retRes, tmp_buff);
    retRes = Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_Handler, NULL, 0);
    return retRes;
}

s32  Ql_PDN_Deactivate(u8 sid)
{
    s32 retRes = 0;
    char strAT[30] ;

    Ql_memset(strAT,0x00, sizeof(strAT));
    Ql_sprintf(strAT,"AT+QGACT=0,%d", sid);
    //APP_DEBUG("Ql_PDN_Deactivate strAT=<%s>\r\n", strAT);

    retRes = Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_Handler, NULL, 0);
    //retRes = Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), NULL, NULL, 0);
    return retRes;
}

s32  Ql_NIDD_CreateID(char* apn, char* userName, char* pw)
{
    s32 retRes 			= -1;
    char strAT[150] 	= {0};

    Ql_memset(strAT, 0x00, sizeof(strAT));

    Ql_strcpy(strAT, "AT+QNIDD=0,");
    Ql_strcat(strAT, "\"");
    Ql_strcat(strAT, apn);
    Ql_strcat(strAT, "\",\"");
    Ql_strcat(strAT, userName);
    Ql_strcat(strAT, "\",\"");
    Ql_strcat(strAT, pw);
    Ql_strcat(strAT, "\"");


    NIDD_CR_Reponse response;
    Ql_memset(&response,0, sizeof(response));

    u32 cmdLen 			= Ql_strlen(strAT);
    APP_DEBUG("Ql_NIDD_CreateID strAT=<%s>, cmdLen=<%d>\r\n", strAT, cmdLen);
    s32 ret = Ql_RIL_SendATCmd(strAT, cmdLen, ATResponse_NIDD_Handler, (void*)&response, 0);

    if(ret == RIL_AT_SUCCESS)
    {
    	retRes = response.id;
    }
    return retRes;
}

s32  Ql_NIDD_Connect(s32 accountId)
{
    s32 retRes = -1;
    char strAT[30] ;

    Ql_memset(strAT,0x00, sizeof(strAT));
    Ql_sprintf(strAT,"AT+QNIDD=1,%d", accountId);
    APP_DEBUG("Ql_NIDD_Connect strAT=<%s>\r\n", strAT);
    NIDD_CR_Reponse response;
    Ql_memset(&response,0, sizeof(response));
    s32 ret = Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_NIDD_Handler, (void*)&response, 0);
    //APP_DEBUG("Ql_NIDD_Connect ret=<%d>\r\n", ret);
    if(ret == RIL_AT_SUCCESS)
    {
    	//APP_DEBUG("Ql_NIDD_CreateID response id=<%d>, st=<%d>\r\n", response.id, response.st);
    	retRes = response.id;
    }
    return retRes;
}

s32  Ql_NIDD_ActivateConnection(s32 niddId)
{
    s32 retRes = 0;
    char strAT[30] ;
    Ql_memset(strAT,0x00, sizeof(strAT));
    Ql_sprintf(strAT,"AT+QNIDD=2,%d", niddId);
    APP_DEBUG("Ql_NIDD_ActivateConnection strAT=<%s>\r\n", strAT);
    retRes = Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_Handler, NULL, 0);
    //retRes = Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), NULL, NULL, 0);
    return retRes;
}

s32  Ql_NIDD_SendData(s32 niddId, char* data)
{
    s32 retRes = -1;
    char strAT[512] = {0};
    //char tmp_buff[50] = {0};
    u32 len = Ql_strlen(data);

    if(data == NULL || (len+15) >= sizeof(strAT) || len == 0)
    {
    	return RIL_AT_INVALID_PARAM;
    }
    Ql_memset(strAT,0x00, sizeof(strAT));

    Ql_sprintf(strAT,"AT+QNIDD=3,%d,", niddId);
    Ql_strcat(strAT, "\"");
    Ql_strcat(strAT, data);
    Ql_strcat(strAT, "\"");

    //APP_DEBUG("Ql_NIDD_SendData strAT=<%s>\r\n", strAT);

    retRes = Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_Handler, NULL, 0);
    //retRes = Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), NULL, NULL, 0);
    return retRes;
}

s32  Ql_NIDD_CloseConnection(s32 niddId)
{
    s32 retRes = 0;
    char strAT[30] ;

    Ql_memset(strAT,0x00, sizeof(strAT));
    Ql_sprintf(strAT,"AT+QNIDD=5,%d", niddId);


    APP_DEBUG("Ql_NIDD_CloseConnection strAT=<%s>\r\n", strAT);

    retRes = Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATResponse_Handler, NULL, 0);
    //retRes = Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), NULL, NULL, 0);
    return retRes;
}

///////////////////////////////////////////////////////////////////////
static s32 ATResponse_PING_Handler(char* line, u32 len, void* userdata)
{
	APP_DEBUG("ATResponse_PING_Handler=<%s>\r\n", line);
    char *head = Ql_RIL_FindString(line, len, "+QPING:"); //continue wait
    if(head)
    {
    	s32 *pingcnt = (u32*)userdata;
    	*pingcnt = *pingcnt - 1;

    	APP_DEBUG("ATResponse_PING_Handler pingcnt=<%d>\r\n", *pingcnt);
    	if(*pingcnt > 0)
    		return  RIL_ATRSP_CONTINUE;//RIL_ATRSP_CONTINUE;
    	else
    		return  RIL_ATRSP_SUCCESS;
    }

    head = Ql_RIL_FindLine(line, len, "OK"); // find <CR><LF>OK<CR><LF>, <CR>OK<CR>??<LF>OK<LF>
    if(head)
    {
        return RIL_ATRSP_CONTINUE;// RIL_ATRSP_SUCCESS;
    }
    head = Ql_RIL_FindLine(line, len, "ERROR");// find <CR><LF>ERROR<CR><LF>, <CR>ERROR<CR>??<LF>ERROR<LF>
    if(head)
    {
        return  RIL_ATRSP_FAILED;
    }
    head = Ql_RIL_FindString(line, len, "+CME ERROR:");//fail
    if(head)
    {
        return  RIL_ATRSP_FAILED;
    }
    return RIL_ATRSP_CONTINUE; //continue wait
}

s32  Ql_NW_Ping(char* addr)
{
    //s32 retRes 			= -1;
    char strAT[150] 	= {0};
    Ql_memset(strAT, 0x00, sizeof(strAT));

    Ql_strcpy(strAT, "AT+QPING=");
    Ql_strcat(strAT, "1,");//contextID
    Ql_strcat(strAT, "\"");
    Ql_strcat(strAT, addr);
    Ql_strcat(strAT, "\"");
    Ql_strcat(strAT, "\r\n");
    //Ql_strcat(strAT, "\",10,");//timeout=10s
    //u32 cmdLen 			= Ql_strlen(strAT);
    s32 pingcnt = 4;
    //Ql_sprintf(&strAT[cmdLen],"%d", pingcnt);
    u32 cmdLen 			= Ql_strlen(strAT);
    APP_DEBUG("Ql_NW_Ping strAT=<%s>, cmdLen=<%d>\r\n", strAT, cmdLen);
    s32 ret = Ql_RIL_SendATCmd(strAT, cmdLen, ATResponse_PING_Handler, (void*)&pingcnt, 0);

    return ret;
}

#endif  //__OCPU_RIL_SUPPORT__

