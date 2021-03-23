#ifdef ESP32
#include <Common.h>
#include <FilesView.h>

FilesView::FilesView(const char *_viewName, const char *_viewFile) : 
   View(_viewName, _viewFile)
{
}

bool FilesView::post(EthClient &client, const String &resource, const String &id)
{
    String normilizedPath = "/" + id;
    normilizedPath.replace("%20", " ");
#ifdef DEBUG_HTTP_SERVER
    Tracef("FilesView: POST: resource=%s, id=%s, path=%s\n", resource.c_str(), id.c_str(), normilizedPath.c_str());
#endif
    SdFile dir = SD.open(normilizedPath, FILE_READ);
    String resp;
    if (!dir)
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
            resp += String(first ? "" : ",\n") + "{ \"time\": \"" + buff + "\", \"name\": \"" + file.name() + "\", \"isDir\": " + (file.isDirectory() ? "true" : "false") + ", \"size\": " + file.size() + " }";
            file.close();
            file = dir.openNextFile(FILE_READ);
            first = false;
        }
        resp += "] } ]";
    }
#ifdef DEBUG_HTTP_SERVER
    Traceln(resp);
#endif
    client.println("HTTP/1.1 200 OK");
    client.print("Content-Length: ");
    client.println(resp.length());
    client.println("Content-Type: application/json");
    client.println("Connection: close"); 
    client.println("Server: Arduino");
    client.println();
    client.print(resp);

    return true;
}

FilesView filesView("/FILES", "/FILES.HTM");
#endif // ESP32