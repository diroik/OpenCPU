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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ql_type.h"
#include "ql_error.h"
#include "cmsis_os2.h"
#include "ql_dbg.h"

#include "ril.h "
#include "ril_network.h"
#include "ril_util.h"
#include "ril_system.h"

#define RIL_NETWORK_DBG_ENABLE 1
#if RIL_NETWORK_DBG_ENABLE > 0
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


/******************************************************************************
* Function:     ATResp_CEREG_Handler
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
static void ATResp_CEREG_Handler(char* line, u32 len, void* userdata)
{
	//APP_DEBUG("[ATResp_CEREG]:%s\r\n",(u8*)line);

    char *head = Ql_RIL_FindString(line, len, "+CEREG:"); //continue wait
    if(head)
    {
        s32 n = 0;
        s32 *state = (s32 *)userdata;
        sscanf(head,"%*[^ ]%d,%d",&n,state);
        return;
    }
}


static void ATResp_CSQ_Handler(char* line, u32 len, void* param)
{
    char* pHead = NULL;
	//APP_DEBUG("[ATResp_CSQ]:%s\r\n",(u8*)line);
	pHead = Ql_RIL_FindString(line, len, "+CSQ:");//continue wait
    if (pHead)
    {
        s32 n = 0;
        //s32 *state =(s32 *)param;
        sscanf(pHead,"%*[^ ]%d,%d",(s32 *)param,&n);
    }
}

static void ATResp_CSCON_Handler(char* line, u32 len, void* userdata)
{
    char *head = Ql_RIL_FindString(line, len, "+CSCON:"); //continue wait
	//APP_DEBUG("[ATResp_CSCON]:%s\r\n",(u8*)line);
    if(head)
    {
        s32 n = 0;
        s32 *state = (s32 *)userdata;
        sscanf(head,"%*[^ ]%d,%d",&n,state);
    }
}

//+QENG: 0,2507,2,146,"772F253",-90,-8,-83,16,5,"785F",0,140,3
static void ATResp_QENG_Handler(char* line, u32 len, void* userData)
{
		//APP_DEBUG("[ATResp_QENG_Handler] %s\r\n", (u8*)line);
		char* pHead = NULL;
		
		pHead = Ql_RIL_FindString(line, len, "+QENG:");//fail
		if (pHead)
		{
			char* p1 = NULL;
			char* p2 = NULL;
			ST_QENG_Reponse* pue_statistics = NULL;
			pue_statistics = (ST_QENG_Reponse*)userData;
			p1 = strstr(pHead, ":");
			p2 = strstr(p1 + 1, "\r\n");
			if (p1 && p2)
			{
					char strTmp[10] = {0};
					QSDK_Get_Str(p1,strTmp,0);

					APP_DEBUG("[mode] %s\r\n", (u8*)strTmp);
					
					memset(strTmp, 0, strlen(strTmp));
					QSDK_Get_Str(p1,strTmp,1);
					pue_statistics->earfcn = atoi(strTmp);
					APP_DEBUG("[earfcn]%d\r\n", pue_statistics->earfcn);

					memset(strTmp, 0, strlen(strTmp));
					QSDK_Get_Str(p1,strTmp,2);

					APP_DEBUG("[sc_earfcn_offset]%s\r\n", (u8*)strTmp);

					memset(strTmp, 0, strlen(strTmp));
					QSDK_Get_Str(p1,strTmp,3);
					pue_statistics->pci = atoi(strTmp);
					APP_DEBUG("[pci]%d\r\n",pue_statistics->pci);

					memset(strTmp, 0, strlen(strTmp));
					QSDK_Get_Str(p1,strTmp,4);

					sscanf(strTmp,"\"%x\"",&pue_statistics->cell_id);
					APP_DEBUG("[cell_id]%x\r\n",pue_statistics->cell_id);

					memset(strTmp, 0, strlen(strTmp));
					QSDK_Get_Str(p1,strTmp,5);
					pue_statistics->signal_power = atoi(strTmp);
					APP_DEBUG("[rsrp]%d\r\n",pue_statistics->signal_power);

					memset(strTmp, 0, strlen(strTmp));
					QSDK_Get_Str(p1,strTmp,6);
					pue_statistics->rsrq = atoi(strTmp);
					APP_DEBUG("[rsrq]%s,%d\r\n", (u8*)strTmp, pue_statistics->rsrq);

					memset(strTmp, 0, strlen(strTmp));
					QSDK_Get_Str(p1,strTmp,7);

					APP_DEBUG("[csq]%s\r\n", (u8*)strTmp);

					memset(strTmp, 0, strlen(strTmp));
					QSDK_Get_Str(p1,strTmp,8);
					pue_statistics->snr = atoi(strTmp);
					APP_DEBUG("[snr]%d\r\n",pue_statistics->snr);

					memset(strTmp, 0, strlen(strTmp));
					QSDK_Get_Str(p1,strTmp,9);
					pue_statistics->band = atoi(strTmp);
					APP_DEBUG("[band]%d\r\n",pue_statistics->band);

					memset(strTmp, 0, strlen(strTmp));
					QSDK_Get_Str(p1,strTmp,10);

					APP_DEBUG("[tac]%s\r\n", (u8*)strTmp);

					memset(strTmp, 0, strlen(strTmp));
					QSDK_Get_Str(p1,strTmp,11);
					pue_statistics->ecl = atoi(strTmp);
					APP_DEBUG("[ecl]%d\r\n",pue_statistics->ecl);

					memset(strTmp, 0, strlen(strTmp));
					QSDK_Get_Str(p1,strTmp,12);
					pue_statistics->tx_power = atoi(strTmp);
					APP_DEBUG("[tx_power]%d\r\n",pue_statistics->tx_power);
			}
		}
}

s32 RIL_NW_SetCFUN(Enum_Cfun_Level cfun)
{
	char strAT[20];
	
    if ((cfun != 0)&&(cfun != 1)&&(cfun != 4)&&(cfun != 7))
    {
        return RIL_AT_INVALID_PARAM;
    }
	sprintf(strAT,"AT+CFUN=%d\0",cfun);
    return Ql_RIL_SendATCmd(strAT, strlen(strAT), DefAtRsp_CallBack, NULL, 0);
}

s32 RIL_NW_SetPSM(Enum_PSM_Mode mode)
{
	char strAT[20];

	if ((mode != 0)&&(mode != 1))
    {
        return RIL_AT_INVALID_PARAM;
    }
	sprintf(strAT,"AT+CPSMS=%d\0",mode);
    return Ql_RIL_SendATCmd(strAT, strlen(strAT), DefAtRsp_CallBack, NULL, 0);
}

s32  RIL_NW_GetEGPRSState(s32 *stat)
{
    s32 retRes = -1;
    s32 nStat = 0;
    char strAT[] = "AT+CEREG?\0";

    retRes = Ql_RIL_SendATCmd(strAT, strlen(strAT), ATResp_CEREG_Handler, &nStat, 0);
    if(RIL_AT_SUCCESS == retRes)
    {
       *stat = nStat; 
    }
    return retRes;
}

s32  RIL_NW_GetCSQ(s32 *value)
{
    //s32 retRes = -1;
    char strAT[] = "AT+CSQ\0";
    //s32 state = 0;
    if (NULL == value)
    {
        return RIL_AT_INVALID_PARAM;
    }
    return Ql_RIL_SendATCmd(strAT, strlen(strAT), ATResp_CSQ_Handler, (void*)value, 0);
}

s32  RIL_NW_GetCSCON(u8 *status)
{
    s32 retRes = -1;
    s32 state = 0;
    char strAT[] = "AT+CSCON?\0";

    retRes = Ql_RIL_SendATCmd(strAT, strlen(strAT), ATResp_CSCON_Handler, &state, 0);
    if(RIL_AT_SUCCESS == retRes)
    {
       *status = state; 
    }
    return retRes;
}

#if 1
s32  RIL_NW_GetQENG(ST_QENG_Reponse* ue_statistics)
{
    //s32 ret = RIL_AT_SUCCESS;
    char strAT[200] ;
    memset(strAT,0, sizeof(strAT));
    sprintf(strAT,"AT+QENG=0\0");
    return Ql_RIL_SendATCmd(strAT,strlen(strAT), ATResp_QENG_Handler,(void*)ue_statistics,0);
}
#else
s32  RIL_NW_GetQENG(ST_QENG_Reponse *qeng_response)
{

		char strAT[] = "AT+QENG=0\0";
		memset(qeng_response,0, sizeof(qeng_response));
		return Ql_RIL_SendATCmd(strAT, strlen(strAT), ATResp_QENG_Handler, qeng_response, 0);
}

#endif 

