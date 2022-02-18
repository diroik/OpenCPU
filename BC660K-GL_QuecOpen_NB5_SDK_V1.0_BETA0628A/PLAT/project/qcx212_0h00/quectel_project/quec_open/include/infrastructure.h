/*
 * infrastructure.h
 *
 *  Created on: 10 дек. 2021 г.
 *      Author: DROIK
 */

#ifndef PROJECT_QCX212_0H00_QUECTEL_PROJECT_QUEC_OPEN_INCLUDE_INFRASTRUCTURE_H_
#define PROJECT_QCX212_0H00_QUECTEL_PROJECT_QUEC_OPEN_INCLUDE_INFRASTRUCTURE_H_

#include "ql_type.h"
#include "ql_dbg.h"
#include "ql_error.h"
#include "ql_gpio.h"
#include "ql_uart.h"
#include "ql_power.h"
#include "ril_network.h"
#include "typedef.h"

static float GetKoeff(float R);
float GetTempValue(u32 adcValue);
s32 GetInputValue(Enum_PinName *pin, s32 *cnt, u32 max_timeout, bool INV);

s32 ReadSerialPort(Enum_SerialPort port, /*[out]*/char* pBuffer, /*[in]*/u32 bufLen);
//char *Parse_Command(char *src_str, char *tmp_buff, sProgrammSettings *sett_in_ram, sProgrammData *programmData);
//char *set_cmd(char *cmdstr, char *tmp_buff, sProgrammSettings* sett_in_ram, sProgrammData *programmData);
//char *get_cmd(char *cmd, char *tmp_buff, sProgrammSettings* sett_in_ram, sProgrammData *programmData);
//char *get_aut_cmd(char *cmdstr, char *tmp_buff, sProgrammSettings* sett_in_ram, sProgrammData *programmData);

void reboot(sProgrammData *programmData);
//char *Gsm_GetSignal(char *tmp_buff);


#endif /* PROJECT_QCX212_0H00_QUECTEL_PROJECT_QUEC_OPEN_INCLUDE_INFRASTRUCTURE_H_ */
