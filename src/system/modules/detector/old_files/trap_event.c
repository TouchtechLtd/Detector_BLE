
/*
 * trap_event.cpp
 *
 *  Created on: Feb 28, 2017
 *      Author: Michael McAdam
 */

#include "./trap_manager_config.h"
#include "system/modules/time/current_time.h"
#include "drivers/timer/timer_interface.h"
#include "drivers/flash/flash_interface.h"

#include "libraries/debug/DEBUG.h"

#include <stdio.h>
#include <stdint.h>

#define MAX_ADC_VALUE 1024

uint8_t TrapEvent::numberOfKills = 0;



TrapEvent::TrapEvent() {
  m_didClip = false;
	m_peakValue = 0;
	m_dataCount = 0;

	//Flash_Record::read(KILL_NUMBER_FILE_ID, KILL_NUMBER_KEY_ID, &numberOfKills, sizeof(numberOfKills));
}


void TrapEvent::triggered()
{
  INFO("Trap Triggered");
}

void TrapEvent::record()
{
  Flash_Record::read(KILL_NUMBER_FILE_ID, KILL_NUMBER_KEY_ID, &numberOfKills, sizeof(numberOfKills));
  ++numberOfKills;
  trap_data.trap_id = 0;          // getID
  trap_data.peak_level = 100;     // getPeak
  trap_data.timestamp = CurrentTime::getCurrentTime();      // getTime
  trap_data.temperature = 100;    // getTemp
  trap_data.killNumber = numberOfKills;
  //memcpy(&trap_data.raw_data, m_rawData, sizeof(m_rawData));
  Flash_Record::write(KILL_DATA_FILE_ID, numberOfKills, &trap_data, sizeof(trap_data));
  Flash_Record::write(KILL_NUMBER_FILE_ID, KILL_NUMBER_KEY_ID, &numberOfKills, sizeof(numberOfKills));
  //clear();
}

void TrapEvent::cancel()
{
  clear();
}



event_data_t TrapEvent::getEvent(uint8_t eventID)
{
  event_data_t recordData = { 0 };
  Flash_Record::read(KILL_DATA_FILE_ID, eventID, &recordData, sizeof(recordData));
  return recordData;
}


void TrapEvent::clear(void) {
  memset(&trap_data, 0, sizeof(event_data_t));
}


void TrapEvent::addData(int dataPoint) {
	if (m_dataCount < RAW_DATA_CAPTURE_SIZE)
	{
	  m_rawData[m_dataCount] = dataPoint;
    m_dataCount++;
	}
	findPeak(dataPoint);
}

void TrapEvent::findPeak(int dataPoint) {
  if (dataPoint > m_peakValue)
  {
    m_peakValue = dataPoint;
  }
}

void TrapEvent::printData(void) {
  //INFO("Kill Number: %d", m_killNumber);
  INFO("Peak Value: %d", m_peakValue);
  INFO("Clipping: %d", m_didClip);
  INFO("");
}

uint8_t* TrapEvent::getKillNumber()
{
  Flash_Record::read(KILL_NUMBER_FILE_ID, KILL_NUMBER_KEY_ID, &numberOfKills, sizeof(numberOfKills));
  return &numberOfKills;
}

/*
void TrapEvent::didOccur()
{
  return newKill;
}
*/
