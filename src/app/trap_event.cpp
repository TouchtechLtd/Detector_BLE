
/*
 * trap_event.cpp
 *
 *  Created on: Feb 28, 2017
 *      Author: Michael McAdam
 */

#include "app/trap_event.h"
#include "peripheral/timer_interface.h"
#include "debug/DEBUG.h"

#include <stdio.h>
#include <stdint.h>

#define MAX_ADC_VALUE 1024

uint8_t TrapEvent::numberOfKills = 0;

TrapEvent::TrapEvent() {
  m_eventTriggered = false;
  m_didClip = false;
  m_dataProcessed = false;

	m_timeStamp = 0;
	m_responseStartTime = 0;
	m_responseEndTime = 0;
	m_responseLength = 0;

	m_peakValue = 0;
	m_responseSize = 0;
	m_dataCount = 0;

	m_killNumber = 0;
}


void TrapEvent::addData(int dataPoint) {
	if (m_dataCount < AVERAGE_SIZE)
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


void TrapEvent::clear(void) {
  m_dataProcessed = false;
	m_eventTriggered = false;

	m_timeStamp = 0;
	m_responseStartTime = 0;
	m_responseEndTime = 0;

	m_peakValue = 0;
	m_responseSize = 0;
	m_dataCount = 0;
}

void TrapEvent::start(void) {
  m_eventTriggered = true;
	m_responseStartTime = Timer::getTicks();
}

void TrapEvent::end(void) {
  m_eventTriggered = false;
	m_responseEndTime = Timer::getTicks();
}

void TrapEvent::processData(void) {
  m_killNumber = ++numberOfKills;
	calculateLength();
	//findPeakValue();
	checkForClipping();
	findResponseSize();
}

void TrapEvent::printData(void) {
  DEBUG("Kill Number: %d", m_killNumber);
	DEBUG("Response Length: %d s", m_responseLength);
	DEBUG("Peak Value: %d", m_peakValue);
	DEBUG("Response Size: %d", m_responseSize);
	DEBUG("Clipping: %d", m_didClip);
}

uint8_t TrapEvent::getKillNumber() {
  return m_killNumber;
}

uint32_t TrapEvent::getResponseLength() {
  return m_responseLength;
}

uint16_t TrapEvent::getPeakValue() {
  return m_peakValue;
}

uint16_t TrapEvent::getResponseSize() {
  return (uint16_t)m_responseSize;
}

uint8_t TrapEvent::getDidClip() {
    return m_didClip;
}

void TrapEvent::checkForClipping(void) {
  m_didClip = (m_peakValue == MAX_ADC_VALUE);
}

void TrapEvent::calculateLength(void) {
  m_responseLength = Timer::getDiff(m_responseEndTime, m_responseStartTime) / Timer::getFrequency();
}

void TrapEvent::findPeakValue(void) {
  //m_peakValue = *std::max_element(m_rawData.begin(),m_rawData.end());
}

void TrapEvent::findResponseSize(void) {
  if (m_dataCount > 0) {
    uint32_t sum = 0;
    for (int i = 0; i < m_dataCount; i++)
    {
      sum += m_rawData[i];
    }
    m_responseSize = sum / m_dataCount;
  }

  //if (rawData.size() > AVERAGE_SIZE)
	//responseSize = std::accumulate(rawData.begin(), rawData.begin() + AVERAGE_SIZE, 0LL) / AVERAGE_SIZE;
}


void TrapEvent::setTimeStamp(uint32_t currentTime) {
  m_timeStamp = currentTime;
}

