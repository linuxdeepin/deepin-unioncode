// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GITTABWIDGET_H
#define GITTABWIDGET_H

#include <QWidget>

class GitTabWidgetPrivate;
class GitTabWidget : public QWidget
{
    Q_OBJECT
public:
    enum Type {
        GitLog,
        GitBlame,
        GitDiff
    };

    explicit GitTabWidget(QWidget *parent = nullptr);
    ~GitTabWidget();

    int addWidget(Type type, const QString &path);
    void setInfo(int index, const QString &info);

Q_SIGNALS:
    void closeRequested();

private:
    GitTabWidgetPrivate *const d;
};

#endif   // GITTABWIDGET_H
