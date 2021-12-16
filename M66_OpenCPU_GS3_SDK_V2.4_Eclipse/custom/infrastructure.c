/*
 * infrastructure.c
 *
 *  Created on: 22 нояб. 2021 г.
 *      Author: Админ
 */

#include "infrastructure.h"

//**************************************************
void reboot(sProgrammData *programmData)
{
    u64 totalMS;
    totalMS = Ql_GetMsSincePwrOn();
	APP_DEBUG("<--%lld: Rebooting-->\r\n", totalMS);

	programmData->needReboot = TRUE;
}
//**************************************************
float GetKoeff(float R)
{
  if(R < 802)//t <0
    return 0.88;

  else if( R >= 802 && R < 874)
  {// t>=0
    return 0.85;
  }
  else if( R >= 874 && R < 950)
  {//t=>10
    return 0.83;
  }
  else if( R >= 950 && R < 990)
  {//t=>20
    return 0.80;
  }
 /*else if( R >= 990 && R < 1029)
  {//t=>25
    return 0.83;
  }*/
  else if( R >= 1029 && R < 1108)
  {//t=>30
    return 0.78;
  }
  else if( R >= 1108 && R < 1192)
  {//t=>40
    return 0.75;
  }
  else if(R >= 1192)// t>50
    return 0.73;
  else
  {
    return 0.79;
  }
}
//**************************************************
float GetTempValue(u32 adcValue)
{
	float U = adcValue/1000.0;
	float R = U/((3.3 - U)/RESISTOR);
	float d = (R - 1000.0)/(GetKoeff(R)*10.0);
	if( d > 199 ){
		return 199.0;
	}
	return 25.0 + d;
}
//**************************************************
s32 GetInputValue(Enum_PinName *pin, s32 *cnt, u32 max_timeout)
{
	s32 ret = -1;
	s32 st = Ql_GPIO_GetLevel(*pin);
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
//**************************************************
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
//**************************************************
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
	if(programmData->firstInit == TRUE)
	{
		char *cmdstart1 = "cmd ";
		if(Ql_strstr(src_str, cmdstart1) == 0){
			return ret;
		}

		if(programmData->autCnt == 0)
		{//
			char *cmdstart = "cmd set ";
			if(Ql_strstr(src_str, "cmd set authorization=") != 0)
			{//set
				s32 len = Ql_strlen(src_str) - 	Ql_strlen(cmdstart);
				if(len > 0)
					ret = get_aut_cmd(&src_str[Ql_strlen(cmdstart)], tmp_buff, sett_in_ram, programmData);
			}
			return ret;
		}

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
		else if(Ql_strcmp(src_str, "cmd update firmware by ftp") == 0)
		{

			//u8 m_URL_Buffer[512];
			//ftp://hostname/filePath/fileName:port@username:password
			s32 strLen = Ql_sprintf(tmp_buff, "ftp://%s%s%s:%d@%s:%s",
					sett_in_ram->ftpSettings.srvAddress,
					sett_in_ram->ftpSettings.filePath,
					sett_in_ram->ftpSettings.fileName,
					sett_in_ram->ftpSettings.srvPort,
					sett_in_ram->ftpSettings.usrName,
					sett_in_ram->ftpSettings.usrPassw
					);

			//strLen = Ql_sprintf(m_URL_Buffer, "ftp://%s%s%s:%s@%s:%s",FTP_SVR_ADDR, FTP_SVR_PATH, FTP_FILENAME, FTP_SVR_PORT, FTP_USER_NAME, FTP_PASSWORD);
			ST_GprsConfig apnCfg;
			Ql_memset(&apnCfg, 0x0, sizeof(apnCfg));

			//Ql_memcpy(apnCfg.apnName, 	sett_in_ram->gsmSettings.gprsApn,  Ql_strlen(sett_in_ram->gsmSettings.gprsApn));
			//Ql_memcpy(apnCfg.apnUserId, 	sett_in_ram->gsmSettings.gprsUser, Ql_strlen(sett_in_ram->gsmSettings.gprsUser));
			//Ql_memcpy(apnCfg.apnPasswd, 	sett_in_ram->gsmSettings.gprsPass, Ql_strlen(sett_in_ram->gsmSettings.gprsPass));

			Ql_strcpy(apnCfg.apnName, 	sett_in_ram->gsmSettings.gprsApn);
			Ql_strcpy(apnCfg.apnUserId, 	sett_in_ram->gsmSettings.gprsUser);
			Ql_strcpy(apnCfg.apnPasswd, 	sett_in_ram->gsmSettings.gprsPass);


            APP_DEBUG("Ql_FOTA_StartUpgrade url=[%s], apnName=[%s] apnUserId=[%s] apnPasswd=[%s]\r\n", tmp_buff, apnCfg.apnName, apnCfg.apnUserId, apnCfg.apnPasswd);
            Ql_FOTA_StartUpgrade(tmp_buff, &apnCfg, NULL);
			ret = tmp_buff;
		}
		else if(Ql_strcmp(src_str, "cmd deep sleep mode") == 0)
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
					ret = set_cmd(&src_str[Ql_strlen(cmdstart)], tmp_buff, sett_in_ram, programmData);
			}
			else
			{
				cmdstart = "cmd get ";
				if(Ql_strstr(src_str, cmdstart) != 0)
				{//get
					s32 len = Ql_strlen(src_str) - 	Ql_strlen(cmdstart);
					if(len > 0)
						ret = get_cmd(&src_str[Ql_strlen(cmdstart)], tmp_buff, sett_in_ram, programmData);
				}
			}
		}

		if(ret != NULL)
			programmData->autCnt = AUT_TIMEOUT;//renew timeout if cmd coming

	}
	return ret;
}
//**************************************************
char *get_aut_cmd(char *cmdstr, char *tmp_buff, sProgrammSettings* sett_in_ram, sProgrammData *programmData)
{
	  char *ret = NULL;
	  bool r = FALSE;

	  char *ch = Ql_strchr(cmdstr, '=');
	  if(ch > 0)
	  {
		  char cmd[50] = {0};
	      char val[50] = {0};

	      int len = Ql_strlen(cmdstr);
	      int clen = (int)ch++ - (int)cmdstr;
	      int vlen = ((int)cmdstr + len) - (int)ch;

	      if(clen > 0 && vlen > 0)
	      {
	    	  Ql_strncpy(cmd, cmdstr, clen);
	    	  Ql_strncpy(val, ch, vlen);

	    	  vlen = clear_all_nulls(val, vlen);
	    	  if(vlen <= 0)
	    		  return NULL;

	    	  APP_DEBUG("<--get_aut_cmd cmd=<%s>, val=<%s>-->\r\n", cmd, val);
	    	  if(Ql_strcmp(cmd, "authorization") == 0)
	    	  {
	    		  if(Ql_strcmp(val, sett_in_ram->securitySettings.cmdPassw) == 0){

	    			  programmData->autCnt = AUT_TIMEOUT;
	    			  r = TRUE;
	    		  }
	    	  }

	    	  if(r == TRUE){
	    	      Ql_strcpy(tmp_buff, "\r\n");
	    	      Ql_strcat(tmp_buff, "authorization successful!");
	    	      Ql_strcat(tmp_buff, "\r\n");
	    		  ret = tmp_buff;
	    	  }
	    	  else{
	    	      Ql_strcpy(tmp_buff, "\r\n");
	    	      Ql_strcat(tmp_buff, "authorization ERROR!");
	    	      Ql_strcat(tmp_buff, "\r\n");
	    	      ret = tmp_buff;
	    	  }

	      }

	  }
	  return ret;
}
//**************************************************
char *set_cmd(char *cmdstr, char *tmp_buff, sProgrammSettings* sett_in_ram, sProgrammData *programmData)
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

      //Ql_Debug_Trace("set_cmd len=<%d> clen=<%d> vlen=<%d>\r\n", len, clen, vlen);

      if(clen > 0 && vlen > 0)
      {
    	  Ql_strncpy(cmd, cmdstr, clen);
    	  Ql_strncpy(val, ch, vlen);

    	  vlen = clear_all_nulls(val, vlen);
    	  if(vlen <= 0)
    		  return NULL;

    	  APP_DEBUG("<--set_cmd cmd=<%s>, val=<%s>-->\r\n", cmd, val);
    	  if(Ql_strcmp(cmd, "mode") == 0)
    	  {
    		  s32 mode = Ql_atoi(val);
    		  if(mode == 0)
    			  sett_in_ram->ipSettings.mode = 0;
    		  else if(mode == 1)
    			  sett_in_ram->ipSettings.mode = 1;
    		  else if(mode == 101)
    			  sett_in_ram->ipSettings.mode = 101;
    		  r = TRUE;
    	  }
    	  else if(Ql_strcmp(cmd, "apn") == 0)
    	  {
    		  if(vlen < MAX_GPRS_APN_LEN)
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


    	  //firmware ftp update
    	  else if(Ql_strcmp(cmd, "ftp user") == 0)
    	  {
    		  if(vlen < MAX_FTP_USER_NAME_LEN)
    		  {
    			  Ql_memset(sett_in_ram->ftpSettings.usrName, 0, MAX_FTP_USER_NAME_LEN);
    			  Ql_strncpy(sett_in_ram->ftpSettings.usrName, val, vlen);
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "ftp password") == 0)
    	  {
    		  if(vlen <= MAX_FTP_PASSWORD_LEN)
    		  {
    			  Ql_memset(sett_in_ram->ftpSettings.usrPassw, 0, MAX_FTP_PASSWORD_LEN);
    			  Ql_strncpy(sett_in_ram->ftpSettings.usrPassw, val, vlen);
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "ftp address") == 0)
    	  {
    		  if(vlen < MAX_FTP_ADDRESS_LEN)
    		  {
    			  Ql_memset(sett_in_ram->ftpSettings.srvAddress, 0, MAX_FTP_ADDRESS_LEN);
    			  Ql_strncpy(sett_in_ram->ftpSettings.srvAddress, val, vlen);
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "ftp port") == 0)
    	  {
    		  s32 port = Ql_atoi(val);
    		  if(port > 0){
    			  sett_in_ram->ftpSettings.srvPort = port;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "ftp filename") == 0)
    	  {
    		  if(vlen < MAX_FTP_FILENAME_LEN)
    		  {
    			  Ql_memset(sett_in_ram->ftpSettings.fileName, 0, MAX_FTP_FILENAME_LEN);
    			  Ql_strncpy(sett_in_ram->ftpSettings.fileName, val, vlen);
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "ftp filepath") == 0)
    	  {
    		  if(vlen < MAX_FTP_FILEPATH_LEN)
    		  {
    			  Ql_memset(sett_in_ram->ftpSettings.filePath, 0, MAX_FTP_FILEPATH_LEN);
    			  Ql_strncpy(sett_in_ram->ftpSettings.filePath, val, vlen);
    			  r = TRUE;
    		  }
    	  }
    	  ///////////////////

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
    		  if(value >= SB_ONE && value <= SB_ONE_DOT_FIVE){
    			  sett_in_ram->serPortSettings.stopBits = value;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "parity") == 0)
    	  {
    		  u32 value = Ql_atoi(val);
    		  if(value >= PB_NONE && value <= PB_MARK){
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
    		  if(timeout >= 120){ // 2 min
    			  sett_in_ram->secondsToReconnect = timeout;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "duration") == 0)
    	  {
    		  s32 value = Ql_atoi(val);
    		  if(value >= 30){//30 sec
    			  sett_in_ram->secondsOfDuration = value;
    			  r = TRUE;
    		  }
    	  }
    	  else if(Ql_strcmp(cmd, "toping") == 0 || Ql_strcmp(cmd, "periodsend") == 0)
    	  {
    		  s32 timeout = Ql_atoi(val);
    		  if(timeout >= 30){ // 0.5 min
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
    	  else if(Ql_strcmp(cmd, "authorization password") == 0)
    	  {
    		  if(vlen <= AUT_PASSWORD_LEN)
    		  {
    			  Ql_memset(sett_in_ram->securitySettings.cmdPassw, 0, AUT_PASSWORD_LEN);
    			  Ql_strncpy(sett_in_ram->securitySettings.cmdPassw, val, vlen);

        	      Ql_strcpy(tmp_buff, "\r\n");
        	      Ql_strcat(tmp_buff, "change authorization password, set commit to save.");
        	      Ql_strcat(tmp_buff, "\r\n");
        	      ret = tmp_buff;
        	      return ret;
    		  }
    	  }

    	  if(r == TRUE){
    		  *(--ch) = 0;
    		  ret = get_cmd(cmdstr, tmp_buff, sett_in_ram, programmData);
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
//**************************************************
char *get_cmd(char *cmd, char *tmp_buff, sProgrammSettings* sett_in_ram, sProgrammData *programmData)
{
  char *ret = NULL;
  char tbuff[50] = {0};

  int len = Ql_strlen(cmd);
  //Ql_Debug_Trace("get_cmd len=<%d>\r\n", len);
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

    //firmware ftp update
    else if(Ql_strcmp(cmd, "ftp user") == 0)
    {
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, sett_in_ram->ftpSettings.usrName);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "ftp password") == 0)
    {
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, sett_in_ram->ftpSettings.usrPassw);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "ftp address") == 0)
    {
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, sett_in_ram->ftpSettings.srvAddress);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "ftp port") == 0)
    {
	  Ql_sprintf(tbuff ,"%d", sett_in_ram->ftpSettings.srvPort);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "ftp filename") == 0)
    {
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, sett_in_ram->ftpSettings.fileName);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "ftp filepath") == 0)
    {
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, sett_in_ram->ftpSettings.filePath);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    ////////////////////////////////////////

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
    else if(Ql_strcmp(cmd, "duration") == 0)
    {
	  Ql_sprintf(tbuff ,"%d", sett_in_ram->secondsOfDuration);
      Ql_strcpy(tmp_buff, "\r\n");
      Ql_strcat(tmp_buff, cmd);
      Ql_strcat(tmp_buff, "=");
      Ql_strcat(tmp_buff, tbuff);
      Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "baudrate") == 0)
    {
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
    else if(Ql_strcmp(cmd, "toping") == 0 || Ql_strcmp(cmd, "periodsend") == 0)
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
    else if(Ql_strcmp(cmd, "input1 value") == 0)
    {
    	Ql_sprintf(tbuff ,"%d", programmData->dataState.in1);
    	Ql_strcpy(tmp_buff, "\r\n");
      	Ql_strcat(tmp_buff, cmd);
      	Ql_strcat(tmp_buff, "=");
      	Ql_strcat(tmp_buff, tbuff);
      	Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "input2 value") == 0)
    {
    	Ql_sprintf(tbuff ,"%d", programmData->dataState.in2);
    	Ql_strcpy(tmp_buff, "\r\n");
      	Ql_strcat(tmp_buff, cmd);
      	Ql_strcat(tmp_buff, "=");
      	Ql_strcat(tmp_buff, tbuff);
      	Ql_strcat(tmp_buff, "\r\n");
      ret = tmp_buff;
    }
    else if(Ql_strcmp(cmd, "termo value") == 0 || Ql_strcmp(cmd, "temp value") == 0)
    {
    	Ql_sprintf(tbuff ,"%f", programmData->dataState.temp);
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

/***********************************************************************
 * Pid analize
************************************************************************/
bool AnalizePidPacket(u8 *buffer, s32 len, sPidPacket *lastPacket)
{
	bool ret = FALSE;
	if(len > 4)
	{
		bShort tmp;
		tmp.Data_b[1] = buffer[2];
		tmp.Data_b[0] = buffer[3];
		if(tmp.Data_s == (len-4)){
			u8 pid = buffer[0];
			u8 typ = buffer[1];
			//if(typ == 0x02){//0x02-from server, 0x03-from device, 0x0B-init packet (???)
			lastPacket->pid 	= pid;
			lastPacket->type 	= typ;
			lastPacket->len		= tmp.Data_s;
			ret = TRUE;

			APP_DEBUG("<-- AnalizePidPacket: pid=%d, type=%d, len=%d -->\r\n", lastPacket->pid, lastPacket->type, lastPacket->len);

			//}
		}
	}
	return ret;
}

s32 AddPidHeader(u8 typ, u8* buffer, s32 len, sPidPacket *lastPacket)
{
	s32 ret = 0;
	if(lastPacket != NULL && len > 0){
		bShort tmp;
		tmp.Data_s = len;

		buffer[ret++] = (u8)lastPacket->pid;
		buffer[ret++] = typ;
		buffer[ret++] = tmp.Data_b[1];
		buffer[ret++] = tmp.Data_b[0];

		APP_DEBUG("<-- AddPidHeader: pid=%d, typ=%d, len=%d -->\r\n", lastPacket->pid, typ, tmp.Data_s);
	}
	return ret;
}



/***********************************************************************
 * SMS GLOBAL DATA DEFINITIONS
************************************************************************/
ConSMSStruct g_asConSMSBuf[CON_SMS_BUF_MAX_CNT];

/*****************************************************************************
 * FUNCTION
 *  Hdlr_RecvNewSMS
 *
 * DESCRIPTION
 *  The handler function of new received SMS.
 *
 * PARAMETERS
 *  <nIndex>     The SMS index in storage,it starts from 1
 *  <bAutoReply> TRUE: The module should reply a SMS to the sender;
 *               FALSE: The module only read this SMS.
 *
 * RETURNS
 *  VOID
 *
 * NOTE
 *  1. This is an internal function
 *****************************************************************************/
void Hdlr_RecvNewSMS(u32 nIndex, bool bAutoReply, sProgrammSettings *sett_in_ram, sProgrammData *programmData)
{
    s32 iResult = 0;
    u32 uMsgRef = 0;
    ST_RIL_SMS_TextInfo *pTextInfo = NULL;
    ST_RIL_SMS_DeliverParam *pDeliverTextInfo = NULL;
    char aPhNum[RIL_SMS_PHONE_NUMBER_MAX_LEN] = {0,};
    //const char aReplyCon[] = {"Module has received SMS."};
    bool bResult = FALSE;


    pTextInfo = Ql_MEM_Alloc(sizeof(ST_RIL_SMS_TextInfo));
    if (NULL == pTextInfo)
    {
        APP_DEBUG("%s/%d:Ql_MEM_Alloc FAIL! size:%u\r\n", sizeof(ST_RIL_SMS_TextInfo), __func__, __LINE__);
        return;
    }
    Ql_memset(pTextInfo, 0x00, sizeof(ST_RIL_SMS_TextInfo));
    iResult = RIL_SMS_ReadSMS_Text(nIndex, LIB_SMS_CHARSET_GSM, pTextInfo);
    if (iResult != RIL_AT_SUCCESS)
    {
        Ql_MEM_Free(pTextInfo);
        APP_DEBUG("Fail to read text SMS[%d], cause:%d\r\n", nIndex, iResult);
        return;
    }

    if ((LIB_SMS_PDU_TYPE_DELIVER != (pTextInfo->type)) || (RIL_SMS_STATUS_TYPE_INVALID == (pTextInfo->status)))
    {
        Ql_MEM_Free(pTextInfo);
        APP_DEBUG("WARNING: NOT a new received SMS.\r\n");
        return;
    }

    pDeliverTextInfo = &((pTextInfo->param).deliverParam);

    if(TRUE == pDeliverTextInfo->conPres)  //Receive CON-SMS segment
    {
        s8 iBufIdx = 0;
        u8 uSeg = 0;
        u16 uConLen = 0;

        iBufIdx = ConSMSBuf_GetIndex(g_asConSMSBuf,CON_SMS_BUF_MAX_CNT,&(pDeliverTextInfo->con));
        if(-1 == iBufIdx)
        {
            APP_DEBUG("Enter Hdlr_RecvNewSMS, WARNING! ConSMSBuf_GetIndex FAIL! Show this CON-SMS-SEG directly!\r\n");

            APP_DEBUG(
                "status:%u,type:%u,alpha:%u,sca:%s,oa:%s,scts:%s,data length:%u,cp:1,cy:%d,cr:%d,ct:%d,cs:%d\r\n",
                    (pTextInfo->status),
                    (pTextInfo->type),
                    (pDeliverTextInfo->alpha),
                    (pTextInfo->sca),
                    (pDeliverTextInfo->oa),
                    (pDeliverTextInfo->scts),
                    (pDeliverTextInfo->length),
                    pDeliverTextInfo->con.msgType,
                    pDeliverTextInfo->con.msgRef,
                    pDeliverTextInfo->con.msgTot,
                    pDeliverTextInfo->con.msgSeg
            );
            APP_DEBUG("data = %s\r\n",(pDeliverTextInfo->data));

            Ql_MEM_Free(pTextInfo);

            return;
        }

        bResult = ConSMSBuf_AddSeg(
                    g_asConSMSBuf,
                    CON_SMS_BUF_MAX_CNT,
                    iBufIdx,
                    &(pDeliverTextInfo->con),
                    (pDeliverTextInfo->data),
                    (pDeliverTextInfo->length)
        );
        if(FALSE == bResult)
        {
            APP_DEBUG("Enter Hdlr_RecvNewSMS,WARNING! ConSMSBuf_AddSeg FAIL! Show this CON-SMS-SEG directly!\r\n");

            APP_DEBUG(
                "status:%u,type:%u,alpha:%u,sca:%s,oa:%s,scts:%s,data length:%u,cp:1,cy:%d,cr:%d,ct:%d,cs:%d\r\n",
                (pTextInfo->status),
                (pTextInfo->type),
                (pDeliverTextInfo->alpha),
                (pTextInfo->sca),
                (pDeliverTextInfo->oa),
                (pDeliverTextInfo->scts),
                (pDeliverTextInfo->length),
                pDeliverTextInfo->con.msgType,
                pDeliverTextInfo->con.msgRef,
                pDeliverTextInfo->con.msgTot,
                pDeliverTextInfo->con.msgSeg
            );
            APP_DEBUG("data = %s\r\n",(pDeliverTextInfo->data));

            Ql_MEM_Free(pTextInfo);

            return;
        }

        bResult = ConSMSBuf_IsIntact(
                    g_asConSMSBuf,
                    CON_SMS_BUF_MAX_CNT,
                    iBufIdx,
                    &(pDeliverTextInfo->con)
        );
        if(FALSE == bResult)
        {
            APP_DEBUG(
                "Enter Hdlr_RecvNewSMS,WARNING! ConSMSBuf_IsIntact FAIL! Waiting. cp:1,cy:%d,cr:%d,ct:%d,cs:%d\r\n",
                pDeliverTextInfo->con.msgType,
                pDeliverTextInfo->con.msgRef,
                pDeliverTextInfo->con.msgTot,
                pDeliverTextInfo->con.msgSeg
            );

            Ql_MEM_Free(pTextInfo);

            return;
        }

        //Show the CON-SMS
        APP_DEBUG(
            "status:%u,type:%u,alpha:%u,sca:%s,oa:%s,scts:%s",
            (pTextInfo->status),
            (pTextInfo->type),
            (pDeliverTextInfo->alpha),
            (pTextInfo->sca),
            (pDeliverTextInfo->oa),
            (pDeliverTextInfo->scts)
        );

        uConLen = 0;
        for(uSeg = 1; uSeg <= pDeliverTextInfo->con.msgTot; uSeg++)
        {
            uConLen += g_asConSMSBuf[iBufIdx].asSeg[uSeg-1].uLen;
        }

        APP_DEBUG(",data length:%u",uConLen);
        APP_DEBUG("\r\n"); //Print CR LF

        for(uSeg = 1; uSeg <= pDeliverTextInfo->con.msgTot; uSeg++)
        {
            APP_DEBUG("data = %s ,len = %d",
                g_asConSMSBuf[iBufIdx].asSeg[uSeg-1].aData,
                g_asConSMSBuf[iBufIdx].asSeg[uSeg-1].uLen
            );
        }

        APP_DEBUG("\r\n"); //Print CR LF

        //Reset CON-SMS context
        bResult = ConSMSBuf_ResetCtx(g_asConSMSBuf,CON_SMS_BUF_MAX_CNT,iBufIdx);
        if(FALSE == bResult)
        {
            APP_DEBUG("Enter Hdlr_RecvNewSMS,WARNING! ConSMSBuf_ResetCtx FAIL! iBufIdx:%d\r\n",iBufIdx);
        }

        Ql_MEM_Free(pTextInfo);

        return;
    }

    APP_DEBUG("<-- RIL_SMS_ReadSMS_Text OK. eCharSet:LIB_SMS_CHARSET_GSM,nIndex:%u -->\r\n",nIndex);
    APP_DEBUG("SMS status:%u,type:%u,alpha:%u,sca:%s,oa:%s,scts:%s,data length:%u\r\n",
        pTextInfo->status,
        pTextInfo->type,
        pDeliverTextInfo->alpha,
        pTextInfo->sca,
        pDeliverTextInfo->oa,
        pDeliverTextInfo->scts,
        pDeliverTextInfo->length);
    APP_DEBUG("SMS data=<%s>\r\n",(pDeliverTextInfo->data));

    Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
    //Ql_MEM_Free(pTextInfo);

    if (bAutoReply)
    {
        if (!Ql_strstr(aPhNum, "10086"))  // Not reply SMS from operator
        {
        	char tmp_buff[150] = {0};
        	char *data = pDeliverTextInfo->data;
        	char *answer = Parse_Command(data, tmp_buff, sett_in_ram,  programmData);

        	if( answer != NULL && Ql_strlen(answer) > 0 )
        	{
				APP_DEBUG("<-- Replying SMS... -->\r\n");
				iResult = RIL_SMS_SendSMS_Text(aPhNum, Ql_strlen(aPhNum), LIB_SMS_CHARSET_GSM, (u8*)answer, Ql_strlen(answer),&uMsgRef);
				if (iResult != RIL_AT_SUCCESS)
				{
					APP_DEBUG("RIL_SMS_SendSMS_Text FAIL! iResult:%u\r\n",iResult);
					return;
				}
				APP_DEBUG("<-- RIL_SMS_SendTextSMS OK. uMsgRef:%d -->\r\n", uMsgRef);
        	}
        }
    }
    Ql_MEM_Free(pTextInfo);

    return;
}
//SMS
/*****************************************************************************
 * FUNCTION
 *  ConSMSBuf_IsIntact
 *
 * DESCRIPTION
 *  This function is used to check the CON-SMS is intact or not
 *
 * PARAMETERS
 *  <pCSBuf>     The SMS index in storage,it starts from 1
 *  <uCSMaxCnt>  TRUE: The module should reply a SMS to the sender; FALSE: The module only read this SMS.
 *  <uIdx>       Index of <pCSBuf> which will be stored
 *  <pCon>       The pointer of 'ST_RIL_SMS_Con' data
 *
 * RETURNS
 *  FALSE:   FAIL!
 *  TRUE: SUCCESS.
 *
 * NOTE
 *  1. This is an internal function
 *****************************************************************************/
bool ConSMSBuf_IsIntact(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,u8 uIdx,ST_RIL_SMS_Con *pCon)
{
    u8 uSeg = 1;

    if(    (NULL == pCSBuf)
        || (0 == uCSMaxCnt)
        || (uIdx >= uCSMaxCnt)
        || (NULL == pCon)
      )
    {
        APP_DEBUG("Enter ConSMSBuf_IsIntact,FAIL! Parameter is INVALID. pCSBuf:%x,uCSMaxCnt:%d,uIdx:%d,pCon:%x\r\n",pCSBuf,uCSMaxCnt,uIdx,pCon);
        return FALSE;
    }

    if((pCon->msgTot) > CON_SMS_MAX_SEG)
    {
        APP_DEBUG("Enter ConSMSBuf_GetIndex,FAIL! msgTot:%d is larger than limit:%d\r\n",pCon->msgTot,CON_SMS_MAX_SEG);
        return FALSE;
    }

	for (uSeg = 1; uSeg <= (pCon->msgTot); uSeg++)
	{
        if(FALSE == pCSBuf[uIdx].abSegValid[uSeg-1])
        {
            APP_DEBUG("Enter ConSMSBuf_IsIntact,FAIL! uSeg:%d has not received!\r\n",uSeg);
            return FALSE;
        }
	}

    return TRUE;
}
/*****************************************************************************
 * FUNCTION
 *  ConSMSBuf_AddSeg
 *
 * DESCRIPTION
 *  This function is used to add segment in <pCSBuf>
 *
 * PARAMETERS
 *  <pCSBuf>     The SMS index in storage,it starts from 1
 *  <uCSMaxCnt>  TRUE: The module should reply a SMS to the sender; FALSE: The module only read this SMS.
 *  <uIdx>       Index of <pCSBuf> which will be stored
 *  <pCon>       The pointer of 'ST_RIL_SMS_Con' data
 *  <pData>      The pointer of CON-SMS-SEG data
 *  <uLen>       The length of CON-SMS-SEG data
 *
 * RETURNS
 *  FALSE:   FAIL!
 *  TRUE: SUCCESS.
 *
 * NOTE
 *  1. This is an internal function
 *****************************************************************************/
bool ConSMSBuf_AddSeg(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,u8 uIdx,ST_RIL_SMS_Con *pCon,u8 *pData,u16 uLen)
{
    u8 uSeg = 1;

    if(    (NULL == pCSBuf) || (0 == uCSMaxCnt)
        || (uIdx >= uCSMaxCnt)
        || (NULL == pCon)
        || (NULL == pData)
        || (uLen > (CON_SMS_SEG_MAX_CHAR * 4))
      )
    {
        APP_DEBUG("Enter ConSMSBuf_AddSeg,FAIL! Parameter is INVALID. pCSBuf:%x,uCSMaxCnt:%d,uIdx:%d,pCon:%x,pData:%x,uLen:%d\r\n",pCSBuf,uCSMaxCnt,uIdx,pCon,pData,uLen);
        return FALSE;
    }

    if((pCon->msgTot) > CON_SMS_MAX_SEG)
    {
        APP_DEBUG("Enter ConSMSBuf_GetIndex,FAIL! msgTot:%d is larger than limit:%d\r\n",pCon->msgTot,CON_SMS_MAX_SEG);
        return FALSE;
    }

    uSeg = pCon->msgSeg;
    pCSBuf[uIdx].abSegValid[uSeg-1] = TRUE;
    Ql_memcpy(pCSBuf[uIdx].asSeg[uSeg-1].aData,pData,uLen);
    pCSBuf[uIdx].asSeg[uSeg-1].uLen = uLen;

	return TRUE;
}
/*****************************************************************************
 * FUNCTION
 *  ConSMSBuf_GetIndex
 *
 * DESCRIPTION
 *  This function is used to get available index in <pCSBuf>
 *
 * PARAMETERS
 *  <pCSBuf>     The SMS index in storage,it starts from 1
 *  <uCSMaxCnt>  TRUE: The module should reply a SMS to the sender; FALSE: The module only read this SMS.
 *  <pCon>       The pointer of 'ST_RIL_SMS_Con' data
 *
 * RETURNS
 *  -1:   FAIL! Can not get available index
 *  OTHER VALUES: SUCCESS.
 *
 * NOTE
 *  1. This is an internal function
 *****************************************************************************/
s8 ConSMSBuf_GetIndex(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,ST_RIL_SMS_Con *pCon)
{
	u8 uIdx = 0;

    if(    (NULL == pCSBuf) || (0 == uCSMaxCnt)
        || (NULL == pCon)
      )
    {
        APP_DEBUG("Enter ConSMSBuf_GetIndex,FAIL! Parameter is INVALID. pCSBuf:%x,uCSMaxCnt:%d,pCon:%x\r\n",pCSBuf,uCSMaxCnt,pCon);
        return -1;
    }

    if((pCon->msgTot) > CON_SMS_MAX_SEG)
    {
        APP_DEBUG("Enter ConSMSBuf_GetIndex,FAIL! msgTot:%d is larger than limit:%d\r\n",pCon->msgTot,CON_SMS_MAX_SEG);
        return -1;
    }

	for(uIdx = 0; uIdx < uCSMaxCnt; uIdx++)  //Match all exist records
	{
        if(    (pCon->msgRef == pCSBuf[uIdx].uMsgRef)
            && (pCon->msgTot == pCSBuf[uIdx].uMsgTot)
          )
        {
            return uIdx;
        }
	}

	for (uIdx = 0; uIdx < uCSMaxCnt; uIdx++)
	{
		if (0 == pCSBuf[uIdx].uMsgTot)  //Find the first unused record
		{
            pCSBuf[uIdx].uMsgTot = pCon->msgTot;
            pCSBuf[uIdx].uMsgRef = pCon->msgRef;

			return uIdx;
		}
	}

    APP_DEBUG("Enter ConSMSBuf_GetIndex,FAIL! No avail index in ConSMSBuf,uCSMaxCnt:%d\r\n",uCSMaxCnt);

	return -1;
}
/*****************************************************************************
 * FUNCTION
 *  ConSMSBuf_ResetCtx
 *
 * DESCRIPTION
 *  This function is used to reset ConSMSBuf context
 *
 * PARAMETERS
 *  <pCSBuf>     The SMS index in storage,it starts from 1
 *  <uCSMaxCnt>  TRUE: The module should reply a SMS to the sender; FALSE: The module only read this SMS.
 *  <uIdx>       Index of <pCSBuf> which will be stored
 *
 * RETURNS
 *  FALSE:   FAIL!
 *  TRUE: SUCCESS.
 *
 * NOTE
 *  1. This is an internal function
 *****************************************************************************/
bool ConSMSBuf_ResetCtx(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,u8 uIdx)
{
    if(    (NULL == pCSBuf) || (0 == uCSMaxCnt)
        || (uIdx >= uCSMaxCnt)
      )
    {
        APP_DEBUG("Enter ConSMSBuf_ResetCtx,FAIL! Parameter is INVALID. pCSBuf:%x,uCSMaxCnt:%d,uIdx:%d\r\n",pCSBuf,uCSMaxCnt,uIdx);
        return FALSE;
    }

    //Default reset
    Ql_memset(&pCSBuf[uIdx],0x00,sizeof(ConSMSStruct));

    //TODO: Add special reset here

    return TRUE;
}
/*****************************************************************************
 * FUNCTION
 *  SMS_Initialize
 *
 * DESCRIPTION
 *  Initialize SMS environment.
 *
 * PARAMETERS
 *  VOID
 *
 * RETURNS
 *  TRUE:  This function works SUCCESS.
 *  FALSE: This function works FAIL!
 *****************************************************************************/
bool SMS_Initialize(void)
{
    s32 iResult = 0;
    //u8  nCurrStorage = 0;
    //u32 nUsed = 0;
    //u32 nTotal = 0;

    // Set SMS storage:
    // By default, short message is stored into SIM card. You can change the storage to ME if needed, or
    // you can do it again to make sure the short message storage is SIM card.
    #if 0
    {
        iResult = RIL_SMS_SetStorage(RIL_SMS_STORAGE_TYPE_SM,&nUsed,&nTotal);
        if (RIL_ATRSP_SUCCESS != iResult)
        {
            APP_DEBUG("Fail to set SMS storage, cause:%d\r\n", iResult);
            return FALSE;
        }
        APP_DEBUG("<-- Set SMS storage to SM, nUsed:%u,nTotal:%u -->\r\n", nUsed, nTotal);

        iResult = RIL_SMS_GetStorage(&nCurrStorage, &nUsed ,&nTotal);
        if(RIL_ATRSP_SUCCESS != iResult)
        {
            APP_DEBUG("Fail to get SMS storage, cause:%d\r\n", iResult);
            return FALSE;
        }
        APP_DEBUG("<-- Check SMS storage: curMem=%d, used=%d, total=%d -->\r\n", nCurrStorage, nUsed, nTotal);
    }
    #endif

    // Enable new short message indication
    // By default, the auto-indication for new short message is enalbed. You can do it again to
    // make sure that the option is open.
    #if 0
    {
        iResult = Ql_RIL_SendATCmd("AT+CNMI=2,1",Ql_strlen("AT+CNMI=2,1"),NULL,NULL,0);
        if (RIL_AT_SUCCESS != iResult)
        {
            APP_DEBUG("Fail to send \"AT+CNMI=2,1\", cause:%d\r\n", iResult);
            return FALSE;
        }
        APP_DEBUG("<-- Enable new SMS indication -->\r\n");
    }
    #endif

    // Delete all existed short messages (if needed)
    iResult = RIL_SMS_DeleteSMS(0, RIL_SMS_DEL_ALL_MSG);
    if (iResult != RIL_AT_SUCCESS)
    {
        APP_DEBUG("Fail to delete all messages, iResult=%d,cause:%d\r\n", iResult, Ql_RIL_AT_GetErrCode());
        return FALSE;
    }
    APP_DEBUG("Delete all existed messages\r\n");

    return TRUE;
}
