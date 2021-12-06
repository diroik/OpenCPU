/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Quectel Co., Ltd. 2021
*
*****************************************************************************/

/*------------------------------------------------------------------------------------------------------------
|		    Task Name     |Stack Size (Bytes) |       Task Priority     |   Task Entry Function | Task Init |
*-------------------------------------------------------------------------------------------------------------*/

#include "cmsis_os2.h"

TASK_ITEM("main_task",     
/*****************************************************************************
*  Cause the interfaces of FS and RTC use the underlying file system interface, 
*  while the FS interface requires a  large task stack of about 600 bytes. 
*  For the sake of safety, customers can use the original FreeRTOS interface 
*  uxTaskGetStackHighWaterMark to monitor the task stack of calling FS and RTC interface. 
*  The official recommendation of FreeRTOS is to reserve twice the maximum task stack.
*****************************************************************************/
#if defined(__EXAMPLE_FILESYSTEM__) || defined(__EXAMPLE_RTC__)
2048	
#else
1024
#endif
,              osPriorityNone,      proc_main_task,         NULL)

#ifdef __EXAMPLE_MULTITASK__
TASK_ITEM("sub_task1",			512,			   osPriorityNone,		sub_task1,		        NULL)
TASK_ITEM("sub_task2",			512,			   osPriorityNone,		sub_task2,	            NULL)
#endif
