/******************************************************************************

    @file    IntrOS: osstatemachine.h
    @author  Rajmund Szymanski
    @date    22.07.2022
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
 *                     transition is possible, if:
 *                     - not transition to NULL state and
 *                       - handling user events or
 *                       - handling hsmInit event and transition to child state
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

#ifdef __cplusplus
	void     init();
#endif
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

#ifdef __cplusplus
	void            init( hsm_state_t*, hsm_handler_t* );
#endif
};

/******************************************************************************
 *
 * Name              : hierarchical state machine - definition
 *
 ******************************************************************************/

struct __hsm
{
	box_t         box;   // event queue
	hsm_event_t   event; // currently handled event
	hsm_state_t * state; // current hsm state

#ifdef __cplusplus
	void          transition( hsm_state_t& );
	void          init      ( void*, size_t );
	void          start     ( tsk_t&, hsm_state_t& );
	void          join      ();
	unsigned      give      ( unsigned, void* );
	void          send      ( unsigned, void* );
	void          push      ( unsigned, void* );
	hsm_state_t * getState  ();
	unsigned      getEvent  ();
	void *        getParam  ();
#if OS_ATOMICS
	void          startAsync( tsk_t&, hsm_state_t& );
	unsigned      giveAsync ( unsigned, void* );
	void          sendAsync ( unsigned, void* );
#endif
#endif
};

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
 *   limit           : size of a hsm event queue (max number of stored events)
 *   data            : hsm event queue data buffer
 *
 * Return            : hsm object
 *
 * Note              : for internal use
 *
 ******************************************************************************/

#define               _HSM_INIT( _limit, _data )                      \
                    { _BOX_INIT( _limit, sizeof(hsm_event_t), _data ), \
                      _HSM_EVENT_INIT(), NULL }

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

#define             OS_HSM( hsm, limit )                                        \
                       hsm_event_t hsm##__buf[limit];                            \
                       hsm_t hsm##__hsm = _HSM_INIT( limit, (char *)hsm##__buf ); \
                       hsm_id hsm = & hsm##__hsm

#define         static_HSM( hsm, limit )                                        \
                static hsm_event_t hsm##__buf[limit];                            \
                static hsm_t hsm##__hsm = _HSM_INIT( limit, (char *)hsm##__buf ); \
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
                      _HSM_INIT(limit, _BOX_DATA( limit, sizeof(hsm_event_t) ) )
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

void hsm_transition( hsm_t *hsm, hsm_state_t *nextState );

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

void hsm_initEvent( hsm_event_t *event );

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

void hsm_initState( hsm_state_t *state, hsm_state_t *parent, hsm_handler_t *handler );

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
 *   param           : event parameter
 *
 * Return
 *   SUCCESS         : event data was successfully transferred to the hsm event queue
 *   FAILURE         : hsm event queue is full
 *
 ******************************************************************************/

unsigned hsm_give( hsm_t *hsm, unsigned value, void *param );

#if OS_ATOMICS
__STATIC_INLINE
unsigned hsm_giveAsync( hsm_t *hsm, unsigned value, void *param ) { return hsm_give(hsm, value, param); }
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
 *   param           : event parameter
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_send( hsm_t *hsm, unsigned value, void *param );

#if OS_ATOMICS
__STATIC_INLINE
void hsm_sendAsync( hsm_t *hsm, unsigned value, void *param ) { hsm_send(hsm, value, param); }
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
 *   param           : event parameter
 *
 * Return            : none
 *
 ******************************************************************************/

void hsm_push( hsm_t *hsm, unsigned value, void *param );

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
hsm_state_t *hsm_getState( hsm_t *hsm ) { return hsm->state; }

/******************************************************************************
 *
 * Name              : hsm_getEvent
 *
 * Description       : get value of the currently handled event
 *
 *   hsm             : pointer to hsm object
 *
 * Return            : value of the currently handled event
 *
 ******************************************************************************/
__STATIC_INLINE
unsigned hsm_getEvent( hsm_t *hsm ) { return hsm->event.value; }

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
void *hsm_getParam( hsm_t *hsm ) { return hsm->event.param; }

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus

inline void          __hsm_event::init()                                                {        hsm_initEvent (this); }
inline void          __hsm_state::init( hsm_state_t *_parent, hsm_handler_t *_handler ) {        hsm_initState (this, _parent, _handler); }

inline void          __hsm::transition( hsm_state_t& _next )                            {        hsm_transition(this, &_next); }
inline void          __hsm::init      ( void *_data, size_t _bufsize )                  {        hsm_init      (this,  _data, _bufsize); }
inline void          __hsm::start     ( tsk_t& _task, hsm_state_t& _init )              {        hsm_start     (this, &_task, &_init); }
inline void          __hsm::join      ()                                                {        hsm_join      (this); }
inline unsigned      __hsm::give      ( unsigned _value, void *_param = nullptr )       { return hsm_give      (this, _value, _param); }
inline void          __hsm::send      ( unsigned _value, void *_param = nullptr )       {        hsm_send      (this, _value, _param); }
inline void          __hsm::push      ( unsigned _value, void *_param = nullptr )       {        hsm_push      (this, _value, _param); }
inline hsm_state_t * __hsm::getState  ()                                                { return hsm_getState  (this); }
inline unsigned      __hsm::getEvent  ()                                                { return hsm_getEvent  (this); }
inline void *        __hsm::getParam  ()                                                { return hsm_getParam  (this); }
#if OS_ATOMICS
inline void          __hsm::startAsync( tsk_t& _task, hsm_state_t& _init )              {        hsm_startAsync(this, &_task, &_init); }
inline unsigned      __hsm::giveAsync ( unsigned _value, void *_param = nullptr )       { return hsm_giveAsync (this, _value, _param); }
inline void          __hsm::sendAsync ( unsigned _value, void *_param = nullptr )       {        hsm_sendAsync (this, _value, _param); }
#endif

namespace intros {

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
	State(                 hsm_handler_t *_handler ): __hsm_state _HSM_STATE_INIT( nullptr, _handler) {}
	State( State& _parent, hsm_handler_t *_handler ): __hsm_state _HSM_STATE_INIT(&_parent, _handler) {}
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

	template<class T>
	unsigned give     ( unsigned _value, T& _param ) { return __hsm::give     (_value, &_param); }
	template<class T>
	void     send     ( unsigned _value, T& _param ) {        __hsm::send     (_value, &_param); }
	template<class T>
	void     push     ( unsigned _value, T& _param ) {        __hsm::push     (_value, &_param); }
#if OS_ATOMICS
	template<class T>
	unsigned giveAsync( unsigned _value, T& _param ) { return __hsm::giveAsync(_value, &_param); }
	template<class T>
	void     sendAsync( unsigned _value, T& _param ) {        __hsm::sendAsync(_value, &_param); }
#endif

	private:
	char data_[limit_ * sizeof(hsm_event_t)];
};

}     //  namespace
#endif//__cplusplus

/* -------------------------------------------------------------------------- */

#endif//__INTROS_HSM_H
