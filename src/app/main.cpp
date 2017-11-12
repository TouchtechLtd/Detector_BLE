
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

#include "app/events.h"


#define LED_FAST_BLINK 200
#define LED_SLOW_BLINK 1000

#define CONFIG_FILE     (0xF010)
#define CONFIG_REC_KEY  (0x7010)


Timer ledTimer;


static TrapState::event_data_t       recordData = { 0 };
static TrapState::event_data_t       eventData = { 0 };
static uint8_t                       killNumber = 0;


///////////////////////////////////////////////////
//////////        Event handlers        ///////////
///////////////////////////////////////////////////

/*
void bleEventHandler(ble_evt_t const * p_ble_evt, void* context)
{
  INFO("Reveived BT event in Main");
  switch (p_ble_evt->header.evt_id)
  {
    case BLE_GAP_EVT_CONNECTED:
        updateBLE = true;
        break; // BLE_GAP_EVT_CONNECTED
  }
}
*/

void onKillEvent()
{
  INFO("Killed");

  eventData = { 0 };
  killNumber++;

  int32_t temp;
  LIS2DH12_updateTemperatureSensor();
  LIS2DH12_getTemperature(&temp);

  eventData.timestamp   =     *CurrentTime::getCurrentTime();
  eventData.trap_id     =     0x12345678;
  eventData.temperature =     static_cast<int8_t>(temp);
  eventData.killNumber  =     killNumber;

  Flash_Record::write(KILL_DATA_FILE_ID, killNumber, &eventData, sizeof(eventData));
  Flash_Record::write(KILL_NUMBER_FILE_ID, KILL_NUMBER_KEY_ID, &killNumber, sizeof(killNumber));

  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DISPLAYED, &killNumber, sizeof(killNumber));


}






///////////////////////////////////////////////////
//////        Interrupt handlers        ///////////
///////////////////////////////////////////////////


void buttonHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  TrapState::simulateTrigger();
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

  switch (TrapState::getState())
  {
    case TrapState::WAIT_STATE:
      ledTimer.stopTimer();
      GPIO::setOutput(LED_2_PIN, HIGH);
      break;

    case TrapState::EVENT_BUFFER_STATE:
      ledTimer.stopTimer();
      GPIO::setOutput(LED_2_PIN, LOW);
      break;

    case TrapState::DETECT_MOVE_STATE:
      ledTimer.stopTimer();
      ledTimer.startTimer(LED_FAST_BLINK, led2Toggle);
      break;

    case TrapState::MOVING_STATE:
      ledTimer.stopTimer();
      ledTimer.startTimer(LED_SLOW_BLINK, led2Toggle);
      break;

    default:
      break;
  }
  //BLE_SERVER::setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_OUTPUT_STATE, &currentState, sizeof(currentState));
}


TrapState::event_data_t* getEvent(uint8_t eventID)
{
  Flash_Record::read(KILL_DATA_FILE_ID, eventID, &recordData, sizeof(recordData));
  return &recordData;
}

///////////////////////////////////////////////////
//////        BLE functions             ///////////
///////////////////////////////////////////////////


void createTrapDataService()
{
  BLE_SERVER::Service trapData;
  trapData.createCustom(BLE_UUID_SERVICE_TRAP_DATA, BLE_UUID_GOODNATURE_BASE);

  BLE_SERVER::Characteristic eventData;
  TrapState::event_data_t blankEvent = { 0 };
  eventData.configure(BLE_UUID_CHAR_TRAP_EVENT_DATA, &blankEvent, sizeof(blankEvent), BLE_SERVER::CHAR_READ_ONLY);
  trapData.addCharacteristic(&eventData, CHAR_EVENT_DATA);

  BLE_SERVER::Characteristic eventConfig;
  TrapState::trap_detector_config_t* defaultConfig = TrapState::getConfig();
  eventConfig.configure(BLE_UUID_CHAR_TRAP_EVENT_CONFIG, &defaultConfig, sizeof(*defaultConfig), BLE_SERVER::CHAR_READ_WRITE);
  trapData.addCharacteristic(&eventConfig, CHAR_EVENT_CONFIG);

  // Characteristic for displaying number of kills, can be written to in order to display a different kill
  BLE_SERVER::Characteristic eventDisplayed;
  //uint8_t killNum = TrapState::getKillNumber();
  eventDisplayed.configure(BLE_UUID_CHAR_TRAP_EVENT_DISPLAYED, &killNumber, sizeof(killNumber), BLE_SERVER::CHAR_READ_WRITE);
  trapData.addCharacteristic(&eventDisplayed, CHAR_EVENT_DISPLAYED);

  BLE_SERVER::Characteristic trapTime;
  CurrentTime::current_time_t startTime = *CurrentTime::getCurrentTime();
  trapTime.configure(BLE_UUID_CHAR_TRAP_TIME, &startTime, sizeof(startTime), BLE_SERVER::CHAR_READ_WRITE);
  trapData.addCharacteristic(&trapTime, CHAR_TRAP_TIME);

  //Characteristic rawEventData;

  trapData.attachService();
  BLE_SERVER::addService(&trapData, SERVICE_TRAP_DATA);

}


void createDeviceInfoService() {

  BLE_SERVER::Service deviceInfo;
  deviceInfo.createSIG(BLE_UUID_SIG_SERVICE_DEVICE_INFO);

  deviceInfo.attachService();
  BLE_SERVER::addService(&deviceInfo, SERVICE_DEVICE_INFO);
}


void updateEventBLE()
{
  //uint8_t currentKillNumber = TrapState::getKillNumber();
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DISPLAYED,  &killNumber,                                sizeof(killNumber));
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DATA,       getEvent(killNumber),                sizeof(TrapState::event_data_t));
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_CONFIG,     TrapState::getConfig(),                     sizeof(TrapState::trap_detector_config_t));
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_TRAP_TIME,        CurrentTime::getCurrentTime(),              sizeof(CurrentTime::current_time_t));

}

///////////////////////////////////////////////////
//////     BLE Write handlers     /////////////////
///////////////////////////////////////////////////

void eventConfigHandler(uint8_t const* data, uint16_t len)
{
  TrapState::trap_detector_config_t* p_config = TrapState::getConfig();
  memcpy(p_config, data, len);
  uint32_t err_code = BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_CONFIG, TrapState::getConfig(), sizeof(TrapState::trap_detector_config_t));
  ERROR_CHECK(err_code);
  DEBUG("Trigger Threshold: %d", p_config->triggerThreshold);
}


void eventDisplayedHandler(uint8_t const* data, uint16_t len)
{
  uint8_t requestedKill;
  memcpy(&requestedKill, data, len);
  uint32_t err_code = BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DATA, getEvent(requestedKill), sizeof(TrapState::event_data_t));
  ERROR_CHECK(err_code);
  DEBUG("Requested Kill: %d", requestedKill);
}

void trapTimeHandler(uint8_t const* data, uint16_t len)
{
  CurrentTime::current_time_t* p_currentTime = CurrentTime::getCurrentTime();
  memcpy(p_currentTime, data, len);
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_TRAP_TIME, p_currentTime, sizeof(CurrentTime::current_time_t));
  DEBUG("Time Set To: %d", p_currentTime->time);
}

///////////////////////////////////////////////////
//////     Initialisation functions      ///////////
///////////////////////////////////////////////////


void initialisePeripherals()
{
  FDS::init();
  FDS::status();
  FDS::clean();


  INFO("Timer Init");
  Timer::initialisePeripheral();

  INFO("Clock Init");
  CurrentTime::startClock();

  INFO("GPIO Init");
  GPIO::init();
  GPIO::setOutput(LED_1_PIN, HIGH);
  GPIO::setOutput(LED_2_PIN, HIGH);

  INFO("BLE Init");
  BLE_SERVER::init();
  BLE_SERVER::setPower(BLE_POWER_LEVEL_HIGH);

  INFO("LIS Init");
  LIS2DH12_init(LIS2DH12_POWER_LOW, LIS2DH12_SCALE4G, LIS2DH12_SAMPLE_50HZ);
  LIS2DH12_enableHighPass();
  LIS2DH12_enableTemperatureSensor();

  INFO("Trap State starting");
  TrapState::initialise();
}



void loadDataFromFlash()
{
  Flash_Record::read(KILL_NUMBER_FILE_ID, KILL_NUMBER_KEY_ID, &killNumber, sizeof(killNumber));
}

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
  INFO("BLE Starting");
  createTrapDataService();
  createDeviceInfoService();
  //NRF_SDH_BLE_OBSERVER(m_main_ble_observer, APP_MAIN_OBSERVER_PRIO, bleEventHandler, NULL);

  BLE_ADVERTISING::start(320);
  BLE_ADVERTISING::advertiseName();
  BLE_ADVERTISING::advertiseUUID(BLE_SERVER::getService(SERVICE_TRAP_DATA)->getUUID());

  BLE_SERVER::setWriteHandler(SERVICE_TRAP_DATA, CHAR_EVENT_CONFIG,      eventConfigHandler);
  BLE_SERVER::setWriteHandler(SERVICE_TRAP_DATA, CHAR_EVENT_DISPLAYED,   eventDisplayedHandler);
  BLE_SERVER::setWriteHandler(SERVICE_TRAP_DATA, CHAR_TRAP_TIME,         trapTimeHandler);

  INFO("BLE Started");


}


void registerEventCallbacks ()
{
  EVENTS::registerEventHandler(TrapState::TRAP_KILLED_EVENT,        onKillEvent);
  EVENTS::registerEventHandler(TrapState::TRAP_STATE_CHANGE_EVENT,  showState);
  EVENTS::registerEventHandler(BLE_SERVER::BLE_CONNECTED_EVENT,     updateEventBLE);
}



///////////////////////////////////////////////////
//////        Main                      ///////////
///////////////////////////////////////////////////


int main(void)
{
	DEBUG_INIT();
  INFO("Started");

	initialisePeripherals();
	loadDataFromFlash();
	registerEventCallbacks();
	setButtonInterrupt();

	startBLE();



  //Timer blinkTimer;
  //blinkTimer.startTimer(1000, led2Toggle);

  EVENTS::eventPut(10);


  INFO("Looping");
  while(true)
  {

    EVENTS::processEvents();

    if (!NRF_LOG_PROCESS())
    {
      uint32_t err_code = sd_app_evt_wait();
      ERROR_CHECK(err_code);
    }

    //nrf_delay_ms(1000);

  }

}


/**
 *@}
 **/



















