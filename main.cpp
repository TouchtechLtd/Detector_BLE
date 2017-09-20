
/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
* @defgroup blinky_example_pca10001_main main.c
* @{
* @ingroup blinky_example_pca10001
*
* @brief Blinky Example Application main file.
*
* This file contains the source code for a sample application using GPIO to drive LEDs.
*
*/

#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_drv_gpiote.h"

#include "gpio_interface.h"
#include "uart_interface.h"
#include "timer_interface.h"
#include "adc_interface.h"
#include "ble_interface.h"
//#include "TrapEvent.h"
//#include "Detector.h"
#include "StateMachine.h"

#define NRF_LOG_MODULE_NAME "MAIN"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"



static StateMachine sm(DETECT_STATE);
static ADC adc_5;
//TrapEvent curEvent;

static bool processDataFlag = false;

static Characteristic trapTriggered;


void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	nrf_gpio_pin_toggle(LED_3_PIN);
}

void adc_handler(void* p_context)
{
	ADC::sample();
}

void low_limit_handler(void) {
	adc_5.clearLimit();
	sm.transition(READ_FINISHED_EVENT);
}

void limit_test(void) {
	adc_5.setLimit(50, 0, low_limit_handler);
	sm.transition(TRIGGERED_EVENT);
}

void sampleHandler_test (int sampleValue) {
	//curEvent.addData(sampleValue);
}


void triggered_event_handler() {
	UART::write("Trap got triggered");
	UART::write("Changing to read state");

	static uint8_t value = 0;
	static uint16_t length = 1;

	//curEvent.start();
	adc_5.attachSampleCallback(sampleHandler_test);
	value++;
	trapTriggered.notify(&value, &length);
}

void read_event_handler() {
	UART::write("Reading finished");
	adc_5.detachSampleCallback();
	processDataFlag = true;
}

void process_event_handler() {
	UART::write("Processing finished");
	adc_5.setLimit(0, 100, limit_test);
	processDataFlag = false;
}




int main(void)
{
	UART::init();
	Timer::init();

	GPIO::init();
    BLE::init();
    UART::write("BLE Manager Initialised.\r\n");

    BLE::setBaseUUID(BLE_UUID_OUR_BASE_UUID);

    Service my_service(0xC001, BLE_UUID_OUR_BASE_UUID);
    UART::write("Service created - ID:%d", my_service.getID());

    trapTriggered.setUUID(0xFEE1);
    trapTriggered.enableRead();
    trapTriggered.enableNotification();
    uint8_t initValue = { 0x00 };
    trapTriggered.initValue(&initValue, 1);
    trapTriggered.setMaxLength(50);

    my_service.addCharacteristic(&trapTriggered);

    BLE::addService(&my_service);

    BLE::adv.start(APP_ADV_DEFAULT_INTERVAL);
    BLE::adv.advertiseName();

	GPIO::setOutput(LED_1_PIN, HIGH);
	GPIO::setOutput(LED_2_PIN, HIGH);
	GPIO::setOutput(LED_3_PIN, HIGH);
	GPIO::setOutput(LED_4_PIN, HIGH);

	adc_5.attachADC(ADC_5);
	adc_5.setLimit(0, 100, limit_test);
	ADC::start();

	Timer::startTimer(TIMER_0, 100, adc_handler);
	Timer::startTimer(TIMER_2, 100000, ADC::timed_recalibrate);

	sm.registerTransition(DETECT_STATE, READ_STATE, TRIGGERED_EVENT, &triggered_event_handler);
	sm.registerTransition(READ_STATE, PROCESS_STATE, READ_FINISHED_EVENT, &read_event_handler);
	sm.registerTransition(PROCESS_STATE, DETECT_STATE, PROCESSING_FINISHED_EVENT, &process_event_handler);


    while(true)
    {

    	GPIO::toggle(LED_4_PIN);
    	//UART::write("Toggled the pin!!");

    	if (processDataFlag) {
    		//curEvent.processData();
			//curEvent.printData();
			//curEvent.clear();
    		sm.transition(PROCESSING_FINISHED_EVENT);
    	}
		nrf_delay_ms(500);
		//UART::write("%d", i++);

    }

}


/**
 *@}
 **/



















