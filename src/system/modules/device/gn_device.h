
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
  ACTIVATE_EVENT = DEVICE_EVENT_OFFSET,
  DEACTIVATE_EVENT,
  DEVICE_ACTIVATED,
  DEVICE_DEACTIVATED
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
} trap_control_t;

typedef struct
{
  uint8_t major;
  uint8_t minor;
} software_version_t;

#pragma pack(pop)

uint32_t getBootNum();
void setBootNum(uint32_t newBootNum);

uint32_t getDeviceID();
void setDeviceID(uint32_t newID);
uint8_t getDeviceState();
software_version_t getSoftwareVersion();

void init();


}


#endif  /* _MODULE_DEVICE_H__ */



