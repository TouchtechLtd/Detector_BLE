
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
#include "app/current_time.h"
#include "peripheral/LIS2DH12.h"
#include "peripheral/flash_interface.h"
#include "debug/DEBUG.h"




namespace EVENT_MANAGER
{

static StateMachine detectorStateMachine(WAIT_STATE, MAX_STATES, MAX_EVENTS);
static event_data_t       eventData;
static event_data_t       recordData = { 0 };
static raw_event_data_t   rawEventData;
static trap_event_handler_t g_eventHandler = NULL;
static bool isKilled = false;

static uint8_t            killNumber = 0;

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


void updateEventHandler(trap_event_e event)
{
  if (g_eventHandler != NULL) { g_eventHandler(event); }
}



void recordCurrentEvent()
{
  isKilled = false;
}


bool isAnimalKilled()
{
  return isKilled;
}


///////////////////////////////////////////////////
//////           Timer handlers         ///////////
///////////////////////////////////////////////////

trap_detector_config_t* getConfig()
{
  return &detectorConfig;
}

detector_state_e getState()
{
  return static_cast<detector_state_e>(detectorStateMachine.getCurrentState());
}

event_data_t* getEvent(uint8_t eventID)
{
  Flash_Record::read(KILL_DATA_FILE_ID, eventID, &recordData, sizeof(recordData));
  return &recordData;
}

uint8_t getKillNumber()
{
  uint8_t returnNumber = killNumber;
  return returnNumber;
}


///////////////////////////////////////////////////
//////           Timer handlers         ///////////
///////////////////////////////////////////////////


void accReadTimerHandler(void* p_context)
{
  LIS2DH12_sample();

  static int32_t accX, accY, accZ = 0;
  LIS2DH12_getALLmG(&accX, &accY, &accZ);
  DEBUG("X: %d, Y: %d, Z: %d", accX, accY, accZ);

  if (rawEventData.count < RAW_DATA_CAPTURE_SIZE)
    {
      rawEventData.raw_data[rawEventData.count] = accX;
      rawEventData.count++;
    }
  if (accX > eventData.peak_level)
    {
      eventData.peak_level = accX;
    }

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
  //sampleTimer.startTimer(100, accReadTimerHandler);
  //LIS2DH12_startDAPolling();
  INFO("Triggerd from wait");
  trapBufferCountdown.startCountdown(detectorConfig.triggerBufferLength, trapBufferCountdownHandler);
}

void trapBufferEndedTransition()
{
  //LIS2DH12_stopDAPolling();
  INFO("Trap buffer end");

  moveBufferCountdown.startCountdown(detectorConfig.moveBufferLength, moveBufferCountdownHandler);
  //LIS2DH12_setInterruptThreshold(detectorConfig.moveThreshold);

}

void moveBufferEndedTransition()
{
  INFO("Killed");

  killNumber++;

  eventData.timestamp   =     CurrentTime::getCurrentTime();
  eventData.trap_id     =     0x12345678;
  eventData.temperature =     20;
  eventData.killNumber  =     killNumber;
  eventData.peak_level  =     100;

  Flash_Record::write(KILL_DATA_FILE_ID, killNumber, &eventData, sizeof(eventData));
  Flash_Record::write(KILL_NUMBER_FILE_ID, KILL_NUMBER_KEY_ID, &killNumber, sizeof(killNumber));
  LIS2DH12_setInterruptThreshold(detectorConfig.triggerThreshold);

  updateEventHandler(ANIMAL_KILLED);

}


void triggeredFromMoveTransition()
{
  INFO("Triggerd from move");
  memset(&eventData, 0, sizeof(event_data_t));
  movementCountdown.stopTimer();
  movementCountdown.startCountdown(detectorConfig.setBufferLength, movementCountdownHandler);

}


void moveToWaitTransition()
{
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



void registerEventHandler(trap_event_handler_t handler)
{
  g_eventHandler = handler;
}

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


void initAccelerometer()
{
  LIS2DH12_init(LIS2DH12_POWER_LOW, LIS2DH12_SCALE2G, LIS2DH12_SAMPLE_50HZ);
  LIS2DH12_enableHighPass();
  LIS2DH12_initThresholdInterrupt(detectorConfig.triggerThreshold, detectorConfig.triggerDuration, LIS2DH12_INTERRUPT_THRESHOLD_XYZ, true, accTriggeredHandler);
  LIS2DH12_initDAPolling(accReadTimerHandler);
  //LIS2DH12_startDAPolling();
}

void initialise()
{
  Flash_Record::read(KILL_NUMBER_FILE_ID, KILL_NUMBER_KEY_ID, &killNumber, sizeof(killNumber));

  createTransitionTable();
  initAccelerometer();
}



}
