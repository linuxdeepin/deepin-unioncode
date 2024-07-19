// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentopenitemdelegate.h"

#include <DPalette>

#include <QPainterPath>
#include <QSize>
#include <QApplication>

RecentOpenItemDelegate::RecentOpenItemDelegate(QObject *parent) :
    BaseItemDelegate(parent)
{
}

void RecentOpenItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    BaseItemDelegate::paint(painter, option, index);

    if (option.state & QStyle::StateFlag::State_MouseOver) {
        const QIcon icon(QIcon::fromTheme("edit-closeBtn").pixmap(16, 16));

        QRect iconRect(option.rect.right() - option.rect.height() - 15,
                       option.rect.top(),
                       option.rect.height(),
                       option.rect.height());

        icon.paint(painter, iconRect, Qt::AlignRight | Qt::AlignVCenter);
    }
}

bool RecentOpenItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
    const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent && mouseEvent->button() == Qt::LeftButton) {
            QRect buttonRect(option.rect.right() - option.rect.height() - 15,
                       option.rect.top(),
                       option.rect.height(),
                       option.rect.height());

            if (buttonRect.contains(mouseEvent->pos())) {
                emit closeBtnClicked(index);
                return true;
            }
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QSize RecentOpenItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    return { option.rect.width(), 24 };
}
