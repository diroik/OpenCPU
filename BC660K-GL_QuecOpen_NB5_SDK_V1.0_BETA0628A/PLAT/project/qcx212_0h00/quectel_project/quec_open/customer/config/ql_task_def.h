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
 *   ql_task_def.h
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   TASK_DEF API defines.
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
#ifndef __QL_TASK_DEF_H__
#define __QL_TASK_DEF_H__
#include "cmsis_os2.h"

#define  MAX_CUSTOM_TASK_NUM   10

typedef void (*customCreateQueueFunc_t)( void );

typedef struct
{
    osThreadAttr_t             attr;
    osThreadFunc_t             func;
    uint32_t                   *task_handle;
    customCreateQueueFunc_t    custom_create_queue_fp;
} custom_task_definition_t;

void* ql_get_task_enter(void);

#endif //__QL_TASK_DEF_H__


#if (defined(TASK_FUNC_DECLARATION) && defined(TASK_ID_DEF))
#error [ Conflict I ]
#endif
#if (defined(TASK_FUNC_DECLARATION) && defined(TASK_DEFINITION))
#error [ Conflict II ]
#endif
#if (defined(TASK_DEFINITION) && defined(TASK_ID_DEF))
#error [ Conflict III ]
#endif


#undef TASK_DEFINITION_BEGIN
#undef TASK_ITEM
#undef TASK_DEFINITION_END


#if defined( TASK_DEFINITION)
#define TASK_DEFINITION_BEGIN  custom_task_definition_t custom_tasks[] = {
#define TASK_ITEM(task_name, stack_size, task_prio, task_entry, task_init)  {{task_name, 0, NULL, 0, NULL, stack_size, task_prio, 0, 0}, (osThreadFunc_t)task_entry, NULL, task_init},
#define TASK_DEFINITION_END {{"END", 0, NULL, 0, NULL, 0, 0, 0, 0}, (osThreadFunc_t)NULL, NULL, NULL},};
#elif defined(TASK_ID_DEF)
#define TASK_DEFINITION_BEGIN typedef enum{
#define TASK_ITEM(task_name, stack_size, task_prio, task_entry, task_init)  task_prio,
#define TASK_DEFINITION_END  TaskId_End } Enum_TaskId;
#elif defined(TASK_FUNC_DECLARATION)
#define TASK_DEFINITION_BEGIN
#define TASK_ITEM(task_name, stack_size, task_prio, task_entry, task_init)   extern void task_entry(void*);
#define TASK_DEFINITION_END
#else
#undef TASK_DEFINITION_BEGIN
#undef TASK_ITEM
#undef TASK_DEFINITION_END
#endif

