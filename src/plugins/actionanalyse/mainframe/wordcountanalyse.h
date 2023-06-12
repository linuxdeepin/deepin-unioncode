/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
