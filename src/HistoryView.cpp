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

typedef std::map<RecoveryStatus, String> RecoveryStatusesMap;
#define X(a) {RecoveryStatus::a, #a},
static const RecoveryStatusesMap recoveryStatusesMap = 
{
    RecoveryStatuses
};
#undef X

typedef std::map<RecoverySource, String> RecoverySourcesMap;
#define X(a) {RecoverySource::a, #a},
static const RecoverySourcesMap recoverySourcesMap = 
{
    RecoverySources
};
#undef X

typedef bool(*fillFile)(SdFile &file);

#define CHECK_PRINT(f, b, l) if ((l) > NELEMS(b) || f.print(b) != (l)) return false

static bool fillAlerts(SdFile &file)
{
    if (historyControl.Available() == 0)
    {
        char noHistory[] = "<div class=\"alert alert-success\">There is no history yet.</div>\n";
        CHECK_PRINT(file, noHistory, NELEMS(noHistory) - 1);
        return true;
    }

    for(int i = 0; i < historyControl.Available(); i++)
    {
        HistoryStorageItem hItem = historyControl.GetHistoryItem(i);
        {
            char str[] = "<div class=\"col-lg-3 col-md-4 col-sm-6 col-xs-12\">\n";
            CHECK_PRINT(file, str, NELEMS(str) - 1);
        }
        char buff[128];
        int len = snprintf(buff, NELEMS(buff), "<div id=\"historyItem%d\" class=\"alert\">\n", i);
        CHECK_PRINT(file, buff, len);
        len = snprintf(buff, NELEMS(buff), "<h4 id=\"recoverySource%d\" class=\"alert-heading\"></h4>\n<hr />\n<p><span class=\"attribute-name\">\n", i);
        CHECK_PRINT(file, buff, len);
        if (hItem.endTime() != INT32_MAX)
        {
            char str[] = "Start ";
            CHECK_PRINT(file, str, NELEMS(str) - 1);
        }
        {
            tm tr;
            time_t startTime = hItem.startTime();
            localtime_r(&startTime, &tr);
            char timeBuff[64];
            strftime(timeBuff, sizeof(buff), "%d/%m/%Y %T", &tr);
            len = snprintf(buff, NELEMS(buff), "Time:</span><br /><span class=\"indented\">%s</span></p>\n<p ", timeBuff);
            CHECK_PRINT(file, buff, len);
        }
        if (hItem.endTime() == INT32_MAX)
        {
            char str[] = "style=\"visibility:hidden\"";
            CHECK_PRINT(file, str, NELEMS(str) - 1);
        }
        {
            char str[] = "><span class=\"attribute-name\">End Time:</span><br /><span class=\"indented\">";
            CHECK_PRINT(file, str, NELEMS(str) - 1);
        }
        {
            tm tr;
            time_t startTime = hItem.endTime();
            localtime_r(&startTime, &tr);
            char timeBuff[64];
            len = strftime(timeBuff, sizeof(buff), "%d/%m/%Y %T", &tr);
            CHECK_PRINT(file, timeBuff, len);
        }
        {
            char str[] = "</span></p>\n<p ";
            CHECK_PRINT(file, str, NELEMS(str) - 1);
        }
        if (hItem.modemRecoveries() == 0 && hItem.routerRecoveries() == 0)
        {
            char str[] = "style=\"visibility:hidden\"";
            CHECK_PRINT(file, str, NELEMS(str) - 1);
        }
        {
            char str[] = "><span class=\"attribute-name\">Recoveries:</span><br />";
            CHECK_PRINT(file, str, NELEMS(str) - 1);
        }
        len = snprintf(buff, NELEMS(buff), "<span class=\"indented\">%s: %d</span>", Config::deviceName, hItem.routerRecoveries());
        CHECK_PRINT(file, buff, len);
        if (!Config::singleDevice)
        {
            len = snprintf(buff, NELEMS(buff), "<span class=\"indented\">Modem: %d</span>", hItem.modemRecoveries());
            CHECK_PRINT(file, buff, len);
        }
        len = snprintf(buff, NELEMS(buff), "</p>\n<hr />\n<h4 id=\"recoveryStatus%d\"></h4>\n</div>\n</div>\n", i);
        CHECK_PRINT(file, buff, len);
    }

    return true;
}

#define recoveryStatusEnumName "recoveryStatus"
#define recoverySourceEnumName "recoverySource"
#define enumClose "\t};\n"

template <typename T>
static bool fillEnum(SdFile &file, T map, const char *varName)
{
    char buff[128];

    size_t len = snprintf(buff, NELEMS(buff), "\tconst %s = {\n", varName);
    CHECK_PRINT(file, buff, len);
    for (typename T::const_iterator i = map.begin(); i != map.end(); i++)
    {
        len = snprintf(buff, NELEMS(buff), "\t\t%s: %d,\n", i->second.c_str(), static_cast<int>(i->first));
        CHECK_PRINT(file, buff, len);
    };
    CHECK_PRINT(file, enumClose, NELEMS(enumClose) - 1);

    return true;
}

static bool fillEnums(SdFile &file)
{
    return fillEnum<RecoveryStatusesMap>(file, recoveryStatusesMap, recoveryStatusEnumName) && 
           fillEnum<RecoverySourcesMap>(file, recoverySourcesMap, recoverySourceEnumName);
}

static bool fillJS(SdFile &file)
{
    char buff[128];
    for (int i = 0; i < historyControl.Available(); i++)
    {
        HistoryStorageItem hItem = historyControl.GetHistoryItem(i);
        int len = snprintf(buff, NELEMS(buff), "\t\tsetRecoverySource(%d, %s.%s);\n", i, recoverySourceEnumName, recoverySourcesMap.at(hItem.recoverySource()).c_str());
        CHECK_PRINT(file, buff, len);
        len = snprintf(buff, NELEMS(buff), "\t\tsetRecoveryStatus(%d, %s.%s);\n", i, recoveryStatusEnumName, recoveryStatusesMap.at(hItem.recoveryStatus()).c_str());
        CHECK_PRINT(file, buff, len);
    }
    return true;
}

static bool fillRecoverySourceEnum(SdFile file, RecoverySource source)
{
    char buff[128];

    size_t len = snprintf(buff, NELEMS(buff), "%s.%s", recoverySourceEnumName, recoverySourcesMap.at(source).c_str());

    CHECK_PRINT(file, buff, len);

    return true;
}

static bool fillRecoveryStatusEnum(SdFile file, RecoveryStatus status)
{
    char buff[128];

    size_t len = snprintf(buff, NELEMS(buff), "%s.%s", recoveryStatusEnumName, recoveryStatusesMap.at(status).c_str());

    CHECK_PRINT(file, buff, len);

    return true;
}

CriticalSection HistoryViewReader::cs;

#define TEMP_HISTORY_FILE_DIR "/wwwroot/temp"
#define TEMP_HISTORY_FILE_PATH TEMP_HISTORY_FILE_DIR "/history.htm"
#define fillerChar '%'
#define STRNCHR(b, c, n) reinterpret_cast<char *>(memchr(b, c, n))

bool HistoryViewReader::open(byte *buff, int buffSize)
{
    AutoSD autoSD;
    Lock lock(cs);

    if (!SD.exists(TEMP_HISTORY_FILE_DIR))
    {
        if (!SD.mkdir(TEMP_HISTORY_FILE_DIR))
        {
#ifdef DEBUG_HTTP_SERVER
            Tracef("Failed to create directory %s\n", TEMP_HISTORY_FILE_DIR);
#endif
            return false;
        }
    }

    SdFile historyFile = SD.open(TEMP_HISTORY_FILE_PATH, FILE_READ);

    if (!historyFile || 
        historyFile.getLastWrite() > t_now || // Handle case where current time wasn't set well
        historyFile.getLastWrite() < historyControl.getLastUpdate())
    {
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

        if (!FileViewReader::open(buff, buffSize))
        {
            historyFile.close();
            SD.remove(TEMP_HISTORY_FILE_PATH);
            return false;
        }
    }
    else
    {
#ifdef DEBUG_HTTP_SERVER
        Tracef("Reusing %s\n", TEMP_HISTORY_FILE_PATH);
#endif
        return FileViewReader::open(buff, buffSize, historyFile);
    }

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
    for (int nBytes = read(offset) + offset; nBytes; nBytes = read(offset) + offset)
    {
        bool eof = nBytes == offset;
        const char *pBuff = reinterpret_cast<const char *>(buff);
        offset = 0;
        for (const char *delim = STRNCHR(pBuff, fillerChar, nBytes); 
             delim != NULL; 
             delim = STRNCHR(pBuff, fillerChar, nBytes))
        {
            size_t delimIndex = delim - pBuff;
            historyFile.write(pBuff, delimIndex);
            size_t fillerIndex = delimIndex + 1;
            for(; isdigit(pBuff[fillerIndex]) && fillerIndex < nBytes; fillerIndex++);

            if (!eof && nBytes == fillerIndex)
            {
                offset = fillerIndex - delimIndex;
                memcpy(buff, delim, offset);
                nBytes = 0;
                break;
            }
            nBytes -= fillerIndex;
            pBuff += fillerIndex;
            if (fillerIndex == delimIndex + 1)
            {
                // Not a filler
                historyFile.write(fillerChar);
                continue;
            }
            int n = atoi(delim + 1);
            if (!fillers[n - 1](historyFile))
            {
                historyFile.close();
                SD.remove(TEMP_HISTORY_FILE_PATH);
                return false;
            }
        }
        historyFile.write(pBuff, nBytes);
    }

    historyFile.close();
    close();

    return FileViewReader::open(buff, buffSize, SD.open(TEMP_HISTORY_FILE_PATH, FILE_READ));
}
