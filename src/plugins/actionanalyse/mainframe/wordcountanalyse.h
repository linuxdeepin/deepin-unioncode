// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORDCOUNTANALYSE_H
#define WORDCOUNTANALYSE_H

#include <QProcess>

struct ActionAnalyseArgs
{
    QString workspace;
    QString language;
    QString storage;
    ActionAnalyseArgs();
    ActionAnalyseArgs(const QString &workspace, const QString &language, const QString &storage);
    ActionAnalyseArgs(const ActionAnalyseArgs &as);
    ActionAnalyseArgs &operator=(const ActionAnalyseArgs &as);
};

class WordCountAnalyse : public QProcess
{
    Q_OBJECT
public:
    explicit WordCountAnalyse(QObject *parent = nullptr);

    void setArgs(const ActionAnalyseArgs& args);
    ActionAnalyseArgs args() const;
    QString getStorage() const;
    QString getWorkspace() const;
    QString getLanguage() const;

    void start();

private:
    QString getPythonVersion();

    void setStorage(const QString &storage);
    void setWorkspace(const QString &workspace);
    void setLanguage(const QString &language);

signals:
    void analyseDone(bool result);

private slots:
    void errorOccurred(QProcess::ProcessError err);
    void finished(int exitCode, QProcess::ExitStatus status);

private:
    ActionAnalyseArgs processArgs;
    QString pythonVersion;
};

#endif // WORDCOUNTANALYSE_H
