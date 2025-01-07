#include <Arduino.h>
#include <Common.h>
#include <PwrCntl.h>
#include <Trace.h>
#include <Config.h>
#include <TimeUtil.h>

#define WATCHDOG_TRIGGER_PIN 32
#define WATCHDOG_LOADED_PIN 33

static xSemaphoreHandle waitSem = NULL;

void hardResetTask(void *param)
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
      tWait = Config::hardResetPeriodDays * 24 * 60 * 60 - millis() / 1000;
    else
    {
      time_t t1 = t_now - millis() / 1000 + Config::hardResetPeriodDays * 24 * 60 * 60;
      struct tm stm;
      localtime_r(&t1, &stm);
      stm.tm_sec = stm.tm_min = stm.tm_hour = 0;
      time_t tHardReset = mktime(&stm) + Config::hardResetTime;
#ifdef DEBUG_POWER
      localtime_r(&tHardReset, &stm);
    	char buff[128];
      memset(buff, 0, sizeof(buff));
    	strftime(buff, sizeof(buff), "Scheduled hard reset at: %a %d/%m/%Y %T%n", &stm);
      Trace(buff);
#endif
      tWait = tHardReset - t_now;
    }
  } while (xSemaphoreTake(waitSem, (tWait * 1000) / portTICK_PERIOD_MS) == pdTRUE);

  HardReset(3000);
  vTaskDelete(NULL);
}

#define MIN_HARD_RESET_PERIOD 1
#define MAX_HARD_RESET_PERIOD 45

void InitHardReset(const TimeChangedParam &now, const void *param)
{
  static TaskHandle_t hHardResetTask = NULL;
  
  if (hHardResetTask == NULL)
  {
    if (Config::hardResetPeriodDays < MIN_HARD_RESET_PERIOD || 
        MAX_HARD_RESET_PERIOD < Config::hardResetPeriodDays)
    {
#ifdef DEBUG_POWER
      Tracef("Periodic hard reset is disabled! Configuration value (HardResetPeriodDays): %d. Allowed range is %d-%d.\n", 
        Config::hardResetPeriodDays,
        MIN_HARD_RESET_PERIOD,
        MAX_HARD_RESET_PERIOD);
#endif
      return;
    }

    xTaskCreate(
      hardResetTask,
      "HardResetTask",
      4 * 1024,
      NULL,
      tskIDLE_PRIORITY,
      &hHardResetTask);
  }
  else if (waitSem != NULL)
    xSemaphoreGive(waitSem);
}

static xSemaphoreHandle wdSem = NULL;

#define WATCHDOG_LOAD_TIME_MS 5000
#define WATCHDOG_TRIGGER_HALF_CYCLE_TIME_MS 500

void InitPowerControl()
{
  pinMode(WATCHDOG_LOADED_PIN, INPUT);

  timeChanged.addObserver(InitHardReset, NULL);

  // Initialize watchdog task.
  wdSem = xSemaphoreCreateBinary();  
  xTaskCreate([](void *param){
    pinMode(WATCHDOG_TRIGGER_PIN, OUTPUT);
    do
    {
      // Keep triggering the watchdog.
      digitalWrite(WATCHDOG_TRIGGER_PIN, HIGH);
      delay(WATCHDOG_TRIGGER_HALF_CYCLE_TIME_MS);
      digitalWrite(WATCHDOG_TRIGGER_PIN, LOW);
    } while (xSemaphoreTake(wdSem, WATCHDOG_TRIGGER_HALF_CYCLE_TIME_MS) == pdFALSE);
        vTaskDelete(NULL);  
  }, "WatchdogTask", 4 * 1024, NULL, tskIDLE_PRIORITY, NULL);

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

void HardReset(int timeout)
{
#ifdef DEBUG_POWER
  Traceln("Performing hard reset");
#endif
  TraceStop(timeout);
  // Signal the watchdog semaphore. This will cause the watchdog task to end. This will
  // cause the watchdog triggers to stop. This will cause the watchdog to temporarily disconnect
  // the power. This will cause a hard reset.
  xSemaphoreGive(wdSem);
  while(true)
    delay(1000);
}
