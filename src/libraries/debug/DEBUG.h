
/*
 * DEBUG.h
 *
 *  Created on: 21/09/2017
 *      Author: michaelmcadam
 */

#ifndef GOODNATURE_DETECTOR_DEBUG_H_
#define GOODNATURE_DETECTOR_DEBUG_H_


//#define USE_DEBUG_MODULE 1
#define DEBUG_ENABLED 1
#define INFO_ENABLED 1
#define ERROR_ENABLED 1



#if defined(INFO_ENABLED) || defined(DEBUG_ENABLED) || defined(ERROR_ENABLED)
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "libraries/events/events.h"

#define DEBUG_ERROR_EVENT 0xFFFF

#define DEBUG_INIT() NRF_LOG_INIT(NULL); \
                      NRF_LOG_DEFAULT_BACKENDS_INIT();

//#define   DEBUG_INIT()  UART_init();
#else
#define DEBUG_INIT()
#endif

#ifdef DEBUG_ENABLED
#define LOG_DEBUG(fmt, ...) do { NRF_LOG_DEBUG("%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__); } while (0)
//#define DEBUG(fmt, ...) do { UART_write("%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__); } while (0)
#else
#define LOG_DEBUG(fmt, ...)
#endif



#ifdef INFO_ENABLED
#define INFO(fmt, ...) do { NRF_LOG_INFO(fmt, ##__VA_ARGS__); } while (0)
//#define INFO(fmt, ...) do { UART_write(fmt, ##__VA_ARGS__); } while (0)
#else
#define INFO(fmt, ...)
#endif


#ifdef ERROR_ENABLED
#define ERROR_CHECK(err_code) do { if (err_code != NRF_SUCCESS) { \
                                  EVENTS::eventPut(DEBUG_ERROR_EVENT, &err_code, sizeof(err_code)); \
                                   LOG_DEBUG("Error: %d", err_code); \
                                   } } while (0)
/*
#define ERROR_CHECK(err_code) do { if (err_code != NRF_SUCCESS) \
									{ NRF_LOG_INFO("Error: %s:%d:%s(): %d", __FILE__, __LINE__, __func__, err_code); \
									while(true) {}}} while(0)
									*/
/*
#define ERROR_CHECK(err_code) do { if (err_code != NRF_SUCCESS) \
                  { UART_write("Error: %s:%d:%s(): %d", __FILE__, __LINE__, __func__, err_code); \
                  while(true) {}}} while(0)
                  */
#else
#define ERROR_CHECK(err_code) if(err_code) {}
#endif


#endif /* GOODNATURE_DETECTOR_DEBUG_H_ */
