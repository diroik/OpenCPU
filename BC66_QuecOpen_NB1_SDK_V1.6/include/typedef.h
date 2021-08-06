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

#define FW_VERSION "1.0"

#define MAX_GPRS_USER_NAME_LEN 32
#define MAX_GPRS_PASSWORD_LEN  32
#define MAX_GPRS_APN_LEN       64

#define MAX_ADDRESS_LEN 64
#define MAX_KOEFF_LEN 2

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
    u16    crc;
    u8     tmp1;
    u8     tmp2; //tmp for aligned to 4 bytes

    sAdcSettings		adcSettings;
    sGsmSettings        gsmSettings;
    sIpSettings         ipSettings;
    ST_UARTDCB       	serPortSettings;

    u32            	secondsToReboot;
    u32            	secondsToReconnect;

    u16            	serPortDataTimeout;
    u16            	gsmPortDataTimeout;
    //u16            	smsRecvTimeout;

    u8				buttonTimeout;//in sec
    u8				in1Timeout;
    u8				in2Timeout;

    //u16 			koeff;
}sProgrammSettings;


typedef struct{
    bool        	needReboot;
    bool 			firstInit;
    bool			initFlash;

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

    u32            	rebootCnt;
    u32            	reconnectCnt;

    u64				totalMS;

}sProgrammData;



#endif /* TYPEDEF_H_ */
