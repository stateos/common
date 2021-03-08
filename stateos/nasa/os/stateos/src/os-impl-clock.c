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
 * \file     os-impl-clock.c
 * \ingroup  stateos
 * \author   Rajmund Szymanski
 *
 */

/****************************************************************************************
                                    INCLUDE FILES
 ***************************************************************************************/

#include "os-stateos.h"
#include "os-impl-clock.h"
#include "os-shared-clock.h"
#include "os-shared-common.h"

/****************************************************************************************
                                    CLOCK API
 ***************************************************************************************/

/*----------------------------------------------------------------
 *
 * Function: OS_GetLocalTime_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_GetLocalTime_Impl(OS_time_t *time_struct)
{
    int64 ticks = (int64)sys_time() * ((OS_TIME_TICKS_PER_SECOND) / (OS_FREQUENCY));
    time_struct->ticks = ticks - OS_impl_GlobalVars.time_shift;

    return OS_SUCCESS;

} /* end OS_GetLocalTime_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_SetLocalTime_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_SetLocalTime_Impl(const OS_time_t *time_struct)
{
    int64 ticks = (int64)sys_time() * ((OS_TIME_TICKS_PER_SECOND) / (OS_FREQUENCY));
    OS_impl_GlobalVars.time_shift = ticks - time_struct->ticks;

    return OS_SUCCESS;

} /* end OS_SetLocalTime_Impl */
