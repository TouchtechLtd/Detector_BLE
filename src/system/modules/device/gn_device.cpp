

/*
 * gn_device.cpp
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#include <stdint.h>
#include <string.h>

#include "libraries/events/events.h"
#include "libraries/debug/DEBUG.h"

#include "./gn_device.h"
#include "./device_state.h"
#include "./device_service.h"
#include "./device_storage.h"


#define NRF_LOG_MODULE_NAME DEVICE
NRF_LOG_MODULE_REGISTER();


namespace DEVICE
{

static const software_version_t softwareVersion = { VERSION_MAJOR, VERSION_MINOR };



static uint32_t  g_trapID   = 0;
static uint32_t  g_bootNum  = 0;


uint32_t getBootNum()
{
  return g_bootNum;
}

void setBootNum(uint32_t newBootNum)
{
  INFO("SETTING - Bootnumber - Value: %d", newBootNum);
  g_bootNum = newBootNum;
  STORAGE::recordBootNum();
}

uint32_t getDeviceID()
{
  return g_trapID;
}


void setDeviceID(uint32_t newID)
{
  g_trapID = newID;
  STORAGE::recordID();
  SERVICE::updateID();
}

uint8_t getDeviceState()
{
  return STATE::getCurrentState();
}

software_version_t getSoftwareVersion()
{
  return softwareVersion;
}


void registerEvents()
{
  EVENTS::registerEventHandler(DEVICE_ACTIVATED,     SERVICE::updateState);
  EVENTS::registerEventHandler(DEVICE_DEACTIVATED,   SERVICE::updateState);
}


void init()
{

  STATE::start();
  SERVICE::start();
  STORAGE::start();

  registerEvents();

  INFO("Software version: %d.%d", softwareVersion.major, softwareVersion.minor);
}


}
