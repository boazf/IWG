#include <HistoryView.h>
#include <SDUtil.h>
#include <HistoryControl.h>
#include <Common.h>

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
        len = sprintf(buff, "<span class=\"indented\">Router: %d</span>", hItem.routerRecoveries());
        file.write((byte *)buff, len);
        len = sprintf(buff, "<span class=\"indented\">Modem: %d</span>", hItem.modemRecoveries());
        file.write((byte *)buff, len);
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
        int len = sprintf(buff, "setRecoverySource(%d, %d);\n", i, (int)hItem.recoverySource());
        file.write((byte *)buff, len);
        len = sprintf(buff, "setRecoveryStatus(%d, %d);\n", i, (int)hItem.recoveryStatus());
        file.write((byte *)buff, len);
    }
    return true;
}

#ifdef ESP32
#define TEMP_HISTORY_FILE_DIR "/wwwroot/temp"
#define TEMP_HISTORY_FILE_PATH TEMP_HISTORY_FILE_DIR "/history.htm"
#endif

bool HistoryView::open(byte *buff, int buffSize)
{
    if (!View::open(buff, buffSize))
        return false;

#ifndef ESP32
    SdFile dir;

    if (!openWWWROOT(dir))
        return false;

    SdFile tempDir;

    if (!tempDir.open(dir, "TEMP", O_READ))
    {
        if (!tempDir.makeDir(dir, "TEMP"))
        {
#ifdef DEBUG_HTTP_SERVER
            Traceln("Failed to open TEMP directory");
#endif            
            dir.close();
            return false;
        }
    }

    dir.close();
#endif

    SdFile historyFile;
#ifndef ESP32
    if (!historyFile.open(tempDir, "HISTORY.HTM", O_WRITE | O_READ | O_CREAT | O_TRUNC))
#else
    if (!SD.exists(TEMP_HISTORY_FILE_DIR))
        SD.mkdir(TEMP_HISTORY_FILE_DIR);
    historyFile = SD.open(TEMP_HISTORY_FILE_PATH, FILE_WRITE);
    if (!historyFile)
#endif
    {
#ifdef DEBUG_HTTP_SERVER
        Traceln("Failed to open HISTORY.HTM file");
#endif
#ifndef ESP32            
        tempDir.close();
#endif
        return false;
    }

#ifndef ESP32            
    tempDir.close();
#endif

    fillFile fillers[] = { fillAlerts, fillJS };
    for (int nBytes = file.read(buff, buffSize); nBytes; nBytes = file.read(buff, buffSize))
    {
        int byte0 = 0;
        for (int i = 0; i < nBytes; i++)
        {
            if (buff[i] == (byte)'%')
            {
                historyFile.write(buff + byte0, i - byte0 - 1);
                int n;
                sscanf((char *)(buff + i + 1), "%d", &n);
                fillers[n - 1](historyFile);
                for(; buff[i] != (byte)'\n'; i++);
                byte0 = i + 1;
            }
        }
        historyFile.write(buff + byte0, nBytes - byte0);
    }

    file.close();
#ifndef ESP32
    file = historyFile;
    file.seekSet(0);
#else
    historyFile.close();
    file = SD.open(TEMP_HISTORY_FILE_PATH, FILE_READ);
#endif

    return true;
}

HistoryView historyView("/HISTORY", "/HISTORY.HTM");