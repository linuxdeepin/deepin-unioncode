#ifndef RUNTIMECFGPROVIDER_H
#define RUNTIMECFGPROVIDER_H

#include "dap/types.h"
#include "dap/protocol.h"

#include <QObject>

class RunTimeCfgProvider : public QObject
{
    Q_OBJECT
public:
    explicit RunTimeCfgProvider(QObject *parent = nullptr);

    const char *ip() const;
    int port() const;

    dap::string launchRequest(const QString &target);
    dap::InitializeRequest initalizeRequest();

signals:

public slots:
};

#endif // RUNTIMECFGPROVIDER_H
