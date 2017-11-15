
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

#include <math.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "app/events.h"


#define LED_FAST_BLINK 200
#define LED_SLOW_BLINK 1000

#define CONFIG_FILE_ID     (0xF010)
#define CONFIG_REC_KEY_ID  (0x7010)

#define ERROR_FILE_ID   0x5432
#define ERROR_KEY_ID    0x6543

#define TRAP_ID       0x12345678

#define RAW_DATA_BLE_SIZE 5

#define MAIN_EVENT_OFFSET 0x1500


typedef enum {
  IDLE_STATE,
  ACTIVE_STATE,
  ERROR_STATE,
  MAX_STATES,
} main_state_e;


typedef enum {
  TIMESET_EVENT,
  PROGRAM_ERROR_EVENT,
  MAX_EVENTS
} main_event_e;

enum {
  RAW_DATA_FULL = MAIN_EVENT_OFFSET,
  SEND_RAW_DATA
};


Timer ledTimer;
static TrapState::event_data_t       recordData = { 0 };
static TrapState::event_data_t       eventData = { 0 };
static uint8_t                       killNumber = 0;

static TrapState::raw_event_data_t g_accelerationData[RAW_DATA_CAPTURE_SIZE] = { 0 };
static uint16_t       g_accDataCount = 0;

static StateMachine mainStateMachine(IDLE_STATE, MAX_STATES, MAX_EVENTS);

///////////////////////////////////////////////////
//////////        Event handlers        ///////////
///////////////////////////////////////////////////

void processRawData();

void onKillEvent(EVENTS::event_data_t data)
{
  INFO("Killed");

  killNumber++;

  int32_t temp;
  LIS2DH12_updateTemperatureSensor();
  LIS2DH12_getTemperature(&temp);

  processRawData();

  eventData.timestamp   =     *CurrentTime::getCurrentTime();
  eventData.trap_id     =     TRAP_ID;
  eventData.temperature =     static_cast<int8_t>(temp);
  eventData.killNumber  =     killNumber;

  Flash_Record::write(KILL_DATA_FILE_ID, killNumber, &eventData, sizeof(eventData));
  Flash_Record::write(KILL_NUMBER_FILE_ID, KILL_NUMBER_KEY_ID, &killNumber, sizeof(killNumber));

  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DISPLAYED, &killNumber, sizeof(killNumber));

  memset(&eventData, 0, sizeof(eventData));
  memset(&g_accelerationData, 0, sizeof(g_accelerationData));

  BLE_SERVER::setPower(BLE_POWER_LEVEL_HIGH);

}


void accReadRawDataHandler(void* p_context)
{

  LIS2DH12_sample();

  if (g_accDataCount >= RAW_DATA_CAPTURE_SIZE - 1)
  {
    EVENTS::eventPut(RAW_DATA_FULL, 0);
    return;
  }
  LIS2DH12_getAccelerationData_8Bit(&g_accelerationData[g_accDataCount].acc);

  g_accDataCount++;

}

void processRawData()
{

  for (int i = 0; i < g_accDataCount; i++)
  {

    g_accelerationData[i].sum = sqrt((g_accelerationData[i].acc.x*g_accelerationData[i].acc.x) +
                                     (g_accelerationData[i].acc.y*g_accelerationData[i].acc.y) +
                                     (g_accelerationData[i].acc.z*g_accelerationData[i].acc.z));

    if (g_accelerationData[i].sum > eventData.peak_level)
    {
      eventData.peak_level = g_accelerationData[i].sum;
    }
  }
  g_accDataCount = 0;
  Flash_Record::write(KILL_RAW_DATA_FILE_ID, killNumber, &g_accelerationData, sizeof(g_accelerationData));

}

void startRawSampling(EVENTS::event_data_t data)
{
  LIS2DH12_startDAPolling();
}

void stopRawSampling(EVENTS::event_data_t data)
{
  LIS2DH12_stopDAPolling();
}


void onBLEDisconnect(EVENTS::event_data_t)
{
  BLE_SERVER::setPower(BLE_POWER_LEVEL_LOW);
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

void showState(EVENTS::event_data_t data)
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

  BLE_SERVER::Characteristic rawEventData;
  TrapState::raw_event_data_t tmpRawData[RAW_DATA_BLE_SIZE] = {0};
  rawEventData.configure(BLE_UUID_CHAR_RAW_DATA, &tmpRawData, sizeof(tmpRawData), BLE_SERVER::CHAR_READ_ONLY);
  trapData.addCharacteristic(&rawEventData, CHAR_RAW_DATA);

  BLE_SERVER::Characteristic errorData;
  uint32_t errorInitData = 0;
  errorData.configure(BLE_UUID_CHAR_ERROR_DATA, &errorInitData, sizeof(errorInitData), BLE_SERVER::CHAR_READ_ONLY);
  trapData.addCharacteristic(&errorData, CHAR_ERROR_DATA);

  trapData.attachService();
  BLE_SERVER::addService(&trapData, SERVICE_TRAP_DATA);

}


void createDeviceInfoService() {

  BLE_SERVER::Service deviceInfo;
  deviceInfo.createSIG(BLE_UUID_SIG_SERVICE_DEVICE_INFO);

  deviceInfo.attachService();
  BLE_SERVER::addService(&deviceInfo, SERVICE_DEVICE_INFO);
}


void updateEventBLE(EVENTS::event_data_t data)
{
  //uint8_t currentKillNumber = TrapState::getKillNumber();
  CurrentTime::current_time_t currentTime = *CurrentTime::getCurrentTime();
  INFO("current time: %d", currentTime.time);
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DISPLAYED,  &killNumber,               sizeof(killNumber));
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DATA,       getEvent(killNumber),      sizeof(TrapState::event_data_t));
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_CONFIG,     TrapState::getConfig(),    sizeof(TrapState::trap_detector_config_t));
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_TRAP_TIME,        &currentTime,              sizeof(CurrentTime::current_time_t));

}

///////////////////////////////////////////////////
//////     BLE Write handlers     /////////////////
///////////////////////////////////////////////////

void sendRawData(EVENTS::event_data_t data)
{
  uint8_t requestedKill = *(uint8_t*)data.p_data;

  DEBUG("Confirm Requested Kill: %d", requestedKill);

  TrapState::raw_event_data_t requestedRawData[RAW_DATA_CAPTURE_SIZE] = { 0 };
  Flash_Record::read(KILL_RAW_DATA_FILE_ID, requestedKill, &requestedRawData, sizeof(requestedRawData));

  TrapState::raw_event_data_t bleRawData[RAW_DATA_BLE_SIZE] = { 0 };

  uint32_t err_code;
  for (int i = 0; i < RAW_DATA_CAPTURE_SIZE/RAW_DATA_BLE_SIZE; i++)
  {
    memcpy(&bleRawData, &requestedRawData[i*RAW_DATA_BLE_SIZE], sizeof(bleRawData));
    err_code = BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_RAW_DATA, &bleRawData, sizeof(bleRawData));

    ERROR_CHECK(err_code);
    nrf_delay_ms(50);
  }

}

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

  EVENTS::eventPut(SEND_RAW_DATA, &requestedKill, 1);
  ERROR_CHECK(err_code);
  DEBUG("Requested Kill: %d", requestedKill);
}

void trapTimeHandler(uint8_t const* data, uint16_t len)
{
  CurrentTime::current_time_t currentTime;
  memcpy(&currentTime, data, len);
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_TRAP_TIME, &currentTime, sizeof(CurrentTime::current_time_t));
  CurrentTime::setCurrentTime(currentTime);
  DEBUG("Time Set To: %d", currentTime.time);

  mainStateMachine.transition(TIMESET_EVENT);
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
  LIS2DH12_initDAPolling(accReadRawDataHandler);


}



void loadDataFromFlash()
{
  Flash_Record::read(KILL_NUMBER_FILE_ID, KILL_NUMBER_KEY_ID, &killNumber, sizeof(killNumber));

  static uint8_t bootNum = 0;
  Flash_Record::read(CONFIG_FILE_ID, CONFIG_REC_KEY_ID, &bootNum, sizeof(bootNum));
  bootNum++;
  INFO("Bootnum: %d", bootNum);
  if (bootNum > 20)
  {
    while(1) sd_app_evt_wait();
  }
  Flash_Record::write(CONFIG_FILE_ID, CONFIG_REC_KEY_ID, &bootNum, sizeof(bootNum));

  static uint32_t previousError = 0;
  Flash_Record::read(ERROR_FILE_ID, ERROR_KEY_ID, &previousError, sizeof(previousError));
  if (previousError != 0) { INFO("Program has exited with error: %d", previousError); }

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


void idleToActiveTransition()
{
  INFO("Clock Init");
  CurrentTime::startClock();

  INFO("Trap State starting");
  TrapState::initialise();
}


////////////////////////////////////////////
//////////    Error Handling    ////////////
////////////////////////////////////////////

static uint32_t finalErrorCode;
void errorHandler(EVENTS::event_data_t data)
{
  finalErrorCode = *(uint32_t*)data.p_data;
  mainStateMachine.transition(PROGRAM_ERROR_EVENT);
}

void errorShowHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  GPIO::toggle(LED_1_PIN);
}

void errorStateHandler()
{
  TrapState::stop();
  GPIO::initIntInput(BUTTON_1,
                NRF_GPIOTE_POLARITY_HITOLO,
                NRF_GPIO_PIN_PULLUP,
                false,
                false,
                errorShowHandler);
  GPIO::interruptEnable(BUTTON_1);

  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_ERROR_DATA,  &finalErrorCode, sizeof(finalErrorCode));
  Flash_Record::write(ERROR_FILE_ID, ERROR_KEY_ID, &finalErrorCode, sizeof(finalErrorCode));

  for(;;)
  {
    sd_app_evt_wait();
  }
}


void registerEventCallbacks ()
{
  EVENTS::registerEventHandler(TrapState::TRAP_KILLED_EVENT,        stopRawSampling);
  EVENTS::registerEventHandler(TrapState::TRAP_KILLED_EVENT,        onKillEvent);
  EVENTS::registerEventHandler(TrapState::TRAP_STATE_CHANGE_EVENT,  showState);
  EVENTS::registerEventHandler(BLE_SERVER::BLE_CONNECTED_EVENT,     updateEventBLE);
  EVENTS::registerEventHandler(BLE_SERVER::BLE_DISCONNECTED_EVENT,  onBLEDisconnect);
  EVENTS::registerEventHandler(TrapState::TRAP_TRIGGERED_EVENT,     startRawSampling);
  EVENTS::registerEventHandler(TrapState::TRAP_MOVING_EVENT,        stopRawSampling);
  EVENTS::registerEventHandler(RAW_DATA_FULL,                       stopRawSampling);
  EVENTS::registerEventHandler(SEND_RAW_DATA,                       sendRawData);
  EVENTS::registerEventHandler(DEBUG_ERROR_EVENT,                   errorHandler);
}

void createMainTransitionTable()
{
  mainStateMachine.registerTransition(IDLE_STATE,   ACTIVE_STATE, TIMESET_EVENT, &idleToActiveTransition);
  mainStateMachine.registerTransition(ACTIVE_STATE, IGNORED,      TIMESET_EVENT, NULL);
  mainStateMachine.registerTransition(IDLE_STATE,   ERROR_STATE,  PROGRAM_ERROR_EVENT, &errorStateHandler);
  mainStateMachine.registerTransition(ACTIVE_STATE, ERROR_STATE,  PROGRAM_ERROR_EVENT, &errorStateHandler);
  mainStateMachine.registerTransition(ERROR_STATE,  IGNORED,      PROGRAM_ERROR_EVENT, NULL);
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
	createMainTransitionTable();


  INFO("Starting main loop");

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



















