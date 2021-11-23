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
#include "ql_gprs.h"


#define DEBUG_ENABLE 1

#if DEBUG_ENABLE > 0
#define DEBUG_PORT  UART_PORT1
#define DBG_BUF_LEN   512
static char DBG_BUFFER[DBG_BUF_LEN];

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
#else
#define APP_DEBUG(FORMAT,...)
#endif


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
#define FW_VERSION "1.2"

typedef enum{
    STATE_NW_GET_SIMSTATE,
    STATE_NW_QUERY_STATE,
    STATE_GPRS_REGISTER,
    STATE_GPRS_CONFIG,
    STATE_GPRS_ACTIVATE,
    STATE_GPRS_ACTIVATING,
    STATE_GPRS_GET_DNSADDRESS,
    STATE_GPRS_GET_LOCALIP,
    STATE_CHACK_SRVADDR,
    STATE_SOC_REGISTER,
    STATE_SOC_CREATE,
    STATE_SOC_CONNECT,
    STATE_SOC_CONNECTING,
    STATE_SOC_SEND,
    STATE_SOC_SENDING,
    STATE_SOC_ACK,
    STATE_SOC_CLOSE,
    STATE_GPRS_DEACTIVATE,
    STATE_TOTAL_NUM
}Enum_TCPSTATE;


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
	char				cmdPassw[AUT_PASSWORD_LEN];
}sSecuritySettings;

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

    u16            	serPortDataTimeout;
    u16            	gsmPortDataTimeout;
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

    //sBuffer     gsmRxBuffer;
    //sBuffer     serRxBuffer;

    //sBuffer     serToGsmRxBuffer;
    //sBuffer     gsmToSerRxBuffer;

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

    u32 			rssi;
    u32		 		ber;


}sProgrammData;


/***********************************************************************
 * SMS CONSTANT DEFINITIONS
************************************************************************/
#define CON_SMS_BUF_MAX_CNT   (1)
#define CON_SMS_SEG_MAX_CHAR  (160)
#define CON_SMS_SEG_MAX_BYTE  (4 * CON_SMS_SEG_MAX_CHAR)
#define CON_SMS_MAX_SEG       (7)

/***********************************************************************
 * SMS STRUCT TYPE DEFINITIONS
************************************************************************/
typedef struct
{
    u8 aData[CON_SMS_SEG_MAX_BYTE];
    u16 uLen;
} ConSMSSegStruct;

typedef struct
{
    u16 uMsgRef;
    u8 uMsgTot;

    ConSMSSegStruct asSeg[CON_SMS_MAX_SEG];
    bool abSegValid[CON_SMS_MAX_SEG];
} ConSMSStruct;



#endif /* TYPEDEF_H_ */
