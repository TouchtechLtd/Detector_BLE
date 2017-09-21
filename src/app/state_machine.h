
/*
 * state_machine.h
 *
 *  Created on: 21/09/2017
 *      Author: michaelmcadam
 */

#ifndef _GOODNATURE_APP_STATE_MACHINE_H
#define _GOODNATURE_APP_STATE_MACHINE_H



typedef enum {
	DETECT_STATE,
	READ_STATE,
	PROCESS_STATE,
	MAX_STATES,
	IGNORED
} state_e;


typedef enum {
	TRIGGERED_EVENT,
	READ_FINISHED_EVENT,
	PROCESSING_FINISHED_EVENT,
	MAX_EVENTS
} event_e;


typedef void (*state_event_handler_t)(void);



class StateMachine
{
public:
    StateMachine(state_e i_initState);
	void transition(event_e event);
	void registerTransition(state_e startState,
							state_e endState,
							event_e event,
							state_event_handler_t event_handler);

private:
    unsigned char _currentState;

    state_event_handler_t event_table[MAX_STATES][MAX_EVENTS];
    state_e transition_table[MAX_STATES][MAX_EVENTS];

    static void error_handler(void);

};

#endif //_GOODNATURE_APP_STATE_MACHINE_H
