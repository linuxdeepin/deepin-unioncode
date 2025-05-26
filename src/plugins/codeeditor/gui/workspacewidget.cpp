// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspacewidget.h"
#include "private/workspacewidget_p.h"
#include "transceiver/codeeditorreceiver.h"
#include "settings/editorsettings.h"
#include "settings/settingsdefine.h"
#include "symbol/symbolwidget.h"
#include "symbol/symbolmanager.h"
#include "texteditor.h"

#include <DDialog>

#include <QVBoxLayout>
#include <QVariant>
#include <QApplication>
#include <QFileDialog>

constexpr char kTextEditorContext[] { "Text Editor" };
constexpr char kOpenedFileList[] { "OpenFileList" };
constexpr char kCurrentFile[] { "CurrentFile" };

using namespace dpfservice;
DWIDGET_USE_NAMESPACE

WorkspaceWidgetPrivate::WorkspaceWidgetPrivate(WorkspaceWidget *qq)
    : QObject(qq),
      q(qq)
{
    sessionSrv = dpfGetService(SessionService);
    fileCheckTimer.setInterval(500);
    fileCheckTimer.setSingleShot(true);
}

void WorkspaceWidgetPrivate::initUI()
{
    stackWidget = new QStackedWidget(q);
    stackWidget->setContentsMargins(0, 0, 0, 0);

    QSplitter *splitter = new QSplitter(q);
    splitter->setHandleWidth(0);
    splitter->setOpaqueResize(true);
    splitter->setChildrenCollapsible(false);

    TabWidget *tabWidget = new TabWidget(splitter);
    tabWidget->setCloseButtonVisible(false);
    tabWidget->setSplitButtonVisible(false);
    tabWidgetList.append(tabWidget);

    QScrollArea *scrollArea = new QScrollArea(q);
    scrollArea->setWidget(splitter);
    scrollArea->setWidgetResizable(true);
    scrollArea->setLineWidth(0);

    connectTabWidgetSignals(tabWidget);
    splitter->addWidget(tabWidget);

    stackWidget->addWidget(scrollArea);
    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(stackWidget);
}

void WorkspaceWidgetPrivate::initActions()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService)
        return;

    // add/del comment
    QAction *commentAction = new QAction(tr("Add/Delete Comment"), q);
    auto cmd = ActionManager::instance()->registerAction(commentAction, "TextEditor.AddAndRemoveComment", { kTextEditorContext });
    cmd->setDefaultKeySequence(Qt::CTRL | Qt::Key_Slash);
    connect(commentAction, &QAction::triggered, this, &WorkspaceWidgetPrivate::handleSetComment);

    // show opened files
    QAction *showOpenedAction = new QAction(tr("Show opened files"), q);
    cmd = ActionManager::instance()->registerAction(showOpenedAction, "TextEditor.ShowOpenedFiles", { kTextEditorContext });
    cmd->setDefaultKeySequence(Qt::CTRL | Qt::Key_Tab);
    connect(showOpenedAction, &QAction::triggered, this, &WorkspaceWidgetPrivate::handleShowOpenedFiles);

    QMetaEnum me = QMetaEnum::fromType<QsciCommand::Command>();
    for (int i = 0; i < me.keyCount(); ++i) {
        QList<QKeySequence> ksList;
        QString actionText;
        QsciCommand::Command val = static_cast<QsciCommand::Command>(me.value(i));
        switch (val) {
        case QsciCommand::LineDownExtend:
            actionText = tr("Extend selection down one line");
            ksList.append(Qt::Key_Down | Qt::SHIFT);
            break;
        case QsciCommand::LineDownRectExtend:
            actionText = tr("Extend rectangular selection down one line");
            ksList.append(Qt::Key_Down | Qt::ALT | Qt::SHIFT);
            break;
        case QsciCommand::LineScrollDown:
            actionText = tr("Scroll view down one line");
            ksList.append(Qt::Key_Down | Qt::CTRL);
            break;
        case QsciCommand::LineUpExtend:
            actionText = tr("Extend selection up one line");
            ksList.append(Qt::Key_Up | Qt::SHIFT);
            break;
        case QsciCommand::LineUpRectExtend:
            actionText = tr("Extend rectangular selection up one line");
            ksList.append(Qt::Key_Up | Qt::ALT | Qt::SHIFT);
            break;
        case QsciCommand::LineScrollUp:
            actionText = tr("Scroll view up one line");
            ksList.append(Qt::Key_Up | Qt::CTRL);
            break;
        case QsciCommand::ScrollToStart:
            actionText = tr("Scroll to start of document");
            break;
        case QsciCommand::ScrollToEnd:
            actionText = tr("Scroll to end of document");
            break;
        case QsciCommand::VerticalCentreCaret:
            actionText = tr("Scroll vertically to centre current line");
            break;
        case QsciCommand::ParaDown:
            actionText = tr("Move down one paragraph");
            ksList.append(Qt::Key_BracketRight | Qt::CTRL);
            break;
        case QsciCommand::ParaDownExtend:
            actionText = tr("Extend selection down one paragraph");
            ksList.append(Qt::Key_BracketRight | Qt::CTRL | Qt::SHIFT);
            break;
        case QsciCommand::ParaUp:
            actionText = tr("Move up one paragraph");
            ksList.append(Qt::Key_BracketLeft | Qt::CTRL);
            break;
        case QsciCommand::ParaUpExtend:
            actionText = tr("Extend selection up one paragraph");
            ksList.append(Qt::Key_BracketLeft | Qt::CTRL | Qt::SHIFT);
            break;
        case QsciCommand::CharLeft:
            actionText = tr("Move left one character");
            ksList.append(Qt::Key_Left);
            break;
        case QsciCommand::CharLeftExtend:
            actionText = tr("Extend selection left one character");
            ksList.append(Qt::Key_Left | Qt::SHIFT);
            break;
        case QsciCommand::CharLeftRectExtend:
            actionText = tr("Extend rectangular selection left one character");
            ksList.append(Qt::Key_Left | Qt::ALT | Qt::SHIFT);
            break;
        case QsciCommand::CharRight:
            actionText = tr("Move right one character");
            ksList.append(Qt::Key_Right);
            break;
        case QsciCommand::CharRightExtend:
            actionText = tr("Extend selection right one character");
            ksList.append(Qt::Key_Right | Qt::SHIFT);
            break;
        case QsciCommand::CharRightRectExtend:
            actionText = tr("Extend rectangular selection right one character");
            ksList.append(Qt::Key_Right | Qt::ALT | Qt::SHIFT);
            break;
        case QsciCommand::WordLeft:
            actionText = tr("Move left one word");
            ksList.append(Qt::Key_Left | Qt::CTRL);
            break;
        case QsciCommand::WordLeftExtend:
            actionText = tr("Extend selection left one word");
            ksList.append(Qt::Key_Left | Qt::CTRL | Qt::SHIFT);
            break;
        case QsciCommand::WordRight:
            actionText = tr("Move right one word");
            ksList.append(Qt::Key_Right | Qt::CTRL);
            break;
        case QsciCommand::WordRightExtend:
            actionText = tr("Extend selection right one word");
            ksList.append(Qt::Key_Right | Qt::CTRL | Qt::SHIFT);
            break;
        case QsciCommand::WordLeftEnd:
            actionText = tr("Move to end of previous word");
            break;
        case QsciCommand::WordLeftEndExtend:
            actionText = tr("Extend selection to end of previous word");
            break;
        case QsciCommand::WordRightEnd:
            actionText = tr("Move to end of next word");
            break;
        case QsciCommand::WordRightEndExtend:
            actionText = tr("Extend selection to end of next word");
            break;
        case QsciCommand::WordPartLeft:
            actionText = tr("Move left one word part");
            break;
        case QsciCommand::WordPartLeftExtend:
            actionText = tr("Extend selection left one word part");
            ksList.append(Qt::Key_Slash | Qt::CTRL | Qt::SHIFT);
            break;
        case QsciCommand::WordPartRight:
            actionText = tr("Move right one word part");
            ksList.append(Qt::Key_Backslash | Qt::CTRL);
            break;
        case QsciCommand::WordPartRightExtend:
            actionText = tr("Extend selection right one word part");
            ksList.append(Qt::Key_Backslash | Qt::CTRL | Qt::SHIFT);
            break;
        case QsciCommand::Home:
            actionText = tr("Move to start of document line");
            break;
        case QsciCommand::HomeExtend:
            actionText = tr("Extend selection to start of document line");
            break;
        case QsciCommand::HomeRectExtend:
            actionText = tr("Extend rectangular selection to start of document line");
            break;
        case QsciCommand::HomeDisplay:
            actionText = tr("Move to start of display line");
            ksList.append(Qt::Key_Home | Qt::ALT);
            break;
        case QsciCommand::HomeDisplayExtend:
            actionText = tr("Extend selection to start of display line");
            break;
        case QsciCommand::HomeWrap:
            actionText = tr("Move to start of display or document line");
            break;
        case QsciCommand::HomeWrapExtend:
            actionText = tr("Extend selection to start of display or document line");
            break;
        case QsciCommand::VCHome:
            actionText = tr("Move to first visible character in document line");
            ksList.append(Qt::Key_Home);
            break;
        case QsciCommand::VCHomeExtend:
            actionText = tr("Extend selection to first visible character in document line");
            ksList.append(Qt::Key_Home | Qt::SHIFT);
            break;
        case QsciCommand::VCHomeRectExtend:
            actionText = tr("Extend rectangular selection to first visible character in document line");
            ksList.append(Qt::Key_Home | Qt::ALT | Qt::SHIFT);
            break;
        case QsciCommand::VCHomeWrap:
            actionText = tr("Move to first visible character of display in document line");
            break;
        case QsciCommand::VCHomeWrapExtend:
            actionText = tr("Extend selection to first visible character in display or document line");
            break;
        case QsciCommand::LineEnd:
            actionText = tr("Move to end of document line");
            ksList.append(Qt::Key_End);
            break;
        case QsciCommand::LineEndExtend:
            actionText = tr("Extend selection to end of document line");
            ksList.append(Qt::Key_End | Qt::SHIFT);
            break;
        case QsciCommand::LineEndRectExtend:
            actionText = tr("Extend rectangular selection to end of document line");
            ksList.append(Qt::Key_End | Qt::ALT | Qt::SHIFT);
            break;
        case QsciCommand::LineEndDisplay:
            actionText = tr("Move to end of display line");
            ksList.append(Qt::Key_End | Qt::ALT);
            break;
        case QsciCommand::LineEndDisplayExtend:
            actionText = tr("Extend selection to end of display line");
            break;
        case QsciCommand::LineEndWrap:
            actionText = tr("Move to end of display or document line");
            break;
        case QsciCommand::LineEndWrapExtend:
            actionText = tr("Extend selection to end of display or document line");
            break;
        case QsciCommand::DocumentStart:
            actionText = tr("Move to start of document");
            ksList.append(Qt::Key_Home | Qt::CTRL);
            break;
        case QsciCommand::DocumentStartExtend:
            actionText = tr("Extend selection to start of document");
            ksList.append(Qt::Key_Home | Qt::CTRL | Qt::SHIFT);
            break;
        case QsciCommand::DocumentEnd:
            actionText = tr("Move to end of document");
            ksList.append(Qt::Key_End | Qt::CTRL);
            break;
        case QsciCommand::DocumentEndExtend:
            actionText = tr("Extend selection to end of document");
            ksList.append(Qt::Key_End | Qt::CTRL | Qt::SHIFT);
            break;
        case QsciCommand::PageUp:
            actionText = tr("Move up one page");
            ksList.append(Qt::Key_PageUp);
            break;
        case QsciCommand::PageUpExtend:
            actionText = tr("Extend selection up one page");
            ksList.append(Qt::Key_PageUp | Qt::SHIFT);
            break;
        case QsciCommand::PageUpRectExtend:
            actionText = tr("Extend rectangular selection up one page");
            ksList.append(Qt::Key_PageUp | Qt::ALT | Qt::SHIFT);
            break;
        case QsciCommand::PageDown:
            actionText = tr("Move down one page");
            ksList.append(Qt::Key_PageDown);
            break;
        case QsciCommand::PageDownExtend:
            actionText = tr("Extend selection down one page");
            ksList.append(Qt::Key_PageDown | Qt::SHIFT);
            break;
        case QsciCommand::PageDownRectExtend:
            actionText = tr("Extend rectangular selection down one page");
            ksList.append(Qt::Key_PageDown | Qt::ALT | Qt::SHIFT);
            break;
        case QsciCommand::StutteredPageUp:
            actionText = tr("Stuttered move up one page");
            break;
        case QsciCommand::StutteredPageUpExtend:
            actionText = tr("Stuttered extend selection up one page");
            break;
        case QsciCommand::StutteredPageDown:
            actionText = tr("Stuttered move down one page");
            break;
        case QsciCommand::StutteredPageDownExtend:
            actionText = tr("Stuttered extend selection down one page");
            break;
        case QsciCommand::Delete:
            actionText = tr("Delete current character");
            ksList.append(Qt::Key_Delete);
            break;
        case QsciCommand::DeleteBack:
            actionText = tr("Delete previous character");
            ksList.append(Qt::Key_Backspace);
            break;
        case QsciCommand::DeleteBackNotLine:
            actionText = tr("Delete previous character if not at start of line");
            break;
        case QsciCommand::DeleteWordLeft:
            actionText = tr("Delete word to left");
            ksList.append(Qt::Key_Backspace | Qt::CTRL);
            break;
        case QsciCommand::DeleteWordRight:
            actionText = tr("Delete word to right");
            ksList.append(Qt::Key_Delete | Qt::CTRL);
            break;
        case QsciCommand::DeleteWordRightEnd:
            actionText = tr("Delete right to end of next word");
            break;
        case QsciCommand::DeleteLineLeft:
            actionText = tr("Delete line to left");
            ksList.append(Qt::Key_Backspace | Qt::CTRL | Qt::SHIFT);
            break;
        case QsciCommand::DeleteLineRight:
            actionText = tr("Delete line to right");
            ksList.append(Qt::Key_Delete | Qt::CTRL | Qt::SHIFT);
            break;
        case QsciCommand::LineDelete:
            actionText = tr("Delete current line");
            ksList.append(Qt::Key_L | Qt::CTRL | Qt::SHIFT);
            break;
        case QsciCommand::LineCut:
            actionText = tr("Cut current line");
            ksList.append(Qt::Key_L | Qt::CTRL);
            break;
        case QsciCommand::LineCopy:
            actionText = tr("Copy current line");
            ksList.append(Qt::Key_T | Qt::CTRL | Qt::SHIFT);
            break;
        case QsciCommand::LineTranspose:
            actionText = tr("Transpose current and previous lines");
            break;
        case QsciCommand::LineDuplicate:
            actionText = tr("Duplicate the current line");
            break;
        case QsciCommand::SelectAll:
            actionText = tr("Select all");
            ksList.append(Qt::Key_A | Qt::CTRL);
            break;
        case QsciCommand::MoveSelectedLinesUp:
            actionText = tr("Move selected lines up one line");
            break;
        case QsciCommand::MoveSelectedLinesDown:
            actionText = tr("Move selected lines down one line");
            break;
        case QsciCommand::SelectionDuplicate:
            actionText = tr("Duplicate selection");
            ksList.append(Qt::Key_D | Qt::CTRL);
            break;
        case QsciCommand::SelectionLowerCase:
            actionText = tr("Convert selection to lower case");
            ksList.append(Qt::Key_U | Qt::CTRL);
            break;
        case QsciCommand::SelectionUpperCase:
            actionText = tr("Convert selection to upper case");
            ksList.append(Qt::Key_U | Qt::CTRL | Qt::SHIFT);
            break;
        case QsciCommand::SelectionCut:
            actionText = tr("Cut selection");
            ksList.append(Qt::Key_X | Qt::CTRL);
            ksList.append(Qt::Key_Delete | Qt::SHIFT);
            break;
        case QsciCommand::SelectionCopy:
            actionText = tr("Copy selection");
            ksList.append(Qt::Key_C | Qt::CTRL);
            ksList.append(Qt::Key_Insert | Qt::CTRL);
            break;
        case QsciCommand::Paste:
            actionText = tr("Paste");
            ksList.append(Qt::Key_V | Qt::CTRL);
            ksList.append(Qt::Key_Insert | Qt::SHIFT);
            break;
        case QsciCommand::EditToggleOvertype:
            actionText = tr("Toggle insert/overtype");
            ksList.append(Qt::Key_Insert);
            break;
        case QsciCommand::Formfeed:
            actionText = tr("Formfeed");
            break;
        case QsciCommand::Backtab:
            actionText = tr("De-indent one level");
            ksList.append(Qt::Key_Tab | Qt::SHIFT);
            break;
        case QsciCommand::Undo:
            actionText = tr("Undo last command");
            ksList.append(Qt::Key_Z | Qt::CTRL);
            ksList.append(Qt::Key_Backspace | Qt::ALT);
            break;
        case QsciCommand::Redo:
            actionText = tr("Redo last command");
            ksList.append(Qt::Key_Y | Qt::CTRL);
            break;
        case QsciCommand::ZoomIn:
            actionText = tr("Zoom in");
            ksList.append(Qt::Key_Plus | Qt::CTRL);
            break;
        case QsciCommand::ZoomOut:
            actionText = tr("Zoom out");
            ksList.append(Qt::Key_Minus | Qt::CTRL);
            break;
        default:
            break;
        }

        if (!actionText.isEmpty()) {
            auto id = QString("TextEditor.%1").arg(me.valueToKey(val));
            auto act = new QAction(actionText, q);
            auto cmd = ActionManager::instance()->registerAction(act, id, { kTextEditorContext });
            if (!ksList.isEmpty())
                cmd->setDefaultKeySequences(ksList);
            connect(act, &QAction::triggered, this, [val, this] {
                auto tabWidget = currentTabWidget();
                if (!tabWidget)
                    return;

                if (auto editor = tabWidget->currentEditor()) {
                    editor->SendScintilla(val);
                    if (val == QsciCommand::EditToggleOvertype)
                        Q_EMIT editor->cursorModeChanged();
                }
            });
        }
    }
}

void WorkspaceWidgetPrivate::handleSetComment()
{
    if (!currentTabWidget())
        return;

    currentTabWidget()->handleSetComment();
}

void WorkspaceWidgetPrivate::handleShowOpenedFiles()
{
    if (!currentTabWidget())
        return;

    currentTabWidget()->handleShowOpenedFiles(q->pos().x() - q->mapFromGlobal(q->pos()).x(), q->pos().y() + q->mapToGlobal(q->pos()).y() - 100, q->size());
}

void WorkspaceWidgetPrivate::handleSaveSession()
{
    sessionSrv->setValue(kOpenedFileList, q->openedFiles());
    sessionSrv->setValue(kCurrentFile, q->currentFile());
}

void WorkspaceWidgetPrivate::handleSessionLoaded()
{
    while (tabWidgetList.size() > 1) {
        auto tabWidget = tabWidgetList.takeLast();
        tabWidget->deleteLater();
    }

    tabWidgetList.first()->closeAllEditor();
    focusTabWidget = tabWidgetList.first();
    auto symbolWidget = SymbolWidgetGenerator::instance()->symbolWidget();
    symbolWidget->setEditor(nullptr);

    const auto &openedFiles = sessionSrv->value(kOpenedFileList).toStringList();
    const auto &currentFile = sessionSrv->value(kCurrentFile).toString();
    for (const auto &file : openedFiles)
        handleOpenFile("", file);
    if (!currentFile.isEmpty())
        handleOpenFile("", currentFile);
}

void WorkspaceWidgetPrivate::initConnection()
{
    connect(&fileCheckTimer, &QTimer::timeout, this, &WorkspaceWidgetPrivate::checkFileState);
    connect(qApp, &QApplication::focusChanged, this, &WorkspaceWidgetPrivate::onFocusChanged);
    connect(Inotify::globalInstance(), &Inotify::deletedSelf, this, &WorkspaceWidgetPrivate::onFileDeleted);
    connect(Inotify::globalInstance(), &Inotify::movedSelf, this, &WorkspaceWidgetPrivate::onFileDeleted);
    connect(Inotify::globalInstance(), &Inotify::modified, this, &WorkspaceWidgetPrivate::onFileModified);

    connect(EditorCallProxy::instance(), &EditorCallProxy::reqOpenFile, this, &WorkspaceWidgetPrivate::handleOpenFile);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqCloseFile, this, &WorkspaceWidgetPrivate::handleCloseFile);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqAddBreakpoint, this, &WorkspaceWidgetPrivate::handleAddBreakpoint);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqRemoveBreakpoint, this, &WorkspaceWidgetPrivate::handleRemoveBreakpoint);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqSetBreakpointEnabled, this, &WorkspaceWidgetPrivate::handleSetBreakpointEnabled);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqBack, this, &WorkspaceWidgetPrivate::handleBack);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqForward, this, &WorkspaceWidgetPrivate::handleForward);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqSetDebugLine, this, &WorkspaceWidgetPrivate::handleSetDebugLine);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqRemoveDebugLine, this, &WorkspaceWidgetPrivate::handleRemoveDebugLine);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqGotoLine, this, &WorkspaceWidgetPrivate::handleGotoLine);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqGotoPosition, this, &WorkspaceWidgetPrivate::handleGotoPosition);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqCloseCurrentEditor, this, &WorkspaceWidgetPrivate::handleCloseCurrentEditor);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqSwitchHeaderSource, this, &WorkspaceWidgetPrivate::handleSwitchHeaderSource);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqFollowSymbolUnderCursor, this, &WorkspaceWidgetPrivate::handleFollowSymbolUnderCursor);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqFindUsage, this, &WorkspaceWidgetPrivate::handleFindUsage);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqRenameSymbol, this, &WorkspaceWidgetPrivate::handleRenameSymbol);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqToggleBreakpoint, this, &WorkspaceWidgetPrivate::handleToggleBreakpoint);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqSetModifiedAutoReload, this, &WorkspaceWidgetPrivate::handleSetModifiedAutoReload);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqSaveSession, this, &WorkspaceWidgetPrivate::handleSaveSession);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqSessionLoaded, this, &WorkspaceWidgetPrivate::handleSessionLoaded);
}

void WorkspaceWidgetPrivate::connectTabWidgetSignals(TabWidget *tabWidget)
{
    connect(tabWidget, &TabWidget::splitRequested, this, &WorkspaceWidgetPrivate::onSplitRequested);
    connect(tabWidget, &TabWidget::closeRequested, this, &WorkspaceWidgetPrivate::onCloseRequested);
    connect(tabWidget, &TabWidget::zoomValueChanged, this, &WorkspaceWidgetPrivate::onZoomValueChanged);
}

TabWidget *WorkspaceWidgetPrivate::currentTabWidget() const
{
    if (focusTabWidget)
        return focusTabWidget;

    // Return a default tabWidget
    return tabWidgetList.isEmpty() ? nullptr : tabWidgetList.first();
}

void WorkspaceWidgetPrivate::doSplit(TabWidget *fromTW, QSplitter *spliter, int index, const QString &fileName)
{
    auto fromEdit = fromTW->findEditor(fileName);
    if (!fromEdit)
        return;

    TabWidget *tabWidget = new TabWidget(spliter);
    connectTabWidgetSignals(tabWidget);

    tabWidgetList.append(tabWidget);
    spliter->insertWidget(index, tabWidget);

    auto doc = fromEdit->document();
    tabWidget->openFile(fileName, &doc);
    // Set the cursor and scroll position
    int cursorPos = fromTW->editorCursorPosition();
    int scroll = fromTW->editorScrollValue();
    tabWidget->setEditorCursorPosition(cursorPos);
    tabWidget->setEditorScrollValue(scroll);
}

int WorkspaceWidgetPrivate::showFileChangedConfirmDialog(const QString &fileName)
{
    if (autoReloadList.contains(fileName))
        return 0;

    DDialog d(qApp->activeWindow());
    const int maxDisplayNameLength = 250;

    QFileInfo info(fileName);
    const QString &fileDisplayName = d.fontMetrics().elidedText(info.fileName(), Qt::ElideMiddle, maxDisplayNameLength);
    const QString &message = tr("The file <i>%1</i> has been changed on disk.Do you want to reload it?").arg(fileDisplayName);
    const QString &title = tr("File Has Been Changed");
    QStringList buttonTexts;

    buttonTexts.append(tr("Yes", "button"));
    buttonTexts.append(tr("Yes To All", "button"));
    buttonTexts.append(tr("No", "button"));
    buttonTexts.append(tr("No To All", "button"));
    buttonTexts.append(tr("Close", "button"));

    d.setIcon(QIcon::fromTheme("ide"));
    d.setTitle(title);
    d.setMessage(message);
    d.addButton(buttonTexts[0]);
    d.addButton(buttonTexts[1], true, DDialog::ButtonRecommend);
    d.addButton(buttonTexts[2]);
    d.addButton(buttonTexts[3]);
    d.addButton(buttonTexts[4]);
    d.setFixedWidth(480);
    return d.exec();
}

int WorkspaceWidgetPrivate::showFileRemovedConfirmDialog(const QString &fileName)
{
    DDialog d(qApp->activeWindow());
    const int maxDisplayNameLength = 250;

    const QString &fileDisplayName = d.fontMetrics().elidedText(fileName, Qt::ElideMiddle, maxDisplayNameLength);
    const QString &message = tr("The file <i>%1</i> has been removed from disk."
                                " Do you want to save it under a different name,"
                                " or close the editor?")
                                     .arg(fileDisplayName);
    const QString &title = tr("File Has Been Removed");
    QStringList buttonTexts;

    buttonTexts.append(tr("Save", "button"));
    buttonTexts.append(tr("Save As", "button"));
    buttonTexts.append(tr("Close", "button"));
    buttonTexts.append(tr("Close All", "button"));

    d.setIcon(QIcon::fromTheme("ide"));
    d.setTitle(title);
    d.setMessage(message);
    d.addButton(buttonTexts[0]);
    d.addButton(buttonTexts[1], true, DDialog::ButtonRecommend);
    d.addButton(buttonTexts[2]);
    d.addButton(buttonTexts[3]);
    d.setFixedWidth(480);
    return d.exec();
}

void WorkspaceWidgetPrivate::handleFileChanged()
{
    if (modifiedFileList.isEmpty())
        return;

    auto fileName = modifiedFileList.takeFirst();
    int ret = showFileChangedConfirmDialog(fileName);
    switch (ret) {
    case 0:   // yes
        q->reloadFile(fileName);
        handleFileChanged();
        break;
    case 1:   // yes to all
        q->reloadFile(fileName);
        while (!modifiedFileList.isEmpty()) {
            fileName = modifiedFileList.takeFirst();
            q->reloadFile(fileName);
        }
        break;
    case 2:   // no
        q->setFileModified(fileName, true);
        handleFileChanged();
        break;
    case 3:   // no to all
        q->setFileModified(fileName, true);
        while (!modifiedFileList.isEmpty()) {
            fileName = modifiedFileList.takeFirst();
            q->setFileModified(fileName, true);
        }
        break;
    case 4:   // close
        handleCloseFile(fileName);
        handleFileChanged();
        break;
    default:
        break;
    }
}

void WorkspaceWidgetPrivate::handleFileRemoved()
{
    if (removedFileList.isEmpty())
        return;

    auto fileName = removedFileList.takeFirst();
    int ret = showFileRemovedConfirmDialog(fileName);
    switch (ret) {
    case 0:   // save
        q->saveAs(fileName, fileName);
        handleFileRemoved();
        break;
    case 1:   // save as
        q->saveAs(fileName);
        handleFileRemoved();
        break;
    case 2:   // close
        handleCloseFile(fileName);
        handleFileRemoved();
        break;
    case 3:   // close all
        handleCloseFile(fileName);
        while (!removedFileList.isEmpty()) {
            fileName = removedFileList.takeFirst();
            handleCloseFile(fileName);
        }
        break;
    default:
        break;
    }
}

void WorkspaceWidgetPrivate::checkFileState()
{
    for (const auto &file : removedFileList) {
        if (!QFile::exists(file))
            continue;

        removedFileList.removeOne(file);
        Inotify::globalInstance()->addPath(file);
        modifiedFileList.append(file);
    }

    handleFileChanged();
    handleFileRemoved();
}

void WorkspaceWidgetPrivate::onSplitRequested(Qt::Orientation ori, const QString &fileName)
{
    auto tabWidgetSender = qobject_cast<TabWidget *>(sender());
    if (!tabWidgetSender)
        return;

    auto spliter = qobject_cast<QSplitter *>(tabWidgetSender->parent());
    if (!spliter)
        return;

    tabWidgetSender->setCloseButtonVisible(true);
    int index = spliter->indexOf(tabWidgetSender);
    if (spliter->count() == 1) {
        // Only one widget is added to the splitter,
        // change its orientation and add a new widget
        spliter->setOrientation(ori);
        doSplit(tabWidgetSender, spliter, index + 1, fileName);
    } else if (spliter->orientation() == ori) {
        doSplit(tabWidgetSender, spliter, index + 1, fileName);
    } else {
        // Use a new splitter to replace
        QSplitter *newSplitter = new QSplitter(q);
        newSplitter->setOrientation(ori);

        spliter->replaceWidget(index, newSplitter);
        newSplitter->addWidget(tabWidgetSender);
        doSplit(tabWidgetSender, newSplitter, 1, fileName);
    }
}

void WorkspaceWidgetPrivate::onCloseRequested()
{
    if (tabWidgetList.size() == 1)
        return;

    TabWidget *tabWidget = qobject_cast<TabWidget *>(sender());
    if (!tabWidget)
        return;

    if (focusTabWidget == tabWidget)
        focusTabWidget = nullptr;

    tabWidgetList.removeOne(tabWidget);
    tabWidget->deleteLater();

    if (!tabWidgetList.isEmpty()) {
        tabWidgetList.last()->setFocus();
        editor.switchedFile(tabWidgetList.last()->currentFile());
        auto symbolWidget = SymbolWidgetGenerator::instance()->symbolWidget();
        symbolWidget->setEditor(tabWidgetList.last()->currentEditor());
    }

    if (tabWidgetList.size() == 1)
        tabWidgetList.first()->setCloseButtonVisible(false);
}

void WorkspaceWidgetPrivate::handleOpenFile(const QString &workspace, const QString &fileName)
{
    Q_UNUSED(workspace)

    if (stackWidget->currentIndex() != 0)
        stackWidget->setCurrentIndex(0);

    // find the editor if the `fileName` is already opened
    TextEditor *editor { nullptr };
    for (auto tabWidget : qAsConst(tabWidgetList)) {
        editor = tabWidget->findEditor(fileName);
        if (editor)
            break;
    }

    if (auto tabWidget = currentTabWidget()) {
        if (editor) {
            auto document = editor->document();
            tabWidget->openFile(fileName, &document);
        } else {
            tabWidget->openFile(fileName);
        }
    }
}

void WorkspaceWidgetPrivate::handleCloseFile(const QString &fileName)
{
    if (auto tabWidget = currentTabWidget())
        tabWidget->closeFileEditor(fileName);
}

void WorkspaceWidgetPrivate::handleAddBreakpoint(const QString &fileName, int line, bool enabled)
{
    for (auto tabWidget : tabWidgetList)
        tabWidget->addBreakpoint(fileName, line, enabled);
}

void WorkspaceWidgetPrivate::handleRemoveBreakpoint(const QString &fileName, int line)
{
    for (auto tabWidget : tabWidgetList)
        tabWidget->removeBreakpoint(fileName, line);
}

void WorkspaceWidgetPrivate::handleSetBreakpointEnabled(const QString &fileName, int line, bool enabled)
{
    for (auto tabWidget : tabWidgetList)
        tabWidget->setBreakpointEnabled(fileName, line, enabled);
}

void WorkspaceWidgetPrivate::handleToggleBreakpoint()
{
    if (auto tabWidget = currentTabWidget())
        tabWidget->toggleBreakpoint();
}

void WorkspaceWidgetPrivate::handleBack()
{
    if (auto tabWidget = currentTabWidget())
        tabWidget->gotoPreviousPosition();
}

void WorkspaceWidgetPrivate::handleForward()
{
    if (auto tabWidget = currentTabWidget())
        tabWidget->gotoNextPosition();
}

void WorkspaceWidgetPrivate::handleSetDebugLine(const QString &fileName, int line)
{
    auto tabWidget = currentTabWidget();
    if (!tabWidget)
        return;

    tabWidget->openFile(fileName);
    tabWidget->setDebugLine(line);
}

void WorkspaceWidgetPrivate::handleRemoveDebugLine()
{
    for (auto tabWidget : tabWidgetList)
        tabWidget->removeDebugLine();
}

void WorkspaceWidgetPrivate::handleGotoLine(const QString &fileName, int line)
{
    auto tabWidget = currentTabWidget();
    if (!tabWidget)
        return;

    tabWidget->openFile(fileName);
    tabWidget->gotoLine(line);
}

void WorkspaceWidgetPrivate::handleGotoPosition(const QString &fileName, int line, int column)
{
    auto tabWidget = currentTabWidget();
    if (!tabWidget)
        return;

    tabWidget->openFile(fileName);
    tabWidget->gotoPosition(line, column);
}

void WorkspaceWidgetPrivate::handleCloseCurrentEditor()
{
    if (stackWidget->currentIndex() == 0) {
        auto tabWidget = currentTabWidget();
        if (!tabWidget)
            return;

        tabWidget->closeFileEditor();
    } else {
        auto widget = qobject_cast<AbstractEditWidget *>(stackWidget->currentWidget());
        if (widget)
            widget->closeWidget();
    }
}

void WorkspaceWidgetPrivate::handleSwitchHeaderSource()
{
    auto tabWidget = currentTabWidget();
    if (!tabWidget)
        return;

    tabWidget->switchHeaderSource();
}

void WorkspaceWidgetPrivate::handleFollowSymbolUnderCursor()
{
    auto tabWidget = currentTabWidget();
    if (!tabWidget)
        return;

    tabWidget->followSymbolUnderCursor();
}

void WorkspaceWidgetPrivate::handleFindUsage()
{
    auto tabWidget = currentTabWidget();
    if (!tabWidget)
        return;

    tabWidget->findUsage();
}

void WorkspaceWidgetPrivate::handleRenameSymbol()
{
    auto tabWidget = currentTabWidget();
    if (!tabWidget)
        return;

    tabWidget->renameSymbol();
}

void WorkspaceWidgetPrivate::handleSetModifiedAutoReload(const QString &fileName, bool flag)
{
    if (!flag)
        autoReloadList.removeOne(fileName);
    else if (!autoReloadList.contains(fileName))
        autoReloadList << fileName;
}

void WorkspaceWidgetPrivate::onFocusChanged(QWidget *old, QWidget *now)
{
    Q_UNUSED(old)

    if (!now) {
        ActionManager::instance()->removeContext({ kTextEditorContext });
        return;
    }

    // the `now` is TextEditor
    auto tabWidget = qobject_cast<TabWidget *>(now->parentWidget());
    if (!tabWidget) {
        ActionManager::instance()->removeContext({ kTextEditorContext });
        return;
    }

    ActionManager::instance()->addContext({ kTextEditorContext });
    focusTabWidget = tabWidget;
    editor.switchedFile(focusTabWidget->currentFile());
    auto symbolWidget = SymbolWidgetGenerator::instance()->symbolWidget();
    symbolWidget->setEditor(tabWidget->currentEditor());
}

void WorkspaceWidgetPrivate::onZoomValueChanged()
{
    auto tabWidget = qobject_cast<TabWidget *>(sender());
    if (!tabWidget)
        return;

    int zoomValue = tabWidget->zoomValue();
    int displayZoomValue = 100 + 10 * zoomValue;
    EditorSettings::instance()->setValue(Node::FontColor, Group::FontGroup, Key::FontZoom, displayZoomValue, false);
    for (auto tabWidget : tabWidgetList)
        tabWidget->updateZoomValue(zoomValue);
}

void WorkspaceWidgetPrivate::onFileDeleted(const QString &fileName)
{
    if (QFile::exists(fileName)) {
        Inotify::globalInstance()->addPath(fileName);
        return onFileModified(fileName);
    }

    if (removedFileList.contains(fileName))
        return;

    QStringList openedFiles;
    for (auto tabWidget : tabWidgetList)
        openedFiles.append(tabWidget->openedFiles());

    if (!openedFiles.contains(fileName))
        return;

    Inotify::globalInstance()->removePath(fileName);
    removedFileList << fileName;
    if (q->isActiveWindow())
        fileCheckTimer.start();
}

void WorkspaceWidgetPrivate::onFileModified(const QString &fileName)
{
    if (modifiedFileList.contains(fileName))
        return;

    QStringList openedFiles;
    for (auto tabWidget : tabWidgetList)
        openedFiles.append(tabWidget->openedFiles());

    if (!openedFiles.contains(fileName))
        return;

    modifiedFileList << fileName;
    if (q->isActiveWindow())
        fileCheckTimer.start();
}

WorkspaceWidget::WorkspaceWidget(QWidget *parent)
    : QWidget(parent),
      d(new WorkspaceWidgetPrivate(this))
{
    d->initActions();
    d->initUI();
    d->initConnection();
}

QString WorkspaceWidget::currentFile() const
{
    if (auto tabWidget = d->currentTabWidget())
        return tabWidget->currentFile();

    return {};
}

QString WorkspaceWidget::currentDocumentContent() const
{
    if (auto tabWidget = d->currentTabWidget())
        return tabWidget->currentDocumentContent();

    return {};
}

QString WorkspaceWidget::selectedText() const
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    if (auto tabWidget = d->currentTabWidget())
        return tabWidget->selectedText();

    return "";
}

QString WorkspaceWidget::cursorBeforeText() const
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    if (auto tabWidget = d->currentTabWidget())
        return tabWidget->cursorBeforeText();

    return "";
}

QString WorkspaceWidget::cursorBehindText() const
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    if (auto tabWidget = d->currentTabWidget())
        return tabWidget->cursorBehindText();

    return "";
}

QStringList WorkspaceWidget::modifiedFiles() const
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    QStringList files;
    for (auto tabWidget : d->tabWidgetList)
        files << tabWidget->modifiedFiles();

    // Delete duplicates
    files.removeDuplicates();
    return files;
}

QString WorkspaceWidget::rangeText(const QString &fileName, const dpfservice::Edit::Range &range)
{
    for (auto tabWidget : d->tabWidgetList) {
        bool found = false;
        const auto &text = tabWidget->rangeText(fileName, range, found);
        if (found)
            return text;
    }

    return {};
}

Edit::Range WorkspaceWidget::codeRange(const QString &fileName, const dpfservice::Edit::Position &pos)
{
    Edit::Range range;
    const auto &info = SymbolManager::instance()->findSymbol(fileName, pos.line, pos.column);
    range.start = { info.range.start.line, info.range.start.character };
    range.end = { info.range.end.line, info.range.end.character };
    return range;
}

Edit::Range WorkspaceWidget::selectionRange(const QString &fileName)
{
    Edit::Range range;
    for (auto tabWidget : d->tabWidgetList) {
        bool found = false;
        range = tabWidget->selectionRange(fileName, found);
        if (found)
            break;
    }

    return range;
}

void WorkspaceWidget::setText(const QString &text)
{
    if (auto tabWidget = d->currentTabWidget())
        tabWidget->setText(text);
}

void WorkspaceWidget::saveAll() const
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    for (auto tabWidget : d->tabWidgetList)
        tabWidget->saveAll();
}

void WorkspaceWidget::saveAs(const QString &from, const QString &to)
{
    auto tmpTo = to;
    if (tmpTo.isEmpty()) {
        tmpTo = QFileDialog::getSaveFileName(this);
        if (tmpTo.isEmpty())
            return;
    }

    Q_UNUSED(std::find_if(d->tabWidgetList.begin(), d->tabWidgetList.end(),
                          [&](TabWidget *w) {
                              return w->saveAs(from, tmpTo);
                          });)
    Inotify::globalInstance()->addPath(tmpTo);
}

void WorkspaceWidget::replaceSelectedText(const QString &text)
{
    if (auto tabWidget = d->currentTabWidget())
        QMetaObject::invokeMethod(tabWidget, "replaceSelectedText",
                                  Qt::QueuedConnection,
                                  Q_ARG(const QString &, text));
}

void WorkspaceWidget::showTips(const QString &tips)
{
    if (auto tabWidget = d->currentTabWidget())
        QMetaObject::invokeMethod(tabWidget, "showTips",
                                  Qt::QueuedConnection,
                                  Q_ARG(const QString &, tips));
}

void WorkspaceWidget::insertText(const QString &text)
{
    if (auto tabWidget = d->currentTabWidget())
        QMetaObject::invokeMethod(tabWidget, "insertText",
                                  Qt::QueuedConnection,
                                  Q_ARG(const QString &, text));
}

void WorkspaceWidget::undo()
{
    if (auto tabWidget = d->currentTabWidget())
        QMetaObject::invokeMethod(tabWidget, "undo", Qt::QueuedConnection);
}

void WorkspaceWidget::reloadFile(const QString &fileName)
{
    for (auto tabWidget : d->tabWidgetList)
        tabWidget->reloadFile(fileName);
}

void WorkspaceWidget::setFileModified(const QString &fileName, bool isModified)
{
    for (auto tabWidget : d->tabWidgetList)
        tabWidget->setFileModified(fileName, isModified);
}

QStringList WorkspaceWidget::openedFiles() const
{
    QStringList files;
    for (auto tabWidget : d->tabWidgetList)
        files << tabWidget->openedFiles();

    // Delete duplicates
    files.removeDuplicates();
    return files;
}

QString WorkspaceWidget::fileText(const QString &fileName) const
{
    for (auto tabWidget : d->tabWidgetList) {
        bool success = false;
        const auto &text = tabWidget->fileText(fileName, &success);
        if (success)
            return text;
    }

    return {};
}

void WorkspaceWidget::replaceAll(const QString &fileName, const QString &oldText,
                                 const QString &newText, bool caseSensitive, bool wholeWords)
{
    for (auto tabWidget : d->tabWidgetList) {
        tabWidget->replaceAll(fileName, oldText, newText, caseSensitive, wholeWords);
    }
}

void WorkspaceWidget::replaceText(const QString &fileName, int line, int index, int length, const QString &after)
{
    for (auto tabWidget : d->tabWidgetList) {
        tabWidget->replaceText(fileName, line, index, length, after);
    }
}

void WorkspaceWidget::replaceRange(const QString &fileName, const dpfservice::Edit::Range &range, const QString &newText)
{
    for (auto tabWidget : d->tabWidgetList) {
        if (tabWidget->replaceRange(fileName, range, newText))
            break;
    }
}

TabWidget *WorkspaceWidget::currentTabWidget() const
{
    return d->currentTabWidget();
}

Edit::Position WorkspaceWidget::cursorPosition()
{
    Edit::Position pos;
    if (auto tabWidget = d->currentTabWidget())
        tabWidget->cursorPosition(&pos.line, &pos.column);

    return pos;
}

QString WorkspaceWidget::lineText(const QString &fileName, int line)
{
    for (auto tabWidget : d->tabWidgetList) {
        const auto &text = tabWidget->lineText(fileName, line);
        if (!text.isEmpty())
            return text;
    }

    return {};
}

void WorkspaceWidget::eOLAnnotate(const QString &fileName, const QString &title, const QString &contents, int line, int type)
{
    for (auto tabWidget : d->tabWidgetList) {
        if (tabWidget->eOLAnnotate(fileName, title, contents, line, type))
            break;
    }
}

void WorkspaceWidget::clearEOLAnnotation(const QString &fileName, const QString &title)
{
    for (auto tabWidget : d->tabWidgetList) {
        if (tabWidget->clearEOLAnnotation(fileName, title))
            break;
    }
}

void WorkspaceWidget::clearAllEOLAnnotation(const QString &title)
{
    for (auto tabWidget : d->tabWidgetList) {
        tabWidget->clearAllEOLAnnotation(title);
    }
}

void WorkspaceWidget::annotate(const QString &fileName, const QString &title, const QString &contents, int line, int type)
{
    for (auto tabWidget : d->tabWidgetList) {
        if (tabWidget->annotate(fileName, title, contents, line, type))
            break;
    }
}

void WorkspaceWidget::clearAnnotation(const QString &fileName, const QString &title)
{
    for (auto tabWidget : d->tabWidgetList) {
        if (tabWidget->clearAnnotation(fileName, title))
            break;
    }
}

void WorkspaceWidget::clearAllAnnotation(const QString &title)
{
    for (auto tabWidget : d->tabWidgetList) {
        tabWidget->clearAllAnnotation(title);
    }
}

void WorkspaceWidget::registerWidget(const QString &id, AbstractEditWidget *widget)
{
    d->registeredWidget.insert(id, widget);
    d->stackWidget->addWidget(widget);
}

void WorkspaceWidget::switchWidget(const QString &id)
{
    if (!d->registeredWidget.contains(id))
        return;

    d->stackWidget->setCurrentWidget(d->registeredWidget[id]);
}

void WorkspaceWidget::switchDefaultWidget()
{
    d->stackWidget->setCurrentIndex(0);
}

int WorkspaceWidget::backgroundMarkerDefine(const QString &fileName, const QColor &color, int defaultMarker)
{
    for (auto tabWidget : d->tabWidgetList) {
        int marker = tabWidget->backgroundMarkerDefine(fileName, color, defaultMarker);
        if (-1 != marker)
            return marker;
    }

    return -1;
}

void WorkspaceWidget::setRangeBackgroundColor(const QString &fileName, int startLine, int endLine, int marker)
{
    for (auto tabWidget : d->tabWidgetList) {
        if (tabWidget->setRangeBackgroundColor(fileName, startLine, endLine, marker))
            break;
    }
}

Edit::Range WorkspaceWidget::getBackgroundRange(const QString &fileName, int marker)
{
    Edit::Range range;
    for (auto tabWidget : d->tabWidgetList) {
        bool found = false;
        range = tabWidget->getBackgroundRange(fileName, marker, found);
        if (found)
            break;
    }

    return range;
}

void WorkspaceWidget::clearAllBackgroundColor(const QString &fileName, int marker)
{
    for (auto tabWidget : d->tabWidgetList) {
        if (tabWidget->clearAllBackground(fileName, marker))
            break;
    }
}

void WorkspaceWidget::showLineWidget(int line, QWidget *widget)
{
    if (auto tabWidget = d->currentTabWidget())
        tabWidget->showLineWidget(line, widget);
}

void WorkspaceWidget::closeLineWidget()
{
    if (auto tabWidget = d->currentTabWidget())
        tabWidget->closeLineWidget();
}

bool WorkspaceWidget::event(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate)
        d->fileCheckTimer.start();

    return QWidget::event(event);
}
