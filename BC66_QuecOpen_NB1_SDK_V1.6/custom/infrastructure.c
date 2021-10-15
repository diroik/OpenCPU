/*
 * init.c
 *
 *  Created on: 18 авг. 2021 г.
 *      Author: Админ
 */

#include "infrastructure.h"

/*****************************************************************************
* Function:
*
* Description:
*
* Parameters:
*
* Return:
*
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
/*****************************************************************************
* Function:
*
* Description:
*
* Parameters:
*
* Return:
*
*****************************************************************************/
s32 GetInputValue(Enum_PinName *pin, s32 *cnt, u32 max_timeout, bool INV)
{
	s32 ret = -1;
	s32 st = Ql_GPIO_GetLevel(*pin);

	if(INV == TRUE){
		st = st == 0 ? 1 : 0;
	}

	if(st > 0){
		if( *cnt <  max_timeout)
			*(cnt) += 1;
		if(*cnt >= max_timeout){
			*(cnt) = max_timeout;
			ret = TRUE;
		}
	}
	else{
		if( *cnt >  0)
			*(cnt) -= 1;
		if(*cnt <= 0){
			*(cnt) = 0;
			ret = FALSE;
		}
	}
	return ret;
}

/*
s32 GetInputValueWithLed(Enum_PinName *pin, Enum_PinName *led, s32 *cnt, u32 max_timeout, bool INV)
{
	s32 ret = -1;
	s32 st = Ql_GPIO_GetLevel(*pin);

	if(INV == TRUE){
		st = st == 0 ? 1 : 0;
	}

	if(led != NULL)
		Ql_GPIO_SetLevel(*led, st);
	if(st > 0){
		if( *cnt <  max_timeout)
			*(cnt) += 1;
		if(*cnt >= max_timeout)
			ret = TRUE;
	}
	else{
		if( *cnt >  0)
			*(cnt) -= 2;//!!! проверить
		if(*cnt <= 0)
			ret = FALSE;
	}
	return ret;
}*/

s32 ReadSerialPort(Enum_SerialPort port, /*[out]*/char* pBuffer, /*[in]*/u32 bufLen)
{
    s32 rdLen = 0;
    s32 rdTotalLen = 0;
    if (NULL == pBuffer || 0 == bufLen)
    {
        return -1;
    }
    Ql_memset(pBuffer, 0x0, bufLen);
    while (1)
    {
        rdLen = Ql_UART_Read(port, (u8*)(pBuffer + rdTotalLen), bufLen - rdTotalLen);
        if (rdLen <= 0)  // All data is read out, or Serial Port Error!
        {
            break;
        }
        rdTotalLen += rdLen;
        // Continue to read...
    }
    if (rdLen < 0) // Serial Port Error!
    {
        APP_DEBUG("<--Fail to read from port[%d]-->\r\n", port);
        return -99;
    }
    return rdTotalLen;
}


char *Gsm_GetSignal(char *tmp_buff)
{
    u32 rssi;
    u32 ber;
    RIL_NW_GetSignalQuality(&rssi, &ber);

    //u64 totalMS;
    //totalMS = Ql_GetMsSincePwrOn();

    Ql_sprintf(tmp_buff ,"<--signal strength: %d, BER: %d -->\r\n", rssi, ber);
    return tmp_buff;
}

#ifdef __PROJECT_SMART_BUTTON__

void reboot(sProgrammData *programmData)
{
	APP_DEBUG("<-- Rebooting -->\r\n");

	Ql_Sleep(2000);
	Ql_Reset(0);
}


char *Parse_Command(char *src_str, char *tmp_buff, sProgrammSettings *sett_in_ram, sProgrammData *programmData)//
{
	char *ret = NULL;
	if(programmData->firstInit == TRUE)
	{
		if(Ql_strcmp(src_str, "cmd reboot") == 0)
		{
			reboot(programmData);
			Ql_strcpy(tmp_buff, "\r\nrebooting\r\n");
			ret = tmp_buff;
		}
		else if(Ql_strcmp(src_str, "cmd commit") == 0)
		{
			if(write_to_flash_settings(sett_in_ram) == TRUE)
				Ql_strcpy(tmp_buff, "\r\ncommit ok\r\n");
			else
				Ql_strcpy(tmp_buff, "\r\ncommit error\r\n");
			ret = tmp_buff;
		}
		if(Ql_strcmp(src_str, "cmd deep sleep mode") == 0)
		{
			Ql_strcpy(tmp_buff, "\r\ngo to deep sleep mode\r\n");
			ret = tmp_buff;
			Ql_SleepEnable();
		}
		else
		{
			char *cmdstart = "cmd set ";
			if(Ql_strstr(src_str, cmdstart) != 0)
			{//set
				s32 len = Ql_strlen(src_str) - 	Ql_strlen(cmdstart);
				if(len > 0)
					ret = set_cmd(&src_str[Ql_strlen(cmdstart)], tmp_buff, sett_in_ram);
			}
			else
			{
				cmdstart = "cmd get ";
				if(Ql_strstr(src_str, cmdstart) != 0)
				{//get
					s32 len = Ql_strlen(src_str) - 	Ql_strlen(cmdstart);
					if(len > 0)
						ret = get_cmd(&src_str[Ql_strlen(cmdstart)], tmp_buff, sett_in_ram);
				}
			}
		}
	}
	return ret;
}

char *get_cmd(char *cmd, char *tmp_buff, sProgrammSettings* sett_in_ram)
{
  char *ret = NULL;
  char tbuff[50] = {0};

  int len = Ql_strlen(cmd);
  //APP_DEBUG("get_cmd len=<%d> cmd=<%s>\r\n", len, cmd);
  if(len > 0)
  {
	tmp_buff[0] = 0;
	if(Ql_strcmp(cmd, "apn") == 0)
    {
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=<");
      Ql_strcat(tmp_buff, sett_in_ram->gsmSettings.gprsApn);
      Ql_strcat(tmp_buff, ">\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "user") == 0)
    {
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=<");
      Ql_strcat(tmp_buff, sett_in_ram->gsmSettings.gprsUser);
      Ql_strcat(tmp_buff, ">\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "password") == 0)
    {
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=<");
      Ql_strcat(tmp_buff, sett_in_ram->gsmSettings.gprsPass);
      Ql_strcat(tmp_buff, ">\r\n");
      ret = tmp_buff;
    }

    else if(Ql_strcmp(cmd, "version") == 0)
    {
    	Ql_sprintf(tbuff ,"%s", FW_VERSION);
    	Ql_strcpy(tmp_buff, "\r\n");
      	Ql_strcat(tmp_buff, cmd);
      	Ql_strcat(tmp_buff, "=");
      	Ql_strcat(tmp_buff, tbuff);
      	Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "button timeout") == 0)
    {
    	Ql_sprintf(tbuff ,"%d", sett_in_ram->buttonTimeout);
    	Ql_strcpy(tmp_buff, "\r\n");
      	Ql_strcat(tmp_buff, cmd);
      	Ql_strcat(tmp_buff, "=");
      	Ql_strcat(tmp_buff, tbuff);
      	Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "timer timeout") == 0)
    {
    	Ql_sprintf(tbuff ,"%d", sett_in_ram->timerTimeout);
    	Ql_strcpy(tmp_buff, "\r\n");
      	Ql_strcat(tmp_buff, cmd);
      	Ql_strcat(tmp_buff, "=");
      	Ql_strcat(tmp_buff, tbuff);
      	Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "rtc interval") == 0)
    {
    	Ql_sprintf(tbuff ,"%d", sett_in_ram->rtcInterval);
    	Ql_strcpy(tmp_buff, "\r\n");
      	Ql_strcat(tmp_buff, cmd);
      	Ql_strcat(tmp_buff, "=");
      	Ql_strcat(tmp_buff, tbuff);
      	Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
	else if(Ql_strcmp(cmd, "signal") == 0)
	{
		ret = Gsm_GetSignal(tmp_buff);
	}
  }
  return ret;
}

char *set_cmd(char *cmdstr, char *tmp_buff, sProgrammSettings* sett_in_ram)
{
  char *ret = NULL;
  //char tbuff[50] = {0};
  bool r = FALSE;
  char *ch = Ql_strchr(cmdstr, '=');
  if(ch > 0)
  {
	  char cmd[50] = {0};
      char val[50] = {0};

      int len = Ql_strlen(cmdstr);
      int clen = (int)ch++ - (int)cmdstr;
      int vlen = ((int)cmdstr + len) - (int)ch;

      //APP_DEBUG("set_cmd len=<%d> clen=<%d> vlen=<%d>\r\n", len, clen, vlen);

      if(clen > 0 && vlen > 0)
      {
    	  Ql_strncpy(cmd, cmdstr, clen);
    	  Ql_strncpy(val, ch, vlen);

    	  vlen = clear_all_nulls(val, vlen);
    	  if(vlen <= 0)
    		  return NULL;

    	  if(Ql_strcmp(cmd, "apn") == 0)
    	  {
    		  if(vlen <= MAX_GPRS_APN_LEN)
    		  {
    			  Ql_memset(sett_in_ram->gsmSettings.gprsApn, 0, MAX_GPRS_APN_LEN);
    			  Ql_strncpy(sett_in_ram->gsmSettings.gprsApn, val, vlen);
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "user") == 0)
    	  {
    		  if(vlen < MAX_GPRS_USER_NAME_LEN)
    		  {
    			  Ql_memset(sett_in_ram->gsmSettings.gprsUser, 0, MAX_GPRS_USER_NAME_LEN);
    			  Ql_strncpy(sett_in_ram->gsmSettings.gprsUser, val, vlen);
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "password") == 0)
    	  {
    		  if(vlen < MAX_GPRS_PASSWORD_LEN)
    		  {
    			  Ql_memset(sett_in_ram->gsmSettings.gprsPass, 0, MAX_GPRS_PASSWORD_LEN);
    			  Ql_strncpy(sett_in_ram->gsmSettings.gprsPass, val, vlen);
    			  r = TRUE;
    		  }
    	  }

    	  else if(Ql_strcmp(cmd, "button timeout") == 0)
    	  {
    		  s32 timeout = Ql_atoi(val);
    		  if(timeout >= 100 && timeout <= 60000){ // 1 s
    			  sett_in_ram->buttonTimeout = timeout;
    			  r = TRUE;
    		  }
    	  }

    	  else if(Ql_strcmp(cmd, "timer timeout") == 0)
    	  {
    		  u64 timeout = atoll(val);
    		  if(timeout >= 10000){ // 10 s
    			  sett_in_ram->timerTimeout = timeout;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "rtc interval") == 0)
    	  {
    		  u64 timeout = atoll(val);
    		  if(timeout >= 60 && timeout <= 86400){ // in seconds
    			  sett_in_ram->rtcInterval = timeout;
    			  r = TRUE;
    		  }
    	  }
    	  if(r == TRUE){
    		  *(--ch) = 0;
    		  ret = get_cmd(cmdstr, tmp_buff, sett_in_ram);
    	  }
    	  else{
    	      Ql_strcpy(tmp_buff, "\r\n");
    	      Ql_strcat(tmp_buff, "cmd set ERROR!");
    	      Ql_strcat(tmp_buff, "\r\n");
    	      ret = tmp_buff;
    	  }
      }
  }
  return ret;
}


#else


void reboot(sProgrammData *programmData)
{
    //u64 totalMS;
    //totalMS = Ql_GetMsSincePwrOn();
	APP_DEBUG("<-- Rebooting -->\r\n");

	programmData->needReboot = TRUE;
	//Ql_Sleep(5000);
	//Ql_Reset(0);
}

/*****************************************************************************
* Function:
*
* Description:
*
* Parameters:
*
* Return:
*
*****************************************************************************/
char *Parse_Command(char *src_str, char *tmp_buff, sProgrammSettings *sett_in_ram, sProgrammData *programmData)//
{
	char *ret = NULL;
	//APP_DEBUG("Parse_Command firstInit=%d\r\n", programmData->firstInit);
	if(programmData->firstInit == TRUE)
	{
		if(Ql_strcmp(src_str, "cmd reboot") == 0)
		{
			reboot(programmData);
			Ql_strcpy(tmp_buff, "\r\nrebooting\r\n");
			ret = tmp_buff;
		}
		else if(Ql_strcmp(src_str, "cmd reconnect") == 0)
		{
			programmData->reconnectCnt = sett_in_ram->secondsToReconnect;
			Ql_strcpy(tmp_buff, "\r\nreconnecting\r\n");
			ret = tmp_buff;
		}
		else if(Ql_strcmp(src_str, "cmd commit") == 0)
		{
			if(write_to_flash_settings(sett_in_ram) == TRUE)
				Ql_strcpy(tmp_buff, "\r\ncommit ok\r\n");
			else
				Ql_strcpy(tmp_buff, "\r\ncommit error\r\n");
			ret = tmp_buff;
		}
		if(Ql_strcmp(src_str, "cmd deep sleep mode") == 0)
		{
			Ql_strcpy(tmp_buff, "\r\ngo to deep sleep mode\r\n");
			ret = tmp_buff;
			Ql_SleepEnable();

		}
		else
		{
			char *cmdstart = "cmd set ";
			if(Ql_strstr(src_str, cmdstart) != 0)
			{//set
				s32 len = Ql_strlen(src_str) - 	Ql_strlen(cmdstart);
				//Ql_Debug_Trace("come cmd len=<%d>\r\n", len);
				if(len > 0)
					ret = set_cmd(&src_str[Ql_strlen(cmdstart)], tmp_buff, sett_in_ram);
			}
			else
			{
				cmdstart = "cmd get ";
				if(Ql_strstr(src_str, cmdstart) != 0)
				{//get
					s32 len = Ql_strlen(src_str) - 	Ql_strlen(cmdstart);
					if(len > 0)
						ret = get_cmd(&src_str[Ql_strlen(cmdstart)], tmp_buff, sett_in_ram);
				}
			}
		}
	}
	return ret;
}

char *set_cmd(char *cmdstr, char *tmp_buff, sProgrammSettings* sett_in_ram)
{
  char *ret = NULL;
  //char tbuff[50] = {0};
  bool r = FALSE;
  char *ch = Ql_strchr(cmdstr, '=');
  if(ch > 0)
  {
	  char cmd[50] = {0};
      char val[50] = {0};

      int len = Ql_strlen(cmdstr);
      int clen = (int)ch++ - (int)cmdstr;
      int vlen = ((int)cmdstr + len) - (int)ch;

      //APP_DEBUG("set_cmd len=<%d> clen=<%d> vlen=<%d>\r\n", len, clen, vlen);

      if(clen > 0 && vlen > 0)
      {
    	  Ql_strncpy(cmd, cmdstr, clen);
    	  Ql_strncpy(val, ch, vlen);

    	  vlen = clear_all_nulls(val, vlen);
    	  if(vlen <= 0)
    		  return NULL;

    	  if(Ql_strcmp(cmd, "mode") == 0)
    	  {
    		  s32 mode = Ql_atoi(val);
    		  if(mode == 0)
    			  sett_in_ram->ipSettings.mode = 0;
    		  else if(mode == 1)
    			  sett_in_ram->ipSettings.mode = 1;
    		  r = TRUE;
    	  }
    	  else if(Ql_strcmp(cmd, "apn") == 0)
    	  {
    		  if(vlen <= MAX_GPRS_APN_LEN)
    		  {
    			  Ql_memset(sett_in_ram->gsmSettings.gprsApn, 0, MAX_GPRS_APN_LEN);
    			  Ql_strncpy(sett_in_ram->gsmSettings.gprsApn, val, vlen);
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "user") == 0)
    	  {
    		  if(vlen < MAX_GPRS_USER_NAME_LEN)
    		  {
    			  Ql_memset(sett_in_ram->gsmSettings.gprsUser, 0, MAX_GPRS_USER_NAME_LEN);
    			  Ql_strncpy(sett_in_ram->gsmSettings.gprsUser, val, vlen);
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "password") == 0)
    	  {
    		  if(vlen < MAX_GPRS_PASSWORD_LEN)
    		  {
    			  Ql_memset(sett_in_ram->gsmSettings.gprsPass, 0, MAX_GPRS_PASSWORD_LEN);
    			  Ql_strncpy(sett_in_ram->gsmSettings.gprsPass, val, vlen);
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "daddress") == 0)
    	  {
    		  if(vlen < MAX_ADDRESS_LEN)
    		  {
    			  Ql_memset(sett_in_ram->ipSettings.dstAddress, 0, MAX_ADDRESS_LEN);
    			  Ql_strncpy(sett_in_ram->ipSettings.dstAddress, val, vlen);
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "saddress") == 0)
    	  {
    		  if(vlen < MAX_ADDRESS_LEN)
    		  {
    			  Ql_memset(sett_in_ram->ipSettings.srcAddress, 0, MAX_ADDRESS_LEN);
    			  Ql_strncpy(sett_in_ram->ipSettings.srcAddress, val, vlen);
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "dport") == 0)
    	  {
    		  s32 port = Ql_atoi(val);
    		  if(port > 0){
    			  sett_in_ram->ipSettings.dstPort = port;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "sport") == 0)
    	  {
    		  s32 port = Ql_atoi(val);
    		  if(port > 0){
    			  sett_in_ram->ipSettings.srcPort = port;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "sertimeout") == 0)
    	  {
    		  s32 timeout = Ql_atoi(val);
    		  if(timeout > 0){
    			  sett_in_ram->serPortDataTimeout = timeout;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "gsmtimeout") == 0)
    	  {
    		  s32 timeout = Ql_atoi(val);
    		  if(timeout > 0){
    			  sett_in_ram->gsmPortDataTimeout = timeout;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "baudrate") == 0)
    	  {
    		  s32 speed = Ql_atoi(val);
    		  if(speed > 0){
        		sett_in_ram->serPortSettings.baudrate = speed;
        		r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "stopbits") == 0)
    	  {
    		  u32 value = Ql_atoi(val);
    		  if(value >= SB_ONE && value <= SB_TWO){
    			  sett_in_ram->serPortSettings.stopBits = value;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "parity") == 0)
    	  {
    		  u32 value = Ql_atoi(val);
    		  if(value >= PB_NONE && value <= PB_EVEN){
    			  sett_in_ram->serPortSettings.parity = value;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "databits") == 0)
    	  {
    		  u32 value = Ql_atoi(val);
    		  if(value >= DB_5BIT && value <= DB_8BIT){
    			  sett_in_ram->serPortSettings.dataBits = value;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "toreboot") == 0)
    	  {
    		  s32 timeout = Ql_atoi(val);
    		  if(timeout >= 1800){ //30 min
    			  sett_in_ram->secondsToReboot = timeout;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "toreconnect") == 0)
    	  {
    		  s32 timeout = Ql_atoi(val);
    		  if(timeout >= 180){ // 3 min
    			  sett_in_ram->secondsToReconnect = timeout;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "toping") == 0)
    	  {
    		  s32 timeout = Ql_atoi(val);
    		  if(timeout >= 60){ // 1 min
    			  sett_in_ram->secondsToPing = timeout;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "button timeout") == 0)
    	  {
    		  s32 timeout = Ql_atoi(val);
    		  if(timeout >= 1){ // 1 s
    			  sett_in_ram->buttonTimeout = timeout;
    			  r = TRUE;
    		  }
    	  }

    	  if(r == TRUE){
    		  *(--ch) = 0;
    		  ret = get_cmd(cmdstr, tmp_buff, sett_in_ram);
    	  }
    	  else{
    	      Ql_strcpy(tmp_buff, "\r\n");
    	      Ql_strcat(tmp_buff, "cmd set ERROR!");
    	      Ql_strcat(tmp_buff, "\r\n");
    	      ret = tmp_buff;
    	  }
      }
  }
  return ret;
}

char *get_cmd(char *cmd, char *tmp_buff, sProgrammSettings* sett_in_ram)
{
  char *ret = NULL;
  char tbuff[50] = {0};

  int len = Ql_strlen(cmd);
  //APP_DEBUG("get_cmd len=<%d> cmd=<%s>\r\n", len, cmd);
  if(len > 0)
  {
	tmp_buff[0] = 0;
    if(Ql_strcmp(cmd, "mode") == 0)
    {
	  Ql_sprintf(tbuff ,"%d", sett_in_ram->ipSettings.mode);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "apn") == 0)
    {
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, sett_in_ram->gsmSettings.gprsApn);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "user") == 0)
    {
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, sett_in_ram->gsmSettings.gprsUser);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "password") == 0)
    {
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, sett_in_ram->gsmSettings.gprsPass);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "daddress") == 0)
    {
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, sett_in_ram->ipSettings.dstAddress);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "saddress") == 0)
    {
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, sett_in_ram->ipSettings.srcAddress);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "dport") == 0)
    {
	  Ql_sprintf(tbuff ,"%d", sett_in_ram->ipSettings.dstPort);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "sport") == 0)
    {
      Ql_sprintf(tbuff ,"%d", sett_in_ram->ipSettings.srcPort);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "sertimeout") == 0)
    {
	  Ql_sprintf(tbuff ,"%d", sett_in_ram->serPortDataTimeout);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "gsmtimeout") == 0)
    {
	  Ql_sprintf(tbuff ,"%d", sett_in_ram->gsmPortDataTimeout);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "baudrate") == 0)
    {

      //APP_DEBUG("get_cmd cmd=<baudrate>, sett_in_ram->serPortSettings.baudrate=%d\r\n", sett_in_ram->serPortSettings.baudrate);

	  Ql_sprintf(tbuff ,"%d", (int)sett_in_ram->serPortSettings.baudrate);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "stopbits") == 0)
    {
      u32 value = sett_in_ram->serPortSettings.stopBits;
	  Ql_sprintf(tbuff ,"%d", value);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "parity") == 0)
    {
      u32 value = sett_in_ram->serPortSettings.parity;
      Ql_sprintf(tbuff ,"%d", value);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "databits") == 0)
    {
      u32 value = sett_in_ram->serPortSettings.dataBits;
      Ql_sprintf(tbuff ,"%d", value);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "toreboot") == 0)
    {
	  Ql_sprintf(tbuff ,"%d", sett_in_ram->secondsToReboot);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "toreconnect") == 0)
    {
	  Ql_sprintf(tbuff ,"%d", sett_in_ram->secondsToReconnect);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "toping") == 0)
    {
	  Ql_sprintf(tbuff ,"%d", sett_in_ram->secondsToPing);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "version") == 0)
    {
    	Ql_sprintf(tbuff ,"%s", FW_VERSION);
    	Ql_strcpy(tmp_buff, "\r\n");
      	Ql_strcat(tmp_buff, cmd);
      	Ql_strcat(tmp_buff, "=");
      	Ql_strcat(tmp_buff, tbuff);
      	Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "sampling count") == 0)
    {
    	Ql_sprintf(tbuff ,"%d", sett_in_ram->adcSettings.samplingCount);
    	Ql_strcpy(tmp_buff, "\r\n");
      	Ql_strcat(tmp_buff, cmd);
      	Ql_strcat(tmp_buff, "=");
      	Ql_strcat(tmp_buff, tbuff);
      	Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "sampling interval") == 0)
    {
    	Ql_sprintf(tbuff ,"%d", sett_in_ram->adcSettings.samplingInterval);
    	Ql_strcpy(tmp_buff, "\r\n");
      	Ql_strcat(tmp_buff, cmd);
      	Ql_strcat(tmp_buff, "=");
      	Ql_strcat(tmp_buff, tbuff);
      	Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "button timeout") == 0)
    {
    	Ql_sprintf(tbuff ,"%d", sett_in_ram->buttonTimeout);
    	Ql_strcpy(tmp_buff, "\r\n");
      	Ql_strcat(tmp_buff, cmd);
      	Ql_strcat(tmp_buff, "=");
      	Ql_strcat(tmp_buff, tbuff);
      	Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "input1 timeout") == 0)
    {
    	Ql_sprintf(tbuff ,"%d", sett_in_ram->in1Timeout);
    	Ql_strcpy(tmp_buff, "\r\n");
      	Ql_strcat(tmp_buff, cmd);
      	Ql_strcat(tmp_buff, "=");
      	Ql_strcat(tmp_buff, tbuff);
      	Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "input2 timeout") == 0)
    {
    	Ql_sprintf(tbuff ,"%d", sett_in_ram->in2Timeout);
    	Ql_strcpy(tmp_buff, "\r\n");
      	Ql_strcat(tmp_buff, cmd);
      	Ql_strcat(tmp_buff, "=");
      	Ql_strcat(tmp_buff, tbuff);
      	Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
	else if(Ql_strcmp(cmd, "signal") == 0)
	{
		//ret = Gsm_GetSignal(tmp_buff);
	}
  }
  return ret;
}

#endif

