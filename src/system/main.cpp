
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


#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "libraries/events/events.h"


#define NRF_LOG_MODULE_NAME MAIN
NRF_LOG_MODULE_REGISTER();

#define LED_FAST_BLINK 200
#define LED_SLOW_BLINK 1000

#define CONFIG_FILE_ID     (0xF010)
#define CONFIG_REC_KEY_ID  (0x7010)

#define ERROR_FILE_ID   0x5432
#define ERROR_KEY_ID    0x6543
#define TRAP_ID_KEY_ID  0x1212


#define MAIN_EVENT_OFFSET 0x1500


typedef enum {
  IDLE_STATE,
  ACTIVE_STATE,
  ERROR_STATE,
  MAX_STATES,
} main_state_e;



/*
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
*/

enum {
  RAW_DATA_FULL = MAIN_EVENT_OFFSET,
  SEND_RAW_DATA,
  KILL_RECORDING_FINISHED,
  TIMESET_EVENT,
  ACTIVATE_EVENT,
  DEACTIVATE_EVENT,
  PROGRAM_ERROR_EVENT
};


Timer ledTimer;
/*
static trap_info_t g_trapInfo = {
    TRAP_ID,                       // Trap ID
    IDLE_STATE,
    TrapState::WAIT_STATE
};

static uint8_t                       killNumber = 0;
static uint32_t                      killNumber_32 = 0;
static TrapState::event_data_t       recordData = { 0 };
static TrapState::event_data_t       eventData = { 0 };
static uint32_t                      g_trapID   = 0;


static StateMachine mainStateMachine;
*/
///////////////////////////////////////////////////
//////////        Event handlers        ///////////
///////////////////////////////////////////////////
/*
void processRawData();

void recordEventData()
{
  Flash_Record::write(KILL_NUMBER_FILE_ID,    KILL_NUMBER_KEY_ID, &killNumber_32,          sizeof(killNumber_32));
  Flash_Record::write(KILL_DATA_FILE_ID,      killNumber,         &eventData,           sizeof(eventData));
  Flash_Record::write(KILL_RAW_DATA_FILE_ID,  killNumber,         &g_accelerationData,  sizeof(g_accelerationData));
}

void resetEventData(EVENTS::event_data_t data)
{
  g_accDataCount = 0;
  memset(&eventData, 0, sizeof(eventData));
  memset(&g_accelerationData, 0, sizeof(g_accelerationData));
}

void onKillEvent(EVENTS::event_data_t data)
{
  INFO("Killed");

  killNumber++;

  int32_t temp;
  LIS2DH12::updateTemperatureSensor();
  LIS2DH12::getTemperature(&temp);

  processRawData();

  eventData.timestamp   =     *CurrentTime::getCurrentTime();
  eventData.trap_id     =     TRAP_ID;
  eventData.temperature =     static_cast<int8_t>(temp);
  eventData.killNumber  =     static_cast<uint8_t>(killNumber);

  INFO("SETTING - Kill timestamp - \t%d", eventData.timestamp.time);
  INFO("SETTING - Kill trap id - \t\t%d", eventData.trap_id);
  INFO("SETTING - Kill temperature - \t%d", eventData.temperature);
  INFO("SETTING - Kill Number - \t\t%d", eventData.killNumber);
  INFO("SETTING - Kill Peak Value - \t%d", eventData.peak_level);

  recordEventData();
  EVENTS::eventPut(KILL_RECORDING_FINISHED);
}
*/
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

/*
void showState()
{

  TrapState::detector_state_e currentState = TrapState::getState();
  switch (currentState)
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
  g_trapInfo.detectorState = currentState;
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_TRAP_INFO, &g_trapInfo, sizeof(g_trapInfo));
}


TrapState::event_data_t* getEvent(uint8_t eventID)
{
  Flash_Record::read(KILL_DATA_FILE_ID, eventID, &recordData, sizeof(recordData));
  return &recordData;
}
*/
///////////////////////////////////////////////////
//////        BLE functions             ///////////
///////////////////////////////////////////////////




void createDeviceInfoService() {

  BLE_SERVER::Service deviceInfo;
  deviceInfo.createSIG(BLE_UUID_SIG_SERVICE_DEVICE_INFO);

  deviceInfo.attachService();
  BLE_SERVER::addService(&deviceInfo, SERVICE_DEVICE_INFO);
}


void updateEventBLE(EVENTS::event_data_t data)
{
  /*
  //uint8_t currentKillNumber = TrapState::getKillNumber();
  INFO("UPDATING - Event BLE");
  TrapState::trap_detector_config_t currentConfig = *TrapState::getConfig();
  CurrentTime::current_time_t currentTime         = *CurrentTime::getCurrentTime();

  INFO("UPDATING - Kill Number");
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DISPLAYED,  &killNumber,               sizeof(killNumber));

  INFO("UPDATING - Current Event");
  //BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DATA,       getEvent(killNumber),      sizeof(TrapState::event_data_t));

  INFO("UPDATING - Event Config");
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_CONFIG,     &currentConfig,            sizeof(TrapState::trap_detector_config_t));

  INFO("UPDATING - Current Time");
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_TRAP_TIME,        &currentTime,              sizeof(CurrentTime::current_time_t));

  INFO("UPDATING - Trap Info");
  g_trapInfo.trapID = g_trapID;
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_TRAP_INFO,        &g_trapInfo,                sizeof(g_trapInfo));
*/
}

///////////////////////////////////////////////////
//////     BLE Write handlers     /////////////////
///////////////////////////////////////////////////

/*


void eventConfigHandler(EVENTS::event_data_t data)
{
  TrapState::trap_detector_config_t config = *(TrapState::trap_detector_config_t*)data.p_data;;
  //memcpy(&config, data, len);
  TrapState::setConfig(config);

  INFO("CONFIGURING - event config");

  uint32_t err_code = BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_CONFIG, &config, sizeof(TrapState::trap_detector_config_t));
  ERROR_CHECK(err_code);

}


void eventDisplayedHandler(EVENTS::event_data_t data)
{
  uint8_t requestedKill = *(uint8_t*)data.p_data;;
  //memcpy(&requestedKill, data, len);

  if (requestedKill > killNumber)
  {
    INFO("Requested Kill too high: %d", requestedKill);
    return;
  }

  TrapState::event_data_t tmpData = *getEvent(requestedKill);

  uint32_t err_code = BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DATA, &tmpData, sizeof(TrapState::event_data_t));

  EVENTS::eventPut(SEND_RAW_DATA, &requestedKill, 1);
  ERROR_CHECK(err_code);
  INFO("Requested Kill: %d", requestedKill);
}

void trapTimeHandler(EVENTS::event_data_t data)
{
  CurrentTime::current_time_t currentTime = *(CurrentTime::current_time_t*)data.p_data;;

  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_TRAP_TIME, &currentTime, sizeof(CurrentTime::current_time_t));
  CurrentTime::setCurrentTime(currentTime);
  INFO("Time Set To: %d", currentTime.time);

  EVENTS::eventPut(TIMESET_EVENT);
}


void trapControlHandler(EVENTS::event_data_t data)
{
  trap_control_t command = *(trap_control_t*)data.p_data;;

  if (1 == command.activate)
  {
    INFO("Activating Trap!");
    EVENTS::eventPut(ACTIVATE_EVENT);
    //mainStateMachine.transition(ACTIVATE_EVENT);
  }
  else if (2 == command.activate)
  {
    INFO("De-activating Trap.");
    EVENTS::eventPut(DEACTIVATE_EVENT);
    //mainStateMachine.transition(DEACTIVATE_EVENT);
  }
  if (0 != command.trapID)
  {
    INFO("Setting Trap ID to: %d", command.trapID);
    g_trapID = command.trapID;
    g_trapInfo.trapID = g_trapID;
    BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_TRAP_INFO, &g_trapInfo, sizeof(g_trapInfo));
    Flash_Record::write(CONFIG_FILE_ID,    TRAP_ID_KEY_ID, &g_trapID,    sizeof(g_trapID));
  }


}
*/

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
  // Find the number of times the program has booted
  static uint8_t bootNum = 0;
  Flash_Record::read(CONFIG_FILE_ID, CONFIG_REC_KEY_ID, &bootNum, sizeof(bootNum));
  bootNum++;
  INFO("READING - Bootnumber - Value: %d", bootNum);
  // If the number of times the program has booted is greater than 20 something has probably gone wrong, so busy wait to save power

  if (bootNum > 20)
  {
    while(1) sd_app_evt_wait();
  }

  Flash_Record::write(CONFIG_FILE_ID, CONFIG_REC_KEY_ID, &bootNum, sizeof(bootNum));

  // If the previous program exited with an error, find it here.
  static uint32_t previousError = 0;
  Flash_Record::read(ERROR_FILE_ID, ERROR_KEY_ID, &previousError, sizeof(previousError));
  if (previousError != 0) { INFO("READING - Previous error - Value: %d", previousError); }

  // Loads the number of kills into program memory
  Flash_Record::read(KILL_NUMBER_FILE_ID, KILL_NUMBER_KEY_ID, &killNumber, sizeof(killNumber));
  INFO("READING - Kill number - Value: %d", killNumber);

  // Loads the number of kills into program memory
  Flash_Record::read(CONFIG_FILE_ID, TRAP_ID_KEY_ID, &g_trapID, sizeof(g_trapID));
  INFO("READING - Trap ID - Value: %d", g_trapID);
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
  //createTrapDataService();
  createDeviceInfoService();

//#ifndef DEBUG
  //BLE_DFU::createDFUService();
//#endif

  BLE_ADVERTISING::start(BLE_ADVERTISING_SPEED_SLOW);
  BLE_ADVERTISING::advertiseName();
  BLE_ADVERTISING::advertiseUUID(BLE_SERVER::getService(SERVICE_TRAP_DATA)->getUUID());

  /*
  BLE_SERVER::setWriteHandler(SERVICE_TRAP_DATA, CHAR_EVENT_CONFIG,      eventConfigHandler);
  BLE_SERVER::setWriteHandler(SERVICE_TRAP_DATA, CHAR_EVENT_DISPLAYED,   eventDisplayedHandler);
  BLE_SERVER::setWriteHandler(SERVICE_TRAP_DATA, CHAR_TRAP_TIME,         trapTimeHandler);
  BLE_SERVER::setWriteHandler(SERVICE_TRAP_DATA, CHAR_TRAP_CONTROL,      trapControlHandler);
   */
}

/*
void idleToActiveTransition()
{
  INFO("INITIALISING - Trap State Machine");
  TrapState::initialise();
  g_trapInfo.mainState = ACTIVE_STATE;
  //BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_TRAP_INFO, &g_trapInfo, sizeof(g_trapInfo));
}

void activeToIdleTransition()
{
  INFO("STOPPING - Trap State Machine");
  g_trapInfo.mainState = IDLE_STATE;
  //BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_TRAP_INFO, &g_trapInfo, sizeof(g_trapInfo));
}

*/
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
  /*
  //EVENTS::registerEventHandler(TrapState::TRAP_KILLED_EVENT,        stopRawSampling);
  //EVENTS::registerEventHandler(TrapState::TRAP_KILLED_EVENT,        onKillEvent);
  //EVENTS::registerEventHandler(TrapState::TRAP_STATE_CHANGE_EVENT,  showState);
  EVENTS::registerEventHandler(BLE_SERVER::BLE_CONNECTED_EVENT,     updateEventBLE);
  EVENTS::registerEventHandler(BLE_SERVER::BLE_CONNECTED_EVENT,     onBLEConnect);
  EVENTS::registerEventHandler(BLE_SERVER::BLE_DISCONNECTED_EVENT,  onBLEDisconnect);
  //EVENTS::registerEventHandler(TrapState::TRAP_TRIGGERED_EVENT,     startRawSampling);
  //EVENTS::registerEventHandler(TrapState::TRAP_MOVING_EVENT,        stopRawSampling);
  //EVENTS::registerEventHandler(RAW_DATA_FULL,                       stopRawSampling);
  //EVENTS::registerEventHandler(SEND_RAW_DATA,                       sendRawData);
  EVENTS::registerEventHandler(DEBUG_ERROR_EVENT,                   errorHandler);
  EVENTS::registerEventHandler(KILL_RECORDING_FINISHED,             updateEventBLE);
  EVENTS::registerEventHandler(KILL_RECORDING_FINISHED,             setBLEOutputHigh);
  //EVENTS::registerEventHandler(KILL_RECORDING_FINISHED,             resetEventData);
  EVENTS::registerEventHandler(BLE_SERVER::BLE_CONNECTED_EVENT,     setBLEOutputHigh);
  EVENTS::registerEventHandler(TIMESET_EVENT,                       CurrentTime::startClock);
  EVENTS::registerEventHandler(TrapState::TRAP_TRIGGERED_EVENT,     LIS2DH12::clearInterrupts);
  */
}

/*
void createMainTransitionTable()
{

  mainStateMachine.registerTransition(IDLE_STATE,   ACTIVE_STATE, ACTIVATE_EVENT,       &idleToActiveTransition);
  mainStateMachine.registerTransition(ACTIVE_STATE, IDLE_STATE,   DEACTIVATE_EVENT,     &activeToIdleTransition);
  mainStateMachine.registerTransition(IDLE_STATE,   ERROR_STATE,  PROGRAM_ERROR_EVENT,  &errorStateHandler);
  mainStateMachine.registerTransition(ACTIVE_STATE, ERROR_STATE,  PROGRAM_ERROR_EVENT,  &errorStateHandler);

}

*/
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

#ifdef DEBUG
	GPIO::low(LED_1_PIN);
#endif

	//loadDataFromFlash();
	registerEventCallbacks();
	setButtonInterrupt();

	TIME::init();
	DETECTOR::init();
	RAW_EVENT::init();

	//sensors_init();
	startBLE();
	//createMainTransitionTable();

	//mainStateMachine.start(IDLE_STATE);

	//GPIO::setOutput(LED_1_PIN, LOW);

	//mainStateMachine.transition(TIMESET_EVENT);

	//EVENTS::eventPut(ACTIVATE_EVENT);


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



















