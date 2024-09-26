// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
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

    void processContextMenuEvent(const dpf::Event &event);
    void processTextChangedEvent(const dpf::Event &event);
    void processSelectionChangedEvent(const dpf::Event &event);
    void processPositionChangedEvent(const dpf::Event &event);
    void processActionInvokedEvent(const dpf::Event &event);
    void processOpenProjectEvent(const dpf::Event &event);

private:
    QHash<QString, std::function<void(const dpf::Event &)>> eventHandleMap;
};

#endif   // EVENTRECEIVER_H
