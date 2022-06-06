/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "codelensdelegate.h"
#include "codelentype.h"

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
        QVariant rangeVar = index.data(CodeLenItemRole::Range);
        if (rangeVar.canConvert<lsp::Range>()) {
            range = rangeVar.value<lsp::Range>();
        }
        QVariant codeTextVar = index.data(CodeLenItemRole::CodeText);
        if (codeTextVar.canConvert<QString>()) {
            codeText = codeTextVar.value<QString>();
        }
        QVariant colorVar = index.data(CodeLenItemRole::HeightColor);
        if (colorVar.canConvert<QColor>()) {
            heightColor = colorVar.value<QColor>();
        }

        //        QStyleOptionViewItem styleOptionViewItem(option);
        //        initStyleOption(&styleOptionViewItem, index);

        QString lineNumCurr = QString::number(range.start.line + 1); //展示时line从1开始
        int sepWidth = option.fontMetrics.width(" ");
        int lineNumMaxWidth = option.fontMetrics.width(QString("99999"));
        int lineNumCurrWidth = option.fontMetrics.width(lineNumCurr);
        int lineNumOffset = lineNumMaxWidth - lineNumCurrWidth;
        int textHeight = option.fontMetrics.height();
        int textHeadOffset = 0;
        if (option.rect.height() > textHeight) {
            textHeadOffset = (option.rect.height() - textHeight) / 2;
        }
        QRect lineNumDrawRect = option.rect.adjusted(lineNumOffset, textHeight, 0, 0);
        painter->setPen(option.widget->palette().text().color());
        painter->drawText(lineNumDrawRect, lineNumCurr);

        int startCharacter = range.start.character;
        int endCharacter = range.end.character;
        QString heightText = codeText.mid(startCharacter, endCharacter - startCharacter);
        QString frontText = codeText.mid(0, startCharacter);
        int frontTextWidth = option.fontMetrics.width(frontText);
        int heightTextWidth = option.fontMetrics.width(heightText);
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
