
/*
 * ble_service.h
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#ifndef _GOODNATURE_BLE_BLE_SERVICE_H__
#define _GOODNATURE_BLE_BLE_SERVICE_H__

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "ble/ble_characteristic.h"


#define MAX_NUMBER_CHAR	10

// This structure contains various status information for our service.
// The name is based on the naming convention used in Nordics SDKs.
// 'ble’ indicates that it is a Bluetooth Low Energy relevant structure and
// ‘os’ is short for Our Service).
typedef struct
{
    uint16_t                    conn_handle;    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection).*/
    uint16_t                    service_handle; /**< Handle of Our Service (as provided by the BLE stack). */
    ble_uuid_t        			    uuid;
} ble_gs_t;

class Service {
	private:
		static uint16_t serviceCount;

		ble_gs_t 			    _service;
    ble_uuid128_t		  _base_uuid;
    uint8_t				    _charCount;
    uint16_t 			    _id;

    uint8_t           m_isUUIDSet;
    bool              m_isRunning;

    void _init();

	public:
    Service();
    Service(uint16_t custom_uuid, ble_uuid128_t base_uuid);
    Service(uint16_t sig_uuid);
    void attachService();
		void createCustom(uint16_t uuid, ble_uuid128_t base_uuid);
		void createSIG(uint16_t uuid);
		void addCharacteristic(Characteristic* p_char, uint8_t charID);
		Characteristic* getCharacteristic(uint8_t charID);
		void eventHandler(ble_evt_t * p_ble_evt);

		uint16_t getID() { return _id; }
		uint8_t getCharCount() { return _charCount; }
		uint8_t isInit() { return m_isUUIDSet; }
		bool isRunning() { return m_isRunning; }
		ble_uuid_t getUUID() { return _service.uuid; }

    Characteristic	_charList[MAX_NUMBER_CHAR];

};	// SERVICE
#endif  /* _ _GOODNATURE_BLE_BLE_SERVICE_H__ */
