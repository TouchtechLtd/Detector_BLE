
/*
 * ble_service.cpp
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */


#include <stdint.h>
#include <string.h>
#include "nrf_gpio.h"
#include "ble_srv_common.h"
#include "app_error.h"

#include "libraries/ble/ble_service.h"
#include "libraries/ble/ble_characteristic.h"

#include "libraries/debug/DEBUG.h"

namespace BLE_SERVER {


uint16_t Service::serviceCount = 0;

Service::Service(uint16_t custom_uuid, ble_uuid128_t base_uuid)
{
	_init();
	createCustom(custom_uuid, base_uuid);
}

Service::Service(uint16_t sig_uuid)
{
	_init();
	createSIG(sig_uuid);
}

Service::Service()
{
	_init();
}

void Service::_init()
{
  memset(&_service, 0, sizeof(_service));
  memset(&_base_uuid, 0, sizeof(_base_uuid));

  _charCount = 0;
  _id = serviceCount++;
  m_isUUIDSet = 0;
  m_isRunning = false;
}


void Service::eventHandler(ble_evt_t const * p_ble_evt)
{
  //DEBUG("Service receiving event: %d", _service.uuid);
	switch (p_ble_evt->header.evt_id)
	{
	    case BLE_GAP_EVT_CONNECTED:
	        _service.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
	        break;
	    case BLE_GAP_EVT_DISCONNECTED:
	        _service.conn_handle = BLE_CONN_HANDLE_INVALID;
	        break;
	    default:
	        // No implementation needed.
	        break;
	}

  for (int i = 0; i< MAX_NUMBER_CHAR; i++)
  {
    if (_charList[i].isRunning()) { _charList[i].eventHandler(p_ble_evt); }
  }

}


void Service::attachService()
{
  if (m_isUUIDSet == true) {
    uint32_t err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &_service.uuid,
                                        &_service.service_handle);

    ERROR_CHECK(err_code);
    for (int i = 0; i < MAX_NUMBER_CHAR; i++)
    {
      if (_charList[i].isInit()) { _charList[i].attachToService(_service.service_handle); }
    }

    m_isRunning = true;
  }
}



/**@brief Function for initiating our new service.
 *
 * @param[in]   p_our_service        Our Service structure.
 *
 */
void Service::createCustom(uint16_t uuid, ble_uuid128_t base_uuid)
{
	_base_uuid = base_uuid;

	// Declare 16-bit service and 128-bit base UUIDs and add them to the BLE stack
  uint32_t   err_code;
  err_code = sd_ble_uuid_vs_add(&_base_uuid, &_service.uuid.type);
  ERROR_CHECK(err_code);

  _service.uuid.uuid		= uuid;
  _service.conn_handle = BLE_CONN_HANDLE_INVALID;

  m_isUUIDSet = true;
}




/**@brief Function for initiating our new service.
 *
 * @param[in]   p_our_service        Our Service structure.
 *
 */
void Service::createSIG(uint16_t uuid)
{
    // Declare 16-bit service and 128-bit base UUIDs and add them to the BLE stack
    _service.uuid.uuid = uuid;
    _service.uuid.type = BLE_UUID_TYPE_BLE;

    // Set our service connection handle to default value. I.e. an invalid handle since we are not yet in a connection.
    _service.conn_handle = BLE_CONN_HANDLE_INVALID;

    m_isUUIDSet = true;
}



void Service::addCharacteristic(Characteristic* p_char, uint8_t charID) {

	p_char->setUUIDType(_service.uuid.type);
	//p_char->setEventModifier(charID);
	_charList[charID] = *p_char;
}

Characteristic* Service::getCharacteristic(uint8_t charID) {
  return &_charList[charID];
}

} //BLE_SERVER
