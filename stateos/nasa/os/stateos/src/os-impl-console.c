/*
 *  NASA Docket No. GSC-18,370-1, and identified as "Operating System Abstraction Layer"
 *
 *  Copyright (c) 2019 United States Government as represented by
 *  the Administrator of the National Aeronautics and Space Administration.
 *  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * \file     os-impl-console.c
 * \ingroup  stateos
 * \author   Rajmund Szymanski
 *
 */

/****************************************************************************************
                                    INCLUDE FILES
 ***************************************************************************************/

#include "os-stateos.h"
#include "os-impl-console.h"
#include "os-shared-common.h"
#include "os-shared-printf.h"
#include "os-shared-idmap.h"

/****************************************************************************************
                                     DEFINES
 ***************************************************************************************/

#define OS_CONSOLE_ASYNC          true
#define OS_CONSOLE_TASK_PRIORITY  OS_UTILITYTASK_PRIORITY
#define OS_CONSOLE_TASK_STACKSIZE OS_UTILITYTASK_STACK_SIZE

/****************************************************************************************
                                   GLOBAL DATA
 ***************************************************************************************/

OS_impl_console_internal_record_t OS_impl_console_table[OS_MAX_CONSOLES];

/****************************************************************************************
                                INITIALIZATION FUNCTION
 ***************************************************************************************/

/*----------------------------------------------------------------
 *
 * Function: OS_ConsoleAPI_Impl_Init
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
int32 OS_ConsoleAPI_Impl_Init(void)
{
    memset(OS_impl_console_table, 0, sizeof(OS_impl_console_table));

    return OS_SUCCESS;

} /* end OS_ConsoleAPI_Impl_Init */

/****************************************************************************************
                                  CONSOLE OUTPUT
 ***************************************************************************************/

/*----------------------------------------------------------------
 *
 * Function: OS_ConsoleWakeup_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_ConsoleWakeup_Impl(const OS_object_token_t *token)
{
    OS_impl_console_internal_record_t *local = OS_OBJECT_TABLE_GET(OS_impl_console_table, *token);

    if (local->is_async)
    {
        /* post the sem for the utility task to run */
        sem_post(&local->data_sem);
    }
    else
    {
        /* output directly */
        OS_ConsoleOutput_Impl(token);
    }
} /* end OS_ConsoleWakeup_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_ConsoleTask_Entry
 *
 *  Purpose: Local Helper function
 *           Implements the console output task
 *
 *-----------------------------------------------------------------*/
static void OS_ConsoleTask_Entry()
{
    OS_object_token_t                  token;
    OS_impl_console_internal_record_t *local;
    osal_id_t id = OS_GET_OBJECT_ID(tsk_this(), OS_impl_console_internal_record_t, tsk);

    if (OS_ObjectIdGetById(OS_LOCK_MODE_REFCOUNT, OS_OBJECT_TYPE_OS_CONSOLE, id, &token) == OS_SUCCESS)
    {
        local = OS_OBJECT_TABLE_GET(OS_impl_console_table, token);

        /* Loop forever (unless shutdown is set) */
        while (OS_SharedGlobalVars.ShutdownFlag != OS_SHUTDOWN_MAGIC_NUMBER)
        {
            OS_ConsoleOutput_Impl(&token);
            sem_wait(&local->data_sem);
        }
        OS_ObjectIdRelease(&token);
    }
} /* end OS_ConsoleTask_Entry */

/*----------------------------------------------------------------
 *
 * Function: OS_ConsoleCreate_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_ConsoleCreate_Impl(const OS_object_token_t *token)
{
    OS_impl_console_internal_record_t *local = OS_OBJECT_TABLE_GET(OS_impl_console_table, *token);

    osal_priority_t priority = OS_CONSOLE_TASK_PRIORITY;
    osal_stackptr_t stack_pointer;

    if (OS_ObjectIndexFromToken(token) != 0)
        return OS_ERR_NOT_IMPLEMENTED;

    local->is_async = OS_CONSOLE_ASYNC;

    if (local->is_async)
    {
        stack_pointer = malloc(OS_CONSOLE_TASK_STACKSIZE);
        if (stack_pointer == NULL)
            return OS_ERROR;

        local->id = OS_ObjectIdFromToken(token);
        sem_init(&local->data_sem, 0, semCounting);
        tsk_init(&local->tsk, OS_PriorityRemap(priority), OS_ConsoleTask_Entry, stack_pointer, OS_CONSOLE_TASK_STACKSIZE);
        local->tsk.obj.res = stack_pointer;
    }

    return OS_SUCCESS;

} /* end OS_ConsoleCreate_Impl */
