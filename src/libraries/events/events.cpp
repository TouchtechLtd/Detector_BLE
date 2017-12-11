/*
 * state_machine.cpp
 *
 *  Created on: 13/09/2017
 *      Author: michaelmcadam
 */

#include <stdlib.h>
#include "app/events.h"
#include "debug/DEBUG.h"


#define NRF_LOG_MODULE_NAME EVENTS
NRF_LOG_MODULE_REGISTER();

namespace EVENTS

{

static event_signal_t m_queue_event_signals[EVENT_PROCESSOR_MAX_SIGNALS] = {{0}};
static event_listener_t m_queue_event_listeners[EVENT_PROCESSOR_MAX_LISTENERS] = {{0}};
static event_repeater_t m_queue_event_repeaters[EVENT_PROCESSOR_MAX_REPEATERS] = {0};

static volatile uint8_t m_queue_start_index = 0;    /**< Index of queue entry at the start of the queue. */
static volatile uint8_t m_queue_end_index = 0;      /**< Index of queue entry at the end of the queue. */

static uint8_t m_handlerCount = 0;
static uint8_t m_repeaterCount = 0;

static __INLINE uint8_t next_index(uint8_t index)
{
    return ((index + 1) < EVENT_PROCESSOR_MAX_SIGNALS) ? (index + 1) : 0;
}


static __INLINE uint8_t queue_full()
{
  uint8_t tmp = m_queue_start_index;
  return next_index(m_queue_end_index) == tmp;
}

static __INLINE uint8_t queue_empty()
{
  uint8_t tmp = m_queue_start_index;
  return m_queue_end_index == tmp;
}


void eventPut(uint16_t eventID, const void* eventData, uint16_t dataLen)
{
  uint16_t event_index = 0xFFFF;

  CRITICAL_REGION_ENTER();

  if (!queue_full())
  {
      event_index                 = m_queue_end_index;
      m_queue_end_index           = next_index(m_queue_end_index);
  }

  CRITICAL_REGION_EXIT();

  if (0xFFFF == event_index)
  {
    INFO("ERROR: Event queue full!");
    return;
  }

  m_queue_event_signals[event_index].eventID                = eventID;

  if (eventData != NULL)
  {
    m_queue_event_signals[event_index].eventData.len          = dataLen;
    m_queue_event_signals[event_index].eventData.p_data       =  (void*) malloc(dataLen);

    if (m_queue_event_signals[event_index].eventData.p_data == NULL)
    {
      INFO("Event data malloc failed for event ID: 0x%04x", eventID);
      return;
    }
    memcpy(m_queue_event_signals[event_index].eventData.p_data, eventData, dataLen);
  }

  INFO("Event: 0x%04x received - Index: %d", eventID, event_index);

}
void registerEventHandler(uint16_t eventID, event_signal_callback_t callback)
{
  INFO("ATTACHING - event handler to eventID: 0x%04x", eventID);
  m_queue_event_listeners[m_handlerCount].eventID = eventID;
  m_queue_event_listeners[m_handlerCount].signal_callback = callback;
  m_queue_event_listeners[m_handlerCount].isSignal = true;
  m_handlerCount++;
}

void registerEventHandler(uint16_t eventID, event_message_callback_t callback)
{
  INFO("ATTACHING - event handler to eventID: 0x%04x", eventID);
  m_queue_event_listeners[m_handlerCount].eventID = eventID;
  m_queue_event_listeners[m_handlerCount].message_callback = callback;
  m_queue_event_listeners[m_handlerCount].isSignal = false;
  m_handlerCount++;
}


void registerEventRepeater(event_repeater_t repeater)
{
  m_queue_event_repeaters[m_repeaterCount] = repeater;
  m_repeaterCount++;
}


void processEvents()
{
  while(!queue_empty())
  {
    uint16_t event_index = m_queue_start_index;

    event_signal_t* p_event_signal = &m_queue_event_signals[event_index];

    INFO("Event: 0x%04x dispatching", p_event_signal->eventID);

    for (int i = 0; i < m_handlerCount; i++)
    {

      if (p_event_signal->eventID == m_queue_event_listeners[i].eventID)
      {
        if (m_queue_event_listeners[i].isSignal == true)
        {
          m_queue_event_listeners[i].signal_callback();
        }
        else
        {
          m_queue_event_listeners[i].message_callback(p_event_signal->eventData);
        }

      }
    }

    for (int j = 0; j < m_repeaterCount; j++)
    {
      m_queue_event_repeaters[j](*p_event_signal);
    }


    free(p_event_signal->eventData.p_data);

    // Event processed, now it is safe to move the queue start index,
    // so the queue entry occupied by this event can be used to store
    // a next one.
    m_queue_start_index = next_index(m_queue_start_index);
  }
}






} //EVENTS
