// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ITEMDELEGATE_H
#define ITEMDELEGATE_H

#include "notify/constants.h"

#include <QStyledItemDelegate>
#include <QTextLayout>

class NotificationListView;
class ItemDelegate : public QStyledItemDelegate
{
public:
    explicit ItemDelegate(NotificationListView *view, QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void drawBackground(QPainter *painter, const QStyleOptionViewItem &option) const;
    QRect drawIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QRect drawDisplayText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &iconRect) const;
    QRect drawNotificationText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rect) const;
    QRect drawSourceText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rect) const;
    QRect drawCloseButton(QPainter *painter, const QStyleOptionViewItem &option) const;
    void drawActionButton(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QSizeF doTextLayout(QTextLayout *textLayout, int width) const;
    QList<ActionBtuuonInfo> actionInfoList(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QRect closeButtonRect(const QRect &itemRect) const;

    NotificationListView *view { nullptr };
};

#endif   // ITEMDELEGATE_H
