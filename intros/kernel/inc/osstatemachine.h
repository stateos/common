/******************************************************************************

    @file    IntrOS: osstatemachine.h
    @author  Rajmund Szymanski
    @date    19.07.2022
    @brief   This file contains definitions for IntrOS.

 ******************************************************************************

   Copyright (c) 2018-2022 Rajmund Szymanski. All rights reserved.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.

 ******************************************************************************/

#ifndef __INTROS_HSM_H
#define __INTROS_HSM_H

#include "oskernel.h"
#include "ostask.h"
#include "osmailboxqueue.h"

/* -------------------------------------------------------------------------- */

enum
{
	hsmOK = 0, // event was successfully handled (should be 0)
	hsmStop,   // stop state machine event
	hsmExit,   // exit from state event
	hsmEntry,  // entry to state event
	hsmInit,   // init state after transition event
	hsmUser,   // values less than hsmUser are reserved for the system
};

/******************************************************************************
 *
 * Name              : hierarchical state machine - event
 *
 ******************************************************************************/

typedef struct __hsm_event hsm_event_t, * const hsm_event_id;

/******************************************************************************
 *
 * Name              : hierarchical state machine - state
 *
 ******************************************************************************/

typedef struct __hsm_state hsm_state_t, * const hsm_state_id;

/******************************************************************************
 *
 * Name              : hierarchical state machine
 *
 ******************************************************************************/

typedef struct __hsm hsm_t, * const hsm_id;

/******************************************************************************
 *
 * Name              : hierarchical state machine - event handler definition
 *
 * Description       : function can handle the following system events:
 *                        hsmExit  (transition disabled)
 *                        hsmEntry (transition disabled)
 *                        hsmInit  (transition possible only to child state)
 *                     function should also handle the necessary user events
 *                     user event values must be greater than or equal to hsmUser
 *                     0 is reserved for hsmOK, which confirms event handling
 *                     transition is always possible when handling user events
 *                     transition is not possible when handling hsmExit and hsmEntry events
 *                     when handling hsmInit event, it only possible to transition to child state
 *                     function must return hsmOK if event has been handled
 *                     function should return event value if event was not handled
 *                        then the event value is passed to parent state handler
 *
 ******************************************************************************/

typedef unsigned hsm_handler_t(hsm_t *, unsigned);

/******************************************************************************
 *
 * Name              : hierarchical state machine - event definition
 *
 ******************************************************************************/

struct __hsm_event
{
	unsigned value; // event value
	void   * param; // optional event parameter
};

/******************************************************************************
 *
 * Name              : hierarchical state machine - state definition
 *
 ******************************************************************************/

struct __hsm_state
{
	hsm_state_t   * parent;  // pointer to parent state in the hsm tree
	hsm_handler_t * handler; // event handler
};

/******************************************************************************
 *
 * Name              : hierarchical state machine - definition
 *
 ******************************************************************************/

struct __hsm
{
	tsk_t         tsk;   // hsm dispatcher
	box_t         box;   // event queue
	hsm_state_t * state; // current hsm state
	hsm_event_t   event; // currently handled event
};

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : _HSM_EVENT_INIT
 *
 * Description       : create and initialize a hsm event object
 *
 * Parameters        : none
 *
 * Return            : hsm event object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _HSM_EVENT_INIT() { 0, NULL }

/******************************************************************************
 *
 * Name              : OS_HSM_EVENT
 * Static alias      : static_HSM_EVENT
 *
 * Description       : define and initialize a hsm event object
 *
 * Parameters
 *   event           : name of a pointer to hsm event object
 *
 ******************************************************************************/

#define             OS_HSM_EVENT( event )                                 \
                       hsm_event_t event##__hsm_event = _HSM_EVENT_INIT(); \
                       hsm_event_id event = & event##__hsm_event

#define         static_HSM_EVENT( event )                                 \
                static hsm_event_t event##__hsm_event = _HSM_EVENT_INIT(); \
                static hsm_event_id event = & event##__hsm_event

/******************************************************************************
 *
 * Name              : HSM_EVENT_INIT
 *
 * Description       : create and initialize hsm event object
 *
 * Parameters        : none
 *
 * Return            : hsm event object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                HSM_EVENT_INIT() \
                      _HSM_EVENT_INIT()
#endif

/******************************************************************************
 *
 * Name              : HSM_EVENT_CREATE
 * Alias             : HSM_EVENT_NEW
 *
 * Description       : create and initialize hsm event object
 *
 * Parameters        : none
 *
 * Return            : pointer to hsm event object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                HSM_EVENT_CREATE() \
     (hsm_event_t[]) { HSM_EVENT_INIT  () }
#define                HSM_EVENT_NEW \
                       HSM_EVENT_CREATE
#endif

/******************************************************************************
 *
 * Name              : _HSM_STATE_INIT
 *
 * Description       : create and initialize a hsm state object
 *
 * Parameters
 *   parent          : pointer to parent state in the hsm tree (NULL for root)
 *   handler         : event handler function
 *
 * Return            : hsm state object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _HSM_STATE_INIT( _parent, _handler ) \
                    { _parent, _handler }

/******************************************************************************
 *
 * Name              : OS_HSM_STATE
 * Static alias      : static_HSM_STATE
 *
 * Description       : define and initialize a hsm state object
 *
 * Parameters
 *   state           : name of a pointer to hsm state object
 *   parent          : pointer to parent state in the hsm tree (NULL for root)
 *   handler         : event handler function
 *
 ******************************************************************************/

#define             OS_HSM_STATE( state, parent, handler )                                 \
                       hsm_state_t state##__hsm_state = _HSM_STATE_INIT( parent, handler ); \
                       hsm_state_id state = & state##__hsm_state

#define         static_HSM_STATE( state, parent, handler )                                 \
                static hsm_state_t state##__hsm_state = _HSM_STATE_INIT( parent, handler ); \
                static hsm_state_id state = & state##__hsm_state

/******************************************************************************
 *
 * Name              : HSM_STATE_INIT
 *
 * Description       : create and initialize hsm state object
 *
 * Parameters
 *   parent          : pointer to parent state in the hsm tree (NULL for root)
 *   handler         : event handler function
 *
 * Return            : hsm state object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                HSM_STATE_INIT( parent, handler ) \
                      _HSM_STATE_INIT( parent, handler )
#endif

/******************************************************************************
 *
 * Name              : HSM_STATE_CREATE
 * Alias             : HSM_STATE_NEW
 *
 * Description       : create and initialize hsm state object
 *
 * Parameters
 *   parent          : pointer to parent state in the hsm tree (NULL for root)
 *   handler         : event handler function
 *
 * Return            : pointer to hsm state object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                HSM_STATE_CREATE( parent, handler ) \
     (hsm_state_t[]) { HSM_STATE_INIT  ( parent, handler ) }
#define                HSM_STATE_NEW \
                       HSM_STATE_CREATE
#endif

/******************************************************************************
 *
 * Name              : _HSM_INIT
 *
 * Description       : create and initialize a hsm object
 *
 * Parameters
 *   stack           : base of hsm dispatcher's private stack storage
 *   size            : size of hsm dispatcher private stack (in bytes)
 *   limit           : size of a hsm event queue (max number of stored events)
 *   data            : hsm event queue data buffer
 *
 * Return            : hsm object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _HSM_INIT( _stack, _size, _limit, _data )              \
                    { _TSK_INIT( NULL, _stack, _size ),                       \
                      _BOX_INIT( _limit, sizeof(hsm_event_t), (char *)_data ), \
                       NULL,                                                    \
                      _HSM_EVENT_INIT() }

/******************************************************************************
 *
 * Name              : OS_HSM
 * Static alias      : static_HSM
 *
 * Description       : define and initialize complete hsm object
 *
 * Parameters
 *   hsm             : name of a pointer to hsm object
 *   limit           : size of a hsm event queue (max number of stored events)
 *   size            : (optional) size of hsm dispatcher private stack (in bytes)
 *
 ******************************************************************************/

#define             OS_HSM( hsm, limit, ... )                                                          \
                       hsm_event_t hsm##__buf[limit];                                                   \
                       stk_t hsm##__stk[STK_SIZE( _VA_STK(__VA_ARGS__) )] __STKALIGN;                    \
                       hsm_t hsm##__hsm = _HSM_INIT( hsm##__stk, sizeof(hsm##__stk), limit, hsm##__buf ); \
                       hsm_id hsm = & hsm##__hsm

#define         static_HSM( hsm, limit, ... )                                                          \
                static hsm_event_t hsm##__buf[limit];                                                   \
                static stk_t hsm##__stk[STK_SIZE( _VA_STK(__VA_ARGS__) )] __STKALIGN;                    \
                static hsm_t hsm##__hsm = _HSM_INIT( hsm##__stk, sizeof(hsm##__stk), limit, hsm##__buf ); \
                static hsm_id hsm = & hsm##__hsm

/******************************************************************************
 *
 * Name              : HSM_INIT
 *
 * Description       : create and initialize complete hsm object
 *
 * Parameters
 *   limit           : size of a hsm event queue (max number of stored events)
 *   size            : (optional) size of hsm dispatcher private stack (in bytes)
 *
 * Return            : hsm object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                HSM_INIT( limit, ... )                                                                     \
                      _HSM_INIT( _TSK_STACK( STK_SIZE( _VA_STK(__VA_ARGS__) ) ), STK_OVER( _VA_STK(__VA_ARGS__) ), \
                                 limit, _BOX_DATA( limit, sizeof(hsm_event_t) ) )
#endif

/******************************************************************************
 *
 * Name              : HSM_CREATE
 * Alias             : HSM_NEW
 *
 * Description       : create and initialize complete hsm object
 *
 * Parameters
 *   limit           : size of a hsm event queue (max number of stored events)
 *   size            : (optional) size of hsm dispatcher private stack (in bytes)
 *
 * Return            : pointer to hsm object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                HSM_CREATE( limit, ... ) \
           (hsm_t[]) { HSM_INIT  ( limit, ##__VA_ARGS__ ) }
#define                HSM_NEW \
                       HSM_CREATE
#endif

/******************************************************************************
 *
 * Name              : hsm_transition
 *
 * Description       : make a transition to another state
 *                     transition is always possible when handling the user event
 *                     transition to a child state is possible when handling the init event
 *                     transition is impossible when handling the entry or exit event
 *
 * Parameters
 *   hsm             : pointer to hsm object
 *   nextState       : pointer to new hsm state
 *
 * Return            : none
 *
 * Note              : use only in event handler function
 *
 ******************************************************************************/

void hsm_transition(hsm_t *hsm, hsm_state_t *nextState);

/******************************************************************************
 *
 * Name              : hsm_initEvent
 *
 * Description       : initialize hsm event object
 *
 * Parameters
 *   event           : pointer to hsm event object
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_initEvent(hsm_event_t *event);

/******************************************************************************
 *
 * Name              : hsm_initState
 *
 * Description       : initialize hsm object
 *
 * Parameters
 *   state           : pointer to hsm state object
 *   parent          : pointer to parent state in the hsm tree (NULL for root)
 *   handler         : event handler function
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_initState(hsm_state_t *state, hsm_state_t *parent, hsm_handler_t *handler);

/******************************************************************************
 *
 * Name              : hsm_init
 *
 * Description       : initialize hsm object
 *
 * Parameters
 *   hsm             : pointer to hsm object
 *   stack           : base of hsm dispatcher's private stack storage
 *   size            : size of hsm dispatcher private stack (in bytes)
 *   data            : hsm event queue data buffer
 *   bufsize         : size of the buffer
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_init(hsm_t *hsm, stk_t *stack, size_t size, void *data, size_t bufsize);

/******************************************************************************
 *
 * Name              : hsm_start
 *
 * Description       : start hsm event dispatcher
 *
 *   hsm             : pointer to hsm object
 *   initState       : pointer to initial hsm state
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_start(hsm_t *hsm, hsm_state_t *initState);

/******************************************************************************
 *
 * Name              : hsm_join
 *
 * Description       : delay execution of current task until termination of hsm
 *
 * Parameters
 *   hsm             : pointer to hsm object
 *
 * Return            : none
 *
 ******************************************************************************/
__STATIC_INLINE
void hsm_join(hsm_t *hsm) { tsk_join(&hsm->tsk); }

/******************************************************************************
 *
 * Name              : hsm_give
 *
 * Description       : try to transfer event data to the hsm event queue,
 *                     don't wait if the hsm event queue is full
 *
 *   hsm             : pointer to hsm object
 *   event           : event value
 *   param           : event parameter
 *
 * Return
 *   SUCCESS         : event data was successfully transferred to the hsm event queue
 *   FAILURE         : hsm event queue is full
 *
 ******************************************************************************/

unsigned hsm_give(hsm_t *hsm, unsigned value, void *param);

/******************************************************************************
 *
 * Name              : hsm_send
 *
 * Description       : try to transfer event data to the hsm event queue,
 *                     wait indefinitely while the hsm event queue is full
 *
 *   hsm             : pointer to hsm object
 *   event           : event value
 *   param           : event parameter
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_send(hsm_t *hsm, unsigned value, void *param);

/******************************************************************************
 *
 * Name              : hsm_push
 *
 * Description       : transfer event data to the hsm event queue,
 *                     remove the oldest event data if the event queue is full
 *
 * Parameters
 *   hsm             : pointer to hsm object
 *   event           : event value
 *   param           : event parameter
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_push(hsm_t *hsm, unsigned value, void *param);

/******************************************************************************
 *
 * Name              : hsm_getState
 *
 * Description       : get current hsm state
 *
 *   hsm             : pointer to hsm object
 *
 * Return            : pointer to current hsm state
 *
 ******************************************************************************/
__STATIC_INLINE
hsm_state_t *hsm_getState(hsm_t *hsm) { return hsm->state; }

/******************************************************************************
 *
 * Name              : hsm_getEvent
 *
 * Description       : get value of the currently handled event
 *
 *   hsm             : pointer to hsm object
 *
 * Return            : parameter of the currently handled event
 *
 ******************************************************************************/
__STATIC_INLINE
unsigned hsm_getEvent(hsm_t *hsm) { return hsm->event.value; }

/******************************************************************************
 *
 * Name              : hsm_getParam
 *
 * Description       : get parameter of the currently handled event
 *
 *   hsm             : pointer to hsm object
 *
 * Return            : parameter of the currently handled event
 *
 ******************************************************************************/
__STATIC_INLINE
void *hsm_getParam(hsm_t *hsm) { return hsm->event.param; }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#endif//__INTROS_HSM_H
