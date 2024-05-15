// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gitdiffwidget.h"
#include "utils/diffutils.h"

#include <DGuiApplicationHelper>

#include <QTextBlock>
#include <QThread>
#include <QSplitter>
#include <QHBoxLayout>
#include <QPainter>
#include <QScrollBar>
#include <QSignalBlocker>

DGUI_USE_NAMESPACE

class GitDiffWidgetPrivate : public QObject
{
public:
    explicit GitDiffWidgetPrivate(GitDiffWidget *qq);
    ~GitDiffWidgetPrivate();

    void initUI();
    void initConnection();
    void showDiff();
    void updateTheme();

public:
    GitDiffWidget *q;

    GitDiffEditor *leftEditor { nullptr };
    GitDiffEditor *rightEditor { nullptr };
    QList<FileData> fileDataList;

    QSharedPointer<GitDiffWorker> worker { nullptr };
    QSharedPointer<QThread> thread { nullptr };
};

GitDiffWidgetPrivate::GitDiffWidgetPrivate(GitDiffWidget *qq)
    : q(qq),
      worker(new GitDiffWorker()),
      thread(new QThread())
{
}

GitDiffWidgetPrivate::~GitDiffWidgetPrivate()
{
    worker->stop();
    thread->quit();
    thread->wait();
}

void GitDiffWidgetPrivate::initUI()
{
    QHBoxLayout *layout = new QHBoxLayout(q);
    layout->setContentsMargins(0, 0, 0, 0);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, q);
    leftEditor = new GitDiffEditor(q);
    leftEditor->setReadOnly(true);
    leftEditor->setTextInteractionFlags(leftEditor->textInteractionFlags() | Qt::TextSelectableByKeyboard);

    rightEditor = new GitDiffEditor(q);
    rightEditor->setReadOnly(true);
    rightEditor->setTextInteractionFlags(rightEditor->textInteractionFlags() | Qt::TextSelectableByKeyboard);

    splitter->addWidget(leftEditor);
    splitter->addWidget(rightEditor);
    layout->addWidget(splitter);
}

void GitDiffWidgetPrivate::initConnection()
{
    q->connect(leftEditor->verticalScrollBar(), &QScrollBar::valueChanged,
               rightEditor->verticalScrollBar(), &QScrollBar::setValue);
    q->connect(rightEditor->verticalScrollBar(), &QScrollBar::valueChanged,
               leftEditor->verticalScrollBar(), &QScrollBar::setValue);

    q->connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &GitDiffWidgetPrivate::updateTheme);
    q->connect(q, &GitDiffWidget::reqParsePatch, worker.data(), &GitDiffWorker::handleParsePatch, Qt::QueuedConnection);
    q->connect(worker.data(), &GitDiffWorker::parsePatchFinished, q, &GitDiffWidget::onParsePathFinished, Qt::QueuedConnection);
    q->connect(q, &GitDiffWidget::reqParseFileDatas, worker.data(), &GitDiffWorker::handleParseFileDatas, Qt::QueuedConnection);
    q->connect(worker.data(), &GitDiffWorker::parseFileDatasFinished, q, &GitDiffWidget::onParseFileDatasFinished, Qt::QueuedConnection);

    worker->moveToThread(thread.data());
    thread->start();
}

void GitDiffWidgetPrivate::showDiff()
{
    Q_EMIT q->reqParseFileDatas(fileDataList, leftEditor->font());
}

void GitDiffWidgetPrivate::updateTheme()
{
    leftEditor->updateTheme();
    rightEditor->updateTheme();
    showDiff();
}

GitDiffWidget::GitDiffWidget(QWidget *parent)
    : GitBaseWidget(parent),
      d(new GitDiffWidgetPrivate(this))
{
    d->initUI();
    d->initConnection();
}

GitDiffWidget::~GitDiffWidget()
{
    delete d;
}

void GitDiffWidget::setGitInfo(const QStringList &infos)
{
    if (infos.isEmpty()) {
        d->leftEditor->setPlainText(tr("No difference."));
        d->rightEditor->setPlainText(tr("No difference."));
        return;
    }

    const auto &info = infos.join("");
    d->fileDataList.clear();
    Q_EMIT reqParsePatch(info);
}

void GitDiffWidget::setReadyMessage(const QString &msg)
{
    d->leftEditor->setPlainText(msg);
    d->rightEditor->setPlainText(msg);
}

void GitDiffWidget::onParsePathFinished()
{
    const auto &dataList = d->worker->getPatchResult();
    if (dataList.isEmpty()) {
        d->leftEditor->setPlainText(tr("No difference."));
        d->rightEditor->setPlainText(tr("No difference."));
    } else {
        d->leftEditor->setPlainText(tr("Waiting for data..."));
        d->rightEditor->setPlainText(tr("Waiting for data..."));

        d->fileDataList = dataList;
        d->showDiff();
    }
}

void GitDiffWidget::onParseFileDatasFinished(bool success)
{
    if (!success) {
        setReadyMessage(tr("Retrieving data failed."));
        return;
    }

    const auto &results = d->worker->getShowResults();
    d->leftEditor->setDiffData(results[LeftSide].diffData);
    d->rightEditor->setDiffData(results[RightSide].diffData);
    QTextDocument *leftDoc = results[LeftSide].textDocument->clone(d->leftEditor);
    leftDoc->setDocumentLayout(new QPlainTextDocumentLayout(leftDoc));
    leftDoc->setDefaultFont(d->leftEditor->font());
    QTextDocument *rightDoc = results[RightSide].textDocument->clone(d->rightEditor);
    rightDoc->setDocumentLayout(new QPlainTextDocumentLayout(rightDoc));
    rightDoc->setDefaultFont(d->rightEditor->font());
    {
        // TextDocument was living in no thread, so it's safe to pull it
        leftDoc->moveToThread(thread());
        rightDoc->moveToThread(thread());
        d->leftEditor->setDocument(leftDoc);
        d->rightEditor->setDocument(rightDoc);
    }

    d->leftEditor->setSelections(results[LeftSide].selections);
    d->rightEditor->setSelections(results[RightSide].selections);
}

GitDiffEditor::GitDiffEditor(QWidget *parent)
    : GitEditor(parent)
{
    updateTheme();
}

void GitDiffEditor::updateTheme()
{
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        fileLineForeground = palette().color(QPalette::Text);
        chunkLineForeground = palette().color(QPalette::Text);
    } else {
        fileLineForeground = Qt::black;
        chunkLineForeground = Qt::black;
    }
}

void GitDiffEditor::paintEvent(QPaintEvent *event)
{
    GitEditor::paintEvent(event);

    paintBlock(event->rect());
    paintSeparator(event->rect());
}

QString GitDiffEditor::skippedText(int skippedNumber) const
{
    if (skippedNumber > 0)
        return tr("Skipped %n lines...", nullptr, skippedNumber);
    if (skippedNumber == -2)
        return tr("Binary files differ");
    return tr("Skipped unknown number of lines...");
}

void GitDiffEditor::paintBlock(const QRect &eventRect) const
{
    QPainter painter(viewport());
    auto offset = contentOffset();
    painter.setBrushOrigin(offset);

    QTextBlock block = firstVisibleBlock();
    while (block.isValid()) {
        PaintEventBlockData blockData;
        blockData.boundingRect = blockBoundingRect(block).translated(offset);

        if (blockData.boundingRect.bottom() >= eventRect.top()
            && blockData.boundingRect.top() <= eventRect.bottom()) {
            blockData.position = block.position();
            blockData.length = block.length();

            paintBlock(&painter, block, offset, blockData.selections, eventRect);
        }

        offset.ry() += blockData.boundingRect.height();
        if (offset.y() > viewport()->rect().height())
            break;

        block = block.next();
    }
}

void GitDiffEditor::paintBlock(QPainter *painter,
                               const QTextBlock &block,
                               const QPointF &offset,
                               const QVector<QTextLayout::FormatRange> &selections,
                               const QRect &clipRect) const
{
    const int blockNumber = block.blockNumber();
    const QList<DiffSelection> diffs = diffSelections.value(blockNumber);

    QVector<QTextLayout::FormatRange> newSelections;
    for (const DiffSelection &diffSelection : diffs) {
        if (diffSelection.format.isValid()) {
            QTextLayout::FormatRange formatRange;
            formatRange.start = qMax(0, diffSelection.start);
            const int end = diffSelection.end < 0
                    ? block.text().count() + 1
                    : qMin(block.text().count(), diffSelection.end);

            formatRange.length = end - formatRange.start;
            formatRange.format = diffSelection.format;
            if (diffSelection.end < 0)
                formatRange.format.setProperty(QTextFormat::FullWidthSelection, true);
            newSelections.append(formatRange);
        }
    }

    newSelections += selections;
    block.layout()->draw(painter, offset, newSelections, clipRect);
}

void GitDiffEditor::paintSeparator(const QRect &eventRect) const
{
    QPainter painter(viewport());
    const QPointF offset = contentOffset();
    QTextBlock currentBlock = firstVisibleBlock();

    while (currentBlock.isValid()) {
        if (currentBlock.isVisible()) {
            qreal top = blockBoundingGeometry(currentBlock).translated(offset).top();
            qreal bottom = top + blockBoundingRect(currentBlock).height();

            if (top > eventRect.bottom())
                break;

            if (bottom >= eventRect.top()) {
                const int blockNumber = currentBlock.blockNumber();

                auto it = diffData.blockSkippedLines.constFind(blockNumber);
                if (it != diffData.blockSkippedLines.constEnd()) {
                    QString skippedRowsText = '[' + skippedText(it->first) + ']';
                    if (!it->second.isEmpty())
                        skippedRowsText += ' ' + it->second;
                    paintSeparator(painter, chunkLineForeground,
                                   skippedRowsText, currentBlock, top);
                }

                const DiffFileInfo fileInfo = diffData.diffFileInfo.value(blockNumber);
                if (!fileInfo.fileName.isEmpty()) {
                    const QString fileNameText = fileInfo.typeInfo.isEmpty()
                            ? fileInfo.fileName
                            : tr("[%1] %2").arg(fileInfo.typeInfo, fileInfo.fileName);
                    paintSeparator(painter, fileLineForeground,
                                   fileNameText, currentBlock, top);
                }
            }
        }
        currentBlock = currentBlock.next();
    }
}

void GitDiffEditor::paintSeparator(QPainter &painter, const QColor &color, const QString &text,
                                   const QTextBlock &block, int top) const
{
    QPointF offset = contentOffset();
    painter.save();

    QColor foreground = color;
    if (!foreground.isValid())
        foreground = palette().windowText().color();

    painter.setPen(foreground);
    int x = offset.x();
    if (x < document()->documentMargin()) {
        x = int(document()->documentMargin());
    }
    const QString elidedText = fontMetrics().elidedText(text,
                                                        Qt::ElideRight,
                                                        viewport()->width() - x);
    QTextLayout *layout = block.layout();
    QTextLine textLine = layout->lineAt(0);
    QRectF lineRect = textLine.naturalTextRect().translated(offset.x(), top);
    QRect clipRect = contentsRect();
    clipRect.setLeft(x);
    painter.setClipRect(clipRect);
    painter.drawText(QPointF(x, lineRect.top() + textLine.ascent()),
                     elidedText);
    painter.restore();
}
