// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tabwidget.h"
#include "private/tabwidget_p.h"
#include "transceiver/codeeditorreceiver.h"
#include "common/common.h"
#include "recent/recentopenwidget.h"
#include "symbol/symbolwidget.h"

#include "services/window/windowservice.h"

#include <DFrame>
#include <DGuiApplicationHelper>
#ifdef DTKWIDGET_CLASS_DPaletteHelper
#    include <DPaletteHelper>
#endif

#include <QAction>
#include <QFileInfo>
#include <QDropEvent>
#include <QMimeData>
#include <QLabel>
#include <QScrollBar>

static constexpr int MAX_PRE_NEXT_TIMES = 30;

using namespace dpfservice;

class KeyLabel : public DFrame
{
public:
    KeyLabel(const QString &key, QWidget *parent = nullptr)
        : DFrame(parent)
    {
        label = new DLabel(key, this);
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(layout->contentsMargins().left(), 0, layout->contentsMargins().right(), 0);
        layout->setSpacing(0);
        label->setForegroundRole(QPalette::ButtonText);
        layout->addWidget(label);
    }

    void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event)
        QStyleOptionFrame opt;
        initStyleOption(&opt);
        QPainter p(this);
        drawShadow(&p, event->rect() - contentsMargins(), QColor(0, 0, 0, 20));

        opt.features |= QStyleOptionFrame::Rounded;

#ifdef DTKWIDGET_CLASS_DPaletteHelper
        const DPalette &dp = DPaletteHelper::instance()->palette(this);
#else
        const DPalette &dp = DGuiApplicationHelper::instance()->applicationPalette();
#endif

        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            p.setBackground(QColor(255, 255, 255));
        } else {
            QColor bgColor(109, 109, 109);
            if ((opt.state & QStyle::State_Active) == 0) {
                auto inactive_mask_color = dp.color(QPalette::Window);
                inactive_mask_color.setAlphaF(0.6);
                bgColor = DGuiApplicationHelper::blendColor(bgColor, inactive_mask_color);
            }
            p.setBackground(bgColor);
        }

        p.setPen(QPen(dp.frameBorder(), opt.lineWidth));
        style()->drawControl(QStyle::CE_ShapedFrame, &opt, &p, this);
    }

    void drawShadow(QPainter *p, const QRect &rect, const QColor &color) const
    {
        DStyle dstyle;
        int frame_radius = dstyle.pixelMetric(DStyle::PM_FrameRadius);
        int shadow_xoffset = dstyle.pixelMetric(DStyle::PM_ShadowHOffset);
        int shadow_yoffset = dstyle.pixelMetric(DStyle::PM_ShadowVOffset);

        QRect shadow = rect;
        QPoint pointOffset(rect.center().x() + shadow_xoffset, rect.center().y() + shadow_yoffset);
        shadow.moveCenter(pointOffset);

        p->setBrush(color);
        p->setPen(Qt::NoPen);
        p->setRenderHint(QPainter::Antialiasing);
        p->drawRoundedRect(shadow, frame_radius, frame_radius);
    }

private:
    DLabel *label { nullptr };
};

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
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    auto spaceWidget = createSpaceWidget();

    editorLayout = new QStackedLayout();
    editorLayout->setSpacing(0);
    editorLayout->setMargin(0);
    editorLayout->addWidget(spaceWidget);

    tabBar = new TabBar(q);
    symbolBar = new SymbolBar(q);
    symbolBar->setVisible(false);

    mainLayout->addWidget(tabBar, 0, Qt::AlignTop);
    mainLayout->addWidget(symbolBar, 0, Qt::AlignTop);
    mainLayout->addLayout(editorLayout, 1);

    openedWidget = new RecentOpenWidget(q);
    openedWidget->hide();

    auto holder = createFindPlaceHolder();
    if (holder)
        mainLayout->addWidget(holder);
}

void TabWidgetPrivate::initConnection()
{
    connect(tabBar, &TabBar::tabSwitched, this, &TabWidgetPrivate::onTabSwitched);
    connect(tabBar, &TabBar::tabClosed, this, &TabWidgetPrivate::onTabClosed);
    connect(tabBar, &TabBar::spliterClicked, this, &TabWidgetPrivate::onSpliterClicked);
    connect(tabBar, &TabBar::closeRequested, q, &TabWidget::closeRequested);
    connect(tabBar, &TabBar::saveFileRequested, q, &TabWidget::saveFile);

    connect(EditorCallProxy::instance(), &EditorCallProxy::reqAddAnnotation, this, &TabWidgetPrivate::handleAddAnnotation);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqRemoveAnnotation, this, &TabWidgetPrivate::handleRemoveAnnotation);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqClearAllAnnotation, this, &TabWidgetPrivate::handleClearAllAnnotation);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqSetLineBackgroundColor, this, &TabWidgetPrivate::handleSetLineBackgroundColor);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqResetLineBackground, this, &TabWidgetPrivate::handleResetLineBackground);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqClearLineBackground, this, &TabWidgetPrivate::handleClearLineBackground);
    connect(EditorCallProxy::instance(), &EditorCallProxy::reqDoRename, this, &TabWidgetPrivate::handleDoRename);
}

void TabWidget::handleSetComment()
{
    if (auto editor = d->currentTextEditor())
        editor->commentOperation();
}

void TabWidget::handleShowOpenedFiles(const int &x, const int &y, const QSize &size)
{
    int count = d->tabBar->tabCount();
    if (count < 2)
        return;

    d->openedWidget->setWindowFlags(Qt::Popup);
    QSize popupSize = d->openedWidget->size();

    int posX = (size.width() - popupSize.width()) / 2 + x;
    int posY = (size.height() - popupSize.height()) / 2 + y;

    d->openedWidget->move(posX, posY);
    d->openedWidget->setOpenedFiles(d->recentOpenedFiles);
    d->openedWidget->setListViewSelection(1);
    connect(d->openedWidget, &RecentOpenWidget::triggered, this, [=](const QModelIndex &index) {
        d->tabBar->setCurrentIndex(d->tabBar->indexOf(index.data(RecentOpenWidget::RecentOpenedUserRole::FilePathRole).toString()));
    });
    d->openedWidget->show();
    d->openedWidget->setFocusListView();
}

QWidget *TabWidgetPrivate::createSpaceWidget()
{
    QWidget *widget = new QWidget(q);
    QVBoxLayout *vLayout = new QVBoxLayout(widget);
    vLayout->setSpacing(5);

    QLabel *titleLabel = new QLabel(tr("File Operation"), q);
    titleLabel->setAlignment(Qt::AlignCenter);
    auto font = titleLabel->font();
    font.setPointSize(font.pointSize() * 1.5);
    titleLabel->setFont(font);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->setSpacing(10);
    auto addCommandLine = [this, gridLayout](const QString &id) {
        auto cmd = ActionManager::instance()->command(id);
        if (!cmd || !cmd->action())
            return;

        int row = gridLayout->rowCount();
        gridLayout->addWidget(new QLabel(cmd->action()->text(), q), row, 0, Qt::AlignRight);
        const auto &keyList = cmd->keySequences();
        for (int i = 0; i < keyList.size(); ++i) {
            auto key = keyList[i];
            gridLayout->addWidget(new KeyLabel(key.toString(), q), row, i + 1, Qt::AlignLeft);
        }
    };

    addCommandLine(A_OPEN_FILE);
    addCommandLine("Find.findInDocument");
    addCommandLine("Editor.close");
    addCommandLine("locator.EnterCommand");
    addCommandLine("Editor.addAndRemoveComment");

    vLayout->addStretch(1);
    vLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    vLayout->addLayout(gridLayout);
    vLayout->addStretch(1);

    return widget;
}

QWidget *TabWidgetPrivate::createFindPlaceHolder()
{
    docFind = new EditorDocumentFind(q);
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService)
        return nullptr;

    return windowService->createFindPlaceHolder(q, docFind);
}

TextEditor *TabWidgetPrivate::createEditor(const QString &fileName, QsciDocument *doc)
{
    TextEditor *editor = new TextEditor(q);
    editor->updateLineNumberWidth(false);
    editor->installEventFilter(q);

    connect(editor, &TextEditor::zoomValueChanged, q, &TabWidget::zoomValueChanged);
    connect(editor, &TextEditor::cursorRecordChanged, this, &TabWidgetPrivate::onCursorRecordChanged);
    connect(editor, &TextEditor::requestOpenFiles, this, &TabWidgetPrivate::handleOpenFiles);
    connect(editor, &TextEditor::cursorPositionChanged, symbolBar, &SymbolBar::updateSymbol);
    connect(editor, &TextEditor::modificationChanged, tabBar, std::bind(&TabBar::onModificationChanged, tabBar, fileName, std::placeholders::_1));

    if (doc) {
        editor->openFileWithDocument(fileName, *doc);
    } else {
        editor->openFile(fileName);
    }

    editor->setCursorPosition(0, 0);
    editorMng.insert(fileName, editor);
    recentOpenedFiles.prepend(fileName);

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
        q->saveFile(editor->getFile());
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

    SymbolWidgetGenerator::instance()->symbolWidget()->setEditor(editorMng[fileName]);
    symbolBar->setPath(fileName);
    editorLayout->setCurrentWidget(editorMng[fileName]);
    recentOpenedFiles.removeOne(fileName);
    recentOpenedFiles.prepend(fileName);
    changeFocusProxy();
}

void TabWidgetPrivate::onTabClosed(const QString &fileName)
{
    auto editor = findEditor(fileName);
    if (!editor)
        return;

    Inotify::globalInstance()->removePath(fileName);
    removePositionRecord(fileName);
    editorMng.remove(fileName);
    recentOpenedFiles.removeOne(fileName);
    editorLayout->removeWidget(editor);
    changeFocusProxy();

    emit editor->fileClosed(fileName);
    editor->deleteLater();

    if (editorMng.isEmpty()) {
        symbolBar->clear();
        symbolBar->setVisible(false);
        q->setSplitButtonVisible(false);
        emit q->closeRequested();
    }
}

void TabWidgetPrivate::onSpliterClicked(Qt::Orientation ori)
{
    const auto &fileName = tabBar->currentFileName();
    emit q->splitRequested(ori, fileName);
}

void TabWidgetPrivate::onCursorRecordChanged(int pos)
{
    auto editor = qobject_cast<TextEditor *>(sender());
    if (!editor)
        return;

    if (curPosRecord.fileName == editor->getFile() && curPosRecord.pos == pos)
        return;

    prePosRecord.append({ pos, editor->getFile() });
    for (const auto &record : nextPosRecord) {
        prePosRecord.removeOne(record);
    }
    nextPosRecord.clear();
    if (prePosRecord.size() >= MAX_PRE_NEXT_TIMES)
        prePosRecord.takeFirst();
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

void TabWidgetPrivate::handleOpenFiles(const QList<QUrl> &fileList)
{
    for (const auto &file : fileList) {
        QFileInfo info(file.toLocalFile());
        if (info.isDir())
            continue;

        q->openFile(info.absoluteFilePath());
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

QString TabWidget::currentFile() const
{
    return d->tabBar->currentFileName();
}

QString TabWidget::currentDocumentContent() const
{
    if (auto editor = d->currentTextEditor())
        return editor->text();

    return "";
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

QStringList TabWidget::modifiedFiles() const
{
    QStringList files;
    for (auto editor : d->editorMng.values()) {
        if (!editor->isModified())
            continue;

        files << editor->getFile();
    }

    return files;
}

QStringList TabWidget::openedFiles() const
{
    QStringList files;
    for (auto editor : d->editorMng.values())
        files << editor->getFile();

    return files;
}

void TabWidget::setText(const QString &text)
{
    if (auto editor = d->currentTextEditor()) {
        int pos = editor->cursorPosition();
        editor->setText(text);
        editor->gotoPosition(pos);
    }
}

QString TabWidget::fileText(const QString &fileName, bool *success)
{
    if (auto editor = d->findEditor(fileName)) {
        if (success) *success = true;
        return editor->text();
    }

    if (success) *success = false;
    return {};
}

void TabWidget::replaceAll(const QString &fileName, const QString &oldText,
                           const QString &newText, bool caseSensitive, bool wholeWords)
{
    if (auto editor = d->findEditor(fileName))
        d->docFind->replaceAll(editor, oldText, newText, caseSensitive, wholeWords);
}

void TabWidget::replaceRange(const QString &fileName, int line, int index, int length, const QString &after)
{
    if (auto editor = d->findEditor(fileName))
        editor->replaceRange(line, index, line, index + length, after);
}

void TabWidget::saveAll() const
{
    for (auto editor : d->editorMng.values()) {
        Inotify::globalInstance()->removePath(editor->getFile());
        editor->save();
        Inotify::globalInstance()->addPath(editor->getFile());
    }
}

bool TabWidget::saveAs(const QString &from, const QString &to)
{
    if (auto editor = d->findEditor(from)) {
        editor->saveAs(to);
        return true;
    }

    return false;
}

void TabWidget::reloadFile(const QString &fileName)
{
    if (auto editor = d->findEditor(fileName))
        editor->reload();
}

void TabWidget::setFileModified(const QString &fileName, bool isModified)
{
    if (auto editor = d->findEditor(fileName)) {
        editor->setModified(isModified);
        if (isModified)
            emit editor->textChanged();
    }
}

void TabWidget::closeFileEditor(const QString &fileName)
{
    d->tabBar->removeTab(fileName);
}

void TabWidget::closeFileEditor()
{
    if (auto editor = d->currentTextEditor())
        closeFileEditor(editor->getFile());
}

void TabWidget::switchHeaderSource()
{
    auto editor = d->currentTextEditor();
    if (!editor || !editor->hasFocus())
        return;

    editor->switchHeaderSource();
}

void TabWidget::followSymbolUnderCursor()
{
    auto editor = d->currentTextEditor();
    if (!editor || !editor->hasFocus())
        return;

    editor->followSymbolUnderCursor();
}

void TabWidget::findUsage()
{
    auto editor = d->currentTextEditor();
    if (!editor || !editor->hasFocus())
        return;

    editor->findUsage();
}

void TabWidget::renameSymbol()
{
    auto editor = d->currentTextEditor();
    if (!editor || !editor->hasFocus())
        return;

    editor->renameSymbol();
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

void TabWidget::setCompletion(const QString &info, const QIcon &icon, const QKeySequence &key)
{
    if (auto editor = d->currentTextEditor())
        editor->setCompletion(info, icon, key);
}

void TabWidget::gotoNextPosition()
{
    if (d->nextPosRecord.isEmpty())
        return;

    auto record = d->nextPosRecord.takeFirst();
    auto editor = d->findEditor(record.fileName);
    if (!editor)
        return;

    d->curPosRecord = record;
    d->prePosRecord.append(record);
    d->tabBar->switchTab(record.fileName);
    editor->gotoPosition(record.pos);
}

void TabWidget::gotoPreviousPosition()
{
    if (d->prePosRecord.isEmpty())
        return;

    auto editor = d->currentTextEditor();
    if (!editor)
        return;

    TabWidgetPrivate::PosRecord editRecord { editor->cursorPosition(), editor->getFile() };
    auto record = d->prePosRecord.last();
    if (record == editRecord) {
        if (d->prePosRecord.size() <= 1)
            return;
        d->prePosRecord.removeLast();
    } else {
        record = editRecord;
    }

    d->nextPosRecord.push_front(record);
    if (d->nextPosRecord.size() >= MAX_PRE_NEXT_TIMES)
        d->nextPosRecord.takeLast();

    record = d->prePosRecord.last();
    editor = d->findEditor(record.fileName);
    if (!editor)
        return;

    d->curPosRecord = record;
    d->tabBar->switchTab(record.fileName);
    editor->gotoPosition(record.pos);
}

void TabWidget::setEditorCursorPosition(int pos)
{
    if (auto editor = d->currentTextEditor())
        editor->gotoPosition(pos);
}

int TabWidget::editorCursorPosition()
{
    if (auto editor = d->currentTextEditor())
        return editor->cursorLastPosition();

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

void TabWidget::addBreakpoint(const QString &fileName, int line, bool enabled)
{
    if (auto editor = d->findEditor(fileName))
        editor->addBreakpoint(line, enabled);
}

void TabWidget::removeBreakpoint(const QString &fileName, int line)
{
    if (auto editor = d->findEditor(fileName))
        editor->removeBreakpoint(line);
}

void TabWidget::setBreakpointEnabled(const QString &fileName, int line, bool enabled)
{
    if (auto editor = d->findEditor(fileName))
        editor->setBreakpointEnabled(line, enabled);
}

void TabWidget::toggleBreakpoint()
{
    auto editor = d->currentTextEditor();
    if (editor && editor->hasFocus())
        editor->toggleBreakpoint();
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

void TabWidget::updateZoomValue(int value)
{
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

TextEditor *TabWidget::currentEditor() const
{
    return d->currentTextEditor();
}

TextEditor *TabWidget::findEditor(const QString &fileName)
{
    return d->findEditor(fileName);
}

void TabWidget::openFile(const QString &fileName, QsciDocument *doc)
{
    if (!QFile::exists(fileName) || QFileInfo(fileName).isDir())
        return;

    if (d->findEditor(fileName)) {
        d->tabBar->switchTab(fileName);
        return;
    }

    // add file monitor
    Inotify::globalInstance()->addPath(fileName);

    d->symbolBar->setPath(fileName);
    d->symbolBar->setVisible(true);
    d->tabBar->setFileName(fileName);
    TextEditor *editor = d->createEditor(fileName, doc);

    SymbolWidgetGenerator::instance()->symbolWidget()->setEditor(editor);
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

void TabWidget::gotoPosition(int line, int column)
{
    if (auto editor = d->currentTextEditor())
        editor->gotoPosition(editor->positionFromLineIndex(line, column));
}

void TabWidget::saveFile(const QString &fileName)
{
    if (auto editor = d->findEditor(fileName)) {
        Inotify::globalInstance()->removePath(fileName);
        editor->save();
        Inotify::globalInstance()->addPath(fileName);
    }
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
