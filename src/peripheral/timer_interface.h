/*
 * timer_interface.h
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#ifndef TIMER_INTERFACE_H_
#define TIMER_INTERFACE_H_

#include "app_timer.h"

APP_TIMER_DEF(app_timer_id_0);
APP_TIMER_DEF(app_timer_id_1);
APP_TIMER_DEF(app_timer_id_2);
APP_TIMER_DEF(app_timer_id_3);
APP_TIMER_DEF(app_timer_id_4);
APP_TIMER_DEF(app_timer_id_5);
APP_TIMER_DEF(app_timer_id_6);

#define TIMER_0 app_timer_id_0
#define TIMER_1 app_timer_id_1
#define TIMER_2 app_timer_id_2
#define TIMER_3 app_timer_id_3
#define TIMER_4 app_timer_id_4
#define TIMER_5 app_timer_id_5


class Timer {
	private:
		static uint8_t _timerCount;
		static bool _timerInitiliased;

		static void lfclk_config(void);
		//static void timer_a_handler(void * p_context);
		app_timer_id_t _id;

	public:
		static void init();
		static void startTimer(app_timer_t* const timer_id,
								uint32_t ms,
								app_timer_timeout_handler_t handler);

		static void stopTimer(app_timer_t* const timer_id);

		static void startCountdown(app_timer_t* const timer_id,
								uint32_t ms,
								app_timer_timeout_handler_t handler);

	}; // End TrapEvent




#endif /* TIMER_INTERFACE_H_ */
