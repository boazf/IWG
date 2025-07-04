#include <Arduino.h>
#include <FakeLock.h>
#include <Trace.h>
#include  <cstdarg>


size_t Tracevf(const char *format, va_list valist)
{
    return vprintf(format, valist); // Use vprintf for simplicity in this test environment
}

size_t Tracef(const char *format, ...)
{
    va_list valist;
    va_start(valist, format);

    return Tracevf(format, valist);
}

size_t Trace(const char *message) 
{ 
    return Tracef("%s", message);
};

CriticalSection csTraceLock;