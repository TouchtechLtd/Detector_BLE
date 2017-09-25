
/*
 * ble_manager.cpp
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */


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
  detectorData.createCustom(0xC001, BLE_UUID_OUR_BASE_UUID);

  Characteristic trapTriggered;
  trapTriggered.setUUID(0xFEE1);
  trapTriggered.enableRead();
  trapTriggered.enableNotification();
  uint8_t initValue = { 0x00 };
  trapTriggered.initValue(&initValue, 1);
  trapTriggered.setMaxLength(50);
  detectorData.addCharacteristic(&trapTriggered, CHAR_DETECTOR_NUMBER_OF_KILLS);

  detectorData.attachService();
  BLE::addService(&detectorData, SERVICE_DETECTOR_DATA);
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

  BLE::adv.start(APP_ADV_DEFAULT_INTERVAL);
  BLE::adv.advertiseName();
}



BLE_Manager & BLE_Manager::manager(){
  static BLE_Manager manager;
  return manager;
}
