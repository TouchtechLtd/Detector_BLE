
/*
 * ble_advertising.cpp
 *
 *  Created on: Feb 28, 2017
 *      Author: Michael McAdam
 */



#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "ble_advertising.h"
#include "ble_srv_common.h"
#include "libraries/ble/gn_ble_advertising.h"

#include "libraries/debug/DEBUG.h"

#define APP_ADV_TIMEOUT_IN_SECONDS      BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED   /**< The advertising time-out (in units of seconds). When set to 0, we will never time out. */
#define DEVICE_NAME                     "RuuviName"                         /**< Name of device. Will be included in the advertising data. */

#define GAP_BLE_OBSERVER_PRIO           3

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(40, UNIT_1_25_MS)        /**< Minimum acceptable connection interval (0.5 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(50, UNIT_1_25_MS)        /**< Maximum acceptable connection interval (1 second). */
#define SLAVE_LATENCY                   0                                       /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory time-out (4 seconds). */


#define NRF_LOG_MODULE_NAME BLE_ADVERTISING
NRF_LOG_MODULE_REGISTER();

namespace BLE_ADVERTISING
{

ble_advdata_t _advdata;
ble_advdata_t _scanrsp;
ble_advdata_manuf_data_t        manuf_data; // Variable to hold manufacturer specific data
ble_advdata_manuf_data_t        scan_response_data; // Variable to hold manufacturer specific data
uint16_t    _dataSize;
const char* _deviceName;

void updateAdvertisingData(void);

//BLE_ADVERTISING_DEF(m_advertising);                                             //!< Advertising module instance.


static void on_adv_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    //ret_code_t err_code;

  switch (p_ble_evt->header.evt_id)
      {
          case BLE_GAP_EVT_CONNECTED:
              INFO("Stopping Advertising");
              break;

          case BLE_GAP_EVT_DISCONNECTED:
              INFO("Starting Advertising");
              break;
      }
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
void params_init(void)
{
    setName(DEVICE_NAME);

    ble_gap_conn_params_t   gap_conn_params;
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;


    uint32_t err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    ERROR_CHECK(err_code);

    NRF_SDH_BLE_OBSERVER(m_gap_observer, GAP_BLE_OBSERVER_PRIO, on_adv_evt, NULL);
}


/**@brief Function for starting advertising.
 */
void start(uint16_t interval)
{

  ble_gap_adv_params_t adv_params;

    // Start advertising
    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    adv_params.p_peer_addr = NULL;
    adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    adv_params.interval    = interval;
    adv_params.timeout     = APP_ADV_TIMEOUT_IN_SECONDS;

    uint32_t err_code = sd_ble_gap_adv_start(&adv_params, 1);
    ERROR_CHECK(err_code);

}

/**@brief Function for starting advertising.
 */
void stop(void)
{
  uint32_t err_code = sd_ble_gap_adv_stop();
  ERROR_CHECK(err_code);
}


void updateInterval(uint16_t interval) {
  stop();
	start(interval);
}


void updateAdvertisingData(void) {

  _advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
  uint32_t err_code = ble_advdata_set(&_advdata, &_scanrsp);
  ERROR_CHECK(err_code);
}


void setName(const char * deviceName)
{
	ble_gap_conn_sec_mode_t sec_mode;
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

  uint32_t err_code = sd_ble_gap_device_name_set(&sec_mode,
                                                  (const uint8_t *)deviceName,
                                                  strlen(deviceName));
  ERROR_CHECK(err_code);

  _deviceName = deviceName;
}



void updateName(const char * deviceName) {
	setName(deviceName);
	advertiseName();
}

/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
void advertiseName(void)
{
    _advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    updateAdvertisingData();
}


void advertiseUUID(ble_uuid_t uuid)
{
  ble_uuid_t adv_uuids[] = { uuid };
  _advdata.uuids_complete.uuid_cnt = 1;
  _advdata.uuids_complete.p_uuids = adv_uuids;
  updateAdvertisingData();
}

void advertiseData(uint8_t * p_data, uint8_t i_len)
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


}
