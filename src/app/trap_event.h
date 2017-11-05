
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

#define RAW_DATA_CAPTURE_SIZE 250


/* A dummy structure to save in flash. */
#pragma pack(push, 1)
typedef struct
{
    uint8_t       peak_level;
    uint32_t      timestamp;
    uint32_t      trap_id;
    uint16_t      temperature;
    uint8_t       killNumber;
} event_data_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
  uint16_t      raw_data_size;
  uint8_t       raw_data[RAW_DATA_CAPTURE_SIZE];
};


class TrapEvent {
	private:

    static uint8_t numberOfKills;

    event_data_t trap_data;

		uint16_t m_peakValue;
		uint16_t m_dataCount;

		bool m_didClip;
		uint8_t m_rawData[RAW_DATA_CAPTURE_SIZE];

	public:
		TrapEvent();
		void triggered();
		void record();
		void cancel();

		event_data_t getEvent(uint16_t eventID);

		void printData(void);
		void addData(int dataPoint);
		void findPeak(int dataPoint);
		void clear();
	}; // End TrapEvent

#endif /* CPP_TRAP_EVENT_H */
