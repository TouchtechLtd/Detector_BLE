
/*
 * main.cpp
 *
 *  Created on: September 5, 2017
 *      Author: Michael McAdam
 */

#include <app/trap_manager_config.h>
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

#include "app/state_machine.h"
#include "ble/ble_interface.h"
#include "ble/gn_ble_advertising.h"
#include "ble/gn_ble_config.h"
#include "debug/DEBUG.h"
#include "peripheral/timer_interface.h"
#include "peripheral/adc_interface.h"
#include "peripheral/gpio_interface.h"
#include "app/current_time.h"
#include "peripheral/flash_interface.h"
#include "peripheral/LIS2DH12.h"

#include "app/trap_manager.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define LED_FAST_BLINK 200
#define LED_SLOW_BLINK 1000

#define CONFIG_FILE     (0xF010)
#define CONFIG_REC_KEY  (0x7010)

static volatile bool updateBLE = true;
volatile bool stateChange = false;

Timer ledTimer;



///////////////////////////////////////////////////
//////////        Event handlers        ///////////
///////////////////////////////////////////////////

void onTrapEvent(EVENT_MANAGER::trap_event_e trap_event)
{
  INFO("Trap update in main: %d", trap_event);
  if (trap_event == EVENT_MANAGER::MOVE_TRIGGERED) {}

  else if (trap_event == EVENT_MANAGER::ANIMAL_KILLED)
  {
    uint8_t data = EVENT_MANAGER::getKillNumber();
    BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DISPLAYED, &data, sizeof(data));
  }
}


void bleEventHandler(ble_evt_t const * p_ble_evt, void* context)
{
  switch (p_ble_evt->header.evt_id)
  {
    case BLE_GAP_EVT_CONNECTED:
        updateBLE = true;
        break; // BLE_GAP_EVT_CONNECTED
  }
}


///////////////////////////////////////////////////
//////        Interrupt handlers        ///////////
///////////////////////////////////////////////////


void buttonHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  EVENT_MANAGER::simulateTrigger();
  DEBUG("Button pressed");
}


///////////////////////////////////////////////////
/////////        Timer handlers         ///////////
///////////////////////////////////////////////////

void led1Toggle(void*)
{
  GPIO::toggle(LED_1_PIN);
}


void led2Toggle(void*)
{
  GPIO::toggle(LED_2_PIN);
}


///////////////////////////////////////////////////
//////////        User output          ///////////
///////////////////////////////////////////////////

void showState()
{

  switch (EVENT_MANAGER::getState())
  {
    case EVENT_MANAGER::WAIT_STATE:
      ledTimer.stopTimer();
      GPIO::setOutput(LED_2_PIN, HIGH);
      break;

    case EVENT_MANAGER::EVENT_BUFFER_STATE:
      ledTimer.stopTimer();
      GPIO::setOutput(LED_2_PIN, LOW);
      break;

    case EVENT_MANAGER::DETECT_MOVE_STATE:
      ledTimer.stopTimer();
      ledTimer.startTimer(LED_FAST_BLINK, led1Toggle);
      break;

    case EVENT_MANAGER::MOVING_STATE:
      ledTimer.stopTimer();
      ledTimer.startTimer(LED_SLOW_BLINK, led1Toggle);
      break;

    default:
      break;
  }
  //BLE_SERVER::setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_OUTPUT_STATE, &currentState, sizeof(currentState));
}


///////////////////////////////////////////////////
//////        BLE functions             ///////////
///////////////////////////////////////////////////


void createTrapDataService()
{
  Service trapData;
  trapData.createCustom(BLE_UUID_SERVICE_TRAP_DATA, BLE_UUID_GOODNATURE_BASE);

  Characteristic eventData;
  eventData.setUUID(BLE_UUID_CHAR_TRAP_EVENT_DATA);
  eventData.enableRead();
  eventData.enableNotification();
  EVENT_MANAGER::event_data_t blankEvent = { 0 };
  eventData.initValue(&blankEvent, sizeof(blankEvent));

  trapData.addCharacteristic(&eventData, CHAR_EVENT_DATA);

  Characteristic eventConfig;
  eventConfig.setUUID(BLE_UUID_CHAR_TRAP_EVENT_CONFIG);
  eventConfig.enableRead();
  eventConfig.enableWrite();
  eventConfig.enableNotification();
  eventConfig.initValue(EVENT_MANAGER::getConfig(), sizeof(EVENT_MANAGER::trap_detector_config_t));

  trapData.addCharacteristic(&eventConfig, CHAR_EVENT_CONFIG);

  // Characteristic for displaying number of kills, can be written to in order to display a different kill
  Characteristic eventDisplayed;
  eventDisplayed.setUUID(BLE_UUID_CHAR_TRAP_EVENT_DISPLAYED);
  eventDisplayed.enableRead();
  eventDisplayed.enableWrite();
  eventDisplayed.enableNotification();
  uint8_t killNum = EVENT_MANAGER::getKillNumber();
  eventDisplayed.initValue(&killNum, sizeof(killNum));

  trapData.addCharacteristic(&eventDisplayed, CHAR_EVENT_DISPLAYED);
  //Characteristic rawEventData;

  Characteristic trapTime;
  trapTime.setUUID(BLE_UUID_CHAR_TRAP_TIME);
  trapTime.enableRead();
  trapTime.enableWrite();
  trapTime.enableNotification();

  trapTime.initValue(CurrentTime::getCurrentTime(), sizeof(CurrentTime::current_time_t));

  trapData.addCharacteristic(&trapTime, CHAR_TRAP_TIME);

  trapData.attachService();
  BLE_SERVER::addService(&trapData, SERVICE_TRAP_DATA);

}


void createDeviceInfoService() {

  Service deviceInfo;
  deviceInfo.createSIG(BLE_UUID_SIG_SERVICE_DEVICE_INFO);

  deviceInfo.attachService();
  BLE_SERVER::addService(&deviceInfo, SERVICE_DEVICE_INFO);
}


void updateEventBLE()
{
  uint8_t currentKillNumber = EVENT_MANAGER::getKillNumber();
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DISPLAYED, &currentKillNumber, sizeof(currentKillNumber));
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DATA, EVENT_MANAGER::getEvent(currentKillNumber), sizeof(EVENT_MANAGER::event_data_t));
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_CONFIG, EVENT_MANAGER::getConfig(), sizeof(EVENT_MANAGER::trap_detector_config_t));
}

///////////////////////////////////////////////////
//////     BLE Write handlers     /////////////////
///////////////////////////////////////////////////

void eventConfigHandler(uint8_t const* data, uint16_t len)
{
  EVENT_MANAGER::trap_detector_config_t* p_config = EVENT_MANAGER::getConfig();
  memcpy(p_config, data, len);
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_CONFIG, EVENT_MANAGER::getConfig(), sizeof(EVENT_MANAGER::trap_detector_config_t));
  DEBUG("Trigger Threshold: %d", p_config->triggerThreshold);
}


void eventDisplayedHandler(uint8_t const* data, uint16_t len)
{
  uint8_t requestedKill;
  memcpy(&requestedKill, data, len);
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DATA, EVENT_MANAGER::getEvent(requestedKill), sizeof(EVENT_MANAGER::event_data_t));
  DEBUG("Requested Kill: %d", requestedKill);
}

void trapTimeHandler(uint8_t const* data, uint16_t len)
{
  CurrentTime::current_time_t* p_currentTime = CurrentTime::getCurrentTime();
  memcpy(p_currentTime, data, len);
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DATA, p_currentTime, sizeof(CurrentTime::current_time_t));
  DEBUG("Time Set To: %d", p_currentTime->time);
}

///////////////////////////////////////////////////
//////     Initialisation functions      ///////////
///////////////////////////////////////////////////


void initGPIO()
{
  GPIO::init();
  GPIO::setOutput(LED_1_PIN, HIGH);
  GPIO::setOutput(LED_2_PIN, HIGH);

  GPIO::initIntInput(BUTTON_1,
                NRF_GPIOTE_POLARITY_HITOLO,
                NRF_GPIO_PIN_PULLUP,
                false,
                false,
                buttonHandler);
  GPIO::interruptEnable(BUTTON_1);

}


void initBLE()
{
  BLE_SERVER::init();
  BLE_SERVER::setPower(BLE_POWER_LEVEL_HIGH);
  createTrapDataService();
  createDeviceInfoService();
  NRF_SDH_BLE_OBSERVER(m_main_ble_observer, APP_MAIN_OBSERVER_PRIO, bleEventHandler, NULL);

  BLE_ADVERTISING::start(320);
  BLE_ADVERTISING::advertiseName();

  BLE_SERVER::setWriteHandler(SERVICE_TRAP_DATA, CHAR_EVENT_CONFIG,      eventConfigHandler);
  BLE_SERVER::setWriteHandler(SERVICE_TRAP_DATA, CHAR_EVENT_DISPLAYED,   eventDisplayedHandler);
  BLE_SERVER::setWriteHandler(SERVICE_TRAP_DATA, CHAR_TRAP_TIME,         trapTimeHandler);


}

void initTimer()
{
  Timer::initialisePeripheral();
  CurrentTime::startClock();
}


void initFlash()
{
  FDS::init();
  FDS::status();
  FDS::clean();
}


///////////////////////////////////////////////////
//////        Main                      ///////////
///////////////////////////////////////////////////


int main(void)
{
	DEBUG_INIT();
  INFO("Started");


  initFlash();
  initTimer();
  initGPIO();
  initBLE();
  EVENT_MANAGER::initialise();
  EVENT_MANAGER::registerEventHandler(onTrapEvent);


  Timer blinkTimer;
  blinkTimer.startTimer(1000, led2Toggle);


  while(true)
  {

    if (!NRF_LOG_PROCESS())
    {
      uint32_t err_code = sd_app_evt_wait();
      ERROR_CHECK(err_code);
    }

    if (updateBLE) {
      updateEventBLE();
      updateBLE = false;
    }

    if (stateChange) {
      showState();
      stateChange = false;
    }

    nrf_delay_ms(1000);

  }

}


/**
 *@}
 **/



















