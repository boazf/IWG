#include <View.h>
#include <SDUtil.h>
#include <time.h>
#include <Common.h>
#include <Config.h>
#include <HttpHeaders.h>
#include <map>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

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

typedef std::map<String, CONTENT_TYPE> FileTypesMap;
static FileTypesMap fileTypesMap = 
{
    {"JS", CONTENT_TYPE::JAVASCRIPT},
    {"ICO", CONTENT_TYPE::ICON},
    {"HTM", CONTENT_TYPE::HTML},
    {"CSS", CONTENT_TYPE::CSS},
    {"JPG", CONTENT_TYPE::JPEG},
    {"MAP", CONTENT_TYPE::CSS},
    {"EOT", CONTENT_TYPE::EOT},
    {"SVG", CONTENT_TYPE::SVG},
    {"TTF", CONTENT_TYPE::TTF},
    {"WOF", CONTENT_TYPE::WOFF},
    {"WF2", CONTENT_TYPE::WOFF2}
};

CONTENT_TYPE View::getContentType()
{
    int dot = viewFilePath.lastIndexOf('.');

    if (dot == -1)
        return CONTENT_TYPE::UNKNOWN;

    String ext = viewFilePath.substring(dot + 1);
    ext.toUpperCase();
    FileTypesMap::iterator fileType = fileTypesMap.find(ext);
    if (fileType == fileTypesMap.end())
        return CONTENT_TYPE::UNKNOWN;

    return fileType->second;
}

bool View::post(EthClient &client, const String &resource, const String &id)
{
    return false;
}
