// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "basehighlighter.h"
#include "constants.h"

#include <QTextDocument>
#include <QTextBlock>
#include <QTextCursor>

#include <cmath>

static bool adjustRange(QTextLayout::FormatRange &range, int from, int charsDelta)
{
    if (range.start >= from) {
        range.start += charsDelta;
        return true;
    } else if (range.start + range.length > from) {
        range.length += charsDelta;
        return true;
    }
    return false;
}

class BaseHighlighterPrivate
{
public:
    enum HighlightProperty {
        SyntaxHighlight = QTextFormat::UserProperty + 1
    };

    explicit BaseHighlighterPrivate(BaseHighlighter *qq);

    void reformatBlocks(int from, int charsRemoved, int charsAdded);
    void reformatBlock(const QTextBlock &block, int from, int charsRemoved, int charsAdded);
    void rehighlight(QTextCursor &cursor, QTextCursor::MoveOperation operation);
    void applyFormatChanges(int from, int charsRemoved, int charsAdded);

    using FormatRangeVec = QVector<QTextLayout::FormatRange>;
    std::tuple<FormatRangeVec, FormatRangeVec> partition(const FormatRangeVec &container);

public:
    BaseHighlighter *q;

    QTextDocument *doc { nullptr };
    bool rehighlightWorking { false };
    bool reformatBlocksWorking { false };
    QVector<QTextCharFormat> formatChanges;
    QTextBlock currentBlock;
};

BaseHighlighterPrivate::BaseHighlighterPrivate(BaseHighlighter *qq)
    : q(qq)
{
}

void BaseHighlighterPrivate::reformatBlocks(int from, int charsRemoved, int charsAdded)
{
    rehighlightWorking = false;
    QTextBlock block = doc->findBlock(from);
    if (!block.isValid())
        return;

    int endPosition;
    QTextBlock lastBlock = doc->findBlock(from + charsAdded + (charsRemoved > 0 ? 1 : 0));
    if (lastBlock.isValid())
        endPosition = lastBlock.position() + lastBlock.length();
    else
        endPosition = doc->lastBlock().position() + doc->lastBlock().length();

    bool forceHighlightOfNextBlock = false;
    while (block.isValid() && (block.position() < endPosition || forceHighlightOfNextBlock)) {
        const int stateBeforeHighlight = block.userState();
        reformatBlock(block, from, charsRemoved, charsAdded);
        forceHighlightOfNextBlock = (block.userState() != stateBeforeHighlight);
        block = block.next();
    }

    formatChanges.clear();
}

void BaseHighlighterPrivate::reformatBlock(const QTextBlock &block, int from, int charsRemoved, int charsAdded)
{
    Q_ASSERT_X(!currentBlock.isValid(), "BaseHighlighterPrivate::reformatBlock()", "reFormatBlock() called recursively");

    currentBlock = block;

    formatChanges.fill(QTextCharFormat(), block.length() - 1);
    q->highlightBlock(block.text());
    applyFormatChanges(from, charsRemoved, charsAdded);

    currentBlock = QTextBlock();
}

void BaseHighlighterPrivate::rehighlight(QTextCursor &cursor, QTextCursor::MoveOperation operation)
{
    reformatBlocksWorking = true;
    int from = cursor.position();
    cursor.movePosition(operation);
    reformatBlocks(from, 0, cursor.position() - from);
    reformatBlocksWorking = false;
}

void BaseHighlighterPrivate::applyFormatChanges(int from, int charsRemoved, int charsAdded)
{
    bool formatsChanged = false;
    QTextLayout *layout = currentBlock.layout();
    QVector<QTextLayout::FormatRange> ranges;
    QVector<QTextLayout::FormatRange> oldRanges;
    std::tie(oldRanges, ranges) = partition(layout->formats());

    if (currentBlock.contains(from)) {
        const int charsDelta = charsAdded - charsRemoved;
        for (QTextLayout::FormatRange &range : ranges)
            formatsChanged |= adjustRange(range, from - currentBlock.position(), charsDelta);
    }

    QTextCharFormat emptyFormat;

    QTextLayout::FormatRange r;

    QVector<QTextLayout::FormatRange> newRanges;
    int i = 0;
    while (i < formatChanges.count()) {
        while (i < formatChanges.count() && formatChanges.at(i) == emptyFormat)
            ++i;

        if (i >= formatChanges.count())
            break;

        r.start = i;
        r.format = formatChanges.at(i);
        while (i < formatChanges.count() && formatChanges.at(i) == r.format)
            ++i;

        r.format.setProperty(SyntaxHighlight, true);
        r.length = i - r.start;
        const QString preeditText = currentBlock.layout()->preeditAreaText();
        if (!preeditText.isEmpty()) {
            const int preeditPosition = currentBlock.layout()->preeditAreaPosition();
            if (r.start >= preeditPosition) {
                r.start += preeditText.length();
            } else if (r.start + r.length > preeditPosition) {
                QTextLayout::FormatRange beforePreeditRange = r;
                r.start = preeditPosition + preeditText.length();
                r.length = r.length - (r.start - preeditPosition);
                beforePreeditRange.length = preeditPosition - beforePreeditRange.start;
                newRanges << beforePreeditRange;
            }
        }

        newRanges << r;
    }

    formatsChanged = formatsChanged || (newRanges.size() != oldRanges.size());
    for (int i = 0; !formatsChanged && i < newRanges.size(); ++i) {
        const QTextLayout::FormatRange &o = oldRanges.at(i);
        const QTextLayout::FormatRange &n = newRanges.at(i);
        formatsChanged = (o.start != n.start || o.length != n.length || o.format != n.format);
    }

    if (formatsChanged) {
        ranges.append(newRanges);
        layout->setFormats(ranges);
        doc->markContentsDirty(currentBlock.position(), currentBlock.length());
    }
}

std::tuple<BaseHighlighterPrivate::FormatRangeVec, BaseHighlighterPrivate::FormatRangeVec>
BaseHighlighterPrivate::partition(const FormatRangeVec &container)
{
    auto needHighlight = [](const QTextLayout::FormatRange &range) {
        return range.format.property(SyntaxHighlight).toBool();
    };

    FormatRangeVec hit;
    hit.reserve(container.size());
    FormatRangeVec miss;
    miss.reserve(container.size());

    for (const auto &i : container) {
        if (needHighlight(i))
            hit.push_back(i);
        else
            miss.push_back(i);
    }

    return std::make_tuple(hit, miss);
}

BaseHighlighter::BaseHighlighter(QObject *parent)
    : QObject(parent),
      d(new BaseHighlighterPrivate(this))
{
}

BaseHighlighter::BaseHighlighter(QTextDocument *doc)
    : QObject(doc),
      d(new BaseHighlighterPrivate(this))
{
    if (doc)
        setDocument(doc);
}

void BaseHighlighter::setDocument(QTextDocument *doc)
{
    if (d->doc) {
        disconnect(d->doc, &QTextDocument::contentsChange, this, &BaseHighlighter::reformatBlocks);

        QTextCursor cursor(d->doc);
        cursor.beginEditBlock();
        for (QTextBlock blk = d->doc->begin(); blk.isValid(); blk = blk.next())
            blk.layout()->clearFormats();
        cursor.endEditBlock();
    }

    d->doc = doc;
    if (d->doc) {
        connect(d->doc, &QTextDocument::contentsChange, this, &BaseHighlighter::reformatBlocks);
        d->rehighlightWorking = true;
        QMetaObject::invokeMethod(this, &BaseHighlighter::rehighlight, Qt::QueuedConnection);
    }
}

QTextDocument *BaseHighlighter::document() const
{
    return d->doc;
}

QList<QColor> BaseHighlighter::generateColors(int n, const QColor &background)
{
    QList<QColor> result;
    const double oneThird = 1.0 / 3.0;
    const int step = qRound(std::ceil(std::pow(double(n), oneThird)));
    result.reserve(step * step * step);
    const int factor = 255 / step;
    const int half = factor / 2;
    const int bgRed = background.red();
    const int bgGreen = background.green();
    const int bgBlue = background.blue();
    for (int r = step; r >= 0; --r) {
        const int red = r * factor;
        if (bgRed - half > red || bgRed + half <= red) {
            for (int g = step; g >= 0; --g) {
                const int green = g * factor;
                if (bgGreen - half > green || bgGreen + half <= green) {
                    for (int b = step; b >= 0; --b) {
                        const int blue = b * factor;
                        if (bgBlue - half > blue || bgBlue + half <= blue)
                            result.append(QColor(red, green, blue));
                    }
                }
            }
        }
    }

    return result;
}

void BaseHighlighter::rehighlight()
{
    if (!d->rehighlightWorking)
        return;

    d->rehighlightWorking = false;
    if (!d->doc)
        return;

    QTextCursor cursor(d->doc);
    d->rehighlight(cursor, QTextCursor::End);
}

void BaseHighlighter::reformatBlocks(int from, int charsRemoved, int charsAdded)
{
    if (d->reformatBlocksWorking)
        return;

    d->reformatBlocks(from, charsRemoved, charsAdded);
}

void BaseHighlighter::highlightBlock(const QString &text)
{
}

void BaseHighlighter::setFormatWithSpaces(const QString &text, int start, int count, const QTextCharFormat &format)
{
    const QTextCharFormat visualSpaceFormat = whitespacified(format);
    const int end = std::min(start + count, int(text.length()));
    int index = start;

    while (index != end) {
        const bool isSpace = text.at(index).isSpace();
        const int startIndex = index;
        do {
            ++index;
        } while (index != end && text.at(index).isSpace() == isSpace);

        const int tokenLength = index - startIndex;
        if (isSpace)
            setFormat(startIndex, tokenLength, visualSpaceFormat);
        else if (format.isValid())
            setFormat(startIndex, tokenLength, format);
    }
}

QTextCharFormat BaseHighlighter::whitespacified(const QTextCharFormat &fmt)
{
    QTextCharFormat format;
    format.setBackground(fmt.background());
    return format;
}

void BaseHighlighter::setFormat(int start, int count, const QTextCharFormat &format)
{
    if (start < 0 || start >= d->formatChanges.count())
        return;

    const int end = qMin(start + count, d->formatChanges.count());
    for (int i = start; i < end; ++i)
        d->formatChanges[i] = format;
}
