#ifndef _DETECTOR_H
#define _DETECTOR_H
#include "StateMachine.h"

// structure to hold event data passed into state machine
struct DetectorData : public EventData
{
    int speed;
};

// the Motor state machine class
class Detector : public StateMachine
{
public:
	Detector() : StateMachine(ST_MAX_STATES) {}

    // external events taken by this state machine
    void Still();
    void Moving();
    void TrapTriggered();
    void EventFinished();
    void ProcessingFinished();
    void Update();
    void UpdateFinished();

private:
    // state machine state functions
    void ST_Moving(DetectorData*);
    void ST_Detect(DetectorData*);
    void ST_Capture(DetectorData*);
    void ST_Process(DetectorData*);
    void ST_Communicate(DetectorData*);

    // state map to define state function order
    BEGIN_STATE_MAP
		STATE_MAP_ENTRY(&Detector::ST_Moving)
        STATE_MAP_ENTRY(&Detector::ST_Detect)
        STATE_MAP_ENTRY(&Detector::ST_Capture)
        STATE_MAP_ENTRY(&Detector::ST_Process)
		STATE_MAP_ENTRY(&Detector::ST_Communicate)
    END_STATE_MAP

    // state enumeration order must match the order of state
    // method entries in the state map
    enum E_States { 
        ST_MOVING = 0,
        ST_DETECT,
        ST_CAPTURE,
        ST_PROCESS,
		ST_COMMUNICATE,
        ST_MAX_STATES
    };
};
#endif // _DETECTOR_H
