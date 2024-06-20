// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTRECEIVER_H
#define PROJECTRECEIVER_H

#include <framework/framework.h>

class ActionAnalyseReceiver : public dpf::EventHandler,
        dpf::AutoEventHandlerRegister<ActionAnalyseReceiver>
{
    Q_OBJECT
    friend class dpf::AutoEventHandlerRegister<ActionAnalyseReceiver>;
public:
    explicit ActionAnalyseReceiver(QObject * parent = nullptr);

    static Type type();

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;
};

#endif // PROJECTRECEIVER_H
