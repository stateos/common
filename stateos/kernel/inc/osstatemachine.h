/******************************************************************************

    @file    StateOS: osstatemachine.h
    @author  Rajmund Szymanski
    @date    18.11.2025
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
#include "oseventqueue.h"

/**
 * @defgroup statemachine Hierarchical state machine
 * @brief    hierarchical state machine
 * @{
 */

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

/**
 * @brief   hierarchical state machine - state
 */

typedef struct __hsm_state hsm_state_t;

/**
 * @brief   hierarchical state machine - state action
 */

typedef struct __hsm_action hsm_action_t;

/**
 * @brief   hierarchical state machine
 */

typedef struct __hsm hsm_t;

/**
 * @brief   hierarchical state machine - event handler definition
 */

typedef void hsm_handler_t(hsm_t *, unsigned);

/**
 * @brief   hierarchical state machine - state definition
 */

struct __hsm_state
{
	hsm_state_t *   parent;  ///< pointer to parent state in the hsm tree
	hsm_action_t *  queue;   ///< state action queue
};

typedef struct __hsm_state hsm_state_id [];

/**
 * @brief   hierarchical state machine - state action definition
 *
 *          each state can handle the following system events:
 *          - hsmExit  (exit from the state during transition)
 *          - hsmEntry (entry to the state during transition)
 *          - hsmInit  (init the state after transition)
 *
 *          each state should also handle the necessary user events
 *
 *          user event values must be greater than or equal to hsmUser
 *
 *          values less than hsmUser are reserved for the system
 *
 *          transition is possible, if:
 *          - handling user events or
 *          - handling hsmInit event and transition to child state
 */

struct __hsm_action
{
	hsm_state_t *   owner;   ///< action owner
	unsigned        event;   ///< event value
	hsm_state_t *   target;  ///< pointer to transition target state (NULL, if no transition)
	hsm_handler_t * handler; ///< event handler (NULL, if no handler)
	hsm_action_t *  next;    ///< next element in the hsm state action queue
};

typedef struct __hsm_action hsm_action_id [];

/**
 * @brief   hierarchical state machine - definition
 */

struct __hsm
{
	evq_t           evq;     ///< event queue
	hsm_state_t *   state;   ///< current hsm state
	hsm_action_t *  action;  ///< current hsm state action
	tsk_t *         tsk;     ///< hsm event dispatcher task
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

/**
 * @brief   define and initialize an hsm state object
 *
 * @param   state      name of a pointer to hsm state object
 * @param   parent     pointer to parent state in the hsm tree (NULL for root)
 */

#define             OS_HSM_STATE( state, parent ) \
                       hsm_state_t state[] = { _HSM_STATE_INIT( parent ) }

/** static alias of OS_HSM_STATE() */
#define         static_HSM_STATE( state, parent ) \
                static hsm_state_t state[] = { _HSM_STATE_INIT( parent ) }

/**
 * @brief   create and initialize hsm state object
 *
 * @param   parent     pointer to parent state in the hsm tree (NULL for root)
 *
 * @return  hsm state object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                HSM_STATE_INIT( parent ) \
                      _HSM_STATE_INIT( parent )
#endif

/**
 * @brief   create and initialize hsm state object
 *
 * @param   parent     pointer to parent state in the hsm tree (NULL for root)
 *
 * @return  hsm state object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                HSM_STATE_CREATE( parent ) \
                     { HSM_STATE_INIT  ( parent ) }
/** alias of HSM_STATE_CREATE() */
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

/**
 * @brief   define and initialize an hsm action object
 *
 * @param   action     name of a pointer to hsm action object
 * @param   owner      action owner
 * @param   event      event value
 * @param   target     pointer to transition target state (NULL, if no transition)
 * @param   handler    event handler
 */

#define             OS_HSM_ACTION( action, owner, event, target, handler ) \
                       hsm_action_t action[] = { _HSM_ACTION_INIT( owner, event, target, handler ) }

/** static alias of OS_HSM_ACTION() */
#define         static_HSM_ACTION( action, owner, event, target, handler ) \
                static hsm_action_t action[] = { _HSM_ACTION_INIT( owner, event, target, handler ) }

/**
 * @brief   create and initialize an hsm action object
 *
 * @param   owner      action owner
 * @param   event      event value
 * @param   target     pointer to transition target state (NULL, if no transition)
 * @param   handler    event handler
 *
 * @return  hsm action object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                HSM_ACTION_INIT( owner, event, target, handler ) \
                      _HSM_ACTION_INIT( owner, event, target, handler )
#endif

/**
 * @brief   create and initialize an hsm action object
 *
 * @param   owner      action owner
 * @param   event      event value
 * @param   target     pointer to transition target state (NULL, if no transition)
 * @param   handler    event handler
 *
 * @return  hsm action object as array
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                HSM_ACTION_CREATE( owner, event, target, handler ) \
                     { HSM_ACTION_INIT  ( owner, event, target, handler ) }
/** alias of HSM_ACTION_CREATE() */
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
                    { _EVQ_INIT( _limit, _data ), NULL, NULL, NULL }

/**
 * @brief   define and initialize complete hsm object
 *
 * @param   hsm        name of a pointer to hsm object
 * @param   limit      size of an hsm event queue (max number of stored events)
 */

#define             OS_HSM( hsm, limit )          \
                static unsigned hsm##__buf[limit]; \
                       hsm_t hsm[] = { _HSM_INIT( limit, hsm##__buf ) }

/** static alias of OS_HSM() */
#define         static_HSM( hsm, limit )          \
                static unsigned hsm##__buf[limit]; \
                static hsm_t hsm[] = { _HSM_INIT( limit, hsm##__buf ) }

/**
 * @brief   create and initialize complete hsm object
 *
 * @param   limit      size of an hsm event queue (max number of stored events)
 *
 * @return  hsm object
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                HSM_INIT( limit ) \
                      _HSM_INIT( limit, _EVQ_DATA( limit ) )
#endif

/**
 * @brief   create and initialize complete hsm object
 *
 * @param   limit      size of an hsm event queue (max number of stored events)
 *
 * @return  hsm object as array (id)
 *
 * @note    use only in 'C' code
 */

#ifndef __cplusplus
#define                HSM_CREATE( limit ) \
                     { HSM_INIT  ( limit ) }
/** alias of HSM_CREATE() */
#define                HSM_NEW \
                       HSM_CREATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   initialize hsm object
 *
 * @param   state      pointer to hsm state object
 * @param   parent     pointer to parent state in the hsm tree (NULL for root)
 */

void hsm_initState( hsm_state_t *state, hsm_state_t *parent );

/**
 * @brief   initialize hsm state action object
 *
 * @param   action     pointer to hsm state action object
 * @param   owner      action owner
 * @param   event      event value
 * @param   target     pointer to transition target state (NULL, if no transition)
 * @param   handler    event handler
 */

void hsm_initAction( hsm_action_t *action, hsm_state_t *owner, unsigned event, hsm_state_t *target, hsm_handler_t *handler );

/**
 * @brief   initialize hsm object
 *
 * @param   hsm        pointer to hsm object
 * @param   data       hsm event queue data buffer
 * @param   bufsize    size of the buffer
 */

void hsm_init( hsm_t *hsm, void *data, size_t bufsize );

/**
 * @brief   create and initialize a new hsm object
 *
 * @param   limit      size of an hsm event queue (max number of stored events)
 *
 * @return  pointer to hsm object
 * @retval  NULL       object not created (not enough free memory)
 *
 * @note    use only in thread mode
 */

hsm_t *hsm_create( unsigned limit );

/** alias of hsm_create() */
__STATIC_INLINE
hsm_t *hsm_new( unsigned limit ) { return hsm_create(limit); }

/**
 * @brief   link hsm state action to the state action queue
 *
 * @param   action     pointer to hsm state action object
 */

void hsm_link( hsm_action_t *action );

/**
 * @brief   start hsm event dispatcher
 *
 * @param   hsm        pointer to hsm object
 * @param   tsk        pointer to hsm dispatcher
 * @param   initState  pointer to initial hsm state
 */

void hsm_start( hsm_t *hsm, tsk_t *tsk, hsm_state_t *initState );

#if OS_ATOMICS
/** Async alias of hsm_start() */
void hsm_startAsync( hsm_t *hsm, tsk_t *tsk, hsm_state_t *initState );
#endif

/**
 * @brief   reset the hsm object, stop the hsm event dispatcher
 *          and wake up all waiting tasks with 'E_STOPPED' event value
 *
 * @param   hsm        pointer to hsm object
 *
 * @note    use only in thread mode
 */

void hsm_reset( hsm_t *hsm );

/** alias of hsm_reset() */
__STATIC_INLINE
void hsm_kill( hsm_t *hsm ) { hsm_reset(hsm); }

/**
 * @brief   reset the hsm object, stop the hsm event dispatcher,
 *          wake up all waiting tasks with 'E_DELETED' event value and free allocated resource
 *
 * @param   hsm        pointer to hsm object
 *
 * @note    use only in thread mode
 */

void hsm_destroy( hsm_t *hsm );

/** alias of hsm_destroy() */
__STATIC_INLINE
void hsm_delete( hsm_t *hsm ) { hsm_destroy(hsm); }

/**
 * @brief   try to transfer event data to the hsm event queue,
 *          don't wait if the hsm event queue is full
 *
 * @param   hsm        pointer to hsm object
 * @param   event      event value
 *
 * @retval  E_SUCCESS  event data was successfully transferred to the hsm object
 * @retval  E_TIMEOUT  hsm event queue is full, try again
 *
 * @note    can be used in both thread and handler mode
 * @note    use ISR alias in blockable interrupt handlers
 * @note    use Async alias for communication with unmasked interrupt handlers
 */

int hsm_give( hsm_t *hsm, unsigned event );

/** ISR alias of hsm_give() */
__STATIC_INLINE
int hsm_giveISR( hsm_t *hsm, unsigned event ) { return hsm_give(hsm, event); }

#if OS_ATOMICS
/** Async alias of hsm_give() */
int hsm_giveAsync( hsm_t *hsm, unsigned event );
#endif

/**
 * @brief   try to transfer event value to the hsm event queue,
 *          wait for given duration of time while the hsm event queue is full
 *
 * @param   hsm        pointer to hsm object
 * @param   event      event value
 * @param   delay      duration of time (maximum number of ticks to wait while the hsm event queue is full)
 *                     - IMMEDIATE: don't wait if the hsm event queue is full
 *                     - INFINITE:  wait indefinitely while the hsm event queue is full
 *
 * @retval  E_SUCCESS  event value was successfully transferred to the hsm object
 * @retval  E_STOPPED  hsm object was reseted before the specified timeout expired
 * @retval  E_DELETED  hsm object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  hsm event queue is full and was not issued data before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int hsm_sendFor( hsm_t *hsm, unsigned event, cnt_t delay );

/**
 * @brief   try to transfer event value to the hsm event queue,
 *          wait until given timepoint while the hsm event queue is full
 *
 * @param   hsm        pointer to hsm object
 * @param   event      event value
 * @param   time       timepoint value
 *
 * @retval  E_SUCCESS  event value was successfully transferred to the hsm object
 * @retval  E_STOPPED  hsm object was reseted before the specified timeout expired
 * @retval  E_DELETED  hsm object was deleted before the specified timeout expired
 * @retval  E_TIMEOUT  hsm event queue is full and was not issued data before the specified timeout expired
 *
 * @note    use only in thread mode
 */

int hsm_sendUntil( hsm_t *hsm, unsigned event, cnt_t time );

/**
 * @brief   try to transfer event data to the hsm event queue,
 *          wait indefinitely while the hsm event queue is full
 *
 * @param   hsm        pointer to hsm object
 * @param   event      event value
 *
 * @retval  E_SUCCESS  event data was successfully transferred to the hsm object
 * @retval  E_STOPPED  hsm object was reseted (unavailable for async version)
 * @retval  E_DELETED  hsm object was deleted (unavailable for async version)
 *
 * @note    use only in thread mode
 * @note    use Async alias for communication with unmasked interrupt handlers
 * @note    the Async alias is intended for tasks of equal priority: it waits in a loop with tsk_yield,
 *          lower priority tasks do not run meanwhile
 */

__STATIC_INLINE
int hsm_send( hsm_t *hsm, unsigned event ) { return hsm_sendFor(hsm, event, INFINITE); }

#if OS_ATOMICS
/** Async alias of hsm_send() */
int hsm_sendAsync( hsm_t *hsm, unsigned event );
#endif

/**
 * @brief   transfer event data to the hsm event queue,
 *          remove the oldest event data if the hsm event queue is full
 *
 * @param   hsm        pointer to hsm object
 * @param   event      event value
 */

void hsm_push( hsm_t *hsm, unsigned event );

/** ISR alias of hsm_push() */
__STATIC_INLINE
void hsm_pushISR( hsm_t *hsm, unsigned event ) { hsm_push(hsm, event); }

/**
 * @brief   get current hsm state
 *
 * @param   hsm        pointer to hsm object
 *
 * @return  pointer to current hsm state
 */

hsm_state_t *hsm_getState( hsm_t *hsm );

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#if defined(__cplusplus) && (__cplusplus >= 201402L) && !defined(__STATEOS_STDCXX)

#include <iterator>
#include <vector>

namespace stateos {

/**
 * @brief   create and initialize an hsm state object
 */

struct State : public __hsm_state
{
	/// constructor, see the constructor parameters of the class
	constexpr
	State():             __hsm_state _HSM_STATE_INIT( nullptr) {}
	template<class S> constexpr
	/// constructor, see the constructor parameters of the class
	State( S& _parent ): __hsm_state _HSM_STATE_INIT(&_parent) {}

	/// move constructor
	State( State&& _src ): __hsm_state(_src)
	{
		assert(_src.queue == nullptr);
	}

	State( const State& ) = delete;
	State& operator=( State&& ) = delete;
	State& operator=( const State& ) = delete;
};

template<unsigned> struct StateMachineT; // forward declaration

/**
 * @brief   create and initialize an hsm action object
 */

struct Action : public __hsm_action
{
	template<class S> constexpr
	/// constructor, see the constructor parameters of the class
	Action( S& _owner, unsigned _event,             hsm_handler_t *_handler = nullptr ): __hsm_action _HSM_ACTION_INIT(&_owner, _event,  nullptr, _handler) {}
	template<class S> constexpr
	/// constructor, see the constructor parameters of the class
	Action( S& _owner, unsigned _event, S& _target, hsm_handler_t *_handler = nullptr ): __hsm_action _HSM_ACTION_INIT(&_owner, _event, &_target, _handler) {}
#if __cplusplus >= 201402L
	template<class S, class F> constexpr
	/// constructor, see the constructor parameters of the class
	Action( S& _owner, unsigned _event,             F&& _handler ): __hsm_action _HSM_ACTION_INIT(&_owner, _event,  nullptr, handler_), handler{_handler} {}
	template<class S, class F> constexpr
	/// constructor, see the constructor parameters of the class
	Action( S& _owner, unsigned _event, S& _target, F&& _handler ): __hsm_action _HSM_ACTION_INIT(&_owner, _event, &_target, handler_), handler{_handler} {}
#endif

	/// move constructor
	Action( Action&& ) = default;
	/// copy constructor
	Action( const Action& ) = default;
	Action& operator=( Action&& ) = delete;
	Action& operator=( const Action& ) = delete;

#if __cplusplus >= 201402L
	static
	void handler_( hsm_t *_hsm, unsigned _event ) { static_cast<Action*>(_hsm->action)->handler(_hsm, _event); }
	std::function<void( hsm_t *, unsigned )> handler; ///< handler of the action (function object)
#endif

	private:

	void link() { hsm_link(this); }

	template<unsigned>
	friend struct StateMachineT;
};

/**
 * @brief   create and initialize a hierarchical state machine object
 *
 * @par     Constructor parameters
 *          - limit: size of an event queue (max number of stored events)
 */

template<unsigned limit_ = 1>
struct StateMachineT : public __hsm
{
	/// constructor, see the constructor parameters of the class
	StateMachineT():                                  __hsm _HSM_INIT(limit_, data_), tab_{}     {}
	/// constructor, see the constructor parameters of the class
	StateMachineT( const std::vector<Action>& _tab ): __hsm _HSM_INIT(limit_, data_), tab_{_tab} {}

	/// move constructor
	StateMachineT( StateMachineT&& _src ): __hsm _HSM_INIT(limit_, data_), tab_(std::move(_src.tab_))
	{
		assert(_src.state == nullptr && _src.evq.count == 0 && _src.evq.obj.queue == nullptr);
		(void) _src;
	}

	StateMachineT( const StateMachineT& ) = delete;
	StateMachineT& operator=( StateMachineT&& ) = delete;
	StateMachineT& operator=( const StateMachineT& ) = delete;

	/// add an action (parameters of the Action constructor) to the table of the state machine
	template<class S, typename... A>
	void          add       ( S& _owner, unsigned _event, A&&... _args ) { tab_.emplace_back(_owner, _event, std::forward<A>(_args)...); }
	/// add the actions to the table of the state machine
	void          add       ( const std::vector<Action>& _tab )   {        std::copy(std::begin(_tab), std::end(_tab), std::back_inserter(tab_)); }
	/// link the actions of the table and start the state machine (hsm_start())
	void          start     ( tsk_t& _task, hsm_state_t& _init )  {        for (auto& _action: tab_) _action.link();
	                                                                       hsm_start     (this, &_task, &_init); }
	void          reset     ()                                    {        hsm_reset     (this); }                                ///< hsm_reset()
	void          kill      ()                                    {        hsm_kill      (this); }                                ///< hsm_kill()
	void          destroy   ()                                    {        hsm_destroy   (this); }                                ///< hsm_destroy()
	int           give      ( unsigned _event )                   { return hsm_give      (this,  _event); }                       ///< hsm_give()
	int           giveISR   ( unsigned _event )                   { return hsm_giveISR   (this,  _event); }                       ///< hsm_giveISR()
	template<typename T>
	int           sendFor   ( unsigned _event, const T& _delay )  { return hsm_sendFor  (this,   _event, Clock::count(_delay)); } ///< hsm_sendFor()
	template<typename T>
	int           sendUntil ( unsigned _event, const T& _time )   { return hsm_sendUntil(this,   _event, Clock::until(_time)); }  ///< hsm_sendUntil()
	int           send      ( unsigned _event )                   { return hsm_send      (this,  _event); }                       ///< hsm_send()
	void          push      ( unsigned _event )                   {        hsm_push      (this,  _event); }                       ///< hsm_push()
	void          pushISR   ( unsigned _event )                   {        hsm_pushISR   (this,  _event); }                       ///< hsm_pushISR()
	hsm_state_t * getState  ()                                    { return hsm_getState  (this); }                                ///< hsm_getState()
#if OS_ATOMICS
	/// link the actions of the table and start the state machine (hsm_startAsync())
	void          startAsync( tsk_t& _task, hsm_state_t& _init )  {        for (auto& _action: tab_) _action.link();
	                                                                       hsm_startAsync(this, &_task, &_init); }
	int           giveAsync ( unsigned _event )                   { return hsm_giveAsync (this,  _event); } ///< hsm_giveAsync()
	int           sendAsync ( unsigned _event )                   { return hsm_sendAsync (this,  _event); } ///< hsm_sendAsync()
#endif

#if __cplusplus >= 201402L
	using Ptr = std::unique_ptr<StateMachineT<limit_>>; ///< result type of Create()
#else
	using Ptr = StateMachineT<limit_> *; ///< result type of Create()
#endif

/**
 * @brief   create dynamic object with manageable resources
 *
 * @par     Parameters
 *          - limit: size of an event queue (max number of stored events)
 *
 * @return  std::unique_pointer / pointer to StateMachineT<> object
 *
 * @note    use only in thread mode
 */

	static
	Ptr Create()
	{
		auto hsm = new (std::nothrow) StateMachineT<limit_>();
		if (hsm != nullptr)
			hsm->__hsm::evq.obj.res = hsm;
		return Ptr(hsm);
	}

	/** @overload */
	static
	Ptr Create( const std::vector<Action>& _tab )
	{
		auto hsm = new (std::nothrow) StateMachineT<limit_>(_tab);
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

/** @} */

#endif//__STATEOS_HSM_H
