
/*
 * raw_event_storage.cpp
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#include <stdint.h>
#include <string.h>

#include "libraries/events/events.h"
#include "drivers/flash/flash_interface.h"
#include "libraries/debug/DEBUG.h"

#include "./raw_event_storage.h"

#define NRF_LOG_MODULE_NAME RAW_EVENT_STORAGE
NRF_LOG_MODULE_REGISTER();

namespace RAW_EVENT
{

namespace STORAGE
{

static raw_event_t recordData = { 0 };
static uint32_t rawEventNumber32 = 0;

void recordEvent(raw_event_t* rawData)
{
  rawEventNumber32 = rawData->eventNumber;
  Flash_Record::write(RAW_DATA_NUMBER_FILE_ID,    RAW_DATA_NUMBER_KEY_ID,     &rawEventNumber32,  sizeof(rawEventNumber32));
  Flash_Record::write(RAW_DATA_FILE_ID,           rawData->eventNumber,       rawData,         sizeof(*rawData));
}


void start()
{
  uint8_t* eventNumber = getEventNumber();

  // Loads the number of kills into program memory
  Flash_Record::read(RAW_DATA_NUMBER_FILE_ID, RAW_DATA_NUMBER_KEY_ID, eventNumber, sizeof(*eventNumber));
  INFO("READING - Event Number - Value: %d", *eventNumber);
}

raw_event_t* getEvent(uint8_t eventID)
{
  Flash_Record::read(RAW_DATA_FILE_ID, eventID, &recordData, sizeof(recordData));
  return &recordData;
}

}
}
