/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Quectel Co., Ltd. 2020
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   example_ssl.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   This example demonstrates how to establish a SSL connection, when the module is used for the client.
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "C_PREDEF=-D __EXAMPLE_SSL" in gcc_makefile file. 
 *	 And compile the app using "make clean/new".
 *     Download image bin to module to run.
 * 
 * -------
 * -------
 *
 *============================================================================
 *             HISTORY
 *----------------------------------------------------------------------------
 * 
 ****************************************************************************/
#ifdef __EXAMPLE_SSL__

#include "custom_feature_def.h"
#include "ql_stdlib.h"
#include "ql_common.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_error.h"
#include "ql_uart.h"
#include "ql_timer.h"
#include "ril.h"
#include "ril_network.h"
#include "ril_ssl.h"
#include "ql_urc_register.h"
#include "ql_network.h"
#include "ql_socket.h"

#define DEBUG_ENABLE 1
#if DEBUG_ENABLE > 0
#define DEBUG_PORT  UART_PORT0
#define DBG_BUF_LEN   1500
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

/*****************************************************************
* define process state
******************************************************************/
typedef enum{
    STATE_NW_QUERY_STATE,
	STATE_NW_GET_LOCALIP,
	STATE_SSL_CFG,
    STATE_SSL_OPEN,
    STATE_SSL_SEND,
    STATE_SSL_CLOSE,
    STATE_SSL_TOTAL_NUM
}Enum_SSL_STATE;
static Enum_SSL_STATE m_ssl_state = STATE_NW_QUERY_STATE;

/*****************************************************************
* UART Param
******************************************************************/
static Enum_SerialPort m_myUartPort  = UART_PORT0;

#define SERIAL_RX_BUFFER_LEN  2048
static u8 m_RxBuf_Uart[SERIAL_RX_BUFFER_LEN];

/*****************************************************************
* timer param
******************************************************************/
#define SSL_TIMER_ID          TIMER_ID_USER_START
#define SSL_TIMER_PERIOD      1000

/*****************************************************************
* ssl param
******************************************************************/

#define QSSL_SEND_DATA_MAX_LEN     (1460) //Maximum length of  total topic information.
#define QSSL_DATAMODE_MAX_LEN      (3*1024) // SSL datamode max length
#define QSSL_HOSTNAME_MAX_LEN      (150) //hostname length.

static u8 m_send_buf[QSSL_SEND_DATA_MAX_LEN] = "123400120d0a551a\0";
static u8 hostname[QSSL_HOSTNAME_MAX_LEN] = "220.180.239.212\0";
static u32 svr_port = 8020;

static u8 cacert[QSSL_DATAMODE_MAX_LEN] = "-----BEGIN CERTIFICATE-----\r\n\
MIID4zCCAsugAwIBAgIJAMfLlxXplSxsMA0GCSqGSIb3DQEBBQUAMIGHMQswCQYD\r\n\
VQQGEwJDTjELMAkGA1UECAwCQUgxCzAJBgNVBAcMAkhGMRAwDgYDVQQKDAdRdWVj\r\n\
dGVsMQswCQYDVQQLDAJTVDEYMBYGA1UEAwwPMjIwLjE4MC4yMzkuMjEyMSUwIwYJ\r\n\
KoZIhvcNAQkBFhZkYWlzeS50aWFuQHF1ZWN0ZWwuY29tMB4XDTIwMDEwMjAyNDkz\r\n\
MVoXDTI5MTIzMDAyNDkzMVowgYcxCzAJBgNVBAYTAkNOMQswCQYDVQQIDAJBSDEL\r\n\
MAkGA1UEBwwCSEYxEDAOBgNVBAoMB1F1ZWN0ZWwxCzAJBgNVBAsMAlNUMRgwFgYD\r\n\
VQQDDA8yMjAuMTgwLjIzOS4yMTIxJTAjBgkqhkiG9w0BCQEWFmRhaXN5LnRpYW5A\r\n\
cXVlY3RlbC5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDfU9Jo\r\n\
SWO1vnrAqpL0pUcHlA6sbvOw7lewLy/A/j9w6IsgJkQtIgZHswkuTPzlzIz1/eeZ\r\n\
8+7h6oyZn9lwC/oFCQXOjZ9PnEc3+SYT6MN+Q8JWHaTk+X5n55aXPwrfRxbvcG6X\r\n\
sm8oDbsdJTmztbP/DsMEeGaWHTcMAiJcF9rQLdIdtlph7LHbbNKI/JyKjEoy35nn\r\n\
Ijh+PoGAEejevKM4hh/+zh3u1o817ayhYOIk1DBhDN4roxJs965Lc1UuQrW25ONv\r\n\
/GM4AYkzLdXv4LitE9jBizt8gfR4ubKGLnwpUXm28Y2NCDohE/eVZMakGOgB1W0y\r\n\
nAlninESRZLfD9PTAgMBAAGjUDBOMB0GA1UdDgQWBBSBDgjfJLVDxs4vQkczlxMn\r\n\
UC5inTAfBgNVHSMEGDAWgBSBDgjfJLVDxs4vQkczlxMnUC5inTAMBgNVHRMEBTAD\r\n\
AQH/MA0GCSqGSIb3DQEBBQUAA4IBAQCM2scsN6MIKe2CI1dPzVL/OskGmbGJcar9\r\n\
a/exbkzQrwShjki0hgkIURl4j0QnH1O2g/XYYyEmtZEjvMZ9TyhrraHCu5lVDl05\r\n\
kI2wncnjxT0nVYgOGOZIYLFttn8CL3PDwYrAEzeknhU7fn93on37eMdNUdLlGLUD\r\n\
9lwTJySZYN++5ea6gvQndze1ceVi1VP7YOuDhxeHz+pzgnTS8KK4c3wlFXjgepnI\r\n\
emOcmixhn/57Uljt2aUxrdeqT1HbqoWaoVtrV1Xt73/QRpBCgxWBLZge10idc/MV\r\n\
vCJuhjccXJZjEnS7xqx1JFZjqoedPIfq/w1st5SGPQlZi5h5dgxn\r\n\
-----END CERTIFICATE-----\r\n\0";


static u8 clientcert[QSSL_DATAMODE_MAX_LEN] = "-----BEGIN CERTIFICATE-----\r\n\
MIID+DCCAuCgAwIBAgIBATANBgkqhkiG9w0BAQUFADCBhzELMAkGA1UEBhMCQ04x\r\n\
CzAJBgNVBAgMAkFIMQswCQYDVQQHDAJIRjEQMA4GA1UECgwHUXVlY3RlbDELMAkG\r\n\
A1UECwwCU1QxGDAWBgNVBAMMDzIyMC4xODAuMjM5LjIxMjElMCMGCSqGSIb3DQEJ\r\n\
ARYWZGFpc3kudGlhbkBxdWVjdGVsLmNvbTAeFw0yMDAxMDIwMjU5NTRaFw0yMTAx\r\n\
MDEwMjU5NTRaMHoxCzAJBgNVBAYTAkNOMQswCQYDVQQIDAJBSDEQMA4GA1UECgwH\r\n\
UXVlY3RlbDELMAkGA1UECwwCU1QxGDAWBgNVBAMMDzIyMC4xODAuMjM5LjIxMjEl\r\n\
MCMGCSqGSIb3DQEJARYWZGFpc3kudGlhbkBxdWVjdGVsLmNvbTCCASIwDQYJKoZI\r\n\
hvcNAQEBBQADggEPADCCAQoCggEBAJx4KEU6HSJb+Fj+6WdDF6mT82dAOtcAqaF9\r\n\
RO/Q7/gbLMraYQejGByLxgQBUKTxugr4Yt8AIzblnhwXGY3IA0KoFlNCzJm+wouZ\r\n\
JE2ZwoZXYEnWlMgPjB8I3AVWfSYeYnaI7y/nEcOs6NBSWui8+m1MXNtFBNGi/6zz\r\n\
N6vciocsFY5dgWWI2AYKw1fui2AuVME/NrqtaEfNdQA1DPj05s5wH6NcemoYyJJq\r\n\
sWqJAsu+fmily8WhL5IfvPlDJ5x3frtPdh4OxcLlsgt4dxkvR4Y+rXCP75JHZKIA\r\n\
YgkK/873+yff2iHRT2Mn0LNCHI7RxCOsjsuN7GfpQdV01xNf6WMCAwEAAaN7MHkw\r\n\
CQYDVR0TBAIwADAsBglghkgBhvhCAQ0EHxYdT3BlblNTTCBHZW5lcmF0ZWQgQ2Vy\r\n\
dGlmaWNhdGUwHQYDVR0OBBYEFKd3P5PUZ/yj4UGjhTIdQKgcG2FBMB8GA1UdIwQY\r\n\
MBaAFIEOCN8ktUPGzi9CRzOXEydQLmKdMA0GCSqGSIb3DQEBBQUAA4IBAQBQb8QK\r\n\
rxByZH0xyBH4D7XMPcQEUn3v68ag2jdCbBvmIw1AuknNlU20FgEMMfxemdVHP293\r\n\
dpztcukpqtjuWnrF50NmRcQ2dS+HhUJ7ziP1c6mQTiaC1i2IORLLH1FJLhuZB4pS\r\n\
twkYLRhK3fjEVqIisw9kf0zyxsy2ZFr9iTfhdCVyVWKqS+IatXW++afjP+x/X+dj\r\n\
o28qMJKGskfDkOs0AuTPtQVM5mzW2YE7frFmYkm9SErHrfWeizO/2LAJfsT4sLCC\r\n\
Mc1Ktgix2xzgcWyL7wEDtgRV8f0E6hSf5CLbJQ3+tylndh+80LkscHD3kWem4RNF\r\n\
wkD2UTo3SiyU0alB\r\n\
-----END CERTIFICATE-----\r\n\0";

static u8 clientkey[QSSL_DATAMODE_MAX_LEN] = "-----BEGIN PRIVATE KEY-----\r\n\
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCceChFOh0iW/hY\r\n\
/ulnQxepk/NnQDrXAKmhfUTv0O/4GyzK2mEHoxgci8YEAVCk8boK+GLfACM25Z4c\r\n\
FxmNyANCqBZTQsyZvsKLmSRNmcKGV2BJ1pTID4wfCNwFVn0mHmJ2iO8v5xHDrOjQ\r\n\
UlrovPptTFzbRQTRov+s8zer3IqHLBWOXYFliNgGCsNX7otgLlTBPza6rWhHzXUA\r\n\
NQz49ObOcB+jXHpqGMiSarFqiQLLvn5opcvFoS+SH7z5Qyecd367T3YeDsXC5bIL\r\n\
eHcZL0eGPq1wj++SR2SiAGIJCv/O9/sn39oh0U9jJ9CzQhyO0cQjrI7Ljexn6UHV\r\n\
dNcTX+ljAgMBAAECggEAIW87i69jUlg8tSejUEFx6PaIaGO9LvwhQzS1allHLckA\r\n\
xUfsu+kJ2e+0HF77kINmsblpxyUMYUqngnEdMMKwj6nDO39moOO2RoYuz7Yvbu10\r\n\
0dJccHOCCY+nFCzrCR4tBA82rRG/JgToqrsuWdUn3fsXTHNvGCywXZCLMNorb76S\r\n\
OKxS5KChdaJ4bJqpmuJ8I1qPNTTjdnlqxiYWFr0NXXbMqAQU0K18H8FumC2UlZcW\r\n\
Vht8PeYjAT6xZwKh59kBzDTJIj8FLOzQC8K+VWg6+pGf4NJjAbG++MM+s5GX5Bo4\r\n\
yQb4HK08EpxfeXOWFwvZ3NTkUTpU0c2s7QWX9GCqAQKBgQDOr3nC6O8Lil+ePvfI\r\n\
R2UmjNPnq4EO9vXjrzI8Gf9artijdWJ19OyvUZHYT6H18laR/GalMgeE8XAZTCOW\r\n\
Y0LQR3C2TK7E8OaKIpLPsRhwuhS8VuKei2iOckAbKwrjZFdR/OF2JgKT8pPeCClV\r\n\
QqFTqZ30hXxBtTYS+JYUNkaGowKBgQDBzXCodamzgc/jO0ZZ/gWGIwo6qZrlpgNY\r\n\
Sgx2skwfMuSXCXmsVPTgfOzEdIv/2UQFoMm/OtweK/PQUpkBkQuOjJx57lQYSx+a\r\n\
s2TZDbyZeFrjhywc9md9TCeuW0DCvOyeSVgEa3CvAwh3WfLWKpc7c2z7af8hJzXy\r\n\
x8PVnrb+QQKBgEev71CeUEM1dGLDvleWA8xyLhF9l97j43dcTdUwwzPlzzgqv8Lf\r\n\
97P7W9WmESvoKQSrgcKsbpU61MHBedpwhT0OelaQlFG0qJGi2j71Ut/OeuC0Vhfd\r\n\
jJ9Tm06RCE7Ef9DtIBpFpsE/8u+g7w0mdiQ5gZLNNcLool/EHvHVvXJ3AoGAchMr\r\n\
LP2VVkQiye4qCu6q8bnOW5lZw7NkZKOxkmyAwhyC0SSSqg2X1kuyUjEH9yE8GNP7\r\n\
7MQIrPnSGd2Ekpd95fp+YIcYmPQfNkBCLEEZsnxVg3gdTy7625XoBYlRJU9vN/yU\r\n\
AoX7Xgi7AwW/GrEXM7dNfu91bV5XMwDDFQrHTAECgYEAlV/i2tdbyqDebhqviHSG\r\n\
/cihE/BVwEnVO/F2rkJi5Cqv6svBvoTt3gCQgdLDGm9yBBfmejJhtOCMDJO/C7gy\r\n\
QopfgXTC2KJDCKVVYhsjbMM7PHXG14W084YOU4EhgIt3KSz5Emo6X5WYADM3GAYW\r\n\
7+Ma2qn0hds8HSB19gD86Uc=\r\n\
-----END PRIVATE KEY-----\r\n\0";


/*****************************************************************
* ssl recv callback function
******************************************************************/
static void callback_ssl_recv(u8* buffer,u32 length);

/*****************************************************************
* uart callback function
******************************************************************/
static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara);

/*****************************************************************
* timer callback function
******************************************************************/
static void Callback_Timer(u32 timerId, void* param);


SSL_Urc_Param_t* ssl_urc_param_ptr = NULL;
ST_SSL_Param_t ssl_param_t = {0,0,NULL,0,0,0,0,0,0,0,0,0};
ST_SSL_Datamode_Userdata_t userdata = {NULL,0};

static void callback_ssl_recv(u8* buffer,u32 length)
{
	APP_DEBUG("<--ssl receive data: %s,len = %d-->\r\n",buffer,length);
	m_ssl_state = STATE_SSL_CLOSE;
}

void proc_main_task(s32 taskId)
{
    ST_MSG msg;
	s32 ret;

    // Register & open UART port
    ret = Ql_UART_Register(m_myUartPort, CallBack_UART_Hdlr, NULL);
    if (ret < QL_RET_OK)
    {
        Ql_Debug_Trace("Fail to register serial port[%d], ret=%d\r\n", m_myUartPort, ret);
    }
	
    ret = Ql_UART_Open(m_myUartPort, 115200, FC_NONE);
    if (ret < QL_RET_OK)
    {
        Ql_Debug_Trace("Fail to open serial port[%d], ret=%d\r\n", m_myUartPort, ret);
    }
    APP_DEBUG("<--QuecOpen: SSL TEST.-->\r\n");

    //register process timer 
    Ql_Timer_Register(SSL_TIMER_ID, Callback_Timer, NULL);

	//register  recv callback
	Ql_SSL_Recv_Register(callback_ssl_recv);
	APP_DEBUG("<--register recv callback successful(%d)-->\r\n",ret);

    while(TRUE)
    {
        Ql_OS_GetMessage(&msg);
        switch(msg.message)
        {
#ifdef __OCPU_RIL_SUPPORT__
        case MSG_ID_RIL_READY:
            APP_DEBUG("<-- RIL is ready -->\r\n");
            Ql_RIL_Initialize();
            break;
#endif
		case MSG_ID_URC_INDICATION:
		{     
			switch (msg.param1)
            {
    		    case URC_SIM_CARD_STATE_IND:
	    			APP_DEBUG("<-- SIM Card Status:%d -->\r\n", msg.param2);
     				if(SIM_STAT_READY == msg.param2)
     				{
                        Ql_Timer_Start(SSL_TIMER_ID, SSL_TIMER_PERIOD, TRUE);
     				}
					
    			break;	
									
				case URC_SSL_CONFIG:
				{
					ssl_urc_param_ptr = msg.param2;
					
					if(Ql_memcmp(ssl_urc_param_ptr->param_item,"\"seclevel\"",Ql_strlen("\"seclevel\"")) == 0)
					{
						APP_DEBUG("+QSSLCFG: %d,%d,%s,%d\r\n",ssl_urc_param_ptr->contextid,ssl_urc_param_ptr->connectid,ssl_urc_param_ptr->param_item,ssl_urc_param_ptr->seclevel);
					}
					else if(Ql_memcmp(ssl_urc_param_ptr->param_item,"\"dataformat\"",Ql_strlen("\"dataformat\"")) == 0)
					{
						APP_DEBUG("+QSSLCFG: %d,%d,%s,%d,%d\r\n",ssl_urc_param_ptr->contextid,ssl_urc_param_ptr->connectid,ssl_urc_param_ptr->param_item,ssl_urc_param_ptr->send_data_format,ssl_urc_param_ptr->recv_data_format);
					}
					else if(Ql_memcmp(ssl_urc_param_ptr->param_item,"\"timeout\"",Ql_strlen("\"timeout\"")) == 0)
					{
						APP_DEBUG("+QSSLCFG: %d,%d,%s,%d\r\n",ssl_urc_param_ptr->contextid,ssl_urc_param_ptr->connectid,ssl_urc_param_ptr->param_item,ssl_urc_param_ptr->timeout);
					}
				}
				break;
						
				case URC_SSL_OPEN:
				{
					ssl_urc_param_ptr = msg.param2;
					APP_DEBUG("<-- open SSL connection successfully!-->\r\n");
					APP_DEBUG("+QSSLOPEN: %d,%d,%d\r\n",ssl_urc_param_ptr->contextid,ssl_urc_param_ptr->connectid,ssl_urc_param_ptr->err);
					m_ssl_state = STATE_SSL_SEND;
				}
				break; 
												
				case URC_SSL_CLOSE:
				{
					ssl_urc_param_ptr = msg.param2;
					APP_DEBUG("<-- close SSL connection successfully!-->\r\n");
					APP_DEBUG("+QSSLCLOSE: %d,%d,%d\r\n",ssl_urc_param_ptr->contextid,ssl_urc_param_ptr->connectid,ssl_urc_param_ptr->err);
				}
				break; 
						
				case URC_SSL_DISC:
				{
					ssl_urc_param_ptr = msg.param2;
					APP_DEBUG("<-- SSL connection closed by server!-->\r\n");
					APP_DEBUG("+QSSLURC: \"closed\",%d,%d\r\n",ssl_urc_param_ptr->contextid,ssl_urc_param_ptr->connectid);
				}
				break; 
						
				default:
					APP_DEBUG("<-- Other URC: type=%d\r\n", msg.param1);
				break;
			}
		}
		break;
	default:
         break;
        }
    }
}
 
static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara)
{

}
static void Callback_Timer(u32 timerId, void* param)
{
	s32 ret;
    if (SSL_TIMER_ID == timerId)
    {
    	switch (m_ssl_state)
        {
        	case STATE_NW_QUERY_STATE:
			{
                s32 cgreg = 0;
				
                ret = RIL_NW_GetEGPRSState(&cgreg);
                APP_DEBUG("<-- Network State: cgreg = %d -->\r\n", cgreg);
				
                if((cgreg == NW_STAT_REGISTERED) || (cgreg == NW_STAT_REGISTERED_ROAMING))
                {
                    m_ssl_state = STATE_NW_GET_LOCALIP;
                }
			}
			break;
			
			case STATE_NW_GET_LOCALIP:
            {
                ST_Addr_Info_t addr_info;

                Ql_memset(addr_info.addr, 0, IP_ADDR_LEN);
				addr_info.addr_len = IP_ADDR_LEN;
				
                ret = Ql_GetLocalIPAddress(0, &addr_info);
                if(ret == SOC_SUCCESS)
                {
                    APP_DEBUG("<-- Get Local IP Successfully, Local IP = %s -->\r\n",addr_info.addr);
                    m_ssl_state = STATE_SSL_CFG;
                }
				else
                {
                    APP_DEBUG("<-- Get Local IP Failed, ret = %d -->\r\n", ret);
                }
            }
			break;

			case STATE_SSL_CFG:
			{
				ssl_param_t.contextid = 1;
		        ssl_param_t.connectid = 0;
		        ssl_param_t.cfg_param = Param_Item_Seclevel;
				ssl_param_t.seclevel= Auth_Mode_Server_and_Client;//Two-way Authentication
					
				ret = RIL_QSSL_CFG(&ssl_param_t, Cfg_Select_Config_Mode, NULL);
				if(ret == RIL_AT_SUCCESS)
				{
					APP_DEBUG("OK\r\n");
				}
				else
				{
					APP_DEBUG("ERROR\r\n");
					return;
				}
				
				ssl_param_t.contextid = 1;
		        ssl_param_t.connectid = 0;
		        ssl_param_t.cfg_param = Param_Item_Cacert;
				
				userdata.pbuffer = Ql_MEM_Alloc(sizeof(u8)*QSSL_DATAMODE_MAX_LEN);
				if(userdata.pbuffer != NULL)
				{
					Ql_memset(userdata.pbuffer, 0, QSSL_DATAMODE_MAX_LEN);
					Ql_memcpy(userdata.pbuffer, cacert, Ql_strlen(cacert));
				}
				
				userdata.length = Ql_strlen(cacert);
				ret = RIL_QSSL_CFG(&ssl_param_t, Cfg_Select_Config_Mode, &userdata);
				if(ret == RIL_AT_SUCCESS)
				{
					APP_DEBUG("OK\r\n");
					APP_DEBUG("<-- Configure cacert successfully!-->\r\n");
				}
				else
				{
					APP_DEBUG("ERROR\r\n");
					APP_DEBUG("<-- Configure cacert failed, ret = %d -->\r\n",ret);
					Ql_MEM_Free(userdata.pbuffer);
					userdata.pbuffer = NULL;
					return;
				}
				
				ssl_param_t.contextid = 1;
				ssl_param_t.connectid = 0;
				ssl_param_t.cfg_param = Param_Item_Clientcert;
				
				userdata.pbuffer = Ql_MEM_Alloc(sizeof(u8)*QSSL_DATAMODE_MAX_LEN);
				if(userdata.pbuffer != NULL)
				{
					Ql_memset(userdata.pbuffer, 0, QSSL_DATAMODE_MAX_LEN);
					Ql_memcpy(userdata.pbuffer, clientcert, Ql_strlen(clientcert));
				}
				
				userdata.length = Ql_strlen(clientcert);
				ret = RIL_QSSL_CFG(&ssl_param_t, Cfg_Select_Config_Mode, &userdata);
				if(ret == RIL_AT_SUCCESS)
				{
					APP_DEBUG("OK\r\n");
					APP_DEBUG("<-- Configure clientcert successfully!-->\r\n");
				}
				else
				{
					APP_DEBUG("ERROR\r\n");
					APP_DEBUG("<-- Configure clientcert failed, ret = %d -->\r\n",ret);
					Ql_MEM_Free(userdata.pbuffer);
					userdata.pbuffer = NULL;
					return;
				}
				
				
				ssl_param_t.contextid = 1;
		        ssl_param_t.connectid = 0;
		        ssl_param_t.cfg_param = Param_Item_Clientkey;
					
				userdata.pbuffer = Ql_MEM_Alloc(sizeof(u8)*QSSL_DATAMODE_MAX_LEN);
				if(userdata.pbuffer != NULL)
				{
					Ql_memset(userdata.pbuffer, 0, QSSL_DATAMODE_MAX_LEN);
					Ql_memcpy(userdata.pbuffer, clientkey, Ql_strlen(clientkey));
				}
				
				userdata.length = Ql_strlen(clientkey);
				ret = RIL_QSSL_CFG(&ssl_param_t, Cfg_Select_Config_Mode, &userdata);
				if(ret == RIL_AT_SUCCESS)
				{
					APP_DEBUG("OK\r\n");
					APP_DEBUG("<-- Configure clientkey successfully!-->\r\n");
					m_ssl_state = STATE_SSL_OPEN;
				}
				else
				{
					APP_DEBUG("ERROR\r\n");
					APP_DEBUG("<-- Configure clientkey failed, ret = %d -->\r\n",ret);
				}
				
				Ql_MEM_Free(userdata.pbuffer);
				userdata.pbuffer = NULL;
			}
			break;

			case STATE_SSL_OPEN:
			{
				ssl_param_t.contextid = 1;
				ssl_param_t.connectid = 0;

				ssl_param_t.host_name = Ql_MEM_Alloc(sizeof(u8)*QSSL_HOSTNAME_MAX_LEN);
				if(ssl_param_t.host_name != NULL)
				{
					Ql_memset(ssl_param_t.host_name, 0, QSSL_HOSTNAME_MAX_LEN);
					Ql_memcpy(ssl_param_t.host_name, hostname, Ql_strlen(hostname));
				}

		        ssl_param_t.port = svr_port;
		        ssl_param_t.connect_mode = 0;

				ret = RIL_QSSL_OPEN(&ssl_param_t);
				if(ret == RIL_AT_SUCCESS)
				{
					APP_DEBUG("OK\r\n");
					m_ssl_state = STATE_SSL_TOTAL_NUM;
				}
				else
				{
					APP_DEBUG("ERROR\r\n");
					APP_DEBUG("<-- open SSL connection failed, ret = %d-->\r\n",ret);
				}
				
				Ql_MEM_Free(ssl_param_t.host_name);
				ssl_param_t.host_name = NULL;
			}
			break;

			case STATE_SSL_SEND:
			{
				ssl_param_t.contextid = 1;
				ssl_param_t.connectid = 0;
				
				userdata.pbuffer = Ql_MEM_Alloc(sizeof(u8)*QSSL_SEND_DATA_MAX_LEN);
				if(userdata.pbuffer != NULL)
				{
					Ql_memset(userdata.pbuffer, 0, QSSL_SEND_DATA_MAX_LEN);
					Ql_memcpy(userdata.pbuffer, m_send_buf, Ql_strlen(m_send_buf));
				}
				
				userdata.length = Ql_strlen(m_send_buf);
				ret = RIL_QSSL_SEND(ssl_param_t.contextid, ssl_param_t.connectid, &userdata);
				if(ret == RIL_AT_SUCCESS)
				{
					APP_DEBUG("OK\r\n");
					m_ssl_state = STATE_SSL_TOTAL_NUM;
				}
				else
				{
					APP_DEBUG("ERROR\r\n");
					APP_DEBUG("<-- open SSL connection failed, ret = %d-->\r\n",ret);
				}
				
				Ql_MEM_Free(userdata.pbuffer);
				userdata.pbuffer = NULL;
			}
			break;

			case STATE_SSL_CLOSE:
			{
				ssl_param_t.contextid = 1;
				ssl_param_t.connectid = 0;
				ret = RIL_QSSL_CLOSE(ssl_param_t.contextid, ssl_param_t.connectid);
				if(ret == RIL_AT_SUCCESS)
				{
					APP_DEBUG("OK\r\n");
					Ql_Timer_Stop(SSL_TIMER_ID);
					m_ssl_state = STATE_SSL_TOTAL_NUM;
				}
				else
				{
					APP_DEBUG("ERROR\r\n");
					APP_DEBUG("<-- close SSL connection failed, ret = %d-->\r\n",ret);
				}
			}
			break;
			
			case STATE_SSL_TOTAL_NUM:
			{
				//APP_DEBUG("QuecOpen SSL test!");
			}
			break;
        }
    }
}

#endif // __EXAMPLE_TCPCLIENT__
