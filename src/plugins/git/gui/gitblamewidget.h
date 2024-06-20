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

    void setGitInfo(const QString &info) override;
    QTextDocument *textDocument() const override;

private:
    GitBlameWidgetPrivate *const d;
};

#endif   // GITBLAMEWIDGET_H
