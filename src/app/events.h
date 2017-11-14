
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
#define EVENT_PROCESSOR_MAX_LISTENERS  20

namespace EVENTS

{

typedef struct
{
  uint16_t len;
  void* p_data;
} event_data_t;


typedef void (*event_callback_t)(event_data_t eventData);



typedef struct
{
  uint16_t eventID;
  event_data_t eventData;
} event_signal_t;

typedef struct
{
  uint16_t eventID;
  event_callback_t callback;
} event_listener_t;



void eventPut(uint16_t eventID, const void* eventData = NULL, uint16_t dataLen = 0);
void registerEventHandler(uint16_t eventID, event_callback_t handler);

void processEvents();


} // EVENTS

#endif //_GOODNATURE_APP_STATE_MACHINE_H
