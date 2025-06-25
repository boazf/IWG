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

#ifndef Common_h
#define Common_h

#include <Lock.h>

#ifndef RELEASE
#define DEBUG_SD
#define DEBUG_ETHERNET
#define DEBUG_TIME
#define DEBUG_SFT
#define DEBUG_CONFIG
#define DEBUG_HTTP_SERVER
#define DEBUG_RECOVERY_CONTROL
#define DEBUG_HISTORY
#define DEBUG_STATE_MACHINE
#define DEBUG_POWER
#endif

#define NELEMS(a) (sizeof(a)/sizeof(*a))
#define MAX_PATH 128

#ifdef USE_WIFI
#define APP_NAME "IWG-WIFI"
#else
#define APP_NAME "IWG-Wired"
#endif

extern CriticalSection csSpi;

#endif // Common_h