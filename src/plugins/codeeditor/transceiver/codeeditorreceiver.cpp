// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codeeditorreceiver.h"

#include "common/common.h"
#include "services/project/projectservice.h"
#include "services/window/windowelement.h"

#include "mainframe/texteditkeeper.h"

CodeEditorReceiver::CodeEditorReceiver(QObject *parent)
    : dpf::EventHandler(parent), dpf::AutoEventHandlerRegister<CodeEditorReceiver>()
{
    using namespace std::placeholders;
    eventHandleMap.insert(editor.openFile.name, std::bind(&CodeEditorReceiver::processOpenFileEvent, this, _1));
    eventHandleMap.insert(editor.back.name, std::bind(&CodeEditorReceiver::processBackEvent, this, _1));
    eventHandleMap.insert(editor.forward.name, std::bind(&CodeEditorReceiver::processForwardEvent, this, _1));
    eventHandleMap.insert(editor.gotoLine.name, std::bind(&CodeEditorReceiver::processGotoLineEvent, this, _1));
    eventHandleMap.insert(editor.setAnnotation.name, std::bind(&CodeEditorReceiver::processSetAnnotationEvent, this, _1));
    eventHandleMap.insert(editor.resetAnnotation.name, std::bind(&CodeEditorReceiver::processResetAnnotationEvent, this, _1));
    eventHandleMap.insert(editor.setDebugLine.name, std::bind(&CodeEditorReceiver::processSetDebugLineEvent, this, _1));
    eventHandleMap.insert(editor.removeDebugLine.name, std::bind(&CodeEditorReceiver::processRemoveDebugLineEvent, this, _1));
    eventHandleMap.insert(editor.setLineBackgroundColor.name, std::bind(&CodeEditorReceiver::processSetLineBackgroundColorEvent, this, _1));
    eventHandleMap.insert(editor.resetLineBackgroundColor.name, std::bind(&CodeEditorReceiver::processResetLineBackgroundEvent, this, _1));
    eventHandleMap.insert(editor.clearLineBackgroundColor.name, std::bind(&CodeEditorReceiver::processClearLineBackgroundEvent, this, _1));
    eventHandleMap.insert(editor.addBreakpoint.name, std::bind(&CodeEditorReceiver::processAddBreakpointEvent, this, _1));
    eventHandleMap.insert(editor.removeBreakpoint.name, std::bind(&CodeEditorReceiver::processRemoveBreakpointEvent, this, _1));
    eventHandleMap.insert(editor.clearAllBreakpoint.name, std::bind(&CodeEditorReceiver::processClearAllBreakpointsEvent, this, _1));
    eventHandleMap.insert(editor.searchText.name, std::bind(&CodeEditorReceiver::processSearchEvent, this, _1));
    eventHandleMap.insert(editor.replaceText.name, std::bind(&CodeEditorReceiver::processReplaceEvent, this, _1));
}

dpf::EventHandler::Type CodeEditorReceiver::type()
{
    return dpf::EventHandler::Type::Async;
}

QStringList CodeEditorReceiver::topics()
{
    return { editor.topic, actionanalyse.topic, project.topic };
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
    QString workspace = event.property(editor.openFile.pKeys[0]).toString();
    QString fileName = event.property(editor.openFile.pKeys[1]).toString();
    EditorCallProxy::instance()->reqOpenFile(workspace, fileName);
}

void CodeEditorReceiver::processSearchEvent(const dpf::Event &event)
{
    QString srcText = event.property(editor.searchText.pKeys[0]).toString();
    int findType = event.property(editor.searchText.pKeys[1]).toInt();
    EditorCallProxy::instance()->reqSearch(srcText, findType);
}

void CodeEditorReceiver::processReplaceEvent(const dpf::Event &event)
{
    QString srcText = event.property(editor.replaceText.pKeys[0]).toString();
    QString targetText = event.property(editor.replaceText.pKeys[1]).toString();
    int replaceType = event.property(editor.replaceText.pKeys[2]).toInt();
    EditorCallProxy::instance()->reqReplace(srcText, targetText, replaceType);
}

void CodeEditorReceiver::processBackEvent(const dpf::Event &event)
{
    Q_UNUSED(event)

    EditorCallProxy::instance()->reqBack();
}

void CodeEditorReceiver::processForwardEvent(const dpf::Event &event)
{
    Q_UNUSED(event)

    EditorCallProxy::instance()->reqForward();
}

void CodeEditorReceiver::processGotoLineEvent(const dpf::Event &event)
{
    QString filePath = event.property(editor.gotoLine.pKeys[0]).toString();
    int line = event.property(editor.gotoLine.pKeys[1]).toInt() - 1;
    EditorCallProxy::instance()->reqGotoLine(filePath, line);
}

void CodeEditorReceiver::processSetLineBackgroundColorEvent(const dpf::Event &event)
{
    QString filePath = event.property(editor.setLineBackgroundColor.pKeys[0]).toString();
    int line = event.property(editor.setLineBackgroundColor.pKeys[1]).toInt() -1;
    QColor color = qvariant_cast<QColor>(event.property(editor.setLineBackgroundColor.pKeys[2]));
    EditorCallProxy::instance()->reqSetLineBackgroundColor(filePath, line, color);
}

void CodeEditorReceiver::processResetLineBackgroundEvent(const dpf::Event &event)
{
    QString filePath = event.property(editor.resetLineBackgroundColor.pKeys[0]).toString();
    int line = event.property(editor.resetLineBackgroundColor.pKeys[1]).toInt() - 1;
    EditorCallProxy::instance()->reqResetLineBackground(filePath, line);
}

void CodeEditorReceiver::processClearLineBackgroundEvent(const dpf::Event &event)
{
    QString filePath = event.property(editor.clearLineBackgroundColor.pKeys[0]).toString();
    EditorCallProxy::instance()->reqClearLineBackground(filePath);
}

void CodeEditorReceiver::processSetAnnotationEvent(const dpf::Event &event)
{
    QString filePath = event.property(editor.setAnnotation.pKeys[0]).toString();
    int line = event.property(editor.setAnnotation.pKeys[1]).toInt() - 1;
    QString title = event.property(editor.setAnnotation.pKeys[2]).toString();
    AnnotationInfo annInfo = qvariant_cast<AnnotationInfo>(event.property(editor.setAnnotation.pKeys[3]));
    EditorCallProxy::instance()->reqSetAnnotation(filePath, line, title, annInfo);
}

void CodeEditorReceiver::processResetAnnotationEvent(const dpf::Event &event)
{
    QString filePath = event.property(editor.resetAnnotation.pKeys[0]).toString();
    QString title = event.property(editor.resetAnnotation.pKeys[1]).toString();
    EditorCallProxy::instance()->reqResetAnnotation(filePath, title);
}

void CodeEditorReceiver::processClearAllAnnotationEvent(const dpf::Event &event)
{
    QString title = event.property(editor.resetAnnotation.pKeys[0]).toString();
    EditorCallProxy::instance()->reqClearAllAnnotation(title);
}

void CodeEditorReceiver::processAddBreakpointEvent(const dpf::Event &event)
{
    QString filePath = event.property(editor.addBreakpoint.pKeys[0]).toString();
    int line = event.property(editor.addBreakpoint.pKeys[1]).toInt() - 1;
    EditorCallProxy::instance()->reqAddBreakpoint(filePath, line);
}

void CodeEditorReceiver::processRemoveBreakpointEvent(const dpf::Event &event)
{
    QString filePath = event.property(editor.removeBreakpoint.pKeys[0]).toString();
    int line = event.property(editor.removeBreakpoint.pKeys[1]).toInt() - 1;
    EditorCallProxy::instance()->reqRemoveBreakpoint(filePath, line);
}

void CodeEditorReceiver::processClearAllBreakpointsEvent(const dpf::Event &event)
{
    Q_UNUSED(event);

    EditorCallProxy::instance()->reqClearAllBreakpoints();
}

void CodeEditorReceiver::processSetDebugLineEvent(const dpf::Event &event)
{
    QString filePath = event.property(editor.setDebugLine.pKeys[0]).toString();
    int line = event.property(editor.setDebugLine.pKeys[1]).toInt() - 1;
    EditorCallProxy::instance()->reqSetDebugLine(filePath, line);
}

void CodeEditorReceiver::processRemoveDebugLineEvent(const dpf::Event &event)
{
    Q_UNUSED(event)

    EditorCallProxy::instance()->reqRemoveDebugLine();
}

EditorCallProxy::EditorCallProxy()
{
}

EditorCallProxy *EditorCallProxy::instance()
{
    static EditorCallProxy proxy;
    return &proxy;
}
