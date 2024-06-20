// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codeeditor.h"
#include "codelens/codelens.h"
#include "mainframe/texteditkeeper.h"
#include "transceiver/codeeditorreceiver.h"
#include "gui/workspacewidget.h"
#include "lexer/lexermanager.h"

#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractwidget.h"

#include "services/window/windowservice.h"
#include "services/language/languageservice.h"
#include "services/editor/editorservice.h"
#include "common/widget/outputpane.h"

#include <DButtonBox>

#include <QAction>
#include <QSplitter>

using namespace dpfservice;
using namespace std::placeholders;
using DTK_WIDGET_NAMESPACE::DButtonBox;

const QString SAVE_ALL_DOCUMENTS = CodeEditor::tr("Save All Documents");
const QString CLOSE_ALL_DOCUMENTS = CodeEditor::tr("Close All Documents");
const QString PRINT = CodeEditor::tr("Print");

void CodeEditor::initialize()
{
    qInfo() << __FUNCTION__;

    QString errStr;
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(dpfservice::LanguageService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
}

bool CodeEditor::start()
{
    qInfo() << __FUNCTION__;

    initWindowService();
    initActions();
    initButtonBox();
    initEditorService();

    return true;
}

dpf::Plugin::ShutdownFlag CodeEditor::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}

void CodeEditor::initButtonBox()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService)
        return;

    DButtonBox *btnBox = new DButtonBox(workspaceWidget);

    DButtonBoxButton *backBtn = new DButtonBoxButton(QIcon::fromTheme("go-previous"), "", btnBox);
    backBtn->setToolTip(tr("backward"));
    connect(backBtn, &DButtonBoxButton::clicked, [=]() {
        editor.back();
    });

    DButtonBoxButton *forwardBtn = new DButtonBoxButton(QIcon::fromTheme("go-next"), "", btnBox);
    forwardBtn->setToolTip(tr("forward"));
    connect(forwardBtn, &DButtonBoxButton::clicked, [=]() {
        editor.forward();
    });

    btnBox->setButtonList({ backBtn, forwardBtn }, false);
    windowService->addWidgetToTopTool(new AbstractWidget(btnBox), "", false, false);
}

void CodeEditor::initActions()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService)
        return;

    QAction *backAction = new QAction(tr("backward"), this);
    QAction *forwardAction = new QAction(tr("forward"), this);
    QAction *closeAction = new QAction(tr("Close"), this);
    QAction *switchHeaderSourceAction = new QAction(tr("Switch Header/Source"), this);
    QAction *follSymbolAction = new QAction(tr("Follow Symbol Under Cursor"), this);
    QAction *toggleBreakpointAction = new QAction(tr("Toggle Breakpoint"), this);

    auto inputBackAction = new AbstractAction(backAction);
    inputBackAction->setShortCutInfo("Editor.back", tr("Backward"), QKeySequence(Qt::Modifier::ALT | Qt::Key_Left));
    auto inputForwardAction = new AbstractAction(forwardAction);
    inputForwardAction->setShortCutInfo("Editor.forward",
                                        tr("Forward"), QKeySequence(Qt::Modifier::ALT | Qt::Key_Right));
    auto inputCloseAction = new AbstractAction(closeAction);
    inputCloseAction->setShortCutInfo("Editor.close",
                                      tr("Close"), QKeySequence(Qt::Modifier::CTRL | Qt::Key_W));
    auto inputswitchHeaderSourceAction = new AbstractAction(switchHeaderSourceAction);
    inputswitchHeaderSourceAction->setShortCutInfo("Editor.switchHS",
                                                   tr("Switch Header/Source"), QKeySequence(Qt::Key_F4));
    auto inputfollSymbolAction = new AbstractAction(follSymbolAction);
    inputfollSymbolAction->setShortCutInfo("Editor.followSymbol",
                                           tr("Follow Symbol Under Cursor"), QKeySequence(Qt::Key_F2));
    auto inputtoggleBreakpointAction = new AbstractAction(toggleBreakpointAction);
    inputtoggleBreakpointAction->setShortCutInfo("Editor.toggleBreak",
                                                 tr("Toggle Breakpoint"), QKeySequence(Qt::Key_F9));

    windowService->addAction(tr("&Edit"), inputBackAction);
    windowService->addAction(tr("&Edit"), inputForwardAction);
    windowService->addAction(tr("&Edit"), inputCloseAction);
    windowService->addAction(tr("&Edit"), inputswitchHeaderSourceAction);
    windowService->addAction(tr("&Edit"), inputfollSymbolAction);
    windowService->addAction(tr("&Edit"), inputtoggleBreakpointAction);

    connect(backAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqBack);
    connect(forwardAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqForward);
    connect(closeAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqCloseCurrentEditor);
    connect(switchHeaderSourceAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqSwitchHeaderSource);
    connect(follSymbolAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqFollowSymbolUnderCursor);
    connect(toggleBreakpointAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqToggleBreakpoint);
}

void CodeEditor::initEditorService()
{
    auto &ctx = dpfInstance.serviceContext();
    QString errStr;
    if (!ctx.load(dpfservice::EditorService::name(), &errStr)) {
        qCritical() << errStr;
    }
    EditorService *editorService = dpfGetService(EditorService);
    if (!editorService)
        return;

    editorService->getSelectedText = std::bind(&WorkspaceWidget::selectedText, workspaceWidget);
    editorService->getCursorBeforeText = std::bind(&WorkspaceWidget::cursorBeforeText, workspaceWidget);
    editorService->getCursorBehindText = std::bind(&WorkspaceWidget::cursorBehindText, workspaceWidget);
    editorService->replaceSelectedText = std::bind(&WorkspaceWidget::replaceSelectedText, workspaceWidget, _1);
    editorService->showTips = std::bind(&WorkspaceWidget::showTips, workspaceWidget, _1);
    editorService->insertText = std::bind(&WorkspaceWidget::insertText, workspaceWidget, _1);
    editorService->undo = std::bind(&WorkspaceWidget::undo, workspaceWidget);
    editorService->modifiedFiles = std::bind(&WorkspaceWidget::modifiedFiles, workspaceWidget);
    editorService->saveAll = std::bind(&WorkspaceWidget::saveAll, workspaceWidget);

    LexerManager::instance()->init(editorService);
}

void CodeEditor::initWindowService()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    workspaceWidget = new WorkspaceWidget;
    using namespace std::placeholders;
    if (windowService) {
        QAction *action = new QAction(MWNA_EDIT, this);
        action->setIcon(QIcon::fromTheme("edit-navigation"));
        windowService->addNavigationItem(new AbstractAction(action), 5);

        windowService->registerWidgetToMode("editWindow", new AbstractWidget(workspaceWidget), CM_EDIT, Position::Central, true, true);
        windowService->registerWidgetToMode("editWindow", new AbstractWidget(workspaceWidget), CM_DEBUG, Position::Central, true, true);

        auto sep = new QAction(this);
        sep->setSeparator(true);
        windowService->addAction(MWM_FILE, new AbstractAction(sep));

        windowService->addContextWidget(QTabWidget::tr("Search &Results"), new AbstractWidget(CodeLens::instance()), true);
        windowService->addContextWidget(tr("&Application Output"), new AbstractWidget(OutputPane::instance()), true);
    }
}
