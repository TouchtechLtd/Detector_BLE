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

static current_time_t m_currentTime = { 0, 0 };
static Timer          m_minuteTimer;

void minuteHandler(void*) {
  m_currentTime.time += 1;
}


void startClock() {
  m_minuteTimer.startTimer(60000, minuteHandler);
}

current_time_t* getCurrentTime() {
  return &m_currentTime;
}

void setCurrentTime(current_time_t currentTime)
{
  m_currentTime.time = currentTime.time;
  m_currentTime.absSet = true;
}

}
