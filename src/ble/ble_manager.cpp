
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

#define BLE_UUID_OUR_CHARACTERISTC_UUID          0xBEEF // Just a random, but recognizable value
#define BLE_UUID_GOODNATURE_BASE              {{0x23, 0xD1, 0x13, 0xEF, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}} // 128-bit base UUID

// Defining 16-bit service and 128-bit base UUIDs
#define BLE_UUID_OUR_SERVICE_UUID_1                0xF00D // Just a random, but recognizable value
#define BLE_UUID_OUR_SERVICE_UUID_2                0xFEED // Just a random, but recognizable value


#define BLE_UUID_SERVICE_DETECTOR_DATA                     0xC001
#define BLE_UUID_SIG_SERVICE_DEVICE_INFO                   0x180A
#define BLE_UUID_SIG_SERVICE_BATTERY_LEVEL                 0x180F
#define BLE_UUID_SIG_SERVICE_CURRENT_TIME                  0x1805

enum BLE_UUID_DetectorData {
  BLE_UUID_CHAR_DETECTOR_NUMBER_OF_KILLS = 0xFEE1,
  BLE_UUID_CHAR_DETECTOR_DID_CLIP,
  BLE_UUID_CHAR_DETECTOR_PEAK_VALUE,
  BLE_UUID_CHAR_DETECTOR_RESPONSE_SIZE,
  BLE_UUID_CHAR_DETECTOR_RESPONSE_LENGTH
};





void BLE_Manager::createDetectorDataService() {

  Service detectorData;
  detectorData.createCustom(BLE_UUID_SERVICE_DETECTOR_DATA, BLE_UUID_GOODNATURE_BASE);

  uint8_t initValue = { 0x00 };

  //// Kill number characteristic ////
  Characteristic killNumber;
  killNumber.setUUID(BLE_UUID_CHAR_DETECTOR_NUMBER_OF_KILLS);
  killNumber.enableRead();
  killNumber.enableNotification();

  killNumber.initValue(&initValue, 1);
  killNumber.setMaxLength(1);

  detectorData.addCharacteristic(&killNumber, CHAR_DETECTOR_NUMBER_OF_KILLS);

  //// Did clip characteristic ////
  Characteristic didClip;
  didClip.setUUID(BLE_UUID_CHAR_DETECTOR_DID_CLIP);
  didClip.enableRead();
  didClip.enableNotification();
  didClip.initValue(&initValue, 1);
  didClip.setMaxLength(1);

  detectorData.addCharacteristic(&didClip, CHAR_DETECTOR_DID_CLIP);


  //// Peak Value characteristic ////
  Characteristic peakValue;
  peakValue.setUUID(BLE_UUID_CHAR_DETECTOR_PEAK_VALUE);
  peakValue.enableRead();
  peakValue.enableNotification();
  peakValue.initValue(&initValue, 1);
  peakValue.setMaxLength(2);

  detectorData.addCharacteristic(&peakValue, CHAR_DETECTOR_PEAK_VALUE);


  //// Response size characteristic ////
  Characteristic responseSize;
  responseSize.setUUID(BLE_UUID_CHAR_DETECTOR_RESPONSE_SIZE);
  responseSize.enableRead();
  responseSize.enableNotification();
  responseSize.initValue(&initValue, 1);
  responseSize.setMaxLength(2);

  detectorData.addCharacteristic(&responseSize, CHAR_DETECTOR_RESPONSE_SIZE);


  //// Response length characteristic ////
  Characteristic responseLength;
  responseLength.setUUID(BLE_UUID_CHAR_DETECTOR_RESPONSE_LENGTH);
  responseLength.enableRead();
  responseLength.enableNotification();
  responseLength.initValue(&initValue, 1);
  responseLength.setMaxLength(4);

  detectorData.addCharacteristic(&responseLength, CHAR_DETECTOR_RESPONSE_LENGTH);


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

void BLE_Manager::notifyCharacteristic(uint8_t serviceID, uint8_t charID, uint8_t* p_data, uint16_t length)
{
  uint16_t localLength = length;
  BLE::getService(serviceID)->getCharacteristic(charID)->notify(p_data, &localLength);
}

void BLE_Manager::setCharacteristic(uint8_t serviceID, uint8_t charID, uint8_t* p_data, uint16_t length)
{
  if (BLE::isConnected()) { notifyCharacteristic(serviceID, charID, p_data, length); }
  else                    { updateCharacteristic(serviceID, charID, p_data, length); }
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
  BLE::adv.advertiseUUID(BLE::getService(SERVICE_DETECTOR_DATA)->getUUID());
}




BLE_Manager & BLE_Manager::manager(){
  static BLE_Manager manager;
  return manager;
}
