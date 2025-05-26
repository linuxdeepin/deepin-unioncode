// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GITLOGWIDGET_H
#define GITLOGWIDGET_H

#include "gitbasewidget.h"

class GitLogWidgetPrivate;
class GitLogWidget : public GitBaseWidget
{
    Q_OBJECT
public:
    explicit GitLogWidget(QWidget *parent = nullptr);
    ~GitLogWidget();

    void setGitInfo(const QStringList &infos) override;
    void setReadyMessage(const QString &msg) override;
    void setSourceFile(const QString &sf) override;

private:
    GitLogWidgetPrivate *const d;
};

#endif   // GITLOGWIDGET_H
