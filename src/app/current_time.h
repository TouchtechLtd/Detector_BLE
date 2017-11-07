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

#pragma pack(push, 1)
typedef struct
{
  uint32_t time;
  uint8_t   absSet;
} current_time_t;
#pragma pack(pop)

current_time_t* getCurrentTime();
void            setCurrentTime(current_time_t currentTime);
void            startClock();



}

#endif /* SRC_APP_CURRENT_TIME_H_ */
