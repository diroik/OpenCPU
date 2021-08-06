
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
 *   ql_network.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   GPRS APIs definition.
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


#ifndef __QL_NETWORK__H__
#define __QL_NETWORK__H__

#define   IP_ADDR_LEN   64       //IP4_IP6_ADDR_LEN

//ip type
typedef enum
{
  IP_TYPE_IPV4,
  IP_TYPE_IPV6,
} Enum_IP_Type;

//local address
typedef struct 
{
  u8 addr[IP_ADDR_LEN];
  u16 addr_len;
}ST_Addr_Info_t;

//dns address
typedef struct 
{
  u8  primaryAddr[IP_ADDR_LEN];
  u8  bkAddr[IP_ADDR_LEN];
  u16 addr_len;
  Enum_IP_Type ip_type;
}ST_Dns_Info_t;


/*****************************************************************
* Function:     Ql_GetLocalIPAddress
*
* Description:
*               This function retrieves local IP corresponding
*               to the specified PDP context.
*
* Parameters:
*               contextId:
*                   [in]PDP context id, only support 0 . 
*
*               addr_info_t:
*                   [out] local addr information,please refer to ST_Addr_Info_t.
*
* Return:
*               SOC_SUCCESS indicates register ok;
*               SOC_ERROR_PARAM indicates the param error.
*               SOC_ERROR indicates get ip error.
*****************************************************************/
s32  Ql_GetLocalIPAddress(u8 contxtId, ST_Addr_Info_t* addr_info_t);

/*****************************************************************
* Function:     Ql_GetDNSAddress
*
* Description:
*               This function retrieves the DNS server's IP address.
*
* Parameters:
*               contextId:
*                   [in]PDP context id, only support 0 . 
*
*               dns_info_t:
*                   [out] local addr information,please refer to ST_Dns_Info_t.
*
* Return:
*               SOC_SUCCESS indicates register ok;
*               SOC_ERROR_PARAM indicates the param error.
*               SOC_ERROR indicates get dns error.
*****************************************************************/
s32  Ql_GetDNSAddress(u8 contxtId,ST_Dns_Info_t* dns_info_t);


/*****************************************************************
* Function:     Ql_SetDNSAddress
*
* Description:
*               This function retrieves the DNS server's IP address.
*
* Parameters:
*               contextId:
*                   [in]PDP context id, only support 0 . 
*
*               dns_info_t:
*                   [out] local addr information,please refer to ST_Dns_Info_t.
*
* Return:
*               SOC_SUCCESS indicates register ok;
*               SOC_ERROR_PARAM indicates the param error.
*               SOC_ERROR indicates set dns error.
*****************************************************************/
s32  Ql_SetDNSAddress(u8 contxtId,ST_Dns_Info_t* dns_info_t);


#endif  //__QL_NETWORK__H__
