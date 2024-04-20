/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as �core Flight System: Bootes�
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
#include "os-impl-countsem.h"

#include "os-shared-countsem.h"
#include "os-shared-idmap.h"
#include "os-shared-timebase.h"

/****************************************************************************************
                                   GLOBAL DATA
 ***************************************************************************************/

/*  tables for the properties of objects */
OS_impl_countsem_internal_record_t OS_impl_count_sem_table[OS_MAX_COUNT_SEMAPHORES];

/*----------------------------------------------------------------
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemAPI_Impl_Init(void)
{
    memset(OS_impl_count_sem_table, 0, sizeof(OS_impl_count_sem_table));

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemCreate_Impl(const OS_object_token_t *token, uint32 sem_initial_value, uint32 options)
{
    (void) options;

    OS_impl_countsem_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    impl->sem = sem_create(sem_initial_value, semCounting);
    if (impl->sem == NULL)
    {
        OS_DEBUG("Unhandled sem_create error\n");
        return OS_SEM_FAILURE;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemDelete_Impl(const OS_object_token_t *token)
{
    OS_impl_countsem_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    sem_delete(impl->sem); impl->sem = NULL;

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemGive_Impl(const OS_object_token_t *token)
{
    OS_impl_countsem_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    int status = sem_give(impl->sem);
    switch (status)
    {
        case E_SUCCESS: return OS_SUCCESS;
        default:        OS_DEBUG("Unhandled sem_give error\n");
                        return OS_SEM_FAILURE;
    }
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemTake_Impl(const OS_object_token_t *token)
{
    OS_impl_countsem_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    int status = sem_wait(impl->sem);
    switch(status)
    {
        case E_SUCCESS: return OS_SUCCESS;
        default:        OS_DEBUG("Unhandled sem_wait error\n");
                        return OS_SEM_FAILURE;
    }
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemTimedWait_Impl(const OS_object_token_t *token, uint32 millis)
{
    OS_impl_countsem_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    int status = sem_waitFor(impl->sem, millis * MSEC);
    switch(status)
    {
        case E_SUCCESS: return OS_SUCCESS;
        case E_TIMEOUT: return OS_SEM_TIMEOUT;
        default:        OS_DEBUG("Unhandled sem_waitFor error\n");
                        return OS_SEM_FAILURE;
    }
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemGetInfo_Impl(const OS_object_token_t *token, OS_count_sem_prop_t *count_prop)
{
    OS_impl_countsem_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    unsigned value = sem_getValue(impl->sem);
    count_prop->value = (int) value;

    return OS_SUCCESS;
}
