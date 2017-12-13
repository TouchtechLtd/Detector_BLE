

/*
 *  gn_time.cpp
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#include <stdint.h>
#include <string.h>

#include "drivers/timer/timer_interface.h"
#include "libraries/events/events.h"
#include "libraries/debug/DEBUG.h"

#include "./gn_time.h"
#include "./time_state.h"
#include "./time_service.h"
#include "./time_storage.h"


#define NRF_LOG_MODULE_NAME TIME
NRF_LOG_MODULE_REGISTER();


namespace TIME
{

static current_time_t m_currentTime = { 0, 0 };
static Timer          m_minuteTimer;



void minuteHandler(void*) {
  m_currentTime.time += 1;
}


void startClock() {
  INFO("STARTING - Clock Peripheral");
  m_minuteTimer.startTimer(60000, minuteHandler);
}

const current_time_t* getCurrentTime() {
  return &m_currentTime;
}

void setCurrentTime(current_time_t currentTime)
{
  m_currentTime.time = currentTime.time;
  m_currentTime.absSet = true;
}



void registerEvents()
{
  EVENTS::registerEventHandler(TIMESET_EVENT, startClock);
  EVENTS::registerEventHandler(TIMESET_EVENT, SERVICE::update);
}


void init()
{

  //STATE::start();
  SERVICE::start();
  //STORAGE::start();

  registerEvents();
}


}
