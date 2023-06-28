// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VALGRINDRECEIVER_H
#define VALGRINDRECEIVER_H

#include "services/project/projectinfo.h"
#include "services/builder/builderglobals.h"

#include <framework/framework.h>

class ValgrindReceiver : public dpf::EventHandler,
        dpf::AutoEventHandlerRegister<ValgrindReceiver>
{
    Q_OBJECT
    friend class dpf::AutoEventHandlerRegister<ValgrindReceiver>;

public:
    explicit ValgrindReceiver(QObject * parent = nullptr);

    static Type type();

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;
};

#endif // VALGRINDRECEIVER_H
