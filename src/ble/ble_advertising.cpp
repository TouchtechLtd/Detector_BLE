
/*
 * ble_advertising.cpp
 *
 *  Created on: Feb 28, 2017
 *      Author: Michael McAdam
 */


#include <stdint.h>
#include <string.h>
#include "nrf_gpio.h"
#include "ble/ble_advertising.h"
#include "ble_srv_common.h"

#include "debug/DEBUG.h"

#define APP_ADV_TIMEOUT_IN_SECONDS      BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED   /**< The advertising time-out (in units of seconds). When set to 0, we will never time out. */


Advertising::Advertising() {
	_advdata = {};
	_scanrsp = {};
	_dataSize = 0;
	_deviceName = NULL;
}


/**@brief Function for starting advertising.
 */
void Advertising::start(uint16_t interval)
{
    ble_gap_adv_params_t adv_params;

    // Start advertising
    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    adv_params.p_peer_addr = NULL;
    adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    adv_params.interval    = interval;
    adv_params.timeout     = APP_ADV_TIMEOUT_IN_SECONDS;

    uint32_t err_code = sd_ble_gap_adv_start(&adv_params, BLE_CONN_CFG_TAG_DEFAULT);
    ERROR_CHECK(err_code);
}

/**@brief Function for starting advertising.
 */
void Advertising::stop(void)
{
  uint32_t err_code = sd_ble_gap_adv_stop();
  ERROR_CHECK(err_code);
}


void Advertising::updateInterval(uint16_t interval) {
	stop();
	start(interval);
}


void Advertising::updateAdvertisingData(void) {

  _advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
  uint32_t err_code = ble_advdata_set(&_advdata, &_scanrsp);
  ERROR_CHECK(err_code);
}


void Advertising::setName(const char * deviceName)
{
	ble_gap_conn_sec_mode_t sec_mode;
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

  uint32_t err_code = sd_ble_gap_device_name_set(&sec_mode,
                                                  (const uint8_t *)deviceName,
                                                  strlen(deviceName));
  ERROR_CHECK(err_code);

  _deviceName = deviceName;
}



void Advertising::updateName(const char * deviceName) {
	setName(deviceName);
	advertiseName();
}

/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
void Advertising::advertiseName(void)
{
    _advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    updateAdvertisingData();
}


void Advertising::advertiseUUID(ble_uuid_t uuid)
{
  ble_uuid_t adv_uuids[] = { uuid };
  _advdata.uuids_complete.uuid_cnt = 1;
  _advdata.uuids_complete.p_uuids = adv_uuids;
  updateAdvertisingData();
}

void Advertising::advertiseData(uint8_t * p_data, uint8_t i_len)
{
    memset(&manuf_data, 0, sizeof(manuf_data));

    uint8_t data_len = 0;
    uint8_t response_len = 0;

    if (i_len <= 10)
    {
    	data_len = i_len;
    }
    else if (i_len <= 20)
    {
    	data_len = 10;
    	response_len = i_len - 10;
    }
    else
    {
    	data_len = 10;
    	response_len = 10;
    }

    manuf_data.company_identifier       = 0x0059; // Nordics company ID
    manuf_data.data.p_data              = p_data;
    manuf_data.data.size                = data_len;
    _advdata.p_manuf_specific_data = &manuf_data;

    if (i_len > 10) {
		memset(&manuf_data, 0, sizeof(manuf_data));
		//scan_response_data.company_identifier       = 0xFFFF; // Nordics company ID
		scan_response_data.data.p_data              = &p_data[10];
		scan_response_data.data.size                = response_len;
		_scanrsp.p_manuf_specific_data = &scan_response_data;
    }

    updateAdvertisingData();

}
