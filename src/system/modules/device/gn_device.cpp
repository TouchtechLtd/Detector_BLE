

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

static uint32_t  g_trapID   = 0;



uint32_t getDeviceID()
{
  return g_trapID;
}


void setDeviceID(uint32_t newID)
{
  g_trapID = newID;
  STORAGE::recordID();
  SERVICE::update();
}

void registerEvents()
{
  //EVENTS::registerEventHandler(DETECTOR_TRIGGERED,     detectorTriggeredEvent);
}


void init()
{

  //STATE::start();
  SERVICE::start();
  STORAGE::start();

  registerEvents();
}


}
