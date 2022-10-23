#include "runtimecfgprovider.h"
#include "debuggerglobals.h"

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

using namespace dap;

RunTimeCfgProvider::RunTimeCfgProvider(QObject *parent)
    : QObject(parent)
{
}

const char *RunTimeCfgProvider::ip() const
{
    return "localhost";
}

dap::InitializeRequest RunTimeCfgProvider::initalizeRequest()
{
    // Just use temporary parameters now, same for the back
    InitializeRequest request;
    request.clientID = "unioncode";
    request.clientName = "unioncode client";
    request.adapterID = "cppdbg";
    request.pathFormat = "path";
    request.linesStartAt1 = true;
    request.columnsStartAt1 = true;
    request.supportsVariableType = false;
    request.supportsVariablePaging = false;
    // Disable RunInTerminal option untile client support.
    request.supportsRunInTerminalRequest = false;
    request.locale = "en-US";
    request.supportsProgressReporting = false;
    request.supportsInvalidatedEvent = false;
    request.supportsMemoryReferences = false;

    return request;
}
