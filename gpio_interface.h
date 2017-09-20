/*
 * uart_driver.h
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#ifndef GPIO_INTERFACE_H_
#define GPIO_INTERFACE_H_

#include "nrf_gpio.h"
#include "app_gpiote.h"
#include "nrf_drv_gpiote.h"
#include "boards.h"


// Pins for LED's and buttons.
// The diodes on the DK are connected with the cathodes to the GPIO pin, so
// clearing a pin will light the LED and setting the pin will turn of the LED.
#define LED_1_PIN                       BSP_LED_0     // LED 1 on the nRF51-DK or nRF52-DK
#define LED_2_PIN                       BSP_LED_1     // LED 2 on the nRF51-DK or nRF52-DK
#define LED_3_PIN                       BSP_LED_2     // LED 3 on the nRF51-DK or nRF52-DK
#define LED_4_PIN                       BSP_LED_3     // LED 4 on the nRF51-DK or nRF52-DK
#define BUTTON_1_PIN                    BSP_BUTTON_0  // Button 1 on the nRF51-DK or nRF52-DK
#define BUTTON_2_PIN                    BSP_BUTTON_1  // Button 2 on the nRF51-DK or nRF52-DK
#define BUTTON_3_PIN                    BSP_BUTTON_2  // Button 3 on the nRF51-DK or nRF52-DK
#define BUTTON_4_PIN                    BSP_BUTTON_3  // Button 4 on the nRF51-DK or nRF52-DK

#define LOW 0
#define HIGH 1


class GPIO {
	private:



	public:
		static void init(void);


		static void initIntInput(unsigned long pin,
				nrf_gpiote_polarity_t sense,
				nrf_gpio_pin_pull_t pull,
				bool is_watcher,
				bool hi_accuracy,
				nrf_drv_gpiote_evt_handler_t handler);


		static void high(uint32_t pin);
		static void interruptDisable(uint32_t pin);
		static void interruptEnable(uint32_t pin);
		static bool inRange(uint32_t pin);
		static void low(uint32_t pin);
		static void toggle(uint32_t pin);
		static uint32_t read(uint32_t pin);
		static void setInput(uint32_t pin);
		static void pullup(uint32_t pin);
		static void pulldown(uint32_t pin);
		static void pullDisable(uint32_t pin);
		//static void setInterruptType(uint32_t pin, gpio_int_type_t intrType);
		static void setOutput(uint32_t pin);
		static void setOutput(uint32_t pin, uint32_t value);
		static void write(uint32_t pin, uint32_t value);
		static void defaultInput(uint32_t pin);

	}; // End GPIO




#endif /* GPIO_INTERFACE_H_ */
