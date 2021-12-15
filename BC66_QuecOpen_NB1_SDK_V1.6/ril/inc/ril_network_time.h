/*
 * ril_network_time.h
 *
 *  Created on: 15 дек. 2021 г.
 *      Author: Админ
 */

#ifndef RIL_NETWORK_TIME_H_
#define RIL_NETWORK_TIME_H_

#include "ql_type.h"

static s32 ATResponse_Handler(char* line, u32 len, void* userData);
static s32 ATResponse_CTZU_Status_Handler(char* line, u32 len, void* userData);

s32 RIL_GetTimeSynch_Status(u8* status);
s32 RIL_SetTimeSynch_Status(u8 status);

#endif /* RIL_NETWORK_TIME_H_ */
