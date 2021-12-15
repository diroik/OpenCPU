/*
 * convert.h
 *
 *  Created on: 12 окт. 2021 г.
 *      Author: Админ
 */

#ifndef CONVERT_H_
#define CONVERT_H_

#include "ql_type.h"
#include "ql_stdlib.h"
#include "ql_memory.h"
#include "typedef.h"

/***********************************************
Encodes ASCCI string into base64 format string
@param plain ASCII string to be encoded
@return encoded base64 format string
***********************************************/
char* base64_encode(char* plain);

/***********************************************
decodes base64 format string into ASCCI string
@param plain encoded base64 format string
@return ASCII string to be encoded
***********************************************/
char* base64_decode(char* cipher);

int HexToByte(char *ptr);
void ByteToHex(char *HEX, char BYTE);

u32 convertToHex(char *dest, char *src, u32 len);
u32 convertFromHex(char *dest, char *src, u32 len);

bool fromJSON(char* str, sDataJsonParams* out);
s32 toJSON(char *dst, sDataJsonParams *src);




//static bool getValue(const char *name, u32 *value);


#endif /* CONVERT_H_ */
