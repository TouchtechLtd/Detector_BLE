
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


bool _timerInitialised = false;
uint8_t _timerCount = 0;


// Function starting the internal LFCLK oscillator.
// This is needed by RTC1 which is used by the application timer
// (When SoftDevice is enabled the LFCLK is always running and this is not needed).
void Timer::lfclk_config(void)
{
    uint32_t err_code = nrf_drv_clock_init();
    ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);
}



// Create timers
void Timer::startTimer(app_timer_t* const timer_id,
						uint32_t ms,
						app_timer_timeout_handler_t handler)
{
    if (!_timerInitialised) { init(); }

    if (app_timer_is_running(timer_id)) {
    	DEBUG("Timer already running");
    	return;
    }
    // Create timers
    uint32_t err_code;
    err_code = app_timer_create(&timer_id,
                                APP_TIMER_MODE_REPEATED,
                                handler);
    ERROR_CHECK(err_code);

    err_code = app_timer_start(timer_id, APP_TIMER_TICKS(ms), NULL);
    ERROR_CHECK(err_code);
}

void Timer::stopTimer(app_timer_t* const timer_id) {

    if (!app_timer_is_running(timer_id)) {
    	DEBUG("Timer not running");
    	return;
    }

	uint32_t err_code = app_timer_stop(timer_id);
	ERROR_CHECK(err_code);
}


void Timer::startCountdown(app_timer_t* const timer_id,
						               uint32_t ms,
						               app_timer_timeout_handler_t handler)
{
    if (!_timerInitialised) { init(); }

    if (app_timer_is_running(timer_id)) {
    	DEBUG("Timer already running");
    	return;
    }

    // Create timers
    uint32_t err_code;
    err_code = app_timer_create(&timer_id,
    							APP_TIMER_MODE_SINGLE_SHOT,
                                handler);
    ERROR_CHECK(err_code);

    err_code = app_timer_start(timer_id, APP_TIMER_TICKS(ms), NULL);
    ERROR_CHECK(err_code);

}


void Timer::init(void) {
  if (!_timerInitialised)
  {
    lfclk_config();
    app_timer_init();
    _timerInitialised = true;
  }
}

