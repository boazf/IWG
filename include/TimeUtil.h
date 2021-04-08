#ifndef TimeUtil_h
#define TimeUtil_h

#include <time.h>

void InitTime();
#define t_now (([](){time_t now; time(&now); return now; })())

#endif // TimeUtil_h