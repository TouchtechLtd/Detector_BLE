
/*
 * ble_interface.h
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#ifndef _GOODNATURE_BLE_BLE_INTERFACE_H_
#define _GOODNATURE_BLE_BLE_INTERFACE_H_

#include "ble_conn_params.h"
#include "libraries/ble/ble_service.h"
#include "libraries/ble/gn_ble_advertising.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"

#define MAX_NUMBER_SERVICES 10
#define BLE_EVENT_OFFSET    0x2000

namespace BLE_SERVER
{

enum {
  BLE_CONNECTED_EVENT = BLE_EVENT_OFFSET,
  BLE_DISCONNECTED_EVENT,
  BLE_WRITE_EVENT,
  BLE_STATE_CHANGE_EVENT,
  BLE_EVENT_EVENT
};


/** Available Power Modes for the LIS2DH12 */
typedef enum{
  BLE_POWER_N_40_DB = -40,
  BLE_POWER_N_30_DB = -20,
  BLE_POWER_N_20_DB = -20,
  BLE_POWER_N_16_DB = -16,
  BLE_POWER_N_12_DB = -12,
  BLE_POWER_N_8_DB  = -8,
  BLE_POWER_N_4_DB  = -4,
  BLE_POWER_0_DB    = 0,
  BLE_POWER_3_DB    = 3,
  BLE_POWER_4_DB    = 4
} BLEPowerLevel;


void init(void);
bool isConnected();
void waitForTx();
void setPower(BLEPowerLevel powerLevel);

void addService(Service* service, uint8_t serviceID);
Service* getService(uint8_t serviceID);

void setWriteHandler(uint8_t serviceID, uint8_t charID, char_write_handler_t writeHandler);
gn_char_error_t setCharacteristic(uint8_t serviceID, uint8_t charID, void* p_data, uint16_t length);


}

#endif /* _GOODNATURE_BLE_BLE_INTERFACE_H_ */
