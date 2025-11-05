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
#ifndef USE_WIFI
#include <EthernetUtil.h>
#include <NTPClient.h>
#include <sys/time.h>
#endif
#include <TimeUtil.h>
#include <Config.h>
#include <Common.h>
#include <AppConfig.h>
#ifdef DEBUG_TIME
#include <Trace.h>
#endif
static bool DST;
Observers<TimeChangedParam> timeChanged;

#define MAX_WAIT_TIME_FOR_NTP_SUCCESS_SEC 30

/// @brief Set the system time.
/// @param ignoreFailure If true, ignore any failures to set the time.
static void setTime(bool ignoreFailure)
{
  time_t now = 0;

#ifdef USE_WIFI
  // Use WiFi to get the time
  configTime(Config::timeZone * 60 + (DST ? (Config::DST * 60) : 0), 0, Config::timeServer);
  tm tr1;
  delay(2000);
  tr1.tm_year = 0;
  // Wait for the time to be set
  // If the time is not set after MAX_WAIT_TIME_FOR_NTP_SUCCESS_SEC seconds, return
  // This is to avoid blocking the application if the time server is not reachable
  if (!getLocalTime(&tr1, MAX_WAIT_TIME_FOR_NTP_SUCCESS_SEC * 1000))
#else
  // Use Ethernet to get the time
  unsigned long t0 = millis();
  EthUDP ntpUDP;
  NTPClient ntpClient(ntpUDP, Config::timeServer, Config::timeZone * 60 + (DST ? (Config::DST * 60) : 0), Config::timeUpdatePeriodMin * 60 * 1000);
  ntpClient.begin();
  // Wait for the time to be set
  do
  {
    ntpClient.update();
    delay(100);
  } while(millis() - t0 < MAX_WAIT_TIME_FOR_NTP_SUCCESS_SEC * 1000 && !isValidTime(now = ntpClient.getEpochTime()));
  ntpClient.end();
  if (!isValidTime(now))
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
    timeval tv = {now, 0};
    settimeofday(&tv, NULL);
  }
#endif

#ifdef DEBUG_TIME
  char buff[128];
  tm tr;
  now = t_now;

  localtime_r(&now, &tr);
  strftime(buff, sizeof(buff), "DateTime: %a %d/%m/%Y %T%n", &tr);
  Trace(buff);
#endif

  // Notify observers that the time has changed
  timeChanged.callObservers(TimeChangedParam(t_now));
}

/// @brief Application configuration changed event handler.
/// @param param The event parameter.
/// @param context Set to NULL here.
static void appConfigChanged(const AppConfigChangedParam &param, void *context)
{
  // See if DST has changed
  // If it has, set the time again to adjust for the new DST setting
  // This is needed because the time server may not provide the correct time with the new DST setting
  if (DST != AppConfig::getDST())
  {
    DST = AppConfig::getDST();
#ifdef DEBUG_TIME
    Tracef("Daylight Saving Time changed: %s\n", DST ? "on" : "off");
#endif
    // Set the time again to adjust for the new DST setting
    // This will also notify observers of the time change.
    setTime(true);
  }
}

#ifndef USE_WIFI
#define TIME_INIT_UPDATE_PERIOD_SEC 30

/// @brief Task to periodically update the time.
void timeUpdateTask(void *param)
{
  while (true)
  {
    tm tr;
    time_t now = t_now;
    localtime_r(&now, &tr);
    // If the year is less than 100, it means the time has not been set yet
    // Wait for TIME_INIT_UPDATE_PERIOD_SEC seconds before trying to set the time again
    // If time has been set, the task will wait the configured time update period and then will try to set the time again
    TickType_t tWait = (tr.tm_year < 100 ? TIME_INIT_UPDATE_PERIOD_SEC : (Config::timeUpdatePeriodMin * 60)) * 1000 / portTICK_PERIOD_MS;
    // Wait for the configured time update period
    vTaskDelay(tWait);
    // Try to set the time again
    setTime(false);
#ifdef DEBUG_TIME
    Tracef("Time task stack high watermark: %d\n", uxTaskGetStackHighWaterMark(NULL));    
#endif    
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
  // Set the system time based on the configured time server and timezone
  // If the time server is not reachable, the time will not be set, but the application will continue to run.
  setTime(true);
  // Register an observer for application configuration changes related to time settings
  AppConfig::getAppConfigChanged().addObserver(appConfigChanged, NULL);
#ifndef USE_WIFI
  // Create a task to periodically update the time
  xTaskCreate(timeUpdateTask, "TimeUpdate", 2*1024, NULL, tskIDLE_PRIORITY, NULL);
#endif
}

/// @brief Check if the given time is valid.
/// @param t The time to check.
/// @return True if the time is valid, false otherwise.
bool isValidTime(time_t t)
{
  struct tm stm;

  localtime_r(&t, &stm);

  // Check if the year is valid (after 2016)
  return stm.tm_year >= 2016 - 1900;
}
