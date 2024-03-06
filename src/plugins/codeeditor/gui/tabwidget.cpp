// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tabwidget.h"
#include "private/tabwidget_p.h"
#include "transceiver/codeeditorreceiver.h"
#include "common/common.h"

#include <QFileInfo>
#include <QDropEvent>
#include <QMimeData>
#include <QLabel>
#include <QScrollBar>

static constexpr int MAX_PRE_NEXT_TIMES = 30;

TabWidgetPrivate::TabWidgetPrivate(TabWidget *qq)
    : QObject(qq),
      q(qq)
{
}

void TabWidgetPrivate::initUI()
{
    q->setFocusPolicy(Qt::ClickFocus);
    q->setAcceptDrops(true);

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

    connect(EditorCallProxy::instance(), &EditorCallProxy::reqAddAnnotation, this, &TabWidgetPrivate::handleAddAnnotation);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqRemoveAnnotation, this, &TabWidgetPrivate::handleRemoveAnnotation);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqClearAllAnnotation, this, &TabWidgetPrivate::handleClearAllAnnotation);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqSetLineBackgroundColor, this, &TabWidgetPrivate::handleSetLineBackgroundColor);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqResetLineBackground, this, &TabWidgetPrivate::handleResetLineBackground);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqClearLineBackground, this, &TabWidgetPrivate::handleClearLineBackground);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqDoRename, this, &TabWidgetPrivate::handleDoRename);
}

TextEditor *TabWidgetPrivate::createEditor(const QString &fileName)
{
    TextEditor *editor = new TextEditor(q);
    editor->zoomTo(zoomValue);
    editor->updateLineNumberWidth(false);
    editor->installEventFilter(q);

    connect(editor, &TextEditor::zoomValueChanged, q, &TabWidget::zoomValueChanged);
    connect(editor, &TextEditor::cursorPositionChanged, this, &TabWidgetPrivate::onLinePositionChanged);
    connect(editor, &TextEditor::fileSaved, tabBar, &TabBar::onFileSaved);

    editor->setFile(fileName);
    editor->setCursorPosition(0, 0);
    connect(editor, &TextEditor::textChanged, this,
            [this, fileName] {
                onFileChanged(fileName);
            });

    editorMng.insert(fileName, editor);

    return editor;
}

TextEditor *TabWidgetPrivate::findEditor(const QString &fileName)
{
    return editorMng.value(fileName, nullptr);
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
    editor.keyPressEvent(event->key());
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

void TabWidgetPrivate::replaceRange(const QString &fileName, const newlsp::Range &range, const QString &text)
{
    auto editor = findEditor(fileName);
    if (editor) {
        editor->replaceRange(range.start.line, range.start.character,
                             range.end.line, range.end.character, text);
        return;
    }

    // Modify the file directly
    if (range.start.line != range.end.line) {
        qWarning() << "Failed, The start line is inconsistent with the end line";
        return;
    }

    QFile changeFile(fileName);
    QString cacheData;
    if (changeFile.open(QFile::ReadOnly)) {
        int i = 0;
        while (i != range.start.line) {
            cacheData += changeFile.readLine();
            i++;
        }
        QString changeLine = changeFile.readLine();
        int removeLength = range.end.character - range.start.character;
        changeLine = changeLine.replace(range.start.character, removeLength, text);
        cacheData += changeLine;
        QByteArray array = changeFile.readLine();
        while (!array.isEmpty()) {
            cacheData += array;
            array = changeFile.readLine();
        }
        changeFile.close();
    }

    if (changeFile.open(QFile::WriteOnly | QFile::Truncate)) {
        auto writeCount = changeFile.write(cacheData.toLatin1());
        if (writeCount != cacheData.size()) {
            qWarning() << "Failed, Write size does not match expectations."
                       << "Expectation: " << cacheData
                       << "Actual: " << writeCount;
        }
        changeFile.close();
    }
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
    if (!editorMng.contains(fileName))
        return;

    editorLayout->setCurrentWidget(editorMng[fileName]);
    changeFocusProxy();
}

void TabWidgetPrivate::onTabClosed(const QString &fileName)
{
    auto editor = findEditor(fileName);
    if (!editor)
        return;

    removePositionRecord(fileName);
    editorMng.remove(fileName);
    editorLayout->removeWidget(editor);
    changeFocusProxy();

    emit editor->fileClosed(fileName);
    editor->deleteLater();

    if (editorMng.isEmpty()) {
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

void TabWidgetPrivate::onFileChanged(const QString &fileName)
{
    auto editor = qobject_cast<TextEditor *>(sender());
    if (!editor)
        return;

    // The correct value cannot be immediately get by `editor->isModified()`
    QTimer::singleShot(50, tabBar, [this, editor, fileName] {
        tabBar->onFileChanged(fileName, editor->isModified());
    });
}

void TabWidgetPrivate::handleAddAnnotation(const QString &fileName, const QString &title, const QString &content, int line, AnnotationType type)
{
    if (auto editor = findEditor(fileName))
        editor->addAnnotation(title, content, line, type);
}

void TabWidgetPrivate::handleRemoveAnnotation(const QString &fileName, const QString &title)
{
    if (auto editor = findEditor(fileName))
        editor->removeAnnotation(title);
}

void TabWidgetPrivate::handleClearAllAnnotation(const QString &title)
{
    for (auto editor : editorMng)
        editor->removeAnnotation(title);
}

void TabWidgetPrivate::handleSetLineBackgroundColor(const QString &fileName, int line, const QColor &color)
{
    if (auto editor = findEditor(fileName))
        editor->setLineBackgroundColor(line, color);
}

void TabWidgetPrivate::handleResetLineBackground(const QString &fileName, int line)
{
    if (auto editor = findEditor(fileName))
        editor->resetLineBackgroundColor(line);
}

void TabWidgetPrivate::handleClearLineBackground(const QString &fileName)
{
    if (auto editor = findEditor(fileName))
        editor->clearLineBackgroundColor();
}

void TabWidgetPrivate::handleDoRename(const newlsp::WorkspaceEdit &info)
{
    if (info.changes) {
        auto changes = info.changes;
        auto itera = changes->begin();
        while (itera != changes->end()) {
            for (auto edit : itera->second) {
                QString filePath = QUrl(QString::fromStdString(itera->first)).toLocalFile();
                QString newText = QString::fromStdString(edit.newText);
                replaceRange(filePath, edit.range, newText);
            }
            itera++;
        }
    }
    if (info.documentChanges) {
        if (newlsp::any_contrast<std::vector<newlsp::TextDocumentEdit>>(info.documentChanges.value())) {
            std::vector<newlsp::TextDocumentEdit> documentChanges = std::any_cast<std::vector<newlsp::TextDocumentEdit>>(info.documentChanges.value());
            for (auto documentChange : documentChanges) {
                QString filePath = QUrl(QString::fromStdString(documentChange.textDocument.uri)).toLocalFile();
                if (!std::vector<newlsp::TextEdit>(documentChange.edits).empty()) {
                    auto edits = std::vector<newlsp::TextEdit>(documentChange.edits);
                    for (auto edit : edits) {
                        QString newText = QString::fromStdString(edit.newText);
                        replaceRange(filePath, edit.range, newText);
                    }
                } else if (!std::vector<newlsp::AnnotatedTextEdit>(documentChange.edits).empty()) {
                    auto edits = std::vector<newlsp::AnnotatedTextEdit>(documentChange.edits);
                    for (auto edit : edits) {
                        QString newText = QString::fromStdString(edit.newText);
                        replaceRange(filePath, edit.range, newText);
                    }
                }
            }
        }
    }
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

void TabWidget::showTips(const QString &tips)
{
    if (auto editor = d->currentTextEditor())
        editor->showTips(tips);
}

void TabWidget::insertText(const QString &text)
{
    if (auto editor = d->currentTextEditor())
        editor->insertText(text);
}

void TabWidget::undo()
{
    if (auto editor = d->currentTextEditor())
        editor->undo();
}

void TabWidget::gotoNextPosition()
{
    if (d->nextPosRecord.isEmpty())
        return;

    auto record = d->nextPosRecord.takeFirst();
    auto editor = d->findEditor(record.fileName);
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
    auto editor = d->findEditor(record.fileName);
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
    if (auto editor = d->findEditor(fileName))
        editor->addBreakpoint(line);
}

void TabWidget::removeBreakpoint(const QString &fileName, int line)
{
    if (auto editor = d->findEditor(fileName))
        editor->removeBreakpoint(line);
}

void TabWidget::clearAllBreakpoints()
{
    for (auto editor : d->editorMng.values())
        editor->clearAllBreakpoints();
}

int TabWidget::zoomValue()
{
    if (auto editor = d->currentTextEditor())
        return static_cast<int>(editor->SendScintilla(TextEditor::SCI_GETZOOM));

    return 0;
}

void TabWidget::setZoomValue(int value)
{
    d->zoomValue = value;
}

void TabWidget::updateZoomValue(int value)
{
    d->zoomValue = value;
    auto editor = d->currentTextEditor();
    if (editor->hasFocus())
        editor->updateLineNumberWidth(false);

    // update other editor
    for (auto other : d->editorMng.values()) {
        if (editor == other && editor->hasFocus())
            continue;

        // Disconnect first to prevent loop triggering
        disconnect(other, &TextEditor::zoomValueChanged, this, &TabWidget::zoomValueChanged);
        other->zoomTo(value);
        other->updateLineNumberWidth(false);
        connect(other, &TextEditor::zoomValueChanged, this, &TabWidget::zoomValueChanged);
    }
}

void TabWidget::openFile(const QString &fileName)
{
    if (!QFile::exists(fileName) || QFileInfo(fileName).isDir())
        return;

    if (d->findEditor(fileName)) {
        d->tabBar->switchTab(fileName);
        return;
    }

    // add file monitor
    Inotify::globalInstance()->addPath(fileName);

    d->tabBar->setFileName(fileName);
    TextEditor *editor = d->createEditor(fileName);

    d->editorLayout->addWidget(editor);
    d->editorLayout->setCurrentWidget(editor);
    d->changeFocusProxy();

    if (!d->editorMng.isEmpty())
        setSplitButtonVisible(true);
}

void TabWidget::setDebugLine(int line)
{
    if (auto editor = d->currentTextEditor())
        editor->setDebugLine(line);
}

void TabWidget::removeDebugLine()
{
    for (auto editor : d->editorMng)
        editor->removeDebugLine();
}

void TabWidget::gotoLine(int line)
{
    if (auto editor = d->currentTextEditor())
        editor->gotoLine(line);
}

void TabWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (!event->mimeData()->hasUrls())
        return event->ignore();

    const QMimeData *mimeData = event->mimeData();
    const auto &urlList = mimeData->urls();
    if (QFileInfo(urlList[0].toLocalFile()).isDir())
        return event->ignore();

    event->acceptProposedAction();
}

void TabWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        const auto &urlList = mimeData->urls();
        for (const auto &url : urlList)
            openFile(url.toLocalFile());
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
