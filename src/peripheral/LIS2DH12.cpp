/**
@addtogroup LIS2DH12Driver LIS2DH12 Acceleration Sensor Driver
@{
@file       LIS2DH12.c

Implementation of the LIS2DH12 driver.

For a detailed description see the detailed description in @ref LIS2DH12.h

* @}
***************************************************************************************************/

/* INCLUDES ***************************************************************************************/
#include "LIS2DH12_registers.h"
#include "LIS2DH12.h"
#include "peripheral/spi.h"
#include "nrf.h"
#include "bsp.h"
#include "boards.h"

#include "nrf_delay.h"

#include "debug/DEBUG.h"
#include "peripheral/timer_interface.h"


/* CONSTANTS **************************************************************************************/
/** Maximum Size of SPI Addresses */
#define ADR_MAX 0x3FU

/** Number of maximum SPI Transfer retries */
#define RETRY_MAX 3U

/** Size of raw sensor data for all 3 axis */
#define SENSOR_DATA_SIZE 6U

/** Max number of registers to read at once. To read all axis at once, 6bytes are neccessary */
#define READ_MAX SENSOR_DATA_SIZE

/** Bit Mask to set read bit for SPI Transfer */
#define SPI_READ 0x80U

/** Bit Mask to enable auto address incrementation for multi read */
#define SPI_ADR_INC 0x40U

/** Max */
#define MAX_INTERRUPT_DURATION 0x7FU

/** Max */
#define MAX_INTERRUPT_THRESHOLD 0x7FU

/* MACROS *****************************************************************************************/

#define GET_SAMPLERATE(samplerate) sampleRateLookup[(samplerate >> 4) - 1]

/* TYPES ******************************************************************************************/
/** Structure containing sensor data for all 3 axis */
typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} acceleration_t;

/** Union to split raw data to values for each axis */
typedef union
{
    uint8_t raw[SENSOR_DATA_SIZE];
    acceleration_t sensor;
} sensor_buffer_t;


static const uint16_t sampleRateLookup[] =
{
    1, 10, 25, 50, 100, 200, 400
};


/* PROTOTYPES *************************************************************************************/

static LIS2DH12_Ret selftest(void);


/* VARIABLES **************************************************************************************/

static sensor_buffer_t g_sensorData;                    /**< Union to covert raw data to value for each axis */
static LIS2DH12_PowerMode g_powerMode = LIS2DH12_POWER_DOWN; /**< Current power mode */
static LIS2DH12_Scale g_scale = LIS2DH12_SCALE2G;       /**< Selected scale */
static LIS2DH12_SampleRate g_sampleRate = LIS2DH12_SAMPLE_1HZ; /**< Current power mode */
static uint16_t g_sampleRateValue = 0;
static uint8_t g_mgpb = 1;                              /**< milli-g per bit */
static uint8_t g_resolution = 10;                        /**< milli-g nb of bits */

static Timer g_daTimer;
static app_timer_timeout_handler_t g_daHandler;
/* EXTERNAL FUNCTIONS *****************************************************************************/


extern LIS2DH12_Ret LIS2DH12_init(LIS2DH12_PowerMode powerMode, LIS2DH12_Scale scale, LIS2DH12_SampleRate sampleRate)
{
    int retVal = LIS2DH12_RET_OK;

    /* Initialize SPI */
    if (spi_isInitialized() == false)
    {
        spi_init();
    }

    /* Start Selftest */
    retVal |= selftest();

    LIS2DH12_clearRegisters();

    if (LIS2DH12_RET_OK == retVal)
    {

        LIS2DH12_setScale(scale);
        /* Set Power Mode */
        LIS2DH12_setPowerMode(powerMode);
        LIS2DH12_setSampleRate(sampleRate);
        LIS2DH12_enableXYZ();
    }

    return (LIS2DH12_Ret)retVal;
}

extern LIS2DH12_Ret LIS2DH12_sample()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  err_code = readRegister(LIS2DH_OUT_X_L, g_sensorData.raw, SENSOR_DATA_SIZE);
  return err_code;
}


extern LIS2DH12_Ret LIS2DH12_setPowerMode(LIS2DH12_PowerMode powerMode)
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  clearRegister(LIS2DH_CTRL_REG1, LIS2DH_LPEN_MASK);
  clearRegister(LIS2DH_CTRL_REG4, LIS2DH_HR_MASK);

  switch(powerMode)
  {
    case LIS2DH12_POWER_NORMAL:
        g_mgpb = 4;         // 4mg per bits at normal power/2g, adjust by scaling
        g_resolution = 10;
        break;
    case LIS2DH12_POWER_LOW:
        setRegister(LIS2DH_CTRL_REG1, LIS2DH_LPEN_MASK);
        g_mgpb = 16;        // 16mg per bits in low power/2g, adjust by scaling
        g_resolution = 8;
        break;
    case LIS2DH12_POWER_HIGHRES:
        setRegister(LIS2DH_CTRL_REG4, LIS2DH_HR_MASK);
        g_mgpb = 1;         // 1 mg bits per mg at high power/2g, adjust by scaling
        g_resolution = 12;
        break;
    case LIS2DH12_POWER_DOWN:
        clearRegister(LIS2DH_CTRL_REG1, LIS2DH_CLEAR_REGISTER_MASK);
        break;
  }

  /* save power mode to check in get functions if power is enabled */
  g_powerMode = powerMode;

  return err_code;

}
extern LIS2DH12_Ret LIS2DH12_setSampleRate(LIS2DH12_SampleRate sampleRate)
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;

  clearRegister(LIS2DH_CTRL_REG1, LIS2DH_ODR_MASK_CLEAR);
  setRegister(LIS2DH_CTRL_REG1, sampleRate);

  g_sampleRate = sampleRate;
  g_sampleRateValue = GET_SAMPLERATE(sampleRate);

  return err_code;

}
extern LIS2DH12_Ret LIS2DH12_setScale(LIS2DH12_Scale scale)
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;

  g_scale = scale;
  uint8_t ctrl4RegVal = ((uint8_t)g_scale)<<4U;
  clearRegister(LIS2DH_CTRL_REG4 , LIS2DH_FS_MASK);
  setRegister(LIS2DH_CTRL_REG4, ctrl4RegVal);

  return err_code;
}


extern LIS2DH12_Ret LIS2DH12_enableHighPass()
{
  uint8_t err_code = LIS2DH12_RET_OK;
  err_code = setRegister(LIS2DH_CTRL_REG2, LIS2DH_FDS_MASK);
  err_code |= LIS2DH12_setHighPassReference();
  return (LIS2DH12_Ret) err_code;
}
extern LIS2DH12_Ret LIS2DH12_disableHighPass()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  err_code = clearRegister(LIS2DH_CTRL_REG2, LIS2DH_FDS_MASK);
  return err_code;
}
extern LIS2DH12_Ret LIS2DH12_setHighPassReference()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  // Read REFERENCE
  uint8_t dummyRead[1];
  err_code = readRegister(LIS2DH_REFERENCE, dummyRead, 1);

  return err_code;
}


extern LIS2DH12_Ret LIS2DH12_enableX()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  err_code = setRegister(LIS2DH_CTRL_REG1, LIS2DH_X_EN_MASK);
  return err_code;
}
extern LIS2DH12_Ret LIS2DH12_enableY()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  err_code = setRegister(LIS2DH_CTRL_REG1, LIS2DH_Y_EN_MASK);
  return err_code;
}
extern LIS2DH12_Ret LIS2DH12_enableZ()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  err_code = setRegister(LIS2DH_CTRL_REG1, LIS2DH_Z_EN_MASK);
  return err_code;
}
extern LIS2DH12_Ret LIS2DH12_enableXYZ()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  err_code = setRegister(LIS2DH_CTRL_REG1, LIS2DH_XYZ_EN_MASK);
  return err_code;
}
extern LIS2DH12_Ret LIS2DH12_disableX()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  err_code = clearRegister(LIS2DH_CTRL_REG1, LIS2DH_X_EN_MASK);
  return err_code;
}
extern LIS2DH12_Ret LIS2DH12_disableY()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  err_code = clearRegister(LIS2DH_CTRL_REG1, LIS2DH_Y_EN_MASK);
  return err_code;
}
extern LIS2DH12_Ret LIS2DH12_disableZ()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  err_code = clearRegister(LIS2DH_CTRL_REG1, LIS2DH_Z_EN_MASK);
  return err_code;
}



extern LIS2DH12_Ret LIS2DH12_getXmG(int32_t* const accX)
{
  if (accX == NULL) { return LIS2DH12_RET_NULL; }

  //Scale value, note: values from accelerometer are 16-bit 2's complement left-justified in all cases. "Extra" LSBs will be noise
  //Add 32768 (1<<(16-1) to get positive, shift, substract (1<<(resolution-1), scale voila!
  *accX = (((32768+g_sensorData.sensor.x)>>(16-g_resolution))-(1<<(g_resolution-1)))*(g_mgpb << g_scale) ;

  return LIS2DH12_RET_OK;
}

extern LIS2DH12_Ret LIS2DH12_getYmG(int32_t* const accY)
{
  if (accY == NULL) { return LIS2DH12_RET_NULL; }

  //Scale value, note: values from accelerometer are 16-bit 2's complement left-justified in all cases. "Extra" LSBs will be noise
  //Add 32768 (1<<(16-1) to get positive, shift, substract (1<<(resolution-1), scale voila!
  *accY = (((32768+g_sensorData.sensor.y)>>(16-g_resolution))-(1<<(g_resolution-1)))*(g_mgpb << g_scale) ;

  return LIS2DH12_RET_OK;
}

extern LIS2DH12_Ret LIS2DH12_getZmG(int32_t* const accZ)
{
  if (accZ == NULL) { return LIS2DH12_RET_NULL; }

  //Scale value, note: values from accelerometer are 16-bit 2's complement left-justified in all cases. "Extra" LSBs will be noise
  //Add 32768 (1<<(16-1) to get positive, shift, substract (1<<(resolution-1), scale voila!
  *accZ = (((32768+g_sensorData.sensor.z)>>(16-g_resolution))-(1<<(g_resolution-1)))*(g_mgpb << g_scale) ;

  return LIS2DH12_RET_OK;
}

extern LIS2DH12_Ret LIS2DH12_getALLmG(int32_t* const accX, int32_t* const accY, int32_t* const accZ)
{

    if ((NULL == accX) || (NULL == accY) || (NULL == accZ))
    {
        return LIS2DH12_RET_NULL;
    }

    LIS2DH12_getXmG(accX);
    LIS2DH12_getYmG(accY);
    LIS2DH12_getZmG(accZ);

    return LIS2DH12_RET_OK;
}


/* INTERRUPT FUNCTIONS *****************************************************************************/



static uint8_t findInterruptDuration(uint16_t intDuration_ms)
{
  float lsbValue = 1000 / g_sampleRateValue;
  uint8_t outputValue = (uint8_t) (intDuration_ms / lsbValue);

  if (outputValue > MAX_INTERRUPT_DURATION) { outputValue = MAX_INTERRUPT_DURATION; }

  return outputValue;
}

static uint8_t findInterruptThreshold(uint16_t intThreshold_mg)
{
  uint16_t scaleValue_mg = ((2 << g_scale));
  uint16_t thresholdPercent = intThreshold_mg/scaleValue_mg;

  if (thresholdPercent > 999) { thresholdPercent = 999; }

  uint8_t outputValue = (uint8_t) (thresholdPercent * (MAX_INTERRUPT_THRESHOLD + 1) / 1000);

  return outputValue;
}


extern void LIS2DH12_clearInterrupts()
{
  uint8_t dummyRead[1];
  readRegister(LIS2DH_INT1_SOURCE, dummyRead, 1);
  readRegister(LIS2DH_INT2_SOURCE, dummyRead, 1);
}




extern void LIS2DH12_setInterruptThreshold(uint16_t threshold)
{
  // Set threshold on INT1
  uint8_t intThreshold = findInterruptThreshold(threshold);
  DEBUG("Threshold: 0x%02x", intThreshold);
  clearRegister(LIS2DH_INT1_THS, LIS2DH_CLEAR_REGISTER_MASK);
  setRegister(LIS2DH_INT1_THS, intThreshold);
}


extern void LIS2DH12_setInterruptDuration(uint8_t duration)
{
  // Set duration on INT1
  uint8_t intDuration = findInterruptDuration(duration);
  DEBUG("Duration: 0x%02x", intDuration);
  clearRegister(LIS2DH_INT1_DURATION, LIS2DH_CLEAR_REGISTER_MASK);
  setRegister(LIS2DH_INT1_DURATION, intDuration);
}


extern void LIS2DH12_setInterruptHandler(gpio_event_handler_t handler)
{
#ifdef BOARD_RUUVITAG_B3
  if (GPIO::interruptIsSet(INT_ACC2_PIN))
  {
    GPIO::interruptClear(INT_ACC2_PIN);
  }

  GPIO::initIntInput(INT_ACC2_PIN,
          NRF_GPIOTE_POLARITY_LOTOHI,
          NRF_GPIO_PIN_NOPULL,
          false,
          false,
          handler);


  GPIO::interruptEnable(INT_ACC2_PIN);
#endif
}


extern void LIS2DH12_initThresholdInterrupt(uint16_t threshold,
                                             uint8_t duration,
                                             LIS2DH12_InterruptThresholdMask intThreshMask,
                                             bool latchEnabled,
                                             gpio_event_handler_t handler)
{

  clearRegister(LIS2DH_INT1_CFG, LIS2DH_CLEAR_REGISTER_MASK);

  // Enable HPF on INT1
  setRegister(LIS2DH_CTRL_REG2, LIS2DH_HPIS1_MASK);
  LIS2DH12_setHighPassReference();

  if (latchEnabled) {
    // Enable/ Disable Latch on INT1
    setRegister(LIS2DH_CTRL_REG5, LIS2DH_LIR_INT1_MASK);
  }

  LIS2DH12_setInterruptThreshold(threshold);
  LIS2DH12_setInterruptDuration(duration);

  // Set cfg on INT1
  setRegister(LIS2DH_INT1_CFG, intThreshMask);

  //Enable Interrupt on INT1
  setRegister(LIS2DH_CTRL_REG6, LIS2DH_I1_IA1);

  LIS2DH12_setInterruptHandler(handler);

}


extern void LIS2DH12_initDAPolling(app_timer_timeout_handler_t handler)
{
#ifdef BOARD_RUUVITAG_B3
  GPIO::setInput(INT_ACC1_PIN);
#endif
  g_daHandler = handler;

  setRegister(LIS2DH_CTRL_REG3, LIS2DH_I1_DRDY2);
}

extern void LIS2DH12_startDAPolling()
{
  g_daTimer.startTimer(1000/ g_sampleRateValue, g_daHandler);
}


extern void LIS2DH12_stopDAPolling()
{
  g_daTimer.killTimer();
}


/* INTERNAL FUNCTIONS *****************************************************************************/

/**
 * Execute LIS2DH12 Selftest
 *
 * @return LIS2DH12_RET_OK Selftest passed
 * @return LIS2DH12_RET_ERROR_SELFTEST Selftest failed
 */
static LIS2DH12_Ret selftest(void)
{
    uint8_t value[1] = {0};
    readRegister(LIS2DH_WHO_AM_I, value, 1);
    return (LIS2DH_I_AM_MASK == value[0]) ? LIS2DH12_RET_OK : LIS2DH12_RET_ERROR;
}










