
/*
 * device_state.cpp
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

#include "./device_state.h"

#define NRF_LOG_MODULE_NAME DEVICE_STATE
NRF_LOG_MODULE_REGISTER();


namespace DEVICE
{

namespace STATE
{

static StateMachine deviceStateMachine;

bool g_initialised = false;


///////////////////////////////////////////////////
//////        Initialisation functions      ///////
///////////////////////////////////////////////////



void createTransitionTable(void)
{
  /*                                      Start state,        End state,          Triggered by,               Transition handler */
  deviceStateMachine.registerTransition(WAIT_STATE,         EVENT_BUFFER_STATE, TRAP_TRIGGERED_EVENT,      DETECTOR_TRIGGERED);
  deviceStateMachine.registerTransition(EVENT_BUFFER_STATE, DETECT_MOVE_STATE,  TRAP_BUFFER_END_EVENT,     DETECTOR_BUFFER_ENDED);
  deviceStateMachine.registerTransition(DETECT_MOVE_STATE,  WAIT_STATE,         MOVEMENT_BUFFER_END_EVENT, DETECTED_KILL);
  //deviceStateMachine.registerTransition(DETECT_MOVE_STATE,  MOVING_STATE,       TRAP_TRIGGERED_EVENT,      DETECTED_MOVEMENT);
  //deviceStateMachine.registerTransition(MOVING_STATE,       MOVING_STATE,       TRAP_TRIGGERED_EVENT,      DETECTED_MOVEMENT);
  //deviceStateMachine.registerTransition(MOVING_STATE,       WAIT_STATE,         SET_BUFFER_END_EVENT,      DETECTED_SET);


}

void start()
{
  INFO("STARTING - Detector State Machine");
  if (false == g_initialised)
  {
    createTransitionTable();
    g_initialised = true;
  }
  deviceStateMachine.start(WAIT_STATE);
}

void stop()
{
  INFO("STOPPING - Detector State Machine");
  EVENTS::eventPut(TRAP_STATE_CHANGE_EVENT);
  deviceStateMachine.stop();
}



}
}
