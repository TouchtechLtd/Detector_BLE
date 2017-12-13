
/*
 * detector_storage.cpp
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#include <stdint.h>
#include <string.h>

#include "libraries/events/events.h"
#include "drivers/flash/flash_interface.h"
#include "libraries/debug/DEBUG.h"

#include "./detector_storage.h"

#define NRF_LOG_MODULE_NAME DETECTOR_STORAGE
NRF_LOG_MODULE_REGISTER();

namespace DETECTOR
{

namespace STORAGE
{

static event_data_t recordData = { 0 };
static uint32_t killNumber32 = 0;

void recordKill(event_data_t* eventData)
{
  killNumber32 = eventData->killNumber;
  Flash_Record::write(KILL_NUMBER_FILE_ID,    KILL_NUMBER_KEY_ID,     &killNumber32,  sizeof(uint32_t));
  Flash_Record::write(KILL_DATA_FILE_ID,      eventData->killNumber,  eventData,     sizeof(*eventData));
}


void start()
{
  uint8_t* killNumber = getKillNumber();

  // Loads the number of kills into program memory
  Flash_Record::read(KILL_NUMBER_FILE_ID, KILL_NUMBER_KEY_ID, killNumber, sizeof(*killNumber));
  INFO("READING - Kill number - Value: %d", *killNumber);
}

event_data_t getKill(uint8_t eventID)
{
  Flash_Record::read(KILL_DATA_FILE_ID, eventID, &recordData, sizeof(recordData));
  return recordData;
}

}
}
