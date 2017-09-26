
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
#include "ble/ble_advertising.h"

#define MAX_NUMBER_SERVICES 10
//#define BLE_UUID_OUR_BASE_UUID              {{0x23, 0xD1, 0x13, 0xEF, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}} // 128-bit base UUID



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

	public:
		static void init(void);
		static void setDeviceName(const char* deviceName);

		static void addService(Service* service, uint8_t serviceID);
		static Service* getService(uint8_t serviceID);

		static void on_ble_evt(ble_evt_t * p_ble_evt);
		static void ble_evt_dispatch(ble_evt_t * p_ble_evt);
		static Advertising adv;

		static bool isConnected();


	}; // End BLE




#endif /* _GOODNATURE_BLE_BLE_INTERFACE_H_ */
