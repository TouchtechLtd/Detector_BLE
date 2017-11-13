
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




namespace TrapState
{
static StateMachine detectorStateMachine(WAIT_STATE, MAX_STATES, MAX_EVENTS);

Timer sampleTimer;
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

trap_detector_config_t* getConfig()
{
  return &detectorConfig;
}

detector_state_e getState()
{
  return static_cast<detector_state_e>(detectorStateMachine.getCurrentState());
}

/*
event_data_t* getEvent(uint8_t eventID)
{
  Flash_Record::read(KILL_DATA_FILE_ID, eventID, &recordData, sizeof(recordData));
  return &recordData;
}
*/

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

  INFO("Triggerd from wait");
  trapBufferCountdown.startCountdown(detectorConfig.triggerBufferLength, trapBufferCountdownHandler);
}

void trapBufferEndedTransition()
{
  EVENTS::eventPut(TRAP_STATE_CHANGE_EVENT);
  INFO("Trap buffer end");

  moveBufferCountdown.startCountdown(detectorConfig.moveBufferLength, moveBufferCountdownHandler);
  LIS2DH12_setInterruptThreshold(detectorConfig.moveThreshold);
}

void moveBufferEndedTransition()
{
  EVENTS::eventPut(TRAP_KILLED_EVENT);
  EVENTS::eventPut(TRAP_STATE_CHANGE_EVENT);
  LIS2DH12_setInterruptThreshold(detectorConfig.triggerThreshold);
}

void triggeredFromMoveTransition()
{
  EVENTS::eventPut(TRAP_MOVING_EVENT);
  EVENTS::eventPut(TRAP_STATE_CHANGE_EVENT);
  INFO("Triggerd from move");

  movementCountdown.stopTimer();
  movementCountdown.startCountdown(detectorConfig.setBufferLength, movementCountdownHandler);
}

void moveToWaitTransition()
{
  EVENTS::eventPut(TRAP_STATE_CHANGE_EVENT);
  EVENTS::eventPut(TRAP_SET_EVENT);

  INFO("Trap set");
  LIS2DH12_setInterruptThreshold(detectorConfig.triggerThreshold);
}

///////////////////////////////////////////////////
//////        Event Handler functions      ////////
///////////////////////////////////////////////////

void accTriggeredHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  LIS2DH12_clearInterrupts();
  detectorStateMachine.transition(TRIGGERED_EVENT);
  INFO("accTriggered");
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
  createTransitionTable();

  LIS2DH12_initThresholdInterrupt(detectorConfig.triggerThreshold, detectorConfig.triggerDuration, LIS2DH12_INTERRUPT_THRESHOLD_XYZ, true, accTriggeredHandler);
}



}
