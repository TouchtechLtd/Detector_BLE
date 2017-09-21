
/*
 * trap_event.cpp
 *
 *  Created on: Feb 28, 2017
 *      Author: Michael McAdam
 */

#include "app/trap_event.h"
#include "debug/DEBUG.h"

#include <stdio.h>
#include <algorithm>
#include <numeric>

#define MAX_ADC_VALUE 4095
#define AVERAGE_SIZE 300



TrapEvent::TrapEvent() {
	eventTriggered = false;
	didClip = false;
	dataProcessed = false;

	timeStamp = 0;
	responseStartTime = 0;
	responseEndTime = 0;
	responseLength = 0;

	peakValue = 0;
	responseSize = 0;
}


void TrapEvent::addData(int dataPoint) {
	if (eventTriggered) rawData.push_back(dataPoint);
}


void TrapEvent::clear(void) {
	dataProcessed = false;
	eventTriggered = false;

	timeStamp = 0;
	responseStartTime = 0;
	responseEndTime = 0;

	peakValue = 0;
	responseSize = 0;
	rawData.clear();
}

void TrapEvent::start(void) {
	eventTriggered = true;
	responseStartTime = 0;
}

void TrapEvent::end(void) {
	eventTriggered = false;
	responseEndTime = 10;
}

void TrapEvent::processData(void) {
	calculateLength();
	findPeakValue();
	checkForClipping();
	findResponseSize();
}

void TrapEvent::printData(void) {
	UART::write("Response Length: %lu\n", responseLength);
	UART::write("Peak Value: %u\n", peakValue);
	UART::write("Response Size: %u\n", responseSize);
	UART::write("Clipping: %d\n", didClip);
}


void TrapEvent::checkForClipping(void) {
	didClip = (peakValue == MAX_ADC_VALUE);
}

void TrapEvent::calculateLength(void) {
	//responseLength = responseEndTime - responseStartTime;
	responseLength = rawData.size();
}

void TrapEvent::findPeakValue(void) {
	peakValue = *std::max_element(rawData.begin(),rawData.end());
}

void TrapEvent::findResponseSize(void) {
	if (rawData.size() > AVERAGE_SIZE)
	responseSize = std::accumulate(rawData.begin(), rawData.begin() + AVERAGE_SIZE, 0LL) / AVERAGE_SIZE;
}


