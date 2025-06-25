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

#include <Common.h>
#include <FileEx.h>

FileEx &FileEx::operator=(const File& file) 
{
    *(dynamic_cast<File*>(this)) = file;
    return *this;
}

size_t FileEx::write(uint8_t byte)
{
    Lock lock(csSpi);
    return File::write(byte);
}

size_t FileEx::write(const uint8_t *buf, size_t size)
{
    Lock lock(csSpi);
    return File::write(buf, size);
}

int FileEx::available()
{
    Lock lock(csSpi);
    return File::available();
}

int FileEx::read()
{
    Lock lock(csSpi);
    return File::read();
}

int FileEx::peek()
{
    Lock lock(csSpi);
    return File::peek();
}

void FileEx::flush()
{
    Lock lock(csSpi);
    File::flush();
}

size_t FileEx::read(uint8_t* buf, size_t size)
{
    Lock lock(csSpi);
    return File::read(buf, size);
}

size_t FileEx::readBytes(char *buffer, size_t length)
{
    return read(reinterpret_cast<uint8_t*>(buffer), length);
}

bool FileEx::seek(uint32_t pos, SeekMode mode)
{
    Lock lock(csSpi);
    return File::seek(pos, mode);
}

bool FileEx::seek(uint32_t pos)
{
    return seek(pos, SeekSet);
}

size_t FileEx::position() const
{
    Lock lock(csSpi);
    return File::position();
}

size_t FileEx::size() const
{
    Lock lock(csSpi);
    return File::size();
}

void FileEx::close()
{
    Lock lock(csSpi);
    File::close();
}

FileEx::operator bool() const
{
    Lock lock(csSpi);
    return !!File::operator bool();
}

time_t FileEx::getLastWrite()
{
    Lock lock(csSpi);
    return File::getLastWrite();
}

const char* FileEx::path() const
{
    Lock lock(csSpi);
    return File::path();
}

const char* FileEx::name() const
{
    Lock lock(csSpi);
    return File::name();
}

boolean FileEx::isDirectory(void)
{
    Lock lock(csSpi);
    return File::isDirectory();
}

FileEx FileEx::openNextFile(const char* mode)
{
    Lock lock(csSpi);
    return File::openNextFile(mode);
}

void FileEx::rewindDirectory(void)
{
    Lock lock(csSpi);
    File::rewindDirectory();
}
