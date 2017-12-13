
/*
 * gn_detector.h
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#ifndef _MODULE_RAW_EVENT_H__
#define _MODULE_RAW_EVENT_H__

#include <stdint.h>

#include "drivers/LIS2DH12/LIS2DH12.h"

namespace RAW_EVENT
{

#define RAW_EVENT_EVENT_OFFSET 0x1100
#define RAW_EVENT_SIZE 250

enum {
  RAW_DATA_FULL = RAW_EVENT_EVENT_OFFSET,
  RAW_DATA_PROCESSED,
  EVENT_TRIGGERED
};



#pragma pack(push, 1)

typedef struct
{
  uint8_t                           sum;
  LIS2DH12::acceleration_8b_t       acc;
}raw_event_data_t;

typedef struct
{
  raw_event_data_t    data[RAW_EVENT_SIZE];
  uint8_t            eventNumber;
  uint8_t             peakLevel;
  bool                processed;
}raw_event_t;

#pragma pack(pop)


uint8_t* getEventNumber();
uint8_t   getEventPeak(uint8_t eventID);
void showEvent(uint8_t eventID);

void init();


}


#endif  /* _MODULE_RAW_EVENT_H__ */



