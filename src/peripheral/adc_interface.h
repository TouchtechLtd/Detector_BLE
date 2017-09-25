
/*
 * adc_interface.h
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#ifndef _GOODNATURE_PERIPHERAL_ADC_INTERFACE_H_
#define _GOODNATURE_PERIPHERAL_ADC_INTERFACE_H_


#include "nrf_drv_saadc.h"

#define SAMPLES_IN_BUFFER 1
#define MAX_ADC_CHANNELS 7

typedef void (*adc_limit_handler_t)(void);
typedef void (*adc_sample_handler_t)(int sample);


typedef enum
{
	ADC_DISABLED = NRF_SAADC_INPUT_DISABLED,
	ADC_0 = NRF_SAADC_INPUT_AIN0,
	ADC_1 = NRF_SAADC_INPUT_AIN1,
	ADC_2 = NRF_SAADC_INPUT_AIN2,
	ADC_3 = NRF_SAADC_INPUT_AIN3,
	ADC_4 = NRF_SAADC_INPUT_AIN4,
	ADC_5 = NRF_SAADC_INPUT_AIN5,
	ADC_6 = NRF_SAADC_INPUT_AIN6,
	ADC_BATT = NRF_SAADC_INPUT_VDD
} adc_input_t;



class ADC {
	private:

  static nrf_saadc_value_t     m_buffer_pool[2][SAMPLES_IN_BUFFER];

	static bool isPeripheralInitialised;
	static bool isPeripheralStarted;
	static bool isCalibrating;
	static bool m_isSamplingEnabled;

	static uint8_t peripheralCount;

	static adc_limit_handler_t limitCallbacks[MAX_ADC_CHANNELS];
	static adc_sample_handler_t sampleCallbacks[MAX_ADC_CHANNELS];
	static void saadc_callback(nrf_drv_saadc_evt_t const * p_event);
	static void init();

	uint8_t m_channel;

	public:

	ADC();
	ADC(adc_input_t adc_input);
	ADC(adc_input_t adc_input, adc_sample_handler_t sample_handler);
	void attachADC(adc_input_t adc_input);
	void setLimit(uint16_t lowLimit, uint16_t highLimit, adc_limit_handler_t handler);
	void clearLimit();
	void channelConfig(adc_input_t adc_input, uint8_t channel);
	void attachSampleCallback(adc_sample_handler_t sample_handler);
	void detachSampleCallback();

	static void start();
	static void restart();
	static void sample();
	static void abort();
	static bool busy();
	static void recalibrate(void*);
	static void timed_recalibrate(void*);

	}; // End ADC




#endif /* _GOODNATURE_PERIPHERAL_ADC_INTERFACE_H_ */
