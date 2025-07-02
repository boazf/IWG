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

#ifndef StringableEnum_h
#define StringableEnum_h

#include <map>

template <typename T>
class StringableEnum
{
public:
    StringableEnum(T e) : e(e)
    {
    }

    std::string ToString()
    {
        try
        {
            return strMap.at(e);
        }
        catch(std::out_of_range)
        {
            return unknown;
        }
    }

    inline static const char *unknown = "Unknown";

private:
    T e;
    static const std::map<T, std::string> strMap;
};

#endif // StringableEnum_h