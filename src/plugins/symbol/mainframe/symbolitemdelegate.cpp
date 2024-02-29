// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "symbolitemdelegate.h"

#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

SymbolItemDelegate::SymbolItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void SymbolItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem iOption = option;
    if (!option.state.testFlag(QStyle::State_HasFocus)) {
        QColor color;
        DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType
                ? color = Qt::black
                : color = QColor("#c5c8c9");
        iOption.palette.setColor(QPalette::Text, color);
    }

    QStyledItemDelegate::paint(painter, iOption, index);
}
