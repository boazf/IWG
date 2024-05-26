#ifndef TimeUtil_h
#define TimeUtil_h

#include <time.h>
#include <Observers.h>

void InitTime();
bool isValidTime(time_t t);

#define t_now (([]()->time_t{time_t now; time(&now); return now; })())

class TimeChangedParam
{
public:
    TimeChangedParam(time_t _currTime)
    {
        currTime = _currTime;
    }

    time_t currTime;
};

extern Observers<TimeChangedParam> timeChanged;

#endif // TimeUtil_h