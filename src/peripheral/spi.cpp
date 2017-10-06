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

/* CONSTANTS **************************************************************************************/
#define SPI_INSTANCE  0 /**< SPI instance index. */

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

/* EXTERNAL FUNCTIONS *****************************************************************************/

extern void spi_init(void)
{
    /* Conigure SPI Interface */
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.sck_pin = SPIM0_SCK_PIN;
    spi_config.miso_pin = SPIM0_MISO_PIN;
    spi_config.mosi_pin = SPIM0_MOSI_PIN;
    spi_config.frequency = NRF_DRV_SPI_FREQ_4M;

    /* Init chipselect for BME280 */
    GPIO::setOutput(SPIM0_SS_HUMI_PIN, HIGH);

    /* Init chipselect for LIS2DH12 */
    GPIO::setOutput(SPIM0_SS_ACC_PIN, HIGH);

    ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL));

    spi_xfer_done = true;
    initDone = true;
}


extern bool spi_isInitialized(void)
{
    return initDone;
}

extern SPI_Ret spi_transfer_bme280(uint8_t* const p_toWrite, uint8_t count, uint8_t* const p_toRead)
{	
	SPI_Ret retVal = SPI_RET_OK;

	if ((NULL == p_toWrite) || (NULL == p_toRead))
	{
	    retVal = SPI_RET_ERROR;
	}

	
    /* check if an other SPI transfer is running */
    if ((true == spi_xfer_done) && (SPI_RET_OK == retVal))
	{
        spi_xfer_done = false;

        nrf_gpio_pin_clear(SPIM0_SS_HUMI_PIN);
        ERROR_CHECK(nrf_drv_spi_transfer(&spi, p_toWrite, count, p_toRead, count));
        while (!spi_xfer_done)
        {
            //Requires initialized softdevice - TODO
            uint32_t err_code = sd_app_evt_wait();
            ERROR_CHECK(err_code);
            //__WFE(); 
        }
        nrf_gpio_pin_set(SPIM0_SS_HUMI_PIN);
        retVal = SPI_RET_OK;
	}
	else
	{
	    retVal = SPI_RET_BUSY;
	}

    return retVal;
}

extern SPI_Ret spi_transfer_lis2dh12(uint8_t* const p_toWrite, uint8_t count, uint8_t* const p_toRead)
{
    SPI_Ret retVal = SPI_RET_OK;
    if ((NULL == p_toWrite) || (NULL == p_toRead))
    {
        retVal = SPI_RET_ERROR;
    }


    /* check if an other SPI transfer is running */
    if ((true == spi_xfer_done) && (SPI_RET_OK == retVal))
    {
        spi_xfer_done = false;

        nrf_gpio_pin_clear(SPIM0_SS_ACC_PIN);
        nrf_drv_spi_transfer(&spi, p_toWrite, count, p_toRead, count);
        while (!spi_xfer_done)
        {
            //Requires initialized softdevice - TODO
            uint32_t err_code = sd_app_evt_wait();
            ERROR_CHECK(err_code);
          DEBUG("Here");
            //__WFE();
        }
        GPIO::setOutput(LED_1_PIN, LOW);
        nrf_gpio_pin_set(SPIM0_SS_ACC_PIN);
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

/*

int main(void) {
  DEBUG_INIT();
  GPIO::init();
  //Timer::initialisePeripheral();
  spi_init();

  GPIO::setOutput(LED_1_PIN, LOW);
  DEBUG("Working");

  spi_xfer_done = false;

  memset(m_rx_buf, 0, m_length);

  //m_tx_buf[0] = 0X0F | 0x80U | 0x40U;
  nrf_delay_ms(20);
  //GPIO::setOutput(3, LOW);
  ERROR_CHECK(nrf_drv_spi_transfer(&spi, m_tx_buf, 2, m_rx_buf, 2));
  Timer timeout;
  timeout.startTimer(1000, timeout_handler);
  while (!spi_xfer_done)
  {
      __WFE();
  }
  timeout.stopTimer();
  //GPIO::setOutput(3, HIGH);
   * */
/*
  nrf_delay_ms(1000);

  spi_xfer_done = false;
  uint8_t ctrl1RegVal = 0;
  // Set Scale
  // Enable all axis
  ctrl1RegVal = 0x07;
  ctrl1RegVal |= 0x50;
  uint8_t to_write[2] = {0U};

  to_write[0] = 0x20;
  to_write[1] = ctrl1RegVal;

  GPIO::setOutput(SPIM0_SS_ACC_PIN, LOW);
  ERROR_CHECK(nrf_drv_spi_transfer(&spi, to_write, 2, m_rx_buf, 2));
  timeout.startTimer(1000, timeout_handler);
  while (!spi_xfer_done)
  {
      __WFE();
  }
  timeout.stopTimer();
  GPIO::setOutput(SPIM0_SS_ACC_PIN, HIGH);

  DEBUG("Done here");
*/
  //Timer testTimer;
  //testTimer.startTimer(1000, event_handler);
/*
  while(1) {
    GPIO::toggle(LED_1_PIN);
    nrf_delay_ms(500);
  }
}
*/



#define SPI_SCK_PIN 29
#define SPI_MISO_PIN 28
#define SPI_MOSI_PIN 25
#define SPI_SS_PIN 8
#define SPI_IRQ_PRIORITY 7


volatile bool event_finished = true;

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

  if (m_rx_buf[0] != 0)
     {
         for (int i = 0; i < count; i++) {
           DEBUG("TX: %x, RX: %x", m_tx_buf[i], m_rx_buf[i]);
         }
     }

  nrf_delay_ms(50);

  memset(m_tx_buf, 0, m_length);
}


int main (void)
{
  DEBUG_INIT();
  GPIO::init();
  Timer::initialisePeripheral();

  GPIO::setOutput(LED_1_PIN, HIGH);

  DEBUG("Started");

  memset(m_tx_buf, 0, m_length);
  memset(m_rx_buf, 0, m_length);

  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config.ss_pin   = SPI_SS_PIN;
  spi_config.miso_pin = SPI_MISO_PIN;
  spi_config.mosi_pin = SPI_MOSI_PIN;
  spi_config.sck_pin  = SPI_SCK_PIN;
  ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL));

  nrf_delay_ms(20);

  //m_tx_buf[0] = 0X0F | 0x80U | 0x40U;
  //m_tx_buf[0] = 0X0F;
  m_tx_buf[0] = 0x20;
  m_tx_buf[1] = 0x07 | 0X50;
  sendSPI(m_tx_buf, 2);

  m_tx_buf[0] = 0X20 | 0x80U | 0x40U;
  sendSPI(m_tx_buf, 2);

  m_tx_buf[0] = 0x21;
  m_tx_buf[1] = 0x08;
  sendSPI(m_tx_buf, 2);

  m_tx_buf[0] = 0X21 | 0x80U | 0x40U;
  sendSPI(m_tx_buf, 2);


  m_tx_buf[0] = 0X26 | 0x80U | 0x40U;
  sendSPI(m_tx_buf, 2);


  sensor_buffer_t sensorData;


  int32_t accX, accY, accZ = 0;

  while(1)
  {

    m_tx_buf[0] = 0x28 | 0x80U | 0x40U;

    while (event_finished != true) {
      DEBUG("Here");
      __WFE();
    }
    event_finished = false;

    uint32_t err_code = nrf_drv_spi_transfer(&spi, m_tx_buf, m_length, m_rx_buf, 7);
    ERROR_CHECK(err_code);

    memcpy(sensorData.raw, m_rx_buf + 1U, 6);

    accX = (((32768+sensorData.sensor.x)>>(16-10))-(1<<(10-1)))*1 ;
    accY = (((32768+sensorData.sensor.y)>>(16-10))-(1<<(10-1)))*1 ;
    accZ = (((32768+sensorData.sensor.z)>>(16-10))-(1<<(10-1)))*1 ;

    //for (int i = 0; i < 7; i++) {
    //  DEBUG("Sensor Data Raw: %x",sensorData.raw[i]);
    //}
    DEBUG("X: %d, Y: %d, Z: %d", accX, accY, accZ);
   //DEBUG("Acc_X: %d", sensorData.sensor.x);

    //DEBUG("New Loop");

    GPIO::toggle(LED_1_PIN);
    nrf_delay_ms(100);
  }
}


















