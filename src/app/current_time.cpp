/*
 * current_time.cpp
 *
 *  Created on: 25/09/2017
 *      Author: michaelmcadam
 */


#include <stdint.h>
#include "app/current_time.h"


namespace CurrentTime
{

static uint32_t timeInMinutes = 0;
static Timer   minuteTimer;

void minuteHandler(void*) {
  timeInMinutes += 1;
}


void startClock() {
  minuteTimer.startTimer(60000, minuteHandler);
}

uint32_t getCurrentTime() {
  return timeInMinutes;
}

void setAbsTime(uint32_t currentTime)
{
  timeInMinutes = currentTime;
  //currentTimeSet = true;
}

}
