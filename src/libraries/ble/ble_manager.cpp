
/*
 * ble_manager.cpp
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#include <ble/gn_ble_config.h>
#include <stdint.h>
#include <string.h>
#include "nrf_gpio.h"
#include "ble_srv_common.h"
#include "app_error.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"

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


#pragma pack(push, 1)
typedef struct
{
  uint8_t a;
  uint16_t b;
  uint8_t c;
} test_t;
#pragma pack(pop)



#define BLE_UUID_SERVICE_DETECTOR_CONFIG                  0x1233
#define BLE_UUID_CHAR_DETECTOR_CONFIG                  0x1234


void BLE_Manager::setConnectionHandler(ble_manager_handler_t connectionHandler)
{
  m_connectionHandler = connectionHandler;
}
void BLE_Manager::setDisconnectHandler(ble_manager_handler_t disconnectHandler)
{
  m_disconnectHandler = disconnectHandler;
}




void BLE_Manager::setCharacteristic(uint8_t serviceID, uint8_t charID, void* p_data, uint16_t length)
{
  BLE_SERVER::getService(serviceID)->getCharacteristic(charID)->set(p_data, length);
}



void BLE_Manager::setWriteHandler(uint8_t serviceID, uint8_t charID, char_write_handler_t writeHandler)
{
  BLE_SERVER::getService(serviceID)->getCharacteristic(charID)->setWriteHandler(writeHandler);
}




void BLE_Manager::createBLEServer() {


  DEBUG("BLE Manager Initialised.");

  BLE_ADVERTISING::start(320);
  BLE_ADVERTISING::advertiseName();
  //gn_ble_adv_advertiseUUID(BLE::getService(SERVICE_DETECTOR_DATA)->getUUID());

  //BLE_SERVER::setPower(BLE_POWER_LEVEL_HIGH);


  //NRF_SDH_BLE_OBSERVER(m_ble_manager_observer, APP_BLE_MANAGER_OBSERVER_PRIO, bleEventHandler, NULL);
  //BLE::setExternalHandler(bleEventHandler);

}



BLE_Manager & BLE_Manager::manager(){
  static BLE_Manager manager;
  return manager;
}
