/**
@addtogroup LIS2DH12Driver LIS2DH12 Acceleration Sensor Driver
@{
@file       LIS2DH12.h

Hardware Driver for the LIS2DH12 Acceleration Sensor

* @}
***************************************************************************************************/
#ifndef LIS2DH12_H
#define LIS2DH12_H

#ifdef __cplusplus
extern "C"
{
#endif

/* INCLUDES ***************************************************************************************/
#include <stdbool.h>
#include <stdint.h>

#include "LIS2DH12_registers.h"
#include "nordic_common.h"

#include "peripheral/gpio_interface.h"
#include "peripheral/timer_interface.h"

/* CONSTANTS **************************************************************************************/

/* MACROS *****************************************************************************************/

/* TYPES ******************************************************************************************/


/** Available Power Modes for the LIS2DH12 */
typedef enum{
	LIS2DH12_POWER_NORMAL = 0,	/**< Normal Power Mode, 10-bit resoulution, 100Hz, 20uA */
	LIS2DH12_POWER_LOW,			    /**< Low Power Mode, 10-bit resolution, 1Hz, 2uA */
	LIS2DH12_POWER_HIGHRES,		  /**< High Power Mode, 12-bit resolution, 1344Hz, 185uA  */
	LIS2DH12_POWER_DOWN			    /**< Stop Operation */
} LIS2DH12_PowerMode;


/** Available Sampling Frequencies for the LIS2DH12 */
typedef enum{
  LIS2DH12_SAMPLE_1HZ = LIS2DH_ODR_MASK_1HZ,      /**< 1Hz,   2uA,  2uA,  2uA */
  LIS2DH12_SAMPLE_10HZ = LIS2DH_ODR_MASK_10HZ,    /**< 10Hz,  3uA,  4uA,  4uA */
  LIS2DH12_SAMPLE_25HZ = LIS2DH_ODR_MASK_25HZ,    /**< 25Hz,  4uA,  6uA,  6uA */
  LIS2DH12_SAMPLE_50HZ = LIS2DH_ODR_MASK_50HZ,    /**< 50Hz,  6uA,  11uA, 11uA */
  LIS2DH12_SAMPLE_100HZ = LIS2DH_ODR_MASK_100HZ,  /**< 100Hz, 10uA, 20uA, 20uA */
  LIS2DH12_SAMPLE_200HZ = LIS2DH_ODR_MASK_200HZ,  /**< 200Hz, 18uA, 38uA, 38uA */
  LIS2DH12_SAMPLE_400HZ = LIS2DH_ODR_MASK_400HZ,  /**< 400Hz, 36uA, 73uA, 185uA */
} LIS2DH12_SampleRate;

/** Available Interrupts */
typedef enum{
  LIS2DH12_INTERRUPT_PIN_1 = 0,
  LIS2DH12_INTERRUPT_PIN_2 = 1
}LIS2DH12_InterruptPinNumber;

/** Available Interrupts */
typedef enum{
  LIS2DH12_INTERRUPT_THRESHOLD_X = LIS2DH_XHIE_MASK,
  LIS2DH12_INTERRUPT_THRESHOLD_Y = LIS2DH_YHIE_MASK,
  LIS2DH12_INTERRUPT_THRESHOLD_Z = LIS2DH_ZHIE_MASK,
  LIS2DH12_INTERRUPT_THRESHOLD_XY = LIS2DH_XHIE_MASK | LIS2DH_YHIE_MASK,
  LIS2DH12_INTERRUPT_THRESHOLD_XZ = LIS2DH_XHIE_MASK | LIS2DH_ZHIE_MASK,
  LIS2DH12_INTERRUPT_THRESHOLD_YZ = LIS2DH_YHIE_MASK | LIS2DH_ZHIE_MASK,
  LIS2DH12_INTERRUPT_THRESHOLD_XYZ = LIS2DH_XYZ_HIE_MASK,
}LIS2DH12_InterruptThresholdMask;


/** Available Scales */
typedef enum{
	LIS2DH12_SCALE2G = 0,		/**< Scale Selection: +/- 2g */
	LIS2DH12_SCALE4G = 1,		/**< Scale Selection: +/- 4g */
	LIS2DH12_SCALE8G = 2,		/**< Scale Selection: +/- 8g */
	LIS2DH12_SCALE16G = 3		/**< Scale Selection: +/- 16g */
}LIS2DH12_Scale;

/** Data Ready Event Callback Type */
typedef void (*LIS2DH12_drdy_event_t)(void);

/* PROTOTYPES *************************************************************************************/

/**
 * Initialize the Acceleration Sensor
 *
 * This Function initializes the Acceleration Sensor LIS2DH12 to work with the SPI Interface. The
 * SPI Interface will be initialized by this function accordingly. This Function also set the
 * requested Power mode for the LIS2DH12. All Axis(x,y,z) will be enabled.
 * The Data Ready Callback Function can be used to get notified when new Data is available. But causen,
 * the callback will be called in interrupt context. 
 *
 * @param[in] powerMode Requested Power Mode the LIS2DH12 should work with.
 * @param[in] scale 	Scale, the Sensor shall operate.
 * @param[in] drdyCB 	Data Ready Callback, optional, pass over NULL if not used
 *
 * @retval LIS2DH12_RET_OK 			Init successful
 * @retval LIS2DH12_RET_ERROR 		Something went wrong
 * @retval LIS2DH12_NOT_SUPPORTED 	Requested powerMode or scale not yet supported
 */
extern LIS2DH12_Ret LIS2DH12_init(LIS2DH12_PowerMode powerMode, LIS2DH12_Scale scale, LIS2DH12_SampleRate sampleRate);


/**
 * Change power mode
 *
 * This function changes the current power mode the acceleration sensor is running. In some use cases it useful for battery saving to
 * run the sensor in a low power mode to detect the start of a movement and then switch to a higher resolution.
 *
 * Note: This function only works after correct initialization.
 * Note: After changeing the power mode, it needs some time till new values are available, see datasheet for details
 *
 * @param[in] powerMode Requested Power Mode the LIS2DH12 should work with.
 *
 * @retval LIS2DH12_RET_OK          Change successful
 * @retval LIS2DH12_RET_ERROR       Something went wrong
 *
 */
extern LIS2DH12_Ret LIS2DH12_setPowerMode(LIS2DH12_PowerMode powerMode);
extern LIS2DH12_Ret LIS2DH12_setSampleRate(LIS2DH12_SampleRate sampleRate);
extern LIS2DH12_Ret LIS2DH12_setScale(LIS2DH12_Scale scale);

extern LIS2DH12_Ret LIS2DH12_sample();

extern LIS2DH12_Ret LIS2DH12_enableHighPass();
extern LIS2DH12_Ret LIS2DH12_disableHighPass();
extern LIS2DH12_Ret LIS2DH12_setHighPassReference();
extern LIS2DH12_Ret LIS2DH12_enableTemperatureSensor();
extern LIS2DH12_Ret LIS2DH12_updateTemperatureSensor();
extern LIS2DH12_Ret LIS2DH12_getTemperature(int32_t* const temp);

extern LIS2DH12_Ret LIS2DH12_enableX();
extern LIS2DH12_Ret LIS2DH12_enableY();
extern LIS2DH12_Ret LIS2DH12_enableZ();
extern LIS2DH12_Ret LIS2DH12_enableXYZ();

extern LIS2DH12_Ret LIS2DH12_disableX();
extern LIS2DH12_Ret LIS2DH12_disableY();
extern LIS2DH12_Ret LIS2DH12_disableZ();

/**
 * Return X acceleration
 *
 * @param[out] accX Acceleration in mG
 *
 * @retval LIS2DH12_RET_OK 			Data valid
 * @retval LIS2DH12_RET_INVALID 	Data invalid because of power down or data not ready
 * @retval LIS2DH12_RET_NULL NULL 	Pointer detected
 */
extern LIS2DH12_Ret LIS2DH12_getXmG(int32_t* const accX);

/**
 * Return Y acceleration
 *
 * @param[out] accY Acceleration in mG
 *
 * @retval LIS2DH12_RET_OK 			Data valid
 * @retval LIS2DH12_RET_INVALID 	Data invalid because of power down or data not ready
 * @retval LIS2DH12_RET_NULL NULL 	Pointer detected
 */
extern LIS2DH12_Ret LIS2DH12_getYmG(int32_t* const accY);

/**
 * Return Z acceleration
 *
 * @param[out] accZ Acceleration in mG
 *
 * @retval LIS2DH12_RET_OK 			Data valid
 * @retval LIS2DH12_RET_INVALID 	Data invalid because of power down or data not ready
 * @retval LIS2DH12_RET_NULL NULL 	Pointer detected
 */
extern LIS2DH12_Ret LIS2DH12_getZmG(int32_t* const accZ);

/**
 * Return acceleration of all axis
 *
 * @param[out] accX Acceleration in mG
 * @param[out] accY Acceleration in mG
 * @param[out] accZ Acceleration in mG
 *
 * @retval LIS2DH12_RET_OK 			Data valid
 * @retval LIS2DH12_RET_INVALID 	Data invalid because of power down or data not ready
 * @retval LIS2DH12_RET_NULL NULL 	Pointer detected
 */
extern LIS2DH12_Ret LIS2DH12_getALLmG(int32_t* const accX, int32_t* const accY, int32_t* const accZ);


extern void LIS2DH12_clearInterrupts();

extern void LIS2DH12_setInterruptThreshold(uint16_t threshold);
extern void LIS2DH12_setInterruptDuration(uint8_t duration);
extern void LIS2DH12_setInterruptHandler(gpio_event_handler_t handler);
extern void LIS2DH12_initThresholdInterrupt(uint16_t threshold,
                                             uint8_t duration,
                                             LIS2DH12_InterruptThresholdMask intThreshMask,
                                             bool latchEnabled,
                                             gpio_event_handler_t handler);



extern void LIS2DH12_initDAPolling(app_timer_timeout_handler_t handler);
extern void LIS2DH12_startDAPolling();
extern void LIS2DH12_stopDAPolling();

#ifdef __cplusplus
}
#endif

#endif  /* LIS2DH12_H */
