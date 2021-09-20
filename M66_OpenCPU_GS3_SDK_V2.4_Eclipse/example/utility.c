/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Quectel Co., Ltd. 2013
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   utility.c 
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   The APIs are used to parse string.
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

#include "ql_stdlib.h"
#include "ql_trace.h"


//**************************************************
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
//**************************************************
int index_of_char(char *src, char sim)
{

  int ret = -1;
  if(!src) return ret;
  int i = 0;
  for(;*src; src++)
  {
    if(*src == sim)
      return i;
    i++;
  }
  return ret;
}
//**************************************************
int HexToByte(char *ptr)
{
   char ch  = *ptr++;
   char k   = 0;
   char result[2] = {0,0};

    while(k < 2)
    {
      if( (ch >= 'A') && (ch <= 'F') )
      {  result[k] = (ch - 'A') + 10; }
      else if( (ch >= '0') && (ch <= '9') )
      {  result[k] = ch - '0';}
      else
      {  return -1;}
      ch = *ptr++;
      k++;
    }
    return (result[0] <<= 4) + result[1];
}
//**************************************************
void ByteToHex(char *HEX, char BYTE)
{
 char ch = (BYTE >> 4) & 0x0F;
 char k  = 0;

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

//check Symbol completeness
static bool Check_Separator(char* pCfgStr)
{
    u32 i=0,j=0,k=0;
    char* pChar1;
    char* pChar2;
    char* pConnCfg = pCfgStr;
    
    while (pChar1 = Ql_strstr(pConnCfg, "<"))
    {
        i++;
        pConnCfg = pChar1 + 1;
    }

    pConnCfg = pCfgStr;
    while (pChar1 = Ql_strstr(pConnCfg, ">"))
    {
        j++;
        pConnCfg = pChar1 + 1;
    }
    
    pConnCfg = pCfgStr;
    while (pChar1 = Ql_strstr(pConnCfg, ">,<"))
    {
        k++;
        pConnCfg = pChar1 + 1;
    }
    
    if (!((i == j)&&(i == k + 1)))
    {
        return FALSE;
    }
    return TRUE;
}

/*****************************************************************************
* Function:     Analyse_Command
*
* Description:
*               Analyse command string
*
* Parameters:
*               src_str:
*                    [in]point to string which need to analyse.
*               symbol_num:
*                    [in]symbol number, the data which want to get in the front
*                        of the symbol.
*               symbol:
*                    [in]symbol ">"
*               dest_buf:
*                    [out]Point to the buffer that save the analysed data .
* Return:
*               None
*****************************************************************************/
s32 Analyse_Command(u8* src_str, s32 symbol_num, u8 symbol, u8* dest_buf)
{
    s32 i = 0;
    u8 *p[30];
    u8 *q;
    s32 result = -1;

    if (!Check_Separator((char*)src_str))
    {
        return result;
    }
    
    if (q = (u8*)Ql_strstr((char*)src_str,"\r\n"))//remove\r\n
    {
        *q = '\0';
    }

    if (!(q = (u8*)Ql_strstr((char*)src_str,"<")))//find first'<'
    {
        return result;
    }
    p[0] = q + 1;//remove first'<'
    
    switch(symbol)
    {
        case '>':
            
            for(i=0;i<symbol_num;i++)
            {
                if (p[i+1] = (u8*)Ql_strstr((char*)p[i],">"))
                {
                    p[i+1] += 3;
                    if (i == symbol_num - 1)
                    {
                        result = 0;
                    }
                }else
                {
                    break;
                }      
            }

            if (!result)
            {
                Ql_strncpy((char*)dest_buf,(const char *)(p[i-1]),p[i]-p[i-1]-3);
            }
            break;

        default:
            result = -1;
            break;
    }

    return result;
}



