// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INSTANTBLAMEWIDGET_H
#define INSTANTBLAMEWIDGET_H

#include "constants.h"

#include <QWidget>

class QLabel;
class InstantBlameWidget : public QWidget
{
    Q_OBJECT
public:
    explicit InstantBlameWidget(QWidget *parent = nullptr);

    void setInfo(const QString &info);
    void clear();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    void initUI();
    CommitInfo parserBlameOutput(const QStringList &blame);

private:
    QLabel *label { nullptr };
};

#endif   // INSTANTBLAMEWIDGET_H
