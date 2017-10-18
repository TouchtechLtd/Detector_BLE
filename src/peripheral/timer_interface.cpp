
/*
 * timer_interface.cpp
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#include <stdbool.h>
#include "boards.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"
#include "app_timer.h"
#include "nrf_drv_clock.h"

#include "debug/DEBUG.h"
#include "peripheral/timer_interface.h"

// General application timer settings.
#define APP_TIMER_PRESCALER             15    // Value of the RTC1 PRESCALER register.
#define APP_TIMER_OP_QUEUE_SIZE         3     // Size of timer operation queues.

#define MAX_NUMBER_TIMERS               10

bool Timer::_timerPeripheralInitialised = false;
uint8_t Timer::_timerCount = 0;

static app_timer_t timer_id_data[MAX_NUMBER_TIMERS] = { { 0 } } ;



// Function starting the internal LFCLK oscillator.
// This is needed by RTC1 which is used by the application timer
// (When SoftDevice is enabled the LFCLK is always running and this is not needed).

void Timer::lfclk_config(void)
{
    uint32_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);
}

void Timer::initialisePeripheral(void) {

  if (!_timerPeripheralInitialised) {
    lfclk_config();

    // Initialize the application timer module.
    app_timer_init();

    _timerPeripheralInitialised = true;
  }
}


uint32_t Timer::getTicks(void) {
  return app_timer_cnt_get();
}

uint32_t Timer::getDiff(uint32_t   ticks_to,
                        uint32_t   ticks_from) {

  return app_timer_cnt_diff_compute(ticks_to, ticks_from);
}


Timer::Timer()
{
  m_is_running = false;
  if (_timerCount >= MAX_NUMBER_TIMERS) {
    DEBUG("Timer limit reached!");
  }
  else {
    m_timer_id = &timer_id_data[_timerCount];
    _timerCount++;
  }
}


void Timer::startTimer(uint32_t ms,
                      app_timer_timeout_handler_t handler)

{
  uint32_t err_code;

  if (!_timerPeripheralInitialised) { initialisePeripheral(); }


  // Create timers
  err_code = app_timer_create(&m_timer_id,
                              APP_TIMER_MODE_REPEATED,
                              handler);
  ERROR_CHECK(err_code);

  app_timer_start(m_timer_id, APP_TIMER_TICKS(ms), NULL);
  m_is_running = true;
}


void Timer::stopTimer() {

    if (!app_timer_is_running(m_timer_id) || !m_is_running) {
      DEBUG("Timer not running");
      return;
    }

  app_timer_stop(m_timer_id);
  m_is_running = false;
}

void Timer::killTimer() {
  app_timer_stop(m_timer_id);
}


void Timer::startCountdown(uint32_t ms,
            app_timer_timeout_handler_t handler)
{
    uint32_t err_code;

    if (!_timerPeripheralInitialised) { initialisePeripheral(); }


    // Create timers
    err_code = app_timer_create(&m_timer_id,
                  APP_TIMER_MODE_SINGLE_SHOT,
                                handler);
    ERROR_CHECK(err_code);


    app_timer_start(m_timer_id, APP_TIMER_TICKS(ms), NULL);

    m_is_running = true;

}


uint32_t Timer::getFrequency() {
  return APP_TIMER_CLOCK_FREQ;
}

