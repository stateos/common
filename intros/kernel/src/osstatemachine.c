/******************************************************************************

    @file    IntrOS: osstatemachine.c
    @author  Rajmund Szymanski
    @date    25.07.2022
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
hsm_state_t *priv_getRootState( hsm_t *hsm, hsm_state_t *testState )
/* -------------------------------------------------------------------------- */
{
	hsm_state_t *state = hsm->state;
	int diff = priv_getStateLevel(state)
	         - priv_getStateLevel(testState);

	while (diff-- > 0) state = state->parent;
	while (++diff < 0) testState = testState->parent;
	while (state != testState)
	{
		state = state->parent;
		testState = testState->parent;
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
hsm_action_t* priv_getAction( hsm_state_t *state, unsigned event )
/* -------------------------------------------------------------------------- */
{
	hsm_action_t *action = state->queue;

	while (action != NULL && action->event != event && action->event != hsmALL)
		action = action->next;

	return action;
}

/* -------------------------------------------------------------------------- */
static
void priv_callHandler( hsm_t *hsm, hsm_state_t *state, unsigned event )
/* -------------------------------------------------------------------------- */
{
	hsm->action = priv_getAction(state, event);

	if (hsm->action == NULL)
		return;

	if (hsm->action->handler != NULL)
		hsm->action->handler(hsm, event);
}

/* -------------------------------------------------------------------------- */
static // forward declaration
void priv_transition( hsm_t *hsm, hsm_state_t *nextState );

static
unsigned priv_callAction( hsm_t *hsm, hsm_state_t *state, unsigned event )
/* -------------------------------------------------------------------------- */
{
	hsm->action = priv_getAction(state, event);

	if (hsm->action == NULL)
		return event;

	if (hsm->action->handler != NULL)
		hsm->action->handler(hsm, event);

	if (hsm->action->target != NULL)
	{
		assert(event >= hsmUser || hsm->action->target->parent == state);

		priv_transition(hsm, hsm->action->target);
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
	unsigned event;

	assert(hsm != NULL);

	do
	{
		sys_lock();
		{
			event = evq_wait(&hsm->evq);
			assert(event >= hsmUser || event == hsmStop);
			if (event >= hsmUser)
				priv_eventHandler(hsm, event);
		}
		sys_unlock();
	}
	while (event != hsmStop);

	hsm->state = NULL;
#if OS_TASK_EXIT == 0
	tsk_stop();
#endif
}

/* -------------------------------------------------------------------------- */
void hsm_initState( hsm_state_t *state, hsm_state_t *parent )
/* -------------------------------------------------------------------------- */
{
	assert(state != NULL);

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
	assert(action != NULL);
	assert(owner != NULL);

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
	assert(hsm != NULL);
	assert(data != NULL);

	sys_lock();
	{
		memset(hsm, 0, sizeof(hsm_t));

		evq_init(&hsm->evq, data, bufsize);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void hsm_link( hsm_t *hsm, hsm_action_t *action )
/* -------------------------------------------------------------------------- */
{
	(void) hsm;

	assert(hsm != NULL);
	assert(action != NULL);
	assert(action->owner != NULL);

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
	assert(hsm != NULL);
	assert(hsm->state == NULL);
	assert(tsk != NULL);
	assert(initState != NULL);
	assert(initState->parent == NULL);

	sys_lock();
	{
		if (hsm->state == NULL)
		{
			tsk->arg = hsm;
			hsm->evq.count = 0; // reset hsm event queue
			hsm->evq.head  = 0; //
			hsm->evq.tail  = 0; //
			priv_transition(hsm, initState);
			tsk_startFrom(tsk, priv_eventDispatcher);
		}
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void hsm_join( hsm_t *hsm )
/* -------------------------------------------------------------------------- */
{
	assert(hsm != NULL);

	while (hsm->state != NULL)
		core_ctx_switch();
}

/* -------------------------------------------------------------------------- */
unsigned hsm_give( hsm_t *hsm, unsigned event )
/* -------------------------------------------------------------------------- */
{
	assert(hsm != NULL);

	return evq_give(&hsm->evq, event);
}

/* -------------------------------------------------------------------------- */
void hsm_send( hsm_t *hsm, unsigned event )
/* -------------------------------------------------------------------------- */
{
	assert(hsm != NULL);

	evq_send(&hsm->evq, event);
}

/* -------------------------------------------------------------------------- */
void hsm_push( hsm_t *hsm, unsigned event )
/* -------------------------------------------------------------------------- */
{
	assert(hsm != NULL);

	evq_push(&hsm->evq, event);
}

/* -------------------------------------------------------------------------- */
