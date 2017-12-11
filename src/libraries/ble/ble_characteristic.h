
/*
 * ble_characteristic.h
 *
 *  Created on: September 5, 2017
 *      Author: Michael McAdam
 */

#ifndef _GOODNATURE_BLE_BLE_CHARACTERISTIC_H__
#define _GOODNATURE_BLE_BLE_CHARACTERISTIC_H__


// SYSTEM INCLUDES
//

// PROJECT INCLUDES
//

// LOCAL INCLUDES
//

// FORWARD REFERENCES
//


#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "libraries/events/events.h"

namespace BLE_SERVER {


#define GN_CHAR_ERROR_OFFSET 20
#define GN_SUCCESS 0


#define CHARACTERISTIC_WRITE_EVENT_OFFSET 0x0101


typedef enum
{
  GN_CHAR_NOT_RUNNING = GN_CHAR_ERROR_OFFSET,
  GN_CHAR_NOT_CONNECTED,
  GN_CHAR_READ_NOT_ENABLED,
  GN_CHAR_NOTIFICATION_NOT_ENABLED,
  GN_CHAR_VALUE_TOO_LONG
} gn_char_error_t;


typedef enum
{
  CHAR_READ_ONLY,
  CHAR_READ_WRITE
}char_access_e;

//typedef void (*char_write_handler_t) (uint8_t const* data, uint16_t len);
typedef EVENTS::event_callback_t char_write_handler_t;

class Characteristic
{
public:
  // LIFECYCLE

  /** Default constructor.
  */
  Characteristic();

  // Use compiler-generated copy constructor, assignment, and destructor.
  //Characteristic(const Characteristic&);
  //~Characteristic(void);
  //Characteristic& operator=(const Characteristic&);

  // OPERATIONS
  gn_char_error_t set(void* data, uint16_t dataLength);
  void attachToService(uint16_t serviceHandle);

  void configure(uint16_t uuid, void* p_data, uint16_t dataLen, char_access_e access);
  void configureAsReadOnly();
  void configureAsReadWrite();

  // ACCESS
  void setUUID (uint16_t uuid);
  void setUUIDType(uint8_t type);
  void setDataPointer(void* p_value, uint16_t len);
  void setWriteHandler(char_write_handler_t writeHandler);
  void setEventModifier(uint8_t eventModifier);

  // INQUIRY
  bool    isInit();
  bool    isRunning();

  // TEMP
  void eventHandler(ble_evt_t const * p_ble_evt);

private:
  ble_gatts_char_md_t 		  m_charMd;
  ble_gatts_attr_md_t 		  m_cccdMd;
  ble_gatts_attr_md_t       m_attrMd;
  ble_gatts_attr_t    		  m_attrCharValue;
  ble_uuid_t          		  m_charUuid;
  ble_uuid128_t				      m_baseUuid;
  ble_gatts_hvx_params_t 		m_hvxParams;
  ble_gatts_char_handles_t 	m_charHandle;

  uint16_t             m_connHandle;
  char_write_handler_t m_writeHandler;

  bool m_isUUIDSet;
  bool m_isRunning;
  bool m_notificationEnabled;
  bool m_readEnabled;
  bool m_txComplete;

  uint8_t            m_eventIDModifier;


  void enableRead();
  void disableRead();
  void enableWrite();
  void disableWrite();
  void enableNotification();

  gn_char_error_t notify(void * i_data, uint16_t data_length);
  gn_char_error_t update(void * i_data, uint16_t data_length);

  void setMaxLength(uint16_t maxLen);




};	// CHARACTERISTIC



// INLINE METHODS
//


} // BLE_SERVER

#endif  /* _ _GOODNATURE_BLE_BLE_CHARACTERISTIC_H__ */

