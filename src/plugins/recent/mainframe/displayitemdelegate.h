// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISPLAYITEMDELEGATE_H
#define DISPLAYITEMDELEGATE_H

#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE

class DisplayItemDelegate : public DStyledItemDelegate
{
public:
    explicit DisplayItemDelegate(QAbstractItemView *parent = nullptr);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

#endif // DISPLAYITEMDELEGATE_H
