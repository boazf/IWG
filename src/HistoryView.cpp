#include <HistoryView.h>
#include <SDUtil.h>
#include <HistoryControl.h>
#include <Common.h>
#include <Config.h>
#include <TimeUtil.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

typedef bool(*fillFile)(SdFile &file);

static bool fillAlerts(SdFile &file)
{
    if (historyControl.Available() == 0)
    {
        char noHistory[] = "<div class=\"alert alert-success\">There is no history yet.</div>\n";
        file.write((byte *)noHistory, NELEMS(noHistory) - 1);
        return true;
    }

    for(int i = 0; i < historyControl.Available(); i++)
    {
        HistoryStorageItem hItem = historyControl.GetHistoryItem(i);
        {
            char str[] = "<div class=\"col-lg-3 col-md-4 col-sm-6 col-xs-12\">\n";
            file.write((byte *)str, NELEMS(str) - 1);
        }
        char buff[128];
        int len = sprintf(buff, "<div id=\"historyItem%d\" class=\"alert\">\n", i);
        file.write((byte *)buff, len);
        len = sprintf(buff, "<h4 id=\"recoverySource%d\" class=\"alert-heading\"></h4>\n<hr />\n<p><span class=\"attribute-name\">\n", i);
        file.write((byte *)buff, len);
        if (hItem.endTime() != INT32_MAX)
        {
            char str[] = "Start ";
            file.write((byte *)str, NELEMS(str) - 1);
        }
        {
            tm tr;
            time_t startTime = hItem.startTime();
            localtime_r(&startTime, &tr);
            char timeBuff[64];
            strftime(timeBuff, sizeof(buff), "%d/%m/%Y %T", &tr);
            len = sprintf(buff, "Time:</span><br /><span class=\"indented\">%s</span></p>\n<p ", timeBuff);
            file.write((byte *)buff, len);
        }
        if (hItem.endTime() == INT32_MAX)
        {
            char str[] = "style=\"visibility:hidden\"";
            file.write((byte *)str, NELEMS(str) - 1);
        }
        {
            char str[] = "><span class=\"attribute-name\">End Time:</span><br /><span class=\"indented\">";
            file.write((byte *)str, NELEMS(str) - 1);
        }
        {
            tm tr;
            time_t startTime = hItem.endTime();
            localtime_r(&startTime, &tr);
            char timeBuff[64];
            len = strftime(timeBuff, sizeof(buff), "%d/%m/%Y %T", &tr);
            file.write((byte *)timeBuff, len);
        }
        {
            char str[] = "</span></p>\n<p ";
            file.write((byte *)str, NELEMS(str) - 1);
        }
        if (hItem.modemRecoveries() == 0 && hItem.routerRecoveries() == 0)
        {
            char str[] = "style=\"visibility:hidden\"";
            file.write((byte *)str, NELEMS(str) - 1);
        }
        {
            char str[] = "><span class=\"attribute-name\">Recoveries:</span><br />";
            file.write((byte *)str, NELEMS(str) - 1);
        }
        len = sprintf(buff, "<span class=\"indented\">%s: %d</span>", Config::deviceName, hItem.routerRecoveries());
        file.write((byte *)buff, len);
        if (!Config::singleDevice)
        {
            len = sprintf(buff, "<span class=\"indented\">Modem: %d</span>", hItem.modemRecoveries());
            file.write((byte *)buff, len);
        }
        len = sprintf(buff, "</p>\n<hr />\n<h4 id=\"recoveryStatus%d\"></h4>\n</div>\n</div>\n", i);
        file.write((byte *)buff, len);
    }

    return true;
}

static bool fillJS(SdFile &file)
{
    char buff[128];
    for (int i = 0; i < historyControl.Available(); i++)
    {
        HistoryStorageItem hItem = historyControl.GetHistoryItem(i);
        int len = sprintf(buff, "setRecoverySource(%d, %d);\n", i, static_cast<int>(hItem.recoverySource()));
        file.write((byte *)buff, len);
        len = sprintf(buff, "setRecoveryStatus(%d, %d);\n", i, static_cast<int>(hItem.recoveryStatus()));
        file.write((byte *)buff, len);
    }
    return true;
}

CriticalSection HistoryView::cs;

#define TEMP_HISTORY_FILE_DIR "/wwwroot/temp"
#define TEMP_HISTORY_FILE_PATH TEMP_HISTORY_FILE_DIR "/history.htm"

bool HistoryView::open(byte *buff, int buffSize)
{
    AutoSD autoSD;
    cs.Enter();

    SdFile historyFile;
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

    historyFile = SD.open(TEMP_HISTORY_FILE_PATH, FILE_READ);
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

        if (!FileView::open(buff, buffSize))
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
        return FileView::open(buff, buffSize, historyFile);
    }

    fillFile fillers[] = { fillAlerts, fillJS };
    for (int nBytes = FileView::read(); nBytes; nBytes = FileView::read())
    {
        int byte0 = 0;
        for (int i = 0; i < nBytes; i++)
        {
            if (buff[i] == (byte)'%')
            {
                if (i > 0)
                    historyFile.write(buff + byte0, i - byte0 - 1);
                if (i == nBytes - 1)
                {
#ifdef DEBUG_HTTP_SERVER
                    Traceln("Filler resides at end of buffer, cannot process filler!");
#endif
                    byte0 = nBytes;
                    continue;
                }
                int fillerIndex = i + 1;
                for(; buff[i] != (byte)'\n' && i < nBytes; i++);
                if (i == nBytes)
                {
#ifdef DEBUG_HTTP_SERVER
                    Traceln("Filler resides at end of buffer, cannot process filler!");
#endif
                    byte0 = nBytes;
                    continue;
                }
                byte0 = i + 1;
                int n;
                sscanf((char *)(buff + fillerIndex), "%d", &n);
                fillers[n - 1](historyFile);
            }
        }
        historyFile.write(buff + byte0, nBytes - byte0);
    }

    FileView::close();
    historyFile.close();

    return FileView::open(buff, buffSize, SD.open(TEMP_HISTORY_FILE_PATH, FILE_READ));
}

void HistoryView::close()
{
    FileView::close();
    cs.Leave();
}
