#include <RecoveryController.h>
#include <RecoveryControl.h>
#include <Ethernet.h>

bool RecoveryController::Get(EthernetClient &client, String &resource)
{
#ifdef DEBUG_HTTP_SERVER
    Serial.println("RecoveryController Get");
#endif
    return false;
}

bool RecoveryController::Post(EthernetClient &client, String &resource)
{
#ifdef DEBUG_HTTP_SERVER
    Serial.println("RecoveryController Post");
#endif
    String content;

    while (client.available())
    {
        content += (char)client.read();
    }

#ifdef DEBUG_HTTP_SERVER
    Serial.print("RecoveryController::Post: ");
    Serial.println(content);
#endif

    RecoveryTypes recoveryType;
    sscanf(content.c_str(), "{\"recoveryType\":%d}", reinterpret_cast<int*>(&recoveryType));

#ifdef DEBUG_HTTP_SERVER
    Serial.print("RecoveryType: ");
    Serial.println(recoveryType);
#endif

    recoveryControl.StartRecoveryCycles(recoveryType);

    client.println("HTTP/1.1 200 OK");
    client.println("Connection: close");  // the connection will be closed after completion of the response
    client.println("Access-Control-Allow-Origin: *");  // allow any connection. We don't want Arduino to host all of the website ;-)
    client.println("Cache-Control: no-cache");  // refresh the page automatically every 5 sec
    client.println("Content-Length: 0");
    client.println();
    client.flush();

    return true;
}

RecoveryController recoveryController;