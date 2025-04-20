#include <RecoveryController.h>
#include <RecoveryControl.h>
#include <EthernetUtil.h>
#include <HttpHeaders.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

bool RecoveryController::Get(HttpClientContext &context, const String id)
{
#ifdef DEBUG_HTTP_SERVER
    Traceln("RecoveryController Get");
#endif
    return false;
}

bool RecoveryController::Put(HttpClientContext &context, const String id)
{
#ifdef DEBUG_HTTP_SERVER
    Traceln("RecoveryController Put");
#endif
    return false;
}

bool RecoveryController::Delete(HttpClientContext &context, const String id)
{
#ifdef DEBUG_HTTP_SERVER
    Traceln("RecoveryController Delete");
#endif
    return false;
}

bool RecoveryController::Post(HttpClientContext &context, const String id)
{
#ifdef DEBUG_HTTP_SERVER
    Traceln("RecoveryController Post");
#endif
    String content;
    EthClient client = context.getClient();

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

HttpController *RecoveryController::getInstance() { return &recoveryController; }

RecoveryController recoveryController;