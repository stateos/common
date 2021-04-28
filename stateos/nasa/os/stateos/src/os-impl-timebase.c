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
                                    INCLUDE FILES
 ***************************************************************************************/

#include "os-stateos.h"
#include "os-impl-timebase.h"
#include "os-shared-timebase.h"
#include "os-shared-common.h"
#include "os-shared-time.h"
#include "os-shared-idmap.h"

/****************************************************************************************
                                   GLOBAL DATA
 ***************************************************************************************/

OS_impl_timebase_internal_record_t OS_impl_timebase_table[OS_MAX_TIMEBASES];

/****************************************************************************************
                                INTERNAL FUNCTIONS
 ***************************************************************************************/

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
    OS_impl_timebase_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    mtx_lock(impl->handler_mtx);

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
    OS_impl_timebase_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    mtx_unlock(impl->handler_mtx);

} /* end OS_TimeBaseUnlock_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TimeBaseHandlerISR
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *           An ISR to service a timer tick interrupt, which in turn
 *           posts a semaphore so the user task can execute
 *
 *-----------------------------------------------------------------*/

static void OS_TimeBaseHandlerISR(void *arg)
{
    OS_object_token_t                   token;
    OS_impl_timebase_internal_record_t *local;
    OS_VoidPtrValueWrapper_t            local_arg = {0};

    local_arg.opaque_arg = arg;

    if (OS_ObjectIdGetById(OS_LOCK_MODE_NONE, OS_OBJECT_TYPE_OS_TIMEBASE, local_arg.id, &token) == OS_SUCCESS)
    {
        local = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, token);
        /* no action is required for the periodic timer */
        sem_post(local->tick_sem);
    }
} /* end OS_TimeBaseHandlerISR */

/*----------------------------------------------------------------
 *
 * Function: OS_TimeBaseHandler
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
static void OS_TimeBaseHandler(void *arg)
{
    OS_VoidPtrValueWrapper_t local_arg = {0};

    local_arg.opaque_arg = arg;

    OS_TimeBase_CallbackThread(local_arg.id);

} /* end OS_TimeBaseHandler */

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
        return 0;

    impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, token);

    sem_wait(impl->tick_sem);

    if (impl->reset_flag)
    {
        impl->reset_flag = false;
        return impl->start_time;
    }
    else
    {
        return impl->interval_time;
    }

} /* end OS_TimeBaseWait_Impl */

/****************************************************************************************
                                INITIALIZATION FUNCTION
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

/****************************************************************************************
                                   TIME BASE API
 ***************************************************************************************/

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
    OS_impl_timebase_internal_record_t *impl;
    OS_timebase_internal_record_t      *timebase;
    OS_VoidPtrValueWrapper_t            impl_arg = {0};

    impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);
    timebase = OS_OBJECT_TABLE_GET(OS_timebase_table, *token);

    if (timebase->external_sync == NULL)
    {
        timebase->external_sync = OS_TimeBaseWait_Impl;

	    impl_arg.id = OS_ObjectIdFromToken(token);

	    impl->tmr = tmr_setup(OS_TimeBaseHandlerISR, impl_arg.opaque_arg);
	    if (impl->tmr == NULL)
	    {
	        return OS_TIMER_ERR_UNAVAILABLE;
	    }

	    impl->tick_sem = sem_create(0, semBinary);
	    if (impl->tick_sem == NULL)
	    {
	        tmr_delete(impl->tmr); impl->tmr = NULL;
	        return OS_TIMER_ERR_INTERNAL;
	    }

	    impl->handler_mtx = mtx_create(mtxNormal + mtxPrioInherit, 0);
	    if (impl->handler_mtx == NULL)
	    {
	        sem_delete(impl->tick_sem); impl->tick_sem = NULL;
	        tmr_delete(impl->tmr);      impl->tmr = NULL;
	        return OS_TIMER_ERR_INTERNAL;
	    }
	}

    impl->handler_tsk = thd_create(OS_PriorityRemap(1), OS_TimeBaseHandler, impl_arg.opaque_arg, OS_STACK_SIZE);
    if (impl->handler_tsk == NULL)
    {
        mtx_delete(impl->handler_mtx); impl->handler_mtx = NULL;
        sem_delete(impl->tick_sem);    impl->tick_sem = NULL;
        tmr_delete(impl->tmr);         impl->tmr = NULL;
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
    OS_timebase_internal_record_t *timebase = OS_OBJECT_TABLE_GET(OS_timebase_table, *token);

    impl->reset_flag = true;
    impl->start_time = start_time;
    impl->interval_time = interval_time;

    timebase->accuracy_usec = interval_time == 0 ? start_time : interval_time;

    tmr_start(impl->tmr, start_time / OS_SharedGlobalVars.MicroSecPerTick, interval_time / OS_SharedGlobalVars.MicroSecPerTick);

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
    OS_impl_timebase_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    tsk_delete(impl->handler_tsk); impl->handler_tsk = NULL;
    mtx_delete(impl->handler_mtx); impl->handler_mtx = NULL;
    sem_delete(impl->tick_sem);    impl->tick_sem = NULL;
    tmr_delete(impl->tmr);         impl->tmr = NULL;

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
