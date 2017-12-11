
/*
 * ble_buttonless_dfu.cpp
 *
 *  Created on: September 5, 2017
 *      Author: Michael McAdam
 */


#include <stdint.h>
#include <string.h>

#include "nrf_dfu_svci.h"
#include "nrf_svci_async_function.h"
#include "nrf_svci_async_handler.h"
#include "ble_dfu.h"


#include "libraries/events/events.h"
#include "libraries/ble/ble_buttonless_dfu.h"
#include "drivers/gpio/gpio_interface.h"
#include "libraries/debug/DEBUG.h"


#define NRF_LOG_MODULE_NAME BUTTONLESS_DFU
NRF_LOG_MODULE_REGISTER();



namespace BLE_DFU
{


static void dfu_evt_handler(ble_dfu_buttonless_evt_type_t event)
{
    switch (event)
    {
        case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE:
            INFO("Device is preparing to enter bootloader mode.");
            // YOUR_JOB: Disconnect all bonded devices that currently are connected.
            //           This is required to receive a service changed indication
            //           on bootup after a successful (or aborted) Device Firmware Update.
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER:
            // YOUR_JOB: Write app-specific unwritten data to FLASH, control finalization of this
            //           by delaying reset by reporting false in app_shutdown_handler
            INFO("Device will enter bootloader mode.");
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED:
            INFO("Request to enter bootloader mode failed asynchroneously.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            break;

        case BLE_DFU_EVT_RESPONSE_SEND_ERROR:
            INFO("Request to send a response to client failed.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            //ERROR_CHECK(false);
            break;

        default:
            INFO("Unknown event from ble_dfu_buttonless.");
            break;
    }
}


extern void createDFUService()
{
  uint32_t err_code;
  ble_dfu_buttonless_init_t dfus_init =
  {
      .evt_handler = dfu_evt_handler
  };

  // Initialize the async SVCI interface to bootloader.
  err_code = ble_dfu_buttonless_async_svci_init();
  ERROR_CHECK(err_code);


  err_code = ble_dfu_buttonless_init(&dfus_init);
  ERROR_CHECK(err_code);
}

}
