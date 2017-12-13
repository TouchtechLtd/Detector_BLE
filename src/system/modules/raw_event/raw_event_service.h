
/*
 * raw_event_state.h
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#ifndef _MODULE_RAW_EVENT_SERVICE_H__
#define _MODULE_RAW_EVENT_SERVICE_H__

#include <stdint.h>
#include "gn_raw_event.h"


namespace RAW_EVENT
{

namespace SERVICE
{

#define BLE_UUID_SERVICE_RAW_EVENT      0xD00D
#define SERVICE_RAW_EVENT               1

enum RawEventService_UUID
{
  BLE_UUID_CHAR_RAW_EVENT                      = 0xD10D,
  BLE_UUID_CHAR_RAW_EVENT_DISPLAYED            = 0xD20D
};

enum RawEventService_Characteristics
{
  CHAR_RAW_EVENT,
  CHAR_RAW_EVENT_DISPLAYED
};


void start();
void sendEvent(raw_event_t* rawEvent);
//void update();
//void sendKill(event_data_t* eventData);

}
}

#endif  /* _MODULE_RAW_EVENT_SERVICE_H__ */
