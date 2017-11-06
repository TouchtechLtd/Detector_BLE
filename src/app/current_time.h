/*
 * current_time.h
 *
 *  Created on: 25/09/2017
 *      Author: michaelmcadam
 */

#ifndef SRC_APP_CURRENT_TIME_H_
#define SRC_APP_CURRENT_TIME_H_


#include "peripheral/timer_interface.h"

namespace CurrentTime
{


uint32_t getCurrentTime();
void startClock();



}

#endif /* SRC_APP_CURRENT_TIME_H_ */
