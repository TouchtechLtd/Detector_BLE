
/*
 * detector_state.h
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#ifndef _MODULE_DETECTOR_SERVICE_H__
#define _MODULE_DETECTOR_SERVICE_H__

#include <stdint.h>
#include "gn_detector.h"


namespace DETECTOR
{

namespace SERVICE
{

#define BLE_UUID_SERVICE_TRAP_DATA      0xDEAD
#define SERVICE_TRAP_DATA               2

enum DetectorService_UUID
{
  BLE_UUID_CHAR_TRAP_EVENT_DATA                = 0xDEED,
  BLE_UUID_CHAR_TRAP_EVENT_CONFIG              = 0xD1ED,
  BLE_UUID_CHAR_TRAP_EVENT_DISPLAYED           = 0xD2ED,
  BLE_UUID_CHAR_TRAP_TIME                      = 0xD3ED,
  BLE_UUID_CHAR_RAW_DATA                       = 0xD4ED,
  BLE_UUID_CHAR_ERROR_DATA                     = 0xD5ED,
  BLE_UUID_CHAR_TRAP_INFO                      = 0xD6ED,
  BLE_UUID_CHAR_TRAP_CONTROL                   = 0xD7ED
};

enum DetectorService_Characteristics
{
  CHAR_EVENT_DATA,
  CHAR_EVENT_CONFIG,
  CHAR_EVENT_DISPLAYED,
  CHAR_TRAP_TIME,
  CHAR_RAW_DATA,
  CHAR_ERROR_DATA,
  CHAR_TRAP_INFO,
  CHAR_TRAP_CONTROL
};

void start();
void update();
void sendKill(event_data_t* eventData);

}
}

#endif  /* _MODULE_DETECTOR_SERVICE_H__ */
