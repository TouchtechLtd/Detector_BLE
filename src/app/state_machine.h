
/*
 * state_machine.h
 *
 *  Created on: 21/09/2017
 *      Author: michaelmcadam
 */

#ifndef _GOODNATURE_APP_STATE_MACHINE_H
#define _GOODNATURE_APP_STATE_MACHINE_H

#include <stdint.h>
#include "app/events.h"




typedef void (*state_event_handler_t)(void);


typedef uint8_t state_t ;
typedef uint8_t event_e ;

#define MAX_NUMBER_MACHINES 10
#define STATE_MACHINE_MAX_STATES 20
#define STATE_MACHINE_MAX_EVENTS 20

typedef enum {
  IGNORED = 0xFE,
  ERROR
} default_states_e;

typedef struct
{
  uint16_t eventID;
  state_t  destinationState;
  state_event_handler_t transitionCallback;
} state_event_lookup_t;

typedef struct
{
  state_event_lookup_t eventLookup[STATE_MACHINE_MAX_EVENTS];
  uint8_t              numberOfEvents;
} my_state_type_t;


class StateMachine
{
public:
  StateMachine();
  //StateMachine(state_t i_initState, uint8_t max_states, uint8_t max_events);
  void init();
  void transition(EVENTS::event_signal_t eventSignal);
	void registerTransition(state_t startState,
							state_t endState,
							uint16_t event,
							state_event_handler_t event_handler);

  state_t getCurrentState();

  void start(state_t state);
  void stop();
  bool isRunning();

private:
    unsigned char _currentState;
    bool m_running;

    my_state_type_t state_table[STATE_MACHINE_MAX_STATES];

    state_event_handler_t event_table[STATE_MACHINE_MAX_STATES][STATE_MACHINE_MAX_EVENTS];
    state_t transition_table[STATE_MACHINE_MAX_STATES][STATE_MACHINE_MAX_EVENTS];


    static void error_handler(void);
    static void addMachine(StateMachine* machine);
    static void dispatch(EVENTS::event_signal_t eventSignal);

    static bool isInitialised;
    static StateMachine* allMachines[MAX_NUMBER_MACHINES];
    static int machineCount;

};

#endif //_GOODNATURE_APP_STATE_MACHINE_H
