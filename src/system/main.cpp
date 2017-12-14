
/*
 * main.cpp
 *
 *  Created on: September 5, 2017
 *      Author: Michael McAdam
 */

#include <system/modules/detector/old_files/trap_manager_config.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "boards.h"
#include "nrf_drv_gpiote.h"

#include "libraries/state/state_machine.h"
#include "libraries/ble/ble_interface.h"
#include "libraries/ble/gn_ble_advertising.h"
#include "libraries/ble/gn_ble_config.h"
#include "libraries/debug/DEBUG.h"
#include "drivers/timer/timer_interface.h"
#include "drivers/adc/adc_interface.h"
#include "drivers/gpio/gpio_interface.h"
#include "libraries/current_time/current_time.h"
#include "drivers/flash/flash_interface.h"
#include "drivers/LIS2DH12/LIS2DH12.h"

#include "libraries/ble/ble_buttonless_dfu.h"

//#include "system/modules/detector/old_files/trap_manager.h"

#include "system/modules/detector/gn_detector.h"
#include "system/modules/raw_event/gn_raw_event.h"
#include "system/modules/time/gn_time.h"
#include "system/modules/device/gn_device.h"


#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "libraries/events/events.h"


#define NRF_LOG_MODULE_NAME MAIN
NRF_LOG_MODULE_REGISTER();



#define ERROR_FILE_ID   0x5432
#define ERROR_KEY_ID    0x6543


#define MAIN_EVENT_OFFSET 0x1500




enum {
  PROGRAM_ERROR_EVENT = MAIN_EVENT_OFFSET,
};



/*

*/
///////////////////////////////////////////////////
//////////        Event handlers        ///////////
///////////////////////////////////////////////////

void setBLEOutputHigh(EVENTS::event_data_t)
{
  BLE_SERVER::setPower(BLE_POWER_LEVEL_HIGH);
}


void onBLEDisconnect()
{
  GPIO::high(LED_1_PIN);
  BLE_SERVER::setPower(BLE_POWER_LEVEL_LOW);
  BLE_ADVERTISING::start(BLE_ADVERTISING_SPEED_SLOW);
}

void onBLEConnect()
{
  GPIO::low(LED_1_PIN);
}



///////////////////////////////////////////////////
//////        Interrupt handlers        ///////////
///////////////////////////////////////////////////


void buttonHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  DETECTOR::simulateTrigger();

  //EVENTS::eventPut(ACTIVATE_EVENT);
  INFO("Event: Button 1 Pressed");
}


///////////////////////////////////////////////////
/////////        Timer handlers         ///////////
///////////////////////////////////////////////////




///////////////////////////////////////////////////
//////////        User output          ///////////
///////////////////////////////////////////////////


///////////////////////////////////////////////////
//////        BLE functions             ///////////
///////////////////////////////////////////////////



///////////////////////////////////////////////////
//////     BLE Write handlers     /////////////////
///////////////////////////////////////////////////


///////////////////////////////////////////////////
//////     Initialisation functions      ///////////
///////////////////////////////////////////////////


void initialisePeripherals()
{
  INFO("INITIALISING - Flash Peripheral");
  FDS::init();
  FDS::status();
  FDS::clean();

  INFO("INITIALISING - Timer Peripheral");
  Timer::initialisePeripheral();


  INFO("INITIALISING - GPIO Peripheral");
  GPIO::init();
  GPIO::setOutput(LED_1_PIN, HIGH);
  GPIO::setOutput(LED_2_PIN, HIGH);

  INFO("INITIALISING - BLE Peripheral");
  BLE_SERVER::init();
  BLE_SERVER::setPower(BLE_POWER_LEVEL_HIGH);

  INFO("INITIALISING - Accelerometer Peripheral");
  LIS2DH12::init(LIS2DH12::POWER_LOW, LIS2DH12::SCALE8G, LIS2DH12::SAMPLE_50HZ);

  LIS2DH12::enableHighPass();
  LIS2DH12::enableFIFO();
  LIS2DH12::enableTemperatureSensor();


}


/*
void loadDataFromFlash()
{

  // If the previous program exited with an error, find it here.
  static uint32_t previousError = 0;
  Flash_Record::read(ERROR_FILE_ID, ERROR_KEY_ID, &previousError, sizeof(previousError));
  if (previousError != 0) { INFO("READING - Previous error - Value: %d", previousError); }

}
*/
void setButtonInterrupt()
{
  GPIO::initIntInput(BUTTON_1,
                NRF_GPIOTE_POLARITY_HITOLO,
                NRF_GPIO_PIN_PULLUP,
                false,
                false,
                buttonHandler);
  GPIO::interruptEnable(BUTTON_1);
}


void startBLE()
{

//#ifndef DEBUG
  //BLE_DFU::createDFUService();
//#endif

  BLE_ADVERTISING::start(BLE_ADVERTISING_SPEED_SLOW);
  BLE_ADVERTISING::advertiseName();
  BLE_ADVERTISING::advertiseUUID(BLE_SERVER::getService(SERVICE_TRAP_DATA)->getUUID());

}

////////////////////////////////////////////
//////////    Error Handling    ////////////
////////////////////////////////////////////

/*
static uint32_t finalErrorCode;


void errorShowHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  GPIO::toggle(LED_1_PIN);
}

void errorStateHandler()
{
  g_trapInfo.mainState = mainStateMachine.getCurrentState();
  //BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_TRAP_INFO, &g_trapInfo, sizeof(g_trapInfo));
  GPIO::interruptClear(BUTTON_1);
  TrapState::stop();
  GPIO::initIntInput(BUTTON_1,
                NRF_GPIOTE_POLARITY_HITOLO,
                NRF_GPIO_PIN_PULLUP,
                false,
                false,
                errorShowHandler);
  GPIO::interruptEnable(BUTTON_1);

  //BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_ERROR_DATA,  &finalErrorCode, sizeof(finalErrorCode));
  Flash_Record::write(ERROR_FILE_ID, ERROR_KEY_ID, &finalErrorCode, sizeof(finalErrorCode));

  for(;;)
  {
    if (!NRF_LOG_PROCESS()) sd_app_evt_wait();
  }
}

void errorHandler(EVENTS::event_data_t data)
{
  finalErrorCode = *(uint32_t*)data.p_data;
  //mainStateMachine.transition(PROGRAM_ERROR_EVENT);
  errorStateHandler();
}
*/


void eventTriggered()
{
  EVENTS::eventPut(RAW_EVENT::EVENT_TRIGGERED);
}

void registerEventCallbacks ()
{
  EVENTS::registerEventHandler(DETECTOR::DETECTOR_TRIGGERED,     eventTriggered);
  EVENTS::registerEventHandler(DEVICE::DEVICE_ACTIVATED,         DETECTOR::start);
  EVENTS::registerEventHandler(DEVICE::DEVICE_DEACTIVATED,       DETECTOR::stop);

  EVENTS::registerEventHandler(BLE_SERVER::BLE_CONNECTED_EVENT,     onBLEConnect);
  EVENTS::registerEventHandler(BLE_SERVER::BLE_DISCONNECTED_EVENT,  onBLEDisconnect);

  EVENTS::registerEventHandler(DETECTOR::DETECTED_KILL,             setBLEOutputHigh);
  EVENTS::registerEventHandler(BLE_SERVER::BLE_CONNECTED_EVENT,     setBLEOutputHigh);
  /*

  //EVENTS::registerEventHandler(TrapState::TRAP_STATE_CHANGE_EVENT,  showState);
  EVENTS::registerEventHandler(DEBUG_ERROR_EVENT,                   errorHandler);
  */
}


///////////////////////////////////////////////////
//////        Main                      ///////////
///////////////////////////////////////////////////


static void sensors_init(void)
{
    nrf_gpio_cfg_output(SPIM0_SS_ACC_PIN);
    nrf_gpio_pin_set(SPIM0_SS_ACC_PIN);
    nrf_gpio_cfg_output(SPIM0_SS_HUMI_PIN);
    nrf_gpio_pin_set(SPIM0_SS_HUMI_PIN);
}


int main(void)
{


	DEBUG_INIT();
  INFO("\n\r\n\rDebug started...");

	initialisePeripherals();

	registerEventCallbacks();
	setButtonInterrupt();

	TIME::init();
	DEVICE::init();
	DETECTOR::init();
	RAW_EVENT::init();

	//sensors_init();
	startBLE();


  INFO("Starting main loop");

  while(true)
  {

    EVENTS::processEvents();

    if (!NRF_LOG_PROCESS())
    {
      uint32_t err_code = sd_app_evt_wait();
      ERROR_CHECK(err_code);
    }

    //sd_app_evt_wait();

    //INFO("Looping");
    //nrf_delay_ms(500);

  }

}


/**
 *@}
 **/



















