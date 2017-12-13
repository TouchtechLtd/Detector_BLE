
/*
 * detector_state.cpp
 *
 *  Created on: 11/12/2017
 *      Author: michaelmcadam
 */

#include <stdint.h>
#include <string.h>

#include "libraries/events/events.h"
#include "libraries/debug/DEBUG.h"

#include "./gn_raw_event.h"
#include "./raw_event_state.h"
#include "./raw_event_service.h"
#include "./raw_event_storage.h"

#include <math.h>

#define NRF_LOG_MODULE_NAME RAW_EVENT
NRF_LOG_MODULE_REGISTER();


namespace RAW_EVENT
{

static raw_event_t    g_eventData = { 0 };
static uint16_t       g_eventDataCount = 0;
static uint8_t       g_eventCount = 0;



void startRawSampling()
{
  LIS2DH12::startDAPolling();
}

void stopRawSampling()
{
  LIS2DH12::stopDAPolling();
}


void startEventCapture()
{
  startRawSampling();
  g_eventCount++;
  g_eventData.eventNumber = g_eventCount;
}


void accReadRawDataHandler(void* p_context)
{

  LIS2DH12::sample();

  if (g_eventDataCount < RAW_EVENT_SIZE)
  {
    LIS2DH12::getAccelerationData(&g_eventData.data[g_eventDataCount].acc);
  }
  else
  {
    EVENTS::eventPut(RAW_DATA_FULL);
    stopRawSampling();
  }

  g_eventDataCount++;

}

void findAccelerationSum()
{
  for (int i = 0; i < RAW_EVENT_SIZE; i++)
  {
    g_eventData.data[i].sum = sqrt((g_eventData.data[i].acc.x*g_eventData.data[i].acc.x) +
                                     (g_eventData.data[i].acc.y*g_eventData.data[i].acc.y) +
                                     (g_eventData.data[i].acc.z*g_eventData.data[i].acc.z));
  }
}



void findEventPeak()
{
  for (int i = 0; i < RAW_EVENT_SIZE; i++)
  {
    if (g_eventData.data[i].sum > g_eventData.peakLevel)
    {
      g_eventData.peakLevel = g_eventData.data[i].sum;
    }
  }
}

uint8_t getEventPeak(uint8_t eventID)
{
  raw_event_t* record_data = STORAGE::getEvent(eventID);
  return record_data->peakLevel;
}


uint8_t* getEventNumber()
{
  return &g_eventCount;
}

void processRawData()
{
  INFO("PROCESSING - Event Data Number: %d", g_eventData.eventNumber);
  findAccelerationSum();
  findEventPeak();
  g_eventData.processed = true;
  EVENTS::eventPut(RAW_DATA_PROCESSED);
}

void storeRawData()
{
  INFO("STORING - Event Data Number: %d", g_eventData.eventNumber);
  STORAGE::recordEvent(&g_eventData);
}

void resetRawData()
{
  INFO("RESETING - Event Data Number: %d", g_eventData.eventNumber);
  g_eventDataCount = 0;
  memset(&g_eventData, 0, sizeof(g_eventData));
}


void showEvent(uint8_t eventID)
{
  raw_event_t* record_data = STORAGE::getEvent(eventID);
  SERVICE::sendEvent(record_data);
}

/*
void sendRawData(EVENTS::event_data_t data)
{
  uint8_t requestedKill = *(uint8_t*)data.p_data;

  INFO("SENDING - Raw Data for kill: %d", requestedKill);
  raw_event_data_packet_t bleRawData = { 0 };
  bleRawData.killNumber = requestedKill;

  raw_event_data_t requestedRawData[RAW_DATA_CAPTURE_SIZE] = { 0 };
  Flash_Record::read(KILL_RAW_DATA_FILE_ID, requestedKill, &requestedRawData, sizeof(requestedRawData));

  uint32_t err_code;
  for (int i = 0; i < RAW_DATA_CAPTURE_SIZE/RAW_DATA_BLE_SIZE; i++)
  {
    //INFO("SENDING - Raw Data Kill %d Packet: %d/%d", requestedKill, i, RAW_DATA_CAPTURE_SIZE/RAW_DATA_BLE_SIZE);

    memcpy(&bleRawData.data, &requestedRawData[i*RAW_DATA_BLE_SIZE], sizeof(bleRawData.data));
    bleRawData.packetNumber = i;
    err_code = BLE_SERVER::setCharacteristic(SERVICE_TRAP_DATA, CHAR_RAW_DATA, &bleRawData, sizeof(bleRawData));

    ERROR_CHECK(err_code);
    //NRF_LOG_PROCESS();
    nrf_delay_ms(10);
  }

}
*/

void registerEvents()
{
  EVENTS::registerEventHandler(EVENT_TRIGGERED,     startEventCapture);
  EVENTS::registerEventHandler(RAW_DATA_FULL,       processRawData);
  EVENTS::registerEventHandler(RAW_DATA_PROCESSED,  storeRawData);
  EVENTS::registerEventHandler(RAW_DATA_PROCESSED,  resetRawData);
}


void init()
{
  LIS2DH12::initDAPolling(accReadRawDataHandler);

  //STATE::start();
  SERVICE::start();
  STORAGE::start();

  registerEvents();
}


}
