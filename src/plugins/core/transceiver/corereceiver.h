// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CORERECEIVER_H
#define CORERECEIVER_H

#include <framework/framework.h>

#include <QMainWindow>

//框架实例化
class CoreReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<CoreReceiver>
{
    Q_OBJECT
public:
    explicit CoreReceiver(QObject *parent = nullptr);
    static Type type();
    static QStringList topics();
    virtual void eventProcess(const dpf::Event& event);
};
#endif // CORERECEIVER_H
