#ifndef GWConnTest_h
#define GWConnTest_h

#include <Arduino.h>

class GWConnTest
{
public:
    GWConnTest() :
        hGWConnTestTask(NULL)
    {
    }

    void Start(time_t delay);
    bool IsConnected();

private:
    TaskHandle_t hGWConnTestTask;
    time_t tDelay;

private:
    static void gwConnTestTask(void *param);
    void gwConnTestTask();
};

extern GWConnTest gwConnTest;

#endif // GWConnTest_h