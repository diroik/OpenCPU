/*
 * network_time.h
 *
 *  Created on: 25 нояб. 2021 г.
 *      Author: Админ
 */

#ifndef __RIL_NETWORK_TIME_H_
#define __RIL_NETWORK_TIME_H_

#include "ql_type.h"

static s32 ATResponse_Handler(char* line, u32 len, void* userData);
static s32 ATResponse_CTZU_Status_Handler(char* line, u32 len, void* userData);

s32 RIL_GetTimeSynch_Status(u8* status);
s32 RIL_SetTimeSynch_Status(u8 status);


#endif /* NETWORK_TIME_H_ */
