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
#include <Common.h>
#include <PwrCntl.h>
#include <Trace.h>
#include <Config.h>
#include <TimeUtil.h>

#define WATCHDOG_TRIGGER_PIN 32
#define WATCHDOG_LOADED_PIN 33

// There is a task that is intended to trigger hard reset at a specific time.
// This task will wait for the time to come and then it will call HardReset().
// Everytime that the time is updated, this semaphore is given and the task will re-evaluate the time to wait.
static TaskHandle_t hHardResetTask = NULL;
static xSemaphoreHandle waitSem = NULL;

/// @brief Task to handle hard reset timing.
/// @param param Unused parameter.
/// @note This task waits for the hard reset time to come and then calls HardReset().
///       Whenever the time is updated, it will re-evaluate the time to wait.
static void hardResetTask(void *param)
{
#ifdef USE_WIFI
  // Wait for valid time
  while (!isValidTime(t_now))
  {
    delay(30000);
  }
#endif

  waitSem = xSemaphoreCreateBinary();
  time_t tWait;

  do
  {
    if (!isValidTime(t_now))
      // If the time is not valid, make best effort calculation of the time to wait.
      // Hopefully, later the time will be valid and the task will be able to re-evaluate the time to wait more accurately.
      tWait = Config::hardResetPeriodDays * 24 * 60 * 60 - millis() / 1000;
    else
    {
      // t1 holds the time of system startup plus number of hard reset period in seconds
      // t_now - mills() / 1000 gives the time of system startup in seconds.
      // Config::hardResetPeriodDays * 24 * 60 * 60 gives the period in seconds.
      time_t t1 = t_now - millis() / 1000 + Config::hardResetPeriodDays * 24 * 60 * 60;
      struct tm stm;
      localtime_r(&t1, &stm);
      // Set the time of midnight of the day of the hard reset
      stm.tm_sec = stm.tm_min = stm.tm_hour = 0;
      // Set the time of the hard reset to be at midnight of the day of the hard reset
      time_t tHardReset = mktime(&stm) + Config::hardResetTime;
#ifdef DEBUG_POWER
      localtime_r(&tHardReset, &stm);
    	char buff[128];
      memset(buff, 0, sizeof(buff));
    	strftime(buff, sizeof(buff), "Scheduled hard reset at: %a %d/%m/%Y %T%n", &stm);
      Trace(buff);
#endif
      // Calculate the time to wait until the hard reset
      tWait = tHardReset - t_now;
    }
  } while (xSemaphoreTake(waitSem, (tWait * 1000) / portTICK_PERIOD_MS) == pdTRUE);
  // If we are here, it means that the time to wait has come. We should perform a hard reset.
  vSemaphoreDelete(waitSem);
  waitSem = NULL;
  hHardResetTask = NULL;
  HardReset(3000, 15000);
  vTaskDelete(NULL);
}

#define MIN_HARD_RESET_PERIOD 1
#define MAX_HARD_RESET_PERIOD 45

/// @brief Initializes the hard reset task.
/// @param now The current time.
/// @param param Unused parameter.
/// @note This function checks the hard reset period from the configuration and creates the hard reset task if needed.
///       If the hard reset period is not in the allowed range, it will not create the task.
void InitHardReset(const TimeChangedParam &now, const void *param)
{
  static TaskHandle_t hHardResetTask = NULL;
  
  if (hHardResetTask == NULL)
  {
    if (Config::hardResetPeriodDays < MIN_HARD_RESET_PERIOD || 
        MAX_HARD_RESET_PERIOD < Config::hardResetPeriodDays)
    {
      // If the hard reset period is not in the allowed range, do not create the task.
#ifdef DEBUG_POWER
      Tracef("Periodic hard reset is disabled! Configuration value (HardResetPeriodDays): %d. Allowed range is %d-%d.\n", 
        Config::hardResetPeriodDays,
        MIN_HARD_RESET_PERIOD,
        MAX_HARD_RESET_PERIOD);
#endif
      return;
    }

    // Create the hard reset task
    xTaskCreate(
      hardResetTask,
      "HardResetTask",
      4 * 1024,
      NULL,
      tskIDLE_PRIORITY,
      &hHardResetTask);
  }
  else if (waitSem != NULL)
    // If the task is already created, give the semaphore to re-evaluate the time to wait.
    xSemaphoreGive(waitSem);
}

/// @brief Semaphore to signal the watchdog task to stop triggering the watchdog.
/// This semaphore is used to signal the watchdog task to stop triggering the watchdog.
static xSemaphoreHandle wdSem = NULL;

#define WATCHDOG_LOAD_TIME_MS 5000 // Time to wait for the watchdog to load
#define WATCHDOG_TRIGGER_HALF_CYCLE_TIME_MS 500 // Half cycle time for the watchdog trigger in milliseconds

void InitPowerControl()
{
  // Initialize the watchdog load pin.
  pinMode(WATCHDOG_LOADED_PIN, INPUT);

  // Set observer for time changes
  timeChanged.addObserver(InitHardReset, NULL);

  // Initialize watchdog task.
  wdSem = xSemaphoreCreateBinary();  
  xTaskCreate([](void *param){
    // Initialize the watchdog trigger pin.
    pinMode(WATCHDOG_TRIGGER_PIN, OUTPUT);
    do
    {
      // Keep triggering the watchdog.
      digitalWrite(WATCHDOG_TRIGGER_PIN, HIGH);
      delay(WATCHDOG_TRIGGER_HALF_CYCLE_TIME_MS);
      digitalWrite(WATCHDOG_TRIGGER_PIN, LOW);
    } while (xSemaphoreTake(wdSem, WATCHDOG_TRIGGER_HALF_CYCLE_TIME_MS) == pdFALSE);
    // If we are here, it means that the watchdog semaphore was signaled to stop.
    vTaskDelete(NULL);
  }, "WatchdogTask", 1024, NULL, tskIDLE_PRIORITY, NULL);

  // Wait for watchdog timer to load
  unsigned long t0 = millis();
  while(millis() < t0 + WATCHDOG_LOAD_TIME_MS && digitalRead(WATCHDOG_LOADED_PIN) == LOW)
    delay(10);
#ifdef DEBUG_POWER
  if (digitalRead(WATCHDOG_LOADED_PIN) == LOW)
  {
    Tracef("Watchdog did not load within %d miliseconds!\n", WATCHDOG_LOAD_TIME_MS);
  }
#endif  
}

void HardReset(int timeout, int returnTimeout)
{
#ifdef DEBUG_POWER
  Traceln("Performing hard reset");
#endif
  // Prepare for hard reset by calling observers.
  hardResetEvent.callObservers(HardResetEventParam(HardResetStage::prepare, 0));
  hardResetEvent.callObservers(HardResetEventParam(HardResetStage::shutdown, timeout));
  // Signal the watchdog semaphore. This will cause the watchdog task to end. This will
  // cause the watchdog triggers to stop. This will cause the watchdog to temporarily disconnect
  // the power. This will cause a hard reset.
  xSemaphoreGive(wdSem);
  delay(returnTimeout);
  // If we are here, the watchdog disable switch is turned on, hence hard reset was not done.
  // Call the observers to indicate that the hard reset failed.
  // This will allow components to reverse shutdown operations.
  hardResetEvent.callObservers(HardResetEventParam(HardResetStage::failure, 0));
}

Observers<HardResetEventParam> hardResetEvent;
