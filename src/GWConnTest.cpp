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

bool GWConnTest::ping(int attempts, int tInterval)
{
    bool success = true;

    for (int i = 0; i < attempts && success; i++)
    {
        delay(tInterval);
        success = ping();
    }

    return success;
}

bool GWConnTest::ping()
{
    IPAddress gw = Eth.gatewayIP();
#ifdef USE_WIFI
    return ping_start(gw, 1);
#else
    ICMPPing ping(MAX_SOCK_NUM, 2);
    return ping(gw, 1).status == SUCCESS;
#endif
}

void GWConnTest::gwConnTestTask()
{
    delay(tDelay);
#ifdef DEBUG_ETHERNET
    Tracef("GWConnTest: Starting pinging %s\n", Eth.gatewayIP().toString().c_str());
#endif
    while (!ping(5, 500));
#ifdef DEBUG_ETHERNET
    Traceln("GW Connection retrieved!");
#endif
    hGWConnTestTask = NULL;
    vTaskDelete(NULL);
}

bool GWConnTest::IsConnected()
{
    return hGWConnTestTask == NULL;
}

GWConnTest gwConnTest;