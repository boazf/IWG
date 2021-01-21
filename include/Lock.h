#ifndef Lock_h
#define Lock_h

#ifdef ESP32
#include <Arduino.h>

class Lock;

class CriticalSection
{
public:
    CriticalSection()
    {
        _binarySem = xSemaphoreCreateBinary();
        xSemaphoreGive(_binarySem);
    }

    ~CriticalSection()
    {
        vSemaphoreDelete(_binarySem);
    }

    void Enter()
    {
        //portENTER_CRITICAL(&mux);
        xSemaphoreTake(_binarySem, portMAX_DELAY);
    }

    void Leave()
    {
        //portEXIT_CRITICAL(&mux);
        xSemaphoreGive(_binarySem);
    }

private:
    //portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    xSemaphoreHandle _binarySem;
};

class Lock
{
public:
    Lock(CriticalSection &cs) : _cs(cs)
    {
        _cs.Enter();
    }

    ~Lock()
    {
        _cs.Leave();
    }

private:
    CriticalSection &_cs;
};
#endif // ESP32

#endif // Lock_h