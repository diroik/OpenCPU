#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ril.h"
#include "ql_type.h"
#include "cmsis_os2.h"
#include "ql_type.h"
#include "ql_dbg.h"
#include "ril_dfota.h"
#include "ril_system.h"
#include "ril_mqtt.h"


#define RIL_URC_DBG_ENABLE 0
#if RIL_URC_DBG_ENABLE > 0
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

osMessageQueueId_t maintask_queue = NULL;
/************************************************************************/
/* Definition for URC param.                                  */
/************************************************************************/				
MQTT_Urc_Param_t   mqtt_urc_param =  {0,0,255,255,255,255,{255}}; //<for mqtt

/************************************************************************/


/******************NTP********************************************/
extern void OnURCHandler_NTPCMD(const char* strURC, u32 Len);


/******************MQTT********************************************/

/******************DFOTA********************************************/
static void OnURCHandler_DFOTA_Hander  (const char* strURC, u32 Len);
/********************PSM***********************************************/


static void OnURCHandler_NbiotEvent(const char* strURC, u32 Len);

/*************** ***MQTT********************************************/
static void OnURCHandler_MQTT_OPEN(const char* strURC, u32 Len);
static void OnURCHandler_MQTT_CONN(const char* strURC, u32 Len);
static void OnURCHandler_MQTT_SUB(const char* strURC, u32 Len);
static void OnURCHandler_MQTT_PUB(const char* strURC, u32 Len);
static void OnURCHandler_MQTT_TUNS(const char* strURC, u32 Len);
static void OnURCHandler_MQTT_STATE(const char* strURC, u32 Len);
static void OnURCHandler_MQTT_CLOSE(const char* strURC, u32 Len);
static void OnURCHandler_MQTT_DISC(const char* strURC, u32 Len);

/************************************************************************/
/* Customer ATC URC callback                                          */
/************************************************************************/

/****************************************************/
/* Definitions for system URCs and the handler      */
/****************************************************/
const static ST_URC_HDLENTRY m_SysURCHdlEntry[] = {

    {"\r\n+QNBIOTEVENT:",                         OnURCHandler_NbiotEvent},

};

/****************************************************/
/* Definitions for AT URCs and the handler          */
/****************************************************/
const static ST_URC_HDLENTRY m_AtURCHdlEntry[] = {
	//NTP unsolicited response
	{"\r\n+QNTP:",							  	  OnURCHandler_NTPCMD},
    //MQTT unsolicited response
	{"\r\n+QMTOPEN:",							  OnURCHandler_MQTT_OPEN},
	{"\r\n+QMTCONN:",							  OnURCHandler_MQTT_CONN},
	{"\r\n+QMTSUB:",							  OnURCHandler_MQTT_SUB},
	{"\r\n+QMTPUB:",							  OnURCHandler_MQTT_PUB},
	{"\r\n+QMTUNS:",							  OnURCHandler_MQTT_TUNS},  
	{"\r\n+QMTSTAT:",							  OnURCHandler_MQTT_STATE},
	{"\r\n+QMTCLOSE:",							  OnURCHandler_MQTT_CLOSE},
	{"\r\n+QMTDISC:",							  OnURCHandler_MQTT_DISC},
	//DFOTA  unsolicited response
	{"\r\n+QIND: \"FOTA\"",                       OnURCHandler_DFOTA_Hander},	
};

static void OnURCHandler_Undefined(const char* strURC, u32 Len)
{
	APP_DEBUG("[OnURCHandler_Undefined]URC:[%s]-->\r\n",strURC);
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
void OnURCHandler(const char* strURC, u32 Len)
{
    s32 i;
    
    if (NULL == strURC)
    {
        return;
    }
	APP_DEBUG("OnURCHandler[%s]",(u8* )strURC);

	// For QMTRECV MSG
	extern bool Ql_RIL_Protocol_Handle(const char* strURC,const u32 length);
    if(Ql_RIL_Protocol_Handle((char*)strURC,Len-1))//cut \0
	{
		return;
	}
	
    // For system URCs
    for (i = 0; i < NUM_ELEMS(m_SysURCHdlEntry); i++)
    {
        if (strstr(strURC, m_SysURCHdlEntry[i].keyword))
        {
            m_SysURCHdlEntry[i].handler(strURC, Len);
            return;
        }
    }

    // For AT URCs
    for (i = 0; i < NUM_ELEMS(m_AtURCHdlEntry); i++)
    {
        if (strstr(strURC, m_AtURCHdlEntry[i].keyword))
        {
            m_AtURCHdlEntry[i].handler(strURC, Len);
            return;
        }
    }

    // For undefined URCs
    OnURCHandler_Undefined(strURC, Len);
}

static void OnURCHandler_DFOTA_Hander(const char* strURC, u32 Len)
{
    Dfota_Upgrade_State upgrade_state = DFOTA_STATE_END;
	s32 dfota_errno; 

	APP_DEBUG("OnURCHandler_DFOTA_Hander(%s)\r\n", strURC);
    APP_DEBUG("DFOTA_Hander:%s",(u8* )strURC);
    
	if (strstr(strURC, "\r\n+QIND: \"FOTA\"") != NULL)
	{
		DFOTA_Analysis((u8* )strURC,&upgrade_state,&dfota_errno);
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

static void OnURCHandler_NbiotEvent(const char* strURC, u32 Len)
{
	APP_DEBUG("[OnURCHandler_NbiotEvent]URC:[%s]-->\r\n",strURC);
}

static void OnURCHandler_MQTT_OPEN(const char* strURC, u32 Len)
{ 
	ST_MSG msg;
    char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10] = {0x00};
	
	p1 = strstr(strURC, "+QMTOPEN:");
	p1 += strlen("+QMTOPEN:");
	p1++;
	p2 = strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= atoi(strTmp);
		
		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.result= atoi(strTmp);
		
		msg.message = MSG_ID_URC_INDICATION;	
		msg.param1 = URC_MQTT_OPEN;
		msg.param2 = (u32)&mqtt_urc_param;
		osMessageQueuePut(maintask_queue, &msg, 0,0);
	}
}

static void OnURCHandler_MQTT_CONN(const char* strURC, u32 Len)
{
	ST_MSG msg;
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10] = {0x00};
	
	p1 = strstr(strURC, "+QMTCONN:");
	p1 += strlen("+QMTCONN:");
	p1++;
	p2 = strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= atoi(strTmp);
		
		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.result= atoi(strTmp);
		
		memset(strTmp, 0x0,	sizeof(strTmp));
		if(TRUE == QSDK_Get_Str(p1,strTmp,2))
		{
		  mqtt_urc_param.connect_code= atoi(strTmp);
		}
		else
		{
			mqtt_urc_param.connect_code= 255;
		}
		
		msg.message = MSG_ID_URC_INDICATION;	
		msg.param1 = URC_MQTT_CONN;
		msg.param2 = (u32)&mqtt_urc_param;
		osMessageQueuePut(maintask_queue, &msg, 0,0);		
	}
}

static void OnURCHandler_MQTT_SUB(const char* strURC, u32 Len)
{
	ST_MSG msg;
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10] = {0x00};
	char i;
	
	p1 = strstr(strURC, "+QMTSUB:");
	p1 += strlen("+QMTSUB:");
	p1++;
	p2 = strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= atoi(strTmp);
		
		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.msgid= atoi(strTmp);

		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		mqtt_urc_param.result= atoi(strTmp);

		for(i=0;i<MQTT_MAX_TOPIC;i++)
        {
			memset(strTmp, 0x0,	sizeof(strTmp));
     		if(TRUE == QSDK_Get_Str(p1,strTmp,(3+i)))
     		{
     		  mqtt_urc_param.sub_value[i]= atoi(strTmp);
     		}
     		else
     		{
     			mqtt_urc_param.sub_value[i]= 255;
				break;
     		}
		}
		
		msg.message = MSG_ID_URC_INDICATION;	
		msg.param1 = URC_MQTT_SUB;
		msg.param2 = (u32)&mqtt_urc_param;
		osMessageQueuePut(maintask_queue, &msg, 0,0);
	}
}

static void OnURCHandler_MQTT_PUB(const char* strURC, u32 Len)
{
	ST_MSG msg;
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10] = {0x00};

	p1 = strstr(strURC, "+QMTPUB:");
	p1 += strlen("+QMTPUB:");
	p1++;
	p2 = strstr(p1, "\r\n");
	*p2 = '\0';
	
	if (p1)
	{
		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= atoi(strTmp);
		
		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.msgid= atoi(strTmp);

		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		mqtt_urc_param.result= atoi(strTmp);

  		memset(strTmp, 0x0,	sizeof(strTmp));
		if(TRUE == QSDK_Get_Str(p1,strTmp,3))
		{
		   mqtt_urc_param.pub_value= atoi(strTmp);
		}
		else
		{
			mqtt_urc_param.pub_value= 255;
		}
		
		msg.message = MSG_ID_URC_INDICATION;	
		msg.param1 = URC_MQTT_PUB;
		msg.param2 = (u32)&mqtt_urc_param;
		osMessageQueuePut(maintask_queue, &msg, 0,0);
	}
}

static void OnURCHandler_MQTT_TUNS(const char* strURC, u32 Len)
{
	ST_MSG msg;
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	
	p1 = strstr(strURC, "+QMTUNS:");
	p1 += strlen("+QMTUNS:");
	p1++;
	p2 = strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= atoi(strTmp);
		
		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.msgid= atoi(strTmp);

		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		mqtt_urc_param.result= atoi(strTmp);
		
		msg.message = MSG_ID_URC_INDICATION;	
		msg.param1 = URC_MQTT_TUNS;
		msg.param2 = (u32)&mqtt_urc_param;
		osMessageQueuePut(maintask_queue, &msg, 0,0);
	}
}

static void OnURCHandler_MQTT_STATE(const char* strURC, u32 Len)
{
	ST_MSG msg;
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	
	p1 = strstr(strURC, "+QMTSTAT:");
	p1 += strlen("+QMTSTAT:");
	p1++;
	p2 = strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= atoi(strTmp);

		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.mqtt_state= atoi(strTmp);
		
		msg.message = MSG_ID_URC_INDICATION;	
		msg.param1 = URC_MQTT_STATE;
		msg.param2 = (u32)&mqtt_urc_param;
		osMessageQueuePut(maintask_queue, &msg, 0,0);
	}
}

static void OnURCHandler_MQTT_CLOSE(const char* strURC, u32 Len)
{
	ST_MSG msg;
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	p1 = strstr(strURC, "+QMTCLOSE:");
	p1 += strlen("+QMTCLOSE:");
	p1++;
	p2 = strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= atoi(strTmp);
		
		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.msgid= atoi(strTmp);

		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		mqtt_urc_param.result= atoi(strTmp);
		
		msg.message = MSG_ID_URC_INDICATION;	
		msg.param1 = URC_MQTT_CLOSE;
		msg.param2 = (u32)&mqtt_urc_param;
		osMessageQueuePut(maintask_queue, &msg, 0,0);
	}
}

static void OnURCHandler_MQTT_DISC(const char* strURC, u32 Len)
{
	ST_MSG msg;
	char* p1 = NULL;
	char* p2 = NULL;
	char strTmp[10];
	
	p1 = strstr(strURC, "+QMTDISC:");
	p1 += strlen("+QMTDISC:");
	p1++;
	p2 = strstr(p1, "\r\n");
	*p2 = '\0';

	if (p1)
	{
		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,0);
		mqtt_urc_param.connectid= atoi(strTmp);
		
		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,1);
		mqtt_urc_param.msgid= atoi(strTmp);

		memset(strTmp, 0x0,	sizeof(strTmp));
		QSDK_Get_Str(p1,strTmp,2);
		mqtt_urc_param.result= atoi(strTmp);
		
		msg.message = MSG_ID_URC_INDICATION;	
		msg.param1 = URC_MQTT_DISC;
		msg.param2 = (u32)&mqtt_urc_param;
		osMessageQueuePut(maintask_queue, &msg, 0,0);
	}
}

