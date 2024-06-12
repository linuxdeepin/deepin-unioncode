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
#include "utils/editorutils.h"
#include "status/statusinfomanager.h"

#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractwidget.h"
#include "common/util/macroexpander.h"

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
    registerVariables();

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

    windowService->addWidgetToTopTool(new AbstractWidget(btnWidget), false, false, Priority::low);
}

void CodeEditor::initActions()
{
    QAction *backAction = new QAction(tr("Backward"), this);
    connect(backAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqBack);
    EditorUtils::registerShortcut(backAction, "Editor.back", QKeySequence(Qt::ALT | Qt::Key_Left));

    QAction *forwardAction = new QAction(tr("Forward"), this);
    connect(forwardAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqForward);
    EditorUtils::registerShortcut(forwardAction, "Editor.forward", QKeySequence(Qt::ALT | Qt::Key_Right));

    QAction *closeAction = new QAction(tr("Close Current Editor"), this);
    connect(closeAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqCloseCurrentEditor);
    EditorUtils::registerShortcut(closeAction, "Editor.close", QKeySequence(Qt::CTRL | Qt::Key_W));

    QAction *switchHeaderSourceAction = new QAction(tr("Switch Header/Source"), this);
    connect(switchHeaderSourceAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqSwitchHeaderSource);
    EditorUtils::registerShortcut(switchHeaderSourceAction, "Editor.switchHS", QKeySequence(Qt::Key_F4));

    QAction *follSymbolAction = new QAction(tr("Follow Symbol Under Cursor"), this);
    connect(follSymbolAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqFollowSymbolUnderCursor);
    EditorUtils::registerShortcut(follSymbolAction, "Editor.followSymbol", QKeySequence(Qt::Key_F2));

    QAction *toggleBreakpointAction = new QAction(tr("Toggle Breakpoint"), this);
    connect(toggleBreakpointAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqToggleBreakpoint);
    EditorUtils::registerShortcut(toggleBreakpointAction, "Editor.toggleBreak", QKeySequence(Qt::Key_F9));

    QAction *findUsageAction = new QAction(tr("Find Usages"), this);
    connect(findUsageAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqFindUsage);
    EditorUtils::registerShortcut(findUsageAction, "Editor.findUsage", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_G));

    QAction *renameAction = new QAction(tr("Rename Symbol Under Cursor"), this);
    connect(renameAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqRenameSymbol);
    EditorUtils::registerShortcut(renameAction, "Editor.rename", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
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
    editorService->currentFile = std::bind(&WorkspaceWidget::currentFile, workspaceWidget);
    editorService->setText = std::bind(&WorkspaceWidget::setText, workspaceWidget, _1);
    editorService->registerWidget = std::bind(&WorkspaceWidget::registerWidget, workspaceWidget, _1, _2);
    editorService->switchWidget = std::bind(&WorkspaceWidget::switchWidget, workspaceWidget, _1);
    editorService->switchDefaultWidget = std::bind(&WorkspaceWidget::switchDefaultWidget, workspaceWidget);
    editorService->openedFiles = std::bind(&WorkspaceWidget::openedFiles, workspaceWidget);
    editorService->fileText = std::bind(&WorkspaceWidget::fileText, workspaceWidget, _1);
    editorService->replaceAll = std::bind(&WorkspaceWidget::replaceAll, workspaceWidget, _1, _2, _3, _4, _5);

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
        windowService->addNavigationItem(new AbstractAction(action), Priority::high);

        windowService->registerWidgetToMode("editWindow", new AbstractWidget(workspaceWidget), CM_EDIT, Position::Central, true, true);
        windowService->registerWidgetToMode("editWindow", new AbstractWidget(workspaceWidget), CM_DEBUG, Position::Central, true, true);

        auto sep = new QAction(this);
        sep->setSeparator(true);
        windowService->addAction(MWM_FILE, new AbstractAction(sep));

        windowService->addContextWidget(QTabWidget::tr("Search &Results"), new AbstractWidget(CodeLens::instance()), true);

        StatusInfoManager::instance()->init(windowService);
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

void CodeEditor::registerVariables()
{
    globalMacroExpander()->registerFileVariables("CurrentDocument",
                                                 tr("Current document"),
                                                 [this] { return QFileInfo(workspaceWidget->currentFile()); });
    globalMacroExpander()->registerVariable("CurrentDocument:Content",
                                            tr("Current document content"),
                                            [this] { return workspaceWidget->currentDocumentContent(); });
}
