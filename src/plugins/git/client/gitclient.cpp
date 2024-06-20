// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gitclient.h"
#include "constants.h"
#include "utils/gitcommand.h"
#include "gui/instantblamewidget.h"
#include "gui/gittabwidget.h"

#include "services/window/windowservice.h"

#include <QDir>
#include <QFile>
#include <QTimer>
#include <QDebug>

using namespace dpfservice;

class GitClientPrivate : public QObject
{
public:
    QString findRepository(const QString &filePath);
    QString normalLogArguments();

    void instantBlame();
    void logFile(const QString &workspace, const QString &filePath);

public:
    QString lastCentral;
    QStringList gitRepositoryCache;
    QTimer *cursorPositionChangedTimer { nullptr };
    QString currentWorkspace;
    QString currentFilePath;
    int currentLine { 0 };

    InstantBlameWidget *ibWidget { nullptr };
    GitTabWidget *gitTabWidget { nullptr };
};

QString GitClientPrivate::findRepository(const QString &filePath)
{
    if (filePath == "/")
        return {};

    QFileInfo info(filePath);
    if (info.isFile())
        return findRepository(info.absolutePath());

    QDir dir(filePath);
    auto dirList = dir.entryList(QDir::AllDirs | QDir::Hidden | QDir::NoDotAndDotDot);
    if (dirList.isEmpty() || !dirList.contains(GitDirectory))
        return findRepository(info.absolutePath());

    return filePath;
}

QString GitClientPrivate::normalLogArguments()
{
    const QString authorName("#007af4");
    const QString commitDate("#006600");
    const QString commitHash("#ff0000");
    const QString decoration("#ff00ff");

    const QString logArgs = QStringLiteral(
                                    "--pretty=format:"
                                    "commit %C(%1)%H%Creset %C(%2)%d%Creset%n"
                                    "Author: %C(%3)%an <%ae>%Creset%n"
                                    "Date:   %C(%4)%cD %Creset%n%n"
                                    "%w(0,4,4)%s%Creset%n%n%b")
                                    .arg(commitHash, decoration, authorName, commitDate);

    return logArgs;
}

void GitClientPrivate::instantBlame()
{
    GitCommand *cmd = new GitCommand(currentWorkspace);
    connect(cmd, &GitCommand::finished, this, [cmd, this](int code) {
        if (code == 0)
            ibWidget->setInfo(cmd->cleanedStdOut());
        else
            qWarning() << cmd->cleanedStdErr();

        cmd->deleteLater();
    });

    const QString lineString = QString("%1,%1").arg(currentLine);
    QStringList arguments = { "blame", "-p" };
    arguments.append({ "-L", lineString, "--", currentFilePath });
    cmd->addJob(GitBinaryPath, arguments);

    cmd->start();
}

void GitClientPrivate::logFile(const QString &workspace, const QString &filePath)
{
    int index = gitTabWidget->addWidget(GitTabWidget::GitLog, filePath);
    GitCommand *cmd = new GitCommand(workspace);
    connect(cmd, &GitCommand::finished, this, [=](int code) {
        if (code == 0) {
            gitTabWidget->setInfo(index, cmd->cleanedStdOut());
        } else {
            gitTabWidget->setInfo(index, tr("Failed to retrieve data."));
            qWarning() << cmd->cleanedStdErr();
        }

        cmd->deleteLater();
    });

    QStringList arguments = { "log", DecorateOption,
                              "-n", QString::number(LogMaxCount),
                              "--patience", "--ignore-space-change",
                              ColorOption, "--follow",
                              normalLogArguments(), "--", filePath };

    cmd->addJob(GitBinaryPath, arguments);
    cmd->start();
}

GitClient::GitClient(QObject *parent)
    : QObject(parent),
      d(new GitClientPrivate)
{
}

GitClient::~GitClient()
{
    delete d;
}

GitClient *GitClient::instance()
{
    static GitClient ins;
    return &ins;
}

void GitClient::init()
{
    d->ibWidget = new InstantBlameWidget;
    d->gitTabWidget = new GitTabWidget;

    connect(d->gitTabWidget, &GitTabWidget::closeRequested, this, &GitClient::switchLastCentralWidget);
}

void GitClient::setLastCentralWidget(const QString &name)
{
    d->lastCentral = name;
}

QString GitClient::gitBinaryPath() const
{
    return GitBinaryPath;
}

bool GitClient::gitBinaryValid()
{
    return QFile::exists(GitBinaryPath);
}

bool GitClient::checkRepositoryExist(const QString &filePath, QString *repository)
{
    for (const auto &cache : d->gitRepositoryCache) {
        if (filePath.startsWith(cache)) {
            if (repository)
                *repository = cache;
            return true;
        }
    }

    auto ret = d->findRepository(filePath);
    if (!ret.isEmpty()) {
        d->gitRepositoryCache << ret;
        if (repository)
            *repository = ret;
        return true;
    }

    return false;
}

bool GitClient::setupInstantBlame(const QString &filePath)
{
    if (filePath.isEmpty())
        return false;

    return checkRepositoryExist(filePath);
}

bool GitClient::logFile(const QString &filePath)
{
    QString repository;
    if (!checkRepositoryExist(filePath, &repository))
        return false;

    d->logFile(repository, filePath);
    return true;
}

QWidget *GitClient::instantBlameWidget() const
{
    return d->ibWidget;
}

QWidget *GitClient::gitTabWidget() const
{
    return d->gitTabWidget;
}

void GitClient::instantBlame(const QString &workspace, const QString &filePath, int line)
{
    if (!d->cursorPositionChangedTimer) {
        d->cursorPositionChangedTimer = new QTimer(this);
        d->cursorPositionChangedTimer->setSingleShot(true);
        d->cursorPositionChangedTimer->setInterval(500);

        connect(d->cursorPositionChangedTimer, &QTimer::timeout, this, std::bind(&GitClientPrivate::instantBlame, d));
    }

    d->currentWorkspace = workspace;
    d->currentFilePath = filePath;
    d->currentLine = line;
    d->cursorPositionChangedTimer->start();
}

void GitClient::switchLastCentralWidget()
{
    if (d->lastCentral.isEmpty())
        return;

    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService)
        return;

    windowService->showWidgetAtPosition(d->lastCentral, Position::Central, true);
    d->lastCentral.clear();
}
