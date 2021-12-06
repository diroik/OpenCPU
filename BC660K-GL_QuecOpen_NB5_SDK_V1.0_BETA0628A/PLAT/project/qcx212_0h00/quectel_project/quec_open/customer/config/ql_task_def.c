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
 *   ql_task_def.c
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   The file is for some common system definition. Developer don't have to
 *   care.
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
#include "ql_task_def.h"

#pragma diag_suppress 188   // [porter][20210127] The compiler ignores specific warnings


#ifdef TASK_ID_DEF
#undef TASK_ID_DEF
#endif
#ifdef TASK_DEFINITION
#undef TASK_DEFINITION
#endif

#define TASK_FUNC_DECLARATION
#include "ql_task_def.h"
TASK_DEFINITION_BEGIN
#include "ql_task_cfg.h"
TASK_DEFINITION_END
#undef TASK_FUNC_DECLARATION


#ifdef TASK_FUNC_DECLARATION
#undef TASK_FUNC_DECLARATION
#endif
#ifdef TASK_ID_DEF
#undef TASK_ID_DEF
#endif
#define TASK_DEFINITION
#include "ql_task_def.h"
TASK_DEFINITION_BEGIN
#include "ql_task_cfg.h"
TASK_DEFINITION_END
#undef TASK_DEFINITION


void* ql_get_task_enter(void)
{
	return (void*)custom_tasks;
}
