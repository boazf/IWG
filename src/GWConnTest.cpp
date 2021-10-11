#include <GWConnTest.h>
#include <EthernetUtil.h>
#ifdef USE_WIFI
#include <ping.h>
#else
#include <ICMPPing.h>
#endif
#include <Trace.h>

void GWConnTest::Start(time_t delay)
{
    if (hGWConnTestTask != NULL)
        return ;

    tDelay = delay;
    xTaskCreate(gwConnTestTask, "GWConnTest", 8 * 1024, this, tskIDLE_PRIORITY, &hGWConnTestTask);
}

void GWConnTest::gwConnTestTask(void *param)
{
    ((GWConnTest *)param)->gwConnTestTask();
}

void GWConnTest::gwConnTestTask()
{
    delay(tDelay);
    IPAddress gw = Eth.gatewayIP();
    Tracef("GWConnTest: Starting pinging %s\n", gw.toString().c_str());
    int attempts = 0;
#ifndef USE_WIFI
    ICMPPing ping(MAX_SOCK_NUM, 2);
#endif
    while (attempts < 5)
    {
#ifdef USE_WIFI
        if (ping_start(gw, 1, 0, 0, 1000))
#else
        if (ping(gw, 1).status != SUCCESS)
#endif
            attempts++;
        else
            attempts = 0;
        delay(500);
    }
    Traceln("GW Connection retrieved!");
    hGWConnTestTask = NULL;
    vTaskDelete(NULL);
}

bool GWConnTest::IsConnected()
{
    return hGWConnTestTask == NULL;
}

GWConnTest gwConnTest;