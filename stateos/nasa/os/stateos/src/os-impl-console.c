/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * \file
 * \ingroup  stateos
 * \author   Rajmund Szymanski
 *
 */

/****************************************************************************************
                                    INCLUDE FILES
 ***************************************************************************************/

#include "os-stateos.h"
#include "os-impl-console.h"
#include "os-shared-printf.h"
#include "os-shared-idmap.h"
#include "os-shared-common.h"

/****************************************************************************************
                                     DEFINES
 ***************************************************************************************/

#define OS_CONSOLE_TASK_PRIORITY  OS_UTILITYTASK_PRIORITY
#define OS_CONSOLE_TASK_STACKSIZE OS_UTILITYTASK_STACK_SIZE

/****************************************************************************************
                                   GLOBAL DATA
 ***************************************************************************************/

/* Tables where the OS object information is stored */
OS_impl_console_internal_record_t OS_impl_console_table[OS_MAX_CONSOLES];

/********************************************************************/
/*                 CONSOLE OUTPUT                                   */
/********************************************************************/

/*----------------------------------------------------------------
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
int32 OS_ConsoleAPI_Impl_Init(void)
{
    memset(OS_impl_console_table, 0, sizeof(OS_impl_console_table));

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_ConsoleWakeup_Impl(const OS_object_token_t *token)
{
    OS_impl_console_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_console_table, *token);

    /* post the sem for the utility task to run */
    sem_post(impl->data_sem);
}

/*----------------------------------------------------------------
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
static void OS_ConsoleTask_Entry(void *arg)
{
    OS_object_token_t        token;
    OS_VoidPtrValueWrapper_t local_arg;

    local_arg.opaque_arg = arg;
    if (OS_ObjectIdGetById(OS_LOCK_MODE_REFCOUNT, OS_OBJECT_TYPE_OS_CONSOLE, local_arg.id, &token) == OS_SUCCESS)
    {
        OS_impl_console_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_console_table, token);

        while (OS_SharedGlobalVars.GlobalState != OS_SHUTDOWN_MAGIC_NUMBER)
        {
            OS_ConsoleOutput_Impl(&token);
            sem_wait(impl->data_sem);
        }

        OS_ObjectIdRelease(&token);
    }
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_ConsoleCreate_Impl(const OS_object_token_t *token)
{
    tsk_t                   *tsk;
    OS_VoidPtrValueWrapper_t local_arg = {0};

    OS_impl_console_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_console_table, *token);
    OS_console_internal_record_t   *console = OS_OBJECT_TABLE_GET(OS_console_table, *token);

    if (OS_ObjectIndexFromToken(token) != 0)
    {
        return OS_ERR_NOT_IMPLEMENTED;
    }

    if (console->IsAsync)
    {
        impl->data_sem = sem_create(0, semCounting);
        if (impl->data_sem == NULL)
        {
            OS_DEBUG("Unhandled sem_create error\n");
            return OS_SEM_FAILURE;
        }

        local_arg.id = OS_ObjectIdFromToken(token);
        tsk = tsk_setup(OS_PriorityRemap(OS_CONSOLE_TASK_PRIORITY), OS_ConsoleTask_Entry, local_arg.opaque_arg, OS_CONSOLE_TASK_STACKSIZE);
        if (tsk == NULL)
        {
            sem_delete(impl->data_sem); impl->data_sem = NULL;

            OS_DEBUG("Unhandled tsk_setup error\n");
            return OS_ERROR;
        }

        tsk_detach(tsk);
    }

    return OS_SUCCESS;
}
