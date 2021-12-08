/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Quectel Co., Ltd. 2021
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   ril_util.c 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   The module defines the information, and APIs related to RIL.
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
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "ril_util.h"
#include "ril.h"

s32 Ql_StrPrefixMatch(const char* str, const char *prefix)
{
    for ( ; *str != '\0' && *prefix != '\0' ; str++, prefix++) {
        if (*str != *prefix) {
            return 0;
        }
    }
    return *prefix == '\0';
}

char* Ql_StrToUpper(char* str)
{
    char* pCh = str;
    if (!str)
    {
        return NULL;
    }
    for ( ; *pCh != '\0'; pCh++)
    {
        if (((*pCh) >= 'a') && ((*pCh) <= 'z'))
        {
            *pCh = toupper(*pCh);
        }
    }
    return str;
}

bool Ql_HexStrToInt(u8* str, u32* val)
{
    u16 i = 0;
    u32 temp = 0;

    //ASSERT((str != NULL) && (val != NULL));
    if (NULL == str || NULL == val)
    {
        return FALSE;
    }
    Ql_StrToUpper((char*)str);

    while (str[i] != '\0')
    {
        if (IS_NUMBER(str[i]))
        {
            temp = (temp << 4) + (str[i] - CHAR_0);
        }
        else if ((str[i] >= CHAR_A) && (str[i] <= CHAR_F))
        {
            temp = (temp << 4) + ((str[i] - CHAR_A) + 10);
        }else{
            return FALSE;
        }
        i++;
    }
    *val = temp;
    return TRUE;
}

/******************************************************************************
* Function:     Ql_RIL_FindString
*  
* Description:
*                This function is used to match string within a specified length.
*                This function is very much like strstr.
*
* Parameters:    
*                line:  
*                    [in]The address of the string.
*                len:   
*                    [in]The length of the string.
*                str:   
*                    [in]The specified item which you want to look for in the string.
*
* Return:  
                The function returns a pointer to the located string,
                or a  null  pointer  if  the specified string is not found.
******************************************************************************/
char* Ql_RIL_FindString(char *line, u32 len,char *str)
{
    s32 i;
    s32 strLen;
    char *p;

    if ((NULL == line) || (NULL == str))
        return NULL;
    
    strLen = strlen(str);
    if(strLen > len)
    {
        return NULL;
    }

    p = line;
    for (i = 0;i < len - strLen + 1; i++)
    {
        if (0 == strncmp (p, str, strLen))
        {
            return p;
        }else{
            p++;
        }
    }
    return NULL;
}

/******************************************************************************
* Function:     Ql_RIL_FindLine
*  
* Description:
*                This function is used to find the specified character line by line.
*                for example,if you want to find "OK", In fact, we think that you are
*                looking for <CR><LF>OK<CR><LF>,OK<CR><LF>,<CR>OK<CR> or <LF>OK<LF>.
*
*
* Parameters:    
*                line:  
*                    [in]The address of the string.
*                len:   
*                    [in]The length of the string.
*                str:   
*                    [in]The specified item which you want to look for in the string.
*
* Return:  
                The function returns a pointer to the located string,
                or a  null  pointer  if  the specified string is not found.
******************************************************************************/
char* Ql_RIL_FindLine(char *line, u32 len,char *str)
{
    s32 i = 0;
    s32 strLen = 0;
    char *p = NULL;
    char *pStr = NULL;
    char *pStr2 = NULL;
    char *pStr3 = NULL;

    if ((NULL == line) || (NULL == str))
        return NULL;
    
    strLen = strlen (str);
    
    pStr = malloc(strLen + 4 + 1);
    if (NULL == pStr)
         return NULL;
    
    if (len >= strLen + 4)//two \r\n
    {
        p = line;
        memset(pStr, 0, strLen + 5);
        sprintf(pStr,"\r\n%s\r\n",str);
        for (i = 0;i < len - (strLen + 4) + 1; i++)
        {
            if (0 == strncmp(p, pStr, strLen + 4))
            {
                free(pStr);
                return p;
            }else{
                p++;
            }
        }
    }

    if (len >= strLen + 2)//two \r or two\n
    {
        p = line;

        // <CR>xx<CR>
        memset(pStr, 0, strLen + 5);
        sprintf(pStr,"\r%s\r",str);

        // <LF>xx<LF>
        pStr2 = (char*)malloc(strLen + 5);
        memset(pStr2, 0, strLen + 5);
        sprintf(pStr2,"\n%s\n",str);

        // xx<CR><LF>
        pStr3 = (char*)malloc(strLen + 5);
        memset(pStr3, 0, strLen + 5);
        sprintf(pStr3,"%s\r\n",str);

        for (i = 0;i < len - (strLen + 2) + 1; i++)
        {
            if ((0 == strncmp (p, pStr, strLen + 2)) ||
                (0 == strncmp (p, pStr2, strLen + 2)) ||
                (0 == strncmp (p, pStr3, strLen + 2)))
            {
                free(pStr);
                free(pStr2);
                free(pStr3);
                pStr = NULL;
                pStr2 = NULL;
                pStr3 = NULL;
                return p;
            }else{
                p++;
            }
        }
        free(pStr2);
        free(pStr3);
        pStr2 = NULL;
        pStr3 = NULL;
    }
    free(pStr);
    pStr = NULL;
    
    return NULL;
}

