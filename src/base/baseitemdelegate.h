// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEITEMDELEGATE_H
#define BASEITEMDELEGATE_H

#include <QStyledItemDelegate>

class BaseItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit BaseItemDelegate(QObject* parent = nullptr);

protected:
    // Virtual method for custom painting
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override; 

private:
    class BaseItemDelegatePrivate;
    std::unique_ptr<BaseItemDelegatePrivate> d; // Use unique_ptr for memory management
};

#endif  // BASEITEMDELEGATE_H

