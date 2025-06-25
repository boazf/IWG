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

#ifndef Trace_h
#define Trace_h

#include <IPAddress.h>
#include <Lock.h>

extern CriticalSection csTraceLock;

#define LOCK_TRACE() Lock lock(csTraceLock)

void InitSerialTrace();
void InitFileTrace();
void TraceStop(int timeout = portMAX_DELAY);
size_t Trace(const char *message);
inline size_t Traceln() { LOCK_TRACE(); return Trace("\r\n"); }
inline size_t Traceln(const char *message) { LOCK_TRACE(); return Trace(message) + Traceln(); };
inline size_t Trace(const String &message) { LOCK_TRACE(); return Trace(message.c_str()); };
inline size_t Traceln(const String &message) { LOCK_TRACE(); return Trace(message) + Traceln(); };
size_t Tracef(const char *format, ...);
inline size_t Trace(long n) { LOCK_TRACE(); return Tracef("%ld", n); }
inline size_t Traceln(long n) { LOCK_TRACE(); return Trace(n) + Traceln(); }
inline size_t Trace(int n) { LOCK_TRACE(); return Tracef("%d", n); }
inline size_t Traceln(int n) { LOCK_TRACE(); return Trace(n) + Traceln(); }
inline size_t Trace(unsigned int n) { LOCK_TRACE(); return Tracef("%u", n); }
inline size_t Traceln(unsigned int n) { LOCK_TRACE(); return Trace(n) + Traceln(); }
inline size_t Trace(char c) { LOCK_TRACE(); char buf[2]; buf[0] = c; buf[1] = '\0'; return Trace(buf); }
inline size_t Trace(uint8_t n) { LOCK_TRACE(); return Tracef("%hhu", n); }
inline size_t Traceln(uint8_t n) { LOCK_TRACE(); return Trace(n) + Traceln(); }
inline size_t Trace(bool b) { LOCK_TRACE(); return Trace(b ? "true" : "false"); }
inline size_t Traceln(bool b) { LOCK_TRACE(); return Trace(b) + Traceln(); }
inline size_t Trace(const IPAddress &a) {LOCK_TRACE();  return Trace(a[0]) + Trace('.') + Trace(a[1]) + Trace('.') + Trace(a[2]) + Trace('.') + Trace(a[3]); }
inline size_t Traceln(const IPAddress &a) { LOCK_TRACE(); return Trace(a) + Traceln(); }

#endif // Trace_h