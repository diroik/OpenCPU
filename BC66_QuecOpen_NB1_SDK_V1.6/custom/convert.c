/*
 * convert.c
 *
 *  Created on: 12 окт. 2021 г.
 *      Author: Админ
 */

#include "convert.h"
#include "ril_util.h"

char base46_map[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                     'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                     'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                     'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};


char* base64_encode(char* plain)
{

    int counts = 0;
    char buffer[3];
    char* cipher = Ql_MEM_Alloc(Ql_strlen(plain) * 4 / 3 + 4);
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

char* base64_decode(char* cipher)
{

    int counts = 0;
    char buffer[4];
    char* plain = Ql_MEM_Alloc(Ql_strlen(cipher) * 3 / 4);
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



int HexToByte(char *ptr)
{
   char ch  = *ptr++;
   int k   = 0;
   char result[2] = {0,0};

    while(k < 2)
    {
          if ((ch >= 'a') && (ch) <= 'z'){
              ch = (char)Ql_toupper(ch);
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

u32 convertToHex(char *dest, char *src, u32 len)
{
	char bf[3] = {0,0,0};
	for(int i = 0; i < len; i++){
		ByteToHex(bf, src[i]); bf[2] = 0;
		Ql_strcat(dest, bf);
	}
	return Ql_strlen(dest);
}

u32 convertFromHex(char *dest, char *src, u32 len)
{
	u32 l = len/2;
	for(int i = 0; i < l; i++){
		int j = i*2;
		dest[i] = (char)HexToByte(&src[j]);
	}
	return l;
}


static bool getIntValue(char *src, const char *name, u32 *value)
{
	bool ret = FALSE;
	char *p1 = Ql_strchr(src, (s32)':');
	s32 len = 0;
	if(p1)
	{
		len = p1 - src;
		if(len > 0)
		{
			s32 cmp = Ql_strncmp(src, name, len);
			if(cmp == 0)
			{
				Ql_sscanf(++p1, "%ld", value);
				APP_DEBUG("getValue name=<%s>, value=<%ld>\r\n", name, *value);
				ret = TRUE;
			}
		}
	}
	return ret;
}

static bool getStringValue(char *src, const char *name, char *value)
{
	bool ret = FALSE;
	char *p1 = Ql_strchr(src, (s32)':');
	s32 len = 0;

	if(p1)
	{
		len = p1 - src;
		if(len > 0)
		{
			s32 cmp = Ql_strncmp(src, name, len);
			//APP_DEBUG("getValue name=<%s>\r\n", name);
			if(cmp == 0)
			{
				Ql_sscanf(++p1, "%s", value);
				APP_DEBUG("getValue name=<%s>, value=<%s>\r\n", name, value);
				ret = TRUE;
			}
		}
	}
	return ret;
}

static bool setStructValue(char *src, sDataJsonParams* out)
{
	bool ret = FALSE;
	u32 value = 0;
	char strValue[50] = {0};

	APP_DEBUG("setStructValue <%s>\r\n", src);

	if(getIntValue(src, "pid", &value))
	{
		out->pid = value;
		ret = TRUE;
	}
	else if(getIntValue(src,"confirm", &value))
	{
		out->confirm = value;
		ret = TRUE;
	}
	else if(getIntValue(src,"state", &value))
	{
		out->state = value;
		ret = TRUE;
	}
	else if(getIntValue(src,"rssi", &value))
	{
		out->rssi = value;
		ret = TRUE;
	}
	else if(getIntValue(src,"ber", &value))
	{
		out->ber = value;
		ret = TRUE;
	}
	else if(getIntValue(src,"voltage", &value))
	{
		out->voltage = value;
		ret = TRUE;
	}
	else if(getIntValue(src,"capacity", &value))
	{
		out->capacity = value;
		ret = TRUE;
	}
	else if(getStringValue(src,"iccid", strValue))
	{
		Ql_strcpy(out->iccid, strValue);
		ret = TRUE;
	}
	return ret;
}


bool fromJSON(char* str, sDataJsonParams* out)
{
	bool ret = FALSE;
	char src[512] = {0};
	char tmp[100] = {0};
	Ql_strcpy(src, str);
	char *p1 = Ql_strchr(src, (s32)'{');

	if(p1)
	{
		p1++;
		char *p2 = Ql_strchr(p1, (s32)'}');
		if(p2)
		{
			*p2 = 0;
			char *p3 = p1;
			u32 len 	= 0;
			u32 index 	= 0;

			p1 = Ql_strchr(p1, (s32)',');
			while(p1++)
			{
				if(p1 < p3 )
					break;
				len = p1 - p3;
				Ql_strncpy(tmp, p3, len); tmp[len] = 0;
				ret |= setStructValue(tmp, out);

				p3 = p1;
				p1 = Ql_strchr(p1, (s32)',');
			};

			len = Ql_strlen(p3);
			if(len > 0){
				Ql_strncpy(tmp, p3, len); tmp[len] = 0;
				ret |= setStructValue(tmp, out);
			}
		}
	}

	if(ret == TRUE)
	{
		APP_DEBUG("fromJSON out={pid:%lu,confirm:%d,state:%d,rssi:%d,ber:%d,voltage:%d,capacity:%d,iccid:%s}\r\n",
				out->pid,out->confirm,out->state,out->rssi,out->ber,out->voltage,out->capacity,out->iccid);
	}
	return ret;
}





