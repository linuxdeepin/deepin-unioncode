// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTRECEIVER_H
#define EVENTRECEIVER_H

#include <framework/framework.h>

class UTEventReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<UTEventReceiver>
{
    friend class dpf::AutoEventHandlerRegister<UTEventReceiver>;

public:
    explicit UTEventReceiver(QObject *parent = nullptr);
    static Type type();
    static QStringList topics();
    void eventProcess(const dpf::Event &event) override;

private:
    void processLLMCountChanged(const dpf::Event &event);

private:
    QHash<QString, std::function<void(const dpf::Event &)>> eventHandleMap;
};

class EventDistributeProxy : public QObject
{
    Q_OBJECT
public:
    static EventDistributeProxy *instance();

Q_SIGNALS:
    void sigLLMCountChanged();
};

#endif   // EVENTRECEIVER_H
