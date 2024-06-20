// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
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
}

class GitMenuManager : public QObject
{
    Q_OBJECT
public:
    static GitMenuManager *instance();

    void initialize(dpfservice::WindowService *service);
    void setCurrentProject(const QString &project);
    void setCurrentFile(const QString &file);

private:
    explicit GitMenuManager(QObject *parent = nullptr);

    void actionHandler(QAction *act, GitType type);
    void createGitSubMenu();
    void createFileSubMenu();
    void createProjectSubMenu();

private:
    dpfservice::WindowService *winSer { nullptr };

    QString curProjectName;
    QString curFileName;

    QMenu gitSubMenu;
    QMenu fileSubMenu;
    QMenu projectSubMenu;

    QAction *curFileAct { nullptr };
    QAction *fileLogAct { nullptr };
    QAction *fileBlameAct { nullptr };
    QAction *fileDiffAct { nullptr };

    QAction *curProjectAct { nullptr };
};

#endif   // GITMENUMANAGER_H
