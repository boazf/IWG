#include "FakeLock.h"

BaseType_t xSemaphoreTake(SemaphoreHandle_t h, TickType_t t) {
    (void)h;
    return pdTRUE;
}
BaseType_t xSemaphoreGive(SemaphoreHandle_t h) {
    (void)h;
    return pdTRUE;
}
SemaphoreHandle_t xSemaphoreCreateMutex(void) {
    return (SemaphoreHandle_t)1;
}

BaseType_t xSemaphoreGiveRecursive(SemaphoreHandle_t h) {
    (void)h;
    return pdTRUE;
}   

BaseType_t xSemaphoreTakeRecursive(SemaphoreHandle_t h, TickType_t t) {
    (void)h;
    return pdTRUE;
}   

void vSemaphoreDelete(SemaphoreHandle_t h) {
    (void)h;
}

xSemaphoreHandle xSemaphoreCreateRecursiveMutex() {
    return (SemaphoreHandle_t)1;
}

