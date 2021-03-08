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
 * \file     os-impl-countsem.c
 * \ingroup  stateos
 * \author   Rajmund Szymanski
 *
 */

/****************************************************************************************
                                    INCLUDE FILES
 ***************************************************************************************/

#include "os-stateos.h"
#include "os-impl-countsem.h"
#include "os-shared-countsem.h"
#include "os-shared-idmap.h"

/****************************************************************************************
                                   GLOBAL DATA
 ***************************************************************************************/

OS_impl_countsem_internal_record_t OS_impl_count_sem_table[OS_MAX_COUNT_SEMAPHORES];

/****************************************************************************************
                                INITIALIZATION FUNCTION
 ***************************************************************************************/

/*----------------------------------------------------------------
 *
 * Function: OS_CountSemAPI_Impl_Init
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemAPI_Impl_Init(void)
{
    memset(OS_impl_count_sem_table, 0, sizeof(OS_impl_count_sem_table));

    return OS_SUCCESS;

} /* end OS_CountSemAPI_Impl_Init */

/****************************************************************************************
                               COUNTING SEMAPHORE API
 ***************************************************************************************/

/*----------------------------------------------------------------
 *
 * Function: OS_CountSemCreate_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemCreate_Impl(const OS_object_token_t *token, uint32 sem_initial_value, uint32 options)
{
    OS_impl_countsem_internal_record_t *local = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    (void) options;

    sem_init(&local->sem, sem_initial_value, semCounting);

    return OS_SUCCESS;

} /* end OS_CountSemCreate_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_CountSemDelete_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemDelete_Impl(const OS_object_token_t *token)
{
    OS_impl_countsem_internal_record_t *local = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    sem_delete(&local->sem);

    return OS_SUCCESS;

} /* end OS_CountSemDelete_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_CountSemGive_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemGive_Impl(const OS_object_token_t *token)
{
    OS_impl_countsem_internal_record_t *local = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    int status = sem_give(&local->sem);

    switch(status)
    {
        case E_SUCCESS: return OS_SUCCESS;
        default:        return OS_SEM_FAILURE;
    }
} /* end OS_CountSemGive_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_CountSemTake_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemTake_Impl(const OS_object_token_t *token)
{
    OS_impl_countsem_internal_record_t *local = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    int status = sem_wait(&local->sem);

    switch(status)
    {
        case E_SUCCESS: return OS_SUCCESS;
        default:        return OS_SEM_FAILURE;
    }
} /* end OS_CountSemTake_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_CountSemTimedWait_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemTimedWait_Impl(const OS_object_token_t *token, uint32 millis)
{
    OS_impl_countsem_internal_record_t *local = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    int status = sem_waitFor(&local->sem, millis * MSEC);

    switch(status)
    {
        case E_SUCCESS: return OS_SUCCESS;
        case E_TIMEOUT: return OS_SEM_TIMEOUT;
        default:        return OS_SEM_FAILURE;
    }
} /* end OS_CountSemTimedWait_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_CountSemGetInfo_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemGetInfo_Impl(const OS_object_token_t *token, OS_count_sem_prop_t *count_prop)
{
    OS_impl_countsem_internal_record_t *local = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    int value = (int)sem_getValue(&local->sem);
    count_prop->value = value;

    return OS_SUCCESS;

} /* end OS_CountSemGetInfo_Impl */
