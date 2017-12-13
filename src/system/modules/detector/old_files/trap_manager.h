
/*
 * trap_manager.h
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#ifndef _GOODNATURE_APP_TRAP_MANAGER_H__
#define _GOODNATURE_APP_TRAP_MANAGER_H__
/*
#include <stdint.h>
#include "system/modules/detector/old_files/trap_manager_config.h"
//#include "system/modules/time/current_time.h"
#include "drivers/LIS2DH12/LIS2DH12.h"

namespace TrapState
{

#define TRAP_EVENT_OFFSET 0x1000
#define TRAP_ID       0x00000000

enum {
  TRAP_TRIGGERED_EVENT = TRAP_EVENT_OFFSET,
  TRAP_STATE_CHANGE_EVENT,
  TRAP_BUFFER_END_EVENT,
  MOVEMENT_BUFFER_END_EVENT,
  SET_BUFFER_END_EVENT,
  TRAP_KILLED_EVENT,
  UPSIDE_DOWN_EVENT
};



#pragma pack(push, 1)
typedef struct
{
                    uint8_t  peak_level;
                   uint32_t  trap_id;
                    int8_t  temperature;
                    uint8_t  killNumber;
CurrentTime::current_time_t  timestamp;
} event_data_t;

typedef struct
{
  uint8_t                           sum;
  LIS2DH12::acceleration_8b_t       acc;
}raw_event_data_t;


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


//typedef void (*trap_event_handler_t)(trap_event_e trap_event);

  void initialise();
  void stop();
  void simulateTrigger();

  detector_state_e getState();
  const trap_detector_config_t* getConfig();
  void setConfig(trap_detector_config_t inputConfig);


}
*/

#endif  /* _ _GOODNATURE_APP_TRAP_MANAGER_H__ */
