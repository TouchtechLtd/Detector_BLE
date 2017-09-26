/*
 * timer_driver.h
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#ifndef TIMER_INTERFACE_H_
#define TIMER_INTERFACE_H_

#include "app_timer.h"


class Timer {
	private:
		static uint8_t _timerCount;
		static bool _timerPeripheralInitialised;

		static void lfclk_config(void);

		app_timer_id_t m_timer_id;

	public:
		static void initialisePeripheral();
		static uint32_t getTicks();
		static uint32_t getDiff(uint32_t   ticks_to,
		                        uint32_t   ticks_from);
    static uint32_t getFrequency();

    Timer();
		void startTimer(uint32_t ms,
								app_timer_timeout_handler_t handler);
		void startCountdown(uint32_t ms,
								app_timer_timeout_handler_t handler);
    void stopTimer();

	}; // End TrapEvent




#endif /* TIMER_INTERFACE_H_ */
