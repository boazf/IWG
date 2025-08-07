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

// This creates an ICMPPing class that uses W5100Ex class instead of W5100 class.
// This is needed to provide the extended functionality of W5100Ex class that is
// required for the implementation of ICMPPing class.
#ifndef USE_WIFI
#include <Common.h>
#include <w5100ex.h>
#define W5100 W5100Ex
#include <ICMPPing.cpp>
#endif