/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "debugmanager.h"
#include "dap/debugger.h"
#include "event/eventsender.h"
#include "debuggersignals.h"
#include "debuggerglobals.h"
#include "interface/appoutputpane.h"
#include "interface/stackframemodel.h"
#include "stackframe.h"
#include "interface/stackframeview.h"
#include "breakpoint.h"

#include <QFileInfo>

using namespace DEBUG_NAMESPACE;
DebugManager::DebugManager(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<OutputFormat>("OutputFormat");
    qRegisterMetaType<StackFrameData>("StackFrameData");
    qRegisterMetaType<StackFrames>("StackFrames");

    qRegisterMetaType<IVariable>("IVariable");
    qRegisterMetaType<IVariables>("IVariables");
}

bool DebugManager::initialize()
{
    debugger.reset(new Debugger(this));

    connect(debuggerSignals, &DebuggerSignals::breakpointAdded, this, &DebugManager::slotBreakpointAdded);
    connect(debuggerSignals, &DebuggerSignals::breakpointRemoved, this, &DebugManager::slotBreakpointRemoved);
    connect(debuggerSignals, &DebuggerSignals::addOutput, this, &DebugManager::slotOutput);

    initializeView();

    return true;
}

AppOutputPane *DebugManager::getOutputPane() const
{
    return outputPane.get();
}

QTreeView *DebugManager::getStackPane() const
{
    return stackView.get();
}

QTreeView *DebugManager::getLocalsPane() const
{
    return localsView.get();
}

QTreeView *DebugManager::getBreakpointPane() const
{
    return breakpointView.get();
}

void DebugManager::startDebug()
{
    debugger->startDebug();
}

void DebugManager::detachDebug()
{
    debugger->detachDebug();
}

void DebugManager::interruptDebug()
{
    debugger->interruptDebug();
}

void DebugManager::continueDebug()
{
    debugger->continueDebug();
}

void DebugManager::abortDebug()
{
    debugger->abortDebug();
}

void DebugManager::restartDebug()
{
    debugger->restartDebug();
}

void DebugManager::stepOver()
{
    debugger->stepOver();
}

void DebugManager::stepIn()
{
    debugger->stepIn();
}

void DebugManager::stepOut()
{
    debugger->stepOut();
}

void DebugManager::slotBreakpointAdded(const QString &filepath, int lineNumber)
{
    debugger->addBreakpoint(filepath, lineNumber);
    Internal::Breakpoint bp;
    bp.filePath = filepath;
    bp.fileName = QFileInfo(filepath).fileName();
    bp.lineNumber = lineNumber;
    breakpointModel.insertBreakpoint(bp);
}

void DebugManager::slotBreakpointRemoved(const QString &filepath, int lineNumber)
{
    debugger->removeBreakpoint(filepath, lineNumber);

    Internal::Breakpoint bp;
    bp.filePath = filepath;
    bp.fileName = QFileInfo(filepath).fileName();
    bp.lineNumber = lineNumber;
    breakpointModel.removeBreakpoint(bp);
}

void DebugManager::slotOutput(const QString &content, OutputFormat format)
{
    outputPane->appendText(content, format);
}

void DebugManager::slotProcessFrames(const StackFrames &stackFrames)
{
    stackModel.setFrames(stackFrames);

    auto curFrame = stackModel.currentFrame();
    EventSender::jumpTo(curFrame.file.toStdString(), curFrame.line);

    // update local variables.
    IVariables locals;
    debugger->getLocals(curFrame.frameId, &locals);
    localsModel.setDatas(locals);
}

void DebugManager::slotFrameSelected(const QModelIndex &index)
{
    Q_UNUSED(index);
    auto curFrame = stackModel.currentFrame();
    EventSender::jumpTo(curFrame.file.toStdString(), curFrame.line);

    // update local variables.
    IVariables locals;
    debugger->getLocals(curFrame.frameId, &locals);
    localsModel.setDatas(locals);
}

void DebugManager::slotBreakpointSelected(const QModelIndex &index)
{
    Q_UNUSED(index);
    auto curBP = breakpointModel.currentBreakpoint();
    EventSender::jumpTo(curBP.filePath.toStdString(), curBP.lineNumber);
}

void DebugManager::initializeView()
{
    // initialize output pane.
    outputPane.reset(new AppOutputPane());

    // initialize stack monitor pane.
    stackView.reset(new StackFrameView());
    stackView->setModel(stackModel.model());

    // intialize breakpint pane.
    breakpointView.reset(new StackFrameView());
    breakpointView->setModel(breakpointModel.model());

    localsView.reset(new QTreeView());
    localsView->setModel(&localsModel);
    QStringList headers{"name", "value", "reference"};
    localsModel.setHeaders(headers);

    connect(stackView.get(), &QTreeView::doubleClicked, this, &DebugManager::slotFrameSelected);
    connect(debuggerSignals, &DebuggerSignals::processStackFrames, this, &DebugManager::slotProcessFrames);
    connect(breakpointView.get(), &QTreeView::doubleClicked, this, &DebugManager::slotBreakpointSelected);
}
