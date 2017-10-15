
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



static StateMachine stateMachine(WAIT_STATE);
static ADC detectorADC;
TrapEvent curEvent;

uint8_t g_killNumber = 0;

static bool updateBLE = false;
Timer movementCountdown;




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
  Timer trapBufferCountdown;
  trapBufferCountdown.startCountdown(1000, trapBufferCountdownHandler);
}

void trapBufferEndedTransition()
{
  INFO("Trap buffer end");
  Timer moveBufferCountdown;
  moveBufferCountdown.startCountdown(5000, moveBufferCountdownHandler);
}

void moveBufferEndedTransition()
{
  INFO("Killed");
  g_killNumber++;
  updateBLE = true;
}


void triggeredFromMoveTransition()
{
  INFO("Triggerd from move");
  movementCountdown.stopTimer();
  movementCountdown.startCountdown(10000, movementCountdownHandler);
}




void accTriggeredHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  LIS2DH12_clearInterrupts();
  stateMachine.transition(TRIGGERED_EVENT);
  DEBUG("accTriggered");
}


void createTransitionTable(void) {
	stateMachine.registerTransition(WAIT_STATE, EVENT_BUFFER_STATE, TRIGGERED_EVENT, &triggeredFromWaitTransition);
	stateMachine.registerTransition(EVENT_BUFFER_STATE, IGNORED, TRIGGERED_EVENT, NULL);
	stateMachine.registerTransition(EVENT_BUFFER_STATE, DETECT_MOVE_STATE, BUFFER_END_EVENT, &trapBufferEndedTransition);
	stateMachine.registerTransition(DETECT_MOVE_STATE, WAIT_STATE, MOVEMENT_BUFFER_END_EVENT, &moveBufferEndedTransition);
	stateMachine.registerTransition(DETECT_MOVE_STATE, MOVING_STATE, TRIGGERED_EVENT, &triggeredFromMoveTransition);
	stateMachine.registerTransition(MOVING_STATE, MOVING_STATE, TRIGGERED_EVENT, &triggeredFromMoveTransition);
	stateMachine.registerTransition(MOVING_STATE, WAIT_STATE, SET_BUFFER_END_EVENT, NULL);
  stateMachine.registerTransition(MOVING_STATE, IGNORED, MOVEMENT_BUFFER_END_EVENT, NULL);

}



uint8_t* bit16Converter(uint16_t inputInt)
{
  static uint8_t result[2];
  result[0] = (inputInt & 0x00ff);
  result[1] = (inputInt & 0xff00) >> 8;
  return result;
};

uint8_t* bit32Converter(uint16_t inputInt)
{
  static uint8_t result[4];
  result[0] = (inputInt & 0x000000ff);
  result[1] = (inputInt & 0x0000ff00) >> 8;
  result[2] = (inputInt & 0x00ff0000) >> 16;
  result[3] = (inputInt & 0xff000000) >> 24;
  return result;
};

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


volatile bool da = false;
void int1Event(void* p_context)
{

  LIS2DH12_sample();

  static int32_t accX, accY, accZ = 0;
  LIS2DH12_getALLmG(&accX, &accY, &accZ);
  DEBUG("X: %d, Y: %d, Z: %d", accX, accY, accZ);

  da = true;


  GPIO::toggle(LED_2_PIN);
}




void buttonHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  DEBUG("Button pressed");
}


int main(void)
{
	DEBUG_INIT();
	GPIO::init();
	Timer::initialisePeripheral();
	CurrentTime::startClock();
	createTransitionTable();

  BLE_Manager::manager().createBLEService();

  DEBUG("Started");

	GPIO::setOutput(LED_1_PIN, LOW);
	GPIO::setOutput(LED_2_PIN, HIGH);

  LIS2DH12_init(LIS2DH12_POWER_LOW, LIS2DH12_SCALE2G, LIS2DH12_SAMPLE_10HZ);
  LIS2DH12_enableHighPass();

  LIS2DH12_initThresholdInterrupt(100, 0, LIS2DH12_INTERRUPT_THRESHOLD_XYZ, true, accTriggeredHandler);


  //LIS2DH12_initDAPolling(int1Event);
  //LIS2DH12_startDAPolling();

  GPIO::initIntInput(BUTTON_1,
      NRF_GPIOTE_POLARITY_HITOLO,
                NRF_GPIO_PIN_PULLUP,
                false,
                false,
                buttonHandler);
  GPIO::interruptEnable(BUTTON_1);



	BLE_Manager::manager().checkService();
	BLE_Manager::manager().checkChar();


  while(true)
  {



    if (updateBLE) {
      updateEventBLE();
    }

    //DEBUG("INT1: %d, INT2: %d", GPIO::read(INT_ACC1_PIN), GPIO::read(INT_ACC2_PIN));

    //GPIO::toggle(LED_1_PIN);
    nrf_delay_ms(1000);

  }

}


/**
 *@}
 **/



















