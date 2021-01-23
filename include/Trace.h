#ifndef Trace_h
#define Trace_h

#include <WString.h>
#include <stddef.h>
#include <IPAddress.h>

void InitSerialTrace();
void InitFileTrace();
size_t Trace(const char *message);
inline size_t Traceln() { return Trace("\r\n"); }
inline size_t Traceln(const char *message) { return Trace(message) + Traceln(); };
inline size_t Trace(const String &message) { return Trace(message.c_str()); };
inline size_t Traceln(const String &message) { return Trace(message) + Traceln(); };
size_t Tracef(const char *format, ...);
#ifndef ESP32
inline size_t Trace(uint32_t n) { return Tracef("%lu", n); }
inline size_t Traceln(uint32_t n) { return Trace(n) + Traceln(); }
#endif
inline size_t Trace(long n) { return Tracef("%ld", n); }
inline size_t Traceln(long n) { return Trace(n) + Traceln(); }
inline size_t Trace(int n) { return Tracef("%d", n); }
inline size_t Traceln(int n) { return Trace(n) + Traceln(); }
inline size_t Trace(unsigned int n) { return Tracef("%u", n); }
inline size_t Traceln(unsigned int n) { return Trace(n) + Traceln(); }
inline size_t Trace(char c) { char buf[2]; buf[0] = c; buf[1] = '\0'; return Trace(buf); }
inline size_t Trace(uint8_t n) { return Tracef("%hhu", n); }
inline size_t Traceln(uint8_t n) { return Trace(n) + Traceln(); }
inline size_t Trace(bool b) { return Trace(b ? "true" : "false"); }
inline size_t Traceln(bool b) { return Trace(b) + Traceln(); }
inline size_t Trace(const IPAddress &a) { return Trace(a[0]) + Trace('.') + Trace(a[1]) + Trace('.') + Trace(a[2]) + Trace('.') + Trace(a[3]); }
inline size_t Traceln(const IPAddress &a) { return Trace(a) + Traceln(); }

#endif // Trace_h