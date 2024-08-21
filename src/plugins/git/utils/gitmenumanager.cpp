// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gitmenumanager.h"
#include "client/gitclient.h"

#include "base/abstractaction.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "services/editor/editorservice.h"

#include <QMenu>

constexpr char M_GIT[] { "Git.Menu" };
constexpr char M_GIT_FILE[] { "Git.Menu.File" };
constexpr char M_GIT_PROJECT[] { "Git.Menu.Project" };
constexpr char A_GIT_LOG_FILE[] { "Git.Action.File.Log" };
constexpr char A_GIT_BLAME_FILE[] { "Git.Action.File.Blame" };
constexpr char A_GIT_DIFF_FILE[] { "Git.Action.File.Diff" };
constexpr char A_GIT_LOG_PROJECT[] { "Git.Action.Project.Log" };
constexpr char A_GIT_DIFF_PROJECT[] { "Git.Action.Project.Diff" };

using namespace dpfservice;

GitMenuManager::GitMenuManager(QObject *parent)
    : QObject(parent)
{
}

void GitMenuManager::actionHandler(Command *cmd, GitType type)
{
    const auto &filePath = cmd->property(GitFilePath).toString();
    bool isProject = cmd->property(GitIsProject).toBool();
    bool ret = false;
    switch (type) {
    case GitLog:
        ret = GitClient::instance()->gitLog(filePath, isProject);
        break;
    case GitBlame:
        ret = GitClient::instance()->blameFile(filePath);
        break;
    case GitDiff:
        ret = GitClient::instance()->gitDiff(filePath, isProject);
    default:
        break;
    }

    if (ret) {
        if (!editSrv)
            editSrv = dpfGetService(EditorService);

        editSrv->switchWidget(GitWindow);
    }
}

GitMenuManager *GitMenuManager::instance()
{
    static GitMenuManager ins;
    return &ins;
}

void GitMenuManager::initialize()
{
    auto mTools = ActionManager::instance()->actionContainer(M_TOOLS);
    auto mGit = ActionManager::instance()->createContainer(M_GIT);
    mGit->menu()->setTitle("&Git");
    mTools->addMenu(mGit);

    createGitSubMenu();
}

void GitMenuManager::setupProjectMenu()
{
    auto mCurProject = ActionManager::instance()->actionContainer(M_GIT_PROJECT);
    auto activeProjInfo = dpfGetService(dpfservice::ProjectService)->getActiveProjectInfo();
    if (!activeProjInfo.isVaild() || !GitClient::instance()->checkRepositoryExist(activeProjInfo.workspaceFolder())) {
        mCurProject->containerAction()->setEnabled(false);
        return;
    }

    mCurProject->containerAction()->setEnabled(true);
    QFileInfo info(activeProjInfo.workspaceFolder());

    auto projectLogAct = ActionManager::instance()->command(A_GIT_LOG_PROJECT);
    projectLogAct->setProperty(GitFilePath, activeProjInfo.workspaceFolder());
    projectLogAct->action()->setText(tr("Log of \"%1\"").arg(info.fileName()));

    auto projectDiffAct = ActionManager::instance()->command(A_GIT_DIFF_PROJECT);
    projectDiffAct->setProperty(GitFilePath, activeProjInfo.workspaceFolder());
    projectDiffAct->action()->setText(tr("Diff of \"%1\"").arg(info.fileName()));
}

void GitMenuManager::setupFileMenu(const QString &filePath)
{
    QString file = filePath;
    if (file.isEmpty()) {
        if (!editSrv)
            editSrv = dpfGetService(EditorService);

        file = editSrv->currentFile();
    }

    auto mCurFile = ActionManager::instance()->actionContainer(M_GIT_FILE);
    if (file.isEmpty() || !GitClient::instance()->checkRepositoryExist(file)) {
        mCurFile->containerAction()->setEnabled(false);
        return;
    }

    mCurFile->containerAction()->setEnabled(true);
    QFileInfo info(file);

    auto fileLogAct = ActionManager::instance()->command(A_GIT_LOG_FILE);
    fileLogAct->setProperty(GitFilePath, file);
    fileLogAct->action()->setText(tr("Log of \"%1\"").arg(info.fileName()));

    auto fileBlameAct = ActionManager::instance()->command(A_GIT_BLAME_FILE);
    fileBlameAct->setProperty(GitFilePath, file);
    fileBlameAct->action()->setText(tr("Blame of \"%1\"").arg(info.fileName()));

    auto fileDiffAct = ActionManager::instance()->command(A_GIT_DIFF_FILE);
    fileDiffAct->setProperty(GitFilePath, file);
    fileDiffAct->action()->setText(tr("Diff of \"%1\"").arg(info.fileName()));
}

QAction *GitMenuManager::gitAction() const
{
    auto mGit = ActionManager::instance()->actionContainer(M_GIT);
    return mGit->containerAction();
}

void GitMenuManager::createGitSubMenu()
{
    auto mGit = ActionManager::instance()->actionContainer(M_GIT);
    auto mCurFile = ActionManager::instance()->createContainer(M_GIT_FILE);
    mCurFile->menu()->setTitle(tr("Current File"));
    mCurFile->containerAction()->setEnabled(false);
    mGit->addMenu(mCurFile);
    createFileSubMenu();

    auto mCurProject = ActionManager::instance()->createContainer(M_GIT_PROJECT);
    mCurProject->menu()->setTitle(tr("Current Project"));
    mCurProject->containerAction()->setEnabled(false);
    mGit->addMenu(mCurProject);
    createProjectSubMenu();
}

void GitMenuManager::createFileSubMenu()
{
    auto mCurFile = ActionManager::instance()->actionContainer(M_GIT_FILE);
    auto fileLogAct = new QAction(this);
    auto cmd = registerShortcut(fileLogAct, A_GIT_LOG_FILE, tr("Git Log"), QKeySequence("Alt+G,Alt+L"));
    mCurFile->addAction(cmd);
    connect(fileLogAct, &QAction::triggered, this, std::bind(&GitMenuManager::actionHandler, this, cmd, GitLog));

    auto fileBlameAct = new QAction(this);
    cmd = registerShortcut(fileBlameAct, A_GIT_BLAME_FILE, tr("Git Blame"), QKeySequence("Alt+G,Alt+B"));
    mCurFile->addAction(cmd);
    connect(fileBlameAct, &QAction::triggered, this, std::bind(&GitMenuManager::actionHandler, this, cmd, GitBlame));

    auto fileDiffAct = new QAction(this);
    cmd = registerShortcut(fileDiffAct, A_GIT_DIFF_FILE, tr("Git Diff"), QKeySequence("Alt+G,Alt+D"));
    mCurFile->addAction(cmd);
    connect(fileDiffAct, &QAction::triggered, this, std::bind(&GitMenuManager::actionHandler, this, cmd, GitDiff));
}

void GitMenuManager::createProjectSubMenu()
{
    auto mCurProject = ActionManager::instance()->actionContainer(M_GIT_PROJECT);
    auto projectLogAct = new QAction(this);
    auto cmd = ActionManager::instance()->registerAction(projectLogAct, A_GIT_LOG_PROJECT);
    cmd->setProperty(GitIsProject, true);
    mCurProject->addAction(cmd);
    connect(projectLogAct, &QAction::triggered, this, std::bind(&GitMenuManager::actionHandler, this, cmd, GitLog));

    auto projectDiffAct = new QAction(this);
    cmd = ActionManager::instance()->registerAction(projectDiffAct, A_GIT_DIFF_PROJECT);
    cmd->setProperty(GitIsProject, true);
    mCurProject->addAction(cmd);
    connect(projectDiffAct, &QAction::triggered, this, std::bind(&GitMenuManager::actionHandler, this, cmd, GitDiff));
}

Command *GitMenuManager::registerShortcut(QAction *act, const QString &id, const QString &description, const QKeySequence &shortCut)
{
    auto cmd = ActionManager::instance()->registerAction(act, id);
    cmd->setDefaultKeySequence(shortCut);
    cmd->setDescription(description);
    return cmd;
}
