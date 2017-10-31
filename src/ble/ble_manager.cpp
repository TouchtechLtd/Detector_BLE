
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
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"

#include "ble/ble_manager.h"
#include "ble/ble_interface.h"
#include "ble/ble_service.h"
#include "ble/ble_characteristic.h"
#include "ble/gn_ble_advertising.h"

#include "debug/DEBUG.h"

#define APP_BLE_MANAGER_OBSERVER_PRIO           3

#define BLE_UUID_OUR_CHARACTERISTC_UUID          0xBEEF // Just a random, but recognizable value
#define BLE_UUID_GOODNATURE_BASE              {{0x23, 0xD1, 0x13, 0xEF, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}} // 128-bit base UUID

// Defining 16-bit service and 128-bit base UUIDs
#define BLE_UUID_OUR_SERVICE_UUID_1                0xF00D // Just a random, but recognizable value
#define BLE_UUID_OUR_SERVICE_UUID_2                0xFEED // Just a random, but recognizable value


#define BLE_UUID_SERVICE_DETECTOR_DATA                     0xC001
#define BLE_UUID_SIG_SERVICE_DEVICE_INFO                   0x180A
#define BLE_UUID_SIG_SERVICE_BATTERY_LEVEL                 0x180F
#define BLE_UUID_SIG_SERVICE_CURRENT_TIME                  0x1805

#define BLE_UUID_SERVICE_CURRENT_TIME                      0xBEAD
#define BLE_UUID_CHAR_TIME_IN_MINS                         0xBEE5

#define BLE_UUID_SERVICE_DEVICE_CONTROL                    0xDEAD
#define BLE_UUID_CHAR_DEVICE_TRIGGER_THRESHOLD             0xDEED
#define BLE_UUID_CHAR_DEVICE_MOVE_THRESHOLD                0xD1ED
#define BLE_UUID_CHAR_DEVICE_TRIGGER_DURATION              0xD2ED
#define BLE_UUID_CHAR_DEVICE_MOVE_DURATION                 0xD3ED
#define BLE_UUID_CHAR_DEVICE_TRIGGER_BUFFER_LENGTH         0xD4ED
#define BLE_UUID_CHAR_DEVICE_MOVE_BUFFER_LENGTH            0xD5ED
#define BLE_UUID_CHAR_DEVICE_SET_BUFFER_LENGTH             0xD6ED
#define BLE_UUID_CHAR_DEVICE_OUTPUT_RAW                    0xD7ED
#define BLE_UUID_CHAR_DEVICE_OUTPUT_STATE                  0xD8ED


enum BLE_UUID_DetectorData {
  BLE_UUID_CHAR_DETECTOR_NUMBER_OF_KILLS = 0xFEE1,
  BLE_UUID_CHAR_DETECTOR_KILL_TIME,
  BLE_UUID_CHAR_DETECTOR_DID_CLIP,
  BLE_UUID_CHAR_DETECTOR_PEAK_VALUE,
  BLE_UUID_CHAR_DETECTOR_RESPONSE_SIZE,
  BLE_UUID_CHAR_DETECTOR_RESPONSE_LENGTH
};



uint8_t* bit16Converter(uint16_t inputInt)
{
  static uint8_t result[2];
  result[0] = (inputInt & 0x00ff);
  result[1] = (inputInt & 0xff00) >> 8;
  return result;
};

uint8_t* bit32Converter(uint32_t inputInt)
{
  static uint8_t result[4];
  result[0] = (inputInt & 0x000000ff);
  result[1] = (inputInt & 0x0000ff00) >> 8;
  result[2] = (inputInt & 0x00ff0000) >> 16;
  result[3] = (inputInt & 0xff000000) >> 24;
  return result;
};


void BLE_Manager::setConnectionHandler(ble_manager_handler_t connectionHandler)
{
  m_connectionHandler = connectionHandler;
}
void BLE_Manager::setDisconnectHandler(ble_manager_handler_t disconnectHandler)
{
  m_disconnectHandler = disconnectHandler;
}



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


  //// Kill time characteristic ////
  Characteristic killTime;
  killNumber.setUUID(BLE_UUID_CHAR_DETECTOR_KILL_TIME);
  killNumber.enableRead();
  killNumber.enableNotification();

  killNumber.initValue(&initValue, 1);
  killNumber.setMaxLength(4);

  detectorData.addCharacteristic(&killNumber, CHAR_DETECTOR_KILL_TIME);

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

void BLE_Manager::createCurrentTimeService() {

  Service currentTime;
  currentTime.createCustom(BLE_UUID_SERVICE_CURRENT_TIME, BLE_UUID_GOODNATURE_BASE);

  uint8_t initValue = { 0x00 };

  //// Current time in minutes characteristic ////
  Characteristic timeInMins;
  timeInMins.setUUID(BLE_UUID_CHAR_TIME_IN_MINS);
  timeInMins.enableRead();
  timeInMins.enableWrite();
  timeInMins.enableNotification();

  timeInMins.initValue(&initValue, 1);
  timeInMins.setMaxLength(4);

  currentTime.addCharacteristic(&timeInMins, CHAR_TIME_IN_MINS);

  currentTime.attachService();
  BLE::addService(&currentTime, SERVICE_CURRENT_TIME);
}

void BLE_Manager::createDeviceControlService() {

  Service deviceControl;
  deviceControl.createCustom(BLE_UUID_SERVICE_DEVICE_CONTROL, BLE_UUID_GOODNATURE_BASE);

  uint8_t initValue[] = { 0x00, 0x00 };

  //// Trigger threshold ////
  Characteristic triggerThreshold;
  triggerThreshold.setUUID(BLE_UUID_CHAR_DEVICE_TRIGGER_THRESHOLD);
  triggerThreshold.enableRead();
  triggerThreshold.enableWrite();
  triggerThreshold.enableNotification();

  triggerThreshold.initValue(initValue, 2);
  triggerThreshold.setMaxLength(2);

  deviceControl.addCharacteristic(&triggerThreshold, CHAR_TRIGGER_THRESHOLD);

  //// Move threshold ////
  Characteristic moveThreshold;
  moveThreshold.setUUID(BLE_UUID_CHAR_DEVICE_MOVE_THRESHOLD);
  moveThreshold.enableRead();
  moveThreshold.enableWrite();
  moveThreshold.enableNotification();

  moveThreshold.initValue(initValue, 2);
  moveThreshold.setMaxLength(2);

  deviceControl.addCharacteristic(&moveThreshold, CHAR_MOVE_THRESHOLD);

  //// Trigger duration ////
  Characteristic triggerDuration;
  triggerDuration.setUUID(BLE_UUID_CHAR_DEVICE_TRIGGER_DURATION);
  triggerDuration.enableRead();
  triggerDuration.enableWrite();
  triggerDuration.enableNotification();

  triggerDuration.initValue(initValue, 1);
  triggerDuration.setMaxLength(1);

  deviceControl.addCharacteristic(&triggerDuration, CHAR_TRIGGER_DURATION);


  //// Move duration ////
  Characteristic moveDuration;
  moveDuration.setUUID(BLE_UUID_CHAR_DEVICE_MOVE_DURATION);
  moveDuration.enableRead();
  moveDuration.enableWrite();
  moveDuration.enableNotification();

  moveDuration.initValue(initValue, 1);
  moveDuration.setMaxLength(1);

  deviceControl.addCharacteristic(&moveDuration, CHAR_MOVE_DURATION);


  //// Trigger buffer length ////
  Characteristic triggerBufferLength;
  triggerBufferLength.setUUID(BLE_UUID_CHAR_DEVICE_TRIGGER_BUFFER_LENGTH);
  triggerBufferLength.enableRead();
  triggerBufferLength.enableWrite();
  triggerBufferLength.enableNotification();

  triggerBufferLength.initValue(initValue, 2);
  triggerBufferLength.setMaxLength(2);

  deviceControl.addCharacteristic(&triggerBufferLength, CHAR_TRIGGER_BUFFER_LENGTH);


  //// Move buffer length ////
  Characteristic moveBufferLength;
  moveBufferLength.setUUID(BLE_UUID_CHAR_DEVICE_MOVE_BUFFER_LENGTH);
  moveBufferLength.enableRead();
  moveBufferLength.enableWrite();
  moveBufferLength.enableNotification();

  moveBufferLength.initValue(initValue, 2);
  moveBufferLength.setMaxLength(2);

  deviceControl.addCharacteristic(&moveBufferLength, CHAR_MOVE_BUFFER_LENGTH);

  //// Move buffer length ////
  Characteristic setBufferLength;
  setBufferLength.setUUID(BLE_UUID_CHAR_DEVICE_SET_BUFFER_LENGTH);
  setBufferLength.enableRead();
  setBufferLength.enableWrite();
  setBufferLength.enableNotification();

  setBufferLength.initValue(initValue, 2);
  setBufferLength.setMaxLength(2);

  deviceControl.addCharacteristic(&setBufferLength, CHAR_SET_BUFFER_LENGTH);

  //// Output raw acc over UART ////
  Characteristic outputRaw;
  outputRaw.setUUID(BLE_UUID_CHAR_DEVICE_OUTPUT_RAW);
  outputRaw.enableRead();
  outputRaw.enableWrite();
  outputRaw.enableNotification();

  outputRaw.initValue(initValue, 1);
  outputRaw.setMaxLength(1);

  deviceControl.addCharacteristic(&outputRaw, CHAR_OUTPUT_RAW);

  //// Output state ////
  Characteristic outputState;
  outputState.setUUID(BLE_UUID_CHAR_DEVICE_OUTPUT_STATE);
  outputState.enableRead();
  outputState.enableNotification();

  outputState.initValue(initValue, 1);
  outputState.setMaxLength(1);

  deviceControl.addCharacteristic(&outputState, CHAR_OUTPUT_STATE);


  deviceControl.attachService();
  BLE::addService(&deviceControl, SERVICE_DEVICE_CONTROL);
}
/*
void BLE_Manager::updateCharacteristic(uint8_t serviceID, uint8_t charID, uint8_t* p_data, uint16_t length)
{
  BLE::getService(serviceID)->getCharacteristic(charID)->update(p_data, length);
}

void BLE_Manager::notifyCharacteristic(uint8_t serviceID, uint8_t charID, uint8_t* p_data, uint16_t length)
{
  uint16_t localLength = length;
  BLE::getService(serviceID)->getCharacteristic(charID)->notify(p_data, &localLength);
}
*/
void BLE_Manager::setCharacteristic(uint8_t serviceID, uint8_t charID, uint8_t* p_data, uint16_t length)
{
  BLE::getService(serviceID)->getCharacteristic(charID)->set(p_data, length);
}


void BLE_Manager::checkService() {
  DEBUG("Service is working: %d", BLE::getService(SERVICE_DETECTOR_DATA)->isInit());
}

void BLE_Manager::checkChar() {
  uint8_t charIsInit = BLE::getService(SERVICE_DETECTOR_DATA)->getCharacteristic(CHAR_DETECTOR_NUMBER_OF_KILLS)->isInit();
  DEBUG("Char is working: %d", charIsInit);
}


void BLE_Manager::setPower(BLEPowerLevel powerLevel)
{
  BLE::setPower(powerLevel);
}


void BLE_Manager::setWriteHandler(uint8_t serviceID, uint8_t charID, char_write_handler_t writeHandler)
{
  BLE::getService(serviceID)->getCharacteristic(charID)->setWriteHandler(writeHandler);
}

void BLE_Manager::bleEventHandler(ble_evt_t const * p_ble_evt, void* context)
{
  manager().m_bleEventHandler(p_ble_evt);
}


void BLE_Manager::m_bleEventHandler(ble_evt_t const * p_ble_evt)
{
  /*
  switch (p_ble_evt->header.evt_id)
     {

         case BLE_GAP_EVT_CONNECTED:
           m_connectionHandler();
           break; // BLE_GAP_EVT_CONNECTED

         case BLE_GAP_EVT_DISCONNECTED:
           m_disconnectHandler();
           break;

         case BLE_GATTS_EVT_WRITE:
            //DEBUG("Handle: %d", p_ble_evt->evt.gatts_evt.params.write.handle);
            break;

         default:
           //DEBUG("Unhandled event: %d", p_ble_evt->header.evt_id);
           break;
     }
     */
}



void BLE_Manager::createBLEServer() {

  BLE::init();

  DEBUG("BLE Manager Initialised.");

  createDetectorDataService();
  createDeviceInfoService();
  createCurrentTimeService();
  createDeviceControlService();

  gn_ble_adv_start(320);
  gn_ble_adv_advertiseName();
  gn_ble_adv_advertiseUUID(BLE::getService(SERVICE_DETECTOR_DATA)->getUUID());

  setPower(BLE_POWER_LEVEL_HIGH);


  NRF_SDH_BLE_OBSERVER(m_ble_manager_observer, APP_BLE_MANAGER_OBSERVER_PRIO, bleEventHandler, NULL);
  //BLE::setExternalHandler(bleEventHandler);

}



BLE_Manager & BLE_Manager::manager(){
  static BLE_Manager manager;
  return manager;
}
