// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
