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

    auto inputBackAction = new AbstractAction(backAction);
    inputBackAction->setShortCutInfo("Editor.back", tr("Back"), QKeySequence(Qt::Modifier::ALT | Qt::Key_Left));
    auto inputForwardAction = new AbstractAction(forwardAction);
    inputForwardAction->setShortCutInfo("Editor.forward",
                                        tr("Forward"), QKeySequence(Qt::Modifier::ALT | Qt::Key_Right));

    windowService->addAction(tr("&Edit"), inputBackAction);
    windowService->addAction(tr("&Edit"), inputForwardAction);

    connect(backAction, &QAction::triggered, [=] {
        editor.back();
    });

    connect(forwardAction, &QAction::triggered, [=] {
        editor.forward();
    });
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

        windowService->addContextWidget(QTabWidget::tr("Code &Lens"), new AbstractWidget(CodeLens::instance()), true);
        windowService->addContextWidget(tr("&Application Output"), new AbstractWidget(OutputPane::instance()), true);
    }
}
