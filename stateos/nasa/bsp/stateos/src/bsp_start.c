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

/*
 * File:     bsp_start.c
 * \ingroup  stateos
 * \author   Rajmund Szymanski
 *
 */

/*
**  Include Files
*/
#include <stdio.h>
#include <stdlib.h>
#include "os.h"
#include "bsp_internal.h"

/*
** Global variables
*/
OS_BSP_impl_GlobalData_t OS_BSP_impl_Global;

/* ---------------------------------------------------------
    OS_BSP_Init()

     Helper function
   --------------------------------------------------------- */
void OS_BSP_Init(void)
{
    /*
     * Initially clear the global objects
     */
    memset(&OS_BSP_Global, 0, sizeof(OS_BSP_Global));
    memset(&OS_BSP_impl_Global, 0, sizeof(OS_BSP_impl_Global));

    /* Initialize the low level access mutex (w/priority inheritance) */
    OS_BSP_impl_Global.AccessMutex = mtx_create(mtxErrorCheck + mtxPrioInherit, 0);
    if (OS_BSP_impl_Global.AccessMutex == NULL)
    {
        BSP_DEBUG("mutex create failed\n");
    }
}

/*----------------------------------------------------------------
   OS_BSP_Lock_Impl
   See full description in header
 ------------------------------------------------------------------*/
void OS_BSP_Lock_Impl(void)
{
    int status;

    status = mtx_lock(OS_BSP_impl_Global.AccessMutex);
    if (status != E_SUCCESS)
    {
        BSP_DEBUG("mutex lock errno: %d\n", status);
    }
}

/*----------------------------------------------------------------
   OS_BSP_Unlock_Impl
   See full description in header
 ------------------------------------------------------------------*/
void OS_BSP_Unlock_Impl(void)
{
    int status;

    status = mtx_unlock(OS_BSP_impl_Global.AccessMutex);
    if (status != E_SUCCESS)
    {
        BSP_DEBUG("mutex unlock errno: %d\n", status);
    }
}

/* ---------------------------------------------------------
    OS_BSP_GetReturnStatus()

     Helper function to convert an OSAL status code into
     a code suitable for returning to the OS.
   --------------------------------------------------------- */
int OS_BSP_GetReturnStatus(void)
{
    return OS_BSP_Global.AppStatus;
}

/* ---------------------------------------------------------
    OS_BSP_Shutdown_Impl()

     Helper function to abort the running task
   --------------------------------------------------------- */
void OS_BSP_Shutdown_Impl(void)
{
    abort();
}

/* ---------------------------------------------------------
    OS_BSP_Main()

     BSP Application entry point.
   --------------------------------------------------------- */
int OS_BSP_Main(void)
{
    /*
     * Perform BSP-specific initialization
     * Currently BSP-specific initialization is performed in the API_Init function
     */
    // OS_BSP_Init();

    /*
     * Call application specific entry point.
     * This should set up all user tasks and resources, then return
     */
    OS_Application_Startup();

    /*
     * OS_Application_Run() implements the background task.
     * The user application may provide this, or a default implementation
     * is used which just calls OS_IdleLoop().
     */
    OS_Application_Run();

    /* Should typically never get here */
    OS_BSP_Shutdown_Impl();
}
