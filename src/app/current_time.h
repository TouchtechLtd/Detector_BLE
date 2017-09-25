/*
 * current_time.h
 *
 *  Created on: 25/09/2017
 *      Author: michaelmcadam
 */

#ifndef SRC_APP_CURRENT_TIME_H_
#define SRC_APP_CURRENT_TIME_H_


#include "peripheral/timer_interface.h"



class CurrentTime {

public:
  static uint32_t getCurrentTime();
  static void startClock();
private:
  static void minuteHandler(void*);
  static uint32_t timeInMinutes;
  static Timer minuteTimer;
};


#endif /* SRC_APP_CURRENT_TIME_H_ */
