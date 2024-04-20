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
 * \file     os-impl-queue.c
 * \ingroup  stateos
 * \author   Rajmund Szymanski
 *
 */

/****************************************************************************************
                                        INCLUDES
 ***************************************************************************************/

#include "os-stateos.h"
#include "os-impl-queue.h"
#include "os-shared-queue.h"
#include "os-shared-idmap.h"

/****************************************************************************************
                                    GLOBAL VARIABLES
 ***************************************************************************************/

/* Tables where the OS object information is stored */
OS_impl_queue_internal_record_t OS_impl_queue_table[OS_MAX_QUEUES];

/****************************************************************************************
                                     IMPLEMENTATION
 ***************************************************************************************/

/*----------------------------------------------------------------
 *
 * Function: OS_QueueAPI_Impl_Init
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
int32 OS_QueueAPI_Impl_Init(void)
{
    memset(OS_impl_queue_table, 0, sizeof(OS_impl_queue_table));

    return OS_SUCCESS;

} /* end OS_QueueAPI_Impl_Init */

/*----------------------------------------------------------------
 *
 * Function: OS_QueueCreate_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_QueueCreate_Impl(const OS_object_token_t *token, uint32 flags)
{
    OS_impl_queue_internal_record_t *local = OS_OBJECT_TABLE_GET(OS_impl_queue_table, *token);
    OS_queue_internal_record_t      *queue = OS_OBJECT_TABLE_GET(OS_queue_table, *token);

    (void) flags;

    local->msg = msg_create(queue->max_depth, queue->max_size);
    if (local->msg == NULL)
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;

} /* end OS_QueueCreate_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_QueueDelete_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_QueueDelete_Impl(const OS_object_token_t *token)
{
    OS_impl_queue_internal_record_t *local = OS_OBJECT_TABLE_GET(OS_impl_queue_table, *token);

    msg_delete(local->msg);
    local->msg = NULL;

    return OS_SUCCESS;

} /* end OS_QueueDelete_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_QueueGet_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_QueueGet_Impl(const OS_object_token_t *token, void *data, size_t size, size_t *size_copied, int32 timeout)
{
    OS_impl_queue_internal_record_t *local = OS_OBJECT_TABLE_GET(OS_impl_queue_table, *token);

    cnt_t ticks = timeout == OS_PEND  ? INFINITE :
                  timeout == OS_CHECK ? IMMEDIATE :
                  /* else */            MSEC * (uint32)timeout;

    int status = msg_waitFor(local->msg, data, size, size_copied, ticks);
    switch (status)
    {
        case E_SUCCESS: return OS_SUCCESS;
		case E_FAILURE: return OS_QUEUE_INVALID_SIZE;
        case E_TIMEOUT: return timeout == OS_CHECK ? OS_QUEUE_EMPTY : OS_QUEUE_TIMEOUT;
        default:        return OS_ERROR;
    }
} /* end OS_QueueGet_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_QueuePut_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_QueuePut_Impl(const OS_object_token_t *token, const void *data, size_t size, uint32 flags)
{
    OS_impl_queue_internal_record_t *local = OS_OBJECT_TABLE_GET(OS_impl_queue_table, *token);

    (void) flags;

    int status = msg_give(local->msg, data, size);
    switch (status)
    {
        case E_SUCCESS: return OS_SUCCESS;
		case E_FAILURE: return OS_QUEUE_INVALID_SIZE;
        case E_TIMEOUT: return OS_QUEUE_FULL;
        default:        return OS_ERROR;
    }
} /* end OS_QueuePut_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_QueueGetInfo_Impl
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

} /* end OS_QueueGetInfo_Impl */
