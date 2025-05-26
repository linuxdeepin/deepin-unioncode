// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GITQLIENTWIDGET_H
#define GITQLIENTWIDGET_H

#include "GitQlient.h"
#include "QLogger.h"

class GitQlientWidget : public GitQlient
{
    Q_OBJECT
public:
    explicit GitQlientWidget(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;

private:
    bool isGitDir(const QString &repoPath);
    QString getRepositoryPath() const;
};

#endif // GITQLIENTWIDGET_H
