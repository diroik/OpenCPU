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
 *   example_file.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   This example demonstrates how to use filesystem in QuecOpen.
 *
 *
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "GLOBAL_EXPORT_FLAG += __EXAMPLE_FILESYSTEM__" in makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
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
#ifdef __EXAMPLE_FILESYSTEM__
#include <stdio.h>
#include <string.h>

#include "ql_type.h"
#include "ql_uart.h"
#include "ql_fs.h"
#include "ql_error.h"
#include "ql_ps.h"
#include "ril.h"
#include "ql_power.h"
#include "ql_dbg.h"

#define DEBUG_ENABLE 1
#if DEBUG_ENABLE > 0
#define DEBUG_PORT  PORT_DBG_LOG
#define DBG_BUF_LEN   512
static char DBG_BUFFER[DBG_BUF_LEN];
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
#else
#define APP_DEBUG(FORMAT,...) 
#endif



typedef enum{
    STATE_GET_SPACE1,
    STATE_CREATE_FILE,
    STATE_CHECK_FILE,
    STATE_WRITE_FILE,
    STATE_GET_SPACE2,
    STATE_READ_FILE,
    STATE_QUERY_FILE_SIZE,
    STATE_DELETE_FILE,
    
    STATE_TOTAL_NUM
}Enum_FILE_OPERATE;
#define LENGTH 64

static u8 ceregCheckTimes = 1;

extern osMessageQueueId_t maintask_queue;


static void UartSendATCmd(u8* dataPtr, u32 dataLen);

static void MainUartRecvCallback(u32 event, void* dataPtr, u32 dataLen)
{
	if((event == USART_EVENT_RX_TIMEOUT) || (event == USART_EVENT_RECEIVE_COMPLETE))
	{
	    Ql_UART_Write(UART_PORT0,(u8 *)dataPtr,dataLen);
	    UartSendATCmd(dataPtr, dataLen);
	}
}

static s32 pscallback(ENUMurcID eventId, void *param, u32 paramLen)
{
	CmiSimImsiStr *pCmiSimImsiInd = NULL;
	ST_MSG msg;
	
	switch (eventId)
	{
		case URC_ID_PS_BEARER_ACTED:
			msg.message = MSG_ID_URC_INDICATION;	
			msg.param1 = URC_EGPRS_NW_STATE_IND;
			osMessageQueuePut(maintask_queue, &msg, 0,0);
			//APP_DEBUG("BEARER_ACTED\r\n");//param NULL
			break;
		case URC_ID_PS_BEARER_DEACTED:
			msg.message = MSG_ID_URC_INDICATION;	
			msg.param1 = URC_EGPRS_NW_STATE_IND;
			osMessageQueuePut(maintask_queue, &msg, 0,0);
			//APP_DEBUG("BEARER_DEACTED\r\n");//param NULL
			break;
		case URC_ID_SIM_READY:
			pCmiSimImsiInd = (CmiSimImsiStr *)param;
			APP_DEBUG("SIM_READY:[%s]\r\n",pCmiSimImsiInd->contents);
			msg.message = MSG_ID_URC_INDICATION;	
			msg.param1 = URC_SIM_CARD_STATE_IND;
			osMessageQueuePut(maintask_queue, &msg, 0,0);
			break;
		case URC_ID_SIM_REMOVED:
			APP_DEBUG("SIM_REMOVED\r\n");//param NULL
			break;

		default:
			break;
	}
	return 0;
}

static void app_file_demo_entry(ST_MSG msg);
void proc_main_task(void)  
{
	s32 ret = -1;
	ST_MSG msg;
	u8 nw_state = 0;
	Ql_SleepDisable();

	Ql_RIL_Initialize();
 	Ql_UART_Open(UART_PORT0,115200,MainUartRecvCallback);
	
	APP_DEBUG("<-- QuecOpen: File Example -->\r\n");
	ret = Ql_PSCallback_Register(GROUP_ALL_MASK,pscallback);
	osDelay(1000);
	maintask_queue = osMessageQueueNew(8, sizeof(msg), NULL);

    while (1)
    {
		if(osOK == osMessageQueueGet(maintask_queue,(void*)&msg, NULL, osWaitForever))
		switch(msg.message)
        {
        	case MSG_ID_URC_INDICATION:
				{
		            //APP_DEBUG("<-- Received URC: type: %d, -->\r\n", msg.param1);
		            switch (msg.param1)
		            {	
		            	case URC_SIM_CARD_STATE_IND:
							{
			                	msg.message = MSG_ID_APP_START;	
								msg.param1 = STATE_GET_SPACE1;
								osMessageQueuePut(maintask_queue, &msg, 0,0);
			                    APP_DEBUG("<-- Module has registered to network, status:%d -->\r\n",nw_state);						
							}
		                	break; 
						
		            	case URC_EGPRS_NW_STATE_IND:
							{
								ret = Ql_GetCeregState(&nw_state);
				                if((1== nw_state) || (5 == nw_state))
				                {
									ceregCheckTimes = 1;
									                	
				                    APP_DEBUG("<-- Module has registered to network, status:%d -->\r\n",nw_state);
				                }
								else
				                {
									ceregCheckTimes++;
									if(ceregCheckTimes > 120)
									{
										ceregCheckTimes = 1;
				                    	APP_DEBUG("<-- Module has deregister to network, status:%d,ret:%d -->\r\n",nw_state,ret);

										// register network fail
									}
									else
									{
										msg.message = MSG_ID_URC_INDICATION;	
										msg.param1 = URC_EGPRS_NW_STATE_IND;
										osMessageQueuePut(maintask_queue, &msg, 0,0);
										osDelay(500);
									}
				                }
							}
		                	break; 
						
		            	default:
		                	APP_DEBUG("<-- Other URC: type = %d\r\n", msg.param1);
		                	break;
		            }
				}
            	break;
			case MSG_ID_APP_START:
				{
					app_file_demo_entry(msg);			
				}
				break;
        	default:
            	break;
        }
       
	}

}


static void next(ST_MSG msg)
{
	msg.message = MSG_ID_APP_START;	
	msg.param1 +=1;
	osMessageQueuePut(maintask_queue, &msg, 0,0);
}

static void app_file_demo_entry(ST_MSG msg)
{
	s32 ret = -1;
    static s32 handle =-1;
    //static s32 handle1 =-1;
    //static s32 handle2 =-1;
	static char fileName[LENGTH] = "test.txt";
	static char fileName1[LENGTH] = "test1.txt";
	static char fileName2[LENGTH] = "test2.txt";
	static u8 w_file_buff[2048] = 	"123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789"\
									"123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789\0";
	static u8 r_file_buff[2048] = {0};
	u32 writenLen = 0;
	u32 readenLen = 0;
	s32 position = 0;
	u32 filesize = 0;
	//APP_DEBUG("<-- app_file_demo_entry: param1 = %d\r\n", msg.param1);
	osDelay(1000);
	switch (msg.param1)
	{
		case STATE_GET_SPACE1:
			APP_DEBUG("<------------ GET_SPACE entry: param1 = %d ------------>\r\n", msg.param1);
            ret = Ql_FS_GetTotalSpace();
            APP_DEBUG("<--GetTotalSpace:%d-->\r\n",ret);
            ret = Ql_FS_GetFreeSpace();
            APP_DEBUG("<--GetFreeSpace:%d-->\r\n",ret);
            next(msg);
			
			break;
		case STATE_CREATE_FILE:				
#if 0
			APP_DEBUG("<------------ CREATE_FILE entry: param1 = %d ------------>\r\n", msg.param1);
            handle = Ql_FS_Open(fileName,QL_READ_WRITE );
            if(handle<0)
            {
            	if(QL_RET_ERR_FILE_STATE == handle)
            	{         		
                	handle = Ql_FS_Open(fileName,QL_READ_WRITE_CREATE_ALWAYS); //if file exist and opened,recreate it
                    APP_DEBUG("<-- Recreate file (%s) OK! handle =%d -->\r\n",fileName,handle);
				}
                else
                {
                    APP_DEBUG("<-- failed!! Create file (%s) fail,handle = %d-->",fileName,handle);
                }
            }
            else
            {
                APP_DEBUG("<--The file(%s) create success,handle = %d-->\r\n",fileName,handle);
            }
            Ql_FS_Close(handle);
#if 1
            handle1 = Ql_FS_Open(fileName1,QL_READ_WRITE );
            if(handle1<0)
            {
            	if(QL_RET_ERR_FILE_STATE == handle1)
            	{
                	handle1 = Ql_FS_Open(fileName1,QL_READ_WRITE_CREATE_ALWAYS); //if file exist and opened,recreate it
                    APP_DEBUG("<-- Recreate file (%s) OK! handle1 =%d -->\r\n",fileName1,handle1);
				}
                else
                {
                    APP_DEBUG("<-- failed!! Create file (%s) fail,handle1 = %d-->",fileName1,handle1);
                }
            }
            else
            {
                APP_DEBUG("<--The file(%s) create success,handle1 = %d-->\r\n",fileName1,handle1);
            }
            //Ql_FS_Close(handle1);

            handle2 = Ql_FS_Open(fileName2,QL_READ_WRITE );
            if(handle2<0)
            {
            	if(QL_RET_ERR_FILE_STATE == handle2)
            	{
                	handle2 = Ql_FS_Open(fileName2,QL_READ_WRITE_CREATE_ALWAYS); //if file exist and opened,recreate it
                    APP_DEBUG("<-- Recreate file (%s) OK! handle =%d -->\r\n",fileName2,handle2);
				}
                else
                {
                    APP_DEBUG("<-- failed!! Create file (%s) fail,handle2 = %d-->",fileName2,handle2);
                }
            }
            else
            {
                APP_DEBUG("<--The file(%s) create success,handle2 = %d-->\r\n",fileName2,handle2);
            }
            Ql_FS_Close(handle2);
            handle2=-1;
#endif	
#endif
			next(msg);

			break;
		case STATE_CHECK_FILE:
			APP_DEBUG("<------------ CHECK_FILE entry: param1 = %d ------------>\r\n", msg.param1);
            ret = Ql_FS_Check(fileName);
            if(ret==0)
            {
               APP_DEBUG("<--check success,file exist,fileName:%s-->\r\n",fileName);
            }
            else if(ret==QL_RET_ERR_FILENOTFOUND)
            {
               APP_DEBUG("<-- check success,file not exist ret=%d-->\r\n",ret);
            }
            else 
            {
               APP_DEBUG("<-- check param error ret=%d-->\r\n",ret);
            }

			ret = Ql_FS_Check(fileName1);
            if(ret==0)
            {
               APP_DEBUG("<--check success,file exist,fileName2:%s-->\r\n",fileName1);
            }
            else if(ret==QL_RET_ERR_FILENOTFOUND)
            {
               APP_DEBUG("<-- check success,file not exist ret=%d-->\r\n",ret);
            }
            else 
            {
               APP_DEBUG("<-- check param error ret=%d-->\r\n",ret);
            }

			ret = Ql_FS_Check(fileName2);
            if(ret==0)
            {
               APP_DEBUG("<--check success,file exist,fileName2:%s-->\r\n",fileName2);
            }
            else if(ret==QL_RET_ERR_FILENOTFOUND)
            {
               APP_DEBUG("<-- check success,file not exist ret=%d-->\r\n",ret);
            }
            else 
            {
               APP_DEBUG("<-- check param error ret=%d-->\r\n",ret);
            }
            next(msg);
			
			break;
		case STATE_WRITE_FILE:
			APP_DEBUG("<------------ WRITE_FILE entry: param1 = %d ------------>\r\n", msg.param1);
#if 0
            handle = Ql_FS_Open(fileName,QL_READ_WRITE);
            if(handle<0)
            {
               APP_DEBUG("<--The file open failed,handle:%d-->\r\n",handle);
               break;
            }
            else
            {
               APP_DEBUG("<--Open success go to write:-->\r\n");
            }
#endif
            //ret = Ql_FS_Seek(handle,0,QL_FS_FILE_END);  //seek end 
            //APP_DEBUG("<-- Ql_FS_Seek  ret =%d  -->\r\n",ret);
            ret = Ql_FS_Write(1027, w_file_buff, strlen((char*)w_file_buff), &writenLen);
            APP_DEBUG("<-- Ql_FS_Write  ret =%d  writenLen =%d-->\r\n",ret,writenLen);
            //Ql_FS_Flush(handle); //fflush
            APP_DEBUG("<-- Ql_FS_Flush  ok-->\r\n");                
            //Ql_FS_Close(handle);//close the file
            //next(msg);
			
			break;
		case STATE_GET_SPACE2:
			APP_DEBUG("<------------ GET_SPACE entry: param1 = %d ------------>\r\n", msg.param1);
            ret = Ql_FS_GetTotalSpace();
            APP_DEBUG("<--GetTotalSpace:%d-->\r\n",ret);
            ret = Ql_FS_GetFreeSpace();
            APP_DEBUG("<--GetFreeSpace:%d-->\r\n",ret);
            
            next(msg);
			
			break;
		case STATE_READ_FILE:
			APP_DEBUG("<------------ READ_FILE entry: param1 = %d ------------>\r\n", msg.param1);
		    handle = Ql_FS_Open(fileName,QL_READ_WRITE);
            if(handle<0)
            {
                APP_DEBUG("<--The file open failed!-->\r\n");
                break;
            }
            APP_DEBUG("<--The r_file_buff is :");
            ret = Ql_FS_Seek(handle,0,QL_FS_FILE_BEGIN); 
            ret = Ql_FS_Read(handle, r_file_buff, 2048-1, &readenLen);
            APP_DEBUG("r_file_buff:%s,readenLen:%d\r\n",r_file_buff,readenLen);
            while(readenLen>=(LENGTH-1))
            {
                memset(r_file_buff, 0x0, sizeof(r_file_buff));
                position = Ql_FS_GetFilePosition(handle);//get postion
                APP_DEBUG("position:%d,handle:%d",position,handle);
                ret = Ql_FS_Seek(handle,position,QL_FS_FILE_BEGIN);//seek
                ret = Ql_FS_Read(handle, r_file_buff, LENGTH-1, &readenLen);
                APP_DEBUG("%s\r\n",r_file_buff);
            }
            Ql_FS_Close(handle);//close the file
            next(msg);
			
			break;
		case STATE_QUERY_FILE_SIZE:
			APP_DEBUG("<--------- QUERY_FILE_SIZE entry: param1 = %d --------->\r\n", msg.param1);
			handle = Ql_FS_Open(fileName,QL_READ_WRITE);
            if(handle<0)
            {
                APP_DEBUG("<--The file open failed!-->\r\n");
                break;
            }
		    filesize = Ql_FS_GetSize(handle);
			APP_DEBUG("<-- Ql_FS_GetSize(%d) = %d \r\n",handle,filesize);
            Ql_FS_Close(handle);//close the file
            next(msg);
			
			break;
		case STATE_DELETE_FILE:
			APP_DEBUG("<------------ DELETE_FILE entry: param1 = %d ------------>\r\n", msg.param1);
//					ret=Ql_FS_Check(fileName);
//                    if(ret!=QL_RET_OK)
//                    {
//                         APP_DEBUG("\r\n<-- file=%s don't exist! -->",fileName);
//                         break;
//                    }
			ret = Ql_FS_Format();
            APP_DEBUG("<-- Ql_FS_Format ret=%d! -->\r\n",ret);
			
#if 1

            ret = Ql_FS_Delete(fileName); 
			if(ret==QL_RET_OK)
            {
                 APP_DEBUG("<-- file=%s delete ok! -->\r\n",fileName);
            }
            else
            {
                APP_DEBUG("<-- Delete error! ret:%d-->\r\n",ret);
            }
			
            ret = Ql_FS_Delete(fileName1);
			if(ret==QL_RET_OK)
            {
                 APP_DEBUG("<-- file=%s delete ok! -->\r\n",fileName1);
            }
            else
            {
                APP_DEBUG("<-- Delete error! ret:%d-->\r\n",ret);
            }
		
            ret = Ql_FS_Delete(fileName2);
            if(ret==QL_RET_OK)
            {
                 APP_DEBUG("<-- file=%s delete ok! -->\r\n",fileName2);
            }
            else
            {
                APP_DEBUG("<-- Delete error! ret:%d-->\r\n",ret);
            }
#endif			
			break;
	}
}

static void ATResponse_Handler(const char *pStr, u32 strLen, void *pArg)
{
    APP_DEBUG("[ATResponse_Handler] receData= [%s].-->\r\n",pStr);

}
static void UartSendATCmd(u8* dataPtr, u32 dataLen)
{
    s32 ret;

    if(0 == dataLen)
    {
        return;
    }
    
    // Echo
    Ql_UART_Write(UART_PORT0, dataPtr, dataLen);

    ret = Ql_RIL_SendATCmd((char* )dataPtr, dataLen, (Callback_ATResponse)ATResponse_Handler, NULL, 0);
    APP_DEBUG("<-- UartSendATCmd,ret = %d\r\n", ret);
}


#endif // __EXAMPLE_FILESYSTEM__

