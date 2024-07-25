// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ELIDEDLABEL_H
#define ELIDEDLABEL_H

#include <DLabel>

class ElidedLabelPrivate;
class ElidedLabel : public DTK_WIDGET_NAMESPACE::DLabel
{
    Q_OBJECT
    ElidedLabelPrivate *const d;

public:
    explicit ElidedLabel(QWidget *parent = nullptr);
    virtual ~ElidedLabel();
    QString text();

public slots:
    void setText(const QString &);
};

#endif // ELIDEDLABEL_H
