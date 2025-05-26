// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
