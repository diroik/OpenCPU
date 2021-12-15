/*
 * typedef.h
 *
 *  Created on: 11 èþíÿ 2021 ã.
 *      Author: Àäìèí
 */

#ifndef TYPEDEF_H_
#define TYPEDEF_H_

#include "ql_stdlib.h"
#include "ql_trace.h"
#include "ql_error.h"
#include "ql_uart.h"

#define DBG_BUF_LEN   512
static char DBG_BUFFER[DBG_BUF_LEN];
#define DEBUG_PORT  UART_PORT0

#define APP_DEBUG(FORMAT,...) {\
    Ql_memset(DBG_BUFFER, 0, DBG_BUF_LEN);\
    Ql_sprintf(DBG_BUFFER,FORMAT,##__VA_ARGS__); \
    if (UART_PORT2 == (DEBUG_PORT)) \
    {\
        Ql_Debug_Trace(DBG_BUFFER);\
    } else {\
        Ql_UART_Write((Enum_SerialPort)(DEBUG_PORT), (u8*)(DBG_BUFFER), Ql_strlen((const char *)(DBG_BUFFER)));\
    }\
}

#define MAX_GPRS_USER_NAME_LEN 32
#define MAX_GPRS_PASSWORD_LEN  32
#define MAX_GPRS_APN_LEN       64

#define MAX_ADDRESS_LEN 64
#define MAX_KOEFF_LEN 2
#define AUT_PASSWORD_LEN 16

#define MAX_FTP_ADDRESS_LEN 32
#define MAX_FTP_USER_NAME_LEN 16
#define MAX_FTP_PASSWORD_LEN  16
#define MAX_FTP_FILENAME_LEN  16
#define MAX_FTP_FILEPATH_LEN  16

#define RESISTOR 2000.0
#define AUT_TIMEOUT 300

typedef enum{
    TCP_STATE_NW_GET_SIMSTATE,
    TCP_STATE_NW_QUERY_STATE,
    TCP_STATE_GET_DNSADDRESS,
    TCP_STATE_GET_LOCALIP,
    TCP_STATE_CHACK_SRVADDR,
    TCP_STATE_SOC_REGISTER,
    TCP_STATE_SOC_CREATE,
    TCP_STATE_SOC_CONNECT,
    TCP_STATE_SOC_CONNECTING,
    TCP_STATE_SOC_SEND,
    TCP_STATE_SOC_SENDING,
    TCP_STATE_SOC_ACK,
    TCP_STATE_SOC_CLOSE,
    TCP_STATE_TOTAL_NUM
}Enum_TCPSTATE;

typedef enum{
	NIDD_STATE_WAIT = 0,
	NIDD_CHECK_NET_REG,
	NIDD_STATE_ACT_PDN,
	NIDD_STATE_CREATE_ACCOUNT,
	NIDD_STATE_CONNECT,
	NIDD_STATE_SEND,
	NIDD_STATE_RECV,
	NIDD_STATE_CLOSE,
	NIDD_STATE_CLOSE_PDN,
	NIDD_STATE_FREE

}Enum_NIDDSTATE;

typedef union
{
    u16 Data_s;
    u8  Data_b[2];
} bShort;

typedef union
{
    u32 Data_w;
    u8  Data_b[4];
} bWord;

typedef struct{
  char  buff[512];
  int   index;
  int   maxsize;
  bool  busy;
  int   len;
} sBuffer;

typedef struct{
  char  buff[64];
  int   index;
  int   maxsize;
} sTinyBuffer;

typedef struct{
	u16 samplingCount;
	u16 samplingInterval;

}sAdcSettings;


typedef struct{
    char gprsApn[MAX_GPRS_APN_LEN];
    char gprsUser[MAX_GPRS_USER_NAME_LEN];
    char gprsPass[MAX_GPRS_PASSWORD_LEN];
    //char gprsDialNumber[32];
}sGsmSettings;

typedef struct{
    unsigned char       mode;//client|server

    char                dstAddress[MAX_ADDRESS_LEN];
    unsigned int        dstPort;

    char                srcAddress[MAX_ADDRESS_LEN];
    unsigned int        srcPort;
}sIpSettings;

typedef struct{
    char                srvAddress[MAX_FTP_ADDRESS_LEN];
    char				filePath[MAX_FTP_FILEPATH_LEN];
    char                fileName[MAX_FTP_FILENAME_LEN];
    unsigned int        srvPort;
    char				usrName[MAX_FTP_USER_NAME_LEN];
    char				usrPassw[MAX_FTP_PASSWORD_LEN];
}sFtpSettings;

typedef struct{
	char				cmdPassw[16];
}sSecuritySettings;


#ifdef __PROJECT_SMART_BUTTON__

#define FW_VERSION "1.0"

typedef struct{
	u32 pid;
	bool state;
	bool confirm;
	u16  rssi;
	u16 ber;
	u16 voltage;
	u16 capacity;

	char iccid[21];

}sDataJsonParams;

typedef struct{
    bool        	needReboot;
    bool 			firstInit;
    bool			initFlash;
    bool 			timerStart;
    bool 			sleepEnable;

    s64				timerTimeout;//in ms, mast be signed!!!

    s64				buttonMaxCnt;
    s32				buttonCnt;//in ms
    bool 			buttonState;
    bool 			HbuttonState;

    s32				ledBlinkCnt;
    s32				ledBlinkCnt2;

    bool 			needSendNidd;
    u32				totalSeconds;

    s32 			cregStatus;
    bool			cregInit;


}sProgrammData;

typedef struct{
    u16    crc;
    u8     tmp1;
    u8     tmp2; //tmp for aligned to 4 bytes

    sGsmSettings        gsmSettings;
    sSecuritySettings 	securitySettings;

    u32					buttonTimeout;//in 100 msec
    u64					timerTimeout;
    u64					rtcInterval;//in sec

    u32					recvTimeout;//sec
    bool				rtcNeedCorrect;

}sProgrammSettings;

#else

#define FW_VERSION "1.31"

typedef struct{
	u32 pid;
	bool state;
	bool confirm;
    char            imei[30];
    char 			iccid[30];
    u32				totalSeconds;
    s32 			timezone;
    bool 			button;
    bool 			in1;
    bool 			in2;
    float 			temp;
	u16 			voltage;
	u16 			capacity;
    s32 			rssi;
    s32		 		ber;
    char 			version[10];
}sDataJsonParams;

typedef struct{
	u16 pid;
	u16 type;
	u16 len;
	u32 timeStamp;
}sPidPacket;

typedef struct{
    u16    crc;
    u8     tmp1;
    u8     tmp2; //tmp for aligned to 4 bytes

    sAdcSettings		adcSettings;
    sGsmSettings        gsmSettings;
    sIpSettings         ipSettings;
    ST_UARTDCB       	serPortSettings;

    u32            	secondsToReboot;
    u32            	secondsToReconnect;
    u32				secondsToPing;
    //u16            	serPortDataTimeout;//period
    //u16            	gsmPortDataTimeout;//duration
    u32				secondsOfDuration;
    //u16            	smsRecvTimeout;
    u8				buttonTimeout;//in sec
    u8				in1Timeout;
    u8				in2Timeout;
    sSecuritySettings 	securitySettings;
    sFtpSettings		ftpSettings;
}sProgrammSettings;

typedef struct{
	s32				mainTaskId;
	s32				subTaskId1;
    bool        	needReboot;
    bool 			firstInit;
    bool			timeInit;
    bool			socketTimersInit;
    //sBuffer     gsmRxBuffer;
    //sBuffer     serRxBuffer;

    //sBuffer     serToGsmRxBuffer;
    //sBuffer     gsmToSerRxBuffer;

    s32				buttonCnt;//in 100 ms
    s32 		   	in1Cnt;
    s32				in2Cnt;
    bool 			HbuttonState;
    bool 			Hin1State;
    bool 			Hin2State;
    u32            	rebootCnt;
    u32            	reconnectCnt;
    u32            	durationCnt;
    u32            	pingCnt;
    u32 			autCnt;
    sDataJsonParams dataState;
    sPidPacket      lastPacket;

    //s64				buttonTimerTimeout;//in 10 ms

}sProgrammData;

#endif



#endif /* TYPEDEF_H_ */
