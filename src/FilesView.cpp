#include <Common.h>
#include <FilesView.h>
#include <HttpHeaders.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

FilesView::FilesView(const char *_viewName, const char *_viewFile) : 
   View(_viewName, _viewFile)
{
}

bool FilesView::post(EthClient &client, const String &resource, const String &id)
{
    String normalizedPath = "/" + id;
    normalizedPath.replace("%20", " ");
#ifdef DEBUG_HTTP_SERVER
    Tracef("FilesView: POST: resource=%s, id=%s, path=%s\n", resource.c_str(), id.c_str(), normalizedPath.c_str());
#endif
    SdFile dir = SD.open(normalizedPath, FILE_READ);
    String resp;
    if (!dir || !dir.isDirectory())
    {
        resp = "[{ \"result\": 1 }]";
    }
    else
    {
        resp = "[ { \"result\": 0 }, { \"files\": [";
        SdFile file = dir.openNextFile(FILE_READ);
        bool first = true;
        while (file)
        {
            time_t fileTime = file.getLastWrite();
            tm tmFile;
            char buff[64];
            localtime_r(&fileTime, &tmFile);
            strftime(buff, sizeof(buff), "%d/%m/%Y %H:%M", &tmFile);
            resp += String(first ? "" : ",\n") + "{ \"time\": \"" + buff + "\", \"name\": \"" + file.path() + "\", \"isDir\": " + (file.isDirectory() ? "true" : "false") + ", \"size\": " + file.size() + " }";
            file.close();
            file = dir.openNextFile(FILE_READ);
            first = false;
        }
        resp += "] } ]";
    }
#ifdef DEBUG_HTTP_SERVER
    Traceln(resp);
#endif
    unsigned int len = resp.length();
    HttpHeaders::Header additionalHeaders[] = {{CONTENT_TYPE::JSON}};
    HttpHeaders headers(client);
    headers.sendHeaderSection(200, true, additionalHeaders, NELEMS(additionalHeaders), len);

    // Send the response in slices because of a limitation of W5500. In case of WiFi this doesn't matter.
    unsigned int index = 0;
    #define BUFF_SIZE 1024U

    for (unsigned int index = 0; index < len; index += BUFF_SIZE)
        client.print(resp.substring(index, min<unsigned int>(index + BUFF_SIZE, len)));

    return true;
}

FilesViewCreator filesViewCreator("/FILES", "/FILES.HTM");
