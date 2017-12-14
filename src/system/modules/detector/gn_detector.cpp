

/*
 * gn_detector.cpp
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#include <stdint.h>
#include <string.h>

#include "libraries/events/events.h"
#include "libraries/debug/DEBUG.h"

#include "./gn_detector.h"
#include "./detector_state.h"
#include "./detector_service.h"
#include "./detector_storage.h"

#include "system/modules/raw_event/gn_raw_event.h"
#include "system/modules/time/gn_time.h"
#include "system/modules/device/gn_device.h"

#define NRF_LOG_MODULE_NAME DETECTOR
NRF_LOG_MODULE_REGISTER();


namespace DETECTOR
{

static event_data_t       g_eventData   = { 0 };
static event_data_t       g_recordData  = { 0 };
static uint8_t            g_killNumber  = 0;


enum DetectorConfig_Default
{
  TRAP_TRIGGER_EVENT_THRESHOLD      = 500,
  TRAP_TRIGGER_MOVE_THRESHOLD       = 200,
  TRAP_TRIGGER_DURATION             = 0,
  TRAP_MOVE_DURATION                = 0,
  TRAP_EVENT_BUFFER_MS              = 2000,
  MOVE_BUFFER_MS                    = 5000,
  SET_BUFFER_MS                     = 10000
};


static detector_config_t g_detectorConfig = {
    TRAP_TRIGGER_EVENT_THRESHOLD,             // Trigger Threshold
    TRAP_TRIGGER_MOVE_THRESHOLD,              // Move Threshold
    TRAP_TRIGGER_DURATION,                    // Trigger Duration
    0,                                        // Move Duration
    TRAP_EVENT_BUFFER_MS,                     // Trigger buffer length
    MOVE_BUFFER_MS,                           // Move buffer length
    SET_BUFFER_MS                             // Set Buffer Length
};


Timer movementCountdown;
Timer trapBufferCountdown;
Timer moveBufferCountdown;



void trapBufferCountdownHandler(void* p_context)
{
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


void detectorTriggeredEvent()
{
  INFO("TRANSITION - Trap Triggered from Set");
  trapBufferCountdown.startCountdown(g_detectorConfig.triggerBufferLength, trapBufferCountdownHandler);
}

void detectorBufferEndedEvent()
{
  INFO("TRANSITION - Trap Buffer Ended");
  moveBufferCountdown.startCountdown(g_detectorConfig.moveBufferLength, moveBufferCountdownHandler);
  LIS2DH12::setInterruptThreshold(g_detectorConfig.moveThreshold, LIS2DH12::INTERRUPT_1);
}


void movementDetectedEvent()
{
  INFO("TRANSITION - Trap Movement Detected");
  movementCountdown.stopTimer();
  movementCountdown.startCountdown(g_detectorConfig.setBufferLength, movementCountdownHandler);
}

void setDetectedEvent()
{
  INFO("TRANSITION - Trap Set");
  LIS2DH12::setInterruptThreshold(g_detectorConfig.triggerThreshold, LIS2DH12::INTERRUPT_1);
}


void killDetectedEvent()
{
  INFO("TRANSITION - Kill Detected");

  EVENTS::eventPut(TRAP_KILLED_EVENT);
}


event_data_t* getEvent(uint8_t eventID)
{
  return &g_eventData;
}
detector_config_t* getConfig()
{
  return &g_detectorConfig;
}

uint8_t getDetectorState()
{
  return STATE::getCurrentState();
}

void setConfig(detector_config_t inputConfig)
{
  g_detectorConfig = inputConfig;
  INFO("UPDATING - Detector Configuration Updated");
  INFO("UPDATING - Trigger Threshold: %d",  g_detectorConfig.triggerThreshold);
  INFO("UPDATING - Move Threshold: %d",     g_detectorConfig.moveThreshold);
  INFO("UPDATING - Trigger Duration: %d",   g_detectorConfig.triggerDuration);
  INFO("UPDATING - Move Duration: %d",      g_detectorConfig.moveDuration);
  INFO("UPDATING - Trigger Buffer: %d",     g_detectorConfig.triggerBufferLength);
  INFO("UPDATING - Move Buffer: %d",        g_detectorConfig.moveBufferLength);
  INFO("UPDATING - Set Buffer: %d",         g_detectorConfig.setBufferLength);
}

uint8_t* getKillNumber()
{
  return &g_killNumber;
}


///////////////////////////////////////////////////
//////        Event Handler functions      ////////
///////////////////////////////////////////////////

void accTriggeredHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  INFO("LIS_EVENT - Accelerometer Triggered");
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

void setEventData()
{
  g_killNumber++;

  int32_t temp;
  LIS2DH12::updateTemperatureSensor();
  LIS2DH12::getTemperature(&temp);

  g_eventData.timestamp   =     *TIME::getCurrentTime();
  g_eventData.trap_id     =     DEVICE::getDeviceID();
  g_eventData.temperature =     static_cast<int8_t>(temp);
  g_eventData.killNumber  =     static_cast<uint8_t>(g_killNumber);
  g_eventData.rawEventData =    *RAW_EVENT::getEventNumber();
  g_eventData.peak_level  =     RAW_EVENT::getEventPeak(g_eventData.rawEventData);

  INFO("SETTING - Kill timestamp - \t%d",   g_eventData.timestamp.time);
  INFO("SETTING - Kill trap id - \t\t%d",   g_eventData.trap_id);
  INFO("SETTING - Kill temperature - \t%d", g_eventData.temperature);
  INFO("SETTING - Kill Number - \t\t%d",    g_eventData.killNumber);
  INFO("SETTING - Kill Peak Value - \t%d",  g_eventData.peak_level);
  INFO("SETTING - Raw Event Number - \t%d", g_eventData.rawEventData);
}


void recordEventData()
{
  STORAGE::recordKill(&g_eventData);
}


void showKill()
{
  showKill(g_killNumber);
}

void showKill(uint8_t eventID)
{
  g_recordData = STORAGE::getKill(eventID);
  SERVICE::sendKill(&g_recordData);
}


void registerEvents()
{
  EVENTS::registerEventHandler(DETECTOR_TRIGGERED,     detectorTriggeredEvent);
  EVENTS::registerEventHandler(DETECTOR_BUFFER_ENDED,  detectorBufferEndedEvent);
  EVENTS::registerEventHandler(DETECTED_KILL,          killDetectedEvent);
  EVENTS::registerEventHandler(DETECTED_KILL,          setDetectedEvent);
  EVENTS::registerEventHandler(DETECTED_MOVEMENT,      movementDetectedEvent);
  EVENTS::registerEventHandler(DETECTED_SET,           setDetectedEvent);

  EVENTS::registerEventHandler(TRAP_KILLED_EVENT, setEventData);
  EVENTS::registerEventHandler(TRAP_KILLED_EVENT, recordEventData);
  EVENTS::registerEventHandler(TRAP_KILLED_EVENT, showKill);
  EVENTS::registerEventHandler(TRAP_TRIGGERED_EVENT,     LIS2DH12::clearInterrupts);

  EVENTS::registerEventHandler(DETECTOR_TRIGGERED,      SERVICE::updateState);
  EVENTS::registerEventHandler(DETECTOR_BUFFER_ENDED,   SERVICE::updateState);
  EVENTS::registerEventHandler(DETECTED_MOVEMENT,       SERVICE::updateState);
  EVENTS::registerEventHandler(DETECTED_KILL,           SERVICE::updateState);
  EVENTS::registerEventHandler(DETECTED_SET,            SERVICE::updateState);
  EVENTS::registerEventHandler(DETECTOR_TRIGGERED,      SERVICE::updateState);

}


void start()
{
  STATE::start();
}

void stop()
{
  STATE::stop();
}

void init()
{
  LIS2DH12::initThresholdInterrupt(g_detectorConfig.triggerThreshold, g_detectorConfig.triggerDuration, LIS2DH12::INTERRUPT_2, LIS2DH12::INTERRUPT_THRESHOLD_XYZ, true, accTriggeredHandler);
  LIS2DH12::clearInterrupts();

  /*
  Timer testTimer;
  testTimer.startTimer(2000, positionTriggered);

  LIS2DH12::initThresholdInterrupt(800, 100, LIS2DH12::INTERRUPT_1, LIS2DH12::INTERRUPT_MOVEMENT_Z, false, NULL);
  LIS2DH12::clearInterrupts();
*/

  SERVICE::start();
  STORAGE::start();

  registerEvents();
}


}
