// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODELENSDELEGATE_H
#define CODELENSDELEGATE_H

#include "common/common.h"

#include <QItemDelegate>

struct LayoutInfo
{
    QRect textRect;
    QRect lineNumberRect;
    QStyleOptionViewItem option;
};

class CodeLensDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit CodeLensDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    LayoutInfo getLayoutInfo(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    int drawLineNumber(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QModelIndex &index) const;
    void drawText(QPainter *painter, const QStyleOptionViewItem &option,
                  const QRect &rect, const QModelIndex &index) const;

private:
    QString tabStr;
};

#endif   // CODELENSDELEGATE_H
