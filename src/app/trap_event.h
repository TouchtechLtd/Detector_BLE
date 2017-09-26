
/*
 * TrapEvent.h
 *
 *  Created on: September 5, 2017
 *      Author: Michael McAdam
 */

#ifndef CPP_TRAP_EVENT_H
#define CPP_TRAP_EVENT_H
#include <stdint.h>


#define TRAP_EVENT_THRESHOLD 100
#define MAX_TRAP_EVENTS 100
#define AVERAGE_SIZE 20

class TrapEvent {
	private:

    static uint8_t numberOfKills;

		uint32_t m_timeStamp;
		uint32_t m_responseStartTime;
		uint32_t m_responseEndTime;
		uint32_t m_responseLength;

		uint16_t m_peakValue;
		uint32_t m_responseSize;
		uint16_t m_dataCount;

		bool m_eventTriggered;
		bool m_didClip;
		bool m_dataProcessed;

		void checkForClipping();
		void findPeakValue();
		void findResponseSize();
		void calculateLength();

		uint16_t m_rawData[AVERAGE_SIZE];

		uint8_t m_killNumber;

	public:
		TrapEvent();
		void addData(int dataPoint);
		void findPeak(int dataPoint);
		void processData();
		void clear();
		void start();
		void end();
		void setTimeStamp(uint32_t currentTime);
		void printData();

		uint8_t getKillNumber();
		uint32_t getResponseLength();
		uint16_t getPeakValue();
		uint16_t getResponseSize();
		uint8_t getDidClip();

	}; // End TrapEvent

#endif /* CPP_TRAP_EVENT_H */
