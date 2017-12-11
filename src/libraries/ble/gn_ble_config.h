
/*
 * ble_manager.h
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#ifndef _GOODNATURE_BLE_BLE_CONFIG_H__
#define _GOODNATURE_BLE_BLE_CONFIG_H__

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "libraries/ble/ble_interface.h"


#define  APP_MAIN_OBSERVER_PRIO           3

// UUIDs //
#define  BLE_POWER_LEVEL_HIGH       BLE_SERVER::BLE_POWER_4_DB
#define  BLE_POWER_LEVEL_LOW        BLE_SERVER::BLE_POWER_N_40_DB

#define BLE_ADVERTISING_SPEED_FAST  320
#define BLE_ADVERTISING_SPEED_SLOW  2000

#define BLE_UUID_GOODNATURE_BASE              {{0x23, 0xD1, 0x13, 0xEF, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}} // 128-bit base UUID

#define BLE_UUID_SERVICE_TRAP_DATA                    0xDEAD
#define BLE_UUID_CHAR_TRAP_EVENT_DATA                 0xDEED
#define BLE_UUID_CHAR_TRAP_EVENT_CONFIG               0xD1ED
#define BLE_UUID_CHAR_TRAP_EVENT_DISPLAYED            0xD2ED
#define BLE_UUID_CHAR_TRAP_TIME                       0xD3ED
#define BLE_UUID_CHAR_RAW_DATA                        0xD4ED
#define BLE_UUID_CHAR_ERROR_DATA                      0xD5ED
#define BLE_UUID_CHAR_TRAP_INFO                       0xD6ED
#define BLE_UUID_CHAR_TRAP_CONTROL                    0xD7ED

#define BLE_UUID_SIG_SERVICE_DEVICE_INFO                   0x180A
#define BLE_UUID_SIG_SERVICE_BATTERY_LEVEL                 0x180F
#define BLE_UUID_SIG_SERVICE_CURRENT_TIME                  0x1805

enum Services {
  SERVICE_DEVICE_INFO,
  SERVICE_BATTERY_LEVEL,
  SERVICE_TRAP_DATA
};


enum Characteristic_DeviceInfo {
  CHAR_DEVICE_INFO_MANUFACTURER_NAME,
  CHAR_DEVICE_INFO_MODEL_NUMBER,
  CHAR_DEVICE_INFO_SERIAL_NUMBER,
  CHAR_DEVICE_INFO_HARDWARE_REVISION,
  CHAR_DEVICE_INFO_FIRMWARE_REVISION,
  CHAR_DEVICE_INFO_SYSTEM_ID
};


enum Characteristic_BatteryLevel {
  CHAR_BATTERY_LEVEL
};


enum Characteristic_TrapData
{
  CHAR_EVENT_DATA,
  CHAR_EVENT_CONFIG,
  CHAR_EVENT_DISPLAYED,
  CHAR_TRAP_TIME,
  CHAR_RAW_DATA,
  CHAR_ERROR_DATA,
  CHAR_TRAP_INFO,
  CHAR_TRAP_CONTROL
};

#endif  /* _ _GOODNATURE_BLE_BLE_MANAGER_H__ */
