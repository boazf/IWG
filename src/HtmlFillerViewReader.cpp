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

/// @brief This function handles the view buffer and replaces filler indices with their corresponding values.
/// It searches for '%' characters followed by a number, which indicates a filler index and replaces it with the corresponding filler value.
/// If the filler index is not valid or there is not enough space in the buffer to fill the value, it will fill whatever possible and will
/// continue processing rest of the buffer.
/// @param buffSize The actual number of bytes in the buffer.
/// @param last Indicates if this is the last chunk of data being processed.
/// @return Number of bytes processed in the buffer.
size_t HtmlFillerViewReader::viewHandler(size_t buffSize, bool last)
{
    for(size_t i = 0; i < buffSize; i++)
    {
        if (buff[i] == (byte)'%')
        {
            size_t j = i + 1;
            // Find the next space after the '%'
            for (; j < buffSize; j++)
                if (buff[j] == (byte)' ')
                    break;
            
            if (j == buffSize)
                return i; // Possibly filler index spans beyond the current the buffer

            
            if (j == i + 1)
                continue; // Just a % with a space right after it

            // We have a filler index, parse it
            int nFill;
            std::string index(buff + i + 1, buff + j);
            char *end;
            nFill = std::strtol(index.c_str(), &end, 10);
            if (*end != '\0')
                continue; // Not a valid number

            String fill;
            if (!DoFill(nFill, fill))
            {
#ifdef DEBUG_HTTP_SERVER
                Traceln("Failed to fill view!");
#endif
                continue;
            }
            
            // Check if we have enough space to fill the value
            // If this is the last chunk, we fill what ever possible.
            // Otherwise, we may have space in the next chunk.
            if (i + fill.length() >= buffSize && !last)
                return i;

            // Replace the filler index characters with spaces
            for(j = i; buff[j] != (byte)' '; j++)
                buff[j] = (byte)' ';
            // Fill the buffer with the filler value
            for (unsigned int j = 0; j < fill.length() && i + j < buffSize; j++)
            {
                if (buff[i + j] != (char)' ' || i + j >= buffSize)
                {
                    // We may have a case where the buffer is not large enough to fill the value
                    // This can happen if either there are not enough spaces after the filler index,
                    // or in case this is the last chunk and the filler is too long.
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

/// @brief This function retrieves the fillers from the provided GetFillers function.
/// @param nFill The index of the filler to retrieve.
/// @param fill The String object to fill with the filler value.
/// @return true if the filler was successfully filled, false otherwise.
/// @note The fillers are expected to be defined in the GetFillers function, which should
/// return the number of fillers available. The fillers are expected to be functions that take a String reference and fill it with the appropriate value.
/// If the index is out of bounds, it returns false.
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
