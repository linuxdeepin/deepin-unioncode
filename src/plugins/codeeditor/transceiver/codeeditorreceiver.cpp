// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codeeditorreceiver.h"

#include "services/window/windowelement.h"

#include "mainframe/texteditkeeper.h"

CodeEditorReceiver::CodeEditorReceiver(QObject *parent)
    : dpf::EventHandler(parent), dpf::AutoEventHandlerRegister<CodeEditorReceiver>()
{
    using namespace std::placeholders;
    eventHandleMap.insert(editor.openFile.name, std::bind(&CodeEditorReceiver::processOpenFileEvent, this, _1));
    eventHandleMap.insert(editor.closeFile.name, std::bind(&CodeEditorReceiver::processCloseFileEvent, this, _1));
    eventHandleMap.insert(editor.back.name, std::bind(&CodeEditorReceiver::processBackEvent, this, _1));
    eventHandleMap.insert(editor.forward.name, std::bind(&CodeEditorReceiver::processForwardEvent, this, _1));
    eventHandleMap.insert(editor.gotoLine.name, std::bind(&CodeEditorReceiver::processGotoLineEvent, this, _1));
    eventHandleMap.insert(editor.gotoPosition.name, std::bind(&CodeEditorReceiver::processGotoPositionEvent, this, _1));
    eventHandleMap.insert(editor.setDebugLine.name, std::bind(&CodeEditorReceiver::processSetDebugLineEvent, this, _1));
    eventHandleMap.insert(editor.removeDebugLine.name, std::bind(&CodeEditorReceiver::processRemoveDebugLineEvent, this, _1));
    eventHandleMap.insert(editor.addBreakpoint.name, std::bind(&CodeEditorReceiver::processAddBreakpointEvent, this, _1));
    eventHandleMap.insert(editor.removeBreakpoint.name, std::bind(&CodeEditorReceiver::processRemoveBreakpointEvent, this, _1));
    eventHandleMap.insert(editor.setBreakpointEnabled.name, std::bind(&CodeEditorReceiver::processSetBreakpointEnabledEvent, this, _1));
    eventHandleMap.insert(editor.clearAllBreakpoint.name, std::bind(&CodeEditorReceiver::processClearAllBreakpointsEvent, this, _1));
    eventHandleMap.insert(editor.setModifiedAutoReload.name, std::bind(&CodeEditorReceiver::processSetModifiedAutoReloadEvent, this, _1));
}

dpf::EventHandler::Type CodeEditorReceiver::type()
{
    return dpf::EventHandler::Type::Async;
}

QStringList CodeEditorReceiver::topics()
{
    return { editor.topic };
}

void CodeEditorReceiver::eventProcess(const dpf::Event &event)
{
    const auto &eventName = event.data().toString();
    if (!eventHandleMap.contains(eventName))
        return;

    eventHandleMap[eventName](event);
}

void CodeEditorReceiver::processOpenFileEvent(const dpf::Event &event)
{
    uiController.doSwitch(dpfservice::MWNA_EDIT);
    QString workspace = event.property("workspace").toString();
    QString fileName = event.property("fileName").toString();
    Q_EMIT EditorCallProxy::instance()->reqOpenFile(workspace, fileName);
}

void CodeEditorReceiver::processCloseFileEvent(const dpf::Event &event)
{
    QString fileName = event.property("fileName").toString();
    Q_EMIT EditorCallProxy::instance()->reqCloseFile(fileName);
}

void CodeEditorReceiver::processBackEvent(const dpf::Event &event)
{
    Q_UNUSED(event)

    Q_EMIT EditorCallProxy::instance()->reqBack();
}

void CodeEditorReceiver::processForwardEvent(const dpf::Event &event)
{
    Q_UNUSED(event)

    Q_EMIT EditorCallProxy::instance()->reqForward();
}

void CodeEditorReceiver::processGotoLineEvent(const dpf::Event &event)
{
    QString filePath = event.property("fileName").toString();
    int line = event.property("line").toInt();
    Q_EMIT EditorCallProxy::instance()->reqGotoLine(filePath, line);
}

void CodeEditorReceiver::processGotoPositionEvent(const dpf::Event &event)
{
    QString filePath = event.property("fileName").toString();
    int line = event.property("line").toInt();
    int col = event.property("column").toInt();
    Q_EMIT EditorCallProxy::instance()->reqGotoPosition(filePath, line, col);
}

void CodeEditorReceiver::processSetModifiedAutoReloadEvent(const dpf::Event &event)
{
    QString filePath = event.property("fileName").toString();
    bool flag = event.property("flag").toBool();
    Q_EMIT EditorCallProxy::instance()->reqSetModifiedAutoReload(filePath, flag);
}

void CodeEditorReceiver::processAddBreakpointEvent(const dpf::Event &event)
{
    QString filePath = event.property("fileName").toString();
    int line = event.property("line").toInt();
    bool enabled = event.property("enabled").toBool();
    Q_EMIT EditorCallProxy::instance()->reqAddBreakpoint(filePath, line, enabled);
}

void CodeEditorReceiver::processRemoveBreakpointEvent(const dpf::Event &event)
{
    QString filePath = event.property("fileName").toString();
    int line = event.property("line").toInt();
    Q_EMIT EditorCallProxy::instance()->reqRemoveBreakpoint(filePath, line);
}

void CodeEditorReceiver::processSetBreakpointEnabledEvent(const dpf::Event &event)
{
    QString filePath = event.property("fileName").toString();
    int line = event.property("line").toInt();
    bool enabled = event.property("enabled").toBool();
    Q_EMIT EditorCallProxy::instance()->reqSetBreakpointEnabled(filePath, line, enabled);
}

void CodeEditorReceiver::processClearAllBreakpointsEvent(const dpf::Event &event)
{
    Q_UNUSED(event);

    Q_EMIT EditorCallProxy::instance()->reqClearAllBreakpoints();
}

void CodeEditorReceiver::processSetDebugLineEvent(const dpf::Event &event)
{
    QString filePath = event.property("fileName").toString();
    int line = event.property("line").toInt();
    Q_EMIT EditorCallProxy::instance()->reqSetDebugLine(filePath, line);
}

void CodeEditorReceiver::processRemoveDebugLineEvent(const dpf::Event &event)
{
    Q_UNUSED(event)

    Q_EMIT EditorCallProxy::instance()->reqRemoveDebugLine();
}

EditorCallProxy::EditorCallProxy()
{
}

EditorCallProxy *EditorCallProxy::instance()
{
    static EditorCallProxy proxy;
    return &proxy;
}
