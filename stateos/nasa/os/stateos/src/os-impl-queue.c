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
#include "os-impl-queue.h"
#include "os-shared-queue.h"
#include "os-shared-idmap.h"

/****************************************************************************************
                                   GLOBAL DATA
 ***************************************************************************************/

/* Tables where the OS object information is stored */
OS_impl_queue_internal_record_t OS_impl_queue_table[OS_MAX_QUEUES];

/****************************************************************************************
                                MESSAGE QUEUE API
 ***************************************************************************************/

/*----------------------------------------------------------------
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
int32 OS_QueueAPI_Impl_Init(void)
{
    memset(OS_impl_queue_table, 0, sizeof(OS_impl_queue_table));

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_QueueCreate_Impl(const OS_object_token_t *token, uint32 flags)
{
    (void) flags;

    OS_impl_queue_internal_record_t *impl  = OS_OBJECT_TABLE_GET(OS_impl_queue_table, *token);
    OS_queue_internal_record_t      *queue = OS_OBJECT_TABLE_GET(OS_queue_table, *token);

    impl->msg = msg_create(queue->max_depth, queue->max_size);
    if (impl->msg == NULL)
    {
        OS_DEBUG("Unhandled msg_create error\n");
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
int32 OS_QueueDelete_Impl(const OS_object_token_t *token)
{
    OS_impl_queue_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_queue_table, *token);

    msg_delete(impl->msg); impl->msg = NULL;

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_QueueGet_Impl(const OS_object_token_t *token, void *data, size_t size, size_t *size_copied, int32 timeout)
{
    OS_impl_queue_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_queue_table, *token);

    cnt_t ticks = timeout == OS_PEND  ? INFINITE :
                  timeout == OS_CHECK ? IMMEDIATE :
                  /* else */            MSEC * (uint32)timeout;

    int status = msg_waitFor(impl->msg, data, size, size_copied, ticks);
    switch (status)
    {
        case E_SUCCESS: return OS_SUCCESS;
		case E_FAILURE: return OS_QUEUE_INVALID_SIZE;
        case E_TIMEOUT: return timeout == OS_CHECK ? OS_QUEUE_EMPTY : OS_QUEUE_TIMEOUT;
        default:        OS_DEBUG("Unhandled msg_waitFor error\n");
                        return OS_ERROR;
    }
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_QueuePut_Impl(const OS_object_token_t *token, const void *data, size_t size, uint32 flags)
{
    (void) flags;

    OS_impl_queue_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_queue_table, *token);

    int status = msg_give(impl->msg, data, size);
    switch (status)
    {
        case E_SUCCESS: return OS_SUCCESS;
		case E_FAILURE: return OS_QUEUE_INVALID_SIZE;
        case E_TIMEOUT: return OS_QUEUE_FULL;
        default:        OS_DEBUG("Unhandled msg_give error\n");
                        return OS_ERROR;
    }
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_QueueGetInfo_Impl(const OS_object_token_t *token, OS_queue_prop_t *queue_prop)
{
    (void) token;
    (void) queue_prop;

    return OS_SUCCESS;
}
