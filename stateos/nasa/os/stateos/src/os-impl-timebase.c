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
    OS_impl_timebase_internal_record_t *local = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    mtx_lock(&local->handler_mtx);

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
    OS_impl_timebase_internal_record_t *local = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    mtx_unlock(&local->handler_mtx);

} /* end OS_TimeBaseUnlock_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_Timer_HandlerISR
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *           An ISR to service a timer tick interrupt, which in turn
 *           posts a semaphore so the user task can execute
 *
 *-----------------------------------------------------------------*/

static void OS_Timer_HandlerISR(void)
{
    OS_object_token_t                   token;
    OS_impl_timebase_internal_record_t *local;
    osal_id_t id = OS_GET_OBJECT_ID(tmr_thisISR(), OS_impl_timebase_internal_record_t, tmr);

    if (OS_ObjectIdGetById(OS_LOCK_MODE_NONE, OS_OBJECT_TYPE_OS_TIMEBASE, id, &token) == OS_SUCCESS)
    {
        local = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, token);
        sem_post(&local->tick_sem);
    }
} /* end OS_Timer_HandlerISR */

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
    OS_impl_timebase_internal_record_t *local;
    int                                 status;

    if (OS_ObjectIdGetById(OS_LOCK_MODE_NONE, OS_OBJECT_TYPE_OS_TIMEBASE, timebase_id, &token) != OS_SUCCESS)
        return 0;

    local = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, token);

    status = sem_wait(&local->tick_sem);
    if (status != E_SUCCESS)
        return 0;

    if (local->reset_flag)
    {
        local->reset_flag = false;
        return local->start_time;
    }

    return local->interval_time;

} /* end OS_TimeBaseWait_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TimeBaseHandler
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
static void OS_TimeBase_Handler(void)
{
    osal_id_t id = OS_GET_OBJECT_ID(tsk_this(), OS_impl_timebase_internal_record_t, handler_tsk);

    OS_TimeBase_CallbackThread(id);

} /* end OS_TimeBaseHandler */

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
    OS_impl_timebase_internal_record_t *local = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);
    OS_timebase_internal_record_t *timebase = OS_OBJECT_TABLE_GET(OS_timebase_table, *token);

    void *stack_pointer = malloc(OS_STACK_SIZE);
    if (stack_pointer == NULL)
        return OS_ERROR;
    
    local->id = OS_ObjectIdFromToken(token);

    if (timebase->external_sync == NULL)
        timebase->external_sync = OS_TimeBaseWait_Impl;

    tmr_init(&local->tmr, OS_Timer_HandlerISR);
    sem_init(&local->tick_sem, 0, semBinary);
    mtx_init(&local->handler_mtx, mtxPrioInherit, 0);
    tsk_init(&local->handler_tsk, OS_PriorityRemap(1), OS_TimeBase_Handler, stack_pointer, OS_STACK_SIZE);

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
    OS_impl_timebase_internal_record_t *local = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);
    OS_timebase_internal_record_t *timebase = OS_OBJECT_TABLE_GET(OS_timebase_table, *token);

	local->reset_flag = true;
    local->start_time = start_time;
    local->interval_time = interval_time;

    timebase->accuracy_usec = interval_time == 0 ? start_time : interval_time;

    tmr_start(&local->tmr, start_time / OS_SharedGlobalVars.MicroSecPerTick, interval_time / OS_SharedGlobalVars.MicroSecPerTick);

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
    OS_impl_timebase_internal_record_t *local = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    tmr_delete(&local->tmr);
    tsk_delete(&local->handler_tsk);
    mtx_delete(&local->handler_mtx);
    sem_delete(&local->tick_sem);

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
