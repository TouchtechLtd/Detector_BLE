
/*
 * state_machine.h
 *
 *  Created on: 21/09/2017
 *      Author: michaelmcadam
 */

#ifndef _GOODNATURE_APP_EVENTS_H
#define _GOODNATURE_APP_EVENTS_H

#include <stdint.h>


#define EVENT_PROCESSOR_MAX_SIGNALS    100
#define EVENT_PROCESSOR_MAX_LISTENERS  40
#define EVENT_PROCESSOR_MAX_REPEATERS  2

namespace EVENTS

{

typedef struct
{
  uint16_t len;
  void* p_data;
} event_data_t;


typedef void (*event_callback_t)(event_data_t eventData);

typedef void (*event_signal_callback_t)();
typedef void (*event_message_callback_t)(event_data_t eventData);


typedef struct
{
  uint16_t eventID;
  event_data_t eventData;
} event_signal_t;

typedef struct
{
  uint16_t eventID;
  event_signal_callback_t signal_callback;
  event_message_callback_t message_callback;
  bool isSignal;
} event_listener_t;

typedef void (*event_repeater_t)(event_signal_t eventSignal);


void eventPut(uint16_t eventID, const void* eventData = NULL, uint16_t dataLen = 0);
void registerEventHandler(uint16_t eventID, event_signal_callback_t handler);
void registerEventHandler(uint16_t eventID, event_message_callback_t handler);
void registerEventRepeater(event_repeater_t repeater);

void processEvents();


} // EVENTS

#endif //_GOODNATURE_APP_STATE_MACHINE_H
