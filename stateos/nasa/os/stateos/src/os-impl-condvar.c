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
#include "os-impl-condvar.h"
#include "os-shared-condvar.h"
#include "os-shared-idmap.h"

/****************************************************************************************
                                   GLOBAL DATA
 ***************************************************************************************/

/* Tables where the OS object information is stored */
OS_impl_condvar_internal_record_t OS_impl_condvar_table[OS_MAX_CONDVARS];

/****************************************************************************************
                                  CONDVAR API
 ***************************************************************************************/

/*----------------------------------------------------------------
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarAPI_Impl_Init(void)
{
    memset(OS_impl_condvar_table, 0, sizeof(OS_impl_condvar_table));

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarCreate_Impl(const OS_object_token_t *token, uint32 options)
{
    (void) options;

    OS_impl_condvar_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

    impl->mtx = mtx_create(mtxRecursive + mtxPrioInherit, 0);
    if (impl->mtx == NULL)
    {
        OS_DEBUG("Unhandled mtx_create error\n");
        return OS_ERROR;
    }
    else
    {
        impl->cnd = cnd_create();
        if (impl->cnd == NULL)
        {
            mtx_delete(impl->mtx); impl->mtx = NULL;

       	    OS_DEBUG("Unhandled cnd_create error\n");
            return OS_ERROR;
        }
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarDelete_Impl(const OS_object_token_t *token)
{
    OS_impl_condvar_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

    cnd_delete(impl->cnd); impl->cnd = NULL;
    mtx_delete(impl->mtx); impl->mtx = NULL;

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarUnlock_Impl(const OS_object_token_t *token)
{
    OS_impl_condvar_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

    int status = mtx_unlock(impl->mtx);
    switch (status)
    {
        case E_SUCCESS: return OS_SUCCESS;
        default:        OS_DEBUG("Unhandled mtx_unlock error\n");
                        return OS_ERROR;
    }
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarLock_Impl(const OS_object_token_t *token)
{
    OS_impl_condvar_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

    int status = mtx_lock(impl->mtx);
    switch (status)
    {
        case E_SUCCESS: return OS_SUCCESS;
        default:        OS_DEBUG("Unhandled mtx_lock error\n");
                        return OS_ERROR;
    }
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarSignal_Impl(const OS_object_token_t *token)
{
    OS_impl_condvar_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

    cnd_notifyOne(impl->cnd);

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarBroadcast_Impl(const OS_object_token_t *token)
{
    OS_impl_condvar_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

    cnd_notifyAll(impl->cnd);

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarWait_Impl(const OS_object_token_t *token)
{
    OS_impl_condvar_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

    int status = cnd_wait(impl->cnd, impl->mtx);
    switch (status)
    {
        case E_SUCCESS: return OS_SUCCESS;
        default:        OS_DEBUG("Unhandled cnd_wait error\n");
                        return OS_ERROR;
    }
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarTimedWait_Impl(const OS_object_token_t *token, const OS_time_t *abs_wakeup_time)
{
    OS_impl_condvar_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

    int status = cnd_waitUntil(impl->cnd, impl->mtx, (cnt_t)abs_wakeup_time->ticks);
    switch (status)
    {
        case E_SUCCESS: return OS_SUCCESS;
        case E_TIMEOUT: return OS_ERROR_TIMEOUT;
        default:        OS_DEBUG("Unhandled cnd_waitFor error\n");
                        return OS_ERROR;
    }
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarGetInfo_Impl(const OS_object_token_t *token, OS_condvar_prop_t *condvar_prop)
{
	(void) token;
	(void) condvar_prop;

    return OS_SUCCESS;
}
