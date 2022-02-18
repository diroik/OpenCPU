/*
 * convert.c
 *
 *  Created on: 10 дек. 2021 г.
 *      Author: DROIK
 */

#include "convert.h"
#include "ril_util.h"

char base46_map[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                     'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                     'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                     'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};


/***********************************************
Encodes ASCCI string into base64 format string
@param plain ASCII string to be encoded
@return encoded base64 format string
***********************************************/
char* base64_encode(char* plain)
{

    int counts = 0;
    char buffer[3];
    char* cipher = malloc(strlen(plain) * 4 / 3 + 4);
    int i = 0, c = 0;

    for(i = 0; plain[i] != '\0'; i++) {
        buffer[counts++] = plain[i];
        if(counts == 3) {
            cipher[c++] = base46_map[buffer[0] >> 2];
            cipher[c++] = base46_map[((buffer[0] & 0x03) << 4) + (buffer[1] >> 4)];
            cipher[c++] = base46_map[((buffer[1] & 0x0f) << 2) + (buffer[2] >> 6)];
            cipher[c++] = base46_map[buffer[2] & 0x3f];
            counts = 0;
        }
    }

    if(counts > 0) {
        cipher[c++] = base46_map[buffer[0] >> 2];
        if(counts == 1) {
            cipher[c++] = base46_map[(buffer[0] & 0x03) << 4];
            cipher[c++] = '=';
        } else {                      // if counts == 2
            cipher[c++] = base46_map[((buffer[0] & 0x03) << 4) + (buffer[1] >> 4)];
            cipher[c++] = base46_map[(buffer[1] & 0x0f) << 2];
        }
        cipher[c++] = '=';
    }

    cipher[c] = '\0';   /* string padding character */
    return cipher;
}

/***********************************************
decodes base64 format string into ASCCI string
@param plain encoded base64 format string
@return ASCII string to be encoded
***********************************************/
char* base64_decode(char* cipher)
{

    int counts = 0;
    char buffer[4];
    char* plain = malloc(strlen(cipher) * 3 / 4);
    int i = 0, p = 0;

    for(i = 0; cipher[i] != '\0'; i++) {
        int k;
        for(k = 0 ; k < 64 && base46_map[k] != cipher[i]; k++);
        buffer[counts++] = k;
        if(counts == 4) {
            plain[p++] = (buffer[0] << 2) + (buffer[1] >> 4);
            if(buffer[2] != 64)
                plain[p++] = (buffer[1] << 4) + (buffer[2] >> 2);
            if(buffer[3] != 64)
                plain[p++] = (buffer[2] << 6) + buffer[3];
            counts = 0;
        }
    }

    plain[p] = '\0';    /* string padding character */
    return plain;
}

/***********************************************
***********************************************/
int HexToByte(char *ptr)
{
   char ch  = *ptr++;
   int k   = 0;
   char result[2] = {0,0};

    while(k < 2)
    {
          if ((ch >= 'a') && (ch) <= 'z'){
              ch = (char)toupper(ch);
          }

		  if( (ch >= 'A') && (ch <= 'F') ){
			  result[k] = (ch - 'A') + 10;
		  }
		  else if( (ch >= '0') && (ch <= '9') ){
			  result[k] = ch - '0';
		  }
		  else{
			  return -1;
		  }
		  ch = *ptr++;
		  k++;
    }
    return (result[0] <<= 4) + result[1];
}

/***********************************************
***********************************************/
void ByteToHex(char *HEX, char BYTE)
{
 char ch = (BYTE >> 4) & 0x0F;
 int k  = 0;

   while(k < 2)
   {
     if(ch > 9)
     {  *HEX++ = ch + 'A' - 10;}
     else
     {  *HEX++ = ch + '0';}

     ch = BYTE & 0x0F;
     k++;
   }
   HEX = NULL;
}

/***********************************************
***********************************************/
u32 convertToHex(char *dest, char *src, u32 len)
{
	char bf[3] = {0,0,0};
	for(int i = 0; i < len; i++){
		ByteToHex(bf, src[i]); bf[2] = 0;
		strcat(dest, bf);
	}
	return strlen(dest);
}

/***********************************************
***********************************************/
u32 convertFromHex(char *dest, char *src, u32 len)
{
	u32 l = len/2;
	for(int i = 0; i < l; i++){
		int j = i*2;
		dest[i] = (char)HexToByte(&src[j]);
	}
	return l;
}

/*****************************************************************************
*****************************************************************************/
int clear_all_nulls(char *_ptr, int _size)
{
        //
        char *ptrDst;
        char *ptrSrc;

        for(int i = 0; i <= _size; i++)
        {
          ptrDst = &_ptr[i];
          if( *ptrDst < ' ')
          {
            ptrSrc = &_ptr[i+1];
            int rsz = (_size-i);
            for(int j = 0; j < rsz; j++)
            {
              ptrDst[j] = ptrSrc[j];
            }
            if(rsz > 0)
            {
              _size--;
              i--;
            }
          }
        }
        _ptr[_size] = 0;
        return _size;
}
