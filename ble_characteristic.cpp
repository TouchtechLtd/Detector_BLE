#include <stdint.h>
#include <string.h>
#include "nrf_gpio.h"
#include "ble_characteristic.h"
#include "ble_srv_common.h"
#include "app_error.h"


#define NRF_LOG_MODULE_NAME "CHAR"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"


Characteristic::Characteristic() {
    // Add read/write properties to our characteristic

	memset(&_char_uuid, 0, sizeof(_char_uuid));
    memset(&_char_md, 0, sizeof(_char_md));
    // Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
    memset(&_cccd_md, 0, sizeof(_cccd_md));
    // Configure the attribute metadata
    memset(&_attr_md, 0, sizeof(_attr_md));
    memset(&_char_handle, 0, sizeof(_char_handle));
    memset(&_attr_char_value, 0, sizeof(_attr_char_value));
	memset(&_hvx_params, 0, sizeof(_hvx_params));

    _parent_service_handle = 0;



}




/**@brief Function for adding our new characterstic to "Our service" that we initiated in the previous tutorial.
 *
 * @param[in]   p_our_service        Our Service structure.
 *
 */

void Characteristic::add(uint16_t uuid, ble_uuid128_t i_base_uuid, uint16_t serviceHandle)
{

	uint32_t            err_code;
	_base_uuid			= i_base_uuid;
	_char_uuid.uuid      = uuid;
	err_code = sd_ble_uuid_vs_add(&_base_uuid, &_char_uuid.type);
	APP_ERROR_CHECK(err_code);



    _attr_md.vlen 		= 1;
	_attr_md.vloc        = BLE_GATTS_VLOC_STACK;

	memset(&_attr_char_value, 0, sizeof(_attr_char_value));
    // Configure the characteristic value attribute
    _attr_char_value.p_uuid      = &_char_uuid;
    _attr_char_value.p_attr_md   = &_attr_md;


    enableRead();
    disableWrite();
    enableNotification();


    // Set characteristic length in number of bytes
    uint8_t value[1] = { 0x00 };
    _attr_char_value.max_len     = BLE_GATTS_VAR_ATTR_LEN_MAX;
    _attr_char_value.init_len    = sizeof(value);
    _attr_char_value.p_value     = value;

    //  Add our new characteristic to the service

    err_code = sd_ble_gatts_characteristic_add(serviceHandle,
                                       &_char_md,
                                       &_attr_char_value,
                                       &_char_handle);

    //_parent_service_handle = serviceHandle;
    APP_ERROR_CHECK(err_code);

}


void Characteristic::enableRead() {
    _char_md.char_props.read = 1;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&_attr_md.read_perm);
}

void Characteristic::disableRead() {
    _char_md.char_props.read = 0;
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&_attr_md.read_perm);
}

void Characteristic::enableWrite() {
    _char_md.char_props.write = 1;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&_attr_md.write_perm);
}

void Characteristic::disableWrite() {
    _char_md.char_props.write = 0;
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&_attr_md.write_perm);
}


void Characteristic::enableNotification() {
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&_cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&_cccd_md.write_perm);
    _cccd_md.vloc                = BLE_GATTS_VLOC_STACK;
    _char_md.p_cccd_md           = &_cccd_md;
    _char_md.char_props.notify   = 1;
}


// Function to be called when updating characteristic value

void Characteristic::notify(uint8_t * i_data, uint16_t * data_length)
{
    // Update characteristic value
	if (_parent_service_handle != BLE_CONN_HANDLE_INVALID)
	{


		_hvx_params.handle = _char_handle.value_handle;
		_hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		_hvx_params.offset = 0;
		_hvx_params.p_len  = data_length;
		_hvx_params.p_data = i_data;

		sd_ble_gatts_hvx(_parent_service_handle, &_hvx_params);
	}

}


void Characteristic::update(uint8_t * i_data, uint16_t data_length)
{
	uint32_t err_code;
	ble_gatts_value_t new_value;

	memset(&new_value, 0, sizeof(new_value));
	new_value.len     = data_length;
	new_value.offset  = 0;
	new_value.p_value = i_data;

    err_code = sd_ble_gatts_value_set(BLE_CONN_HANDLE_INVALID, _char_handle.value_handle, &new_value);
    APP_ERROR_CHECK(err_code);


}
