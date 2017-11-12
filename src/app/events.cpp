/*
 * state_machine.cpp
 *
 *  Created on: 13/09/2017
 *      Author: michaelmcadam
 */

#include <stdlib.h>
#include "app/events.h"
#include "debug/DEBUG.h"


namespace EVENTS

{

static event_signal_t m_queue_event_signals[EVENT_PROCESSOR_MAX_SIGNALS] = {{0}};
static event_listener_t m_queue_event_listeners[EVENT_PROCESSOR_MAX_LISTENERS] = {{0}};

static volatile uint8_t m_queue_start_index;    /**< Index of queue entry at the start of the queue. */
static volatile uint8_t m_queue_end_index;      /**< Index of queue entry at the end of the queue. */

static uint8_t m_handlerCount = 0;

static __INLINE uint8_t next_index(uint8_t index)
{
    return (index < EVENT_PROCESSOR_MAX_SIGNALS) ? (index + 1) : 0;
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


void eventPut(uint16_t eventID)
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
    DEBUG("Queue full!");
    return;
  }

  event_signal_t* p_event = &m_queue_event_signals[event_index];
  p_event->eventID    = eventID;

}
void registerEventHandler(uint16_t eventID, event_callback_t callback)
{
  event_listener_t* p_eventListener = &m_queue_event_listeners[m_handlerCount++];
  p_eventListener->eventID = eventID;
  p_eventListener->callback = callback;
}

void processEvents()
{
  while(!queue_empty())
  {
    uint16_t event_index = m_queue_start_index;

    event_signal_t* p_event_signal = &m_queue_event_signals[event_index];

    for(int i = 0; i < m_handlerCount; i++)
    {

      if (p_event_signal->eventID == m_queue_event_listeners[i].eventID)
      {
        m_queue_event_listeners[i].callback();
      }
    }

    // Event processed, now it is safe to move the queue start index,
    // so the queue entry occupied by this event can be used to store
    // a next one.
    m_queue_start_index = next_index(m_queue_start_index);
  }
}






} //EVENTS
