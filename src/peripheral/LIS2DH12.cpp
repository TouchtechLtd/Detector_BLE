/**
@addtogroup LIS2DH12Driver LIS2DH12 Acceleration Sensor Driver
@{
@file       LIS2DH12.c

Implementation of the LIS2DH12 driver.

For a detailed description see the detailed description in @ref LIS2DH12.h

* @}
***************************************************************************************************/

/* INCLUDES ***************************************************************************************/
#include "LIS2DH12.h"
#include "LIS2DH12_registers.h"
#include "peripheral/spi.h"
#include "nrf_drv_gpiote.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf.h"
#include "app_timer.h"
#include "bsp.h"
#include "boards.h"

#include "nrf_delay.h"

#include "debug/DEBUG.h"
#include "peripheral/timer_interface.h"
#include "peripheral/gpio_interface.h"

#include <string.h>

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

LIS2DH12_Ret readRegister(uint8_t address, uint8_t* const p_toRead, uint8_t count);

static LIS2DH12_Ret writeRegister(uint8_t address, uint8_t dataToWrite);

static void setRegister(uint8_t address, uint8_t dataToWrite);

static void clearRegister(uint8_t address, uint8_t dataToClear);

void timer_lis2dh12_event_handler(void* p_context);


/* VARIABLES **************************************************************************************/
static LIS2DH12_drdy_event_t g_fp_drdyCb = NULL;        /**< Data Ready Callback */
static sensor_buffer_t g_sensorData;                    /**< Union to covert raw data to value for each axis */
static LIS2DH12_PowerMode g_powerMode = LIS2DH12_POWER_DOWN; /**< Current power mode */
static LIS2DH12_SampleRate g_sampleRate = LIS2DH12_SAMPLE_1HZ; /**< Current power mode */
static uint16_t g_sampleRateValue = 0;
static LIS2DH12_Scale g_scale = LIS2DH12_SCALE2G;       /**< Selected scale */
static uint8_t g_mgpb = 1;                              /**< milli-g per bit */
static uint8_t g_resolution = 10;                        /**< milli-g nb of bits */

/* EXTERNAL FUNCTIONS *****************************************************************************/


extern void LIS2DH12_setPowerMode(LIS2DH12_PowerMode powerMode)
{
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


}

extern void LIS2DH12_setSampleRate(LIS2DH12_SampleRate sampleRate)
{
    clearRegister(LIS2DH_CTRL_REG1, LIS2DH_ODR_MASK_CLEAR);
    setRegister(LIS2DH_CTRL_REG1, sampleRate);

    g_sampleRate = sampleRate;
    g_sampleRateValue = GET_SAMPLERATE(sampleRate);

}

extern void LIS2DH12_setScale(LIS2DH12_Scale scale)
{
  g_scale = scale;
  uint8_t ctrl4RegVal = ((uint8_t)g_scale)<<4U;
  clearRegister(LIS2DH_CTRL_REG4 , LIS2DH_FS_MASK);
  setRegister(LIS2DH_CTRL_REG4, ctrl4RegVal);
}

extern void LIS2DH12_setHighPass()
{
  // Read REFERENCE
  uint8_t dummyRead[1];
  readRegister(LIS2DH_REFERENCE, dummyRead, 1);
}

extern void LIS2DH12_enableHighPass()
{
  setRegister(LIS2DH_CTRL_REG2, LIS2DH_FDS_MASK);
  LIS2DH12_setHighPass();
}


extern void LIS2DH12_disableHighPass()
{
  clearRegister(LIS2DH_CTRL_REG2, LIS2DH_FDS_MASK);
}

extern void LIS2DH12_enableX()
{
  setRegister(LIS2DH_CTRL_REG1, LIS2DH_X_EN_MASK);
}

extern void LIS2DH12_enableY()
{
  setRegister(LIS2DH_CTRL_REG1, LIS2DH_Y_EN_MASK);
}

extern void LIS2DH12_enableZ()
{
  setRegister(LIS2DH_CTRL_REG1, LIS2DH_Z_EN_MASK);
}

extern void LIS2DH12_disableX()
{
  clearRegister(LIS2DH_CTRL_REG1, LIS2DH_X_EN_MASK);
}

extern void LIS2DH12_disableY()
{
  clearRegister(LIS2DH_CTRL_REG1, LIS2DH_Y_EN_MASK);
}

extern void LIS2DH12_disableZ()
{
  clearRegister(LIS2DH_CTRL_REG1, LIS2DH_Z_EN_MASK);
}

extern void LIS2DH12_enableXYZ()
{
  setRegister(LIS2DH_CTRL_REG1, LIS2DH_XYZ_EN_MASK);
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


void LIS2DH12_clearRegisters()
{
  clearRegister(LIS2DH_CTRL_REG1 , LIS2DH_CLEAR_REGISTER_MASK);
  clearRegister(LIS2DH_CTRL_REG2 , LIS2DH_CLEAR_REGISTER_MASK);
  clearRegister(LIS2DH_CTRL_REG3 , LIS2DH_CLEAR_REGISTER_MASK);
  clearRegister(LIS2DH_CTRL_REG4 , LIS2DH_CLEAR_REGISTER_MASK);
  clearRegister(LIS2DH_CTRL_REG5 , LIS2DH_CLEAR_REGISTER_MASK);
  clearRegister(LIS2DH_CTRL_REG6 , LIS2DH_CLEAR_REGISTER_MASK);
}


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


void int1Event(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  DEBUG("Interrupted!");
}


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

  if (thresholdPercent > 1000) { thresholdPercent = 1000; }

  uint8_t outputValue = (uint8_t) (thresholdPercent * (MAX_INTERRUPT_THRESHOLD + 1) / 1000);

  return outputValue;
}


extern void LIS2DH12_initThresholdInterrupt1(uint8_t threshold,
                                             uint8_t duration,
                                             LIS2DH12_InterruptPinNumber intPinNum,
                                             LIS2DH12_InterruptThresholdMask intThreshMask,
                                             bool hpEnabled,
                                             bool latchEnabled,
                                             gpio_event_handler_t handler)
{

  unsigned long interruptPin = 0;
  uint8_t interruptRegister = 0;

  if (intPinNum == LIS2DH12_INTERRUPT_PIN_1)
  {
    interruptPin  = INT_ACC1_PIN;
    interruptRegister   = LIS2DH_CTRL_REG3;
  }
  else if (intPinNum == LIS2DH12_INTERRUPT_PIN_2)
  {
    interruptPin  = INT_ACC2_PIN;
    interruptRegister   = LIS2DH_CTRL_REG6;
  }


  GPIO::initIntInput(interruptPin,
          NRF_GPIOTE_POLARITY_LOTOHI,
          NRF_GPIO_PIN_NOPULL,
          false,
          false,
          handler);


  if (hpEnabled) {
    // Enable HPF on INT1
    setRegister(LIS2DH_CTRL_REG2, LIS2DH_HPIS1_MASK);
  }

  if (latchEnabled) {
    // Enable/ Disable Latch on INT1
    setRegister(LIS2DH_CTRL_REG5, LIS2DH_LIR_INT1_MASK);
  }

  // Set duration on INT1
  uint8_t intDuration = findInterruptDuration(duration);
  setRegister(LIS2DH_INT1_DURATION, intDuration);

  // Set threshold on INT1
  uint8_t intThreshold = findInterruptThreshold(threshold);
  setRegister(LIS2DH_INT1_THS, intThreshold);

  // Set cfg on INT1
  setRegister(LIS2DH_INT1_CFG, intThreshMask);

  //Enable Interrupt on INT1
  setRegister(interruptRegister, LIS2DH_I1_IA1);


  GPIO::interruptEnable(INT_ACC1_PIN);
}

//extern void LIS2DH12_initThresholdInterrupt2(uint8_t threshold, uint8_t duration, handler)
//{

//}

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

/**
 * Read registers
 *
 * Read one or more registers from the sensor
 *
 * @param[in] address Start address to read from
 * @param[out] p_toRead Pointer to result buffer
 * @param[in] cound Number of bytes to read
 *
 * @return LIS2DH12_RET_OK No Error
 * @return LIS2DH12_RET_NULL Result buffer is NULL Pointer
 * @return LIS2DH12_RET_ERROR Read attempt was not successful
 */
LIS2DH12_Ret readRegister(uint8_t address, uint8_t* const p_toRead, uint8_t count)
{
    // Check that the inputs are valid

    if (p_toRead == NULL) return LIS2DH12_RET_NULL;
    if (count > READ_MAX) return LIS2DH12_RET_ERROR;
    /* SPI Addresses are 5bit only */
    if (address > ADR_MAX) return LIS2DH12_RET_ERROR;


    uint8_t writeBuf[READ_MAX + 1U] = {0}; /* Bytes to read + 1 for address */
    writeBuf[0] = address | SPI_READ | SPI_ADR_INC;

    uint8_t readBuf[READ_MAX + 1U] = {0};  /* Bytes to read + 1 for address */
    uint8_t retries = 0; /* retry counter */

    SPI_Ret retValSpi = SPI_RET_OK;
    do
    {
      retValSpi = spi_transfer(writeBuf, (count + 1U), readBuf);
      retries++;
    }
    while ((retValSpi == SPI_RET_BUSY) && (retries < RETRY_MAX)); /* Retry if SPI is busy */


    if (retValSpi != SPI_RET_OK) return LIS2DH12_RET_ERROR;
    else {
      /* Transfer was ok, copy result */
      memcpy(p_toRead, readBuf + 1U, count);
      return LIS2DH12_RET_OK;
    }
}

/**
 * Write a register
 *
 * @param[in] address Register address to write, address is 5bit, so max value is 0x1F
 * @param[in] dataToWrite Data to write to register
 *
 * @return LIS2DH12_RET_OK No Error
 * @return LIS2DH12_RET_ERROR Address is lager than allowed
 */
static LIS2DH12_Ret writeRegister(uint8_t address, uint8_t dataToWrite)
{

    /* SPI Addresses are 5bit only */
    if (address > ADR_MAX) { return LIS2DH12_RET_ERROR; }

    uint8_t to_write[2] = {0U};
    to_write[0] = address;
    to_write[1] = dataToWrite;

    uint8_t to_read[2] = {0U}; /* dummy, not used for writing */
    uint8_t retries = 0; /* retry counter */

    SPI_Ret retValSpi = SPI_RET_OK;
    do
    {
        retValSpi = spi_transfer(to_write, 2, to_read);
        retries++;
    }
    while ((retValSpi == SPI_RET_BUSY) && (retries < RETRY_MAX)); /* Retry if SPI is busy */


    if (retValSpi != SPI_RET_OK) { return LIS2DH12_RET_ERROR; }
    else { return LIS2DH12_RET_OK; }
}


static void setRegister(uint8_t address, uint8_t dataToWrite)
{
  uint8_t currentRegisterState[1];
  readRegister(address, currentRegisterState, 1);
  writeRegister(address, dataToWrite | currentRegisterState[0]);
}


static void clearRegister(uint8_t address, uint8_t dataToClear)
{
  uint8_t currentRegisterState[1];
  readRegister(address, currentRegisterState, 1);
  writeRegister(address, currentRegisterState[0] & ~dataToClear);
}






/**
 * Event Handler that is called by the timer to read the sensor values.
 *
 * This is a workaround because data ready interrupt from LIS2DH12 is not working
 *
 * @param [in] pContext Timer Context
 */
void timer_lis2dh12_event_handler(void* p_context)
{
    //DEBUG("LIS2DH12 Timer event'\r\n");
    //nrf_gpio_pin_toggle(19);

  readRegister(LIS2DH_OUT_X_L, g_sensorData.raw, SENSOR_DATA_SIZE);

}



int main (void)
{
  DEBUG_INIT();
  GPIO::init();

  spi_init();

  GPIO::setOutput(LED_1_PIN, HIGH);


  DEBUG("Started");


  nrf_delay_ms(20);

  LIS2DH12_init(LIS2DH12_POWER_LOW, LIS2DH12_SCALE2G, LIS2DH12_SAMPLE_10HZ);
  //LIS2DH12_enableHighPass();

  LIS2DH12_initThresholdInterrupt1(250, 0, LIS2DH12_INTERRUPT_PIN_1, LIS2DH12_INTERRUPT_THRESHOLD_XYZ, true, false, int1Event);



  //GPIO::interruptEnable(6);



  int32_t accX, accY, accZ = 0;


  while(1)
  {


    readRegister(LIS2DH_OUT_X_L, g_sensorData.raw, SENSOR_DATA_SIZE);


    LIS2DH12_getALLmG(&accX, &accY, &accZ);

    DEBUG("X: %d, Y: %d, Z: %d", accX, accY, accZ);



    GPIO::toggle(LED_1_PIN);
    nrf_delay_ms(1000);
  }
}









