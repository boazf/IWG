#ifndef GWConnTest_h
#define GWConnTest_h

#include <Arduino.h>

class GWConnTest
{
public:
    GWConnTest() :
        hGWConnTestTask(NULL),
        isConnected(true)
    {
    }

    void Start(time_t delay);
    bool IsConnected();
    static bool ping(int attempts, int tInterval);

private:
    bool isConnected;
    TaskHandle_t hGWConnTestTask;
    time_t tDelay;

private:
    static void gwConnTestTask(void *param);
    void gwConnTestTask();
    static bool ping();
};

extern GWConnTest gwConnTest;

#endif // GWConnTest_h