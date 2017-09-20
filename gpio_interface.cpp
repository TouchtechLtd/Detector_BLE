/*
 * uart_driver.cpp
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include "app_uart.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf.h"
#include "boards.h"

#include "uart_interface.h"
#include "gpio_interface.h"


// Function for configuring GPIO.
void GPIO::init()
{
	ret_code_t err_code;

	err_code = nrf_drv_gpiote_init();
	APP_ERROR_CHECK(err_code);
}


void GPIO::setOutput(uint32_t pin) {
	    nrf_gpio_cfg_output(pin);
}

void GPIO::setOutput(uint32_t pin, uint32_t value) {
	    nrf_gpio_cfg_output(pin);
	    write(pin, value);
}


void GPIO::setInput(uint32_t pin) {
	nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_NOPULL);
}


void GPIO::pullup(uint32_t pin) {
	nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_PULLUP);
}

void GPIO::pulldown(uint32_t pin) {
	nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_PULLDOWN);
}

void GPIO::low(uint32_t pin) {
	nrf_gpio_pin_clear(pin);
}

void GPIO::high(uint32_t pin) {
	nrf_gpio_pin_set(pin);
}

void GPIO::toggle(uint32_t pin) {
	nrf_gpio_pin_toggle(pin);
}

void GPIO::write(uint32_t pin, uint32_t value) {
	nrf_gpio_pin_write(pin, value);
}

uint32_t GPIO::read(uint32_t pin){
	return nrf_gpio_pin_read(pin);
}

void GPIO::initIntInput(unsigned long pin,
						nrf_gpiote_polarity_t sense,
						nrf_gpio_pin_pull_t pull,
						bool is_watcher,
						bool hi_accuracy,
						nrf_drv_gpiote_evt_handler_t handler) {

	uint32_t err_code;

	nrf_drv_gpiote_in_config_t in_config;
	in_config.sense = sense;					//NRF_GPIOTE_POLARITY_HITOLO;
	in_config.pull = pull;						//NRF_GPIO_PIN_PULLUP;
	in_config.is_watcher = is_watcher;			//false;
	in_config.hi_accuracy = hi_accuracy;		//false;

	err_code = nrf_drv_gpiote_in_init(pin, &in_config, handler);
	APP_ERROR_CHECK(err_code);
}


void GPIO::interruptEnable(uint32_t pin) {
	nrf_drv_gpiote_in_event_enable(pin, true);
}


void GPIO::interruptDisable(uint32_t pin) {
	nrf_drv_gpiote_in_event_disable(pin);
}
