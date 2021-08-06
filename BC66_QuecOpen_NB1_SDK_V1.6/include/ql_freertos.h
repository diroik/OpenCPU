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
 *   ql_freertos.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *  System related APIs
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
#ifndef __QL_FREERTOS_H__
#define __QL_FREERTOS_H__



/**************************************************************
 * Message Structure Definition
 **************************************************************/
typedef struct {
    u32  message;
    u32  param1;
    u32  param2;
    u32  srcTaskId;
} ST_MSG;

typedef enum {
	EVENT_FLAG0 = 0x00000001,
	EVENT_FLAG1 = 0x00000002,
	EVENT_FLAG2 = 0x00000004,
	EVENT_FLAG3 = 0x00000008,
	EVENT_FLAG4 = 0x00000010,
	EVENT_FLAG5 = 0x00000020,
	EVENT_FLAG6 = 0x00000040,
	EVENT_FLAG7 = 0x00000080,
	EVENT_FLAG8 = 0x00000100,
	EVENT_FLAG9 = 0x00000200,
	EVENT_FLAG_END
}Enum_EventFlag;

typedef void * TaskHandle_t;

/*****************************************************************
* Function:     Ql_OS_GetMessage 
* 
* Description:
*               This function retrieves a message from the current task's 
*               message queue. When there is no message in the task's
*               message queue, the task is in the waiting state..
* Parameters:
*               msg: 
*                   A pointer to ST_MSG.
* Return:        
*               Always return QL_RET_OK.
*****************************************************************/
s32 Ql_OS_GetMessage(ST_MSG* msg);

/*****************************************************************
* Function:     Ql_OS_SendMessage 
* 
* Description:
*               Send message between tasks.

* Parameters:
*               destTaskId: 
*                   One value of Enum_TaskId.
*               msgId: 
*                   User message id, must bigger than 0xFF.
*               param1:
*               param2:
*                   Parameters to send to another task or current task.
* Return:        
*               OS_SUCCESS, sent data to the target task successfully.
*               OS_INVALID_ID, the task id is invalid.
*               OS_NOT_INITIALIZED, the target task is invalid.
*               OS_Q_FULL, the message queue of target task is full.
*****************************************************************/
s32 Ql_OS_SendMessage(s32 destTaskId, u32 msgId, u32 param1, u32 param2);

/*****************************************************************
* Function:     Ql_OS_SendMessageFromISR 
* 
* Description:
*               Send message between tasks,It Supports sending messages in ISR(except callback of psm_eint).

* Parameters:
*               destTaskId: 
*                   One value of Enum_TaskId.
*               msgId: 
*                   User message id, must bigger than 0xFF.
*               param1:
*               param2:
*                   Parameters to send to another task.
* Return:        
*               OS_SUCCESS, sent data to the target task successfully.
*               OS_INVALID_ID, the task id is invalid.
*               OS_NOT_INITIALIZED, the target task is invalid.
*               OS_Q_FULL, the message queue of target task is full.
*****************************************************************/
s32 Ql_OS_SendMessageFromISR(s32 destTaskId, u32 msgId, u32 param1, u32 param2);

/*****************************************************************
* Function:     Ql_OS_CreateMutex 
* 
* Description:
*               Create a mutex with name.

* Parameters:
*               None.
*
* Return:        
*               Mutex Id
*****************************************************************/
u32 Ql_OS_CreateMutex(void);

/*****************************************************************
* Function:     Ql_OS_TakeMutex 
* 
* Description:
*               Obtain an instance of the specified mutex.

* Parameters:
*               mutexId: 
*                        Mutex Id
*               block_time:
*                        The time in ticks to wait for the mutex to become  available.unit:ms
* Return:        
*                TRUE indicates success or FALSE indicates failure. 
*****************************************************************/
bool Ql_OS_TakeMutex(u32 mutexId,u32 block_time);

/*****************************************************************
* Function:     Ql_OS_GiveMutex 
* 
* Description:
*               Release the instance of the specified mutex.

* Parameters:
*               mutexId: Mutex Id
* Return:        
*               TRUE indicates success or FALSE indicates failure. 
*****************************************************************/
bool Ql_OS_GiveMutex(u32 mutexId);

/*****************************************************************
* Function:     Ql_OS_TakeMutexFromISR 
* 
* Description:
*               A version of Ql_OS_TakeMutex() that can be called from an ISR.
*
* Parameters:
*               mutexId: 
*                        Mutex Id
* Return:        
*                TRUE indicates success or FALSE indicates failure. 
*****************************************************************/
bool Ql_OS_TakeMutexFromISR(u32 mutexId);

/*****************************************************************
* Function:     Ql_OS_GiveMutexFromISR 
* 
* Description:
*               A version of Ql_OS_GiveMutex() that can be called from an ISR.
*
* Parameters:
*               mutexId: Mutex Id
* Return:        
*               TRUE indicates success or FALSE indicates failure. 
*****************************************************************/
bool Ql_OS_GiveMutexFromISR(u32 mutexId);

/*****************************************************************
* Function:     Ql_OS_DeleteMutex 
* 
* Description:
*               Delete an instance of the specified mutex.

* Parameters:
*               mutexId: Mutex Id
* Return:        
*               void
*****************************************************************/
void Ql_OS_DeleteMutex(u32 mutexId);

/*****************************************************************
* Function:     Ql_OS_CreateSemaphore 
* 
* Description:
*               Creates a counting semaphore.
* Parameters:
*               maxCount: 
*                   The maximum count value that can be reached.  When the semaphore reaches this value it can no longer be 'given'.
*               InitialCount: 
*                  The count value assigned to the semaphore when it is created.
* Return:        
*               Value of created semaphore
*****************************************************************/
u32 Ql_OS_CreateSemaphore(u32 maxCount,u32 InitialCount);

/*****************************************************************
* Function:     Ql_OS_TakeSemaphore 
* 
* Description:
*               Obtain an instance of the specified semaphore.

* Parameters:
*               semId:
*                        A handle to the semaphore being taken - obtained when the semaphore was created.
*               block_time: 
*                        wait for the semaphore to become available, unit:ms
* Return:        
*               TRUE: if the semaphore was obtained. 
*               FALSE: if xBlockTime expired without the semaphore becoming available..
*****************************************************************/
bool Ql_OS_TakeSemaphore(u32 semId, u32 block_time);

/*****************************************************************
* Function:     Ql_OS_GiveSemaphore 
* 
* Description:
*               Release the instance of the specified semaphore.

* Parameters:
*               semId: 
*                       A handle to the semaphore being taken - obtained when the semaphore was created.
* Return:        
*                TRUE indicates success or FALSE indicates failure. 
*****************************************************************/
bool Ql_OS_GiveSemaphore(u32 semId);

/*****************************************************************
* Function:     Ql_OS_TakeSemaphoreFromISR 
* 
* Description:
*              A version of Ql_OS_TakeSemaphore() that can be called from an ISR.
*
* Parameters:
*               semId:
*                        A handle to the semaphore being taken - obtained when the semaphore was created.
* Return:        
*               TRUE: if the semaphore was obtained. 
*               FALSE: if xBlockTime expired without the semaphore becoming available..
*****************************************************************/
bool Ql_OS_TakeSemaphoreFromISR(u32 semId);

/*****************************************************************
* Function:     Ql_OS_GiveSemaphoreFromISR 
* 
* Description:
*               A version of Ql_OS_GiveSemaphore() that can be called from an ISR.
*
* Parameters:
*               semId: 
*                       A handle to the semaphore being taken - obtained when the semaphore was created.
* Return:        
*                TRUE indicates success or FALSE indicates failure. 
*****************************************************************/
bool Ql_OS_GiveSemaphoreFromISR(u32 semId);


/*****************************************************************
* Function:     Ql_OS_CreateEvent 
* 
* Description:
* 			This function creates an event-flag group with the specified name.
*			Each event-flag group contains 10 event flags. 
*
* Parameters:
*			None
*
* Return:        
*			An event Id that identify this event uniquely.
*****************************************************************/
u32 Ql_OS_CreateEvent(void);

/*****************************************************************
* Function:     Ql_OS_WaitEvent 
* 
* Description:
* 			This function waits until the specified type of event is in the signaled 
*                  state. Developers can specify different types of events for purposes.
*                  The event flags are defined in 'Enum_EventFlag'.
*
* Parameters:
*			evtId:
*				Event id that is returned by calling Ql_OS_CreateEvent().
*			evtFlag:
*				Event flag type. Please refer to Enum_EventFlag.
*               block_time:
*                         wait for the event to become available, unit:ms
*
* Return:        
*		    Returns the current event flag group value.
*****************************************************************/
s32 Ql_OS_WaitEvent(u32 evtId, u32 evtFlag,u32 block_time);

/*****************************************************************
* Function:     Ql_OS_SetEvent 
* 
* Description:
*                  This function sets the specified event flag. Any task waiting on the
*                  event, whose event flag request is satisfied, is resumed.
*
* Parameters:
*			evtId:
*				Event id that is returned by calling Ql_OS_CreateEvent().
*			evtFlag:
*				Event flag type. Please refer to Enum_EventFlag.
*
* Return:        
*			Returns the current event flag group value.
*****************************************************************/
s32 Ql_OS_SetEvent(u32 evtId, u32 evtFlag);

/*****************************************************************
* Function:     Ql_OS_WaitEventFromISR 
* 
* Description:
* 			A version of Ql_OS_WaitEvent() that can be called from an ISR.
*
* Parameters:
*			evtId:
*				Event id that is returned by calling Ql_OS_CreateEvent().
*
* Return:        
*		    If the message was sent to the RTOS daemon task then pdPASS is returned, 
* otherwise pdFAIL is returned. pdFAIL will be returned if the timer service queue was full.
*****************************************************************/
bool Ql_OS_WaitEventFromISR(u32 evtId);

/*****************************************************************
* Function:     Ql_OS_SetEventFromISR
* 
* Description:
*                   A version of Ql_OS_SetEvent() that can be called from an ISR.
*
* Parameters:
*			evtId:
*				Event id that is returned by calling Ql_OS_CreateEvent().
*			evtFlag:
*				Event flag type. Please refer to Enum_EventFlag.
*
* Return:        
*			Returns the current event flag group value.
*****************************************************************/
s32 Ql_OS_SetEventFromISR(u32 evtId, u32 evtFlag);

/*****************************************************************
* Function:     Ql_OS_DeleteEvent
* 
* Description:
*                  Delete an event group that was previously created using a call to Ql_OS_CreateEvent().
*
* Parameters:
*			evtId:
*				Event id that is returned by calling Ql_OS_CreateEvent().
*
* Return:        
*			None
*****************************************************************/
void Ql_OS_DeleteEvent(u32 evtId);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@Task-related interface
*/


/*****************************************************************
* Function:     Ql_OS_GetCurrentTaskHandle 
* 
* Description:
*              Get the handle id of the current task.
*
* Parameters:
*               None
* Return:        
*               If a task has been created then the handle of the task is returned, otherwise NULL is returned.
*****************************************************************/
TaskHandle_t Ql_OS_GetCurrentTaskHandle(void);

/*****************************************************************
* Function:     Ql_OS_TaskSuspend 
* 
* Description:
*               Suspend a task. When suspended a task will never get any microcontroller processing time, 
*     no matter what its priority.
*
* Parameters:
*               task_handle:
*               Handle to the task being suspended. Passing a NULL handle will cause the calling task to be suspended.
* Return:        
*               None
*****************************************************************/
void Ql_OS_TaskSuspend(TaskHandle_t task_handle);

/*****************************************************************
* Function:     Ql_OS_TaskResume 
* 
* Description:
*               Resumes a suspended task.
*
* Parameters:
*               task_handle:
*               Handle to the task being readied.
* Return:        
*               None.
*****************************************************************/
void Ql_OS_TaskResume(TaskHandle_t task_handle);

/*****************************************************************
* Function:     Ql_OS_TaskResumeFromISR 
* 
* Description:
*              A function to resume a suspended task that can be called from within an ISR.
*
* Parameters:
*               task_handle:
*               Handle to the task being readied.
* Return:        
*             pdTRUE if resuming the task should result in a context switch, otherwise pdFALSE. 
*This is used by the ISR to determine if a context switch may be required following the ISR.
*****************************************************************/
bool Ql_OS_TaskResumeFromISR(TaskHandle_t task_handle);


/*****************************************************************
* Function:     Ql_OS_GetCurrentTaskPriority 
* 
* Description:
*               Get the priority of the current task

* Parameters:
*               None
* Return:        
*               Task priority,The default value is 4.
*****************************************************************/
u32 Ql_OS_GetCurrentTaskPriority(void);

/*****************************************************************
* Function:     Ql_OS_GetCurrenTaskLeftStackSize 
* 
* Description:
*               Get the left number of bytes in the current task stack
* Parameters:
*               None
* Return:        
*               Number of bytes, ranges from 1024 Bytes to 10*1024 Bytes.
*****************************************************************/
u32 Ql_OS_GetCurrenTaskLeftStackSize(void);

/*****************************************************************
* Function:     Ql_OS_GetActiveTaskId 
* 
* Description:
*               This function returns the task ID of the current task.
* Parameters:
*               None.
* Return:        
*               The Id number of current task.
*****************************************************************/
s32 Ql_OS_GetActiveTaskId(void);

/*****************************************************************
* Function:     Ql_OS_GetTaskTickCount 
* 
* Description:
*               This function returns the task ticks since task start scheduler.
*               1 tick equals 10ms.
* Parameters:
*               None.
* Return:        
*               the count of ticks.
*****************************************************************/
u32 Ql_OS_GetTaskTickCount(void);

/*****************************************************************
* Function:     Ql_OS_GetTaskTickCountFromISR 
* 
* Description:
*               This function returns the task ticks since task start scheduler,it is safe to be called from an ISR.
*               1 tick equals 10ms.
*
* Parameters:
*               None.
* Return:        
*               the count of ticks.
*****************************************************************/
u32 Ql_OS_GetTaskTickCountFromISR(void);

#endif 

