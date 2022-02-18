/*
 * typedef.h
 *
 *  Created on: 10 дек. 2021 г.
 *      Author: DROIK
 */

#ifndef PROJECT_QCX212_0H00_QUECTEL_PROJECT_QUEC_OPEN_INCLUDE_TYPEDEF_H_
#define PROJECT_QCX212_0H00_QUECTEL_PROJECT_QUEC_OPEN_INCLUDE_TYPEDEF_H_

#include "ql_type.h"
#include "ql_dbg.h"
#include "ql_error.h"
#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"
#include "ql_uart.h"

#define DBG_BUF_LEN   256
static char DBG_BUFFER[DBG_BUF_LEN];
#define DEBUG_PORT  UART_PORT0

#define APP_DEBUG(FORMAT,...) {\
    memset(DBG_BUFFER, 0, DBG_BUF_LEN);\
    snprintf(DBG_BUFFER,DBG_BUF_LEN,FORMAT,##__VA_ARGS__); \
    if (UART_PORT2 == (DEBUG_PORT)) \
    {\
        Ql_Debug_Trace((u8* )DBG_BUFFER);\
    } else {\
		Ql_UART_Write((Enum_SerialPort)(DEBUG_PORT), (u8*)(DBG_BUFFER), strlen((const char *)(DBG_BUFFER)));\
    }\
}

#define MAX_GPRS_USER_NAME_LEN 32
#define MAX_GPRS_PASSWORD_LEN  32
#define MAX_GPRS_APN_LEN       64

#define MAX_ADDRESS_LEN 64
#define MAX_KOEFF_LEN 2
#define AUT_PASSWORD_LEN 16

#define AUT_TIMEOUT 300

#define RESISTOR 2000.0

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
    char                srvAddress[32];
    char				filePath[16];
    char                fileName[16];

    unsigned int        srvPort;

    char				usrName[16];
    char				usrPassw[16];
}sFtpSettings;

typedef struct{
	char				cmdPassw[16];
}sSecuritySettings;

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



#ifdef __PROJECT_SMART_BUTTON__

#define FW_VERSION "1.0"

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

#define FW_VERSION "1.1"

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

    u16            	serPortDataTimeout;//period
    u16            	gsmPortDataTimeout;//duration
    //u16            	smsRecvTimeout;

    u8				buttonTimeout;//in sec
    u8				in1Timeout;
    u8				in2Timeout;

    sSecuritySettings 	securitySettings;
    sFtpSettings		ftpSettings;
}sProgrammSettings;

typedef struct{
    bool        	needReboot;
    bool 			firstInit;
    bool			initFlash;

    s32				buttonCnt;//in 100 ms
    s32 		   	in1Cnt;
    s32				in2Cnt;

    bool 			buttonState;
    bool 			HbuttonState;
    bool 			in1State;
    bool 			Hin1State;
    bool 			in2State;
    bool 			Hin2State;
    float 			tempValue;

    u32            	rebootCnt;
    u32            	reconnectCnt;
    u32            	pingCnt;
    u32 			autCnt;

    u32				totalSeconds;

    //s64				buttonTimerTimeout;//in 10 ms

}sProgrammData;

#endif

#endif /* TYPEDEF_H_ */
