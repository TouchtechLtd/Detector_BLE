#include <stdint.h>
#include <string.h>
#include "nrf_gpio.h"
#include "ble_service.h"
#include "ble_characteristic.h"
#include "ble_srv_common.h"
#include "app_error.h"

#define NRF_LOG_MODULE_NAME "SER"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"



Service::Service(uint16_t custom_uuid, ble_uuid128_t base_uuid)
{
	_init();
	createCustomService(custom_uuid, base_uuid);
}

Service::Service(uint16_t sig_uuid)
{
	_init();
	createSIGService(sig_uuid);
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
}


void Service::onEvent(ble_evt_t * p_ble_evt)
{
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
}


/**@brief Function for initiating our new service.
 *
 * @param[in]   p_our_service        Our Service structure.
 *
 */
void Service::createCustomService(uint16_t uuid, ble_uuid128_t base_uuid)
{
	_base_uuid = base_uuid;

	// Declare 16-bit service and 128-bit base UUIDs and add them to the BLE stack
    uint32_t   err_code;
    err_code = sd_ble_uuid_vs_add(&_base_uuid, &_service.uuid.type);
    APP_ERROR_CHECK(err_code);

    _service.uuid.uuid		= uuid;
    _service.conn_handle = BLE_CONN_HANDLE_INVALID;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &_service.uuid,
                                        &_service.service_handle);
    APP_ERROR_CHECK(err_code);
}




/**@brief Function for initiating our new service.
 *
 * @param[in]   p_our_service        Our Service structure.
 *
 */
void Service::createSIGService(uint16_t uuid)
{
    // Declare 16-bit service and 128-bit base UUIDs and add them to the BLE stack
    _service.uuid.uuid = uuid;
    _service.uuid.type = BLE_UUID_TYPE_BLE;

    // Set our service connection handle to default value. I.e. an invalid handle since we are not yet in a connection.
    _service.conn_handle = BLE_CONN_HANDLE_INVALID;

    // Add our service
    uint32_t   err_code;
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &_service.uuid,
                                        &_service.service_handle);
    APP_ERROR_CHECK(err_code);

}


ble_char_id_t Service::addCharacteristic(uint16_t i_uuid) {

	_charList[_charCount].add(&_service.service_handle, i_uuid, _service.uuid.type);

	ble_char_id_t charId;
	charId.id =_charCount++;
	return charId;
}


ble_char_id_t Service::addCharacteristic(Characteristic* p_char) {

	p_char->setUUIDType(_service.uuid.type);
	p_char->add(&_service.service_handle);
	_charList[_charCount] = *p_char;

	ble_char_id_t charId;
	charId.id =_charCount++;
	return charId;
}

