// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GITBLAMEWIDGET_H
#define GITBLAMEWIDGET_H

#include "gitbasewidget.h"

class GitBlameWidgetPrivate;
class GitBlameWidget : public GitBaseWidget
{
    Q_OBJECT
public:
    explicit GitBlameWidget(QWidget *parent = nullptr);
    ~GitBlameWidget();

    void setGitInfo(const QStringList &infos) override;
    void setReadyMessage(const QString &msg) override;
    void setSourceFile(const QString &sf) override;

private:
    GitBlameWidgetPrivate *const d;
};

#endif   // GITBLAMEWIDGET_H
