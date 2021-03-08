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
 * \file     os-impl-idmap.c
 * \ingroup  stateos
 * \author   Rajmund Szymanski
 *
 */

/****************************************************************************************
                                    INCLUDE FILES
 ***************************************************************************************/

#include "os-stateos.h"
#include "os-shared-idmap.h"

/****************************************************************************************
                                     GLOBALS
 ***************************************************************************************/

enum
{
    MUTEX_TABLE_SIZE = OS_OBJECT_TYPE_USER
};

static mtx_t MUTEX_TABLE[MUTEX_TABLE_SIZE];

/****************************************************************************************
                                INITIALIZATION FUNCTION
 ***************************************************************************************/

/*---------------------------------------------------------------------------------------
   Name: OS_TableMutex_Init

   Purpose: Initialize the tables that the OS API uses to keep track of information
            about objects

   returns: OS_SUCCESS or OS_ERROR
---------------------------------------------------------------------------------------*/
int32 OS_TableMutex_Init(osal_objtype_t idtype)
{
    if (idtype >= MUTEX_TABLE_SIZE)
        return OS_ERROR;
    
    mtx_init(&MUTEX_TABLE[idtype], mtxPrioInherit, 0);

    return OS_SUCCESS;

} /* end OS_TableMutex_Init */

/****************************************************************************************
                                       API
 ***************************************************************************************/

/*----------------------------------------------------------------
 *
 * Function: OS_Lock_Global_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_Lock_Global_Impl(osal_objtype_t idtype)
{
    if (idtype < MUTEX_TABLE_SIZE)
		mtx_lock(&MUTEX_TABLE[idtype]);

} /* end OS_Lock_Global_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_Unlock_Global_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_Unlock_Global_Impl(osal_objtype_t idtype)
{
    if (idtype < MUTEX_TABLE_SIZE)
		mtx_unlock(&MUTEX_TABLE[idtype]);

} /* end OS_Unlock_Global_Impl */

/*----------------------------------------------------------------
 *
 *  Function: OS_WaitForStateChange_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_WaitForStateChange_Impl(osal_objtype_t idtype, uint32 attempts)
{
    uint32 wait_ms;

    if (attempts <= 10)
    {
        wait_ms = attempts * attempts * 10;
    }
    else
    {
        wait_ms = 1000;
    }

    OS_Unlock_Global_Impl(idtype);
    OS_TaskDelay(wait_ms);
    OS_Lock_Global_Impl(idtype);
}
