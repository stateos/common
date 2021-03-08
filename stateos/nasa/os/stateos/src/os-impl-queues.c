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
 * \file     os-impl-queues.c
 * \ingroup  stateos
 * \author   Rajmund Szymanski
 *
 */

/****************************************************************************************
                                    INCLUDE FILES
 ***************************************************************************************/

#include "os-stateos.h"
#include "os-impl-queues.h"
#include "os-shared-queue.h"
#include "os-shared-idmap.h"

/****************************************************************************************
                                     DEFINES
 ***************************************************************************************/

/****************************************************************************************
                                   GLOBAL DATA
 ***************************************************************************************/

OS_impl_queue_internal_record_t OS_impl_queue_table[OS_MAX_QUEUES];

/****************************************************************************************
                                INITIALIZATION FUNCTION
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

/****************************************************************************************
                                MESSAGE QUEUE API
 ***************************************************************************************/

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
	OS_queue_internal_record_t *     queue = OS_OBJECT_TABLE_GET(OS_queue_table, *token);

    (void) flags;

    void *data_buffer = malloc(queue->max_depth * queue->max_size);
    if (data_buffer == NULL)
        return OS_ERROR;

    box_init(&local->box, queue->max_size, data_buffer, queue->max_depth * queue->max_size);
    local->box.obj.res = data_buffer;

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

    box_delete(&local->box);

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

    *size_copied = 0;
    if (size < local->box.size)
        return OS_QUEUE_INVALID_SIZE;
    
    int status = box_waitFor(&local->box, data, ticks);

    switch (status)
    {
        case E_SUCCESS: *size_copied = local->box.size; return OS_SUCCESS;
        case E_TIMEOUT: return timeout != IMMEDIATE ? OS_QUEUE_TIMEOUT : OS_QUEUE_EMPTY;
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

    if (size > local->box.size)
        return OS_QUEUE_INVALID_SIZE;

    int status = box_give(&local->box, data);

    switch (status)
    {
        case E_SUCCESS: return OS_SUCCESS;
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
