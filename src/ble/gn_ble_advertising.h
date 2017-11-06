/*
 * ble_advertising.h
 *
 *  Created on: September 5, 2017
 *      Author: Michael McAdam
 */

#ifndef _GOODNATURE_BLE_BLE_ADVERTISING_H__
#define _GOODNATURE_BLE_BLE_ADVERTISING_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "ble.h"
#include "ble_advdata.h"
#include "ble_srv_common.h"


#define APP_ADV_DEFAULT_INTERVAL                BLE_GAP_SCAN_INTERVAL_MAX                                      /**< The advertising interval (in units of 0.625 ms; this value corresponds to 40 ms). */


namespace BLE_ADVERTISING
{

void params_init(void);
void start(uint16_t interval);
void stop();
void updateInterval(uint16_t interval);

void setName(const char* deviceName);
void updateName(const char * deviceName);
void advertiseName();
void advertiseUUID(ble_uuid_t uuid);
void advertiseData(uint8_t * p_data, uint8_t i_len);


}

#ifdef __cplusplus
}
#endif

#endif  /* _GOODNATURE_BLE_BLE_ADVERTISING_H__ */
