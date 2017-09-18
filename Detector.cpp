
#include "uart_interface.h"
#include <Detector.h>

#ifndef NULL
#define NULL 0
#endif

using namespace std;

// halt motor external event
void Detector::Still(void)
{
    // given the Halt event, transition to a new state based upon
    // the current state of the state machine
    BEGIN_TRANSITION_MAP                      // - Current State -
        TRANSITION_MAP_ENTRY (ST_DETECT)  		// ST_Moving
        TRANSITION_MAP_ENTRY (CANNOT_HAPPEN)  	// ST_Detect
        TRANSITION_MAP_ENTRY (CANNOT_HAPPEN)    // ST_Capture
        TRANSITION_MAP_ENTRY (CANNOT_HAPPEN)    // ST_Process
		TRANSITION_MAP_ENTRY (CANNOT_HAPPEN)    // ST_Communicate
    END_TRANSITION_MAP(NULL)
}


// halt motor external event
void Detector::Moving(void)
{
    // given the Halt event, transition to a new state based upon 
    // the current state of the state machine
    BEGIN_TRANSITION_MAP                      // - Current State -
        TRANSITION_MAP_ENTRY (ST_MOVING)  		// ST_Moving
        TRANSITION_MAP_ENTRY (ST_MOVING)  		// ST_Detect
        TRANSITION_MAP_ENTRY (ST_MOVING)    	// ST_Capture
        TRANSITION_MAP_ENTRY (ST_MOVING)    	// ST_Process
		TRANSITION_MAP_ENTRY (ST_MOVING)    	// ST_Communicate
    END_TRANSITION_MAP(NULL)
}


// halt motor external event
void Detector::TrapTriggered(void)
{
    // given the Halt event, transition to a new state based upon
    // the current state of the state machine
    BEGIN_TRANSITION_MAP                      // - Current State -
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  	// ST_Moving
        TRANSITION_MAP_ENTRY (ST_CAPTURE)  		// ST_Detect
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)    // ST_Capture
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)    // ST_Process
		TRANSITION_MAP_ENTRY (ST_CAPTURE)    	// ST_Communicate
    END_TRANSITION_MAP(NULL)
}


// halt motor external event
void Detector::EventFinished(void)
{
    // given the Halt event, transition to a new state based upon
    // the current state of the state machine
    BEGIN_TRANSITION_MAP                      // - Current State -
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  		// ST_Moving
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  		// ST_Detect
        TRANSITION_MAP_ENTRY (ST_PROCESS)    		// ST_Capture
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)    	// ST_Process
		TRANSITION_MAP_ENTRY (EVENT_IGNORED)    	// ST_Communicate
    END_TRANSITION_MAP(NULL)
}

// halt motor external event
void Detector::ProcessingFinished(void)
{
    // given the Halt event, transition to a new state based upon
    // the current state of the state machine
    BEGIN_TRANSITION_MAP                      // - Current State -
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  		// ST_Moving
        TRANSITION_MAP_ENTRY (CANNOT_HAPPEN)  		// ST_Detect
        TRANSITION_MAP_ENTRY (CANNOT_HAPPEN)    	// ST_Capture
        TRANSITION_MAP_ENTRY (ST_DETECT)    		// ST_Process
		TRANSITION_MAP_ENTRY (CANNOT_HAPPEN)    	// ST_Communicate
    END_TRANSITION_MAP(NULL)
}

// halt motor external event
void Detector::Update(void)
{
    // given the Halt event, transition to a new state based upon
    // the current state of the state machine
    BEGIN_TRANSITION_MAP                      // - Current State -
        TRANSITION_MAP_ENTRY (ST_COMMUNICATE)  		// ST_Moving
        TRANSITION_MAP_ENTRY (ST_COMMUNICATE)  		// ST_Detect
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)    		// ST_Capture
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)    	// ST_Process
		TRANSITION_MAP_ENTRY (EVENT_IGNORED)    	// ST_Communicate
    END_TRANSITION_MAP(NULL)
}


// halt motor external event
void Detector::UpdateFinished(void)
{
    // given the Halt event, transition to a new state based upon
    // the current state of the state machine
    BEGIN_TRANSITION_MAP                      // - Current State -
        TRANSITION_MAP_ENTRY (CANNOT_HAPPEN)  		// ST_Moving
        TRANSITION_MAP_ENTRY (CANNOT_HAPPEN)  		// ST_Detect
        TRANSITION_MAP_ENTRY (CANNOT_HAPPEN)    		// ST_Capture
        TRANSITION_MAP_ENTRY (CANNOT_HAPPEN)    	// ST_Process
		TRANSITION_MAP_ENTRY (ST_DETECT)    	// ST_Communicate
    END_TRANSITION_MAP(NULL)
}

/*

// set motor speed external event
void Detector::SetSpeed(MotorData* pData)
{
    BEGIN_TRANSITION_MAP                      // - Current State -
        TRANSITION_MAP_ENTRY (ST_START)       // ST_Idle       
        TRANSITION_MAP_ENTRY (CANNOT_HAPPEN)  // ST_Stop       
        TRANSITION_MAP_ENTRY (ST_CHANGE_SPEED)// ST_Start      
        TRANSITION_MAP_ENTRY (ST_CHANGE_SPEED)// ST_ChangeSpeed
    END_TRANSITION_MAP(pData)
}

*/

// state machine sits here when motor is not running
void Detector::ST_Moving(DetectorData* pData)
{
	UART::write("Detector::ST_Moving");
}

// stop the motor 
void Detector::ST_Detect(DetectorData* pData)
{
	UART::write("Detector::ST_Detect");

    // perform the stop motor processing here
    // transition to ST_Idle via an internal event
    //InternalEvent(ST_IDLE);
}

// start the motor
void Detector::ST_Capture(DetectorData* pData)
{
	UART::write("Detector::ST_Capture");
    // set initial motor speed processing here
}

// changes the motor speed once the motor is moving
void Detector::ST_Process(DetectorData* pData)
{
	UART::write("Detector::ST_Process");
    // perform the change motor speed to pData->speed here
}


// changes the motor speed once the motor is moving
void Detector::ST_Communicate(DetectorData* pData)
{
	UART::write("Detector::ST_Communicate");
    // perform the change motor speed to pData->speed here
}
