/*
 * state_machine.cpp
 *
 *  Created on: 13/09/2017
 *      Author: michaelmcadam
 */

#include <stdlib.h>
#include "app/state_machine.h"
#include "debug/DEBUG.h"

StateMachine::StateMachine(state_e i_initState) {
    for (int i = 0; i < MAX_STATES; i++) {
    	for (int j = 0; j < MAX_EVENTS; j++) {
    		event_table[i][j] = &StateMachine::error_handler;
    		transition_table[i][j] = ERROR;
    	}
    }
    _currentState = i_initState;
}


void StateMachine::error_handler() {
	DEBUG("Error - cannot make this transition");
}


void StateMachine::registerTransition(	state_e startState,
										state_e endState,
										event_e event,
										state_event_handler_t event_handler)
{
	event_table[startState][event] = event_handler;
	transition_table[startState][event] = endState;
}


void StateMachine::transition(event_e event) {
  if (transition_table[_currentState][event] != IGNORED) {
    if (event_table[_currentState][event] != NULL) {
      event_table[_currentState][event]();
    }
    _currentState = transition_table[_currentState][event];
  }
}


uint8_t StateMachine::getCurrentState()
{
  return _currentState;
}
