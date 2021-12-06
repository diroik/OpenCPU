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
 *   ql_ps.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   PS API defines.
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
#ifndef __QL_PS_H__
#define __QL_PS_H__

#include "cmips.h"
#include "cmicomm.h"
#include "cmimm.h"

#include "ql_type.h"
typedef enum{
    U_CAC_BASE = 1,//basic/common
    U_CAC_DEV = 2,
    U_CAC_MM = 3,
    U_CAC_PS = 4,
    U_CAC_SIM = 5,
    U_CAC_SMS = 6,

    U_CAC_MAX = 15
}EnumCacSgId;

typedef enum {
    URC_ID_START = 0,      //< URC event ID for command +CREG
    URC_GROUP_MM = (U_CAC_MM << 12),
    URC_ID_MM_SIGQ,
    URC_ID_MM_TAU_EXPIRED,
    URC_ID_MM_BLOCK_STATE_CHANGED,
    URC_ID_MM_CERES_CHANGED,
    URC_ID_MM_NITZ_REPORT,
    URC_GROUP_PS = (U_CAC_PS << 12),
    URC_ID_PS_BEARER_ACTED,
    URC_ID_PS_BEARER_DEACTED,
    URC_ID_PS_CEREG_CHANGED,
    URC_ID_PS_NETINFO,
    URC_GROUP_SIM = (U_CAC_SIM << 12),
    URC_ID_SIM_READY,      //< SIM card is ready
    URC_ID_SIM_REMOVED,    //< SIM card is removed
    URC_GROUP_DEV = (U_CAC_DEV << 12),
  

    URC_ID_END,            //< End of the URC event ID enumeration
} ENUMurcID;

typedef enum {
    GROUP_BASE_MASK = (0X01 << U_CAC_BASE),
    GROUP_DEV_MASK  = (0X01 << U_CAC_DEV),
    GROUP_MM_MASK   = (0X01 << U_CAC_MM),
    GROUP_PS_MASK   = (0X01 << U_CAC_PS),
    GROUP_SIM_MASK  = (0X01 << U_CAC_SIM),
    GROUP_SMS_MASK  = (0X01 << U_CAC_SMS),
    GROUP_ALL_MASK  = 0X7FFFFFFF
} ENUMgroupMask;



typedef struct{
    u16          mcc;
    u16          mncWithAddInfo; // if 2-digit MNC type, the 4 MSB bits should set to 'F',
    /* Example:
    *  46000; mcc = 0x0460, mnc = 0xf000
    *  00101; mcc = 0x0001, mnc = 0xf001
    *  46012; mcc = 0x0460, mnc = 0xf012
    *  460123; mcc = 0x0460, mnc = 0x0123
    */

    //DL earfcn (anchor earfcn), range 0 - 262143
    u32          earfcn;
    //the 28 bits Cell-Identity in SIB1, range 0 - 268435455
    u32          cellId;

    //physical cell ID, range 0 - 503
    u16          phyCellId;
    // value in dB, value range: -30 ~ 30
    bool            snrPresent;
    s8            snr;

    //value in units of dBm, value range: -156 ~ -44
    s16           rsrp;
    //value in units of dB, value range: -34 ~ 25
    s16           rsrq;
}Ql_SrvCellInfo;

typedef struct{
    u32          earfcn;     //DL earfcn (anchor earfcn), range 0 - 262143

    u16          phyCellId;
    u16          revd0;

    //value in units of dBm, value range: -156 ~ -44
    s16           rsrp;
    //value in units of dB, value range: -34 ~ 25
    s16           rsrq;
}Ql_NCellInfo;

#define NCELL_INFO_CELL_NUM     4
typedef struct{
    bool                sCellPresent;
    u8               nCellNum;

    Ql_SrvCellInfo    		sCellInfo;
    Ql_NCellInfo      		nCellList[NCELL_INFO_CELL_NUM];
}Ql_CellListInfo_t;



#if 1
/*
 * the netif related ip info
*/
typedef struct {
	u32 addr;
} Ql_ip4_addr_t;

typedef struct {
	u32 addr[4];
} Ql_ip6_addr_t;

#define NM_PDN_TYPE_MAX_DNS_NUM     2
typedef struct{
	Ql_ip6_addr_t	ipv6Addr;

	u8		dnsNum;
	u8		rsvd0;
	u16		ipv6GetPrefixDelay;
	Ql_ip6_addr_t	dns[NM_PDN_TYPE_MAX_DNS_NUM];
}Ql_NmNetIpv6Info;   //52 bytes

typedef struct{
	Ql_ip4_addr_t	ipv4Addr;

	u8		dnsNum;
	u8		rsvd0;
	u16		rsvd1;
	Ql_ip4_addr_t	dns[NM_PDN_TYPE_MAX_DNS_NUM];
}Ql_NmNetIpv4Info;   //16 bytes

/*
 * One NETIF info
*/
typedef struct
{
    u8               netStatus;  //NmNetifStatus
    u8               netifType;  //NmNetifType
    u8               ipType;     //NmNetIpType
    u8               rsvd0;

    /*
     * 1> if NETIF is ipv4v6 type:
     *    a> if two bearers created, "ipv4Cid" is for IPV4 bearer, and "ipv6Cid" is for IPV6 bearer
     *    b> if one bearer (ipv4v6) created, "ipv4Cid" = "ipv6Cid"
     * 2> if NETIF is ipv4 type
     *    a> "ipv6Cid" should set to NM_PS_INVALID_CID
     * 3> if NETIF is ipv6 type
     *    a> "ipv4Cid" should set to NM_PS_INVALID_CID
    */
    u8               ipv4Cid;
    u8               ipv6Cid;
    u16              rsvd1;

    Ql_NmNetIpv4Info       ipv4Info;   //first need to check "ipType", if "ipType" indicate ipv4 or ipv4v6, then this info must be valid
    Ql_NmNetIpv6Info       ipv6Info;   //first need to check "ipType", if "ipType" indicate ipv6 or ipv4v6, then this info must be valid
}Ql_NmAtiNetifInfo;    // 76 bytes


typedef struct
{
    Ql_NmAtiNetifInfo  netifInfo;
}Ql_NmAtiNetInfoInd;   // 76 bytes
#endif


/*****************************************************************
* Function:     Ql_PSCallback_Register
* 
* Description:
*		This function is used for register psCallback,psCallback indicates some PS event.
*
* Parameters:
* 		event:
* 		[in] ENUMgroupMask.
*                 
* Return:        
*		QL_RET_ERR_PARAM,param callback error.
*****************************************************************/
typedef s32 (*psCallback)(ENUMurcID eventId, void *param, u32 paramLen);
s32 Ql_PSCallback_Register(ENUMgroupMask event,psCallback callback );


/*****************************************************************
* Function:     Ql_GetSN
* 
* Description:
*		This function is used for get sn number.
*
* Parameters:
* 		str:
* 		[out] sn number.
*                 
* Return:        
*		QL_RET_ERR_PARAM,param callback error.
*****************************************************************/
s32 Ql_GetSN(char* str);


/*****************************************************************
* Function:     Ql_GetIMEI
* 
* Description:
*		This function is used for get imei number.
*
* Parameters:
* 		str:
* 		[out] imei number.
*                 
* Return:        
*		QL_RET_ERR_PARAM,param callback error.
*****************************************************************/
s32 Ql_GetIMEI(char* str);


/*****************************************************************
* Function:     Ql_GetIMSI
* 
* Description:
*		This function is used for get imsi number.
*
* Parameters:
* 		str:
* 		[out] imsi number.
*                 
* Return:        
*		QL_RET_ERR_PARAM,param callback error.
*****************************************************************/
s32 Ql_GetIMSI(char* str);


/*****************************************************************
* Function:     Ql_GetICCID
* 
* Description:
*		This function is used for get iccid number.
*
* Parameters:
* 		str:
* 		[out] iccid number.
*                 
* Return:        
*		QL_RET_ERR_PARAM,param callback error.
*****************************************************************/
s32 Ql_GetICCID(char* str);


/*****************************************************************
* Function:     Ql_GetCeregState
* 
* Description:
*		This function is used for get CEREG state.
*
* Parameters:
* 		state:
* 		[out] CAREG state.
*                 
* Return:        
*****************************************************************/
s32 Ql_GetCeregState(u8 *state);


/*****************************************************************
* Function:     Ql_GetLocInfo
* 
* Description:
*		This function is used for get current cellID and tac,same as CEREG cmd.
*
* Parameters:
* 		tac:
* 		[out] Two bytes tracking area.
* 		cellId:
* 		[out] Four bytes E-UTRAN cell ID.
*                 
* Return:        
*****************************************************************/
s32 Ql_GetLocInfo(u16 *tac, u32 *cellId);


/*****************************************************************
* Function:     Ql_GetECL
* 
* Description:
*		This function is used for get current ecl.same as QENG cmd.
*
* Parameters:
* 		
*                 
* Return:        
*		range:0-2.
*****************************************************************/
s32 Ql_GetECL(void);


/*****************************************************************
* Function:     Ql_GetPowerLvl
* 
* Description:
*		This function is used for get PLMN power level.
*
* Parameters:
*                 
* Return:        
*		range:0-3.
*****************************************************************/
s32 Ql_GetPowerLvl(void);


/*****************************************************************
* Function:     Ql_GetSignalInfo
* 
* Description:
*		This function is used for Get NB signal information.
*
* Parameters:  
*		csq:
*		[out]   *csq Pointer to signal info csq
*		            * CSQ mapping with RSSI
*		            *<rssi>: integer type
*		            * 0        -113 dBm or less
*		            * 1        -111 dBm
*		            * 2...30   -109... -53 dBm
*		            * 31       -51 dBm or greater
*		            * 99       not known or not detectable
*		snr:
*		[out]   *snr Pointer to signal info snr(value in dB, value range: -30 ~ 30);
*		rsrp:
*		[out]   *rsrp Pointer to signal info rsrp(value range: -17 ~ 97, 127);
*		            * 1> AS NB extended the RSRP value in: TS 36.133-v14.5.0, Table 9.1.4-1
*		            *   -17 rsrp < -156 dBm
*		            *   -16 -156 dBm <= rsrp < -155 dBm
*		            *    ...
*		            *   -3 -143 dBm <= rsrp < -142 dBm
*		            *   -2 -142 dBm <= rsrp < -141 dBm
*		            *   -1 -141 dBm <= rsrp < -140 dBm
*		            *   0 rsrp < -140 dBm
*		            *   1 -140 dBm <= rsrp < -139 dBm
*		            *   2 -139 dBm <= rsrp < -138 dBm
*		            *   ...
*		            *   95 -46 dBm <= rsrp < -45 dBm
*		            *   96 -45 dBm <= rsrp < -44 dBm
*		            *   97 -44 dBm <= rsrp
*		            * 2> If not valid, set to 127
*                 
* Return:        
*		QL_RET_ERR_PARAM,error param.
*****************************************************************/
s32 Ql_GetSignalInfo(u8 *csq, s8 *snr, s8 *rsrp);


/*****************************************************************
* Function:     Ql_GetCellInfo
* 
* Description:
*		This function is used for get SrvCellInfo.
*
* Parameters:
* 		bcListInfo:
* 		[out] CellListInfo.
*                 
* Return:        
*		QL_RET_ERR_PARAM,error param.
*****************************************************************/
s32 Ql_GetCellInfo(Ql_CellListInfo_t *bcListInfo);


#endif

