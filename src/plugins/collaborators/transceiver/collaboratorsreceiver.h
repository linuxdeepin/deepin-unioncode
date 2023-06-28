// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLABORATORSRECEIVER_H
#define COLLABORATORSRECEIVER_H

#include <framework/framework.h>

class CollaboratorsReceiver: public dpf::EventHandler, dpf::AutoEventHandlerRegister<CollaboratorsReceiver>
{
    friend class dpf::AutoEventHandlerRegister<CollaboratorsReceiver>;
public:
    explicit CollaboratorsReceiver(QObject * parent = nullptr);

    static Type type();

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;
};

#endif // COLLABORATORSRECEIVER_H
