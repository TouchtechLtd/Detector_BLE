
/*
 * detector_state.h
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#ifndef _MODULE_DETECTOR_STATE_H__
#define _MODULE_DETECTOR_STATE_H__

#include <stdint.h>
#include "gn_detector.h"


namespace DETECTOR
{

namespace STATE
{

typedef enum {
  WAIT_STATE,
  EVENT_BUFFER_STATE,
  DETECT_MOVE_STATE,
  MOVING_STATE,
  MAX_STATES,
} detector_state_e;


uint8_t getCurrentState();
void start();
void stop();


}
}

#endif  /* _MODULE_DETECTOR_STATE_H__ */
