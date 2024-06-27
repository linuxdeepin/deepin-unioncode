// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentopenlistdelegate.h"

#include <DPalette>

#include <QPainterPath>
#include <QSize>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

RecentOpenListDelegate::RecentOpenListDelegate(QAbstractItemView *parent) :
    QStyledItemDelegate(parent)
{
}

void RecentOpenListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->save();
    if (option.widget) {
        DPalette pl = DGuiApplicationHelper::instance()->applicationPalette();
        QColor baseColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);
        QColor adjustColor = baseColor;

        bool isSelected = (option.state & QStyle::State_Selected) && option.showDecorationSelected;

        if (isSelected) {
            adjustColor = option.palette.color(DPalette::ColorGroup::Active, QPalette::Highlight);
        } else if (option.state & QStyle::StateFlag::State_MouseOver) {
            // hover color
            adjustColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +10);
        } else {
            // alternately background color
            painter->setOpacity(0);
            if (index.row() % 2 == 0) {
                adjustColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +5);
                painter->setOpacity(1);
            }
        }

        // set paint path
        QPainterPath path;
        path.addRoundedRect(option.rect, 8, 8);
        painter->fillPath(path, adjustColor);
    }
    painter->restore();
    QStyledItemDelegate::paint(painter, option, index);
}

QSize RecentOpenListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize s;
    s.setWidth(option.rect.width());
    s.setHeight(30);
    return s;
}
