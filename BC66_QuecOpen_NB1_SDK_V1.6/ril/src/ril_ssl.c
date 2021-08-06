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
 *   ril_ssl.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   The module implements SSL related APIs.
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
#include "ril_ssl.h"
#include "ril.h"
#include "ril_util.h"
#include "ql_stdlib.h"
#include "ql_error.h"
#include "ql_system.h"
#include "ql_trace.h"
#include "ql_memory.h"
#include "ql_uart.h"

#ifdef __OCPU_RIL_SUPPORT__

#define RIL_SSL_DEBUG_ENABLE 0
#if RIL_SSL_DEBUG_ENABLE > 0
#define RIL_SSL_DEBUG_PORT  UART_PORT0
static char DBG_Buffer[1024];
#define RIL_SSL_DEBUG(BUF,...)  QL_TRACE_LOG(RIL_SSL_DEBUG_PORT,BUF,1024,__VA_ARGS__)
#else
#define RIL_SSL_DEBUG(BUF,...)
#endif

static s32 ATResponse_Handler(char* line, u32 len, void* userData)
{
    RIL_SSL_DEBUG(DBG_Buffer,"[ATResponse_Handler] %s\r\n", (u8*)line);

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
    return RIL_ATRSP_CONTINUE; //continue wait
}

static s32 ATResponse_Cfg_Handler(char* line, u32 len, void* userData)
{
    RIL_SSL_DEBUG(DBG_Buffer,"[ATResponse_Cfg_Handler] %s\r\n", (u8*)line);

	u8 send_byte = 0x1A;
	u8 send_times = 0;

    if (Ql_RIL_FindString(line, len, "\r\n+QSSLCFG:"))
    {
        return  RIL_ATRSP_SUCCESS;
    }
	else if (Ql_RIL_FindLine(line, len, ">"))
    {	
    	Ql_Delay_ms(500);
    	ST_SSL_Datamode_Userdata_t *buf = (ST_SSL_Datamode_Userdata_t *)userData;
		
		if(buf->length > Ql_strlen(buf->pbuffer))
		{
			buf->length = Ql_strlen(buf->pbuffer);
		}
		
		while(buf->length > 1024)
		{
			Ql_RIL_WriteDataToCore(buf->pbuffer + 1024*send_times,1024);
			buf->length = buf->length - 1024;
			send_times += 1;
		}
		
		Ql_RIL_WriteDataToCore(buf->pbuffer + 1024*send_times,buf->length);

		//the byte to control send data, must reserve
		Ql_RIL_WriteDataToCore(&send_byte,1);

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
    return RIL_ATRSP_CONTINUE; //continue wait
}

static s32 ATResponse_Send_Handler(char* line, u32 len, void* userData)
{
    RIL_SSL_DEBUG(DBG_Buffer,"[ATResponse_Send_Handler] %s\r\n", (u8*)line);
	
	u8 send_byte = 0x1A;
	u8 send_times = 0;

    if (Ql_RIL_FindString(line, len, "\r\n+QSSLSEND:"))
    {
        return  RIL_ATRSP_SUCCESS;
    }
	else if (Ql_RIL_FindLine(line, len, ">"))
    {	
    	Ql_Delay_ms(500);
    	ST_SSL_Datamode_Userdata_t *buf = (ST_SSL_Datamode_Userdata_t *)userData;
		
		if(buf->length > Ql_strlen(buf->pbuffer))
		{
			buf->length = Ql_strlen(buf->pbuffer);
		}
		
		while(buf->length > 1024)
		{
			Ql_RIL_WriteDataToCore(buf->pbuffer + 1024*send_times,1024);
			buf->length = buf->length - 1024;
			send_times += 1;
		}
		
		Ql_RIL_WriteDataToCore(buf->pbuffer + 1024*send_times,buf->length);

		//the byte to control send data, must reserve
		
		if((buf->length + 1024*send_times) != 1460) // if senddata length is 1460, exit datamode automatically.
		{
			Ql_RIL_WriteDataToCore(&send_byte,1);
		}
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
    return RIL_ATRSP_CONTINUE; //continue wait
}

s32 RIL_QSSL_CFG(ST_SSL_Param_t* ssl_param_t, Enum_SSL_Cfg_Select config_flag,ST_SSL_Datamode_Userdata_t *pdata)
{
	s32 ret = RIL_AT_SUCCESS;
	char strAT[200];

	if(config_flag == Cfg_Select_Query_Mode)
	{
		if(ssl_param_t->cfg_param == Param_Item_None)
		{
			Ql_memset(strAT, 0, sizeof(strAT));
			Ql_sprintf(strAT, "AT+QSSLCFG=%d,%d\n",ssl_param_t->contextid,ssl_param_t->connectid);
			ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
			RIL_SSL_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
		}
		else if(ssl_param_t->cfg_param == Param_Item_Seclevel)
		{
			Ql_memset(strAT, 0, sizeof(strAT));
			Ql_sprintf(strAT, "AT+QSSLCFG=%d,%d,\"seclevel\"\n",ssl_param_t->contextid,ssl_param_t->connectid);
			ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
			RIL_SSL_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
		}
		else if(ssl_param_t->cfg_param == Param_Item_Dataformat)
		{
			Ql_memset(strAT, 0, sizeof(strAT));
			Ql_sprintf(strAT, "AT+QSSLCFG=%d,%d,\"dataformat\"\n",ssl_param_t->contextid,ssl_param_t->connectid);
			ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
			RIL_SSL_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
		}
		else if(ssl_param_t->cfg_param == Param_Item_Timeout)
		{
			Ql_memset(strAT, 0, sizeof(strAT));
			Ql_sprintf(strAT, "AT+QSSLCFG=%d,%d,\"timeout\"\n",ssl_param_t->contextid,ssl_param_t->connectid);
			ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
			RIL_SSL_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
		}
		else if(ssl_param_t->cfg_param == Param_Item_Debug)
		{
			Ql_memset(strAT, 0, sizeof(strAT));
			Ql_sprintf(strAT, "AT+QSSLCFG=%d,%d,\"debug\"\n",ssl_param_t->contextid,ssl_param_t->connectid);
			ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
			RIL_SSL_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
		}
	}
	else if(config_flag == Cfg_Select_Config_Mode)
	{
		if(ssl_param_t->cfg_param == Param_Item_Seclevel)
		{
			Ql_memset(strAT, 0, sizeof(strAT));
			Ql_sprintf(strAT, "AT+QSSLCFG=%d,%d,\"seclevel\",%d\n",ssl_param_t->contextid,ssl_param_t->connectid,ssl_param_t->seclevel);
			ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
			RIL_SSL_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
		}
		else if(ssl_param_t->cfg_param == Param_Item_Dataformat)
		{
			Ql_memset(strAT, 0, sizeof(strAT));
			Ql_sprintf(strAT, "AT+QSSLCFG=%d,%d,\"dataformat\",%d,%d\n",ssl_param_t->contextid,ssl_param_t->connectid,ssl_param_t->send_data_format,ssl_param_t->recv_data_format);
			ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
			RIL_SSL_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
		}
		else if(ssl_param_t->cfg_param == Param_Item_Timeout)
		{
			Ql_memset(strAT, 0, sizeof(strAT));
			Ql_sprintf(strAT, "AT+QSSLCFG=%d,%d,\"timeout\",%d\n",ssl_param_t->contextid,ssl_param_t->connectid,ssl_param_t->timeout);
			ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
			RIL_SSL_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
		}
		else if(ssl_param_t->cfg_param == Param_Item_Debug)
		{
			Ql_memset(strAT, 0, sizeof(strAT));
			Ql_sprintf(strAT, "AT+QSSLCFG=%d,%d,\"debug\",%d\n",ssl_param_t->contextid,ssl_param_t->connectid,ssl_param_t->debug_level);
			ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
			RIL_SSL_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
		}
		else if(ssl_param_t->cfg_param == Param_Item_Cacert)
		{
			if((pdata->length == 0)||(Ql_strlen(pdata->pbuffer) == 0))
			{
				return RIL_AT_INVALID_PARAM;
			}
			
			Ql_memset(strAT, 0, sizeof(strAT));
			Ql_sprintf(strAT, "AT+QSSLCFG=%d,%d,\"cacert\"\n",ssl_param_t->contextid,ssl_param_t->connectid);
			ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Cfg_Handler,pdata,20*1000);
			//RIL_SSL_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
		}
		else if(ssl_param_t->cfg_param == Param_Item_Clientcert)
		{
			if((pdata->length == 0)||(Ql_strlen(pdata->pbuffer) == 0))
			{
				return RIL_AT_INVALID_PARAM;
			}
			
			Ql_memset(strAT, 0, sizeof(strAT));
			Ql_sprintf(strAT, "AT+QSSLCFG=%d,%d,\"clientcert\"\n",ssl_param_t->contextid,ssl_param_t->connectid);
			ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Cfg_Handler,pdata,20*1000);
			//RIL_SSL_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
		}
		else if(ssl_param_t->cfg_param == Param_Item_Clientkey)
		{
			if((pdata->length == 0)||(Ql_strlen(pdata->pbuffer) == 0))
			{
				return RIL_AT_INVALID_PARAM;
			}
			
			Ql_memset(strAT, 0, sizeof(strAT));
			Ql_sprintf(strAT, "AT+QSSLCFG=%d,%d,\"clientkey\"\n",ssl_param_t->contextid,ssl_param_t->connectid);
			ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Cfg_Handler,pdata,20*1000);
			//RIL_SSL_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n", strAT, ret);
		}
	}
	
	return ret;
}

s32 RIL_QSSL_OPEN(ST_SSL_Param_t* ssl_param_t)
{
	s32 ret = RIL_AT_SUCCESS;
	char strAT[200];

	Ql_memset(strAT, 0, sizeof(strAT));
	Ql_sprintf(strAT, "AT+QSSLOPEN=%d,%d,\"%s\",%d,%d\n",ssl_param_t->contextid,ssl_param_t->connectid,\
						ssl_param_t->host_name,ssl_param_t->port,ssl_param_t->connect_mode);
	ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
	RIL_SSL_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n", strAT, ret);

	return ret;
}

s32 RIL_QSSL_SEND(u8 contextID, u8 connectID, ST_SSL_Datamode_Userdata_t *pdata)
{
	s32 ret = RIL_AT_SUCCESS;
	char strAT[200];
	
	if((pdata->length == 0)||(Ql_strlen(pdata->pbuffer) == 0))
	{
		return RIL_AT_INVALID_PARAM;
	}

	if(Ql_strlen(pdata->pbuffer) > 1460)
	{
		return RIL_AT_INVALID_PARAM;
	}
	
	Ql_memset(strAT, 0, sizeof(strAT));
	Ql_sprintf(strAT, "AT+QSSLSEND=%d,%d\n",contextID,connectID);
	ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Send_Handler,pdata,20*1000);
	//RIL_SSL_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n", strAT, ret);

	return ret;
}

s32 RIL_QSSL_CLOSE(u8 contextID, u8 connectID)
{
	s32 ret = RIL_AT_SUCCESS;
	char strAT[200];

	Ql_memset(strAT, 0, sizeof(strAT));
	Ql_sprintf(strAT, "AT+QSSLCLOSE=%d,%d\n",contextID,connectID);
	ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),NULL,NULL,0);
	RIL_SSL_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n", strAT, ret);
	
	return ret;
}

#endif
