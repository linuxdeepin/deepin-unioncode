#include "runtimecfgprovider.h"
#include "debuggerglobals.h"

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

using namespace dap;
static const int kPort = 4711;
RunTimeCfgProvider::RunTimeCfgProvider(QObject *parent)
    : QObject(parent)
{
}

const char *RunTimeCfgProvider::ip() const
{
    return "localhost";
}

int RunTimeCfgProvider::port()
{
    if (iPort > 0)
        return iPort;

    int numAttempt = 10;
    while (numAttempt-- > 0) {
        QDBusInterface interface("com.deepin.unioncode.service", "/",
                                 "com.deepin.unioncode.interface",
                                 QDBusConnection::sessionBus());

        if (interface.isValid()) {
            QDBusReply<int> reply = interface.call("port");
            if (reply.isValid()) {
                iPort = reply.value();
                break;
            }
        } else {
            qInfo() << "qdbus error:" << qPrintable(QDBusConnection::sessionBus().lastError().message());

            QDBusError::ErrorType errtype = QDBusConnection::sessionBus().lastError().type();
            if (errtype == QDBusError::ErrorType::ServiceUnknown) {
                QThread::msleep(200);
                continue;
            } else {
                break;
            }
        }
    };

    return iPort;
}

dap::string RunTimeCfgProvider::launchRequest(const QString &target)
{
#if DBG_TEST
    QString launchRequest = QString("{ \
        \"arguments\":{\
                \"name\": \"(gdb) Launch\",\
                \"type\": \"cppdbg\",\
                \"request\": \"launch\",\
                \"program\": 1%,\
                \"args\": [],\
                \"stopAtEntry\": false,\
                \"cwd\": \"your project config directory.\",\
                \"environment\": [],\
                \"externalConsole\": false,\
                \"MIMode\": \"gdb\",\
                \"setupCommands\": [{\
                    \"description\": \"Enable pretty-printing for gdb\",\
                    \"text\": \"-enable-pretty-printing\",\
                    \"ignoreFailures\": true\
                }, {\
                    \"description\": \"Set Disassembly Flavor to Intel\",\
                    \"text\": \"-gdb-set disassembly-flavor intel\",\
                    \"ignoreFailures\": true\
                }],\
                \"logging\": {\
                    \"trace\": true,\
                    \"traceResponse\": true,\
                    \"engineLogging\": false\
                },\
                \"__configurationTarget\": 5,\
                \"__sessionId\": \"7f671b42-2db0-4896-ad6d-ed8e3ac62380\"\
        },\
        \"command\":\"launch\",\
        \"seq\":4,\
        \"type\":\"request\"\
    }").arg(target);
#else
    // get cfg from local file.
#endif
    std::string ret = launchRequest.toStdString();
    return ret;
}

dap::InitializeRequest RunTimeCfgProvider::initalizeRequest()
{
    // Just use temporary parameters now, same for the back
    InitializeRequest request;
#ifdef DBG_TEST
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
#else
    // get cfg from local file.
#endif

    return request;
}
