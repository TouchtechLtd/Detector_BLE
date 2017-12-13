
/*
 *  time_service.cpp
 *
 *  Created on: 12/12/2017
 *      Author: michaelmcadam
 */

#include <stdint.h>
#include <string.h>

#include "libraries/events/events.h"
#include "libraries/ble/ble_interface.h"
#include "libraries/ble/gn_ble_config.h"
#include "libraries/debug/DEBUG.h"

#include "./time_service.h"

#define NRF_LOG_MODULE_NAME TIME_SERVICE
NRF_LOG_MODULE_REGISTER();

namespace TIME
{

namespace SERVICE
{


void trapTimeHandler(EVENTS::event_data_t data)
{
  current_time_t currentTime = *(current_time_t*)data.p_data;;

  setCurrentTime(currentTime);
  INFO("Time Set To: %d", currentTime.time);

  EVENTS::eventPut(TIMESET_EVENT);
}


void start()
{
  INFO("INITIALISING - trap time service\t\t\t - UUID: %x, ID: %d", BLE_UUID_SERVICE_TIME, SERVICE_TRAP_TIME);
  BLE_SERVER::Service trapTimeService;
  trapTimeService.createCustom(BLE_UUID_SERVICE_TIME, BLE_UUID_GOODNATURE_BASE);


  INFO("INITIALISING - trap time characteristic\t\t - UUID: %x, ID: %d", BLE_UUID_CHAR_TRAP_TIME, CHAR_TRAP_TIME);
  BLE_SERVER::Characteristic trapTime;
  current_time_t startTime = *getCurrentTime();
  trapTime.configure(BLE_UUID_CHAR_TRAP_TIME, &startTime, sizeof(startTime), BLE_SERVER::CHAR_READ_WRITE);
  trapTimeService.addCharacteristic(&trapTime, CHAR_TRAP_TIME);


  INFO("ATTACHING - trap time service");
  trapTimeService.attachService();
  BLE_SERVER::addService(&trapTimeService, SERVICE_TRAP_TIME);


  BLE_SERVER::setWriteHandler(SERVICE_TRAP_TIME, CHAR_TRAP_TIME, trapTimeHandler);
}


void update()
{
  INFO("UPDATING - Current Time");
  current_time_t currentTime         = *getCurrentTime();
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_TIME, CHAR_TRAP_TIME, &currentTime,  sizeof(current_time_t));
}


} // Namespace SERVICE
} // Namespace DETECTOR
