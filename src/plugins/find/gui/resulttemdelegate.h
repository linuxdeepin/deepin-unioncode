// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESULTTEMDELEGATE_H
#define RESULTTEMDELEGATE_H

#include <QItemDelegate>

struct LayoutInfo
{
    QRect iconRect;
    QRect textRect;
    QRect lineNumberRect;
    QIcon icon;
    QStyleOptionViewItem option;
};

class ResultItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit ResultItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    LayoutInfo getLayoutInfo(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    int drawLineNumber(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QModelIndex &index) const;
    void drawText(QPainter *painter, const QStyleOptionViewItem &option,
                  const QRect &rect, const QModelIndex &index) const;

    QString tabStr;
};

#endif   // RESULTTEMDELEGATE_H
