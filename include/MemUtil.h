#ifndef MemUtil_h
#define MemUtil_h

#include <Common.h>

#ifdef TRACK_MEMORY

#include <Arduino.h>

int freeMemory();

class TrackFreeMem
{
public:
    String m_name;
    int m0;

    TrackFreeMem(const char *name)
    {
        m_name = name;
        m0 = freeMemory();
        Serial.print(name);
        Serial.print(" on enter: ");
        Serial.println(m0);
    }

    ~TrackFreeMem()
    {
        Serial.print(m_name.c_str());
        Serial.print(": ");
        Serial.println(m0 - freeMemory());
    }
};

#define TRACK_FREE_MEMORY(name) TrackFreeMem trackFreeMem(name)

#else

#define TRACK_FREE_MEMORY(name)

#endif

#endif // MemUtil_h