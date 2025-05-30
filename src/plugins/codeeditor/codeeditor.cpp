// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
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
#include "utils/resourcemanager.h"
#include "statusbar/statusinfomanager.h"
#include "symbol/symbollocator.h"
#include "symbol/symbolwidget.h"

#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractwidget.h"
#include "common/util/macroexpander.h"

#include "services/window/windowservice.h"
#include "services/language/languageservice.h"
#include "services/editor/editorservice.h"
#include "services/option/optionservice.h"
#include "services/option/optiondatastruct.h"
#include "services/locator/locatorservice.h"

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
    initSymbol();
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
    layout->setContentsMargins(0, 0, 0, 0);

    windowService->addWidgetToTopTool(new AbstractWidget(btnWidget), false, false, Priority::low);
}

void CodeEditor::initActions()
{
    auto mEdit = ActionManager::instance()->actionContainer(M_EDIT);

    QAction *backAction = new QAction(tr("Backward"), this);
    connect(backAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqBack);
    auto cmd = EditorUtils::registerShortcut(backAction, "TextEditor.back", QKeySequence(Qt::ALT | Qt::Key_Left));
    mEdit->addAction(cmd);

    QAction *forwardAction = new QAction(tr("Forward"), this);
    connect(forwardAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqForward);
    cmd = EditorUtils::registerShortcut(forwardAction, "TextEditor.forward", QKeySequence(Qt::ALT | Qt::Key_Right));
    mEdit->addAction(cmd);

    QAction *closeAction = new QAction(tr("Close Current Editor"), this);
    connect(closeAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqCloseCurrentEditor);
    cmd = EditorUtils::registerShortcut(closeAction, "TextEditor.close", QKeySequence(Qt::CTRL | Qt::Key_W));
    mEdit->addAction(cmd, G_EDIT_OTHER);

    QAction *switchHeaderSourceAction = new QAction(tr("Switch Header/Source"), this);
    connect(switchHeaderSourceAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqSwitchHeaderSource);
    EditorUtils::registerShortcut(switchHeaderSourceAction, "TextEditor.switchHS", QKeySequence(Qt::Key_F4));

    QAction *follSymbolAction = new QAction(tr("Follow Symbol Under Cursor"), this);
    connect(follSymbolAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqFollowSymbolUnderCursor);
    EditorUtils::registerShortcut(follSymbolAction, "TextEditor.followSymbol", QKeySequence(Qt::Key_F2));

    QAction *toggleBreakpointAction = new QAction(tr("Toggle Breakpoint"), this);
    connect(toggleBreakpointAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqToggleBreakpoint);
    EditorUtils::registerShortcut(toggleBreakpointAction, "TextEditor.toggleBreak", QKeySequence(Qt::Key_F9));

    QAction *findUsageAction = new QAction(tr("Find Usages"), this);
    connect(findUsageAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqFindUsage);
    EditorUtils::registerShortcut(findUsageAction, "TextEditor.findUsage", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_G));

    QAction *renameAction = new QAction(tr("Rename Symbol Under Cursor"), this);
    connect(renameAction, &QAction::triggered, EditorCallProxy::instance(), &EditorCallProxy::reqRenameSymbol);
    EditorUtils::registerShortcut(renameAction, "TextEditor.rename", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
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
    editorService->currentFile = std::bind(&WorkspaceWidget::currentFile, workspaceWidget);
    editorService->setText = std::bind(&WorkspaceWidget::setText, workspaceWidget, _1);
    editorService->registerWidget = std::bind(&WorkspaceWidget::registerWidget, workspaceWidget, _1, _2);
    editorService->switchWidget = std::bind(&WorkspaceWidget::switchWidget, workspaceWidget, _1);
    editorService->switchDefaultWidget = std::bind(&WorkspaceWidget::switchDefaultWidget, workspaceWidget);
    editorService->openedFiles = std::bind(&WorkspaceWidget::openedFiles, workspaceWidget);
    editorService->fileText = std::bind(&WorkspaceWidget::fileText, workspaceWidget, _1);
    editorService->replaceAll = std::bind(&WorkspaceWidget::replaceAll, workspaceWidget, _1, _2, _3, _4, _5);
    editorService->replaceText = std::bind(&WorkspaceWidget::replaceText, workspaceWidget, _1, _2, _3, _4, _5);
    editorService->replaceRange = std::bind(&WorkspaceWidget::replaceRange, workspaceWidget, _1, _2, _3);
    editorService->backgroundMarkerDefine = std::bind(&WorkspaceWidget::backgroundMarkerDefine, workspaceWidget, _1, _2, _3);
    editorService->setRangeBackgroundColor = std::bind(&WorkspaceWidget::setRangeBackgroundColor, workspaceWidget, _1, _2, _3, _4);
    editorService->getBackgroundRange = std::bind(&WorkspaceWidget::getBackgroundRange, workspaceWidget, _1, _2);
    editorService->clearAllBackgroundColor = std::bind(&WorkspaceWidget::clearAllBackgroundColor, workspaceWidget, _1, _2);
    editorService->showLineWidget = std::bind(&WorkspaceWidget::showLineWidget, workspaceWidget, _1, _2);
    editorService->closeLineWidget = std::bind(&WorkspaceWidget::closeLineWidget, workspaceWidget);
    editorService->cursorPosition = std::bind(&WorkspaceWidget::cursorPosition, workspaceWidget);
    editorService->registerDiagnosticRepairTool = std::bind(&ResourceManager::registerDiagnosticRepairTool, ResourceManager::instance(), _1, _2);
    editorService->getDiagnosticRepairTool = std::bind(&ResourceManager::getDiagnosticRepairTool, ResourceManager::instance());
    editorService->lineText = std::bind(&WorkspaceWidget::lineText, workspaceWidget, _1, _2);
    editorService->eOLAnnotate = std::bind(&WorkspaceWidget::eOLAnnotate, workspaceWidget, _1, _2, _3, _4, _5);
    editorService->clearEOLAnnotation = std::bind(&WorkspaceWidget::clearEOLAnnotation, workspaceWidget, _1, _2);
    editorService->clearAllEOLAnnotation = std::bind(&WorkspaceWidget::clearAllEOLAnnotation, workspaceWidget, _1);
    editorService->annotate = std::bind(&WorkspaceWidget::annotate, workspaceWidget, _1, _2, _3, _4, _5);
    editorService->clearAnnotation = std::bind(&WorkspaceWidget::clearAnnotation, workspaceWidget, _1, _2);
    editorService->clearAllAnnotation = std::bind(&WorkspaceWidget::clearAllAnnotation, workspaceWidget, _1);
    editorService->rangeText = std::bind(&WorkspaceWidget::rangeText, workspaceWidget, _1, _2);
    editorService->selectionRange = std::bind(&WorkspaceWidget::selectionRange, workspaceWidget, _1);
    editorService->codeRange = std::bind(&WorkspaceWidget::codeRange, workspaceWidget, _1, _2);
    editorService->registerInlineCompletionProvider = std::bind(&ResourceManager::registerInlineCompletionProvider, ResourceManager::instance(), _1);

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

void CodeEditor::initSymbol()
{
    auto locatorSrv = dpfGetService(LocatorService);
    if (!locatorSrv)
        return;

    SymbolLocator *locator = new SymbolLocator(workspaceWidget);
    locator->setWorkspaceWidget(workspaceWidget);
    locatorSrv->registerLocator(locator);

    SymbolWidgetGenerator::instance()->registerToDock(workspaceWidget);
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
