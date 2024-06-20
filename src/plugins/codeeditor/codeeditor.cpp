// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codeeditor.h"
#include "codelens/codelens.h"
#include "mainframe/texteditkeeper.h"
#include "transceiver/codeeditorreceiver.h"
#include "gui/workspacewidget.h"
#include "gui/settings/editorsettingswidget.h"
#include "lexer/lexermanager.h"

#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractwidget.h"

#include "services/window/windowservice.h"
#include "services/language/languageservice.h"
#include "services/editor/editorservice.h"
#include "services/option/optionservice.h"
#include "services/option/optiondatastruct.h"

#include <DButtonBox>
#include <DToolButton>

#include <QAction>
#include <QSplitter>

using namespace dpfservice;
using namespace std::placeholders;
using DTK_WIDGET_NAMESPACE::DButtonBox;

const QString SAVE_ALL_DOCUMENTS = CodeEditor::tr("Save All Documents");
const QString CLOSE_ALL_DOCUMENTS = CodeEditor::tr("Close All Documents");
const QString PRINT = CodeEditor::tr("Print");

CodeEditor::CodeEditor()
    : dpf::Plugin()
{
}

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
    initButtonBox();
    initEditorService();
    initOptionService();

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

    DToolButton *backBtn = new DToolButton(workspaceWidget);
    backBtn->setIcon(QIcon::fromTheme("edit-back"));
    backBtn->setToolTip(tr("backward"));
    backBtn->setFixedSize(36, 36);
    connect(backBtn, &DToolButton::clicked, [=]() {
        editor.back();
    });

    DToolButton *forwardBtn = new DToolButton(workspaceWidget);
    forwardBtn->setIcon(QIcon::fromTheme("edit-forward"));
    forwardBtn->setToolTip(tr("forward"));
    forwardBtn->setFixedSize(36, 36);
    connect(forwardBtn, &DButtonBoxButton::clicked, [=]() {
        editor.forward();
    });

    QWidget *btnWidget = new QWidget(workspaceWidget);
    QHBoxLayout *layout = new QHBoxLayout(btnWidget);
    layout->addWidget(backBtn);
    layout->addWidget(forwardBtn);
    layout->setSpacing(0);

    windowService->addWidgetToTopTool(new AbstractWidget(btnWidget), "", false, false);
}

void CodeEditor::initActions()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService)
        return;

    QAction *backAction = new QAction(tr("backward"), this);
    QAction *forwardAction = new QAction(tr("forward"), this);
    QAction *closeAction = new QAction(tr("Close Current Editor"), this);
    QAction *switchHeaderSourceAction = new QAction(tr("Switch Header/Source"), this);
    QAction *follSymbolAction = new QAction(tr("Follow Symbol Under Cursor"), this);
    QAction *toggleBreakpointAction = new QAction(tr("Toggle Breakpoint"), this);
    QAction *findUsageAction = new QAction(tr("Find Usages"), this);
    QAction *renameAction = new QAction(tr("Rename Symbol Under Cursor"), this);

    auto inputBackAction = new AbstractAction(backAction, this);
    inputBackAction->setShortCutInfo("Editor.back", tr("Backward"), QKeySequence(Qt::Modifier::ALT | Qt::Key_Left));
    auto inputForwardAction = new AbstractAction(forwardAction, this);
    inputForwardAction->setShortCutInfo("Editor.forward",
                                        tr("Forward"), QKeySequence(Qt::Modifier::ALT | Qt::Key_Right));
    auto inputCloseAction = new AbstractAction(closeAction, this);
    inputCloseAction->setShortCutInfo("Editor.close",
                                      tr("Close Current Editor"), QKeySequence(Qt::Modifier::CTRL | Qt::Key_W));
    auto inputSwitchHeaderSourceAction = new AbstractAction(switchHeaderSourceAction, this);
    inputSwitchHeaderSourceAction->setShortCutInfo("Editor.switchHS",
                                                   tr("Switch Header/Source"), QKeySequence(Qt::Key_F4));
    auto inputFollSymbolAction = new AbstractAction(follSymbolAction, this);
    inputFollSymbolAction->setShortCutInfo("Editor.followSymbol",
                                           tr("Follow Symbol Under Cursor"), QKeySequence(Qt::Key_F2));
    auto inputToggleBreakpointAction = new AbstractAction(toggleBreakpointAction, this);
    inputToggleBreakpointAction->setShortCutInfo("Editor.toggleBreak",
                                                 tr("Toggle Breakpoint"), QKeySequence(Qt::Key_F9));
    auto inputFindUsageAction = new AbstractAction(findUsageAction, this);
    inputFindUsageAction->setShortCutInfo("Editor.findUsage",
                                          tr("Find Usages"), QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key_G));
    auto inputRenameAction = new AbstractAction(renameAction, this);
    inputRenameAction->setShortCutInfo("Editor.rename",
                                       tr("Rename Symbol Under Cursor"), QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key_R));

    windowService->addAction(tr("&Edit"), inputBackAction);
    windowService->addAction(tr("&Edit"), inputForwardAction);
    windowService->addAction(tr("&Edit"), inputCloseAction);
    windowService->addAction(tr("&Edit"), inputSwitchHeaderSourceAction);
    windowService->addAction(tr("&Edit"), inputFollSymbolAction);
    windowService->addAction(tr("&Edit"), inputToggleBreakpointAction);
    windowService->addAction(tr("&Edit"), inputFindUsageAction);
    windowService->addAction(tr("&Edit"), inputRenameAction);

    connect(backAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqBack);
    connect(forwardAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqForward);
    connect(closeAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqCloseCurrentEditor);
    connect(switchHeaderSourceAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqSwitchHeaderSource);
    connect(follSymbolAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqFollowSymbolUnderCursor);
    connect(toggleBreakpointAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqToggleBreakpoint);
    connect(findUsageAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqFindUsage);
    connect(renameAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqRenameSymbol);
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
    editorService->setCompletion = std::bind(&WorkspaceWidget::setCompletion, workspaceWidget, _1, _2, _3);

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
    }
}

void CodeEditor::initOptionService()
{
    auto &ctx = dpfInstance.serviceContext();
    OptionService *optionService = ctx.service<OptionService>(OptionService::name());
    if (!optionService) {
        qCritical() << "Failed, not found OptionPython service!";
        abort();
    }
    optionService->implGenerator<EditorSettingsWidgetGenerator>(option::GROUP_GENERAL, EditorSettingsWidgetGenerator::kitName());
}
