/*
 * uart_interface.cpp
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include "app_uart.h"
#include "app_error.h"
#include "nrf.h"
#include "boards.h"

#include <string.h>
#include "peripheral/uart_interface.h"


#define MAX_TEST_DATA_BYTES     (15U)                /**< max number of test bytes to be used for tx and rx. */
#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256                         /**< UART RX buffer size. */


bool UART::isPeripheralInitialised = false;

void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

void UART::init(void)
{
	if (isPeripheralInitialised) { return; }

    uint32_t err_code;

    const app_uart_comm_params_t comm_params =
      {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          0,
          0,
          APP_UART_FLOW_CONTROL_DISABLED,
          false,
          UART_BAUDRATE_BAUDRATE_Baud115200
      };

    APP_UART_FIFO_INIT(&comm_params,
                         UART_RX_BUF_SIZE,
                         UART_TX_BUF_SIZE,
                         uart_error_handle,
                         APP_IRQ_PRIORITY_LOWEST,
                         err_code);

    APP_ERROR_CHECK(err_code);

    isPeripheralInitialised = true;

    //write("UART Started!");
}


void UART::write(const char * format, ...) {

	if (!isPeripheralInitialised) { init(); }

	char tmp_str[100];
    va_list args;
    va_start(args, format);
    vsprintf(tmp_str, format, args);
    va_end(args);

	uint32_t slen	= strlen(tmp_str);
	for (uint32_t i =0; i < slen; i++) {
		while(app_uart_put(tmp_str[i]))
			;
	}
	while(app_uart_put('\n'));
	while(app_uart_put('\r'));
}

