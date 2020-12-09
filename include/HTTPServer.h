#ifndef HTTPServer_h
#define HTTPServer_h

#include <SPI.h>
#ifdef ESP32
#include <WiFi.h>
#else
#include <Ethernet.h>
#endif
#include <View.h>
#include <Controller.h>

class HTTPServer
{
public:
    static void Init();
    static void ServeClient();
    static void AddView(View *view);
    static void AddController(Controller *controller);
private:
    static EthernetServer server;
    static void CheckForNewClients();
};

void InitHTTPServer();
void DoHTTPService();

#endif // HTTPServer_h