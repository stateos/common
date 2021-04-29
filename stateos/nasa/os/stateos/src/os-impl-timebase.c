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
 * \file     os-impl-timebase.c
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
 * Function: OS_TimeBaseAPI_Impl_Init
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

} /* end OS_TimeBaseAPI_Impl_Init */

/*----------------------------------------------------------------
 *
 * Function: OS_TimeBaseLock_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_TimeBaseLock_Impl(const OS_object_token_t *token)
{
    OS_impl_timebase_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    mtx_lock(impl->mtx);

} /* end OS_TimeBaseLock_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TimeBaseUnlock_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_TimeBaseUnlock_Impl(const OS_object_token_t *token)
{
    OS_impl_timebase_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    mtx_unlock(impl->mtx);

} /* end OS_TimeBaseUnlock_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TimeBaseWait_Impl
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *           Pends on the semaphore for the next timer tick
 *
 *-----------------------------------------------------------------*/
static uint32 OS_TimeBaseWait_Impl(osal_id_t timebase_id)
{
    OS_object_token_t                   token;
    OS_impl_timebase_internal_record_t *impl;

    if (OS_ObjectIdGetById(OS_LOCK_MODE_NONE, OS_OBJECT_TYPE_OS_TIMEBASE, timebase_id, &token) != OS_SUCCESS)
    {
        return 0;
    }

    impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, token);

    if (impl->start_flag)
    {
        if (sem_wait(impl->sem) != E_SUCCESS)
        {
            return 0;
        }

        tsk_sleepUntil(impl->start_time_point + impl->start_time / OS_SharedGlobalVars.MicroSecPerTick);

        impl->start_flag = false;

        return impl->start_time;
    }
    else
    {
        tsk_sleepNext(impl->interval_time / OS_SharedGlobalVars.MicroSecPerTick);

        return impl->interval_time;
    }
} /* end OS_TimeBaseWait_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TimeBaseCreate_Impl
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

    impl->start_flag = true;

    impl->sem = sem_create(0, semBinary);
    if (impl->sem == NULL)
    {
        return OS_TIMER_ERR_INTERNAL;
    }

    impl->mtx = mtx_create(mtxNormal + mtxPrioInherit, 0);
    if (impl->mtx == NULL)
    {
        sem_delete(impl->sem); impl->sem = NULL;

        return OS_TIMER_ERR_INTERNAL;
    }

    impl->tsk = tsk_setup(0, OS_TimeBase_CallbackThread, impl_arg.opaque_arg, OS_STACK_SIZE);
    if (impl->tsk == NULL)
    {
        mtx_delete(impl->mtx); impl->mtx = NULL;
        sem_delete(impl->sem); impl->sem = NULL;

        return OS_TIMER_ERR_INTERNAL;
    }

    return OS_SUCCESS;

} /* end OS_TimeBaseCreate_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TimeBaseSet_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TimeBaseSet_Impl(const OS_object_token_t *token, uint32 start_time, uint32 interval_time)
{
    OS_impl_timebase_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    impl->start_time = start_time;
    impl->interval_time = interval_time;
    impl->start_time_point = sys_time();

    sem_post(impl->sem);

    return OS_SUCCESS;

} /* end OS_TimeBaseSet_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TimeBaseDelete_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TimeBaseDelete_Impl(const OS_object_token_t *token)
{
    OS_impl_timebase_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    tsk_delete(impl->tsk); impl->tsk = NULL;
    mtx_delete(impl->mtx); impl->mtx = NULL;
    sem_delete(impl->sem); impl->sem = NULL;

    return OS_SUCCESS;

} /* end OS_TimeBaseDelete_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TimeBaseGetInfo_Impl
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

} /* end OS_TimeBaseGetInfo_Impl */
