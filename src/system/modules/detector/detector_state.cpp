
/*
 * detector_state.cpp
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#include <stdint.h>
#include <string.h>

#include "libraries/state/state_machine.h"
#include "libraries/events/events.h"
#include "libraries/debug/DEBUG.h"

#include "drivers/timer/timer_interface.h"

#include "./detector_state.h"

#define NRF_LOG_MODULE_NAME DETECTOR_STATE
NRF_LOG_MODULE_REGISTER();


namespace DETECTOR
{

namespace STATE
{

static StateMachine detectorStateMachine;

bool g_initialised = false;


///////////////////////////////////////////////////
//////        Initialisation functions      ///////
///////////////////////////////////////////////////


uint8_t getCurrentState()
{
  return detectorStateMachine.getCurrentState();
}


void createTransitionTable(void)
{
  /*                                      Start state,        End state,          Triggered by,               Transition handler */
  detectorStateMachine.registerTransition(WAIT_STATE,         EVENT_BUFFER_STATE, TRAP_TRIGGERED_EVENT,      DETECTOR_TRIGGERED);
  detectorStateMachine.registerTransition(EVENT_BUFFER_STATE, DETECT_MOVE_STATE,  TRAP_BUFFER_END_EVENT,     DETECTOR_BUFFER_ENDED);
  detectorStateMachine.registerTransition(DETECT_MOVE_STATE,  WAIT_STATE,         MOVEMENT_BUFFER_END_EVENT, DETECTED_KILL);
  //detectorStateMachine.registerTransition(DETECT_MOVE_STATE,  MOVING_STATE,       TRAP_TRIGGERED_EVENT,      DETECTED_MOVEMENT);
  //detectorStateMachine.registerTransition(MOVING_STATE,       MOVING_STATE,       TRAP_TRIGGERED_EVENT,      DETECTED_MOVEMENT);
  //detectorStateMachine.registerTransition(MOVING_STATE,       WAIT_STATE,         SET_BUFFER_END_EVENT,      DETECTED_SET);


}

void start()
{
  INFO("STARTING - Detector State Machine");
  if (false == g_initialised)
  {
    createTransitionTable();
    g_initialised = true;
  }
  detectorStateMachine.start(WAIT_STATE);
}

void stop()
{
  INFO("STOPPING - Detector State Machine");
  //EVENTS::eventPut(TRAP_STATE_CHANGE_EVENT);
  detectorStateMachine.stop();
}



}
}
