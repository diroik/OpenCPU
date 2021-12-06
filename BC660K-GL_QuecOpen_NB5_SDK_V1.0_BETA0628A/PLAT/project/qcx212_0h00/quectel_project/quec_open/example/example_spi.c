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
 *   example_spi.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   This example describes how to read and write the W25QXX flash module through SPI.
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "C_PREDEF=-D __EXAMPLE_SPI__" in makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
 * 
 *   Operation:
 *            
 *     If input "ql_spi_rd_flash_id", that will read the flash's ID.
 *     If input "ql_spi_rd_flash_status", that will read the flash's status register.
 *     If input "ql_flash_block_lock", that will lock flash lower 1/2.
 *     If input "ql_flash_block_unlock", that will unlock flash lower 1/2.
 *     If input "ql_spi_rd_flash_md", that will read the manufacturer ID + device ID.
 *	   If input "ql_spi_rd_flash_data", that will read data from spi flash.
 *	   If input "ql_spi_erase_flash_sector", that will erase the specificed sector.
 *	   If input "ql_spi_erase_flash_block", that will erase the specificed block .
 *	   If input "ql_spi_erase_flash_chip", that will erase the whole chip.
 *	   If input "ql_spi_wr_flash_data", that will write data to spi flash.
 *	   If input "Ql_SPI_Uninit", that will disable SPI and release associated pins.
 *		  
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
#ifdef __EXAMPLE_SPI__

#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"
#include "ril.h"
#include "ql_power.h"
#include "ql_spi.h"
#include "ql_dbg.h"
#include "ql_gpio.h"



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

#define USR_SPI_CHANNAL		     (1)

/***************************************************************************
* This example is used for test SPI funtion by hardware.
****************************************************************************/

char uart_read_buff[256];
char buffer[256];

u8 spi_usr_type = 1;

void time_delay(u32 cnt);
static void MainUartRecvCallback(u32 event, void* dataPtr, u32 dataLen);


//spi device select set, only for simulating SPI by software.
void spi_flash_cs(bool CS)
{
	if (!spi_usr_type)
	{
		if (CS)
			Ql_GPIO_SetLevel(PINNAME_SPI_CS,PINLEVEL_HIGH);
		else
			Ql_GPIO_SetLevel(PINNAME_SPI_CS,PINLEVEL_LOW);
	}
}

void proc_main_task(void)
{
    s32 ret = 0;

    Ql_SleepDisable();
	Ql_RIL_Initialize();
 	Ql_UART_Open(UART_PORT0,115200,MainUartRecvCallback);
	
	APP_DEBUG("<-- QuecOpen: SPI Example: %d -->\r\n",spi_usr_type);

    Ql_GPIO_Init( PINNAME_SPI_CS, PINDIRECTION_OUT, PINLEVEL_HIGH , PINPULLSEL_PULLUP);

                        //chnnlNo, 	     //pinClk,        //pinMiso,      	//pinMosi,        //pinCs, 	//spiType
    ret = Ql_SPI_Init(USR_SPI_CHANNAL, PINNAME_SPI_SCLK, PINNAME_SPI_MISO, PINNAME_SPI_MOSI, PINNAME_SPI_CS, spi_usr_type);
    if(ret < QL_RET_OK)
    {
        APP_DEBUG("\r\n<-- Failed!! Ql_SPI_Init fail , ret =%d-->\r\n",ret);
    }
    else
    {
        APP_DEBUG("\r\n<-- Ql_SPI_Init ret =%d -->\r\n",ret);
    }    

    ret = Ql_SPI_Config(USR_SPI_CHANNAL,1,0,0,200000U);     // config sclk about 200KHz;
    if(ret < QL_RET_OK)
    {
        APP_DEBUG("\r\n<--Failed!! Ql_SPI_Config fail  ret=%d -->\r\n",ret);
    }
    else
    {
        APP_DEBUG("\r\n<-- Ql_SPI_Config  =%d -->\r\n",ret);
    }      

	//init cs pin,only for simulating SPI by software.
	if (!spi_usr_type)
	{
		Ql_GPIO_Init(PINNAME_SPI_CS,PINDIRECTION_OUT,PINLEVEL_HIGH,PINPULLSEL_PULLUP);   //CS high
	}

    while (1)
    {
        osDelay(1000);
	}
}


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
u8 status_reg = 0;          //status register low 8 bits
u8 status_suspend_reg = 0;  //status register high 8 bits

//@flash write enable
void flash_wr_enable()
{
    s32 ret = 0;               
    u8  cmd = 0x06;
	spi_flash_cs(0);
    ret = Ql_SPI_Write(USR_SPI_CHANNAL, &cmd, 1);
	spi_flash_cs(1);
    if(ret==1)
    {
        APP_DEBUG("write enable.\r\n");
    }  
    else 
    {
        APP_DEBUG("write enable failed.\r\n");
    }       
}

//@read the flash id
//manufacturer ID | memory type | memory density
//      C2        |      20     |     15
void flash_rd_id(void)
{
    s32 ret = 0;
    u8 wr_buff[32]={0x9f};
    u8 rd_buff[32]={0xff};

    memset(rd_buff, 0, 32);

	flash_wr_enable();
	spi_flash_cs(0);
    ret = Ql_SPI_WriteRead(USR_SPI_CHANNAL,wr_buff,1,rd_buff,3);
	spi_flash_cs(1);
    if(ret==3)
    {
        APP_DEBUG("0x%x 0x%x 0x%x \r\n", rd_buff[0],rd_buff[1],rd_buff[2]);
    }    
    else 
    {
        APP_DEBUG("func(%s),line(%d),here has a failed operation.\r\n",__func__,__LINE__);
    }
}

//@REMS  manufacturer ID device ID
void flash_rd_id2(void)
{
    s32 ret = 0;
    u8 wr_buff[32]={0x90,0xff,0xff,0x00};
    u8 rd_buff[32]={0xff};

	flash_wr_enable();
	spi_flash_cs(0);
    ret = Ql_SPI_WriteRead(USR_SPI_CHANNAL,wr_buff,4,rd_buff,2);
	spi_flash_cs(1);
    if(ret==2)
    {
        APP_DEBUG("[Debug Log]0x%x 0x%x\r\n", rd_buff[0],rd_buff[1]);
    }
    else 
    {
        APP_DEBUG("[Debug Log]read flash id 2 failed, ret=%d", ret);
    }
}

//read status register low 8 bits
void flash_rd_status_reg(void)
{
    s32 ret = 0;
    u8 wr_buff[32]={0x05};

	flash_wr_enable();
	spi_flash_cs(0);
	
    ret = Ql_SPI_WriteRead(USR_SPI_CHANNAL,wr_buff,1,&status_reg,1);
	spi_flash_cs(1);
    if(ret==1)
    {
        APP_DEBUG("[Debug Log]status_reg: 0x%x.\r\n", status_reg);
    }
    else
    {
        APP_DEBUG("[Debug Log]read status register low 8 bits failed, ret=%d.\r\n",ret);
    }
}

//@read status register high 8 bits
void flash_rd_suspend_status_reg(void)
{
    s32 ret = 0;
    u8 wr_buff[32]={0x35};

	flash_wr_enable();
	spi_flash_cs(0);
    ret = Ql_SPI_WriteRead(USR_SPI_CHANNAL,wr_buff,1,&status_suspend_reg,1);
	spi_flash_cs(1);
    if(ret==1)
    {
        APP_DEBUG("[Debug Log]status_suspend_reg: 0x%x.\r\n", status_suspend_reg);
    }
    else
    {
        APP_DEBUG("[Debug Log]read status register high 8 bits failed, ret=%d.\r\n", ret);
    }
}

/***************************************************************************
* Function: cmd_is_over
* Description: query the operation has been completed or incompleted.
****************************************************************************/ 
s8 cmd_is_over(void)
{
    s8 ret = TRUE;
    flash_rd_status_reg();

    while(status_reg & 0x01)
    {
        osDelay(1000);
        flash_rd_status_reg();
    }
    if (status_reg & 0x01)
    {
        ret = FALSE;
    }
    return ret;
}

//@20210322 lock flash lower 1/2, for Winbond W25Q64DW
void ql_winbond_flash_block_lock(void)
{
    s32 ret = 0;
    u8 wr_buff[32]={0};

    wr_buff[0] = 0x01;
    wr_buff[1] = (status_reg & (~0x7C)) | 0x18;
    wr_buff[2] = status_suspend_reg | 0x40;

	flash_wr_enable();
	spi_flash_cs(0);
    ret = Ql_SPI_Write(USR_SPI_CHANNAL,wr_buff,3);
	spi_flash_cs(1);

    if(ret == 3)
    {
        APP_DEBUG("[Debug Log]flash lock ok.\r\n");
    }  
    else 
    {
        APP_DEBUG("[Debug Log]flash lock ok failed, ret=%d.\r\n", ret);
    }
}

//@20210322 unlock flash lower 1/2, for Winbond W25Q64DW
void ql_winbond_flash_block_unlock(void)
{
    s32 ret = 0;
    u8 wr_buff[32]={0};

    wr_buff[0] = 0x01;

    if(status_suspend_reg & 0x40)
        wr_buff[1] = status_reg | 0x7C;
    else
        wr_buff[1] = status_reg & (~0x7C);

	flash_wr_enable();
	spi_flash_cs(0);
    ret = Ql_SPI_Write(USR_SPI_CHANNAL,wr_buff,2);
	spi_flash_cs(1);
    if(ret==2)
    {
        APP_DEBUG("[Debug Log]flash unlock ok.\r\n");
    }
    else
    {
        APP_DEBUG("[Debug Log]flash unlock ok failed, ret=%d.\r\n", ret);
    }
}

//@flash chip erase()
void flash_erase_chip()
{   
    s32 ret = 0;
    u8 cmd = 0x60;
    flash_wr_enable();    
	spi_flash_cs(0);
    ret = Ql_SPI_Write(USR_SPI_CHANNAL,&cmd,1);
	spi_flash_cs(1);
    //Because erase the whole chip must need more time to wait,
    APP_DEBUG("wait for erasure to complete.\r\n");
    osDelay(10000);
    if(ret==1)
    {
        //Because erase the whole chip must need more time to wait ,so we must check the bit 1 of status register .
        APP_DEBUG("chip erase done.\r\n");        
    }
    else
    {
        APP_DEBUG("func(%s),line(%d),here has a failed operation.\r\n",__func__,__LINE__);
    }    
}

//@flash block erase(0-31)
void flash_erase_block(s8 nblock)
{
    u8 cmd = 0x52;
    s32 ret = 0;   
    u32 addr = nblock * 0x10000;
    u8 wr_buff[4]={cmd,(addr>>16)&0xff,(addr>>8)&0xff,addr & 0xff};
    if (nblock > 31)
    {
        APP_DEBUG("nblock is error para.\r\n");
        return ;
    }    
    flash_wr_enable();    
	spi_flash_cs(0);
    ret = Ql_SPI_Write(USR_SPI_CHANNAL,wr_buff,4);
	spi_flash_cs(1);
    if(ret==4)
    {   
        APP_DEBUG("chip erase ok.\r\n");
    }
    else 
    {
        APP_DEBUG("func(%s),line(%d),here has a failed operation.\r\n",__func__,__LINE__);
    }  
}

//@flash sector erase(0-511)
void flash_erase_sector(s16 nsector)
{
    u8 cmd = 0x20;
    s32 ret = 0;   
    u32 addr = nsector * 0x1000;
    u8 wr_buff[4]={cmd,(addr>>16)&0xff,(addr>>8)&0xff,addr & 0xff};

    if (nsector > 511)
    {
        APP_DEBUG("nsector is error para.\r\n");
        return ;
    }
    flash_wr_enable();
	spi_flash_cs(0);
    ret = Ql_SPI_Write(USR_SPI_CHANNAL,wr_buff,4);
	spi_flash_cs(1);
    if(ret==4)
    {
        APP_DEBUG("sector erase ok.\r\n");
    }
    else 
    {
        APP_DEBUG("func(%s),line(%d),here has a failed operation.\r\n",__func__,__LINE__);
    }
}


//@flash write data (page program)
void flash_wr_data(u32 addr, u8 *pbuff,u32 len)
{
    u8 cmd = 0x02;
    u8 *p_buff = pbuff;
    s16 i=0;
    s32 ret = 0;
    u8 wr_buff[300]={cmd,(addr>>16)&0xff, (addr>>8)&0xff, addr&0xff};
    
    if (len > 256)
    {
        APP_DEBUG("length is too long.\r\n");
        return ;
    }
    for(i=0; i<len; i++)
    {
       wr_buff[i+4] =  *p_buff;
       p_buff++;
    }
    
    flash_wr_enable();
	spi_flash_cs(0);
    ret = Ql_SPI_Write(USR_SPI_CHANNAL,wr_buff,len+4);
	spi_flash_cs(1);
    if(ret==(len+4))
    {
        APP_DEBUG("write data ok.\r\n");
    }
    else 
    {
        APP_DEBUG("func(%s),line(%d),here has a failed operation.\r\n",__func__,__LINE__);
    }       
}

//@flash read data
void flash_rd_data(u32 addr, u8 *pbuff, u32 len)
{
    s32 ret = 0;   
    u8 cmd = 0x03;
    u8 wr_buff[4]={cmd,(addr>>16)&0xff,(addr>>8)&0xff,addr & 0xff};

    if (len > 1024)
    {
        APP_DEBUG("length is too long.\r\n");
        return ;
    }
	spi_flash_cs(0);
    ret = Ql_SPI_WriteRead(USR_SPI_CHANNAL,wr_buff,4, pbuff, len);
	spi_flash_cs(1);
    if(ret==len)
    {
        u16 i=0;
        for(i=0;i<len;i++)
        {
            APP_DEBUG("%d ",pbuff[i]);
            osDelay(10);
        }
        APP_DEBUG("\r\n");
    }
    else 
    {
        APP_DEBUG("func(%s),line(%d),here has a failed operation.\r\n",__func__,__LINE__);
    }
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


static void MainUartRecvCallback(u32 event, void* dataPtr, u32 dataLen)
{
	if((USART_EVENT_RX_TIMEOUT == event) || (USART_EVENT_RECEIVE_COMPLETE == event))
	{
        Ql_UART_Write(UART_PORT0,(u8 *)dataPtr,dataLen);

        s32 ret = 0;
            
        memset(uart_read_buff, 0x00, sizeof(uart_read_buff));
        memcpy(uart_read_buff, dataPtr, dataLen);

        //read the flash's ID
        memset(buffer, 0x0, sizeof(buffer));
        sprintf(buffer, "ql_spi_rd_flash_id");
        ret = strncmp(uart_read_buff, buffer, strlen(buffer));
        if (!ret)
        {
            if (cmd_is_over())
                flash_rd_id();
            else
                APP_DEBUG("error.\r\n");
            return;
        }
        //read the flash's status register 
        memset(buffer, 0x0, sizeof(buffer));
        sprintf(buffer, "ql_spi_rd_flash_status");
        ret = strncmp(uart_read_buff, buffer, strlen(buffer));
        if (!ret)
        {                    
            flash_rd_status_reg();
            flash_rd_suspend_status_reg();
            return;
        }

        //write the flash's status register
        memset(buffer, 0x0, sizeof(buffer));
        sprintf(buffer, "ql_flash_block_lock");
        ret = strncmp(uart_read_buff, buffer, strlen(buffer));
        if (!ret)
        {
            ql_winbond_flash_block_lock();
            return;
        }

        //write the flash's status register
        memset(buffer, 0x0, sizeof(buffer));
        sprintf(buffer, "ql_flash_block_unlock");
        ret = strncmp(uart_read_buff, buffer, strlen(buffer));
        if (!ret)
        {
            ql_winbond_flash_block_unlock();               
            return;
        }
                
        //read the manufacturer ID + device ID
        memset(buffer, 0x0, sizeof(buffer));
        sprintf(buffer, "ql_spi_rd_flash_md");               
        ret = strncmp(uart_read_buff, buffer, strlen(buffer)); 
        if (!ret)
        {
            if (cmd_is_over())
                flash_rd_id2();
            else
                APP_DEBUG("error.\r\n");
            return;
        }    
        // read data from spi flash.
        memset(buffer, 0x0, sizeof(buffer));
        sprintf(buffer, "ql_spi_rd_flash_data");
        ret = strncmp(uart_read_buff, buffer, strlen(buffer));
        if (!ret)
        {
            u8 buff[255]={0};
            if (cmd_is_over())
                flash_rd_data(0,buff,255);
            else
                APP_DEBUG("error.\r\n");
            return;
        } 

        //erase the specificed sector
        memset(buffer, 0x0, sizeof(buffer));
        sprintf(buffer, "ql_spi_erase_flash_sector");
        ret = strncmp(uart_read_buff, buffer, strlen(buffer));
        if (!ret)
        {
            if (cmd_is_over())
                flash_erase_sector(0);
            else
                APP_DEBUG("error.\r\n");
            return;
        }
        //erase the specificed block
        memset(buffer, 0x0, sizeof(buffer));
        sprintf(buffer, "ql_spi_erase_flash_block");
        ret = strncmp(uart_read_buff, buffer, strlen(buffer));
        if (!ret)
        {
            if (cmd_is_over())
                flash_erase_block(0);
            else
                APP_DEBUG("error.\r\n");
            return;
        }
        //erase the whose chip 
        memset(buffer, 0x0, sizeof(buffer));
        sprintf(buffer, "ql_spi_erase_flash_chip");
        ret = strncmp(uart_read_buff, buffer, strlen(buffer));
        if (!ret)
        {
            if (cmd_is_over())
                flash_erase_chip();
            else
                APP_DEBUG("error.\r\n");
            return;
        }
       //write data to spi flash.
        memset(buffer, 0x0, sizeof(buffer));
        sprintf(buffer, "ql_spi_wr_flash_data");               
        ret = strncmp(uart_read_buff, buffer, strlen(buffer)); 
        if (!ret)
        {                       
            u8 buff[255];
            u16 i;
            for(i=0;i<256;i++)
                buff[i] = i;
            if (cmd_is_over())
                flash_wr_data(0,buff,255);
            else
                APP_DEBUG("error.\r\n");

            return;
        }                

        memset(buffer, 0x0, sizeof(buffer));
        sprintf(buffer, "Ql_SPI_Uninit");
        ret = strncmp(uart_read_buff, buffer, strlen(buffer));   
        if(0 == ret)
        {
            ret = Ql_SPI_Uninit(1);
            if( ret < 0)
            {
                APP_DEBUG("<\r\n<--Failed!! Ql_SPI_Uninit fail, ret=%d\r\n",ret);
                return;
            }
            APP_DEBUG("\r\n<-- Ql_SPI_Uninit ret =%d -->\r\n",ret);
            return;
        }
    }
}

#endif // __EXAMPLE_SPI__

