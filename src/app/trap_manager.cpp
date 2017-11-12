
/*
 * trap_manager.cpp
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#include <stdint.h>
#include <string.h>
#include <math.h>

#include "app/trap_manager.h"
#include "app/state_machine.h"
#include "app/events.h"
#include "peripheral/LIS2DH12.h"
#include "peripheral/flash_interface.h"
#include "debug/DEBUG.h"




namespace TrapState
{
static StateMachine detectorStateMachine(WAIT_STATE, MAX_STATES, MAX_EVENTS);
//static event_data_t       eventData;
//static event_data_t       recordData = { 0 };
//static raw_event_data_t   rawEventData;
//static trap_event_handler_t g_eventHandler = NULL;

//static uint8_t            killNumber = 0;

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

/*
void updateEventHandler(trap_event_e event)
{
  if (g_eventHandler != NULL) { g_eventHandler(event); }
}*/


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

uint8_t accConverter(int32_t inputInt)
{
  if (inputInt < 0) { inputInt = -inputInt; }
  return inputInt >> 5;
}

void accReadTimerHandler(void* p_context)
{
  /*
  LIS2DH12_sample();

  static int32_t accX32, accY32, accZ32 = 0;
  static uint8_t accX8, accY8, accZ8 = 0;

  LIS2DH12_getALLmG(&accX32, &accY32, &accZ32);

  accX8 = accConverter(accX32);
  accY8 = accConverter(accY32);
  accZ8 = accConverter(accZ32);
  uint8_t sum = sqrt((accX8*accX8) + (accY8*accY8) + (accZ8*accZ8));
  //INFO("X: %d, Y: %d, Z: %d", accX8, accY8, accZ8);
  //INFO("Sum: %d", sum);

  if (rawEventData.count < RAW_DATA_CAPTURE_SIZE)
  {
    rawEventData.raw_data[rawEventData.count] = accX8;
    rawEventData.count++;
  }
  if (sum > eventData.peak_level)
  {
    eventData.peak_level = sum;
  }
  */
}

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

  LIS2DH12_startDAPolling();
  INFO("Triggerd from wait");
  trapBufferCountdown.startCountdown(detectorConfig.triggerBufferLength, trapBufferCountdownHandler);
}

void trapBufferEndedTransition()
{
  LIS2DH12_stopDAPolling();

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
  LIS2DH12_initDAPolling(accReadTimerHandler);
}



}
