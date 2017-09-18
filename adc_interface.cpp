/**
 * Copyright (c) 2014 - 2017, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 * @defgroup nrf_adc_example main.c
 * @{
 * @ingroup nrf_adc_example
 * @brief ADC Example Application main file.
 *
 * This file contains the source code for a sample application using ADC.
 *
 * @image html example_board_setup_a.jpg "Use board setup A for this example."
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nrf.h"
#include "nrf_drv_saadc.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "uart_interface.h"
#include "timer_interface.h"

#include "adc_interface.h"

#define SAMPLES_IN_BUFFER 1
#define SAADC_CALIBRATION_INTERVAL 100

bool ADC::adcInitialised = false;
bool ADC::adcStarted = false;
uint8_t ADC::adcCount = 0;
bool ADC::_calibrating = false;
adc_limit_handler_t ADC::limitCallback[MAX_ADC_CHANNELS] = {0};
adc_sample_handler_t ADC::sampleCallback[MAX_ADC_CHANNELS] = {0};


ADC::ADC(adc_input_t adc_input) {
	if (adcStarted) { return; }

	if (!adcInitialised) { init(); }

	channelConfig(adc_input, adcCount);

	adcCount++;
}


void ADC::attachSampleCallback(adc_sample_handler_t sample_handler) {
	ADC::sampleCallback[_channel] = sample_handler;
}

void ADC::detachSampleCallback() {
	ADC::sampleCallback[_channel] = NULL;
}




void ADC::channelConfig(adc_input_t adc_input, uint8_t channel) {

	ret_code_t err_code;

	nrf_saadc_channel_config_t channel_config;

	//Configure SAADC channel
	channel_config.reference = NRF_SAADC_REFERENCE_INTERNAL;                              //Set internal reference of fixed 0.6 volts
	channel_config.gain = NRF_SAADC_GAIN1_6;                                              //Set input gain to 1/6. The maximum SAADC input voltage is then 0.6V/(1/6)=3.6V. The single ended input range is then 0V-3.6V
	channel_config.acq_time = NRF_SAADC_ACQTIME_10US;                                     //Set acquisition time. Set low acquisition time to enable maximum sampling frequency of 200kHz. Set high acquisition time to allow maximum source resistance up to 800 kohm, see the SAADC electrical specification in the PS.
	channel_config.mode = NRF_SAADC_MODE_SINGLE_ENDED;                                    //Set SAADC as single ended. This means it will only have the positive pin as input, and the negative pin is shorted to ground (0V) internally.
	channel_config.pin_p = (nrf_saadc_input_t)adc_input;                                          //Select the input pin for the channel. AIN0 pin maps to physical pin P0.02.
	channel_config.pin_n = NRF_SAADC_INPUT_DISABLED;                                      //Since the SAADC is single ended, the negative pin is disabled. The negative pin is shorted to ground internally.
	channel_config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;                              //Disable pullup resistor on the input pin
	channel_config.resistor_n = NRF_SAADC_RESISTOR_PULLDOWN;                              //Disable pulldown resistor on the input pin


	//Initialize SAADC channel
	err_code = nrf_drv_saadc_channel_init(0, &channel_config);                            //Initialize SAADC channel 0 with the channel configuration
	APP_ERROR_CHECK(err_code);

	NRF_SAADC->CH[0].CONFIG |= 0x01000000;                                            //Configure burst mode for channel 0. Burst is useful together with oversampling. When triggering the SAMPLE task in burst mode, the SAADC will sample "Oversample" number of times as fast as it can and then output a single averaged value to the RAM buffer. If burst mode is not enabled, the SAMPLE task needs to be triggered "Oversample" number of times to output a single averaged value to the RAM buffer.

	_channel = channel;
}


void ADC::setLimit(uint16_t lowLimit, uint16_t highLimit, adc_limit_handler_t handler) {
	if (lowLimit == 0) { lowLimit = NRF_DRV_SAADC_LIMITL_DISABLED; }
	if (highLimit == 0) { highLimit = NRF_DRV_SAADC_LIMITH_DISABLED; }

	nrf_drv_saadc_limits_set(_channel,	lowLimit, highLimit);
	ADC::limitCallback[_channel] = handler;
}


void ADC::clearLimit() {
	nrf_drv_saadc_limits_set(_channel,	NRF_DRV_SAADC_LIMITL_DISABLED, NRF_DRV_SAADC_LIMITH_DISABLED);
}


void ADC::saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
	if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
	{
		if(_calibrating == true && busy()) {                                 //Evaluate if offset calibration should be performed. Configure the SAADC_CALIBRATION_INTERVAL constant to change the calibration frequency
			abort();						    							 // Abort all ongoing conversions. Calibration cannot be run if SAADC is busy
			Timer::startCountdown(TIMER_5, 1, recalibrate);                 // Set flag to trigger calibration in main context when SAADC is stopped
		}

		if (ADC::sampleCallback[0] != 0) {
			for (int i = 0; i < SAMPLES_IN_BUFFER; i++) {
				//UART::write("%d", p_event->data.done.p_buffer[i]);
				ADC::sampleCallback[0](p_event->data.done.p_buffer[i]);
			}
		}

		if (_calibrating == false) {
			ret_code_t err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
			APP_ERROR_CHECK(err_code);
		}
	}

	else if (p_event->type == NRF_DRV_SAADC_EVT_CALIBRATEDONE)
	{
		start();
		_calibrating = false;
		UART::write("SAADC calibration complete ! \r\n");                                              //Print on UART
	}

	else if (p_event->type == NRF_DRV_SAADC_EVT_LIMIT) {
		ADC::limitCallback[p_event->data.limit.channel]();
	}
}


void ADC::init() {
	ret_code_t err_code;

	UART::write("Init");

	nrf_drv_saadc_config_t saadc_config;
	saadc_config.resolution = NRF_SAADC_RESOLUTION_10BIT;
	saadc_config.oversample = NRF_SAADC_OVERSAMPLE_16X;
	saadc_config.interrupt_priority = SAADC_CONFIG_IRQ_PRIORITY;
	saadc_config.low_power_mode     = SAADC_CONFIG_LP_MODE;

	err_code = nrf_drv_saadc_init(&saadc_config, saadc_callback);
	APP_ERROR_CHECK(err_code);

	adcInitialised = true;
}


void ADC::start() {

	ret_code_t err_code;

	if (adcCount % SAMPLES_IN_BUFFER != 0) { UART::write("Error, samples need to be multiple of initialised ADC"); }

	static nrf_saadc_value_t     m_buffer_pool[2][SAMPLES_IN_BUFFER];

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

    adcStarted = true;
}



void ADC::sample() {
	nrf_drv_saadc_sample();
}

void ADC::abort() {
	nrf_drv_saadc_abort();
}

bool ADC::busy(void) {
	return nrf_drv_saadc_is_busy();
}

void ADC::timed_recalibrate(void* p_context) {
	UART::write("Triggering calibration");
	_calibrating = true;
}

void ADC::recalibrate(void* p_context) {
	while (nrf_drv_saadc_calibrate_offset() != NRF_SUCCESS);		//Trigger calibration task
}

