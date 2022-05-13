#ifndef LANGSERVER_H
#define LANGSERVER_H

#include "route.h"

#include <QTcpServer>

class Frontend;
class LangServer : public QTcpServer
{
    Q_OBJECT
public:
    LangServer(quint16 port);

protected:
    virtual void incomingConnection(qintptr handle);
};

#endif // LANGSERVER_H
