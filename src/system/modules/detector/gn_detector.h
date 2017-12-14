
/*
 * gn_detector.h
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#ifndef _MODULE_DETECTOR_H__
#define _MODULE_DETECTOR_H__

#include <stdint.h>

#include "system/modules/time/gn_time.h"
#include "drivers/LIS2DH12/LIS2DH12.h"

namespace DETECTOR
{

#define DETECTOR_EVENT_OFFSET 0x1000

enum {
  TRAP_TRIGGERED_EVENT = DETECTOR_EVENT_OFFSET,
  TRAP_STATE_CHANGE_EVENT,
  TRAP_BUFFER_END_EVENT,
  MOVEMENT_BUFFER_END_EVENT,
  SET_BUFFER_END_EVENT,
  TRAP_KILLED_EVENT,
  UPSIDE_DOWN_EVENT,
  DETECTOR_TRIGGERED,
  DETECTOR_BUFFER_ENDED,
  DETECTED_MOVEMENT,
  DETECTED_KILL,
  DETECTED_SET
};


#pragma pack(push, 1)
typedef struct
{
             uint8_t   peak_level;
            uint32_t   trap_id;
              int8_t    temperature;
             uint8_t   killNumber;
TIME::current_time_t   timestamp;
             uint8_t   rawEventData;
} event_data_t;


typedef struct {
    uint16_t triggerThreshold;
    uint16_t moveThreshold;
    uint8_t triggerDuration;
    uint8_t moveDuration;
    uint16_t triggerBufferLength;
    uint16_t moveBufferLength;
    uint16_t setBufferLength;
  } detector_config_t;
#pragma pack(pop)


event_data_t*      getEvent(uint8_t eventID);
detector_config_t* getConfig();
uint8_t getDetectorState();
void setConfig(detector_config_t inputConfig);
uint8_t*           getKillNumber();

void showKill(uint8_t eventID);

void simulateTrigger();
void start();
void stop();
void init();


}


#endif  /* _MODULE_DETECTOR_H__ */



