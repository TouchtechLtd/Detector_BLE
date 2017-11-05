
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
#include "ble/ble_interface.h"



#define  BLE_POWER_LEVEL_HIGH BLE_POWER_4_DB
#define  BLE_POWER_LEVEL_LOW  BLE_POWER_N_40_DB


typedef void (*ble_manager_handler_t)(void);

enum Services {
  SERVICE_DETECTOR_DATA,
  SERVICE_DEVICE_INFO,
  SERVICE_BATTERY_LEVEL,
  SERVICE_CURRENT_TIME,
  SERVICE_DEVICE_CONTROL,
  SERVICE_DETECTOR_CONFIG
};

enum Characteristic_DetectorData {
  CHAR_DETECTOR_NUMBER_OF_KILLS,
  CHAR_DETECTOR_KILL_TIME,
  CHAR_DETECTOR_DID_CLIP,
  CHAR_DETECTOR_PEAK_VALUE,
  CHAR_DETECTOR_RESPONSE_SIZE,
  CHAR_DETECTOR_RESPONSE_LENGTH
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
  CHAR_TIME_IN_MINS
};

enum Characteristic_DeviceControl {
  CHAR_TRIGGER_THRESHOLD,
  CHAR_MOVE_THRESHOLD,
  CHAR_TRIGGER_DURATION,
  CHAR_MOVE_DURATION,
  CHAR_TRIGGER_BUFFER_LENGTH,
  CHAR_MOVE_BUFFER_LENGTH,
  CHAR_SET_BUFFER_LENGTH,
  CHAR_OUTPUT_RAW,
  CHAR_OUTPUT_STATE
};

enum Characteristic_DetectorConfig
{
  CHAR_DETECTOR_CONFIG
};


uint8_t* bit16Converter(uint16_t inputInt);
uint8_t* bit32Converter(uint32_t inputInt);


class BLE_Manager {
	private:

    ble_manager_handler_t m_connectionHandler;
    ble_manager_handler_t m_disconnectHandler;

    static void bleEventHandler(ble_evt_t const * p_ble_evt, void* context);
    void m_bleEventHandler(ble_evt_t const * p_ble_evt);

    void checkService();
    void checkChar();

    void createDetectorDataService();
    void createDeviceInfoService();
    void createCurrentTimeService();
    void createDeviceControlService();
    void createDetectorConfigService();

	public:

    void createBLEServer();

    void updateCharacteristic(uint8_t serviceID, uint8_t charID, uint8_t* p_data, uint16_t length);
    void notifyCharacteristic(uint8_t serviceID, uint8_t charID, uint8_t* p_data, uint16_t length);
    void setCharacteristic(uint8_t serviceID, uint8_t charID, uint8_t* p_data, uint16_t length);


    void setPower(BLEPowerLevel powerLevel);
    void setWriteHandler(uint8_t serviceID, uint8_t charID, char_write_handler_t writeHandler);


    void setConnectionHandler(ble_manager_handler_t connectionHandler);
    void setDisconnectHandler(ble_manager_handler_t disconnectHandler);


    static BLE_Manager & manager();

};	// BLE_Manager
#endif  /* _ _GOODNATURE_BLE_BLE_MANAGER_H__ */
