#ifndef Lock_h
#define Lock_h

#include <Arduino.h>

class CriticalSection
{
public:
    CriticalSection()
    {
        _binarySem = xSemaphoreCreateRecursiveMutex();
        xSemaphoreGiveRecursive(_binarySem);
    }

    ~CriticalSection()
    {
        vSemaphoreDelete(_binarySem);
    }

    void Enter() const
    {
        //portENTER_CRITICAL(&mux);
        xSemaphoreTakeRecursive(_binarySem, portMAX_DELAY);
    }

    void Leave() const
    {
        //portEXIT_CRITICAL(&mux);
        xSemaphoreGiveRecursive(_binarySem);
    }

private:
    //portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    xSemaphoreHandle _binarySem;
};

class Lock
{
public:
    Lock(const CriticalSection &cs) : _cs(cs)
    {
        _cs.Enter();
    }

    ~Lock()
    {
        _cs.Leave();
    }

private:
    const CriticalSection &_cs;
};
#endif // Lock_h