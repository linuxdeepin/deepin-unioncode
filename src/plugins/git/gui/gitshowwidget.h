// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GITSHOWWIDGET_H
#define GITSHOWWIDGET_H

#include "gitbasewidget.h"

class GitShowWidgetPrivate;
class GitShowWidget : public GitBaseWidget
{
    Q_OBJECT
public:
    explicit GitShowWidget(QWidget *parent = nullptr);
    ~GitShowWidget();

    void setGitInfo(const QStringList &infos) override;
    void setReadyMessage(const QString &msg) override;

private:
    GitShowWidgetPrivate *const d;
};

#endif   // GITSHOWWIDGET_H
