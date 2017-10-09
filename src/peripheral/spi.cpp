/**
@addtogroup SPIWrapper SPI Wrapper for Ruuvitag
@{
@file       spi.c

Implementation of the SPI Wrapper.

Vesa Koskinen
May 11, 2016

For a detailed description see the detailed description in @ref spi.h

* @}
***************************************************************************************************/




/* INCLUDES ***************************************************************************************/
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "spi.h"
#include "nrf_drv_spi.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "boards.h"
#include "SEGGER_RTT.h"
#include "debug/DEBUG.h"
#include "peripheral/gpio_interface.h"
#include "peripheral/timer_interface.h"
#include "ruuvitag_b3.h"
#include "LIS2DH12_registers.h"

/* CONSTANTS **************************************************************************************/
#define SPI_INSTANCE  0 /**< SPI instance index. */

#define SPI_SCK_PIN 29
#define SPI_MISO_PIN 28
#define SPI_MOSI_PIN 25
#define SPI_SS_PIN 8
#define SPI_IRQ_PRIORITY 7

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


#define TEST_STRING "Test_String"
static uint8_t       m_tx_buf[] = TEST_STRING;           /**< TX buffer. */
static uint8_t       m_rx_buf[sizeof(TEST_STRING) + 1];    /**< RX buffer. */
static const uint8_t m_length = sizeof(m_tx_buf);        /**< Transfer length. */


/* MACROS *****************************************************************************************/

/* TYPES ******************************************************************************************/

/* PROTOTYPES *************************************************************************************/

void spi_event_handler(nrf_drv_spi_evt_t const * p_event, void*);

/* VARIABLES **************************************************************************************/
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
volatile bool spi_xfer_done; /**< Semaphore to indicate that SPI instance completed the transfer. */
static bool initDone = false;       /**< Flag to indicate if this module is already initilized */

volatile bool event_finished = true;
volatile bool trap_interrupt_tripped = false;

/* EXTERNAL FUNCTIONS *****************************************************************************/

extern void spi_init(void)
{

  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config.ss_pin   = SPI_SS_PIN;
  spi_config.miso_pin = SPI_MISO_PIN;
  spi_config.mosi_pin = SPI_MOSI_PIN;
  spi_config.sck_pin  = SPI_SCK_PIN;

  ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL));

  initDone = true;
}


extern bool spi_isInitialized(void)
{
    return initDone;
}

extern SPI_Ret spi_transfer(uint8_t* const p_toWrite, uint8_t count, uint8_t* const p_toRead)
{
    SPI_Ret retVal = SPI_RET_OK;
    if ((NULL == p_toWrite) || (NULL == p_toRead))
    {
        retVal = SPI_RET_ERROR;
    }


    /* check if an other SPI transfer is running */
    if ((true == event_finished) && (SPI_RET_OK == retVal))
    {
      event_finished = false;

        nrf_drv_spi_transfer(&spi, p_toWrite, count, p_toRead, count);
        while (!event_finished)
        {
            //Requires initialized softdevice - TODO
            //uint32_t err_code = sd_app_evt_wait();
            //ERROR_CHECK(err_code);
            //DEBUG("Here");
            __WFE();
        }
        retVal = SPI_RET_OK;
    }
    else
    {
        retVal = SPI_RET_BUSY;
    }

    return retVal;
}


/* INTERNAL FUNCTIONS *****************************************************************************/


/**
 * SPI user event handler
 *
 * Callback for Softdevice SPI Driver. Release blocking semaphore for SPI Transfer
 */
/*
void spi_event_handler(nrf_drv_spi_evt_t const * p_event, void*)
{
  //DEBUG("SPI Event handler called");
  spi_xfer_done = true;

  if (m_rx_buf[0] != 0)
  {
      DEBUG("Received: ");

    for (int i = 0; i < 6; i++)
    {
      DEBUG("%x", m_rx_buf[i]);
    }
      //NRF_LOG_HEXDUMP_INFO(m_rx_buf, strlen((const char *)m_rx_buf));
  }
}
*/

/*
void event_handler(void* p_context)
{
    DEBUG("LIS2DH12 Timer event'\r\n");
    //nrf_gpio_pin_toggle(19);
  memset(m_tx_buf, 0, m_length);

  m_tx_buf[0] = 0x28;

  nrf_drv_spi_transfer(&spi, m_tx_buf, 6, m_rx_buf, 6);

}*/
/*
void timeout_handler(void* p_context)
{
  DEBUG("Timed out");
  spi_xfer_done = true;
}
*/




void spi_event_handler(nrf_drv_spi_evt_t const * p_event, void*)
{
  event_finished = true;
}



typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} acceleration_t;

/** Union to split raw data to values for each axis */
typedef union
{
    uint8_t raw[6];
    acceleration_t sensor;
} sensor_buffer_t;



void sendSPI (uint8_t* data, uint8_t count)
{

  event_finished = false;
  uint32_t err_code = nrf_drv_spi_transfer(&spi, m_tx_buf, 2, m_rx_buf, count);
  ERROR_CHECK(err_code);

  while (event_finished != true) {
    __WFE();
  }


  memset(m_tx_buf, 0, m_length);
}


void m_readRegister(uint8_t address, uint8_t* const p_toRead, uint8_t count)
{
    SPI_Ret retValSpi = SPI_RET_ERROR;
    uint8_t writeBuf[READ_MAX + 1U] = {0}; /* Bytes to read + 1 for address */
    uint8_t readBuf[READ_MAX + 1U] = {0};  /* Bytes to read + 1 for address */
    uint8_t i = 0; /* retry counter */

    do
    {
    writeBuf[0] = address | SPI_READ | SPI_ADR_INC;

    retValSpi = spi_transfer(writeBuf, (count + 1U), readBuf);
    i++;
    }
    while ((SPI_RET_BUSY == retValSpi) && (i < RETRY_MAX)); /* Retry if SPI is busy */


    if (SPI_RET_OK == retValSpi)
    {
        /* Transfer was ok, copy result */
        memcpy(p_toRead, readBuf + 1U, count);
    }
}


void m_writeRegister(uint8_t address, uint8_t dataToWrite)
{
  SPI_Ret retValSpi = SPI_RET_ERROR;

  uint8_t to_read[2] = {0U}; /* dummy, not used for writing */
  uint8_t to_write[2] = {0U};
  uint8_t i = 0;

  to_write[0] = address;
  to_write[1] = dataToWrite;

  do
  {
      retValSpi = spi_transfer(to_write, 2, to_read);
      i++;
  }
  while ((SPI_RET_BUSY == retValSpi) && (i < RETRY_MAX)); /* Retry if SPI is busy */

}

void m_adjustRegister(uint8_t address, uint8_t dataToWrite)
{
  uint8_t readRegister[1];
  m_readRegister(address, readRegister, 1);
  m_writeRegister(address, dataToWrite | readRegister[0]);

}


void int1Event (nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  DEBUG("Interrupted by 1!");
  GPIO::interruptDisable(2);
  trap_interrupt_tripped = true;
}

void int2Event (nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  DEBUG("Interrupted by 2!");
}

int main (void)
{
  DEBUG_INIT();
  GPIO::init();
  Timer::initialisePeripheral();

  spi_init();

  GPIO::setOutput(LED_1_PIN, HIGH);


  GPIO::initIntInput(INT_ACC1_PIN,
          NRF_GPIOTE_POLARITY_LOTOHI,
          NRF_GPIO_PIN_NOPULL,
          false,
          false,
          int1Event);


  DEBUG("Started");

  memset(m_tx_buf, 0, m_length);
  memset(m_rx_buf, 0, m_length);



  nrf_delay_ms(20);

  // Enable XYZ
  m_adjustRegister(LIS2DH_CTRL_REG1, LIS2DH_ODR_MASK_10HZ);
  m_adjustRegister(LIS2DH_CTRL_REG1, LIS2DH_XYZ_EN_MASK);
  m_adjustRegister(LIS2DH_CTRL_REG1, LIS2DH_LPEN_MASK);


  // Enable HPF on INT1
  m_writeRegister(LIS2DH_CTRL_REG2, LIS2DH_FDS_MASK | LIS2DH_HPIS1_MASK);
  //Enable Interrupt on INT1
  m_writeRegister(LIS2DH_CTRL_REG3, LIS2DH_I1_IA1);
  // Enable/ Disable Latch on INT1
  //m_adjustRegister(LIS2DH_CTRL_REG5, LIS2DH_LIR_INT1_MASK);

  // Set threshold on INT1
  m_writeRegister(LIS2DH_INT1_THS, 0x10);
  // Set duration on INT1
  m_writeRegister(LIS2DH_INT1_DURATION, 0x00);
  // Set cfg on INT1
  m_writeRegister(LIS2DH_INT1_CFG, LIS2DH_XYZ_HIE_MASK);


  // Read REFERENCE
  uint8_t dummyRead[1];
  m_readRegister(LIS2DH_REFERENCE, dummyRead, 1);

  GPIO::interruptEnable(INT_ACC1_PIN);
  //GPIO::interruptEnable(6);


  sensor_buffer_t sensorData;
  int32_t accX, accY, accZ = 0;

  uint8_t collectData = 0;

  while(1)
  {

    if (trap_interrupt_tripped) {

      m_readRegister(LIS2DH_OUT_X_L, sensorData.raw, SENSOR_DATA_SIZE);


      accX = (((32768+sensorData.sensor.x)>>(16-8))-(1<<(8-1)))*1 ;
      accY = (((32768+sensorData.sensor.y)>>(16-8))-(1<<(8-1)))*1 ;
      accZ = (((32768+sensorData.sensor.z)>>(16-8))-(1<<(8-1)))*1 ;


      DEBUG("X: %d, Y: %d, Z: %d", accX, accY, accZ);

      collectData++;

      if (collectData >= 5)
      {
        trap_interrupt_tripped = false;
        collectData = 0;
        GPIO::interruptEnable(2);
      }

    }


    GPIO::toggle(LED_1_PIN);
    nrf_delay_ms(100);
  }
}


















