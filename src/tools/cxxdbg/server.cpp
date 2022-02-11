#include "cxxdebugger.h"
#include "server.h"
#include "context.h"
#include "protocol.h"

#include <memory>
#include <QDebug>

namespace cxxdbg {

static Context configured;
static Context terminate;

class ServerPrivate
{
public:
    explicit ServerPrivate(Server *parent)
        : q(parent)
        , m_server(dap::net::Server::create())
        , m_session(dap::Session::create())
        , m_debugger(cxxdbg::CxxDebugger::create(parent)) {}

    void registerHandler();
    bool start(const char * addr, const int port);
    bool stop();

    std::shared_ptr<CxxDebugger> getDebugger();
    std::shared_ptr<dap::net::Server> getServer();
    std::shared_ptr<dap::Session> getSession();

    Server *q = nullptr;
    const char *addr = "localhost";
    const int port = 4711;
    std::shared_ptr<CxxDebugger> m_debugger;
    std::unique_ptr<dap::net::Server>  m_server;
    std::shared_ptr<dap::Session> m_session;
};

void ServerPrivate::registerHandler()
{
    // The Initialize request is the first message sent from the client and
    // the response reports debugger capabilities.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Initialize
    m_session->registerHandler([&](const dap::InitializeRequest& request) {
        QJsonObject requestJsonObj = getInitializeJsonObj();
        qInfo() << "<- C(initialize-1): \n"
                << qPrintable(QJsonDocument(requestJsonObj).toJson(QJsonDocument::Indented));
        dap::InitializeResponse response{};
        response.supportsConfigurationDoneRequest = true;
        response.supportsFunctionBreakpoints = true;
        response.supportsInstructionBreakpoints = true;
        QJsonObject responseJsonObj = getCapabilitiesJsonObj();
        qInfo() << "-> R(initialize-1): \n"
                << qPrintable(QJsonDocument(responseJsonObj).toJson(QJsonDocument::Indented));

        return response;
    });

    // When the Initialize response has been sent, we need to send the initialized
    // event.
    // We use the registerSentHandler() to ensure the event is sent *after* the
    // initialize response.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Events_Initialized
    m_session->registerSentHandler(
        [&](const dap::ResponseOrError<dap::InitializeResponse>&) {
          m_session->send(dap::InitializedEvent());
          QJsonObject responseJsonObj = getInitializedJsonObj();
          qInfo() << "-> E(initialized): \n"
                  << qPrintable(QJsonDocument(responseJsonObj).toJson(QJsonDocument::Indented));
    });

    // The Pause request instructs the debugger to pause execution of one or all
    // threads.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Pause
    m_session->registerHandler([&](const dap::PauseRequest& request) {
        dap::PauseResponse response;
        QJsonObject requestJsonObj{};
        QJsonObject responseJsonObj{};
        qInfo() << "\n<-- C (pause): \n"
                << qPrintable(QJsonDocument(requestJsonObj).toJson(QJsonDocument::Indented));;
        emit q->ReadyPause();
        qInfo() << "\n--> R (pause): \n"
                << qPrintable(QJsonDocument(responseJsonObj).toJson(QJsonDocument::Indented));

      return response;
    });

    // The Continue request instructs the debugger to resume execution of one or
    // all threads.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Continue
    m_session->registerHandler([&](const dap::ContinueRequest& request) {
      dap::ContinueResponse response{};
      QJsonObject requestJsonObj{};
      qInfo() << "<- C(Continue): \n"
              << qPrintable(QJsonDocument(requestJsonObj).toJson(QJsonDocument::Indented));
      emit q->ReadyContinue();
      QJsonObject responseJsonObj{};
      qInfo() << "-> R(Continue): \n"
              << qPrintable(QJsonDocument(responseJsonObj).toJson(QJsonDocument::Indented));
      return response;
    });

    // The Next request instructs the debugger to single line step for a specific
    // thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Next
    m_session->registerHandler([&](const dap::NextRequest& request) {
      dap::NextResponse response{};
      QJsonObject requestJsonObj{};
      qInfo() << "<- C(Next/StepOver): \n"
              << qPrintable(QJsonDocument(requestJsonObj).toJson(QJsonDocument::Indented));
      emit q->ReadyStepOver();
      QJsonObject responseJsonObj{};
      qInfo() << "-> R(Next/StepOver): \n"
              << qPrintable(QJsonDocument(responseJsonObj).toJson(QJsonDocument::Indented));
      return response;
    });

    // The StepIn request instructs the debugger to step-in for a specific thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StepIn
    m_session->registerHandler([&](const dap::StepInRequest& request) {
      dap::StepInResponse response{};
      QJsonObject requestJsonObj{};
      qInfo() << "<- C(StepIn): \n"
              << qPrintable(QJsonDocument(requestJsonObj).toJson(QJsonDocument::Indented));
      emit q->ReadyStepIn();
      QJsonObject responseJsonObj{};
      qInfo() << "-> R(StepIn): \n"
              << qPrintable(QJsonDocument(responseJsonObj).toJson(QJsonDocument::Indented));
      return response;
    });

    // The StepOut request instructs the debugger to step-out for a specific
    // thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StepOut
    m_session->registerHandler([&](const dap::StepOutRequest& request) {
      dap::StepOutResponse response{};
      QJsonObject requestJsonObj{};
      qInfo() << "<- C(StepOut): \n"
              << qPrintable(QJsonDocument(requestJsonObj).toJson(QJsonDocument::Indented));
      emit q->ReadyStepOut();
      QJsonObject responseJsonObj{};
      qInfo() << "-> R(StepOut): \n"
              << qPrintable(QJsonDocument(responseJsonObj).toJson(QJsonDocument::Indented));
      return response;
    });

    // The SetBreakpoints request instructs the debugger to clear and set a number
    // of line breakpoints for a specific source file.
    // This example debugger only exposes a single source file.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_SetBreakpoints
    m_session->registerHandler([&](const dap::SetBreakpointsRequest& request) {
      dap::SetBreakpointsResponse response;

//      auto breakpoints = request.breakpoints.value({});
//      m_debugger->clearAllBreakpoints();
//      response.breakpoints.resize(breakpoints.size());
//        for (size_t i = 0; i < breakpoints.size(); i++) {
//          m_debugger->addBreakpoint(breakpoints[i].line);
//        }

      auto breakpoints = request.breakpoints.value();
      std::vector<int64_t> lines;
      for (auto breakpoint : breakpoints) {
          auto line = breakpoint.line;
          lines.push_back(line);
      }

      auto sourceName = request.source.name.value();
      auto sourcePath = request.source.path.value();
      std::string source = sourceName.c_str();

      QJsonObject requestJson{

      };
      QJsonObject responseJsonObj{};

      return response;
    });

    // The Launch request is made when the client instructs the debugger adapter
    // to start the debuggee. This request contains the launch arguments.
    // This example debugger does nothing with this request.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Launch
    m_session->registerHandler([=](const dap::LaunchRequest&) {
        QJsonObject requestJsonObj = getLaunchJsonObj();
        qInfo() << "<- C(launchRequest): \n"
                << qPrintable(QJsonDocument(requestJsonObj).toJson(QJsonDocument::Indented));

        QString program = "/usr/bin/gdb";
        QStringList arguments;
        arguments.push_back("-q");
        arguments.push_back("-i=mi2");
        arguments.push_back("/home/zhxiao/workspaces/qtcreator/demo/main");
        QByteArray writeData;
        emit q->ReadyLaunch(program, arguments, writeData); //thread send signal
        //emit q->ReadySetBreakpoints(); // break insert on main
        //emit q->ReadyRun();
        dap::LaunchResponse response{};
        QJsonObject responseJsonObj{};
        qInfo() << "-> R(launchResponse): \n"
                << qPrintable(QJsonDocument(responseJsonObj).toJson(QJsonDocument::Indented));

        return response;
    });

    // Handler for disconnect requests
    m_session->registerHandler([&](const dap::DisconnectRequest& request) {
      if (request.terminateDebuggee.value(false)) {
        terminate.fire();
      }
      return dap::DisconnectResponse();
    });

    // The ConfigurationDone request is made by the client once all configuration
    // requests have been made.
    // This example debugger uses this request to 'start' the debugger.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_ConfigurationDone
    m_session->registerHandler([&](const dap::ConfigurationDoneRequest&) {
      configured.fire();
      auto response = dap::ConfigurationDoneResponse();
      auto responseJsonObj = getConfigurationDoneJsonObj();
      qInfo() << "-> R(configurationDone): "
              << QJsonDocument(responseJsonObj).toJson();
      return response;
    });
}

bool ServerPrivate::start(const char *addr, const int port)
{
    auto onConnect = [&](const std::shared_ptr<dap::ReaderWriter>& rwsocket) {
        registerHandler();
        m_session->bind(rwsocket);
         configured.wait();
        // broadcast the existance of those threads to the client

        terminate.wait();
    };

    auto onError = [&](const char* msg) {
        terminate.fire();
    };

    m_server->start(port, onConnect, onError);
}

bool ServerPrivate::stop()
{
    m_server->stop();
}

std::shared_ptr<CxxDebugger> ServerPrivate::getDebugger()
{
    return m_debugger;
}

std::shared_ptr<dap::net::Server> ServerPrivate::getServer()
{
    return std::move(m_server);
}

std::shared_ptr<dap::Session> ServerPrivate::getSession()
{
    return m_session;
}

Server::Server()
    : d(new ServerPrivate(this))
{
    QObject::connect(this, &Server::ReadyLaunch, this, &Server::onLaunch);
    QObject::connect(this, &Server::ReadyRun, this, &Server::onRun);
    QObject::connect(this, &Server::ReadyContinue, this, &Server::onContinue);
    QObject::connect(this, &Server::ReadyPause, this, &Server::onPause);
    QObject::connect(this, &Server::ReadyStepIn, this, &Server::onStepin);
    QObject::connect(this, &Server::ReadyStepOut, this, &Server::onStepout);
    QObject::connect(this, &Server::ReadyStepOver, this, &Server::onStepover);
    QObject::connect(this, &Server::ReadySetBreakpoints, this, &Server::onSetBreakpoints);
}

Server::~Server()
{
    if (d) {          
        delete d;
    }
}

bool Server::start()
{
    d->start(d->addr, d->port);
    return true;
}

bool Server::stop()
{
    d->stop();
}

std::shared_ptr<dap::net::Server> Server::getServer()
{
    return d->getServer();
}

std::shared_ptr<dap::Session> Server::getSession()
{
    return d->getSession();
}

std::shared_ptr<CxxDebugger> Server::getDebugger()
{
    return d->getDebugger();
}

void Server::onLaunch(const QString &program, const QStringList &arguments, const QByteArray &writeData)
{
    getDebugger()->ReadyLaunch(program, arguments, writeData);
}

void Server::onRun()
{
    getDebugger()->ReadyRun();
}

void Server::onContinue()
{
    getDebugger()->ReadyContinue();
}

void Server::onPause()
{
    getDebugger()->ReadyPause();
}

void Server::onStepin()
{
    getDebugger()->ReadyStepIn();
}

void Server::onStepout()
{
    getDebugger()->ReadyStepOut();
}

void Server::onStepover()
{
    getDebugger()->ReadyStepOver();
}

void Server::onSetBreakpoints()
{
    getDebugger()->ReadySetBreakpoints();
}

} // namespace cxxdbg
