/**************************************************************************//**
 * @file
 * @brief
 * @version  V1.0.0
 * @date
 ******************************************************************************/
/*
 *
 *
 */
#include <stdlib.h>
#include <string.h>
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core
#include "event_groups.h"               // ARM.FreeRTOS::RTOS:Event Groups
#include "semphr.h"                     // ARM.FreeRTOS::RTOS:Core
#include "bsp.h"
#include "Driver_USART.h"
#include "debug_log.h"
#include "ostask.h"
#include "os_exception.h"
#include "os_callback_hook.h"

typedef enum OsHookTaskIdT  /* tasks sent to identified by their task ids */
{
    IDLE_TASK_ID=64,
    TIME_TASK_ID,
    APP_TASK_ID,
    DEBUG_TASK_ID,
    TCPIP_TASK_ID,
    CTIOT_TASK_SEND_ID,
    CTIOT_TASK_RECV_ID,
    CTIOT_TASK_FOTA_DOWNLOAD_ID,
    CTIOT_TASK_FOTA_EXE_ID,
    CTIOT_TASK_COAP_LOOP_ID,
    CTIOT_TASK_COAP_INIT_ID,
    MQTT_RECV_TASK_ID,
    MQTT_SEND_TASK_ID,
    COAP_RECV_TASK_ID,
    COAP_SEND_TASK_ID,
    ATCMD_TASK_ID,
    CTIOT_TASK_ID,
    AUTOREG_TASK_ID,
    HTTP_SEND_TASK_ID,
    HTTP_RECV_TASK_ID,
    UNILOG_TASK_ID,
    SIMBIP_TASK_ID,
    MAIN_TASK_ID,
    LWM2M_TASK_ID,
    SOCKET_TASK_ID,
    CIS_RECV_TASK_ID,
    TEST_CMIOT_OTA_TASK_ID,
    CIS_SAMPLE_THREAD_TASK_ID,
    CTLWM2M_MAIN_TASK_ID,
    CTLWM2M_RECV_TASK_ID,
    OTA_SAMPLE_TASK_ID,
    ONENET_TASK_ID,
    APPENGINE_TASK_ID,
    CMCC_DM_TASK_ID,
    CTCC_DM_TASK_ID,
    CUCC_DM_TASK_ID,
    DM_CHECK_TASK_ID,
    ABUP_FOTA_TASK_ID, //abup_fota
    IPERF_C_TASK_ID,   //iperf_client_thread
    IPERF_S_TASK_ID,   //iperf_server_thread
    PING_TASK_ID,      //ping_thread
    SLIPIF_TASK_ID,    //slipif_loop
    SNMP_TASK_ID,      //snmp_netconn
    ABUP_TIMER_TASK_ID,
    UART_SEND_TASK_ID,
    UART_RECV_TASK_ID,
    UNKNOWN_TASK_ID = 255,
    MAX_TASK_ID=256
} OsHookTaskId;

task_node_list ec_task_list[TASK_LIST_LEN] = {0};
int curr_task_numb = 0;

char over_buf[64];

void vApplicationStackOverflowHook (TaskHandle_t xTask, signed char *pcTaskName)
{
    QCOMM_STRING(UNILOG_PLA_STRING, StackOverflow, P_ERROR, "\r\n!!!error!!!..task:%s..stack.over.flow!!!\r\n", (uint8_t *)pcTaskName);
    sprintf(over_buf, "\r\n!!!error!!!..task:%s..stack.over.flow!!!\r\n",pcTaskName);

    if (UsartPrintHandle != NULL)
    {
        UsartPrintHandle->Send(over_buf, 64);
    }
    EC_ASSERT(0,0,0,0);
}

void ecTraceTaskSwitchOut(void)
{
#if(INCLUDE_xTaskGetCurrentTaskHandle == 1)
    extern uint8_t PhyLogLevelGet(void);
    if(PhyLogLevelGet() >= 1)
    {
        uint32_t task_numb = 0;
        // 7-----If "UNILOG_PHY_SCHEDULE_MODULE" changed should adjust this value accordingly.
        // 0x41----If "PHY_SCHD_LOG_OSTASK_INFO" changed should adjust this value accordingly.
        UINT32 swLogID = UNILOG_ID_CONSTRUCT(UNILOG_PHY_LOG, 7, 0x41);

        task_numb = uxTaskGetTaskNumber(xTaskGetCurrentTaskHandle());
        task_numb = (0xF1U<<24)|task_numb;
        swLogMsgDump(swLogID, (uint32_t *)&task_numb, 1);
    }
#endif
}

void ecTraceTaskSwitchIn(void)
{
#if(INCLUDE_xTaskGetCurrentTaskHandle == 1)
    const char * task_name;
    uint32_t task_numb = 0;
    // 7-----If "UNILOG_PHY_SCHEDULE_MODULE" changed should adjust this value accordingly.
    // 0x41----If "PHY_SCHD_LOG_OSTASK_INFO" changed should adjust this value accordingly.
    UINT32 swLogID = UNILOG_ID_CONSTRUCT(UNILOG_PHY_LOG, 7, 0x41);

    task_name = pcTaskGetTaskName(xTaskGetCurrentTaskHandle());
    task_numb = uxTaskGetTaskNumber(xTaskGetCurrentTaskHandle());
    //ec_task_list[curr_task_numb].ec_task_tcb_numb = task_numb;

    task_numb = (0xF0U<<24)|task_numb;
    swLogMsgDump(swLogID, (uint32_t *)&task_numb, 1);

    //snprintf(ec_task_list[curr_task_numb].ec_task_name, 8, "%s\r",task_name);

    memcpy(ec_task_list[curr_task_numb].ec_task_name, task_name, 7);
    ec_task_list[curr_task_numb].ec_task_in_time = xTaskGetTickCount();
    //ec_task_list[curr_task_numb].ec_task_index = curr_task_numb;
    curr_task_numb++;

    if(curr_task_numb == TASK_LIST_LEN)
    {
        curr_task_numb = 0;
    }

#endif
}

//void ecTraceTaskCreate(TCB_t *pxNewTCB);
void ecTraceTaskCreate(StaticTask_t *pxTCB)
{
#if(INCLUDE_xTaskGetCurrentTaskHandle == 1)
    const char * task_name;

    task_name =(char *)&(pxTCB->ucDummy7[ 0 ]); //pcTaskGetTaskName(pxTCB);

    if(strcmp(task_name, "CcmTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CCM_TASK_ID);
    }
    else if(strcmp(task_name, "CemmTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CEMM_TASK_ID);
    }
    else if(strcmp(task_name, "CerrcTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CERRC_TASK_ID);
    }
    else if(strcmp(task_name, "CeupTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CEUP_TASK_ID);
    }
    else if(strcmp(task_name, "UiccCtrlTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, UICC_CTRL_TASK_ID);
    }
    else if(strcmp(task_name, "UiccDrvTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, UICC_DRV_TASK_ID);
    }
    else if(strcmp(task_name, "PhyHighTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, PHY_HIGH_TASK_ID);
    }
    else if(strcmp(task_name, "PhyMidTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, PHY_MID_TASK_ID);
    }
    else if(strcmp(task_name, "PhyLowTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, PHY_LOW_TASK_ID);
    }
    else if(strcmp(task_name, "PsProxyTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CMS_TASK_ID);
    }
    else if(strcmp(task_name, "IDLE") == 0)
    {
        vTaskSetTaskNumber(pxTCB, IDLE_TASK_ID);
    }
    else if(strcmp(task_name, "Tmr Svc") == 0)
    {
        vTaskSetTaskNumber(pxTCB, TIME_TASK_ID);
    }
    else if(strcmp(task_name, "app") == 0)
    {
        vTaskSetTaskNumber(pxTCB, APP_TASK_ID);
    }
    else if(strcmp(task_name, "debug") == 0)
    {
        vTaskSetTaskNumber(pxTCB, DEBUG_TASK_ID);
    }
    else if(strcmp(task_name, "tcpip_thread") == 0)
    {
        vTaskSetTaskNumber(pxTCB, TCPIP_TASK_ID);
    }
    else if(strcmp(task_name, "CTSEND") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CTIOT_TASK_SEND_ID);
    }
    else if(strcmp(task_name, "CTRECV") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CTIOT_TASK_RECV_ID);
    }
    else if(strcmp(task_name, "CTOTAD") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CTIOT_TASK_FOTA_DOWNLOAD_ID);
    }
    else if(strcmp(task_name, "CTOTAE") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CTIOT_TASK_FOTA_EXE_ID);
    }
    else if(strcmp(task_name, "CTCOAP") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CTIOT_TASK_COAP_LOOP_ID);
    }
    else if(strcmp(task_name, "CTINIT") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CTIOT_TASK_COAP_INIT_ID);
    }
    else if(strcmp(task_name, "mqttRecv") == 0)
    {
        vTaskSetTaskNumber(pxTCB, MQTT_RECV_TASK_ID);
    }
    else if(strcmp(task_name, "mqttSend") == 0)
    {
        vTaskSetTaskNumber(pxTCB, MQTT_SEND_TASK_ID);
    }
    else if(strcmp(task_name, "coapRecv") == 0)
    {
        vTaskSetTaskNumber(pxTCB, COAP_RECV_TASK_ID);
    }
    else if(strcmp(task_name, "coapSend") == 0)
    {
        vTaskSetTaskNumber(pxTCB, COAP_SEND_TASK_ID);
    }
    else if(strcmp(task_name, "atcmd") == 0)
    {
        vTaskSetTaskNumber(pxTCB, ATCMD_TASK_ID);
    }
    else if(strcmp(task_name, "ctiot") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CTIOT_TASK_ID);
    }
    else if(strcmp(task_name, "autoReg") == 0)
    {
        vTaskSetTaskNumber(pxTCB, AUTOREG_TASK_ID);
    }
    else if(strcmp(task_name, "httpRecv") == 0)
    {
        vTaskSetTaskNumber(pxTCB, HTTP_RECV_TASK_ID);
    }
    else if(strcmp(task_name, "httpSend") == 0)
    {
        vTaskSetTaskNumber(pxTCB, HTTP_SEND_TASK_ID);
    }
    else if(strcmp(task_name, "unilog") == 0)
    {
        vTaskSetTaskNumber(pxTCB, UNILOG_TASK_ID);
    }
    else if(strcmp(task_name, "SimBipTak") == 0)
    {
        vTaskSetTaskNumber(pxTCB, SIMBIP_TASK_ID);
    }
    else if(strcmp(task_name, "mainTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, MAIN_TASK_ID);
    }
    else if(strcmp(task_name, "lwm2mloop") == 0)
    {
        vTaskSetTaskNumber(pxTCB, LWM2M_TASK_ID);
    }
    else if(strcmp(task_name, "socket_atcmd") == 0)
    {
        vTaskSetTaskNumber(pxTCB, SOCKET_TASK_ID);
    }
    else if(strcmp(task_name, "cis_recv") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CIS_RECV_TASK_ID);
    }
    else if(strcmp(task_name, "testCmiotOta") == 0)
    {
        vTaskSetTaskNumber(pxTCB, TEST_CMIOT_OTA_TASK_ID);
    }
    else if(strcmp(task_name, "cis_sample_thread") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CIS_SAMPLE_THREAD_TASK_ID);
    }
    else if(strcmp(task_name, "ctlwm2m_main") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CTLWM2M_MAIN_TASK_ID);
    }
    else if(strcmp(task_name, "ctlwm2m_recv") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CTLWM2M_RECV_TASK_ID);
    }
    else if(strcmp(task_name, "ota_sample_thread") == 0)
    {
        vTaskSetTaskNumber(pxTCB, OTA_SAMPLE_TASK_ID);
    }
    else if(strcmp(task_name, "onenet_task") == 0)
    {
        vTaskSetTaskNumber(pxTCB, ONENET_TASK_ID);
    }
    else if(strcmp(task_name, "appEngine") == 0)
    {
        vTaskSetTaskNumber(pxTCB, APPENGINE_TASK_ID);
    }
    else if(strcmp(task_name, "cmccAutoReg") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CMCC_DM_TASK_ID);
    }
    else if(strcmp(task_name, "ctccAutoReg") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CTCC_DM_TASK_ID);
    }
    else if(strcmp(task_name, "cuccAutoReg") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CUCC_DM_TASK_ID);
    }
    else if(strcmp(task_name, "autoregCheck") == 0)
    {
        vTaskSetTaskNumber(pxTCB, DM_CHECK_TASK_ID);
    }
    else if(strcmp(task_name, "abup_fota") == 0)
    {
        vTaskSetTaskNumber(pxTCB, ABUP_FOTA_TASK_ID);
    }
    else if(strcmp(task_name, "iperf_client_thread") == 0)
    {
        vTaskSetTaskNumber(pxTCB, IPERF_C_TASK_ID);
    }
    else if(strcmp(task_name, "iperf_server_thread") == 0)
    {
        vTaskSetTaskNumber(pxTCB, IPERF_S_TASK_ID);
    }
    else if(strcmp(task_name, "ping_thread") == 0)
    {
        vTaskSetTaskNumber(pxTCB, PING_TASK_ID);
    }
    else if(strcmp(task_name, "slipif_loop") == 0)
    {
        vTaskSetTaskNumber(pxTCB, SLIPIF_TASK_ID);
    }
    else if(strcmp(task_name, "snmp_netconn") == 0)
    {
        vTaskSetTaskNumber(pxTCB, SNMP_TASK_ID);
    }
    else if(strcmp(task_name, "abup_timer") == 0)
    {
        vTaskSetTaskNumber(pxTCB, ABUP_TIMER_TASK_ID);
    }
    else if(strcmp(task_name, "uartsend") == 0)
    {
        vTaskSetTaskNumber(pxTCB, UART_SEND_TASK_ID);
    }
    else if(strcmp(task_name, "uartrecv") == 0)
    {
        vTaskSetTaskNumber(pxTCB, UART_RECV_TASK_ID);
    }
    else
    {
        vTaskSetTaskNumber(pxTCB, UNKNOWN_TASK_ID);
    }
#endif

}













