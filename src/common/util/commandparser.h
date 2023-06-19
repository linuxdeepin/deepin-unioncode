/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
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
