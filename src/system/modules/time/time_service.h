
/*
 *  time_service.h
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#ifndef _MODULE_TIME_SERVICE_H__
#define _MODULE_TIME_SERVICE_H__

#include <stdint.h>
#include "gn_time.h"


namespace TIME
{

namespace SERVICE
{

#define BLE_UUID_SERVICE_TIME      0xF1AE
#define SERVICE_TRAP_TIME               3

enum TimeService_UUID
{
  BLE_UUID_CHAR_TRAP_TIME = 0xF1AF
};

enum TimeService_Characteristics
{
  CHAR_TRAP_TIME
};

void start();
void update();


}
}

#endif  /* _MODULE_TIME_SERVICE_H__ */
