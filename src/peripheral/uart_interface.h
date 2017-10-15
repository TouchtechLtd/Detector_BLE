
/*
 * uart_interface.h
 *
 *  Created on: 6/09/2017
 *      Author: michaelmcadam
 */

#ifndef _GOODNATURE_PERIPHERAL_UART_INTERFACE_H_
#define _GOODNATURE_PERIPHERAL_UART_INTERFACE_H_


#ifdef __cplusplus
extern "C" {
#endif


/*
class UART {
	private:
		static bool isPeripheralInitialised;


	public:
		static void init(void);
		static void write(const char * format, ...);

	}; // End UART
*/

void UART_init(void);
void UART_write(const char * format, ...);


#ifdef __cplusplus
}
#endif

#endif /* _GOODNATURE_PERIPHERAL_UART_INTERFACE_H_ */
