#include <Arduino.h>
#include <TimeUtil.h>
#include <NTPClient.h>
#include <Config.h>
#include <Common.h>
#ifndef USE_WIFI
#include <sys/time.h>
#endif
#include <AppConfig.h>

static bool DST;

static void setTime(bool ignoreFailure)
{
#ifdef USE_WIFI
  configTime(Config::timeZone * 60 + (DST ? (Config::DST * 60) : 0), 0, Config::timeServer);
  tm tr1;
  delay(2000);
  tr1.tm_year = 0;
  getLocalTime(&tr1, 5000);
#else
  {
    unsigned long t0 = millis();
    time_t utc = 0;
    while(millis() - t0 < 5*60*1000 && (utc = NTPClient::getUTC()) == 0);
    if (utc == 0 && !ignoreFailure)
    {
#ifdef DEBUG_TIME
      Traceln("Failed to query current time from time server!");
#endif
      return;
    }

    time_t now = utc + Config::timeZone * 60 + (DST ? (Config::DST * 60) : 0);
    timeval tv = {now, 0};
    settimeofday(&tv, NULL);
  }
#endif

#ifdef DEBUG_TIME
  char buff[128];
  tm tr;
  time_t now = t_now;

  localtime_r(&now, &tr);
  strftime(buff, sizeof(buff), "DateTime: %a %d/%m/%Y %T%n", &tr);
  Trace(buff);
#endif
}

static void appConfigChanged(const AppConfigChangedParam &param, const void *context)
{
  if (DST != AppConfig::getDST())
  {
    DST = AppConfig::getDST();
#ifdef DEBUG_TIME
    Tracef("Daylight Saving Time changed: %s\n", DST ? "on" : "off");
#endif
    setTime(true);
  }
}

#ifndef USE_WIFI
static TaskHandle_t timeUpdateTaskHandle;

#define TIME_INIT_UPDATE_PERIODE_SEC 30

void timeUpdateTask(void *param)
{
  while (true)
  {
    tm tr;
    time_t now = t_now;
    localtime_r(&now, &tr);
    TickType_t tWait = (tr.tm_year < 100 ? TIME_INIT_UPDATE_PERIODE_SEC : (Config::timeUpdatePeriodMin * 60)) * 1000 / portTICK_PERIOD_MS;
    vTaskDelay(tWait);
    setTime(false);
  }
}
#endif

void InitTime()
{
#ifdef DEBUG_TIME
  {
    LOCK_TRACE();
    Trace("Time Server: ");
    Traceln(Config::timeServer);
  }
#endif
  DST = AppConfig::getDST();
  setTime(true);
  AppConfig::getAppConfigChanged().addObserver(appConfigChanged, NULL);
#ifndef USE_WIFI
  xTaskCreate(timeUpdateTask, "TimeUpdate", 8*1024, NULL, tskIDLE_PRIORITY, &timeUpdateTaskHandle);
#endif
}
