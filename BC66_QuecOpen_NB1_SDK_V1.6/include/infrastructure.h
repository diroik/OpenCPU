/*
 * init.h
 *
 *  Created on: 18 авг. 2021 г.
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
#include "ril_network.h"
#include "typedef.h"
#include "flash.h"


int clear_all_nulls(char *_ptr, int _size);



s32 GetInputValue(Enum_PinName *pin, s32 *cnt, u32 max_timeout, bool INV);
//s32 GetInputValueWithLed(Enum_PinName *pin, Enum_PinName *led, s32 *cnt, u32 max_timeout, bool INV);
s32 ReadSerialPort(Enum_SerialPort port, /*[out]*/char* pBuffer, /*[in]*/u32 bufLen);
void reboot(sProgrammData *programmData);
char *Gsm_GetSignal(char *tmp_buff);

char *Parse_Command(char *src_str, char *tmp_buff, sProgrammSettings *sett_in_ram, sProgrammData *programmData);
char *set_cmd(char *cmdstr, char *tmp_buff, sProgrammSettings* sett_in_ram);
char *get_cmd(char *cmd, char *tmp_buff, sProgrammSettings* sett_in_ram);




#endif /* INFRASTRUCTURE_H_ */
