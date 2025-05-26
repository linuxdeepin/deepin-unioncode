// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
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
    eventHandleMap.insert(editor.contextMenu.name, std::bind(&GitReceiver::handleContextMenuEvent, this, _1));
    eventHandleMap.insert(project.activatedProject.name, std::bind(&GitReceiver::handleProjectChangedEvent, this, _1));
    eventHandleMap.insert(project.deletedProject.name, std::bind(&GitReceiver::handleProjectChangedEvent, this, _1));
}

dpf::EventHandler::Type GitReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList GitReceiver::topics()
{
    return { editor.topic, project.topic };
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

void GitReceiver::handleContextMenuEvent(const dpf::Event &event)
{
    auto editorMenu = event.property("menu").value<QMenu *>();
    if (editorMenu)
        editorMenu->addAction(GitMenuManager::instance()->gitAction());
}

void GitReceiver::handleSwitchedFileEvent(const dpf::Event &event)
{
    const auto &fileName = event.property("fileName").toString();
    GitMenuManager::instance()->setupFileMenu(fileName);

    bool ret = GitClient::instance()->setupInstantBlame(fileName);
    if (ret) {
        if (!eventHandleMap.contains(editor.cursorPositionChanged.name))
            eventHandleMap.insert(editor.cursorPositionChanged.name, std::bind(&GitReceiver::handleCursorPositionChangedEvent, this, std::placeholders::_1));
    } else if (eventHandleMap.contains(editor.cursorPositionChanged.name)) {
        eventHandleMap.remove(editor.cursorPositionChanged.name);
        GitClient::instance()->clearInstantBlame();
    }
}

void GitReceiver::handleProjectChangedEvent(const dpf::Event &event)
{
    GitMenuManager::instance()->setupProjectMenu();
}
