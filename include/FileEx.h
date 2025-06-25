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

#ifndef FileEx_h
#define FileEx_h

#include <Arduino.h>
#include <SD.h>

class FileEx : public File
{
public:
    FileEx() {}
    FileEx(File file) : File(file) {}
    FileEx& operator=(const File& file);
    size_t write(uint8_t) override;
    size_t write(const uint8_t *buf, size_t size) override;
    size_t write(const char *buf, size_t size) { return write(reinterpret_cast<const byte *>(buf), size); }
    int available() override;
    int read() override;
    int peek() override;
    void flush() override;
    size_t read(uint8_t* buf, size_t size);
    size_t readBytes(char *buffer, size_t length) override;
    bool seek(uint32_t pos, SeekMode mode);
    bool seek(uint32_t pos);
    size_t position() const;
    size_t size() const;
    void close();
    operator bool() const;
    time_t getLastWrite();
    const char* path() const;
    const char* name() const;

    boolean isDirectory(void);
    FileEx openNextFile(const char* mode = FILE_READ);
    void rewindDirectory(void);
};

#endif // FileEx_h
