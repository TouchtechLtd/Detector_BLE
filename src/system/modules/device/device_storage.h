




/*
 * device_storage.h
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#ifndef _MODULE_DEVICE_STORAGE_H__
#define _MODULE_DEVICE_STORAGE_H__

#include <stdint.h>
#include "gn_device.h"


namespace DEVICE
{

namespace STORAGE
{

#define CONFIG_FILE_ID      (0xF010)
#define CONFIG_REC_KEY_ID   (0x7010)
#define TRAP_ID_KEY_ID      (0x1212)


void       recordID();
uint32_t   getID();

void start();


}
}


#endif  /* _MODULE_DEVICE_STORAGE_H__ */
