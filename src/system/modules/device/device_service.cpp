
/*
 * detector_service.cpp
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

#include "./device_service.h"

#define NRF_LOG_MODULE_NAME DEVICE_SERVICE
NRF_LOG_MODULE_REGISTER();

namespace DEVICE
{

namespace SERVICE
{


void trapControlHandler(EVENTS::event_data_t data)
{
  /*
  trap_control_t command = *(trap_control_t*)data.p_data;;

  if (1 == command.activate)
  {
    INFO("Activating Trap!");
    EVENTS::eventPut(ACTIVATE_EVENT);
    //mainStateMachine.transition(ACTIVATE_EVENT);
  }
  else if (2 == command.activate)
  {
    INFO("De-activating Trap.");
    EVENTS::eventPut(DEACTIVATE_EVENT);
    //mainStateMachine.transition(DEACTIVATE_EVENT);
  }
  */

}

void deviceIDHandler(EVENTS::event_data_t data)
{
  uint32_t newID = *(uint32_t*)data.p_data;

  INFO("Setting Trap ID to: %d", newID);
  setDeviceID(newID);

  BLE_SERVER::setCharacteristic(SERVICE_DEVICE_INFO, CHAR_DEVICE_ID, &newID, sizeof(newID));
}


void start()
{
  INFO("INITIALISING - device info service\t\t\t - UUID: %x, ID: %d", BLE_UUID_SERVICE_DEVICE_INFO, SERVICE_DEVICE_INFO);
  BLE_SERVER::Service deviceInfo;
  deviceInfo.createCustom(BLE_UUID_SERVICE_DEVICE_INFO, BLE_UUID_GOODNATURE_BASE);


  INFO("INITIALISING - device id characteristic\t\t - UUID: %x, ID: %d", BLE_UUID_CHAR_DEVICE_ID, CHAR_DEVICE_ID);
  BLE_SERVER::Characteristic trapDeviceID;
  uint32_t deviceID = getDeviceID();
  trapDeviceID.configure(BLE_UUID_CHAR_DEVICE_ID, &deviceID, sizeof(deviceID), BLE_SERVER::CHAR_READ_WRITE);
  deviceInfo.addCharacteristic(&trapDeviceID, CHAR_DEVICE_ID);





  INFO("ATTACHING - device info service");
  deviceInfo.attachService();
  BLE_SERVER::addService(&deviceInfo, SERVICE_DEVICE_INFO);


  BLE_SERVER::setWriteHandler(SERVICE_DEVICE_INFO, CHAR_DEVICE_ID,   deviceIDHandler);
}


void update()
{
  INFO("UPDATING - Trap Info");
  uint32_t deviceID = getDeviceID();
  BLE_SERVER::setCharacteristic(SERVICE_DEVICE_INFO, CHAR_DEVICE_ID, &deviceID, sizeof(deviceID));

}






} // Namespace SERVICE
} // Namespace DEVICE
