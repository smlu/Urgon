#ifndef LIBIM_COGMESSAGE_TYPE_H
#define LIBIM_COGMESSAGE_TYPE_H
#include <cstdint>

namespace libim::content::asset {
    enum class CogMessageType
    {
        Activate    =  0,
        Activated   =  1,
        Removed     =  2,
        Startup     =  3,
        Timer       =  4,
        Blocker     =  5,
        Entered     =  6,
        Exited      =  7,
        Crossed     =  8,
        Sighted     =  9,
        Damaged     = 10,
        Arrived     = 11,
        Killed      = 12,
        Pulse       = 13,
        Touched     = 14,
        Created     = 15,
        Loading     = 16,
        Selected    = 17,
        Deselected  = 18,
        Aim         = 19,
        Changed     = 20,
        Deactivated = 21,
        Shutdown    = 22,
        Respawn     = 23,
        AIEvent     = 24,
        Callback    = 25,
        Taken       = 26,
        User0       = 27,
        User1       = 28,
        User2       = 29,
        User3       = 30,
        User4       = 31,
        User5       = 32,
        User6       = 33,
        User7       = 34,
        NewPlayer   = 35,
        Fire        = 36,
        Join        = 37,
        Leave       = 38,
        Splash      = 39,
        Trigger     = 40,
        StateChange = 41,
        Missed      = 42,
        Boarded     = 43,
        Unboarded   = 44,
        ArrivedWpnt = 45,
        Initialized = 46,
        UpdateWpnts = 47
    };
}

#endif // LIBIM_COGMESSAGE_TYPE_H
