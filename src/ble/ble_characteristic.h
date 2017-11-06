
/*
 * ble_characteristic.h
 *
 *  Created on: September 5, 2017
 *      Author: Michael McAdam
 */

#ifndef _GOODNATURE_BLE_BLE_CHARACTERISTIC_H__
#define _GOODNATURE_BLE_BLE_CHARACTERISTIC_H__

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"


typedef struct
{
    uint8_t						id;
}ble_char_id_t;


typedef void (*char_write_handler_t) (uint8_t const* data, uint16_t len);

class Characteristic {
	private:
		ble_gatts_char_md_t 		_char_md;
		ble_gatts_attr_md_t 		_cccd_md;
		ble_gatts_attr_t    		_attr_char_value;
		ble_uuid_t          		_char_uuid;
		ble_uuid128_t				_base_uuid;
		ble_gatts_attr_md_t 		_attr_md;
		ble_gatts_hvx_params_t 		_hvx_params;
		ble_gatts_char_handles_t 	_char_handle;

		uint16_t _conn_handle;
		char_write_handler_t m_writeHandler;

		bool m_isUUIDSet;
		bool m_isRunning;
		bool _charAdded;
		bool _notificationEnabled;
		bool _readEnabled;

		void _init();

    void notify(void * i_data, uint16_t data_length);
    void update(void * i_data, uint16_t data_length);
	public:
	    Characteristic();
	    Characteristic(uint16_t i_uuid);

	    void setUUID (uint16_t i_uuid);
	    void setUUIDType(uint8_t i_type);
	    void configureUUID (uint16_t i_uuid, uint8_t i_type);
	    void attachToService(uint16_t i_serviceHandle);
	    void add(uint16_t i_serviceHandle, uint16_t i_uuid, uint8_t i_uuidType);
	    void set(void* i_data, uint16_t dataLength);
	    void enableRead();
	    void disableRead();
	    void enableWrite();
	    void disableWrite();
	    void enableNotification();
	    void initValue(void* p_value, uint16_t i_len);
	    void setMaxLength(uint16_t i_maxLen);
	    void eventHandler(ble_evt_t const * p_ble_evt);

	    void setConnHandle(uint16_t i_connHandle);
	    void setWriteHandler(char_write_handler_t writeHandler);

	    uint8_t isInit();
	    bool isRunning() { return m_isRunning; }

};	// CHARACTERISTIC


#endif  /* _ _GOODNATURE_BLE_BLE_CHARACTERISTIC_H__ */

