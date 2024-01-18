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

#include <QAction>
#include <QSplitter>

using namespace dpfservice;

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

    initActions();
    initWindowService();
    initEditorService();

    return true;
}

dpf::Plugin::ShutdownFlag CodeEditor::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}

void CodeEditor::initActions()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService)
        return;

    QAction *backAction = new QAction(this);
    QAction *forwardAction = new QAction(this);
    ActionManager::getInstance()->registerAction(backAction, "Editor.back",
                                                 tr("Back"), QKeySequence(Qt::Modifier::ALT | Qt::Key_Left));
    ActionManager::getInstance()->registerAction(forwardAction, "Editor.forward",
                                                 tr("Forward"), QKeySequence(Qt::Modifier::ALT | Qt::Key_Right));

    windowService->addAction(tr("&Edit"), new AbstractAction(backAction));
    windowService->addAction(tr("&Edit"), new AbstractAction(forwardAction));
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

    //    if (editorService) {
    //        if (!editorService->getSelectedText) {
    //            editorService->getSelectedText = std::bind(&TextEditSplitter::getSelectedText, editManager);
    //        }
    //        if (!editorService->getCursorBeforeText) {
    //            editorService->getCursorBeforeText = std::bind(&TextEditSplitter::getCursorBeforeText, editManager);
    //        }
    //        if (!editorService->getCursorAfterText) {
    //            editorService->getCursorAfterText = std::bind(&TextEditSplitter::getCursorAfterText, editManager);
    //        }
    //        if (!editorService->replaceSelectedText) {
    //            editorService->replaceSelectedText = std::bind(&TextEditSplitter::replaceSelectedText, editManager, _1);
    //        }
    //        if (!editorService->showTips) {
    //            editorService->showTips = std::bind(&TextEditSplitter::showTips, editManager, _1);
    //        }
    //        if (!editorService->insertText) {
    //            editorService->insertText = std::bind(&TextEditSplitter::insertText, editManager, _1);
    //        }
    //        if (!editorService->undo) {
    //            editorService->undo = std::bind(&TextEditSplitter::undo, editManager);
    //        }
    //    }
    LexerManager::instance()->init(editorService);
}

void CodeEditor::initWindowService()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    auto workspaceWidget = new WorkspaceWidget;
    using namespace std::placeholders;
    if (windowService) {
        QAction *action = new QAction(MWNA_EDIT, this);
        action->setIcon(QIcon::fromTheme("edit-navigation"));
        windowService->addNavigationItem(new AbstractAction(action));

        windowService->registerWidgetToMode("editWindow", new AbstractWidget(workspaceWidget), CM_EDIT, Position::Central, true, true);
        windowService->registerWidgetToMode("editWindow", new AbstractWidget(workspaceWidget), CM_DEBUG, Position::Central, true, true);

        auto sep = new QAction(this);
        sep->setSeparator(true);
        windowService->addAction(MWM_FILE, new AbstractAction(sep));

        windowService->addContextWidget(QTabWidget::tr("Code &Lens"), new AbstractWidget(CodeLens::instance()), true);
        windowService->addContextWidget(tr("&Application Output"), new AbstractWidget(OutputPane::instance()), true);
    }
}
