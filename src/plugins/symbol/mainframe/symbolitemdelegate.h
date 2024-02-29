// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYMBOLITEMDELEGATE_H
#define SYMBOLITEMDELEGATE_H

#include <QStyledItemDelegate>

class SymbolItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SymbolItemDelegate(QObject *parent = nullptr);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // SYMBOLITEMDELEGATE_H
