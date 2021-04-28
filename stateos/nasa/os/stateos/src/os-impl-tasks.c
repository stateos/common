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
 * \file     os-impl-tasks.c
 * \ingroup  stateos
 * \author   Rajmund Szymanski
 *
 */

/****************************************************************************************
                                        INCLUDES
 ***************************************************************************************/

#include "os-stateos.h"
#include "os-impl-tasks.h"
#include "os-shared-task.h"
#include "os-shared-idmap.h"

/****************************************************************************************
                                    GLOBAL VARIABLES
 ***************************************************************************************/

/* Tables where the OS object information is stored */
OS_impl_task_internal_record_t OS_impl_task_table[OS_MAX_TASKS];

/****************************************************************************************
                                     IMPLEMENTATION
 ***************************************************************************************/

/*----------------------------------------------------------------
 *
 * Function: OS_TaskAPI_Impl_Init
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
int32 OS_TaskAPI_Impl_Init(void)
{
    memset(OS_impl_task_table, 0, sizeof(OS_impl_task_table));

    return OS_SUCCESS;

} /* end OS_TaskAPI_Impl_Init */

/*----------------------------------------------------------------
 *
 * Function: OS_TaskCreate_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TaskCreate_Impl(const OS_object_token_t *token, uint32 flags)
{
    OS_impl_task_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_task_table, *token);
    OS_task_internal_record_t      *task = OS_OBJECT_TABLE_GET(OS_task_table, *token);
    OS_VoidPtrValueWrapper_t        impl_arg = {0};

    (void) flags;

    impl_arg.id = OS_ObjectIdFromToken(token);
    impl->id = tsk_setup(OS_PriorityRemap(task->priority), OS_TaskEntryPoint, impl_arg.opaque_arg, task->stack_size);
    if (impl->id == NULL)
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;

} /* end OS_TaskCreate_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TaskDelete_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TaskDelete_Impl(const OS_object_token_t *token)
{
    OS_impl_task_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_task_table, *token);

    tsk_delete(impl->id);
    impl->id = NULL;

    return OS_SUCCESS;

} /* end OS_TaskDelete_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TaskDetach_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TaskDetach_Impl(const OS_object_token_t *token)
{
    OS_impl_task_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_task_table, *token);

    int status = tsk_detach(impl->id);
    switch (status)
    {
        case E_SUCCESS: return OS_SUCCESS;
        default:        return OS_ERROR;
    }

} /* end OS_TaskDetach_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TaskExit_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_TaskExit_Impl()
{
    tsk_exit();

} /* end OS_TaskExit_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TaskDelay_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TaskDelay_Impl(uint32 millis)
{
    tsk_delay(millis * MSEC);

    return OS_SUCCESS;

} /* end OS_TaskDelay_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TaskSetPriority_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TaskSetPriority_Impl(const OS_object_token_t *token, osal_priority_t new_priority)
{
    OS_impl_task_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_task_table, *token);

    sys_lock();
    {
        core_tsk_prio(impl->id, impl->id->basic = OS_PriorityRemap(new_priority));
    }
    sys_unlock();

    return OS_SUCCESS;

} /* end OS_TaskSetPriority_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TaskMatch_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TaskMatch_Impl(const OS_object_token_t *token)
{
    OS_impl_task_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_task_table, *token);

    if (impl->id != tsk_this())
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;

} /* end OS_TaskMatch_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TaskRegister_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TaskRegister_Impl(osal_id_t global_task_id)
{
    (void) global_task_id;

    return OS_SUCCESS;

} /* end OS_TaskRegister_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TaskGetId_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
osal_id_t OS_TaskGetId_Impl(void)
{
    OS_VoidPtrValueWrapper_t impl_arg = {0};

    impl_arg.opaque_arg = tsk_this()->arg;

    return impl_arg.id;

} /* end OS_TaskGetId_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TaskGetInfo_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TaskGetInfo_Impl(const OS_object_token_t *token, OS_task_prop_t *task_prop)
{
    (void) token;
    (void) task_prop;

    return OS_SUCCESS;

} /* end OS_TaskGetInfo_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TaskValidateSystemData_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TaskValidateSystemData_Impl(const void *sysdata, size_t sysdata_size)
{
    if (sysdata == NULL || sysdata_size != sizeof(tsk_t *))
    {
        return OS_INVALID_POINTER;
    }

    return OS_SUCCESS;

} /* end OS_TaskValidateSystemData_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_TaskIdMatchSystemData_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
bool OS_TaskIdMatchSystemData_Impl(void *ref, const OS_object_token_t *token, const OS_common_record_t *obj)
{
    const tsk_t                   **target = (const tsk_t **)ref;
    OS_impl_task_internal_record_t *impl = OS_OBJECT_TABLE_GET(OS_impl_task_table, *token);

    (void) obj;

    return (*target == impl->id);

} /* end OS_TaskIdMatchSystemData_Impl */
