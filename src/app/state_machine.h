
/*
 * state_machine.h
 *
 *  Created on: 21/09/2017
 *      Author: michaelmcadam
 */

#ifndef _GOODNATURE_APP_STATE_MACHINE_H
#define _GOODNATURE_APP_STATE_MACHINE_H

#include <stdint.h>




typedef void (*state_event_handler_t)(void);


typedef uint8_t state_t ;
typedef uint8_t event_e ;

#define STATE_MACHINE_MAX_STATES 20
#define STATE_MACHINE_MAX_EVENTS 20

typedef enum {
  IGNORED = 0xFE,
  ERROR
} default_states_e;



class StateMachine
{
public:
  StateMachine(state_t i_initState, uint8_t max_states, uint8_t max_events);
  void init();
	void transition(event_e event);
	void registerTransition(state_t startState,
							state_t endState,
							event_e event,
							state_event_handler_t event_handler);

  state_t getCurrentState();

  void start(state_t state);
  void stop();
  bool isRunning();

private:
    unsigned char _currentState;
    bool m_running;

    state_event_handler_t event_table[STATE_MACHINE_MAX_STATES][STATE_MACHINE_MAX_EVENTS];
    state_t transition_table[STATE_MACHINE_MAX_STATES][STATE_MACHINE_MAX_EVENTS];

    static void error_handler(void);

};

#endif //_GOODNATURE_APP_STATE_MACHINE_H
