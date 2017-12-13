
/*
 * raw_event_storage.h
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#ifndef _MODULE_RAW_EVENT_STORAGE_H__
#define _MODULE_RAW_EVENT_STORAGE_H__

#include <stdint.h>
#include "gn_raw_event.h"


namespace RAW_EVENT
{

namespace STORAGE
{

#define RAW_DATA_FILE_ID            (0xDA8A)
#define RAW_DATA_NUMBER_FILE_ID     (0x5112)
#define RAW_DATA_NUMBER_KEY_ID      (0x1112)


void         recordEvent(raw_event_t* eventData);
raw_event_t* getEvent(uint8_t eventID);

void start();

//void writeData();
//void getData();


}
}


#endif  /* _MODULE_RAW_EVENT_STORAGE_H__ */
