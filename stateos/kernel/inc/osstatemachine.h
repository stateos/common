/******************************************************************************

    @file    StateOS: osstatemachine.h
    @author  Rajmund Szymanski
    @date    31.07.2022
    @brief   This file contains definitions for StateOS.

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

#ifndef __STATEOS_HSM_H
#define __STATEOS_HSM_H

#include "oskernel.h"
#include "osclock.h"
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

typedef struct __hsm_state hsm_state_t;

/******************************************************************************
 *
 * Name              : hierarchical state machine - state action
 *
 ******************************************************************************/

typedef struct __hsm_action hsm_action_t;

/******************************************************************************
 *
 * Name              : hierarchical state machine
 *
 ******************************************************************************/

typedef struct __hsm hsm_t;

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
	hsm_state_t *   parent;  // pointer to parent state in the hsm tree
	hsm_action_t *  queue;   // state action queue
};

typedef struct __hsm_state hsm_state_id [];

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

typedef struct __hsm_action hsm_action_id [];

/******************************************************************************
 *
 * Name              : hierarchical state machine - definition
 *
 ******************************************************************************/

struct __hsm
{
	evq_t           evq;     // event queue
	hsm_state_t *   state;   // current hsm state
	hsm_action_t *  action;  // current hsm state action
};

typedef struct __hsm hsm_id [];

/******************************************************************************
 *
 * Name              : _HSM_STATE_INIT
 *
 * Description       : create and initialize an hsm state object
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
 * Description       : define and initialize an hsm state object
 *
 * Parameters
 *   state           : name of a pointer to hsm state object
 *   parent          : pointer to parent state in the hsm tree (NULL for root)
 *
 ******************************************************************************/

#define             OS_HSM_STATE( state, parent ) \
                       hsm_state_t state[] = { _HSM_STATE_INIT( parent ) }

#define         static_HSM_STATE( state, parent ) \
                static hsm_state_t state[] = { _HSM_STATE_INIT( parent ) }

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
 * Return            : hsm state object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                HSM_STATE_CREATE( parent ) \
                     { HSM_STATE_INIT  ( parent ) }
#define                HSM_STATE_NEW \
                       HSM_STATE_CREATE
#endif

/******************************************************************************
 *
 * Name              : _HSM_ACTION_INIT
 *
 * Description       : create and initialize an hsm action object
 *
 * Parameters
 *   owner           : action owner
 *   event           : event value
 *   target          : pointer to transition target state (NULL, if no transition)
 *   handler         : event handler
 *
 * Return            : hsm action object
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
 * Description       : define and initialize an hsm action object
 *
 * Parameters
 *   owner           : action owner
 *   event           : event value
 *   target          : pointer to transition target state (NULL, if no transition)
 *   handler         : event handler
 *
 ******************************************************************************/

#define             OS_HSM_ACTION( owner, event, target, handler ) \
                       hsm_action_t action[] = { _HSM_ACTION_INIT( owner, event, target, handler ) }

#define         static_HSM_ACTION( owner, event, target, handler ) \
                static hsm_action_t action[] = { _HSM_ACTION_INIT( owner, event, target, handler ) }

/******************************************************************************
 *
 * Name              : HSM_ACTION_INIT
 *
 * Description       : create and initialize an hsm action object
 *
 * Parameters
 *   owner           : action owner
 *   event           : event value
 *   target          : pointer to transition target state (NULL, if no transition)
 *   handler         : event handler
 *
 * Return            : hsm action object
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
 * Description       : create and initialize an hsm action object
 *
 * Parameters
 *   owner           : action owner
 *   event           : event value
 *   target          : pointer to transition target state (NULL, if no transition)
 *   handler         : event handler
 *
 * Return            : hsm action object as array
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                HSM_ACTION_CREATE( owner, event, target, handler ) \
                     { HSM_ACTION_INIT  ( owner, event, target, handler ) }
#define                HSM_ACTION_NEW \
                       HSM_ACTION_CREATE
#endif

/******************************************************************************
 *
 * Name              : _HSM_INIT
 *
 * Description       : create and initialize an hsm object
 *
 * Parameters
 *   limit           : size of an hsm event queue (max number of stored events)
 *   data            : hsm event queue data buffer
 *
 * Return            : hsm object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _HSM_INIT( _limit, _data ) \
                    { _EVQ_INIT( _limit, _data ), NULL, NULL }

/******************************************************************************
 *
 * Name              : OS_HSM
 * Static alias      : static_HSM
 *
 * Description       : define and initialize complete hsm object
 *
 * Parameters
 *   hsm             : name of a pointer to hsm object
 *   limit           : size of an hsm event queue (max number of stored events)
 *
 ******************************************************************************/

#define             OS_HSM( hsm, limit )          \
                static unsigned hsm##__buf[limit]; \
                       hsm_t hsm[] = { _HSM_INIT( limit, hsm##__buf ) }

#define         static_HSM( hsm, limit )          \
                static unsigned hsm##__buf[limit]; \
                static hsm_t hsm[] = { _HSM_INIT( limit, hsm##__buf ) }

/******************************************************************************
 *
 * Name              : HSM_INIT
 *
 * Description       : create and initialize complete hsm object
 *
 * Parameters
 *   limit           : size of an hsm event queue (max number of stored events)
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
 *   limit           : size of an hsm event queue (max number of stored events)
 *
 * Return            : hsm object as array (id)
 *
 * Note              : use only in 'C' code
 *
 ******************************************************************************/

#ifndef __cplusplus
#define                HSM_CREATE( limit ) \
                     { HSM_INIT  ( limit ) }
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
 * Name              : hsm_create
 * Alias             : hsm_new
 *
 * Description       : create and initialize a new hsm object
 *
 * Parameters
 *   limit           : size of an hsm event queue (max number of stored events)
 *
 * Return            : pointer to hsm object
 *   NULL            : object not created (not enough free memory)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

hsm_t *hsm_create( unsigned limit );

__STATIC_INLINE
hsm_t *hsm_new( unsigned limit ) { return hsm_create(limit); }

/******************************************************************************
 *
 * Name              : hsm_link
 *
 * Description       : link hsm state action to the state action queue
 *
 * Parameters
 *   action          : pointer to hsm state action object
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_link( hsm_action_t *action );

/******************************************************************************
 *
 * Name              : hsm_start
 * Async alias       : hsm_startAsync
 *
 * Description       : start hsm event dispatcher
 *
 * Parameters
 *   hsm             : pointer to hsm object
 *   tsk             : pointer to hsm dispatcher
 *   initState       : pointer to initial hsm state
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_start( hsm_t *hsm, tsk_t *tsk, hsm_state_t *initState );

#if OS_ATOMICS
void hsm_startAsync( hsm_t *hsm, tsk_t *tsk, hsm_state_t *initState );
#endif

/******************************************************************************
 *
 * Name              : hsm_reset
 * Alias             : hsm_kill
 *
 * Description       : reset the hsm object and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * Parameters
 *   hsm             : pointer to hsm object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void hsm_reset( hsm_t *hsm );

__STATIC_INLINE
void hsm_kill( hsm_t *hsm ) { hsm_reset(hsm); }

/******************************************************************************
 *
 * Name              : hsm_destroy
 * Alias             : hsm_delete
 *
 * Description       : reset the hsm object, wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * Parameters
 *   hsm             : pointer to hsm object
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

void hsm_destroy( hsm_t *hsm );

__STATIC_INLINE
void hsm_delete( hsm_t *hsm ) { hsm_destroy(hsm); }

/******************************************************************************
 *
 * Name              : hsm_give
 * ISR alias         : hsm_giveISR
 * Async alias       : hsm_giveAsync
 *
 * Description       : try to transfer event data to the hsm event queue,
 *                     don't wait if the hsm event queue is full
 *
 * Parameters
 *   hsm             : pointer to hsm object
 *   event           : event value
 *
 * Return
 *   E_SUCCESS       : event data was successfully transferred to the hsm object
 *   E_TIMEOUT       : hsm event queue is full, try again
 *
 * Note              : can be used in both thread and handler mode
 *                     use ISR alias in blockable interrupt handlers
 *                     use Async alias for communication with unmasked interrupt handlers
 *
 ******************************************************************************/

int hsm_give( hsm_t *hsm, unsigned event );

__STATIC_INLINE
int hsm_giveISR( hsm_t *hsm, unsigned event ) { return hsm_give(hsm, event); }

#if OS_ATOMICS
int hsm_giveAsync( hsm_t *hsm, unsigned event );
#endif

/******************************************************************************
 *
 * Name              : hsm_sendFor
 *
 * Description       : try to transfer event value to the hsm event queue,
 *                     wait for given duration of time while the hsm event queue is full
 *
 * Parameters
 *   hsm             : pointer to hsm object
 *   event           : event value
 *   delay           : duration of time (maximum number of ticks to wait while the hsm event queue is full)
 *                     IMMEDIATE: don't wait if the hsm event queue is full
 *                     INFINITE:  wait indefinitely while the hsm event queue is full
 *
 * Return
 *   E_SUCCESS       : event value was successfully transferred to the hsm object
 *   E_STOPPED       : hsm object was reseted before the specified timeout expired
 *   E_DELETED       : hsm object was deleted before the specified timeout expired
 *   E_TIMEOUT       : hsm event queue is full and was not issued data before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int hsm_sendFor( hsm_t *hsm, unsigned event, cnt_t delay );

/******************************************************************************
 *
 * Name              : hsm_sendUntil
 *
 * Description       : try to transfer event value to the hsm event queue,
 *                     wait until given timepoint while the hsm event queue is full
 *
 * Parameters
 *   hsm             : pointer to hsm object
 *   event           : event value
 *   time            : timepoint value
 *
 * Return
 *   E_SUCCESS       : event value was successfully transferred to the hsm object
 *   E_STOPPED       : hsm object was reseted before the specified timeout expired
 *   E_DELETED       : hsm object was deleted before the specified timeout expired
 *   E_TIMEOUT       : hsm event queue is full and was not issued data before the specified timeout expired
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

int hsm_sendUntil( hsm_t *hsm, unsigned event, cnt_t time );

/******************************************************************************
 *
 * Name              : hsm_send
 * Async alias       : hsm_sendAsync
 *
 * Description       : try to transfer event data to the hsm event queue,
 *                     wait indefinitely while the hsm event queue is full
 *
 * Parameters
 *   hsm             : pointer to hsm object
 *   event           : event value
 *
 * Return
 *   E_SUCCESS       : event data was successfully transferred to the hsm object
 *   E_STOPPED       : hsm object was reseted (unavailable for async version)
 *   E_DELETED       : hsm object was deleted (unavailable for async version)
 *
 * Note              : use only in thread mode
 *                     use Async alias for communication with unmasked interrupt handlers
 *
 ******************************************************************************/

__STATIC_INLINE
int hsm_send( hsm_t *hsm, unsigned event ) { return hsm_sendFor(hsm, event, INFINITE); }

#if OS_ATOMICS
int hsm_sendAsync( hsm_t *hsm, unsigned event );
#endif

/******************************************************************************
 *
 * Name              : hsm_push
 * ISR alias         : hsm_pushISR
 *
 * Description       : transfer event data to the hsm event queue,
 *                     remove the oldest event data if the hsm event queue is full
 *
 * Parameters
 *   hsm             : pointer to hsm object
 *   event           : event value
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_push( hsm_t *hsm, unsigned event );

__STATIC_INLINE
void hsm_pushISR( hsm_t *hsm, unsigned event ) { hsm_push(hsm, event); }

/******************************************************************************
 *
 * Name              : hsm_getState
 *
 * Description       : get current hsm state
 *
 * Parameters
 *   hsm             : pointer to hsm object
 *
 * Return            : pointer to current hsm state
 *
 ******************************************************************************/

hsm_state_t *hsm_getState( hsm_t *hsm );

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201103L) && !defined(_GLIBCXX_HAS_GTHREADS)

#include <iterator>
#include <vector>

namespace stateos {

/******************************************************************************
 *
 * Class             : State
 *
 * Description       : create and initialize an hsm state object
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

template<unsigned> struct StateMachineT; // forward declaration

/******************************************************************************
 *
 * Class             : Action
 *
 * Description       : create and initialize an hsm action object
 *
 * Constructor parameters
 *                   : none
 *
 ******************************************************************************/

struct Action : public __hsm_action
{
	template<class S> constexpr
	Action( S& _owner, unsigned _event,             hsm_handler_t *_handler = nullptr ): __hsm_action _HSM_ACTION_INIT(&_owner, _event,  nullptr, _handler) {}
	template<class S> constexpr
	Action( S& _owner, unsigned _event, S& _target, hsm_handler_t *_handler = nullptr ): __hsm_action _HSM_ACTION_INIT(&_owner, _event, &_target, _handler) {}
#if __cplusplus >= 201402L
	template<class S, class F> constexpr
	Action( S& _owner, unsigned _event,             F&& _handler ): __hsm_action _HSM_ACTION_INIT(&_owner, _event,  nullptr, handler_), handler{_handler} {}
	template<class S, class F> constexpr
	Action( S& _owner, unsigned _event, S& _target, F&& _handler ): __hsm_action _HSM_ACTION_INIT(&_owner, _event, &_target, handler_), handler{_handler} {}
#endif

	Action( Action&& ) = default;
	Action( const Action& ) = default;
	Action& operator=( Action&& ) = delete;
	Action& operator=( const Action& ) = delete;

#if __cplusplus >= 201402L
	static
	void handler_( hsm_t *_hsm, unsigned _event ) { static_cast<Action*>(_hsm->action)->handler(_hsm, _event); }
	std::function<void( hsm_t *, unsigned )> handler;
#endif

	private:

	void link() { hsm_link(this); }

	template<unsigned>
	friend struct StateMachineT;
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

template<unsigned limit_ = 1>
struct StateMachineT : public __hsm
{
	StateMachineT():                                  __hsm _HSM_INIT(limit_, data_), tab_{}     {}
	StateMachineT( const std::vector<Action>& _tab ): __hsm _HSM_INIT(limit_, data_), tab_{_tab} {}

	StateMachineT( StateMachineT&& ) = default;
	StateMachineT( const StateMachineT& ) = delete;
	StateMachineT& operator=( StateMachineT&& ) = delete;
	StateMachineT& operator=( const StateMachineT& ) = delete;

	template<class S, typename... A>
	void          add       ( S& _owner, unsigned _event, A&&... _args ) { tab_.emplace_back(_owner, _event, std::forward<A>(_args)...); }
	void          add       ( const std::vector<Action>& _tab )   {        std::copy(std::begin(_tab), std::end(_tab), std::back_inserter(tab_)); }
	void          start     ( tsk_t& _task, hsm_state_t& _init )  {        for (auto& _action: tab_) _action.link();
	                                                                       hsm_start     (this, &_task, &_init); }
	void          reset     ()                                    {        hsm_reset     (this); }
	void          kill      ()                                    {        hsm_kill      (this); }
	void          destroy   ()                                    {        hsm_destroy   (this); }
	int           give      ( unsigned _event )                   { return hsm_give      (this,  _event); }
	int           giveISR   ( unsigned _event )                   { return hsm_giveISR   (this,  _event); }
	template<typename T>
	int           sendFor   ( unsigned _event, const T& _delay )  { return hsm_sendFor  (this,   _event, Clock::count(_delay)); }
	template<typename T>
	int           sendUntil ( unsigned _event, const T& _time )   { return hsm_sendUntil(this,   _event, Clock::until(_time)); }
	int           send      ( unsigned _event )                   { return hsm_send      (this,  _event); }
	void          push      ( unsigned _event )                   {        hsm_push      (this,  _event); }
	void          pushISR   ( unsigned _event )                   {        hsm_pushISR   (this,  _event); }
	hsm_state_t * getState  ()                                    { return hsm_getState  (this); }
#if OS_ATOMICS
	void          startAsync( tsk_t& _task, hsm_state_t& _init )  {        hsm_startAsync(this, &_task, &_init); }
	unsigned      giveAsync ( unsigned _event )                   { return hsm_giveAsync (this,  _event); }
	void          sendAsync ( unsigned _event )                   {        hsm_sendAsync (this,  _event); }
#endif

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<EventQueueT<limit_>>;
#else
	using Ptr = EventQueueT<limit_> *;
#endif

/******************************************************************************
 *
 * Name              : StateMachineT<>::Create
 *
 * Description       : create dynamic object with manageable resources
 *
 * Parameters
 *   limit           : size of an event queue (max number of stored events)
 *
 * Return            : std::unique_pointer / pointer to StateMachineT<> object
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

	static
	Ptr Create()
	{
		auto hsm = new StateMachineT<limit_>();
		if (hsm != nullptr)
			hsm->__hsm::evq.obj.res = hsm;
		return Ptr(hsm);
	}

	private:

	unsigned            data_[limit_];
	std::vector<Action> tab_;
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__STATEOS_HSM_H
