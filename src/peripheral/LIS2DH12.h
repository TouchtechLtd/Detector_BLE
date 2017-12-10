/**
@addtogroup LIS2DH12Driver LIS2DH12 Acceleration Sensor Driver
@{
@file       LIS2DH12.h

Hardware Driver for the LIS2DH12 Acceleration Sensor

* @}
***************************************************************************************************/
#ifndef LIS2DH12_H
#define LIS2DH12_H


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


namespace LIS2DH12 {

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} acceleration_t;

typedef struct
{
    int8_t x;
    int8_t y;
    int8_t z;
} acceleration_8b_t;


/** Available Power Modes for the LIS2DH12 */
typedef enum{
	POWER_NORMAL = 0,	/**< Normal Power Mode, 10-bit resoulution, 100Hz, 20uA */
	POWER_LOW,			    /**< Low Power Mode, 10-bit resolution, 1Hz, 2uA */
	POWER_HIGHRES,		  /**< High Power Mode, 12-bit resolution, 1344Hz, 185uA  */
	POWER_DOWN			    /**< Stop Operation */
} power_mode_t;


/** Available Sampling Frequencies for the LIS2DH12 */
typedef enum{
  SAMPLE_1HZ = LIS2DH_ODR_MASK_1HZ,      /**< 1Hz,   2uA,  2uA,  2uA */
  SAMPLE_10HZ = LIS2DH_ODR_MASK_10HZ,    /**< 10Hz,  3uA,  4uA,  4uA */
  SAMPLE_25HZ = LIS2DH_ODR_MASK_25HZ,    /**< 25Hz,  4uA,  6uA,  6uA */
  SAMPLE_50HZ = LIS2DH_ODR_MASK_50HZ,    /**< 50Hz,  6uA,  11uA, 11uA */
  SAMPLE_100HZ = LIS2DH_ODR_MASK_100HZ,  /**< 100Hz, 10uA, 20uA, 20uA */
  SAMPLE_200HZ = LIS2DH_ODR_MASK_200HZ,  /**< 200Hz, 18uA, 38uA, 38uA */
  SAMPLE_400HZ = LIS2DH_ODR_MASK_400HZ,  /**< 400Hz, 36uA, 73uA, 185uA */
} sample_rate_t;

/** Available Interrupts */
typedef enum{
  INTERRUPT_1 = 0,
  INTERRUPT_2 = 1
} interrupt_number_t;

/** Available Interrupts */
typedef enum{
  INTERRUPT_THRESHOLD_X = LIS2DH_XHIE_MASK,
  INTERRUPT_THRESHOLD_Y = LIS2DH_YHIE_MASK,
  INTERRUPT_THRESHOLD_Z = LIS2DH_ZHIE_MASK,
  INTERRUPT_THRESHOLD_XY = LIS2DH_XHIE_MASK | LIS2DH_YHIE_MASK,
  INTERRUPT_THRESHOLD_XZ = LIS2DH_XHIE_MASK | LIS2DH_ZHIE_MASK,
  INTERRUPT_THRESHOLD_YZ = LIS2DH_YHIE_MASK | LIS2DH_ZHIE_MASK,
  INTERRUPT_THRESHOLD_XYZ = LIS2DH_XYZ_HIE_MASK,
  INTERRUPT_MOVEMENT_XH = LIS2DH_6D_MASK | LIS2DH_XHIE_MASK,
  INTERRUPT_MOVEMENT_XL = LIS2DH_6D_MASK | LIS2DH_XLIE_MASK,
  INTERRUPT_MOVEMENT_X =  LIS2DH_6D_MASK | LIS2DH_XLIE_MASK | LIS2DH_XHIE_MASK,
  INTERRUPT_MOVEMENT_YH = LIS2DH_6D_MASK | LIS2DH_YHIE_MASK,
  INTERRUPT_MOVEMENT_YL = LIS2DH_6D_MASK | LIS2DH_YLIE_MASK,
  INTERRUPT_MOVEMENT_Y =  LIS2DH_6D_MASK | LIS2DH_YLIE_MASK | LIS2DH_YHIE_MASK,
  INTERRUPT_MOVEMENT_ZH = LIS2DH_6D_MASK | LIS2DH_XHIE_MASK,
  INTERRUPT_MOVEMENT_ZL = LIS2DH_6D_MASK | LIS2DH_XLIE_MASK,
  INTERRUPT_MOVEMENT_Z =  LIS2DH_6D_MASK | LIS2DH_ZLIE_MASK | LIS2DH_ZHIE_MASK,
  INTERRUPT_POSITION_XH = LIS2DH_AOI_MASK | LIS2DH_6D_MASK | LIS2DH_XHIE_MASK,
  INTERRUPT_POSITION_XL = LIS2DH_AOI_MASK | LIS2DH_6D_MASK | LIS2DH_XLIE_MASK,
  INTERRUPT_POSITION_X =  LIS2DH_AOI_MASK | LIS2DH_6D_MASK | LIS2DH_XLIE_MASK | LIS2DH_XHIE_MASK,
  INTERRUPT_POSITION_YH = LIS2DH_AOI_MASK | LIS2DH_6D_MASK | LIS2DH_YHIE_MASK,
  INTERRUPT_POSITION_YL = LIS2DH_AOI_MASK | LIS2DH_6D_MASK | LIS2DH_YLIE_MASK,
  INTERRUPT_POSITION_Y =  LIS2DH_AOI_MASK | LIS2DH_6D_MASK | LIS2DH_YLIE_MASK | LIS2DH_YHIE_MASK,
  INTERRUPT_POSITION_ZH = LIS2DH_AOI_MASK | LIS2DH_6D_MASK | LIS2DH_XHIE_MASK,
  INTERRUPT_POSITION_ZL = LIS2DH_AOI_MASK | LIS2DH_6D_MASK | LIS2DH_XLIE_MASK,
  INTERRUPT_POSITION_Z =  LIS2DH_AOI_MASK | LIS2DH_6D_MASK | LIS2DH_ZLIE_MASK | LIS2DH_ZHIE_MASK,
} interrupt_mode_t;


typedef enum{
  INTERRUPT_SOURCE_XL = 0x01,
  INTERRUPT_SOURCE_XH = 0x02,
  INTERRUPT_SOURCE_YL = 0x04,
  INTERRUPT_SOURCE_YH = 0x08,
  INTERRUPT_SOURCE_ZL = 0x10,
  INTERRUPT_SOURCE_ZH = 0x20
} interrupt_source_trigger_t;


/** Available Scales */
typedef enum{
	SCALE2G = 0,		/**< Scale Selection: +/- 2g */
	SCALE4G = 1,		/**< Scale Selection: +/- 4g */
	SCALE8G = 2,		/**< Scale Selection: +/- 8g */
	SCALE16G = 3		/**< Scale Selection: +/- 16g */
} scale_t;

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
extern LIS2DH12_Ret init(power_mode_t powerMode, scale_t scale, sample_rate_t sampleRate);


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
extern LIS2DH12_Ret setPowerMode(power_mode_t powerMode);
extern LIS2DH12_Ret setSampleRate(sample_rate_t sampleRate);
extern LIS2DH12_Ret setScale(scale_t scale);

extern LIS2DH12_Ret sample();

extern LIS2DH12_Ret enableHighPass();
extern LIS2DH12_Ret disableHighPass();
extern LIS2DH12_Ret enableFIFO();
extern LIS2DH12_Ret setHighPassReference();
extern LIS2DH12_Ret enableTemperatureSensor();
extern LIS2DH12_Ret updateTemperatureSensor();
extern LIS2DH12_Ret getTemperature(int32_t* const temp);

extern LIS2DH12_Ret enableX();
extern LIS2DH12_Ret enableY();
extern LIS2DH12_Ret enableZ();
extern LIS2DH12_Ret enableXYZ();

extern LIS2DH12_Ret disableX();
extern LIS2DH12_Ret disableY();
extern LIS2DH12_Ret disableZ();

/**
 * Return X acceleration
 *
 * @param[out] accX Acceleration in mG
 *
 * @retval LIS2DH12_RET_OK 			Data valid
 * @retval LIS2DH12_RET_INVALID 	Data invalid because of power down or data not ready
 * @retval LIS2DH12_RET_NULL NULL 	Pointer detected
 */
extern LIS2DH12_Ret getXmG(int32_t* const accX);

/**
 * Return Y acceleration
 *
 * @param[out] accY Acceleration in mG
 *
 * @retval LIS2DH12_RET_OK 			Data valid
 * @retval LIS2DH12_RET_INVALID 	Data invalid because of power down or data not ready
 * @retval LIS2DH12_RET_NULL NULL 	Pointer detected
 */
extern LIS2DH12_Ret getYmG(int32_t* const accY);

/**
 * Return Z acceleration
 *
 * @param[out] accZ Acceleration in mG
 *
 * @retval LIS2DH12_RET_OK 			Data valid
 * @retval LIS2DH12_RET_INVALID 	Data invalid because of power down or data not ready
 * @retval LIS2DH12_RET_NULL NULL 	Pointer detected
 */
extern LIS2DH12_Ret getZmG(int32_t* const accZ);

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
extern LIS2DH12_Ret getALLmG(int32_t* const accX, int32_t* const accY, int32_t* const accZ);

extern LIS2DH12_Ret getAccelerationData(acceleration_t* accData);
extern LIS2DH12_Ret getAccelerationData(acceleration_8b_t* accData);


extern void clearInterrupts();

extern void setInterruptThreshold(uint16_t threshold, interrupt_number_t intNum);
extern void setInterruptDuration(uint8_t duration,    interrupt_number_t intNum);
extern void setInterruptHandler(gpio_event_handler_t handler);
extern void clearInterruptHandler();

extern void initThresholdInterrupt(uint16_t threshold,
                                   uint8_t duration,
                                   interrupt_number_t intNum,
                                   interrupt_mode_t mode,
                                   bool latchEnabled,
                                   gpio_event_handler_t handler);

extern uint8_t getInterruptSource(interrupt_number_t intNum);


extern void initDAPolling(app_timer_timeout_handler_t handler);
extern void startDAPolling();
extern void stopDAPolling();


} // namespace LIS2DH12


#endif  /* LIS2DH12_H */
