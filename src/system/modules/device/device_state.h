
/*
 * device_state.h
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#ifndef _MODULE_DEVICE_STATE_H__
#define _MODULE_DEVICE_STATE_H__

#include <stdint.h>
#include "gn_device.h"


namespace DEVICE
{

namespace STATE
{

typedef enum {
  IDLE_STATE,
  ACTIVE_STATE
} device_state_e;


uint8_t getCurrentState();
void start();
void stop();


}
}

#endif  /* _MODULE_DEVICE_STATE_H__ */
