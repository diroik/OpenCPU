/*
 * convert.h
 *
 *  Created on: 10 дек. 2021 г.
 *      Author: DROIK
 */

#ifndef PROJECT_QCX212_0H00_QUECTEL_PROJECT_QUEC_OPEN_INCLUDE_CONVERT_H_
#define PROJECT_QCX212_0H00_QUECTEL_PROJECT_QUEC_OPEN_INCLUDE_CONVERT_H_

#include "ql_type.h"
#include "typedef.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "cmsis_os2.h"

char* base64_encode(char* plain);
char* base64_decode(char* cipher);

int HexToByte(char *ptr);
void ByteToHex(char *HEX, char BYTE);

u32 convertToHex(char *dest, char *src, u32 len);
u32 convertFromHex(char *dest, char *src, u32 len);
int clear_all_nulls(char *_ptr, int _size);

#endif /* PROJECT_QCX212_0H00_QUECTEL_PROJECT_QUEC_OPEN_INCLUDE_CONVERT_H_ */
