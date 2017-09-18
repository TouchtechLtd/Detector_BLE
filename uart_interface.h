/*
 * uart_driver.h
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#ifndef UART_INTERFACE_H_
#define UART_INTERFACE_H_



class UART {
	private:



	public:
		static void init(void);
		static void write(const char * format, ...);

	}; // End UART




#endif /* UART_INTERFACE_H_ */
