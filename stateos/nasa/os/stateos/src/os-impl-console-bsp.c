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
 * \file     os-impl-console-bsp.c
 * \ingroup  stateos
 * \author   Rajmund Szymanski
 *
 */

/****************************************************************************************
                                    INCLUDE FILES
 ***************************************************************************************/

#include "os-impl-console.h"
#include "os-shared-printf.h"
#include "os-shared-idmap.h"
#include "bsp-impl.h"

/****************************************************************************************
                                CONSOLE OUTPUT
 ***************************************************************************************/

/*----------------------------------------------------------------
 *
 * Function: OS_ConsoleOutput_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_ConsoleOutput_Impl(const OS_object_token_t *token)
{
    OS_console_internal_record_t *console  = OS_OBJECT_TABLE_GET(OS_console_table, *token);
    size_t                        StartPos = console->ReadPos;
    size_t                        EndPos   = console->WritePos;
    size_t                        WriteSize;

    while (StartPos != EndPos)
    {
        if (StartPos > EndPos)
        {
            /* handle wrap */
            WriteSize = console->BufSize - StartPos;
        }
        else
        {
            WriteSize = EndPos - StartPos;
        }

        OS_BSP_ConsoleOutput_Impl(&console->BufBase[StartPos], WriteSize);

        if (StartPos >= console->BufSize)
        {
            /* handle wrap */
            StartPos = 0;
        }
        else
        {
            StartPos += WriteSize;
        }
    }

    /* Update the global with the new read location */
    console->ReadPos = StartPos;

} /* end OS_ConsoleOutput_Impl */
