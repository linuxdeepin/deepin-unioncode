// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codelensdelegate.h"
#include "codelenstype.h"

#include <QApplication>

CodeLensDelegate::CodeLensDelegate(QObject *parent)
    : QStyledItemDelegate (parent)
    , characterStart(-1)
    , characterEnd(-1)
    , color(QColor(Qt::yellow))
{
    color.setAlpha(50);
}

void CodeLensDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    QString displayName = index.data().toString();
    QFont font = painter->font();
    QFontMetrics fontMetrics(font);
    if (index.parent().isValid()){
        lsp::Range range;
        QString codeText;
        QColor heightColor;
        QVariant rangeVar = index.data(CodeLensItemRole::Range);
        if (rangeVar.canConvert<lsp::Range>()) {
            range = rangeVar.value<lsp::Range>();
        }
        QVariant codeTextVar = index.data(CodeLensItemRole::CodeText);
        if (codeTextVar.canConvert<QString>()) {
            codeText = codeTextVar.value<QString>();
        }
        QVariant colorVar = index.data(CodeLensItemRole::HeightColor);
        if (colorVar.canConvert<QColor>()) {
            heightColor = colorVar.value<QColor>();
        }

        //        QStyleOptionViewItem styleOptionViewItem(option);
        //        initStyleOption(&styleOptionViewItem, index);

        QString lineNumCurr = QString::number(range.start.line + 1); //展示时line从1开始
        int sepWidth = option.fontMetrics.horizontalAdvance(" ");
        int lineNumMaxWidth = option.fontMetrics.horizontalAdvance(QString("99999"));
        int lineNumCurrWidth = option.fontMetrics.horizontalAdvance(lineNumCurr);
        int lineNumOffset = lineNumMaxWidth - lineNumCurrWidth;
        int textHeight = option.fontMetrics.height();
        QRect lineNumDrawRect = option.rect.adjusted(lineNumOffset, textHeight, 0, 0);
        painter->setPen(option.widget->palette().text().color());
        painter->drawText(lineNumDrawRect, lineNumCurr);

        int startCharacter = range.start.character;
        int endCharacter = range.end.character;
        QString heightText = codeText.mid(startCharacter, endCharacter - startCharacter);
        QString frontText = codeText.mid(0, startCharacter);
        int frontTextWidth = option.fontMetrics.horizontalAdvance(frontText);
        int heightTextWidth = option.fontMetrics.horizontalAdvance(heightText);
        QRect heightTextRect = option.rect.adjusted(sepWidth + lineNumMaxWidth+ frontTextWidth, 0, 0, 0);
        heightTextRect.setSize(QSize(heightTextWidth, option.rect.height()));
        //        {, option.rect.y(), heightTextWidth, option.rect.height()};
        painter->setBrush(CodeLensDelegate::color);
        painter->setPen(Qt::NoPen);
        painter->drawRect(heightTextRect);

        painter->setPen(option.widget->palette().text().color());
        QRect codeTextDrawRect = option.rect.adjusted(sepWidth + lineNumMaxWidth, textHeight, 0, 0);
        painter->drawText(codeTextDrawRect, codeText);

    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

void CodeLensDelegate::setHeightColor(const QColor &color)
{
    CodeLensDelegate::color = color;
}

void CodeLensDelegate::setHeightRange(int characterStart, int characterEnd)
{
    CodeLensDelegate::characterStart = characterStart;
    CodeLensDelegate::characterEnd = characterEnd;
}
