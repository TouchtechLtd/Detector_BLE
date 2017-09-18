#ifndef _STATE_MACHINE_H
#define _STATE_MACHINE_H
//#include <stdio.h>

#ifndef NULL
#define NULL 0
#endif

class EventData
{
public:
    //virtual ~EventData() {};
};

struct StateStruct;

// base class for state machines
class StateMachine
{
public:
    StateMachine(unsigned char maxStates);
    //virtual ~StateMachine() {}

private:
    const unsigned char _maxStates;
    bool _eventGenerated;
    EventData* _pEventData;
    void StateEngine(void);


protected:
    enum { EVENT_IGNORED = 0xFE, CANNOT_HAPPEN };
    unsigned char currentState;
    void ExternalEvent(unsigned char, EventData* = NULL);
    void InternalEvent(unsigned char, EventData* = NULL);
    virtual const StateStruct* GetStateMap() = 0;
};

typedef void (StateMachine::*StateFunc)(EventData *);
struct StateStruct
{
    StateFunc pStateFunc;
};

#define BEGIN_STATE_MAP \
public:\
const StateStruct* GetStateMap() {\
    static const StateStruct StateMap[] = {

#define STATE_MAP_ENTRY(stateFunc)\
    { reinterpret_cast<StateFunc>(stateFunc) },

#define END_STATE_MAP \
    }; \
    return &StateMap[0]; }

#define BEGIN_TRANSITION_MAP \
    static const unsigned char TRANSITIONS[] = {\

#define TRANSITION_MAP_ENTRY(entry)\
    entry,

#define END_TRANSITION_MAP(data) \
    0 };\
    ExternalEvent(TRANSITIONS[currentState], data);

#endif //STATE_MACHINE_H
