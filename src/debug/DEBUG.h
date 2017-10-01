
/*
 * DEBUG.h
 *
 *  Created on: 21/09/2017
 *      Author: michaelmcadam
 */

#ifndef GOODNATURE_DETECTOR_DEBUG_H_
#define GOODNATURE_DETECTOR_DEBUG_H_


#define USE_DEBUG_MODULE 1
#define DEBUG_ENABLED 1
#define INFO_ENABLED 1


#if defined(INFO_ENABLED) || defined(DEBUG_ENABLED) || defined(ERROR_ENABLED)
#include "peripheral/uart_interface.h"
#define DEBUG_INIT() UART::init()
#else
#define DEBUG_INIT()
#endif

#ifdef DEBUG_ENABLED
#define DEBUG(fmt, ...) do { UART::write("%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__); } while (0)
#else
#define DEBUG(fmt, ...)
#endif



#ifdef INFO_ENABLED
#define INFO(fmt, ...) do { UART::write(fmt, ##__VA_ARGS__); } while (0)
#else
#define INFO(fmt, ...)
#endif


#ifdef ERROR_ENABLED
#define ERROR_CHECK(err_code) do { if (err_code != NRF_SUCCESS) \
									{ UART::write("%s:%d:%s(): %d", __FILE__, __LINE__, __func__, err_code); \
                  while(true) } } while (0)
#else
#define ERROR_CHECK(err_code) if(err_code) {}
#endif


#endif /* GOODNATURE_DETECTOR_DEBUG_H_ */
