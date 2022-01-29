#ifndef Common_h
#define Common_h

#include <Trace.h>
#include <Lock.h>

//#define DEBUG_SD
// #define DEBUG_ETHERNET
// #define DEBUG_TIME
// #define DEBUG_SFT
// #define DEBUG_CONFIG
// #define DEBUG_HTTP_SERVER
// #define DEBUG_RECOVERY_CONTROL
// #define DEBUG_HISTORY
// #define DEBUG_STATE_MACHINE
// #define DEBUG_POWER

#define NELEMS(a) (sizeof(a)/sizeof(*a))
#define MAX_PATH 128

extern CriticalSection csSpi;

#endif // Common_h