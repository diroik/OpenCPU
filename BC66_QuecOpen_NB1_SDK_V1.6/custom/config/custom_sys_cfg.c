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
 *   custom_sys_cfg.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   The file intends for the system definitions of application. 
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
#include "ril.h"
#include "ql_common.h"

/*========================================================================
| 
| Customized configurations.
|------------------
| Define the working modes of power key pin and 
| specify the GPIO for watchdog (MTK2625 supports internal hardware watchdog).
\=========================================================================*/

/************************************************************************/
/* Configure the enable of application.                                                */
/* The possible values are 'APP_DISABLE' and 'APP_ENABLE', and the       */
/* default value is 'APP_ENABLE'.                                                        */
/* APP_ENABLE: the application will work after downloading into module  */
/* APP_DISABLE:the application will not work after downloading.             */
/*     In this  case, the module still work as a STANDARD module.            */
/************************************************************************/
static const ST_AppEnable appEnableCfg = {
    APP_ENABLE
};

static const ST_AppVerCfg appvercfg = {
	"QuecOpen_NB1_SDK_V1.6\0"
};

/************************************************************************/
/* Define the GPIO pin for external watchdog.                           */
/* NOTES:                                                                            */
/*     Customer may specify two GPIOs if needed.     
/* There is a hardware watchdog inside the module, so customers do not need to add an external watchdog.  */
/************************************************************************/
static const ST_ExtWatchdogCfg wtdCfg = {
		PINNAME_RI,  // Specify a pin which connects to the external watchdog
    PINNAME_END,   // Specify another pin for watchdog if needed
};
ST_ExtWatchdogCfg* Ql_WTD_GetWDIPinCfg(void)
{
    return (ST_ExtWatchdogCfg*)(&wtdCfg);
}

/************************************************************************/
/* Define the working mode of serial  port                    */
/*                                                                           */
/* The serial  port (UART0/UART1/UART2) may work as a common serial port, as   */
/* well as work as a special debug port that can debug some issues                     */
/************************************************************************/
static const ST_DebugPortCfg debugPortCfg = {
     BASIC_MODE      // Set the serial  port  to a common serial port
     //ADVANCE_MODE   // Set the serial  port to a special debug port
};

static const ST_DebugPortSet debugPortSet = {
    PORTNAME_UART2,      // Set the serial debug port (UART2) to GKI log .Can choose UART1/UART2/USB1/USB2
    //PORTNAME_UART1       // Set the serial debug port (UART1) to HSL log .Can choose UART1/UART2/USB1/USB2
};

const ST_SystemConfig SystemCfg[] = {
    {SYS_CONFIG_APP_ENABLE_ID,      SYS_CONFIG_APPENABLE_DATA_SIZE, (void*)&appEnableCfg},
    {SYS_CONFIG_WATCHDOG_DATA_ID,   SYS_CONFIG_WATCHDOG_DATA_SIZE,  (void*)&wtdCfg      },
    {SYS_CONFIG_DEBUG_MODE_ID,      SYS_CONFIG_DEBUGMODE_DATA_SIZE, (void*)&debugPortCfg},
    {SYS_CONFIG_DEBUG_SET_ID,       SYS_CONFIG_DEBUGSET_DATA_SIZE,  (void*)&debugPortSet},
	{SYS_CONFIG_APP_VERSION_ID, 	SYS_CONFIG_APPVERSION_DATA_SIZE,(void*)&appvercfg	},
    {SYS_CONFIG_END,                0,                               NULL               }
};

