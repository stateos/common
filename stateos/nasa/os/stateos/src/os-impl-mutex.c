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
#include "os-impl-mutex.h"
#include "os-shared-mutex.h"
#include "os-shared-idmap.h"

/****************************************************************************************
                                   GLOBAL DATA
 ***************************************************************************************/

/* Tables where the OS object information is stored */
OS_impl_mutex_internal_record_t OS_impl_mutex_table[OS_MAX_MUTEXES];

/****************************************************************************************
                                  MUTEX API
 ***************************************************************************************/

/*----------------------------------------------------------------
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
int32 OS_MutexAPI_Impl_Init(void)
{
    memset(OS_impl_mutex_table, 0, sizeof(OS_impl_mutex_table));

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_MutSemCreate_Impl(const OS_object_token_t *token, uint32 options)
{
    (void) options;

    OS_impl_mutex_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_mutex_table, *token);

    impl->mtx = mtx_create(mtxRecursive + mtxPrioInherit, 0);
    if (impl->mtx == NULL)
    {
        OS_DEBUG("Unhandled mtx_create error\n");
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_MutSemDelete_Impl(const OS_object_token_t *token)
{
    OS_impl_mutex_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_mutex_table, *token);

    mtx_delete(impl->mtx); impl->mtx = NULL;

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_MutSemGive_Impl(const OS_object_token_t *token)
{
    OS_impl_mutex_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_mutex_table, *token);

    int status = mtx_give(impl->mtx);
    switch (status)
    {
        case E_SUCCESS: return OS_SUCCESS;
        default:        OS_DEBUG("Unhandled mtx_give error\n");
                        return OS_ERROR;
    }
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_MutSemTake_Impl(const OS_object_token_t *token)
{
    OS_impl_mutex_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_mutex_table, *token);

    int status = mtx_wait(impl->mtx);
    switch (status)
    {
        case E_SUCCESS: return OS_SUCCESS;
        default:        OS_DEBUG("Unhandled mtx_wait error\n");
                        return OS_ERROR;
    }
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_MutSemGetInfo_Impl(const OS_object_token_t *token, OS_mut_sem_prop_t *mut_prop)
{
    (void) token;
    (void) mut_prop;

    return OS_SUCCESS;
}
