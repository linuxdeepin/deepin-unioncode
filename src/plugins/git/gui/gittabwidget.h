// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GITTABWIDGET_H
#define GITTABWIDGET_H

#include "constants.h"

#include "base/abstracteditwidget.h"

class GitTabWidgetPrivate;
class GitTabWidget : public AbstractEditWidget
{
    Q_OBJECT
public:
    explicit GitTabWidget(QWidget *parent = nullptr);
    ~GitTabWidget();

    int addWidget(GitType type, const QString &name);
    void setInfo(int index, const QStringList &infos);
    void setErrorMessage(int index, const QString &msg);

    void closeWidget() override;

private:
    GitTabWidgetPrivate *const d;
};

#endif   // GITTABWIDGET_H
