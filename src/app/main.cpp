
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
#include "peripheral/LIS2DH12.h"

#include "peripheral/flash_interface.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define CONFIG_FILE     (0xF010)
#define CONFIG_REC_KEY  (0x7010)


#define TRAP_EVENT_BUFFER_MS      2000
#define MOVE_BUFFER_MS            5000
#define SET_BUFFER_MS             10000

#define TRAP_TRIGGER_EVENT_THRESHOLD      500
#define TRAP_TRIGGER_MOVE_THRESHOLD       100

#define TRAP_TRIGGER_DURATION       40

#define LED_FAST_BLINK 200
#define LED_SLOW_BLINK 1000


static StateMachine stateMachine(WAIT_STATE);

uint8_t g_killNumber = 0;
uint32_t g_killTime = 0;

uint16_t g_triggerThreshold = TRAP_TRIGGER_EVENT_THRESHOLD;
uint16_t g_moveThreshold = TRAP_TRIGGER_MOVE_THRESHOLD;
uint8_t g_triggerDuration = TRAP_TRIGGER_DURATION;
uint8_t g_moveDuration = 0;
uint16_t g_triggerBufferLength = TRAP_EVENT_BUFFER_MS;
uint16_t g_moveBufferLength = MOVE_BUFFER_MS;
uint16_t g_setBufferLength = SET_BUFFER_MS;



static bool updateBLE = true;
volatile bool stateChange = false;
Timer sampleTimer;
Timer movementCountdown;
Timer trapBufferCountdown;
Timer moveBufferCountdown;
Timer ledTimer;



///////////////////////////////////////////////////
//////           Timer handlers         ///////////
///////////////////////////////////////////////////


void accReadTimerHandler(void* p_context)
{
  LIS2DH12_sample();

  static int32_t accX, accY, accZ = 0;
  LIS2DH12_getALLmG(&accX, &accY, &accZ);
  DEBUG("X: %d, Y: %d, Z: %d", accX, accY, accZ);
}

void ledToggle(void* p_context)
{
  GPIO::toggle(LED_2_PIN);
}

void trapBufferCountdownHandler(void* p_context)
{
  stateMachine.transition(BUFFER_END_EVENT);
}

void moveBufferCountdownHandler(void* p_context)
{
  stateMachine.transition(MOVEMENT_BUFFER_END_EVENT);
}

void movementCountdownHandler(void* p_context)
{
  stateMachine.transition(SET_BUFFER_END_EVENT);
}


///////////////////////////////////////////////////
//////        Transition functions      ///////////
///////////////////////////////////////////////////


void showState()
{
  uint8_t currentState = stateMachine.getCurrentState();
  switch (currentState)
  {
    case WAIT_STATE:
      ledTimer.stopTimer();
      GPIO::setOutput(LED_2_PIN, HIGH);
      break;

    case EVENT_BUFFER_STATE:
      ledTimer.stopTimer();
      GPIO::setOutput(LED_2_PIN, LOW);
      break;

    case DETECT_MOVE_STATE:
      ledTimer.stopTimer();
      ledTimer.startTimer(LED_FAST_BLINK, ledToggle);
      break;

    case MOVING_STATE:
      ledTimer.stopTimer();
      ledTimer.startTimer(LED_SLOW_BLINK, ledToggle);
      break;

    default:
      break;
  }
  BLE_Manager::manager().setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_OUTPUT_STATE, &currentState, sizeof(currentState));
}


void triggeredFromWaitTransition()
{
  //sampleTimer.startTimer(100, accReadTimerHandler);
  //LIS2DH12_startDAPolling();
  INFO("Triggerd from wait");
  trapBufferCountdown.startCountdown(g_triggerBufferLength, trapBufferCountdownHandler);
  stateChange = true;
}

void trapBufferEndedTransition()
{
  //LIS2DH12_stopDAPolling();
  INFO("Trap buffer end");

  moveBufferCountdown.startCountdown(g_moveBufferLength, moveBufferCountdownHandler);
  LIS2DH12_setInterruptThreshold(g_moveThreshold);

  stateChange = true;
}

void moveBufferEndedTransition()
{
  INFO("Killed");
  g_killNumber++;
  g_killTime = CurrentTime::getCurrentTime();
  updateBLE = true;
  LIS2DH12_setInterruptThreshold(g_triggerThreshold);

  stateChange = true;
}


void triggeredFromMoveTransition()
{
  INFO("Triggerd from move");
  movementCountdown.stopTimer();
  movementCountdown.startCountdown(g_setBufferLength, movementCountdownHandler);

  stateChange = true;
}


void moveToWaitTransition()
{
  INFO("Trap set");
  LIS2DH12_setInterruptThreshold(g_triggerThreshold);

  stateChange = true;
}



///////////////////////////////////////////////////
//////        Interrupt handlers        ///////////
///////////////////////////////////////////////////

void accTriggeredHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  LIS2DH12_clearInterrupts();
  stateMachine.transition(TRIGGERED_EVENT);
  INFO("accTriggered");
}



void buttonHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  /*
  if (stateMachine.isRunning()) { stateMachine.stop(); }
  else { stateMachine.start(WAIT_STATE); }
  //LIS2DH12_setInterruptThreshold(TRAP_TRIGGER_MOVE_THRESHOLD);
   *
   */
  updateBLE = true;
  DEBUG("Button pressed");
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

  uint8_t* triggerThreshold = bit16Converter(g_triggerThreshold);
  BLE_Manager::manager().setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_TRIGGER_THRESHOLD, triggerThreshold, sizeof(g_triggerThreshold));

  uint8_t* moveThreshold = bit16Converter(g_moveThreshold);
  BLE_Manager::manager().setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_MOVE_THRESHOLD, moveThreshold, sizeof(g_moveThreshold));

  BLE_Manager::manager().setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_TRIGGER_DURATION, &g_triggerDuration, sizeof(g_triggerDuration));

  BLE_Manager::manager().setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_MOVE_DURATION, &g_moveDuration, sizeof(g_moveDuration));

  uint8_t* triggerBufferLength = bit16Converter(g_triggerBufferLength);
  BLE_Manager::manager().setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_TRIGGER_BUFFER_LENGTH, triggerBufferLength, sizeof(g_triggerBufferLength));

  uint8_t* moveBufferLength = bit16Converter(g_moveBufferLength);
  BLE_Manager::manager().setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_MOVE_BUFFER_LENGTH, moveBufferLength, sizeof(g_moveBufferLength));

  uint8_t* setBufferLength = bit16Converter(g_setBufferLength);
  BLE_Manager::manager().setCharacteristic(SERVICE_DEVICE_CONTROL, CHAR_SET_BUFFER_LENGTH, setBufferLength, sizeof(g_setBufferLength));



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


void triggerThresholdHandler(uint8_t const* data, uint16_t len)
{
  g_triggerThreshold = data[0] + (data[1] << 8);
  //LIS2DH12_setInterruptThreshold(g_triggerThreshold);
  DEBUG("Trigger Threshold: %d", g_triggerThreshold);
  updateBLE = true;
}

void moveThresholdHandler(uint8_t const* data, uint16_t len)
{
  g_moveThreshold = data[0] + (data[1] << 8);
  DEBUG("Move Threshold: %d", g_moveThreshold);
  updateBLE = true;
}

void triggerDurationHandler(uint8_t const* data, uint16_t len)
{
  g_triggerDuration = data[0];
  DEBUG("Trigger Duration: %d", g_triggerDuration);
  updateBLE = true;
}

void moveDurationHandler(uint8_t const* data, uint16_t len)
{
  g_moveDuration = data[0];
  DEBUG("Move Duration: %d", g_moveDuration);
  updateBLE = true;
}

void triggerBufferLengthHandler(uint8_t const* data, uint16_t len)
{
  g_triggerBufferLength = data[0] + (data[1] << 8);
  DEBUG("Trigger Buffer Length: %d", g_triggerBufferLength);
  updateBLE = true;
}

void moveBufferLengthHandler(uint8_t const* data, uint16_t len)
{
  g_moveBufferLength = data[0] + (data[1] << 8);
  DEBUG("Move Buffer Length: %d", g_moveBufferLength);
  updateBLE = true;
}

void setBufferLengthHandler(uint8_t const* data, uint16_t len)
{
  g_setBufferLength = data[0] + (data[1] << 8);
  DEBUG("Set Buffer Length: %d", g_setBufferLength);
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

void initSensors()
{
  LIS2DH12_init(LIS2DH12_POWER_LOW, LIS2DH12_SCALE2G, LIS2DH12_SAMPLE_50HZ);
  LIS2DH12_enableHighPass();
  LIS2DH12_initThresholdInterrupt(g_triggerThreshold, g_triggerDuration, LIS2DH12_INTERRUPT_THRESHOLD_XYZ, true, accTriggeredHandler);
  LIS2DH12_initDAPolling(accReadTimerHandler);
}

void initBLE()
{
  BLE_Manager::manager().createBLEServer();

  BLE_Manager::manager().setPower(BLE_POWER_LEVEL_HIGH);
  BLE_Manager::manager().setConnectionHandler(bleConnectHandler);
  BLE_Manager::manager().setDisconnectHandler(bleDisconnectHandler);

  BLE_Manager::manager().setWriteHandler(SERVICE_DEVICE_CONTROL, CHAR_TRIGGER_THRESHOLD,      triggerThresholdHandler);
  BLE_Manager::manager().setWriteHandler(SERVICE_DEVICE_CONTROL, CHAR_MOVE_THRESHOLD,         moveThresholdHandler);
  BLE_Manager::manager().setWriteHandler(SERVICE_DEVICE_CONTROL, CHAR_TRIGGER_DURATION,       triggerDurationHandler);
  BLE_Manager::manager().setWriteHandler(SERVICE_DEVICE_CONTROL, CHAR_MOVE_DURATION,          moveDurationHandler);
  BLE_Manager::manager().setWriteHandler(SERVICE_DEVICE_CONTROL, CHAR_TRIGGER_BUFFER_LENGTH,  triggerBufferLengthHandler);
  BLE_Manager::manager().setWriteHandler(SERVICE_DEVICE_CONTROL, CHAR_MOVE_BUFFER_LENGTH,     moveBufferLengthHandler);
  BLE_Manager::manager().setWriteHandler(SERVICE_DEVICE_CONTROL, CHAR_SET_BUFFER_LENGTH,      setBufferLengthHandler);
  BLE_Manager::manager().setWriteHandler(SERVICE_DEVICE_CONTROL, CHAR_OUTPUT_RAW,             outputRawHandler);

}

void initTimer()
{
  Timer::initialisePeripheral();
  CurrentTime::startClock();
}

void createTransitionTable(void)
{
  /*                              Start state,        End state,          Triggered by,               Transition handler */
  stateMachine.registerTransition(WAIT_STATE,         EVENT_BUFFER_STATE, TRIGGERED_EVENT,           &triggeredFromWaitTransition);
  stateMachine.registerTransition(EVENT_BUFFER_STATE, IGNORED,            TRIGGERED_EVENT,           NULL);
  stateMachine.registerTransition(EVENT_BUFFER_STATE, DETECT_MOVE_STATE,  BUFFER_END_EVENT,          &trapBufferEndedTransition);
  stateMachine.registerTransition(DETECT_MOVE_STATE,  WAIT_STATE,         MOVEMENT_BUFFER_END_EVENT, &moveBufferEndedTransition);
  stateMachine.registerTransition(DETECT_MOVE_STATE,  MOVING_STATE,       TRIGGERED_EVENT,           &triggeredFromMoveTransition);
  stateMachine.registerTransition(MOVING_STATE,       MOVING_STATE,       TRIGGERED_EVENT,           &triggeredFromMoveTransition);
  stateMachine.registerTransition(MOVING_STATE,       WAIT_STATE,         SET_BUFFER_END_EVENT,      &moveToWaitTransition);
  stateMachine.registerTransition(MOVING_STATE,       IGNORED,            MOVEMENT_BUFFER_END_EVENT, NULL);

}


void blinkHandler(void*)
{
  GPIO::toggle(LED_2_PIN);
}

///////////////////////////////////////////////////
//////        Main                      ///////////
///////////////////////////////////////////////////


int main(void)
{
	DEBUG_INIT();
  INFO("Started");

  INFO("Log is working");

  createTransitionTable();

  initTimer();
  initGPIO();
  initBLE();
  initSensors();

  Timer blinkTimer;
  blinkTimer.startTimer(1000, blinkHandler);

  //LIS2DH12_startDAPolling();


  uint16_t data[] = { 12 };
  Flash_Record dataRecord(CONFIG_FILE, CONFIG_REC_KEY+1, data, sizeof(data[0]));
  dataRecord.read();
  NRF_LOG_INFO("Data: %d", data[0]);
  data[0] = data[0] * 2;
  dataRecord.update();

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

    //DEBUG("State: %d", stateMachine.getCurrentState());

    //GPIO::toggle(LED_1_PIN);
    //nrf_delay_ms(1000);

    //NRF_LOG_FLUSH();

  }

}


/**
 *@}
 **/



















