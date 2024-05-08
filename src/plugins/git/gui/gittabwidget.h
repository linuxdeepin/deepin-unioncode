// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GITTABWIDGET_H
#define GITTABWIDGET_H

#include "constants.h"

#include <QWidget>

class GitTabWidgetPrivate;
class GitTabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GitTabWidget(QWidget *parent = nullptr);
    ~GitTabWidget();

    int addWidget(GitType type, const QString &name);
    void setInfo(int index, const QStringList &infos);
    void setErrorMessage(int index, const QString &msg);

Q_SIGNALS:
    void closeRequested();

private:
    GitTabWidgetPrivate *const d;
};

#endif   // GITTABWIDGET_H
