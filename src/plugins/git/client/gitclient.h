// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GITCLIENT_H
#define GITCLIENT_H

#include <QObject>

class GitClientPrivate;
class GitClient : public QObject
{
    Q_OBJECT
public:
    static GitClient *instance();

    void init();

    QString gitBinaryPath() const;
    bool gitBinaryValid();
    bool checkRepositoryExist(const QString &filePath, QString *repository = nullptr);

    bool setupInstantBlame(const QString &filePath);
    void clearInstantBlame();
    bool gitLog(const QString &filePath, bool isProject);
    bool blameFile(const QString &filePath);
    bool gitDiff(const QString &filePath, bool isProject);
    bool show(const QString &source, const QString &commitId);

    QWidget *instantBlameWidget() const;
    QWidget *gitTabWidget() const;

public Q_SLOTS:
    void instantBlame(const QString &workspace, const QString &filePath, int line);

private:
    explicit GitClient(QObject *parent = nullptr);
    ~GitClient();

private:
    GitClientPrivate *const d;
};

#endif   // GITCLIENT_H
