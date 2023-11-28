// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "navigationdelegate.h"

#include <QPainter>
#include <QDebug>
#include <DApplication>
#include <DFontSizeManager>

DWIDGET_BEGIN_NAMESPACE

NavigationDelegate::NavigationDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{

}

NavigationDelegate::~NavigationDelegate()
{

}

void NavigationDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    opt.text = QString();
    DStyledItemDelegate::paint(painter, opt, index);
    auto level = static_cast<NavLevel>(index.data(NavLevelRole).toInt());
    bool isSelected = option.state & QStyle::State_Selected;

    // draw text
    switch (level) {
    case Level1: {
        QColor pen = option.palette.color(isSelected ? QPalette::HighlightedText : QPalette::BrightText);
        painter->setPen(pen);
        painter->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T4, QFont::Medium, opt.font));
        QRect rect = opt.rect.marginsRemoved(QMargins(10, 0, 10, 0));
        auto text = painter->fontMetrics().elidedText(index.data().toString(), Qt::ElideRight, rect.width());
        painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, text);
        break;
    }
    case Level2: {
        QColor pen = option.palette.color(isSelected ? QPalette::HighlightedText : QPalette::WindowText);
        painter->setPen(pen);
        auto rect = option.rect.marginsRemoved(QMargins(30, 0, 10, 0));
        auto text = opt.fontMetrics.elidedText(index.data().toString(), Qt::ElideRight, rect.width());
        painter->setFont(opt.font);
        painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, text);
        break;
    }
    case Split:
    case Level3:
        break;
    }
}

void NavigationDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    DStyledItemDelegate::initStyleOption(option, index);
    option->features &= ~QStyleOptionViewItem::HasDisplay;

    auto level = static_cast<NavLevel>(index.data(NavLevelRole).toInt());

    if (level == Level1) {
        option->font = DFontSizeManager::instance()->get(DFontSizeManager::T4, option->font);
        option->font.setBold(true);
        option->fontMetrics = QFontMetrics(option->font);
    }
}

DWIDGET_END_NAMESPACE
