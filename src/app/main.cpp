
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



static StateMachine stateMachine(DETECT_STATE);
static ADC detectorADC;
TrapEvent curEvent;

static bool shouldProcessData = false;


void inPinHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	nrf_gpio_pin_toggle(LED_2_PIN);
}

void adcHandler(void* p_context)
{
	ADC::sample();
}

void lowLimitHandler(void) {
	detectorADC.clearLimit();
	stateMachine.transition(READ_FINISHED_EVENT);
}

void detectorADCSampleHandler (int sampleValue) {
  INFO("%d", sampleValue);
  //curEvent.addData(sampleValue);
}

void highLimitHandler(void) {
  INFO("~");
  curEvent.start();
  //detectorADC.attachSampleCallback(detectorADCSampleHandler);
	detectorADC.setLimit(50, 0, lowLimitHandler);
	stateMachine.transition(TRIGGERED_EVENT);
}



void triggeredEventTransition() {
  //INFO("Trap got triggered");

	GPIO::setOutput(LED_1_PIN, LOW);

}

void readEventTransition() {
  INFO("!");
  //INFO("Reading finished");
	GPIO::setOutput(LED_1_PIN, HIGH);

	curEvent.end();
	//detectorADC.detachSampleCallback();
	shouldProcessData = true;
}

void processEventTransition() {
  //INFO("Processing finished");
	detectorADC.setLimit(0, 50, highLimitHandler);
	shouldProcessData = false;
}



void createTransitionTable(void) {
	stateMachine.registerTransition(DETECT_STATE, READ_STATE, TRIGGERED_EVENT, &triggeredEventTransition);
	stateMachine.registerTransition(READ_STATE, PROCESS_STATE, READ_FINISHED_EVENT, &readEventTransition);
	stateMachine.registerTransition(PROCESS_STATE, DETECT_STATE, PROCESSING_FINISHED_EVENT, &processEventTransition);
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



void updateEventBLE(TrapEvent event)
{
  uint8_t killNumber = curEvent.getKillNumber();
  BLE_Manager::manager().setCharacteristic(SERVICE_DETECTOR_DATA, CHAR_DETECTOR_NUMBER_OF_KILLS, &killNumber, sizeof(killNumber));

  uint8_t didClip = curEvent.getDidClip();
  BLE_Manager::manager().setCharacteristic(SERVICE_DETECTOR_DATA, CHAR_DETECTOR_DID_CLIP, &didClip, sizeof(didClip));

  uint16_t peakValue = curEvent.getPeakValue();
  BLE_Manager::manager().setCharacteristic(SERVICE_DETECTOR_DATA, CHAR_DETECTOR_PEAK_VALUE, bit16Converter(peakValue), sizeof(peakValue));

  uint16_t responseSize = curEvent.getResponseSize();
  BLE_Manager::manager().setCharacteristic(SERVICE_DETECTOR_DATA, CHAR_DETECTOR_RESPONSE_SIZE, bit16Converter(responseSize), sizeof(responseSize));

  uint32_t responseLength = curEvent.getResponseLength();
  BLE_Manager::manager().setCharacteristic(SERVICE_DETECTOR_DATA, CHAR_DETECTOR_RESPONSE_LENGTH, bit32Converter(responseLength), sizeof(responseLength));

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

void intThreshEvent(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  LIS2DH12_clearInterrupts();
  stateMachine.transition(TRIGGERED_EVENT);
}




int main(void)
{
	DEBUG_INIT();
	GPIO::init();
	Timer::initialisePeripheral();
	CurrentTime::startClock();

  BLE_Manager::manager().createBLEService();

  DEBUG("Started");

	GPIO::setOutput(LED_1_PIN, LOW);
	GPIO::setOutput(LED_2_PIN, HIGH);

  LIS2DH12_init(LIS2DH12_POWER_LOW, LIS2DH12_SCALE2G, LIS2DH12_SAMPLE_1HZ);
  LIS2DH12_enableHighPass();

  LIS2DH12_initThresholdInterrupt1(100, 0, LIS2DH12_INTERRUPT_PIN_2, LIS2DH12_INTERRUPT_THRESHOLD_XYZ, true, true, intThreshEvent);
  LIS2DH12_initDAPolling(int1Event);
  LIS2DH12_startDAPolling();



	BLE_Manager::manager().checkService();
	BLE_Manager::manager().checkChar();


  while(true)
  {



    if (shouldProcessData) {
      curEvent.setTimeStamp(CurrentTime::getCurrentTime());
      curEvent.processData();
      curEvent.printData();
      updateEventBLE(curEvent);
      curEvent.clear();
      stateMachine.transition(PROCESSING_FINISHED_EVENT);
    }

    //DEBUG("INT1: %d, INT2: %d", GPIO::read(INT_ACC1_PIN), GPIO::read(INT_ACC2_PIN));

    //GPIO::toggle(LED_1_PIN);
    nrf_delay_ms(500);

  }

}


/**
 *@}
 **/



















