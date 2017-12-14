
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

static uint32_t deviceIDRecord = 0;
static uint32_t deviceBootNum  = 0;

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

void recordBootNum()
{
  deviceBootNum = getBootNum();
  Flash_Record::write(CONFIG_FILE_ID, CONFIG_REC_KEY_ID, &deviceBootNum, sizeof(deviceBootNum));
}

void start()
{
  Flash_Record::read(CONFIG_FILE_ID, CONFIG_REC_KEY_ID, &deviceBootNum, sizeof(deviceBootNum));
  deviceBootNum++;
  setBootNum(deviceBootNum);


  setDeviceID(getID());
}


}
}
