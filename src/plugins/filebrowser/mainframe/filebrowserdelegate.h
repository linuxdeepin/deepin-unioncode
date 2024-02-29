// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEBROWSERDELEGATE_H
#define FILEBROWSERDELEGATE_H

#include <QStyledItemDelegate>

class FileBrowserDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit FileBrowserDelegate(QObject *parent = nullptr);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // FILEBROWSERDELEGATE_H
