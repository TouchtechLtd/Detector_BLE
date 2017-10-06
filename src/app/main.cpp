
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




static StateMachine stateMachine(DETECT_STATE);
static ADC detectorADC;
TrapEvent curEvent;

static bool shouldProcessData = false;


void inPinHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	nrf_gpio_pin_toggle(LED_3_PIN);
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


/*
int main(void)
{
	DEBUG_INIT();
	GPIO::init();
	Timer::initialisePeripheral();
	CurrentTime::startClock();

  BLE_Manager::manager().createBLEService();


	GPIO::setOutput(LED_1_PIN, LOW);
	GPIO::setOutput(LED_2_PIN, HIGH);
	GPIO::setOutput(LED_3_PIN, HIGH);
	GPIO::setOutput(LED_4_PIN, HIGH);


	//detectorADC.attachADC(ADC_5);
	//detectorADC.setLimit(0, 50, highLimitHandler);
	//ADC::start();

	BLE_Manager::manager().checkService();
	BLE_Manager::manager().checkChar();

	//createTransitionTable();
	//Timer adcSampleTimer;
	//Timer adcRecalibrationTimer;
	//adcSampleTimer.startTimer(100, adcHandler);

	//detectorADC.attachSampleCallback(detectorADCSampleHandler);

	uint16_t x = 1567;
	uint8_t* array = bit16Converter(x);
	DEBUG("%d", array[1]);

  while(true)
  {
    GPIO::toggle(LED_1_PIN);

    if (shouldProcessData) {
      curEvent.setTimeStamp(CurrentTime::getCurrentTime());
      curEvent.processData();
      curEvent.printData();
      updateEventBLE(curEvent);
      curEvent.clear();
      stateMachine.transition(PROCESSING_FINISHED_EVENT);
    }
    nrf_delay_ms(500);

  }

}
*/

/**
 *@}
 **/



















