// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTOPENITEMDELEGATE_H
#define RECENTOPENITEMDELEGATE_H

#include "base/baseitemdelegate.h"

#include <DGuiApplicationHelper>

#include <QPainter>
#include <QFocusEvent>

class RecentOpenItemDelegate : public BaseItemDelegate
{
    Q_OBJECT
public:
    explicit RecentOpenItemDelegate(QObject *parent = nullptr);

signals:
    void closeBtnClicked(const QModelIndex &index);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // RECENTOPENLISTDELEGATE_H
