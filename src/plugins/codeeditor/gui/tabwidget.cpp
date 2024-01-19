// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tabwidget.h"
#include "private/tabwidget_p.h"
#include "common/common.h"

#include <QFileInfo>
#include <QDropEvent>
#include <QMimeData>
#include <QLabel>
#include <QScrollBar>

static constexpr int MAX_PRE_NEXT_TIMES = 30;

TabWidgetPrivate::TabWidgetPrivate(TabWidget *qq)
    : QObject(qq),
      q(qq),
      editorMng(new TextEditorManager(this))
{
}

void TabWidgetPrivate::initUI()
{
    q->setFocusPolicy(Qt::ClickFocus);

    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    // TODO: space page
    QLabel *spaceWidget = new QLabel(tr("This is a blank page"), q);
    spaceWidget->setAlignment(Qt::AlignCenter);
    spaceWidget->setBackgroundRole(QPalette::Dark);

    editorLayout = new QStackedLayout();
    editorLayout->setSpacing(0);
    editorLayout->setMargin(0);
    editorLayout->addWidget(spaceWidget);

    tabBar = new TabBar(q);
    mainLayout->addWidget(tabBar, 0, Qt::AlignTop);
    mainLayout->addLayout(editorLayout);
}

void TabWidgetPrivate::initConnection()
{
    connect(tabBar, &TabBar::tabSwitched, this, &TabWidgetPrivate::onTabSwitched);
    connect(tabBar, &TabBar::tabClosed, this, &TabWidgetPrivate::onTabClosed);
    connect(tabBar, &TabBar::spliterClicked, this, &TabWidgetPrivate::onSpliterClicked);
    connect(tabBar, &TabBar::closeRequested, q, &TabWidget::closeRequested);
}

TextEditor *TabWidgetPrivate::currentTextEditor() const
{
    return qobject_cast<TextEditor *>(editorLayout->currentWidget());
}

void TabWidgetPrivate::changeFocusProxy()
{
    auto editor = currentTextEditor();
    q->setFocusProxy(editor);
    tabBar->setFocusProxy(editor);

    // need to set the proxy first,
    // otherwise `setFocus` will not work
    q->setFocus();
}

bool TabWidgetPrivate::processKeyPressEvent(QKeyEvent *event)
{
    switch (event->modifiers()) {
    case Qt::ControlModifier: {
        switch (event->key()) {
        case Qt::Key_S:
            doSave();
            return true;
        }
    } break;
    }

    return false;
}

void TabWidgetPrivate::doSave()
{
    if (auto editor = currentTextEditor())
        editor->save();
}

void TabWidgetPrivate::removePositionRecord(const QString &fileName)
{
    auto iter = std::remove_if(prePosRecord.begin(), prePosRecord.end(),
                               [=](const PosRecord &record) {
                                   return record.fileName == fileName;
                               });
    prePosRecord.erase(iter, prePosRecord.end());

    iter = std::remove_if(nextPosRecord.begin(), nextPosRecord.end(),
                          [=](const PosRecord &record) {
                              return record.fileName == fileName;
                          });
    nextPosRecord.erase(iter, nextPosRecord.end());
}

void TabWidgetPrivate::onTabSwitched(const QString &fileName)
{
    if (!editorIndexHash.contains(fileName))
        return;

    editorLayout->setCurrentIndex(editorIndexHash[fileName]);
    changeFocusProxy();
}

void TabWidgetPrivate::onTabClosed(const QString &fileName)
{
    auto editor = editorMng->findEditor(fileName);
    if (!editor)
        return;

    removePositionRecord(fileName);
    editorIndexHash.remove(fileName);
    editorLayout->removeWidget(editor);
    changeFocusProxy();

    emit editor->fileClosed(fileName);
    editor->deleteLater();

    if (editorIndexHash.isEmpty()) {
        q->setSplitButtonVisible(false);
        emit q->closeRequested();
    }
}

void TabWidgetPrivate::onSpliterClicked(Qt::Orientation ori)
{
    const auto &fileName = tabBar->currentFileName();
    emit q->splitRequested(ori, fileName);
}

void TabWidgetPrivate::onLinePositionChanged(int line, int index)
{
    auto editor = qobject_cast<TextEditor *>(sender());
    if (!editor)
        return;

    int pos = editor->positionFromLineIndex(line, index);

    if (curPosRecord.fileName == editor->getFile() && curPosRecord.pos == pos)
        return;

    prePosRecord.append({ pos, editor->getFile() });
    if (prePosRecord.size() >= MAX_PRE_NEXT_TIMES)
        prePosRecord.takeFirst();
}

TabWidget::TabWidget(QWidget *parent)
    : QWidget(parent),
      d(new TabWidgetPrivate(this))
{
    d->initUI();
    d->initConnection();
}

void TabWidget::setCloseButtonVisible(bool visible)
{
    d->tabBar->setCloseButtonVisible(visible);
}

void TabWidget::setSplitButtonVisible(bool visible)
{
    d->tabBar->setSplitButtonVisible(visible);
}

QString TabWidget::selectedText() const
{
    if (auto editor = d->currentTextEditor())
        return editor->selectedText();

    return "";
}

QString TabWidget::cursorBeforeText() const
{
    if (auto editor = d->currentTextEditor())
        return editor->cursorBeforeText();

    return "";
}

QString TabWidget::cursorBehindText() const
{
    if (auto editor = d->currentTextEditor())
        return editor->cursorBehindText();

    return "";
}

void TabWidget::replaceSelectedText(const QString &text)
{
    if (auto editor = d->currentTextEditor())
        editor->replaceSelectedText(text);
}

void TabWidget::gotoNextPosition()
{
    if (d->nextPosRecord.isEmpty())
        return;

    auto record = d->nextPosRecord.takeFirst();
    auto editor = d->editorMng->findEditor(record.fileName);
    if (!editor)
        return;

    d->prePosRecord.append(record);
    d->tabBar->switchTab(record.fileName);
    editor->gotoPosition(record.pos);
    d->curPosRecord = record;
}

void TabWidget::gotoPreviousPosition()
{
    if (d->prePosRecord.size() <= 1)
        return;

    auto record = d->prePosRecord.takeLast();
    d->nextPosRecord.push_front(record);
    if (d->nextPosRecord.size() >= MAX_PRE_NEXT_TIMES)
        d->nextPosRecord.takeLast();

    record = d->prePosRecord.last();
    auto editor = d->editorMng->findEditor(record.fileName);
    if (!editor)
        return;

    d->tabBar->switchTab(record.fileName);
    editor->gotoPosition(record.pos);
    d->curPosRecord = record;
}

void TabWidget::setEditorCursorPosition(int pos)
{
    if (auto editor = d->currentTextEditor())
        editor->gotoPosition(pos);
}

int TabWidget::editorCursorPosition()
{
    if (auto editor = d->currentTextEditor())
        return editor->cursorPosition();

    return 0;
}

void TabWidget::setEditorScrollValue(int value)
{
    if (auto editor = d->currentTextEditor())
        editor->verticalScrollBar()->setValue(value);
}

int TabWidget::editorScrollValue()
{
    if (auto editor = d->currentTextEditor())
        return editor->verticalScrollBar()->value();

    return 0;
}

void TabWidget::addBreakpoint(const QString &fileName, int line)
{
    if (auto editor = d->editorMng->findEditor(fileName))
        editor->addBreakpoint(line);
}

void TabWidget::removeBreakpoint(const QString &fileName, int line)
{
    if (auto editor = d->editorMng->findEditor(fileName))
        editor->removeBreakpoint(line);
}

void TabWidget::clearAllBreakpoints()
{
    d->editorMng->clearAllBreakpoints();
}

void TabWidget::openFile(const QString &fileName)
{
    if (!QFile::exists(fileName))
        return;

    if (d->editorMng->findEditor(fileName)) {
        d->tabBar->switchTab(fileName);
        return;
    }

    // add file monitor
    Inotify::globalInstance()->addPath(fileName);

    d->tabBar->setFileName(fileName);
    TextEditor *editor = d->editorMng->createEditor(this, fileName);
    editor->installEventFilter(this);
    editor->setCursorPosition(0, 0);
    connect(editor, &TextEditor::cursorPositionChanged, d.data(), &TabWidgetPrivate::onLinePositionChanged);
    connect(editor, &TextEditor::fileSaved, d->tabBar, &TabBar::onFileSaved);
    connect(editor, &TextEditor::textChanged, d->tabBar,
            [this, fileName] {
                d->tabBar->onFileChanged(fileName);
            },
            Qt::UniqueConnection);

    int index = d->editorLayout->addWidget(editor);
    d->editorLayout->setCurrentIndex(index);
    d->changeFocusProxy();

    if (d->editorIndexHash.isEmpty())
        setSplitButtonVisible(true);
    d->editorIndexHash.insert(fileName, index);
}

void TabWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void TabWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        const auto &urlList = mimeData->urls();
        const auto &fileName = urlList.at(0).toLocalFile();
        if (!fileName.isEmpty())
            openFile(fileName);
    }
}

bool TabWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() != QEvent::KeyPress)
        return false;

    auto editor = qobject_cast<TextEditor *>(obj);
    if (!editor)
        return false;

    return d->processKeyPressEvent(static_cast<QKeyEvent *>(event));
}
