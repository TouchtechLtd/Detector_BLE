
/*
 * ble_manager.cpp
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */


#define BLE_UUID_OUR_CHARACTERISTC_UUID          0xBEEF // Just a random, but recognizable value
#define BLE_UUID_GOODNATURE_BASE              {{0x23, 0xD1, 0x13, 0xEF, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}} // 128-bit base UUID

// Defining 16-bit service and 128-bit base UUIDs
#define BLE_UUID_OUR_SERVICE_UUID_1                0xF00D // Just a random, but recognizable value
#define BLE_UUID_OUR_SERVICE_UUID_2                0xFEED // Just a random, but recognizable value


#define BLE_UUID_SERVICE_DETECTOR_DATA                     0xC001
#define BLE_UUID_SIG_SERVICE_DEVICE_INFO                   0x180A
#define BLE_UUID_SIG_SERVICE_BATTERY_LEVEL                 0x180F
#define BLE_UUID_SIG_SERVICE_CURRENT_TIME                  0x1805


#define BLE_UUID_CHAR_DETECTOR_NUMBER_OF_KILLS            0xFEE1


#include <stdint.h>
#include <string.h>
#include "nrf_gpio.h"
#include "ble_srv_common.h"
#include "app_error.h"

#include "ble/ble_manager.h"
#include "ble/ble_interface.h"
#include "ble/ble_service.h"
#include "ble/ble_characteristic.h"

#include "debug/DEBUG.h"





void BLE_Manager::createDetectorDataService() {

  Service detectorData;
  detectorData.createCustom(BLE_UUID_SERVICE_DETECTOR_DATA, BLE_UUID_GOODNATURE_BASE);

  Characteristic trapTriggered;
  trapTriggered.setUUID(BLE_UUID_CHAR_DETECTOR_NUMBER_OF_KILLS);
  trapTriggered.enableRead();
  trapTriggered.enableWrite();
  trapTriggered.enableNotification();

  uint8_t initValue = { 0x00 };
  trapTriggered.initValue(&initValue, 1);
  trapTriggered.setMaxLength(50);

  detectorData.addCharacteristic(&trapTriggered, CHAR_DETECTOR_NUMBER_OF_KILLS);

  detectorData.attachService();
  BLE::addService(&detectorData, SERVICE_DETECTOR_DATA);
}


void BLE_Manager::createDeviceInfoService() {

  Service deviceInfo;
  deviceInfo.createSIG(BLE_UUID_SIG_SERVICE_DEVICE_INFO);

  deviceInfo.attachService();
  BLE::addService(&deviceInfo, SERVICE_DEVICE_INFO);
}




void BLE_Manager::updateCharacteristic(uint8_t serviceID, uint8_t charID, uint8_t* p_data, uint16_t length)
{
  BLE::getService(serviceID)->getCharacteristic(charID)->update(p_data, length);
}

void BLE_Manager::notifyCharacteristic(uint8_t serviceID, uint8_t charID, uint8_t* p_data, uint16_t* length)
{
  BLE::getService(serviceID)->getCharacteristic(charID)->notify(p_data, length);
}


void BLE_Manager::checkService() {
  DEBUG("Service is working: %d", BLE::getService(SERVICE_DETECTOR_DATA)->isInit());
}

void BLE_Manager::checkChar() {
  uint8_t charIsInit = BLE::getService(SERVICE_DETECTOR_DATA)->getCharacteristic(CHAR_DETECTOR_NUMBER_OF_KILLS)->isInit();
  DEBUG("Char is working: %d", charIsInit);
}


void BLE_Manager::createBLEService() {
  BLE::init();
  DEBUG("BLE Manager Initialised.");

  createDetectorDataService();
  createDeviceInfoService();

  BLE::adv.start(APP_ADV_DEFAULT_INTERVAL);
  BLE::adv.advertiseName();
}



BLE_Manager & BLE_Manager::manager(){
  static BLE_Manager manager;
  return manager;
}
