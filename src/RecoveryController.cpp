#include <RecoveryController.h>
#include <RecoveryControl.h>
#include <EthernetUtil.h>

bool RecoveryController::Get(EthClient &client, String &resource)
{
#ifdef DEBUG_HTTP_SERVER
    Traceln("RecoveryController Get");
#endif
    return false;
}

bool RecoveryController::Put(EthClient &client, String &resource)
{
#ifdef DEBUG_HTTP_SERVER
    Traceln("RecoveryController Put");
#endif
    return false;
}

bool RecoveryController::Delete(EthClient &client, String &resource)
{
#ifdef DEBUG_HTTP_SERVER
    Traceln("RecoveryController Delete");
#endif
    return false;
}

bool RecoveryController::Post(EthClient &client, String &resource, size_t contentLength, String contentType)
{
#ifdef DEBUG_HTTP_SERVER
    Traceln("RecoveryController Post");
#endif
    String content;

    while (client.available())
    {
        content += (char)client.read();
    }

#ifdef DEBUG_HTTP_SERVER
    {
		LOCK_TRACE();
        Trace("RecoveryController::Post: ");
        Traceln(content);
    }
#endif

    RecoveryTypes recoveryType;
    sscanf(content.c_str(), "{\"recoveryType\":%d}", reinterpret_cast<int*>(&recoveryType));

#ifdef DEBUG_HTTP_SERVER
    {
		LOCK_TRACE();
        Trace("RecoveryType: ");
        Traceln(recoveryType);
    }
#endif

    recoveryControl.StartRecoveryCycles(recoveryType);

    client.println("HTTP/1.1 200 OK");
    client.println("Connection: close");  // the connection will be closed after completion of the response
    client.println("Access-Control-Allow-Origin: *");  // allow any connection.
    client.println("Cache-Control: no-cache");
    client.println("Content-Length: 0");
    client.println();
#ifdef USE_WIFI
    client.flush();
#endif

    return true;
}

RecoveryController recoveryController;