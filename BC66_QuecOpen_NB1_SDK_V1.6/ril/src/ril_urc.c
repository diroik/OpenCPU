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
 *   ril_urc.c 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   The module handles URC in RIL.
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
#include "ql_stdlib.h"
#include "ril.h "
#include "ril_util.h"
#include "ql_system.h"
#include "ql_common.h"
#include "ql_uart.h"
#include "ql_type.h"
#include "ril_lwm2m.h"
#include "ril_socket.h"
#include "ril_system.h"
#include "ril_dfota.h"
#include "ril_network.h"
#include "ril_sim.h"
#include "ril_ssl.h"

#ifdef __OCPU_RIL_SUPPORT__

#define RIL_URC_DEBUG_ENABLE 0
#if RIL_URC_DEBUG_ENABLE > 0
#define RIL_URC_DEBUG_PORT  UART_PORT0
static char DBG_Buffer[1024];
#define RIL_URC_DEBUG(BUF,...)  QL_TRACE_LOG(RIL_URC_DEBUG_PORT,BUF,1024,__VA_ARGS__)
#else
#define RIL_URC_DEBUG(BUF,...) 
#endif

/************************************************************************/
/* Definition for URC receive task id.                                  */
/************************************************************************/
#define URC_RCV_TASK_ID  main_task_id

/************************************************************************/
/* Definition for URC param.                                  */
/************************************************************************/
Socket_Urc_Param_t socket_urc_param= {0,0,NULL,NULL,0,0,0,0,0};  //tcp or udp

Lwm2m_Urc_Param_t  lwm2m_urc_param = {0,0,0,-1,0,0,255,255,255};//lwm2m

MQTT_Urc_Param_t   mqtt_urc_param =  {0,0,255,255,255,255,{255}}; //mqtt

SSL_Urc_Param_t ssl_urc_param = {0,0,0,0,0,0,0,0,{0},0}; //ssl

/************************************************************************/
/* Declarations for URC handler.                                        */
/************************************************************************/
static void OnURCHandler_Network(const char* strURC, void* reserved);
static void OnURCHandler_SIM(const char* strURC, void* reserved);
static void OnURCHandler_CFUN(const char* strURC, void* reserved);
static void OnURCHandler_InitStat(const char* strURC, void* reserved);

/********************PSM***********************************************/
static void OnURCHandler_NbiotEvent(const char* strURC, void* reserved);

/*************** ***TCP &&UDP********************************************/
static void OnURCHandler_QIURC_OPEN(const char* strURC, void* reserved);
static void OnURCHandler_SOCKET_QIURC(const char* strURC, void* reserved);

/*************** ***LWM2M********************************************/
static void OnURCHandler_LwM2M_REG(const char* strURC, void* reserved);
static void OnURCHandler_LwM2M_UPDATE(const char* strURC, void* reserved);
static void OnURCHandler_LwM2M_DEREG(const char* strURC, void* reserved);
static void OnURCHandler_LwM2M_URC(const char* strURC, void* reserved); 
static void OnURCHandler_LwM2M_OBSERVE(const char* strURC, void* reserved);
static void OnURCHandler_LwM2M_READ(const char* strURC, void* reserved);
static void OnURCHandler_LwM2M_WRITE(const char* strURC, void* reserved);
static void OnURCHandler_LwM2M_EXECUTE(const char* strURC, void* reserved);
static void OnURCHandler_LwM2M_NOTIFY(const char* strURC, void* reserved);

/*************** ***DFOTA***************************************/
static void OnURCHandler_DFOTA_Hander  (const char* strURC, void* reserved);

/*************** ***MQTT********************************************/
static void OnURCHandler_MQTT_OPEN(const char* strURC, void* reserved);
static void OnURCHandler_MQTT_CONN(const char* strURC, void* reserved);
static void OnURCHandler_MQTT_SUB(const char* strURC, void* reserved);
static void OnURCHandler_MQTT_PUB(const char* strURC, void* reserved);
static void OnURCHandler_MQTT_TUNS(const char* strURC, void* reserved);
static void OnURCHandler_MQTT_STATE(const char* strURC, void* reserved);
static void OnURCHandler_MQTT_CLOSE(const char* strURC, void* reserved);
static void OnURCHandler_MQTT_DISC(const char* strURC, void* reserved);

/************************SSL*************************************/
static void OnURCHandler_SSL_CFG(const char* strURC, void* reserved);
static void OnURCHandler_SSL_OPEN(const char* strURC, void* reserved);
static void OnURCHandler_SSL_CLOSE(const char* strURC, void* reserved);
static void OnURCHandler_SSL_URC(const char* strURC, void* reserved);

/************************************************************************/
/* Customer ATC URC callback                                          */
/************************************************************************/

/******************************************************************************
* Definitions for URCs and the handler.
* -------------------------------------
* -------------------------------------
* In QuecOpen RIL, URC contains two types: system URC and AT URC.
*   - System URCs indicate the various status of module.
*   - AT URC serves some specific AT command. 
*     For example, some AT command responses as below:
*         AT+QABC     (send at command)
*
*         OK          (response1)
*         +QABC:xxx   (response2) --> this is the final result which is reported by URC.
*     When calling Ql_RIL_SendATCmd() to send such AT command, the return value of 
*     Ql_RIL_SendATCmd indicates the response1, and the response2 may be reported
*     via the callback function. 
******************************************************************************/
/****************************************************/
/* Definitions for system URCs and the handler      */
/****************************************************/
const static ST_URC_HDLENTRY m_SysURCHdlEntry[] = {
    //Network status unsolicited response
    {"\r\n+CEREG:",                               OnURCHandler_Network},
    //SIM card unsolicited response
    {"\r\n+CPIN:",                                OnURCHandler_SIM},                       
    //CFUN unsolicited response
    {"\r\n+CFUN:",                                OnURCHandler_CFUN},
	//PSM EVENT unsolicited response
    {"\r\n+QNBIOTEVENT:",                         OnURCHandler_NbiotEvent},
};

/****************************************************/
/* Definitions for AT URCs and the handler          */
/****************************************************/
const static ST_URC_HDLENTRY m_AtURCHdlEntry[] = {
	{"\r\n+QIURC:",                               OnURCHandler_SOCKET_QIURC},
	{"\r\n+QIOPEN:",                              OnURCHandler_QIURC_OPEN},
	{"\r\n+QIND: \"FOTA\"",                       OnURCHandler_DFOTA_Hander},	
	{"\r\n+QMTOPEN:",                             OnURCHandler_MQTT_OPEN},
	{"\r\n+QMTCONN:",                             OnURCHandler_MQTT_CONN},
	{"\r\n+QMTSUB:",                              OnURCHandler_MQTT_SUB},
    {"\r\n+QMTPUB:",                              OnURCHandler_MQTT_PUB},
	{"\r\n+QMTUNS:",                              OnURCHandler_MQTT_TUNS},
	{"\r\n+QMTSTAT:",                             OnURCHandler_MQTT_STATE},
	{"\r\n+QMTCLOSE:",                            OnURCHandler_MQTT_CLOSE},
	{"\r\n+QMTDISC:",                             OnURCHandler_MQTT_DISC},
	{"\r\n+QLWREG:",                              OnURCHandler_LwM2M_REG},
	{"\r\n+QLWUPDATE:",                           OnURCHandler_LwM2M_UPDATE},
	{"\r\n+QLWDEREG:",                            OnURCHandler_LwM2M_DEREG},
	{"\r\n+QLWURC:",                              OnURCHandler_LwM2M_URC},
	{"\r\n+QLWOBSRSP:",                           OnURCHandler_LwM2M_OBSERVE},
	{"\r\n+QLWRDRSP:",                            OnURCHandler_LwM2M_READ},
	{"\r\n+QLWWRRSP:",							  OnURCHandler_LwM2M_WRITE},
	{"\r\n+QLWEXERSP:",                           OnURCHandler_LwM2M_EXECUTE},
	{"\r\n+QLWNOTIFY:", 						  OnURCHandler_LwM2M_NOTIFY},
	{"\r\n+QSSLCFG:", 						  	  OnURCHandler_SSL_CFG},
	{"\r\n+QSSLOPEN:",							  OnURCHandler_SSL_OPEN},
	{"\r\n+QSSLCLOSE:",							  OnURCHandler_SSL_CLOSE},
	{"\r\n+QSSLURC:", 						  	  OnURCHandler_SSL_URC},
};



/**********************************************URC handler*****************************************************/
static void OnURCHandler_QIURC_OPEN(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	
	p1 = Ql_strstr(strURC, "+QIOPEN:");
	p1 += Ql_strlen("+QIOPEN:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		socket_urc_param.connectID= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		socket_urc_param.error_no= Ql_atoi(strTmp);
		
		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_SOCKET_OPEN, &socket_urc_param);	
	}
}

static void OnURCHandler_SOCKET_QIURC(const char* strURC, void* reserved)
{
    u8* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
    s32 ret;
	
    p1 = Ql_strstr(strURC, "+QIURC:");
	p1 += Ql_strlen("+QIURC:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';
	
    if (p1)
    {		
		Ql_memset(strTmp, 0x0,  sizeof(strTmp));
	    ret = QSDK_Get_Str(p1,strTmp,0);
        if(Ql_memcmp(strTmp,"\"closed\"",Ql_strlen("\"closed\"")) == 0)
		{
			Ql_memset(strTmp, 0x0,  sizeof(strTmp));
		    QSDK_Get_Str(p1,strTmp,1);
			socket_urc_param.connectID = Ql_atoi(strTmp);
	
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_SOCKET_CLOSE,socket_urc_param.connectID);
		}
	}
}

static void OnURCHandler_SIM(const char* strURC, void* reserved)
{
    char* p1 = NULL;
    char* p2 = NULL;
    char strTmp[20];
    s32 len;
	
    Ql_memset(strTmp, 0x0, sizeof(strTmp));
    len = Ql_sprintf(strTmp, "\r\n+CPIN: ");
    if (Ql_StrPrefixMatch(strURC, strTmp))
    {
        p1 = Ql_strstr(strURC, "\r\n+CPIN: ");
        p1 += len;
        p2 = Ql_strstr(p1, "\r\n");
        if (p1 && p2)
        {
            Enum_SIMState cpinStat;
            Ql_memset(strTmp, 0x0, sizeof(strTmp));
            Ql_memcpy(strTmp, p1, p2 - p1);
            cpinStat = RIL_SIM_GetSimStateByName(strTmp, p2 - p1);
            Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_SIM_CARD_STATE_IND, cpinStat);
        }
    }
}

static void OnURCHandler_Network(const char* strURC, void* reserved)
{
    char* p1 = NULL;
    char* p2 = NULL;
    char strTmp[10];
    
    if (Ql_StrPrefixMatch(strURC, "\r\n+CEREG: "))
    {
        u32 nwStat;
        p1 = Ql_strstr(strURC, "\r\n+CEREG: ");
        p1 += Ql_strlen("\r\n+CEREG: ");
		if(*(p1+1) == 0x2C)          //Active query network status without reporting URCS
		{
		   return;
		}
        p2 = Ql_strstr(p1, "\r\n");
        if (p1 && p2)
        {
            Ql_memset(strTmp, 0x0, sizeof(strTmp));
            Ql_memcpy(strTmp, p1, p2 - p1);
            nwStat = Ql_atoi(strTmp);
            Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_EGPRS_NW_STATE_IND, nwStat);
        }
    }
}

static void OnURCHandler_InitStat(const char* strURC, void* reserved)
{
    u32 sysInitStat = SYS_STATE_START;
    
    if (Ql_strstr(strURC, "\r\nCall Ready\r\n") != NULL)
    {
        sysInitStat = SYS_STATE_PHBOK;
    }
    else if(Ql_strstr(strURC, "\r\nSMS Ready\r\n") != NULL)
    {
        sysInitStat = SYS_STATE_SMSOK;
    }
    Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_SYS_INIT_STATE_IND, sysInitStat);
}

static void OnURCHandler_CFUN(const char* strURC, void* reserved)
{
    char* p1 = NULL;
    char* p2 = NULL;
    char strTmp[10];
    s32 len;
    u32 cfun;

    len = Ql_strlen("\r\n+CFUN: ");
    p1 = Ql_strstr(strURC, "\r\n+CFUN: ");
    p1 += len;
    p2 = Ql_strstr(p1, "\r\n");
    if (p1 && p2)
    {
        Ql_memset(strTmp, 0x0, sizeof(strTmp));
        Ql_memcpy(strTmp, p1, 1);
        cfun = Ql_atoi(strTmp);
        Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_CFUN_STATE_IND, cfun);
    }
}


static void OnURCHandler_NbiotEvent(const char* strURC, void* reserved)
{
    Enum_PSM_State psm_state = END_PSM;

	RIL_URC_DEBUG(DBG_Buffer,"OnURCHandler_NbiotEvent(%s)\r\n", strURC);
	
	if (Ql_strstr(strURC, "\r\n+QNBIOTEVENT: \"ENTER PSM\"") != NULL)
	{
		psm_state = ENTER_PSM;
	}
	else if (Ql_strstr(strURC, "\r\n+QNBIOTEVENT: \"EXIT PSM\"") != NULL)
	{
		psm_state= EXIT_PSM;
	}
	Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_PSM_EVENT, psm_state);
}

static void OnURCHandler_DFOTA_Hander  (const char* strURC, void* reserved)
{
    Dfota_Upgrade_State upgrade_state = DFOTA_STATE_END;
	s32 dfota_errno; 

	//RIL_URC_DEBUG(DBG_Buffer,"OnURCHandler_DFOTA_Hander(%s)\r\n", strURC);
	if (Ql_strstr(strURC, "\r\n+QIND: \"FOTA\"") != NULL)
	{
		DFOTA_Analysis(strURC,&upgrade_state,&dfota_errno);
		if(dfota_errno == 0)//normal
		{
            Dfota_Upgrade_States(upgrade_state,0);
		}
		else//failed
		{
			Dfota_Upgrade_States(upgrade_state,dfota_errno);
		}
	}
}

static void OnURCHandler_MQTT_OPEN(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	
	p1 = Ql_strstr(strURC, "+QMTOPEN:");
	p1 += Ql_strlen("+QMTOPEN:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.result= Ql_atoi(strTmp);
		
		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_MQTT_OPEN, &mqtt_urc_param);	
	}

}

static void OnURCHandler_MQTT_CONN(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	
	p1 = Ql_strstr(strURC, "+QMTCONN:");
	p1 += Ql_strlen("+QMTCONN:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.result= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		if(TRUE == QSDK_Get_Str(p1,strTmp,2))
		{
		  mqtt_urc_param.connect_code= Ql_atoi(strTmp);
		}
		else
		{
			mqtt_urc_param.connect_code= 255;
		}
		
		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_MQTT_CONN, &mqtt_urc_param);
	}
}

static void OnURCHandler_MQTT_SUB(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	char i;
	
	p1 = Ql_strstr(strURC, "+QMTSUB:");
	p1 += Ql_strlen("+QMTSUB:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.msgid= Ql_atoi(strTmp);

		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		mqtt_urc_param.result= Ql_atoi(strTmp);

		for(i=0;i<MQTT_MAX_TOPIC;i++)
        {
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
     		if(TRUE == QSDK_Get_Str(p1,strTmp,(3+i)))
     		{
     		  mqtt_urc_param.sub_value[i]= Ql_atoi(strTmp);
     		}
     		else
     		{
     			mqtt_urc_param.sub_value[i]= 255;
				break;
     		}
		}

		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_MQTT_SUB, &mqtt_urc_param);	
	}

}

static void OnURCHandler_MQTT_PUB(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];

	p1 = Ql_strstr(strURC, "+QMTPUB:");
	p1 += Ql_strlen("+QMTPUB:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';
	

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.msgid= Ql_atoi(strTmp);

		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		mqtt_urc_param.result= Ql_atoi(strTmp);

  		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		if(TRUE == QSDK_Get_Str(p1,strTmp,3))
		{
		   mqtt_urc_param.pub_value= Ql_atoi(strTmp);
		}
		else
		{
			mqtt_urc_param.pub_value= 255;
		}
		
		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_MQTT_PUB, &mqtt_urc_param);	
	}
}

static void OnURCHandler_MQTT_TUNS(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	
	p1 = Ql_strstr(strURC, "+QMTUNS:");
	p1 += Ql_strlen("+QMTUNS:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.msgid= Ql_atoi(strTmp);

		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		mqtt_urc_param.result= Ql_atoi(strTmp);
		
		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_MQTT_TUNS, &mqtt_urc_param);	
	}
}

static void OnURCHandler_MQTT_STATE(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	
	p1 = Ql_strstr(strURC, "+QMTSTAT:");
	p1 += Ql_strlen("+QMTSTAT:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= Ql_atoi(strTmp);

		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.mqtt_state= Ql_atoi(strTmp);
		
		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_MQTT_STATE, &mqtt_urc_param);	
	}
}

static void OnURCHandler_MQTT_CLOSE(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	p1 = Ql_strstr(strURC, "+QMTCLOSE:");
	p1 += Ql_strlen("+QMTCLOSE:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.msgid= Ql_atoi(strTmp);

		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		mqtt_urc_param.result= Ql_atoi(strTmp);
		
		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_MQTT_CLOSE, &mqtt_urc_param);	
	}
}

static void OnURCHandler_MQTT_DISC(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	
	p1 = Ql_strstr(strURC, "+QMTDISC:");
	p1 += Ql_strlen("+QMTDISC:");
	p1++;
	p2 = Ql_strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= Ql_atoi(strTmp);
		
		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.msgid= Ql_atoi(strTmp);

		Ql_memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		mqtt_urc_param.result= Ql_atoi(strTmp);
		
		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_MQTT_DISC, &mqtt_urc_param);	
	}
}


static void OnURCHandler_LwM2M_REG(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char strTmp[50]; 
	u32 reg_status = 255;
	
	p1 = Ql_strstr(strURC, "\r\n+QLWREG:");
	p1 += Ql_strlen("\r\n+QLWREG:");
	p1++;
	
	if (p1)
	{	
    	Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	QSDK_Get_Str(p1,strTmp,0);
    	reg_status = Ql_atoi(strTmp);
    	Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_LWM2M_REG, reg_status); 	 
	}
}

static void OnURCHandler_LwM2M_UPDATE(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char strTmp[50]; 
	
	p1 = Ql_strstr(strURC, "\r\n+QLWUPDATE:");
	p1 += Ql_strlen("\r\n+QLWUPDATE:");
	p1++;
	
	if (p1)
	{	
    	Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	if(TRUE == QSDK_Get_Str(p1,strTmp,1))
    	{
			lwm2m_urc_param.msgid= Ql_atoi(strTmp);
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
        	QSDK_Get_Str(p1,strTmp,0);
        	lwm2m_urc_param.update_status= Ql_atoi(strTmp);
		}
	    else 
	    {
			QSDK_Get_Str(p1,strTmp,0);
			lwm2m_urc_param.msgid= Ql_atoi(strTmp);
			lwm2m_urc_param.update_status = 255;
		}
    	Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_LWM2M_UPDATE, &lwm2m_urc_param); 	 
	}
}
static void OnURCHandler_LwM2M_DEREG(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	u32 dereg_status = 255;
	char strTmp[50]; 
	
	p1 = Ql_strstr(strURC, "\r\n+QLWDEREG:");
	p1 += Ql_strlen("\r\n+QLWDEREG:");
	p1++;
	
	if (p1)
	{	
    	Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	QSDK_Get_Str(p1,strTmp,0);
    	dereg_status = Ql_atoi(strTmp);
    	Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_LWM2M_DEREG, dereg_status); 	 
	}
}

static void OnURCHandler_LwM2M_URC(const char* strURC, void* reserved)
{
    char* p1 = NULL;
	char strTmp[50]; 
	u32 ping_status = 255;
	u32 lw_status = 255;
	
    p1 = Ql_strstr(strURC, "\r\n+QLWURC:");
	p1 += Ql_strlen("\r\n+QLWURC:");
	p1++;

	if (p1)
    {	
	   Ql_memset(strTmp, 0x0,  sizeof(strTmp));
	   QSDK_Get_Str(p1,strTmp,0); 
	   RIL_URC_DEBUG(DBG_Buffer,"OnURCHandler_LwM2M_URC(%s)\r\n",strTmp);

	   if(Ql_memcmp(strTmp,"\"ping\"",Ql_strlen("\"ping\"")) == 0)
	   {
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
			QSDK_Get_Str(p1,strTmp,1);
			ping_status= Ql_atoi(strTmp);
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_LWM2M_PING, ping_status);	
		} 
		else if(Ql_memcmp(strTmp,"\"lwstatus\"",Ql_strlen("\"lwstatus\"")) == 0)
		{
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
			QSDK_Get_Str(p1,strTmp,1);
			lw_status= Ql_atoi(strTmp);
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_LWM2M_STATUS, lw_status);	
		} 
		else if(Ql_memcmp(strTmp,"\"observe\"",Ql_strlen("\"observe\"")) == 0)
		{
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
	        QSDK_Get_Str(p1,strTmp,1);
		    lwm2m_urc_param.msgid= Ql_atoi(strTmp);

			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,2);
    		lwm2m_urc_param.observe_flag= Ql_atoi(strTmp);
			
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,3);
    		lwm2m_urc_param.obj_id= Ql_atoi(strTmp);
			
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,4);
    		lwm2m_urc_param.ins_id= Ql_atoi(strTmp);

			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,5);
    		lwm2m_urc_param.res_id= Ql_atoi(strTmp);
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_LWM2M_OBSERVE_REQ, &lwm2m_urc_param) ;
		} 
	    else if(Ql_memcmp(strTmp,"\"read\"",Ql_strlen("\"read\"")) == 0)
		{
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
	        QSDK_Get_Str(p1,strTmp,1);
		    lwm2m_urc_param.msgid= Ql_atoi(strTmp);
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,2);
    		lwm2m_urc_param.obj_id= Ql_atoi(strTmp);
			
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,3);
    		lwm2m_urc_param.ins_id= Ql_atoi(strTmp);

			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,4);
    		lwm2m_urc_param.res_id= Ql_atoi(strTmp);
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_LWM2M_READ_REQ,&lwm2m_urc_param);	
		} 
		else if(Ql_memcmp(strTmp,"\"write\"",Ql_strlen("\"write\"")) == 0)
		{
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
	        QSDK_Get_Str(p1,strTmp,1);
		    lwm2m_urc_param.msgid= Ql_atoi(strTmp);
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,2);
    		lwm2m_urc_param.obj_id= Ql_atoi(strTmp);
			
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,3);
    		lwm2m_urc_param.ins_id= Ql_atoi(strTmp);

			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,4);
    		lwm2m_urc_param.res_id= Ql_atoi(strTmp);
			
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_LWM2M_WRITE_REQ,&lwm2m_urc_param);	
		}
		else if(Ql_memcmp(strTmp,"\"report\"",Ql_strlen("\"report\"")) == 0)
		{
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,1);
			lwm2m_urc_param.result_code= 255;
    		lwm2m_urc_param.msgid= Ql_atoi(strTmp);

			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_LWM2M_CON_REPORT,&lwm2m_urc_param);	
		}
		else if(Ql_memcmp(strTmp,"\"report_ack\"",Ql_strlen("\"report_ack\"")) == 0)
		{
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
	        QSDK_Get_Str(p1,strTmp,1);
		    lwm2m_urc_param.result_code= Ql_atoi(strTmp);
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,2);
    		lwm2m_urc_param.msgid= Ql_atoi(strTmp);

			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_LWM2M_CON_REPORT,&lwm2m_urc_param);	
		}
		else if(Ql_memcmp(strTmp,"\"execute\"",Ql_strlen("\"execute\"")) == 0)
		{
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
	        QSDK_Get_Str(p1,strTmp,1);
		    lwm2m_urc_param.msgid= Ql_atoi(strTmp);
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,2);
    		lwm2m_urc_param.obj_id= Ql_atoi(strTmp);
			
			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,3);
    		lwm2m_urc_param.ins_id= Ql_atoi(strTmp);

			Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,4);
    		lwm2m_urc_param.res_id= Ql_atoi(strTmp);
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_LWM2M_EXECUTE_REQ, &lwm2m_urc_param);
		}
    }
}


static void OnURCHandler_LwM2M_OBSERVE(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char strTmp[50]; 
	
	p1 = Ql_strstr(strURC, "\r\n+QLWOBSRSP:");
	p1 += Ql_strlen("\r\n+QLWOBSRSP:");
	p1++;
	
	if (p1)
	{	
    	Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	QSDK_Get_Str(p1,strTmp,0);
    	lwm2m_urc_param.error_code= Ql_atoi(strTmp);
    	Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_LWM2M_OBSERVE_RSQ, &lwm2m_urc_param); 	 
	}
}

static void OnURCHandler_LwM2M_READ(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char strTmp[50]; 
	
	p1 = Ql_strstr(strURC, "\r\n+QLWRDRSP:");
	p1 += Ql_strlen("\r\n+QLWRDRSP:");
	p1++;
	
	if (p1)
	{	
    	Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	QSDK_Get_Str(p1,strTmp,0);
    	lwm2m_urc_param.error_code= Ql_atoi(strTmp);
    	Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_LWM2M_READ_RSQ, &lwm2m_urc_param); 	 
	}
}

static void OnURCHandler_LwM2M_WRITE(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char strTmp[50];
	
	p1 = Ql_strstr(strURC, "\r\n+QLWWRRSP:");
	p1 += Ql_strlen("\r\n+QLWWRRSP:");
	p1++;
	
	if (p1)
	{	
    	Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	QSDK_Get_Str(p1,strTmp,0);
    	lwm2m_urc_param.error_code= Ql_atoi(strTmp);
    	Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_LWM2M_WRITE_RSQ, &lwm2m_urc_param); 	 
	}
}

static void OnURCHandler_LwM2M_EXECUTE(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char strTmp[50]; 
	
	p1 = Ql_strstr(strURC, "\r\n+QLWEXERSP:");
	p1 += Ql_strlen("\r\n+QLWEXERSP:");
	p1++;
	
	if (p1)
	{	
    	Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	QSDK_Get_Str(p1,strTmp,0);
    	lwm2m_urc_param.error_code= Ql_atoi(strTmp);
    	Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_LWM2M_EXECUTE_RSQ, &lwm2m_urc_param); 	 
	}
}

static void OnURCHandler_LwM2M_NOTIFY(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char strTmp[50]; 
	
	p1 = Ql_strstr(strURC, "\r\n+QLWNOTIFY:");
	p1 += Ql_strlen("\r\n+QLWNOTIFY:");
	p1++;
	
	if (p1)
	{	
    	Ql_memset(strTmp, 0x0,	sizeof(strTmp));
    	QSDK_Get_Str(p1,strTmp,0);
    	lwm2m_urc_param.error_code= Ql_atoi(strTmp);
    	Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_LWM2M_NOTIFY, &lwm2m_urc_param); 	 
	}
}

static void OnURCHandler_SSL_CFG(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char strTmp[50]; 
	
	p1 = Ql_strstr(strURC, "\r\n+QSSLCFG:");
	p1 += Ql_strlen("\r\n+QSSLCFG:");
	p1++;
	
	if (p1)
	{		   
    	Ql_memset(strTmp, 0x0, sizeof(strTmp));
    	QSDK_Get_Str(p1,strTmp,0);
    	ssl_urc_param.contextid = Ql_atoi(strTmp);

    	Ql_memset(strTmp, 0x0, sizeof(strTmp));
    	QSDK_Get_Str(p1,strTmp,1);
    	ssl_urc_param.connectid = Ql_atoi(strTmp);

		Ql_memset(strTmp, 0x0, sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		RIL_URC_DEBUG(DBG_Buffer,"OnURCHandler_SSL_URC(%s)\r\n",strTmp);
		Ql_memcpy(ssl_urc_param.param_item, strTmp, Ql_strlen(strTmp));
		
		if(Ql_memcmp(strTmp,"\"seclevel\"",Ql_strlen("\"seclevel\"")) == 0)
		{
			Ql_memset(strTmp, 0x0, sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,3);
    		ssl_urc_param.seclevel = Ql_atoi(strTmp);
			
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_SSL_CONFIG, &ssl_urc_param);
		}
		else if(Ql_memcmp(strTmp,"\"dataformat\"",Ql_strlen("\"dataformat\"")) == 0)
		{
			Ql_memset(strTmp, 0x0, sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,3);
    		ssl_urc_param.send_data_format = Ql_atoi(strTmp);

			Ql_memset(strTmp, 0x0, sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,4);
    		ssl_urc_param.recv_data_format = Ql_atoi(strTmp);
		
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_SSL_CONFIG, &ssl_urc_param);
		}
		else if(Ql_memcmp(strTmp,"\"timeout\"",Ql_strlen("\"timeout\"")) == 0)
		{
			Ql_memset(strTmp, 0x0, sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,3);
    		ssl_urc_param.timeout = Ql_atoi(strTmp);
			
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_SSL_CONFIG, &ssl_urc_param);
		}
		else if(Ql_memcmp(strTmp,"\"debug\"",Ql_strlen("\"debug\"")) == 0)
		{
			Ql_memset(strTmp, 0x0, sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,3);
    		ssl_urc_param.debug_level = Ql_atoi(strTmp);
			
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_SSL_CONFIG, &ssl_urc_param);
		}
		else if(Ql_memcmp(strTmp,"\"cacert\"",Ql_strlen("\"cacert\"")) == 0)
		{
			Ql_memset(strTmp, 0x0, sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,3);
    		ssl_urc_param.checksum = Ql_atoi(strTmp);
			
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_SSL_CONFIG, &ssl_urc_param);
		}
		else if(Ql_memcmp(strTmp,"\"clientcert\"",Ql_strlen("\"clientcert\"")) == 0)
		{
			Ql_memset(strTmp, 0x0, sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,3);
    		ssl_urc_param.checksum = Ql_atoi(strTmp);
			
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_SSL_CONFIG, &ssl_urc_param);
		}
		else if(Ql_memcmp(strTmp,"\"clientkey\"",Ql_strlen("\"clientkey\"")) == 0)
		{
			Ql_memset(strTmp, 0x0, sizeof(strTmp));
    	    QSDK_Get_Str(p1,strTmp,3);
    		ssl_urc_param.checksum = Ql_atoi(strTmp);
			
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_SSL_CONFIG, &ssl_urc_param);
		}
	}
}

static void OnURCHandler_SSL_OPEN(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char strTmp[50]; 
	
	p1 = Ql_strstr(strURC, "\r\n+QSSLOPEN:");
	p1 += Ql_strlen("\r\n+QSSLOPEN:");
	p1++;
	
	if (p1)
	{
    	Ql_memset(strTmp, 0x0, sizeof(strTmp));
    	QSDK_Get_Str(p1,strTmp,0);
    	ssl_urc_param.contextid = Ql_atoi(strTmp);

    	Ql_memset(strTmp, 0x0, sizeof(strTmp));
    	QSDK_Get_Str(p1,strTmp,1);
    	ssl_urc_param.connectid = Ql_atoi(strTmp);
		
    	Ql_memset(strTmp, 0x0, sizeof(strTmp));
    	QSDK_Get_Str(p1,strTmp,2);
    	ssl_urc_param.err = Ql_atoi(strTmp);

		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_SSL_OPEN, &ssl_urc_param);
	}
}

static void OnURCHandler_SSL_CLOSE(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char strTmp[50]; 
	
	p1 = Ql_strstr(strURC, "\r\n+QSSLCLOSE:");
	p1 += Ql_strlen("\r\n+QSSLCLOSE:");
	p1++;
	
	if (p1)
	{
    	Ql_memset(strTmp, 0x0, sizeof(strTmp));
    	QSDK_Get_Str(p1,strTmp,0);
    	ssl_urc_param.contextid = Ql_atoi(strTmp);

    	Ql_memset(strTmp, 0x0, sizeof(strTmp));
    	QSDK_Get_Str(p1,strTmp,1);
    	ssl_urc_param.connectid = Ql_atoi(strTmp);
		
    	Ql_memset(strTmp, 0x0, sizeof(strTmp));
    	QSDK_Get_Str(p1,strTmp,2);
    	ssl_urc_param.err = Ql_atoi(strTmp);

		Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_SSL_CLOSE, &ssl_urc_param);
	}
}

static void OnURCHandler_SSL_URC(const char* strURC, void* reserved)
{
	char* p1 = NULL;
	char strTmp[50]; 
	
	p1 = Ql_strstr(strURC, "\r\n+QSSLURC:");
	p1 += Ql_strlen("\r\n+QSSLURC:");
	p1++;

	if (p1)
    {	
	   Ql_memset(strTmp, 0x0,  sizeof(strTmp));
	   QSDK_Get_Str(p1,strTmp,0); 
	   RIL_URC_DEBUG(DBG_Buffer,"OnURCHandler_LwM2M_URC(%s)\r\n",strTmp);

	   if(Ql_memcmp(strTmp,"\"closed\"",Ql_strlen("\"closed\"")) == 0)
	   {
			Ql_memset(strTmp, 0x0, sizeof(strTmp));
			QSDK_Get_Str(p1,strTmp,1);
			ssl_urc_param.contextid = Ql_atoi(strTmp);
			
			Ql_memset(strTmp, 0x0, sizeof(strTmp));
			QSDK_Get_Str(p1,strTmp,2);
			ssl_urc_param.connectid = Ql_atoi(strTmp);
			
			Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_SSL_DISC, &ssl_urc_param);	
		}
	}
}

static void OnURCHandler_Undefined(const char* strURC, void* reserved)
{
    Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_END, 0);
}

/*****************************************************************
* Function:     OnURCHandler 
* 
* Description:
*               This function is the entrance for Unsolicited Result Code (URC) Handler.
*
* Parameters:
*               strURC:      
*                   [IN] a URC string terminated by '\0'.
*
*               reserved:       
*                   reserved, can be NULL.
* Return:        
*               The function returns "ptrUrc".
*****************************************************************/
void OnURCHandler(const char* strURC, void* reserved)
{
    s32 i;
    
    if (NULL == strURC)
    {
        return;
    }
    // For system URCs
    for (i = 0; i < NUM_ELEMS(m_SysURCHdlEntry); i++)
    {
        if (Ql_strstr(strURC, m_SysURCHdlEntry[i].keyword))
        {
            m_SysURCHdlEntry[i].handler(strURC, reserved);
            return;
        }
    }

    // For AT URCs
    for (i = 0; i < NUM_ELEMS(m_AtURCHdlEntry); i++)
    {
        if (Ql_strstr(strURC, m_AtURCHdlEntry[i].keyword))
        {
            m_AtURCHdlEntry[i].handler(strURC, reserved);
            return;
        }
    }

    // For undefined URCs
    OnURCHandler_Undefined(strURC, reserved);
}

/******************************************************************************
* Function:     Ql_RIL_IsURCStr
*  
* Description:
*               This function is used to check whether a string is URC information
*               you defined.
.
* Parameters:    
*               strRsp: 
*                     [in]a string for the response of the AT command.
* Return:  
*               0 : not URC information
*               1 : URC information
******************************************************************************/
s32 Ql_RIL_IsURCStr(const char* strRsp)
{
    s32 i;
    for (i = 0; i < NUM_ELEMS(m_SysURCHdlEntry); i++) 
    {
        if (Ql_strstr(strRsp, m_SysURCHdlEntry[i].keyword)) 
        {
            return 1;
        }
    }
    for (i = 0; i < NUM_ELEMS(m_AtURCHdlEntry); i++) 
    {
        if (Ql_strstr(strRsp, m_AtURCHdlEntry[i].keyword)) 
        {
            return 1;
        }
    }
    return 0;
}

#endif  // __OCPU_RIL_SUPPORT__
