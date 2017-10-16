/*
 * ble_advertising.h
 *
 *  Created on: September 5, 2017
 *      Author: Michael McAdam
 */

#ifndef _GOODNATURE_BLE_BLE_ADVERTISING_H__
#define _GOODNATURE_BLE_BLE_ADVERTISING_H__

#include <stdint.h>
#include "ble.h"
#include "ble_advdata.h"
#include "ble_srv_common.h"

#define APP_ADV_DEFAULT_INTERVAL                BLE_GAP_SCAN_INTERVAL_MAX                                      /**< The advertising interval (in units of 0.625 ms; this value corresponds to 40 ms). */

class Advertising {
	private:
		ble_advdata_t _advdata;
		ble_advdata_t _scanrsp;
		ble_advdata_manuf_data_t        manuf_data; // Variable to hold manufacturer specific data
		ble_advdata_manuf_data_t        scan_response_data; // Variable to hold manufacturer specific data
		uint16_t		_dataSize;
		const char*	_deviceName;

		void updateAdvertisingData(void);
	public:
		Advertising();
	    void init();
	    void start(uint16_t interval);
	    void stop();
	    void updateInterval(uint16_t interval);

	    void setName(const char* deviceName);
	    void updateName(const char * deviceName);
	    void advertiseName();
	    void advertiseUUID(ble_uuid_t uuid);
	    void advertiseData(uint8_t * p_data, uint8_t i_len);

};	// SERVICE
#endif  /* _GOODNATURE_BLE_BLE_ADVERTISING_H__ */
