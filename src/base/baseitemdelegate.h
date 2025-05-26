// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEITEMDELEGATE_H
#define BASEITEMDELEGATE_H

#include <QStyledItemDelegate>

class BaseItemDelegate : public QStyledItemDelegate
{
public:
    explicit BaseItemDelegate(QObject *parent = nullptr);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif   // BASEITEMDELEGATE_H
