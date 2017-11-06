
/*
 * ble_characteristic.cpp
 *
 *  Created on: September 5, 2017
 *      Author: Michael McAdam
 */

#include <stdint.h>
#include <string.h>
#include "nrf_gpio.h"
#include "ble/ble_characteristic.h"
#include "ble_srv_common.h"
#include "app_error.h"

#include "debug/DEBUG.h"

Characteristic::Characteristic() {
    // Add read/write properties to our characteristic
    _init();
}


Characteristic::Characteristic(uint16_t i_uuid) {
    // Add read/write properties to our characteristic

    _init();
    setUUID(i_uuid);
}

void Characteristic::_init()
{
	// Add read/write properties to our characteristic
  memset(&_char_handle, 0, sizeof(_char_handle));
  memset(&_char_uuid, 0, sizeof(_char_uuid));
  memset(&_cccd_md, 0, sizeof(_cccd_md));
  memset(&_char_md, 0, sizeof(_char_md));
  memset(&_attr_md, 0, sizeof(_attr_md));
  memset(&_attr_char_value, 0, sizeof(_attr_char_value));

  m_isUUIDSet = false;
  m_isRunning = false;
  _charAdded = false;
  _notificationEnabled = false;
  _readEnabled = false;

  m_writeHandler = NULL;
  _conn_handle = BLE_CONN_HANDLE_INVALID;
}


void Characteristic::setUUID(uint16_t i_uuid)
{
	_char_uuid.uuid = i_uuid;
	if (_char_uuid.type != 0) { m_isUUIDSet = true; }
}

void Characteristic::setUUIDType(uint8_t i_type)
{
	_char_uuid.type = i_type;
	if (_char_uuid.uuid != 0) { m_isUUIDSet = true; }
}

void Characteristic::configureUUID (uint16_t i_uuid, uint8_t i_type)
{
	_char_uuid.uuid = i_uuid;
	_char_uuid.type = i_type;
	m_isUUIDSet = true;
}

void Characteristic::attachToService(uint16_t i_serviceHandle)
{

	if (m_isUUIDSet && !m_isRunning)
	{
		// Configure the characteristic value attribute
		_attr_md.vloc        = BLE_GATTS_VLOC_STACK;
		_attr_char_value.p_uuid      = &_char_uuid;
		_attr_char_value.p_attr_md   = &_attr_md;

		//  Add our new characteristic to the service
    uint32_t err_code = sd_ble_gatts_characteristic_add(i_serviceHandle,
                     &_char_md,
                     &_attr_char_value,
                     &_char_handle);

		ERROR_CHECK(err_code);

		m_isRunning = true;

	} else {
	  DEBUG("Please set UUID before adding service");
	}
}



/**@brief Function for adding our new characterstic to "Our service" that we initiated in the previous tutorial.
 *
 * @param[in]   p_our_service        Our Service structure.
 *
 */

void Characteristic::add(uint16_t i_serviceHandle, uint16_t i_uuid, uint8_t i_uuidType)
{
	configureUUID(i_uuid, i_uuidType);
	attachToService(i_serviceHandle);
}


void Characteristic::enableRead() {
    _char_md.char_props.read = 1;
    _attr_md.rd_auth    = 0;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&_attr_md.read_perm);
    _readEnabled = true;
}

void Characteristic::disableRead() {
    _char_md.char_props.read = 0;
    _attr_md.rd_auth    = 0;
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&_attr_md.read_perm);
}

void Characteristic::enableWrite() {
    _char_md.char_props.write = 1;
    _attr_md.wr_auth    = 0;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&_attr_md.write_perm);
}

void Characteristic::disableWrite() {
    _char_md.char_props.write = 0;
    _attr_md.wr_auth    = 0;
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&_attr_md.write_perm);
}


void Characteristic::enableNotification() {
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&_cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&_cccd_md.write_perm);
    _cccd_md.vloc                = BLE_GATTS_VLOC_STACK;
    _char_md.p_cccd_md           = &_cccd_md;
    _char_md.char_props.notify   = 1;
}


void Characteristic::initValue(void* p_value, uint16_t len)
{
    _attr_char_value.init_len    = len;
    _attr_char_value.p_value     = static_cast<uint8_t*>(p_value);
    setMaxLength(len);
}

void Characteristic::setMaxLength(uint16_t i_maxLen)
{
	_attr_char_value.max_len = i_maxLen;
	if (_attr_char_value.max_len == _attr_char_value.init_len) {
		_attr_md.vlen 		= 0;
	}
	else { _attr_md.vlen 		= 1; }
}

// Function to be called when updating characteristic value

void Characteristic::set(void* i_data, uint16_t dataLength)
{
  if (!m_isRunning)
  {
    DEBUG("Not running!");
    return;
  }

  if (_notificationEnabled) { notify(i_data, dataLength); }
  else if (_readEnabled)    { update(i_data, dataLength); }
  else                      { DEBUG("Not enabled"); }
}


void Characteristic::notify(void* i_data, uint16_t  data_length)
{
	if (_notificationEnabled && m_isRunning) {
		// Update characteristic value
		if ( _conn_handle != BLE_CONN_HANDLE_INVALID)
		{
			memset(&_hvx_params, 0, sizeof(_hvx_params));
			_hvx_params.handle = _char_handle.value_handle;
			_hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
			_hvx_params.offset = 0;
			_hvx_params.p_len  = &data_length;
			_hvx_params.p_data = static_cast<uint8_t*>(i_data);

			uint32_t err_code;
			err_code = sd_ble_gatts_hvx(_conn_handle, &_hvx_params);
			ERROR_CHECK(err_code);
		}
		else { DEBUG("Connection handle = invalid"); }
	}
	else { DEBUG("Notify not enabled"); }

}


void Characteristic::update(void* i_data, uint16_t data_length)
{
	if (_readEnabled && m_isRunning) {
		if (data_length <= _attr_char_value.max_len) {

		  ble_gatts_value_t new_value;
			memset(&new_value, 0, sizeof(new_value));
			new_value.len     = data_length;
			new_value.offset  = 0;
			new_value.p_value = static_cast<uint8_t*>(i_data);

			uint32_t err_code = sd_ble_gatts_value_set(BLE_CONN_HANDLE_INVALID, _char_handle.value_handle, &new_value);
			ERROR_CHECK(err_code);
		}
		else { DEBUG("Value too long"); }
	}
	else { DEBUG("Value not updated"); }
}


uint8_t Characteristic::isInit() {
  return m_isUUIDSet;
}

void Characteristic::setWriteHandler(char_write_handler_t writeHandler)
{
  m_writeHandler = writeHandler;
}


void Characteristic::eventHandler(ble_evt_t const * p_ble_evt)
{
  switch (p_ble_evt->header.evt_id)
  {
      case BLE_GAP_EVT_CONNECTED:
          _conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
          break;
      case BLE_GAP_EVT_DISCONNECTED:
          _conn_handle = BLE_CONN_HANDLE_INVALID;
          break;
      case BLE_GATTS_EVT_WRITE:

        if (p_ble_evt->evt.gatts_evt.params.write.handle == _char_handle.cccd_handle)
        {
          _notificationEnabled = p_ble_evt->evt.gatts_evt.params.write.data[0];
        }
        if (p_ble_evt->evt.gatts_evt.params.write.handle == _char_handle.value_handle)
        {
          if (m_writeHandler != NULL)
          {
           m_writeHandler(p_ble_evt->evt.gatts_evt.params.write.data, p_ble_evt->evt.gatts_evt.params.write.len);
          }

        }

        break;

      default:
          // No implementation needed.
          break;
  }
}

void Characteristic::setConnHandle(uint16_t i_connHandle)
{
	_conn_handle = i_connHandle;
}
