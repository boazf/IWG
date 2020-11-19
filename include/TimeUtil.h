#ifndef TimeUtil_h
#define TimeUtil_h

#include <time.h>

void InitTime();
void getFatDateTime(uint16_t *date, uint16_t *time);

extern volatile time_t t;

#endif // TimeUtil_h