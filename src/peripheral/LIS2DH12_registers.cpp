/**
@addtogroup LIS2DH12Driver LIS2DH12 Acceleration Sensor Driver
@{
@file       LIS2DH12.c

Implementation of the LIS2DH12 driver.

For a detailed description see the detailed description in @ref LIS2DH12.h

* @}
***************************************************************************************************/

/* INCLUDES ***************************************************************************************/
#include <stdlib.h>
#include <stdint.h>

#include "LIS2DH12_registers.h"
#include "peripheral/spi.h"
#include "nrf.h"
#include "bsp.h"
#include "boards.h"

#include "debug/DEBUG.h"

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


/* TYPES ******************************************************************************************/


/* PROTOTYPES *************************************************************************************/


/* VARIABLES **************************************************************************************/


/* FUNCTIONS *****************************************************************************/


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
extern LIS2DH12_Ret readRegister(uint8_t address, uint8_t* const p_toRead, uint8_t count)
{
  // Check that the inputs are valid

  if (p_toRead == NULL) return LIS2DH12_RET_NULL;
  if (count > READ_MAX) return LIS2DH12_RET_ERROR;
  if (address > ADR_MAX) return LIS2DH12_RET_ERROR;           /* SPI Addresses are 5bit only */


  uint8_t writeBuf[READ_MAX + 1U] = {0};                      /* Bytes to read + 1 for address */
  writeBuf[0] = address | SPI_READ | SPI_ADR_INC;

  uint8_t readBuf[READ_MAX + 1U] = {0};                       /* Bytes to read + 1 for address */
  uint8_t retries = 0;                                        /* retry counter */

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

  uint8_t to_read[2] = {0U};    /* dummy, not used for writing */
  uint8_t retries = 0;          /* retry counter */

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


extern LIS2DH12_Ret setRegister(uint8_t address, uint8_t dataToWrite)
{
  uint8_t err_code = LIS2DH12_RET_OK;
  uint8_t currentRegisterState[1];
  err_code |= readRegister(address, currentRegisterState, 1);
  err_code |= writeRegister(address, dataToWrite | currentRegisterState[0]);
  return (LIS2DH12_Ret) err_code;
}


extern LIS2DH12_Ret clearRegister(uint8_t address, uint8_t dataToClear)
{
  uint8_t err_code = LIS2DH12_RET_OK;
  uint8_t currentRegisterState[1];
  err_code |= readRegister(address, currentRegisterState, 1);
  err_code |= writeRegister(address, currentRegisterState[0] & ~dataToClear);
  return (LIS2DH12_Ret) err_code;
}


extern LIS2DH12_Ret LIS2DH12_clearRegisters()
{
  clearRegister(LIS2DH_CTRL_REG1 , LIS2DH_CLEAR_REGISTER_MASK);
  clearRegister(LIS2DH_CTRL_REG2 , LIS2DH_CLEAR_REGISTER_MASK);
  clearRegister(LIS2DH_CTRL_REG3 , LIS2DH_CLEAR_REGISTER_MASK);
  clearRegister(LIS2DH_CTRL_REG4 , LIS2DH_CLEAR_REGISTER_MASK);
  clearRegister(LIS2DH_CTRL_REG5 , LIS2DH_CLEAR_REGISTER_MASK);
  clearRegister(LIS2DH_CTRL_REG6 , LIS2DH_CLEAR_REGISTER_MASK);
}




