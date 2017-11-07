
/*
 * trap_manager.h
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#ifndef _GOODNATURE_APP_TRAP_MANAGER_H__
#define _GOODNATURE_APP_TRAP_MANAGER_H__

#include <stdint.h>
#include "trap_manager_config.h"

namespace EVENT_MANAGER
{

#pragma pack(push, 1)
typedef struct
{
    uint8_t       peak_level;
    uint32_t      timestamp;
    uint32_t      trap_id;
    uint16_t      temperature;
    uint8_t       killNumber;
} event_data_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
  uint16_t      count;
  uint8_t       raw_data[RAW_DATA_CAPTURE_SIZE];
}raw_event_data_t;
#pragma pack(pop)

#pragma pack(push, 1)
  typedef struct {
    uint16_t triggerThreshold;
    uint16_t moveThreshold;
    uint8_t triggerDuration;
    uint8_t moveDuration;
    uint16_t triggerBufferLength;
    uint16_t moveBufferLength;
    uint16_t setBufferLength;
  } trap_detector_config_t;
#pragma pack(pop)


typedef void (*trap_event_handler_t)(trap_event_e trap_event);


  void initialise();
  void simulateTrigger();
  detector_state_e getState();

  event_data_t* getEvent(uint8_t eventID);
  void recordCurrentEvent();
  bool isAnimalKilled();

  trap_detector_config_t* getConfig();
  uint8_t getKillNumber();
  void registerEventHandler(trap_event_handler_t handler);



}


#endif  /* _ _GOODNATURE_APP_TRAP_MANAGER_H__ */