
/*
 * ble_manager.h
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#ifndef _GOODNATURE_BLE_BLE_MANAGER_H__
#define _GOODNATURE_BLE_BLE_MANAGER_H__

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "ble/ble_characteristic.h"

enum Services {
  SERVICE_DETECTOR_DATA,
  SERVICE_DEVICE_INFO,
  SERVICE_BATTERY_LEVEL,
  SERVICE_CURRENT_TIME
};



enum Characteristic_DetectorData {
  CHAR_DETECTOR_NUMBER_OF_KILLS
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


enum Characteristic_CurrentTime {
  CHAR_CURRENT_TIME
};


class BLE_Manager {
	private:

	public:
    void createBLEService();

    void checkService();
    void checkChar();
    void createDetectorDataService();

    static BLE_Manager & manager();

};	// BLE_Manager
#endif  /* _ _GOODNATURE_BLE_BLE_MANAGER_H__ */
