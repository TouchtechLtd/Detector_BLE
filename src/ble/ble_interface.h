
/*
 * ble_interface.h
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#ifndef _GOODNATURE_BLE_BLE_INTERFACE_H_
#define _GOODNATURE_BLE_BLE_INTERFACE_H_

#include "ble_conn_params.h"
#include "ble/ble_service.h"
#include "gn_ble_advertising.h"

#define MAX_NUMBER_SERVICES 10
//#define BLE_UUID_OUR_BASE_UUID              {{0x23, 0xD1, 0x13, 0xEF, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}} // 128-bit base UUID



/** Available Power Modes for the LIS2DH12 */
typedef enum{
  BLE_POWER_N_40_DB = -40,
  BLE_POWER_N_30_DB = -20,
  BLE_POWER_N_20_DB = -20,
  BLE_POWER_N_16_DB = -16,
  BLE_POWER_N_12_DB = -12,
  BLE_POWER_N_8_DB  = -8,
  BLE_POWER_N_4_DB  = -4,
  BLE_POWER_0_DB    = 0,
  BLE_POWER_3_DB    = 3,
  BLE_POWER_4_DB    = 4
} BLEPowerLevel;


typedef void (*ble_external_handler_t)(ble_evt_t const * p_evt);


class BLE {
	private:
		static void ble_stack_init();
		static void gap_params_init();
		static void gatt_init();
		static void conn_params_init();
		static void on_conn_params_evt(ble_conn_params_evt_t * p_evt);
		static void conn_params_error_handler(uint32_t nrf_error);

		static Service serviceList[MAX_NUMBER_SERVICES];
		static uint8_t _serviceCount;
		static ble_uuid128_t _base_uuid;

		static bool m_isConnected;

		static ble_external_handler_t m_externalHandler;

	public:
		static void init(void);
		static void setExternalHandler(void);
		static void setDeviceName(const char* deviceName);

		static void addService(Service* service, uint8_t serviceID);
		static Service* getService(uint8_t serviceID);

		static void on_ble_evt(ble_evt_t const * p_ble_evt, void* context);
		static void ble_evt_dispatch(ble_evt_t const * p_ble_evt);

		static bool isConnected();

		static void setExternalHandler(ble_external_handler_t externalHandler);

		static void setPower(BLEPowerLevel powerLevel);


	}; // End BLE




#endif /* _GOODNATURE_BLE_BLE_INTERFACE_H_ */
