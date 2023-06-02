/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#ifndef SYMBOLPARSER_H
#define SYMBOLPARSER_H

#include <QProcess>

struct SymbolParseArgs
{
    QString workspace;
    QString language;
    QString storage;
    SymbolParseArgs();
    SymbolParseArgs(const QString &workspace, const QString &language, const QString &storage);
    SymbolParseArgs(const SymbolParseArgs &as);
    SymbolParseArgs &operator=(const SymbolParseArgs &as);
};

class SymbolParser : public QProcess
{
    Q_OBJECT
public:
    explicit SymbolParser(QObject *parent = nullptr);

    void setArgs(const SymbolParseArgs& args);
    SymbolParseArgs args() const;

    void setStorage(const QString &storage);
    QString getStorage() const;

    void setWorkspace(const QString &workspace);
    QString getWorkspace() const;

    void setLanguage(const QString &language);
    QString getLanguage() const;

    void start();

signals:
    void parseDone(bool result);

private slots:
    void errorOccurred(QProcess::ProcessError err);
    void finished(int exitCode, QProcess::ExitStatus status);
    void redirectOut();
    void redirectErr();

private:
    QString getPython();

    SymbolParseArgs processArgs;
    QString pythonCmd;
};

#endif // SYMBOLPARSER_H
