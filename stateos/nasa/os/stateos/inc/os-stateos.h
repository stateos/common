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
 * \file     os-stateos.h
 * \ingroup  stateos
 * \author   Rajmund Szymanski
 *
 */

#ifndef OS_STATEOS_H_
#define OS_STATEOS_H_

/****************************************************************************************
                                    COMMON INCLUDE FILES
 ***************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <os.h>

#ifdef  DEBUG
#ifndef OSAL_CONFIG_DEBUG_PRINTF
#define OSAL_CONFIG_DEBUG_PRINTF
#endif//OSAL_CONFIG_DEBUG_PRINTF
#endif//DEBUG

#include "osapi-idmap.h"
#include "os-shared-clock.h"
#include "os-shared-task.h"
#include "os-shared-globaldefs.h"

/****************************************************************************************
                                     DEFINES
 ***************************************************************************************/

#if     OS_FREQUENCY < 1000 || OS_FREQUENCY > 1000000
#error  osconfig.h: Invalid OS_FREQUENCY value! It must be a value between 1000 and 1000000.
#endif
#if     OS_TIMER_SIZE != 64
#error  osconfig.h: Invalid OS_TIMER_SIZE value! It must be a value equal to 64.
#endif

#define OS_GET_OBJECT_ID(PTR, REC, OBJ) \
    ((REC *)((uintptr_t)PTR - offsetof(REC, OBJ)))->id;

/****************************************************************************************
                                INLINE FUNCTIONS
 ***************************************************************************************/

/*----------------------------------------------------------------------------
 * Name: OS_PriorityRemap
 *
 * Purpose: Remaps the OSAL priority into one that is viable for this OS
 *
----------------------------------------------------------------------------*/
static inline unsigned OS_PriorityRemap(osal_priority_t priority)
{
    return (unsigned)(OS_MAX_TASK_PRIORITY - priority);
}

/****************************************************************************************
                                    TYPEDEFS
 ***************************************************************************************/

typedef struct
{
    tsk_t *idle_task;
    int64  time_shift;
} OS_impl_GlobalVars_t;

/****************************************************************************************
                                   GLOBAL DATA
 ***************************************************************************************/

extern OS_impl_GlobalVars_t OS_impl_GlobalVars;

/****************************************************************************************
                         IMPLEMENTATION FUNCTION PROTOTYPES
 ***************************************************************************************/

              int32 OS_TaskAPI_Impl_Init(void);
              int32 OS_QueueAPI_Impl_Init(void);
              int32 OS_CountSemAPI_Impl_Init(void);
              int32 OS_BinSemAPI_Impl_Init(void);
              int32 OS_MutexAPI_Impl_Init(void);
static inline int32 OS_StreamAPI_Impl_Init(void)   { return OS_SUCCESS; }
static inline int32 OS_DirAPI_Impl_Init(void)      { return OS_SUCCESS; }
              int32 OS_TimeBaseAPI_Impl_Init(void);
static inline int32 OS_TimerCbAPI_Impl_Init(void)  { return OS_SUCCESS; }
static inline int32 OS_ModuleAPI_Impl_Init(void)   { return OS_SUCCESS; }
static inline int32 OS_FileSysAPI_Impl_Init(void)  { return OS_SUCCESS; }
              int32 OS_ConsoleAPI_Impl_Init(void);

              int32 OS_TableMutex_Init(osal_objtype_t idtype);

#endif  /* OS_STATEOS_H */
