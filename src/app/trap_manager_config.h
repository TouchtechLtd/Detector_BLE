
/*
 * TrapEvent.h
 *
 *  Created on: September 5, 2017
 *      Author: Michael McAdam
 */

#ifndef CPP_TRAP_EVENT_H
#define CPP_TRAP_EVENT_H
#include <stdint.h>



#define TRAP_EVENT_BUFFER_MS              2000
#define MOVE_BUFFER_MS                    5000
#define SET_BUFFER_MS                     10000

#define TRAP_TRIGGER_EVENT_THRESHOLD      500
#define TRAP_TRIGGER_MOVE_THRESHOLD       100

#define TRAP_TRIGGER_DURATION             40


#define KILL_DATA_FILE_ID       (0xDA7A)
#define KILL_RAW_DATA_FILE_ID   (0xDA8A)
#define KILL_NUMBER_FILE_ID     (0x5111)
#define KILL_NUMBER_KEY_ID      (0x1111)

#define RAW_DATA_CAPTURE_SIZE 250

namespace TrapState
{


  typedef enum {
    MOVE_TRIGGERED,
    ANIMAL_KILLED
  } trap_event_e;

  typedef enum {
    WAIT_STATE,
    EVENT_BUFFER_STATE,
    DETECT_MOVE_STATE,
    MOVING_STATE,
    MAX_STATES,
  } detector_state_e;


  typedef enum {
    TRIGGERED_EVENT,
    BUFFER_END_EVENT,
    ANIMAL_KILLED_EVENT,
    MOVEMENT_BUFFER_END_EVENT,
    SET_BUFFER_END_EVENT,
    MAX_EVENTS
  } detector_event_e;

}

#endif /* CPP_TRAP_EVENT_H */
