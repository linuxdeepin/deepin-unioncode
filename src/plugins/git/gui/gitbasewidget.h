// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GITBASEWIDGET_H
#define GITBASEWIDGET_H

#include <QWidget>
#include <QTextDocument>

class GitBaseWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GitBaseWidget(QWidget *parent = nullptr)
        : QWidget(parent) {}

    virtual void setGitInfo(const QStringList &infos) = 0;
    virtual void setReadyMessage(const QString &msg) = 0;
    virtual void setSourceFile(const QString &sf) {}
};

#endif   // GITBASEWIDGET_H
