
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


void deviceControlHandler(EVENTS::event_data_t data)
{

  trap_control_t command = *(trap_control_t*)data.p_data;;

  if (1 == command.activate)
  {
    EVENTS::eventPut(ACTIVATE_EVENT);
  }
  else if (2 == command.activate)
  {
    EVENTS::eventPut(DEACTIVATE_EVENT);
  }
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


  INFO("INITIALISING - device control characteristic\t\t - UUID: %x, ID: %d", BLE_UUID_CHAR_DEVICE_CONTROL, CHAR_DEVICE_CONTROL);
  BLE_SERVER::Characteristic trapDeviceControl;
  trap_control_t deviceControl = { 0 };
  trapDeviceControl.configure(BLE_UUID_CHAR_DEVICE_CONTROL, &deviceControl, sizeof(deviceControl), BLE_SERVER::CHAR_READ_WRITE);
  deviceInfo.addCharacteristic(&trapDeviceControl, CHAR_DEVICE_CONTROL);

  INFO("INITIALISING - device state characteristic\t\t - UUID: %x, ID: %d", BLE_UUID_CHAR_DEVICE_STATE, CHAR_DEVICE_STATE);
  BLE_SERVER::Characteristic deviceStateChar;
  uint8_t deviceState = getDeviceState();
  deviceStateChar.configure(BLE_UUID_CHAR_DEVICE_STATE, &deviceControl, sizeof(deviceControl), BLE_SERVER::CHAR_READ_ONLY);
  deviceInfo.addCharacteristic(&deviceStateChar, CHAR_DEVICE_STATE);

  INFO("INITIALISING - software version characteristic\t\t - UUID: %x, ID: %d", BLE_UUID_CHAR_SOFTWARE_VERSION, CHAR_SOFTWARE_VERSION);
  BLE_SERVER::Characteristic softwareVersionChar;
  software_version_t currentVersion = getSoftwareVersion();
  softwareVersionChar.configure(BLE_UUID_CHAR_SOFTWARE_VERSION, &currentVersion, sizeof(currentVersion), BLE_SERVER::CHAR_READ_ONLY);
  deviceInfo.addCharacteristic(&softwareVersionChar, CHAR_SOFTWARE_VERSION);


  INFO("ATTACHING - device info service");
  deviceInfo.attachService();
  BLE_SERVER::addService(&deviceInfo, SERVICE_DEVICE_INFO);


  BLE_SERVER::setWriteHandler(SERVICE_DEVICE_INFO, CHAR_DEVICE_ID,        deviceIDHandler);
  BLE_SERVER::setWriteHandler(SERVICE_DEVICE_INFO, CHAR_DEVICE_CONTROL,   deviceControlHandler);
}


void updateID()
{
  INFO("UPDATING - Trap ID");
  uint32_t deviceID = getDeviceID();
  BLE_SERVER::setCharacteristic(SERVICE_DEVICE_INFO, CHAR_DEVICE_ID, &deviceID, sizeof(deviceID));
}

void updateState()
{
  INFO("UPDATING - Trap State");
  uint8_t deviceState = getDeviceState();
  BLE_SERVER::setCharacteristic(SERVICE_DEVICE_INFO, CHAR_DEVICE_STATE, &deviceState, sizeof(deviceState));
}






} // Namespace SERVICE
} // Namespace DEVICE
