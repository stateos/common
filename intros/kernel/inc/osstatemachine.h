/******************************************************************************

    @file    IntrOS: osstatemachine.h
    @author  Rajmund Szymanski
    @date    25.07.2022
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
#include "oseventqueue.h"

/* -------------------------------------------------------------------------- */

enum
{
	hsmALL = 0,// the state action applies to all events
	hsmStop,   // stop the state machine
	hsmExit,   // exit from the state during transition
	hsmEntry,  // entry to the state during transition
	hsmInit,   // init the state after transition
	hsmUser,   // values less than hsmUser are reserved for the system
};

/******************************************************************************
 *
 * Name              : hierarchical state machine - state
 *
 ******************************************************************************/

typedef struct __hsm_state hsm_state_t, * const hsm_state_id;

/******************************************************************************
 *
 * Name              : hierarchical state machine - state action
 *
 ******************************************************************************/

typedef struct __hsm_action hsm_action_t, * const hsm_action_id;

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
 ******************************************************************************/

typedef void hsm_handler_t(hsm_t *, unsigned);

/******************************************************************************
 *
 * Name              : hierarchical state machine - state definition
 *
 ******************************************************************************/

struct __hsm_state
{
	hsm_state_t *  parent;  // pointer to parent state in the hsm tree
	hsm_action_t * queue;   // state action queue
};

/******************************************************************************
 *
 * Name              : hierarchical state machine - state action definition
 *
 * Description       : each state can handle the following system events:
 *                     - hsmExit  (exit from the state during transition)
 *                     - hsmEntry (entry to the state during transition)
 *                     - hsmInit  (init the state after transition)
 *                     each state should also handle the necessary user events
 *                     user event values must be greater than or equal to hsmUser
 *                     values less than hsmUser are reserved for the system
 *                     transition is possible, if:
 *                     - handling user events or
 *                     - handling hsmInit event and transition to child state
 *
 ******************************************************************************/

struct __hsm_action
{
	hsm_state_t *   owner;   // action owner
	unsigned        event;   // event value
	hsm_state_t *   target;  // pointer to transition target state (NULL, if no transition)
	hsm_handler_t * handler; // event handler (NULL, if no handler)
	hsm_action_t *  next;    // next element in the hsm state action queue
};

/******************************************************************************
 *
 * Name              : hierarchical state machine - definition
 *
 ******************************************************************************/

struct __hsm
{
	evq_t         evq;   // event queue
	hsm_state_t * state; // current hsm state
};

/******************************************************************************
 *
 * Name              : _HSM_STATE_INIT
 *
 * Description       : create and initialize a hsm state object
 *
 * Parameters
 *   parent          : pointer to parent state in the hsm tree (NULL for root)
 *
 * Return            : hsm state object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _HSM_STATE_INIT( _parent ) \
                    { _parent, NULL }

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
 *
 ******************************************************************************/

#define             OS_HSM_STATE( state, parent )                                 \
                       hsm_state_t state##__hsm_state = _HSM_STATE_INIT( parent ); \
                       hsm_state_id state = & state##__hsm_state

#define         static_HSM_STATE( state, parent )                                 \
                static hsm_state_t state##__hsm_state = _HSM_STATE_INIT( parent ); \
                static hsm_state_id state = & state##__hsm_state

/******************************************************************************
 *
 * Name              : HSM_STATE_INIT
 *
 * Description       : create and initialize hsm state object
 *
 * Parameters
 *   parent          : pointer to parent state in the hsm tree (NULL for root)
 *
 * Return            : hsm state object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                HSM_STATE_INIT( parent ) \
                      _HSM_STATE_INIT( parent )
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
 *
 * Return            : pointer to hsm state object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                HSM_STATE_CREATE( parent ) \
     (hsm_state_t[]) { HSM_STATE_INIT  ( parent ) }
#define                HSM_STATE_NEW \
                       HSM_STATE_CREATE
#endif

/******************************************************************************
 *
 * Name              : _HSM_ACTION_INIT
 *
 * Description       : create and initialize a hsm transition object
 *
 * Parameters
 *   owner           : action owner
 *   event           : event value
 *   target          : pointer to transition target state (NULL, if no transition)
 *   handler         : event handler
 *
 * Return            : hsm transition object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _HSM_ACTION_INIT( _owner, _event, _target, _handler ) \
                    { _owner, _event, _target, _handler, NULL }

/******************************************************************************
 *
 * Name              : OS_HSM_ACTION
 * Static alias      : static_HSM_ACTION
 *
 * Description       : define and initialize a hsm transition object
 *
 * Parameters
 *   owner           : action owner
 *   event           : event value
 *   target          : pointer to transition target state (NULL, if no transition)
 *   handler         : event handler
 *
 ******************************************************************************/

#define             OS_HSM_ACTION( owner, event, target, handler )                                        \
                       hsm_action_t action##__hsm_action = _HSM_ACTION_INIT( owner, event, target, handler ); \
                       hsm_action_id action = & action##__hsm_action

#define         static_HSM_ACTION( owner, event, target, handler )                                        \
                static hsm_action_t action##__hsm_action = _HSM_ACTION_INIT( owner, event, target, handler ); \
                static hsm_action_id action = & action##__hsm_action

/******************************************************************************
 *
 * Name              : HSM_ACTION_INIT
 *
 * Description       : create and initialize hsm transition object
 *
 * Parameters
 *   owner           : action owner
 *   event           : event value
 *   target          : pointer to transition target state (NULL, if no transition)
 *   handler         : event handler
 *
 * Return            : hsm transition object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                HSM_ACTION_INIT( owner, event, target, handler ) \
                      _HSM_ACTION_INIT( owner, event, target, handler )
#endif

/******************************************************************************
 *
 * Name              : HSM_ACTION_CREATE
 * Alias             : HSM_ACTION_NEW
 *
 * Description       : create and initialize hsm transition object
 *
 * Parameters
 *   owner           : action owner
 *   event           : event value
 *   target          : pointer to transition target state (NULL, if no transition)
 *   handler         : event handler
 *
 * Return            : pointer to hsm transition object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                HSM_ACTION_CREATE( owner, event, target, handler ) \
     (hsm_action_t[]) { HSM_ACTION_INIT  ( owner, event, target, handler ) }
#define                HSM_ACTION_NEW \
                       HSM_ACTION_CREATE
#endif

/******************************************************************************
 *
 * Name              : _HSM_INIT
 *
 * Description       : create and initialize a hsm object
 *
 * Parameters
 *   limit           : size of a hsm event queue (max number of stored events)
 *   data            : hsm event queue data buffer
 *
 * Return            : hsm object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _HSM_INIT( _limit, _data ) \
                    { _EVQ_INIT( _limit, _data ), NULL }

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
 *
 ******************************************************************************/

#define             OS_HSM( hsm, limit )                                \
                       unsigned hsm##__buf[limit];                       \
                       hsm_t hsm##__hsm = _HSM_INIT( limit, hsm##__buf ); \
                       hsm_id hsm = & hsm##__hsm

#define         static_HSM( hsm, limit )                                \
                static unsigned hsm##__buf[limit];                       \
                static hsm_t hsm##__hsm = _HSM_INIT( limit, hsm##__buf ); \
                static hsm_id hsm = & hsm##__hsm

/******************************************************************************
 *
 * Name              : HSM_INIT
 *
 * Description       : create and initialize complete hsm object
 *
 * Parameters
 *   limit           : size of a hsm event queue (max number of stored events)
 *
 * Return            : hsm object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                HSM_INIT( limit ) \
                      _HSM_INIT( limit, _EVQ_DATA( limit ) )
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
 *
 * Return            : pointer to hsm object
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                HSM_CREATE( limit ) \
           (hsm_t[]) { HSM_INIT  ( limit ) }
#define                HSM_NEW \
                       HSM_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : hsm_initState
 *
 * Description       : initialize hsm object
 *
 * Parameters
 *   state           : pointer to hsm state object
 *   parent          : pointer to parent state in the hsm tree (NULL for root)
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_initState( hsm_state_t *state, hsm_state_t *parent );

/******************************************************************************
 *
 * Name              : hsm_initAction
 *
 * Description       : initialize hsm state action object
 *
 * Parameters
 *   action          : pointer to hsm state action object
 *   owner           : action owner
 *   event           : event value
 *   target          : pointer to transition target state (NULL, if no transition)
 *   handler         : event handler
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_initAction( hsm_action_t *action, hsm_state_t *owner, unsigned event, hsm_state_t *target, hsm_handler_t *handler );

/******************************************************************************
 *
 * Name              : hsm_init
 *
 * Description       : initialize hsm object
 *
 * Parameters
 *   hsm             : pointer to hsm object
 *   data            : hsm event queue data buffer
 *   bufsize         : size of the buffer
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_init( hsm_t *hsm, void *data, size_t bufsize );

/******************************************************************************
 *
 * Name              : hsm_link
 *
 * Description       : link hsm state action to the state action queue
 *
 * Parameters
 *   hsm             : pointer to hsm object
 *   action          : pointer to hsm state action object
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_link( hsm_t *hsm, hsm_action_t *action );

/******************************************************************************
 *
 * Name              : hsm_start
 * Async alias       : hsm_startAsync
 *
 * Description       : start hsm event dispatcher
 *
 *   hsm             : pointer to hsm object
 *   tsk             : pointer to hsm dispatcher
 *   initState       : pointer to initial hsm state
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_start( hsm_t *hsm, tsk_t *tsk, hsm_state_t *initState );

#if OS_ATOMICS
__STATIC_INLINE
void hsm_startAsync( hsm_t *hsm, tsk_t *tsk, hsm_state_t *initState ) { hsm_start(hsm, tsk, initState); }
#endif

/******************************************************************************
 *
 * Name              : hsm_join
 *
 * Description       : delay execution of the current job until hsm stop
 *
 * Parameters
 *   hsm             : pointer to hsm object
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_join( hsm_t *hsm );

/******************************************************************************
 *
 * Name              : hsm_give
 * Async alias       : hsm_giveAsync
 *
 * Description       : try to transfer event data to the hsm event queue,
 *                     don't wait if the hsm event queue is full
 *
 *   hsm             : pointer to hsm object
 *   event           : event value
 *
 * Return
 *   SUCCESS         : event data was successfully transferred to the hsm event queue
 *   FAILURE         : hsm event queue is full
 *
 ******************************************************************************/

unsigned hsm_give( hsm_t *hsm, unsigned event );

#if OS_ATOMICS
__STATIC_INLINE
unsigned hsm_giveAsync( hsm_t *hsm, unsigned event ) { return hsm_give(hsm, event); }
#endif

/******************************************************************************
 *
 * Name              : hsm_send
 * Async alias       : hsm_sendAsync
 *
 * Description       : try to transfer event data to the hsm event queue,
 *                     wait indefinitely while the hsm event queue is full
 *
 *   hsm             : pointer to hsm object
 *   event           : event value
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_send( hsm_t *hsm, unsigned event );

#if OS_ATOMICS
__STATIC_INLINE
void hsm_sendAsync( hsm_t *hsm, unsigned event ) { hsm_send(hsm, event); }
#endif

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
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_push( hsm_t *hsm, unsigned event );

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
hsm_state_t* hsm_getState( hsm_t *hsm ) { return hsm->state; }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
namespace intros {

/******************************************************************************
 *
 * Class             : Action
 *
 * Description       : create and initialize a hsm transition object
 *
 * Constructor parameters
 *                   : none
 *
 ******************************************************************************/

struct Action : public __hsm_action
{
	constexpr
	Action( hsm_state_t& _owner, unsigned _event,                       hsm_handler_t *_handler = nullptr ): __hsm_action _HSM_ACTION_INIT(&_owner, _event,  nullptr, _handler) {}
	constexpr
	Action( hsm_state_t& _owner, unsigned _event, hsm_state_t& _target, hsm_handler_t *_handler = nullptr ): __hsm_action _HSM_ACTION_INIT(&_owner, _event, &_target, _handler) {}

	Action( Action&& ) = default;
	Action( const Action& ) = default;
	Action& operator=( Action&& ) = delete;
	Action& operator=( const Action& ) = delete;
};

/******************************************************************************
 *
 * Class             : State
 *
 * Description       : create and initialize a hsm state object
 *
 * Constructor parameters
 *                   : none
 *
 ******************************************************************************/

struct State : public __hsm_state
{
	constexpr
	State():             __hsm_state _HSM_STATE_INIT( nullptr) {}
	template<class S> constexpr
	State( S& _parent ): __hsm_state _HSM_STATE_INIT(&_parent) {}

	State( State&& ) = default;
	State( const State& ) = delete;
	State& operator=( State&& ) = delete;
	State& operator=( const State& ) = delete;
};

/******************************************************************************
 *
 * Class             : StateMachineT<>
 *
 * Description       : create and initialize a hierarchical state machine object
 *
 * Constructor parameters
 *   limit           : size of an event queue (max number of stored events)
 *
 ******************************************************************************/

template<unsigned limit_>
struct StateMachineT : public __hsm
{
	constexpr
	StateMachineT(): __hsm _HSM_INIT(limit_, data_) {}

	StateMachineT( StateMachineT&& ) = default;
	StateMachineT( const StateMachineT& ) = delete;
	StateMachineT& operator=( StateMachineT&& ) = delete;
	StateMachineT& operator=( const StateMachineT& ) = delete;

	void          link      ( hsm_action_t& _action )            {        hsm_link      (this, &_action); }
	void          start     ( tsk_t& _task, hsm_state_t& _init ) {        hsm_start     (this, &_task, &_init); }
	void          join      ()                                   {        hsm_join      (this); }
	unsigned      give      ( unsigned _event )                  { return hsm_give      (this,  _event); }
	void          send      ( unsigned _event )                  {        hsm_send      (this,  _event); }
	void          push      ( unsigned _event )                  {        hsm_push      (this,  _event); }
	hsm_state_t * getState  ()                                   { return hsm_getState  (this); }
#if OS_ATOMICS
	void          startAsync( tsk_t& _task, hsm_state_t& _init ) {        hsm_startAsync(this, &_task, &_init); }
	unsigned      giveAsync ( unsigned _event )                  { return hsm_giveAsync (this,  _event); }
	void          sendAsync ( unsigned _event )                  {        hsm_sendAsync (this,  _event); }
#endif

	private:
	unsigned data_[limit_];
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__INTROS_HSM_H
