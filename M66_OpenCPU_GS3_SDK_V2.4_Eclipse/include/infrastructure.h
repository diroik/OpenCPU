/*
 * infrastructure.h
 *
 *  Created on: 22 нояб. 2021 г.
 *      Author: Админ
 */

#ifndef INFRASTRUCTURE_H_
#define INFRASTRUCTURE_H_

#include "ql_type.h"
#include "ql_stdlib.h"
#include "ql_trace.h"
#include "ql_error.h"
#include "ql_gpio.h"
#include "ql_uart.h"
#include "ql_power.h"
#include "ril.h"
#include "ril_util.h"
#include "ril_sms.h"
#include "ril_sim.h"
#include "ril_telephony.h"
#include "ril_network.h"
#include "ql_gprs.h"
#include "fota_main.h"
#include "typedef.h"
#include "convert.h"
#include "flash.h"

/***********************************************************************
 *
************************************************************************/
void reboot(sProgrammData *programmData);

/***********************************************************************
 * Datchiks FUNCTION DECLARATIONS
************************************************************************/
float GetKoeff(float R);
float GetTempValue(u32 adcValue);
s32 GetInputValue(Enum_PinName *pin, s32 *cnt, u32 max_timeout);

/***********************************************************************
 * PROTOCOL DECLARATIONS
************************************************************************/
s32 ReadSerialPort(Enum_SerialPort port, /*[out]*/char* pBuffer, /*[in]*/u32 bufLen);
char *Parse_Command(char *src_str, char *tmp_buff, sProgrammSettings *sett_in_ram, sProgrammData *programmData);
char *get_aut_cmd(char *cmdstr, char *tmp_buff, sProgrammSettings* sett_in_ram, sProgrammData *programmData);
char *set_cmd(char *cmdstr, char *tmp_buff, sProgrammSettings* sett_in_ram, sProgrammData *programmData);
char *get_cmd(char *cmd, char *tmp_buff, sProgrammSettings* sett_in_ram, sProgrammData *programmData);

/***********************************************************************
 *
************************************************************************/
char *Gsm_GetSignal(char *tmp_buff);

/***********************************************************************
 * SMS FUNCTION DECLARATIONS
************************************************************************/
void Hdlr_RecvNewSMS(u32 nIndex, bool bAutoReply, sProgrammSettings *sett_in_ram, sProgrammData *programmData);
bool ConSMSBuf_IsIntact(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,u8 uIdx,ST_RIL_SMS_Con *pCon);
bool ConSMSBuf_AddSeg(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,u8 uIdx,ST_RIL_SMS_Con *pCon,u8 *pData,u16 uLen);
s8 ConSMSBuf_GetIndex(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,ST_RIL_SMS_Con *pCon);
bool ConSMSBuf_ResetCtx(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,u8 uIdx);
bool SMS_Initialize(void);


#endif /* INFRASTRUCTURE_H_ */
