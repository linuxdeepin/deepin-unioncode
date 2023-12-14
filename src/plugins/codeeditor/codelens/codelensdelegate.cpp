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
    ,color(QColor(Qt::yellow))
{
    color.setAlpha(95);
}

QSize CodeLensDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(24);
    return size;
}

void CodeLensDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    QString displayName = index.data().toString();  //从模型索引index中获取显示的文本。
    QFont font = painter->font();                   //获取画笔的字体，并创建一个字体度量对象，用于后续计算文本的尺寸
    QFontMetrics fontMetrics(font);                 //使用字体创建一个字体度量对象，用于计算文本的布局信息。

    if (index.parent().isValid()){                  //如果是root item，就按默认绘制即可
        // 获取数据用于绘制
        lsp::Range range;
        QString codeText;
        QColor heightColor;
        QVariant rangeVar = index.data(CodeLensItemRole::Range);

        // 从一个可能包含不同数据类型的 QVariant 对象中提取出数据
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

        // 获取当前行号并计算相关尺寸
        QString lineNumCurr = QString::number(range.start.line + 1); // 展示时line从1开始
        int sepWidth = option.fontMetrics.horizontalAdvance(" ");   // 计算一个空格字符的宽度
        int lineNumMaxWidth = option.fontMetrics.horizontalAdvance(QString("99999")); // 计算最大行号宽度
        int lineNumCurrWidth = option.fontMetrics.horizontalAdvance(lineNumCurr); // 计算当前行号字符串的宽度
        int lineNumOffset = lineNumMaxWidth - lineNumCurrWidth; // 计算行号的偏移量，对齐
        int textHeight = 24; // 设置每行的绘制高度为24

        // 开始绘制行号
        QRect lineNumDrawRect = option.rect.adjusted(lineNumOffset, 0, 0, 0);
        lineNumDrawRect.setHeight(textHeight);
        painter->setPen(option.widget->palette().text().color());
        painter->drawText(lineNumDrawRect, lineNumCurr);

        // 计算高亮文本的位置和大小
        int startCharacter = range.start.character;
        int endCharacter = range.end.character;
        QString heightText = codeText.mid(startCharacter, endCharacter - startCharacter);
        QString frontText = codeText.mid(0, startCharacter);
        int frontTextWidth = option.fontMetrics.horizontalAdvance(frontText);
        int heightTextWidth = option.fontMetrics.horizontalAdvance(heightText);
        QRect heightTextRect = option.rect.adjusted(sepWidth + lineNumMaxWidth + frontTextWidth, 0, 0, 0);
        heightTextRect.setSize(QSize(heightTextWidth, textHeight));

        // 绘制高亮文本的背景
        painter->setBrush(CodeLensDelegate::color);
        painter->setPen(Qt::NoPen);
        painter->drawRect(heightTextRect);

        // 绘制全部代码文本
        painter->setPen(option.widget->palette().text().color());
        QRect codeTextDrawRect = option.rect.adjusted(sepWidth + lineNumMaxWidth, 0, 0, 0);
        codeTextDrawRect.setHeight(textHeight);
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
