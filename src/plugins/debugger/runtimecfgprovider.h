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

    dap::InitializeRequest initalizeRequest();

signals:

public slots:

private:
};

#endif // RUNTIMECFGPROVIDER_H
