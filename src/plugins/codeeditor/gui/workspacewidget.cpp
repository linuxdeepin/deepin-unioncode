// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspacewidget.h"
#include "private/workspacewidget_p.h"
#include "transceiver/codeeditorreceiver.h"
#include "settings/editorsettings.h"
#include "settings/settingsdefine.h"
#include "base/abstractaction.h"

#include <DDialog>

#include <QVBoxLayout>
#include <QVariant>
#include <QApplication>
#include <QFileDialog>

using namespace dpfservice;
DWIDGET_USE_NAMESPACE

WorkspaceWidgetPrivate::WorkspaceWidgetPrivate(WorkspaceWidget *qq)
    : QObject(qq),
      q(qq)
{
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

    auto abstractCommentAction = new AbstractAction(commentAction, q);
    abstractCommentAction->setShortCutInfo("Editor.addAndRemoveComment",
                                           tr("Add/Remove Comment"), QKeySequence(Qt::Modifier::CTRL | Qt::Key_Slash));
    windowService->addAction(tr("&Add/Remove Comment"), abstractCommentAction);
    connect(commentAction, &QAction::triggered, this, &WorkspaceWidgetPrivate::handleSetComment);

    // show opened files
    QAction *showOpenedAction = new QAction(tr("Show opened files"), q);

    auto abstractShowOpenedAction = new AbstractAction(showOpenedAction, q);
    abstractShowOpenedAction->setShortCutInfo("Editor.showOpened",
                                              tr("Show opened files"), QKeySequence(Qt::CTRL | Qt::Key_Tab));
    windowService->addAction(tr("&Show open files"), abstractShowOpenedAction);
    connect(showOpenedAction, &QAction::triggered, this, &WorkspaceWidgetPrivate::handleShowOpenedFiles);
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

void WorkspaceWidgetPrivate::doSplit(QSplitter *spliter, int index, const QString &fileName, int pos, int scroll)
{
    TabWidget *tabWidget = new TabWidget(spliter);
    connectTabWidgetSignals(tabWidget);

    tabWidgetList.append(tabWidget);
    spliter->insertWidget(index, tabWidget);

    tabWidget->openFile(fileName);
    // Set the cursor and scroll position
    tabWidget->setEditorCursorPosition(pos);
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
    int cursorPos = tabWidgetSender->editorCursorPosition();
    int scroll = tabWidgetSender->editorScrollValue();

    if (spliter->count() == 1) {
        // Only one widget is added to the splitter,
        // change its orientation and add a new widget
        spliter->setOrientation(ori);
        doSplit(spliter, index + 1, fileName, cursorPos, scroll);
    } else if (spliter->orientation() == ori) {
        doSplit(spliter, index + 1, fileName, cursorPos, scroll);
    } else {
        // Use a new splitter to replace
        QSplitter *newSplitter = new QSplitter(q);
        newSplitter->setOrientation(ori);

        spliter->replaceWidget(index, newSplitter);
        newSplitter->addWidget(tabWidgetSender);
        doSplit(newSplitter, 1, fileName, cursorPos, scroll);
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
    }

    if (tabWidgetList.size() == 1)
        tabWidgetList.first()->setCloseButtonVisible(false);
}

void WorkspaceWidgetPrivate::handleOpenFile(const QString &workspace, const QString &fileName)
{
    Q_UNUSED(workspace)

    if (stackWidget->currentIndex() != 0)
        stackWidget->setCurrentIndex(0);

    if (auto tabWidget = currentTabWidget())
        tabWidget->openFile(fileName);
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
        if (!tabWidget || !tabWidget->hasFocus())
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

    if (!now)
        return;

    // the `now` is TextEditor
    auto tabWidget = qobject_cast<TabWidget *>(now->parentWidget());
    if (!tabWidget)
        return;

    focusTabWidget = tabWidget;
    editor.switchedFile(focusTabWidget->currentFile());
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
    auto tmp = files.toSet();
    return tmp.toList();
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

void WorkspaceWidget::setCompletion(const QString &info, const QIcon &icon, const QKeySequence &key)
{
    if (auto tabWidget = d->currentTabWidget())
        QMetaObject::invokeMethod(tabWidget, "setCompletion",
                                  Qt::QueuedConnection,
                                  Q_ARG(const QString &, info),
                                  Q_ARG(const QIcon &, icon),
                                  Q_ARG(const QKeySequence &, key));
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
    auto tmp = files.toSet();
    return tmp.toList();
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

TabWidget *WorkspaceWidget::currentTabWidget() const
{
    return d->currentTabWidget();
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

bool WorkspaceWidget::event(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate)
        d->fileCheckTimer.start();

    return QWidget::event(event);
}
