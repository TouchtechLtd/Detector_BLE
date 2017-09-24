
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


// FROM_SERVICE_TUTORIAL: Defining 16-bit service and 128-bit base UUIDs
#define BLE_UUID_OUR_SERVICE_UUID_1                0xF00D // Just a random, but recognizable value
#define BLE_UUID_OUR_SERVICE_UUID_2                0xFEED // Just a random, but recognizable value



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

		ble_gs_t 			_service;
    ble_uuid128_t		_base_uuid;
    uint8_t				_charCount;
    uint16_t 			_id;

    void _init();

	public:
    Service();
    Service(uint16_t custom_uuid, ble_uuid128_t base_uuid);
    Service(uint16_t sig_uuid);
		void createCustomService(uint16_t uuid, ble_uuid128_t base_uuid);
		void createSIGService(uint16_t uuid);
		ble_char_id_t addCharacteristic(uint16_t char_uuid);
		ble_char_id_t addCharacteristic(Characteristic* p_char);
		void eventHandler(ble_evt_t * p_ble_evt);

		uint16_t getID() { return _id; }
		uint8_t getCharCount() { return _charCount; }
		uint8_t returnOne() { return 1; }

    Characteristic*	_charList[MAX_NUMBER_CHAR];

};	// SERVICE
#endif  /* _ _GOODNATURE_BLE_BLE_SERVICE_H__ */
