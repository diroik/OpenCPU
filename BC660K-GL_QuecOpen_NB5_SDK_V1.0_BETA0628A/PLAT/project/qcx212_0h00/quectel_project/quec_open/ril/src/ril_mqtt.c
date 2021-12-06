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
 *   ril_mqtt.c 
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
#include "ril_util.h "
#include "ril_mqtt.h"
#include "cmsis_os2.h"
#include "ql_dbg.h"

#define RIL_MQTT_DEBUG_ENABLE 0
#if RIL_MQTT_DEBUG_ENABLE > 0
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

static void ATResponse_MQTT_Handler(char* line, u32 len, void* userData)
{
    APP_DEBUG("[ATResponse_MQTT_Handler] %s\r\n", (u8*)line);

    if (Ql_RIL_FindLine(line, len, "OK"))
    {
        return;
    }
    else if (Ql_RIL_FindLine(line, len, "ERROR"))
    {
        return;
    }
    else if (Ql_RIL_FindString(line, len, "+CME ERROR"))
    {
        return;
    }
    else if (Ql_RIL_FindString(line, len, "+CMS ERROR:"))
    {
        return;
    }
    return; //continue wait
}

s32 RIL_MQTT_QMTCFG_Ali( Enum_ConnectID connectID,u8* product_key,u8* device_name,u8* device_secret)
{
    s32 ret = RIL_AT_FAILED;
    char strAT[200] = {0x00};

    memset(strAT, 0, sizeof(strAT));
    sprintf(strAT, "AT+QMTCFG=\"ALIAUTH\",%d,\"%s\",\"%s\",\"%s\"\r\n",connectID,product_key,device_name,device_secret);
    ret = Ql_RIL_SendATCmd(strAT,strlen(strAT),ATResponse_MQTT_Handler,NULL,0);
    APP_DEBUG("<--Send AT:%s, ret = %d -->\r\n",strAT, ret);

	return ret;
}

s32 RIL_MQTT_QMTCFG_Showrecvlen( Enum_ConnectID connectID,Enum_ShowFlag show_flag)
{
    s32 ret = RIL_AT_FAILED;
    char strAT[64]= {0x00};

    memset(strAT, 0, sizeof(strAT));
    sprintf(strAT, "AT+QMTCFG=\"SHOWRECVLEN\",%d,%d\r\n",connectID,show_flag);
    ret = Ql_RIL_SendATCmd(strAT,strlen(strAT),ATResponse_MQTT_Handler,NULL,0);
    APP_DEBUG("<--Send AT:%s, ret = %d -->\r\n",strAT, ret);

	return ret;
}

s32 RIL_MQTT_QMTCFG_Version_Select( Enum_ConnectID connectID,Enum_VersionNum version_num)
{
    s32 ret = RIL_AT_FAILED;
    char strAT[200] = {0x00};

    memset(strAT, 0, sizeof(strAT));
    sprintf(strAT, "AT+QMTCFG=\"VERSION\",%d,%d\r\n",connectID,version_num);
    ret = Ql_RIL_SendATCmd(strAT,strlen(strAT),ATResponse_MQTT_Handler,NULL,0);
    APP_DEBUG("<--Send AT:%s, ret = %d -->\r\n",strAT, ret);

	return ret;
}

s32 RIL_MQTT_QMTOPEN(Enum_ConnectID connectID, u8* hostName, u32 port)
{
    s32 ret = RIL_AT_FAILED;
    char strAT[200] = {0x00};
    
	memset(strAT, 0, sizeof(strAT));
    sprintf(strAT, "AT+QMTOPEN=%d,\"%s\",%d\r\n", connectID,hostName,port);	
    
    ret = Ql_RIL_SendATCmd(strAT,strlen(strAT),ATResponse_MQTT_Handler,NULL,0);
    APP_DEBUG("<--Send AT:%s, ret = %d -->\r\n",strAT, ret);

	return ret;
}

s32 RIL_MQTT_QMTCONN(Enum_ConnectID connectID, u8* clientID, u8* username, u8* password)
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200]= {0x00};

    memset(strAT, 0, sizeof(strAT));
	if((NULL != username) && (NULL !=password))
	{
        sprintf(strAT, "AT+QMTCONN=%d,\"%s\",\"%s\",\"%s\"\r\n", connectID,clientID,username,password);
	}
	else
	{
		sprintf(strAT, "AT+QMTCONN=%d,\"%s\"\r\n", connectID,clientID);
	}
    ret = Ql_RIL_SendATCmd(strAT,strlen(strAT),ATResponse_MQTT_Handler,NULL,0);
    APP_DEBUG("<--Send AT:%s, ret = %d -->\r\n",strAT, ret);

	return ret;
}

s32 RIL_MQTT_QMTSUB(Enum_ConnectID connectID, u32 msgId, ST_MQTT_topic_info_t* mqtt_topic_info_t)
{
    s32 ret = RIL_AT_SUCCESS;
    char* strAT  = NULL;
    u8* info = NULL;
	u8  temp_buffer[RIL_MQTT_TOPIC_MAX];
	u8 i = 0;

	if((mqtt_topic_info_t->count > MQTT_MAX_TOPIC)&&(mqtt_topic_info_t->count <= 0))
	{
		return RIL_AT_INVALID_PARAM;
	}
	
	strAT = (char*)malloc(sizeof(char)*RIL_MQTT_LENGTH_MAX);
	if(NULL == strAT)
	{
       return RIL_AT_INVALID_PARAM;
	}

	info = (u8*)malloc(sizeof(u8)*RIL_MQTT_INFO_MAX);
	if(NULL == info)
	{
	   ret = RIL_AT_INVALID_PARAM;
	   goto rilQMTSUBFail;
	}
    memset(info,0, sizeof(u8)*RIL_MQTT_INFO_MAX);
	for(i = 0;i< mqtt_topic_info_t->count;i++)
	{
		memset(temp_buffer,0, sizeof(u8)*RIL_MQTT_TOPIC_MAX);
		if(0 == i)
		{
			sprintf((char *)temp_buffer, "\"%s\",%d",mqtt_topic_info_t->topic[i],mqtt_topic_info_t->qos[i]);
		}
		else 
		{
        	sprintf((char *)temp_buffer, ",\"%s\",%d",mqtt_topic_info_t->topic[i],mqtt_topic_info_t->qos[i]);
		}
		strncat((char *)info,(char *)temp_buffer,strlen((char *)temp_buffer));
	}

    memset(strAT, 0x00, sizeof(u8)*RIL_MQTT_LENGTH_MAX);
    sprintf(strAT, "AT+QMTSUB=%d,%d,%s\r\n", connectID,msgId,info);
    ret = Ql_RIL_SendATCmd(strAT,strlen(strAT),ATResponse_MQTT_Handler,NULL,0);
    APP_DEBUG("<--Send AT:%s, ret = %d -->\r\n",strAT, ret);

rilQMTSUBFail:
	if(NULL != strAT)
	{
       free(strAT);
	   strAT = NULL;
	}

	if(NULL != info)
	{
       free(info);
	   info = NULL;
	}
    return ret;
}

s32 RIL_MQTT_QMTUNS(Enum_ConnectID connectID, u32 msgId,ST_MQTT_topic_info_t* mqtt_topic_info_t)
{
    s32 ret = RIL_AT_SUCCESS;
    char* strAT  = NULL;
    u8* info = NULL;
	u8  temp_buffer[RIL_MQTT_TOPIC_MAX];
	u8 i = 0;

	if((mqtt_topic_info_t->count > MQTT_MAX_TOPIC)&&(mqtt_topic_info_t->count <= 0))
	{
		return RIL_AT_INVALID_PARAM;
	}
	
	strAT = (char*)malloc(sizeof(char)*RIL_MQTT_LENGTH_MAX);
	if(NULL == strAT)
	{
       return RIL_AT_INVALID_PARAM;
	}

	info = (u8*)malloc(sizeof(u8)*RIL_MQTT_INFO_MAX);
	if(NULL == info)
	{
	   ret = RIL_AT_INVALID_PARAM;
	   goto rilQMTUNSFail;
	}

    memset(info, 0, sizeof(u8)*RIL_MQTT_INFO_MAX);

	for(i = 0;i< mqtt_topic_info_t->count;i++)
	{
		memset(temp_buffer,0, sizeof(u8)*RIL_MQTT_TOPIC_MAX);
		if(0 == i)
		{
		  sprintf((char *)temp_buffer, "\"%s\"",mqtt_topic_info_t->topic[i]);
		}
		else 
		{
          sprintf((char *)temp_buffer, ",\"%s\"",mqtt_topic_info_t->topic[i]);
		}
		strncat((char *)info, (char *)temp_buffer,strlen((char *)temp_buffer));
	}

    memset(strAT, 0, sizeof(u8)*RIL_MQTT_LENGTH_MAX);
    sprintf(strAT, "AT+QMTUNS=%d,%d,%s\r\n", connectID,msgId,info);
	
    ret = Ql_RIL_SendATCmd(strAT,strlen(strAT),ATResponse_MQTT_Handler,NULL,0);
    APP_DEBUG("<--Send AT:%s, ret = %d -->\r\n",strAT, ret);

rilQMTUNSFail:	
    if(NULL != strAT)
	{
       free(strAT);
	   strAT = NULL;
	}

	if(NULL != info)
	{
       free(info);
	   info = NULL;
	}
	
    return ret;
}

s32 RIL_MQTT_QMTCLOSE(Enum_ConnectID connectID)
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];

    memset(strAT, 0, sizeof(strAT));
	sprintf(strAT, "AT+QMTCLOSE=%d\r\n", connectID);
    ret = Ql_RIL_SendATCmd(strAT,strlen(strAT),ATResponse_MQTT_Handler,NULL,0);
    APP_DEBUG("<--Send AT:%s, ret = %d -->\r\n",strAT, ret);
 
    return ret;
}

s32 RIL_MQTT_QMTDISC(Enum_ConnectID connectID)
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];

    memset(strAT, 0, sizeof(strAT));
	sprintf(strAT, "AT+QMTDISC=%d\r\n", connectID);

    ret = Ql_RIL_SendATCmd(strAT, strlen(strAT),ATResponse_MQTT_Handler,NULL,0);
    APP_DEBUG("<--Send AT:%s, ret = %d -->\r\n",strAT, ret);

	return ret;
}

s32 RIL_MQTT_QMTPUB(Enum_ConnectID connectID, u32 msgId, Enum_Qos qos, u8 retain, u8* topic, u32 size, u8* message)
{
    s32 ret = RIL_AT_SUCCESS;
	char* strAT  = NULL;
	u8* strTopic = NULL;
    MQTT_PUB_Data pub_data;

	if(RIL_MQTT_INFO_MAX < size)
	{
		return RIL_AT_INVALID_PARAM;
	}
	
	strAT = (char*)malloc(sizeof(char)*RIL_MQTT_LENGTH_MAX);
	if(NULL == strAT)
	{
       return RIL_AT_INVALID_PARAM;
	}

	strTopic = (u8*)malloc(sizeof(u8)*RIL_MQTT_TOPIC_MAX + 1);
	if(NULL == strTopic)
	{
	   ret = RIL_AT_INVALID_PARAM;
	   goto rilQMTPUBFail; 
	}

	pub_data.datalength = size;
    pub_data.data = (u8*)malloc(size + 1);
	if(NULL == pub_data.data)
	{
	   ret = RIL_AT_INVALID_PARAM;
	   goto rilQMTPUBFail;
	}
    //pub_data.data = message;
    memset(pub_data.data, 0x00, (pub_data.datalength + 1));
    memcpy(pub_data.data, message, pub_data.datalength);

	APP_DEBUG("<--QMTPUB data:%s\r\n", pub_data.data);

	// strTopic = topic
	memset(strTopic, 0x00, (sizeof(u8)*RIL_MQTT_TOPIC_MAX + 1));
	memcpy(strTopic, topic, (sizeof(u8)*RIL_MQTT_TOPIC_MAX + 1));

	// AT Cmd
    memset(strAT, 0x00, (sizeof(u8)*RIL_MQTT_LENGTH_MAX));
	sprintf(strAT, "AT+QMTPUB=%d,%d,%d,%d,\"%s\",%d,\"%s\"\r\n", connectID,msgId,qos,retain,strTopic, pub_data.datalength, pub_data.data);
    ret = Ql_RIL_SendATCmd(strAT, strlen(strAT),ATResponse_MQTT_Handler, NULL, 0);
	APP_DEBUG("<--Send AT:%s, ret = %d -->\r\n",strAT, ret);
rilQMTPUBFail:
    if(NULL != pub_data.data)
    {
        free(pub_data.data);
		pub_data.data = NULL;
    }

	if(NULL != strTopic)
	{
		free(strTopic);
		strTopic = NULL;
	}

	if(NULL != strAT)
	{
		free(strAT);
		strAT = NULL;
	}

    return ret;
}

