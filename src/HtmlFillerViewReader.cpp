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

#include <HtmlFillerViewReader.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

size_t HtmlFillerViewReader::viewHandler(size_t buffSize, bool last)
{
    for(size_t i = 0; i < buffSize; i++)
    {
        if (buff[i] == (byte)'%')
        {
            size_t j = i + 1;
            for (; j < buffSize; j++)
                if (buff[j] == (byte)' ')
                    break;
            if (j == buffSize)
                return i;

            int nFill;
            if (sscanf(reinterpret_cast<const char *>(buff + i + 1), "%d", &nFill) != 1)
            {
                continue;
            }

            String fill;
            if (!DoFill(nFill, fill))
            {
#ifdef DEBUG_HTTP_SERVER
                Traceln("Failed to fill view!");
#endif
                continue;
            }
            
            if (i + fill.length() >= buffSize && !last)
                return i;

            for(j = i; buff[j] != (byte)' '; j++)
                buff[j] = (byte)' ';
            for (unsigned int j = 0; j < fill.length() && i + j < buffSize; j++)
            {
                if (buff[i + j] != (char)' ' || i + j >= buffSize)
                {
#ifdef DEBUG_HTTP_SERVER
                    Traceln("Not enough spaces for filled value!");
#endif
                    break;
                }
                buff[i + j] = fill.c_str()[j];
            }
        }
    }

    return buffSize;
}

bool HtmlFillerViewReader::DoFill(int nFill, String &fill)
{
    const ViewFiller *fillers;
    int nFillers = getFillers(fillers);
    if (nFill >= nFillers)
        return false;
    fillers[nFill](fill);
    return true;
}

bool HtmlFillerViewReader::open(byte *buff, int buffSize)
{
    offset = buffSize;
    endOfView = false;
    return ViewReader::open(buff, buffSize) && viewReader->open(buff, buffSize);
}

int HtmlFillerViewReader::read()
{
    if (endOfView)
        return -1;

    memcpy(buff, buff + offset, buffSize - offset);
    offset = buffSize - offset;
    int nBytes = viewReader->read(offset);
    if (nBytes == -1)
    {
        endOfView = true;
        if (offset == 0)
            return -1;
        viewHandler(offset, true);
        return offset;
    }
    nBytes += offset;
    if (nBytes < buffSize)
    {
        endOfView = true;
        viewHandler(nBytes, true);
        return nBytes;
    }

    offset = viewHandler(nBytes);
    return offset;
}
