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
 *   example_i2c.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   This example describes how to read and write the ADXL345 sensor module through I2C.
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "C_PREDEF=-D __EXAMPLE_I2C__" in makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
 * 
 *   Operation:
 *            
 *     If input "Ql_I2C_Init=", that will initialize the I2C channel.
 *     If input "Ql_I2C_Config=", that will configure the I2C parameters.
 *     If input "Ql_I2C_Write=", that will write bytes to slave equipment through I2C interface.
 *     If input "Ql_I2C_Read=", that will read bytes from slave equipment through I2C interface.
 *     If input "Ql_I2C_Write_Read=", that will read and write bytes through I2C interface.
 *     If input "Ql_I2C_Uninit=", that will release the I2C pins.
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
#ifdef __EXAMPLE_I2C__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cmsis_os2.h"
#include "ril.h"
#include "ql_i2c.h"
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

#define I2C_SLAVE_ADDR          0xA6 // 8bit address,including the last read-write bit(0)
#define I2C_RECVBUF_LENGTH      8
#define I2C_SENDBUF_LENGTH      8

u8 i2c_recv_buff[I2C_RECVBUF_LENGTH] = {0}; 

u8 i2c_send_buff1[I2C_SENDBUF_LENGTH] = {0x2d,0x08};      // power ctrl
u8 i2c_send_buff2[I2C_SENDBUF_LENGTH] = {0x00};           // devId
u8 i2c_send_buff3[I2C_SENDBUF_LENGTH] = {0x32};           // x_data

char uart_read_buff[1024];

static void MainUartRecvCallback(u32 event, void* dataPtr, u32 dataLen)
{
	if((USART_EVENT_RX_TIMEOUT == event) || (USART_EVENT_RECEIVE_COMPLETE == event))
	{
	    // Ql_UART_Write(UART_PORT0,(u8 *)dataPtr,dataLen);

        if(dataLen > 0)
        {
            s32 ret;
            char *p = NULL;
            char* p1 = NULL;
            char* p2 = NULL;
            s8 i2c_type = 0;
            char number_buff[10];
            char temp_buff[100];
            
            memset(uart_read_buff, 0x00, sizeof(uart_read_buff));
            memcpy(uart_read_buff, dataPtr, dataLen);

            // command --> Init the I2C , type 0 or 1.
            p = strstr(uart_read_buff,"Ql_I2C_Init=");
            if(p)
            {
                p1 = strstr(uart_read_buff, "=");
                p2 = strstr(uart_read_buff, "\r\n");
                if(p1 && p2)
                {

                    memset(number_buff, 0x0, 10);
                    memcpy(number_buff, p1 + 1, p2 - p1 -1);
                
                    i2c_type = atoi(number_buff);
                    if(0 == i2c_type)
                    {
                        ret = Ql_I2C_Init(0,PINNAME_GPIO4,PINNAME_GPIO5,0);
                        if(ret < QL_RET_OK)
                        {
                            APP_DEBUG("\r\n<-- failed!! I2C controller Ql_I2C_Init channel 0 fail ret=%d-->\r\n",ret);
                        }
                        else
    					{
                        	APP_DEBUG("\r\n<--pins(SCL=%d,SDA=%d) I2C controller Ql_I2C_Init channel 0 ret=%d-->\r\n",PINNAME_GPIO4,PINNAME_GPIO5,ret);
                        }
                    }
                    else if(1 == i2c_type)// I2C controller
                    {
                        ret = Ql_I2C_Init(1,PINNAME_I2C_SCL,PINNAME_I2C_SDA,1);
                        if(ret < QL_RET_OK)
                        {
                            APP_DEBUG("\r\n<-- failed!! I2C controller Ql_I2C_Init channel 1 fail ret=%d-->\r\n",ret);
                        }
                        else
    					{
                        	APP_DEBUG("\r\n<--pins(SCL=%d,SDA=%d) I2C controller Ql_I2C_Init channel 1 ret=%d-->\r\n",PINNAME_I2C_SCL,PINNAME_I2C_SDA,ret);
                        }
                    }
                    else
                    {
                        APP_DEBUG("\r\n<--I2C type error!!!!-->\r\n");
                    }
                }
                else
                {
                    APP_DEBUG("\r\n<-- The command must have \"\\r\\n\" or \"=\" -->\r\n");
                }
				return;
            }


            memset(temp_buff, 0x00,sizeof(temp_buff));
            sprintf(temp_buff, "Ql_I2C_Config=0\r\n");  // simultion I2C  (channel 0)
            ret = strncmp(uart_read_buff, temp_buff, strlen(temp_buff));                
            if(0 == ret)
            {
                ret = Ql_I2C_Config(0, TRUE, HAL_I2C_BPS_100K);// just for the I2C controller
                if(ret < QL_RET_OK)
                {
                    APP_DEBUG("\r\n<--failed !! I2C controller Ql_I2C_Config channel 0 fail ret=%d-->\r\n",ret);
                }
				else
				{
                	APP_DEBUG("\r\n<--I2C controller Ql_I2C_Config channel 0 ret=%d-->\r\n",ret);
                }
                return;
            }
            
            //command-->I2C config,  I2C controller interface
            memset(temp_buff, 0x0, sizeof(temp_buff));
            sprintf(temp_buff, "Ql_I2C_Config=1\r\n");//   I2C controller  (channel 1)
            ret = strncmp(uart_read_buff, temp_buff, strlen(temp_buff));
            if(0 == ret)
            {
                ret = Ql_I2C_Config(1, TRUE, HAL_I2C_BPS_100K);// just for the I2C controller
                if(ret < QL_RET_OK)
                {
                    APP_DEBUG("\r\n<--failed !! I2C controller Ql_I2C_Config channel 1 fail ret=%d-->\r\n",ret);
                }
				else
				{
                	APP_DEBUG("\r\n<--I2C controller Ql_I2C_Config channel 1 ret=%d-->\r\n",ret);
                }
                return;
            }
            
            //command-->I2C write  
            memset(temp_buff, 0x0, sizeof(temp_buff));
            sprintf(temp_buff, "Ql_I2C_Write=0\r\n");//  channel 0 
            ret = strncmp(uart_read_buff, temp_buff, strlen(temp_buff));                
            if(0 == ret)
            {
                ret = Ql_I2C_Write(0,I2C_SLAVE_ADDR,(u8*)i2c_send_buff1,2);
                if(ret < QL_RET_OK)
                {
                    APP_DEBUG("\r\n<--failed !! I2C controller Ql_I2C_Write channel 0 fail ret=%d-->\r\n",ret);
                }
				else
				{
                	APP_DEBUG("\r\n<--I2C controller Ql_I2C_Write ret=%d-->\r\n",ret);
				}
                return;
            }

            memset(temp_buff, 0x0, sizeof(temp_buff));
            sprintf(temp_buff, "Ql_I2C_Write=1\r\n");//  channel 1
            ret = strncmp(uart_read_buff, temp_buff, strlen(temp_buff));                
            if(0 == ret)
            {
                ret = Ql_I2C_Write(1,I2C_SLAVE_ADDR,(u8*)i2c_send_buff1,2);
                if(ret < QL_RET_OK)
                {
                    APP_DEBUG("\r\n<--failed !! I2C controller Ql_I2C_Write channel 1 fail ret=%d-->\r\n",ret);
                }
				else
				{
                	APP_DEBUG("\r\n<--I2C controller Ql_I2C_Write ret=%d-->\r\n",ret);
				}
                return;
            }

            //command-->I2C read
            memset(temp_buff, 0x0, sizeof(temp_buff));
            sprintf(temp_buff, "Ql_I2C_Read=0\r\n");//  channel 0 
            ret = strncmp(uart_read_buff, temp_buff, strlen(temp_buff));                
            if(0 == ret)
            {
                ret = Ql_I2C_Write(0,I2C_SLAVE_ADDR,i2c_send_buff2,1);
                ret = Ql_I2C_Read(0, I2C_SLAVE_ADDR, i2c_recv_buff, 1);// read dev_id

                if(ret < QL_RET_OK)
                {
                    APP_DEBUG("\r\n<--failed !! I2C controller Ql_I2C_Read channel 0 fail ret=%d-->\r\n",ret);
                }
				else
				{
                	APP_DEBUG("\r\n<--read_buffer[0]=(%d)-->\r\n",i2c_recv_buff[0]);
                }
                return;
            }

            memset(temp_buff, 0x0, sizeof(temp_buff));
            sprintf(temp_buff, "Ql_I2C_Read=1\r\n");//  channel 1 
            ret = strncmp(uart_read_buff, temp_buff, strlen(temp_buff));                
            if(0 == ret)
            {
                ret = Ql_I2C_Write(1,I2C_SLAVE_ADDR,i2c_send_buff2, 1);
                ret = Ql_I2C_Read(1, I2C_SLAVE_ADDR, i2c_recv_buff, 1);// read dev_id
                if(ret < QL_RET_OK)
                {
                    APP_DEBUG("\r\n<--failed !! I2C controller Ql_I2C_Read channel 1 fail ret=%d-->\r\n",ret);
                }
				else
				{
                	APP_DEBUG("\r\n<--read_buffer[0]=(%d)-->\r\n",i2c_recv_buff[0]);
                }
                return;
            }

            //command-->I2C write then read  
            memset(temp_buff, 0x0, sizeof(temp_buff));
            sprintf(temp_buff, "Ql_I2C_Write_Read=0\r\n");//  channel 0
            ret = strncmp(uart_read_buff, temp_buff, strlen(temp_buff));
            if(0 == ret)
            {
                ret = Ql_I2C_Write(0,I2C_SLAVE_ADDR,(u8*)i2c_send_buff1,2);
                ret = Ql_I2C_Write_Read(0, I2C_SLAVE_ADDR, (u8*)i2c_send_buff3, 1,i2c_recv_buff, 6);
                if(ret < QL_RET_OK)
                {
                    APP_DEBUG("\r\n<--failed !! I2C controller Ql_I2C_Write_Read channel 0 fail ret=%d-->\r\n",ret);
                }
				else
				{
                	APP_DEBUG("\r\n<--read_buffer[0]=(%d),read_buffer[1]=(%d),read_buffer[2]=(%d),read_buffer[3]=(%d),read_buffer[4]=(%d),read_buffer[5]=(%d)-->\r\n",i2c_recv_buff[0],i2c_recv_buff[1],i2c_recv_buff[2],i2c_recv_buff[3],i2c_recv_buff[4],i2c_recv_buff[5]);
                }
				return;
            }

            memset(temp_buff, 0x0, sizeof(temp_buff));
            sprintf(temp_buff, "Ql_I2C_Write_Read=1\r\n");//  channel 1 
            ret = strncmp(uart_read_buff, temp_buff, strlen(temp_buff));                
            if(0 == ret)
            {
                ret = Ql_I2C_Write(1,I2C_SLAVE_ADDR,(u8*)i2c_send_buff1,2);
                ret = Ql_I2C_Write_Read(1, I2C_SLAVE_ADDR, (u8*)i2c_send_buff3, 1,i2c_recv_buff, 6);
                if(ret < QL_RET_OK)
                {
                    APP_DEBUG("\r\n<--failed !! I2C controller Ql_I2C_Write_Read channel 1 fail ret=%d-->\r\n",ret);
                }
				else
				{
                	APP_DEBUG("\r\n<--read_buffer[0]=(%d),read_buffer[1]=(%d),read_buffer[2]=(%d),read_buffer[3]=(%d),read_buffer[4]=(%d),read_buffer[5]=(%d)-->\r\n",i2c_recv_buff[0],i2c_recv_buff[1],i2c_recv_buff[2],i2c_recv_buff[3],i2c_recv_buff[4],i2c_recv_buff[5]);
                }
				return;
            }

            //command-->I2C write then read
            memset(temp_buff, 0x0, sizeof(temp_buff));
            sprintf(temp_buff, "Ql_I2C_Uninit=0\r\n");//  channel 0 
            ret = strncmp(uart_read_buff, temp_buff, strlen(temp_buff));                
            if(0 == ret)
            {
                ret = Ql_I2C_Uninit(0);
                if(ret < QL_RET_OK)
                {
                    APP_DEBUG("\r\n<--failed !! I2C controller Ql_I2C_Uninit channel 0 fail ret=%d-->\r\n",ret);
                }
				else
				{
                	APP_DEBUG("\r\n<--I2C controller (chnnlNo 0) Ql_I2C_Uninit  ret=%d-->\r\n",ret);
				}
                return;
            }

            memset(temp_buff, 0x0, sizeof(temp_buff));
            sprintf(temp_buff, "Ql_I2C_Uninit=1\r\n");//  channel 1 
            ret = strncmp(uart_read_buff, temp_buff, strlen(temp_buff));                
            if(0 == ret)
            {
                ret = Ql_I2C_Uninit(1);
                if(ret < QL_RET_OK)
                {
                    APP_DEBUG("\r\n<--failed !! I2C controller Ql_I2C_Uninit channel 1 fail ret=%d-->\r\n",ret);
                }
				else
				{
                	APP_DEBUG("\r\n<--I2C controller (chnnlNo 1) Ql_I2C_Uninit  ret=%d-->\r\n",ret);
				}
                return;
            }
            
        	APP_DEBUG("\r\n<--Not found this command, please check you command-->\r\n");            
            
        }
	}
}


void proc_main_task(void)
{
    
    Ql_SleepDisable();
	Ql_RIL_Initialize();

 	Ql_UART_Open(UART_PORT0,115200,MainUartRecvCallback);
	
	APP_DEBUG("<-- QuecOpen: I2C Example -->\r\n");

    
    while (1)
    {
    	osDelay(1000);
	}
}

#endif // __EXAMPLE_I2C__

