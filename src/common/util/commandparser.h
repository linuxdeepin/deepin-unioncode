// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QCommandLineParser;
class QCoreApplication;
class QCommandLineOption;
QT_END_NAMESPACE

class CommandParser : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CommandParser)

public:
    enum CommandModel {
        Normal,
        CommandLine
    };

    static CommandParser &instance();

    bool isSet(const QString &name) const;
    QString value(const QString &name) const;
    void process();
    void process(const QStringList &arguments);
    void setModel(CommandModel model);
    CommandModel getModel();
    bool isBuildModel();

private:
    void initialize();
    void initOptions();
    void addOption(const QCommandLineOption &option);

    QStringList positionalArguments() const;
    QStringList unknownOptionNames() const;

private:
    explicit CommandParser(QObject *parent = nullptr);

private:
    QCommandLineParser *commandParser = nullptr;
    CommandModel commandModel = CommandModel::Normal;
};

#endif   // COMMANDPARSER_H
