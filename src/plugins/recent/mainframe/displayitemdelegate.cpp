// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "displayitemdelegate.h"

#include <DApplication>
#include <DGuiApplicationHelper>
#include <DStyle>
#include <DStyleOptionViewItem>
#include <DPaletteHelper>
#include <DSizeMode>

#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QFileInfo>

DisplayItemDelegate::DisplayItemDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate (parent)
{

}

void DisplayItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();

        int textTopAlign = Qt::AlignmentFlag::AlignTop;
        int textBottomAlign = Qt::AlignmentFlag::AlignBottom;
        QString filePath = index.data(Qt::DisplayRole).toString();
        QString fileName = QFileInfo(filePath).fileName();

        QStyleOptionViewItem drawStyle(option);
        DStyledItemDelegate::initStyleOption(&drawStyle, index);
        painter->setRenderHint(QPainter::Antialiasing);

        drawStyle.text = ""; //清空文本绘制
        QStyle *pStyle = drawStyle.widget ? drawStyle.widget->style() : DApplication::style();
        pStyle->drawControl(DStyle::CE_ItemViewItem, &drawStyle, painter, drawStyle.widget);

        QRect iconRect = pStyle->itemPixmapRect(drawStyle.rect, drawStyle.displayAlignment,
                                                drawStyle.icon.pixmap(drawStyle.rect.height()));
        QRect titleTextRect = pStyle->itemTextRect(drawStyle.fontMetrics, drawStyle.rect, textTopAlign, true, fileName)
                .adjusted(iconRect.width(), 0, 0, 0);
        titleTextRect.setWidth(option.rect.width());

        QString eliedFileName = drawStyle.fontMetrics.elidedText(fileName, Qt::ElideRight, titleTextRect.width());
        pStyle->drawItemText(painter, titleTextRect, drawStyle.displayAlignment, drawStyle.palette, true, eliedFileName);

        //缩小字体
        QFont nativeFont(drawStyle.font);
        nativeFont.setPointSize(drawStyle.font.pointSize() - 1);
        nativeFont.setItalic(true);
        painter->setFont(nativeFont);

        //计算绘制坐标
        QRect nativeTextRect = pStyle->itemTextRect(QFontMetrics(nativeFont), drawStyle.rect, textBottomAlign, true, filePath)
                .adjusted(iconRect.width(), 0, 0, 0);
        nativeTextRect.setWidth(option.rect.width());

        QString eliedFilePath = drawStyle.fontMetrics.elidedText(filePath, Qt::ElideRight, nativeTextRect.width());
        painter->drawText(nativeTextRect, textBottomAlign, eliedFilePath);
        painter->restore();
    } else {
        return DStyledItemDelegate::paint(painter, option, index);
    }
}

QSize DisplayItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    if (index.isValid()) {
        QSize size = index.data(Qt::SizeHintRole).toSize();
        if (size.isValid()) {
            return size;
        } else {
            return {option.rect.width(), option.fontMetrics.height() * 2};
        }
    }
    return DStyledItemDelegate::sizeHint(option, index);
}
