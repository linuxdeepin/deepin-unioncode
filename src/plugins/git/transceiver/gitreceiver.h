// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GITRECEIVER_H
#define GITRECEIVER_H

#include <framework/framework.h>

class GitReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<GitReceiver>
{
    Q_OBJECT
public:
    explicit GitReceiver(QObject *parent = nullptr);
    static Type type();
    static QStringList topics();
    virtual void eventProcess(const dpf::Event &event);

private:
    void handleCursorPositionChangedEvent(const dpf::Event &event);
    void handleContextMenuEvent(const dpf::Event &event);
    void handleSwitchedFileEvent(const dpf::Event &event);
    void handleProjectChangedEvent(const dpf::Event &event);

private:
    QHash<QString, std::function<void(const dpf::Event &)>> eventHandleMap;
};

#endif   // GITRECEIVER_H
