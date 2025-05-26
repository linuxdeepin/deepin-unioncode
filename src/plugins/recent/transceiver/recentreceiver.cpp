// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentreceiver.h"
#include "common/common.h"
#include "mainframe/recentdisplaywidget.h"

RecentReceiver::RecentReceiver(QObject *parent)
    : dpf::EventHandler(parent), dpf::AutoEventHandlerRegister<RecentReceiver>()
{
    using namespace std::placeholders;
    eventHandleMap.insert(recent.saveOpenedProject.name, std::bind(&RecentReceiver::processSaveOpenedProjectEvent, this, _1));
    eventHandleMap.insert(recent.saveOpenedFile.name, std::bind(&RecentReceiver::processSaveOpenedFileEvent, this, _1));
    eventHandleMap.insert(session.sessionCreated.name, std::bind(&RecentReceiver::processSessionCreatedEvent, this, _1));
    eventHandleMap.insert(session.sessionRemoved.name, std::bind(&RecentReceiver::processSessionRemovedEvent, this, _1));
    eventHandleMap.insert(session.sessionLoaded.name, std::bind(&RecentReceiver::processSessionLoadedEvent, this, _1));
    eventHandleMap.insert(session.sessionStatusChanged.name, std::bind(&RecentReceiver::processSessionLoadedEvent, this, _1));
}

dpf::EventHandler::Type RecentReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList RecentReceiver::topics()
{
    return { recent.topic, session.topic };
}

void RecentReceiver::eventProcess(const dpf::Event &event)
{
    const auto &eventName = event.data().toString();
    if (!eventHandleMap.contains(eventName))
        return;

    eventHandleMap[eventName](event);
}

void RecentReceiver::processSaveOpenedProjectEvent(const dpf::Event &event)
{
    QString kitName = event.property("kitName").toString();
    QString language = event.property("language").toString();
    QString workspace = event.property("workspace").toString();
    if (QDir(workspace).exists())
        Q_EMIT RecentProxy::instance()->saveOpenedProject(kitName, language, workspace);
}

void RecentReceiver::processSaveOpenedFileEvent(const dpf::Event &event)
{
    QString filePath = event.property("filePath").toString();
    if (QFileInfo(filePath).exists())
        Q_EMIT RecentProxy::instance()->saveOpenedFile(filePath);
}

void RecentReceiver::processSessionCreatedEvent(const dpf::Event &event)
{
    const QString session = event.property("session").toString();
    RecentDisplayWidget::instance()->addSession(session);
}

void RecentReceiver::processSessionRemovedEvent(const dpf::Event &event)
{
    const QString session = event.property("session").toString();
    RecentDisplayWidget::instance()->removeSession(session);
}

void RecentReceiver::processSessionLoadedEvent(const dpf::Event &event)
{
    RecentDisplayWidget::instance()->updateSessions();
}

RecentProxy *RecentProxy::instance()
{
    static RecentProxy ins;
    return &ins;
}
