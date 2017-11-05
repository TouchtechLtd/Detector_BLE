/*
 * state_machine.cpp
 *
 *  Created on: 13/09/2017
 *      Author: michaelmcadam
 */

#include <stdlib.h>
#include "app/state_machine.h"
#include "debug/DEBUG.h"

StateMachine::StateMachine(state_t i_initState, uint8_t max_states, uint8_t max_events) {
    for (int i = 0; i < max_states; i++) {
    	for (int j = 0; j < max_events; j++) {
    		event_table[i][j] = &StateMachine::error_handler;
    		transition_table[i][j] = ERROR;
    	}
    }
    _currentState = i_initState;
    m_running = true;
}


void StateMachine::error_handler() {
	DEBUG("Error - cannot make this transition");
}


void StateMachine::registerTransition(	state_t startState,
										state_t endState,
										event_e event,
										state_event_handler_t event_handler)
{
	event_table[startState][event] = event_handler;
	transition_table[startState][event] = endState;
}


void StateMachine::transition(event_e event) {
  if (!m_running) { return; }

  if (transition_table[_currentState][event] != IGNORED) {
    if (event_table[_currentState][event] != NULL) {
      event_table[_currentState][event]();
    }
    _currentState = transition_table[_currentState][event];
  }
}

void StateMachine::stop()
{
  m_running = false;
}

void StateMachine::start(state_t state)
{
  _currentState = state;
  m_running = true;
}

bool StateMachine::isRunning()
{
  return m_running;
}

state_t StateMachine::getCurrentState()
{
  return _currentState;
}

