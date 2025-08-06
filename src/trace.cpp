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
#include <Trace.h>
#include <AutoPtr.h>
#include <SDUtil.h>
#include <TimeUtil.h>
#include <queue>
#include <PwrCntl.h>

static char logFileName[80];

#define LOG_DIR "/logs"
#define MAX_LOG_FILE_SIZE (4 * 1024 * 1024)

/// @brief Get the file time from the file name.
/// @param file The SD file to extract the time from.
/// @return The extracted time as a time_t value.
static time_t GetFileTimeFromFileName(SdFile file)
{
    const char *fileName = file.name();
    tm tmFile;
    memset(&tmFile, 0, sizeof(tm));
    int n = sscanf(
                fileName, 
                "Log%d-%d-%d-%d-%d-%d.txt", 
                &tmFile.tm_year, 
                &tmFile.tm_mon, 
                &tmFile.tm_mday, 
                &tmFile.tm_hour, 
                &tmFile.tm_min, 
                &tmFile.tm_sec);

    if (n != 6)
        return (time_t)0;
    // Adjust the tm structure to match the expected format.
    // tm_year is years since 1900, tm_mon is 0-11.
    tmFile.tm_year -= 1900;
    tmFile.tm_mon -= 1;
    // Convert the tm structure to time_t.
    return mktime(&tmFile);
}

/// @brief Holds the esp log function pointer.
static vprintf_like_t esp_log_func = NULL;
// Forward declaration of the function to be used for logging.
size_t Tracevf(const char *format, va_list valist);

/// @brief Function to log messages to a file.
/// @param format The format string for the log message.
/// @param valist The variable argument list containing the values to be formatted.
/// @return The number of characters written.
/// @note After logging the message to the file, the original ESP log function is called.
static int esp_log_to_file(const char *format, va_list valist)
{
    Tracevf(format, valist);
    return esp_log_func(format, valist);
}

void InitSerialTrace()
{
    // Open serial communications and wait for port to open:
    Serial.begin(115200);
    // Wait for serial port to connect. Needed for native USB port only
    while (!Serial);
}

/// @brief Semaphore for log producer-consumer pattern.
static SemaphoreHandle_t logSem = xSemaphoreCreateCounting(INT32_MAX , 0);
/// @brief Queue to hold log messages.
static std::queue<String> messages;

/// @brief Create a new log file name based on the current time.
/// @note The log file name is created in the format "LogYYYY-MM-DD-HH-MM-SS.txt" and stored in the LOG_DIR directory.
static void CreateNewLogFileName()
{
    time_t now = t_now;
    tm tmFile;
    localtime_r(&now, &tmFile);
    sprintf(logFileName, "%s/Log%4d-%02d-%02d-%02d-%02d-%02d.txt", LOG_DIR, tmFile.tm_year + 1900, tmFile.tm_mon + 1, tmFile.tm_mday, tmFile.tm_hour, tmFile.tm_min, tmFile.tm_sec);    
}

/// @brief Function to write a timestamp to the log file.
/// @param logFile The SD file to write the timestamp to.
/// @note The timestamp is written in the format "YYYY-MM-DD HH:MM:SS> ".
static void TraceTimeStamp(SdFile logFile)
{
    time_t now = t_now;
    tm timeStamp;
    localtime_r(&now, &timeStamp);
    char buff[64];
    strftime(buff, sizeof(buff), "%F %H:%M:%S> ", &timeStamp);
    logFile.write(reinterpret_cast<const uint8_t *>(buff), strlen(buff));
    logFile.flush();
}

/// @brief Function to log messages to a file.
/// @param logFile The SD file to write the log message to.
/// @param shouldTraceTimeStamp A reference to a boolean indicating whether to write a timestamp.
static void Log(SdFile logFile, bool &shouldTraceTimeStamp)
{
    // Get the next message from the queue. Since we are using a producer-consumer pattern, we should always have at least one message to log.
    String messageStr = messages.front();

    const char *message = messageStr.c_str();
    const char *newLine = strchr(message, '\n');
    while (newLine != NULL)
    {
        // If we are at the start of a new line, write the timestamp.
        if (shouldTraceTimeStamp)
            TraceTimeStamp(logFile);
        // Write the message up to the newline character.
        logFile.write(reinterpret_cast<const uint8_t *>(message), newLine - message + 1);
        // Since we are at the beginning of a new line, set the flag to write a timestamp for the next message.
        shouldTraceTimeStamp = true;
        // Update the message pointer to the next part of the message.
        message = newLine + 1;
        // Find the next newline character, if any.
        newLine = strchr(message, '\n');
    }

    // If we didn't reach the end of message, it means we have a partial message left, or a message that doesn't contain a newline character.
    if (message[0] != '\0')
    {
        if (shouldTraceTimeStamp)
            // If we are at the start of a new line, write the timestamp.
            TraceTimeStamp(logFile);
        // Since after logging this message we will not be at the start of a new line, we set the flag to false.
        shouldTraceTimeStamp = false;
        // Write the remaining part of the message.
        logFile.write(reinterpret_cast<const uint8_t *>(message), strlen(message));
    }
    // Flush the log file to ensure all data is written.
    logFile.flush();
    // Pop the message from the queue.
    messages.pop();
}

/// @brief Task to log messages to a file.
/// This task runs indefinitely, waiting for messages to log.
/// It will write messages to the log file and handle the hard reset stages.
/// @param parameter Unused parameter, can be NULL.
/// @note This task is pinned to the second core of the ESP32.
static void FileLoggerTask(void *parameter)
{
    // Any message at this point will be logged as a new line, so we set the flag to true.
    bool shouldTraceTimeStamp = true;
    while(true)
    {
        // Wait for a message to log.
        xSemaphoreTake(logSem, portMAX_DELAY);
        {
            // Start a SD card session.
            AutoSD autoSD;
            // Open the log file in append mode.
            SdFile logFile = SD.open(logFileName, FILE_APPEND);

            // If the log file exceeded the maximum size, create a new log file.
            size_t fileSize = logFile.size();
            if (fileSize > MAX_LOG_FILE_SIZE)
            {
                logFile.close();
                CreateNewLogFileName();
                logFile = SD.open(logFileName, FILE_WRITE);
                shouldTraceTimeStamp = true;
            }

            // Log the next message from the queue.
            Log(logFile, shouldTraceTimeStamp);

            // If new messages keeps arriving we will keep logging them until there are no more messages for 2 seconds.
            // This is in order not to create a new SD session for every message, which would be inefficient.
            while(xSemaphoreTake(logSem, 2000 / portTICK_PERIOD_MS ))
            {
                Log(logFile, shouldTraceTimeStamp);
            }

            // For now, close the log file after logging all messages.
            logFile.close();
        }
    }
}

void InitFileTrace()
{
    // Initialize the hard reset event observer to handle hard reset stages.
    hardResetEvent.addObserver([](const HardResetEventParam &param, const void *context)
    {
        switch (param.stage)
        {
            case HardResetStage::prepare:
                // Stop any further logging.
                csTraceLock.Enter();
                break;
            case HardResetStage::shutdown:
                {   
                    // Wait for the log task to write all pending messages to the log file.
                    unsigned long t0 = millis();
                    while (!messages.empty() && millis() - t0 < param.timeout);
                }
                break;
            case HardResetStage::failure:
                // Allow logging to continue after the hard reset failure.
                csTraceLock.Leave();
                break;
        }
    }, NULL);

    // Start an SD card session.
    AutoSD autoSD;

    // Create the log directory if it doesn't exist.
    if (!SD.exists(LOG_DIR))
        SD.mkdir(LOG_DIR);

    // Open the log directory to find the latest log file.
    SdFile logDir = SD.open(LOG_DIR);
    // Iterate through the files in the log directory to find the latest log file.
    SdFile fileInLogDir = logDir.openNextFile();
    time_t lastTime = 0;
    while (fileInLogDir)
    {
        time_t fileTime = GetFileTimeFromFileName(fileInLogDir);
        if (lastTime < fileTime)
        {
            // If this file is newer than the last one, update the log file name.
            strcpy(logFileName, fileInLogDir.path());
            lastTime = fileTime;
        }
        // Close the current file and move to the next one.
        fileInLogDir.close();
        fileInLogDir = logDir.openNextFile();
    }

    if (lastTime == 0)
    {
        // If no log file was found, create the first log file name.
        CreateNewLogFileName();
    }
    logDir.close();
    // Set ESP log function to log to file.
    // This will redirect all ESP log messages to the file logger task.
    // The esp_log_to_file function will be called with the log messages.
    // The original esp_log function will be called after logging to the file.
    esp_log_func = esp_log_set_vprintf(esp_log_to_file);
    // Create the file logger task to log messages to the file.
    xTaskCreatePinnedToCore(
        FileLoggerTask,
        "FileLoggerTask",
        1024*8,
        NULL,
        1,
        NULL,
        1 - xPortGetCoreID());
}

/// @brief Function to log a message to the serial port and the log file.
/// @param message The message to log.
/// @note This function locks the trace lock to ensure no trace messages are interleaved.
/// @return The number of characters written to the serial port and log file.
size_t Trace(const char *message) 
{ 
    LOCK_TRACE;
    
    // Trace the message to the serial port.
    size_t ret = Serial.print(message);

    // Add the message to the messages queue for logging to the file.
    messages.push(message);
    // Notify the file logger task that there is a new message to log (consume).
    xSemaphoreGive(logSem);

    return ret;
};

/// @brief Function to log a formatted message to the serial port and the log file.
/// @param format The format string.
/// @param valist The variable argument list.
/// @return The number of characters written to the serial port and log file, or negative value on error.
size_t Tracevf(const char *format, va_list valist)
{
    LOCK_TRACE;

    char buff[81];

    // Use vsnprintf to format the message into a fixed-size buffer.
    int len = vsnprintf(buff, sizeof(buff), format, valist);
    if (len < 0)
        // An error occurred during formatting, return the error code.
        return len;
    if (len >= (int)sizeof(buff))
    {
        // The formatted message is too long for the buffer, allocate a larger buffer.
        AutoPtr<char> tempBuff(new char[len+1]);
        // Use vsnprintf again to format the message into the larger buffer.
        if (vsnprintf(tempBuff, len + 1, format, valist) != len)
            // If vsnprintf fails again, return error code.
            return -1;
        // Log the message using the larger buffer.
        return Trace(tempBuff);
    }

    // If the formatted message fits in the fixed-size buffer, log it directly.
    return Trace(buff);
}

/// @brief Function to log a formatted message to the serial port and the log file.
/// @param format The format string.
/// @param  ... The variable arguments to format the message.
/// @return The number of characters written to the serial port and log file, or negative value on error.
size_t Tracef(const char *format, ...)
{
    LOCK_TRACE;

    va_list valist;
    va_start(valist, format);

    return Tracevf(format, valist);
}

/// @brief Global critical section object for trace operations.
/// This critical section is used to ensure that trace operations are not interleaved.
CriticalSection csTraceLock;

