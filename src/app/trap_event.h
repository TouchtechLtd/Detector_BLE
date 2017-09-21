
/*
 * TrapEvent.h
 *
 *  Created on: September 5, 2017
 *      Author: Michael McAdam
 */

#ifndef CPP_TRAP_EVENT_H
#define CPP_TRAP_EVENT_H
#include <vector>


#define TRAP_EVENT_THRESHOLD 100
#define MAX_TRAP_EVENTS 100

class TrapEvent {
	private:

		unsigned long timeStamp;
		unsigned long responseStartTime;
		unsigned long responseEndTime;
		unsigned long responseLength;

		unsigned int peakValue;
		unsigned int responseSize;

		bool eventTriggered;
		bool didClip;
		bool dataProcessed;

		void checkForClipping();
		void findPeakValue();
		void findResponseSize();
		void calculateLength();

	public:
		TrapEvent();
		void addData(int dataPoint);
		void processData();
		void clear();
		void start();
		void end();
		void printData();

		std::vector<int> rawData;

	}; // End TrapEvent

#endif /* CPP_TRAP_EVENT_H */
