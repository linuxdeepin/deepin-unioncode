// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTRECEIVER_H
#define EVENTRECEIVER_H

#include <framework/framework.h>

class EventReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<EventReceiver>
{
    Q_OBJECT
public:
    explicit EventReceiver(QObject *parent = nullptr);
    static Type type();
    static QStringList topics();
    virtual void eventProcess(const dpf::Event &event);

private:
    void handleNotifyEvent(const dpf::Event &event);
    void handleFileSavedEvent(const dpf::Event &event);

    QHash<QString, std::function<void(const dpf::Event &)>> eventHandleMap;
};

#endif // EVENTRECEIVER_H
