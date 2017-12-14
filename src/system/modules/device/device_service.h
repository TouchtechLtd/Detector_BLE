
/*
 *  device_service.h
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#ifndef _MODULE_DEVICE_SERVICE_H__
#define _MODULE_DEVICE_SERVICE_H__

#include <stdint.h>
#include "gn_device.h"


namespace DEVICE
{

namespace SERVICE
{

#define BLE_UUID_SERVICE_DEVICE_INFO      0xDE11
#define SERVICE_DEVICE_INFO               5

enum DeviceService_UUID
{
  BLE_UUID_CHAR_DEVICE_ID       = 0xDE12,
  BLE_UUID_CHAR_DEVICE_CONTROL  = 0xDE13
};

enum DeviceService_Characteristics
{
  CHAR_DEVICE_ID,
  CHAR_DEVICE_CONTROL
};

void start();
void update();

}
}

#endif  /* _MODULE_DEVICE_SERVICE_H__ */
