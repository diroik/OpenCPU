/*
 * flash.c
 *
 *  Created on: 16 èþíÿ 2021 ã.
 *      Author: Àäìèí
 */

#include "flash.h"
#include "ql_flash.h"

#ifdef __PROJECT_SMART_BUTTON__
static sProgrammSettings firstInitSettings =
{
    .crc        = 0xAA,
    .tmp1       = 0xFF,
    .tmp2       = 0xFF,

    .gsmSettings.gprsApn = "internet",
    .gsmSettings.gprsUser = "",
    .gsmSettings.gprsPass = "",

    .securitySettings.cmdPassw = "12345678",

    .buttonTimeout  = 1000,
    .timerTimeout   = 60000,
    .rtcInterval	= 7200,
    .recvTimeout	= 30,

    .rtcNeedCorrect = FALSE

};
#else


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
    .counter1.ImpulseCnt = 0,
    .counter1.Koeff = 1000,

    .in2Timeout	   = 3,
    .counter2.ImpulseCnt = 0,
    .counter2.Koeff = 1000,

    .securitySettings.cmdPassw = "12345678",

    .ftpSettings.srvAddress = "94.228.255.152",
    .ftpSettings.srvPort = 8080,
    .ftpSettings.filePath = "/BC66/",
    .ftpSettings.fileName = "APPGS3MDM32.bin",
    .ftpSettings.usrName = "firmware",
    .ftpSettings.usrPassw = "123qwe45RTY"
};

#endif



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
	APP_DEBUG("<-- restore_default_flash ->\r\n");

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
  u8 index = 1;//0-2048b; 1-2048b
  u32 len = sizeof(sProgrammSettings);
  //APP_DEBUG("<--init_flash need len=%d index=%d, start Ql_Flash_Read-->\r\n", len, index);

  s32 r = Ql_Flash_Read(index, 0, (u8*)&tmp, len);

  //APP_DEBUG("<--init_flash read len (r)=%d need len (len)=%d-->\r\n", r, len);

  if(r == 0 )
  {
	  if( tmp.tmp1 != 0xAA )
	  {//first init
		  APP_DEBUG("<--first_init tmp1=%d-->\r\n", tmp.tmp1);
		  ret = restore_default_flash(sett_in_ram);
	  }
	  else
	  {//check crc
		  u16 crc = calc_settings_crc(&tmp);
		  APP_DEBUG("<--init_flash calc_settings_crc crc=%d tmp->crc=%d-->\r\n", crc, tmp.crc);
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

  u8 index = 1; //2048 byte
  u32 len = sizeof(sProgrammSettings);
  APP_DEBUG("<--start write_to_flash_settings len=%d-->\r\n", len);

  s32 r = Ql_Flash_Write(index, 0, (u8*)sett, len);

  APP_DEBUG("<--write_to_flash_settings r=%d len=%d-->\r\n", r, len);

  if(r == 0)
	  ret = TRUE;
  return ret;
}

