// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODELENSDELEGATE_H
#define CODELENSDELEGATE_H

#include "common/common.h"

#include <QObject>
#include <QStyledItemDelegate>

class CodeLensDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    int characterStart;
    int characterEnd;
    QColor color;
public:
    explicit CodeLensDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    void setHeightColor(const QColor &color);
    void setHeightRange(int characterStart, int characterEnd);
};

#endif // CODELENSDELEGATE_H
