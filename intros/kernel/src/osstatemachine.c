/******************************************************************************

    @file    IntrOS: osstatemachine.c
    @author  Rajmund Szymanski
    @date    18.07.2022
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
void priv_handleEvent(hsm_t *hsm)
/* -------------------------------------------------------------------------- */
{
	hsm_state_t *state = hsm->state;
	unsigned     event = hsm->event.value;

	assert(event != hsmOK);

	while (event != hsmOK && state != NULL)
	{
		assert(state->handler != NULL);

		event = state->handler(hsm, event);
		state = state->parent;
	}
}

/* -------------------------------------------------------------------------- */
static
void priv_eventDispatcher( void )
/* -------------------------------------------------------------------------- */
{
	void  *tmp = tsk_this(); // because of CSMCC
	hsm_t *hsm = tmp;        //

	for (;;)
	{
		box_wait(&hsm->box, &hsm->event);
		if (hsm->event.value == hsmStop)
			tsk_stop();
		priv_handleEvent(hsm);
	}
}

/* -------------------------------------------------------------------------- */
static
int priv_getStateLevel(hsm_state_t *state)
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
hsm_state_t *priv_getRootState(hsm_t *hsm, hsm_state_t *testState)
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
void priv_setPrevState(hsm_t *hsm)
/* -------------------------------------------------------------------------- */
{
	if (hsm->state != NULL)
		hsm->state = hsm->state->parent;
}

/* -------------------------------------------------------------------------- */
static
void priv_setNextState(hsm_t *hsm, hsm_state_t *state)
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
bool priv_transitionPossible(hsm_t *hsm, hsm_state_t *state)
/* -------------------------------------------------------------------------- */
{
	return (hsm->event.value >= hsmUser ||
	       (hsm->event.value == hsmInit && state != NULL && hsm->state == state->parent));
}

/* -------------------------------------------------------------------------- */
void hsm_transition(hsm_t *hsm, hsm_state_t *nextState)
/* -------------------------------------------------------------------------- */
{
	assert(hsm != NULL);

	if (!priv_transitionPossible(hsm, nextState))
	{
		assert(false);
		return;
	}

	hsm_state_t *rootState = priv_getRootState(hsm, nextState);

	while (hsm->state != rootState)
	{
		assert(hsm->state->handler != NULL);
		hsm->state->handler(hsm, hsmExit);
		priv_setPrevState(hsm);
	}

	while (hsm->state != nextState)
	{
		priv_setNextState(hsm, nextState);
		assert(hsm->state->handler != NULL);
		hsm->state->handler(hsm, hsmEntry);
	}

	if (hsm->state != NULL)
	{
		assert(hsm->state->handler != NULL);
		hsm->state->handler(hsm, hsmInit);
	}
}

/* -------------------------------------------------------------------------- */
void hsm_initEvent( hsm_event_t *event )
/* -------------------------------------------------------------------------- */
{
	assert(event != NULL);

	sys_lock();
	{
		memset(event, 0, sizeof(hsm_event_t));
	}
	sys_unlock();
}


/* -------------------------------------------------------------------------- */
void hsm_initState( hsm_state_t *state, hsm_state_t *parent, hsm_handler_t handler )
/* -------------------------------------------------------------------------- */
{
	assert(state != NULL);
	assert(handler != NULL);

	sys_lock();
	{
		memset(state, 0, sizeof(hsm_state_t));

		state->parent  = parent;
		state->handler = handler;
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void hsm_init( hsm_t *hsm, stk_t *stack, size_t size, void *data, size_t bufsize )
/* -------------------------------------------------------------------------- */
{
	assert(hsm != NULL);

	sys_lock();
	{
		memset(hsm, 0, sizeof(hsm_t));

		wrk_init(&hsm->tsk, NULL, stack, size);
		box_init(&hsm->box, sizeof(hsm_event_t), data, bufsize);
		hsm_initEvent(&hsm->event);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void hsm_start( hsm_t *hsm, hsm_state_t *initState )
/* -------------------------------------------------------------------------- */
{
	assert(hsm != NULL);
	assert(initState != NULL);

	sys_lock();
	{
		hsm->box.count = 0; // reset hsm event queue
		hsm->box.head  = 0; //
		hsm->box.tail  = 0; //
		hsm->state = NULL;  // reset hsm state
		hsm->event.value = hsmInit;
		hsm_transition(hsm, initState);
		tsk_startFrom(&hsm->tsk, priv_eventDispatcher);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
unsigned hsm_give(hsm_t *hsm, unsigned value, void *param)
/* -------------------------------------------------------------------------- */
{
	hsm_event_t event = { value, param };

	return box_give(&hsm->box, &event);
}

/* -------------------------------------------------------------------------- */
void hsm_send(hsm_t *hsm, unsigned value, void *param)
/* -------------------------------------------------------------------------- */
{
	hsm_event_t event = { value, param };

	box_send(&hsm->box, &event);
}

/* -------------------------------------------------------------------------- */
void hsm_push(hsm_t *hsm, unsigned value, void *param)
/* -------------------------------------------------------------------------- */
{
	hsm_event_t event = { value, param };

	box_push(&hsm->box, &event);
}

/* -------------------------------------------------------------------------- */
