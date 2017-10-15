
/*
 * state_machine.h
 *
 *  Created on: 21/09/2017
 *      Author: michaelmcadam
 */

#ifndef _GOODNATURE_APP_STATE_MACHINE_H
#define _GOODNATURE_APP_STATE_MACHINE_H

#include <stdint.h>

typedef enum {
	WAIT_STATE,
	EVENT_BUFFER_STATE,
	DETECT_MOVE_STATE,
	ANIMAL_KILLED_STATE,
	MOVING_STATE,
	MAX_STATES,
	IGNORED,
	ERROR
} state_e;


typedef enum {
	TRIGGERED_EVENT,
	BUFFER_END_EVENT,
	ANIMAL_KILLED_EVENT,
  MOVEMENT_BUFFER_END_EVENT,
	SET_BUFFER_END_EVENT,
	MAX_EVENTS
} event_e;


typedef void (*state_event_handler_t)(void);



class StateMachine
{
public:
  StateMachine(state_e i_initState);
	void transition(event_e event);
	void registerTransition(state_e startState,
							state_e endState,
							event_e event,
							state_event_handler_t event_handler);

  uint8_t getCurrentState();

private:
    unsigned char _currentState;

    state_event_handler_t event_table[MAX_STATES][MAX_EVENTS];
    state_e transition_table[MAX_STATES][MAX_EVENTS];

    static void error_handler(void);

};

#endif //_GOODNATURE_APP_STATE_MACHINE_H
