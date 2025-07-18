/*
 * Copyright 2020-2025 Boaz Feldboim
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// SPDX-License-Identifier: Apache-2.0

#include <Arduino.h>
#include <TimeUtil.h>
#include <NTPClient.h>
#include <Config.h>
#include <Common.h>
#ifndef USE_WIFI
#include <sys/time.h>
#endif
#include <AppConfig.h>
#ifdef DEBUG_TIME
#include <Trace.h>
#endif
static bool DST;
Observers<TimeChangedParam> timeChanged;

#define MAX_WAIT_TIME_FOR_NTP_SUCCESS_SEC 30

static void setTime(bool ignoreFailure)
{
#ifdef USE_WIFI
  configTime(Config::timeZone * 60 + (DST ? (Config::DST * 60) : 0), 0, Config::timeServer);
  tm tr1;
  delay(2000);
  tr1.tm_year = 0;
  if (!getLocalTime(&tr1, MAX_WAIT_TIME_FOR_NTP_SUCCESS_SEC * 1000))
#else
  unsigned long t0 = millis();
  time_t utc = 0;
  while(millis() - t0 < MAX_WAIT_TIME_FOR_NTP_SUCCESS_SEC * 1000 && !isValidTime(utc = NTPClient::getUTC()))
    delay(100);
  if (!isValidTime(utc))
#endif
  {
#ifdef DEBUG_TIME
    Traceln("Failed to query current time from time server!");
#endif
    if (!ignoreFailure)
    {
      return;
    }
  }
#ifndef USE_WIFI
  else
  {
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

  timeChanged.callObservers(TimeChangedParam(t_now));
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
#define TIME_INIT_UPDATE_PERIOD_SEC 30

void timeUpdateTask(void *param)
{
  while (true)
  {
    tm tr;
    time_t now = t_now;
    localtime_r(&now, &tr);
    TickType_t tWait = (tr.tm_year < 100 ? TIME_INIT_UPDATE_PERIOD_SEC : (Config::timeUpdatePeriodMin * 60)) * 1000 / portTICK_PERIOD_MS;
    vTaskDelay(tWait);
    setTime(false);
  }
}
#endif

void InitTime()
{
#ifdef DEBUG_TIME
  TRACE_BLOCK
  {
    Trace("Time Server: ");
    Traceln(Config::timeServer);
  }
#endif
  DST = AppConfig::getDST();
  setTime(true);
  AppConfig::getAppConfigChanged().addObserver(appConfigChanged, NULL);
#ifndef USE_WIFI
  xTaskCreate(timeUpdateTask, "TimeUpdate", 8*1024, NULL, tskIDLE_PRIORITY, NULL);
#endif
}

bool isValidTime(time_t t)
{
  struct tm stm;

  localtime_r(&t, &stm);

  return stm.tm_year >= 2016 - 1900;
}


