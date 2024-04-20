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
 * \file     bsp_internal.h
 * \ingroup  stateos
 * \author   Rajmund Szymanski
 *
 */

#ifndef BSP_INTERNAL_H
#define BSP_INTERNAL_H

/*
** OSAL includes
*/
#include "bsp-impl.h"
#include "os.h"

/*
** BSP types
*/
typedef struct
{
    mtx_t *AccessMutex;
} OS_BSP_impl_GlobalData_t;

/*
 * Global Data object
 */
extern OS_BSP_impl_GlobalData_t OS_BSP_impl_Global;

#endif /* BSP_INTERNAL_H */
