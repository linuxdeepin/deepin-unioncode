// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QVariantMap>
#include <QSharedPointer>

enum ToolType {
    CommandTool = 0,
    CombinationTool
};

enum OutputOption {
    Ignore,
    ShowInApplicationOutput
};

struct ToolInfo
{
    QString id;
    QString displyGroup;
    QString name;
    QString description;
    int type { CommandTool };
    QString command;
    QString arguments;
    QString workingDirectory;
    int outputOption { ShowInApplicationOutput };
    int errorOutputOption { ShowInApplicationOutput };
    bool addToToolbar { false };
    QString icon { "binarytools_default" };
    QVariantMap environment;

    ToolInfo() = default;
    explicit ToolInfo(const ToolInfo &other)
        : id(other.id),
          displyGroup(other.displyGroup),
          name(other.name),
          description(other.description),
          type(other.type),
          command(other.command),
          arguments(other.arguments),
          workingDirectory(other.workingDirectory),
          outputOption(other.outputOption),
          errorOutputOption(other.errorOutputOption),
          addToToolbar(other.addToToolbar),
          icon(other.icon),
          environment(other.environment)
    {
    }

    bool isValid() const { return !name.isEmpty(); }
    inline bool operator==(const ToolInfo &other) const { return id == other.id; }
    ToolInfo &operator=(const ToolInfo &other)
    {
        id = other.id;
        displyGroup = other.displyGroup;
        name = other.name;
        description = other.description;
        type = other.type;
        command = other.command;
        arguments = other.arguments;
        workingDirectory = other.workingDirectory;
        outputOption = other.outputOption;
        errorOutputOption = other.errorOutputOption;
        addToToolbar = other.addToToolbar;
        icon = other.icon;
        environment = other.environment;

        return *this;
    }
};

#endif   // CONSTANTS_H
