
/*
 * device_storage.cpp
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#include <stdint.h>
#include <string.h>

#include "libraries/events/events.h"
#include "drivers/flash/flash_interface.h"
#include "libraries/debug/DEBUG.h"

#include "./device_storage.h"

#define NRF_LOG_MODULE_NAME DEVICE_STORAGE
NRF_LOG_MODULE_REGISTER();

namespace DEVICE
{

namespace STORAGE
{

static uint32_t deviceIDRecord = 0;;

void recordID()
{
  deviceIDRecord = getDeviceID();
  Flash_Record::write(CONFIG_FILE_ID, TRAP_ID_KEY_ID, &deviceIDRecord,    sizeof(deviceIDRecord));
}


uint32_t getID()
{
  Flash_Record::read(CONFIG_FILE_ID, TRAP_ID_KEY_ID, &deviceIDRecord, sizeof(deviceIDRecord));
  return deviceIDRecord;
}

void start()
{
  /*

  static uint8_t bootNum = 0;
    Flash_Record::read(CONFIG_FILE_ID, CONFIG_REC_KEY_ID, &bootNum, sizeof(bootNum));
    bootNum++;
    INFO("READING - Bootnumber - Value: %d", bootNum);
    // If the number of times the program has booted is greater than 20 something has probably gone wrong, so busy wait to save power

    if (bootNum > 20)
    {
      while(1) sd_app_evt_wait();
    }

    Flash_Record::write(CONFIG_FILE_ID, CONFIG_REC_KEY_ID, &bootNum, sizeof(bootNum));

  */

  setDeviceID(getID());
}


}
}
