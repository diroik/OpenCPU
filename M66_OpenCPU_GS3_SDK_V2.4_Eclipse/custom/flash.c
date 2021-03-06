/*
 * flash.c
 *
 *  Created on: 16 ???? 2021 ?.
 *      Author: ?????
 */

#include "flash.h"

static sProgrammSettings firstInitSettings =
{
    .crc        = 0xAA,
    .tmp1       = 0xFF,
    .tryConnectCnt = 5,

    .adcSettings.samplingCount = 5,
    .adcSettings.samplingInterval = 200,

    .gsmSettings.gprsApn = "internet",
    .gsmSettings.gprsUser = "",
    .gsmSettings.gprsPass = "",
    //.gsmSettings.gprsDialNumber = "*99***1#",

    .ipSettings.mode = 1,//0-tcp server, 1-tcp client, 101-tcp client+packetId. When secondsToReconnect!=secondsOfDuration=> periodic connect
    .ipSettings.dstAddress = "server.techmonitor.ru",//"asdu.megafon.ru",
    .ipSettings.dstPort = 9003,

    .ipSettings.srcAddress = "127.0.0.1",
    .ipSettings.srcPort = 5000,

    .serPortSettings.baudrate           = 9600,
    .serPortSettings.stopBits           = SB_ONE,//
    .serPortSettings.parity             = PB_NONE,//0-none 1-odd 2-even
    .serPortSettings.dataBits         	= DB_8BIT,
    .serPortSettings.flowCtrl			= FC_NONE,

    .secondsToReboot 	= 604800,//86400s=1d 604800s=1week
    .secondsToReconnect = 3600,//3600s=1h 5400s=1.5h 21600s=6h
    .secondsToPing		= 60, // 300s = 5min

    //.serPortDataTimeout = 500,//
    //.gsmPortDataTimeout = 500,//
    .secondsOfDuration = 300,

    .buttonTimeout = 30,
    .in1Timeout	   = 3,
    .in2Timeout	   = 3,

    .securitySettings.cmdPassw = "12345678",

    .ftpSettings.srvAddress = "94.228.255.152",
    .ftpSettings.srvPort = 21,
    .ftpSettings.filePath = "/M66/",
    .ftpSettings.fileName = "GSM-M_v1_36.bin",
    .ftpSettings.usrName = "firmware",
    .ftpSettings.usrPassw = "123qwe45RTY"
};


u16 calc_settings_crc(sProgrammSettings *sett)
{
  u16 crc = 0;
  u8 *tmp = (u8 *)sett + 4;
  int len = sizeof(*sett) - 4;
  if(len > 0)
  {
    for(int i=0; i<len; i++)
      crc += tmp[i];
  }
  return crc;
}

bool restore_default_flash(sProgrammSettings *sett_in_ram)
{
	bool ret = FALSE;
	Ql_Debug_Trace("<-- restore_default_flash ->\r\n");

	*sett_in_ram = firstInitSettings;//*tmp;
	sett_in_ram->tmp1 = 0xAA;
	//sett_in_ram->tmp2 = 0xAA;
	//sett_in_ram->crc = calc_settings_crc(sett_in_ram); //in write_to_flash_settings

	ret = write_to_flash_settings(sett_in_ram);

	return ret;
}

bool init_flash(sProgrammSettings *sett_in_ram)
{
  bool ret = FALSE;
  //sConnectionSettings temp;
  sProgrammSettings tmp;
  u8 index = 13;//13 - 500byte
  u32 len = sizeof(sProgrammSettings);
  Ql_Debug_Trace("<--init_flash need len=%d index=%d, start Ql_SecureData_Read-->\r\n", len, index);

  s32 r = Ql_SecureData_Read(index, (u8*)&tmp, len);

  Ql_Debug_Trace("<--init_flash read len (r)=%d need len (len)=%d-->\r\n", r, len);

  if(r >= 0 )
  {
	  if(r == 0 || r != len || tmp.tmp1 != 0xAA)
	  {//first init
		  Ql_Debug_Trace("<--first_init tmp1=%d-->\r\n", tmp.tmp1);

		  ret = restore_default_flash(sett_in_ram);
	  }
	  else
	  {//check crc
		  u16 crc = calc_settings_crc((sProgrammSettings *)&tmp);
		  Ql_Debug_Trace("<--init_flash calc_settings_crc crc=%d tmp->crc=%d-->\r\n", crc, tmp.crc);
		  if(tmp.crc == crc)
		  {
			  *sett_in_ram = tmp;
			  ret = TRUE;
		  }
		  else
		  {
			  ret = FALSE;
		  }
	  }
  }
  return ret;
}

bool write_to_flash_settings(sProgrammSettings *sett)
{
  bool                          ret = FALSE;//TRUE;

  sett->crc = calc_settings_crc(sett);

  u8 index = 13; //500byte
  u32 len = sizeof(sProgrammSettings);
  Ql_Debug_Trace("<--start write_to_flash_settings len=%d-->\r\n", len);

  s32 r = Ql_SecureData_Store(index, (u8*)sett, len);

  Ql_Debug_Trace("<--write_to_flash_settings r=%d len=%d-->\r\n", r, len);

  if(r == 0)
	  ret = TRUE;
  return ret;
}

