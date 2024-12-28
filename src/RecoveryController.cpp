#include <RecoveryController.h>
#include <RecoveryControl.h>
#include <EthernetUtil.h>
#include <HttpHeaders.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

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
        Traceln((int)recoveryType);
    }
#endif

    recoveryControl.StartRecoveryCycles(recoveryType);

    HttpHeaders::Header additionalHeaders[] = { {"Access-Control-Allow-Origin", "*" }, {"Cache-Control", "no-cache"} };
    HttpHeaders headers(client);
    headers.sendHeaderSection(200, true, additionalHeaders, NELEMS(additionalHeaders));

    return true;
}

RecoveryController recoveryController;