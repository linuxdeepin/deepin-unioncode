// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTRECEIVER_H
#define EVENTRECEIVER_H

#include <framework/framework.h>
#include <QObject>

/**
 * @brief The EventReceiverDemo class
 * It will auto registered, and receive subscribed topics events.
 */
class EventReceiverDemo : public dpf::EventHandler, dpf::AutoEventHandlerRegister<EventReceiverDemo>
{
    friend class dpf::AutoEventHandlerRegister<EventReceiverDemo>;

public:
    explicit EventReceiverDemo(QObject *parent = nullptr);
    static Type type();
    static QStringList topics();

private:
    virtual void eventProcess(const dpf::Event &event) override;
};

#endif   // EVENTRECEIVER_H
