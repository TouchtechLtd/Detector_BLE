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


// ALREADY_DONE_FOR_YOU: Declaration of a function that will take care of some housekeeping of ble connections related to our service and characteristic
void Service::on_ble_evt(ble_evt_t * p_ble_evt)
{
    // OUR_JOB: Step 3.D Implement switch case handling BLE events related to our service.
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



Service::Service(uint16_t custom_uuid, ble_uuid128_t base_uuid)
{
	Service();
	createCustomService(custom_uuid, base_uuid);
}

Service::Service(uint16_t sig_uuid)
{
	Service();
	createSIGService(sig_uuid);
}

Service::Service()
{
    memset(&_service, 0, sizeof(_service));
    // Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
    memset(&service_uuid, 0, sizeof(service_uuid));
    // Configure the attribute metadata
    memset(&_base_uuid, 0, sizeof(_base_uuid));
    _charCount = 0;
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

    err_code = sd_ble_uuid_vs_add(&_base_uuid, &service_uuid.type);
    APP_ERROR_CHECK(err_code);

    service_uuid.uuid = uuid;

    _service.uuid_type = service_uuid.type;
    _service.conn_handle = BLE_CONN_HANDLE_INVALID;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &service_uuid,
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
    uint32_t   err_code; // Variable to hold return codes from library and softdevice functions

    // FROM_SERVICE_TUTORIAL: Declare 16-bit service and 128-bit base UUIDs and add them to the BLE stack
    service_uuid.uuid = uuid;

    service_uuid.type = BLE_UUID_TYPE_BLE;

    // OUR_JOB: Step 3.B, Set our service connection handle to default value. I.e. an invalid handle since we are not yet in a connection.
    _service.conn_handle = BLE_CONN_HANDLE_INVALID;

    // FROM_SERVICE_TUTORIAL: Add our service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &service_uuid,
                                        &_service.service_handle);

    APP_ERROR_CHECK(err_code);

    // OUR_JOB: Call the function our_char_add() to add our new characteristic to the service.
    //our_char_add(p_our_service);
}


ble_char_id_t Service::addCharacteristic(uint16_t i_uuid) {
	_charList[_charCount].add(i_uuid, _base_uuid, _service.service_handle);
	ble_char_id_t charId;
	charId.id =_charCount++;
	return charId;
}

