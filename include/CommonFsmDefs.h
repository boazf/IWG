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

#ifndef CommonFsmDefs_h
#define CommonFsmDefs_h
#include <tinyfsm.hpp>
#include <RecoveryControl.h>

struct RecoveryStateChanged : tinyfsm::Event, RecoveryStateChangedParams
{
    RecoveryStateChanged(RecoveryTypes recoveryType, RecoverySource recoverySource) : RecoveryStateChangedParams(recoveryType, recoverySource) {}
};

#endif // CommonFsmDefs_h