#include <View.h>
#include <SDUtil.h>
#include <time.h>
#include <Common.h>
#include <Config.h>
#include <HttpHeaders.h>

bool View::open(byte *_buff, int _buffSize, SdFile _file)
{
    buff = _buff;
    buffSize = _buffSize;
    file = _file;

    return file;
}

bool View::open(byte *buff, int buffSize)
{
    String fileName;
    fileName = "/wwwroot" + viewFilePath;
    SdFile file = SD.open(fileName, FILE_READ);
    if (!file)
    {
#ifdef DEBUG_HTTP_SERVER
        LOCK_TRACE();
        Trace("Failed to open file ");
        Traceln(fileName.c_str());
#endif
    }

    return open(buff, buffSize, file);
}

void View::close()
{
    file.close();
}

int View::read()
{
    return file.read(buff, buffSize);
}

long View::getViewSize()
{
    return file.size();
}

bool View::redirect(EthClient &client, const String &id)
{
    return false;
}

bool View::getLastModifiedTime(String &lastModifiedTimeStr)
{
    tm tr;
    time_t fileTime = file.getLastWrite();
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
        return CONTENT_TYPE::UNKNOWN;

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

    return CONTENT_TYPE::UNKNOWN;
}

bool View::post(EthClient &client, const String &resource, const String &id)
{
    return false;
}
