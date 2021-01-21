#include <View.h>
#include <SDUtil.h>
#include <time.h>
#include <Common.h>
#include <Config.h>

int View::viewHandler(byte *buff, int buffSize)
{
    for(int i = 0; i < buffSize; i++)
    {
        if (buff[i] == (byte)'%')
        {
            int j = i + 1;
            for (; j < buffSize; j++)
                if (buff[j] == (byte)' ')
                    break;
            if (j == buffSize)
                return i;

            int nFill;
            if (sscanf((const char *)buff + i + 1, "%d", &nFill) != 1)
            {
#ifdef DEBUG_HTTP_SERVER
                Traceln("Bad filler index!");
#endif
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
            for(j = i; buff[j] != (byte)' '; j++)
                buff[j] = (byte)' ';
            for (unsigned int j = 0; j < fill.length(); j++)
            {
                if (buff[i + j] != (char)' ')
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

#ifndef ESP32
bool View::openWWWROOT(SdFile &dir)
{
    SdFile root;
    if (root.openRoot(vol) == 0)
    {
#ifdef DEBUG_HTTP_SERVER
        Traceln("Faield to open root directory");
#endif
        return false;
    }

    if (dir.open(root, "wwwroot", O_READ) == 0)
    {
#ifdef DEBUG_HTTP_SERVER
        Traceln("Failed to open wwwroot directory");
#endif
        root.close();
        return false;
    }

    root.close();
    
    return true;
}
#endif

bool View::open(byte *_buff, int _buffSize)
{
    buff = _buff;
    buffSize = _buffSize;
    offset = buffSize;

#ifndef ESP32
    SdFile dir;
    if (!openWWWROOT(dir))
        return false;

    String dirName;
    int index = 1;
    int slash;
    while((slash = viewFilePath.indexOf('/', index)) != -1)
    {
        String dirName = viewFilePath.substring(index, slash);
        SdFile subDir;
        if (subDir.open(dir, dirName.c_str(), O_READ) == 0)
        {
#ifdef DEBUG_HTTP_SERVER
            Trace("Faield to open directory ");
            Traceln(dirName.c_str());
#endif
            dir.close();
            return false;
        }
        dir.close();
        dir = subDir;
        index = slash + 1;
    }
#endif

    String fileName;
#ifndef ESP32
    fileName = viewFilePath.substring(index);
#else
    fileName = "/wwwroot" + viewFilePath;
#endif
#ifndef ESP32
    if (file.open(dir, fileName.c_str(), O_READ) == 0)
#else
    file = SD.open(fileName, FILE_READ);
    if (!file)
#endif
    {
#ifdef DEBUG_HTTP_SERVER
        Trace("Failed to open file ");
        Traceln(fileName.c_str());
#endif
#ifndef ESP32
        dir.close();
#endif
        return false;
    }

#ifndef ESP32
    dir.close();
#endif

    return true;
}

void View::close()
{
    file.close();
}

long View::getViewSize()
{
#ifndef ESP32
    return file.fileSize();
#else
    return file.size();
#endif
}

bool View::redirect(EthernetClient &client, const String &id)
{
    return false;
}

bool View::getLastModifiedTime(String &lastModifiedTimeStr)
{
#ifndef ESP32
    dir_t dirEntry;
    memset(&dirEntry, 0, sizeof(dirEntry));
    if (file.dirEntry(dirEntry) == 0)
    {
#ifdef DEBUG_HTTP_SERVER
        Trace(__func__);
        Traceln(": failed in file.dirEntry()");
#endif
        return false;
    }
    uint16_t lastWriteTime = dirEntry.lastWriteTime;
    uint16_t lastWriteDate = dirEntry.lastWriteDate;
    tm tr;
    memset(&tr, 0, sizeof(tr));
    tr.tm_hour = FAT_HOUR(lastWriteTime);
    tr.tm_min = FAT_MINUTE(lastWriteTime);
    tr.tm_sec = FAT_SECOND(lastWriteTime);
    tr.tm_year = FAT_YEAR(lastWriteDate) - 1900;
    tr.tm_mon = FAT_MONTH(lastWriteDate) - 1;
    tr.tm_mday = FAT_DAY(lastWriteDate);
    time_t fileTime = mktime(&tr);
    if (fileTime == (time_t)-1)
    {
#ifdef DEBUG_HTTP_SERVER
        Trace(__func__);
        Traceln(": failed in mktime");
#endif
        return false;
    }
    fileTime = fileTime - Config::timeZone * 60; // Set to GMT
#else
    tm tr;
    time_t fileTime = file.getLastWrite();
#endif
    gmtime_r(&fileTime, &tr);
    char lastModifiedTime[64];
    // Last-Modified: Sun, 21 Jun 2020 14:33:06 GMT
    strftime(lastModifiedTime, sizeof(lastModifiedTime), "%a, %d %h %Y %H:%M:%S GMT", &tr);
    lastModifiedTimeStr = lastModifiedTime;

    return true;
}

CONTENT_TYPE View::getContentType()
{
    int dot = viewFilePath.lastIndexOf('.');

    if (dot == -1)
        return CONTENT_TYPE::CT_UNKNOWN;

    String ext = viewFilePath.substring(dot + 1);
    ext.toUpperCase();
    if (ext.equals("JS"))
        return CONTENT_TYPE::JAVASCRIPT;
    else if (ext.equals("ICO"))
        return CONTENT_TYPE::ICON;
    else if (ext.equals("HTM"))
        return CONTENT_TYPE::HTML;
    else if (ext.equals("CSS"))
        return CONTENT_TYPE::CSS;
    else if (ext.equals("JPG"))
        return CONTENT_TYPE::JPEG;
    else if (ext.equals("MAP"))
        return CONTENT_TYPE::CSS;
    else if (ext.equals("EOT"))
        return CONTENT_TYPE::EOT;
    else if (ext.equals("SVG"))
        return CONTENT_TYPE::SVG;
    else if (ext.equals("TTF"))
        return CONTENT_TYPE::TTF;
    else if (ext.equals("WOF"))
        return CONTENT_TYPE::WOFF;
    else if (ext.equals("WF2"))
        return CONTENT_TYPE::WOFF2;

    return CONTENT_TYPE::CT_UNKNOWN;
}

int View::read()
{
    memcpy(buff, buff + offset, buffSize - offset);
    offset = buffSize - offset;
    int nBytes = file.read(buff + offset, buffSize - offset) + offset;
    offset = viewHandler(buff, nBytes);
    return offset;
}

bool View::post(EthernetClient &client, const String &resource, const String &id)
{
    return false;
}

bool View::DoFill(int nFill, String &fill)
{
    return false;
}
