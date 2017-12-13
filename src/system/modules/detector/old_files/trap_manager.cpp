
/*
 * trap_manager.cpp
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */
/*
#include <stdint.h>
#include <string.h>

#include "./trap_manager.h"
#include "libraries/state/state_machine.h"
#include "libraries/events/events.h"
#include "drivers/LIS2DH12/LIS2DH12.h"
#include "drivers/flash/flash_interface.h"
#include "libraries/debug/DEBUG.h"


#define NRF_LOG_MODULE_NAME TRAP_STATE
NRF_LOG_MODULE_REGISTER();

namespace TrapState
{
static StateMachine detectorStateMachine;

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
  //detectorStateMachine.transition(BUFFER_END_EVENT);
  EVENTS::eventPut(TRAP_BUFFER_END_EVENT);
}

void moveBufferCountdownHandler(void* p_context)
{
  EVENTS::eventPut(MOVEMENT_BUFFER_END_EVENT);
}

void movementCountdownHandler(void* p_context)
{
  EVENTS::eventPut(SET_BUFFER_END_EVENT);
}


///////////////////////////////////////////////////
//////        Transition functions      ///////////
///////////////////////////////////////////////////


void triggeredFromWaitTransition()
{
  EVENTS::eventPut(TRAP_STATE_CHANGE_EVENT);

  INFO("TRANSITION - Wait to Trap Buffer");
  trapBufferCountdown.startCountdown(detectorConfig.triggerBufferLength, trapBufferCountdownHandler);
}

void trapBufferEndedTransition()
{
  EVENTS::eventPut(TRAP_STATE_CHANGE_EVENT);

  INFO("TRANSITION - Trap Buffer to Move Buffer");
  moveBufferCountdown.startCountdown(detectorConfig.moveBufferLength, moveBufferCountdownHandler);
  LIS2DH12::setInterruptThreshold(detectorConfig.moveThreshold, LIS2DH12::INTERRUPT_1);
}

void moveBufferEndedTransition()
{
  EVENTS::eventPut(TRAP_KILLED_EVENT);
  EVENTS::eventPut(TRAP_STATE_CHANGE_EVENT);

  INFO("TRANSITION - Move Buffer to Wait");
  LIS2DH12::setInterruptThreshold(detectorConfig.triggerThreshold, LIS2DH12::INTERRUPT_1);
}

void triggeredFromMoveTransition()
{
  EVENTS::eventPut(TRAP_STATE_CHANGE_EVENT);

  INFO("TRANSITION - Move Buffer to Set Buffer");
  movementCountdown.stopTimer();
  movementCountdown.startCountdown(detectorConfig.setBufferLength, movementCountdownHandler);
}

void moveToWaitTransition()
{
  EVENTS::eventPut(TRAP_STATE_CHANGE_EVENT);

  INFO("TRANSITION - Set Buffer to Wait");
  LIS2DH12::setInterruptThreshold(detectorConfig.triggerThreshold, LIS2DH12::INTERRUPT_1);
}

///////////////////////////////////////////////////
//////        Event Handler functions      ////////
///////////////////////////////////////////////////

void accTriggeredHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  INFO("Event: Accelerometer Triggered");
  //detectorStateMachine.transition(TRIGGERED_EVENT);

  EVENTS::eventPut(TRAP_TRIGGERED_EVENT);
}

void simulateTrigger()
{
  accTriggeredHandler(0, NRF_GPIOTE_POLARITY_LOTOHI);
}


void positionTriggered(void*)
{
  if (LIS2DH12::getInterruptSource(LIS2DH12::INTERRUPT_2) == LIS2DH12::INTERRUPT_SOURCE_ZL)
  {
    EVENTS::eventPut(UPSIDE_DOWN_EVENT);
  }
}

///////////////////////////////////////////////////
//////        Initialisation functions      ///////
///////////////////////////////////////////////////



void createTransitionTable(void)
{
  //                                      Start state,        End state,          Triggered by,               Transition handler

  detectorStateMachine.registerTransition(WAIT_STATE,         EVENT_BUFFER_STATE, TRAP_TRIGGERED_EVENT,      &triggeredFromWaitTransition);
  detectorStateMachine.registerTransition(EVENT_BUFFER_STATE, DETECT_MOVE_STATE,  TRAP_BUFFER_END_EVENT,     &trapBufferEndedTransition);
  detectorStateMachine.registerTransition(DETECT_MOVE_STATE,  WAIT_STATE,         MOVEMENT_BUFFER_END_EVENT, &moveBufferEndedTransition);
  //detectorStateMachine.registerTransition(DETECT_MOVE_STATE,  MOVING_STATE,       TRAP_TRIGGERED_EVENT,      &triggeredFromMoveTransition);
  //detectorStateMachine.registerTransition(MOVING_STATE,       MOVING_STATE,       TRAP_TRIGGERED_EVENT,      &triggeredFromMoveTransition);
  //detectorStateMachine.registerTransition(MOVING_STATE,       WAIT_STATE,         SET_BUFFER_END_EVENT,      &moveToWaitTransition);
  detectorStateMachine.registerTransition(EVENT_BUFFER_STATE, WAIT_STATE,         0x1505,                    &stop);
  detectorStateMachine.registerTransition(DETECT_MOVE_STATE,  WAIT_STATE,         0x1505,                    &stop);
  detectorStateMachine.registerTransition(MOVING_STATE,       WAIT_STATE,         0x1505,                    &stop);
  detectorStateMachine.registerTransition(WAIT_STATE,         WAIT_STATE,         0x1505,                    &stop);


}

void initialise()
{
  LIS2DH12::initThresholdInterrupt(detectorConfig.triggerThreshold, detectorConfig.triggerDuration, LIS2DH12::INTERRUPT_2, LIS2DH12::INTERRUPT_THRESHOLD_XYZ, true, accTriggeredHandler);
  LIS2DH12::clearInterrupts();


 // Timer testTimer;
 // testTimer.startTimer(2000, positionTriggered);

 // LIS2DH12::initThresholdInterrupt(800, 100, LIS2DH12::INTERRUPT_2, LIS2DH12::INTERRUPT_MOVEMENT_Z, false, NULL);
 // LIS2DH12::clearInterrupts();

  createTransitionTable();
  detectorStateMachine.start(WAIT_STATE);
}

void stop()
{
  EVENTS::eventPut(TRAP_STATE_CHANGE_EVENT);
  detectorStateMachine.stop();
  LIS2DH12::clearInterrupts();
}



}
*/