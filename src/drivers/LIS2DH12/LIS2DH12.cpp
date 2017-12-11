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
#include "nrf.h"
#include "bsp.h"
#include "boards.h"

#include "nrf_delay.h"

#include "libraries/debug/DEBUG.h"
#include "drivers/spi/spi.h"
#include "drivers/timer/timer_interface.h"

#define NRF_LOG_MODULE_NAME LIS2DH12
NRF_LOG_MODULE_REGISTER();


namespace LIS2DH12 {

/* CONSTANTS **************************************************************************************/
/** Maximum Size of SPI Addresses */
#define ADR_MAX 0x3FU

/** Number of maximum SPI Transfer retries */
#define RETRY_MAX 3U

/** Size of raw sensor data for all 3 axis */
#define SENSOR_DATA_SIZE 6U

/** Temperature reference */
#define TEMPERATURE_REFERENCE 10

/** Size of raw temperature data size */
#define TEMPERATURE_DATA_SIZE 2U

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

#ifdef BOARD_RUUVITAG_B3

#define INT1_PIN INT_ACC1_PIN
#define INT2_PIN INT_ACC2_PIN

#else

#define INT1_PIN 0
#define INT2_PIN 0

#endif

/* MACROS *****************************************************************************************/

#define GET_SAMPLERATE(samplerate) sampleRateLookup[(samplerate >> 4) - 1]

/* TYPES ******************************************************************************************/
/** Structure containing sensor data for all 3 axis */

/** Union to split raw data to values for each axis */
typedef union
{
    uint8_t raw[SENSOR_DATA_SIZE];
    acceleration_t sensor;
} sensor_buffer_t;

typedef union
{
  uint8_t raw[TEMPERATURE_DATA_SIZE];
  int16_t temp;
} temperature_t;


static const uint16_t sampleRateLookup[] =
{
    1, 10, 25, 50, 100, 200, 400
};

typedef struct
{
  uint8_t highpassMask;
  uint8_t latchMask;
  uint8_t cfgRegister;
  uint8_t srcRegister;
  uint8_t thresholdRegister;
  uint8_t durationRegister;
  uint8_t enableMask;
  uint8_t enableRegister;
  uint8_t interruptPin;
}interrupt_lookup_t;

interrupt_lookup_t interrupt1 =
{
    LIS2DH_HPIS1_MASK,
    LIS2DH_LIR_INT1_MASK,
    LIS2DH_INT1_CFG,
    LIS2DH_INT1_SOURCE,
    LIS2DH_INT1_THS,
    LIS2DH_INT1_DURATION,
    LIS2DH_I1_IA1,
    LIS2DH_CTRL_REG3,
    INT1_PIN
};

interrupt_lookup_t interrupt2 =
{
    LIS2DH_HPIS2_MASK,
    LIS2DH_LIR_INT2_MASK,
    LIS2DH_INT2_CFG,
    LIS2DH_INT2_SOURCE,
    LIS2DH_INT2_THS,
    LIS2DH_INT2_DURATION,
    LIS2DH_I2_IA2,
    LIS2DH_CTRL_REG6,
    INT2_PIN
};

interrupt_lookup_t interrupts[2] = { interrupt1, interrupt2 };


/* PROTOTYPES *************************************************************************************/

static LIS2DH12_Ret selftest(void);


/* VARIABLES **************************************************************************************/

static sensor_buffer_t g_sensorData;                    /**< Union to covert raw data to value for each axis */
static temperature_t   g_temperatureData;
static power_mode_t g_powerMode   = POWER_DOWN; /**< Current power mode */
static scale_t g_scale            = SCALE2G;       /**< Selected scale */
static sample_rate_t g_sampleRate = SAMPLE_1HZ; /**< Current power mode */
static uint16_t g_sampleRateValue = 0;
static uint8_t g_mgpb = 1;                              /**< milli-g per bit */
static uint8_t g_resolution = 10;                        /**< milli-g nb of bits */

static Timer g_daTimer;
static app_timer_timeout_handler_t g_daHandler;

static bool isDAPolling = false;
/* EXTERNAL FUNCTIONS *****************************************************************************/


extern LIS2DH12_Ret init(power_mode_t powerMode, scale_t scale, sample_rate_t sampleRate)
{
    int retVal = LIS2DH12_RET_OK;

    /* Initialize SPI */
    if (spi_isInitialized() == false)
    {
        spi_init();
    }

    // Start Selftest
    retVal |= selftest();

    clearRegisters();

    if (LIS2DH12_RET_OK == retVal)
    {

        setScale(scale);
        setPowerMode(powerMode);
        setSampleRate(sampleRate);
        enableXYZ();
    }

    return (LIS2DH12_Ret)retVal;
}

extern LIS2DH12_Ret sample()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  err_code = readRegister(LIS2DH_OUT_X_L, g_sensorData.raw, SENSOR_DATA_SIZE);
  return err_code;
}


extern LIS2DH12_Ret setPowerMode(power_mode_t powerMode)
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  clearRegister(LIS2DH_CTRL_REG1, LIS2DH_LPEN_MASK);
  clearRegister(LIS2DH_CTRL_REG4, LIS2DH_HR_MASK);

  switch(powerMode)
  {
    case POWER_NORMAL:
        INFO("INTIALISING - Power Mode: Normal");
        g_mgpb = 4;         // 4mg per bits at normal power/2g, adjust by scaling
        g_resolution = 10;
        break;
    case POWER_LOW:
        INFO("INTIALISING - Power Mode: Low Power");
        setRegister(LIS2DH_CTRL_REG1, LIS2DH_LPEN_MASK);
        g_mgpb = 16;        // 16mg per bits in low power/2g, adjust by scaling
        g_resolution = 8;
        break;
    case POWER_HIGHRES:
        INFO("INTIALISING - Power Mode: High Resolution");
        setRegister(LIS2DH_CTRL_REG4, LIS2DH_HR_MASK);
        g_mgpb = 1;         // 1 mg bits per mg at high power/2g, adjust by scaling
        g_resolution = 12;
        break;
    case POWER_DOWN:
        INFO("INTIALISING - Power Mode: Power Down");
        clearRegister(LIS2DH_CTRL_REG1, LIS2DH_CLEAR_REGISTER_MASK);
        break;
  }

  /* save power mode to check in get functions if power is enabled */
  g_powerMode = powerMode;

  return err_code;

}
extern LIS2DH12_Ret setSampleRate(sample_rate_t sampleRate)
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;

  clearRegister(LIS2DH_CTRL_REG1, LIS2DH_ODR_MASK_CLEAR);
  setRegister(LIS2DH_CTRL_REG1, sampleRate);

  g_sampleRate = sampleRate;
  g_sampleRateValue = GET_SAMPLERATE(sampleRate);

  INFO("INTIALISING - Sample Rate: %d", g_sampleRateValue);

  return err_code;

}
extern LIS2DH12_Ret setScale(scale_t scale)
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;

  g_scale = scale;

  uint8_t ctrl4RegVal = ((uint8_t)g_scale)<<4U;
  clearRegister(LIS2DH_CTRL_REG4 , LIS2DH_FS_MASK);
  setRegister(LIS2DH_CTRL_REG4, ctrl4RegVal);

  INFO("INTIALISING - Scale: %d", g_scale);

  return err_code;
}


extern LIS2DH12_Ret enableHighPass()
{
  uint8_t err_code = LIS2DH12_RET_OK;
  err_code = setRegister(LIS2DH_CTRL_REG2, LIS2DH_FDS_MASK);
  err_code |= setHighPassReference();
  return (LIS2DH12_Ret) err_code;
}
extern LIS2DH12_Ret disableHighPass()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  err_code = clearRegister(LIS2DH_CTRL_REG2, LIS2DH_FDS_MASK);
  return err_code;
}
extern LIS2DH12_Ret setHighPassReference()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  // Read REFERENCE
  uint8_t dummyRead[1];
  err_code = readRegister(LIS2DH_REFERENCE, dummyRead, 1);

  return err_code;
}


extern LIS2DH12_Ret enableFIFO()
{
  uint8_t err_code = LIS2DH12_RET_OK;
  err_code = setRegister(LIS2DH_CTRL_REG5, LIS2DH_FIFO_EN_MASK);
  err_code = setRegister(LIS2DH_FIFO_CTRL_REG, LIS2DH_FIFO_MODE_STREAM);

  return (LIS2DH12_Ret) err_code;
}


extern LIS2DH12_Ret enableTemperatureSensor()
{
  uint8_t err_code = LIS2DH12_RET_OK;
  err_code = setRegister(LIS2DH_TEMP_CFG_REG, LIS2DH_TEMP_EN_MASK);
  err_code = setRegister(LIS2DH_CTRL_REG4, LIS2DH_BDU_MASK);

  return (LIS2DH12_Ret) err_code;
}

extern LIS2DH12_Ret updateTemperatureSensor()
{
  uint8_t err_code = LIS2DH12_RET_OK;

  err_code = readRegister(LIS2DH_OUT_TEMP_L, g_temperatureData.raw, TEMPERATURE_DATA_SIZE);
  return (LIS2DH12_Ret) err_code;
}


extern LIS2DH12_Ret getTemperature(int32_t* const temp)
{
  if (temp == NULL) { return LIS2DH12_RET_NULL; }

  //Scale value, note: values from accelerometer are 16-bit 2's complement left-justified in all cases. "Extra" LSBs will be noise
  //Add 32768 (1<<(16-1)) to get positive, shift, substract (1<<(resolution-1), scale voila!
  *temp = TEMPERATURE_REFERENCE + (((32768+g_temperatureData.temp)>>(16-g_resolution))-(1<<(g_resolution-1))) ;

  return LIS2DH12_RET_OK;
}



extern LIS2DH12_Ret enableX()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  err_code = setRegister(LIS2DH_CTRL_REG1, LIS2DH_X_EN_MASK);
  return err_code;
}
extern LIS2DH12_Ret enableY()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  err_code = setRegister(LIS2DH_CTRL_REG1, LIS2DH_Y_EN_MASK);
  return err_code;
}
extern LIS2DH12_Ret enableZ()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  err_code = setRegister(LIS2DH_CTRL_REG1, LIS2DH_Z_EN_MASK);
  return err_code;
}
extern LIS2DH12_Ret enableXYZ()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  err_code = setRegister(LIS2DH_CTRL_REG1, LIS2DH_XYZ_EN_MASK);
  return err_code;
}
extern LIS2DH12_Ret disableX()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  err_code = clearRegister(LIS2DH_CTRL_REG1, LIS2DH_X_EN_MASK);
  return err_code;
}
extern LIS2DH12_Ret disableY()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  err_code = clearRegister(LIS2DH_CTRL_REG1, LIS2DH_Y_EN_MASK);
  return err_code;
}
extern LIS2DH12_Ret disableZ()
{
  LIS2DH12_Ret err_code = LIS2DH12_RET_OK;
  err_code = clearRegister(LIS2DH_CTRL_REG1, LIS2DH_Z_EN_MASK);
  return err_code;
}



extern LIS2DH12_Ret getXmG(int32_t* const accX)
{
  if (accX == NULL) { return LIS2DH12_RET_NULL; }

  //Scale value, note: values from accelerometer are 16-bit 2's complement left-justified in all cases. "Extra" LSBs will be noise
  //Add 32768 (1<<(16-1) to get positive, shift, substract (1<<(resolution-1), scale voila!
  *accX = (((32768+g_sensorData.sensor.x)>>(16-g_resolution))-(1<<(g_resolution-1)))*(g_mgpb << g_scale) ;

  return LIS2DH12_RET_OK;
}

extern LIS2DH12_Ret getYmG(int32_t* const accY)
{
  if (accY == NULL) { return LIS2DH12_RET_NULL; }

  //Scale value, note: values from accelerometer are 16-bit 2's complement left-justified in all cases. "Extra" LSBs will be noise
  //Add 32768 (1<<(16-1) to get positive, shift, substract (1<<(resolution-1), scale voila!
  *accY = (((32768+g_sensorData.sensor.y)>>(16-g_resolution))-(1<<(g_resolution-1)))*(g_mgpb << g_scale) ;

  return LIS2DH12_RET_OK;
}

extern LIS2DH12_Ret getZmG(int32_t* const accZ)
{
  if (accZ == NULL) { return LIS2DH12_RET_NULL; }

  //Scale value, note: values from accelerometer are 16-bit 2's complement left-justified in all cases. "Extra" LSBs will be noise
  //Add 32768 (1<<(16-1) to get positive, shift, substract (1<<(resolution-1), scale voila!
  *accZ = (((32768+g_sensorData.sensor.z)>>(16-g_resolution))-(1<<(g_resolution-1)))*(g_mgpb << g_scale) ;

  return LIS2DH12_RET_OK;
}

extern void accToMgConverter(int16_t acc, int16_t* mgOut)
{
  int32_t tmpMg = (((32768+acc)>>(16-g_resolution))-(1<<(g_resolution-1)))*(g_mgpb << g_scale) ;
  *mgOut = tmpMg;
}

extern LIS2DH12_Ret getALLmG(int32_t* const accX, int32_t* const accY, int32_t* const accZ)
{

    if ((NULL == accX) || (NULL == accY) || (NULL == accZ))
    {
        return LIS2DH12_RET_NULL;
    }

    getXmG(accX);
    getYmG(accY);
    getZmG(accZ);

    return LIS2DH12_RET_OK;
}

extern LIS2DH12_Ret getAccelerationData(acceleration_t* const p_accData)
{
  if (p_accData == NULL) { return LIS2DH12_RET_NULL; }

  accToMgConverter(g_sensorData.sensor.x, &p_accData->x);
  accToMgConverter(g_sensorData.sensor.y, &p_accData->y);
  accToMgConverter(g_sensorData.sensor.z, &p_accData->z);

  return LIS2DH12_RET_OK;
}


extern LIS2DH12_Ret getAccelerationData(acceleration_8b_t* const p_accData)
{

  if (p_accData == NULL) { return LIS2DH12_RET_NULL; }

  int16_t tmp_accX, tmp_accY, tmp_accZ = 0;

  accToMgConverter(g_sensorData.sensor.x, &tmp_accX);
  accToMgConverter(g_sensorData.sensor.y, &tmp_accY);
  accToMgConverter(g_sensorData.sensor.z, &tmp_accZ);

  p_accData->x = tmp_accX >> 5;
  p_accData->y = tmp_accY >> 5;
  p_accData->z = tmp_accZ >> 5;

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


extern void clearInterrupts()
{
  uint8_t dummyRead[1];
  readRegister(LIS2DH_INT1_SOURCE, dummyRead, 1);
  readRegister(LIS2DH_INT2_SOURCE, dummyRead, 1);
}




extern void setInterruptThreshold(uint16_t threshold, interrupt_number_t intNum)
{
  interrupt_lookup_t interrupt = interrupts[intNum];

  // Set threshold on selected interrupt
  uint8_t intThreshold = findInterruptThreshold(threshold);
  INFO("SETTING - Threshold Value: 0x%02x", intThreshold);
  clearRegister(interrupt.thresholdRegister, LIS2DH_CLEAR_REGISTER_MASK);
  setRegister  (interrupt.thresholdRegister, intThreshold);
}


extern void setInterruptDuration(uint8_t duration, interrupt_number_t intNum)
{
  interrupt_lookup_t interrupt = interrupts[intNum];

  // Set duration on selected interrupt
  uint8_t intDuration = findInterruptDuration(duration);
  INFO("SETTING - Threshold Duration: 0x%02x on Interrupt: ", intDuration, intNum + 1);
  clearRegister(interrupt.durationRegister, LIS2DH_CLEAR_REGISTER_MASK);
  setRegister  (interrupt.durationRegister, intDuration);
}


extern void setInterruptHandler(gpio_event_handler_t handler, uint32_t pin)
{
#ifdef BOARD_RUUVITAG_B3

  GPIO::interruptClear(pin);

  GPIO::initIntInput(pin,
          NRF_GPIOTE_POLARITY_LOTOHI,
          NRF_GPIO_PIN_PULLDOWN,
          false,
          false,
          handler);

  nrf_delay_ms(100);

  GPIO::interruptEnable(pin);
#endif
}



extern void initThresholdInterrupt(uint16_t threshold,
                                   uint8_t duration,
                                   interrupt_number_t intNum,
                                   interrupt_mode_t mode,
                                   bool latchEnabled,
                                   gpio_event_handler_t handler)
{

  interrupt_lookup_t interrupt = interrupts[intNum];

  clearRegister(interrupt.cfgRegister, LIS2DH_CLEAR_REGISTER_MASK);

  if (!(mode & (LIS2DH_AOI_MASK | LIS2DH_6D_MASK)))
  {
    // Enable HPF on INT1
    setRegister(LIS2DH_CTRL_REG2, interrupt.highpassMask);
    setHighPassReference();
  }


  if (latchEnabled) {
    // Enable/ Disable Latch on INT1
    setRegister(LIS2DH_CTRL_REG5, interrupt.latchMask);
  }

  setInterruptThreshold(threshold, intNum);
  setInterruptDuration(duration, intNum);

  // Set cfg on INT1
  setRegister(interrupt.cfgRegister, mode);

  //Enable Interrupt on INT1
  setRegister(interrupt.enableRegister, interrupt.enableMask);

  if (handler != NULL)
  {
    setInterruptHandler(handler, interrupt.interruptPin);
  }

}




extern uint8_t getInterruptSource(interrupt_number_t intNum)
{
  interrupt_lookup_t interrupt = interrupts[intNum];

  uint8_t sourceData[1];
  readRegister(interrupt.srcRegister, sourceData, 1);
  return sourceData[0] & 0x3F;
}


extern void initDAPolling(app_timer_timeout_handler_t handler)
{
  /*
#ifdef BOARD_RUUVITAG_B3
  GPIO::setInput(INT_ACC1_PIN);
#endif
*/
  g_daHandler = handler;

  //setRegister(LIS2DH_CTRL_REG3, LIS2DH_I1_DRDY2);
}

extern void startDAPolling()
{
  if (!isDAPolling)
  {
  g_daTimer.startTimer(1000/ g_sampleRateValue, g_daHandler);
  isDAPolling = true;
  }
}


extern void stopDAPolling()
{
  g_daTimer.stopTimer();
  isDAPolling = false;
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


} // namespace LIS2DH12








