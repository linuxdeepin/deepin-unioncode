// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codecompletionwidget.h"
#include "codecompletionmodel.h"
#include "codecompletionview.h"
#include "codecompletionextendwidget.h"

#include "gui/texteditor.h"
#include "Qsci/qscilexer.h"

#include <QVBoxLayout>
#include <QScrollBar>
#include <QApplication>

CodeCompletionWidget::CodeCompletionWidget(TextEditor *parent)
    : QFrame(parent, Qt::ToolTip)
{
    initUI();
    initConnection();
}

void CodeCompletionWidget::initUI()
{
    setFocusPolicy(Qt::ClickFocus);
    setFrameStyle(QFrame::Box | QFrame::Raised);
    setLineWidth(1);

    automaticInvocationTimer = new QTimer(this);
    automaticInvocationTimer->setSingleShot(true);
    automaticInvocationTimer->setInterval(300);

    completionView = new CodeCompletionView(this);
    completionModel = new CodeCompletionModel(this);
    proxyModel = new CompletionSortFilterProxyModel(this);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->sort(0);
    proxyModel->setSourceModel(completionModel);
    completionView->setModel(proxyModel);
    completionView->setFixedWidth(500);
    completionView->setVisible(false);

    completionExtWidget = new CodeCompletionExtendWidget(this);
    completionExtWidget->setFixedWidth(500);
    completionExtWidget->setVisible(false);
    completionExtWidget->setTextEditor(editor());

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(completionView);
    layout->addWidget(completionExtWidget);
}

void CodeCompletionWidget::initConnection()
{
    connect(completionView, &CodeCompletionView::doubleClicked, this, &CodeCompletionWidget::execute);
    connect(proxyModel, &CodeCompletionModel::modelReset, this, [this] {
        proxyModel->setFilterRegExp(filterString());
        modelContentChanged();
    });
    connect(proxyModel, &CodeCompletionModel::layoutChanged, this, &CodeCompletionWidget::modelContentChanged);
    connect(completionExtWidget, &CodeCompletionExtendWidget::completionChanged, this, &CodeCompletionWidget::onCompletionChanged);
    connect(automaticInvocationTimer, &QTimer::timeout, this, &CodeCompletionWidget::automaticInvocation);

    connect(editor(), &TextEditor::textAdded, this, &CodeCompletionWidget::onTextAdded);
    connect(editor(), &TextEditor::textRemoved, this, &CodeCompletionWidget::onTextRemoved);
    connect(editor(), &TextEditor::focusOut, this, &CodeCompletionWidget::viewFocusOut);
    connect(editor(), &TextEditor::cursorPositionChanged, this, &CodeCompletionWidget::cursorPositionChanged);
    connect(editor()->verticalScrollBar(), &QScrollBar::valueChanged, this, [this] {
        abortCompletion();
    });
}

bool CodeCompletionWidget::shouldStartCompletion(const QString &insertedText)
{
    if (insertedText.isEmpty())
        return false;

    QChar lastChar = insertedText.at(insertedText.count() - 1);
    if (lastChar.isLetter() || lastChar.isNumber() || lastChar == QLatin1Char('_'))
        return true;

    if (editor()->lexer()) {
        auto spList = editor()->lexer()->autoCompletionWordSeparators();
        auto iter = std::find_if(spList.begin(), spList.end(),
                                 [&insertedText](const QString &sp) {
                                     return insertedText.endsWith(sp);
                                 });
        return iter != spList.end();
    }

    return false;
}

void CodeCompletionWidget::updateAndShow()
{
    if (!editor()->hasFocus())
        return;

    setUpdatesEnabled(false);

    updateHeight();
    updatePosition(true);

    setUpdatesEnabled(true);

    show();
}

bool CodeCompletionWidget::hasAtLeastNRows(int rows)
{
    int count = 0;
    for (int row = 0; row < proxyModel->rowCount(); ++row) {
        ++count;

        QModelIndex index(proxyModel->index(row, 0));
        if (index.isValid())
            count += proxyModel->rowCount(index);

        if (count > rows)
            return true;
    }

    return false;
}

QString CodeCompletionWidget::filterString()
{
    const auto &word = editor()->wordAtPosition(editor()->cursorPosition());
    if (word.isEmpty())
        return {};

    auto range = completionModel->range();
    if (range.start.line == -1 || range.start.character == -1)
        return {};

    int pos = editor()->wordStartPositoin(editor()->cursorPosition());
    if (pos != editor()->positionFromLineIndex(range.start.line, range.start.character))
        return {};

    int startPos = editor()->positionFromLineIndex(range.start.line, range.start.character);
    return editor()->text(startPos, automaticInvocationAt);
}

bool CodeCompletionWidget::isFunctionKind(int kind)
{
    return kind == lsp::CompletionItem::Function || kind == lsp::CompletionItem::Method
            || kind == lsp::CompletionItem::Constructor;
}

void CodeCompletionWidget::executeCompletionItem(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= proxyModel->rowCount())
        return;

    auto srcIndex = proxyModel->mapToSource(index);
    auto item = completionModel->item(srcIndex);
    if (!item)
        return;

    if (!item->textEdit.newText.isEmpty())
        executeWithTextEdit(item);
    else
        executeWithoutTextEdit(item);
}

void CodeCompletionWidget::executeWithTextEdit(lsp::CompletionItem *item)
{
    int labelOpenParenOffset = item->label.indexOf('(');
    int labelClosingParenOffset = item->label.indexOf(')');
    bool isMacroCall = item->kind == lsp::CompletionItem::Text && labelOpenParenOffset != -1
            && labelClosingParenOffset > labelOpenParenOffset;
    bool isFunctionLike = isFunctionKind(item->kind) || isMacroCall;
    QString rawInsertText = item->textEdit.newText;
    if (isFunctionLike && !rawInsertText.contains('(')) {
        if (labelOpenParenOffset != -1) {
            // function takes no arguments
            if (labelClosingParenOffset == labelOpenParenOffset + 1)
                rawInsertText += "()";
            else   // function takes arguments
                rawInsertText += "( )";
        }
    }

    int firstParenOffset = rawInsertText.indexOf('(');
    int lastParenOffset = rawInsertText.lastIndexOf(')');
    QString textToBeInserted = rawInsertText.left(firstParenOffset);
    QString extraCharacters;
    int cursorOffset = 0;
    if (isFunctionLike) {
        extraCharacters += '(';
        // If the function takes no arguments, automatically place the closing parenthesis
        if (firstParenOffset + 1 == lastParenOffset) {
            extraCharacters += QLatin1Char(')');
        } else {
            extraCharacters += ')';
            --cursorOffset;
        }
    }

    int curLine = 0, curIndex = 0;
    editor()->lineIndexFromPosition(editor()->cursorPosition(), &curLine, &curIndex);
    textToBeInserted += extraCharacters;
    auto range = item->textEdit.range;
    editor()->replaceRange(range.start.line, range.start.character,
                           curLine, curIndex, textToBeInserted);
    if (cursorOffset) {
        editor()->lineIndexFromPosition(editor()->cursorPosition(), &curLine, &curIndex);
        editor()->setCursorPosition(curLine, curIndex + cursorOffset);
    }
}

void CodeCompletionWidget::executeWithoutTextEdit(lsp::CompletionItem *item)
{
    const int pos = editor()->cursorPosition();
    const QString textToInsert(item->insertText);
    int length = 0;
    for (auto it = textToInsert.crbegin(), end = textToInsert.crend(); it != end; ++it) {
        char character = editor()->SendScintilla(TextEditor::SCI_GETCHARAT, pos - length - 1);
        if (it->toLower() != QChar(character).toLower()) {
            length = 0;
            break;
        }
        ++length;
    }

    int line = editor()->SendScintilla(TextEditor::SCI_LINEFROMPOSITION, pos);
    int lineStartPos = editor()->SendScintilla(TextEditor::SCI_POSITIONFROMLINE, line);
    const auto &text = editor()->text(lineStartPos, pos);
    static QRegularExpression identifier("[a-zA-Z_][a-zA-Z0-9_]*$");
    QRegularExpressionMatch match = identifier.match(text);
    int matchLength = match.hasMatch() ? match.capturedLength(0) : 0;
    length = qMax(length, matchLength);
    editor()->replaceRange(pos - length, pos, textToInsert);
}

void CodeCompletionWidget::modelContentChanged()
{
    if (!editor()->hasFocus())
        return;

    if ((completionView->isHidden() || needShow) && proxyModel->rowCount() != 0) {
        needShow = false;
        completionView->setVisible(true);
        updateAndShow();
    }

    if (proxyModel->rowCount() == 0) {
        completionView->setVisible(false);
        if (!completionExtWidget->isVisible())
            hide();
        else
            updateAndShow();
    } else {
        updateHeight();
        completionView->setCurrentIndex(proxyModel->index(0, 0));
    }
}

void CodeCompletionWidget::onCompletionChanged()
{
    if (!editor()->hasFocus())
        return;

    if (!completionExtWidget->isValid()) {
        completionExtWidget->setVisible(false);
        if (completionView->isHidden())
            hide();
    } else {
        if (proxyModel->rowCount() == 0)
            completionView->setVisible(false);

        if (!completionExtWidget->isVisible()) {
            completionExtWidget->setVisible(true);
            updateAndShow();
        }
    }
}

TextEditor *CodeCompletionWidget::editor() const
{
    return qobject_cast<TextEditor *>(const_cast<QObject *>(parent()));
}

bool CodeCompletionWidget::isCompletionActive() const
{
    return isVisible();
}

void CodeCompletionWidget::startCompletion()
{
    needShow = true;

    completionModel->completionInvoked(editor(), automaticInvocationAt);
}

void CodeCompletionWidget::updateHeight()
{
    QRect geom = geometry();

    constexpr int minBaseHeight = 10;
    constexpr int maxBaseHeight = 300;

    int baseHeight = 0;
    if (hasAtLeastNRows(15)) {
        baseHeight = maxBaseHeight;
    } else {
        // Calculate size-hints to determine the best height
        for (int row = 0; row < proxyModel->rowCount(); ++row) {
            baseHeight += completionView->sizeHintForRow(row);

            QModelIndex index(proxyModel->index(row, 0));
            if (index.isValid()) {
                for (int row2 = 0; row2 < proxyModel->rowCount(index); ++row2) {
                    int h = 0;
                    for (int a = 0; a < proxyModel->columnCount(index); ++a) {
                        const QModelIndex child = proxyModel->index(row2, a, index);
                        int localHeight = completionView->sizeHintForIndex(child).height();
                        if (localHeight > h)
                            h = localHeight;
                    }

                    baseHeight += h;
                    if (baseHeight > maxBaseHeight)
                        break;
                }

                if (baseHeight > maxBaseHeight)
                    break;
            }
        }
    }

    baseHeight += 2 * frameWidth();

    if (completionView->horizontalScrollBar()->isVisible())
        baseHeight += completionView->horizontalScrollBar()->height();

    if (baseHeight < minBaseHeight)
        baseHeight = minBaseHeight;

    if (baseHeight > maxBaseHeight) {
        baseHeight = maxBaseHeight;
        completionView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    } else {
        // Somewhere there seems to be a bug that makes QTreeView add a scroll-bar
        // even if the content exactly fits in. So forcefully disable the scroll-bar in that case
        completionView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    if (baseHeight < 10) {
        completionView->resize(completionView->width(), height() - 2 * frameWidth());
        return;
    }

    geom.setHeight(baseHeight);
    if (geometry() != geom)
        setGeometry(geom);

    QSize entryListSize = QSize(completionView->width(), baseHeight - 2 * frameWidth());
    if (completionView->size() != entryListSize)
        completionView->resize(entryListSize);
}

void CodeCompletionWidget::updatePosition(bool force)
{
    if (!force && !isCompletionActive())
        return;

    auto cursorPosition = editor()->pointFromPosition(editor()->cursorPosition());
    if (cursorPosition == QPoint(-1, -1)) {
        abortCompletion();
        return;
    }

    QPoint p = editor()->mapToGlobal(cursorPosition);
    p.setY(p.y() + editor()->fontMetrics().height() + 2);
    move(p);
}

void CodeCompletionWidget::setCompletion(const QString &info, const QIcon &icon, const QKeySequence &key)
{
    completionExtWidget->setCompletionInfo(info, icon, key);
}

bool CodeCompletionWidget::processKeyPressEvent(QKeyEvent *event)
{
    if (!isCompletionActive())
        return false;

    if (completionExtWidget->processEvent(event)) {
        abortCompletion();
        return true;
    }

    switch (event->key()) {
    case Qt::Key_Tab:
    case Qt::Key_Enter:
    case Qt::Key_Return:
        return execute();
    case Qt::Key_Down:
        if (!completionView->isVisible())
            return false;
        completionView->nextCompletion();
        return true;
    case Qt::Key_Up:
        if (!completionView->isVisible())
            return false;
        completionView->previousCompletion();
        return true;
    case Qt::Key_Escape:
        abortCompletion();
        return true;
    default:
        break;
    }

    return false;
}

bool CodeCompletionWidget::execute()
{
    if (!isCompletionActive())
        return false;

    auto index = completionView->currentIndex();
    if (!index.isValid()) {
        abortCompletion();
        return false;
    }

    isCompletionInput = true;
    executeCompletionItem(index);
    abortCompletion();
    isCompletionInput = false;
    return true;
}

void CodeCompletionWidget::abortCompletion()
{
    if (hasFocus()) {
        editor()->activateWindow();
        editor()->setFocus();
    }

    completionModel->clear();
    if (isVisible())
        hide();
}

void CodeCompletionWidget::automaticInvocation()
{
    if (automaticInvocationAt != editor()->cursorPosition())
        return;

    if (!shouldStartCompletion(automaticInvocationLine)) {
        completionModel->clear();
        return;
    }

    const auto &word = editor()->wordAtPosition(editor()->cursorPosition());
    if (!completionView->isVisible() || completionModel->rowCount() == 0 || word.isEmpty()) {
        startCompletion();
    } else if (!word.isEmpty()) {
        int pos = editor()->wordStartPositoin(editor()->cursorPosition());
        const auto &range = completionModel->range();
        if (pos != editor()->positionFromLineIndex(range.start.line, range.start.character))
            startCompletion();
    }
}

void CodeCompletionWidget::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    abortCompletion();
}

void CodeCompletionWidget::onTextAdded(int pos, int len, int added, const QString &text, int line)
{
    Q_UNUSED(line)

    if (isCompletionInput)
        return;

    if (!editor()->isAutomaticInvocationEnabled() || added != 0) {
        automaticInvocationLine.clear();
        automaticInvocationTimer->stop();
        return;
    }

    if (pos != automaticInvocationAt)
        automaticInvocationLine.clear();

    automaticInvocationLine += text;
    automaticInvocationAt = pos + len;

    if (automaticInvocationLine.isEmpty()) {
        automaticInvocationTimer->stop();
        return;
    }

    automaticInvocationTimer->start();
}

void CodeCompletionWidget::onTextRemoved(int, int, int, const QString &, int)
{
    automaticInvocationLine.clear();
    automaticInvocationTimer->stop();
}

void CodeCompletionWidget::viewFocusOut()
{
    if (QApplication::focusWidget() != this)
        abortCompletion();
}

void CodeCompletionWidget::cursorPositionChanged()
{
    if (!isCompletionActive())
        return;

    if (editor()->cursorPosition() != automaticInvocationAt)
        return abortCompletion();

    QString filter = filterString();
    if (filter.isEmpty())
        return;

    proxyModel->setFilterRegExp(filter);
    proxyModel->invalidate();
}
