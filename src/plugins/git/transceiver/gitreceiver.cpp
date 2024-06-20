// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gitreceiver.h"
#include "client/gitclient.h"
#include "utils/gitmenumanager.h"

#include "common/util/eventdefinitions.h"

#include <QFileInfo>
#include <QMenu>

GitReceiver::GitReceiver(QObject *parent)
    : dpf::EventHandler(parent)
{
    using namespace std::placeholders;
    eventHandleMap.insert(editor.switchedFile.name, std::bind(&GitReceiver::handleSwitchedFileEvent, this, _1));
}

dpf::EventHandler::Type GitReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList GitReceiver::topics()
{
    return { editor.topic };
}

void GitReceiver::eventProcess(const dpf::Event &event)
{
    const auto &eventName = event.data().toString();
    if (!eventHandleMap.contains(eventName))
        return;

    eventHandleMap[eventName](event);
}

void GitReceiver::handleCursorPositionChangedEvent(const dpf::Event &event)
{
    const auto &fileName = event.property("fileName").toString();
    int line = event.property("line").toInt() + 1;

    QFileInfo info(fileName);
    GitClient::instance()->instantBlame(info.absolutePath(), fileName, line);
}

void GitReceiver::handleSwitchedFileEvent(const dpf::Event &event)
{
    const auto &fileName = event.property("fileName").toString();
    GitMenuManager::instance()->setCurrentFile(fileName);

    bool ret = GitClient::instance()->setupInstantBlame(fileName);
    if (ret) {
        if (!eventHandleMap.contains(editor.cursorPositionChanged.name))
            eventHandleMap.insert(editor.cursorPositionChanged.name, std::bind(&GitReceiver::handleCursorPositionChangedEvent, this, std::placeholders::_1));
    } else if (eventHandleMap.contains(editor.cursorPositionChanged.name)) {
        eventHandleMap.remove(editor.cursorPositionChanged.name);
    }
}
