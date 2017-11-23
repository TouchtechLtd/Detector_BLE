
/*
 * trap_manager.cpp
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#include <stdint.h>
#include <string.h>

#include "app/trap_manager.h"
#include "app/state_machine.h"
#include "app/events.h"
#include "peripheral/LIS2DH12.h"
#include "peripheral/flash_interface.h"
#include "debug/DEBUG.h"


#define NRF_LOG_MODULE_NAME TRAP_STATE
NRF_LOG_MODULE_REGISTER();

namespace TrapState
{
static StateMachine detectorStateMachine(WAIT_STATE, MAX_STATES, MAX_EVENTS);

Timer movementCountdown;
Timer trapBufferCountdown;
Timer moveBufferCountdown;

static trap_detector_config_t detectorConfig = {
    TRAP_TRIGGER_EVENT_THRESHOLD,             // Trigger Threshold
    TRAP_TRIGGER_MOVE_THRESHOLD,              // Move Threshold
    TRAP_TRIGGER_DURATION,                    // Trigger Duration
    0,                                        // Move Duration
    TRAP_EVENT_BUFFER_MS,                     // Trigger buffer length
    MOVE_BUFFER_MS,                           // Move buffer length
    SET_BUFFER_MS                             // Set Buffer Length
};




///////////////////////////////////////////////////
//////           Getter functions       ///////////
///////////////////////////////////////////////////

const trap_detector_config_t* getConfig()
{
  return &detectorConfig;
}


void setConfig(trap_detector_config_t inputConfig)
{
  detectorConfig = inputConfig;
  INFO("UPDATING - Detector Configuration Updated");
  INFO("UPDATING - Trigger Threshold: %d",  detectorConfig.triggerThreshold);
  INFO("UPDATING - Move Threshold: %d",     detectorConfig.moveThreshold);
  INFO("UPDATING - Trigger Duration: %d",   detectorConfig.triggerDuration);
  INFO("UPDATING - Move Duration: %d",      detectorConfig.moveDuration);
  INFO("UPDATING - Trigger Buffer: %d",     detectorConfig.triggerBufferLength);
  INFO("UPDATING - Move Buffer: %d",        detectorConfig.moveBufferLength);
  INFO("UPDATING - Set Buffer: %d",         detectorConfig.setBufferLength);
}

detector_state_e getState()
{
  return static_cast<detector_state_e>(detectorStateMachine.getCurrentState());
}


///////////////////////////////////////////////////
//////           Timer handlers         ///////////
///////////////////////////////////////////////////


void trapBufferCountdownHandler(void* p_context)
{
  detectorStateMachine.transition(BUFFER_END_EVENT);
}

void moveBufferCountdownHandler(void* p_context)
{
  detectorStateMachine.transition(MOVEMENT_BUFFER_END_EVENT);
}

void movementCountdownHandler(void* p_context)
{
  detectorStateMachine.transition(SET_BUFFER_END_EVENT);
}


///////////////////////////////////////////////////
//////        Transition functions      ///////////
///////////////////////////////////////////////////


void triggeredFromWaitTransition()
{
  EVENTS::eventPut(TRAP_TRIGGERED_EVENT);
  EVENTS::eventPut(TRAP_STATE_CHANGE_EVENT);

  INFO("TRANSITION - Wait to Trap Buffer");
  trapBufferCountdown.startCountdown(detectorConfig.triggerBufferLength, trapBufferCountdownHandler);
}

void trapBufferEndedTransition()
{
  EVENTS::eventPut(TRAP_STATE_CHANGE_EVENT);

  INFO("TRANSITION - Trap Buffer to Move Buffer");
  moveBufferCountdown.startCountdown(detectorConfig.moveBufferLength, moveBufferCountdownHandler);
  LIS2DH12::setInterruptThreshold(detectorConfig.moveThreshold);
}

void moveBufferEndedTransition()
{
  EVENTS::eventPut(TRAP_KILLED_EVENT);
  EVENTS::eventPut(TRAP_STATE_CHANGE_EVENT);

  INFO("TRANSITION - Move Buffer to Wait");
  LIS2DH12::setInterruptThreshold(detectorConfig.triggerThreshold);
}

void triggeredFromMoveTransition()
{
  EVENTS::eventPut(TRAP_MOVING_EVENT);
  EVENTS::eventPut(TRAP_STATE_CHANGE_EVENT);

  INFO("TRANSITION - Move Buffer to Set Buffer");
  movementCountdown.stopTimer();
  movementCountdown.startCountdown(detectorConfig.setBufferLength, movementCountdownHandler);
}

void moveToWaitTransition()
{
  EVENTS::eventPut(TRAP_STATE_CHANGE_EVENT);
  EVENTS::eventPut(TRAP_SET_EVENT);

  INFO("TRANSITION - Set Buffer to Wait");
  LIS2DH12::setInterruptThreshold(detectorConfig.triggerThreshold);
}

///////////////////////////////////////////////////
//////        Event Handler functions      ////////
///////////////////////////////////////////////////

void accTriggeredHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  LIS2DH12::clearInterrupts();
  INFO("Event: Accelerometer Triggered");
  detectorStateMachine.transition(TRIGGERED_EVENT);

}

void simulateTrigger()
{
  accTriggeredHandler(0, NRF_GPIOTE_POLARITY_LOTOHI);
}

///////////////////////////////////////////////////
//////        Initialisation functions      ///////
///////////////////////////////////////////////////



void createTransitionTable(void)
{
  /*                                      Start state,        End state,          Triggered by,               Transition handler */
  detectorStateMachine.registerTransition(WAIT_STATE,         EVENT_BUFFER_STATE, TRIGGERED_EVENT,           &triggeredFromWaitTransition);
  detectorStateMachine.registerTransition(EVENT_BUFFER_STATE, IGNORED,            TRIGGERED_EVENT,           NULL);
  detectorStateMachine.registerTransition(EVENT_BUFFER_STATE, DETECT_MOVE_STATE,  BUFFER_END_EVENT,          &trapBufferEndedTransition);
  detectorStateMachine.registerTransition(DETECT_MOVE_STATE,  WAIT_STATE,         MOVEMENT_BUFFER_END_EVENT, &moveBufferEndedTransition);
  detectorStateMachine.registerTransition(DETECT_MOVE_STATE,  MOVING_STATE,       TRIGGERED_EVENT,           &triggeredFromMoveTransition);
  detectorStateMachine.registerTransition(MOVING_STATE,       MOVING_STATE,       TRIGGERED_EVENT,           &triggeredFromMoveTransition);
  detectorStateMachine.registerTransition(MOVING_STATE,       WAIT_STATE,         SET_BUFFER_END_EVENT,      &moveToWaitTransition);
  detectorStateMachine.registerTransition(MOVING_STATE,       IGNORED,            MOVEMENT_BUFFER_END_EVENT, NULL);

}

void initialise()
{
  LIS2DH12::initThresholdInterrupt(detectorConfig.triggerThreshold, detectorConfig.triggerDuration, LIS2DH12::INTERRUPT_THRESHOLD_XYZ, true, accTriggeredHandler);
  LIS2DH12::clearInterrupts();
  createTransitionTable();
  detectorStateMachine.start(WAIT_STATE);
}

void stop()
{
  detectorStateMachine.stop();
  LIS2DH12::clearInterruptHandler();
}



}
