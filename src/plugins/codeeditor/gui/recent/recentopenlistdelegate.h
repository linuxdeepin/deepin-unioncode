// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTOPENLISTDELEGATE_H
#define RECENTOPENLISTDELEGATE_H

#include <DStyledItemDelegate>
#include <DGuiApplicationHelper>

#include <QPainter>
#include <QFocusEvent>

class RecentOpenListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit RecentOpenListDelegate(QAbstractItemView *parent = nullptr);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // RECENTOPENLISTDELEGATE_H
