
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
#include "boards.h"
#include "nrf_drv_gpiote.h"

#include "app/state_machine.h"
#include "ble/ble_manager.h"
#include "ble/ble_interface.h"
#include "debug/DEBUG.h"
#include "peripheral/timer_interface.h"
#include "peripheral/adc_interface.h"
#include "peripheral/gpio_interface.h"
#include "app/current_time.h"
#include "app/trap_event.h"
#include "peripheral/LIS2DH12.h"


#include "app_util_platform.h"


static StateMachine stateMachine(WAIT_STATE);

uint8_t g_killNumber = 0;

static bool updateBLE = false;
Timer movementCountdown;


void accReadTimerHandler(void* p_context)
{
  LIS2DH12_sample();

  static int32_t accX, accY, accZ = 0;
  LIS2DH12_getALLmG(&accX, &accY, &accZ);
  DEBUG("X: %d, Y: %d, Z: %d", accX, accY, accZ);
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


void triggeredFromWaitTransition()
{
  INFO("Triggerd from wait");
  GPIO::setOutput(LED_1_PIN, LOW);
  GPIO::setOutput(LED_2_PIN, HIGH);
  Timer trapBufferCountdown;
  trapBufferCountdown.startCountdown(1000, trapBufferCountdownHandler);
}

void trapBufferEndedTransition()
{
  INFO("Trap buffer end");
  GPIO::setOutput(LED_1_PIN, LOW);
  GPIO::setOutput(LED_2_PIN, LOW);
  Timer moveBufferCountdown;
  moveBufferCountdown.startCountdown(5000, moveBufferCountdownHandler);
}

void moveBufferEndedTransition()
{
  INFO("Killed");
  GPIO::setOutput(LED_1_PIN, HIGH);
  GPIO::setOutput(LED_2_PIN, HIGH);
  g_killNumber++;
  updateBLE = true;
}


void triggeredFromMoveTransition()
{
  INFO("Triggerd from move");
  GPIO::setOutput(LED_1_PIN, HIGH);
  GPIO::setOutput(LED_2_PIN, LOW);
  movementCountdown.stopTimer();
  movementCountdown.startCountdown(10000, movementCountdownHandler);
}


void moveToWaitTransition()
{
  INFO("Trap set");
  GPIO::setOutput(LED_1_PIN, HIGH);
  GPIO::setOutput(LED_2_PIN, HIGH);
}



void accTriggeredHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  LIS2DH12_clearInterrupts();
  stateMachine.transition(TRIGGERED_EVENT);
  INFO("accTriggered");
}



void buttonHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  DEBUG("Button pressed");
}


void updateEventBLE()
{
  uint8_t killNumber = g_killNumber;
  BLE_Manager::manager().setCharacteristic(SERVICE_DETECTOR_DATA, CHAR_DETECTOR_NUMBER_OF_KILLS, &killNumber, sizeof(killNumber));

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
  LIS2DH12_init(LIS2DH12_POWER_LOW, LIS2DH12_SCALE2G, LIS2DH12_SAMPLE_10HZ);
  LIS2DH12_enableHighPass();
  LIS2DH12_initThresholdInterrupt(100, 0, LIS2DH12_INTERRUPT_THRESHOLD_XYZ, true, accTriggeredHandler);
  LIS2DH12_initDAPolling(int1Event);
  //LIS2DH12_startDAPolling();
}

void initBLE()
{
  BLE_Manager::manager().createBLEServer();
}

void initTime()
{
  Timer::initialisePeripheral();
  CurrentTime::startClock();
}

void createTransitionTable(void)
{
  stateMachine.registerTransition(WAIT_STATE, EVENT_BUFFER_STATE, TRIGGERED_EVENT, &triggeredFromWaitTransition);
  stateMachine.registerTransition(EVENT_BUFFER_STATE, IGNORED, TRIGGERED_EVENT, NULL);
  stateMachine.registerTransition(EVENT_BUFFER_STATE, DETECT_MOVE_STATE, BUFFER_END_EVENT, &trapBufferEndedTransition);
  stateMachine.registerTransition(DETECT_MOVE_STATE, WAIT_STATE, MOVEMENT_BUFFER_END_EVENT, &moveBufferEndedTransition);
  stateMachine.registerTransition(DETECT_MOVE_STATE, MOVING_STATE, TRIGGERED_EVENT, &triggeredFromMoveTransition);
  stateMachine.registerTransition(MOVING_STATE, MOVING_STATE, TRIGGERED_EVENT, &triggeredFromMoveTransition);
  stateMachine.registerTransition(MOVING_STATE, WAIT_STATE, SET_BUFFER_END_EVENT, &moveToWaitTransition);
  stateMachine.registerTransition(MOVING_STATE, IGNORED, MOVEMENT_BUFFER_END_EVENT, NULL);

}

int main(void)
{
	DEBUG_INIT();
  DEBUG("Started");

	createTransitionTable();

	initTimer();
	initBLE();
  initGPIO();
  initSensors();

  while(true)
  {

    uint32_t err_code = sd_app_evt_wait();
    ERROR_CHECK(err_code);


    if (updateBLE) {
      updateEventBLE();
      updateBLE = false;
    }


    GPIO::toggle(LED_1_PIN);
    //nrf_delay_ms(1000);

  }

}


/**
 *@}
 **/



















