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

#include <HistoryView.h>
#include <SDUtil.h>
#include <HistoryControl.h>
#include <Common.h>
#include <Config.h>
#include <TimeUtil.h>
#include <map>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif
#include <restrictions.hpp>

using namespace historycontrol;

/// @brief Map of recovery statuses to their string representations
typedef std::map<RecoveryStatus, String> RecoveryStatusesMap;
#define X(a) {RecoveryStatus::a, #a},
static const RecoveryStatusesMap recoveryStatusesMap = 
{
    RecoveryStatuses
};
#undef X

/// @brief Map of recovery sources to their string representations
typedef std::map<RecoverySource, String> RecoverySourcesMap;
#define X(a) {RecoverySource::a, #a},
static const RecoverySourcesMap recoverySourcesMap = 
{
    RecoverySources
};
#undef X

typedef bool(*fillFile)(SdFile &file);

/// @brief Check and print a buffer to the file
/// @param f The file to print to
/// @param b The buffer to print
/// @param l The length of the buffer
/// @note l is the return value of a call to sprintf(), b is the buffer passed to 
/// sprintf(). So we check that the return value of sprintf() is not more than the
/// size of the buffer. Otherwise, we have a buffer overflow. We also check that
/// the file was written successfully.
#define CHECK_PRINT(f, b, l) if ((l) > NELEMS(b) || f.print(b) != (l)) return false
/// @brief Check and print a string literal to the file.
/// @note We call the CHECK_PRINT macro with the length of the string literal.
/// This should eliminate the length check because the length check always
/// calculates to false. So in this case, we only check that the file was 
/// written successfully.
#define CHECK_PRINT_STRL(f, s) \
    REQUIRE_STRING_LITERAL(s); \
    CHECK_PRINT(f, s, NELEMS(s) - 1)

/// @brief Format a time value as a string.
/// @param time The time value to format.
/// @param buff The buffer to write the formatted string to.
/// @param buffSize The size of the buffer.
/// @return The length of the formatted string, or 0 on failure.
int formatTime(time_t time, char *buff, size_t buffSize)
{
    tm tr;
    localtime_r(&time, &tr);
    return static_cast<int>(strftime(buff, buffSize, "%d/%m/%Y %T", &tr));
}

/// @brief Fill the alerts section of the history view
/// @param file The SD file to write to
/// @return True if successful, false otherwise
/// @note This function writes the HTML for the alerts section of the history view.
static bool fillAlerts(SdFile &file)
{
    if (historyControl.Available() == 0)
    {
        // No history available
        CHECK_PRINT_STRL(file, "<div class=\"alert alert-success\">There is no history yet.</div>\n");
        return true;
    }

    // Iterate over the available history items
    for(int i = 0; i < historyControl.Available(); i++)
    {
        HistoryStorageItem hItem = historyControl.GetHistoryItem(i);
        CHECK_PRINT_STRL(file, "<div class=\"col-lg-3 col-md-4 col-sm-6 col-xs-12\">\n");
        char buff[128];
        // Start the history alert item div element
        int len = snprintf(buff, NELEMS(buff), "<div id=\"historyItem%d\" class=\"alert\">\n", i);
        CHECK_PRINT(file, buff, len);
        // Write the recovery source alert header element
        len = snprintf(buff, NELEMS(buff), "<h4 id=\"recoverySource%d\" class=\"alert-heading\"></h4>\n<hr />\n<p><span class=\"attribute-name\">\n", i);
        CHECK_PRINT(file, buff, len);
        if (hItem.endTime() != INT32_MAX)
        {
            // If there is end time for the recovery, then write the start time first.
            CHECK_PRINT_STRL(file, "Start ");
        }

        // Write the recovery time.
        char timeBuff[64];
        len = formatTime(hItem.startTime(), timeBuff, NELEMS(timeBuff));
        if (len == 0)
            return false;
        len = snprintf(buff, NELEMS(buff), "Time:</span><br /><span class=\"indented\">%s</span></p>\n<p ", timeBuff);
        CHECK_PRINT(file, buff, len);

        if (hItem.endTime() == INT32_MAX)
        {
            // If there in no end time for the recovery then hide the end time element.
            CHECK_PRINT_STRL(file, "style=\"visibility:hidden\"");
        }
        // Write the end time span element.
        CHECK_PRINT_STRL(file, "><span class=\"attribute-name\">End Time:</span><br /><span class=\"indented\">");
        // Write the end time
        len = formatTime(hItem.endTime(), timeBuff, NELEMS(timeBuff));
        if (len == 0)
            return false;
        CHECK_PRINT(file, timeBuff, len);
        // Close the end time span and paragraph elements and start the modem/router recovery counters paragraph element
        CHECK_PRINT_STRL(file, "</span></p>\n<p ");
        if (hItem.modemRecoveries() == 0 && hItem.routerRecoveries() == 0)
        {
            // If both modem and router recovery counters are zero, hide the recoveries counters element
            CHECK_PRINT_STRL(file, "style=\"visibility:hidden\"");
        }
        // Write the recoveries counters element header
        CHECK_PRINT_STRL(file, "><span class=\"attribute-name\">Recoveries:</span><br />");
        // Write the router recovery counter span element
        len = snprintf(buff, NELEMS(buff), "<span class=\"indented\">%s: %d</span>", Config::deviceName, hItem.routerRecoveries());
        CHECK_PRINT(file, buff, len);
        if (!Config::singleDevice)
        {
            // Write the modem recovery counter span element
            len = snprintf(buff, NELEMS(buff), "<span class=\"indented\">Modem: %d</span>", hItem.modemRecoveries());
            CHECK_PRINT(file, buff, len);
        }
        // Close the recoveries counters paragraph element and write the recovery status element
        len = snprintf(buff, NELEMS(buff), "</p>\n<hr />\n<h4 id=\"recoveryStatus%d\"></h4>\n</div>\n</div>\n", i);
        CHECK_PRINT(file, buff, len);
    }

    return true;
}

#define recoveryStatusEnumName "recoveryStatus"
#define recoverySourceEnumName "recoverySource"

/// @brief Fills the JS enum definition in the specified file.
/// @tparam T The type of the map containing the enum values.
/// @param file The file to write the enum definition to.
/// @param map The map containing the enum values.
/// @param varName The name of the enum variable.
/// @return True if the operation was successful, false otherwise.
template <typename T>
static bool fillEnum(SdFile &file, T map, const char *varName)
{
    char buff[128];

    // Write the enum definition header
    size_t len = snprintf(buff, NELEMS(buff), "\tconst %s = {\n", varName);
    CHECK_PRINT(file, buff, len);
    // Write the enum values
    for (typename T::const_iterator i = map.begin(); i != map.end(); i++)
    {
        len = snprintf(buff, NELEMS(buff), "\t\t%s: %d,\n", i->second.c_str(), static_cast<int>(i->first));
        CHECK_PRINT(file, buff, len);
    };
    // Write the enum definition footer
    CHECK_PRINT_STRL(file, "\t};\n");

    return true;
}

/// @brief Fill the enums section of the history view
/// @param file The file to write the enums section to
/// @return True if the operation was successful, false otherwise
static bool fillEnums(SdFile &file)
{
    return fillEnum<RecoveryStatusesMap>(file, recoveryStatusesMap, recoveryStatusEnumName) && 
           fillEnum<RecoverySourcesMap>(file, recoverySourcesMap, recoverySourceEnumName);
}

/// @brief Fill the recovery source enum value in the specified file.
/// @param file The file to write the enum value to.
/// @param source The recovery source enum value to write.
/// @return True if the operation was successful, false otherwise.
static bool fillRecoverySourceEnum(SdFile file, RecoverySource source)
{
    char buff[128];

    size_t len = snprintf(buff, NELEMS(buff), "%s.%s", recoverySourceEnumName, recoverySourcesMap.at(source).c_str());

    CHECK_PRINT(file, buff, len);

    return true;
}

/// @brief Fill the recovery status enum value in the specified file.
/// @param file The file to write the enum value to.
/// @param status The recovery status enum value to write.
/// @return True if the operation was successful, false otherwise.
static bool fillRecoveryStatusEnum(SdFile file, RecoveryStatus status)
{
    char buff[128];

    size_t len = snprintf(buff, NELEMS(buff), "%s.%s", recoveryStatusEnumName, recoveryStatusesMap.at(status).c_str());

    CHECK_PRINT(file, buff, len);

    return true;
}

/// @brief Fills the JavaScript section of the history view
/// @param file The file to write the JavaScript section to
/// @return True if the operation was successful, false otherwise
/// @note This function writes JavaScript code that initializes the recovery source and status for each history item.
static bool fillJS(SdFile &file)
{
    char buff[128];
    // Iterate through history items
    for (int i = 0; i < historyControl.Available(); i++)
    {
        HistoryStorageItem hItem = historyControl.GetHistoryItem(i);
        // Write a call to setRecoverySource for the given history item
        int len = snprintf(buff, NELEMS(buff), "\t\tsetRecoverySource(%d, ", i);
        CHECK_PRINT(file, buff, len);
        fillRecoverySourceEnum(file, hItem.recoverySource());
        char closeCall[] = ");\n";
        CHECK_PRINT_STRL(file, closeCall);
        // Write a call to setRecoveryStatus for the given history item
        len = snprintf(buff, NELEMS(buff), "\t\tsetRecoveryStatus(%d, ", i);
        CHECK_PRINT(file, buff, len);
        fillRecoveryStatusEnum(file, hItem.recoveryStatus());
        CHECK_PRINT_STRL(file, closeCall);
    }
    return true;
}

CriticalSection HistoryViewReader::cs;

#define TEMP_HISTORY_FILE_DIR "/wwwroot/temp"
#define TEMP_HISTORY_FILE_PATH TEMP_HISTORY_FILE_DIR "/history.htm"
#define fillerChar '%'
#define STRNCHR(b, c, n) static_cast<char *>(memchr(b, c, n))

bool HistoryViewReader::open(byte *buff, int buffSize)
{
    // Initialize the SD card
    AutoSD autoSD;
    // Avoid multiple view accesses
    Lock lock(cs);

    // Check if the temporary history file directory exists
    if (!SD.exists(TEMP_HISTORY_FILE_DIR))
    {
        // Create the temporary history file directory
        if (!SD.mkdir(TEMP_HISTORY_FILE_DIR))
        {
#ifdef DEBUG_HTTP_SERVER
            Tracef("Failed to create directory %s\n", TEMP_HISTORY_FILE_DIR);
#endif
            return false;
        }
    }

    // Open the history file for reading
    SdFile historyFile = SD.open(TEMP_HISTORY_FILE_PATH, FILE_READ);

    if (historyFile && 
        historyFile.getLastWrite() < t_now && // Handle case where current time wasn't set well
        historyFile.getLastWrite() > historyControl.getLastUpdate())
    {
#ifdef DEBUG_HTTP_SERVER
        Tracef("Reusing %s\n", TEMP_HISTORY_FILE_PATH);
#endif
        // Use the existing history file
        return FileViewReader::open(buff, buffSize, historyFile);
    }

    // If the history file is outdated, or doesn't exist, create a new one
    if (historyFile)
        historyFile.close();

    historyFile = SD.open(TEMP_HISTORY_FILE_PATH, FILE_WRITE);
    if (!historyFile)
    {
#ifdef DEBUG_HTTP_SERVER
        Tracef("Failed to create %s\n", TEMP_HISTORY_FILE_PATH);
#endif
        return false;
    }

    // Open the base view reader, this will open the history.htm file
    if (!FileViewReader::open(buff, buffSize))
    {
        // If we couldn't open the base view reader, delete the temporary history file
        historyFile.close();
        SD.remove(TEMP_HISTORY_FILE_PATH);
        return false;
    }

    // Filler functions array. The filler index corresponds to the filler indicator index in history.htm file
    fillFile fillers[] = 
    { 
        /* 1 */ fillAlerts, 
        /* 2 */ fillEnums, 
        /* 3 */ fillJS, 
        /* 4 */ [](SdFile &file)->bool { return fillRecoveryStatusEnum(file, RecoveryStatus::RecoveryFailure); },
        /* 5 */ [](SdFile &file)->bool { return fillRecoveryStatusEnum(file, RecoveryStatus::RecoverySuccess); },
        /* 6 */ [](SdFile &file)->bool { return fillRecoveryStatusEnum(file, RecoveryStatus::OnGoingRecovery); },
        /* 7 */ [](SdFile &file)->bool { return fillRecoverySourceEnum(file, RecoverySource::Auto); },
        /* 8 */ [](SdFile &file)->bool { return fillRecoverySourceEnum(file, RecoverySource::UserInitiated); },
        /* 9 */ [](SdFile &file)->bool { return fillRecoverySourceEnum(file, RecoverySource::Periodic); },
    };

    int offset = 0;
    // Read the file contents using the base view reader
    for (int nBytes = read(offset) + offset; nBytes; nBytes = read(offset) + offset)
    {
        bool eof = nBytes == offset; // Check if we reached the end of the file
        const char *pBuff = reinterpret_cast<const char *>(buff);
        offset = 0;
        // Find the next filler delimiter and fill
        for (const char *delim = STRNCHR(pBuff, fillerChar, nBytes); 
             delim != NULL; 
             delim = STRNCHR(pBuff, fillerChar, nBytes))
        {
            size_t delimIndex = delim - pBuff;
            // Write everything up to the delimiter
            historyFile.write(pBuff, delimIndex);
            // Get the filler index
            size_t fillerIndex = delimIndex + 1;
            for(; isdigit(pBuff[fillerIndex]) && fillerIndex < nBytes; fillerIndex++);

            if (!eof && nBytes == fillerIndex)
            {
                // The filler indicator may span over to the next buffer,
                // So move the beginning of the filler indicator to the start of the buffer
                offset = fillerIndex - delimIndex;
                memcpy(buff, delim, offset);
                nBytes = 0;
                // Break the loop to read the next buffer
                break;
            }
            // Skip the filler indicator.
            nBytes -= fillerIndex;
            pBuff += fillerIndex;
            if (fillerIndex == delimIndex + 1)
            {
                // Not a filler
                historyFile.write(fillerChar);
                continue;
            }
            // Get the filler index
            int n = atoi(delim + 1);
            // Call the appropriate filler function
            if (!fillers[n - 1](historyFile))
            {
                // If the filler function failed, close the history file and remove the temporary file
                historyFile.close();
                SD.remove(TEMP_HISTORY_FILE_PATH);
                return false;
            }
        }
        // Write any remaining bytes to the history file
        historyFile.write(pBuff, nBytes);
    }

    // Close the history file and the base view reader
    historyFile.close();
    // Close the base view reader
    close();

    // Reopen the base view reader with the history file object
    return FileViewReader::open(buff, buffSize, SD.open(TEMP_HISTORY_FILE_PATH, FILE_READ));
}
