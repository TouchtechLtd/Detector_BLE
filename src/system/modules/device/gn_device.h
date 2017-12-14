
/*
 * gn_device.h
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#ifndef _MODULE_DEVICE_H__
#define _MODULE_DEVICE_H__

#include <stdint.h>


namespace DEVICE
{

#define TRAP_ID       0x00000000
#define DEVICE_EVENT_OFFSET 0x1400

enum {
  DEVICE_EVENT = DEVICE_EVENT_OFFSET
};


#pragma pack(push, 1)

typedef struct
{
  uint32_t trapID;
  uint8_t mainState;
  uint8_t detectorState;
}trap_info_t;

typedef struct
{
  uint8_t activate;
  uint32_t trapID;
} trap_control_t;

#pragma pack(pop)


uint32_t getDeviceID();
void setDeviceID(uint32_t newID);
void init();


}


#endif  /* _MODULE_DEVICE_H__ */



