// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GITMENUMANAGER_H
#define GITMENUMANAGER_H

#include "constants.h"

#include <QObject>
#include <QAction>
#include <QMenu>

namespace dpfservice {
class WindowService;
class EditorService;
}
class Command;
class GitMenuManager : public QObject
{
    Q_OBJECT
public:
    static GitMenuManager *instance();

    void initialize();
    void setupProjectMenu();
    void setupFileMenu(const QString &filePath = QString());
    QAction *gitAction() const;

private:
    explicit GitMenuManager(QObject *parent = nullptr);

    void actionHandler(Command *cmd, GitType type);
    void createGitSubMenu();
    void createFileSubMenu();
    void createProjectSubMenu();
    Command *registerShortcut(QAction *act, const QString &id, const QString &description, const QKeySequence &shortCut);

private:
    dpfservice::EditorService *editSrv { nullptr };
};

#endif   // GITMENUMANAGER_H
