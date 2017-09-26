
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

void highLimitHandler(void) {
	detectorADC.setLimit(100, 0, lowLimitHandler);
	stateMachine.transition(TRIGGERED_EVENT);
}

void detectorADCSampleHandler (int sampleValue) {
  INFO("%d", sampleValue);
	curEvent.addData(sampleValue);
}


void triggeredEventTransition() {
	DEBUG("Trap got triggered");

	GPIO::setOutput(LED_1_PIN, LOW);

	curEvent.start();
	detectorADC.attachSampleCallback(detectorADCSampleHandler);
}

void readEventTransition() {
	DEBUG("Reading finished");

	GPIO::setOutput(LED_1_PIN, HIGH);

	curEvent.end();
	detectorADC.detachSampleCallback();
	shouldProcessData = true;
}

void processEventTransition() {
	DEBUG("Processing finished");
	detectorADC.setLimit(0, 100, highLimitHandler);
	shouldProcessData = false;
}



void createTransitionTable(void) {
	stateMachine.registerTransition(DETECT_STATE, READ_STATE, TRIGGERED_EVENT, &triggeredEventTransition);
	stateMachine.registerTransition(READ_STATE, PROCESS_STATE, READ_FINISHED_EVENT, &readEventTransition);
	stateMachine.registerTransition(PROCESS_STATE, DETECT_STATE, PROCESSING_FINISHED_EVENT, &processEventTransition);
}



union TwoBytes
{
  uint16_t u16;
  uint8_t u8[2];
};

int main(void)
{
	DEBUG_INIT();
	GPIO::init();
	Timer::initialisePeripheral();
	CurrentTime::startClock();

  BLE_Manager::manager().createBLEService();


	GPIO::setOutput(LED_1_PIN, HIGH);
	GPIO::setOutput(LED_2_PIN, HIGH);
	GPIO::setOutput(LED_3_PIN, HIGH);
	GPIO::setOutput(LED_4_PIN, HIGH);

	detectorADC.attachADC(ADC_5);
	detectorADC.setLimit(0, 100, highLimitHandler);
	ADC::start();

	BLE_Manager::manager().checkService();
	BLE_Manager::manager().checkChar();

	createTransitionTable();
	Timer adcSampleTimer;
	Timer adcRecalibrationTimer;
	adcSampleTimer.startTimer(100, adcHandler);

	//detectorADC.attachSampleCallback(detectorADCSampleHandler);

	uint16_t x = 1567;
	uint8_t* array = reinterpret_cast<uint8_t*>(&x);
	DEBUG("%d", sizeof(x));

  while(true)
  {
    GPIO::toggle(LED_4_PIN);

    if (shouldProcessData) {
      curEvent.setTimeStamp(CurrentTime::getCurrentTime());
      curEvent.processData();
      curEvent.printData();
      uint16_t killNumber = curEvent.getPeakValue();
      BLE_Manager::manager().updateCharacteristic(SERVICE_DETECTOR_DATA, CHAR_DETECTOR_NUMBER_OF_KILLS, reinterpret_cast<uint8_t*>(&killNumber), sizeof(killNumber));

      curEvent.clear();
      stateMachine.transition(PROCESSING_FINISHED_EVENT);
    }
    nrf_delay_ms(500);

  }

}


/**
 *@}
 **/



















