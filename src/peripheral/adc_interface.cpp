
/*
 * adc_interface.cpp
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nrf.h"
#include "nrf_drv_saadc.h"
#include "app_error.h"
#include "app_util_platform.h"

#include "debug/DEBUG.h"
#include "peripheral/timer_interface.h"
#include "peripheral/adc_interface.h"

#define SAADC_CALIBRATION_INTERVAL 100

bool ADC::isPeripheralInitialised = false;
bool ADC::isPeripheralStarted = false;
uint8_t ADC::peripheralCount = 0;
bool ADC::isCalibrating = false;
bool ADC::m_isSamplingEnabled = false;
adc_limit_handler_t ADC::limitCallbacks[MAX_ADC_CHANNELS] = {0};
adc_sample_handler_t ADC::sampleCallbacks[MAX_ADC_CHANNELS] = {0};

nrf_saadc_value_t     ADC::m_buffer_pool[2][SAMPLES_IN_BUFFER];


ADC::ADC () {
	m_channel = 0;
}

ADC::ADC(adc_input_t adc_input) {
	attachADC(adc_input);
}


void ADC::attachADC(adc_input_t adc_input) {

	if (isPeripheralStarted) 		{ return; }
	if (!isPeripheralInitialised) 	{ init(); }

	channelConfig(adc_input, peripheralCount);
	peripheralCount++;
}


void ADC::attachSampleCallback(adc_sample_handler_t sample_handler) {
	ADC::sampleCallbacks[m_channel] = sample_handler;
}

void ADC::detachSampleCallback() {
	ADC::sampleCallbacks[m_channel] = NULL;
}




void ADC::channelConfig(adc_input_t adc_input, uint8_t channel) {

	nrf_saadc_channel_config_t channel_config;

	//Configure SAADC channel
	channel_config.reference = NRF_SAADC_REFERENCE_INTERNAL;                              //Set internal reference of fixed 0.6 volts
	channel_config.gain = NRF_SAADC_GAIN1_6;                                              //Set input gain to 1/6. The maximum SAADC input voltage is then 0.6V/(1/6)=3.6V. The single ended input range is then 0V-3.6V
	channel_config.acq_time = NRF_SAADC_ACQTIME_10US;                                     //Set acquisition time. Set low acquisition time to enable maximum sampling frequency of 200kHz. Set high acquisition time to allow maximum source resistance up to 800 kohm, see the SAADC electrical specification in the PS.
	channel_config.mode = NRF_SAADC_MODE_SINGLE_ENDED;                                    //Set SAADC as single ended. This means it will only have the positive pin as input, and the negative pin is shorted to ground (0V) internally.
	channel_config.pin_p = (nrf_saadc_input_t)adc_input;                                  //Select the input pin for the channel. AIN0 pin maps to physical pin P0.02.
	channel_config.pin_n = NRF_SAADC_INPUT_DISABLED;                                      //Since the SAADC is single ended, the negative pin is disabled. The negative pin is shorted to ground internally.
	channel_config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;                              //Disable pullup resistor on the input pin
	channel_config.resistor_n = NRF_SAADC_RESISTOR_PULLDOWN;                              //Disable pulldown resistor on the input pin


	//Initialize SAADC channel
	uint32_t err_code = nrf_drv_saadc_channel_init(0, &channel_config);                            //Initialize SAADC channel 0 with the channel configuration
	ERROR_CHECK(err_code);

	NRF_SAADC->CH[0].CONFIG |= 0x01000000;                                            //Configure burst mode for channel 0. Burst is useful together with oversampling. When triggering the SAMPLE task in burst mode, the SAADC will sample "Oversample" number of times as fast as it can and then output a single averaged value to the RAM buffer. If burst mode is not enabled, the SAMPLE task needs to be triggered "Oversample" number of times to output a single averaged value to the RAM buffer.

	m_channel = channel;
}


void ADC::setLimit(uint16_t lowLimit, uint16_t highLimit, adc_limit_handler_t handler) {
	if (lowLimit == 0) { lowLimit = NRF_DRV_SAADC_LIMITL_DISABLED; }
	if (highLimit == 0) { highLimit = NRF_DRV_SAADC_LIMITH_DISABLED; }

	nrf_drv_saadc_limits_set(m_channel,	lowLimit, highLimit);
	ADC::limitCallbacks[m_channel] = handler;
}


void ADC::clearLimit() {
	nrf_drv_saadc_limits_set(m_channel,	NRF_DRV_SAADC_LIMITL_DISABLED, NRF_DRV_SAADC_LIMITH_DISABLED);
}


void ADC::saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
	if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
	{
		if(isCalibrating == true && busy()) {                                 //Evaluate if offset calibration should be performed. Configure the SAADC_CALIBRATION_INTERVAL constant to change the calibration frequency
		  m_isSamplingEnabled = false;
		  abort();						    							 // Abort all ongoing conversions. Calibration cannot be run if SAADC is busy
			Timer adcRecalibrationCountdown;
			adcRecalibrationCountdown.startCountdown(1, recalibrate);                 // Set flag to trigger calibration in main context when SAADC is stopped
		}

		if (isCalibrating == false) {

      ret_code_t err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
      if (err_code != NRF_SUCCESS) { ERROR_CHECK(err_code); }

	    if (ADC::sampleCallbacks[0] != 0) {
	      for (int i = 0; i < SAMPLES_IN_BUFFER; i++) {
	        ADC::sampleCallbacks[0](p_event->data.done.p_buffer[i]);
	      }
	    }
		}
	}

	else if (p_event->type == NRF_DRV_SAADC_EVT_CALIBRATEDONE)
	{
		restart();
		isCalibrating = false;
		DEBUG("SAADC calibration complete.");
	}

	else if (p_event->type == NRF_DRV_SAADC_EVT_LIMIT) {
		ADC::limitCallbacks[p_event->data.limit.channel]();
	}
}


void ADC::init() {

	nrf_drv_saadc_config_t saadc_config;
	saadc_config.resolution = NRF_SAADC_RESOLUTION_10BIT;
	saadc_config.oversample = NRF_SAADC_OVERSAMPLE_16X;
	saadc_config.interrupt_priority = SAADC_CONFIG_IRQ_PRIORITY;
	saadc_config.low_power_mode     = SAADC_CONFIG_LP_MODE;

	uint32_t err_code = nrf_drv_saadc_init(&saadc_config, saadc_callback);
	ERROR_CHECK(err_code);

	isPeripheralInitialised = true;
}


void ADC::start() {

	if (peripheralCount % SAMPLES_IN_BUFFER != 0)
	{
		DEBUG("Samples need to be multiple of initialised ADC");
	}

	uint32_t err_code;
  err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);
  ERROR_CHECK(err_code);

  err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1], SAMPLES_IN_BUFFER);
  ERROR_CHECK(err_code);

  isPeripheralStarted = true;
  m_isSamplingEnabled = true;
}


void ADC::restart() {
  uint32_t err_code;
  err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);
  ERROR_CHECK(err_code);

  isPeripheralStarted = true;
  m_isSamplingEnabled = true;
}



void ADC::sample() {
  if (m_isSamplingEnabled) {
    uint32_t err_code = nrf_drv_saadc_sample();
    ERROR_CHECK(err_code);
  }
}

void ADC::abort() {
	nrf_drv_saadc_abort();
}

bool ADC::busy(void) {
	return nrf_drv_saadc_is_busy();
}

void ADC::timed_recalibrate(void* p_context) {
	DEBUG("Triggering calibration");
	isCalibrating = true;
}

void ADC::recalibrate(void* p_context) {
  DEBUG("Calibration in progress");
	while (nrf_drv_saadc_calibrate_offset() != NRF_SUCCESS);		//Trigger calibration task
}

