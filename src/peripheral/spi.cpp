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
#include "debug/DEBUG.h"

#include "peripheral/gpio_interface.h"

/* CONSTANTS **************************************************************************************/
#define SPI_INSTANCE  1 /**< SPI instance index. */

#define SPI_SCK_PIN 29
#define SPI_MISO_PIN 28
#define SPI_MOSI_PIN 25
#define SPI_SS_PIN 8
#define SPI_IRQ_PRIORITY 6


/* MACROS *****************************************************************************************/

/* TYPES ******************************************************************************************/

/* PROTOTYPES *************************************************************************************/

void spi_event_handler(nrf_drv_spi_evt_t const * p_event, void*);

/* VARIABLES **************************************************************************************/
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
static bool initDone = false;       /**< Flag to indicate if this module is already initilized */
volatile bool event_finished = true;

/* EXTERNAL FUNCTIONS *****************************************************************************/

extern void spi_init(void)
{

  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config.ss_pin   = SPI_SS_PIN;
  spi_config.miso_pin = SPI_MISO_PIN;
  spi_config.mosi_pin = SPI_MOSI_PIN;
  spi_config.sck_pin  = SPI_SCK_PIN;
  spi_config.irq_priority = SPI_IRQ_PRIORITY;

  uint32_t err_code = nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL);
  ERROR_CHECK(err_code);

  initDone = true;
}


extern bool spi_isInitialized(void)
{
    return initDone;
}

extern SPI_Ret spi_transfer(uint8_t* const p_toWrite, uint8_t count, uint8_t* const p_toRead)
{
    /* Ensure read and write pointers are valid */
    if ((NULL == p_toWrite) || (NULL == p_toRead)) { return SPI_RET_ERROR; }
    /* check if an other SPI transfer is running */
    if (event_finished == false) { return SPI_RET_BUSY; }


    event_finished = false;

    nrf_drv_spi_transfer(&spi, p_toWrite, count, p_toRead, count);
    while (!event_finished)
    {
      //NRF_LOG_PROCESS();
        //Requires initialized softdevice - TODO
        uint32_t err_code = sd_app_evt_wait();
        ERROR_CHECK(err_code);
        //DEBUG("Here");
        //__WFE();
    }
    return  SPI_RET_OK;
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
















