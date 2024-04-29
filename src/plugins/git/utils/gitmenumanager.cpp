// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gitmenumanager.h"
#include "constants.h"
#include "client/gitclient.h"

#include "base/abstractaction.h"
#include "services/window/windowservice.h"

#include <QMenu>

using namespace dpfservice;

GitMenuManager::GitMenuManager(QObject *parent)
    : QObject(parent)
{
}

GitMenuManager *GitMenuManager::instance()
{
    static GitMenuManager ins;
    return &ins;
}

void GitMenuManager::initialize(dpfservice::WindowService *service)
{
    if (!service)
        return;

    winSer = service;
    auto initAction = [&](QAction *action, const QString &id = QString(),
                          const QString &description = QString(),
                          const QKeySequence &key = QKeySequence()) -> AbstractAction * {
        auto actionImpl = new AbstractAction(action, this);
        if (!key.isEmpty())
            actionImpl->setShortCutInfo(id, description, key);
        return actionImpl;
    };

    QAction *gitAct = new QAction("&Git", this);
    auto gitActImpl = initAction(gitAct);
    service->addAction(MWM_TOOLS, gitActImpl);

    createGitSubMenu();
    gitAct->setMenu(&gitSubMenu);
}

void GitMenuManager::setCurrentProject(const QString &project)
{
    // TODO: act
}

void GitMenuManager::setCurrentFile(const QString &file)
{
    if (file.isEmpty() || !GitClient::instance()->checkRepositoryExist(file)) {
        curFileAct->setEnabled(false);
        return;
    }

    curFileAct->setEnabled(true);
    QFileInfo info(file);

    fileLogAct->setProperty(GitFilePath, file);
    fileLogAct->setText(tr("Log of \"%1\"").arg(info.fileName()));

    fileBlameAct->setProperty(GitFilePath, file);
    fileBlameAct->setText(tr("Blame of \"%1\"").arg(info.fileName()));
}

void GitMenuManager::createGitSubMenu()
{
    curFileAct = gitSubMenu.addAction(tr("Current File"));
    curFileAct->setEnabled(false);
    curFileAct->setMenu(&fileSubMenu);
    createFileSubMenu();

    curProjectAct = gitSubMenu.addAction(tr("Current Project"));
    curProjectAct->setEnabled(false);
    curProjectAct->setMenu(&projectSubMenu);
    createProjectSubMenu();
}

void GitMenuManager::createFileSubMenu()
{
    fileLogAct = new QAction(this);
    connect(fileLogAct, &QAction::triggered, this, [this] {
        const auto &filePath = fileLogAct->property(GitFilePath).toString();
        if (GitClient::instance()->logFile(filePath)) {
            auto dockName = winSer->getCurrentDockName(Position::Central);
            if (dockName == GitWindow)
                return;

            GitClient::instance()->setLastCentralWidget(dockName);
            winSer->showWidgetAtPosition(GitWindow, Position::Central, true);
        }
    });

    fileBlameAct = new QAction(this);
    connect(fileBlameAct, &QAction::triggered, this, [this] {
        const auto &filePath = fileBlameAct->property(GitFilePath).toString();
        if (GitClient::instance()->blameFile(filePath)) {
            auto dockName = winSer->getCurrentDockName(Position::Central);
            if (dockName == GitWindow)
                return;

            GitClient::instance()->setLastCentralWidget(dockName);
            winSer->showWidgetAtPosition(GitWindow, Position::Central, true);
        }
    });

    fileSubMenu.addAction(fileLogAct);
    fileSubMenu.addAction(fileBlameAct);
}

void GitMenuManager::createProjectSubMenu()
{
    // TODO: create
}
