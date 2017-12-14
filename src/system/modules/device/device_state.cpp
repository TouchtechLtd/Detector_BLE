
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


uint8_t getCurrentState()
{
  return deviceStateMachine.getCurrentState();
}

void createTransitionTable(void)
{
  /*                                      Start state,    End state,        Triggered by,       Transition handler */
  deviceStateMachine.registerTransition(IDLE_STATE,       ACTIVE_STATE,     ACTIVATE_EVENT,     DEVICE_ACTIVATED);
  deviceStateMachine.registerTransition(ACTIVE_STATE,     IDLE_STATE,       DEACTIVATE_EVENT,   DEVICE_DEACTIVATED);
}

void start()
{
  INFO("STARTING - Device State Machine");
  if (false == g_initialised)
  {
    createTransitionTable();
    g_initialised = true;
  }
  deviceStateMachine.start(IDLE_STATE);
}

void stop()
{
  INFO("STOPPING - Device State Machine");
  deviceStateMachine.stop();
}



}
}
