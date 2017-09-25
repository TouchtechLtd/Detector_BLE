/*
 * current_time.cpp
 *
 *  Created on: 25/09/2017
 *      Author: michaelmcadam
 */


#include <stdint.h>
#include "app/current_time.h"



uint32_t CurrentTime::timeInMinutes = 0;
Timer CurrentTime::minuteTimer;

void CurrentTime::minuteHandler(void*) {
  timeInMinutes += 1;
}


void CurrentTime::startClock() {
  minuteTimer.startTimer(60000, minuteHandler);
}

uint32_t CurrentTime::getCurrentTime() {
  return timeInMinutes;
}
