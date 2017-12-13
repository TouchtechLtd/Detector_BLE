
/*
 * detector_service.cpp
 *
 *  Created on: 12/12/2017
 *      Author: michaelmcadam
 */

#include <stdint.h>
#include <string.h>

#include "libraries/events/events.h"
#include "libraries/ble/ble_interface.h"
#include "libraries/ble/gn_ble_config.h"
#include "libraries/debug/DEBUG.h"

#include "./detector_service.h"

#define NRF_LOG_MODULE_NAME DETECTOR_SERVICE
NRF_LOG_MODULE_REGISTER();

namespace DETECTOR
{

namespace SERVICE
{


void eventConfigHandler(EVENTS::event_data_t data)
{
  detector_config_t config = *(detector_config_t*)data.p_data;;
  //memcpy(&config, data, len);
  setConfig(config);

  INFO("CONFIGURING - event config");

  uint32_t err_code = BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_CONFIG, &config, sizeof(detector_config_t));
  ERROR_CHECK(err_code);

}


void eventDisplayedHandler(EVENTS::event_data_t data)
{
  uint8_t requestedKill = *(uint8_t*)data.p_data;;
  showKill(requestedKill);

  /*
  TrapState::event_data_t tmpData = *getEvent(requestedKill);
  uint32_t err_code = BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DATA, &tmpData, sizeof(TrapState::event_data_t));
  EVENTS::eventPut(SEND_RAW_DATA, &requestedKill, 1);
  ERROR_CHECK(err_code);
  */
  INFO("Requested Kill: %d", requestedKill);
}


void start()
{
  INFO("INITIALISING - trap data service\t\t\t - UUID: %x, ID: %d", BLE_UUID_SERVICE_TRAP_DATA, SERVICE_TRAP_DATA);
  BLE_SERVER::Service trapData;
  trapData.createCustom(BLE_UUID_SERVICE_TRAP_DATA, BLE_UUID_GOODNATURE_BASE);

  /*
  INFO("INITIALISING - trap info characteristic\t\t - UUID: %x, ID: %d", BLE_UUID_CHAR_TRAP_INFO, CHAR_TRAP_INFO);
   BLE_SERVER::Characteristic trapInfoChar;
   //trap_info_t trapInfo = *TrapState::getTrapInfo();
   trapInfoChar.configure(BLE_UUID_CHAR_TRAP_INFO, &g_trapInfo, sizeof(g_trapInfo), BLE_SERVER::CHAR_READ_ONLY);
   trapData.addCharacteristic(&trapInfoChar, CHAR_TRAP_INFO);
*/

  INFO("INITIALISING - event data characteristic\t\t - UUID: %x, ID: %d", BLE_UUID_CHAR_TRAP_EVENT_DATA, CHAR_EVENT_DATA);
  BLE_SERVER::Characteristic eventData;
  event_data_t blankEvent = { 0 };
  eventData.configure(BLE_UUID_CHAR_TRAP_EVENT_DATA, &blankEvent, sizeof(blankEvent), BLE_SERVER::CHAR_READ_ONLY);
  trapData.addCharacteristic(&eventData, CHAR_EVENT_DATA);

  INFO("INITIALISING - event config characteristic\t\t - UUID: %x, ID: %d", BLE_UUID_CHAR_TRAP_EVENT_CONFIG, CHAR_EVENT_CONFIG);
  BLE_SERVER::Characteristic eventConfig;
  detector_config_t* defaultConfig = getConfig();
  eventConfig.configure(BLE_UUID_CHAR_TRAP_EVENT_CONFIG, defaultConfig, sizeof(*defaultConfig), BLE_SERVER::CHAR_READ_WRITE);
  trapData.addCharacteristic(&eventConfig, CHAR_EVENT_CONFIG);

  INFO("INITIALISING - event displayed characteristic\t - UUID: %x, ID: %d", BLE_UUID_CHAR_TRAP_EVENT_DISPLAYED, CHAR_EVENT_DISPLAYED);
  BLE_SERVER::Characteristic eventDisplayed;
  uint8_t* killNumber = getKillNumber();
  eventDisplayed.configure(BLE_UUID_CHAR_TRAP_EVENT_DISPLAYED, killNumber, sizeof(*killNumber), BLE_SERVER::CHAR_READ_WRITE);
  trapData.addCharacteristic(&eventDisplayed, CHAR_EVENT_DISPLAYED);

  /*
  INFO("INITIALISING - trap time characteristic\t\t - UUID: %x, ID: %d", BLE_UUID_CHAR_TRAP_TIME, CHAR_TRAP_TIME);
  BLE_SERVER::Characteristic trapTime;
  CurrentTime::current_time_t startTime = *CurrentTime::getCurrentTime();
  trapTime.configure(BLE_UUID_CHAR_TRAP_TIME, &startTime, sizeof(startTime), BLE_SERVER::CHAR_READ_WRITE);
  trapData.addCharacteristic(&trapTime, CHAR_TRAP_TIME);


  INFO("INITIALISING - raw event data characteristic\t - UUID: %x, ID: %d", BLE_UUID_CHAR_RAW_DATA, CHAR_RAW_DATA);
  BLE_SERVER::Characteristic rawEventData;
  raw_event_data_packet_t tmpRawData = { 0 };
  rawEventData.configure(BLE_UUID_CHAR_RAW_DATA, &tmpRawData, sizeof(tmpRawData), BLE_SERVER::CHAR_READ_ONLY);
  trapData.addCharacteristic(&rawEventData, CHAR_RAW_DATA);

  INFO("INITIALISING - error data characteristic\t\t - UUID: %x, ID: %d", BLE_UUID_CHAR_ERROR_DATA, CHAR_ERROR_DATA);
  BLE_SERVER::Characteristic errorData;
  uint32_t errorInitData = 0;
  errorData.configure(BLE_UUID_CHAR_ERROR_DATA, &errorInitData, sizeof(errorInitData), BLE_SERVER::CHAR_READ_ONLY);
  trapData.addCharacteristic(&errorData, CHAR_ERROR_DATA);

  INFO("INITIALISING - trap control characteristic\t\t - UUID: %x, ID: %d", BLE_UUID_CHAR_TRAP_CONTROL, CHAR_TRAP_CONTROL);
  BLE_SERVER::Characteristic trapControl;
  trap_control_t trapControlData = { 0 };
  trapControl.configure(BLE_UUID_CHAR_TRAP_CONTROL, &trapControlData, sizeof(trap_control_t), BLE_SERVER::CHAR_READ_WRITE);
  trapData.addCharacteristic(&trapControl, CHAR_TRAP_CONTROL);
*/

  INFO("ATTACHING - trap data service");
  trapData.attachService();
  BLE_SERVER::addService(&trapData, SERVICE_TRAP_DATA);


  BLE_SERVER::setWriteHandler(SERVICE_TRAP_DATA, CHAR_EVENT_DISPLAYED,   eventDisplayedHandler);
  BLE_SERVER::setWriteHandler(SERVICE_TRAP_DATA, CHAR_EVENT_CONFIG,      eventConfigHandler);
}


void update()
{
  INFO("UPDATING - Kill Number");
  uint8_t killNumber = *getKillNumber();
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DISPLAYED,  &killNumber,               sizeof(killNumber));

  INFO("UPDATING - Current Event");
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DATA,     getEvent(killNumber),      sizeof(event_data_t));

  INFO("UPDATING - Detector Config");
  detector_config_t detectorConfig = *getConfig();
  BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_CONFIG,     &detectorConfig,            sizeof(detector_config_t));

}

void sendKill(event_data_t* eventData)
{
  uint32_t err_code = BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_EVENT_DATA, eventData, sizeof(*eventData));
  ERROR_CHECK(err_code);
}

} // Namespace SERVICE
} // Namespace DETECTOR
