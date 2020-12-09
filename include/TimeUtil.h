#ifndef TimeUtil_h
#define TimeUtil_h

#include <time.h>

void InitTime();
#ifdef ESP32
#include <Config.h>
#define t_now (([](){time_t now; time(&now); return now; })())
#else
void getFatDateTime(uint16_t *date, uint16_t *time);

extern volatile time_t t_now;
#endif

#endif // TimeUtil_h