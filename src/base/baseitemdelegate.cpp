// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "baseitemdelegate.h"

#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

BaseItemDelegate::BaseItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
    , m_isDarkTheme(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) // Store theme type for efficiency
{
}

void BaseItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option; // Copy option to avoid modifying the original

    if (!option.state.testFlag(QStyle::State_HasFocus)) {
        // Set text color based on the stored theme type
        opt.palette.setColor(QPalette::Text, m_isDarkTheme ? QColor("#c5c8c9") : Qt::black); 
    }

    QStyledItemDelegate::paint(painter, opt, index);
}

