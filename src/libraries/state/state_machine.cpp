/*
 * state_machine.cpp
 *
 *  Created on: 13/09/2017
 *      Author: michaelmcadam
 */

#include <stdlib.h>
#include "libraries/state/state_machine.h"
#include "libraries/events/events.h"
#include "libraries/debug/DEBUG.h"


int StateMachine::machineCount = 0;
bool StateMachine::isInitialised = false;
StateMachine* StateMachine::allMachines[MAX_NUMBER_MACHINES] = {0};


void StateMachine::addMachine(StateMachine* machine)
{
  if (machineCount < MAX_NUMBER_MACHINES)
  {
    allMachines[machineCount] = machine;
    machineCount++;
  }
  else { INFO("Machine max reached"); }
}

void StateMachine::dispatch(EVENTS::event_signal_t eventSignal)
{
  //INFO("State machine disptach called");
  for (int i = 0; i < machineCount; i++)
  {
    allMachines[i]->transition(eventSignal);
  }
}


StateMachine::StateMachine()
{
  m_running = false;

  if (!isInitialised)
  {
    EVENTS::registerEventRepeater(dispatch);
    isInitialised = true;
  }
}



void StateMachine::error_handler() {
	INFO("ERROR: Cannot make this transition");
}


void StateMachine::registerTransition(	state_t startState,
										state_t endState,
										uint16_t event,
										state_change_event_t transition_event)
{
  state_table[startState].eventLookup[state_table[startState].numberOfEvents].eventID          = event;
  state_table[startState].eventLookup[state_table[startState].numberOfEvents].destinationState = endState;
  state_table[startState].eventLookup[state_table[startState].numberOfEvents].transitionEvent  = transition_event;

  state_table[startState].numberOfEvents++;

}


void StateMachine::transition(EVENTS::event_signal_t eventSignal)
{
  if (!m_running) { return; }

  //INFO("Made it here for event: 0x%04x", eventSignal.eventID);


  for (int i = 0; i < state_table[_currentState].numberOfEvents; i++)
  {
    if (eventSignal.eventID == state_table[_currentState].eventLookup[i].eventID)
    {
      EVENTS::eventPut(state_table[_currentState].eventLookup[i].transitionEvent);
      _currentState = state_table[_currentState].eventLookup[i].destinationState;
      INFO("Current State Now: %d", _currentState);
    }
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
  addMachine(this);
}

bool StateMachine::isRunning()
{
  return m_running;
}

state_t StateMachine::getCurrentState()
{
  return _currentState;
}

