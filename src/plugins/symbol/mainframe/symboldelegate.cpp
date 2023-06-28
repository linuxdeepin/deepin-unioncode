// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "symboldelegate.h"

#include <QPainter>

class SymbolDelegatePrivate
{
    friend class SymbolDelegate;
    QModelIndex index;
};

SymbolDelegate::SymbolDelegate(QObject *parent)
  : QStyledItemDelegate (parent)
  , d (new SymbolDelegatePrivate)
{

}

void SymbolDelegate::setActiveProject(const QModelIndex &root)
{
    d->index = root;
}

void SymbolDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    QStyleOptionViewItem iOption = option;
    if (d->index.isValid() && d->index == index) {
        iOption.font.setBold(true);
    }
    QStyledItemDelegate::paint(painter, iOption, index);
}
