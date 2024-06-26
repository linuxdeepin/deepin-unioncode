// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codelensdelegate.h"
#include "codelenstype.h"

#include <DGuiApplicationHelper>
#include <QApplication>

const int LinePadding = 4;
const int LineDigits = 6;

DGUI_USE_NAMESPACE

static std::pair<int, QString> lineNumberInfo(const QStyleOptionViewItem &option,
                                              const QModelIndex &index)
{
    int line = index.data(CodeLensItemRole::LineRole).toInt();
    if (line < 1)
        return { 0, {} };

    const QString lineText = QString::number(line + 1);
    const int lineDigits = qMax(LineDigits, lineText.size());
    const int fontWidth = option.fontMetrics.horizontalAdvance(QString(lineDigits, QLatin1Char('0')));
    const QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    return { LinePadding + fontWidth + LinePadding + style->pixelMetric(QStyle::PM_FocusFrameHMargin), lineText };
}

static QString itemText(const QModelIndex &index)
{
    QString text = index.data(Qt::DisplayRole).toString();
    if (index.model()->hasChildren(index))
        text += " (" + QString::number(index.model()->rowCount(index)) + ')';

    return text;
}

CodeLensDelegate::CodeLensDelegate(QObject *parent)
    : QItemDelegate(parent)
{
    tabStr = QString(8, ' ');
}

QSize CodeLensDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QItemDelegate::sizeHint(option, index);
    size.setHeight(24);
    return size;
}

void CodeLensDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    painter->save();
    const LayoutInfo info = getLayoutInfo(option, index);
    painter->setFont(info.option.font);
    QItemDelegate::drawBackground(painter, info.option, index);

    // line numbers
    drawLineNumber(painter, info.option, info.lineNumberRect, index);

    // text
    drawText(painter, info.option, info.textRect, index);
    QItemDelegate::drawFocus(painter, info.option, info.option.rect);
    painter->restore();
}

LayoutInfo CodeLensDelegate::getLayoutInfo(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto op = option;
    if (!option.state.testFlag(QStyle::State_HasFocus)) {
        QColor color;
        DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType
                ? color = Qt::black
                : color = QColor("#c5c8c9");
        op.palette.setColor(QPalette::Text, color);
    }

    LayoutInfo info;
    info.option = setOptions(index, op);

    // text
    info.textRect = info.option.rect;

    // do basic layout
    QRect checkRect, iconRect;
    doLayout(info.option, &checkRect, &iconRect, &info.textRect, false);

    // adapt for line numbers
    const int lineNumberWidth = lineNumberInfo(info.option, index).first;
    info.lineNumberRect = info.textRect;
    info.lineNumberRect.setWidth(lineNumberWidth);
    info.textRect.adjust(lineNumberWidth, 0, 0, 0);
    return info;
}

int CodeLensDelegate::drawLineNumber(QPainter *painter, const QStyleOptionViewItem &option,
                                     const QRect &rect, const QModelIndex &index) const
{
    const bool isSelected = option.state & QStyle::State_Selected;
    const std::pair<int, QString> numberInfo = lineNumberInfo(option, index);
    if (numberInfo.first == 0)
        return 0;

    QRect lineNumberAreaRect(rect);
    lineNumberAreaRect.setWidth(numberInfo.first);

    QPalette::ColorGroup cg = QPalette::Normal;
    if (!(option.state & QStyle::State_Active))
        cg = QPalette::Inactive;
    else if (!(option.state & QStyle::State_Enabled))
        cg = QPalette::Disabled;

    painter->fillRect(lineNumberAreaRect,
                      QBrush(isSelected
                                 ? option.palette.brush(cg, QPalette::Highlight)
                                 : option.palette.color(cg, QPalette::Base).darker(111)));

    QStyleOptionViewItem opt = option;
    opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
    opt.palette.setColor(cg, QPalette::Text, Qt::darkGray);

    const QStyle *style = QApplication::style();
    const int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, nullptr) + 1;

    const QRect rowRect = lineNumberAreaRect.adjusted(-textMargin, 0, textMargin - LinePadding, 0);
    QItemDelegate::drawDisplay(painter, opt, rowRect, numberInfo.second);

    return numberInfo.first;
}

void CodeLensDelegate::drawText(QPainter *painter, const QStyleOptionViewItem &option,
                                const QRect &rect, const QModelIndex &index) const
{
    const auto text = itemText(index);
    const int termStart = index.data(CodeLensItemRole::TermStartRole).toInt();
    const auto termEnd = index.data(CodeLensItemRole::TermEndRole).toInt();
    int termLength = termEnd - termStart;
    if (termStart < 0 || termStart >= text.length() || termLength < 1) {
        QItemDelegate::drawDisplay(painter,
                                   option,
                                   rect,
                                   QString(text).replace(QLatin1Char('\t'), tabStr));
        return;
    }

    // clip searchTermLength to end of line
    termLength = qMin(termLength, text.length() - termStart);
    const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    const QString textBefore = text.left(termStart).replace(QLatin1Char('\t'), tabStr);
    const QString textHighlight = text.mid(termStart, termLength).replace(QLatin1Char('\t'), tabStr);
    const QString textAfter = text.mid(termStart + termLength).replace(QLatin1Char('\t'), tabStr);
    int searchTermStartPixels = option.fontMetrics.horizontalAdvance(textBefore);
    int searchTermLengthPixels = option.fontMetrics.horizontalAdvance(textHighlight);
    int textAfterLengthPixels = option.fontMetrics.horizontalAdvance(textAfter);

    // rects
    QRect beforeHighlightRect(rect);
    beforeHighlightRect.setRight(beforeHighlightRect.left() + searchTermStartPixels);

    QRect resultHighlightRect(rect);
    resultHighlightRect.setLeft(beforeHighlightRect.right());
    resultHighlightRect.setRight(resultHighlightRect.left() + searchTermLengthPixels);

    QRect afterHighlightRect(rect);
    afterHighlightRect.setLeft(resultHighlightRect.right());
    afterHighlightRect.setRight(afterHighlightRect.left() + textAfterLengthPixels);

    // paint all highlight backgrounds
    bool isSelected = option.state & QStyle::State_Selected;
    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? QPalette::Normal
                                  : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
        cg = QPalette::Inactive;
    QStyleOptionViewItem baseOption = option;
    baseOption.state &= ~QStyle::State_Selected;
    if (isSelected) {
        painter->fillRect(beforeHighlightRect.adjusted(textMargin, 0, textMargin, 0),
                          option.palette.brush(cg, QPalette::Highlight));
        painter->fillRect(afterHighlightRect.adjusted(textMargin, 0, textMargin, 0),
                          option.palette.brush(cg, QPalette::Highlight));
    }

    QColor highlightBackground("#ffef0b");
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        highlightBackground = QColor("#8a7f2c");
    painter->fillRect(resultHighlightRect.adjusted(textMargin, 0, textMargin - 1, 0),
                      QBrush(highlightBackground));

    // Text before the highlighting
    QColor textColor = baseOption.palette.color(QPalette::Text);
    QStyleOptionViewItem noHighlightOpt = baseOption;
    noHighlightOpt.rect = beforeHighlightRect;
    noHighlightOpt.textElideMode = Qt::ElideNone;
    if (isSelected)
        noHighlightOpt.palette.setColor(QPalette::Text, noHighlightOpt.palette.color(cg, QPalette::HighlightedText));
    QItemDelegate::drawDisplay(painter, noHighlightOpt, beforeHighlightRect, textBefore);

    // Highlight text
    QStyleOptionViewItem highlightOpt = noHighlightOpt;
    highlightOpt.palette.setColor(QPalette::Text, textColor);
    QItemDelegate::drawDisplay(painter, highlightOpt, resultHighlightRect, textHighlight);

    // Text after the Highlight
    noHighlightOpt.rect = afterHighlightRect;
    QItemDelegate::drawDisplay(painter, noHighlightOpt, afterHighlightRect, textAfter);
}
