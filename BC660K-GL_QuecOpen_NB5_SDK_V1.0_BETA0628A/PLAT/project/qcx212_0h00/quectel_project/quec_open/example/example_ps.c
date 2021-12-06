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
 *   example_ps.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   This example demonstrates how to use PS API in QuecOpen.
 *
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "GLOBAL_EXPORT_FLAG += __EXAMPLE_PS__" in makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
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
#ifdef __EXAMPLE_PS__
#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"
#include "ril.h"
#include "ql_power.h"
#include "ql_ps.h"
#include "ql_dbg.h"

#define DEBUG_ENABLE 1
#if DEBUG_ENABLE > 0
#define DEBUG_PORT  PORT_DBG_LOG
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

typedef enum{
	QUERY_LOC_INFO,
	QUERY_POWER_LVL,
	GET_SIGNAL_INFO,
	GET_CELL_INFO,
	GET_CELL_LVL,

	TEST_ALL_OK
}ENUMPStest;

static u8 ceregCheckTimes = 1;

extern osMessageQueueId_t maintask_queue;
Ql_CellListInfo_t Curr_cellInfo = {0};

void UartSendATCmd(u8* dataPtr, u32 dataLen);
static s32 ATResponse_Handler(const char *pStr, u32 strLen, void *pArg);

static void MainUartRecvCallback(u32 event, void* dataPtr, u32 dataLen)
{
	if((USART_EVENT_RX_TIMEOUT == event) || (USART_EVENT_RECEIVE_COMPLETE == event))
	{
	   // Ql_UART_Write(UART_PORT0,(u8 *)dataPtr,dataLen);
	    UartSendATCmd(dataPtr, dataLen);
	}
}

static s32 pscallback(ENUMurcID eventId, void *param, u32 paramLen)
{
	CmiPsCeregInd *pPsCeregInd = NULL;
	CmiSimImsiStr *pCmiSimImsiInd = NULL;
	CmiMmCesqInd *pMmCesqInd = NULL;
	CmiMmCEStatusInd *pMmCesInd = NULL;
	CmiMmNITZInd *pMmNitzInd = NULL;
	Ql_NmAtiNetInfoInd *pNetInfoInd = NULL;
	ST_MSG msg;
	
	switch (eventId)
	{
		case URC_ID_PS_BEARER_ACTED:
			msg.message = MSG_ID_URC_INDICATION;	
			msg.param1 = URC_EGPRS_NW_STATE_IND;
			osMessageQueuePut(maintask_queue, &msg, 0,0);
			APP_DEBUG("BEARER_ACTED\r\n");//param NULL
			
			break;
		case URC_ID_PS_BEARER_DEACTED:
			msg.message = MSG_ID_URC_INDICATION;	
			msg.param1 = URC_EGPRS_NW_STATE_IND;
			osMessageQueuePut(maintask_queue, &msg, 0,0);
			APP_DEBUG("BEARER_DEACTED\r\n");//param NULL
			
			break;			
		case URC_ID_PS_CEREG_CHANGED:
			pPsCeregInd = (CmiPsCeregInd *)param;
			APP_DEBUG("CEREG_CHANGED:state:[%d]\r\n",pPsCeregInd->state);
			if( 0 != pPsCeregInd->act || 0 != pPsCeregInd->tac || 0 != pPsCeregInd->celId )
			{
				APP_DEBUG("CEREG_CHANGED act:[%d],tac:[%x],celID:[%x]\r\n",pPsCeregInd->act,pPsCeregInd->tac,pPsCeregInd->celId);//act = 9--> NB
			}	
			if(0 != pPsCeregInd->rejCause)
			{
				APP_DEBUG("CEREG_CHANGED rejCause:[%d]\r\n",pPsCeregInd->rejCause);//if the net is rejected,return 3Gpp rejcause #number
			}
			if(pPsCeregInd->activeTimePresent)
			{
				APP_DEBUG("CEREG_CHANGED activeTime:[%d],activeTimeS:[%ds]\r\n",pPsCeregInd->activeTime,pPsCeregInd->activeTimeS);
			}
			if(pPsCeregInd->extTauTimePresent)
			{
				APP_DEBUG("CEREG_CHANGED extPeriodicTau:[%d],extPeriodicTauS:[%ds]\r\n",pPsCeregInd->extPeriodicTau,pPsCeregInd->extPeriodicTauS);
			}
			if(0 != pPsCeregInd->periodicTauS)
			{
				APP_DEBUG("CEREG_CHANGED:periodicTauS:[%ds]\r\n",pPsCeregInd->periodicTauS);
			}
			
			break;
		case URC_ID_PS_NETINFO:
			pNetInfoInd = (Ql_NmAtiNetInfoInd *)param;
			APP_DEBUG("PS_NETINFO ipType:[%d]\r\n",pNetInfoInd->netifInfo.ipType);
			//IPV4 = 1,    IPV6 = 2,      IPV4V6 = 3, 
			
			break;
		case URC_ID_SIM_READY:
			pCmiSimImsiInd = (CmiSimImsiStr *)param;
			APP_DEBUG("SIM_READY:[%s]\r\n",pCmiSimImsiInd->contents);
			msg.message = MSG_ID_URC_INDICATION;	
			msg.param1 = URC_SIM_CARD_STATE_IND;
			osMessageQueuePut(maintask_queue, &msg, 0,0);
			
			break;
		case URC_ID_SIM_REMOVED:
			APP_DEBUG("SIM_REMOVED\r\n");//param NULL
			
			break;			
		case URC_ID_MM_SIGQ:
		 	pMmCesqInd = (CmiMmCesqInd *)param;
			APP_DEBUG("MM_SIGQ rsrp:[%d],rsrq:[%d],snr:[%d]\r\n",pMmCesqInd->rsrp,pMmCesqInd->rsrq,pMmCesqInd->snr);
			
			break;
		case URC_ID_MM_CERES_CHANGED:
			pMmCesInd = (CmiMmCEStatusInd *)param;
			APP_DEBUG("CERES_CHANGED ECL:[%d]\r\n",pMmCesInd->ceLevel-1);
		
			break;	
		case URC_ID_MM_NITZ_REPORT:
			pMmNitzInd = (CmiMmNITZInd *)param;
			if(pMmNitzInd->fullNwNameLen)
			{
				APP_DEBUG("NITZ_REPORT fullNwName:[%s]\r\n",pMmNitzInd->fullNwName);
			}
			if(pMmNitzInd->shortNwNameLen)
			{
				APP_DEBUG("NITZ_REPORT shortNwName:[%s]\r\n",pMmNitzInd->shortNwName);

			}
			if(pMmNitzInd->utcInfoPst)
			{
				APP_DEBUG("NITZ_REPORT date:%d.%d.%d\r\n",pMmNitzInd->utcInfo.year,pMmNitzInd->utcInfo.mon,pMmNitzInd->utcInfo.day);
				APP_DEBUG("NITZ_REPORT utctime:%d.%d.%d\r\n",pMmNitzInd->utcInfo.hour,pMmNitzInd->utcInfo.mins,pMmNitzInd->utcInfo.sec);
				APP_DEBUG("NITZ_REPORT timeZone:%d,%d\r\n",pMmNitzInd->utcInfo.tz,pMmNitzInd->localTimeZone);
			}
			
			break;
		default:
			APP_DEBUG("undef eventID:[%x]\r\n",eventId);
			break;
	}
	return 0;
}


static void app_ps_demo_entry(ST_MSG msg);
void proc_main_task(void)
{
	s32 ret = -1;
	ST_MSG msg;
	char iccid[21] = {0};
	char imsi[17] = {0};
	char imei[18] = {0};
	char sn[32] = {0};
	u8 nw_state = 0;
	Ql_SleepDisable();
	
	Ql_RIL_Initialize();
 	Ql_UART_Open(UART_PORT0,115200,MainUartRecvCallback);
	
	APP_DEBUG("<-- QuecOpen: PS Example -->\r\n");
	ret = Ql_PSCallback_Register(GROUP_ALL_MASK,pscallback);
	osDelay(1000);
	maintask_queue = osMessageQueueNew(MAINTASK_QUEUE_LEN, sizeof(msg), NULL);
	memset(imei, 0, sizeof(imei));
	ret = Ql_GetIMEI(imei);
	APP_DEBUG("<-- Ql_GetIMEI:%d imei:%s -->\r\n",ret,imei);
	memset(sn, 0, sizeof(sn));
	ret = Ql_GetSN(sn);
	APP_DEBUG("<-- Ql_GetSN:%d sn:%s -->\r\n",ret,sn);

    while (1)
    {
    	
		if(osOK == osMessageQueueGet(maintask_queue,(void*)&msg, NULL, osWaitForever))
		switch(msg.message)
        {		
        	case MSG_ID_URC_INDICATION:
			{
	            switch (msg.param1)
	            {
	            	case URC_SIM_CARD_STATE_IND:
	            	{
						memset(imsi, 0, sizeof(imsi));
						ret = Ql_GetIMSI(imsi);
						APP_DEBUG("<-- Ql_GetIMSI:%d imsi:%s -->\r\n",ret,imsi);

						memset(iccid, 0, sizeof(iccid));
						ret = Ql_GetICCID(iccid);
						APP_DEBUG("<-- Ql_GetICCID:%d iccid:%s -->\r\n",ret,iccid);
	            	}	
	                	break;
	            	case URC_EGPRS_NW_STATE_IND:
					{
						ret = Ql_GetCeregState(&nw_state);
		                if((1== nw_state) || (5 == nw_state))
		                {
							ceregCheckTimes = 1;
							
		                	msg.message = MSG_ID_APP_START;	
							msg.param1 = QUERY_LOC_INFO;
							osMessageQueuePut(maintask_queue, &msg, 0,0);
		                    APP_DEBUG("<-- Module has registered to network, status:%d -->\r\n",nw_state);
							
		                }
						else
		                {
							ceregCheckTimes++;
							if(ceregCheckTimes > 120)
							{
								ceregCheckTimes = 1;
		                    	APP_DEBUG("<-- Module has deregister to network, status:%d,ret:%d -->\r\n",nw_state,ret);

								// register network fail
							}
							else
							{
								msg.message = MSG_ID_URC_INDICATION;	
								msg.param1 = URC_EGPRS_NW_STATE_IND;
								osMessageQueuePut(maintask_queue, &msg, 0,0);
								osDelay(500);
							}
		                }

					}
	                	break; 					
	            	default:
	                	APP_DEBUG("<-- Other URC: type = %d\r\n", msg.param1);
	                	break;
	            }
			}
            break;
			case MSG_ID_APP_START:
			{
				app_ps_demo_entry(msg);
			}
				break;
        	default:
            	break;
        }  
	}
}

static void next_test(ST_MSG msg)
{
	msg.message = MSG_ID_APP_START;	
	msg.param1 +=1;
	osMessageQueuePut(maintask_queue, &msg, 0,0);
}


static void app_ps_demo_entry(ST_MSG msg)
{
	s32 ret = -1;
	u16 tac = 0;
	u32 cellId = 0;
	u8 csq = 0;
	s8 snr = 0;
	s8 rsrp = 0;
	
	osDelay(1000);	
	switch (msg.param1)
	{
		case QUERY_LOC_INFO:
			ret = Ql_GetLocInfo(&tac, &cellId);
			next_test(msg);
			APP_DEBUG("<-- Ql_GetLocInfo:%d tac=0x%x,cellId=0x%x\r\n",ret, tac,cellId);
			
			break;
		case QUERY_POWER_LVL:
			ret = Ql_GetPowerLvl();
			next_test(msg);
			APP_DEBUG("<-- Ql_GetPowerLvl:%d \r\n",ret);
			
			break;
		case GET_SIGNAL_INFO:
			ret = Ql_GetSignalInfo(&csq,&snr,&rsrp);
			next_test(msg);
			APP_DEBUG("<-- Ql_GetSignalInfo:%d  csq:%d,snr:%d,rsrp:%d\r\n",ret,csq,snr,rsrp);
			
			break;	
		case GET_CELL_INFO:
			ret = Ql_GetCellInfo(&Curr_cellInfo);
			next_test(msg);
			if(0 != ret)
			{
				APP_DEBUG("<-- Ql_GetCellInfo fail:%d \r\n",ret);
				break;
			}
			APP_DEBUG("<------ SrvCellInfo phyCellId:%x------>\r\n",Curr_cellInfo.sCellInfo.phyCellId);
			APP_DEBUG("<-- earfcn:%x,cellId:%x,mcc:%x,mnc:%x -->\r\n",Curr_cellInfo.sCellInfo.earfcn,Curr_cellInfo.sCellInfo.cellId,Curr_cellInfo.sCellInfo.mcc,Curr_cellInfo.sCellInfo.mncWithAddInfo);
			APP_DEBUG("<-- rsrp:%d,rsrq:%d,snr:%d -->\r\n",Curr_cellInfo.sCellInfo.rsrp,Curr_cellInfo.sCellInfo.rsrq,Curr_cellInfo.sCellInfo.snr);
			APP_DEBUG("<------ nCellNum:%d ------> \r\n",Curr_cellInfo.nCellNum);
			for(u8 i=0;i<Curr_cellInfo.nCellNum;i++)
			{
				APP_DEBUG("<------ NearCellInfo[%d] phyCellId:%x------>\r\n",i,Curr_cellInfo.nCellList[i].phyCellId);
				APP_DEBUG("<-- earfcn:%x -->\r\n",Curr_cellInfo.nCellList[i].earfcn);
				APP_DEBUG("<-- rsrp:%d,rsrq:%d-->\r\n",Curr_cellInfo.nCellList[i].rsrp,Curr_cellInfo.nCellList[i].rsrq);
			}
			
			break;
		case GET_CELL_LVL:
			ret = Ql_GetECL();
			next_test(msg);
			APP_DEBUG("<-- Ql_GetECL:%d \r\n",ret);
			
			break;
		case TEST_ALL_OK:
#if 0
		{
			s32 cereg=0;
			s32 csq=0;
			u8 cscon=0;
			ST_QENG_Reponse qeng = {0};
			osDelay(500);	
			ret = RIL_NW_SetPSM(PSM_DISABLE);
			APP_DEBUG("<-- RIL_NW_SetPSM:%d \r\n",ret);
			osDelay(500);	
			ret = RIL_NW_GetEGPRSState(&cereg);
			APP_DEBUG("<-- RIL_NW_GetEGPRSState:%d cereg:%d\r\n",ret,cereg);
			osDelay(500);	
			ret = RIL_NW_GetCSQ(&csq);
			APP_DEBUG("<-- RIL_NW_GetCSQ:%d csq:%d\r\n",ret,csq);
			osDelay(500);	
			ret = RIL_NW_GetCSCON(&cscon);
			APP_DEBUG("<-- RIL_NW_GetCSCON:%d cscon:%d\r\n",ret,cscon);
			osDelay(500);	
			ret = RIL_NW_GetQENG(&qeng);
			APP_DEBUG("<-- RIL_GetQENG:%d \r\n",ret);
		}
#endif
			APP_DEBUG("<---------- TEST_ALL_DONE ---------->\r\n");

			break;
	}
}

void UartSendATCmd(u8* dataPtr, u32 dataLen)
{
    s32 ret;

    if(0 == dataLen)
    {
        return;
    }
    
    // Echo
    Ql_UART_Write(UART_PORT0, dataPtr, dataLen);

    ret = Ql_RIL_SendATCmd((char* )dataPtr, dataLen, (Callback_ATResponse)ATResponse_Handler, NULL, 0);
    APP_DEBUG("<-- UartSendATCmd,ret = %d\r\n", ret);
}

static s32 ATResponse_Handler(const char *pStr, u32 strLen, void *pArg)
{
    APP_DEBUG("[ATResponse_Handler] receData= [%s].-->\r\n",pStr);

    return 0;
}


#endif

