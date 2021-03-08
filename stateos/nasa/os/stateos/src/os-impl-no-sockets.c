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
 * \file   os-impl-no-sockets.c
 * \author joseph.p.hickey@nasa.gov
 *
 */

/****************************************************************************************
                                    INCLUDE FILES
 ***************************************************************************************/

#include "os-shared-sockets.h"

/****************************************************************************************
                                    Socket API
 ***************************************************************************************/

/*----------------------------------------------------------------
 *
 * Function: OS_SocketOpen_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_SocketOpen_Impl(const OS_object_token_t *token)
{
    (void) token;

    return OS_ERR_NOT_IMPLEMENTED;

} /* end OS_SocketOpen_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_SocketBind_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_SocketBind_Impl(const OS_object_token_t *token, const OS_SockAddr_t *Addr)
{
    (void) token;
    (void) Addr;

    return OS_ERR_NOT_IMPLEMENTED;

} /* end OS_SocketBind_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_SocketConnect_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_SocketConnect_Impl(const OS_object_token_t *token, const OS_SockAddr_t *Addr, int32 timeout)
{
    (void) token;
    (void) Addr;
    (void) timeout;

    return OS_ERR_NOT_IMPLEMENTED;

} /* end OS_SocketConnect_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_SocketAccept_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_SocketAccept_Impl(const OS_object_token_t *sock_token, const OS_object_token_t *conn_token, OS_SockAddr_t *Addr, int32 timeout)
{
    (void) sock_token;
    (void) conn_token;
    (void) Addr;
    (void) timeout;

    return OS_ERR_NOT_IMPLEMENTED;

} /* end OS_SocketAccept_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_SocketRecvFrom_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_SocketRecvFrom_Impl(const OS_object_token_t *token, void *buffer, size_t buflen, OS_SockAddr_t *RemoteAddr, int32 timeout)
{
    (void) token;
    (void) buffer;
    (void) buflen;
    (void) RemoteAddr;
    (void) timeout;

    return OS_ERR_NOT_IMPLEMENTED;

} /* end OS_SocketRecvFrom_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_SocketSendTo_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_SocketSendTo_Impl(const OS_object_token_t *token, const void *buffer, size_t buflen, const OS_SockAddr_t *RemoteAddr)
{
    (void) token;
    (void) buffer;
    (void) buflen;
    (void) RemoteAddr;

    return OS_ERR_NOT_IMPLEMENTED;

} /* end OS_SocketSendTo_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_SocketGetInfo_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_SocketGetInfo_Impl(const OS_object_token_t *token, OS_socket_prop_t *sock_prop)
{
    (void) token;
    (void) sock_prop;

    return OS_SUCCESS;

} /* end OS_SocketGetInfo_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_SocketAddrInit_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_SocketAddrInit_Impl(OS_SockAddr_t *Addr, OS_SocketDomain_t Domain)
{
    (void) Addr;
    (void) Domain;

    return OS_ERR_NOT_IMPLEMENTED;

} /* end OS_SocketAddrInit_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_SocketAddrToString_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_SocketAddrToString_Impl(char *buffer, size_t buflen, const OS_SockAddr_t *Addr)
{
    (void) buffer;
    (void) buflen;
    (void) Addr;

    return OS_ERR_NOT_IMPLEMENTED;

} /* end OS_SocketAddrToString_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_SocketAddrFromString_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_SocketAddrFromString_Impl(OS_SockAddr_t *Addr, const char *string)
{
    (void) Addr;
    (void) string;

    return OS_ERR_NOT_IMPLEMENTED;

} /* end OS_SocketAddrFromString_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_SocketAddrGetPort_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_SocketAddrGetPort_Impl(uint16 *PortNum, const OS_SockAddr_t *Addr)
{
    (void) PortNum;
    (void) Addr;

    return OS_ERR_NOT_IMPLEMENTED;

} /* end OS_SocketAddrGetPort_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_SocketAddrSetPort_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_SocketAddrSetPort_Impl(OS_SockAddr_t *Addr, uint16 PortNum)
{
    (void) Addr;
    (void) PortNum;

    return OS_ERR_NOT_IMPLEMENTED;

} /* end OS_SocketAddrSetPort_Impl */
