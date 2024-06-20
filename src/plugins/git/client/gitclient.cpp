// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gitclient.h"
#include "constants.h"
#include "utils/gitcommand.h"
#include "gui/instantblamewidget.h"

#include <QDir>
#include <QFile>
#include <QTimer>
#include <QDebug>

class GitClientPrivate : public QObject
{
public:
    QString findRepository(const QString &filePath);
    void instantBlame();

public:
    QStringList gitRepositoryCache;
    QTimer *cursorPositionChangedTimer { nullptr };
    QString currentWorkspace;
    QString currentFilePath;
    int currentLine { 0 };

    InstantBlameWidget *ibWidget { nullptr };
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
    if (dirList.isEmpty() || !dirList.contains(".git"))
        return findRepository(info.absolutePath());

    return filePath;
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
}

QString GitClient::gitBinaryPath() const
{
    return GitBinaryPath;
}

bool GitClient::gitBinaryValid()
{
    return QFile::exists(GitBinaryPath);
}

bool GitClient::setupInstantBlame(const QString &filePath)
{
    if (filePath.isEmpty())
        return false;

    for (const auto &cache : d->gitRepositoryCache) {
        if (filePath.startsWith(cache))
            return true;
    }

    auto repository = d->findRepository(filePath);
    if (!repository.isEmpty()) {
        d->gitRepositoryCache << repository;
        return true;
    }

    return false;
}

QWidget *GitClient::instantBlameWidget() const
{
    return d->ibWidget;
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
