// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTDELEGATE_H
#define PROJECTDELEGATE_H

#include <QStyledItemDelegate>

class SymbolDelegatePrivate;
class SymbolDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    SymbolDelegatePrivate *const d;
public:
    explicit SymbolDelegate(QObject *parent = nullptr);
    void setActiveProject(const QModelIndex &root);
protected:
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

#endif // PROJECTDELEGATE_H
