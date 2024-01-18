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
    auto editor = d->currentTextEditor();
    if (!editor)
        return "";

    return d->currentTextEditor()->selectedText();
}

QString TabWidget::cursorBeforeText() const
{
    auto editor = d->currentTextEditor();
    if (!editor)
        return "";

    return editor->cursorBeforeText();
}

QString TabWidget::cursorBehindText() const
{
    auto editor = d->currentTextEditor();
    if (!editor)
        return "";

    return editor->cursorBehindText();
}

void TabWidget::replaceSelectedText(const QString &text)
{
    auto editor = d->currentTextEditor();
    if (!editor)
        return;

    return editor->replaceSelectedText(text);
}

void TabWidget::setEditorCursorPosition(int pos)
{
    auto editor = d->currentTextEditor();
    if (!editor)
        return;

    editor->gotoPosition(pos);
}

int TabWidget::editorCursorPosition()
{
    auto editor = d->currentTextEditor();
    if (!editor)
        return 0;

    return editor->cursorPosition();
}

void TabWidget::setEditorScrollValue(int value)
{
    auto editor = d->currentTextEditor();
    if (!editor)
        return;

    editor->verticalScrollBar()->setValue(value);
}

int TabWidget::editorScrollValue()
{
    auto editor = d->currentTextEditor();
    if (!editor)
        return 0;

    return editor->verticalScrollBar()->value();
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
    editor->setCursorPosition(0, 0);
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

void TabWidget::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void TabWidget::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
}

void TabWidget::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
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
