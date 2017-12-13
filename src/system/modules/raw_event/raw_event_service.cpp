
/*
 * raw_event_service.cpp
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

#include "./raw_event_service.h"

#include "nrf_delay.h"

#define NRF_LOG_MODULE_NAME RAW_EVENT_SERVICE
NRF_LOG_MODULE_REGISTER();

namespace RAW_EVENT
{

namespace SERVICE
{

#define RAW_DATA_BLE_SIZE 4


#pragma pack(push, 1)
typedef struct
{
  raw_event_data_t data[RAW_DATA_BLE_SIZE];
  uint8_t packetNumber;
  uint8_t eventNumber;
} raw_event_data_packet_t;
#pragma pack(pop)




void rawEventDisplayedHandler(EVENTS::event_data_t data)
{
  uint8_t requestedEvent = *(uint8_t*)data.p_data;;
  showEvent(requestedEvent);

  INFO("Requested Event: %d", requestedEvent);
}


void start()
{
  INFO("INITIALISING - trap raw data service\t\t\t - UUID: %x, ID: %d", BLE_UUID_SERVICE_RAW_EVENT, SERVICE_TRAP_DATA);
  BLE_SERVER::Service rawEvent;
  rawEvent.createCustom(BLE_UUID_SERVICE_RAW_EVENT, BLE_UUID_GOODNATURE_BASE);



  INFO("INITIALISING - raw event data characteristic\t - UUID: %x, ID: %d", BLE_UUID_CHAR_RAW_EVENT, CHAR_RAW_EVENT);
  BLE_SERVER::Characteristic rawEventData;
  raw_event_data_packet_t tmpRawData = { 0 };
  rawEventData.configure(BLE_UUID_CHAR_RAW_EVENT, &tmpRawData, sizeof(tmpRawData), BLE_SERVER::CHAR_READ_ONLY);
  rawEvent.addCharacteristic(&rawEventData, CHAR_RAW_EVENT);


  INFO("INITIALISING - raw event displayed characteristic\t - UUID: %x, ID: %d", BLE_UUID_CHAR_RAW_EVENT_DISPLAYED, CHAR_RAW_EVENT_DISPLAYED);
  BLE_SERVER::Characteristic rawEventDisplayed;
  uint8_t eventNumber =  *getEventNumber();
  rawEventDisplayed.configure(BLE_UUID_CHAR_RAW_EVENT_DISPLAYED, &eventNumber, sizeof(eventNumber), BLE_SERVER::CHAR_READ_WRITE);
  rawEvent.addCharacteristic(&rawEventDisplayed, CHAR_RAW_EVENT_DISPLAYED);



  INFO("ATTACHING - trap raw data service");
  rawEvent.attachService();
  BLE_SERVER::addService(&rawEvent, SERVICE_RAW_EVENT);

  BLE_SERVER::setWriteHandler(SERVICE_RAW_EVENT, CHAR_RAW_EVENT_DISPLAYED,   rawEventDisplayedHandler);


}


void update()
{

}



void sendEvent(raw_event_t* rawEvent)
{
  raw_event_data_packet_t eventPacket = { 0 };

  eventPacket.eventNumber = rawEvent->eventNumber;

  uint32_t err_code;
  for (int i = 0; i < RAW_EVENT_SIZE/RAW_DATA_BLE_SIZE; i++)
  {
    memcpy(&eventPacket.data, &rawEvent->data[i*RAW_DATA_BLE_SIZE], sizeof(eventPacket.data));
    eventPacket.packetNumber = i;
    err_code = BLE_SERVER::setCharacteristic(SERVICE_RAW_EVENT, CHAR_RAW_EVENT, &eventPacket, sizeof(eventPacket));

    ERROR_CHECK(err_code);
    //nrf_delay_ms(10);
  }
  INFO("SEND - Raw Event Data");
}



} // Namespace SERVICE
} // Namespace DETECTOR
