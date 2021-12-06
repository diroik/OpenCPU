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
 *   ql_uart.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   UART API defines.
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

#ifndef __QL_ERROR_H__
#define __QL_ERROR_H__

/****************************************************************************
 * Error Code Definition
 ***************************************************************************/
typedef enum {

/* 1 ~ 20 general */    

    QL_RET_OK                       =  0,           // execute success
    QL_RET_ERR_ERROR                = -1,           // execute error
    QL_RET_ERR_PARAM                = -2,           // Invalid parameter(null pointer/!=0)
    QL_RET_ERR_FULL                 = -3,           // buff full (maximizing)
    QL_RET_ERR_NOSUPPORT            = -4,           // unsupported features
    QL_RET_ERR_TIMEOUT              = -5,           // execute timeout
    QL_RET_ERR_BUSY                 = -6,           // busy
	QL_RET_ERR_INVALID_OP			= -7,			// invalid option 
    QL_RET_ERR_NOSUPPORTSET      	= -8,			// not support setting 
    QL_RET_ERR_USED                 = -9,           // used
    
/* 11 ~ 200   ALL Peripheral */
    /* 10 ~ 15   GPIO Peripheral  */			
    QL_RET_ERR_NOSUPPORTPIN      	= -10,			// pin and bus error
    QL_RET_ERR_NOSUPPORTEINT      	= -11,			// eint pin and bus error
    QL_RET_ERR_PINALREADYSUBCRIBE   = -12,			// pin already sub
    QL_RET_ERR_ALREADYUNSUBCRIBE    = -13,			// pin already unsub


    /* 16 ~ 20   UART Peripheral  */    
    QL_RET_ERR_UART_BUSY 			= -16,			// uart init error 
    QL_RET_ERR_INVALID_PORT 		= -17,			// uart port error
    QL_RET_ERR_BAUDRATE		 		= -18,			// uart baudrate error
    QL_RET_ERR_OVER_LEN		 		= -19,			// uart baudrate error


    /* 21 ~ 35   I2C Peripheral  */
    QL_RET_ERR_I2C_NOT_CHAN         = -21,          // unsupported channel        
    QL_RET_ERR_I2C_CHAN_USED        = -22,          // The channel is already in use
    QL_RET_ERR_I2C_COUNT_FULL       = -23,          // Exceeding the maximum total
    QL_RET_ERR_I2C_INVALID_PIN      = -24,          // Invalid pins
    QL_RET_ERR_I2C_HW_FAILED        = -25,          // Hardware error
    QL_RET_ERR_I2C_HW_INIT_FAILED   = -26,          // Hardware I2C initialization failed
    QL_RET_ERR_I2C_NOSUPPORT_SPEND  = -27,          // Unsupported configuration param
    QL_RET_ERR_I2C_CHAN_NOT_FOUND   = -28,          
    
    /* 36 ~ 45   ADC Peripheral */
    Ql_RET_ERR_ADC_CONVER_STOP      = -36,          // ADC channel conversion stop
    Ql_RET_ERR_ADC_CONVER_TIMEOUT   = -37,          // ADC channel conversion timeout
    Ql_RET_ERR_ADC_NOT_OPEN         = -38,          // The ADC channel is not open 
    Ql_RET_ERR_ADC_ONGOING          = -39,          // The ADC previous conversion is ongoing
    

    /* 46 ~ 60   SPI Peripheral */
    QL_RET_ERR_SPI_INVALID_PIN      = -46,          // Invalid pins
    QL_RET_ERR_SPI_HW_INIT_FAILED   = -47,          // Hardware initialization failed
    QL_RET_ERR_SPI_ERROR            = -48,          // Unspecified error
    QL_RET_ERR_SPI_BUSY             = -49,          // Driver is busy
    QL_RET_ERR_SPI_TIMEOUT          = -50,          // Timeout occurred
    QL_RET_ERR_SPI_UNSUPPORTED      = -51,          // Operation not supported
    QL_RET_ERR_SPI_SPECIFIC         = -52,          // Hardware initialization failed
    QL_RET_ERR_SPI_NOT_CHAN         = -53,          // unsupported channel

    /* 61 ~ 70   RTC Peripheral */
    QL_RET_ERR_RTC_COUNT_FULL       = -61,          // Exceeding the maximum total
    QL_RET_ERR_RTC_RUNNING          = -62,          // RTC is running
    QL_RET_ERR_RTC_NOTRUNNING       = -63,          // RTC Is not running
    QL_RET_ERR_RTC_REGISTERED       = -64,          // The RTC is registered

	/* 71 ~ 80	 TIMER Peripheral */
	QL_RET_ERR_INVALID_TIMER		= -71,			// Timer is already being used or the timer is started or stopped.
	QL_RET_ERR_TIMER_SET_CLOCK		= -72,			// Set Timer Internal clock error
	QL_RET_ERR_TIMER_FULL			= -73,			// Timer full

	/* 81 ~ 90	 DATETIME Peripheral */
	QL_RET_ERR_GET_DATETIME 		= -81,			// Get current datetime error
	QL_RET_ERR_SET_DATETIME 		= -82,			// Set current datetime error
	QL_RET_ERR_GET_DATETIME_SEC 	= -83,			// Get current datetime of seconds error
	QL_RET_ERR_GET_DATETIME_MS		= -84,			// Get current datetime of milliseconds error


	/* 91 ~ 100   FILE ERROR */
    QL_RET_ERR_FILEFAILED      		= -91,
    QL_RET_ERR_FILE_STATE			= -92,
    QL_RET_ERR_FILEREADFAILED 		= -93,
    QL_RET_ERR_FILEWRITEFAILED  	= -94,
    QL_RET_ERR_FILESEEKFAILED  		= -95,
    QL_RET_ERR_FILENOTFOUND  		= -96,
/* 201 ~ 500  For customer use */



}QL_RET;


#endif // End-of QL_ERROR_H 

