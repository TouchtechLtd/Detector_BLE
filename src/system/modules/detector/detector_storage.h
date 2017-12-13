
/*
 * detector_storage.h
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#ifndef _MODULE_DETECTOR_STORAGE_H__
#define _MODULE_DETECTOR_STORAGE_H__

#include <stdint.h>
#include "gn_detector.h"


namespace DETECTOR
{

namespace STORAGE
{

#define KILL_DATA_FILE_ID       (0xDA7A)
#define KILL_NUMBER_FILE_ID     (0x5111)
#define KILL_NUMBER_KEY_ID      (0x1111)


void          recordKill(event_data_t* eventData);
event_data_t  getKill   (uint8_t killNumber);

void start();

//void writeData();
//void getData();


}
}


#endif  /* _MODULE_DETECTOR_STORAGE_H__ */
