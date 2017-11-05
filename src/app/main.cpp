
/*
 * main.cpp
 *
 *  Created on: September 5, 2017
 *      Author: Michael McAdam
 */

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
#include "ble/ble_manager.h"
#include "debug/DEBUG.h"
#include "peripheral/timer_interface.h"
#include "peripheral/adc_interface.h"
#include "peripheral/gpio_interface.h"
#include "app/current_time.h"
#include "app/trap_event.h"

#include "peripheral/flash_interface.h"

#include "app/trap_manager.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define CONFIG_FILE     (0xF010)
#define CONFIG_REC_KEY  (0x7010)


#define LED_FAST_BLINK 200
#define LED_SLOW_BLINK 1000




uint8_t g_killNumber = 0;
uint32_t g_killTime = 0;




static bool updateBLE = true;
volatile bool stateChange = false;

Timer ledTimer;

//TrapEvent trapEvent;






///////////////////////////////////////////////////
//////        Interrupt handlers        ///////////
///////////////////////////////////////////////////





void buttonHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  /*
  if (stateMachine.isRunning()) { stateMachine.stop(); }
  else { stateMachine.start(WAIT_STATE); }
  //LIS2DH12_setInterruptThreshold(TRAP_TRIGGER_MOVE_THRESHOLD);
   *
   */
  //updateBLE = true;
  DEBUG("Button pressed");

  TRAP_EVENT_DETECTOR::simulateTrigger();

}



///////////////////////////////////////////////////
//////        BLE functions             ///////////
///////////////////////////////////////////////////

void updateEventBLE()
{
  uint8_t killNumber = g_killNumber;
  BLE_Manager::manager().setCharacteristic(SERVICE_DETECTOR_DATA, CHAR_DETECTOR_NUMBER_OF_KILLS, &killNumber, sizeof(killNumber));

  uint8_t* killTime = bit32Converter(g_killTime);
  BLE_Manager::manager().setCharacteristic(SERVICE_DETECTOR_DATA, CHAR_DETECTOR_KILL_TIME, killTime, sizeof(g_killTime));


  /*
  uint8_t* triggerThreshold = bit16Converter(TRAP_EVENT_DETECTOR::triggerThreshold);
  BLE_Manager::manager().setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_TRIGGER_THRESHOLD, triggerThreshold, sizeof(TRAP_EVENT_DETECTOR::triggerThreshold));

  uint8_t* moveThreshold = bit16Converter(TRAP_EVENT_DETECTOR::moveThreshold);
  BLE_Manager::manager().setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_MOVE_THRESHOLD, moveThreshold, sizeof(TRAP_EVENT_DETECTOR::moveThreshold));

  BLE_Manager::manager().setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_TRIGGER_DURATION, &TRAP_EVENT_DETECTOR::triggerDuration, sizeof(TRAP_EVENT_DETECTOR::triggerDuration));

  BLE_Manager::manager().setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_MOVE_DURATION, &TRAP_EVENT_DETECTOR::moveDuration, sizeof(TRAP_EVENT_DETECTOR::moveDuration));

  uint8_t* triggerBufferLength = bit16Converter(TRAP_EVENT_DETECTOR::triggerBufferLength);
  BLE_Manager::manager().setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_TRIGGER_BUFFER_LENGTH, triggerBufferLength, sizeof(TRAP_EVENT_DETECTOR::triggerBufferLength));

  uint8_t* moveBufferLength = bit16Converter(TRAP_EVENT_DETECTOR::moveBufferLength);
  BLE_Manager::manager().setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_MOVE_BUFFER_LENGTH, moveBufferLength, sizeof(TRAP_EVENT_DETECTOR::moveBufferLength));

  uint8_t* setBufferLength = bit16Converter(TRAP_EVENT_DETECTOR::setBufferLength);
  BLE_Manager::manager().setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_SET_BUFFER_LENGTH, setBufferLength, sizeof(TRAP_EVENT_DETECTOR::setBufferLength));
*/


/*
  uint8_t didClip = curEvent.getDidClip();
  BLE_Manager::manager().setCharacteristic(SERVICE_DETECTOR_DATA, CHAR_DETECTOR_DID_CLIP, &didClip, sizeof(didClip));

  uint16_t peakValue = curEvent.getPeakValue();
  BLE_Manager::manager().setCharacteristic(SERVICE_DETECTOR_DATA, CHAR_DETECTOR_PEAK_VALUE, bit16Converter(peakValue), sizeof(peakValue));

  uint16_t responseSize = curEvent.getResponseSize();
  BLE_Manager::manager().setCharacteristic(SERVICE_DETECTOR_DATA, CHAR_DETECTOR_RESPONSE_SIZE, bit16Converter(responseSize), sizeof(responseSize));

  uint32_t responseLength = curEvent.getResponseLength();
  BLE_Manager::manager().setCharacteristic(SERVICE_DETECTOR_DATA, CHAR_DETECTOR_RESPONSE_LENGTH, bit32Converter(responseLength), sizeof(responseLength));
*/
}


void bleConnectHandler()
{
  DEBUG("Connected at time: %d", CurrentTime::getCurrentTime());
  uint8_t* currentTime = bit32Converter(CurrentTime::getCurrentTime());
  BLE_Manager::manager().setCharacteristic(SERVICE_CURRENT_TIME, CHAR_TIME_IN_MINS, currentTime, 4);

}

void bleDisconnectHandler()
{
  DEBUG("Where did it go?");
}

///////////////////////////////////////////////////
//////     Initialisation functions      ///////////
///////////////////////////////////////////////////

/*
void triggerThresholdHandler(uint8_t const* data, uint16_t len)
{
  TRAP_EVENT_DETECTOR::triggerThreshold = data[0] + (data[1] << 8);
  //LIS2DH12_setInterruptThreshold(TRAP_EVENT_DETECTOR::triggerThreshold);
  DEBUG("Trigger Threshold: %d", TRAP_EVENT_DETECTOR::triggerThreshold);
  updateBLE = true;
}

void moveThresholdHandler(uint8_t const* data, uint16_t len)
{
  TRAP_EVENT_DETECTOR::moveThreshold = data[0] + (data[1] << 8);
  DEBUG("Move Threshold: %d", TRAP_EVENT_DETECTOR::moveThreshold);
  updateBLE = true;
}

void triggerDurationHandler(uint8_t const* data, uint16_t len)
{
  TRAP_EVENT_DETECTOR::triggerDuration = data[0];
  DEBUG("Trigger Duration: %d", TRAP_EVENT_DETECTOR::triggerDuration);
  updateBLE = true;
}

void moveDurationHandler(uint8_t const* data, uint16_t len)
{
  TRAP_EVENT_DETECTOR::moveDuration = data[0];
  DEBUG("Move Duration: %d", TRAP_EVENT_DETECTOR::moveDuration);
  updateBLE = true;
}

void triggerBufferLengthHandler(uint8_t const* data, uint16_t len)
{
  TRAP_EVENT_DETECTOR::triggerBufferLength = data[0] + (data[1] << 8);
  DEBUG("Trigger Buffer Length: %d", TRAP_EVENT_DETECTOR::triggerBufferLength);
  updateBLE = true;
}

void moveBufferLengthHandler(uint8_t const* data, uint16_t len)
{
  TRAP_EVENT_DETECTOR::moveBufferLength = data[0] + (data[1] << 8);
  DEBUG("Move Buffer Length: %d", TRAP_EVENT_DETECTOR::moveBufferLength);
  updateBLE = true;
}

void setBufferLengthHandler(uint8_t const* data, uint16_t len)
{
  TRAP_EVENT_DETECTOR::setBufferLength = data[0] + (data[1] << 8);
  DEBUG("Set Buffer Length: %d", TRAP_EVENT_DETECTOR::setBufferLength);
  updateBLE = true;
}

void outputRawHandler(uint8_t const* data, uint16_t len)
{
  //BLE_Manager::manager().setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_OUTPUT_RAW, data, len);
  //if (data[0] == 1) { LIS2DH12_startDAPolling(); }
  //if (data[0] == 0) { LIS2DH12_stopDAPolling(); }
  stateMachine.stop();
  stateMachine.start(WAIT_STATE);
  updateBLE = true;
  showState();
}
*/

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
  BLE_Manager::manager().createBLEServer();

  BLE_Manager::manager().setPower(BLE_POWER_LEVEL_HIGH);
  BLE_Manager::manager().setConnectionHandler(bleConnectHandler);
  BLE_Manager::manager().setDisconnectHandler(bleDisconnectHandler);
/*
  BLE_Manager::manager().setWriteHandler(SERVICE_DEVICE_CONTROL, CHAR_TRIGGER_THRESHOLD,      triggerThresholdHandler);
  BLE_Manager::manager().setWriteHandler(SERVICE_DEVICE_CONTROL, CHAR_MOVE_THRESHOLD,         moveThresholdHandler);
  BLE_Manager::manager().setWriteHandler(SERVICE_DEVICE_CONTROL, CHAR_TRIGGER_DURATION,       triggerDurationHandler);
  BLE_Manager::manager().setWriteHandler(SERVICE_DEVICE_CONTROL, CHAR_MOVE_DURATION,          moveDurationHandler);
  BLE_Manager::manager().setWriteHandler(SERVICE_DEVICE_CONTROL, CHAR_TRIGGER_BUFFER_LENGTH,  triggerBufferLengthHandler);
  BLE_Manager::manager().setWriteHandler(SERVICE_DEVICE_CONTROL, CHAR_MOVE_BUFFER_LENGTH,     moveBufferLengthHandler);
  BLE_Manager::manager().setWriteHandler(SERVICE_DEVICE_CONTROL, CHAR_SET_BUFFER_LENGTH,      setBufferLengthHandler);
  BLE_Manager::manager().setWriteHandler(SERVICE_DEVICE_CONTROL, CHAR_OUTPUT_RAW,             outputRawHandler);
*/
}

void initTimer()
{
  Timer::initialisePeripheral();
  CurrentTime::startClock();
}



void blinkHandler(void*)
{
  GPIO::toggle(LED_2_PIN);
}

void ledToggle(void*)
{
  GPIO::toggle(LED_1_PIN);
}


void showState()
{

  switch (TRAP_EVENT_DETECTOR::getState())
  {
    case TRAP_EVENT_DETECTOR::WAIT_STATE:
      ledTimer.stopTimer();
      GPIO::setOutput(LED_2_PIN, HIGH);
      break;

    case TRAP_EVENT_DETECTOR::EVENT_BUFFER_STATE:
      ledTimer.stopTimer();
      GPIO::setOutput(LED_2_PIN, LOW);
      break;

    case TRAP_EVENT_DETECTOR::DETECT_MOVE_STATE:
      ledTimer.stopTimer();
      ledTimer.startTimer(LED_FAST_BLINK, ledToggle);
      break;

    case TRAP_EVENT_DETECTOR::MOVING_STATE:
      ledTimer.stopTimer();
      ledTimer.startTimer(LED_SLOW_BLINK, ledToggle);
      break;

    default:
      break;
  }
  //BLE_Manager::manager().setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_OUTPUT_STATE, &currentState, sizeof(currentState));
}



///////////////////////////////////////////////////
//////        Main                      ///////////
///////////////////////////////////////////////////


int main(void)
{
	DEBUG_INIT();
  INFO("Started");

  INFO("Log is working");

  //createTransitionTable();

  initTimer();
  initGPIO();
  initBLE();
  //initSensors();

  //TRAP_EVENT_DETECTOR::initialise();

  Timer blinkTimer;
  blinkTimer.startTimer(1000, blinkHandler);

  //LIS2DH12_startDAPolling();

  FDS::init();
  FDS::status();
  FDS::clean();


  uint16_t data[] = { 12 };
  Flash_Record::read(CONFIG_FILE, CONFIG_REC_KEY+1, data, sizeof(data[0]));
  NRF_LOG_INFO("Data: %d", data[0]);
  data[0] = data[0] * 2;
  Flash_Record::write(CONFIG_FILE, CONFIG_REC_KEY+1, data, sizeof(data[0]));

  while(true)
  {

    if (!NRF_LOG_PROCESS())
    {
      uint32_t err_code = sd_app_evt_wait();
      ERROR_CHECK(err_code);
    }


    if (updateBLE) {
      //updateEventBLE();
      updateBLE = false;
    }

    if (stateChange) {
      //showState();
      stateChange = false;
    }

    //DEBUG("State: %d", stateMachine.getCurrentState());

    //GPIO::toggle(LED_1_PIN);
    //nrf_delay_ms(1000);

    //NRF_LOG_FLUSH();

  }

}


/**
 *@}
 **/



















