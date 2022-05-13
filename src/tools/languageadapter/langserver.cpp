#include "langserver.h"
#include "setting.h"
#include "route.h"
#include "handlerfrontend.h"

#include <QTcpSocket>
#include <QRunnable>
#include <QtConcurrent>

#include <iostream>

LangServer::LangServer(quint16 port)
{
    if (!listen(QHostAddress::Any, port)) {
        std::cerr << "Failed, listen " << errorString().toUtf8().toStdString()
                  << std::endl;
        abort();
    }
}

void LangServer::incomingConnection(qintptr handle)
{
    auto tcpSocket = new QTcpSocket();
    tcpSocket->setSocketDescriptor(handle);
    tcpSocket->setReadBufferSize(8192);
    auto frontend = new HandlerFrontend;
    frontend->bind(tcpSocket);
}

