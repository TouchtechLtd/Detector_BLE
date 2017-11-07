
/*
 * gpio_interface.h
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#ifndef _GOODNATURE_PERIPHERAL_FLASH_INTERFACE_H_
#define _GOODNATURE_PERIPHERAL_FLASH_INTERFACE_H_

#include "nrf_gpio.h"
#include "app_gpiote.h"
#include "nrf_drv_gpiote.h"
#include "boards.h"

#include "fds.h"



class Flash_Record {
	private:

	public:
		static void read(uint16_t file_id, uint16_t key_id, void* p_data, uint32_t len);
		static void write(uint16_t file_id, uint16_t key_id, void* p_data, uint32_t len);
		static bool doesExist(uint16_t file_id, uint16_t key_id);

	}; // End Flash_Record


class FDS {
  private:
    static volatile bool m_initialised;
    static volatile bool m_writeEvent;
    static volatile bool m_updateEvent;

  public:
    static void init();
    static void event_handler(fds_evt_t const * p_evt);
    static void waitForInitialisation();
    static void waitForWrite();
    static void waitForUpdate();

    static void clean();
    static void status();
};



#endif /* _GOODNATURE_PERIPHERAL_FLASH_INTERFACE_H_ */
