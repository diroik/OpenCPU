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
 *   example_eint.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   This example demonstrates how to program a EINT pin in QuecOpen.
 *   This example choose PINNAME_I2C_SCL\PINNAME_GPIO2\PINNAME_SPI_CS pin as EINT.
 *
 *   The "Enum_PinName" enumeration defines all the GPIO pins.
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "GLOBAL_EXPORT_FLAG += __EXAMPLE_EINT__" in makefile file. And compile the 
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
#ifdef __EXAMPLE_EINT_VCOM__
#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"
#include "ril.h"
#include "ql_gpio.h"
#include "ql_power.h"
#include "ql_dbg.h"
#include "ql_urc_register.h"
#include "ql_uart.h"
#include "ql_adc.h"
#include "ril.h"
#include <stdarg.h>
#include "ril_util.h"
#include "stdlib.h"
#include "ql_error.h"
#include "stdint.h"
#include "cmsis_os2.h"
#include "ril.h"
#include "ril_util.h"
#include "stdlib.h"
#include "ql_error.h"
// #include "ql_trace.h"
#include "ql_uart.h"
#include "ql_ps.h"
#include "ql_timer.h"
#include "ql_power.h"
#include "ql_i2c.h"
#include "ril_ctlwm2m.h"
#include "ec616.h"



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

// u32 gpioInterruptCount1 = 0;
// u32 gpioInterruptCount2 = 0;
// u32 gpioInterruptCount3 = 0;
// extern osMessageQueueId_t maintask_queue;

// Enum_PinName  eintPin1 = PINNAME_I2C_SCL;
// Enum_PinName  eintPin2 = PINNAME_GPIO2;
// Enum_PinName  eintPin3 = PINNAME_SPI_CS;


// static void EintCallback(Enum_PinName pinName)
// {	
// 	ST_MSG msg;

// 	switch(pinName)
// 	{
// 		case PINNAME_I2C_SCL:
// 		        msg.message = (u32)PINNAME_I2C_SCL;
// 		        msg.param1 = gpioInterruptCount1++;
// 				osMessageQueuePut(maintask_queue, (ST_MSG*)&msg, 0,0);
// 			break;
// 		case PINNAME_GPIO2:			
// 		        msg.message = (u32)PINNAME_GPIO2;
// 		        msg.param1 = gpioInterruptCount2++;
// 				osMessageQueuePut(maintask_queue, (ST_MSG*)&msg, 0,0);
// 			break;
// 		case PINNAME_SPI_CS:
// 		        msg.message = (u32)PINNAME_SPI_CS;
// 		        msg.param1 = gpioInterruptCount3++;
// 				osMessageQueuePut(maintask_queue, (ST_MSG*)&msg, 0,0);
// 			break;
// 		default:
// 			break;
// 	}
// }


// static void MainUartRecvCallback(u32 event, void* dataPtr, u32 dataLen)
// {
// 	if((USART_EVENT_RX_TIMEOUT == event) || (USART_EVENT_RECEIVE_COMPLETE == event))
// 	{
// 	    Ql_UART_Write(UART_PORT0,(u8 *)dataPtr,dataLen);
// 	}
// }


// void proc_main_task(void)
// {
// 	s32 ret=-1;	
// 	ST_MSG msg;
// 	Ql_SleepDisable();
// 	Ql_RIL_Initialize();
//  	Ql_UART_Open(UART_PORT0,9600,MainUartRecvCallback);
// 	maintask_queue = osMessageQueueNew(MAINTASK_QUEUE_LEN, sizeof(ST_MSG), NULL);
// 	APP_DEBUG("<-- QuecOpen: EINT Example -->\r\n");

// 	ret = Ql_EINT_Init(eintPin1 ,EINT_EDGE_RISING,50,EintCallback);
// 	APP_DEBUG("eintPin1 EINT_Init ret:%d,%d\r\n",ret,eintPin1);
// 	ret = Ql_EINT_Init(eintPin2 ,EINT_EDGE_RISING,1000,EintCallback);
// 	APP_DEBUG("eintPin2 EINT_Init ret:%d,%d\r\n",ret,eintPin2);
// 	ret = Ql_EINT_Init(eintPin3 ,EINT_EDGE_RISING,1000,EintCallback);
// 	APP_DEBUG("eintPin3 EINT_Init ret:%d,%d\r\n",ret,eintPin3);
//     while (1)
//     {
//     	if(osOK == osMessageQueueGet(maintask_queue,(void *)&msg, NULL, osWaitForever))
//     	{	
//     		switch(msg.message)
//     			{
// 				case PINNAME_I2C_SCL:
// 					APP_DEBUG("eintPin1 trigger gpioInterruptCount1:%d\r\n",gpioInterruptCount1);
// 					if(gpioInterruptCount1 > 5)
// 					{
// 						ret = Ql_EINT_Uninit(eintPin1);
// 						APP_DEBUG("eintPin1 EINT_UnInit ret:%d\r\n",ret);
// 					}
// 					break;
// 				case PINNAME_GPIO2:
// 					APP_DEBUG("eintPin2 trigger gpioInterruptCount2:%d\r\n",gpioInterruptCount2);
// 					if(gpioInterruptCount2 > 5)
// 					{
// 						ret = Ql_EINT_Uninit(eintPin2);
// 						APP_DEBUG("eintPin2 EINT_UnInit ret:%d\r\n",ret);
// 					}			
// 					break;
// 				case PINNAME_SPI_CS:
// 					APP_DEBUG("eintPin3 trigger gpioInterruptCount3:%d\r\n",gpioInterruptCount3);
// 					if(gpioInterruptCount3 > 5)
// 					{
// 						ret = Ql_EINT_Uninit(eintPin3);
// 						APP_DEBUG("eintPin3 EINT_UnInit ret:%d\r\n",ret);
// 					}			
// 					break;
// 				default:
// 					APP_DEBUG("default message:%d\r\n",msg.message);
// 					break;
//     			}
// 		}
    	
// 	}

// }

extern osMessageQueueId_t maintask_queue;

Enum_PinLevel vcom_app_vcc5v_dec_exist(void)
{
	return (Enum_PinLevel)Ql_GPIO_GetLevel(PINNAME_SPI_CS);
}

Enum_PinLevel vcom_app_shock_exist(void)
{
	return (Enum_PinLevel)Ql_GPIO_GetLevel(PINNAME_SPI_MOSI);
}

Enum_PinLevel vcom_app_triger_exist(void)
{
	return (Enum_PinLevel)Ql_GPIO_GetLevel(PINNAME_GPIO2);
}

void vcom_app_led_init(void)
{
	// Specify a GPIO pin
	Enum_PinName gpioPin = PINNAME_GPIO3;
	// Define the initial level for GPIO pin
	Enum_PinLevel gpioLvl = PINLEVEL_HIGH;
	Ql_GPIO_Init(gpioPin, PINDIRECTION_OUT, gpioLvl, PINPULLSEL_DISABLE);
}

void vcom_app_triger_pin_init(void)
{
	// Specify a GPIO pin
	Enum_PinName gpioPin = PINNAME_GPIO2;
	// Define the initial level for GPIO pin
	Enum_PinLevel gpioLvl = PINLEVEL_HIGH;
	Ql_GPIO_Init(gpioPin, PINDIRECTION_IN, gpioLvl, PINPULLSEL_PULLUP);
}

void vcom_app_bat_power_en_init(void)
{

	// Specify a GPIO pin
	Enum_PinName gpioPin = PINNAME_GPIO4;
	// Define the initial level for GPIO pin
	Enum_PinLevel gpioLvl = PINLEVEL_HIGH;
	Ql_GPIO_Init(gpioPin, PINDIRECTION_OUT, gpioLvl, PINPULLSEL_DISABLE);
}

void vcom_app_hd8020_en_init(void)
{
	// Specify a GPIO pin
	Enum_PinName gpioPin = PINNAME_GPIO5;
	// Define the initial level for GPIO pin
	Enum_PinLevel gpioLvl = PINLEVEL_LOW;
	Ql_GPIO_Init(gpioPin, PINDIRECTION_OUT, gpioLvl, PINPULLSEL_DISABLE);
}

static bool isWakeup = false;
static uint32_t powerCount = 0;
static uint32_t shock_triger = 0;
static void vcom_power_detect_event_handler(Enum_PinName pinName)
{
	ST_MSG msg;	
	isWakeup = true;
	powerCount++;
	msg.message = MSG_ID_APP_START;
	osMessageQueuePut(maintask_queue, &msg, 0, 0);
}

void vcom_power_detect_init(void)
{
	Ql_EINT_Init(PINNAME_SPI_CS, EINT_EDGE_RISING, 1000, vcom_power_detect_event_handler);
}

void vcom_app_get_bat_level(void)
{
	uint32_t adc_voltage;
	s32 ret = -1;
	ret = Ql_ADC_Open(PIN_ADC1);
    if(ret < QL_RET_OK)
	{
        APP_DEBUG("<-- open adc1 failed, ret = %d -->\r\n",ret);        
	}
	else
	{
		Ql_ADC_Read(PIN_ADC1, (u32 *)&adc_voltage); //53   gpio7  adc2  vol
		// pwr_mag.power_voltage = adc_voltage * 0.268 / 68.0;
		APP_DEBUG("<-- bat_level_counts:%d -->\r\n", adc_voltage);
		Ql_ADC_Close(PIN_ADC1);
	}
}

void vcom_app_get_bat_temp(void)
{
	uint32_t adc_counts = 0;
	s32 ret = -1;
	ret = Ql_ADC_Open(PIN_ADC0);
    if(ret < QL_RET_OK)
	{
        APP_DEBUG("<-- open adc0 failed, ret = %d -->\r\n",ret);        
	}
	else
	{
		Ql_ADC_Read(PIN_ADC0, (u32 *)&adc_counts);
		// pwr_mag.tem_vol = (adc_counts / 1000.0) * 2.8 / 1.8; //9     adc0    temp
		APP_DEBUG("<-- bat_temp_counts:%d -->\r\n", adc_counts);
		Ql_ADC_Close(PIN_ADC0);
	}
}

static void vcom_app_sensor_int_event_handler(Enum_PinName eintPinName)
{
	ST_MSG msg;	
	if (vcom_app_shock_exist() == PINLEVEL_HIGH)
	{
		shock_triger += 1;
		msg.message = MSG_ID_APP_START;
		osMessageQueuePut(maintask_queue, &msg, 0, 0);
		// start_timing(app_timer_list, SHOCK_LOW_POWER_INTERVAL, SHOCK_LOW_POWER);
	}
}

void vcom_app_shock_detect_init(void)
{
	Ql_EINT_Init(PINNAME_SPI_MOSI, EINT_EDGE_RISING, 500, vcom_app_sensor_int_event_handler);
}

#if 0
static void vcom_app_callback_uart_pn(u32 event, void* dataPtr, u32 dataLen)
{
	if((USART_EVENT_RX_TIMEOUT == event) || (USART_EVENT_RECEIVE_COMPLETE == event))
	{
	}
}
void vcom_app_init_uart_pn(void)
{
	s32 ret;
	ret = Ql_UART_Open(UART_PORT1, 115200, vcom_app_callback_uart_pn);
	if (ret < QL_RET_OK)
		APP_DEBUG("Fail to open serial port[%d], ret=%d\r\n", UART_PORT1, ret);
}
#endif

void vcom_app_deinit_uart_pn()
{
	s32 ret = Ql_UART_Close(UART_PORT1);
	if (ret < QL_RET_OK)
		APP_DEBUG("Fail to close serial port[%d], ret=%d\r\n", UART_PORT1, ret);
}

void vcom_app_bma250_i2c_init(void)
{
	s32 ret;
	ret = Ql_I2C_Init(0, PINNAME_I2C_SCL, PINNAME_I2C_SDA, 0);
	if (ret < 0)
		APP_DEBUG("\r\n<--Failed!! IIC controller Ql_IIC_Init channel 1 fail ret=%d-->\r\n", ret);

	ret = Ql_I2C_Config(0, TRUE, HAL_I2C_BPS_400K);
	if (ret < 0)
		APP_DEBUG("\r\n<--Failed !! IIC controller Ql_IIC_Config channel 1 fail ret=%d-->\r\n", ret);
}

char uart_read_buff[1024];   
static void vcom_app_callback_uart_debug(u32 event, void* dataPtr, u32 dataLen)
{
    char temp_buff[100];
    s32 ret;    
	ST_MSG msg;	
	if((event == USART_EVENT_RX_TIMEOUT) || (event == USART_EVENT_RECEIVE_COMPLETE))
	{        
            memset(uart_read_buff, 0x00, sizeof(uart_read_buff));
            memcpy(uart_read_buff, dataPtr, dataLen);
			
            memset(temp_buff, 0x0, sizeof(temp_buff));
            sprintf(temp_buff, "Get_GPIO_reg\r\n");
            ret = strncmp(uart_read_buff, temp_buff, strlen(temp_buff));
            if(0 == ret)
            {
				msg.message = MSG_ID_APP_START;
				osMessageQueuePut(maintask_queue, &msg, 0, 0);
                return;
            }
#if 0
            memset(temp_buff, 0x0, sizeof(temp_buff));
            sprintf(temp_buff, "Clear_reg\r\n");
            ret = strncmp(uart_read_buff, temp_buff, strlen(temp_buff));
            if(0 == ret)
            {
            	GPIO_0->INTSTATUS =	0;
            	GPIO_1->INTSTATUS =	0;
				APP_DEBUG("\r\n<--Clear_reg OK-->\r\n");
                return;
            }
#endif
		// Ql_UART_Write(UART_PORT0, (u8 *)dataPtr, dataLen);
		// UartSendATCmd(dataPtr, dataLen);
	}
}

void vcom_app_init_uart_debug(void)
{
	s32 ret;
	ret = Ql_UART_Open(UART_PORT0, 115200, vcom_app_callback_uart_debug);
	// ret = Ql_UART_Open(UART_PORT0, 9600, vcom_app_callback_uart_debug); //LJK
	if (ret < QL_RET_OK)
	{
		Ql_Debug_Trace("Fail to open serial port[0]\r\n");
	}
	APP_DEBUG("open serial port[%d], ret=%d\r\n", UART_PORT0, ret);
}

#define BMA250_SLAVE_ADDRESS 0x98
int hal_bma250_interface_init()
{
	vcom_app_bma250_i2c_init();
	return 0;
}

/**
* @brief  Generic Reading function. It must be fullfilled with either
*         I2C or SPI reading functions
* @param  Reg       Register Address
* @param  Data      Data Read
* @return Reserve
*/
extern s32 Ql_I2C_Write_Read_Ex(u32 chnnlNo, u8 slaveAddr, u8* pData, u32 wrtLen, u8* pBuffer, u32 rdLen); //该接口并不会开开放给所有客户，请使用extern
uint8_t BMA250_ReadReg(uint8_t Reg, uint8_t *r_val)
{
	s32 ret;
	// ret = Ql_I2C_Write_Read(1, BMA250_SLAVE_ADDRESS, &Reg, 1, r_val, 1);
	ret = Ql_I2C_Write_Read_Ex(0, BMA250_SLAVE_ADDRESS, &Reg, 1, r_val, 1);
	if (ret < 0)
		APP_DEBUG("ERROR: I2C Read Error! ret = %d\r\n", ret);
	return ret;
}

/**
* @brief  Generic Writing function. It must be fullfilled with either
*          I2C or SPI writing function
* @param  Reg       Register Address
* @param  Data      Data to be written
* @return Reserve
*/
uint8_t BMA250_WriteReg(uint8_t WriteAddr, uint8_t Data)
{
	s32 ret;
	uint8_t w_data[2];
	w_data[0] = WriteAddr;
	w_data[1] = Data;
	ret = Ql_I2C_Write(0, BMA250_SLAVE_ADDRESS, w_data, 2);
	if (ret < 0)
		APP_DEBUG("ERROR: I2C Write Error! ret = %d\r\n", ret);
	return 0;
}

int bma250_init()
{
	// uint8_t id = 0;
	volatile uint32_t i;
	hal_bma250_interface_init();
	for (i = 0; i < 10000; i++);
	BMA250_WriteReg(0x07, 0xC3);
	BMA250_WriteReg(0x20, 0x21);
	BMA250_WriteReg(0x08, 0x00);

	BMA250_WriteReg(0x06, 0x44);
	BMA250_WriteReg(0x43, 0x50); //c8
	BMA250_WriteReg(0x44, 0x00); //00
	BMA250_WriteReg(0x45, 0x64); // 0x44
	BMA250_WriteReg(0x09, 0x04);
	BMA250_WriteReg(0x4A, 0x00);
	BMA250_WriteReg(0x07, 0xC1);
	return 0;
}

void vcom_app_sys_peripheral_init(void)
{
	vcom_app_init_uart_debug();
	APP_DEBUG("<---------------------GPIO_0--------------------->\r\n");
	APP_DEBUG("%08x, %08x, %08x, %08x\r\n", GPIO_0->INTENSET,GPIO_0->INTENCLR,GPIO_0->INTTYPESET,GPIO_0->INTTYPECLR);
	APP_DEBUG("%08x, %08x, %08x\r\n", GPIO_0->INTPOLSET,GPIO_0->INTPOLCLR,GPIO_0->INTSTATUS);
	APP_DEBUG("<--GPIO_1-->\r\n");
	APP_DEBUG("%08x, %08x, %08x, %08x\r\n", GPIO_1->INTENSET,GPIO_1->INTENCLR,GPIO_1->INTTYPESET,GPIO_1->INTTYPECLR);
	APP_DEBUG("%08x, %08x, %08x\r\n", GPIO_1->INTPOLSET,GPIO_1->INTPOLCLR,GPIO_1->INTSTATUS);
	//vcom_app_led_init();
	//vcom_app_triger_pin_init();
	//vcom_app_bat_power_en_init();
	//vcom_app_hd8020_en_init();
	vcom_power_detect_init();
	vcom_app_shock_detect_init();	
	APP_DEBUG("<---------------------GPIO_0--------------------->\r\n");
	APP_DEBUG("%08x, %08x, %08x, %08x\r\n", GPIO_0->INTENSET,GPIO_0->INTENCLR,GPIO_0->INTTYPESET,GPIO_0->INTTYPECLR);
	APP_DEBUG("%08x, %08x, %08x\r\n", GPIO_0->INTPOLSET,GPIO_0->INTPOLCLR,GPIO_0->INTSTATUS);
	APP_DEBUG("<--GPIO_1-->\r\n");
	APP_DEBUG("%08x, %08x, %08x, %08x\r\n", GPIO_1->INTENSET,GPIO_1->INTENCLR,GPIO_1->INTTYPESET,GPIO_1->INTTYPECLR);
	APP_DEBUG("%08x, %08x, %08x\r\n", GPIO_1->INTPOLSET,GPIO_1->INTPOLCLR,GPIO_1->INTSTATUS);

	//bma250_init();
	Ql_RIL_Initialize();
}

static u32 GP_timer = 0x101;
static s32 m_param2 = 0;
static osMessageQueueId_t subtask_queue = NULL;

void vcom_app_timer_set(u32 interval)
{
	if (Ql_TIMER_Stop(GP_timer) == QL_RET_OK)
	{
		APP_DEBUG("LJK: -------------------------vcom set timer, interval=%d\n", interval);
		if (Ql_TIMER_Start(GP_timer, interval, TRUE) < 0)
			APP_DEBUG("ERROR: Timer_Start_error, interval=%d\n", interval);
	}
	else
		APP_DEBUG("ERROR: Timer_Stop_error, interval=%d\n", interval);
}

void Timer_handler(u32 timerId, void *param)
{
	ST_MSG msg;
	msg.message = 0;
	msg.param1 = 0;
	osMessageQueuePut(subtask_queue, &msg, 0, 0);
}

void proc_main_task(s32 taskId)
{
	s32 ret = -1;
	ST_MSG msg;
	u8 nw_state = 0;
	CT_NNMI_Param_t *ct_recv_param_ptr = NULL;
	Enum_CTLWM2M_REG_State ctlwm2m_reg_state = UNINITIALISED;
	vcom_app_sys_peripheral_init();
	ret = Ql_TIMER_RegisterFast(GP_timer, Timer_handler, &m_param2);
	if (ret < 0)
		APP_DEBUG("Timer_Register_error");
	osDelay(1000);
	APP_DEBUG("<-- vcom app start -->\r\n");
	Ql_SleepDisable();
	maintask_queue = osMessageQueueNew(MAINTASK_QUEUE_LEN, sizeof(msg), NULL);
	while (1)
	{
		if (osOK == osMessageQueueGet(maintask_queue, (void *)&msg, NULL, osWaitForever))
			switch (msg.message)
			{
			case MSG_ID_URC_INDICATION:
			{
				switch (msg.param1)
				{
				case URC_SIM_CARD_STATE_IND:
				{
				}
				break;
				case URC_EGPRS_NW_STATE_IND:
				{
					ret = Ql_GetCeregState(&nw_state);
					if ((1 == nw_state) || (5 == nw_state))
					{
						// msg.message = MSG_ID_APP_START;
						// msg.param1 = STATE_CTLWM2M_NCFG;
						// osMessageQueuePut(maintask_queue, &msg, 0, 0);
						APP_DEBUG("<-- Module has registered to network, status:%d -->\r\n", nw_state);
					}
					else
					{
						APP_DEBUG("<-- Module has deregister to network, status:%d,ret:%d -->\r\n", nw_state, ret);
					}
				}
				break;
				case URC_CTLWM2M_NCDPOPEN:
				{
					s8 QLWEVTIND_value = (s8)msg.param2;
					if (CONNECT_SUC == QLWEVTIND_value)
					{
						// bc95_info.connectStatus = 1;
						APP_DEBUG("<-- REG Successful, QLWEVTIND: %d -->\r\n", QLWEVTIND_value);
					}
					else if (CONNECTED_OBSERD == QLWEVTIND_value)
					{
						// bc95_info.observeStatus = 1;
						APP_DEBUG("<-- OBSR Successful, QLWEVTIND: %d -->\r\n", QLWEVTIND_value);
					}
					else if (CONNECT_FAIL == QLWEVTIND_value)
					{
						// bc95_info.connectStatus = -1;
						APP_DEBUG("<-- OBSR Failed, QLWEVTIND: %d -->\r\n", QLWEVTIND_value);
					}
					else
					{
						// bc95_info.observeStatus = -1;
						APP_DEBUG("<-- OBSR QLWEVTIND: %d -->\r\n", QLWEVTIND_value);
					}
					break;
				}

				case URC_CTLWM2M_SEND_CON_MSG:
				{
					s8 QLWEVTIND_value = (s8)msg.param2;
					if (MSG_SEND_SUC == QLWEVTIND_value)
					{
						// uart_info_nbio.ReAck |= DATASTATUS_RECV_BIT;
						APP_DEBUG("<-- Con MSG to the server Successful,QLWEVTIND: %d -->\r\n", QLWEVTIND_value);
					}
					else if (MSG_SEND_FAIL == QLWEVTIND_value)
					{
						// uart_info_nbio.ReAck |= CME_ERROR_RECV_BIT;
						APP_DEBUG("<-- Con MSG to the server Failed,QLWEVTIND: %d -->\r\n", QLWEVTIND_value);
					}
					break;
				}

				case URC_CTLWM2M_RECOV_IND:
				{
					APP_DEBUG("<-- State timer start after wakeup,ret = %d -->\r\n", ret);

					s8 QLWEVTIND_value = (s8)msg.param2;
					if (RECOV_SUC_FROM_SLEEP == QLWEVTIND_value)
					{
						// msg.message = MSG_ID_APP_START;
						// msg.param1 = STATE_CTLWM2M_NMGS;
						// osMessageQueuePut(maintask_queue, &msg, 0, 0);
						APP_DEBUG("<-- Recover Successful From Deep Sleep,QLWEVTIND: %d -->\r\n", QLWEVTIND_value);
					}
					else if (RECOV_FAIL_FROM_SLEEP == QLWEVTIND_value)
					{
						RIL_CTLWM2M_GET_NMSTATUS((s8 *)&ctlwm2m_reg_state);
						APP_DEBUG("<-- Recover Fail From Deep Sleep,QLWEVTIND: %d -->\r\n", QLWEVTIND_value);
					}
					break;
				}

				case URC_CTLWM2M_FOTAOBSE_IND:
				{
					s8 QLWEVTIND_value = (s8)msg.param2;
					if (OBSV_FOTA_OBJ == QLWEVTIND_value)
					{
						APP_DEBUG("<-- FOTA Obj5/0/3 Obsr,QLWEVTIND: %d -->\r\n", QLWEVTIND_value);
						// vcom_app_powermode_set(SYS_POWER_STATE_FOTA);
						// start_timing(app_timer_list, 60 * 10, POWER_TIME_OUT);
					}
					else if (DISOBSV_FOTA_OBJ == QLWEVTIND_value)
					{
						APP_DEBUG("<-- FOTA Obj5/0/3 Disobsr,QLWEVTIND: %d -->\r\n", QLWEVTIND_value);
						// vcom_sys_reset();
					}
					break;
				}
				case URC_CTLWM2M_RECV_DATA:
				{
					ct_recv_param_ptr = (CT_NNMI_Param_t *)msg.param2;

					if (BUFFER_MODE == ct_recv_param_ptr->mode) //buffer mode
					{
						// ret = RIL_CTLWM2M_NMGR(ct_recv_param_ptr);
						// APP_DEBUG("RECV DATA by DIRECT_PUSH_MODE,buflen:%d,buffer:", ct_recv_param_ptr->buflen);
						// Ql_UART_Write(UART_PORT0, ct_recv_param_ptr->buffer, ct_recv_param_ptr->buflen);
					}
					else if (DIRECT_PUSH_MODE == ct_recv_param_ptr->mode) //Direct push mode
					{
						// for (size_t i = 0, j = 0; i < ct_recv_param_ptr->buflen; i += 2, j++)
						// 	bc95_info.recvData[j] = (char2hex(ct_recv_param_ptr->buffer[i]) << 4) | char2hex(ct_recv_param_ptr->buffer[i + 1]);
						APP_DEBUG("RECV: %d, %s", ct_recv_param_ptr->buflen, ct_recv_param_ptr->buffer);
						// nb_recv_payload_parase(bc95_info.recvData, ct_recv_param_ptr->buflen / 2);
						// Ql_UART_Write(UART_PORT0, ct_recv_param_ptr->buffer, ct_recv_param_ptr->buflen);
					}
					break;
				}
				case URC_CTLWM2M_RECV_RST:
				{
					s8 QLWEVTIND_value = (s8)msg.param2;
					RIL_CTLWM2M_GET_NMSTATUS((s8 *)&ctlwm2m_reg_state);
					// msg.message = MSG_ID_APP_START;
					// msg.param1 = STATE_CTLWM2M_NCDPCLOSE;
					// osMessageQueuePut(maintask_queue, &msg, 0, 0);
					APP_DEBUG("<-- RECV RST MSG,QLWEVTIND: %d -->\r\n", QLWEVTIND_value);
					break;
				}

				default:
					APP_DEBUG("<-- Other URC: type = %d\r\n", msg.param1);
					break;
				}
			}
			break;
			 case MSG_ID_APP_START:
			 {
				APP_DEBUG("<-- APP_START SPI_CS = %d,SPI_MOSI = %d\r\n", powerCount,shock_triger);
				APP_DEBUG("<--------------GPIO_0 SPI_MOSI:1-------------->\r\n");
				APP_DEBUG("%08x, %08x, %08x, %08x\r\n", GPIO_0->INTENSET,GPIO_0->INTENCLR,GPIO_0->INTTYPESET,GPIO_0->INTTYPECLR);
				APP_DEBUG("%08x, %08x, %08x\r\n", GPIO_0->INTPOLSET,GPIO_0->INTPOLCLR,GPIO_0->INTSTATUS);
				APP_DEBUG("<--GPIO_1 SPI_CS:3-->\r\n");
				APP_DEBUG("%08x, %08x, %08x, %08x\r\n", GPIO_1->INTENSET,GPIO_1->INTENCLR,GPIO_1->INTTYPESET,GPIO_1->INTTYPECLR);
				APP_DEBUG("%08x, %08x, %08x\r\n", GPIO_1->INTPOLSET,GPIO_1->INTPOLCLR,GPIO_1->INTSTATUS);
			 }
			 break;
			default:
				break;
			}
		osDelay(10);
	}
}

void vcc_judge(uint8_t duration)
{
	static int count = 0;
	if (isWakeup == true)
	{
		APP_DEBUG("INFO: VCC Change PIN %d, count=%X\n", vcom_app_vcc5v_dec_exist(), powerCount);
		isWakeup = false;
	}
	if (shock_triger == 1)
	{
		APP_DEBUG("INFO: shock happen\n");
		shock_triger = 0;
	}
	if (count % 50 == 0)
	{
		APP_DEBUG("INFO: heart\n");
	}
	count++;
}

void proc_subtask1(s32 taskId) //proc_main_task
{
	osDelay(500);
	ST_MSG msg;
	vcom_app_timer_set(100);
	Ql_SleepDisable();
	subtask_queue = osMessageQueueNew(MAINTASK_QUEUE_LEN, sizeof(msg), NULL);
	while (TRUE)
	{
		if (osOK == osMessageQueueGet(subtask_queue, (void *)&msg, NULL, osWaitForever))
			vcc_judge(5);
		osDelay(10);
	}
}

#endif

