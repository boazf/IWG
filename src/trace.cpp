#include <Arduino.h>
#include <Trace.h>
#include <AutoPtr.h>
#include <SDUtil.h>
#include <TimeUtil.h>
#include <LinkedList.h>

static char logFileName[80];

#define LOG_DIR "/logs"

static time_t GetFileTimeFromFileName(SdFile file)
{
    const char *fileName = file.name();
    tm tmFile;
    memset(&tmFile, 0, sizeof(tm));
    int n = sscanf(
                fileName, 
                LOG_DIR "/Log%d-%d-%d-%d-%d-%d.txt", 
                &tmFile.tm_year, 
                &tmFile.tm_mon, 
                &tmFile.tm_mday, 
                &tmFile.tm_hour, 
                &tmFile.tm_min, 
                &tmFile.tm_sec);

    if (n != 6)
        return (time_t)0;
    tmFile.tm_year -= 1900;
    tmFile.tm_mon -= 1;
    return mktime(&tmFile);
}

static vprintf_like_t esp_log_func = NULL;
size_t Tracevf(const char *format, va_list valist);

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

static SemaphoreHandle_t logSem = xSemaphoreCreateCounting(INT32_MAX , 0);
static LinkedList<String> messages;

static void CreateNewLogFileName()
{
    time_t now = t_now;
    tm tmFile;
    localtime_r(&now, &tmFile);
    sprintf(logFileName, "%s/Log%4d-%02d-%02d-%02d-%02d-%02d.txt", LOG_DIR, tmFile.tm_year + 1900, tmFile.tm_mon + 1, tmFile.tm_mday, tmFile.tm_hour, tmFile.tm_min, tmFile.tm_sec);    
}

static void TraceTimeStamp(SdFile logFile)
{
    time_t now = t_now;
    tm timeStamp;
    localtime_r(&now, &timeStamp);
    char buff[64];
    strftime(buff, sizeof(buff), "%F %H:%M:%S> ", &timeStamp);
    logFile.write((const uint8_t *)buff, strlen(buff));
    logFile.flush();
}

static void Log(SdFile logFile, bool &shouldTraceTimeStamp)
{
    String messageStr;

    messages.ScanNodes([](const String &messageStr, const void *param)->bool
    {
        ((String *)param)->concat(messageStr);
        return false;
    }, &messageStr);

    const char *message = messageStr.c_str();
    char *newLine = strchr(message, '\n');
    while (newLine != NULL)
    {
        if (shouldTraceTimeStamp)
            TraceTimeStamp(logFile);
        logFile.write((const uint8_t *)message, newLine - message + 1);
        shouldTraceTimeStamp = true;
        message = newLine + 1;
        newLine = strchr(message, '\n');
    }
    if (message[0] != '\0')
    {
        if (shouldTraceTimeStamp)
            TraceTimeStamp(logFile);
        shouldTraceTimeStamp = false;
        logFile.write((const uint8_t *)message, strlen(message));
    }
    logFile.flush();
    messages.Delete(messageStr);
}

static void FileLoggerTask(void *parameter)
{
    bool shouldTraceTimeStamp = true;
    while(true)
    {
        xSemaphoreTake(logSem, portMAX_DELAY);
        {
            AutoSD autoSD;
            SdFile logFile = SD.open(logFileName, FILE_APPEND);

            size_t fileSize = logFile.size();
            if (fileSize > 4 * 1024 * 1024)
            {
                logFile.close();
                CreateNewLogFileName();
                logFile = SD.open(logFileName, FILE_WRITE);
                shouldTraceTimeStamp = true;
            }

            Log(logFile, shouldTraceTimeStamp);

            while(xSemaphoreTake(logSem, 2000 / portTICK_PERIOD_MS ))
            {
                Log(logFile, shouldTraceTimeStamp);
            }

            logFile.close();
        }
    }
}

void InitFileTrace()
{
    AutoSD autoSD;

    if (!SD.exists(LOG_DIR))
        SD.mkdir(LOG_DIR);

    SdFile logDir = SD.open(LOG_DIR);
    SdFile fileInLogDir = logDir.openNextFile();
    time_t lastTime = 0;
    while (fileInLogDir)
    {
        time_t fileTime = GetFileTimeFromFileName(fileInLogDir);
        if (lastTime < fileTime)
        {
            strcpy(logFileName, fileInLogDir.name());
            lastTime = fileTime;
        }
        fileInLogDir.close();
        fileInLogDir = logDir.openNextFile();
    }
    if (lastTime == 0)
    {
        CreateNewLogFileName();
    }
    logDir.close();
    esp_log_func = esp_log_set_vprintf(esp_log_to_file);
    xTaskCreatePinnedToCore(
        FileLoggerTask,
        "FileLoggerTask",
        1024*16,
        NULL,
        1,
        NULL,
        1 - xPortGetCoreID());
}

size_t Trace(const char *message) 
{ 
    LOCK_TRACE();
    
    size_t ret = Serial.print(message);

    messages.Insert(message);
    xSemaphoreGive(logSem);

    return ret;
};

size_t Tracevf(const char *format, va_list valist)
{
    LOCK_TRACE();

    char buff[81];

    int len = vsnprintf(buff, sizeof(buff), format, valist);
    if (len < 0)
        return len;
    if (len >= (int)sizeof(buff))
    {
        AutoPtr<char> tempBuff(new char[len+1]);
        vsnprintf(tempBuff, len + 1, format, valist);
        return Trace(tempBuff);
    }

    return Trace(buff);
}

size_t Tracef(const char *format, ...)
{
    LOCK_TRACE();

    va_list valist;
    va_start(valist, format);

    return Tracevf(format, valist);
}

CriticalSection csTraceLock;

