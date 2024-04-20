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
                                        INCLUDES
 ***************************************************************************************/

#include "os-stateos.h"
#include "os-impl-timebase.h"
#include "os-shared-timebase.h"
#include "os-shared-common.h"
#include "os-shared-time.h"
#include "os-shared-idmap.h"

/****************************************************************************************
                                    GLOBAL VARIABLES
 ***************************************************************************************/

/* Tables where the OS object information is stored */
OS_impl_timebase_internal_record_t OS_impl_timebase_table[OS_MAX_TIMEBASES];

/****************************************************************************************
                                     IMPLEMENTATION
 ***************************************************************************************/

/*----------------------------------------------------------------
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
int32 OS_TimeBaseAPI_Impl_Init(void)
{
    memset(OS_impl_timebase_table, 0, sizeof(OS_impl_timebase_table));

    OS_SharedGlobalVars.TicksPerSecond  = OS_FREQUENCY;
    OS_SharedGlobalVars.MicroSecPerTick = 1000000U / (OS_FREQUENCY);

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_TimeBaseLock_Impl(const OS_object_token_t *token)
{
    OS_impl_timebase_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    mtx_lock(impl->handler_mutex);
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_TimeBaseUnlock_Impl(const OS_object_token_t *token)
{
    OS_impl_timebase_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    mtx_unlock(impl->handler_mutex);
}

/*----------------------------------------------------------------
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *           Pends on the semaphore for the next timer tick
 *
 *-----------------------------------------------------------------*/
static uint32 OS_TimeBaseWait_Impl(osal_id_t timebase_id)
{
    OS_object_token_t token;
    uint32            tick_time = 0;

    if (OS_ObjectIdGetById(OS_LOCK_MODE_NONE, OS_OBJECT_TYPE_OS_TIMEBASE, timebase_id, &token) == OS_SUCCESS)
    {
        OS_impl_timebase_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, token);

        if (impl->reset_flag)
        {
            sem_wait(impl->tick_sem);
            impl->reset_flag = false;
            tick_time = impl->configured_start_time;
        }
        else
        {
            tick_time = impl->configured_interval_time;
        }

        tsk_sleepNext(tick_time / OS_SharedGlobalVars.MicroSecPerTick);
    }

    return tick_time;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *
 *-----------------------------------------------------------------*/
static void OS_TimeBase_CallbackThread_Impl(void *arg)
{
    OS_VoidPtrValueWrapper_t local_arg;

    local_arg.opaque_arg = arg;
    OS_TimeBase_CallbackThread(local_arg.id);
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TimeBaseCreate_Impl(const OS_object_token_t *token)
{
    OS_impl_timebase_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);
    OS_timebase_internal_record_t      *timebase = OS_OBJECT_TABLE_GET(OS_timebase_table, *token);
    OS_VoidPtrValueWrapper_t            impl_arg = {0};

    impl_arg.id = OS_ObjectIdFromToken(token);

    if (timebase->external_sync == NULL)
    {
        timebase->external_sync = OS_TimeBaseWait_Impl;
    }

    impl->reset_flag = true;

    impl->tick_sem = sem_create(0, semBinary);
    if (impl->tick_sem == NULL)
    {
        OS_DEBUG("Unhandled sem_create error\n");
        return OS_TIMER_ERR_INTERNAL;
    }

    impl->handler_mutex = mtx_create(mtxNormal + mtxPrioInherit, 0);
    if (impl->handler_mutex == NULL)
    {
        sem_delete(impl->tick_sem); impl->tick_sem = NULL;

        OS_DEBUG("Unhandled mtx_create error\n");
        return OS_TIMER_ERR_INTERNAL;
    }

    impl->handler_task = tsk_setup(0, OS_TimeBase_CallbackThread_Impl, impl_arg.opaque_arg, OS_STACK_SIZE);
    if (impl->handler_task == NULL)
    {
        mtx_delete(impl->handler_mutex); impl->handler_mutex = NULL;
        sem_delete(impl->tick_sem);      impl->tick_sem = NULL;

        OS_DEBUG("Unhandled tsk_setup error\n");
        return OS_TIMER_ERR_INTERNAL;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TimeBaseSet_Impl(const OS_object_token_t *token, uint32 start_time, uint32 interval_time)
{
    OS_impl_timebase_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    impl->configured_start_time = start_time;
    impl->configured_interval_time = interval_time;
    impl->handler_task->start = sys_time();

    sem_post(impl->tick_sem);

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TimeBaseDelete_Impl(const OS_object_token_t *token)
{
    OS_impl_timebase_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    tsk_delete(impl->handler_task);  impl->handler_task = NULL;
    mtx_delete(impl->handler_mutex); impl->handler_mutex = NULL;
    sem_delete(impl->tick_sem);      impl->tick_sem = NULL;

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TimeBaseGetInfo_Impl(const OS_object_token_t *token, OS_timebase_prop_t *timer_prop)
{
    (void) token;
    (void) timer_prop;

    return OS_SUCCESS;
}
