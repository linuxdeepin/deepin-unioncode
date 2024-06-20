// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
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

    void setGitInfo(const QString &info) override;
    QTextDocument *textDocument() const override;

private:
    GitLogWidgetPrivate *const d;
};

#endif   // GITLOGWIDGET_H
