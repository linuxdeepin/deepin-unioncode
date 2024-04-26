// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUILDERWIDGET_H
#define BUILDERWIDGET_H

#include "services/builder/builderservice.h"

#include <QWidget>

class BuilderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BuilderWidget(QWidget *parent = nullptr);
    virtual ~BuilderWidget() override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor backGroundColor;

signals:

public slots:
};

#endif // BUILDERWIDGET_H
