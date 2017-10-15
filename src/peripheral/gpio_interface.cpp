
/*
 * gpio_interface.cpp
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

#include "debug/DEBUG.h"
#include "peripheral/gpio_interface.h"


// Function for configuring GPIO.
void GPIO::init()
{

	uint32_t err_code = nrf_drv_gpiote_init();
	ERROR_CHECK(err_code);
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
						gpio_event_handler_t handler) {


	nrf_drv_gpiote_in_config_t in_config;
	in_config.sense = sense;					//NRF_GPIOTE_POLARITY_HITOLO;
	in_config.pull = pull;						//NRF_GPIO_PIN_PULLUP;
	in_config.is_watcher = is_watcher;			//false;
	in_config.hi_accuracy = hi_accuracy;		//false;

	uint32_t err_code = nrf_drv_gpiote_in_init(pin, &in_config, handler);
	ERROR_CHECK(err_code);
}


void GPIO::interruptEnable(uint32_t pin) {
	nrf_drv_gpiote_in_event_enable(pin, true);
}


void GPIO::interruptDisable(uint32_t pin) {
	nrf_drv_gpiote_in_event_disable(pin);
}




/*
void gpiote_int_init()
{
    nrf_gpio_cfg_sense_input(BUTTON_1, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(BUTTON_2, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);

    NRF_GPIOTE->EVENTS_PORT = 0;

    NRF_GPIOTE->INTENSET = (GPIOTE_INTENSET_PORT_Enabled << GPIOTE_INTENSET_PORT_Pos);

    NVIC_SetPriority(GPIOTE_IRQn, APP_IRQ_PRIORITY_LOW);
    NVIC_EnableIRQ(GPIOTE_IRQn);
}

extern "C" {
void GPIOTE_IRQHandler(void)
{
  uint32_t input[GPIO_COUNT] = {0};
    if(NRF_GPIOTE->EVENTS_PORT != 0)
    {
        NRF_GPIOTE->EVENTS_PORT = 0;
        nrf_gpio_ports_read(0, GPIO_COUNT, input);
        for (int i = 0; i < GPIO_COUNT; i++) {
          DEBUG("%d", input[i]);
        }

    }
}
}
*/



