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
    completionView->setModel(completionModel);
    completionView->setFixedWidth(500);
    completionView->setVisible(false);

    completionExtWidget = new CodeCompletionExtendWidget(this);
    completionExtWidget->setFixedWidth(300);
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
    connect(completionModel, &CodeCompletionModel::modelReset, this, &CodeCompletionWidget::modelContentChanged);
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
    for (int row = 0; row < completionModel->rowCount(); ++row) {
        ++count;

        QModelIndex index(completionModel->index(row, 0));
        if (index.isValid())
            count += completionModel->rowCount(index);

        if (count > rows)
            return true;
    }

    return false;
}

void CodeCompletionWidget::modelContentChanged()
{
    if (!editor()->hasFocus())
        return;

    int realItemCount = completionModel->rowCount();
    if ((completionView->isHidden() || needShow) && realItemCount != 0) {
        needShow = false;
        completionView->setVisible(true);
        updateAndShow();
    }

    if (completionModel->rowCount() == 0) {
        completionView->setVisible(false);
        if (!completionExtWidget->isVisible())
            hide();
        else
            updateAndShow();
    } else {
        completionView->setCurrentIndex(completionModel->index(0, 0));
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
        if (completionModel->rowCount() == 0)
            completionView->setVisible(false);

        completionExtWidget->setVisible(true);
        updateAndShow();
    }
}

TextEditor *CodeCompletionWidget::editor() const
{
    return qobject_cast<TextEditor *>(const_cast<QObject *>(parent()));
}

bool CodeCompletionWidget::isCompletionActive() const
{
    return (!isHidden() && isVisible()) || (!completionView->isHidden() && completionView->isVisible());
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
        for (int row = 0; row < completionModel->rowCount(); ++row) {
            baseHeight += completionView->sizeHintForRow(row);

            QModelIndex index(completionModel->index(row, 0));
            if (index.isValid()) {
                for (int row2 = 0; row2 < completionModel->rowCount(index); ++row2) {
                    int h = 0;
                    for (int a = 0; a < completionModel->columnCount(index); ++a) {
                        const QModelIndex child = completionModel->index(row2, a, index);
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
    auto pos = editor()->cursorPosition();
    auto startPos = editor()->wordStartPositoin(pos);
    auto endPos = editor()->wordEndPosition(pos);

    completionModel->executeCompletionItem(editor(), startPos, endPos, index);
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

    if (!shouldStartCompletion(automaticInvocationLine))
        return;

    startCompletion();
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
        abortCompletion();
}
