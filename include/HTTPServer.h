#ifndef HTTPServer_h
#define HTTPServer_h

#include <SPI.h>
#include <Ethernet.h>
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
    static void CheckForNewClient();
};

void InitHTTPServer();
void DoHTTPService();

#endif // HTTPServer_h