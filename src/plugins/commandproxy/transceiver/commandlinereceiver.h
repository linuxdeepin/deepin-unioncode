// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMANDLINERECEIVER_H
#define COMMANDLINERECEIVER_H

#include "common/project/projectinfo.h"
#include "services/builder/builderglobals.h"

#include <framework/framework.h>

class CommandLineReceiver : public dpf::EventHandler,
        dpf::AutoEventHandlerRegister<CommandLineReceiver>
{
    Q_OBJECT
    friend class dpf::AutoEventHandlerRegister<CommandLineReceiver>;

public:
    explicit CommandLineReceiver(QObject * parent = nullptr);

    static Type type();

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;
};

#endif // COMMANDLINERECEIVER_H
