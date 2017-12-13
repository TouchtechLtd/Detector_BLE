
/*
 * ble_characteristic.cpp
 *
 *  Created on: September 5, 2017
 *      Author: Michael McAdam
 */

#include <stdint.h>
#include <string.h>
#include "nrf_gpio.h"
#include "ble_srv_common.h"
#include "app_error.h"

#include "libraries/debug/DEBUG.h"
#include "libraries/ble/ble_characteristic.h"

namespace BLE_SERVER {



/////////////////////////////// PUBLIC ///////////////////////////////////////
//============================= LIFECYCLE ====================================

Characteristic::Characteristic() {

  // Add read/write properties to our characteristic
  memset(&m_charHandle,     0, sizeof(m_charHandle));
  memset(&m_charUuid,       0, sizeof(m_charUuid));
  memset(&m_cccdMd,         0, sizeof(m_cccdMd));
  memset(&m_charMd,         0, sizeof(m_charMd));
  memset(&m_attrMd,         0, sizeof(m_attrMd));
  memset(&m_attrCharValue,  0, sizeof(m_attrCharValue));

  m_isUUIDSet =           false;
  m_isRunning =           false;
  m_notificationEnabled = false;
  m_readEnabled =         false;

  m_writeHandler =  NULL;
  m_connHandle =    BLE_CONN_HANDLE_INVALID;
  m_txComplete = false;

  m_eventIDModifier = 0;
}



//============================= OPERATIONS ===================================


void Characteristic::attachToService(uint16_t i_serviceHandle)
{

  if (m_isUUIDSet && !m_isRunning)
  {
    // Configure the characteristic value attribute
    m_attrMd.vloc        = BLE_GATTS_VLOC_STACK;
    m_attrCharValue.p_uuid      = &m_charUuid;
    m_attrCharValue.p_attr_md   = &m_attrMd;

    //  Add our new characteristic to the service
    uint32_t err_code = sd_ble_gatts_characteristic_add(i_serviceHandle,
                     &m_charMd,
                     &m_attrCharValue,
                     &m_charHandle);

    ERROR_CHECK(err_code);

    m_isRunning = true;
    m_eventIDModifier = m_charHandle.value_handle;

  } else {
    INFO("ERROR: Please set UUID before adding service");
  }
}


// Function to be called when updating characteristic value

gn_char_error_t Characteristic::set(void* data, uint16_t dataLength)
{
  gn_char_error_t err_code;
  if (m_readEnabled)
  {
    err_code = update(data, dataLength);
  }
  if (m_notificationEnabled && BLE_CONN_HANDLE_INVALID != m_connHandle)
  {
    err_code = notify(data, dataLength);
  }
  return err_code;
}

void Characteristic::configure(uint16_t uuid, void* p_data, uint16_t dataLen, char_access_e access)
{
  setUUID(uuid);
  setDataPointer(p_data, dataLen);
  switch (access)
  {
  case CHAR_READ_ONLY:
    configureAsReadOnly();
    break;

  case CHAR_READ_WRITE:
    configureAsReadWrite();
    break;

  }
}


void Characteristic::configureAsReadOnly()
{
  enableRead();
  enableNotification();
}

void Characteristic::configureAsReadWrite()
{
  enableRead();
  enableNotification();
  enableWrite();
}


//============================= ACCESS      ===================================

/**
 * Write description of function here
 * This comment will last the test of time.
 * @param i_uuid
 */
void Characteristic::setUUID(uint16_t uuid)
{
  m_charUuid.uuid = uuid;
  if (m_charUuid.type != 0) { m_isUUIDSet = true; }
}

void Characteristic::setUUIDType(uint8_t type)
{
  m_charUuid.type = type;
  if (m_charUuid.uuid != 0) { m_isUUIDSet = true; }
}


void Characteristic::setDataPointer(void* p_value, uint16_t len)
{
    m_attrCharValue.init_len    = len;
    m_attrCharValue.p_value     = static_cast<uint8_t*>(p_value);
    setMaxLength(len);
}


void Characteristic::setWriteHandler(char_write_handler_t writeHandler)
{
  m_writeHandler = writeHandler;
  EVENTS::registerEventHandler(CHARACTERISTIC_WRITE_EVENT_OFFSET + m_eventIDModifier, writeHandler);
}

/*
void Characteristic::setEventModifier(uint8_t eventModifier)
{
  //m_eventIDModifier = eventModifier;
  //m_eventIDModifier = m_charHandle.value_handle;
}
*/

//============================= INQUIRY    ===================================


/**
 *
 * @return
 */
bool Characteristic::isInit()
{
  return m_isUUIDSet;
}


bool Characteristic::isRunning()
{
  return m_isRunning;
}



/////////////////////////////// PRIVATE    ///////////////////////////////////


void Characteristic::enableRead() {
    m_charMd.char_props.read = 1;
    m_attrMd.rd_auth    = 0;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&m_attrMd.read_perm);
    m_readEnabled = true;
}

void Characteristic::disableRead() {
    m_charMd.char_props.read = 0;
    m_attrMd.rd_auth    = 0;
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&m_attrMd.read_perm);
}

void Characteristic::enableWrite() {
    m_charMd.char_props.write = 1;
    m_attrMd.wr_auth    = 0;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&m_attrMd.write_perm);
}

void Characteristic::disableWrite() {
    m_charMd.char_props.write = 0;
    m_attrMd.wr_auth    = 0;
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&m_attrMd.write_perm);
}


void Characteristic::enableNotification() {
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&m_cccdMd.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&m_cccdMd.write_perm);
    m_cccdMd.vloc                = BLE_GATTS_VLOC_STACK;
    m_charMd.p_cccd_md           = &m_cccdMd;
    m_charMd.char_props.notify   = 1;
}



void Characteristic::setMaxLength(uint16_t maxLen)
{
	m_attrCharValue.max_len = maxLen;
	if (m_attrCharValue.max_len == m_attrCharValue.init_len)
	{
		m_attrMd.vlen 		= 0;
	}
	else { m_attrMd.vlen 		= 1; }
}



gn_char_error_t Characteristic::notify(void* data, uint16_t  dataLength)
{
  if (true != m_notificationEnabled)              { return GN_CHAR_NOTIFICATION_NOT_ENABLED; }
  if (true != m_isRunning)                        { return GN_CHAR_NOT_RUNNING; }
  if (BLE_CONN_HANDLE_INVALID == m_connHandle)    { return GN_CHAR_NOT_CONNECTED; }

  memset(&m_hvxParams, 0, sizeof(m_hvxParams));
  m_hvxParams.handle = m_charHandle.value_handle;
  m_hvxParams.type   = BLE_GATT_HVX_NOTIFICATION;
  m_hvxParams.offset = 0;
  m_hvxParams.p_len  = &dataLength;
  m_hvxParams.p_data = static_cast<uint8_t*>(data);

  uint32_t err_code;
  err_code = sd_ble_gatts_hvx(m_connHandle, &m_hvxParams);
  ERROR_CHECK(err_code);

  while (!m_txComplete)
  {
    (void) sd_app_evt_wait();
  }
  m_txComplete = false;

  return static_cast <gn_char_error_t> (GN_SUCCESS);
}


gn_char_error_t Characteristic::update(void* data, uint16_t dataLength)
{
  if (true != m_readEnabled)                      { return GN_CHAR_READ_NOT_ENABLED; }
  if (true != m_isRunning)                        { return GN_CHAR_NOT_RUNNING; }
  if (dataLength > m_attrCharValue.max_len)       { return GN_CHAR_VALUE_TOO_LONG; }

  ble_gatts_value_t new_value;
  memset(&new_value, 0, sizeof(new_value));
  new_value.len     = dataLength;
  new_value.offset  = 0;
  new_value.p_value = static_cast<uint8_t*>(data);

  uint32_t err_code = sd_ble_gatts_value_set(BLE_CONN_HANDLE_INVALID, m_charHandle.value_handle, &new_value);
  ERROR_CHECK(err_code);

  return static_cast <gn_char_error_t> (GN_SUCCESS);
}



void Characteristic::eventHandler(ble_evt_t const * p_ble_evt)
{
  switch (p_ble_evt->header.evt_id)
  {
      case BLE_GAP_EVT_CONNECTED:
          m_connHandle = p_ble_evt->evt.gap_evt.conn_handle;
          break;
      case BLE_GAP_EVT_DISCONNECTED:
          m_connHandle = BLE_CONN_HANDLE_INVALID;
          m_notificationEnabled = false;
          break;
      case BLE_GATTS_EVT_WRITE:

        if (p_ble_evt->evt.gatts_evt.params.write.handle == m_charHandle.cccd_handle)
        {
          m_notificationEnabled = p_ble_evt->evt.gatts_evt.params.write.data[0];
        }
        if (p_ble_evt->evt.gatts_evt.params.write.handle == m_charHandle.value_handle)
        {
          EVENTS::eventPut(CHARACTERISTIC_WRITE_EVENT_OFFSET + m_eventIDModifier,
              p_ble_evt->evt.gatts_evt.params.write.data,
              p_ble_evt->evt.gatts_evt.params.write.len);
          /*
          if (m_writeHandler != NULL)
          {
           m_writeHandler(p_ble_evt->evt.gatts_evt.params.write.data, p_ble_evt->evt.gatts_evt.params.write.len);
          }
          */
        }
        break;

      case BLE_GATTS_EVT_HVN_TX_COMPLETE:
        m_txComplete = true;
        break;

/*
      case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        if (p_ble_evt->evt.gatts_evt.params.authorize_request.request.read.handle == m_charHandle.value_handle)
        {
          m_readHandler();
        }
        */

      default:
          // No implementation needed.
          break;
  }
}
/*
void Characteristic::updateConnectionState (uint16_t connHandle)
{

}

void Characteristic::updateReadHandler

*/


} // BLE_SERVER
