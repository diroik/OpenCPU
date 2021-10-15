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
 *   ril_lwm2m.c 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   The module implements lwm2m related APIs.
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
#include "ql_common.h"
#include "ril_LwM2M.h"
#include "ql_uart.h"

#ifdef __OCPU_RIL_SUPPORT__

#define RIL_LwM2M_DEBUG_ENABLE 0
#if RIL_LwM2M_DEBUG_ENABLE > 0
#define RIL_LwM2M_DEBUG_PORT  UART_PORT0
static char DBG_Buffer[1024];
#define RIL_LwM2M_DEBUG(BUF,...)  QL_TRACE_LOG(RIL_LwM2M_DEBUG_PORT,BUF,1024,__VA_ARGS__)
#else
#define RIL_LwM2M_DEBUG(BUF,...) 
#endif

static s32 ATResponse_Handler(char* line, u32 len, void* userData)
{
    RIL_LwM2M_DEBUG(DBG_Buffer,"[ATResponse_Handler] %s\r\n", (u8*)line);
    
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
    else if (Ql_RIL_FindString(line, len, "+CMEE ERROR"))
    {
        return  RIL_ATRSP_FAILED;
    }
    else if (Ql_RIL_FindString(line, len, "+CMS ERROR:"))
    {
        return  RIL_ATRSP_FAILED;
    }
    return RIL_ATRSP_CONTINUE; //continue wait
}


static s32 ATResponse_Status_Handler(char* line, u32 len, void* userData)
{
	RIL_LwM2M_DEBUG(DBG_Buffer,"[ATResponse_Handler] %s\r\n", (u8*)line);
	
	char *head = Ql_RIL_FindString(line, len, "+QLWSTATUS:"); //continue wait
    if(head)
    {
        u8 *state = (u8 *)userData;
        Ql_sscanf(head,"%*[^ ]%d,%[^\r\n]",state);
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

/********************************************RIL API********************************************/
s32 RIL_QLwM2M_Cfg(ST_Lwm2m_Config_Param_t *lwm2m_cfg_t,bool psk_flag)
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[512];

    Ql_memset(strAT, 0, sizeof(strAT));
	if(FALSE == psk_flag)
	{
	    Ql_sprintf(strAT,"AT+QLWCONFIG=%d,\"%s\",%d,%s,%d,%d\n",lwm2m_cfg_t->bs_mode,\
			                           lwm2m_cfg_t->address,lwm2m_cfg_t->remote_port,\
			                           lwm2m_cfg_t->endpoint_name,lwm2m_cfg_t->lifetime,\
			                           lwm2m_cfg_t->security_mode);
	}
	else 
	{
	    Ql_sprintf(strAT,"AT+QLWCONFIG=%d,\"%s\",%d,%s,%d,%d,%s,%s\n",lwm2m_cfg_t->bs_mode,\
			                           lwm2m_cfg_t->address,lwm2m_cfg_t->remote_port,\
			                           lwm2m_cfg_t->endpoint_name,lwm2m_cfg_t->lifetime,\
			                           lwm2m_cfg_t->security_mode,lwm2m_cfg_t->psk_id,\
			                           lwm2m_cfg_t->psk);
	}
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
	
	RIL_LwM2M_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;    
}

s32 RIL_QLwM2M_Addobj(ST_Lwm2m_object_Param_t* lwm2m_object_param_t)
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];
	u8 temp[20];
	u8 i;
	
    Ql_memset(strAT,0, sizeof(strAT));
    Ql_sprintf(strAT, "AT+QLWADDOBJ=%d,%d,%d",lwm2m_object_param_t->obj_id,lwm2m_object_param_t->ins_id,lwm2m_object_param_t->res_num);

	for(i=0;i<lwm2m_object_param_t->res_num;i++)
	{
    	Ql_memset(temp,0, sizeof(temp));	
    	Ql_sprintf(temp,",%d",lwm2m_object_param_t->res_id[i]);
    	Ql_strncat(strAT,temp,Ql_strlen(temp));
	}
	{
    	Ql_memset(temp,0, sizeof(temp));	
    	Ql_sprintf(temp,"\n");
    	Ql_strncat(strAT,temp,Ql_strlen(temp));
	}
	ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
	
	RIL_LwM2M_DEBUG(DBG_Buffer,"<--Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;
}

s32 RIL_QLwM2M_Delobj(u32 obj_id) 
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];
	
    Ql_memset(strAT,0, sizeof(strAT));
	Ql_sprintf(strAT, "AT+QLWDELOBJ=%d\n",obj_id);
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
	
	RIL_LwM2M_DEBUG(DBG_Buffer,"<--Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;
}

s32 RIL_QLwM2M_Cfg_Autoreg(bool auto_reg_enable)
{
	s32 ret = RIL_AT_SUCCESS;
	char strAT[200];

	Ql_memset(strAT,0, sizeof(strAT));
	Ql_sprintf(strAT, "AT+QLWCFG=\"auto_reg\",%d\n",auto_reg_enable);
	ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
	
	RIL_LwM2M_DEBUG(DBG_Buffer,"<--Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;
}
s32 RIL_QLwM2M_Cfg_Retrans(u32 ack_timeout, u8 retrains_max_times)
{
	s32 ret = RIL_AT_SUCCESS;
	char strAT[200];

	Ql_memset(strAT,0, sizeof(strAT));
	Ql_sprintf(strAT, "AT+QLWCFG=\"retransmit\",%d,%d\n",ack_timeout, retrains_max_times);
	ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
	
	RIL_LwM2M_DEBUG(DBG_Buffer,"<--Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;	
}

s32 RIL_QLwM2M_Reg(void)
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200] ;
	
    Ql_memset(strAT,0, sizeof(strAT));
    Ql_sprintf(strAT,"AT+QLWREG\n");
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Handler,NULL,0);
	
	RIL_LwM2M_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT,ret);
    return ret;
}

s32 RIL_QLwM2M_Update(Enum_Lwm2m_Update_Mode update_mode,u32 param) 
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];

    Ql_memset(strAT,0, sizeof(strAT));
    Ql_sprintf(strAT, "AT+QLWUPDATE=%d,%d\n",update_mode, param);
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
	
	RIL_LwM2M_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;
}

s32 RIL_LwM2M_Notify(ST_LwM2M_Param_t* lwm2m_notify_param_t,bool ack_flag,bool rai_flag)
{
	s32 ret = RIL_AT_SUCCESS;
	char* strAT  = NULL;
    u8 temp[20];

	strAT = (char*)Ql_MEM_Alloc(sizeof(u8)*RIL_lWM2M_LENGTH_MAX);
	if(NULL == strAT)
	{
       return RIL_AT_INVALID_PARAM;
	}
	
	Ql_memset(strAT,0,sizeof(u8)*RIL_lWM2M_LENGTH_MAX);

	Ql_sprintf(strAT,"AT+QLWNOTIFY=%d,%d,%d,%d,%d,%s,%d",lwm2m_notify_param_t->objid,\
		                       lwm2m_notify_param_t->insid,lwm2m_notify_param_t->resid,\
		                       lwm2m_notify_param_t->value_type,lwm2m_notify_param_t->len,\
		                       lwm2m_notify_param_t->value,lwm2m_notify_param_t->index);
	
    if(TRUE == ack_flag)
	{
		Ql_memset(temp,0, sizeof(temp));
		Ql_sprintf(temp,",%d",lwm2m_notify_param_t->ack);
		Ql_strncat(strAT,temp,Ql_strlen(temp));
		
		if(TRUE == rai_flag)
		{
			Ql_memset(temp,0, sizeof(temp));
			Ql_sprintf(temp,",%d",lwm2m_notify_param_t->rai);
			Ql_strncat(strAT,temp,Ql_strlen(temp));
		}
	}
	{
    	Ql_memset(temp,0, sizeof(temp));	
    	Ql_sprintf(temp,"\n");
    	Ql_strncat(strAT,temp,Ql_strlen(temp));
    }
	ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Handler,NULL,0);
	
	RIL_LwM2M_DEBUG(DBG_Buffer,"<-- Send AT:%s,ret=(%d) -->\r\n",strAT,ret);
	if(NULL != strAT)
	{
       Ql_MEM_Free(strAT);
	   strAT  = NULL;
	}
	if(RIL_AT_SUCCESS != ret)
	{
		return ret;
	}
	return ret;
}

s32 RIL_QLwM2M_Write_Rsp(u32 msgid, Enum_Lwm2m_Result_Code result)
{
     s32 ret = RIL_AT_SUCCESS;
	 char strAT[200] ;
	
	 Ql_memset(strAT,0, sizeof(strAT));
	 Ql_sprintf(strAT,"AT+QLWWRRSP=%d,%d\n",msgid,result);
	 ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Handler,NULL,0);
	 
	 RIL_LwM2M_DEBUG(DBG_Buffer,"<-- Send AT:%s,ret=(%d) -->\r\n",strAT,ret);
	 if(RIL_AT_SUCCESS != ret)
	 {
		 return ret;
	 }
	 return ret;
}

s32 RIL_QLwM2M_Observe_Rsp(ST_LwM2M_Param_t* lwm2m_observe_param_t)
{
	s32 ret = RIL_AT_SUCCESS;
	char* strAT = NULL;
	
	strAT = (char*)Ql_MEM_Alloc(sizeof(u8)*RIL_lWM2M_LENGTH_MAX);
	if(NULL == strAT)
	{
       return RIL_AT_INVALID_PARAM;
	}
	
	Ql_memset(strAT,0,sizeof(u8)*RIL_lWM2M_LENGTH_MAX);
	Ql_sprintf(strAT,"AT+QLWOBSRSP=%d,%d,%d,%d,%d,%d,%d,%s,%d\n",lwm2m_observe_param_t->msgid,\
	           lwm2m_observe_param_t->result_code, lwm2m_observe_param_t->objid,lwm2m_observe_param_t->insid,\
		       lwm2m_observe_param_t->resid,lwm2m_observe_param_t->value_type,lwm2m_observe_param_t->len,\
		       lwm2m_observe_param_t->value,lwm2m_observe_param_t->index);
	ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Handler, NULL, 0);
	
	RIL_LwM2M_DEBUG(DBG_Buffer,"<-- Send AT:%s,ret=(%d) -->\r\n",strAT,ret);
	if(NULL != strAT)
	{
       Ql_MEM_Free(strAT);
	   strAT  = NULL;
	}
	if(RIL_AT_SUCCESS != ret)
	{
		return ret;
	}
	return ret;
}

s32 RIL_QLwM2M_Read_Rsp(ST_LwM2M_Param_t* lwm2m_observe_param_t)
{
	 s32 ret = RIL_AT_SUCCESS;
	 char* strAT = NULL;
	
	 strAT = (char*)Ql_MEM_Alloc(sizeof(u8)*RIL_lWM2M_LENGTH_MAX);
	 if(NULL == strAT)
	 {
       return RIL_AT_INVALID_PARAM;
	 }
	
	 Ql_memset(strAT,0,sizeof(u8)*RIL_lWM2M_LENGTH_MAX);
	 Ql_sprintf(strAT,"AT+QLWRDRSP=%d,%d,%d,%d,%d,%d,%d,%s,%d\n",lwm2m_observe_param_t->msgid,\
	           lwm2m_observe_param_t->result_code, lwm2m_observe_param_t->objid,lwm2m_observe_param_t->insid,\
		       lwm2m_observe_param_t->resid,lwm2m_observe_param_t->value_type,lwm2m_observe_param_t->len,\
		       lwm2m_observe_param_t->value,lwm2m_observe_param_t->index);

	 ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Handler, NULL, 0);
	 
	 RIL_LwM2M_DEBUG(DBG_Buffer,"<-- Send AT:%s,ret=(%d) -->\r\n",strAT,ret);
	 if(NULL != strAT)
	 {
       Ql_MEM_Free(strAT);
	   strAT  = NULL;
	 }
	 if(RIL_AT_SUCCESS != ret)
	 {
		 return ret;
	 }
	 return ret;
}

s32 RIL_QLwM2M_Execute_Rsp(u32 msgid,Enum_Lwm2m_Result_Code  result)
{
     s32 ret = RIL_AT_SUCCESS;
	 char strAT[200] ;
	
	 Ql_memset(strAT,0, sizeof(strAT));
	 Ql_sprintf(strAT,"AT+QLWEXERSP=%d,%d\n",msgid,result);

	 ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT), ATResponse_Handler,NULL,0);
	 
	 RIL_LwM2M_DEBUG(DBG_Buffer,"<-- Send AT:%s,ret=(%d) -->\r\n",strAT,ret);
	 if(RIL_AT_SUCCESS != ret)
	 {
		 return ret;
	 }
	 return ret;
}

s32 RIL_QLwM2M_Dereg(void) 
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];

    Ql_memset(strAT,0, sizeof(strAT));
	Ql_sprintf(strAT,"AT+QLWDEREG\n");
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Handler,NULL,0);
	
	RIL_LwM2M_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret;    
}

s32 RIL_QLwM2M_Status(u8* status)
{
    s32 ret = RIL_AT_SUCCESS;
    char strAT[200];

    Ql_memset(strAT,0, sizeof(strAT));
	Ql_sprintf(strAT,"AT+QLWSTATUS?\n");
    ret = Ql_RIL_SendATCmd(strAT,Ql_strlen(strAT),ATResponse_Status_Handler,status,0);
	
	RIL_LwM2M_DEBUG(DBG_Buffer,"<-- Send AT:%s, ret = %d -->\r\n",strAT, ret);
    return ret; 
}

#endif  //__OCPU_RIL_SUPPORT__

