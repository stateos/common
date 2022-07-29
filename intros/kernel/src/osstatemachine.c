/******************************************************************************

    @file    IntrOS: osstatemachine.c
    @author  Rajmund Szymanski
    @date    29.07.2022
    @brief   This file provides set of functions for IntrOS.

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

#include "inc/osstatemachine.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
int priv_getStateLevel( hsm_state_t *state )
/* -------------------------------------------------------------------------- */
{
	int level = 0;

	while (state != NULL)
	{
		level++;
		state = state->parent;
	}

	return level;
}

/* -------------------------------------------------------------------------- */
static
hsm_state_t *priv_getRootState( hsm_t *hsm, hsm_state_t *other )
/* -------------------------------------------------------------------------- */
{
	hsm_state_t *state = hsm->state;
	int diff = priv_getStateLevel(state)
	         - priv_getStateLevel(other);

	while (diff-- > 0) state = state->parent;
	while (++diff < 0) other = other->parent;
	while (state != other)
	{
		state = state->parent;
		other = other->parent;
	}

	return state;
}

/* -------------------------------------------------------------------------- */
static
void priv_setPrevState( hsm_t *hsm )
/* -------------------------------------------------------------------------- */
{
	if (hsm->state != NULL)
		hsm->state = hsm->state->parent;
}

/* -------------------------------------------------------------------------- */
static
void priv_setNextState( hsm_t *hsm, hsm_state_t *state )
/* -------------------------------------------------------------------------- */
{
	while (state != NULL)
	{
		if (hsm->state == state->parent)
		{
			hsm->state = state;
			return;
		}
		state = state->parent;
	}
}

/* -------------------------------------------------------------------------- */
static
hsm_action_t* priv_getAction( hsm_t *hsm, hsm_state_t *state, unsigned event )
/* -------------------------------------------------------------------------- */
{
	hsm_action_t *action = state->queue;

	while (action != NULL && action->event != event && action->event != hsmALL)
		action = action->next;

	hsm->action = action;

	return action;
}

/* -------------------------------------------------------------------------- */
static
hsm_action_t *priv_callHandler( hsm_t *hsm, hsm_state_t *state, unsigned event )
/* -------------------------------------------------------------------------- */
{
	hsm_action_t *action = priv_getAction(hsm, state, event);

	if (action != NULL && action->handler != NULL)
		action->handler(hsm, event);

	return action;
}

/* -------------------------------------------------------------------------- */
static // forward declaration
void priv_transition( hsm_t *hsm, hsm_state_t *nextState );

static
unsigned priv_callAction( hsm_t *hsm, hsm_state_t *state, unsigned event )
/* -------------------------------------------------------------------------- */
{
	hsm_action_t *action = priv_callHandler(hsm, state, event);

	if (action == NULL)
		return event;

	if (action->target != NULL)
	{
		assert(event >= hsmUser || action->target->parent == state);

		priv_transition(hsm, action->target);
	}

	return hsmALL;
}

/* -------------------------------------------------------------------------- */
static
void priv_transition( hsm_t *hsm, hsm_state_t *nextState )
/* -------------------------------------------------------------------------- */
{
	hsm_state_t *rootState = priv_getRootState(hsm, nextState);

	while (hsm->state != rootState)
	{
		priv_callHandler(hsm, hsm->state, hsmExit);
		priv_setPrevState(hsm);
	}

	while (hsm->state != nextState)
	{
		priv_setNextState(hsm, nextState);
		priv_callHandler(hsm, hsm->state, hsmEntry);
	}

	priv_callAction(hsm, hsm->state, hsmInit);
}

/* -------------------------------------------------------------------------- */
static
void priv_eventHandler( hsm_t *hsm, unsigned event )
/* -------------------------------------------------------------------------- */
{
	hsm_state_t *state = hsm->state;

	while (state != NULL && event != hsmALL)
	{
		event = priv_callAction(hsm, state, event);
		state = state->parent;
	}
}

/* -------------------------------------------------------------------------- */
static
void priv_eventDispatcher( hsm_t *hsm )
/* -------------------------------------------------------------------------- */
{
	assert(hsm);

	for (;;)
	{
		unsigned event = evq_wait(&hsm->evq);

		if (event == hsmStop)
			break;

		assert(event >= hsmUser);

		sys_lock();
		{
			if (event >= hsmUser)
				priv_eventHandler(hsm, event);
		}
		sys_unlock();
	}

	hsm->state = NULL;
#if OS_TASK_EXIT == 0
	tsk_stop();
#endif
}

/* -------------------------------------------------------------------------- */
void hsm_initState( hsm_state_t *state, hsm_state_t *parent )
/* -------------------------------------------------------------------------- */
{
	assert(state);

	sys_lock();
	{
		memset(state, 0, sizeof(hsm_state_t));

		state->parent = parent;
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void hsm_initAction( hsm_action_t *action, hsm_state_t *owner, unsigned event, hsm_state_t *target, hsm_handler_t *handler )
/* -------------------------------------------------------------------------- */
{
	assert(action);
	assert(owner);

	sys_lock();
	{
		memset(action, 0, sizeof(hsm_action_t));

		action->owner = owner;
		action->event = event;
		action->target = target;
		action->handler = handler;
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void hsm_init( hsm_t *hsm, void *data, size_t bufsize )
/* -------------------------------------------------------------------------- */
{
	assert(hsm);
	assert(data);
	assert(bufsize);

	sys_lock();
	{
		memset(hsm, 0, sizeof(hsm_t));

		evq_init(&hsm->evq, data, bufsize);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void hsm_link( hsm_action_t *action )
/* -------------------------------------------------------------------------- */
{
	assert(action);
	assert(action->owner);

	sys_lock();
	{
		action->next = action->owner->queue;
		action->owner->queue = action;
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void hsm_start( hsm_t *hsm, tsk_t *tsk, hsm_state_t *initState )
/* -------------------------------------------------------------------------- */
{
	assert(hsm);
	assert(hsm->state == NULL);
	assert(tsk);
	assert(initState);
	assert(initState->parent == NULL);

	sys_lock();
	{
		if (hsm->state == NULL)
		{
			hsm->evq.count = 0; // reset hsm event queue
			hsm->evq.head  = 0; //
			hsm->evq.tail  = 0; //
			priv_transition(hsm, initState);
			tsk_startWith(tsk, priv_eventDispatcher, hsm);
		}
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
unsigned hsm_give( hsm_t *hsm, unsigned event )
/* -------------------------------------------------------------------------- */
{
	assert(hsm);
	assert(event >= hsmUser || event == hsmStop);

	return evq_give(&hsm->evq, event);
}

/* -------------------------------------------------------------------------- */
void hsm_send( hsm_t *hsm, unsigned event )
/* -------------------------------------------------------------------------- */
{
	assert(hsm);
	assert(event >= hsmUser || event == hsmStop);

	evq_send(&hsm->evq, event);
}

/* -------------------------------------------------------------------------- */
void hsm_push( hsm_t *hsm, unsigned event )
/* -------------------------------------------------------------------------- */
{
	assert(hsm);
	assert(event >= hsmUser || event == hsmStop);

	evq_push(&hsm->evq, event);
}

/* -------------------------------------------------------------------------- */
