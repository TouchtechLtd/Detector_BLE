
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

#define NRF_LOG_MODULE_NAME "MAIN"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"


#define ADC_STATE_DETECT 0
#define ADC_STATE_READ 1
#define ADC_STATE_RESET 2


static bool trapEventFlag = false;
static bool eventTimeoutFlag = false;
//TrapEvent curEvent;

void high_limit_test(void);

void test_handler(void* p_context)
{
    nrf_gpio_pin_toggle(LED_2_PIN);
}



void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	nrf_gpio_pin_toggle(LED_3_PIN);
}

void adc_handler(void* p_context)
{
	ADC::sample();
}


void eventTimeoutHandler(void * p_context) {
	UART::write("Event timed out");
	eventTimeoutFlag = true;
}


void limit_test(void) {
	UART::write("Limit triggered");
	trapEventFlag = true;
}


void low_limit_test(void) {
	Timer::startCountdown(TIMER_4, 2000, eventTimeoutHandler);
	//adc_5.setLimit(0, 50, high_limit_test);
}
/*
void high_limit_test(void) {
	Timer::stopTimer(TIMER_4);
	adc_5.setLimit(50, 0, low_limit_test);
}
*/

void sampleHandler_test (int sampleValue) {
	//curEvent.addData(sampleValue);
}


void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
}


int main(void)
{
	UART::init();
	Timer::init();

	GPIO::init();
    BLE::init();
    UART::write("BLE Manager Initialised.\r\n");

    BLE ble_manager;
    ble_manager.setBaseUUID(BLE_UUID_OUR_BASE_UUID);

    Service my_service(0xC001, BLE_UUID_OUR_BASE_UUID);
    UART::write("Service created.\r\n");

    Characteristic trapTriggered(0xFEE1);
    trapTriggered.enableRead();
    trapTriggered.enableNotification();
    uint8_t initValue = { 0x00 };
    trapTriggered.initValue(&initValue, 1);
    trapTriggered.setMaxLength(50);

    ble_char_id_t trapTriggered_id = my_service.addCharacteristic(&trapTriggered);

    BLE::adv.start(APP_ADV_DEFAULT_INTERVAL);

	GPIO::setOutput(LED_1_PIN, HIGH);
	GPIO::setOutput(LED_2_PIN, HIGH);
	GPIO::setOutput(LED_3_PIN, HIGH);
	GPIO::setOutput(LED_4_PIN, HIGH);


	GPIO::initIntInput(BUTTON_1_PIN,
						NRF_GPIOTE_POLARITY_HITOLO,
						NRF_GPIO_PIN_PULLUP,
						false,
						false,
						in_pin_handler);


	GPIO::interruptEnable(BUTTON_1_PIN);

	ADC adc_5(ADC_5);
	adc_5.setLimit(0, 100, limit_test);


	ADC::start();

	Timer::startTimer(TIMER_0, 100, adc_handler);
	Timer::startTimer(TIMER_1, 1000, test_handler);
	Timer::startTimer(TIMER_2, 100000, ADC::timed_recalibrate);


	int adcState = ADC_STATE_RESET;

    while(true)
    {

    	GPIO::toggle(LED_4_PIN);
    	//UART::write("Toggled the pin!!");

    	if (adcState == ADC_STATE_DETECT) {

			GPIO::low(LED_1_PIN);

			if (trapEventFlag == true) {
				UART::write("Changing to read state");
				adcState = ADC_STATE_READ;
				//curEvent.start();
				adc_5.setLimit(50, 0, low_limit_test);
				adc_5.attachSampleCallback(sampleHandler_test);

			    uint8_t value = { 0xAA };
			    trapTriggered.update(&value, 1);

				trapEventFlag = false;
			}

    	}


		if (adcState == ADC_STATE_READ) {

				GPIO::high(LED_1_PIN);

				if (eventTimeoutFlag == true) {
					adcState = ADC_STATE_RESET;
					eventTimeoutFlag = false;
				}
		}


		if (adcState == ADC_STATE_RESET) {


				//curEvent.processData();
				//curEvent.printData();
				//curEvent.clear();

				adc_5.detachSampleCallback();
				adc_5.setLimit(0, 100, limit_test);
				adcState = ADC_STATE_DETECT;


		}
		nrf_delay_ms(500);
		//UART::write("%d", i++);

    }

}


/**
 *@}
 **/



















