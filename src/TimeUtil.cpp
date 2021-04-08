#include <Arduino.h>
#include <TimeUtil.h>
#include <NTPClient.h>
#include <Config.h>
#include <Common.h>
#ifndef USE_WIFI
#include <sys/time.h>
#endif

void InitTime()
{
#ifdef DEBUG_TIME
  Trace("Time Server: ");
  Traceln(Config::timeServer);
#endif
#ifdef USE_WIFI
  configTime(Config::timeZone * 60, 0, Config::timeServer);
  tm tr1;
  delay(2000);
  tr1.tm_year = 0;
  getLocalTime(&tr1, 5000);
#else
  {
    time_t now = NTPClient::getUTC() + Config::timeZone * 60;
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
