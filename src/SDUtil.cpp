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

#include <Arduino.h>
#include <SD.h>
#include <SDUtil.h>
#include <assert.h>
#include <TimeUtil.h>
#include <Common.h>

AutoSD::AutoSD()
{
  SD.begin();
}

AutoSD::~AutoSD()
{
  SD.end();
}

int SDExClass::count = 0;

#undef SD

bool SDExClass::begin(uint8_t ssPin, SPIClass &spi, uint32_t frequency, const char * mountpoint, uint8_t max_files)
{
  Lock lock(csSpi);
  if (count++ == 0)
    return SD.begin(ssPin, spi, frequency, mountpoint, max_files);

  return true;
}

void SDExClass::end()
{
  Lock lock(csSpi);
  if (--count == 0)
    SD.end();
}

sdcard_type_t SDExClass::cardType()
{
  Lock lock(csSpi);
  return SD.cardType();
}

uint64_t SDExClass::cardSize()
{
  Lock lock(csSpi);
  return SD.cardSize();
}

uint64_t SDExClass::totalBytes()
{
  Lock lock(csSpi);
  return SD.totalBytes();
}

uint64_t SDExClass::usedBytes()
{
  Lock lock(csSpi);
  return SD.usedBytes(); 
}

File SDExClass::open(const char* path, const char* mode)
{
  Lock lock(csSpi);
  return SD.open(path, mode);
}

File SDExClass::open(const String& path, const char* mode)
{
  return open(path.c_str(), mode);  
}

bool SDExClass::exists(const char* path)
{
  Lock lock(csSpi);
  return SD.exists(path);
}

bool SDExClass::exists(const String& path)
{
  return exists(path.c_str());
}

bool SDExClass::remove(const char* path)
{
  Lock lock(csSpi);
  return SD.remove(path);
}
bool SDExClass::remove(const String& path)
{
  return remove(path.c_str());
}

bool SDExClass::rename(const char* pathFrom, const char* pathTo)
{
  Lock lock(csSpi);
  return SD.rename(pathFrom, pathTo);
}

bool SDExClass::rename(const String& pathFrom, const String& pathTo)
{
  return rename(pathFrom.c_str(), pathTo.c_str());
}
bool SDExClass::mkdir(const char *path)
{
  Lock lock(csSpi);
  return SD.mkdir(path);
}

bool SDExClass::mkdir(const String &path)
{
  return mkdir(path.c_str());
}

bool SDExClass::rmdir(const char *path)
{
  Lock lock(csSpi);
  return SD.rmdir(path);
}

bool SDExClass::rmdir(const String &path)
{
  return rmdir(path.c_str());
}

SDExClass SDEx;

void InitSD()
{
}

