#ifndef FakeLock_h
#define FakeLock_h

typedef void* SemaphoreHandle_t;
#define xSemaphoreHandle              SemaphoreHandle_t
typedef int BaseType_t;
typedef unsigned long TickType_t;
#define pdTRUE 1
#define pdFALSE 0
#define portMAX_DELAY 0xFFFFFFFF

BaseType_t xSemaphoreTake(SemaphoreHandle_t h, TickType_t t);
BaseType_t xSemaphoreGive(SemaphoreHandle_t h);
SemaphoreHandle_t xSemaphoreCreateMutex(void);
BaseType_t xSemaphoreGiveRecursive(SemaphoreHandle_t h);
BaseType_t xSemaphoreTakeRecursive(SemaphoreHandle_t h, TickType_t t);
void vSemaphoreDelete(SemaphoreHandle_t h);
xSemaphoreHandle xSemaphoreCreateRecursiveMutex();

#endif // FakeLock_h