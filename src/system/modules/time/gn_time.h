
/*
 * gn_time.h
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#ifndef _MODULE_TIME_H__
#define _MODULE_TIME_H__

#include <stdint.h>


namespace TIME
{

#define TIME_EVENT_OFFSET 0x1200

enum {
  TIMESET_EVENT = TIME_EVENT_OFFSET
};


#pragma pack(push, 1)
typedef struct
{
  uint32_t time;
  uint8_t   absSet;
} current_time_t;
#pragma pack(pop)


const current_time_t* getCurrentTime();
void                  setCurrentTime(current_time_t currentTime);
void                  startClock();

void init();


}


#endif  /* _MODULE_TIME_H__ */



