// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUILDERRECEIVER_H
#define BUILDERRECEIVER_H

#include <framework/framework.h>

class BuilderReceiver : public dpf::EventHandler
        , dpf::AutoEventHandlerRegister<BuilderReceiver>
{
    Q_OBJECT
public:
    explicit BuilderReceiver(QObject *parent = nullptr);
    static Type type();
    static QStringList topics();
    virtual void eventProcess(const dpf::Event& event) override;
};

#endif // BUILDERRECEIVER_H
